#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <assert.h>
#include "parep_mpi_ckpt_coordinator.h"

#define SERVERPORT 2579
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 64
#define THREAD_POOL_SIZE 16

#define QTHREAD_POOL_SIZE 4

#define LID_HASH_KEYS 10
#define RKEY_HASH_KEYS 20
#define QP_HASH_KEYS 10

uint16_t current_lid = 0;
uint32_t current_qp_num = 0;
uint32_t current_pd_id = 0;
uint32_t current_rkey = 0;

node_t *head = NULL;
node_t *tail = NULL;

qnode_t *qhead = NULL;
qnode_t *qtail = NULL;

lid_pair_t *lid_map[LID_HASH_KEYS];
rkey_pair_t *rkey_map[RKEY_HASH_KEYS];
qp_pair_t *qp_map[QP_HASH_KEYS];

void enqueue (int *client_socket) {
	node_t *newnode = malloc(sizeof(node_t));
	newnode->client_socket = client_socket;
	newnode->next = NULL;
	if(tail == NULL) {
		head = newnode;
	} else {
		tail->next = newnode;
	}
	tail = newnode;
}

void qenqueue(int *client_socket, recv_data_t *rdata) {
	qnode_t *newnode = malloc(sizeof(qnode_t));
	newnode->client_socket = client_socket;
	newnode->rdata = rdata;
	newnode->next = NULL;
	if(qtail == NULL) {
		qhead = newnode;
	} else {
		qtail->next = newnode;
	}
	qtail = newnode;
}

int *dequeue() {
	if (head == NULL) {
		return NULL;
	} else {
		int *result = head->client_socket;
		node_t *temp = head;
		head = head->next;
		if(head == NULL) {tail = NULL;}
		free(temp);
		return result;
	}
}

qnode_t *qdequeue() {
	if (qhead == NULL) {
		return NULL;
	} else {
		qnode_t *result = qhead;
		qhead = qhead->next;
		if(qhead == NULL) {qtail = NULL;}
		return result;
	}
}

int lid_hash(uint16_t key) {
	return key % LID_HASH_KEYS;
}

int rkey_hash(uint32_t key_rkey, uint32_t key_pd_id) {
	return (key_rkey+key_pd_id) % RKEY_HASH_KEYS;
}

int qp_hash(uint32_t key) {
	return key % QP_HASH_KEYS;
}

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_t qthread_pool[QTHREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

pthread_mutex_t qmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t qcondition_var = PTHREAD_COND_INITIALIZER;

pthread_mutex_t lid_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qp_num_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pd_id_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rkey_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t lid_pair_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rkey_pair_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qp_pair_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t lid_pair_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t rkey_pair_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t qp_pair_cond = PTHREAD_COND_INITIALIZER;

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void *handle_connection(void *p_client_socket);
void handle_query_cmd(int, recv_data_t *);
int check(int exp, const char *msg);
void *thread_function(void *arg);
void *qthread_function(void *arg);

int main(int argc, char **argv) {
	int server_socket, client_socket, addr_size;
	SA_IN server_addr, client_addr;
	
	for(int i = 0 ; i < LID_HASH_KEYS; i++) {
		lid_map[i] = (lid_pair_t *)NULL;
	}
	for(int i = 0 ; i < RKEY_HASH_KEYS; i++) {
		rkey_map[i] = (rkey_pair_t *)NULL;
	}
	for(int i = 0 ; i < QP_HASH_KEYS; i++) {
		qp_map[i] = (qp_pair_t *)NULL;
	}
	
	for (int i = 0; i < THREAD_POOL_SIZE; i++) {
		pthread_create(&thread_pool[i],NULL,thread_function,NULL);
	}
	
	for (int i = 0; i < QTHREAD_POOL_SIZE; i++) {
		pthread_create(&qthread_pool[i],NULL,qthread_function,NULL);
	}
	
	check((server_socket = socket(AF_INET,SOCK_STREAM,0)),"Failed to create socket");
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(SERVERPORT);
	
	check(bind(server_socket,(SA *)&server_addr,sizeof(server_addr)),"Bind Failed!");
	check(listen(server_socket,SERVER_BACKLOG),"Listen Failed!");
	
	while(true) {
		addr_size = sizeof(SA_IN);
		check(client_socket = accept(server_socket, (SA *)&client_addr, (socklen_t *)&addr_size),"accept failed");
		
		int *pclient = malloc(sizeof(int));
		*pclient = client_socket;
		
		pthread_mutex_lock(&mutex);
		enqueue(pclient);
		pthread_cond_signal(&condition_var);
		pthread_mutex_unlock(&mutex);
	}
	return 0;
}

int check(int exp, const char *msg) {
	if (exp == SOCKETERROR) {
		perror(msg);
		exit(1);
	}
	return exp;
}

void *thread_function(void *arg) {
	while(true) {
		int *pclient;
		pthread_mutex_lock(&mutex);
		if((pclient = dequeue()) == NULL) {
			pthread_cond_wait(&condition_var, &mutex);
			pclient = dequeue();
		}
		pthread_mutex_unlock(&mutex);
		if(pclient != NULL) {
			handle_connection(pclient);
		}
	}
}

void *qthread_function(void *arg) {
	while(true) {
		qnode_t *qn;
		pthread_mutex_lock(&qmutex);
		if((qn = qdequeue()) == NULL) {
			pthread_cond_wait(&qcondition_var, &qmutex);
			qn = qdequeue();
		}
		pthread_mutex_unlock(&qmutex);
		if(qn != NULL) {
			int client_socket = *(qn->client_socket);
			free(qn->client_socket);
			handle_query_cmd(client_socket,qn->rdata);
			free(qn);
		}
	}
}

uint16_t lid_add_pair(uint16_t virt_lid, uint16_t lid) {
	if(virt_lid == (uint16_t)-1) {
		pthread_mutex_lock(&lid_mutex);
		virt_lid = current_lid;
		current_lid++;
		pthread_mutex_unlock(&lid_mutex);
	} else {
		pthread_mutex_lock(&lid_mutex);
		if(virt_lid >= current_lid) current_lid = virt_lid + 1;
		pthread_mutex_unlock(&lid_mutex);
	}
	pthread_mutex_lock(&lid_pair_mutex);
	int hash = lid_hash(virt_lid);
	
	if(lid_map[hash] == NULL) {
		lid_pair_t *pair = (lid_pair_t *)malloc(sizeof(lid_pair_t));
		pair->virt_lid = virt_lid;
		pair->real_lid = lid;
		pair->next = (lid_pair_t *)NULL;
		lid_map[hash] = pair;
	} else {
		lid_pair_t *temp = lid_map[hash];
		if(temp->virt_lid == virt_lid) {
			temp->real_lid = lid;
		} else {
			while(temp->next != NULL) {
				temp = temp->next;
				if(temp->virt_lid == virt_lid) {
					temp->real_lid = lid;
					break;
				}
			}
			if(temp->next == NULL) {
				lid_pair_t *pair = (lid_pair_t *)malloc(sizeof(lid_pair_t));
				pair->virt_lid = virt_lid;
				pair->real_lid = lid;
				pair->next = (lid_pair_t *)NULL;
				temp->next = pair;
			}
		}
	}
	pthread_cond_broadcast(&lid_pair_cond);
	pthread_mutex_unlock(&lid_pair_mutex);
	
	return virt_lid;
}

uint16_t lid_get_pair(uint16_t virt_lid) {
	uint16_t real_lid = (uint16_t)-1;
	int hash = lid_hash(virt_lid);
	pthread_mutex_lock(&lid_pair_mutex);
	while(real_lid == (uint16_t)-1) {
		if(lid_map[hash] != NULL) {
			lid_pair_t *temp = lid_map[hash];
			while(temp != NULL) {
				if(temp->virt_lid == virt_lid) {
					real_lid = temp->real_lid;
					break;
				}
				temp = temp->next;
			}
			if(real_lid != (uint16_t)-1) break;
		}
		pthread_cond_wait(&lid_pair_cond, &lid_pair_mutex);
	}
	pthread_mutex_unlock(&lid_pair_mutex);
	return real_lid;
}

void lid_clean_maps() {
	pthread_mutex_lock(&lid_pair_mutex);
	for(int i = 0; i < LID_HASH_KEYS; i++) {
		lid_pair_t *pair = lid_map[i];
		while(pair != NULL) {
			lid_pair_t *curpair = pair;
			pair = pair->next;
			free(curpair);
		}
		lid_map[i] = NULL;
	}
	pthread_mutex_unlock(&lid_pair_mutex);
	pthread_mutex_lock(&lid_mutex);
	current_lid = (uint16_t)0;
	pthread_mutex_unlock(&lid_mutex);
}

uint32_t rkey_add_pair(uint32_t virt_rkey, uint32_t pd_id, uint32_t real_rkey) {
	if(virt_rkey == (uint32_t)-1) {
		pthread_mutex_lock(&rkey_mutex);
		virt_rkey = current_rkey;
		current_rkey++;
		pthread_mutex_unlock(&rkey_mutex);
	} else {
		pthread_mutex_lock(&rkey_mutex);
		if(virt_rkey >= current_rkey) current_rkey = virt_rkey + 1;
		pthread_mutex_unlock(&rkey_mutex);
		pthread_mutex_lock(&pd_id_mutex);
		if(pd_id >= current_pd_id) current_pd_id = pd_id + 1;
		pthread_mutex_unlock(&pd_id_mutex);
	}
	pthread_mutex_lock(&rkey_pair_mutex);
	
	int hash = rkey_hash(virt_rkey,pd_id);
	
	if(rkey_map[hash] == NULL) {
		rkey_pair_t *pair = (rkey_pair_t *)malloc(sizeof(rkey_pair_t));
		pair->virt_rkey = virt_rkey;
		pair->pd_id = pd_id;
		pair->real_rkey = real_rkey;
		pair->next = (rkey_pair_t *)NULL;
		rkey_map[hash] = pair;
	} else {
		rkey_pair_t *temp = rkey_map[hash];
		if((temp->virt_rkey == virt_rkey) && (temp->pd_id == pd_id)) {
			temp->real_rkey = real_rkey;
		} else {
			while(temp->next != NULL) {
				temp = temp->next;
				if((temp->virt_rkey == virt_rkey) && (temp->pd_id == pd_id)) {
					temp->real_rkey = real_rkey;
					break;
				}
			}
			if(temp->next == NULL) {
				rkey_pair_t *pair = (rkey_pair_t *)malloc(sizeof(rkey_pair_t));
				pair->virt_rkey = virt_rkey;
				pair->pd_id = pd_id;
				pair->real_rkey = real_rkey;
				pair->next = (rkey_pair_t *)NULL;
				temp->next = pair;
			}
		}
	}
	pthread_cond_broadcast(&rkey_pair_cond);
	pthread_mutex_unlock(&rkey_pair_mutex);
	
	return virt_rkey;
}

uint32_t rkey_get_pair(uint32_t virt_rkey, uint32_t pd_id) {
	uint32_t real_rkey = -1;
	int hash = rkey_hash(virt_rkey,pd_id);
	pthread_mutex_lock(&rkey_pair_mutex);
	while(real_rkey == -1) {
		if(rkey_map[hash] != NULL) {
			rkey_pair_t *temp = rkey_map[hash];
			while(temp != NULL) {
				if((temp->virt_rkey == virt_rkey) && (temp->pd_id == pd_id)) {
					real_rkey = temp->real_rkey;
					break;
				}
				temp = temp->next;
			}
			if(real_rkey != -1) break;
		}
		pthread_cond_wait(&rkey_pair_cond, &rkey_pair_mutex);
	}
	pthread_mutex_unlock(&rkey_pair_mutex);
	return real_rkey;
}

void rkey_clean_maps() {
	pthread_mutex_lock(&rkey_pair_mutex);
	for(int i = 0; i < RKEY_HASH_KEYS; i++) {
		rkey_pair_t *pair = rkey_map[i];
		while(pair != NULL) {
			rkey_pair_t *curpair = pair;
			pair = pair->next;
			free(curpair);
		}
		rkey_map[i] = NULL;
	}
	pthread_mutex_unlock(&rkey_pair_mutex);
	pthread_mutex_lock(&rkey_mutex);
	current_rkey = 0;
	pthread_mutex_unlock(&rkey_mutex);
	pthread_mutex_lock(&pd_id_mutex);
	current_pd_id = 0;
	pthread_mutex_unlock(&pd_id_mutex);
}

uint32_t qp_add_pair(uint32_t virt_qp_num, uint32_t real_qp_num, uint32_t pd_id) {
	if(virt_qp_num == (uint32_t)-1) {
		pthread_mutex_lock(&qp_num_mutex);
		virt_qp_num = current_qp_num;
		current_qp_num++;
		pthread_mutex_unlock(&qp_num_mutex);
	} else {
		pthread_mutex_lock(&qp_num_mutex);
		if(virt_qp_num >= current_qp_num) current_qp_num = virt_qp_num + 1;
		pthread_mutex_unlock(&qp_num_mutex);
		pthread_mutex_lock(&pd_id_mutex);
		if(pd_id >= current_pd_id) current_pd_id = pd_id + 1;
		pthread_mutex_unlock(&pd_id_mutex);
	}
	
	pthread_mutex_lock(&qp_pair_mutex);
	
	int hash = qp_hash(virt_qp_num);
	
	if(qp_map[hash] == NULL) {
		qp_pair_t *pair = (qp_pair_t *)malloc(sizeof(qp_pair_t));
		pair->virt_qp_num = virt_qp_num;
		pair->real_qp_num = real_qp_num;
		pair->pd_id = pd_id;
		pair->next = (qp_pair_t *)NULL;
		qp_map[hash] = pair;
	} else {
		qp_pair_t *temp = qp_map[hash];
		if((temp->virt_qp_num == virt_qp_num)) {
			temp->real_qp_num = real_qp_num;
			temp->pd_id = pd_id;
		} else {
			while(temp->next != NULL) {
				temp = temp->next;
				if((temp->virt_qp_num == virt_qp_num)) {
					temp->real_qp_num = real_qp_num;
					temp->pd_id = pd_id;
					break;
				}
			}
			if(temp->next == NULL) {
				qp_pair_t *pair = (qp_pair_t *)malloc(sizeof(qp_pair_t));
				pair->virt_qp_num = virt_qp_num;
				pair->real_qp_num = real_qp_num;
				pair->pd_id = pd_id;
				pair->next = (qp_pair_t *)NULL;
				temp->next = pair;
			}
		}
	}
	pthread_cond_broadcast(&qp_pair_cond);
	pthread_mutex_unlock(&qp_pair_mutex);
	
	return virt_qp_num;
}

uint32_t qp_get_pair(uint32_t virt_qp_num, uint32_t *pd_id) {
	uint32_t real_qp_num = -1;
	int hash = qp_hash(virt_qp_num);
	pthread_mutex_lock(&qp_pair_mutex);
	while(real_qp_num == -1) {
		if(qp_map[hash] != NULL) {
			qp_pair_t *temp = qp_map[hash];
			while(temp != NULL) {
				if(temp->virt_qp_num == virt_qp_num) {
					real_qp_num = temp->real_qp_num;
					*pd_id = temp->pd_id;
					break;
				}
				temp = temp->next;
			}
			if(real_qp_num != -1) break;
		}
		pthread_cond_wait(&qp_pair_cond, &qp_pair_mutex);
	}
	pthread_mutex_unlock(&qp_pair_mutex);
	return real_qp_num;
}

void qp_clean_maps() {
	pthread_mutex_lock(&qp_pair_mutex);
	for(int i = 0; i < QP_HASH_KEYS; i++) {
		qp_pair_t *pair = qp_map[i];
		while(pair != NULL) {
			qp_pair_t *curpair = pair;
			pair = pair->next;
			free(curpair);
		}
		qp_map[i] = NULL;
	}
	pthread_mutex_unlock(&qp_pair_mutex);
	pthread_mutex_lock(&qp_num_mutex);
	current_qp_num = 0;
	pthread_mutex_unlock(&qp_num_mutex);
}

void handle_store_cmd(int client_socket, recv_data_t *recv) {
	if(!strcmp(recv->id,"ib_lid")) {
		uint16_t virt_lid = *((uint16_t *)recv->buf);
		uint16_t lid = *((uint16_t *)(recv->buf + sizeof(uint16_t)));
		uint16_t vlid = lid_add_pair(virt_lid, lid);
		
		if(virt_lid == (uint16_t)-1) {
			write(client_socket, &vlid, sizeof(uint16_t));
		}
		close(client_socket);
	} else if(!strcmp(recv->id,"ib_qp")) {
		uint32_t virt_qp_num = *((uint32_t *)recv->buf);
		uint32_t real_qp_num = *((uint32_t *)(recv->buf + sizeof(uint32_t)));
		uint32_t pd_id = *((uint32_t *)(recv->buf + sizeof(uint32_t) + sizeof(uint32_t)));
		uint32_t vqpnum = qp_add_pair(virt_qp_num,real_qp_num,pd_id);
		
		if(virt_qp_num == (uint32_t)-1) {
			write(client_socket, &vqpnum, sizeof(uint32_t));
		}
		close(client_socket);
	} else if(!strcmp(recv->id,"ib_rkey")) {
		uint32_t virt_rkey = *((uint32_t *)recv->buf);
		uint32_t pd_id = *((uint32_t *)(recv->buf + sizeof(uint32_t)));
		uint32_t real_rkey = *((uint32_t *)(recv->buf + sizeof(uint32_t) + sizeof(uint32_t)));
		uint32_t vrkey = rkey_add_pair(virt_rkey,pd_id,real_rkey);
		
		if(virt_rkey == (uint32_t)-1) {
			write(client_socket, &vrkey, sizeof(uint32_t));
		}
		close(client_socket);
	} else {
		perror("Invalid id");
		exit(1);
	}
}

void handle_query_cmd(int client_socket, recv_data_t *recv) {
	if(!strcmp(recv->id,"ib_lid")) {
		uint16_t virt_lid = *((uint16_t *)recv->buf);
		uint16_t real_lid = lid_get_pair(virt_lid);
		
		write(client_socket, &real_lid, sizeof(uint16_t));
		close(client_socket);
	} else if(!strcmp(recv->id,"ib_qp")) {
		uint32_t virt_qp_num = *((uint32_t *)recv->buf);
		uint32_t pd_id;
		uint32_t real_qp_num = qp_get_pair(virt_qp_num, &pd_id);
		
		char buf[2*sizeof(uint32_t)];
		*((uint32_t *)buf) = real_qp_num;
		*((uint32_t *)(buf + sizeof(uint32_t))) = pd_id;
		
		write(client_socket, buf, 2*sizeof(uint32_t));
		close(client_socket);
	} else if(!strcmp(recv->id,"ib_rkey")) {
		uint32_t virt_rkey = *((uint32_t *)recv->buf);
		uint32_t pd_id = *((uint32_t *)(recv->buf + sizeof(uint32_t)));
		uint32_t real_rkey = rkey_get_pair(virt_rkey,pd_id);
		
		write(client_socket, &real_rkey, sizeof(uint32_t));
		close(client_socket);
	}
	free(recv);
}

void handle_get_pd_id(int client_socket, recv_data_t *recv) {
	uint32_t pd_id;
	pthread_mutex_lock(&pd_id_mutex);
	pd_id = current_pd_id;
	current_pd_id++;
	pthread_mutex_unlock(&pd_id_mutex);
	
	write(client_socket, &pd_id, sizeof(uint32_t));
	close(client_socket);
}

void handle_get_rkey(int client_socket, recv_data_t *recv) {
	uint32_t rkey;
	pthread_mutex_lock(&rkey_mutex);
	rkey = current_rkey;
	current_rkey++;
	pthread_mutex_unlock(&rkey_mutex);
	
	write(client_socket, &rkey, sizeof(uint32_t));
	close(client_socket);
}

void handle_clear_store(int client_socket) {
	lid_clean_maps();
	qp_clean_maps();
	rkey_clean_maps();
	
	uint32_t resp = 0;
	write(client_socket, &resp, sizeof(uint32_t));
	close(client_socket);
}

void *handle_connection(void *p_client_socket) {
	int client_socket = *((int *)p_client_socket);
	free(p_client_socket);
	
	recv_data_t *recv;
	
	char buffer[BUFFER_SIZE];
	size_t bytes_read;
	int msgsize = 0;
	while((bytes_read = read(client_socket,buffer+msgsize, sizeof(buffer)-msgsize-1)) > 0) {
		msgsize += bytes_read;
		if(msgsize > BUFFER_SIZE-1 || buffer[msgsize-1] == '\n') break;
	}
	check(bytes_read,"recv error");
	buffer[msgsize-1] = 0;
	
	recv = (recv_data_t *)malloc(sizeof(recv_data_t));
	
	recv->cmd = *((int *)buffer);
	
	switch(recv->cmd) {
		case CMD_STORE:
			memcpy(recv->id,buffer+sizeof(int),20);
			memcpy(recv->buf,buffer+sizeof(int)+20,msgsize - sizeof(int) - 20);
			handle_store_cmd(client_socket,recv);
			break;
		case CMD_QUERY:
			memcpy(recv->id,buffer+sizeof(int),20);
			memcpy(recv->buf,buffer+sizeof(int)+20,msgsize - sizeof(int) - 20);
			int *pclient = malloc(sizeof(int));
			*pclient = client_socket;
			pthread_mutex_lock(&qmutex);
			qenqueue(pclient,recv);
			pthread_cond_signal(&qcondition_var);
			pthread_mutex_unlock(&qmutex);
			break;
		case CMD_GET_PD_ID:
			handle_get_pd_id(client_socket,recv);
			break;
		case CMD_GET_RKEY:
			handle_get_rkey(client_socket,recv);
			break;
		case CMD_CLEAR_STORE:
			handle_clear_store(client_socket);
			break;
		case CMD_EXIT: {
			close(client_socket);
			free(recv);
			exit(0);
			break;
		}
	}
	free(recv);
	return NULL;
}