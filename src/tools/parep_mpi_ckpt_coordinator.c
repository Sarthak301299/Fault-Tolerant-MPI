#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <assert.h>
#include <poll.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include "parep_mpi_ckpt_coordinator.h"

ssize_t write_to_fd(int fd, void *buf, size_t count) {
	ssize_t ret;
	ssize_t bytes_written;
	size_t msgsize = 0;
	while((bytes_written = write(fd,(char*)buf+msgsize, count-msgsize)) > 0) {
		msgsize += bytes_written;
		if(msgsize >= count) break;
	}
	if(bytes_written >= 0) ret = msgsize;
	else ret = bytes_written;
	if((ret <= 0) && (errno == EPIPE)) {
		printf("%d: EPIPE detected. Marking failed if applicable ret %d\n",getpid(),ret);
		fflush(stdout);
		int local_terminated = 1;
		for(int j = 0; j < parep_mpi_node_size; j++) {
			int *pstate;
			if(isMainServer) pstate = &(global_proc_state[j]);
			else pstate = &(local_proc_state[j]);
			if(*pstate != PROC_TERMINATED) {
				if(client_socket[j] == fd) {
					if(!isMainServer) {
						*pstate = PROC_TERMINATED;
						parep_mpi_num_failed++;
						int dsock;
						check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
						int ret;
						do {
							ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
						} while(ret != 0);
						int cmd = CMD_PROC_STATE_UPDATE;
						int num_failed = 1;
						msgsize = 0;
						while((bytes_written = write(dsock,(&cmd)+msgsize, sizeof(int)-msgsize)) > 0) {
							msgsize += bytes_written;
							if(msgsize >= sizeof(int)) break;
						}
						msgsize = 0;
						while((bytes_written = write(dsock,(&num_failed)+msgsize, sizeof(int)-msgsize)) > 0) {
							msgsize += bytes_written;
							if(msgsize >= sizeof(int)) break;
						}
						char *proc_state_data_buf = (char *)malloc(((2*sizeof(int))+sizeof(pid_t)));
						*((int *)proc_state_data_buf) = parep_mpi_ranks[j];
						*((int *)(proc_state_data_buf+sizeof(int))) = *pstate;
						*((pid_t *)(proc_state_data_buf+(2*sizeof(int)))) = parep_mpi_pids[j];
						msgsize = 0;
						while((bytes_written = write(dsock,proc_state_data_buf+msgsize, ((2*sizeof(int))+sizeof(pid_t))-msgsize)) > 0) {
							msgsize += bytes_written;
							if(msgsize >= ((2*sizeof(int))+sizeof(pid_t))) break;
						}
						free(proc_state_data_buf);
						close(dsock);
					}
				}
			}
			if(*pstate != PROC_TERMINATED) {
				local_terminated = 0;
			}
		}
		if(local_terminated) {
			pthread_mutex_lock(&local_procs_term_mutex);
			local_procs_term = local_terminated;
			pthread_cond_signal(&local_procs_term_cond);
			pthread_mutex_unlock(&local_procs_term_mutex);
		}
	}
	return ret;
}

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

void qenqueue(int *client_socket) {
	qnode_t *newnode = malloc(sizeof(qnode_t));
	newnode->client_socket = client_socket;
	newnode->next = NULL;
	if(qtail == NULL) {
		qhead = newnode;
	} else {
		qtail->next = newnode;
	}
	qtail = newnode;
}

void propenqueue(void *buffer, size_t size) {
	propnode_t *newnode = malloc(sizeof(propnode_t));
	newnode->buffer = buffer;
	newnode->size = size;
	newnode->next = NULL;
	if(proptail == NULL) {
		prophead = newnode;
	} else {
		proptail->next = newnode;
	}
	proptail = newnode;
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

propnode_t *propdequeue() {
	if (prophead == NULL) {
		return NULL;
	} else {
		propnode_t *result = prophead;
		prophead = prophead->next;
		if(prophead == NULL) {proptail = NULL;}
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

void parep_mpi_ckpt_coordinator_init() {
	int addr_size;
	SA_IN server_addr, client_addr;
	int option = 1;
	
	parep_mpi_size = atoi(getenv("PAREP_MPI_SIZE"));
	parep_mpi_node_id = atoi(getenv("PAREP_MPI_NODE_ID"));
	parep_mpi_node_num = atoi(getenv("PAREP_MPI_NODE_NUM"));
	parep_mpi_node_size = atoi(getenv("PAREP_MPI_NODE_SIZE"));
	
	if((parep_mpi_node_num >= 4) && (parep_mpi_node_num <= 1024)) {
		int N2_next = parep_mpi_node_num - 1;
		N2_next |= N2_next >> 1;
		N2_next |= N2_next >> 2;
		N2_next |= N2_next >> 4;
		N2_next |= N2_next >> 8;
		N2_next |= N2_next >> 16;
		N2_next++;
		
		int power = 0;
		while(N2_next >>= 1) {
			power++;
		}
		int group_size_power = (power % 2 == 0) ? (power/2) : ((power/2) + 1);
		PAREP_MPI_NODE_GROUP_SIZE_MAX = 1 << group_size_power;
	}
	
	if((parep_mpi_node_num % PAREP_MPI_NODE_GROUP_SIZE_MAX) == 0)	parep_mpi_node_group_num = parep_mpi_node_num/PAREP_MPI_NODE_GROUP_SIZE_MAX;
	else parep_mpi_node_group_num = 1 + ((parep_mpi_node_num - (parep_mpi_node_num % PAREP_MPI_NODE_GROUP_SIZE_MAX))/PAREP_MPI_NODE_GROUP_SIZE_MAX);
	parep_mpi_node_group_id = parep_mpi_node_id/PAREP_MPI_NODE_GROUP_SIZE_MAX;
	parep_mpi_node_group_nodeid = parep_mpi_node_id%PAREP_MPI_NODE_GROUP_SIZE_MAX;
	parep_mpi_node_group_size = PAREP_MPI_NODE_GROUP_SIZE_MAX;
	if(parep_mpi_node_group_id == (parep_mpi_node_group_num-1)) {
		if(parep_mpi_node_num < (parep_mpi_node_group_size*parep_mpi_node_group_num)) {
			parep_mpi_node_group_size = parep_mpi_node_group_size - ((parep_mpi_node_group_size*parep_mpi_node_group_num) - parep_mpi_node_num);
		}
	}
	
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(COORDINATOR_PORT);
	srvaddr.sin_addr.s_addr = inet_addr(getenv("PAREP_MPI_MAIN_COORDINATOR_IP"));
	
	main_coordinator_addr.sin_family = AF_INET;
	main_coordinator_addr.sin_port = htons(DYN_COORDINATOR_PORT);
	main_coordinator_addr.sin_addr.s_addr = inet_addr(getenv("PAREP_MPI_MAIN_COORDINATOR_IP"));
	
	if(isMainServer) {
		coordinator_addr = (SA_IN *)malloc(sizeof(SA_IN) * parep_mpi_node_num);
		coordinator_name = (char **)malloc(sizeof(char *) * parep_mpi_node_num);
		coordinator_name[0] = (char *)malloc(HOST_NAME_MAX);
		group_coordinator_addr = (SA_IN *)malloc(sizeof(SA_IN) * parep_mpi_node_group_num);
		daemon_socket = (int *)malloc(sizeof(int) * parep_mpi_node_num);
		parep_mpi_node_sizes = (int *)malloc(sizeof(int) * parep_mpi_node_num);
		parep_mpi_node_group_ids = (int *)malloc(sizeof(int) * parep_mpi_node_num);
		parep_mpi_node_group_nodeids = (int *)malloc(sizeof(int) * parep_mpi_node_num);
		parep_mpi_node_group_sizes = (int *)malloc(sizeof(int) * parep_mpi_node_group_num);
		parep_mpi_node_group_nodesizes = (int *)malloc(sizeof(int) * parep_mpi_node_group_num);
		parep_mpi_node_group_leader_nodeids = (int *)malloc(sizeof(int) * parep_mpi_node_num);
		parep_mpi_all_ranks = (int *)malloc(sizeof(int) * parep_mpi_size);
		
		parep_mpi_num_inform_failed_node = (int *)malloc(sizeof(int) * parep_mpi_node_num);
		parep_mpi_num_inform_failed_node_group = (int *)malloc(sizeof(int) * parep_mpi_node_group_num);
		
		for(int i = 0; i < parep_mpi_node_group_num; i++) {
			parep_mpi_node_group_sizes[i] = 0;
			parep_mpi_node_group_nodesizes[i] = 0;
			parep_mpi_num_inform_failed_node_group[i] = 0;
		}
		rank_lims_all = (rlims *)malloc(sizeof(rlims) * parep_mpi_node_num);
		strcpy(coordinator_name[0],my_coordinator_name);
		coordinator_addr[0].sin_family = AF_INET;
		coordinator_addr[0].sin_port = htons(DYN_COORDINATOR_PORT);
		coordinator_addr[0].sin_addr.s_addr = inet_addr(getenv("PAREP_MPI_MAIN_COORDINATOR_IP"));
		group_coordinator_addr[0].sin_family = AF_INET;
		group_coordinator_addr[0].sin_port = htons(DYN_COORDINATOR_PORT);
		group_coordinator_addr[0].sin_addr.s_addr = inet_addr(getenv("PAREP_MPI_MAIN_COORDINATOR_IP"));
		daemon_socket[0] = -1;
		parep_mpi_node_sizes[0] = parep_mpi_node_size;
		parep_mpi_node_group_ids[0] = 0;
		parep_mpi_node_group_nodeids[0] = 0;
		rank_lims.start = 0;
		rank_lims.end = parep_mpi_node_size-1;
		rank_lims_all[0].start = rank_lims.start;
		rank_lims_all[0].end = rank_lims.end;
	} else if(parep_mpi_node_group_nodeid == 0) {
		group_coordinator_addr = (SA_IN *)malloc(sizeof(SA_IN) * parep_mpi_node_group_size);
		daemon_socket = (int *)malloc(sizeof(int) * parep_mpi_node_group_size);
		parep_mpi_node_sizes = (int *)malloc(sizeof(int) * parep_mpi_node_group_size);
	}
	client_socket = (int *)malloc(sizeof(int) * parep_mpi_node_size);
	parep_mpi_ranks = (int *)malloc(sizeof(int) * parep_mpi_node_size);
	parep_mpi_pids = (pid_t *)malloc(sizeof(pid_t) * parep_mpi_node_size);
	
	if(isMainServer) {
		parep_mpi_global_pids = (pid_t *)malloc(sizeof(pid_t) * parep_mpi_size);
		parep_mpi_inform_failed_check = (bool *)malloc(sizeof(bool) * parep_mpi_size);
		global_proc_state = (int *)malloc(sizeof(int) * parep_mpi_size);
		daemon_state = (int *)malloc(sizeof(int) * parep_mpi_node_num);
		group_daemon_state = (int *)malloc(sizeof(int) * parep_mpi_node_group_size);
		pthread_mutex_lock(&proc_state_mutex);
		for(int i = 0; i < parep_mpi_size; i++) {
			parep_mpi_inform_failed_check[i] = false;
			global_proc_state[i] = PROC_UNINITIALIZED;
		}
		for(int i = 1; i < parep_mpi_node_num; i++) {
			coordinator_name[i] = (char *)malloc(HOST_NAME_MAX);
			daemon_state[i] = PROC_UNINITIALIZED;
			parep_mpi_num_inform_failed_node[i] = 0;
			parep_mpi_node_group_leader_nodeids[i] = 0;
		}
		for(int i = 1; i < parep_mpi_node_group_size; i++) {
			group_daemon_state[i] = PROC_UNINITIALIZED;
		}
		pthread_mutex_unlock(&proc_state_mutex);
	} else {
		parep_mpi_inform_failed_check = (bool *)malloc(sizeof(bool) * parep_mpi_node_size);
		local_proc_state = (int *)malloc(sizeof(int) * parep_mpi_node_size);
		if(parep_mpi_node_group_nodeid == 0) group_daemon_state = (int *)malloc(sizeof(int) * parep_mpi_node_group_size);
		pthread_mutex_lock(&proc_state_mutex);
		for(int i = 0; i < parep_mpi_node_size; i++) {
			parep_mpi_inform_failed_check[i] = false;
			local_proc_state[i] = PROC_UNINITIALIZED;
		}
		if(parep_mpi_node_group_nodeid == 0) {
			for(int i = 1; i < parep_mpi_node_group_size; i++) {
				group_daemon_state[i] = PROC_UNINITIALIZED;
			}
		}
		pthread_mutex_unlock(&proc_state_mutex);
	}
	
	int reserve_bits = 0;
	if(parep_mpi_node_num == 1) {
		reserve_bits = reserve_bits + 1;
	} else {
		int temp = parep_mpi_node_num-1;
		while(temp > 0) {
			reserve_bits = reserve_bits + 1;
			temp = temp >> 1;
		}
	}
	int qp_shift_bits = 24 - reserve_bits;
	int rkey_shift_bits = 32 - reserve_bits;
	uint32_t qp_mask = ((uint32_t)(0x00ffffff)) >> reserve_bits;
	uint32_t rkey_mask = ((uint32_t)(0xffffffff)) >> reserve_bits;

	current_lid = ((uint16_t)parep_mpi_node_id) + 1;
	qp_num_lims[0] = (uint32_t)((uint32_t)(parep_mpi_node_id) << qp_shift_bits);
	qp_num_lims[1] = qp_num_lims[0] | qp_mask;
	pd_id_lims[0] = (uint32_t)((uint32_t)(parep_mpi_node_id) << rkey_shift_bits);
	pd_id_lims[1] = pd_id_lims[0] | rkey_mask;
	rkey_lims[0] = (uint32_t)((uint32_t)(parep_mpi_node_id) << rkey_shift_bits);
	rkey_lims[1] = rkey_lims[0] | rkey_mask;

	current_qp_num = qp_num_lims[0];
	current_pd_id = pd_id_lims[0];
	current_rkey = rkey_lims[0];
	
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
	
	for (int i = 0; i < PROPTHREAD_POOL_SIZE; i++) {
		pthread_create(&propthread_pool[i],NULL,propthread_function,NULL);
	}
	
	pthread_create(&empi_thread,NULL,empi_thread_function,NULL);
	if(isMainServer) pthread_create(&empi_exec_thread,NULL,empi_thread_function,NULL);
	
	if(isMainServer) {
		check((server_socket = socket(AF_INET,SOCK_STREAM,0)),"Failed to create socket");
		setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
		
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = htons(COORDINATOR_PORT);
		
		check(bind(server_socket,(SA *)&server_addr,sizeof(server_addr)),"Bind Failed!");
		check(listen(server_socket,parep_mpi_node_num - 1),"Listen Failed!");
		parep_mpi_node_group_sizes[0]++;
		parep_mpi_node_group_nodesizes[0] += parep_mpi_node_size;
		for(int i = 1; i < parep_mpi_node_num; i++) {
			addr_size = sizeof(SA_IN);
			int tmpsock;
			tmpsock = accept(server_socket, (SA *)&client_addr, (socklen_t *)&addr_size);
			if((tmpsock == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
				while((tmpsock == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
					rlim_nofile.rlim_cur *= 2;
					assert(rlim_nofile.rlim_cur <= rlim_nofile.rlim_max);
					setrlimit(RLIMIT_NOFILE,&rlim_nofile);
					tmpsock = accept(server_socket, (SA *)&client_addr, (socklen_t *)&addr_size);
				}
				check(tmpsock,"accept failed");
			} else check(tmpsock,"accept failed");
			size_t bytes_read;
			int msgsize = 0;
			char buffer[BUFFER_SIZE];
			while((bytes_read = read(tmpsock,buffer+msgsize, (sizeof(int)*4)-msgsize)) > 0) {
				msgsize += bytes_read;
				if(msgsize >= (sizeof(int)*4)) break;
			}
			check(bytes_read,"recv error");
			int node_id = *((int *)buffer);
			int node_size = *((int *)(buffer + sizeof(int)));
			int node_group_id = *((int *)(buffer + sizeof(int) + sizeof(int)));
			int node_group_nodeid = *((int *)(buffer + sizeof(int) + sizeof(int) + sizeof(int)));
			daemon_socket[node_id] = tmpsock;
			parep_mpi_node_sizes[node_id] = node_size;
			parep_mpi_node_group_ids[node_id] = node_group_id;
			parep_mpi_node_group_nodeids[node_id] = node_group_nodeid;
			
			parep_mpi_node_group_sizes[node_group_id]++;
			parep_mpi_node_group_nodesizes[node_group_id] += node_size;
			
			char IP_addr[256];
			msgsize = 0;
			while((bytes_read = read(tmpsock,IP_addr+msgsize, 256-msgsize)) > 0) {
				msgsize += bytes_read;
				if(msgsize >= 256) break;
			}
			check(bytes_read,"recv error Coordinator addr");
			coordinator_addr[node_id].sin_family = AF_INET;
			coordinator_addr[node_id].sin_port = htons(DYN_COORDINATOR_PORT);
			coordinator_addr[node_id].sin_addr.s_addr = inet_addr(IP_addr);
			if(node_group_nodeid == 0) {
				group_coordinator_addr[node_group_id].sin_family = AF_INET;
				group_coordinator_addr[node_group_id].sin_port = htons(DYN_COORDINATOR_PORT);
				group_coordinator_addr[node_group_id].sin_addr.s_addr = inet_addr(IP_addr);
			}
			msgsize = 0;
			char tmpname[HOST_NAME_MAX];
			while((bytes_read = read(tmpsock, tmpname+msgsize, HOST_NAME_MAX-msgsize)) > 0) {
				msgsize += bytes_read;
				if(msgsize >= HOST_NAME_MAX) break;
			}
			check(bytes_read,"recv error Coordinator name");
			strcpy(coordinator_name[node_id],tmpname);
		}
		for(int i = 1; i < parep_mpi_node_num; i++) {
			rank_lims_all[i].start = rank_lims_all[i-1].end + 1;
			rank_lims_all[i].end = rank_lims_all[i].start + parep_mpi_node_sizes[i] - 1;
			write_to_fd(daemon_socket[i],&rank_lims_all[i].start,sizeof(int));
			
			char IP_addr[256];
			strcpy(IP_addr,inet_ntoa(group_coordinator_addr[parep_mpi_node_group_ids[i]].sin_addr));
			write_to_fd(daemon_socket[i],IP_addr,256);
			
			pthread_mutex_lock(&proc_state_mutex);
			daemon_state[i] = PROC_RUNNING;
			pthread_mutex_unlock(&proc_state_mutex);
		}
		
		for(int i = 1; i < parep_mpi_node_num; i++) {
			if((parep_mpi_node_group_ids[i] != 0) && (parep_mpi_node_group_nodeids[i] == 0)) {
					write_to_fd(daemon_socket[i],&(coordinator_addr[i]),sizeof(SA_IN)*(parep_mpi_node_group_sizes[parep_mpi_node_group_ids[i]]));
					write_to_fd(daemon_socket[i],&(parep_mpi_node_sizes[i]),sizeof(int)*(parep_mpi_node_group_sizes[parep_mpi_node_group_ids[i]]));
			}
			if((parep_mpi_node_group_ids[i] != 0) && (parep_mpi_node_group_nodeids[i] != 0)) {
				close(daemon_socket[i]);
				daemon_socket[i] = -1;
			}
		}
	} else {
		check((daemon_client_socket = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
		int ret;
		do {
			ret = connect(daemon_client_socket,(struct sockaddr *)(&srvaddr),sizeof(srvaddr));
		} while(ret != 0);
		char buffer[2*sizeof(int)];
		*((int *)buffer) = parep_mpi_node_id;
		*((int *)(buffer + sizeof(int))) = parep_mpi_node_size;
		*((int *)(buffer + sizeof(int) + sizeof(int))) = parep_mpi_node_group_id;
		*((int *)(buffer + sizeof(int) + sizeof(int) + sizeof(int))) = parep_mpi_node_group_nodeid;
		write_to_fd(daemon_client_socket,buffer,4*sizeof(int));
		
		char IP_addr[256];
		strcpy(IP_addr,getenv("PAREP_MPI_COORDINATOR_IP"));
		write_to_fd(daemon_client_socket,IP_addr,256);
		write_to_fd(daemon_client_socket,my_coordinator_name,HOST_NAME_MAX);
		
		size_t bytes_read;
		int msgsize = 0;
		while((bytes_read = read(daemon_client_socket,buffer+msgsize, sizeof(int)-msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= (sizeof(int))) break;
		}
		check(bytes_read,"recv error ranklims");
		
		msgsize = 0;
		while((bytes_read = read(daemon_client_socket,IP_addr+msgsize, 256-msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= 256) break;
		}
		check(bytes_read,"recv error Gcoord addr main");
		
		rank_lims.start = *((int *)buffer);
		rank_lims.end = rank_lims.start + parep_mpi_node_size - 1;
		
		if(parep_mpi_node_group_id == 0) {
			assert(strcmp(IP_addr,getenv("PAREP_MPI_MAIN_COORDINATOR_IP")) == 0);
			group_main_coordinator_addr.sin_family = AF_INET;
			group_main_coordinator_addr.sin_addr.s_addr = inet_addr(IP_addr);
			group_main_coordinator_addr.sin_port = htons(DYN_COORDINATOR_PORT);
		} else {
			group_main_coordinator_addr.sin_family = AF_INET;
			group_main_coordinator_addr.sin_addr.s_addr = inet_addr(IP_addr);
			group_main_coordinator_addr.sin_port = htons(COORDINATOR_GROUP_PORT);
			if(parep_mpi_node_group_nodeid == 0) {
				assert(strcmp(IP_addr,getenv("PAREP_MPI_COORDINATOR_IP")) == 0);
				msgsize = 0;
				while((bytes_read = read(daemon_client_socket,((char *)group_coordinator_addr)+msgsize, (sizeof(SA_IN) * parep_mpi_node_group_size)-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(SA_IN) * parep_mpi_node_group_size)) break;
				}
				check(bytes_read,"recv error Gcoord addr");
				msgsize = 0;
				while((bytes_read = read(daemon_client_socket,((char *)parep_mpi_node_sizes)+msgsize, (sizeof(int) * parep_mpi_node_group_size)-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int) * parep_mpi_node_group_size)) break;
				}
				check(bytes_read,"recv error Gcoord sizes");
				
				parep_mpi_node_group_nodesize = 0;
				for(int i = 0; i < parep_mpi_node_group_size; i++) {
					parep_mpi_node_group_nodesize += parep_mpi_node_sizes[i];
				}
				
				check((server_group_socket = socket(AF_INET,SOCK_STREAM,0)),"Failed to create socket");
				setsockopt(server_group_socket,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
				
				server_addr.sin_family = AF_INET;
				server_addr.sin_addr.s_addr = INADDR_ANY;
				server_addr.sin_port = htons(COORDINATOR_GROUP_PORT);
				
				check(bind(server_group_socket,(SA *)&server_addr,sizeof(server_addr)),"Bind Failed!");
				check(listen(server_group_socket,parep_mpi_node_group_size - 1),"Listen Failed!");
				
				daemon_socket[0] = -1;
				
				for(int i = 1; i < parep_mpi_node_group_size; i++) {
					addr_size = sizeof(SA_IN);
					int tmpsock;
					tmpsock = accept(server_group_socket, (SA *)&client_addr, (socklen_t *)&addr_size);
					if((tmpsock == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
						while((tmpsock == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
							rlim_nofile.rlim_cur *= 2;
							assert(rlim_nofile.rlim_cur <= rlim_nofile.rlim_max);
							setrlimit(RLIMIT_NOFILE,&rlim_nofile);
							tmpsock = accept(server_group_socket, (SA *)&client_addr, (socklen_t *)&addr_size);
						}
						check(tmpsock,"accept failed");
					} else check(tmpsock,"accept failed");
					size_t bytes_read;
					int msgsize = 0;
					char buffer[BUFFER_SIZE];
					while((bytes_read = read(tmpsock,buffer+msgsize, (sizeof(int))-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					check(bytes_read,"recv error gnid");
					int group_nodeid = *((int *)buffer);
					daemon_socket[group_nodeid] = tmpsock;
				}
			} else {
				close(daemon_client_socket);
				check((daemon_client_socket = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int ret;
				do {
					ret = connect(daemon_client_socket,(struct sockaddr *)(&group_main_coordinator_addr),sizeof(group_main_coordinator_addr));
				} while(ret != 0);
				write_to_fd(daemon_client_socket,&parep_mpi_node_group_nodeid,sizeof(int));
			}
			group_main_coordinator_addr.sin_port = htons(DYN_COORDINATOR_PORT);
		}
	}
	
	/*stdout_pipe = (int **)malloc(sizeof(int *) * parep_mpi_node_size);
	stderr_pipe = (int **)malloc(sizeof(int *) * parep_mpi_node_size);
	if(isMainServer) {
		if(pipe(stdin_pipe) < 0) {
			exit(1);
		}
	}
	for(int i = 0; i < parep_mpi_node_size; i++) {
		stdout_pipe[i] = (int *)malloc(sizeof(int)*2);
		if(pipe(stdout_pipe[i]) < 0) {
			exit(1);
		}
		stderr_pipe[i] = (int *)malloc(sizeof(int)*2);
		if(pipe(stderr_pipe[i]) < 0) {
			exit(1);
		}
	}*/
	
	check((daemon_server_socket = socket(AF_UNIX,SOCK_STREAM,0)),"Failed to create socket");
	setsockopt(daemon_server_socket,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
	
	struct sockaddr_un daemon_sock_addr;
	memset(&daemon_sock_addr,0,sizeof(daemon_sock_addr));
	daemon_sock_addr.sun_family = AF_UNIX;
	strcpy(daemon_sock_addr.sun_path,"#LocalServer");
	daemon_sock_addr.sun_path[0] = 0;
	
	check(bind(daemon_server_socket,(SA *)&daemon_sock_addr,sizeof(daemon_sock_addr)),"Bind Failed!");
	check(listen(daemon_server_socket,parep_mpi_node_size),"Listen Failed!");
	int rank = rank_lims.start;
	for(int i = 0; i < parep_mpi_node_size; i++) {
		addr_size = sizeof(SA_IN);
		int tmpsock;
		tmpsock = accept(daemon_server_socket, (SA *)&client_addr, (socklen_t *)&addr_size);
		if((tmpsock == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
			while((tmpsock == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
				rlim_nofile.rlim_cur *= 2;
				assert(rlim_nofile.rlim_cur <= rlim_nofile.rlim_max);
				setrlimit(RLIMIT_NOFILE,&rlim_nofile);
				tmpsock = accept(daemon_server_socket, (SA *)&client_addr, (socklen_t *)&addr_size);
			}
			check(tmpsock,"accept failed");
		} else check(tmpsock,"accept failed");
		size_t bytes_read;
		int msgsize = 0;
		char buffer[BUFFER_SIZE];
		while((bytes_read = read(tmpsock,buffer+msgsize, sizeof(pid_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(pid_t)) break;
		}
		check(bytes_read,"recv error lproc");
		pid_t pid = *((pid_t *)(buffer));
		assert((rank >= rank_lims.start) && (rank <= rank_lims.end));
		int node_rank = rank - rank_lims.start;
		client_socket[node_rank] = tmpsock;
		parep_mpi_ranks[node_rank] = rank;
		parep_mpi_pids[node_rank] = pid;
		
		*((int *)buffer) = rank;
		*((int *)(buffer + sizeof(int))) = node_rank;
		*((int *)(buffer + (2*sizeof(int)))) = parep_mpi_node_id;
		*((int *)(buffer + (3*sizeof(int)))) = parep_mpi_node_num;
		*((int *)(buffer + (4*sizeof(int)))) = parep_mpi_node_size;
		*((pid_t *)(buffer + (5*sizeof(int)))) = getpid();
		write_to_fd(client_socket[node_rank],buffer,(5*sizeof(int)) + sizeof(pid_t));
		
		/*write_to_fd(client_socket[node_rank],&(stdout_pipe[node_rank]),sizeof(int));
		write_to_fd(client_socket[node_rank],&(stderr_pipe[node_rank]),sizeof(int));
		if(rank == 0) write_to_fd(client_socket[node_rank],&stdin_pipe,sizeof(int));*/
		
		struct msghdr mhdr;
		struct iovec iov;
		union
		{
			struct cmsghdr cmhdr;
			char cntrl[CMSG_SPACE(sizeof(int))];
		} cntrl_unix;
		
		char tmp = '!';
		iov.iov_base = &tmp;
		iov.iov_len = sizeof(char);

		mhdr.msg_name = NULL;
		mhdr.msg_namelen = 0;
		mhdr.msg_iov = &iov;
		mhdr.msg_iovlen = 1;
		mhdr.msg_control = cntrl_unix.cntrl;
		mhdr.msg_controllen = sizeof(cntrl_unix.cntrl);

		cntrl_unix.cmhdr.cmsg_len = CMSG_LEN(sizeof(int));
		cntrl_unix.cmhdr.cmsg_level = SOL_SOCKET;
		cntrl_unix.cmhdr.cmsg_type = SCM_RIGHTS;

		*((int *) CMSG_DATA(CMSG_FIRSTHDR(&mhdr))) = fileno(stdout);/*stdout_pipe[node_rank][1];*/
		int size = sendmsg(client_socket[node_rank],&mhdr,0);
		if(size < 0) {
			printf("Error sending size for STDOUT %d\n",size);
			exit(1);
		}
		*((int *) CMSG_DATA(CMSG_FIRSTHDR(&mhdr))) = fileno(stderr);/*stderr_pipe[node_rank][1];*/
		size = sendmsg(client_socket[node_rank],&mhdr,0);
		if(size < 0) {
			printf("Error sending size for STDERR %d\n",size);
			exit(1);
		}

		if(rank == 0) {
			*((int *) CMSG_DATA(CMSG_FIRSTHDR(&mhdr))) = fileno(stdin);/*stdin_pipe[0];*/
			size = sendmsg(client_socket[node_rank],&mhdr,0);
			if(size < 0) {
				printf("Error sending size for STDERR %d\n",size);
				exit(1);
			}
		}
		
		/*if(isMainServer) {
			close(stdin_pipe[0]);
		}
		for(int i = 0; i < parep_mpi_node_size; i++) {
			close(stdout_pipe[i][1]);
			close(stderr_pipe[i][1]);
		}*/
		
		rank++;
	}
	
	if(isMainServer) {
		pthread_mutex_lock(&proc_state_mutex);
		for(int i = rank_lims_all[0].start; i <= rank_lims_all[0].end; i++) {
			global_proc_state[i] = PROC_RUNNING;
			parep_mpi_global_pids[i] = parep_mpi_pids[i];
			parep_mpi_all_ranks[i] = parep_mpi_ranks[i];
		}
		pthread_mutex_unlock(&proc_state_mutex);
		size_t bytes_read;
		int msgsize = 0;
		char buffer[BUFFER_SIZE];
		for(int i = 1; i < parep_mpi_node_num; i++) {
			if(((parep_mpi_node_group_ids[i] == 0) || (parep_mpi_node_group_nodeids[i] == 0))) {
				msgsize = 0;
				while((bytes_read = read(daemon_socket[i],buffer+msgsize, sizeof(int)-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int))) break;
				}
				check(bytes_read,"recv error lpinit");
				int lpinit = *((int *)buffer);
				assert(lpinit == 1);
				msgsize = 0;
				while((bytes_read = read(daemon_socket[i],(&(parep_mpi_global_pids[rank_lims_all[i].start])) + msgsize, (sizeof(pid_t) * (rank_lims_all[i].end - rank_lims_all[i].start + 1))-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(pid_t) * (rank_lims_all[i].end - rank_lims_all[i].start + 1))) break;
				}
				check(bytes_read,"recv error gpids");
				
				msgsize = 0;
				while((bytes_read = read(daemon_socket[i],(&(parep_mpi_all_ranks[rank_lims_all[i].start])) + msgsize, (sizeof(int) * (rank_lims_all[i].end - rank_lims_all[i].start + 1))-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int) * (rank_lims_all[i].end - rank_lims_all[i].start + 1))) break;
				}
				check(bytes_read,"recv error granks");
				
				if(parep_mpi_node_group_nodeids[i] == 0) {
					int num_ranks = 0;
					for(int j = 1; (i+j < parep_mpi_node_num) && (parep_mpi_node_group_nodeids[j] != 0); j++) {
						msgsize = 0;
						while((bytes_read = read(daemon_socket[i],(&(parep_mpi_global_pids[rank_lims_all[i+j].start])) + msgsize, (sizeof(pid_t) * (rank_lims_all[i+j].end - rank_lims_all[i+j].start + 1))-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(pid_t) * (rank_lims_all[i+j].end - rank_lims_all[i+j].start + 1))) break;
						}
						check(bytes_read,"recv error ghpids");
						
						msgsize = 0;
						while((bytes_read = read(daemon_socket[i],(&(parep_mpi_all_ranks[rank_lims_all[i+j].start])) + msgsize, (sizeof(int) * (rank_lims_all[i+j].end - rank_lims_all[i+j].start + 1))-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(int) * (rank_lims_all[i+j].end - rank_lims_all[i+j].start + 1))) break;
						}
						check(bytes_read,"recv error ghranks");
					}
				}
				
				pthread_mutex_lock(&proc_state_mutex);
				for(int j = rank_lims_all[i].start; j <= rank_lims_all[i].end; j++) {
					global_proc_state[j] = PROC_RUNNING;
				}
				if(parep_mpi_node_group_nodeids[i] == 0) {
					for(int k = 1; (k < parep_mpi_node_num) && (parep_mpi_node_group_nodeids[k] != 0); k++) {
						for(int j = rank_lims_all[i+k].start; j <= rank_lims_all[i+k].end; j++) {
							global_proc_state[j] = PROC_RUNNING;
						}
					}
				}
				pthread_mutex_unlock(&proc_state_mutex);
			}
		}
	} else {
		pthread_mutex_lock(&proc_state_mutex);
		for(int i = 0; i < parep_mpi_node_size; i++) {
			local_proc_state[i] = PROC_RUNNING;
		}
		pthread_mutex_unlock(&proc_state_mutex);
		int local_procs_initialized = 1;
		write_to_fd(daemon_client_socket,&local_procs_initialized,sizeof(int));
		write_to_fd(daemon_client_socket,parep_mpi_pids,parep_mpi_node_size*sizeof(pid_t));
		write_to_fd(daemon_client_socket,parep_mpi_ranks,parep_mpi_node_size*sizeof(int));
		if(parep_mpi_node_group_nodeid == 0) {
			size_t bytes_read;
			int msgsize = 0;
			char buffer[BUFFER_SIZE];
			for(int i = 1; i < parep_mpi_node_group_size; i++) {
				msgsize = 0;
				while((bytes_read = read(daemon_socket[i],buffer+msgsize, sizeof(int)-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int))) break;
				}
				check(bytes_read,"recv error llpinit");
				int lpinit = *((int *)buffer);
				assert(lpinit == 1);
				group_daemon_state[i] = PROC_RUNNING;
				msgsize = 0;
				pid_t *pidbuf = (pid_t *)malloc(sizeof(pid_t)*parep_mpi_node_sizes[i]);
				while((bytes_read = read(daemon_socket[i],((char *)pidbuf) + msgsize, (sizeof(pid_t) * parep_mpi_node_sizes[i])-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= ((sizeof(pid_t) * parep_mpi_node_sizes[i])-msgsize)) break;
				}
				check(bytes_read,"recv error pidbuf");
				
				msgsize = 0;
				int *rankbuf = (int *)malloc(sizeof(int)*parep_mpi_node_sizes[i]);
				while((bytes_read = read(daemon_socket[i],((char *)rankbuf) + msgsize, (sizeof(int) * parep_mpi_node_sizes[i])-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= ((sizeof(int) * parep_mpi_node_sizes[i])-msgsize)) break;
				}
				check(bytes_read,"recv error rankbuf");
				
				write_to_fd(daemon_client_socket,pidbuf,parep_mpi_node_sizes[i]*sizeof(pid_t));
				write_to_fd(daemon_client_socket,rankbuf,parep_mpi_node_sizes[i]*sizeof(int));
				free(pidbuf);
				free(rankbuf);
			}
		}
	}

	main_thread = pthread_self();
	
	if(!isMainServer) {
		pthread_create(&server_poller,NULL,polling_server,NULL);
	} else {
		pthread_create(&ckpt_thread,NULL,run_ckpt_timer,NULL);
	}
	
	//pthread_create(&iopoller,NULL,poll_iopipes,NULL);
	
	int csock;
	
	check((dyn_server_sock = socket(AF_INET,SOCK_STREAM,0)),"Failed to create socket");
	setsockopt(dyn_server_sock,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(DYN_COORDINATOR_PORT);
	
	check(bind(dyn_server_sock,(SA *)&server_addr,sizeof(server_addr)),"Bind Failed!");
	check(listen(dyn_server_sock,SERVER_BACKLOG),"Listen Failed!");
	
	while(true) {
		addr_size = sizeof(SA_IN);
		pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
		while(parep_mpi_num_dyn_socks >= PAREP_MPI_MAX_DYN_SOCKS) {
			pthread_cond_wait(&parep_mpi_num_dyn_socks_cond,&parep_mpi_num_dyn_socks_mutex);
		}
		pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
		csock = accept(dyn_server_sock, (SA *)&client_addr, (socklen_t *)&addr_size);
		if(csock == -1 && errno == EBADF) {
			if(exit_cmd_recvd == 1) {
				if(!isMainServer) exit(0);
				if(parep_mpi_completed) exit(RET_COMPLETED);
				else exit(RET_INCOMPLETE);
			}
		}
		if((csock == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
			while((csock == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
				rlim_nofile.rlim_cur *= 2;
				assert(rlim_nofile.rlim_cur <= rlim_nofile.rlim_max);
				setrlimit(RLIMIT_NOFILE,&rlim_nofile);
				csock = accept(dyn_server_sock, (SA *)&client_addr, (socklen_t *)&addr_size);
				//csock = accept(dyn_server_sock, (SA *)&client_addr, (socklen_t *)&addr_size);
				if(csock == -1 && errno == EBADF) {
					if(exit_cmd_recvd == 1) {
						if(!isMainServer) exit(0);
						if(parep_mpi_completed) exit(RET_COMPLETED);
						else exit(RET_INCOMPLETE);
					}
				}
			}
			check(csock,"accept failed");
		} else check(csock,"accept failed");
		pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
		parep_mpi_num_dyn_socks++;
		pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
		pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
		
		int *pclient = malloc(sizeof(int));
		*pclient = csock;
		
		pthread_mutex_lock(&mutex);
		enqueue(pclient);
		pthread_cond_signal(&condition_var);
		pthread_mutex_unlock(&mutex);
	}
}

int check(int exp, const char *msg) {
	if (exp == SOCKETERROR) {
		perror(msg);
		exit(1);
	}
	return exp;
}

/*void *poll_iopipes(void *arg) {
	struct pollfd *iopfds;
	nfds_t nfds;
	int pollret;
	if(isMainServer) {
		nfds = (2*parep_mpi_node_size) + 1;
		iopfds = (struct pollfd *)malloc(sizeof(struct pollfd) * nfds);
		iopfds[2*parep_mpi_node_size].fd = fileno(stdin);
		iopfds[2*parep_mpi_node_size].events = POLLIN;
	} else {
		nfds = 2*parep_mpi_node_size;
		iopfds = (struct pollfd *)malloc(sizeof(struct pollfd) * nfds);
	}
	for(int i = 0; i < parep_mpi_node_size; i++) {
		iopfds[i].fd = stdout_pipe[i][0];
		iopfds[i].events = POLLIN;
		iopfds[i+parep_mpi_node_size].fd = stderr_pipe[i][0];
		iopfds[i+parep_mpi_node_size].events = POLLIN;
	}
	while(true) {
		pollret = poll(iopfds,nfds,-1);
		for(int i = 0; i < nfds; i++) {
			if(iopfds[i].revents != 0) {
				if((iopfds[i].revents & POLLHUP) || (iopfds[i].revents & POLLERR)) {
					iopfds[i].fd = -1;
				} else {
					if(i < parep_mpi_node_size) {
						if(iopfds[i].revents & POLLIN) {
							char buf[1024];
							size_t bytes_read;
							int msgsize = 0;
							
						}
					} else if(i < (2*parep_mpi_node_size)) {
						if(iopfds[i].revents & POLLIN) {
						}
					}	else {
						if(iopfds[i].revents & POLLIN) {
						}
					}
				}
			}
		}
	}
}*/

void *run_ckpt_timer(void *arg) {
	if(ckpt_interval >= 0) {
		while(true) {
			pthread_mutex_lock(&ckpt_mutex);
			while(ckpt_active == 0) {
				pthread_cond_wait(&ckpt_cond,&ckpt_mutex);
			}
			pthread_mutex_unlock(&ckpt_mutex);
			sleep(ckpt_interval);
			pthread_mutex_lock(&ckpt_mutex);
			ckpt_active = 0;
			pthread_cond_broadcast(&ckpt_cond);
			pthread_mutex_unlock(&ckpt_mutex);
			int ecr;
			pthread_mutex_lock(&exit_cmd_recvd_mutex);
			ecr = exit_cmd_recvd;
			pthread_mutex_unlock(&exit_cmd_recvd_mutex);
			if(ecr == 1) {
				pthread_exit(NULL);
			}
			
			pthread_mutex_lock(&rem_recv_running_mutex);
			while(rem_recv_running == 1) {
				pthread_cond_wait(&rem_recv_running_cond,&rem_recv_running_mutex);
			}
			pthread_mutex_unlock(&rem_recv_running_mutex);
			
			int fin_reached = 0;
			pthread_mutex_lock(&parep_mpi_finalize_reached_mutex);
			fin_reached = parep_mpi_finalize_reached;
			pthread_mutex_unlock(&parep_mpi_finalize_reached_mutex);
			if(!fin_reached) {
				int cmd = CMD_CREATE_CKPT;
				pthread_mutex_lock(&daemon_sock_mutex);
				for(int j = 1; j < parep_mpi_node_num; j++) {
					if((daemon_state[j] != PROC_TERMINATED) && ((parep_mpi_node_group_ids[j] == 0) || (parep_mpi_node_group_nodeids[j] == parep_mpi_node_group_leader_nodeids[j]))) {
						write_to_fd(daemon_socket[j],&cmd,sizeof(int));
					}
				}
				pthread_mutex_unlock(&daemon_sock_mutex);
				for(int j = 0; j < parep_mpi_node_size; j++) {
					if(global_proc_state[j] != PROC_TERMINATED) {
						kill(parep_mpi_pids[j],SIGUSR1);
					}
				}
			}
		}
	}
}

void get_group_server_from_main() {
	int dsock;
	check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
	int cmd = CMD_GET_NGROUP_LEADER;
	int ret;
	int old_leader;
	do {
		ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
	} while(ret != 0);
	write_to_fd(dsock,&cmd,sizeof(int));
	write_to_fd(dsock,&parep_mpi_node_group_id,sizeof(int));
	write_to_fd(dsock,&parep_mpi_node_group_nodeid,sizeof(int));
	write_to_fd(dsock,&parep_mpi_node_group_leader_nodeid,sizeof(int));
	
	int new_group_leader_nodeid;
	int msgsize = 0;
	size_t bytes_read;
	while((bytes_read = read(dsock,(&new_group_leader_nodeid)+msgsize, sizeof(int)-msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= (sizeof(int))) break;
	}
	old_leader = parep_mpi_node_group_leader_nodeid;
	parep_mpi_node_group_leader_nodeid = new_group_leader_nodeid;
	
	char IP_addr[256];
	msgsize = 0;
	while((bytes_read = read(dsock,IP_addr+msgsize, 256-msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= 256) break;
	}
	group_main_coordinator_addr.sin_family = AF_INET;
	group_main_coordinator_addr.sin_addr.s_addr = inet_addr(IP_addr);
	group_main_coordinator_addr.sin_port = htons(COORDINATOR_GROUP_PORT);
	if(new_group_leader_nodeid == parep_mpi_node_group_nodeid) {
		group_coordinator_addr = (SA_IN *)malloc(sizeof(SA_IN) * parep_mpi_node_group_size);
		daemon_socket = (int *)malloc(sizeof(int) * parep_mpi_node_group_size);
		parep_mpi_node_sizes = (int *)malloc(sizeof(int) * parep_mpi_node_group_size);
		group_daemon_state = (int *)malloc(sizeof(int) * parep_mpi_node_group_size);
		msgsize = 0;
		while((bytes_read = read(dsock,((char *)group_coordinator_addr)+msgsize, (sizeof(SA_IN) * parep_mpi_node_group_size)-msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= (sizeof(SA_IN) * parep_mpi_node_group_size)) break;
		}
		check(bytes_read,"recv error Gcoord addr");
		
		msgsize = 0;
		while((bytes_read = read(dsock,((char *)parep_mpi_node_sizes)+msgsize, (sizeof(int) * parep_mpi_node_group_size)-msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= (sizeof(int) * parep_mpi_node_group_size)) break;
		}
		check(bytes_read,"recv error Gcoord sizes");
		
		msgsize = 0;
		while((bytes_read = read(dsock,((char *)group_daemon_state)+msgsize, (sizeof(int) * parep_mpi_node_group_size)-msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= (sizeof(int) * parep_mpi_node_group_size)) break;
		}
		check(bytes_read,"recv error Dstate");
		
		group_daemon_state[old_leader] = PROC_TERMINATED;
		
		parep_mpi_node_group_nodesize = 0;
		for(int i = 0; i < parep_mpi_node_group_size; i++) {
			parep_mpi_node_group_nodesize += parep_mpi_node_sizes[i];
			daemon_socket[i] = -1;
		}
		parep_mpi_num_failed += parep_mpi_node_sizes[old_leader];
		
		int option = 1;
		check((server_group_socket = socket(AF_INET,SOCK_STREAM,0)),"Failed to create socket");
		setsockopt(server_group_socket,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
		
		SA_IN server_addr, client_addr;
		int addr_size;
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = htons(COORDINATOR_GROUP_PORT);
		
		check(bind(server_group_socket,(SA *)&server_addr,sizeof(server_addr)),"Bind Failed!");
		check(listen(server_group_socket,parep_mpi_node_group_size - 1),"Listen Failed!");
		
		pthread_mutex_lock(&proc_state_mutex);
		for(int i = 0; i < parep_mpi_node_group_size; i++) {
			if((i != parep_mpi_node_group_nodeid) && (group_daemon_state[i] != PROC_TERMINATED)) {
				addr_size = sizeof(SA_IN);
				int tmpsock;
				tmpsock = accept(server_group_socket, (SA *)&client_addr, (socklen_t *)&addr_size);
				if((tmpsock == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
					while((tmpsock == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
						rlim_nofile.rlim_cur *= 2;
						assert(rlim_nofile.rlim_cur <= rlim_nofile.rlim_max);
						setrlimit(RLIMIT_NOFILE,&rlim_nofile);
						tmpsock = accept(server_group_socket, (SA *)&client_addr, (socklen_t *)&addr_size);
					}
					check(tmpsock,"accept failed");
				} else check(tmpsock,"accept failed");
				size_t bytes_read;
				int msgsize = 0;
				char buffer[BUFFER_SIZE];
				while((bytes_read = read(tmpsock,buffer+msgsize, (sizeof(int))-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int))) break;
				}
				check(bytes_read,"recv error gnid");
				int group_nodeid = *((int *)buffer);
				daemon_socket[group_nodeid] = tmpsock;
			}
		}
		pthread_mutex_unlock(&proc_state_mutex);
		check((daemon_client_socket = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
		int ret;
		do {
			ret = connect(daemon_client_socket,(struct sockaddr *)(&srvaddr),sizeof(srvaddr));
		} while(ret != 0);
	} else {
		check((daemon_client_socket = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
		group_main_coordinator_addr.sin_port = htons(COORDINATOR_GROUP_PORT);
		int ret;
		do {
			ret = connect(daemon_client_socket,(struct sockaddr *)(&group_main_coordinator_addr),sizeof(group_main_coordinator_addr));
		} while(ret != 0);
		write_to_fd(daemon_client_socket,&parep_mpi_node_group_nodeid,sizeof(int));
		group_main_coordinator_addr.sin_port = htons(DYN_COORDINATOR_PORT);
	}
	
	close(dsock);
}

void *polling_server(void *arg) {
	nfds_t nfds;
	struct pollfd pfd;
	pfd.fd = daemon_client_socket;
	pfd.events = POLLIN;
	nfds = 1;
	int pollret;
	while(true) {
		pollret = poll(&pfd,nfds,-1);
		if (pfd.revents != 0) {
			if(pfd.revents & POLLIN) {
				int cmd;
				int msgsize = 0;
				size_t bytes_read;
				while((bytes_read = read(pfd.fd,(&cmd)+msgsize, sizeof(int)-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int))) break;
				}
				if(bytes_read == 0) {
					if(parep_mpi_node_group_id != 0) {
						close(daemon_client_socket);
						get_group_server_from_main();
						pfd.fd = daemon_client_socket;
						continue;
					}
				}
				if(cmd == CMD_CREATE_CKPT) {
					int fin_reached = 0;
					pthread_mutex_lock(&parep_mpi_finalize_reached_mutex);
					fin_reached = parep_mpi_finalize_reached;
					pthread_mutex_unlock(&parep_mpi_finalize_reached_mutex);
					if(!fin_reached) {
						if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
							pthread_mutex_lock(&daemon_sock_mutex);
							for(int j = 0; j < parep_mpi_node_group_size; j++) {
								if(daemon_socket[j] != -1) write_to_fd(daemon_socket[j],&cmd,sizeof(int));
							}
							pthread_mutex_unlock(&daemon_sock_mutex);
						}
						for(int j = 0; j < parep_mpi_node_size; j++) {
							if(local_proc_state[j] != PROC_TERMINATED) {
								kill(parep_mpi_pids[j],SIGUSR1);
							}
						}
					}
				} else if(cmd == CMD_MPI_INITIALIZED) {
					pthread_mutex_lock(&parep_mpi_initialized_mutex);
					if(parep_mpi_initialized == 0) {
						parep_mpi_initialized = 1;
						int cmd = CMD_MPI_INITIALIZED;
						if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
							pthread_mutex_lock(&daemon_sock_mutex);
							for(int j = 0; j < parep_mpi_node_group_size; j++) {
								if(daemon_socket[j] != -1) write_to_fd(daemon_socket[j],&cmd,sizeof(int));
							}
							pthread_mutex_unlock(&daemon_sock_mutex);
						}
						write_to_fd(empi_client_socket,&cmd,sizeof(int));
						pthread_mutex_lock(&client_sock_mutex);
						for(int j = 0; j < parep_mpi_node_size; j++) {
							if(isMainServer) {
								if(global_proc_state[j] != PROC_TERMINATED) {
									write_to_fd(client_socket[j],&cmd,sizeof(int));
								}
							} else {
								if(local_proc_state[j] != PROC_TERMINATED) {
									write_to_fd(client_socket[j],&cmd,sizeof(int));
								}
							}
						}
						pthread_mutex_unlock(&client_sock_mutex);
					}
					pthread_mutex_unlock(&parep_mpi_initialized_mutex);
				} else if(cmd == CMD_BARRIER) {
					int fail_ready;
					msgsize = 0;
					while((bytes_read = read(pfd.fd,(&fail_ready)+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					int cmd = CMD_BARRIER;
					pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
					//assert(parep_mpi_performing_barrier == 1);
					parep_mpi_performing_barrier = 0;
					pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
					parep_mpi_barrier_running = 0;
					pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
					if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
						pthread_mutex_lock(&daemon_sock_mutex);
						for(int j = 0; j < parep_mpi_node_group_size; j++) {
							if(daemon_socket[j] != -1) {
								write_to_fd(daemon_socket[j],&cmd,sizeof(int));
								write_to_fd(daemon_socket[j],&fail_ready,sizeof(int));
							}
						}
						pthread_mutex_unlock(&daemon_sock_mutex);
					}
					pthread_mutex_lock(&proc_state_mutex);
					pthread_mutex_lock(&client_sock_mutex);
					for(int j = 0; j < parep_mpi_node_size; j++) {
						if(isMainServer) {
							if(global_proc_state[j] != PROC_TERMINATED) {
								write_to_fd(client_socket[j],&cmd,sizeof(int));
								write_to_fd(client_socket[j],&fail_ready,sizeof(int));
							}
						} else {
							if(local_proc_state[j] != PROC_TERMINATED) {
								write_to_fd(client_socket[j],&cmd,sizeof(int));
								write_to_fd(client_socket[j],&fail_ready,sizeof(int));
							}
						}
					}
					pthread_mutex_unlock(&client_sock_mutex);
					pthread_mutex_unlock(&proc_state_mutex);
					pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
				} else if(cmd == CMD_REM_RECV) {
					int fin_reached = 0;
					pthread_mutex_lock(&parep_mpi_finalize_reached_mutex);
					fin_reached = parep_mpi_finalize_reached;
					pthread_mutex_unlock(&parep_mpi_finalize_reached_mutex);
					if(!fin_reached) {
						int cmd = CMD_REM_RECV;
						if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
							pthread_mutex_lock(&daemon_sock_mutex);
							for(int j = 0; j < parep_mpi_node_group_size; j++) {
								if(daemon_socket[j] != -1) write_to_fd(daemon_socket[j],&cmd,sizeof(int));
							}
							pthread_mutex_unlock(&daemon_sock_mutex);
						}
						pthread_mutex_lock(&client_sock_mutex);
						for(int j = 0; j < parep_mpi_node_size; j++) {
							if(isMainServer) {
								if(global_proc_state[j] != PROC_TERMINATED) {
									write_to_fd(client_socket[j],&cmd,sizeof(int));
								}
							} else {
								if(local_proc_state[j] != PROC_TERMINATED) {
									write_to_fd(client_socket[j],&cmd,sizeof(int));
								}
							}
						}
						pthread_mutex_unlock(&client_sock_mutex);
					}
				} else if(cmd == CMD_INFORM_PREDICT) {
					int targrank;
					time_t timestamp;
					msgsize = 0;
					while((bytes_read = read(pfd.fd,(&targrank)+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					msgsize = 0;
					while((bytes_read = read(pfd.fd,(&timestamp)+msgsize, sizeof(time_t)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(time_t))) break;
					}

					int exit_recvd;
					int local_term;
					pthread_mutex_lock(&exit_cmd_recvd_mutex);
					exit_recvd = exit_cmd_recvd;
					pthread_mutex_unlock(&exit_cmd_recvd_mutex);
					pthread_mutex_lock(&local_procs_term_mutex);
					local_term = local_procs_term;
					pthread_mutex_unlock(&local_procs_term_mutex);
					if((exit_recvd == 0) && (local_term == 0)) {
						pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
						if(parep_mpi_performing_barrier == 1) {
							pollret = poll(&pfd,nfds,-1);
							if (pfd.revents != 0) {
								if(pfd.revents & POLLIN) {
									int tempcmd;
									int tempmsgsize = 0;
									size_t temp_bytes_read;
									while((temp_bytes_read = read(pfd.fd,(&tempcmd)+tempmsgsize, sizeof(int)-tempmsgsize)) > 0) {
										tempmsgsize += temp_bytes_read;
										if(tempmsgsize >= (sizeof(int))) break;
									}
									assert(tempcmd == CMD_BARRIER);
									int fail_ready;
									tempmsgsize = 0;
									while((temp_bytes_read = read(pfd.fd,(&fail_ready)+tempmsgsize, sizeof(int)-tempmsgsize)) > 0) {
										tempmsgsize += temp_bytes_read;
										if(tempmsgsize >= (sizeof(int))) break;
									}
									tempcmd = CMD_BARRIER;
									assert(parep_mpi_performing_barrier == 1);
									parep_mpi_performing_barrier = 0;
									pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
									parep_mpi_barrier_running = 0;
									pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
									if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
										pthread_mutex_lock(&daemon_sock_mutex);
										for(int j = 0; j < parep_mpi_node_group_size; j++) {
											if(daemon_socket[j] != -1) {
												write_to_fd(daemon_socket[j],&tempcmd,sizeof(int));
												write_to_fd(daemon_socket[j],&fail_ready,sizeof(int));
											}
										}
										pthread_mutex_unlock(&daemon_sock_mutex);
									}
									pthread_mutex_lock(&proc_state_mutex);
									pthread_mutex_lock(&client_sock_mutex);
									for(int j = 0; j < parep_mpi_node_size; j++) {
										if(isMainServer) {
											if(global_proc_state[j] != PROC_TERMINATED) {
												write_to_fd(client_socket[j],&tempcmd,sizeof(int));
												write_to_fd(client_socket[j],&fail_ready,sizeof(int));
											}
										} else {
											if(local_proc_state[j] != PROC_TERMINATED) {
												write_to_fd(client_socket[j],&tempcmd,sizeof(int));
												write_to_fd(client_socket[j],&fail_ready,sizeof(int));
											}
										}
									}
									pthread_mutex_unlock(&client_sock_mutex);
									pthread_mutex_unlock(&proc_state_mutex);
								}
							}
						}
						int cmd = CMD_INFORM_PREDICT;
						if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
							pthread_mutex_lock(&daemon_sock_mutex);
							for(int j = 0; j < parep_mpi_node_group_size; j++) {
								if(daemon_socket[j] != -1) {
									write_to_fd(daemon_socket[j],&cmd,sizeof(int));
									write_to_fd(daemon_socket[j],&targrank,sizeof(int));
									write_to_fd(daemon_socket[j],&timestamp,sizeof(time_t));
								}
							}
							pthread_mutex_unlock(&daemon_sock_mutex);
						}
						pthread_mutex_lock(&proc_state_mutex);
						pthread_mutex_lock(&client_sock_mutex);
						for(int j = 0; j < parep_mpi_node_size; j++) {
							if(isMainServer) {
								if(global_proc_state[j] != PROC_TERMINATED) {
									write_to_fd(client_socket[j],&cmd,sizeof(int));
									write_to_fd(client_socket[j],&targrank,sizeof(int));
									write_to_fd(client_socket[j],&timestamp,sizeof(time_t));
								}
							} else {
								if(local_proc_state[j] != PROC_TERMINATED) {
									write_to_fd(client_socket[j],&cmd,sizeof(int));
									write_to_fd(client_socket[j],&targrank,sizeof(int));
									write_to_fd(client_socket[j],&timestamp,sizeof(time_t));
								}
							}
						}
						pthread_mutex_unlock(&client_sock_mutex);
						pthread_mutex_unlock(&proc_state_mutex);
						pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
					}

					/*int targrank;
					time_t timestamp;
					msgsize = 0;
					while((bytes_read = read(pfd.fd,(&targrank)+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					msgsize = 0;
					while((bytes_read = read(pfd.fd,(&timestamp)+msgsize, sizeof(time_t)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(time_t))) break;
					}
					int fin_reached = 0;
					pthread_mutex_lock(&parep_mpi_finalize_reached_mutex);
					fin_reached = parep_mpi_finalize_reached;
					pthread_mutex_unlock(&parep_mpi_finalize_reached_mutex);
					if(!fin_reached) {
						int cmd = CMD_INFORM_PREDICT;
						if(parep_mpi_node_group_nodeid == 0) {
							pthread_mutex_lock(&daemon_sock_mutex);
							for(int j = 1; j < parep_mpi_node_group_size; j++) {
								write_to_fd(daemon_socket[j],&cmd,sizeof(int));
								write_to_fd(daemon_socket[j],&targrank,sizeof(int));
								write_to_fd(daemon_socket[j],&timestamp,sizeof(time_t));
							}
							pthread_mutex_unlock(&daemon_sock_mutex);
						}
						pthread_mutex_lock(&proc_state_mutex);
						pthread_mutex_lock(&client_sock_mutex);
						for(int j = 0; j < parep_mpi_node_size; j++) {
							if(isMainServer) {
								if(global_proc_state[j] != PROC_TERMINATED) {
									write_to_fd(client_socket[j],&cmd,sizeof(int));
									write_to_fd(client_socket[j],&targrank,sizeof(int));
									write_to_fd(client_socket[j],&timestamp,sizeof(time_t));
								}
							} else {
								if(local_proc_state[j] != PROC_TERMINATED) {
									write_to_fd(client_socket[j],&cmd,sizeof(int));
									write_to_fd(client_socket[j],&targrank,sizeof(int));
									write_to_fd(client_socket[j],&timestamp,sizeof(time_t));
								}
							}
						}
						pthread_mutex_unlock(&client_sock_mutex);
						pthread_mutex_unlock(&proc_state_mutex);
					}*/
				} else if(cmd == CMD_INFORM_PREDICT_NODE) {
					int size;
					int *ranks;
					time_t timestamp;
					msgsize = 0;
					while((bytes_read = read(pfd.fd,(&size)+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					ranks = (int *)malloc(sizeof(int)*size);
					msgsize = 0;
					while((bytes_read = read(pfd.fd,((char *)ranks)+msgsize, (size*sizeof(int))-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (size*sizeof(int))) break;
					}
					msgsize = 0;
					while((bytes_read = read(pfd.fd,(&timestamp)+msgsize, sizeof(time_t)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(time_t))) break;
					}

					int exit_recvd;
					int local_term;
					pthread_mutex_lock(&exit_cmd_recvd_mutex);
					exit_recvd = exit_cmd_recvd;
					pthread_mutex_unlock(&exit_cmd_recvd_mutex);
					pthread_mutex_lock(&local_procs_term_mutex);
					local_term = local_procs_term;
					pthread_mutex_unlock(&local_procs_term_mutex);
					if((exit_recvd == 0) && (local_term == 0)) {
						pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
						if(parep_mpi_performing_barrier == 1) {
							pollret = poll(&pfd,nfds,-1);
							if (pfd.revents != 0) {
								if(pfd.revents & POLLIN) {
									int tempcmd;
									int tempmsgsize = 0;
									size_t temp_bytes_read;
									while((temp_bytes_read = read(pfd.fd,(&tempcmd)+tempmsgsize, sizeof(int)-tempmsgsize)) > 0) {
										tempmsgsize += temp_bytes_read;
										if(tempmsgsize >= (sizeof(int))) break;
									}
									assert(tempcmd == CMD_BARRIER);
									int fail_ready;
									tempmsgsize = 0;
									while((temp_bytes_read = read(pfd.fd,(&fail_ready)+tempmsgsize, sizeof(int)-tempmsgsize)) > 0) {
										tempmsgsize += temp_bytes_read;
										if(tempmsgsize >= (sizeof(int))) break;
									}
									tempcmd = CMD_BARRIER;
									assert(parep_mpi_performing_barrier == 1);
									parep_mpi_performing_barrier = 0;
									pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
									parep_mpi_barrier_running = 0;
									pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
									if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
										pthread_mutex_lock(&daemon_sock_mutex);
										for(int j = 0; j < parep_mpi_node_group_size; j++) {
											if(daemon_socket[j] != -1) {
												write_to_fd(daemon_socket[j],&tempcmd,sizeof(int));
												write_to_fd(daemon_socket[j],&fail_ready,sizeof(int));
											}
										}
										pthread_mutex_unlock(&daemon_sock_mutex);
									}
									pthread_mutex_lock(&proc_state_mutex);
									pthread_mutex_lock(&client_sock_mutex);
									for(int j = 0; j < parep_mpi_node_size; j++) {
										if(isMainServer) {
											if(global_proc_state[j] != PROC_TERMINATED) {
												write_to_fd(client_socket[j],&tempcmd,sizeof(int));
												write_to_fd(client_socket[j],&fail_ready,sizeof(int));
											}
										} else {
											if(local_proc_state[j] != PROC_TERMINATED) {
												write_to_fd(client_socket[j],&tempcmd,sizeof(int));
												write_to_fd(client_socket[j],&fail_ready,sizeof(int));
											}
										}
									}
									pthread_mutex_unlock(&client_sock_mutex);
									pthread_mutex_unlock(&proc_state_mutex);
								}
							}
						}
						int cmd = CMD_INFORM_PREDICT_NODE;
						if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
							pthread_mutex_lock(&daemon_sock_mutex);
							for(int j = 0; j < parep_mpi_node_group_size; j++) {
								if(daemon_socket[j] != -1) {
									write_to_fd(daemon_socket[j],&cmd,sizeof(int));
									write_to_fd(daemon_socket[j],&size,sizeof(int));
									write_to_fd(daemon_socket[j],ranks,sizeof(int)*size);
									write_to_fd(daemon_socket[j],&timestamp,sizeof(time_t));
								}
							}
							pthread_mutex_unlock(&daemon_sock_mutex);
						}
						pthread_mutex_lock(&proc_state_mutex);
						pthread_mutex_lock(&client_sock_mutex);
						for(int j = 0; j < parep_mpi_node_size; j++) {
							if(isMainServer) {
								if(global_proc_state[j] != PROC_TERMINATED) {
									write_to_fd(client_socket[j],&cmd,sizeof(int));
									write_to_fd(client_socket[j],&size,sizeof(int));
									write_to_fd(client_socket[j],ranks,sizeof(int)*size);
									write_to_fd(client_socket[j],&timestamp,sizeof(time_t));
								}
							} else {
								if(local_proc_state[j] != PROC_TERMINATED) {
									write_to_fd(client_socket[j],&cmd,sizeof(int));
									write_to_fd(client_socket[j],&size,sizeof(int));
									write_to_fd(client_socket[j],ranks,sizeof(int)*size);
									write_to_fd(client_socket[j],&timestamp,sizeof(time_t));
								}
							}
						}
						pthread_mutex_unlock(&client_sock_mutex);
						pthread_mutex_unlock(&proc_state_mutex);
						pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
					}
					free(ranks);
				} else if(cmd == CMD_INFORM_PROC_FAILED) {
					int num_failed_procs;
					msgsize = 0;
					while((bytes_read = read(pfd.fd,(&num_failed_procs)+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					int *failed_ranks = (int *)malloc(sizeof(int) * num_failed_procs);
					msgsize = 0;
					while((bytes_read = read(pfd.fd,((void *)failed_ranks)+msgsize, (sizeof(int) * num_failed_procs)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int) * num_failed_procs)) break;
					}
					
					int exit_recvd;
					int local_term;
					pthread_mutex_lock(&exit_cmd_recvd_mutex);
					exit_recvd = exit_cmd_recvd;
					pthread_mutex_unlock(&exit_cmd_recvd_mutex);
					pthread_mutex_lock(&local_procs_term_mutex);
					local_term = local_procs_term;
					pthread_mutex_unlock(&local_procs_term_mutex);
					if((exit_recvd == 0) && (local_term == 0)) {
						pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
						if(parep_mpi_performing_barrier == 1) {
							pollret = poll(&pfd,nfds,-1);
							if (pfd.revents != 0) {
								if(pfd.revents & POLLIN) {
									int tempcmd;
									int tempmsgsize = 0;
									size_t temp_bytes_read;
									while((temp_bytes_read = read(pfd.fd,(&tempcmd)+tempmsgsize, sizeof(int)-tempmsgsize)) > 0) {
										tempmsgsize += temp_bytes_read;
										if(tempmsgsize >= (sizeof(int))) break;
									}
									assert(tempcmd == CMD_BARRIER);
									int fail_ready;
									tempmsgsize = 0;
									while((temp_bytes_read = read(pfd.fd,(&fail_ready)+tempmsgsize, sizeof(int)-tempmsgsize)) > 0) {
										tempmsgsize += temp_bytes_read;
										if(tempmsgsize >= (sizeof(int))) break;
									}
									tempcmd = CMD_BARRIER;
									assert(parep_mpi_performing_barrier == 1);
									parep_mpi_performing_barrier = 0;
									pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
									parep_mpi_barrier_running = 0;
									pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
									if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
										pthread_mutex_lock(&daemon_sock_mutex);
										for(int j = 0; j < parep_mpi_node_group_size; j++) {
											if(daemon_socket[j] != -1) {
												write_to_fd(daemon_socket[j],&tempcmd,sizeof(int));
												write_to_fd(daemon_socket[j],&fail_ready,sizeof(int));
											}
										}
										pthread_mutex_unlock(&daemon_sock_mutex);
									}
									pthread_mutex_lock(&proc_state_mutex);
									pthread_mutex_lock(&client_sock_mutex);
									for(int j = 0; j < parep_mpi_node_size; j++) {
										if(isMainServer) {
											if(global_proc_state[j] != PROC_TERMINATED) {
												write_to_fd(client_socket[j],&tempcmd,sizeof(int));
												write_to_fd(client_socket[j],&fail_ready,sizeof(int));
											}
										} else {
											if(local_proc_state[j] != PROC_TERMINATED) {
												write_to_fd(client_socket[j],&tempcmd,sizeof(int));
												write_to_fd(client_socket[j],&fail_ready,sizeof(int));
											}
										}
									}
									pthread_mutex_unlock(&client_sock_mutex);
									pthread_mutex_unlock(&proc_state_mutex);
								}
							}
						}
						if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
							pthread_mutex_lock(&daemon_sock_mutex);
							for(int j = 0; j < parep_mpi_node_group_size; j++) {
								if(daemon_socket[j] != -1) {
									write_to_fd(daemon_socket[j],&cmd,sizeof(int));
									write_to_fd(daemon_socket[j],&num_failed_procs,sizeof(int));
									write_to_fd(daemon_socket[j],failed_ranks, sizeof(int) * num_failed_procs);
								}
							}
							pthread_mutex_unlock(&daemon_sock_mutex);
						}
						pthread_mutex_lock(&proc_state_mutex);
						pthread_mutex_lock(&client_sock_mutex);
						for(int j = 0; j < parep_mpi_node_size; j++) {
							if(local_proc_state[j] != PROC_TERMINATED) {
								write_to_fd(client_socket[j],&cmd,sizeof(int));
								write_to_fd(client_socket[j],&num_failed_procs,sizeof(int));
								write_to_fd(client_socket[j],failed_ranks, sizeof(int) * num_failed_procs);
							}
						}
						pthread_mutex_unlock(&client_sock_mutex);
						int *fprocs = (int *)failed_ranks;
						for(int j = 0; j < num_failed_procs; j++) {
							if((fprocs[j] >= rank_lims.start) && (fprocs[j] <= rank_lims.end)) {
								int local_rank = fprocs[j] - rank_lims.start;
								if(!parep_mpi_inform_failed_check[local_rank]) {
									parep_mpi_inform_failed_check[local_rank] = true;
									parep_mpi_num_inform_failed++;
								}
							}
						}
						pthread_mutex_unlock(&proc_state_mutex);
						pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
					}
					
					free(failed_ranks);
				} else if(cmd == CMD_CHECK_NODE_FAIL) {
					assert(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid);
					int num_nodes_failed = 0;
					int *failed_node_group_nodeids = (int *)malloc(sizeof(int)*parep_mpi_node_group_size);
					for(int j = 0; j < parep_mpi_node_group_size; j++) {
						failed_node_group_nodeids[j] = -1;
					}
					pthread_mutex_lock(&proc_state_mutex);
					pthread_mutex_lock(&daemon_sock_mutex);
					int index = 0;
					for(int j = 0; j < parep_mpi_node_group_size; j++) {
						if((group_daemon_state[j] != PROC_TERMINATED)) {
							struct pollfd pfd;
							pfd.fd = daemon_socket[j];
							pfd.events = POLLIN;
							int pollret = poll(&pfd,1,0);
							if(pollret > 0) {
								bool failcase = false;
								if(pfd.revents & POLLIN) {
									int temp;
									size_t bytes_read;
									bytes_read = read(pfd.fd, &temp, sizeof(int));
									if(bytes_read <= 0) failcase = true;
								}
								if((pfd.revents & POLLHUP) || (pfd.revents & POLLERR) || failcase) {
									close(pfd.fd);
									daemon_socket[j] = -1;
									group_daemon_state[j] = PROC_TERMINATED;
									pthread_cond_signal(&proc_state_cond);
									num_nodes_failed++;
									failed_node_group_nodeids[index] = j;
									parep_mpi_num_failed += parep_mpi_node_sizes[j];
									index++;
								}
							}
						}
					}
					pthread_mutex_unlock(&daemon_sock_mutex);
					pthread_mutex_unlock(&proc_state_mutex);
					int dsock;
					check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
					int cmd = CMD_CHECK_NODE_FAIL;
					int ret;
					do {
						ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
					} while(ret != 0);
					write_to_fd(dsock,&cmd,sizeof(int));
					write_to_fd(dsock,&num_nodes_failed,sizeof(int));
					if(num_nodes_failed > 0) {
						write_to_fd(dsock,&parep_mpi_node_group_id,sizeof(int));
						write_to_fd(dsock,(void *)failed_node_group_nodeids,sizeof(int)*num_nodes_failed);
					}
					close(dsock);
					free(failed_node_group_nodeids);
				}
			} else if((pfd.revents & POLLHUP) || (pfd.revents & POLLERR)) {
				if(parep_mpi_node_group_id != 0) {
					close(daemon_client_socket);
					get_group_server_from_main();
					pfd.fd = daemon_client_socket;
				}
			}
		}
	}
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

int check_node_failed(int infinite_timeout) {
	int num_nodes_failed = 0;
	pthread_mutex_lock(&proc_state_mutex);
	while(parep_mpi_reconf_ngroup) {
		pthread_cond_wait(&proc_state_cond,&proc_state_mutex);
	}
	pthread_mutex_lock(&daemon_sock_mutex);
	for(int j = 0; j < parep_mpi_node_num; j++) {
		if((daemon_state[j] != PROC_TERMINATED)) {
			if((parep_mpi_node_group_ids[j] == 0)) {
				struct pollfd pfd;
				pfd.fd = daemon_socket[j];
				pfd.events = POLLIN;
				int pollret = poll(&pfd,1,0);
				if(pollret > 0) {
					bool failcase = false;
					if(pfd.revents & POLLIN) {
						int temp;
						size_t bytes_read;
						bytes_read = read(pfd.fd, &temp, sizeof(int));
						if(bytes_read <= 0) failcase = true;
					}
					if((pfd.revents & POLLHUP) || (pfd.revents & POLLERR) || failcase) {
						daemon_state[j] = PROC_TERMINATED;
						pthread_cond_signal(&proc_state_cond);
						num_nodes_failed++;
					}
				}
			} else if((parep_mpi_node_group_nodeids[j] == parep_mpi_node_group_leader_nodeids[j])) {
				struct pollfd pfd;
				pfd.fd = daemon_socket[j];
				pfd.events = POLLIN;
				int pollret = poll(&pfd,1,0);
				if(pollret > 0) {
					bool failcase = false;
					if(pfd.revents & POLLIN) {
						int temp;
						size_t bytes_read;
						bytes_read = read(pfd.fd, &temp, sizeof(int));
						if(bytes_read <= 0) failcase = true;
					}
					if((pfd.revents & POLLHUP) || (pfd.revents & POLLERR) || failcase) {
						pthread_mutex_unlock(&daemon_sock_mutex);
						while(daemon_state[j] != PROC_TERMINATED) {
							waiting_for_node_fail_detect = true;
							pthread_cond_wait(&proc_state_cond,&proc_state_mutex);
							waiting_for_node_fail_detect = false;
						}
						pthread_mutex_lock(&daemon_sock_mutex);
						pthread_cond_signal(&proc_state_cond);
						num_nodes_failed++;
					}
				} else if(pollret == 0) {
					int cmd = CMD_CHECK_NODE_FAIL;
					write_to_fd(daemon_socket[j],&cmd,sizeof(int));
				}
			}
		}
	}
	pthread_mutex_unlock(&daemon_sock_mutex);
	pthread_mutex_unlock(&proc_state_mutex);
	parep_mpi_node_group_checked += 1;
	pthread_mutex_lock(&parep_mpi_node_group_checked_mutex);
	while(parep_mpi_node_group_checked < parep_mpi_node_group_num) {
		pthread_cond_wait(&parep_mpi_node_group_checked_cond,&parep_mpi_node_group_checked_mutex);
	}
	parep_mpi_node_group_checked = 0;
	num_nodes_failed += parep_mpi_num_nodes_failed;
	parep_mpi_num_nodes_failed = 0;
	pthread_mutex_unlock(&parep_mpi_node_group_checked_mutex);
	return num_nodes_failed;
}

void *empi_thread_function(void *arg) {
	int addr_size;
	struct sockaddr_un empi_server_addr,empi_client_addr;
	int option = 1;
	
	if(pthread_self() == empi_thread) {
		check((empi_socket = socket(AF_UNIX,SOCK_STREAM,0)),"Failed to create socket");
		setsockopt(empi_socket,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
	} else if(pthread_self() == empi_exec_thread) {
		check((empi_exec_socket = socket(AF_UNIX,SOCK_STREAM,0)),"Failed to create socket");
		setsockopt(empi_exec_socket,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
	}
	
	memset(&empi_server_addr,0,sizeof(empi_server_addr));
	empi_server_addr.sun_family = AF_UNIX;
	if(pthread_self() == empi_thread) strcpy(empi_server_addr.sun_path,"#EMPIServer");
	else if(pthread_self() == empi_exec_thread) strcpy(empi_server_addr.sun_path,"#EMPIServermpiexec");
	empi_server_addr.sun_path[0] = 0;
	
	if(pthread_self() == empi_thread) {
		check(bind(empi_socket,(SA *)&empi_server_addr,sizeof(empi_server_addr)),"Bind Failed!");
		check(listen(empi_socket, 1),"Listen Failed!");
		empi_client_socket = accept(empi_socket, (SA *)&empi_client_addr, (socklen_t *)&addr_size);
		if((empi_client_socket == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
			while((empi_client_socket == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
				rlim_nofile.rlim_cur *= 2;
				assert(rlim_nofile.rlim_cur <= rlim_nofile.rlim_max);
				setrlimit(RLIMIT_NOFILE,&rlim_nofile);
				empi_client_socket = accept(empi_socket, (SA *)&empi_client_addr, (socklen_t *)&addr_size);
			}
			check(empi_client_socket,"accept failed");
		} else check(empi_client_socket,"accept failed");
	
		{
			size_t bytes_read;
			int msgsize = 0;
			while((bytes_read = read(empi_client_socket,(&parep_mpi_empi_pid)+msgsize, sizeof(pid_t)-msgsize)) > 0) {
				msgsize += bytes_read;
				if(msgsize >= (sizeof(pid_t))) break;
			}
			check(bytes_read,"recv error empi pid");
		}
	} else if(pthread_self() == empi_exec_thread) {
		check(bind(empi_exec_socket,(SA *)&empi_server_addr,sizeof(empi_server_addr)),"Bind Failed!");
		check(listen(empi_exec_socket, 1),"Listen Failed!");
		empi_exec_client_socket = accept(empi_exec_socket, (SA *)&empi_client_addr, (socklen_t *)&addr_size);
		if((empi_exec_client_socket == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
			while((empi_exec_client_socket == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
				rlim_nofile.rlim_cur *= 2;
				assert(rlim_nofile.rlim_cur <= rlim_nofile.rlim_max);
				setrlimit(RLIMIT_NOFILE,&rlim_nofile);
				empi_exec_client_socket = accept(empi_exec_socket, (SA *)&empi_client_addr, (socklen_t *)&addr_size);
			}
			check(empi_exec_client_socket,"accept failed");
		} else check(empi_exec_client_socket,"accept failed");
	
		{
			size_t bytes_read;
			int msgsize = 0;
			while((bytes_read = read(empi_exec_client_socket,(&parep_mpi_empi_exec_pid)+msgsize, sizeof(pid_t)-msgsize)) > 0) {
				msgsize += bytes_read;
				if(msgsize >= (sizeof(pid_t))) break;
			}
			check(bytes_read,"recv error empi pid");
		}
	}
	
	nfds_t nfds;
	struct pollfd pfd;
	if(pthread_self() == empi_thread) pfd.fd = empi_client_socket;
	else if(pthread_self() == empi_exec_thread) pfd.fd = empi_exec_client_socket;
	pfd.events = POLLIN;
	nfds = 1;
	int pollret;
	
	while(true) {
		pollret = poll(&pfd,nfds,-1);
		if((pollret == -1) && (errno == EINTR)) continue;
		if (pfd.revents != 0) {
			if((pfd.revents & POLLHUP) || (pfd.revents & POLLERR)) {
				//printf("mainserv %d: Detected POLLHUP/POLLERR from empi_client_socket\n",isMainServer);
				//fflush(stdout);
				pthread_exit(NULL);
			}
			if(pfd.revents & POLLIN) {
				size_t bytes_read;
				int msgsize = 0;
				char buffer[BUFFER_SIZE];
				
				msgsize = 0;
				if(pthread_self() == empi_thread) {
					while((bytes_read = read(empi_client_socket,buffer+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					check(bytes_read,"recv error empi wnohang");
				} else if(pthread_self() == empi_exec_thread) {
					while((bytes_read = read(empi_exec_client_socket,buffer+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					check(bytes_read,"recv error empi wnohang");
				}
				int cmd = *((int *)buffer);
				if(cmd == CMD_MISC_PROC_FAILED) {
					int infinite_timeout;
					msgsize = 0;
					if(pthread_self() == empi_thread) {
						while((bytes_read = read(empi_client_socket,buffer+msgsize, sizeof(int)-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(int))) break;
						}
						check(bytes_read,"recv error empi infinite_timeout");
						infinite_timeout = *((int *)buffer);
						
						msgsize = 0;
						while((bytes_read = read(empi_client_socket,buffer+msgsize, sizeof(int)-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(int))) break;
						}
						check(bytes_read,"recv error empi num active fds");
					} else if(pthread_self() == empi_exec_thread) {
						while((bytes_read = read(empi_exec_client_socket,buffer+msgsize, sizeof(int)-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(int))) break;
						}
						check(bytes_read,"recv error empi infinite_timeout");
						infinite_timeout = *((int *)buffer);
						
						msgsize = 0;
						while((bytes_read = read(empi_exec_client_socket,buffer+msgsize, sizeof(int)-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(int))) break;
						}
						check(bytes_read,"recv error empi num active fds");
					}
					int num_active_fds = *((int *)buffer);
					
					int num_nodes_failed = 0;
					if(isMainServer) {
						num_nodes_failed = check_node_failed(infinite_timeout);
					}
					
					if(num_nodes_failed > 0) {
						int num_failed = 0;
						int *proc_failed = (int *)malloc(sizeof(int) * parep_mpi_size);
						for(int j = 0; j < parep_mpi_size; j++) proc_failed[j] = 0;
						pthread_mutex_lock(&proc_state_mutex);
						int rank_offset = 0;
						for(int i = 0; i < parep_mpi_node_num; i++) {
							if(daemon_state[i] == PROC_TERMINATED) {
								int start_rank = rank_offset;
								for(int j = start_rank; j < start_rank+parep_mpi_node_sizes[i]; j++) {
									if(global_proc_state[j] != PROC_TERMINATED) {
										global_proc_state[j] = PROC_TERMINATED;
										num_failed++;
										proc_failed[j] = 1;
										pthread_mutex_lock(&parep_mpi_num_failed_mutex);
										parep_mpi_num_failed++;
										pthread_cond_signal(&parep_mpi_barrier_count_cond);
										pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
									}
								}
							}
							rank_offset += parep_mpi_node_sizes[i];
						}
						
						int exit_recvd;
						int local_term;
						pthread_mutex_lock(&exit_cmd_recvd_mutex);
						exit_recvd = exit_cmd_recvd;
						pthread_mutex_unlock(&exit_cmd_recvd_mutex);
						pthread_mutex_lock(&local_procs_term_mutex);
						local_term = local_procs_term;
						pthread_mutex_unlock(&local_procs_term_mutex);
						if((exit_recvd == 0) && (local_term == 0)) {
							char *proc_state_data_buf = (char *)malloc(sizeof(int) * num_failed);
							size_t offset = 0;
							for(int i = 0; i < parep_mpi_size; i++) {
								if(proc_failed[i]) {
									char *proc_state_data = proc_state_data_buf + offset;
									*((int *)proc_state_data) = parep_mpi_all_ranks[i];
									offset += sizeof(int);
								}
							}
							int cmd = CMD_INFORM_PROC_FAILED;
							pthread_mutex_lock(&parep_mpi_fail_ready_mutex);
							parep_mpi_fail_ready = 1;
							pthread_mutex_unlock(&parep_mpi_fail_ready_mutex);
							pthread_cond_signal(&proc_state_cond);
							pthread_mutex_unlock(&proc_state_mutex);
							pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
							pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
							while(parep_mpi_barrier_running == 1) {
								pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
								pthread_cond_wait(&parep_mpi_barrier_running_cond,&parep_mpi_barrier_running_mutex);
								if(parep_mpi_barrier_running == 0) pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
							}
							pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
							pthread_mutex_lock(&proc_state_mutex);
							pthread_mutex_lock(&daemon_sock_mutex);
							for(int j = 1; j < parep_mpi_node_num; j++) {
								if((daemon_state[j] != PROC_TERMINATED) && ((parep_mpi_node_group_ids[j] == 0) || (parep_mpi_node_group_nodeids[j] == parep_mpi_node_group_leader_nodeids[j]))) {
									write_to_fd(daemon_socket[j],&cmd,sizeof(int));
									write_to_fd(daemon_socket[j],&num_failed,sizeof(int));
									write_to_fd(daemon_socket[j],proc_state_data_buf, sizeof(int) * num_failed);
								}
							}
							pthread_mutex_unlock(&daemon_sock_mutex);
							pthread_mutex_lock(&client_sock_mutex);
							for(int j = 0; j < parep_mpi_node_size; j++) {
								if(global_proc_state[j] != PROC_TERMINATED) {
									write_to_fd(client_socket[j],&cmd,sizeof(int));
									write_to_fd(client_socket[j],&num_failed,sizeof(int));
									write_to_fd(client_socket[j],proc_state_data_buf, sizeof(int) * num_failed);
								}
							}
							pthread_mutex_unlock(&client_sock_mutex);
							int *fprocs = (int *)proc_state_data_buf;
							for(int j = 0; j < num_failed; j++) {
								if(!parep_mpi_inform_failed_check[fprocs[j]]) {
									parep_mpi_inform_failed_check[fprocs[j]] = true;
									parep_mpi_num_inform_failed++;
									int nodeid = fprocs[j] / parep_mpi_node_size;
									int nodegroupid = parep_mpi_node_group_ids[nodeid];
									parep_mpi_num_inform_failed_node[nodeid]++;
									parep_mpi_num_inform_failed_node_group[nodegroupid]++;
								}
							}
							pthread_mutex_lock(&parep_mpi_fail_ready_mutex);
							parep_mpi_fail_ready = 0;
							pthread_mutex_unlock(&parep_mpi_fail_ready_mutex);
							pthread_mutex_unlock(&proc_state_mutex);
							pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
							pthread_mutex_lock(&proc_state_mutex);
							free(proc_state_data_buf);
						}
						
						pthread_cond_signal(&proc_state_cond);
						pthread_mutex_unlock(&proc_state_mutex);
						free(proc_failed);
					} else {
						struct pollfd *pfds;
						nfds_t nfds = parep_mpi_node_size;
						pfds = (struct pollfd *)malloc(nfds * sizeof(struct pollfd));
						pthread_mutex_lock(&proc_state_mutex);
						int num_failed = 0;
						int *proc_failed = (int *)malloc(sizeof(int) * parep_mpi_node_size);
						int lterm = 0;
						pthread_mutex_lock(&local_procs_term_mutex);
						lterm = local_procs_term;
						pthread_mutex_unlock(&local_procs_term_mutex);
						do {
							if(lterm == 1) break;
							num_failed = 0;
							for(int j = 0; j < parep_mpi_node_size; j++) {
								if(isMainServer) {
									if(global_proc_state[j] != PROC_TERMINATED) pfds[j].fd = client_socket[j];
									else pfds[j].fd = -1;
								} else {
									if(local_proc_state[j] != PROC_TERMINATED) pfds[j].fd = client_socket[j];
									else pfds[j].fd = -1;
								}
								pfds[j].events = POLLIN;
							}
							for(int j = 0; j < parep_mpi_node_size; j++) proc_failed[j] = 0;
							while(num_failed == 0) {
								pthread_mutex_lock(&local_procs_term_mutex);
								lterm = local_procs_term;
								pthread_mutex_unlock(&local_procs_term_mutex);
								if(lterm == 1) break;
								int pollret = 0;
								pthread_cond_signal(&proc_state_cond);
								pthread_mutex_unlock(&proc_state_mutex);
								while(pollret <= 0) {
									pollret = poll(pfds,nfds,-1);
								}
								pthread_mutex_lock(&proc_state_mutex);
								
								for(int j = 0; j < parep_mpi_node_size; j++) {
									if(pfds[j].revents != 0) {
										if((pfds[j].revents & POLLHUP) || (pfds[j].revents & POLLERR)) {
											if(isMainServer) {
												if(global_proc_state[j] != PROC_TERMINATED) {
													global_proc_state[j] = PROC_TERMINATED;
													num_failed++;
													proc_failed[j] = 1;
													pthread_mutex_lock(&parep_mpi_num_failed_mutex);
													parep_mpi_num_failed++;
													pthread_cond_signal(&parep_mpi_barrier_count_cond);
													pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
												}
											} else {
												if(local_proc_state[j] != PROC_TERMINATED) {
													local_proc_state[j] = PROC_TERMINATED;
													num_failed++;
													proc_failed[j] = 1;
													pthread_mutex_lock(&parep_mpi_num_failed_mutex);
													parep_mpi_num_failed++;
													pthread_cond_signal(&parep_mpi_barrier_count_cond);
													pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
												}
											}
										} else if(pfds[j].revents & POLLIN) {
											bytes_read = recv(pfds[j].fd,buffer,sizeof(buffer),0);
											assert(bytes_read <= 0);
											if(isMainServer) {
												if(global_proc_state[j] != PROC_TERMINATED) {
													global_proc_state[j] = PROC_TERMINATED;
													num_failed++;
													proc_failed[j] = 1;
													pthread_mutex_lock(&parep_mpi_num_failed_mutex);
													parep_mpi_num_failed++;
													pthread_cond_signal(&parep_mpi_barrier_count_cond);
													pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
												}
											} else {
												if(local_proc_state[j] != PROC_TERMINATED) {
													local_proc_state[j] = PROC_TERMINATED;
													num_failed++;
													proc_failed[j] = 1;
													pthread_mutex_lock(&parep_mpi_num_failed_mutex);
													parep_mpi_num_failed++;
													pthread_cond_signal(&parep_mpi_barrier_count_cond);
													pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
												}
											}
										}
									}
								}
							}
							
							if((!isMainServer) && (num_failed > 0)) {
								int dsock;
								check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
								int ret;
								do {
									ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
								} while(ret != 0);
								int cmd = CMD_PROC_STATE_UPDATE;
								write_to_fd(dsock,&cmd,sizeof(int));
								write_to_fd(dsock,&num_failed,sizeof(int));
								
								char *proc_state_data_buf = (char *)malloc(((2*sizeof(int))+sizeof(pid_t)) * num_failed);
								size_t offset = 0;
								int local_terminated = 1;
								for(int i = 0; i < parep_mpi_node_size; i++) {
									if(proc_failed[i]) {
										char *proc_state_data = proc_state_data_buf + offset;
										*((int *)proc_state_data) = parep_mpi_ranks[i];
										*((int *)(proc_state_data+sizeof(int))) = local_proc_state[i];
										*((pid_t *)(proc_state_data+(2*sizeof(int)))) = parep_mpi_pids[i];
										offset += (2*sizeof(int))+sizeof(pid_t);
									}
									if(local_proc_state[i] != PROC_TERMINATED) {
										local_terminated = 0;
									}
								}
								write_to_fd(dsock,proc_state_data_buf,(((2*sizeof(int))+sizeof(pid_t)) * num_failed));
								free(proc_state_data_buf);
								if(local_terminated) {
									pthread_mutex_lock(&local_procs_term_mutex);
									local_procs_term = local_terminated;
									pthread_cond_signal(&local_procs_term_cond);
									pthread_mutex_unlock(&local_procs_term_mutex);
									lterm = 1;
								}
								close(dsock);
							}
							
							if((isMainServer) && (num_failed > 0)) {
								int exit_recvd;
								int local_term;
								pthread_mutex_lock(&exit_cmd_recvd_mutex);
								exit_recvd = exit_cmd_recvd;
								pthread_mutex_unlock(&exit_cmd_recvd_mutex);
								pthread_mutex_lock(&local_procs_term_mutex);
								local_term = local_procs_term;
								pthread_mutex_unlock(&local_procs_term_mutex);
								if((exit_recvd == 0) && (local_term == 0)) {
									int local_terminated = 1;
									char *proc_state_data_buf = (char *)malloc(sizeof(int) * num_failed);
									size_t offset = 0;
									for(int i = 0; i < parep_mpi_node_size; i++) {
										if(proc_failed[i]) {
											char *proc_state_data = proc_state_data_buf + offset;
											*((int *)proc_state_data) = parep_mpi_ranks[i];
											offset += sizeof(int);
										}
										if(global_proc_state[i] != PROC_TERMINATED) {
											local_terminated = 0;
										}
									}
									int cmd = CMD_INFORM_PROC_FAILED;
									pthread_mutex_lock(&parep_mpi_fail_ready_mutex);
									parep_mpi_fail_ready = 1;
									pthread_mutex_unlock(&parep_mpi_fail_ready_mutex);
									pthread_cond_signal(&proc_state_cond);
									pthread_mutex_unlock(&proc_state_mutex);
									pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
									pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
									while(parep_mpi_barrier_running == 1) {
										pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
										pthread_cond_wait(&parep_mpi_barrier_running_cond,&parep_mpi_barrier_running_mutex);
										if(parep_mpi_barrier_running == 0) pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
									}
									pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
									pthread_mutex_lock(&proc_state_mutex);
									pthread_mutex_lock(&daemon_sock_mutex);
									for(int j = 1; j < parep_mpi_node_num; j++) {
										if((daemon_state[j] != PROC_TERMINATED) && ((parep_mpi_node_group_ids[j] == 0) || (parep_mpi_node_group_nodeids[j] == parep_mpi_node_group_leader_nodeids[j]))) {
											write_to_fd(daemon_socket[j],&cmd,sizeof(int));
											write_to_fd(daemon_socket[j],&num_failed,sizeof(int));
											write_to_fd(daemon_socket[j],proc_state_data_buf, sizeof(int) * num_failed);
										}
									}
									pthread_mutex_unlock(&daemon_sock_mutex);
									pthread_mutex_lock(&client_sock_mutex);
									for(int j = 0; j < parep_mpi_node_size; j++) {
										if(global_proc_state[j] != PROC_TERMINATED) {
											write_to_fd(client_socket[j],&cmd,sizeof(int));
											write_to_fd(client_socket[j],&num_failed,sizeof(int));
											write_to_fd(client_socket[j],proc_state_data_buf, sizeof(int) * num_failed);
										}
									}
									pthread_mutex_unlock(&client_sock_mutex);
									int *fprocs = (int *)proc_state_data_buf;
									for(int j = 0; j < num_failed; j++) {
										if(!parep_mpi_inform_failed_check[fprocs[j]]) {
											parep_mpi_inform_failed_check[fprocs[j]] = true;
											parep_mpi_num_inform_failed++;
											int nodeid = fprocs[j] / parep_mpi_node_size;
											int nodegroupid = parep_mpi_node_group_ids[nodeid];
											parep_mpi_num_inform_failed_node[nodeid]++;
											parep_mpi_num_inform_failed_node_group[nodegroupid]++;
										}
									}
									pthread_mutex_lock(&parep_mpi_fail_ready_mutex);
									parep_mpi_fail_ready = 0;
									pthread_mutex_unlock(&parep_mpi_fail_ready_mutex);
									pthread_mutex_unlock(&proc_state_mutex);
									pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
									pthread_mutex_lock(&proc_state_mutex);
									free(proc_state_data_buf);
									if(local_terminated) {
										pthread_mutex_lock(&local_procs_term_mutex);
										local_procs_term = local_terminated;
										pthread_cond_signal(&local_procs_term_cond);
										pthread_mutex_unlock(&local_procs_term_mutex);
										lterm = 1;
									}
								} else if(local_term == 0) {
									int local_terminated = 1;
									for(int i = 0; i < parep_mpi_node_size; i++) {
										if(global_proc_state[i] != PROC_TERMINATED) {
											local_terminated = 0;
										}
									}
									if(local_terminated) {
										pthread_mutex_lock(&local_procs_term_mutex);
										local_procs_term = local_terminated;
										pthread_cond_signal(&local_procs_term_cond);
										pthread_mutex_unlock(&local_procs_term_mutex);
										lterm = 1;
									}
								} else lterm = 1;
							}
						} while((lterm == 0) && (infinite_timeout == 1) && (num_active_fds == 0));
						
						pthread_cond_signal(&proc_state_cond);
						pthread_mutex_unlock(&proc_state_mutex);
						
						free(pfds);			
						free(proc_failed);
					}
					if((!infinite_timeout) || (num_active_fds > 0)) {
					} else {
						int empi_safe_ret = 0;
						while(empi_safe_ret == 0) {
							pthread_mutex_lock(&empi_waitpid_safe_mutex);
							if(empi_waitpid_safe == 0) pthread_cond_wait(&empi_waitpid_safe_cond,&empi_waitpid_safe_mutex);
							empi_safe_ret = empi_waitpid_safe;
							pthread_mutex_unlock(&empi_waitpid_safe_mutex);
						}
						assert(empi_safe_ret == 1);
					}
				} else if(cmd == CMD_WAITPID_DATA) {
					msgsize = 0;
					if(pthread_self() == empi_thread) {
						while((bytes_read = read(empi_client_socket,buffer+msgsize, sizeof(int)-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(int))) break;
						}
						check(bytes_read,"recv error empi wnohang");
					} else if(pthread_self() == empi_exec_thread) {
						while((bytes_read = read(empi_exec_client_socket,buffer+msgsize, sizeof(int)-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(int))) break;
						}
						check(bytes_read,"recv error empi wnohang");
					}
					int wnohang_used = *((int *)buffer);
					
					msgsize = 0;
					if(pthread_self() == empi_thread) {
						while((bytes_read = read(empi_client_socket,buffer+msgsize, sizeof(int)-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(int))) break;
						}
						check(bytes_read,"recv error empi num pid");
					} else if(pthread_self() == empi_exec_thread) {
						while((bytes_read = read(empi_exec_client_socket,buffer+msgsize, sizeof(int)-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(int))) break;
						}
						check(bytes_read,"recv error empi num pid");
					}
					int waitpid_num = *((int *)buffer);
					pid_t *pid_buffer;
					if(waitpid_num > 0) {
						pid_buffer = (pid_t *)malloc(waitpid_num * sizeof(pid_t));
						msgsize = 0;
						if(pthread_self() == empi_thread) {
							while((bytes_read = read(empi_client_socket,pid_buffer+msgsize, (sizeof(pid_t)*waitpid_num)-msgsize)) > 0) {
								msgsize += bytes_read;
								if(msgsize >= (sizeof(pid_t) * waitpid_num)) break;
							}
							check(bytes_read,"recv error empi pid buffer");
						} if(pthread_self() == empi_exec_thread) {
							while((bytes_read = read(empi_exec_client_socket,pid_buffer+msgsize, (sizeof(pid_t)*waitpid_num)-msgsize)) > 0) {
								msgsize += bytes_read;
								if(msgsize >= (sizeof(pid_t) * waitpid_num)) break;
							}
							check(bytes_read,"recv error empi pid buffer");
						}
						handle_waitpid_output(pid_buffer,waitpid_num);
						free(pid_buffer);
					}
					
					if(wnohang_used) {
					} else {
						if(waitpid_num > 0) {
						} else {
							int empi_safe_ret = 0;
							while(empi_safe_ret == 0) {
								pthread_mutex_lock(&empi_waitpid_safe_mutex);
								if(empi_waitpid_safe == 0) pthread_cond_wait(&empi_waitpid_safe_cond,&empi_waitpid_safe_mutex);
								empi_safe_ret = empi_waitpid_safe;
								pthread_mutex_unlock(&empi_waitpid_safe_mutex);
							}
							assert(empi_safe_ret == 1);
						}
					}
				}
			}
		}
	}
}

void get_rank_from_pid(pid_t pid, int *rank, int *node_rank) {
	*rank = -1;
	*node_rank = -1;
	for(int i = 0; i < parep_mpi_node_size; i++) {
		if(parep_mpi_pids[i] == pid) {
			*node_rank = i;
			*rank = parep_mpi_ranks[i];
			return;
		}
	}
}

void handle_waitpid_output(pid_t *pid_buffer, int pid_num) {
	int num_proc_update = 0;
	bool *proc_updated = (bool *)malloc(sizeof(bool) * parep_mpi_node_size);
	for(int j = 0; j < parep_mpi_node_size; j++) proc_updated[j] = false;
	
	pthread_mutex_lock(&proc_state_mutex);
	for(int i = 0; i < pid_num; i++) {
		int node_rank;
		int rank;
		get_rank_from_pid(pid_buffer[i],&rank,&node_rank);
		assert((rank >= 0) && (node_rank >= 0));
		
		if(isMainServer) {
			assert((rank >= rank_lims_all[0].start) && (rank <= rank_lims_all[0].end));
			if(global_proc_state[rank] != PROC_TERMINATED) {
				global_proc_state[rank] = PROC_TERMINATED;
				num_proc_update++;
				proc_updated[rank] = true;
				pthread_mutex_lock(&parep_mpi_num_failed_mutex);
				parep_mpi_num_failed++;
				pthread_cond_signal(&parep_mpi_barrier_count_cond);
				pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
			}
		} else {
			if(local_proc_state[node_rank] != PROC_TERMINATED) {
				local_proc_state[node_rank] = PROC_TERMINATED;
				num_proc_update++;
				proc_updated[node_rank] = true;
				pthread_mutex_lock(&parep_mpi_num_failed_mutex);
				parep_mpi_num_failed++;
				pthread_cond_signal(&parep_mpi_barrier_count_cond);
				pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
			}
		}
	}
	
	if((!isMainServer) && (num_proc_update > 0)) {
		int dsock;
		check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
		int ret;
		do {
			ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
		} while(ret != 0);
		int cmd = CMD_PROC_STATE_UPDATE;
		write_to_fd(dsock,&cmd,sizeof(int));
		write_to_fd(dsock,&num_proc_update,sizeof(int));
		
		char *proc_state_data_buf = (char *)malloc(((2*sizeof(int))+sizeof(pid_t)) * num_proc_update);
		size_t offset = 0;
		int local_terminated = 1;
		for(int i = 0; i < parep_mpi_node_size; i++) {
			if(proc_updated[i]) {
				char *proc_state_data = proc_state_data_buf + offset;
				*((int *)proc_state_data) = parep_mpi_ranks[i];
				*((int *)(proc_state_data+sizeof(int))) = local_proc_state[i];
				*((pid_t *)(proc_state_data+(2*sizeof(int)))) = parep_mpi_pids[i];
				offset += (2*sizeof(int))+sizeof(pid_t);
			}
			if(local_proc_state[i] != PROC_TERMINATED) {
				local_terminated = 0;
			}
		}
		write_to_fd(dsock,proc_state_data_buf,(((2*sizeof(int))+sizeof(pid_t)) * num_proc_update));
		free(proc_state_data_buf);
		if(local_terminated) {
			pthread_mutex_lock(&local_procs_term_mutex);
			local_procs_term = local_terminated;
			pthread_cond_signal(&local_procs_term_cond);
			pthread_mutex_unlock(&local_procs_term_mutex);
		}
		close(dsock);
	}
	
	if((isMainServer) && (num_proc_update > 0)) {
		int exit_recvd;
		int local_term;
		pthread_mutex_lock(&exit_cmd_recvd_mutex);
		exit_recvd = exit_cmd_recvd;
		pthread_mutex_unlock(&exit_cmd_recvd_mutex);
		pthread_mutex_lock(&local_procs_term_mutex);
		local_term = local_procs_term;
		pthread_mutex_unlock(&local_procs_term_mutex);
		if((exit_recvd == 0) && (local_term == 0)) {
			int local_terminated = 1;
			char *proc_state_data_buf = (char *)malloc(sizeof(int) * num_proc_update);
			size_t offset = 0;
			for(int i = 0; i < parep_mpi_node_size; i++) {
				if(proc_updated[i]) {
					char *proc_state_data = proc_state_data_buf + offset;
					*((int *)proc_state_data) = parep_mpi_ranks[i];
					offset += sizeof(int);
				}
				if(global_proc_state[i] != PROC_TERMINATED) {
					local_terminated = 0;
				}
			}
			int cmd = CMD_INFORM_PROC_FAILED;
			pthread_mutex_lock(&parep_mpi_fail_ready_mutex);
			parep_mpi_fail_ready = 1;
			pthread_mutex_unlock(&parep_mpi_fail_ready_mutex);
			pthread_cond_signal(&proc_state_cond);
			pthread_mutex_unlock(&proc_state_mutex);
			pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
			pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
			while(parep_mpi_barrier_running == 1) {
				pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
				pthread_cond_wait(&parep_mpi_barrier_running_cond,&parep_mpi_barrier_running_mutex);
				if(parep_mpi_barrier_running == 0) pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
			}
			pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
			pthread_mutex_lock(&proc_state_mutex);
			pthread_mutex_lock(&daemon_sock_mutex);
			for(int j = 1; j < parep_mpi_node_num; j++) {
				if((daemon_state[j] != PROC_TERMINATED)  && ((parep_mpi_node_group_ids[j] == 0) || (parep_mpi_node_group_nodeids[j] == parep_mpi_node_group_leader_nodeids[j]))) {
					write_to_fd(daemon_socket[j],&cmd,sizeof(int));
					write_to_fd(daemon_socket[j],&num_proc_update,sizeof(int));
					write_to_fd(daemon_socket[j],proc_state_data_buf, sizeof(int) * num_proc_update);
				}
			}
			pthread_mutex_unlock(&daemon_sock_mutex);
			pthread_mutex_lock(&client_sock_mutex);
			for(int j = 0; j < parep_mpi_node_size; j++) {
				if(global_proc_state[j] != PROC_TERMINATED) {
					write_to_fd(client_socket[j],&cmd,sizeof(int));
					write_to_fd(client_socket[j],&num_proc_update,sizeof(int));
					write_to_fd(client_socket[j],proc_state_data_buf, sizeof(int) * num_proc_update);
				}
			}
			pthread_mutex_unlock(&client_sock_mutex);
			int *fprocs = (int *)proc_state_data_buf;
			for(int j = 0; j < num_proc_update; j++) {
				if(!parep_mpi_inform_failed_check[fprocs[j]]) {
					parep_mpi_inform_failed_check[fprocs[j]] = true;
					parep_mpi_num_inform_failed++;
					int nodeid = fprocs[j] / parep_mpi_node_size;
					int nodegroupid = parep_mpi_node_group_ids[nodeid];
					parep_mpi_num_inform_failed_node[nodeid]++;
					parep_mpi_num_inform_failed_node_group[nodegroupid]++;
				}
			}
			pthread_mutex_lock(&parep_mpi_fail_ready_mutex);
			parep_mpi_fail_ready = 0;
			pthread_mutex_unlock(&parep_mpi_fail_ready_mutex);
			pthread_mutex_unlock(&proc_state_mutex);
			pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
			pthread_mutex_lock(&proc_state_mutex);
			free(proc_state_data_buf);
			if(local_terminated) {
				pthread_mutex_lock(&local_procs_term_mutex);
				local_procs_term = local_terminated;
				pthread_cond_signal(&local_procs_term_cond);
				pthread_mutex_unlock(&local_procs_term_mutex);
			}
		}
	}
	
	pthread_cond_signal(&proc_state_cond);
	pthread_mutex_unlock(&proc_state_mutex);
	
	free(proc_updated);
}

void *ethread_function(void *arg) {
	int csock = *((int *)arg);
	free(arg);
	handle_exit_cmd(csock);
	if(!isMainServer) exit(0);
	if(parep_mpi_completed) exit(RET_COMPLETED);
	else exit(RET_INCOMPLETE);
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
			handle_query_cmd(client_socket);
			free(qn);
		}
	}
}

void *propthread_function(void *arg) {
	while(true) {
		propnode_t *propn;
		pthread_mutex_lock(&propmutex);
		if((propn = propdequeue()) == NULL) {
			pthread_mutex_lock(&parep_mpi_propagating_mutex);
			parep_mpi_propagating = 0;
			pthread_cond_broadcast(&parep_mpi_propagating_cond);
			pthread_mutex_unlock(&parep_mpi_propagating_mutex);
			pthread_cond_wait(&propcondition_var, &propmutex);
			propn = propdequeue();
		}
		pthread_mutex_lock(&parep_mpi_propagating_mutex);
		parep_mpi_propagating = 1;
		pthread_cond_broadcast(&parep_mpi_propagating_cond);
		pthread_mutex_unlock(&parep_mpi_propagating_mutex);
		pthread_mutex_unlock(&propmutex);
		if(propn != NULL) {
			void *buffer = propn->buffer;
			size_t size = propn->size;
			perform_propagate(buffer,size);
			free(buffer);
			free(propn);
		}
	}
}

void perform_propagate(void *buffer, size_t size) {
	if(isMainServer) {
		int dsock;
		for(int i = 1; i < parep_mpi_node_num; i++) {
			if(((parep_mpi_node_group_ids[i] == 0) || (parep_mpi_node_group_nodeids[i] == parep_mpi_node_group_leader_nodeids[i]))) {
				check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int ret;
				do {
					ret = connect(dsock,(struct sockaddr *)(&(coordinator_addr[i])),sizeof(SA_IN));
				} while(ret != 0);
				write_to_fd(dsock,buffer,size);
				close(dsock);
			}
		}
	} else {
		if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
			int cmd = *((int *)buffer);
			if((cmd == CMD_STORE) || (cmd == CMD_STORE_MULTIPLE)) {
				int dsock;
				check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int ret;
				do {
					ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
				} while(ret != 0);
				write_to_fd(dsock,buffer,size);
				close(dsock);
			} else if((cmd == CMD_STORE_NO_PROP) || (cmd == CMD_STORE_MULTIPLE_NO_PROP)) {
				int dsock;
				for(int i = 1; i < parep_mpi_node_group_size; i++) {
					check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
					int ret;
					do {
						ret = connect(dsock,(struct sockaddr *)(&(group_coordinator_addr[i])),sizeof(SA_IN));
					} while(ret != 0);
					write_to_fd(dsock,buffer,size);
					close(dsock);
				}
			}
		} else {
			int dsock;
			check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
			int ret;
			do {
				ret = connect(dsock,(struct sockaddr *)(&(group_main_coordinator_addr)),sizeof(group_main_coordinator_addr));
			} while(ret != 0);
			write_to_fd(dsock,buffer,size);
			close(dsock);
		}
	}
}

uint16_t *lid_add_pair_multiple(uint16_t *virt_lid, uint16_t *lid, int storenum) {
	pthread_mutex_lock(&lid_pair_mutex);
	uint16_t *vlid = (uint16_t *)malloc(storenum*sizeof(uint16_t));
	for(int i = 0; i < storenum; i++) {
		vlid[i] = (uint16_t)-1;
		if(virt_lid[i] ==  (uint16_t)-1) {
			//Check if it already exists
			for(int hash = 0; hash < LID_HASH_KEYS; hash++) {
				if(lid_map[hash] != NULL) {
					lid_pair_t *temp = lid_map[hash];
					while(temp != NULL) {
						if(temp->real_lid == lid[i]) {
							vlid[i] = temp->virt_lid;
							hash = LID_HASH_KEYS;
							break;
						}
						temp = temp->next;
					}
				}
			}
			if((virt_lid[i] == (uint16_t)-1) && (vlid[i] == (uint16_t)-1)) {
				pthread_mutex_lock(&lid_mutex);
				vlid[i] = current_lid;
				current_lid += parep_mpi_node_num;
				pthread_mutex_unlock(&lid_mutex);
			} else {
				continue;
			}
		} else {
			vlid[i] = virt_lid[i];
		}
		
		int hash = lid_hash(vlid[i]);
		if(lid_map[hash] == NULL) {
			lid_pair_t *pair = (lid_pair_t *)malloc(sizeof(lid_pair_t));
			pair->virt_lid = vlid[i];
			pair->real_lid = lid[i];
			pair->next = (lid_pair_t *)NULL;
			lid_map[hash] = pair;
		} else {
			lid_pair_t *temp = lid_map[hash];
			if(temp->virt_lid == vlid[i]) {
				temp->real_lid = lid[i];
			} else {
				while(temp->next != NULL) {
					temp = temp->next;
					if(temp->virt_lid == vlid[i]) {
						temp->real_lid = lid[i];
						break;
					}
				}
				if(temp->next == NULL) {
					lid_pair_t *pair = (lid_pair_t *)malloc(sizeof(lid_pair_t));
					pair->virt_lid = vlid[i];
					pair->real_lid = lid[i];
					pair->next = (lid_pair_t *)NULL;
					temp->next = pair;
				}
			}
		}
	}
	pthread_cond_broadcast(&lid_pair_cond);
	pthread_mutex_unlock(&lid_pair_mutex);
	return vlid;
}

uint16_t lid_add_pair(uint16_t virt_lid, uint16_t lid) {
	if(virt_lid == (uint16_t)-1) {
		//Check if it already exists
		pthread_mutex_lock(&lid_pair_mutex);
		for(int hash = 0; hash < LID_HASH_KEYS; hash++) {
			if(lid_map[hash] != NULL) {
				lid_pair_t *temp = lid_map[hash];
				while(temp != NULL) {
					if(temp->real_lid == lid) {
						virt_lid = temp->virt_lid;
						hash = LID_HASH_KEYS;
						break;
					}
					temp = temp->next;
				}
			}
		}
		pthread_mutex_unlock(&lid_pair_mutex);
		if(virt_lid == (uint16_t)-1) {
			pthread_mutex_lock(&lid_mutex);
			virt_lid = current_lid;
			current_lid += parep_mpi_node_num;
			pthread_mutex_unlock(&lid_mutex);
		} else {
			return virt_lid;
		}
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
	current_lid = ((uint16_t)parep_mpi_node_id) + 1;
	pthread_mutex_unlock(&lid_mutex);
}

uint32_t *rkey_add_pair_multiple(uint32_t *virt_rkey, uint32_t *pd_id, uint32_t *real_rkey, int storenum) {
	pthread_mutex_lock(&rkey_pair_mutex);
	uint32_t *vrkey = (uint32_t *)malloc(storenum*sizeof(uint32_t));
	for(int i = 0; i < storenum; i++) {
		if(virt_rkey[i] == (uint32_t)-1) {
			pthread_mutex_lock(&rkey_mutex);
			vrkey[i] = current_rkey;
			current_rkey++;
			pthread_mutex_unlock(&rkey_mutex);
		} else {
			pthread_mutex_lock(&rkey_mutex);
			if((virt_rkey[i] >= current_rkey) && (virt_rkey[i] >= rkey_lims[0]) && (virt_rkey[i] <= rkey_lims[1])) current_rkey = virt_rkey[i] + 1;
			pthread_mutex_unlock(&rkey_mutex);
			pthread_mutex_lock(&pd_id_mutex);
			if((pd_id[i] >= current_pd_id) && (pd_id[i] >= pd_id_lims[0]) && (pd_id[i] <= pd_id_lims[1])) current_pd_id = pd_id[i] + 1;
			pthread_mutex_unlock(&pd_id_mutex);
			vrkey[i] = virt_rkey[i];
		}
		
		int hash = rkey_hash(vrkey[i],pd_id[i]);
		if(rkey_map[hash] == NULL) {
			rkey_pair_t *pair = (rkey_pair_t *)malloc(sizeof(rkey_pair_t));
			pair->virt_rkey = vrkey[i];
			pair->pd_id = pd_id[i];
			pair->real_rkey = real_rkey[i];
			pair->next = (rkey_pair_t *)NULL;
			rkey_map[hash] = pair;
		} else {
			rkey_pair_t *temp = rkey_map[hash];
			if((temp->virt_rkey == vrkey[i]) && (temp->pd_id == pd_id[i])) {
				temp->real_rkey = real_rkey[i];
			} else {
				while(temp->next != NULL) {
					temp = temp->next;
					if((temp->virt_rkey == vrkey[i]) && (temp->pd_id == pd_id[i])) {
						temp->real_rkey = real_rkey[i];
						break;
					}
				}
				if(temp->next == NULL) {
					rkey_pair_t *pair = (rkey_pair_t *)malloc(sizeof(rkey_pair_t));
					pair->virt_rkey = vrkey[i];
					pair->pd_id = pd_id[i];
					pair->real_rkey = real_rkey[i];
					pair->next = (rkey_pair_t *)NULL;
					temp->next = pair;
				}
			}
		}
	}
	pthread_cond_broadcast(&rkey_pair_cond);
	pthread_mutex_unlock(&rkey_pair_mutex);
	return vrkey;
}

uint32_t rkey_add_pair(uint32_t virt_rkey, uint32_t pd_id, uint32_t real_rkey) {
	if(virt_rkey == (uint32_t)-1) {
		pthread_mutex_lock(&rkey_mutex);
		virt_rkey = current_rkey;
		current_rkey++;
		pthread_mutex_unlock(&rkey_mutex);
	} else {
		pthread_mutex_lock(&rkey_mutex);
		if((virt_rkey >= current_rkey) && (virt_rkey >= rkey_lims[0]) && (virt_rkey <= rkey_lims[1])) current_rkey = virt_rkey + 1;
		pthread_mutex_unlock(&rkey_mutex);
		pthread_mutex_lock(&pd_id_mutex);
		if((pd_id >= current_pd_id) && (pd_id >= pd_id_lims[0]) && (pd_id <= pd_id_lims[1])) current_pd_id = pd_id + 1;
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
	current_rkey = rkey_lims[0];
	pthread_mutex_unlock(&rkey_mutex);
	pthread_mutex_lock(&pd_id_mutex);
	current_pd_id = pd_id_lims[0];
	pthread_mutex_unlock(&pd_id_mutex);
}

uint32_t *qp_add_pair_multiple(uint32_t *virt_qp_num, uint32_t *real_qp_num, uint32_t *pd_id, int storenum) {
	pthread_mutex_lock(&qp_pair_mutex);
	uint32_t *vqpnum = (uint32_t *)malloc(storenum*sizeof(uint32_t));
	for(int i = 0; i < storenum; i++) {
		if(virt_qp_num[i] == (uint32_t)-1) {
			pthread_mutex_lock(&qp_num_mutex);
			vqpnum[i] = current_qp_num;
			current_qp_num++;
			pthread_mutex_unlock(&qp_num_mutex);
		} else {
			pthread_mutex_lock(&qp_num_mutex);
			if((virt_qp_num[i] >= current_qp_num) && (virt_qp_num[i] >= qp_num_lims[0]) && (virt_qp_num[i] <= qp_num_lims[1])) current_qp_num = virt_qp_num[i] + 1;
			pthread_mutex_unlock(&qp_num_mutex);
			pthread_mutex_lock(&pd_id_mutex);
			if((pd_id[i] >= current_pd_id) && (pd_id[i] >= pd_id_lims[0]) && (pd_id[i] <= pd_id_lims[1])) current_pd_id = pd_id[i] + 1;
			pthread_mutex_unlock(&pd_id_mutex);
			vqpnum[i] = virt_qp_num[i];
		}
		
		int hash = qp_hash(vqpnum[i]);
		if(qp_map[hash] == NULL) {
			qp_pair_t *pair = (qp_pair_t *)malloc(sizeof(qp_pair_t));
			pair->virt_qp_num = vqpnum[i];
			pair->real_qp_num = real_qp_num[i];
			pair->pd_id = pd_id[i];
			pair->next = (qp_pair_t *)NULL;
			qp_map[hash] = pair;
		} else {
			qp_pair_t *temp = qp_map[hash];
			if((temp->virt_qp_num == vqpnum[i])) {
				temp->real_qp_num = real_qp_num[i];
				temp->pd_id = pd_id[i];
			} else {
				while(temp->next != NULL) {
					temp = temp->next;
					if((temp->virt_qp_num == vqpnum[i])) {
						temp->real_qp_num = real_qp_num[i];
						temp->pd_id = pd_id[i];
						break;
					}
				}
				if(temp->next == NULL) {
					qp_pair_t *pair = (qp_pair_t *)malloc(sizeof(qp_pair_t));
					pair->virt_qp_num = vqpnum[i];
					pair->real_qp_num = real_qp_num[i];
					pair->pd_id = pd_id[i];
					pair->next = (qp_pair_t *)NULL;
					temp->next = pair;
				}
			}
		}
	}
	pthread_cond_broadcast(&qp_pair_cond);
	pthread_mutex_unlock(&qp_pair_mutex);
	return vqpnum;
}

uint32_t qp_add_pair(uint32_t virt_qp_num, uint32_t real_qp_num, uint32_t pd_id) {
	if(virt_qp_num == (uint32_t)-1) {
		pthread_mutex_lock(&qp_num_mutex);
		virt_qp_num = current_qp_num;
		current_qp_num++;
		pthread_mutex_unlock(&qp_num_mutex);
	} else {
		pthread_mutex_lock(&qp_num_mutex);
		if((virt_qp_num >= current_qp_num) && (virt_qp_num >= qp_num_lims[0]) && (virt_qp_num <= qp_num_lims[1])) current_qp_num = virt_qp_num + 1;
		pthread_mutex_unlock(&qp_num_mutex);
		pthread_mutex_lock(&pd_id_mutex);
		if((pd_id >= current_pd_id) && (pd_id >= pd_id_lims[0]) && (pd_id <= pd_id_lims[1])) current_pd_id = pd_id + 1;
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
	current_qp_num = qp_num_lims[0];
	pthread_mutex_unlock(&qp_num_mutex);
}

void handle_store_multiple_cmd(int csocket, bool propagate) {
	char id[20];
	int msgsize = 0;
	int storenum;
	size_t bytes_read;
	while((bytes_read = read(csocket,(&storenum)+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	msgsize = 0;
	while((bytes_read = read(csocket,id+msgsize, 20 - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= 20) break;
	}
	if(!strcmp(id,"ib_lid")) {
		uint16_t *virt_lid = (uint16_t *)malloc(storenum*sizeof(uint16_t));
		uint16_t *lid = (uint16_t *)malloc(storenum*sizeof(uint16_t));
		msgsize = 0;
		while((bytes_read = read(csocket,((char *)virt_lid)+msgsize, (storenum*sizeof(uint16_t)) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= (storenum*sizeof(uint16_t))) break;
		}
		msgsize = 0;
		while((bytes_read = read(csocket,((char *)lid)+msgsize, (storenum*sizeof(uint16_t)) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= (storenum*sizeof(uint16_t))) break;
		}
		uint16_t *vlid = lid_add_pair_multiple(virt_lid, lid, storenum);
		
		for(int i = 0; i < storenum; i++) {
			if(virt_lid[i] == (uint16_t)-1) {
				write_to_fd(csocket, vlid, storenum*sizeof(uint16_t));
				break;
			}
		}
		close(csocket);
		pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
		parep_mpi_num_dyn_socks--;
		pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
		pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
		if(propagate || (parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid)) {
			int cmd;
			if(isMainServer) cmd = CMD_STORE_MULTIPLE_NO_PROP;
			else if((parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) && (!propagate)) cmd = CMD_STORE_MULTIPLE_NO_PROP;
			else cmd = CMD_STORE_MULTIPLE;
			
			void *propbuf = malloc(sizeof(int) + sizeof(int) + 20 + (storenum*sizeof(uint16_t)) + (storenum*sizeof(uint16_t)));
			
			char id_buf[20];
			*((int *)propbuf) = cmd;
			strcpy(id_buf,"ib_lid");
			memcpy(propbuf + sizeof(int), &storenum, sizeof(int));
			memcpy(propbuf + sizeof(int) + sizeof(int), id_buf, 20);
			memcpy(propbuf + sizeof(int) + sizeof(int) + 20, vlid, storenum*sizeof(uint16_t));
			memcpy(propbuf + sizeof(int) + sizeof(int) + 20 + (storenum*sizeof(uint16_t)), lid, storenum*sizeof(uint16_t));
			
			pthread_mutex_lock(&propmutex);
			propenqueue(propbuf,sizeof(int) + sizeof(int) + 20 + (storenum*sizeof(uint16_t)) + (storenum*sizeof(uint16_t)));
			pthread_cond_signal(&propcondition_var);
			pthread_mutex_unlock(&propmutex);
		}
		free(vlid);
		free(virt_lid);
		free(lid);
	} else if(!strcmp(id,"ib_qp")) {
		uint32_t *virt_qp_num = (uint32_t *)malloc(storenum*sizeof(uint32_t));
		uint32_t *real_qp_num = (uint32_t *)malloc(storenum*sizeof(uint32_t));
		uint32_t *pd_id = (uint32_t *)malloc(storenum*sizeof(uint32_t));
		msgsize = 0;
		while((bytes_read = read(csocket,((char *)virt_qp_num)+msgsize, (storenum*sizeof(uint32_t)) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= (storenum*sizeof(uint32_t))) break;
		}
		msgsize = 0;
		while((bytes_read = read(csocket,((char *)real_qp_num)+msgsize, (storenum*sizeof(uint32_t)) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= (storenum*sizeof(uint32_t))) break;
		}
		msgsize = 0;
		while((bytes_read = read(csocket,((char *)pd_id)+msgsize, (storenum*sizeof(uint32_t)) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= (storenum*sizeof(uint32_t))) break;
		}
		uint32_t *vqpnum = qp_add_pair_multiple(virt_qp_num, real_qp_num, pd_id, storenum);
		
		for(int i = 0; i < storenum; i++) {
			if(virt_qp_num[i] == (uint32_t)-1) {
				write_to_fd(csocket, vqpnum, storenum*sizeof(uint32_t));
				break;
			}
		}
		close(csocket);
		pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
		parep_mpi_num_dyn_socks--;
		pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
		pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
		if(propagate || (parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid)) {
			int cmd;
			if(isMainServer) cmd = CMD_STORE_MULTIPLE_NO_PROP;
			else if((parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) && (!propagate)) cmd = CMD_STORE_MULTIPLE_NO_PROP;
			else cmd = CMD_STORE_MULTIPLE;
			
			void *propbuf = malloc(sizeof(int) + sizeof(int) + 20 + (storenum*sizeof(uint32_t)) + (storenum*sizeof(uint32_t)) + (storenum*sizeof(uint32_t)));
			
			char id_buf[20];
			*((int *)propbuf) = cmd;
			strcpy(id_buf,"ib_qp");
			memcpy(propbuf + sizeof(int), &storenum, sizeof(int));
			memcpy(propbuf + sizeof(int) + sizeof(int), id_buf, 20);
			memcpy(propbuf + sizeof(int) + sizeof(int) + 20, vqpnum, storenum*sizeof(uint32_t));
			memcpy(propbuf + sizeof(int) + sizeof(int) + 20 + (storenum*sizeof(uint32_t)), real_qp_num, storenum*sizeof(uint32_t));
			memcpy(propbuf + sizeof(int) + sizeof(int) + 20 + (storenum*sizeof(uint32_t)) + (storenum*sizeof(uint32_t)), pd_id, storenum*sizeof(uint32_t));
			
			pthread_mutex_lock(&propmutex);
			propenqueue(propbuf,sizeof(int) + sizeof(int) + 20 + (storenum*sizeof(uint32_t)) + (storenum*sizeof(uint32_t)) + (storenum*sizeof(uint32_t)));
			pthread_cond_signal(&propcondition_var);
			pthread_mutex_unlock(&propmutex);
		}
		free(vqpnum);
		free(virt_qp_num);
		free(real_qp_num);
		free(pd_id);
	} else if(!strcmp(id,"ib_rkey")) {
		uint32_t *virt_rkey = (uint32_t *)malloc(storenum*sizeof(uint32_t));
		uint32_t *pd_id = (uint32_t *)malloc(storenum*sizeof(uint32_t));
		uint32_t *real_rkey = (uint32_t *)malloc(storenum*sizeof(uint32_t));
		msgsize = 0;
		while((bytes_read = read(csocket,((char *)virt_rkey)+msgsize,  (storenum*sizeof(uint32_t)) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >=  (storenum*sizeof(uint32_t))) break;
		}
		msgsize = 0;
		while((bytes_read = read(csocket,((char *)pd_id)+msgsize,  (storenum*sizeof(uint32_t)) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >=  (storenum*sizeof(uint32_t))) break;
		}
		msgsize = 0;
		while((bytes_read = read(csocket,((char *)real_rkey)+msgsize,  (storenum*sizeof(uint32_t)) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >=  (storenum*sizeof(uint32_t))) break;
		}
		uint32_t *vrkey = rkey_add_pair_multiple(virt_rkey,pd_id,real_rkey,storenum);
		
		for(int i = 0; i < storenum; i++) {
			if(virt_rkey[i] == (uint32_t)-1) {
				write_to_fd(csocket, vrkey, storenum*sizeof(uint32_t));
				break;
			}
		}
		close(csocket);
		pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
		parep_mpi_num_dyn_socks--;
		pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
		pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
		if(propagate || (parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid)) {
			int cmd;
			if(isMainServer) cmd = CMD_STORE_MULTIPLE_NO_PROP;
			else if((parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) && (!propagate)) cmd = CMD_STORE_MULTIPLE_NO_PROP;
			else cmd = CMD_STORE_MULTIPLE;
			
			void *propbuf = malloc(sizeof(int) + sizeof(int) + 20 + (storenum*sizeof(uint32_t)) + (storenum*sizeof(uint32_t)) + (storenum*sizeof(uint32_t)));
			
			char id_buf[20];
			*((int *)propbuf) = cmd;
			strcpy(id_buf,"ib_rkey");
			memcpy(propbuf + sizeof(int), &storenum, sizeof(int));
			memcpy(propbuf + sizeof(int) + sizeof(int), id_buf, 20);
			memcpy(propbuf + sizeof(int) + sizeof(int) + 20, vrkey, storenum*sizeof(uint32_t));
			memcpy(propbuf + sizeof(int) + sizeof(int) + 20 + (storenum*sizeof(uint32_t)), pd_id, storenum*sizeof(uint32_t));
			memcpy(propbuf + sizeof(int) + sizeof(int) + 20 + (storenum*sizeof(uint32_t)) + (storenum*sizeof(uint32_t)), real_rkey, storenum*sizeof(uint32_t));
			
			pthread_mutex_lock(&propmutex);
			propenqueue(propbuf,sizeof(int) + sizeof(int) + 20 + (storenum*sizeof(uint32_t)) + (storenum*sizeof(uint32_t)) + (storenum*sizeof(uint32_t)));
			pthread_cond_signal(&propcondition_var);
			pthread_mutex_unlock(&propmutex);
		}
		free(vrkey);
		free(virt_rkey);
		free(pd_id);
		free(real_rkey);
	} else {
		perror("Invalid id");
		exit(1);
	}
}

void handle_store_cmd(int client_socket, bool propagate) {
	char id[20];
	char buffer[BUFFER_SIZE];
	int msgsize = 0;
	size_t bytes_read;
	while((bytes_read = read(client_socket,id+msgsize, 20 - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= 20) break;
	}
	if(!strcmp(id,"ib_lid")) {
		uint16_t virt_lid;
		uint16_t lid;
		msgsize = 0;
		while((bytes_read = read(client_socket,(&virt_lid)+msgsize, sizeof(uint16_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(uint16_t)) break;
		}
		msgsize = 0;
		while((bytes_read = read(client_socket,(&lid)+msgsize, sizeof(uint16_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(uint16_t)) break;
		}
		uint16_t vlid = lid_add_pair(virt_lid, lid);

		if(virt_lid == (uint16_t)-1) {
			write_to_fd(client_socket, &vlid, sizeof(uint16_t));
		}
		close(client_socket);
		pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
		parep_mpi_num_dyn_socks--;
		pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
		pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
		if(propagate || (parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid)) {
			int cmd;
			if(isMainServer) cmd = CMD_STORE_NO_PROP;
			else if((parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) && (!propagate)) cmd = CMD_STORE_NO_PROP;
			else cmd = CMD_STORE;
			
			void *propbuf = malloc(sizeof(int) + 20 + sizeof(uint16_t) + sizeof(uint16_t));
			
			char id_buf[20];
			*((int *)propbuf) = cmd;
			strcpy(id_buf,"ib_lid");
			memcpy(propbuf + sizeof(int), id_buf, 20);
			memcpy(propbuf + sizeof(int) + 20, &vlid, sizeof(uint16_t));
			memcpy(propbuf + sizeof(int) + 20 + sizeof(uint16_t), &lid, sizeof(uint16_t));
			
			pthread_mutex_lock(&propmutex);
			propenqueue(propbuf,sizeof(int) + 20 + sizeof(uint16_t) + sizeof(uint16_t));
			pthread_cond_signal(&propcondition_var);
			pthread_mutex_unlock(&propmutex);
		}
	} else if(!strcmp(id,"ib_qp")) {
		uint32_t virt_qp_num;
		uint32_t real_qp_num;
		uint32_t pd_id;
		msgsize = 0;
		while((bytes_read = read(client_socket,(&virt_qp_num)+msgsize, sizeof(uint32_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(uint32_t)) break;
		}
		msgsize = 0;
		while((bytes_read = read(client_socket,(&real_qp_num)+msgsize, sizeof(uint32_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(uint32_t)) break;
		}
		msgsize = 0;
		while((bytes_read = read(client_socket,(&pd_id)+msgsize, sizeof(uint32_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(uint32_t)) break;
		}
		uint32_t vqpnum = qp_add_pair(virt_qp_num,real_qp_num,pd_id);

		if(virt_qp_num == (uint32_t)-1) {
			write_to_fd(client_socket, &vqpnum, sizeof(uint32_t));
		}
		close(client_socket);
		pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
		parep_mpi_num_dyn_socks--;
		pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
		pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
		if(propagate || (parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid)) {
			int cmd;
			if(isMainServer) cmd = CMD_STORE_NO_PROP;
			else if((parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) && (!propagate)) cmd = CMD_STORE_NO_PROP;
			else cmd = CMD_STORE;
			
			void *propbuf = malloc(sizeof(int) + 20 + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t));
			
			char id_buf[20];
			*((int *)propbuf) = cmd;
			strcpy(id_buf,"ib_qp");
			memcpy(propbuf + sizeof(int), id_buf, 20);
			memcpy(propbuf + sizeof(int) + 20, &vqpnum, sizeof(uint32_t));
			memcpy(propbuf + sizeof(int) + 20 + sizeof(uint32_t), &real_qp_num, sizeof(uint32_t));
			memcpy(propbuf + sizeof(int) + 20 + sizeof(uint32_t) + sizeof(uint32_t), &pd_id, sizeof(uint32_t));
			
			pthread_mutex_lock(&propmutex);
			propenqueue(propbuf,sizeof(int) + 20 + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t));
			pthread_cond_signal(&propcondition_var);
			pthread_mutex_unlock(&propmutex);
		}
	} else if(!strcmp(id,"ib_rkey")) {
		uint32_t virt_rkey;
		uint32_t pd_id;
		uint32_t real_rkey;
		msgsize = 0;
		while((bytes_read = read(client_socket,(&virt_rkey)+msgsize, sizeof(uint32_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(uint32_t)) break;
		}
		msgsize = 0;
		while((bytes_read = read(client_socket,(&pd_id)+msgsize, sizeof(uint32_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(uint32_t)) break;
		}
		msgsize = 0;
		while((bytes_read = read(client_socket,(&real_rkey)+msgsize, sizeof(uint32_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(uint32_t)) break;
		}
		uint32_t vrkey = rkey_add_pair(virt_rkey,pd_id,real_rkey);

		if(virt_rkey == (uint32_t)-1) {
			write_to_fd(client_socket, &vrkey, sizeof(uint32_t));
		}
		close(client_socket);
		pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
		parep_mpi_num_dyn_socks--;
		pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
		pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
		if(propagate || (parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid)) {
			int cmd;
			if(isMainServer) cmd = CMD_STORE_NO_PROP;
			else if((parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) && (!propagate)) cmd = CMD_STORE_NO_PROP;
			else cmd = CMD_STORE;
			
			void *propbuf = malloc(sizeof(int) + 20 + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t));
			
			char id_buf[20];
			*((int *)propbuf) = cmd;
			strcpy(id_buf,"ib_rkey");
			memcpy(propbuf + sizeof(int), id_buf, 20);
			memcpy(propbuf + sizeof(int) + 20, &vrkey, sizeof(uint32_t));
			memcpy(propbuf + sizeof(int) + 20 + sizeof(uint32_t), &pd_id, sizeof(uint32_t));
			memcpy(propbuf + sizeof(int) + 20 + sizeof(uint32_t) + sizeof(uint32_t), &real_rkey, sizeof(uint32_t));
			
			pthread_mutex_lock(&propmutex);
			propenqueue(propbuf,sizeof(int) + 20 + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t));
			pthread_cond_signal(&propcondition_var);
			pthread_mutex_unlock(&propmutex);
		}
	} else {
		perror("Invalid id");
		exit(1);
	}
}

void handle_query_cmd(int csocket) {
	char id[20];
	char buffer[BUFFER_SIZE];
	int msgsize = 0;
	size_t bytes_read;
	while((bytes_read = read(csocket,id+msgsize, 20 - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= 20) break;
	}
	if(!strcmp(id,"ib_lid")) {
		uint16_t virt_lid;
		msgsize = 0;
		while((bytes_read = read(csocket,(&virt_lid)+msgsize, sizeof(uint16_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(uint16_t)) break;
		}
		uint16_t real_lid = lid_get_pair(virt_lid);
		
		write_to_fd(csocket, &real_lid, sizeof(uint16_t));
	} else if(!strcmp(id,"ib_qp")) {
		uint32_t virt_qp_num;
		msgsize = 0;
		while((bytes_read = read(csocket,(&virt_qp_num)+msgsize, sizeof(uint32_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(uint32_t)) break;
		}
		uint32_t pd_id;
		uint32_t real_qp_num = qp_get_pair(virt_qp_num, &pd_id);
		
		char buf[2*sizeof(uint32_t)];
		*((uint32_t *)buf) = real_qp_num;
		*((uint32_t *)(buf + sizeof(uint32_t))) = pd_id;
		
		write_to_fd(csocket, buf, 2*sizeof(uint32_t));
	} else if(!strcmp(id,"ib_rkey")) {
		uint32_t virt_rkey;
		uint32_t pd_id;
		msgsize = 0;
		while((bytes_read = read(csocket,(&virt_rkey)+msgsize, sizeof(uint32_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(uint32_t)) break;
		}
		msgsize = 0;
		while((bytes_read = read(csocket,(&pd_id)+msgsize, sizeof(uint32_t) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(uint32_t)) break;
		}
		uint32_t real_rkey = rkey_get_pair(virt_rkey,pd_id);
		
		write_to_fd(csocket, &real_rkey, sizeof(uint32_t));
	}
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	/*int restartfd = -1;
	for(int i = 0; i < parep_mpi_node_size; i++) {
		if(client_socket[i] == csocket) {
			restartfd = i;
			break;
		}
	}
	assert(restartfd != -1);
	write_to_fd(clientpollpair[1],&restartfd,sizeof(int));*/
}

void handle_get_pd_id(int client_socket) {
	uint32_t pd_id;
	pthread_mutex_lock(&pd_id_mutex);
	pd_id = current_pd_id;
	current_pd_id++;
	pthread_mutex_unlock(&pd_id_mutex);
	
	write_to_fd(client_socket, &pd_id, sizeof(uint32_t));
	close(client_socket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
}

void handle_get_rkey(int client_socket) {
	uint32_t rkey;
	pthread_mutex_lock(&rkey_mutex);
	rkey = current_rkey;
	current_rkey++;
	pthread_mutex_unlock(&rkey_mutex);
	
	write_to_fd(client_socket, &rkey, sizeof(uint32_t));
	close(client_socket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
}

void handle_clear_store(int client_socket) {
	lid_clean_maps();
	qp_clean_maps();
	rkey_clean_maps();
	
	uint32_t resp = 0;
	write_to_fd(client_socket, &resp, sizeof(uint32_t));
	close(client_socket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
}

void wait_process_end() {
	int i = 0;
	int restartfd = 0;
	pthread_mutex_lock(&proc_state_mutex);
	while(i < parep_mpi_node_size) {
		if(isMainServer) {
			assert((i >= rank_lims_all[0].start) && (i <= rank_lims_all[0].end));
			if(global_proc_state[i] != PROC_TERMINATED) {
				pthread_cond_wait(&proc_state_cond,&proc_state_mutex);
			} else {
				i++;
			}
		} else {
			if(local_proc_state[i] != PROC_TERMINATED) {
				pthread_cond_wait(&proc_state_cond,&proc_state_mutex);
			} else {
				i++;
			}
		}
	}
	if(isMainServer) {
		while(i < parep_mpi_size) {
			if(global_proc_state[i] != PROC_TERMINATED) {
				pthread_cond_wait(&proc_state_cond,&proc_state_mutex);
			} else {
				i++;
			}
		}
		int cmd = CMD_RPROC_TERM_ACK;
		int dsock;
		for(int i = 1; i < parep_mpi_node_num; i++) {
			if(daemon_state[i] != PROC_TERMINATED) {
				check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int ret;
				do {
					ret = connect(dsock,(struct sockaddr *)(&(coordinator_addr[i])),sizeof(SA_IN));
				} while(ret != 0);
				write_to_fd(dsock,&cmd,sizeof(int));
				close(dsock);
			}
		}
		i = 1;
		while(i < parep_mpi_node_num) {
			if(daemon_state[i] != PROC_TERMINATED) {
				pthread_cond_wait(&proc_state_cond,&proc_state_mutex);
			} else {
				i++;
			}
		}
	}
	pthread_mutex_unlock(&proc_state_mutex);
	if(!isMainServer) {
		pthread_mutex_lock(&local_procs_term_mutex);
		while(local_procs_term == 0) {
			pthread_cond_wait(&local_procs_term_cond,&local_procs_term_mutex);
		}
		pthread_mutex_unlock(&local_procs_term_mutex);
		/*pthread_mutex_lock(&local_procs_term_ack_mutex);
		while(local_procs_term_ack == 0) {
			pthread_cond_wait(&local_procs_term_ack_cond,&local_procs_term_ack_mutex);
		}
		pthread_mutex_unlock(&local_procs_term_ack_mutex);*/
		pthread_mutex_lock(&remote_procs_term_ack_mutex);
		while(remote_procs_term_ack == 0) {
			pthread_cond_wait(&remote_procs_term_ack_cond,&remote_procs_term_ack_mutex);
		}
		pthread_mutex_unlock(&remote_procs_term_ack_mutex);
	}
}

void perform_cleanup() {
	//SOCKET CLEANUP
	pthread_join(empi_thread,NULL);
	if(isMainServer) pthread_join(empi_exec_thread,NULL);
	close(empi_socket);
	close(empi_client_socket);
	if(isMainServer) {
		close(empi_exec_socket);
		close(empi_exec_client_socket);
	}
	close(dyn_server_sock);
	if(isMainServer) {
		for(int i = 0; i < parep_mpi_node_size; i++) close(client_socket[i]);
		close(daemon_server_socket);
		for(int i = 1; i < parep_mpi_node_num; i++) close(daemon_socket[i]);
		close(server_socket);
	} else {
		for(int i = 0; i < parep_mpi_node_size; i++) close(client_socket[i]);
		close(daemon_server_socket);
		if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
			for(int i = 1; i < parep_mpi_node_group_size; i++) close(daemon_socket[i]);
			close(server_group_socket);
		}
		close(daemon_client_socket);
	}
	/*close(daemonpollpair[0]);
	close(daemonpollpair[1]);
	close(clientpollpair[0]);
	close(clientpollpair[1]);*/
	//DEALLOCATION
	if(isMainServer) {
		free(daemon_socket);
		free(parep_mpi_node_sizes);
		free(rank_lims_all);
		free(parep_mpi_all_ranks);
	} else if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
		free(daemon_socket);
		free(parep_mpi_node_sizes);
	}
	free(client_socket);
	free(parep_mpi_ranks);
	free(parep_mpi_pids);
	
	if(isMainServer) {
		free(parep_mpi_global_pids);
		free(global_proc_state);
		free(daemon_state);
		free(group_daemon_state);
		free(coordinator_addr);
		free(group_coordinator_addr);
	} else {
		if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
			free(group_coordinator_addr);
			free(group_daemon_state);
		}
		free(local_proc_state);
	}
	//lid_clean_maps();
	//qp_clean_maps();
	//rkey_clean_maps();
}

void handle_inform_exit_cmd(int client_socket) {
	int nodeid;
	size_t bytes_read;
	int msgsize = 0;
	while((bytes_read = read(client_socket,(&nodeid)+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	check(bytes_read,"recv error inform exit cmd recv");
	close(client_socket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	pthread_mutex_lock(&proc_state_mutex);
	if(daemon_state[nodeid] != PROC_TERMINATED) {
		daemon_state[nodeid] = PROC_TERMINATED;
	}
	/*if(daemon_state[nodeid] == PROC_POLLHUP_RECVD) {
		daemon_state[nodeid] = PROC_TERMINATED;
	} else if((daemon_state[nodeid] != PROC_TERMINATED) && (daemon_state[nodeid] != PROC_WAITPID_RECVD)) {
		daemon_state[nodeid] = PROC_WAITPID_RECVD;
	}*/
	pthread_cond_signal(&proc_state_cond);
	pthread_mutex_unlock(&proc_state_mutex);
}

void handle_exit_cmd(int csocket) {
	if(isMainServer) {
		size_t bytes_read;
		int msgsize = 0;
		while((bytes_read = read(csocket,(&parep_mpi_completed)+msgsize, sizeof(int) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(int)) break;
		}
		check(bytes_read,"recv error completed exit cmd recv");
	}
	printf("Got parep_mpi_completed %d\n",parep_mpi_completed);
	fflush(stdout);
	close(csocket);
	pthread_mutex_lock(&exit_cmd_recvd_mutex);
	exit_cmd_recvd = 1;
	pthread_mutex_unlock(&exit_cmd_recvd_mutex);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	pthread_mutex_lock(&proc_state_mutex);
	int client_cmd = CMD_EXIT_CALLED;
	pthread_mutex_lock(&client_sock_mutex);
	for(int j = 0; j < parep_mpi_node_size; j++) {
		if(isMainServer) {
			if(global_proc_state[j] != PROC_TERMINATED) {
				write_to_fd(client_socket[j],&client_cmd,sizeof(int));
			}
		} else {
			if(local_proc_state[j] != PROC_TERMINATED) {
				write_to_fd(client_socket[j],&client_cmd,sizeof(int));
			}
		}
	}
	pthread_mutex_unlock(&client_sock_mutex);
	pthread_mutex_unlock(&proc_state_mutex);
	size_t bytes_read;
	int msgsize = 0;
	char buffer[BUFFER_SIZE];
	if(isMainServer) {
		int cmd = CMD_EXIT;
		*((int *)buffer) = cmd;
		int dsock;
		pthread_mutex_lock(&proc_state_mutex);
		for(int i = 1; i < parep_mpi_node_num; i++) {
			if(daemon_state[i] != PROC_TERMINATED) {
				check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int ret;
				do {
					ret = connect(dsock,(struct sockaddr *)(&(coordinator_addr[i])),sizeof(SA_IN));
				} while(ret != 0);
				write_to_fd(dsock,buffer,sizeof(int));
				close(dsock);
			}
		}
		pthread_mutex_unlock(&proc_state_mutex);
		
		int local_term;
		pthread_mutex_lock(&local_procs_term_mutex);
		local_term = local_procs_term;
		pthread_mutex_unlock(&local_procs_term_mutex);
		if(local_term == 0) {
			struct pollfd *pfds;
			nfds_t nfds = parep_mpi_node_size;
			pfds = (struct pollfd *)malloc(nfds * sizeof(struct pollfd));
			pthread_mutex_lock(&proc_state_mutex);
			int num_failed = 0;
			int *proc_failed = (int *)malloc(sizeof(int) * parep_mpi_node_size);
			num_failed = 0;
			for(int j = 0; j < parep_mpi_node_size; j++) {
				if(global_proc_state[j] != PROC_TERMINATED) pfds[j].fd = client_socket[j];
				else pfds[j].fd = -1;
				pfds[j].events = POLLIN;
			}
			pthread_mutex_unlock(&proc_state_mutex);
			for(int j = 0; j < parep_mpi_node_size; j++) proc_failed[j] = 0;
			int pollret = 0;
			pthread_cond_signal(&proc_state_cond);
			pollret = poll(pfds,nfds,0);
			
			pthread_mutex_lock(&proc_state_mutex);
			for(int j = 0; j < parep_mpi_node_size; j++) {
				if(pfds[j].revents != 0) {
					if((pfds[j].revents & POLLHUP) || (pfds[j].revents & POLLERR)) {
						if(global_proc_state[j] != PROC_TERMINATED) {
							global_proc_state[j] = PROC_TERMINATED;
							num_failed++;
							proc_failed[j] = 1;
							pthread_mutex_lock(&parep_mpi_num_failed_mutex);
							parep_mpi_num_failed++;
							pthread_cond_signal(&parep_mpi_barrier_count_cond);
							pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
						}
					} else if(pfds[j].revents & POLLIN) {
						bytes_read = recv(pfds[j].fd,buffer,sizeof(buffer),0);
						assert(bytes_read <= 0);
						if(global_proc_state[j] != PROC_TERMINATED) {
							global_proc_state[j] = PROC_TERMINATED;
							num_failed++;
							proc_failed[j] = 1;
							pthread_mutex_lock(&parep_mpi_num_failed_mutex);
							parep_mpi_num_failed++;
							pthread_cond_signal(&parep_mpi_barrier_count_cond);
							pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
						}
					}
				}
			}
			
			if(num_failed > 0) {
				int local_term;
				pthread_mutex_lock(&local_procs_term_mutex);
				local_term = local_procs_term;
				pthread_mutex_unlock(&local_procs_term_mutex);
				if(local_term == 0) {
					int local_terminated = 1;
					for(int i = 0; i < parep_mpi_node_size; i++) {
						if(global_proc_state[i] != PROC_TERMINATED) {
							local_terminated = 0;
						}
					}
					if(local_terminated) {
						pthread_mutex_lock(&local_procs_term_mutex);
						local_procs_term = local_terminated;
						pthread_cond_signal(&local_procs_term_cond);
						pthread_mutex_unlock(&local_procs_term_mutex);
					}
				}
			}
			pthread_cond_signal(&proc_state_cond);
			pthread_mutex_unlock(&proc_state_mutex);
			free(pfds);
			free(proc_failed);
		}
		
		for(int i = 0; i < parep_mpi_node_size; i++) {
			int ret;
			do {
				ret = kill(parep_mpi_pids[i],SIGKILL);
			} while((ret == -1) && (errno != ESRCH));
		}
		
		wait_process_end();
		pthread_mutex_lock(&empi_waitpid_safe_mutex);
		empi_waitpid_safe = 1;
		pthread_cancel(empi_thread);
		if(isMainServer) {
			pthread_cancel(empi_exec_thread);
		}
		//write_to_fd(empi_client_socket,&empi_waitpid_safe,sizeof(int));
		int ret;
		do {
			ret = kill(parep_mpi_empi_pid,SIGKILL);
		} while((ret == -1) && (errno != ESRCH));
		/*if(isMainServer && (parep_mpi_empi_exec_pid != -1)) {
			do {
				ret = kill(parep_mpi_empi_exec_pid,SIGKILL);
			} while((ret == -1) && (errno != ESRCH));
		}*/
		pthread_cond_signal(&empi_waitpid_safe_cond);
		pthread_mutex_unlock(&empi_waitpid_safe_mutex);
		perform_cleanup();
	} else {
		
		int local_term;
		pthread_mutex_lock(&local_procs_term_mutex);
		local_term = local_procs_term;
		pthread_mutex_unlock(&local_procs_term_mutex);
		if(local_term == 0) {
			struct pollfd *pfds;
			nfds_t nfds = parep_mpi_node_size;
			pfds = (struct pollfd *)malloc(nfds * sizeof(struct pollfd));
			pthread_mutex_lock(&proc_state_mutex);
			int num_failed = 0;
			int *proc_failed = (int *)malloc(sizeof(int) * parep_mpi_node_size);
			num_failed = 0;
			for(int j = 0; j < parep_mpi_node_size; j++) {
				if(local_proc_state[j] != PROC_TERMINATED) pfds[j].fd = client_socket[j];
				else pfds[j].fd = -1;
				pfds[j].events = POLLIN;
			}
			pthread_mutex_unlock(&proc_state_mutex);
			for(int j = 0; j < parep_mpi_node_size; j++) proc_failed[j] = 0;
			int pollret = 0;
			pthread_cond_signal(&proc_state_cond);
			pollret = poll(pfds,nfds,0);
			
			pthread_mutex_lock(&proc_state_mutex);
			for(int j = 0; j < parep_mpi_node_size; j++) {
				if(pfds[j].revents != 0) {
					if((pfds[j].revents & POLLHUP) || (pfds[j].revents & POLLERR)) {
						if(local_proc_state[j] != PROC_TERMINATED) {
							local_proc_state[j] = PROC_TERMINATED;
							num_failed++;
							proc_failed[j] = 1;
							pthread_mutex_lock(&parep_mpi_num_failed_mutex);
							parep_mpi_num_failed++;
							pthread_cond_signal(&parep_mpi_barrier_count_cond);
							pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
						}
					} else if(pfds[j].revents & POLLIN) {
						bytes_read = recv(pfds[j].fd,buffer,sizeof(buffer),0);
						assert(bytes_read <= 0);
						if(local_proc_state[j] != PROC_TERMINATED) {
							local_proc_state[j] = PROC_TERMINATED;
							num_failed++;
							proc_failed[j] = 1;
							pthread_mutex_lock(&parep_mpi_num_failed_mutex);
							parep_mpi_num_failed++;
							pthread_cond_signal(&parep_mpi_barrier_count_cond);
							pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
						}
					}
				}
			}
			
			if(num_failed > 0) {
				int dsock;
				check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int ret;
				do {
					ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
				} while(ret != 0);
				int cmd = CMD_PROC_STATE_UPDATE;
				write_to_fd(dsock,&cmd,sizeof(int));
				write_to_fd(dsock,&num_failed,sizeof(int));
				
				char *proc_state_data_buf = (char *)malloc(((2*sizeof(int))+sizeof(pid_t)) * num_failed);
				size_t offset = 0;
				int local_terminated = 1;
				for(int i = 0; i < parep_mpi_node_size; i++) {
					if(proc_failed[i]) {
						char *proc_state_data = proc_state_data_buf + offset;
						*((int *)proc_state_data) = parep_mpi_ranks[i];
						*((int *)(proc_state_data+sizeof(int))) = local_proc_state[i];
						*((pid_t *)(proc_state_data+(2*sizeof(int)))) = parep_mpi_pids[i];
						offset += (2*sizeof(int))+sizeof(pid_t);
					}
					if(local_proc_state[i] != PROC_TERMINATED) {
						local_terminated = 0;
					}
				}
				write_to_fd(dsock,proc_state_data_buf,(((2*sizeof(int))+sizeof(pid_t)) * num_failed));
				free(proc_state_data_buf);
				if(local_terminated) {
					pthread_mutex_lock(&local_procs_term_mutex);
					local_procs_term = local_terminated;
					pthread_cond_signal(&local_procs_term_cond);
					pthread_mutex_unlock(&local_procs_term_mutex);
				}
				close(dsock);
			}
			pthread_cond_signal(&proc_state_cond);
			pthread_mutex_unlock(&proc_state_mutex);
			free(pfds);
			free(proc_failed);
		}
		
		for(int i = 0; i < parep_mpi_node_size; i++) {
			int ret;
			do {
				ret = kill(parep_mpi_pids[i],SIGKILL);
			} while((ret == -1) && (errno != ESRCH));
		}
		
		wait_process_end();
		int cmd = CMD_INFORM_EXIT;
		*((int *)buffer) = cmd;
		*((int *)(buffer + sizeof(int))) = parep_mpi_node_id;
		int dsock;
		check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
		int ret;
		do {
			ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
		} while(ret != 0);
		write_to_fd(dsock,buffer,(2*sizeof(int)));
		close(dsock);
		pthread_mutex_lock(&empi_waitpid_safe_mutex);
		empi_waitpid_safe = 1;
		pthread_cancel(empi_thread);
		do {
			ret = kill(parep_mpi_empi_pid,SIGKILL);
		} while((ret == -1) && (errno != ESRCH));
		//write_to_fd(empi_client_socket,&empi_waitpid_safe,sizeof(int));
		pthread_cond_signal(&empi_waitpid_safe_cond);
		pthread_mutex_unlock(&empi_waitpid_safe_mutex);
		perform_cleanup();
	}
}

void handle_proc_state_update(int csocket) {
	char buffer[BUFFER_SIZE];
	size_t bytes_read;
	int msgsize = 0;
	int num_failed_procs = 0;
	int *proc_fail_update = (int *)malloc(sizeof(int) * parep_mpi_size);
	for(int i = 0; i < parep_mpi_size; i++) proc_fail_update[i] = -1;
	while((bytes_read = read(csocket,buffer+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	check(bytes_read,"recv error proc update nprocs");
	int num_procs = *((int *)buffer);
	char *proc_state_data_buf = (char *)malloc(((2*sizeof(int))+sizeof(pid_t)) * num_procs);
	
	msgsize = 0;
	while((bytes_read = read(csocket,proc_state_data_buf+msgsize, (((2*sizeof(int))+sizeof(pid_t)) * num_procs) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= (((2*sizeof(int))+sizeof(pid_t)) * num_procs)) break;
	}
	check(bytes_read,"recv error proc update proc state data");
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	size_t offset = 0;
	pthread_mutex_lock(&proc_state_mutex);
	for(int i = 0; i < num_procs; i++) {
		char *proc_state_data = proc_state_data_buf + offset;
		int rank = *((int *)proc_state_data);
		int state = *((int *)(proc_state_data+sizeof(int)));
		pid_t pid = *((pid_t *)(proc_state_data+(2*sizeof(int))));
		if(isMainServer) {
			if(global_proc_state[rank] != PROC_TERMINATED && state == PROC_TERMINATED) {
				pthread_mutex_lock(&parep_mpi_num_failed_mutex);
				parep_mpi_num_failed++;
				pthread_cond_signal(&parep_mpi_barrier_count_cond);
				pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
			}
			global_proc_state[rank] = state;
		}
		if(state == PROC_TERMINATED) {
			proc_fail_update[rank] = rank;
			num_failed_procs++;
		}
		offset += (2*sizeof(int))+sizeof(pid_t);
	}
	pthread_cond_signal(&proc_state_cond);
	pthread_mutex_unlock(&proc_state_mutex);
	
	if(num_failed_procs > 0) {
		pthread_mutex_lock(&proc_state_mutex);
		int exit_recvd;
		int local_term;
		pthread_mutex_lock(&exit_cmd_recvd_mutex);
		exit_recvd = exit_cmd_recvd;
		pthread_mutex_unlock(&exit_cmd_recvd_mutex);
		pthread_mutex_lock(&local_procs_term_mutex);
		local_term = local_procs_term;
		pthread_mutex_unlock(&local_procs_term_mutex);
		if((exit_recvd == 0) && (local_term == 0)) {
			int *failed_ranks = (int *)malloc(sizeof(int) * num_failed_procs);
			int k = 0;
			for(int i = 0; i < parep_mpi_size; i++) {
				if(proc_fail_update[i] != -1) {
					failed_ranks[k] = proc_fail_update[i];
					k++;
				}
			}
			int cmd = CMD_INFORM_PROC_FAILED;
			pthread_mutex_lock(&parep_mpi_fail_ready_mutex);
			parep_mpi_fail_ready = 1;
			pthread_mutex_unlock(&parep_mpi_fail_ready_mutex);
			pthread_mutex_unlock(&proc_state_mutex);
			if(isMainServer) {
				pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
				pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
				while(parep_mpi_barrier_running == 1) {
					pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
					pthread_cond_wait(&parep_mpi_barrier_running_cond,&parep_mpi_barrier_running_mutex);
					if(parep_mpi_barrier_running == 0) pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
				}
				pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
			}
			pthread_mutex_lock(&proc_state_mutex);
			if(isMainServer) {
				pthread_mutex_lock(&daemon_sock_mutex);
				for(int j = 1; j < parep_mpi_node_num; j++) {
					if((daemon_state[j] != PROC_TERMINATED)  && ((parep_mpi_node_group_ids[j] == 0) || (parep_mpi_node_group_nodeids[j] == parep_mpi_node_group_leader_nodeids[j]))) {
						write_to_fd(daemon_socket[j],&cmd,sizeof(int));
						write_to_fd(daemon_socket[j],&num_failed_procs,sizeof(int));
						write_to_fd(daemon_socket[j],failed_ranks, sizeof(int) * num_failed_procs);
					}
				}
				pthread_mutex_unlock(&daemon_sock_mutex);
			}
			pthread_mutex_lock(&client_sock_mutex);
			for(int j = 0; j < parep_mpi_node_size; j++) {
				if(isMainServer) {
					if(global_proc_state[j] != PROC_TERMINATED) {
						write_to_fd(client_socket[j],&cmd,sizeof(int));
						write_to_fd(client_socket[j],&num_failed_procs,sizeof(int));
						write_to_fd(client_socket[j],failed_ranks, sizeof(int) * num_failed_procs);
					}
				} else {
					if(local_proc_state[j] != PROC_TERMINATED) {
						write_to_fd(client_socket[j],&cmd,sizeof(int));
						write_to_fd(client_socket[j],&num_failed_procs,sizeof(int));
						write_to_fd(client_socket[j],failed_ranks, sizeof(int) * num_failed_procs);
					}
				}
			}
			pthread_mutex_unlock(&client_sock_mutex);
			if(isMainServer) {
				int *fprocs = (int *)failed_ranks;
				for(int j = 0; j < num_failed_procs; j++) {
					if(!parep_mpi_inform_failed_check[fprocs[j]]) {
						parep_mpi_inform_failed_check[fprocs[j]] = true;
						parep_mpi_num_inform_failed++;
						int nodeid = fprocs[j] / parep_mpi_node_size;
						int nodegroupid = parep_mpi_node_group_ids[nodeid];
						parep_mpi_num_inform_failed_node[nodeid]++;
						parep_mpi_num_inform_failed_node_group[nodegroupid]++;
					}
				}
			}
			pthread_mutex_lock(&parep_mpi_fail_ready_mutex);
			parep_mpi_fail_ready = 0;
			pthread_mutex_unlock(&parep_mpi_fail_ready_mutex);
			pthread_mutex_unlock(&proc_state_mutex);
			if(isMainServer) pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
			pthread_mutex_lock(&proc_state_mutex);
			free(failed_ranks);
		}
		pthread_mutex_unlock(&proc_state_mutex);
	}
	free(proc_fail_update);
	free(proc_state_data_buf);
}

void handle_rproc_term_ack(int csocket) {
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	pthread_mutex_lock(&remote_procs_term_ack_mutex);
	remote_procs_term_ack = 1;
	pthread_cond_signal(&remote_procs_term_ack_cond);
	pthread_mutex_unlock(&remote_procs_term_ack_mutex);
}

void handle_shrink_performed(int csocket) {
	int node_rank;
	size_t bytes_read;
	int msgsize = 0;
	while((bytes_read = read(csocket,(&node_rank)+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	int cmd = CMD_SHRINK_PERFORMED;
	pthread_mutex_lock(&client_sock_mutex);
	write_to_fd(client_socket[node_rank],&cmd,sizeof(int));
	pthread_mutex_unlock(&client_sock_mutex);
	pthread_mutex_lock(&ckpt_mutex);
	ckpt_active = 1;
	pthread_cond_signal(&ckpt_cond);
	pthread_mutex_unlock(&ckpt_mutex);
}

void handle_ckpt_created(int csocket) {
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	pthread_mutex_lock(&ckpt_mutex);
	ckpt_active = 1;
	pthread_cond_broadcast(&ckpt_cond);
	pthread_mutex_unlock(&ckpt_mutex);
	if(getenv("PAREP_MPI_MTBF") != NULL) {
		pthread_mutex_lock(&parep_mpi_fpid_created_mutex);
		if(parep_mpi_fpid_created == 0) {
			char file_name[256];
			char dfile_name[256];
			sprintf(dfile_name,"%s/parep_mpi_pids_done",getenv("PAREP_MPI_WORKDIR"));
			
			sprintf(file_name,"%s/parep_mpi_nodes",getenv("PAREP_MPI_WORKDIR"));
			FILE *file = fopen(file_name, "w+");
			for (int i = 0; i < parep_mpi_node_num; i++) {
				for(int j = 0; j < parep_mpi_node_sizes[i]; j++) {
					fprintf(file, "%s\n", coordinator_name[i]);
				}
			}
			fclose(file);
			
			sprintf(file_name,"%s/parep_mpi_pids",getenv("PAREP_MPI_WORKDIR"));
			file = fopen(file_name, "w+");
			for (int i = 0; i < parep_mpi_size; i++) {
				fprintf(file, "%d\n", parep_mpi_global_pids[i]);
			}
			fclose(file);
			file = fopen(dfile_name, "w+");
			fclose(file);
			parep_mpi_fpid_created = 1;
		}
		pthread_mutex_unlock(&parep_mpi_fpid_created_mutex);
	}
}

void handle_mpi_initialized(int csocket) {
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	pthread_mutex_lock(&parep_mpi_initialized_mutex);
	if(parep_mpi_initialized == 0) {
		parep_mpi_initialized = 1;
		int cmd = CMD_MPI_INITIALIZED;
		pthread_mutex_lock(&daemon_sock_mutex);
		for(int j = 1; j < parep_mpi_node_num; j++) {
			if((daemon_state[j] != PROC_TERMINATED)  && ((parep_mpi_node_group_ids[j] == 0) || (parep_mpi_node_group_nodeids[j] == parep_mpi_node_group_leader_nodeids[j]))) {
				write_to_fd(daemon_socket[j],&cmd,sizeof(int));
			}
		}
		pthread_mutex_unlock(&daemon_sock_mutex);
		write_to_fd(empi_client_socket,&cmd,sizeof(int));
		if(isMainServer) write_to_fd(empi_exec_client_socket,&cmd,sizeof(int));
		pthread_mutex_lock(&client_sock_mutex);
		for(int j = 0; j < parep_mpi_node_size; j++) {
			if(isMainServer) {
				if(global_proc_state[j] != PROC_TERMINATED) {
					write_to_fd(client_socket[j],&cmd,sizeof(int));
				}
			} else {
				if(local_proc_state[j] != PROC_TERMINATED) {
					write_to_fd(client_socket[j],&cmd,sizeof(int));
				}
			}
		}
		pthread_mutex_unlock(&client_sock_mutex);
	}
	pthread_mutex_unlock(&parep_mpi_initialized_mutex);
}

int perform_local_barrier(int rank, bool *recv_rank) {
	int local_rank = rank - rank_lims.start;
	int barrier_size = 0;
	if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) barrier_size = parep_mpi_node_group_nodesize;
	else barrier_size = parep_mpi_node_size;
	if(parep_mpi_barrier_count == 0) {
		int failed_proc_reached_barrier = 0;
		pthread_mutex_lock(&proc_state_mutex);
		if(local_proc_state[local_rank] != PROC_TERMINATED) {
			recv_rank[local_rank] = true;
			parep_mpi_barrier_count++;
			if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) parep_mpi_group_barrier_count++;
		}
		pthread_mutex_unlock(&proc_state_mutex);
		int nfailed;
		pthread_mutex_lock(&parep_mpi_num_failed_mutex);
		nfailed = parep_mpi_num_failed;
		pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
		if(nfailed > parep_mpi_num_inform_failed) {
			pthread_mutex_lock(&proc_state_mutex);
			for(int i = 0; i < parep_mpi_node_size; i++) {
				if(recv_rank[i] && (local_proc_state[i] == PROC_TERMINATED)) {
					recv_rank[i] = false;
					failed_proc_reached_barrier++;
					if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) global_failed_proc_reached_barrier++;
					parep_mpi_barrier_count--;
					if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) parep_mpi_group_barrier_count--;
				}
			}
			pthread_mutex_unlock(&proc_state_mutex);
		}
		while(parep_mpi_barrier_count < barrier_size - nfailed) {
			pthread_cond_wait(&parep_mpi_barrier_count_cond,&parep_mpi_barrier_count_mutex);
			pthread_mutex_lock(&parep_mpi_num_failed_mutex);
			nfailed = parep_mpi_num_failed;
			pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
			if(nfailed > parep_mpi_num_inform_failed) {
				pthread_mutex_lock(&proc_state_mutex);
				for(int i = 0; i < parep_mpi_node_size; i++) {
					if(recv_rank[i] && local_proc_state[i] == PROC_TERMINATED) {
						recv_rank[i] = false;
						failed_proc_reached_barrier++;
						if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) global_failed_proc_reached_barrier++;
						parep_mpi_barrier_count--;
						if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) parep_mpi_group_barrier_count--;
					}
				}
				pthread_mutex_unlock(&proc_state_mutex);
			}
		}
		assert(parep_mpi_barrier_count == barrier_size - nfailed);
		if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) return global_failed_proc_reached_barrier;
		return failed_proc_reached_barrier;
	} else {
		pthread_mutex_lock(&proc_state_mutex);
		if(local_proc_state[local_rank] != PROC_TERMINATED) {
			int nfailed;
			recv_rank[local_rank] = true;
			parep_mpi_barrier_count++;
			if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) parep_mpi_group_barrier_count++;
			pthread_mutex_lock(&parep_mpi_num_failed_mutex);
			nfailed = parep_mpi_num_failed;
			pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
			if(parep_mpi_barrier_count >= barrier_size - nfailed) pthread_cond_signal(&parep_mpi_barrier_count_cond);
		}
		pthread_mutex_unlock(&proc_state_mutex);
		return 0;
	}
}

void handle_remote_barrier(int csocket) {
	int bcount,fprb,nid;
	size_t bytes_read;
	int msgsize = 0;
	while((bytes_read = read(csocket,(&nid)+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	msgsize = 0;
	while((bytes_read = read(csocket,(&bcount)+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	msgsize = 0;
	while((bytes_read = read(csocket,(&fprb)+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	if(isMainServer) {
		pthread_mutex_lock(&parep_mpi_barrier_count_mutex);
		if(parep_mpi_barrier_count == 0) {
			pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
			global_failed_proc_reached_barrier = -1;
			recv_rank = (bool *)malloc(sizeof(bool)*parep_mpi_size);
			for(int i = 0; i < parep_mpi_size; i++) recv_rank[i] = false;
			parep_mpi_barrier_count += bcount;
			int lfail;
			int sid = 0;
			if(parep_mpi_node_group_nodeids[nid] == parep_mpi_node_group_leader_nodeids[nid]) {
				int gid = parep_mpi_node_group_ids[nid];
				for(int i = 0; i < gid; i++) {
					sid += parep_mpi_node_group_sizes[i];
				}
				for(int i = rank_lims_all[sid].start; i < rank_lims_all[sid].start + parep_mpi_node_group_nodesizes[gid]; i++) recv_rank[i] = true;
			} else {
				for(int i = rank_lims_all[nid].start; i < rank_lims_all[nid].start + parep_mpi_node_sizes[nid]; i++) recv_rank[i] = true;
			}
			if(parep_mpi_node_group_nodeids[nid] == parep_mpi_node_group_leader_nodeids[nid]) lfail = parep_mpi_node_group_nodesizes[parep_mpi_node_group_ids[nid]] - bcount - parep_mpi_num_inform_failed_node_group[parep_mpi_node_group_ids[nid]];
			else lfail = parep_mpi_node_sizes[nid] - bcount - parep_mpi_num_inform_failed_node[nid];
			assert(lfail >= fprb);
			if(lfail > 0) {
				if(lfail == fprb) {
					if(global_failed_proc_reached_barrier != 0) global_failed_proc_reached_barrier = 1;
				} else {
					global_failed_proc_reached_barrier = 0;
				}
			}
			int nfailed;
			pthread_mutex_lock(&parep_mpi_num_failed_mutex);
			nfailed = parep_mpi_num_failed;
			pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
			if(nfailed > parep_mpi_num_inform_failed) {
				pthread_mutex_lock(&proc_state_mutex);
				bool found = false;
				bool fail_found = false;
				for(int i = 0; i < parep_mpi_node_size; i++) {
					if(!parep_mpi_inform_failed_check[i] && recv_rank[i] && (global_proc_state[i] == PROC_TERMINATED)) {
						fail_found = true;
						recv_rank[i] = false;
						if(global_failed_proc_reached_barrier != 0) global_failed_proc_reached_barrier = 1;
						parep_mpi_barrier_count--;
					} else if(!parep_mpi_inform_failed_check[i] && (global_proc_state[i] == PROC_TERMINATED)) {
						fail_found = true;
						found = true;
					}
				}
				if(!fail_found) {
					for(int i = parep_mpi_node_size; i < parep_mpi_size; i++) {
						if(!parep_mpi_inform_failed_check[i] && (global_proc_state[i] == PROC_TERMINATED)) {
							fail_found = true;
							if(recv_rank[i]) {
								if(global_failed_proc_reached_barrier == -1) {
									global_failed_proc_reached_barrier = 1;
									recv_rank[i] = false;
									parep_mpi_barrier_count--;
								}
							}
						}
					}
				}
				//assert(fail_found);
				if(found) global_failed_proc_reached_barrier = 0;
				pthread_mutex_unlock(&proc_state_mutex);
			}
			while(parep_mpi_barrier_count < parep_mpi_size - nfailed) {
				pthread_cond_wait(&parep_mpi_barrier_count_cond,&parep_mpi_barrier_count_mutex);
				pthread_mutex_lock(&parep_mpi_num_failed_mutex);
				nfailed = parep_mpi_num_failed;
				pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
				if(nfailed > parep_mpi_num_inform_failed) {
					pthread_mutex_lock(&proc_state_mutex);
					bool found = false;
					bool fail_found = false;
					for(int i = 0; i < parep_mpi_node_size; i++) {
						if(!parep_mpi_inform_failed_check[i] && recv_rank[i] && (global_proc_state[i] == PROC_TERMINATED)) {
							fail_found = true;
							recv_rank[i] = false;
							if(global_failed_proc_reached_barrier != 0) global_failed_proc_reached_barrier = 1;
							parep_mpi_barrier_count--;
						} else if(!parep_mpi_inform_failed_check[i] && (global_proc_state[i] == PROC_TERMINATED)) {
							fail_found = true;
							found = true;
						}
					}
					if(!fail_found) {
						for(int i = parep_mpi_node_size; i < parep_mpi_size; i++) {
							if(!parep_mpi_inform_failed_check[i] && (global_proc_state[i] == PROC_TERMINATED)) {
								fail_found = true;
								if(recv_rank[i]) {
									if(global_failed_proc_reached_barrier == -1) {
										global_failed_proc_reached_barrier = 1;
										recv_rank[i] = false;
										parep_mpi_barrier_count--;
									}
								}
							}
						}
					}
					//assert(fail_found);
					if(found) global_failed_proc_reached_barrier = 0;
					pthread_mutex_unlock(&proc_state_mutex);
				}
			}
			assert(parep_mpi_barrier_count == parep_mpi_size - nfailed);
			
			if(global_failed_proc_reached_barrier == -1) global_failed_proc_reached_barrier = 0;
			int cmd = CMD_BARRIER;
			pthread_mutex_lock(&proc_state_mutex);
			int fail_ready;
			pthread_mutex_lock(&parep_mpi_fail_ready_mutex);
			fail_ready = parep_mpi_fail_ready;
			pthread_mutex_unlock(&parep_mpi_fail_ready_mutex);
			if(global_failed_proc_reached_barrier && fail_ready) fail_ready = 0;
			pthread_mutex_lock(&daemon_sock_mutex);
			for(int j = 1; j < parep_mpi_node_num; j++) {
				if((daemon_state[j] != PROC_TERMINATED)  && ((parep_mpi_node_group_ids[j] == 0) || (parep_mpi_node_group_nodeids[j] == parep_mpi_node_group_leader_nodeids[j]))) {
					write_to_fd(daemon_socket[j],&cmd,sizeof(int));
					write_to_fd(daemon_socket[j],&fail_ready,sizeof(int));
				}
			}
			pthread_mutex_unlock(&daemon_sock_mutex);
			
			pthread_mutex_lock(&client_sock_mutex);
			for(int j = 0; j < parep_mpi_node_size; j++) {
				if(global_proc_state[j] != PROC_TERMINATED) {
					write_to_fd(client_socket[j],&cmd,sizeof(int));
					write_to_fd(client_socket[j],&fail_ready,sizeof(int));
				}
			}
			pthread_mutex_unlock(&client_sock_mutex);
			pthread_mutex_unlock(&proc_state_mutex);
			parep_mpi_barrier_count = 0;
			parep_mpi_group_barrier_count = 0;
			global_failed_proc_reached_barrier = -1;
			free(recv_rank);
			pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
			parep_mpi_barrier_running = 0;
			pthread_cond_broadcast(&parep_mpi_barrier_running_cond);
			pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
			pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
		} else {
			int nfailed;
			parep_mpi_barrier_count += bcount;
			int lfail;
			int sid = 0;
			if(parep_mpi_node_group_nodeids[nid] == parep_mpi_node_group_leader_nodeids[nid]) {
				int gid = parep_mpi_node_group_ids[nid];
				for(int i = 0; i < gid; i++) {
					sid += parep_mpi_node_group_sizes[i];
				}
				for(int i = rank_lims_all[sid].start; i < rank_lims_all[sid].start + parep_mpi_node_group_nodesizes[gid]; i++) recv_rank[i] = true;
			} else {
				for(int i = rank_lims_all[nid].start; i < rank_lims_all[nid].start + parep_mpi_node_sizes[nid]; i++) recv_rank[i] = true;
			}
			if(parep_mpi_node_group_nodeids[nid] == parep_mpi_node_group_leader_nodeids[nid]) lfail = parep_mpi_node_group_nodesizes[parep_mpi_node_group_ids[nid]] - bcount - parep_mpi_num_inform_failed_node_group[parep_mpi_node_group_ids[nid]];
			else lfail = parep_mpi_node_sizes[nid] - bcount - parep_mpi_num_inform_failed_node[nid];
			assert(lfail >= fprb);
			if(lfail > 0) {
				if(lfail == fprb) {
					if(global_failed_proc_reached_barrier != 0) global_failed_proc_reached_barrier = 1;
				} else {
					global_failed_proc_reached_barrier = 0;
				}
			}
			pthread_mutex_lock(&parep_mpi_num_failed_mutex);
			nfailed = parep_mpi_num_failed;
			pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
			if(parep_mpi_barrier_count >= parep_mpi_size - nfailed) pthread_cond_signal(&parep_mpi_barrier_count_cond);
		}
		pthread_mutex_unlock(&parep_mpi_barrier_count_mutex);
	} else {
		assert(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid);
		int group_nid = nid - parep_mpi_node_id;
		int barrier_size = parep_mpi_node_group_nodesize;
		pthread_mutex_lock(&parep_mpi_barrier_count_mutex);
		if(parep_mpi_barrier_count == 0) {
			pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
			if(parep_mpi_barrier_running == 0) {
				parep_mpi_barrier_running = 1;
				int dsock;
				check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int cmd = CMD_INFORM_BARRIER_RUNNING;
				int ret;
				do {
					ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
				} while(ret != 0);
				write_to_fd(dsock,&cmd,sizeof(int));
				close(dsock);
			}
			pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
			pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
			int barcount = 0;
			int fprocrb = 0;
			recv_rank = (bool *)malloc(sizeof(bool)*parep_mpi_node_size);
			for(int i = 0; i < parep_mpi_node_size; i++) recv_rank[i] = false;
			parep_mpi_barrier_count += parep_mpi_node_sizes[group_nid];
			parep_mpi_group_barrier_count += bcount;
			global_failed_proc_reached_barrier += fprb;
			int nfailed;
			pthread_mutex_lock(&parep_mpi_num_failed_mutex);
			nfailed = parep_mpi_num_failed;
			pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
			if(nfailed > parep_mpi_num_inform_failed) {
				pthread_mutex_lock(&proc_state_mutex);
				for(int i = 0; i < parep_mpi_node_size; i++) {
					if(recv_rank[i] && (local_proc_state[i] == PROC_TERMINATED)) {
						recv_rank[i] = false;
						global_failed_proc_reached_barrier++;
						parep_mpi_barrier_count--;
						if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) parep_mpi_group_barrier_count--;
					}
				}
				pthread_mutex_unlock(&proc_state_mutex);
			}
			while(parep_mpi_barrier_count < barrier_size - nfailed) {
				pthread_cond_wait(&parep_mpi_barrier_count_cond,&parep_mpi_barrier_count_mutex);
				pthread_mutex_lock(&parep_mpi_num_failed_mutex);
				nfailed = parep_mpi_num_failed;
				pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
				if(nfailed > parep_mpi_num_inform_failed) {
					pthread_mutex_lock(&proc_state_mutex);
					for(int i = 0; i < parep_mpi_node_size; i++) {
						if(recv_rank[i] && local_proc_state[i] == PROC_TERMINATED) {
							recv_rank[i] = false;
							global_failed_proc_reached_barrier++;
							parep_mpi_barrier_count--;
							if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) parep_mpi_group_barrier_count--;
						}
					}
					pthread_mutex_unlock(&proc_state_mutex);
				}
			}
			assert(parep_mpi_barrier_count == barrier_size - nfailed);
			
			barcount = parep_mpi_group_barrier_count;
			fprocrb = global_failed_proc_reached_barrier;
			parep_mpi_barrier_count = 0;
			parep_mpi_group_barrier_count = 0;
			global_failed_proc_reached_barrier = 0;
			int dsock;
			check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
			int cmd = CMD_REMOTE_BARRIER;
			int ret;
			do {
				ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
			} while(ret != 0);
			write_to_fd(dsock,&cmd,sizeof(int));
			write_to_fd(dsock,&parep_mpi_node_id,sizeof(int));
			write_to_fd(dsock,&barcount,sizeof(int));
			write_to_fd(dsock,&fprocrb,sizeof(int));
			close(dsock);
			free(recv_rank);
			parep_mpi_performing_barrier = 1;
			pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
		} else {
			int nfailed;
			parep_mpi_barrier_count += parep_mpi_node_sizes[group_nid];
			parep_mpi_group_barrier_count += bcount;
			global_failed_proc_reached_barrier += fprb;
			pthread_mutex_lock(&parep_mpi_num_failed_mutex);
			nfailed = parep_mpi_num_failed;
			pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
			if(parep_mpi_barrier_count >= barrier_size - nfailed) pthread_cond_signal(&parep_mpi_barrier_count_cond);
		}
		pthread_mutex_unlock(&parep_mpi_barrier_count_mutex);
	}
}

void handle_barrier(int csocket) {
	int rank;
	size_t bytes_read;
	int msgsize = 0;
	while((bytes_read = read(csocket,(&rank)+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	pthread_mutex_lock(&parep_mpi_propagating_mutex);
	if(!parep_mpi_propagating_checking) {
		parep_mpi_propagating_checking = 1;
		while(parep_mpi_propagating == 1) {
			pthread_cond_wait(&parep_mpi_propagating_cond,&parep_mpi_propagating_mutex);
		}
		parep_mpi_propagating_checking = 0;
	}
	pthread_mutex_unlock(&parep_mpi_propagating_mutex);
	if(isMainServer) {
		pthread_mutex_lock(&parep_mpi_barrier_count_mutex);
		if(parep_mpi_barrier_count == 0) {
			pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
			global_failed_proc_reached_barrier = -1;
			recv_rank = (bool *)malloc(sizeof(bool)*parep_mpi_size);
			for(int i = 0; i < parep_mpi_size; i++) recv_rank[i] = false;
			pthread_mutex_lock(&proc_state_mutex);
			if(global_proc_state[rank] != PROC_TERMINATED) {
				recv_rank[rank] = true;
				parep_mpi_barrier_count++;
				pthread_cond_signal(&parep_mpi_barrier_count_cond);
			}
			pthread_mutex_unlock(&proc_state_mutex);
			int nfailed;
			pthread_mutex_lock(&parep_mpi_num_failed_mutex);
			nfailed = parep_mpi_num_failed;
			pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
			if(nfailed > parep_mpi_num_inform_failed) {
				pthread_mutex_lock(&proc_state_mutex);
				bool found = false;
				bool fail_found = false;
				for(int i = 0; i < parep_mpi_node_size; i++) {
					if(!parep_mpi_inform_failed_check[i] && recv_rank[i] && (global_proc_state[i] == PROC_TERMINATED)) {
						fail_found = true;
						recv_rank[i] = false;
						if(global_failed_proc_reached_barrier != 0) global_failed_proc_reached_barrier = 1;
						parep_mpi_barrier_count--;
					} else if(!parep_mpi_inform_failed_check[i] && (global_proc_state[i] == PROC_TERMINATED)) {
						fail_found = true;
						found = true;
					}
				}
				if(!fail_found) {
					for(int i = parep_mpi_node_size; i < parep_mpi_size; i++) {
						if(!parep_mpi_inform_failed_check[i] && (global_proc_state[i] == PROC_TERMINATED)) {
							fail_found = true;
							if(recv_rank[i]) {
								if(global_failed_proc_reached_barrier == -1) {
									global_failed_proc_reached_barrier = 1;
									recv_rank[i] = false;
									parep_mpi_barrier_count--;
								}
							}
						}
					}
				}
				//assert(fail_found);
				if(found) global_failed_proc_reached_barrier = 0;
				pthread_mutex_unlock(&proc_state_mutex);
			}
			while(parep_mpi_barrier_count < parep_mpi_size - nfailed) {
				pthread_cond_wait(&parep_mpi_barrier_count_cond,&parep_mpi_barrier_count_mutex);
				pthread_mutex_lock(&parep_mpi_num_failed_mutex);
				nfailed = parep_mpi_num_failed;
				pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
				if(nfailed > parep_mpi_num_inform_failed) {
					pthread_mutex_lock(&proc_state_mutex);
					bool found = false;
					bool fail_found = false;
					for(int i = 0; i < parep_mpi_node_size; i++) {
						if(!parep_mpi_inform_failed_check[i] && recv_rank[i] && (global_proc_state[i] == PROC_TERMINATED)) {
							fail_found = true;
							recv_rank[i] = false;
							if(global_failed_proc_reached_barrier != 0) global_failed_proc_reached_barrier = 1;
							parep_mpi_barrier_count--;
						} else if(!parep_mpi_inform_failed_check[i] && (global_proc_state[i] == PROC_TERMINATED)) {
							fail_found = true;
							found = true;
						}
					}
					if(!fail_found) {
						for(int i = parep_mpi_node_size; i < parep_mpi_size; i++) {
							if(!parep_mpi_inform_failed_check[i] && (global_proc_state[i] == PROC_TERMINATED)) {
								fail_found = true;
								if(recv_rank[i]) {
									if(global_failed_proc_reached_barrier == -1) {
										global_failed_proc_reached_barrier = 1;
										recv_rank[i] = false;
										parep_mpi_barrier_count--;
									}
								}
							}
						}
					}
					//assert(fail_found);
					if(found) global_failed_proc_reached_barrier = 0;
					pthread_mutex_unlock(&proc_state_mutex);
				}
			}
			assert(parep_mpi_barrier_count == parep_mpi_size - nfailed);
			
			if(global_failed_proc_reached_barrier == -1) global_failed_proc_reached_barrier = 0;
			int cmd = CMD_BARRIER;
			pthread_mutex_lock(&proc_state_mutex);
			int fail_ready;
			pthread_mutex_lock(&parep_mpi_fail_ready_mutex);
			fail_ready = parep_mpi_fail_ready;
			pthread_mutex_unlock(&parep_mpi_fail_ready_mutex);
			if(global_failed_proc_reached_barrier && fail_ready) fail_ready = 0;
			pthread_mutex_lock(&daemon_sock_mutex);
			for(int j = 1; j < parep_mpi_node_num; j++) {
				if((daemon_state[j] != PROC_TERMINATED)  && ((parep_mpi_node_group_ids[j] == 0) || (parep_mpi_node_group_nodeids[j] == parep_mpi_node_group_leader_nodeids[j]))) {
					write_to_fd(daemon_socket[j],&cmd,sizeof(int));
					write_to_fd(daemon_socket[j],&fail_ready,sizeof(int));
				}
			}
			pthread_mutex_unlock(&daemon_sock_mutex);
			
			pthread_mutex_lock(&client_sock_mutex);
			for(int j = 0; j < parep_mpi_node_size; j++) {
				if(global_proc_state[j] != PROC_TERMINATED) {
					write_to_fd(client_socket[j],&cmd,sizeof(int));
					write_to_fd(client_socket[j],&fail_ready,sizeof(int));
				}
			}
			pthread_mutex_unlock(&client_sock_mutex);
			pthread_mutex_unlock(&proc_state_mutex);
			parep_mpi_barrier_count = 0;
			parep_mpi_group_barrier_count = 0;
			global_failed_proc_reached_barrier = -1;
			free(recv_rank);
			pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
			parep_mpi_barrier_running = 0;
			pthread_cond_broadcast(&parep_mpi_barrier_running_cond);
			pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
			pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
		} else {
			pthread_mutex_lock(&proc_state_mutex);
			if(global_proc_state[rank] != PROC_TERMINATED) {
				int nfailed;
				recv_rank[rank] = true;
				parep_mpi_barrier_count++;
				pthread_mutex_lock(&parep_mpi_num_failed_mutex);
				nfailed = parep_mpi_num_failed;
				pthread_mutex_unlock(&parep_mpi_num_failed_mutex);
				if(parep_mpi_barrier_count >= parep_mpi_size - nfailed) pthread_cond_signal(&parep_mpi_barrier_count_cond);
			}
			pthread_mutex_unlock(&proc_state_mutex);
		}
		pthread_mutex_unlock(&parep_mpi_barrier_count_mutex);
	} else {
		pthread_mutex_lock(&parep_mpi_barrier_count_mutex);
		if(parep_mpi_barrier_count == 0) {
			if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
				pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
				if(parep_mpi_barrier_running == 0) {
					parep_mpi_barrier_running = 1;
					int dsock;
					check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
					int cmd = CMD_INFORM_BARRIER_RUNNING;
					int ret;
					do {
						ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
					} while(ret != 0);
					write_to_fd(dsock,&cmd,sizeof(int));
					close(dsock);
				}
				pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
			} else {
				int dsock;
				check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int cmd = CMD_INFORM_BARRIER_RUNNING;
				int ret;
				do {
					ret = connect(dsock,(struct sockaddr *)(&(group_main_coordinator_addr)),sizeof(group_main_coordinator_addr));
				} while(ret != 0);
				write_to_fd(dsock,&cmd,sizeof(int));
				close(dsock);
			}
			pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
			int bcount = 0;
			int fprocrb = 0;
			recv_rank = (bool *)malloc(sizeof(bool)*parep_mpi_node_size);
			global_failed_proc_reached_barrier = 0;
			for(int i = 0; i < parep_mpi_node_size; i++) recv_rank[i] = false;
			int fprb = perform_local_barrier(rank,recv_rank);
			free(recv_rank);
			if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) bcount = parep_mpi_group_barrier_count;
			else bcount = parep_mpi_barrier_count;
			fprocrb = global_failed_proc_reached_barrier;
			parep_mpi_barrier_count = 0;
			parep_mpi_group_barrier_count = 0;
			global_failed_proc_reached_barrier = 0;
			if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
				int dsock;
				check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int cmd = CMD_REMOTE_BARRIER;
				int ret;
				do {
					ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
				} while(ret != 0);
				write_to_fd(dsock,&cmd,sizeof(int));
				write_to_fd(dsock,&parep_mpi_node_id,sizeof(int));
				write_to_fd(dsock,&bcount,sizeof(int));
				write_to_fd(dsock,&fprocrb,sizeof(int));
				close(dsock);
			} else {
				int dsock;
				check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int cmd = CMD_REMOTE_BARRIER;
				int ret;
				do {
					ret = connect(dsock,(struct sockaddr *)(&(group_main_coordinator_addr)),sizeof(group_main_coordinator_addr));
				} while(ret != 0);
				write_to_fd(dsock,&cmd,sizeof(int));
				write_to_fd(dsock,&parep_mpi_node_id,sizeof(int));
				write_to_fd(dsock,&bcount,sizeof(int));
				write_to_fd(dsock,&fprb,sizeof(int));
				close(dsock);
			}
			parep_mpi_performing_barrier = 1;
			pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
		} else {
			int ret = perform_local_barrier(rank,recv_rank);
		}
		pthread_mutex_unlock(&parep_mpi_barrier_count_mutex);
	}
}

void handle_rem_recv(int csocket) {
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	if(isMainServer) {
		int fin_reached = 0;
		pthread_mutex_lock(&parep_mpi_finalize_reached_mutex);
		fin_reached = parep_mpi_finalize_reached;
		pthread_mutex_unlock(&parep_mpi_finalize_reached_mutex);
		if(!fin_reached) {
			pthread_mutex_lock(&rem_recv_running_mutex);
			if(rem_recv_recvd == 0) {
				rem_recv_recvd = 1;
				pthread_mutex_unlock(&rem_recv_running_mutex);
				pthread_mutex_lock(&ckpt_mutex);
				while(ckpt_active == 0) {
					pthread_cond_wait(&ckpt_cond,&ckpt_mutex);
				}
				pthread_mutex_unlock(&ckpt_mutex);
				pthread_mutex_lock(&rem_recv_running_mutex);
				rem_recv_running = 1;
				int cmd = CMD_REM_RECV;
				pthread_mutex_lock(&daemon_sock_mutex);
				for(int j = 1; j < parep_mpi_node_num; j++) {
					if((daemon_state[j] != PROC_TERMINATED)  && ((parep_mpi_node_group_ids[j] == 0) || (parep_mpi_node_group_nodeids[j] == parep_mpi_node_group_leader_nodeids[j]))) {
						write_to_fd(daemon_socket[j],&cmd,sizeof(int));
					}
				}
				pthread_mutex_unlock(&daemon_sock_mutex);
				
				pthread_mutex_lock(&client_sock_mutex);
				for(int j = 0; j < parep_mpi_node_size; j++) {
					if(global_proc_state[j] != PROC_TERMINATED) {
						write_to_fd(client_socket[j],&cmd,sizeof(int));
					}
				}
				pthread_mutex_unlock(&client_sock_mutex);
			}
			pthread_cond_broadcast(&rem_recv_running_cond);
			pthread_mutex_unlock(&rem_recv_running_mutex);
		}
	} else {
		if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
			int dsock;
			check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
			int cmd = CMD_REM_RECV;
			int ret;
			do {
				ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
			} while(ret != 0);
			write_to_fd(dsock,&cmd,sizeof(int));
			close(dsock);
		} else {
			int dsock;
			check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
			int cmd = CMD_REM_RECV;
			int ret;
			do {
				ret = connect(dsock,(struct sockaddr *)(&(group_main_coordinator_addr)),sizeof(group_main_coordinator_addr));
			} while(ret != 0);
			write_to_fd(dsock,&cmd,sizeof(int));
			close(dsock);
		}
	}
}

void handle_inform_barrier_running(int csocket) {
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	if(isMainServer) {
		pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
		if(parep_mpi_barrier_running == 0) {
			parep_mpi_barrier_running = 1;
		}
		pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
	} else {
		assert(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid);
		pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
		if(parep_mpi_barrier_running == 0) {
			parep_mpi_barrier_running = 1;
			int dsock;
			check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
			int cmd = CMD_INFORM_BARRIER_RUNNING;
			int ret;
			do {
				ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
			} while(ret != 0);
			write_to_fd(dsock,&cmd,sizeof(int));
			close(dsock);
		}
		pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
	}
}

void handle_rem_recv_finished(int csocket) {
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	if(isMainServer) {
		pthread_mutex_lock(&rem_recv_running_mutex);
		rem_recv_running = 0;
		rem_recv_recvd = 0;
		pthread_cond_broadcast(&rem_recv_running_cond);
		pthread_mutex_unlock(&rem_recv_running_mutex);
	} else {
		if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
			int dsock;
			check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
			int cmd = CMD_REM_RECV_FINISHED;
			int ret;
			do {
				ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
			} while(ret != 0);
			write_to_fd(dsock,&cmd,sizeof(int));
			close(dsock);
		} else {
			int dsock;
			check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
			int cmd = CMD_REM_RECV_FINISHED;
			int ret;
			do {
				ret = connect(dsock,(struct sockaddr *)(&(group_main_coordinator_addr)),sizeof(main_coordinator_addr));
			} while(ret != 0);
			write_to_fd(dsock,&cmd,sizeof(int));
			close(dsock);
		}
	}
}

void handle_get_all_rkeys(int csocket) {
	pthread_mutex_lock(&rkey_pair_mutex);
	int num_maps = 0;
	for(int hash = 0; hash < RKEY_HASH_KEYS; hash++) {
		if(rkey_map[hash] != NULL) {
			rkey_pair_t *temp = rkey_map[hash];
			while(temp != NULL) {
				num_maps++;
				temp = temp->next;
			}
		}
	}
	write_to_fd(csocket,&num_maps,sizeof(int));
	
	uint32_t *maps = (uint32_t *)malloc(sizeof(uint32_t) * 3 * num_maps);
	int idx = 0;
	for(int hash = 0; hash < RKEY_HASH_KEYS; hash++) {
		if(rkey_map[hash] != NULL) {
			rkey_pair_t *temp = rkey_map[hash];
			while(temp != NULL) {
				maps[3*idx] = temp->virt_rkey;
				maps[(3*idx)+1] = temp->pd_id;
				maps[(3*idx)+2] = temp->real_rkey;
				idx++;
				temp = temp->next;
			}
		}
	}
	write_to_fd(csocket,maps,sizeof(uint32_t) * 3 * num_maps);
	
	free(maps);
	pthread_mutex_unlock(&rkey_pair_mutex);
	
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
}

void handle_get_all_qps(int csocket) {
	pthread_mutex_lock(&qp_pair_mutex);
	int num_maps = 0;
	for(int hash = 0; hash < QP_HASH_KEYS; hash++) {
		if(qp_map[hash] != NULL) {
			qp_pair_t *temp = qp_map[hash];
			while(temp != NULL) {
				num_maps++;
				temp = temp->next;
			}
		}
	}
	write_to_fd(csocket,&num_maps,sizeof(int));
	
	uint32_t *maps = (uint32_t *)malloc(sizeof(uint32_t) * 3 * num_maps);
	int idx = 0;
	for(int hash = 0; hash < QP_HASH_KEYS; hash++) {
		if(qp_map[hash] != NULL) {
			qp_pair_t *temp = qp_map[hash];
			while(temp != NULL) {
				maps[3*idx] = temp->virt_qp_num;
				maps[(3*idx)+1] = temp->real_qp_num;
				maps[(3*idx)+2] = temp->pd_id;
				idx++;
				temp = temp->next;
			}
		}
	}
	write_to_fd(csocket,maps,sizeof(uint32_t) * 3 * num_maps);
	
	free(maps);
	pthread_mutex_unlock(&qp_pair_mutex);
	
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
}

void handle_get_all_lids(int csocket) {
	pthread_mutex_lock(&lid_pair_mutex);
	int num_maps = 0;
	for(int hash = 0; hash < LID_HASH_KEYS; hash++) {
		if(lid_map[hash] != NULL) {
			lid_pair_t *temp = lid_map[hash];
			while(temp != NULL) {
				num_maps++;
				temp = temp->next;
			}
		}
	}
	write_to_fd(csocket,&num_maps,sizeof(int));
	
	uint16_t *maps = (uint16_t *)malloc(sizeof(uint16_t) * 2 * num_maps);
	int idx = 0;
	for(int hash = 0; hash < LID_HASH_KEYS; hash++) {
		if(lid_map[hash] != NULL) {
			lid_pair_t *temp = lid_map[hash];
			while(temp != NULL) {
				maps[2*idx] = temp->virt_lid;
				maps[(2*idx)+1] = temp->real_lid;
				idx++;
				temp = temp->next;
			}
		}
	}
	write_to_fd(csocket,maps,sizeof(uint16_t) * 2 * num_maps);
	
	free(maps);
	pthread_mutex_unlock(&lid_pair_mutex);
	
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
}

void handle_inform_finalize_reached(int csocket) {
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	pthread_mutex_lock(&parep_mpi_finalize_reached_mutex);
	if(parep_mpi_finalize_reached == 0) {
		if(!isMainServer) {
			if(parep_mpi_node_group_nodeid == parep_mpi_node_group_leader_nodeid) {
				int dsock;
				check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int cmd = CMD_INFORM_FINALIZE_REACHED;
				int ret;
				do {
					ret = connect(dsock,(struct sockaddr *)(&(main_coordinator_addr)),sizeof(main_coordinator_addr));
				} while(ret != 0);
				write_to_fd(dsock,&cmd,sizeof(int));
				close(dsock);
			} else {
				int dsock;
				check((dsock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int cmd = CMD_INFORM_FINALIZE_REACHED;
				int ret;
				do {
					ret = connect(dsock,(struct sockaddr *)(&(group_main_coordinator_addr)),sizeof(main_coordinator_addr));
				} while(ret != 0);
				write_to_fd(dsock,&cmd,sizeof(int));
				close(dsock);
			}
		}
		parep_mpi_finalize_reached = 1;
	}
	pthread_mutex_unlock(&parep_mpi_finalize_reached_mutex);
}

void handle_inform_predict(int csocket) {
	pid_t fpid;
	char fnode[256];
	size_t bytes_read;
	int msgsize = 0;
	while((bytes_read = read(csocket,(&fpid)+msgsize, sizeof(pid_t) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(pid_t)) break;
	}
	msgsize = 0;
	while((bytes_read = read(csocket,(fnode)+msgsize, 256 - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= 256) break;
	}
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	
	pthread_mutex_lock(&proc_state_mutex);
	int node_id = -1;
	for(int j = 0; j < parep_mpi_node_num; j++) {
		if(!strcmp(fnode,coordinator_name[j])) {
			node_id = j;
			break;
		}
	}
	assert(node_id >= 0);
	int targrank = -1;
	for(int j = rank_lims_all[node_id].start; j <= rank_lims_all[node_id].end; j++) {
		if(parep_mpi_global_pids[j] == fpid) {
			targrank = j;
			break;
		}
	}
	assert(targrank >= 0);
	if(global_proc_state[targrank] == PROC_TERMINATED) {
		printf("%d: Failed rank %d pid %d node %s predicted\n",getpid(),targrank,fpid,fnode);
		fflush(stdout);
	}
	assert(global_proc_state[targrank] != PROC_TERMINATED);
	pthread_mutex_unlock(&proc_state_mutex);
	
	time_t timestamp = time(NULL);
	
	//Propagate probranks
	int exit_recvd;
	int local_term;
	pthread_mutex_lock(&exit_cmd_recvd_mutex);
	exit_recvd = exit_cmd_recvd;
	pthread_mutex_unlock(&exit_cmd_recvd_mutex);
	pthread_mutex_lock(&local_procs_term_mutex);
	local_term = local_procs_term;
	pthread_mutex_unlock(&local_procs_term_mutex);
	if((exit_recvd == 0) && (local_term == 0)) {
		pthread_mutex_lock(&parep_mpi_block_predict_mutex);
		while(parep_mpi_block_predict) {
			pthread_cond_wait(&parep_mpi_block_predict_cond,&parep_mpi_block_predict_mutex);
		}
		pthread_mutex_unlock(&parep_mpi_block_predict_mutex);
		pthread_mutex_lock(&ckpt_mutex);
		while(ckpt_active == 0) {
			pthread_cond_wait(&ckpt_cond,&ckpt_mutex);
		}
		pthread_mutex_unlock(&ckpt_mutex);
		int cmd = CMD_INFORM_PREDICT;
		pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
		pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
		while(parep_mpi_barrier_running == 1) {
			pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
			pthread_cond_wait(&parep_mpi_barrier_running_cond,&parep_mpi_barrier_running_mutex);
			if(parep_mpi_barrier_running == 0) pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
		}
		pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
		pthread_mutex_lock(&rem_recv_running_mutex);
		while(rem_recv_recvd || rem_recv_running) {
			pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
			pthread_cond_wait(&rem_recv_running_cond,&rem_recv_running_mutex);
			if(!(rem_recv_recvd || rem_recv_running)) pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
		}
		pthread_mutex_unlock(&rem_recv_running_mutex);
		pthread_mutex_lock(&proc_state_mutex);
		if(global_proc_state[targrank] == PROC_TERMINATED) {
			printf("%d: Predicted proc already failed rank %d pid %d node %s\n",getpid(),targrank,fpid,fnode);
			fflush(stdout);
			pthread_mutex_unlock(&proc_state_mutex);
			pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
			return;
		}
		timestamp = time(NULL);
		pthread_mutex_lock(&daemon_sock_mutex);
		for(int j = 1; j < parep_mpi_node_num; j++) {
			if((daemon_state[j] != PROC_TERMINATED)  && ((parep_mpi_node_group_ids[j] == 0) || (parep_mpi_node_group_nodeids[j] == parep_mpi_node_group_leader_nodeids[j]))) {
				write_to_fd(daemon_socket[j],&cmd,sizeof(int));
				write_to_fd(daemon_socket[j],&targrank,sizeof(int));
				write_to_fd(daemon_socket[j],&timestamp,sizeof(time_t));
			}
		}
		pthread_mutex_unlock(&daemon_sock_mutex);

		pthread_mutex_lock(&client_sock_mutex);
		for(int j = 0; j < parep_mpi_node_size; j++) {
			if(global_proc_state[j] != PROC_TERMINATED) {
				write_to_fd(client_socket[j],&cmd,sizeof(int));
				write_to_fd(client_socket[j],&targrank,sizeof(int));
				write_to_fd(client_socket[j],&timestamp,sizeof(time_t));
			}
		}
		pthread_mutex_unlock(&client_sock_mutex);
		pthread_mutex_unlock(&proc_state_mutex);
		pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
	}
}

void handle_inform_predict_node(int csocket) {
	char fnode[256];
	size_t bytes_read;
	int msgsize = 0;
	while((bytes_read = read(csocket,(fnode)+msgsize, 256 - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= 256) break;
	}
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	
	pthread_mutex_lock(&proc_state_mutex);
	int node_id = -1;
	for(int j = 0; j < parep_mpi_node_num; j++) {
		if(!strcmp(fnode,coordinator_name[j])) {
			node_id = j;
			break;
		}
	}
	if((node_id < 0) || (node_id >= parep_mpi_node_num)) {
		pthread_mutex_unlock(&proc_state_mutex);
		return;
	}
	if(daemon_state[node_id] == PROC_TERMINATED) {
		printf("%d: Predicted node already failed %s\n",getpid(),fnode);
		fflush(stdout);
		pthread_mutex_unlock(&proc_state_mutex);
		return;
	}
	assert(daemon_state[node_id] != PROC_TERMINATED);
	pthread_mutex_unlock(&proc_state_mutex);
	
	time_t timestamp = time(NULL);
	
	//Propagate probranks
	int exit_recvd;
	int local_term;
	pthread_mutex_lock(&exit_cmd_recvd_mutex);
	exit_recvd = exit_cmd_recvd;
	pthread_mutex_unlock(&exit_cmd_recvd_mutex);
	pthread_mutex_lock(&local_procs_term_mutex);
	local_term = local_procs_term;
	pthread_mutex_unlock(&local_procs_term_mutex);
	if((exit_recvd == 0) && (local_term == 0)) {
		pthread_mutex_lock(&parep_mpi_block_predict_mutex);
		while(parep_mpi_block_predict) {
			pthread_cond_wait(&parep_mpi_block_predict_cond,&parep_mpi_block_predict_mutex);
		}
		pthread_mutex_unlock(&parep_mpi_block_predict_mutex);
		pthread_mutex_lock(&ckpt_mutex);
		while(ckpt_active == 0) {
			pthread_cond_wait(&ckpt_cond,&ckpt_mutex);
		}
		pthread_mutex_unlock(&ckpt_mutex);
		int cmd = CMD_INFORM_PREDICT_NODE;
		pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
		pthread_mutex_lock(&parep_mpi_barrier_running_mutex);
		while(parep_mpi_barrier_running == 1) {
			pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
			pthread_cond_wait(&parep_mpi_barrier_running_cond,&parep_mpi_barrier_running_mutex);
			if(parep_mpi_barrier_running == 0) pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
		}
		pthread_mutex_unlock(&parep_mpi_barrier_running_mutex);
		pthread_mutex_lock(&rem_recv_running_mutex);
		while(rem_recv_recvd || rem_recv_running) {
			pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
			pthread_cond_wait(&rem_recv_running_cond,&rem_recv_running_mutex);
			if(!(rem_recv_recvd || rem_recv_running)) pthread_mutex_lock(&parep_mpi_inform_failed_mutex);
		}
		pthread_mutex_unlock(&rem_recv_running_mutex);
		pthread_mutex_lock(&proc_state_mutex);
		if(daemon_state[node_id] == PROC_TERMINATED) {
			printf("%d: Predicted node already failed %s\n",getpid(),fnode);
			fflush(stdout);
			pthread_mutex_unlock(&proc_state_mutex);
			pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
			return;
		}
		timestamp = time(NULL);
		pthread_mutex_lock(&daemon_sock_mutex);
		for(int j = 1; j < parep_mpi_node_num; j++) {
			if((daemon_state[j] != PROC_TERMINATED)  && ((parep_mpi_node_group_ids[j] == 0) || (parep_mpi_node_group_nodeids[j] == parep_mpi_node_group_leader_nodeids[j]))) {
				write_to_fd(daemon_socket[j],&cmd,sizeof(int));
				write_to_fd(daemon_socket[j],&(parep_mpi_node_sizes[node_id]),sizeof(int));
				write_to_fd(daemon_socket[j],&(parep_mpi_all_ranks[rank_lims_all[node_id].start]),parep_mpi_node_sizes[node_id]*sizeof(int));
				write_to_fd(daemon_socket[j],&timestamp,sizeof(time_t));
			}
		}
		pthread_mutex_unlock(&daemon_sock_mutex);

		pthread_mutex_lock(&client_sock_mutex);
		for(int j = 0; j < parep_mpi_node_size; j++) {
			if(global_proc_state[j] != PROC_TERMINATED) {
				write_to_fd(client_socket[j],&cmd,sizeof(int));
				write_to_fd(client_socket[j],&(parep_mpi_node_sizes[node_id]),sizeof(int));
				write_to_fd(client_socket[j],&(parep_mpi_all_ranks[rank_lims_all[node_id].start]),parep_mpi_node_sizes[node_id]*sizeof(int));
				write_to_fd(client_socket[j],&timestamp,sizeof(time_t));
			}
		}
		pthread_mutex_unlock(&client_sock_mutex);
		pthread_mutex_unlock(&proc_state_mutex);
		pthread_mutex_unlock(&parep_mpi_inform_failed_mutex);
	}
}

void handle_block_predict(int csocket) {
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	
	pthread_mutex_lock(&parep_mpi_block_predict_mutex);
	parep_mpi_block_predict = 1;
	pthread_cond_signal(&parep_mpi_block_predict_cond);
	pthread_mutex_unlock(&parep_mpi_block_predict_mutex);
}

void handle_check_node_fail(int csocket) {
	int num_nodes_failed;
	int node_group_id;
	int *failed_node_group_nodeids;
	size_t bytes_read;
	int msgsize = 0;
	while((bytes_read = read(csocket,(&num_nodes_failed)+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	if(num_nodes_failed > 0) {
		msgsize = 0;
		while((bytes_read = read(csocket,(&node_group_id)+msgsize, sizeof(int) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= sizeof(int)) break;
		}
		failed_node_group_nodeids = (int *)malloc(sizeof(int)*num_nodes_failed);
		msgsize = 0;
		while((bytes_read = read(csocket,((char *)failed_node_group_nodeids)+msgsize, (sizeof(int)*num_nodes_failed) - msgsize)) > 0) {
			msgsize += bytes_read;
			if(msgsize >= (sizeof(int)*num_nodes_failed)) break;
		}
	}
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
	
	if(num_nodes_failed > 0) {
		pthread_mutex_lock(&proc_state_mutex);
		for(int i = 0; i < num_nodes_failed; i++) {
			int node_group_offset = 0;
			for(int j = 0; j < node_group_id; j++) {
				node_group_offset += parep_mpi_node_group_sizes[j];
			}
			int failed_node_id = failed_node_group_nodeids[i] + node_group_offset;
			daemon_state[failed_node_id] = PROC_TERMINATED;
			pthread_cond_signal(&proc_state_cond);
			parep_mpi_num_nodes_failed++;
		}
		pthread_mutex_unlock(&proc_state_mutex);
	}
	pthread_mutex_lock(&parep_mpi_node_group_checked_mutex);
	parep_mpi_node_group_checked++;
	pthread_cond_signal(&parep_mpi_node_group_checked_cond);
	pthread_mutex_unlock(&parep_mpi_node_group_checked_mutex);
	if(num_nodes_failed > 0) {
		free(failed_node_group_nodeids);
	}
}

void handle_get_ngroup_leader(int csocket) {
	size_t bytes_read;
	int msgsize = 0;
	int ngid,ngnid,leader_ngid;
	int nid;
	int new_leader;
	while((bytes_read = read(csocket,(&ngid)+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	
	msgsize = 0;
	while((bytes_read = read(csocket,(&ngnid)+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	
	msgsize = 0;
	while((bytes_read = read(csocket,(&leader_ngid)+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	
	pthread_mutex_lock(&proc_state_mutex);
	
	int node_group_offset = 0;
	for(int j = 0; j < ngid; j++) {
		node_group_offset += parep_mpi_node_group_sizes[j];
	}
	nid = node_group_offset + ngnid;
	
	if(leader_ngid == parep_mpi_node_group_leader_nodeids[nid]) {
		parep_mpi_reconf_ngroup = 1;
		new_leader = leader_ngid + 1;
		while(daemon_state[node_group_offset + new_leader] == PROC_TERMINATED) {
			new_leader += 1;
		}
		close(daemon_socket[node_group_offset+parep_mpi_node_group_leader_nodeids[nid]]);
		daemon_socket[node_group_offset+parep_mpi_node_group_leader_nodeids[nid]] = -1;
		for(int j = node_group_offset; j < node_group_offset + parep_mpi_node_group_sizes[ngid]; j++) {
			parep_mpi_node_group_leader_nodeids[j] = new_leader;
		}
	} else {
		new_leader = parep_mpi_node_group_leader_nodeids[nid];
	}
	
	write_to_fd(csocket,&new_leader,sizeof(int));
	
	memcpy(&(group_coordinator_addr[ngid].sin_addr.s_addr),&(coordinator_addr[node_group_offset+new_leader].sin_addr.s_addr),sizeof(coordinator_addr[node_group_offset+new_leader].sin_addr.s_addr));
	
	char IP_addr[256];
	strcpy(IP_addr,inet_ntoa(group_coordinator_addr[ngid].sin_addr));
	write_to_fd(csocket,IP_addr,256);
	
	if(ngnid == new_leader) {
		write_to_fd(csocket, &(coordinator_addr[node_group_offset]), (sizeof(SA_IN) * parep_mpi_node_group_sizes[ngid]));
		write_to_fd(csocket, &(parep_mpi_node_sizes[node_group_offset]), (sizeof(int) * parep_mpi_node_group_sizes[ngid]));
		write_to_fd(csocket, &(daemon_state[node_group_offset]), (sizeof(SA_IN) * parep_mpi_node_group_sizes[ngid]));
		int tmpsock;
		SA_IN client_addr;
		int addr_size;
		pthread_mutex_unlock(&proc_state_mutex);
		tmpsock = accept(server_socket, (SA *)&client_addr, (socklen_t *)&addr_size);
		if((tmpsock == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
			while((tmpsock == -1) && ((errno == EMFILE) || (errno == ENFILE))) {
				rlim_nofile.rlim_cur *= 2;
				assert(rlim_nofile.rlim_cur <= rlim_nofile.rlim_max);
				setrlimit(RLIMIT_NOFILE,&rlim_nofile);
				tmpsock = accept(server_socket, (SA *)&client_addr, (socklen_t *)&addr_size);
			}
			check(tmpsock,"accept reconfig failed");
		} else check(tmpsock,"accept reconfig wrong failed");
		pthread_mutex_lock(&proc_state_mutex);
		daemon_socket[node_group_offset+new_leader] = tmpsock;
		daemon_state[node_group_offset+leader_ngid] = PROC_TERMINATED;
		if(!waiting_for_node_fail_detect) parep_mpi_num_nodes_failed += 1;
		parep_mpi_reconf_ngroup = 0;
	}
	pthread_cond_signal(&proc_state_cond);
	pthread_mutex_unlock(&proc_state_mutex);
	
	close(csocket);
	pthread_mutex_lock(&parep_mpi_num_dyn_socks_mutex);
	parep_mpi_num_dyn_socks--;
	pthread_cond_signal(&parep_mpi_num_dyn_socks_cond);
	pthread_mutex_unlock(&parep_mpi_num_dyn_socks_mutex);
}

void *handle_connection(void *p_client_socket) {
	int *pclient;
	int ecreated;
	int csocket = *((int *)p_client_socket);
	free(p_client_socket);
	
	char buffer[BUFFER_SIZE];
	size_t bytes_read;
	int msgsize = 0;
	int cmd = -1;
	while((bytes_read = read(csocket,buffer+msgsize, sizeof(int) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= sizeof(int)) break;
	}
	check(bytes_read,"recv error CMD");
	cmd = *((int *)buffer);
	
	switch(cmd) {
		case CMD_STORE:
			handle_store_cmd(csocket,true);
			break;
		case CMD_STORE_NO_PROP:
			handle_store_cmd(csocket,false);
			break;
		case CMD_STORE_MULTIPLE:
			handle_store_multiple_cmd(csocket,true);
			break;
		case CMD_STORE_MULTIPLE_NO_PROP:
			handle_store_multiple_cmd(csocket,false);
			break;
		case CMD_QUERY:
			pclient = malloc(sizeof(int));
			*pclient = csocket;
			pthread_mutex_lock(&qmutex);
			qenqueue(pclient);
			pthread_cond_signal(&qcondition_var);
			pthread_mutex_unlock(&qmutex);
			return NULL;
			break;
		case CMD_GET_PD_ID:
			handle_get_pd_id(csocket);
			break;
		case CMD_GET_RKEY:
			handle_get_rkey(csocket);
			break;
		case CMD_CLEAR_STORE:
			handle_clear_store(csocket);
			break;
		case CMD_EXIT:
			pthread_mutex_lock(&ethread_created_mutex);
			ecreated = ethread_created;
			if(ecreated == 0) {
				pclient = malloc(sizeof(int));
				*pclient = csocket;
				pthread_create(&exit_thread,NULL,ethread_function,pclient);
				ethread_created = 1;
			}
			pthread_mutex_unlock(&ethread_created_mutex);
			break;
		case CMD_INFORM_EXIT:
			handle_inform_exit_cmd(csocket);
			break;
		case CMD_PROC_STATE_UPDATE:
			handle_proc_state_update(csocket);
			break;
		case CMD_RPROC_TERM_ACK:
			handle_rproc_term_ack(csocket);
			break;
		case CMD_SHRINK_PERFORMED:
			handle_shrink_performed(csocket);
			break;
		case CMD_CKPT_CREATED:
			handle_ckpt_created(csocket);
			break;
		case CMD_MPI_INITIALIZED:
			handle_mpi_initialized(csocket);
			break;
		case CMD_BARRIER:
			handle_barrier(csocket);
			break;
		case CMD_REMOTE_BARRIER:
			handle_remote_barrier(csocket);
			break;
		case CMD_REM_RECV:
			handle_rem_recv(csocket);
			break;
		case CMD_REM_RECV_FINISHED:
			handle_rem_recv_finished(csocket);
			break;
		case CMD_GET_ALL_RKEYS:
			handle_get_all_rkeys(csocket);
			break;
		case CMD_GET_ALL_QPS:
			handle_get_all_qps(csocket);
			break;
		case CMD_GET_ALL_LIDS:
			handle_get_all_lids(csocket);
			break;
		case CMD_INFORM_BARRIER_RUNNING:
			handle_inform_barrier_running(csocket);
			break;
		case CMD_INFORM_FINALIZE_REACHED:
			handle_inform_finalize_reached(csocket);
			break;
		case CMD_INFORM_PREDICT:
			handle_inform_predict(csocket);
			break;
		case CMD_BLOCK_PREDICT:
			handle_block_predict(csocket);
			break;
		case CMD_CHECK_NODE_FAIL:
			handle_check_node_fail(csocket);
			break;
		case CMD_GET_NGROUP_LEADER:
			handle_get_ngroup_leader(csocket);
			break;
		case CMD_INFORM_PREDICT_NODE:
			handle_inform_predict_node(csocket);
			break;
	}
	
	return NULL;
}

int main(int argc, char **argv) {
	isMainServer = false;
	ckpt_interval = -1;
	for(int i=1; i < argc; i++) {
		if(strcmp(argv[i],"-ismainserver") == 0) {
			isMainServer = true;
			break;
		}
	}
	
	for(int i=1; i < argc; i++) {
		if(strcmp(argv[i],"-ckpt_interval") == 0) {
			ckpt_interval = atoi(argv[i+1]);
			break;
		}
	}
	
	if(getenv("PAREP_MPI_COMPUTE_CKPT") != NULL) {
		if(!strcmp(getenv("PAREP_MPI_COMPUTE_CKPT"),"1")) {
			ckpt_interval = atoi(getenv("PAREP_MPI_CKPT_INTERVAL"));
		}
	}
	
	getrlimit(RLIMIT_NOFILE,&rlim_nofile);
	
	char host_name[HOST_NAME_MAX+1];
	my_coordinator_name = (char *)malloc(HOST_NAME_MAX);
	char *IP_address;
	int thname = gethostname(host_name,sizeof(host_name));
	if(thname == -1) {
		perror("gethostname");
		exit(1);
	}
	strcpy(my_coordinator_name,host_name);

	struct hostent *entry = gethostbyname(host_name);
	if(entry == NULL) {
		perror("gethostbyname");
		exit(1);
	}

	IP_address = inet_ntoa(*((struct in_addr *)entry->h_addr_list[0]));
	
	setenv("PAREP_MPI_COORDINATOR_IP",IP_address,1);
	
	signal(SIGPIPE, SIG_IGN);
	
	parep_mpi_ckpt_coordinator_init();
	return 0;
}
