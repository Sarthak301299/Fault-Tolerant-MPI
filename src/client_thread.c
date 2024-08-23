#include "client_thread.h"
#include "heap_allocator.h"
#include "wrappers.h"
#include "request_handler.h"

extern int parep_mpi_coordinator_socket;

extern struct sockaddr_in parep_mpi_dyn_coordinator_addr;

extern int parep_mpi_size;
extern int parep_mpi_rank;
extern int parep_mpi_node_rank;
extern int parep_mpi_node_id;
extern int parep_mpi_node_num;
extern int parep_mpi_node_size;

extern int parep_mpi_sendid;
extern int parep_mpi_collective_id;

extern int parep_mpi_leader_rank;
extern pthread_mutex_t parep_mpi_leader_rank_mutex;

extern int (*_real_pthread_create)(pthread_t *restrict,const pthread_attr_t *restrict,void *(*)(void *),void *restrict);
extern void *(*_real_malloc)(size_t);
extern void (*_real_free)(void *);

extern pthread_rwlock_t commLock;
extern pthread_rwlock_t fgroupLock;

extern reqNode *reqHead;
extern reqNode *reqTail;

extern pthread_mutex_t reqListLock;
extern pthread_cond_t reqListCond;

recvDataNode *recvDataHead = NULL;
recvDataNode *recvDataTail = NULL;

recvDataNode *recvDataRedHead = NULL;
recvDataNode *recvDataRedTail = NULL;

pthread_mutex_t recvDataListLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t recvDataListCond = PTHREAD_COND_INITIALIZER;

extern int rem_recv_msg_sent;
extern pthread_mutex_t rem_recv_msg_sent_mutex;

extern long parep_mpi_store_buf_sz;
extern pthread_mutex_t parep_mpi_store_buf_sz_mutex;
extern pthread_cond_t parep_mpi_store_buf_sz_cond;

extern struct peertopeer_data *first_peertopeer;
extern struct peertopeer_data *last_peertopeer;
extern struct collective_data *last_collective;

extern pthread_mutex_t peertopeerLock;
extern pthread_cond_t peertopeerCond;

extern pthread_mutex_t collectiveLock;
extern pthread_cond_t collectiveCond;

extern bool parep_mpi_empi_initialized;
extern pthread_mutex_t parep_mpi_empi_initialized_mutex;
extern pthread_cond_t parep_mpi_empi_initialized_cond;

extern volatile sig_atomic_t parep_mpi_wait_block;
extern volatile sig_atomic_t parep_mpi_failed_proc_recv;

volatile sig_atomic_t parep_mpi_polling_started = 0;

void recvDataListInsert(ptpdata *pdata) {
	recvDataNode *newnode = parep_mpi_malloc(sizeof(recvDataNode));
	newnode->pdata = pdata;
	newnode->next = NULL;
	newnode->prev = NULL;
	if(recvDataTail == NULL) {
		recvDataHead = newnode;
	} else {
		newnode->prev = recvDataTail;
		recvDataTail->next = newnode;
	}
	recvDataTail = newnode;
}

void recvDataListDelete(recvDataNode *rnode) {
	if(rnode->prev != NULL) rnode->prev->next = rnode->next;
	else recvDataHead = rnode->next;
	if(rnode->next != NULL) rnode->next->prev = rnode->prev;
	else recvDataTail = rnode->prev;
	parep_mpi_free(rnode);
}

bool recvDataListIsEmpty() {
	return recvDataHead == NULL;
}

recvDataNode *recvDataListFind(int src,int tag,MPI_Comm comm) {
	recvDataNode *out = recvDataHead;
	while(out != NULL) {
		if(((out->pdata->target == src) || (src == MPI_ANY_SOURCE)) && ((out->pdata->tag == tag) || (tag == MPI_ANY_TAG)) && (out->pdata->comm == comm)) break;
		out = out->next;
	}
	return out;
}

void recvDataRedListInsert(ptpdata *pdata) {
	recvDataNode *newnode = parep_mpi_malloc(sizeof(recvDataNode));
	newnode->pdata = pdata;
	newnode->next = NULL;
	newnode->prev = NULL;
	if(recvDataRedTail == NULL) {
		recvDataRedHead = newnode;
	} else {
		newnode->prev = recvDataRedTail;
		recvDataRedTail->next = newnode;
	}
	recvDataRedTail = newnode;
}

void recvDataRedListDelete(recvDataNode *rnode) {
	if(rnode->prev != NULL) rnode->prev->next = rnode->next;
	else recvDataRedHead = rnode->next;
	if(rnode->next != NULL) rnode->next->prev = rnode->prev;
	else recvDataRedTail = rnode->prev;
	parep_mpi_free(rnode);
}

bool recvDataRedListIsEmpty() {
	return recvDataRedHead == NULL;
}

recvDataNode *recvDataRedListFind(int id,int type) {
	recvDataNode *out = recvDataRedHead;
	while(out != NULL) {
		clcdata *cdata = (clcdata *)out->pdata;
		if((cdata->id == id) && (cdata->type == type)) break;
		out = out->next;
	}
	return out;
}

EMPI_Comm parep_mpi_new_comm;

bool aborting = false;

void *comm_shrink(void * arg) {
	//Actual shink (for now just for MPI_COMM_WORLD) expand later
	pthread_mutex_lock(&thread_active_mutex);
	thread_active = 1;
	pthread_cond_signal(&thread_active_cond);
	pthread_mutex_unlock(&thread_active_mutex);
	pthread_mutex_lock(&performing_shrink_mutex);
	performing_shrink = 1;
	pthread_cond_signal(&performing_shrink_cond);
	pthread_mutex_unlock(&performing_shrink_mutex);
	EMPI_Group current_alive_group;
	memcpy(&current_alive_group,arg,sizeof(EMPI_Group));
	int ret = EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm,current_alive_group,1234,&parep_mpi_new_comm);
	
	pthread_mutex_lock(&performing_shrink_mutex);
	if(ret == EMPI_SUCCESS) performing_shrink = 0;
	pthread_cond_signal(&performing_shrink_cond);
	pthread_mutex_unlock(&performing_shrink_mutex);
	
	if(ret != EMPI_SUCCESS) {
		while(1) {
			sleep(10);
		}
	}
	
	int dyn_sock;
	dyn_sock = socket(AF_INET, SOCK_STREAM, 0);
	do {
		ret = connect(dyn_sock,(struct sockaddr *)(&parep_mpi_dyn_coordinator_addr),sizeof(parep_mpi_dyn_coordinator_addr));
	} while(ret != 0);
	char buffer[64];
	int cmd = CMD_SHRINK_PERFORMED;
	*((int *)buffer) = cmd;
	*((int *)(buffer + sizeof(int))) = parep_mpi_node_rank;
	write(dyn_sock,buffer,2*sizeof(int));
	close(dyn_sock);
	pthread_mutex_lock(&thread_active_mutex);
	thread_active = 0;
	pthread_cond_signal(&thread_active_cond);
	pthread_mutex_unlock(&thread_active_mutex);
	pthread_exit(NULL);
}

bool num_in_array(int *arr,int arr_size,int num) {
	bool out = false;
	for(int j = 0; j < arr_size; j++) {
		if(num == arr[j]) {
			out = true;
			break;
		}
	}
	return out;
}

int failed_proc_check() {
	struct pollfd pfd;
	pfd.fd = parep_mpi_coordinator_socket;
	pfd.events = POLLIN;
	nfds_t nfds = 1;
	int pollret = -1;
	
	pollret = poll(&pfd,nfds,0);
	if(pollret > 0) {
		if(pfd.revents & POLLIN) {
			int cmd;
			int msgsize = 0;
			size_t bytes_read;
			while((bytes_read = read(pfd.fd,(&cmd)+msgsize, sizeof(int)-msgsize)) > 0) {
				msgsize += bytes_read;
				if(msgsize >= (sizeof(int))) break;
			}
			if(cmd != CMD_INFORM_PROC_FAILED) printf("%d: Got cmd %d during rem recv\n",getpid(),cmd);
			assert(cmd == CMD_INFORM_PROC_FAILED);
			if(cmd == CMD_INFORM_PROC_FAILED) {
				int num_failed_procs;
				msgsize = 0;
				while((bytes_read = read(pfd.fd,(&num_failed_procs)+msgsize, sizeof(int)-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int))) break;
				}
				int *abs_failed_ranks = (int *)_real_malloc(sizeof(int) * (num_failed_procs));
				int *failed_ranks = (int *)_real_malloc(sizeof(int) * (num_failed_procs));
				msgsize = 0;
				while((bytes_read = read(pfd.fd,abs_failed_ranks+msgsize, (sizeof(int) * num_failed_procs)-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int) * num_failed_procs)) break;
				}
				int myrank;
				EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
				int *worldComm_ranks = (int *)_real_malloc(sizeof(int) * (nC+nR));
				int *rankadj = (int *)_real_malloc(sizeof(int) * (nC+nR));
				int *rankadjrep = (int *)_real_malloc(sizeof(int) * (nC+nR));
				int i;
				int num_failed_procs_current;
				
				EMPI_Group current_group;
				EMPI_Group current_failed_group;
				EMPI_Group current_alive_group;
				EMPI_Comm_group(MPI_COMM_WORLD->eworldComm,&current_group);
				
				pthread_rwlock_wrlock(&fgroupLock);
				EMPI_Group failed_group;
				if(parep_mpi_failed_group == EMPI_GROUP_EMPTY) {
					EMPI_Group_incl(parep_mpi_original_group, num_failed_procs, abs_failed_ranks, &parep_mpi_failed_group);
				} else {
					EMPI_Group_incl(parep_mpi_original_group, num_failed_procs, abs_failed_ranks, &failed_group);
					EMPI_Group_union(parep_mpi_failed_group,failed_group,&parep_mpi_failed_group);
				}
				pthread_rwlock_unlock(&fgroupLock);
				
				EMPI_Group_intersection(current_group,parep_mpi_failed_group,&current_failed_group);
				EMPI_Group_size(current_failed_group,&num_failed_procs_current);
				
				for(int j = 0; j < num_failed_procs_current; j++) abs_failed_ranks[j] = j;
				
				EMPI_Group_translate_ranks(current_failed_group,num_failed_procs_current,abs_failed_ranks,current_group,failed_ranks);
				
				pthread_mutex_unlock(&peertopeerLock);
				pthread_rwlock_unlock(&commLock);
				test_all_requests();
				pthread_rwlock_rdlock(&commLock);
				pthread_mutex_lock(&peertopeerLock);
				
				bool progressed;
					
				bool *isProcAlive = (bool *)_real_malloc(sizeof(bool)*(nC+nR));
				for(int j = 0; j < nC+nR; j++) {
					if(num_in_array(failed_ranks,num_failed_procs_current,j)) isProcAlive[j] = false;
					else isProcAlive[j] = true;
				}
				if(myrank >= nC) {
					for(int j = 0; j < nC+nR; j++) {
						if((j != myrank) && (j != repToCmpMap[myrank-nC])) {
							if(isProcAlive[j]) {
								int flag = 0;
								EMPI_Status stat;
								do {
									MPI_Comm comm = MPI_COMM_WORLD;
									if(j < nC) EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
									else EMPI_Iprobe(j-nC,EMPI_ANY_TAG,comm->EMPI_COMM_REP,&flag,&stat);
									if(flag) {
										pthread_mutex_unlock(&peertopeerLock);
										pthread_rwlock_unlock(&commLock);
										do {
											progressed = test_all_ptp_requests();
										} while(progressed);
										pthread_rwlock_rdlock(&commLock);
										pthread_mutex_lock(&peertopeerLock);
										if(j < nC) EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
										else EMPI_Iprobe(j-nC,EMPI_ANY_TAG,comm->EMPI_COMM_REP,&flag,&stat);
										if(flag == 0) {
											flag = 1;
											continue;
										}
										int count;
										int extracount;
										int size;
										ptpdata *curargs;
										curargs = (ptpdata *)parep_mpi_malloc(sizeof(ptpdata));
										curargs->markdelcmp = false;
										curargs->markdelrep = false;
										EMPI_Type_size(EMPI_BYTE,&size);
										EMPI_Get_count(&stat,EMPI_BYTE,&count);
										if(size >= sizeof(int)) extracount = 1;
										else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
										else extracount = (((int)sizeof(int))/size) + 1;
										count -= extracount;
										curargs->buf = parep_mpi_malloc((count+extracount)*size);
										
										if(j < nC) EMPI_Recv(curargs->buf,count+extracount,EMPI_BYTE,j,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
										else EMPI_Recv(curargs->buf,count+extracount,EMPI_BYTE,j-nC,stat.EMPI_TAG,comm->EMPI_COMM_REP,&stat);
										
										memcpy(&(curargs->id),(curargs->buf) + (count * size),sizeof(int));
										MPI_Status status;
										memcpy(&(status.status),&(stat),sizeof(EMPI_Status));
										status.count = count;
										if(stat.EMPI_TAG == EMPI_ANY_TAG) status.MPI_TAG = MPI_ANY_TAG;
										else status.MPI_TAG = stat.EMPI_TAG;
										status.MPI_ERROR = stat.EMPI_ERROR;
										status.MPI_SOURCE = stat.EMPI_SOURCE;
										if(j >= nC) status.MPI_SOURCE = repToCmpMap[stat.EMPI_SOURCE];
										
										curargs->type = MPI_FT_RECV;
										curargs->count = count;
										curargs->dt = MPI_BYTE;
										curargs->target = status.MPI_SOURCE;
										curargs->tag = status.MPI_TAG;
										curargs->comm = comm;
										curargs->completecmp = true;
										curargs->completerep = true;
										
										curargs->req = (MPI_Request *)parep_mpi_malloc(sizeof(MPI_Request));
										*(curargs->req) = MPI_REQUEST_NULL;
										
										if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
										else first_peertopeer = curargs;
										curargs->prev = NULL;
										curargs->next = last_peertopeer;
										last_peertopeer = curargs;
										
										pthread_mutex_lock(&recvDataListLock);
										recvDataListInsert(curargs);
										pthread_cond_signal(&reqListCond);
										pthread_mutex_unlock(&recvDataListLock);
									}
								} while(flag != 0);
							}
						} else if((j < nC) && (j == repToCmpMap[myrank-nC])) {
							if(isProcAlive[j]) {
								pthread_mutex_unlock(&collectiveLock);
								pthread_rwlock_unlock(&commLock);
								do {
									progressed = test_all_coll_requests();
								} while(progressed);
								pthread_rwlock_rdlock(&commLock);
								pthread_mutex_lock(&collectiveLock);
								int flag = 0;
								EMPI_Status stat;
								do {
									MPI_Comm comm = MPI_COMM_WORLD;
									EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
									if(flag) {
										pthread_mutex_unlock(&collectiveLock);
										pthread_rwlock_unlock(&commLock);
										do {
											progressed = test_all_coll_requests();
										} while(progressed);
										pthread_rwlock_rdlock(&commLock);
										pthread_mutex_lock(&collectiveLock);
										EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
										if(flag == 0) {
											flag = 1;
											continue;
										}
										int count;
										int extracount;
										int size;
										int tag = stat.EMPI_TAG;
										assert((tag == MPI_FT_REDUCE_TAG) || (tag == MPI_FT_ALLREDUCE_TAG));
										clcdata *curargs;
										curargs = (clcdata *)parep_mpi_malloc(sizeof(clcdata));
										EMPI_Type_size(EMPI_BYTE,&size);
										EMPI_Get_count(&stat,EMPI_BYTE,&count);
										if(size >= sizeof(int)) extracount = 1;
										else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
										else extracount = (((int)sizeof(int))/size) + 1;
										count -= extracount;
										curargs->completecmp = false;
										curargs->completerep = false;
										curargs->completecolls = NULL;
										curargs->num_colls = 0;
										curargs->req = (MPI_Request *)parep_mpi_malloc(sizeof(MPI_Request));
										*(curargs->req) = MPI_REQUEST_NULL;
										if(tag == MPI_FT_REDUCE_TAG) {
											curargs->type = MPI_FT_REDUCE_TEMP;
											(curargs->args).reduce.alloc_recvbuf = true;
											(curargs->args).reduce.recvbuf = parep_mpi_malloc((count+extracount)*size);
											EMPI_Recv((curargs->args).reduce.recvbuf,count+extracount,EMPI_BYTE,j,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
											memcpy(&(curargs->id),((curargs->args).reduce.recvbuf) + (count * size),sizeof(int));
											(curargs->args).reduce.comm = comm;
										} else if(tag == MPI_FT_ALLREDUCE_TAG) {
											curargs->type = MPI_FT_ALLREDUCE_TEMP;
											(curargs->args).allreduce.alloc_recvbuf = true;
											(curargs->args).allreduce.recvbuf = parep_mpi_malloc((count+extracount)*size);
											EMPI_Recv((curargs->args).allreduce.recvbuf,count+extracount,EMPI_BYTE,j,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
											memcpy(&(curargs->id),((curargs->args).allreduce.recvbuf) + (count * size),sizeof(int));
											(curargs->args).allreduce.comm = comm;
										}
										
										assert(curargs->id >= parep_mpi_collective_id);
										if(last_collective == NULL) {
											curargs->prev = NULL;
											curargs->next = NULL;
											last_collective = curargs;
										} else {
											clcdata *temp = last_collective;
											while((curargs->id <= temp->id) && (temp->next != NULL)) {
												temp = temp->next;
											}
											if(curargs->id > temp->id) {
												curargs->prev = temp->prev;
												curargs->next = temp;
												if(temp->prev != NULL) temp->prev->next = curargs;
												else last_collective = curargs;
												temp->prev = curargs;
											} else {
												curargs->prev = temp;
												curargs->next = NULL;
												temp->next = curargs;
											}
										}
										
										pthread_mutex_lock(&recvDataListLock);
										recvDataRedListInsert((ptpdata *)curargs);
										pthread_cond_signal(&reqListCond);
										pthread_mutex_unlock(&recvDataListLock);
									}
								} while(flag != 0);
							}
						}
					}
				} else {
					for(int j = 0; j < nC; j++) {
						if(j != myrank) {
							if(isProcAlive[j]) {
								int flag = 0;
								EMPI_Status stat;
								do {
									MPI_Comm comm = MPI_COMM_WORLD;
									EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_COMM_CMP,&flag,&stat);
									if(flag) {
										pthread_mutex_unlock(&peertopeerLock);
										pthread_rwlock_unlock(&commLock);
										do {
											progressed = test_all_ptp_requests();
										} while(progressed);
										pthread_rwlock_rdlock(&commLock);
										pthread_mutex_lock(&peertopeerLock);
										EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_COMM_CMP,&flag,&stat);
										if(flag == 0) {
											flag = 1;
											continue;
										}
										int count;
										int extracount;
										int size;
										ptpdata *curargs;
										curargs = (ptpdata *)parep_mpi_malloc(sizeof(ptpdata));
										curargs->markdelcmp = false;
										curargs->markdelrep = false;
										EMPI_Type_size(EMPI_BYTE,&size);
										EMPI_Get_count(&stat,EMPI_BYTE,&count);
										if(size >= sizeof(int)) extracount = 1;
										else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
										else extracount = (((int)sizeof(int))/size) + 1;
										count -= extracount;
										curargs->buf = parep_mpi_malloc((count+extracount)*size);
										
										EMPI_Recv(curargs->buf,count+extracount,EMPI_BYTE,j,stat.EMPI_TAG,comm->EMPI_COMM_CMP,&stat);
										
										memcpy(&(curargs->id),(curargs->buf) + (count * size),sizeof(int));
										MPI_Status status;
										memcpy(&(status.status),&(stat),sizeof(EMPI_Status));
										status.count = count;
										if(stat.EMPI_TAG == EMPI_ANY_TAG) status.MPI_TAG = MPI_ANY_TAG;
										else status.MPI_TAG = stat.EMPI_TAG;
										status.MPI_ERROR = stat.EMPI_ERROR;
										status.MPI_SOURCE = stat.EMPI_SOURCE;
										
										curargs->type = MPI_FT_RECV;
										curargs->count = count;
										curargs->dt = MPI_BYTE;
										curargs->target = status.MPI_SOURCE;
										curargs->tag = status.MPI_TAG;
										curargs->comm = comm;
										curargs->completecmp = true;
										curargs->completerep = true;
										
										curargs->req = (MPI_Request *)parep_mpi_malloc(sizeof(MPI_Request));
										*(curargs->req) = MPI_REQUEST_NULL;
										
										if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
										else first_peertopeer = curargs;
										curargs->prev = NULL;
										curargs->next = last_peertopeer;
										last_peertopeer = curargs;
										
										pthread_mutex_lock(&recvDataListLock);
										recvDataListInsert(curargs);
										pthread_cond_signal(&reqListCond);
										pthread_mutex_unlock(&recvDataListLock);
									}
								} while(flag != 0);
							}
						}
					}
				}
				_real_free(isProcAlive);
				
				for(i = 0; i < nR+nC; i++) {
					rankadjrep[i] = 0;
					rankadj[i] = 0;
					worldComm_ranks[i] = i;
				}
				int nRdiff = 0;
				int nCdiff = 0;
				bool aborting = false;
				bool switching_to_cmp = false;
				for (int j = 0; j < num_failed_procs_current;j++) {
					if(myrank >= nC) {
						if (repToCmpMap[myrank-nC] == failed_ranks[j]) switching_to_cmp = true;
					}
					if(failed_ranks[j] < nC) {
						if(cmpToRepMap[failed_ranks[j]] == -1) {
							//NO REPLICA ABORT
							printf("No Replica exists aborting after failed proc check\n");
							aborting = true;
							break;
						} else {
							nRdiff++;
							worldComm_ranks[failed_ranks[j]] = worldComm_ranks[cmpToRepMap[failed_ranks[j]]+nC];
							worldComm_ranks[cmpToRepMap[failed_ranks[j]]+nC] = -1;
							for(i = cmpToRepMap[failed_ranks[j]]+nC+1; i < nC+nR; i++) rankadjrep[i]++;
							repToCmpMap[cmpToRepMap[failed_ranks[j]]] = -1;
							cmpToRepMap[failed_ranks[j]] = -1;
							//failed_ranks[j] = worldComm_ranks[failed_ranks[j]];
						}
					} else {
						nRdiff++;
						worldComm_ranks[failed_ranks[j]] = -1;
						for(i = failed_ranks[j]+1; i < nC+nR; i++) rankadjrep[i]++;
						cmpToRepMap[repToCmpMap[failed_ranks[j]-nC]] = -1;
						repToCmpMap[failed_ranks[j]-nC] = -1;
					}
				}
				if(!aborting) {
					for(i = 0; i < nC+nR; i++) {
						if(worldComm_ranks[i] != -1) worldComm_ranks[i - rankadj[i] - rankadjrep[i]] = worldComm_ranks[i];
						if(i < nC) if(cmpToRepMap[i] != -1) cmpToRepMap[i - rankadj[i]] = cmpToRepMap[i] - rankadjrep[cmpToRepMap[i]+nC];
						if(i < nR) if(repToCmpMap[i] != -1) repToCmpMap[i - rankadjrep[i+nC]] = repToCmpMap[i] - rankadj[repToCmpMap[i]];
					}
					
					nC = nC - nCdiff;
					nR = nR - nRdiff;
				}
				
				if(!aborting) {
					EMPI_Group_incl(current_group, nR+nC, worldComm_ranks, &current_alive_group);
				}
				else {
					EMPI_Group_difference(current_group,current_failed_group,&current_alive_group);
				}
				
				_real_free(worldComm_ranks);
				_real_free(rankadj);
				_real_free(rankadjrep);
				_real_free(abs_failed_ranks);
				_real_free(failed_ranks);
				
				EMPI_Comm parep_mpi_new_comm;
				EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm,current_alive_group,1234,&parep_mpi_new_comm);
				
				memcpy(&(MPI_COMM_WORLD->eworldComm),&parep_mpi_new_comm,sizeof(EMPI_Comm));
				int newrank;
				EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&newrank);
				int *node_ids = (int *)_real_malloc(sizeof(int) * (nC+nR));
				node_ids[newrank] = parep_mpi_node_id;
				EMPI_Allgather(&parep_mpi_node_id,1,EMPI_INT,node_ids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm);
				for(int j = 0; j < nC+nR; j++) {
					if(node_ids[j] == 0) {
						pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
						parep_mpi_leader_rank = j;
						pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
						break;
					}
				}
				_real_free(node_ids);
				
				if(aborting) {
					EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
					pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
					if(newrank == parep_mpi_leader_rank) {
						parep_infiniband_cmd(PAREP_IB_KILL_COORDINATOR);
					}
					pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
					dlclose(extLib);
					
					while(1);
					
					pollret = poll(&pfd,1,-1);
					int cmd;
					int msgsize = 0;
					size_t bytes_read;
					while((bytes_read = read(pfd.fd,(&cmd)+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					close(parep_mpi_coordinator_socket);							
					assert(cmd == CMD_EXIT_CALLED);
					exit(1);
				}
				
				if (nR > 0) {
					int color = (newrank < nC) ? 0 : 1;
					int cmpLeader = 0;
					int repLeader = nC;
					
					if (color == 0) {
						EMPI_Comm_split(MPI_COMM_WORLD->eworldComm, color, 0, &(MPI_COMM_WORLD->EMPI_COMM_CMP));
						EMPI_Intercomm_create(MPI_COMM_WORLD->EMPI_COMM_CMP, 0, MPI_COMM_WORLD->eworldComm, repLeader, 100, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
						int colorinternal;
						if(cmpToRepMap[newrank] != -1) colorinternal = EMPI_UNDEFINED;
						else colorinternal = 1;
						EMPI_Comm_split (MPI_COMM_WORLD->EMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
						if(MPI_COMM_WORLD->EMPI_CMP_NO_REP != EMPI_COMM_NULL) EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->eworldComm, repLeader, 101, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
						MPI_COMM_WORLD->EMPI_COMM_REP = EMPI_COMM_NULL;
					} else if (color == 1) {
						EMPI_Comm_split (MPI_COMM_WORLD->eworldComm, color, 0, &(MPI_COMM_WORLD->EMPI_COMM_REP));
						EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, cmpLeader, 100, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
						if(nC > nR) {
							int rleader = nR;
							for(int j = 0; j < nC; j++) {
								if(cmpToRepMap[j] == -1) {
									rleader = j;
									break;
								}
							}
							EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, rleader, 101, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
						}
						MPI_COMM_WORLD->EMPI_COMM_CMP = EMPI_COMM_NULL;
					}
				} else {
					EMPI_Comm_dup(MPI_COMM_WORLD->eworldComm,&(MPI_COMM_WORLD->EMPI_COMM_CMP));
					EMPI_Comm_dup(MPI_COMM_WORLD->eworldComm,&(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
				}
				
				printf("Shrink completed from failed_proc_check newrank %d parep_mpi_rank %d new leader rank %d\n",newrank,parep_mpi_rank,parep_mpi_leader_rank);
				parep_mpi_restore_messages();
				pthread_cond_signal(&collectiveCond);
				pthread_cond_signal(&peertopeerCond);
				pthread_cond_signal(&reqListCond);
				
				return 1;
			}
		}
	}
	return 0;
}

int handle_rem_recv_no_poll() {
	int ret = 0;
	int myrank,cmprank=-1,reprank=-1;
	EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
	if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_CMP,&cmprank);
	else if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_REP,&reprank);

	int *recvnums = (int *)malloc((nC+nR)*sizeof(int));
	int *recvnumsdispls = (int *)malloc((nC+nR)*sizeof(int));
	int *recvfrommenums = (int *)malloc((nC+nR)*sizeof(int));
	int *recvfrommenumsdispls = (int *)malloc((nC+nR)*sizeof(int));
	
	int *collids = (int *)malloc((nC+nR)*sizeof(int));
	bool completed = false;
	clcdata *last_completed_collective;
	clcdata *cdata;
	int minid;
	
	int *allrecvids;
	int *allsendids;
	
	EMPI_Request ereq = EMPI_REQUEST_NULL;
	int ereqflag = 0;
	
	for(int i = 0; i<nC+nR;i++) {
		recvnums[i] = 0;
	}
	
	struct peertopeer_data *pdata = first_peertopeer;
	while(pdata != NULL) {
		if(pdata->type == MPI_FT_RECV) {
			if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
				recvnums[pdata->target]++;
				if(cmpToRepMap[pdata->target] != -1) recvnums[cmpToRepMap[pdata->target]+nC]++;
			}
		}
		pdata = pdata->prev;
	}
	
	ereq = EMPI_REQUEST_NULL;
	ereqflag = 0;
	EMPI_Ialltoall(recvnums,1,EMPI_INT,recvfrommenums,1,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
	
	do {
		if(parep_mpi_failed_proc_recv) {
			pthread_mutex_unlock(&collectiveLock);
			pthread_mutex_unlock(&peertopeerLock);
			pthread_rwlock_unlock(&commLock);
			pthread_mutex_unlock(&reqListLock);
			
			if(ereq != EMPI_REQUEST_NULL) {
				//EMPI_Cancel(&ereq);
				//EMPI_Request_free(&ereq);
			}

			while(parep_mpi_failed_proc_recv);

			pthread_mutex_lock(&reqListLock);
			pthread_rwlock_rdlock(&commLock);
			pthread_mutex_lock(&peertopeerLock);
			pthread_mutex_lock(&collectiveLock);
			ret = 1;
			goto EXIT_ALLTOALL;
		}
		EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
	} while(ereqflag == 0);
	
	allrecvids = (int *)malloc(array_sum(recvnums,nC+nR)*sizeof(int));
	allsendids = (int *)malloc(array_sum(recvfrommenums,nC+nR)*sizeof(int));	
	
	recvnumsdispls[0] = 0;
	recvfrommenumsdispls[0] = 0;
	int index = 0;
	for(int i = 0; i<nC+nR;i++) {
		pdata = first_peertopeer;
		while(pdata != NULL) {
			if((pdata->type == MPI_FT_RECV) && ((pdata->target == i) || ((cmpToRepMap[pdata->target] != -1) && ((cmpToRepMap[pdata->target]+nC) == i))) && pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
				allrecvids[index] = pdata->id;
				index++;
			}
			pdata = pdata->prev;
		}
		if(i > 0) recvnumsdispls[i] = recvnumsdispls[i-1] + recvnums[i-1];
		if(i > 0) recvfrommenumsdispls[i] = recvfrommenumsdispls[i-1] + recvfrommenums[i-1];
	}
	
	ereq = EMPI_REQUEST_NULL;
	ereqflag = 0;
	EMPI_Ialltoallv(allrecvids,recvnums,recvnumsdispls,EMPI_INT,allsendids,recvfrommenums,recvfrommenumsdispls,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
	do {
		if(parep_mpi_failed_proc_recv) {
			pthread_mutex_unlock(&collectiveLock);
			pthread_mutex_unlock(&peertopeerLock);
			pthread_rwlock_unlock(&commLock);
			pthread_mutex_unlock(&reqListLock);
			
			if(ereq != EMPI_REQUEST_NULL) {
				//EMPI_Cancel(&ereq);
				//EMPI_Request_free(&ereq);
			}

			while(parep_mpi_failed_proc_recv);

			pthread_mutex_lock(&reqListLock);
			pthread_rwlock_rdlock(&commLock);
			pthread_mutex_lock(&peertopeerLock);
			pthread_mutex_lock(&collectiveLock);
			ret = 1;
			goto EXIT_ALLTOALLV;
		}
		EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
	} while(ereqflag == 0);
	
	pdata = first_peertopeer;
	while(pdata != NULL) {
		if(pdata->type == MPI_FT_SEND) {
			if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
				int cmpindex = -1;
				int repindex = -1;
				int cmpoffset = -1;
				int repoffset = -1;
				int cmptarg = pdata->target;
				int reptarg = cmpToRepMap[pdata->target] + nC;
				bool cmprecvd = false;
				bool reprecvd = false;
				index = recvfrommenumsdispls[cmptarg];
				for(int j = 0; j<recvfrommenums[cmptarg];j++) {
					if(allsendids[index + j] == pdata->id) {
						cmpindex = index;
						cmpoffset = j;
						cmprecvd = true;
						break;
					}
				}
				if(cmpToRepMap[pdata->target] != -1) {
					index = recvfrommenumsdispls[reptarg];
					for(int j = 0; j<recvfrommenums[reptarg];j++) {
						if(allsendids[index + j] == pdata->id) {
							repindex = index;
							repoffset = j;
							reprecvd = true;
							break;
						}
					}
				} else {
					reprecvd = true;
				}
				
				if(cmprecvd && reprecvd) {
					struct peertopeer_data *deldata = pdata;
					pdata = pdata->prev;
					
					deldata->markdelcmp = true;
					deldata->markdelrep = true;
					
					allsendids[cmpindex+cmpoffset] = -1;
					if(repindex != -1) allsendids[repindex+repoffset] = -1;
					
					continue;
				}
			}
		}
		pdata = pdata->prev;
	}
	
	ereq = EMPI_REQUEST_NULL;
	ereqflag = 0;
	EMPI_Ialltoallv(allsendids,recvfrommenums,recvfrommenumsdispls,EMPI_INT,allrecvids,recvnums,recvnumsdispls,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
	do {
		if(parep_mpi_failed_proc_recv) {
			pthread_mutex_unlock(&collectiveLock);
			pthread_mutex_unlock(&peertopeerLock);
			pthread_rwlock_unlock(&commLock);
			pthread_mutex_unlock(&reqListLock);
			
			if(ereq != EMPI_REQUEST_NULL) {
				//EMPI_Cancel(&ereq);
				//EMPI_Request_free(&ereq);
			}

			while(parep_mpi_failed_proc_recv);

			pthread_mutex_lock(&reqListLock);
			pthread_rwlock_rdlock(&commLock);
			pthread_mutex_lock(&peertopeerLock);
			pthread_mutex_lock(&collectiveLock);
			ret = 1;
			goto EXIT_ALLTOALLV;
		}
		EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
	} while(ereqflag == 0);
	
	pdata = first_peertopeer;
	while(pdata != NULL) {
		if(pdata->type == MPI_FT_RECV) {
			if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
				int cmptarg = pdata->target;
				int reptarg = cmpToRepMap[pdata->target] + nC;
				bool marknodelcmp = false;
				bool marknodelrep = false;
				index = recvnumsdispls[cmptarg];
				for(int j = 0; j<recvnums[cmptarg];j++) {
					if(allrecvids[index + j] == pdata->id) {
						marknodelcmp = true;
						break;
					}
				}
				if(cmpToRepMap[pdata->target] != -1) {
					index = recvnumsdispls[reptarg];
					for(int j = 0; j<recvnums[reptarg];j++) {
						if(allrecvids[index + j] == pdata->id) {
							marknodelrep = true;
							break;
						}
					}
				}
				pdata->markdelcmp = pdata->markdelcmp || (!marknodelcmp);
				pdata->markdelrep = pdata->markdelrep || (!marknodelrep);
			}
		}
		pdata = pdata->prev;
	}
	
	ereq = EMPI_REQUEST_NULL;
	ereqflag = 0;
	last_completed_collective = last_collective;
	if(last_completed_collective == NULL) {
		int temp = -1;
		EMPI_Iallgather(&temp,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
	} else {
		while(last_completed_collective != NULL) {
			completed = last_completed_collective->completecmp & last_completed_collective->completerep & (last_completed_collective->req == NULL);
			for (int i = 0; i < last_completed_collective->num_colls; i++) {
				completed = completed & last_completed_collective->completecolls[i];
			}
			if(completed) break;
			last_completed_collective = last_completed_collective->next;
		}
		if(last_completed_collective == NULL) {
			int temp = -1;
			EMPI_Iallgather(&temp,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
		} else {
			EMPI_Iallgather(&last_completed_collective->id,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
		}
	}
	do {
		if(parep_mpi_failed_proc_recv) {
			pthread_mutex_unlock(&collectiveLock);
			pthread_mutex_unlock(&peertopeerLock);
			pthread_rwlock_unlock(&commLock);
			pthread_mutex_unlock(&reqListLock);
			
			if(ereq != EMPI_REQUEST_NULL) {
				//EMPI_Cancel(&ereq);
				//EMPI_Request_free(&ereq);
			}

			while(parep_mpi_failed_proc_recv);

			pthread_mutex_lock(&reqListLock);
			pthread_rwlock_rdlock(&commLock);
			pthread_mutex_lock(&peertopeerLock);
			pthread_mutex_lock(&collectiveLock);
			ret = 1;
			goto EXIT_ALLTOALLV;
		}
		EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
	} while(ereqflag == 0);
	
	minid = collids[0];
	for(int i = 0; i < nC+nR; i++) {
		if(collids[i] < minid) {
			minid = collids[i];
		}
	}
	
	cdata = last_collective;
	while(cdata != NULL) {
		if(cdata->id <= minid) cdata->id = -1;
		cdata = cdata->next;
	}

EXIT_ALLTOALLV:
	free(allrecvids);
	free(allsendids);
EXIT_ALLTOALL:
	free(recvnums);
	free(recvnumsdispls);
	free(recvfrommenums);
	free(recvfrommenumsdispls);
	free(collids);
	pthread_cond_signal(&parep_mpi_store_buf_sz_cond);
	
	return ret;
}

int handle_rem_recv_with_tests() {
	int ret = 0;
	int myrank,cmprank=-1,reprank=-1;
	EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
	if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_CMP,&cmprank);
	else if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_REP,&reprank);

	int *recvnums = (int *)malloc((nC+nR)*sizeof(int));
	int *recvnumsdispls = (int *)malloc((nC+nR)*sizeof(int));
	int *recvfrommenums = (int *)malloc((nC+nR)*sizeof(int));
	int *recvfrommenumsdispls = (int *)malloc((nC+nR)*sizeof(int));
	
	int *collids = (int *)malloc((nC+nR)*sizeof(int));
	bool completed = false;
	clcdata *last_completed_collective;
	clcdata *cdata;
	int minid;
	
	int *allrecvids;
	int *allsendids;
	
	test_all_requests_no_lock();
	
	EMPI_Request ereq = EMPI_REQUEST_NULL;
	int ereqflag = 0;
	
	for(int i = 0; i<nC+nR;i++) {
		recvnums[i] = 0;
	}
	
	struct peertopeer_data *pdata = first_peertopeer;
	while(pdata != NULL) {
		if(pdata->type == MPI_FT_RECV) {
			if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
				recvnums[pdata->target]++;
				if(cmpToRepMap[pdata->target] != -1) recvnums[cmpToRepMap[pdata->target]+nC]++;
			}
		}
		pdata = pdata->prev;
	}
	
	ereq = EMPI_REQUEST_NULL;
	ereqflag = 0;
	EMPI_Ialltoall(recvnums,1,EMPI_INT,recvfrommenums,1,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
	do {
		ret = failed_proc_check();
		if(ret != 0) {
			goto EXIT_ALLTOALL;
		}
		test_all_requests_no_lock();
		EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
	} while(ereqflag == 0);
	
  //Identify items from sendnums that were not in recvfrommenums and resend them
	//Identify items from recvnums that were not in senttomenums and mark them to be skipped
	allrecvids = (int *)malloc(array_sum(recvnums,nC+nR)*sizeof(int));
	allsendids = (int *)malloc(array_sum(recvfrommenums,nC+nR)*sizeof(int));	
	
	recvnumsdispls[0] = 0;
	recvfrommenumsdispls[0] = 0;
	int index = 0;
	for(int i = 0; i<nC+nR;i++) {
		pdata = first_peertopeer;
		while(pdata != NULL) {
			if((pdata->type == MPI_FT_RECV) && ((pdata->target == i) || ((cmpToRepMap[pdata->target] != -1) && ((cmpToRepMap[pdata->target]+nC) == i))) && pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
				allrecvids[index] = pdata->id;
				index++;
			}
			pdata = pdata->prev;
		}
		if(i > 0) recvnumsdispls[i] = recvnumsdispls[i-1] + recvnums[i-1];
		if(i > 0) recvfrommenumsdispls[i] = recvfrommenumsdispls[i-1] + recvfrommenums[i-1];
	}
	
	ereq = EMPI_REQUEST_NULL;
	ereqflag = 0;
	EMPI_Ialltoallv(allrecvids,recvnums,recvnumsdispls,EMPI_INT,allsendids,recvfrommenums,recvfrommenumsdispls,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
	do {
		ret = failed_proc_check();
		if(ret != 0) {
			goto EXIT_ALLTOALLV;
		}
		test_all_requests_no_lock();
		EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
	} while(ereqflag == 0);
	
	pdata = first_peertopeer;
	while(pdata != NULL) {
		if(pdata->type == MPI_FT_SEND) {
			if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
				int cmpindex = -1;
				int repindex = -1;
				int cmpoffset = -1;
				int repoffset = -1;
				int cmptarg = pdata->target;
				int reptarg = cmpToRepMap[pdata->target] + nC;
				bool cmprecvd = false;
				bool reprecvd = false;
				index = recvfrommenumsdispls[cmptarg];
				for(int j = 0; j<recvfrommenums[cmptarg];j++) {
					if(allsendids[index + j] == pdata->id) {
						cmpindex = index;
						cmpoffset = j;
						cmprecvd = true;
						break;
					}
				}
				if(cmpToRepMap[pdata->target] != -1) {
					index = recvfrommenumsdispls[reptarg];
					for(int j = 0; j<recvfrommenums[reptarg];j++) {
						if(allsendids[index + j] == pdata->id) {
							repindex = index;
							repoffset = j;
							reprecvd = true;
							break;
						}
					}
				} else {
					reprecvd = true;
				}
				
				if(cmprecvd && reprecvd) {
					struct peertopeer_data *deldata = pdata;
					pdata = pdata->prev;
					
					deldata->markdelcmp = true;
					deldata->markdelrep = true;
					
					allsendids[cmpindex+cmpoffset] = -1;
					if(repindex != -1) allsendids[repindex+repoffset] = -1;
					
					continue;
				}
			}
		}
		pdata = pdata->prev;
	}
	
	ereq = EMPI_REQUEST_NULL;
	ereqflag = 0;
	EMPI_Ialltoallv(allsendids,recvfrommenums,recvfrommenumsdispls,EMPI_INT,allrecvids,recvnums,recvnumsdispls,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
	do {
		ret = failed_proc_check();
		if(ret != 0) {
			goto EXIT_ALLTOALLV;
		}
		test_all_requests_no_lock();
		EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
	} while(ereqflag == 0);
	
	pdata = first_peertopeer;
	while(pdata != NULL) {
		if(pdata->type == MPI_FT_RECV) {
			if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
				int cmptarg = pdata->target;
				int reptarg = cmpToRepMap[pdata->target] + nC;
				bool marknodelcmp = false;
				bool marknodelrep = false;
				index = recvnumsdispls[cmptarg];
				for(int j = 0; j<recvnums[cmptarg];j++) {
					if(allrecvids[index + j] == pdata->id) {
						marknodelcmp = true;
						break;
					}
				}
				if(cmpToRepMap[pdata->target] != -1) {
					index = recvnumsdispls[reptarg];
					for(int j = 0; j<recvnums[reptarg];j++) {
						if(allrecvids[index + j] == pdata->id) {
							marknodelrep = true;
							break;
						}
					}
				}
				pdata->markdelcmp = pdata->markdelcmp || (!marknodelcmp);
				pdata->markdelrep = pdata->markdelrep || (!marknodelrep);
			}
		}
		pdata = pdata->prev;
	}
	
	ereq = EMPI_REQUEST_NULL;
	ereqflag = 0;
	last_completed_collective = last_collective;
	if(last_completed_collective == NULL) {
		int temp = -1;
		EMPI_Iallgather(&temp,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
	} else {
		while(last_completed_collective != NULL) {
			completed = last_completed_collective->completecmp & last_completed_collective->completerep & (last_completed_collective->req == NULL);
			for (int i = 0; i < last_completed_collective->num_colls; i++) {
				completed = completed & last_completed_collective->completecolls[i];
			}
			if(completed) break;
			last_completed_collective = last_completed_collective->next;
		}
		if(last_completed_collective == NULL) {
			int temp = -1;
			EMPI_Iallgather(&temp,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
		} else {
			EMPI_Iallgather(&last_completed_collective->id,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
		}
	}
	do {
		ret = failed_proc_check();
		if(ret != 0) {
			goto EXIT_ALLTOALLV;
		}
		test_all_requests_no_lock();
		EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
	} while(ereqflag == 0);
	
	minid = collids[0];
	for(int i = 0; i < nC+nR; i++) {
		if(collids[i] < minid) {
			minid = collids[i];
		}
	}
	
	cdata = last_collective;
	while(cdata != NULL) {
		if(cdata->id <= minid) cdata->id = -1;
		cdata = cdata->next;
	}

EXIT_ALLTOALLV:
	free(allrecvids);
	free(allsendids);
EXIT_ALLTOALL:
	free(recvnums);
	free(recvnumsdispls);
	free(recvfrommenums);
	free(recvfrommenumsdispls);
	free(collids);
	pthread_cond_signal(&parep_mpi_store_buf_sz_cond);
	
	return ret;
}

int handle_rem_recv() {
	int ret = 0;
	int myrank,cmprank=-1,reprank=-1;
	EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
	if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_CMP,&cmprank);
	else if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_REP,&reprank);

	int *recvnums = (int *)malloc((nC+nR)*sizeof(int));
	int *recvnumsdispls = (int *)malloc((nC+nR)*sizeof(int));
	int *recvfrommenums = (int *)malloc((nC+nR)*sizeof(int));
	int *recvfrommenumsdispls = (int *)malloc((nC+nR)*sizeof(int));
	
	int *collids = (int *)malloc((nC+nR)*sizeof(int));
	bool completed = false;
	clcdata *last_completed_collective;
	clcdata *cdata;
	int minid;
	
	int *allrecvids;
	int *allsendids;
	
	EMPI_Request ereq = EMPI_REQUEST_NULL;
	int ereqflag = 0;
	
	for(int i = 0; i<nC+nR;i++) {
		recvnums[i] = 0;
	}
	
	struct peertopeer_data *pdata = first_peertopeer;
	while(pdata != NULL) {
		if(pdata->type == MPI_FT_RECV) {
			if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
				recvnums[pdata->target]++;
				if(cmpToRepMap[pdata->target] != -1) recvnums[cmpToRepMap[pdata->target]+nC]++;
			}
		}
		pdata = pdata->prev;
	}
	
	ereq = EMPI_REQUEST_NULL;
	ereqflag = 0;
	EMPI_Ialltoall(recvnums,1,EMPI_INT,recvfrommenums,1,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
	do {
		ret = failed_proc_check();
		if(ret != 0) {
			goto EXIT_ALLTOALL;
		}
		EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
	} while(ereqflag == 0);
	
  //Identify items from sendnums that were not in recvfrommenums and resend them
	//Identify items from recvnums that were not in senttomenums and mark them to be skipped
	allrecvids = (int *)malloc(array_sum(recvnums,nC+nR)*sizeof(int));
	allsendids = (int *)malloc(array_sum(recvfrommenums,nC+nR)*sizeof(int));	
	
	recvnumsdispls[0] = 0;
	recvfrommenumsdispls[0] = 0;
	int index = 0;
	for(int i = 0; i<nC+nR;i++) {
		pdata = first_peertopeer;
		while(pdata != NULL) {
			if((pdata->type == MPI_FT_RECV) && ((pdata->target == i) || ((cmpToRepMap[pdata->target] != -1) && ((cmpToRepMap[pdata->target]+nC) == i))) && pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
				allrecvids[index] = pdata->id;
				index++;
			}
			pdata = pdata->prev;
		}
		if(i > 0) recvnumsdispls[i] = recvnumsdispls[i-1] + recvnums[i-1];
		if(i > 0) recvfrommenumsdispls[i] = recvfrommenumsdispls[i-1] + recvfrommenums[i-1];
	}
	
	ereq = EMPI_REQUEST_NULL;
	ereqflag = 0;
	EMPI_Ialltoallv(allrecvids,recvnums,recvnumsdispls,EMPI_INT,allsendids,recvfrommenums,recvfrommenumsdispls,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
	do {
		ret = failed_proc_check();
		if(ret != 0) {
			goto EXIT_ALLTOALLV;
		}
		EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
	} while(ereqflag == 0);
	
	pdata = first_peertopeer;
	while(pdata != NULL) {
		if(pdata->type == MPI_FT_SEND) {
			if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
				int cmpindex = -1;
				int repindex = -1;
				int cmpoffset = -1;
				int repoffset = -1;
				int cmptarg = pdata->target;
				int reptarg = cmpToRepMap[pdata->target] + nC;
				bool cmprecvd = false;
				bool reprecvd = false;
				index = recvfrommenumsdispls[cmptarg];
				for(int j = 0; j<recvfrommenums[cmptarg];j++) {
					if(allsendids[index + j] == pdata->id) {
						cmpindex = index;
						cmpoffset = j;
						cmprecvd = true;
						break;
					}
				}
				if(cmpToRepMap[pdata->target] != -1) {
					index = recvfrommenumsdispls[reptarg];
					for(int j = 0; j<recvfrommenums[reptarg];j++) {
						if(allsendids[index + j] == pdata->id) {
							repindex = index;
							repoffset = j;
							reprecvd = true;
							break;
						}
					}
				} else {
					reprecvd = true;
				}
				
				if(cmprecvd && reprecvd) {
					struct peertopeer_data *deldata = pdata;
					pdata = pdata->prev;
					
					deldata->markdelcmp = true;
					deldata->markdelrep = true;
					
					allsendids[cmpindex+cmpoffset] = -1;
					if(repindex != -1) allsendids[repindex+repoffset] = -1;
					
					continue;
				}
			}
		}
		pdata = pdata->prev;
	}
	
	ereq = EMPI_REQUEST_NULL;
	ereqflag = 0;
	EMPI_Ialltoallv(allsendids,recvfrommenums,recvfrommenumsdispls,EMPI_INT,allrecvids,recvnums,recvnumsdispls,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
	do {
		ret = failed_proc_check();
		if(ret != 0) {
			goto EXIT_ALLTOALLV;
		}
		EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
	} while(ereqflag == 0);
	
	pdata = first_peertopeer;
	while(pdata != NULL) {
		if(pdata->type == MPI_FT_RECV) {
			if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
				int cmptarg = pdata->target;
				int reptarg = cmpToRepMap[pdata->target] + nC;
				bool marknodelcmp = false;
				bool marknodelrep = false;
				index = recvnumsdispls[cmptarg];
				for(int j = 0; j<recvnums[cmptarg];j++) {
					if(allrecvids[index + j] == pdata->id) {
						marknodelcmp = true;
						break;
					}
				}
				if(cmpToRepMap[pdata->target] != -1) {
					index = recvnumsdispls[reptarg];
					for(int j = 0; j<recvnums[reptarg];j++) {
						if(allrecvids[index + j] == pdata->id) {
							marknodelrep = true;
							break;
						}
					}
				}
				pdata->markdelcmp = pdata->markdelcmp || (!marknodelcmp);
				pdata->markdelrep = pdata->markdelrep || (!marknodelrep);
			}
		}
		pdata = pdata->prev;
	}
	
	ereq = EMPI_REQUEST_NULL;
	ereqflag = 0;
	last_completed_collective = last_collective;
	if(last_completed_collective == NULL) {
		int temp = -1;
		EMPI_Iallgather(&temp,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
	} else {
		while(last_completed_collective != NULL) {
			completed = last_completed_collective->completecmp & last_completed_collective->completerep & (last_completed_collective->req == NULL);
			for (int i = 0; i < last_completed_collective->num_colls; i++) {
				completed = completed & last_completed_collective->completecolls[i];
			}
			if(completed) break;
			last_completed_collective = last_completed_collective->next;
		}
		if(last_completed_collective == NULL) {
			int temp = -1;
			EMPI_Iallgather(&temp,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
		} else {
			EMPI_Iallgather(&last_completed_collective->id,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm,&ereq);
		}
	}
	do {
		ret = failed_proc_check();
		if(ret != 0) {
			goto EXIT_ALLTOALLV;
		}
		EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
	} while(ereqflag == 0);
	
	minid = collids[0];
	for(int i = 0; i < nC+nR; i++) {
		if(collids[i] < minid) {
			minid = collids[i];
		}
	}
	
	cdata = last_collective;
	while(cdata != NULL) {
		if(cdata->id <= minid) cdata->id = -1;
		cdata = cdata->next;
	}

EXIT_ALLTOALLV:
	free(allrecvids);
	free(allsendids);
EXIT_ALLTOALL:
	free(recvnums);
	free(recvnumsdispls);
	free(recvfrommenums);
	free(recvfrommenumsdispls);
	free(collids);
	pthread_cond_signal(&parep_mpi_store_buf_sz_cond);
	
	return ret;
}

void *polling_daemon(void *arg) {
	if(_real_pthread_create == NULL) _real_pthread_create = dlsym(RTLD_NEXT,"pthread_create");
	if(_real_malloc == NULL) _real_malloc = dlsym(RTLD_NEXT,"malloc");
	if(_real_free == NULL) _real_free = dlsym(RTLD_NEXT,"free");
	nfds_t nfds;
	struct pollfd pfd;
	pfd.fd = parep_mpi_coordinator_socket;
	pfd.events = POLLIN;
	nfds = 1;
	int pollret;
	parep_mpi_polling_started = 1;
	while(true) {
		pollret = poll(&pfd,nfds,-1);
		if((pollret == -1) && (errno == EINTR)) continue;
		if (pfd.revents != 0) {
			if(pfd.revents & POLLIN) {
				int cmd;
				int msgsize = 0;
				size_t bytes_read;
				while((bytes_read = read(pfd.fd,(&cmd)+msgsize, sizeof(int)-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int))) break;
				}
				
				if(cmd == CMD_REM_RECV) {
					PAREP_MPI_DISABLE_CKPT();
					parep_mpi_wait_block = 1;
					pthread_mutex_lock(&reqListLock);
					
					probe_reduce_messages();
					
					pthread_rwlock_rdlock(&commLock);
					pthread_mutex_lock(&peertopeerLock);
					pthread_mutex_lock(&collectiveLock);
					//parep_mpi_wait_block = 0;
					
					int ret = 0;
					do {
						ret = handle_rem_recv_with_tests();
						if(ret != 0) continue;
						EMPI_Request ereq = EMPI_REQUEST_NULL;
						int ereqflag = 0;
						
						EMPI_Ibarrier(MPI_COMM_WORLD->eworldComm,&ereq);
						do {
							ret = failed_proc_check();
							if(ret != 0) {
								if(ereq != EMPI_REQUEST_NULL) {
									//EMPI_Cancel(&ereq);
									//EMPI_Request_free(&ereq);
								}
								break;
							}
							EMPI_Test(&ereq,&ereqflag,EMPI_STATUS_IGNORE);
						} while(ereqflag == 0);
					} while(ret != 0);
					
					ptpdata *pdata = first_peertopeer;
					while(pdata != NULL) {
						if(pdata->type == MPI_FT_RECV) {
							if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
								if(pdata->markdelcmp && pdata->markdelrep) {
									struct peertopeer_data *deldata = pdata;
									pdata = pdata->prev;
									
									if(deldata == first_peertopeer) {
										first_peertopeer = first_peertopeer->prev;
									}
									if(deldata == last_peertopeer) {
										last_peertopeer = last_peertopeer->next;
									}
									
									if(deldata->next != NULL) deldata->next->prev = deldata->prev;
									if(deldata->prev != NULL) deldata->prev->next = deldata->next;
									
									parep_mpi_free(deldata);
									
									continue;
								}
							}
						} else if(pdata->type == MPI_FT_SEND) {
							if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
								if(pdata->markdelcmp && pdata->markdelrep) {
									struct peertopeer_data *deldata = pdata;
									pdata = pdata->prev;
									
									if(deldata == first_peertopeer) {
										first_peertopeer = first_peertopeer->prev;
									}
									if(deldata == last_peertopeer) {
										last_peertopeer = last_peertopeer->next;
									}
									
									if(deldata->next != NULL) deldata->next->prev = deldata->prev;
									if(deldata->prev != NULL) deldata->prev->next = deldata->next;
									
									int size;
									int count, extracount;
									EMPI_Type_size(deldata->dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									count = deldata->count;
									
									long delsize = (count+extracount)*size;
									
									if(deldata->buf != NULL) {
										parep_mpi_free(deldata->buf);
										deldata->buf = NULL;
										pthread_mutex_lock(&parep_mpi_store_buf_sz_mutex);
										parep_mpi_store_buf_sz = parep_mpi_store_buf_sz - delsize;
										pthread_cond_signal(&parep_mpi_store_buf_sz_cond);
										pthread_mutex_unlock(&parep_mpi_store_buf_sz_mutex);
									}
									
									parep_mpi_free(deldata);
									
									continue;
								}
							}
						}
						pdata = pdata->prev;
					}
					
					clcdata *cdata = last_collective;
					while(cdata != NULL) {
						if(cdata->id == -1) {
							struct collective_data *deldata = cdata;
							cdata = cdata->next;
							
							if(deldata == last_collective) {
								last_collective = last_collective->next;
							}
							
							if(deldata->next != NULL) deldata->next->prev = deldata->prev;
							if(deldata->prev != NULL) deldata->prev->next = deldata->next;
							
							if(deldata->type == MPI_FT_BCAST) {
								if((deldata->args).bcast.buf != NULL) parep_mpi_free((deldata->args).bcast.buf);
							} else if(deldata->type == MPI_FT_SCATTER) {
								if((deldata->args).scatter.sendbuf != NULL) parep_mpi_free((deldata->args).scatter.sendbuf);
								//parep_mpi_free((deldata->args).scatter.recvbuf);
							} else if(deldata->type == MPI_FT_GATHER) {
								parep_mpi_free((deldata->args).gather.sendbuf);
								//parep_mpi_free((deldata->args).gather.recvbuf);
							} else if(deldata->type == MPI_FT_REDUCE) {
								parep_mpi_free((deldata->args).reduce.sendbuf);
								if((deldata->args).reduce.alloc_recvbuf) parep_mpi_free((deldata->args).reduce.recvbuf);
							} else if(deldata->type == MPI_FT_ALLGATHER) {
								parep_mpi_free((deldata->args).allgather.sendbuf);
								//parep_mpi_free((deldata->args).allgather.recvbuf);
							} else if(deldata->type == MPI_FT_ALLTOALL) {
								parep_mpi_free((deldata->args).alltoall.sendbuf);
								//parep_mpi_free((deldata->args).alltoall.recvbuf);
							} else if(deldata->type == MPI_FT_ALLTOALLV) {
								parep_mpi_free((deldata->args).alltoallv.sendbuf);
								//parep_mpi_free((deldata->args).alltoallv.recvbuf);
								parep_mpi_free((deldata->args).alltoallv.sendcounts);
								parep_mpi_free((deldata->args).alltoallv.recvcounts);
								parep_mpi_free((deldata->args).alltoallv.sdispls);
								parep_mpi_free((deldata->args).alltoallv.rdispls);
							} else if(deldata->type == MPI_FT_ALLREDUCE) {
								int size;
								EMPI_Type_size((deldata->args).allreduce.dt->edatatype,&size);
								long delsize = 2 * (deldata->args).allreduce.count * size;
								parep_mpi_free((deldata->args).allreduce.sendbuf);
								if((deldata->args).allreduce.alloc_recvbuf) parep_mpi_free((deldata->args).allreduce.recvbuf);
								pthread_mutex_lock(&parep_mpi_store_buf_sz_mutex);
								parep_mpi_store_buf_sz = parep_mpi_store_buf_sz - delsize;
								pthread_cond_signal(&parep_mpi_store_buf_sz_cond);
								pthread_mutex_unlock(&parep_mpi_store_buf_sz_mutex);
							}
							
							if((deldata->num_colls > 0) && (deldata->completecolls != NULL)) parep_mpi_free(deldata->completecolls);
							
							parep_mpi_free(deldata);
							
							continue;
						}
						cdata = cdata->next;
					}
					
					int rank;
					EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&rank);
					pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
					if(rank == parep_mpi_leader_rank) {
						int dyn_sock;
						dyn_sock = socket(AF_INET, SOCK_STREAM, 0);
						int ret;
						do {
							ret = connect(dyn_sock,(struct sockaddr *)(&parep_mpi_dyn_coordinator_addr),sizeof(parep_mpi_dyn_coordinator_addr));
						} while(ret != 0);
						int cmd = CMD_REM_RECV_FINISHED;
						write(dyn_sock,&cmd,sizeof(int));
						close(dyn_sock);
					}
					pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
					
					parep_mpi_wait_block = 0;
					
					pthread_mutex_lock(&parep_mpi_store_buf_sz_mutex);
					if(parep_mpi_store_buf_sz <= 0x1000000) {
						pthread_mutex_lock(&rem_recv_msg_sent_mutex);
						rem_recv_msg_sent = 0;
						pthread_mutex_unlock(&rem_recv_msg_sent_mutex);
					} else if(parep_mpi_store_buf_sz >= 0x4000000) {
						pthread_mutex_lock(&rem_recv_msg_sent_mutex);
						rem_recv_msg_sent = 1;
						pthread_mutex_unlock(&rem_recv_msg_sent_mutex);
					}
					pthread_mutex_unlock(&parep_mpi_store_buf_sz_mutex);
					
					pthread_cond_signal(&collectiveCond);
					pthread_cond_signal(&peertopeerCond);
					pthread_cond_signal(&reqListCond);
					pthread_mutex_unlock(&collectiveLock);
					pthread_mutex_unlock(&peertopeerLock);
					pthread_rwlock_unlock(&commLock);
					pthread_mutex_unlock(&reqListLock);
					PAREP_MPI_ENABLE_CKPT();
				} else if(cmd == CMD_INFORM_PROC_FAILED) {
					parep_mpi_failed_proc_recv = 1;
					int num_failed_procs;
					msgsize = 0;
					while((bytes_read = read(pfd.fd,(&num_failed_procs)+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					int *abs_failed_ranks = (int *)_real_malloc(sizeof(int) * (num_failed_procs));
					int *failed_ranks = (int *)_real_malloc(sizeof(int) * (num_failed_procs));
					msgsize = 0;
					while((bytes_read = read(pfd.fd,abs_failed_ranks+msgsize, (sizeof(int) * num_failed_procs)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int) * num_failed_procs)) break;
					}
					
					int myrank;
					EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
					int *worldComm_ranks = (int *)_real_malloc(sizeof(int) * (nC+nR));
					int *rankadj = (int *)_real_malloc(sizeof(int) * (nC+nR));
					int *rankadjrep = (int *)_real_malloc(sizeof(int) * (nC+nR));
					//int worldComm_ranks[nC+nR];
					//int rankadj[nC+nR];
					//int rankadjrep[nC+nR];
					int i;
					int num_failed_procs_current;
					
					EMPI_Group current_group;
					EMPI_Group current_failed_group;
					EMPI_Group current_alive_group;
					EMPI_Comm_group(MPI_COMM_WORLD->eworldComm,&current_group);
					
					pthread_rwlock_wrlock(&fgroupLock);
					EMPI_Group failed_group;
					if(parep_mpi_failed_group == EMPI_GROUP_EMPTY) {
						EMPI_Group_incl(parep_mpi_original_group, num_failed_procs, abs_failed_ranks, &parep_mpi_failed_group);
					} else {
						EMPI_Group_incl(parep_mpi_original_group, num_failed_procs, abs_failed_ranks, &failed_group);
						EMPI_Group_union(parep_mpi_failed_group,failed_group,&parep_mpi_failed_group);
					}
					pthread_rwlock_unlock(&fgroupLock);
					
					EMPI_Group_intersection(current_group,parep_mpi_failed_group,&current_failed_group);
					EMPI_Group_size(current_failed_group,&num_failed_procs_current);
					
					for(int j = 0; j < num_failed_procs_current; j++) abs_failed_ranks[j] = j;
					
					EMPI_Group_translate_ranks(current_failed_group,num_failed_procs_current,abs_failed_ranks,current_group,failed_ranks);
					
					parep_mpi_wait_block = 1;
					
					pthread_mutex_lock(&reqListLock);
					
					parep_mpi_failed_proc_recv = 0;
					parep_mpi_wait_block = 0;
					
					//1. Test all current requests if complete
					test_all_requests();
					
					bool progressed;
					
					pthread_rwlock_rdlock(&commLock);
					//2. Probe for any receives. CMP from only CMP. REP from everywhere.
					bool *isProcAlive = (bool *)_real_malloc(sizeof(bool)*(nC+nR));
					for(int j = 0; j < nC+nR; j++) {
						if(num_in_array(failed_ranks,num_failed_procs_current,j)) isProcAlive[j] = false;
						else isProcAlive[j] = true;
					}
					if(myrank >= nC) {
						for(int j = 0; j < nC+nR; j++) {
							if((j != myrank) && (j != repToCmpMap[myrank-nC])) {
								if(isProcAlive[j]) {
									int flag = 0;
									EMPI_Status stat;
									do {
										MPI_Comm comm = MPI_COMM_WORLD;
										if(j < nC) EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
										else EMPI_Iprobe(j-nC,EMPI_ANY_TAG,comm->EMPI_COMM_REP,&flag,&stat);
										if(flag) {
											pthread_rwlock_unlock(&commLock);
											do {
												progressed = test_all_ptp_requests();
											} while(progressed);
											pthread_rwlock_rdlock(&commLock);
											if(j < nC) EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
											else EMPI_Iprobe(j-nC,EMPI_ANY_TAG,comm->EMPI_COMM_REP,&flag,&stat);
											if(flag == 0) {
												flag = 1;
												continue;
											}
											int count;
											int extracount;
											int size;
											ptpdata *curargs;
											curargs = (ptpdata *)parep_mpi_malloc(sizeof(ptpdata));
											curargs->markdelcmp = false;
											curargs->markdelrep = false;
											EMPI_Type_size(EMPI_BYTE,&size);
											EMPI_Get_count(&stat,EMPI_BYTE,&count);
											if(size >= sizeof(int)) extracount = 1;
											else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
											else extracount = (((int)sizeof(int))/size) + 1;
											count -= extracount;
											curargs->buf = parep_mpi_malloc((count+extracount)*size);
											
											if(j < nC) EMPI_Recv(curargs->buf,count+extracount,EMPI_BYTE,j,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
											else EMPI_Recv(curargs->buf,count+extracount,EMPI_BYTE,j-nC,stat.EMPI_TAG,comm->EMPI_COMM_REP,&stat);
											
											memcpy(&(curargs->id),(curargs->buf) + (count * size),sizeof(int));
											MPI_Status status;
											memcpy(&(status.status),&(stat),sizeof(EMPI_Status));
											status.count = count;
											if(stat.EMPI_TAG == EMPI_ANY_TAG) status.MPI_TAG = MPI_ANY_TAG;
											else status.MPI_TAG = stat.EMPI_TAG;
											status.MPI_ERROR = stat.EMPI_ERROR;
											status.MPI_SOURCE = stat.EMPI_SOURCE;
											if(j >= nC) status.MPI_SOURCE = repToCmpMap[stat.EMPI_SOURCE];
											
											curargs->type = MPI_FT_RECV;
											curargs->count = count;
											curargs->dt = MPI_BYTE;
											curargs->target = status.MPI_SOURCE;
											curargs->tag = status.MPI_TAG;
											curargs->comm = comm;
											curargs->completecmp = true;
											curargs->completerep = true;
											
											curargs->req = (MPI_Request *)parep_mpi_malloc(sizeof(MPI_Request));
											*(curargs->req) = MPI_REQUEST_NULL;
											
											pthread_mutex_lock(&peertopeerLock);
											if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
											else first_peertopeer = curargs;
											curargs->prev = NULL;
											curargs->next = last_peertopeer;
											last_peertopeer = curargs;
											pthread_cond_signal(&peertopeerCond);
											pthread_mutex_unlock(&peertopeerLock);
											
											pthread_mutex_lock(&recvDataListLock);
											recvDataListInsert(curargs);
											pthread_cond_signal(&reqListCond);
											pthread_mutex_unlock(&recvDataListLock);
										}
									} while(flag != 0);
								}
							} else if((j < nC) && (j == repToCmpMap[myrank-nC])) {
								if(isProcAlive[j]) {
									pthread_rwlock_unlock(&commLock);
									do {
										progressed = test_all_coll_requests();
									} while(progressed);
									pthread_rwlock_rdlock(&commLock);
									int flag = 0;
									EMPI_Status stat;
									do {
										MPI_Comm comm = MPI_COMM_WORLD;
										EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
										if(flag) {
											pthread_rwlock_unlock(&commLock);
											do {
												progressed = test_all_coll_requests();
											} while(progressed);
											pthread_rwlock_rdlock(&commLock);
											EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
											if(flag == 0) {
												flag = 1;
												continue;
											}
											int count;
											int extracount;
											int size;
											int tag = stat.EMPI_TAG;
											assert((tag == MPI_FT_REDUCE_TAG) || (tag == MPI_FT_ALLREDUCE_TAG));
											clcdata *curargs;
											curargs = (clcdata *)parep_mpi_malloc(sizeof(clcdata));
											EMPI_Type_size(EMPI_BYTE,&size);
											EMPI_Get_count(&stat,EMPI_BYTE,&count);
											if(size >= sizeof(int)) extracount = 1;
											else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
											else extracount = (((int)sizeof(int))/size) + 1;
											count -= extracount;
											curargs->completecmp = false;
											curargs->completerep = false;
											curargs->completecolls = NULL;
											curargs->num_colls = 0;
											curargs->req = (MPI_Request *)parep_mpi_malloc(sizeof(MPI_Request));
											*(curargs->req) = MPI_REQUEST_NULL;
											if(tag == MPI_FT_REDUCE_TAG) {
												curargs->type = MPI_FT_REDUCE_TEMP;
												(curargs->args).reduce.alloc_recvbuf = true;
												(curargs->args).reduce.recvbuf = parep_mpi_malloc((count+extracount)*size);
												EMPI_Recv((curargs->args).reduce.recvbuf,count+extracount,EMPI_BYTE,j,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
												memcpy(&(curargs->id),((curargs->args).reduce.recvbuf) + (count * size),sizeof(int));
												(curargs->args).reduce.comm = comm;
											} else if(tag == MPI_FT_ALLREDUCE_TAG) {
												curargs->type = MPI_FT_ALLREDUCE_TEMP;
												(curargs->args).allreduce.alloc_recvbuf = true;
												(curargs->args).allreduce.recvbuf = parep_mpi_malloc((count+extracount)*size);
												EMPI_Recv((curargs->args).allreduce.recvbuf,count+extracount,EMPI_BYTE,j,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
												memcpy(&(curargs->id),((curargs->args).allreduce.recvbuf) + (count * size),sizeof(int));
												(curargs->args).allreduce.comm = comm;
											}
											
											assert(curargs->id >= parep_mpi_collective_id);
											pthread_mutex_lock(&collectiveLock);
											if(last_collective == NULL) {
												curargs->prev = NULL;
												curargs->next = NULL;
												last_collective = curargs;
											} else {
												clcdata *temp = last_collective;
												while((curargs->id <= temp->id) && (temp->next != NULL)) {
													temp = temp->next;
												}
												if(curargs->id > temp->id) {
													curargs->prev = temp->prev;
													curargs->next = temp;
													if(temp->prev != NULL) temp->prev->next = curargs;
													else last_collective = curargs;
													temp->prev = curargs;
												} else {
													curargs->prev = temp;
													curargs->next = NULL;
													temp->next = curargs;
												}
											}
											pthread_cond_signal(&collectiveCond);
											pthread_mutex_unlock(&collectiveLock);
											
											pthread_mutex_lock(&recvDataListLock);
											recvDataRedListInsert((ptpdata *)curargs);
											pthread_cond_signal(&reqListCond);
											pthread_mutex_unlock(&recvDataListLock);
										}
									} while(flag != 0);
								}
							}
						}
					} else {
						for(int j = 0; j < nC; j++) {
							if(j != myrank) {
								if(isProcAlive[j]) {
									int flag = 0;
									EMPI_Status stat;
									do {
										MPI_Comm comm = MPI_COMM_WORLD;
										EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_COMM_CMP,&flag,&stat);
										if(flag) {
											pthread_rwlock_unlock(&commLock);
											do {
												progressed = test_all_ptp_requests();
											} while(progressed);
											pthread_rwlock_rdlock(&commLock);
											EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_COMM_CMP,&flag,&stat);
											if(flag == 0) {
												flag = 1;
												continue;
											}
											int count;
											int extracount;
											int size;
											ptpdata *curargs;
											curargs = (ptpdata *)parep_mpi_malloc(sizeof(ptpdata));
											curargs->markdelcmp = false;
											curargs->markdelrep = false;
											EMPI_Type_size(EMPI_BYTE,&size);
											EMPI_Get_count(&stat,EMPI_BYTE,&count);
											if(size >= sizeof(int)) extracount = 1;
											else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
											else extracount = (((int)sizeof(int))/size) + 1;
											count -= extracount;
											curargs->buf = parep_mpi_malloc((count+extracount)*size);
											
											EMPI_Recv(curargs->buf,count+extracount,EMPI_BYTE,j,stat.EMPI_TAG,comm->EMPI_COMM_CMP,&stat);
											
											memcpy(&(curargs->id),(curargs->buf) + (count * size),sizeof(int));
											MPI_Status status;
											memcpy(&(status.status),&(stat),sizeof(EMPI_Status));
											status.count = count;
											if(stat.EMPI_TAG == EMPI_ANY_TAG) status.MPI_TAG = MPI_ANY_TAG;
											else status.MPI_TAG = stat.EMPI_TAG;
											status.MPI_ERROR = stat.EMPI_ERROR;
											status.MPI_SOURCE = stat.EMPI_SOURCE;
											
											curargs->type = MPI_FT_RECV;
											curargs->count = count;
											curargs->dt = MPI_BYTE;
											curargs->target = status.MPI_SOURCE;
											curargs->tag = status.MPI_TAG;
											curargs->comm = comm;
											curargs->completecmp = true;
											curargs->completerep = true;
											
											curargs->req = (MPI_Request *)parep_mpi_malloc(sizeof(MPI_Request));
											*(curargs->req) = MPI_REQUEST_NULL;
											
											pthread_mutex_lock(&peertopeerLock);
											if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
											else first_peertopeer = curargs;
											curargs->prev = NULL;
											curargs->next = last_peertopeer;
											last_peertopeer = curargs;
											pthread_cond_signal(&peertopeerCond);
											pthread_mutex_unlock(&peertopeerLock);
											
											pthread_mutex_lock(&recvDataListLock);
											recvDataListInsert(curargs);
											pthread_cond_signal(&reqListCond);
											pthread_mutex_unlock(&recvDataListLock);
										}
									} while(flag != 0);
								}
							}
						}
					}
					_real_free(isProcAlive);
					pthread_rwlock_unlock(&commLock);
					
					for(i = 0; i < nR+nC; i++) {
						rankadjrep[i] = 0;
						rankadj[i] = 0;
						worldComm_ranks[i] = i;
					}
					int nRdiff = 0;
					int nCdiff = 0;
					bool switching_to_cmp = false;
					for (int j = 0; j < num_failed_procs_current;j++) {
						if(myrank >= nC) {
							if (repToCmpMap[myrank-nC] == failed_ranks[j]) switching_to_cmp = true;
						}
						if(failed_ranks[j] < nC) {
							if(cmpToRepMap[failed_ranks[j]] == -1) {
								//NO REPLICA ABORT
								printf("No Replica exists aborting\n");
								aborting = true;
								break;
							} else {
								nRdiff++;
								worldComm_ranks[failed_ranks[j]] = worldComm_ranks[cmpToRepMap[failed_ranks[j]]+nC];
								worldComm_ranks[cmpToRepMap[failed_ranks[j]]+nC] = -1;
								for(i = cmpToRepMap[failed_ranks[j]]+nC+1; i < nC+nR; i++) rankadjrep[i]++;
								repToCmpMap[cmpToRepMap[failed_ranks[j]]] = -1;
								cmpToRepMap[failed_ranks[j]] = -1;
								//failed_ranks[j] = worldComm_ranks[failed_ranks[j]];
							}
						} else {
							nRdiff++;
							worldComm_ranks[failed_ranks[j]] = -1;
							for(i = failed_ranks[j]+1; i < nC+nR; i++) rankadjrep[i]++;
							cmpToRepMap[repToCmpMap[failed_ranks[j]-nC]] = -1;
							repToCmpMap[failed_ranks[j]-nC] = -1;
						}
					}
					if(!aborting) {
						for(i = 0; i < nC+nR; i++) {
							if(worldComm_ranks[i] != -1) worldComm_ranks[i - rankadj[i] - rankadjrep[i]] = worldComm_ranks[i];
							if(i < nC) if(cmpToRepMap[i] != -1) cmpToRepMap[i - rankadj[i]] = cmpToRepMap[i] - rankadjrep[cmpToRepMap[i]+nC];
							if(i < nR) if(repToCmpMap[i] != -1) repToCmpMap[i - rankadjrep[i+nC]] = repToCmpMap[i] - rankadj[repToCmpMap[i]];
						}
						
						nC = nC - nCdiff;
						nR = nR - nRdiff;
					}
					
					if(!aborting) {
						EMPI_Group_incl(current_group, nR+nC, worldComm_ranks, &current_alive_group);
					}
					else {
						EMPI_Group_difference(current_group,current_failed_group,&current_alive_group);
					}
					
					_real_free(worldComm_ranks);
					_real_free(rankadj);
					_real_free(rankadjrep);
					_real_free(abs_failed_ranks);
					_real_free(failed_ranks);
					
					int active_thread;
					int shrink_perf;
					
					pthread_mutex_lock(&thread_active_mutex);
					active_thread = thread_active;
					pthread_mutex_unlock(&thread_active_mutex);
					pthread_mutex_lock(&performing_shrink_mutex);
					shrink_perf = performing_shrink;
					pthread_mutex_unlock(&performing_shrink_mutex);
					if(active_thread == 0) {
						pthread_rwlock_wrlock(&commLock);
						//parep_infiniband_cmd(PAREP_IB_PRE_SHRINK);
						//pthread_create(&comm_shrinker,NULL,comm_shrink,&current_alive_group);
						_real_pthread_create(&comm_shrinker,NULL,comm_shrink,&current_alive_group);
					} else if(active_thread == 1) {
						if(shrink_perf == 1) {
							pthread_cancel(comm_shrinker);
							pthread_join(comm_shrinker,NULL);
							_real_pthread_create(&comm_shrinker,NULL,comm_shrink,&current_alive_group);
							//pthread_create(&comm_shrinker,NULL,comm_shrink,&current_alive_group);
						} else if(shrink_perf == 0) {
							pthread_mutex_lock(&thread_active_mutex);
							while(thread_active == 1) {
								pthread_cond_wait(&thread_active_cond,&thread_active_mutex);
							}
							pthread_mutex_unlock(&thread_active_mutex);
							memcpy(&(MPI_COMM_WORLD->eworldComm),&parep_mpi_new_comm,sizeof(EMPI_Comm));
							pthread_join(comm_shrinker,NULL);
							waiting_for_resp = 1;
							_real_pthread_create(&comm_shrinker,NULL,comm_shrink,&current_alive_group);
							//pthread_create(&comm_shrinker,NULL,comm_shrink,&current_alive_group);
						}
					}
				} else if(cmd == CMD_SHRINK_PERFORMED) {
					if(waiting_for_resp == 0) {
						memcpy(&(MPI_COMM_WORLD->eworldComm),&parep_mpi_new_comm,sizeof(EMPI_Comm));
						int newrank;
						EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&newrank);
						int *node_ids = (int *)_real_malloc(sizeof(int) * (nC+nR));
						node_ids[newrank] = parep_mpi_node_id;
						EMPI_Allgather(&parep_mpi_node_id,1,EMPI_INT,node_ids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm);
						for(int j = 0; j < nC+nR; j++) {
							if(node_ids[j] == 0) {
								pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
								parep_mpi_leader_rank = j;
								pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
								break;
							}
						}
						_real_free(node_ids);
						if(aborting) {
							EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
							pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
							if(newrank == parep_mpi_leader_rank) {
								parep_infiniband_cmd(PAREP_IB_KILL_COORDINATOR);
							}
							pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
							//pthread_join(daemon_poller,NULL);
							//dlclose(extLib);
							
							while(1);
							
							pollret = poll(&pfd,1,-1);
							int cmd;
							int msgsize = 0;
							size_t bytes_read;
							while((bytes_read = read(pfd.fd,(&cmd)+msgsize, sizeof(int)-msgsize)) > 0) {
								msgsize += bytes_read;
								if(msgsize >= (sizeof(int))) break;
							}
							close(parep_mpi_coordinator_socket);
							
							assert(cmd == CMD_EXIT_CALLED);
							
							exit(1);
						}
						if (nR > 0) {
							int color = (newrank < nC) ? 0 : 1;
							int cmpLeader = 0;
							int repLeader = nC;
							
							if (color == 0) {
								EMPI_Comm_split(MPI_COMM_WORLD->eworldComm, color, 0, &(MPI_COMM_WORLD->EMPI_COMM_CMP));
								EMPI_Intercomm_create(MPI_COMM_WORLD->EMPI_COMM_CMP, 0, MPI_COMM_WORLD->eworldComm, repLeader, 100, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
								int colorinternal;
								if(cmpToRepMap[newrank] != -1) colorinternal = EMPI_UNDEFINED;
								else colorinternal = 1;
								EMPI_Comm_split (MPI_COMM_WORLD->EMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
								if(MPI_COMM_WORLD->EMPI_CMP_NO_REP != EMPI_COMM_NULL) EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->eworldComm, repLeader, 101, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
								MPI_COMM_WORLD->EMPI_COMM_REP = EMPI_COMM_NULL;
							} else if (color == 1) {
								EMPI_Comm_split (MPI_COMM_WORLD->eworldComm, color, 0, &(MPI_COMM_WORLD->EMPI_COMM_REP));
								EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, cmpLeader, 100, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
								if(nC > nR) {
									int rleader = nR;
									for(int j = 0; j < nC; j++) {
										if(cmpToRepMap[j] == -1) {
											rleader = j;
											break;
										}
									}
									EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, rleader, 101, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
								}
								MPI_COMM_WORLD->EMPI_COMM_CMP = EMPI_COMM_NULL;
							}
						} else {
							EMPI_Comm_dup(MPI_COMM_WORLD->eworldComm,&(MPI_COMM_WORLD->EMPI_COMM_CMP));
							EMPI_Comm_dup(MPI_COMM_WORLD->eworldComm,&(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
						}
						pthread_join(comm_shrinker,NULL);
						printf("%d: Shrink completed newrank %d parep_mpi_rank %d new leader rank %d\n",getpid(),newrank,parep_mpi_rank,parep_mpi_leader_rank);
						//parep_infiniband_cmd(PAREP_IB_POST_SHRINK);
						pthread_mutex_lock(&peertopeerLock);
						pthread_mutex_lock(&collectiveLock);
						parep_mpi_restore_messages();
						pthread_cond_signal(&collectiveCond);
						pthread_cond_signal(&peertopeerCond);
						pthread_cond_signal(&reqListCond);
						pthread_mutex_unlock(&collectiveLock);
						pthread_mutex_unlock(&peertopeerLock);
						pthread_rwlock_unlock(&commLock);
						pthread_mutex_unlock(&reqListLock);
					} else {
						waiting_for_resp = 0;
					}
				} else if(cmd == CMD_MPI_INITIALIZED) {
					pthread_mutex_lock(&parep_mpi_empi_initialized_mutex);
					parep_mpi_empi_initialized = true;
					pthread_cond_signal(&parep_mpi_empi_initialized_cond);
					pthread_mutex_unlock(&parep_mpi_empi_initialized_mutex);
				} else if(cmd == CMD_EXIT_CALLED) {
					pthread_exit(NULL);
				}
			}
		}
	}
}