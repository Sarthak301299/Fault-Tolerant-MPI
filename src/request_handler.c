#include "client_thread.h"
#include "request_handler.h"
#include "heap_allocator.h"

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

extern pthread_rwlock_t commLock;

reqNode *reqHead = NULL;
reqNode *reqTail = NULL;

pthread_mutex_t reqListLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t reqListCond = PTHREAD_COND_INITIALIZER;

extern struct peertopeer_data *first_peertopeer;
extern struct peertopeer_data *last_peertopeer;
extern struct collective_data *last_collective;

extern pthread_mutex_t peertopeerLock;
extern pthread_cond_t peertopeerCond;

extern recvDataNode *recvDataRedHead;
extern recvDataNode *recvDataRedTail;

extern pthread_mutex_t recvDataListLock;
extern pthread_cond_t recvDataListCond;

extern pthread_mutex_t collectiveLock;
extern pthread_cond_t collectiveCond;

extern int parep_mpi_num_commbuf_nodes;
extern bool parep_mpi_empi_entered;

extern volatile sig_atomic_t parep_mpi_wait_block;

reqNode *reqListInsert(MPI_Request req) {
	reqNode *newnode = parep_mpi_malloc(sizeof(reqNode));
	newnode->req = req;
	newnode->next = NULL;
	newnode->prev = NULL;
	if(reqTail == NULL) {
		reqHead = newnode;
	} else {
		newnode->prev = reqTail;
		reqTail->next = newnode;
	}
	reqTail = newnode;
	return newnode;
}

void reqListDelete(reqNode *rnode) {
	rnode->req = MPI_REQUEST_NULL;
	if(rnode->prev != NULL) rnode->prev->next = rnode->next;
	else reqHead = rnode->next;
	if(rnode->next != NULL) rnode->next->prev = rnode->prev;
	else reqTail = rnode->prev;
	parep_mpi_free(rnode);
}

bool reqListIsEmpty() {
	return reqHead == NULL;
}

bool test_all_coll_requests() {
	bool progressed = false;
	bool signal_completion = false;
	for(reqNode *start = reqHead; start != NULL; start = start->next) {
		if(!(start->req->complete)) {
			if(start->req->type == MPI_FT_COLLECTIVE_REQUEST) {
				MPI_Request req = start->req;
				clcdata *cdata = (clcdata *)(req->storeloc);
				if(!(cdata->completecmp)) {
					int flag;
					EMPI_Test(req->reqcmp,&flag,&((req->status).status));
					cdata->completecmp = flag > 0;
					if(cdata->completecmp) {
						progressed = true;
						bool alt_comms = false;
						if(getenv("ENABLE_ALT_COMMS")) {
							if(!strcmp(getenv("ENABLE_ALT_COMMS"),"1")) {
								alt_comms = true;
							}
						}
						if(alt_comms) {
							if((cdata->type == MPI_FT_BCAST) || (cdata->type == MPI_FT_ALLTOALL)) {
								void *dest_recvbuf = start->req->bufloc;
								int size;
								int myrank;
								if(cdata->type == MPI_FT_BCAST) {
									EMPI_Comm_rank((cdata->args).bcast.comm->EMPI_COMM_CMP,&myrank);
									if(!(cdata->completerep)) EMPI_Isend((cdata->args).bcast.buf,(cdata->args).bcast.count,(cdata->args).bcast.dt->edatatype,cmpToRepMap[myrank],MPI_FT_REDUCE_TAG,(cdata->args).bcast.comm->EMPI_CMP_REP_INTERCOMM,req->reqrep);
								} else if(cdata->type == MPI_FT_ALLTOALL) {
									EMPI_Comm_rank((cdata->args).alltoall.comm->EMPI_COMM_CMP,&myrank);
									if(!(cdata->completerep)) EMPI_Isend (dest_recvbuf, (cdata->args).alltoall.recvcount, (cdata->args).alltoall.recvdt->edatatype, cmpToRepMap[myrank], MPI_FT_ALLREDUCE_TAG, (cdata->args).alltoall.comm->EMPI_CMP_REP_INTERCOMM, req->reqrep);
								}
							}
						} else {
							if((cdata->type == MPI_FT_REDUCE) || (cdata->type == MPI_FT_ALLREDUCE)) {
								void *dest_recvbuf = start->req->bufloc;
								int size;
								int myrank;
								if(cdata->type == MPI_FT_REDUCE) {
									int extracount;
									int dis;
									pthread_rwlock_rdlock(&commLock);
									EMPI_Comm_rank((cdata->args).reduce.comm->EMPI_COMM_CMP,&myrank);
									EMPI_Type_size((cdata->args).reduce.dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									dis = (((cdata->args).reduce.count+extracount)*size) - sizeof(int);
									memcpy((cdata->args).reduce.recvbuf,dest_recvbuf,(cdata->args).reduce.count * size);
									assert(*((int *)((cdata->args).reduce.recvbuf + dis)) == cdata->id);
									if(!(cdata->completerep)) EMPI_Isend((cdata->args).reduce.recvbuf,(cdata->args).reduce.count+extracount,(cdata->args).reduce.dt->edatatype,cmpToRepMap[myrank],MPI_FT_REDUCE_TAG,(cdata->args).reduce.comm->EMPI_CMP_REP_INTERCOMM,req->reqrep);
									pthread_rwlock_unlock(&commLock);
								} else if (cdata->type == MPI_FT_ALLREDUCE) {
									int extracount;
									int dis;
									pthread_rwlock_rdlock(&commLock);
									EMPI_Comm_rank((cdata->args).allreduce.comm->EMPI_COMM_CMP,&myrank);
									EMPI_Type_size((cdata->args).allreduce.dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									dis = (((cdata->args).allreduce.count+extracount)*size) - sizeof(int);
									memcpy((cdata->args).allreduce.recvbuf,dest_recvbuf,(cdata->args).allreduce.count * size);
									assert(*((int *)((cdata->args).allreduce.recvbuf + dis)) == cdata->id);
									if(!(cdata->completerep)) EMPI_Isend((cdata->args).allreduce.recvbuf,(cdata->args).allreduce.count+extracount,(cdata->args).allreduce.dt->edatatype,cmpToRepMap[myrank],MPI_FT_ALLREDUCE_TAG,(cdata->args).allreduce.comm->EMPI_CMP_REP_INTERCOMM,req->reqrep);
									pthread_rwlock_unlock(&commLock);
								}
							}
						}
					}
				}
				if(!(cdata->completerep)) {
					int flag;
					if(*(req->reqrep) != EMPI_REQUEST_NULL) EMPI_Test(req->reqrep,&flag,&((req->status).status));
					else flag = 0;
					cdata->completerep = flag > 0;
					if(cdata->completerep) {
						progressed = true;
						bool alt_comms = false;
						if(getenv("ENABLE_ALT_COMMS")) {
							if(!strcmp(getenv("ENABLE_ALT_COMMS"),"1")) {
								alt_comms = true;
							}
						}
						if(alt_comms) {
						} else {
							if((cdata->type == MPI_FT_REDUCE) || (cdata->type == MPI_FT_ALLREDUCE)) {
								void *dest_recvbuf = start->req->bufloc;
								int size;
								int myrank;
								if(cdata->type == MPI_FT_REDUCE) {
									int extracount;
									int dis;
									pthread_rwlock_rdlock(&commLock);
									EMPI_Type_size((cdata->args).reduce.dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									dis = (((cdata->args).reduce.count+extracount)*size) - sizeof(int);
									if((cdata->args).reduce.comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
										memcpy(dest_recvbuf,(cdata->args).reduce.recvbuf,(cdata->args).reduce.count * size);
										if((*((int *)((cdata->args).reduce.recvbuf + dis)) & 0xF0000000) == 0x70000000) {
											assert(*((int *)((cdata->args).reduce.recvbuf + dis)) == cdata->id);
										} else {
											cdata->completerep = false;
											*(req->reqrep) = EMPI_REQUEST_NULL;
										}
									}
									pthread_rwlock_unlock(&commLock);
								} else if (cdata->type == MPI_FT_ALLREDUCE) {
									int extracount;
									int dis;
									pthread_rwlock_rdlock(&commLock);
									EMPI_Type_size((cdata->args).allreduce.dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									dis = (((cdata->args).allreduce.count+extracount)*size) - sizeof(int);
									if((cdata->args).allreduce.comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
										memcpy(dest_recvbuf,(cdata->args).allreduce.recvbuf,(cdata->args).allreduce.count * size);
										if((*((int *)((cdata->args).allreduce.recvbuf + dis)) & 0xF0000000) == 0x70000000) {
											assert(*((int *)((cdata->args).allreduce.recvbuf + dis)) == cdata->id);
										} else {
											cdata->completerep = false;
											*(req->reqrep) = EMPI_REQUEST_NULL;
										}
									}
									pthread_rwlock_unlock(&commLock);
								}
							}
						}
					}
				}
				for(int i = 0; i < cdata->num_colls; i++) {
					if(!(cdata->completecolls[i])) {
						int flag;
						EMPI_Test(req->reqcolls[i],&flag,&((req->status).status));
						cdata->completecolls[i] = flag > 0;
						if(cdata->completecolls[i]) progressed = true;
					}
				}
				req->complete = (cdata->completecmp) & (cdata->completerep);
				for(int i = 0; i < cdata->num_colls; i++) {
					req->complete = req->complete & cdata->completecolls[i];
				}
				if(req->complete) {
					progressed = true;
					bool alt_comms = false;
					if(getenv("ENABLE_ALT_COMMS")) {
						if(!strcmp(getenv("ENABLE_ALT_COMMS"),"1")) {
							alt_comms = true;
						}
					}
					if(alt_comms){
						if(cdata->type == MPI_FT_ALLREDUCE) {
							pthread_rwlock_rdlock(&commLock);
							if((cdata->args).allreduce.comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
								if((cdata->args).allreduce.dt == MPI_INT) {
									if((cdata->args).allreduce.op == MPI_SUM) {
										int *dest_recvbuf = (int *)start->req->bufloc;
										int *src = (int *)(cdata->args).allreduce.recvbuf;
										for(int i = 0; i < (cdata->args).allreduce.count; i++) {
											dest_recvbuf[i] += src[i];
										}
									} else if((cdata->args).allreduce.op == MPI_MAX) {
										int *dest_recvbuf = (int *)start->req->bufloc;
										int *src = (int *)(cdata->args).allreduce.recvbuf;
										for(int i = 0; i < (cdata->args).allreduce.count; i++) {
											if(src[i] > dest_recvbuf[i]) {
												dest_recvbuf[i] = src[i];
											}
										}
									}
								} else if ((cdata->args).allreduce.dt == MPI_DOUBLE) {
									if((cdata->args).allreduce.op == MPI_SUM) {
										double *dest_recvbuf = (double *)start->req->bufloc;
										double *src = (double *)(cdata->args).allreduce.recvbuf;
										for(int i = 0; i < (cdata->args).allreduce.count; i++) {
											dest_recvbuf[i] += src[i];
										}
									} else if((cdata->args).allreduce.op == MPI_MAX) {
										double *dest_recvbuf = (double *)start->req->bufloc;
										double *src = (double *)(cdata->args).allreduce.recvbuf;
										for(int i = 0; i < (cdata->args).allreduce.count; i++) {
											if(src[i] > dest_recvbuf[i]) {
												dest_recvbuf[i] = src[i];
											}
										}
									}
								}
							}
							pthread_rwlock_unlock(&commLock);
						}
					}
					signal_completion = true;
				}
			}
		}/* else {
			signal_completion = true;
		}*/
		if(signal_completion) pthread_cond_signal(&reqListCond);
	}
	return progressed;
}

bool test_all_ptp_requests() {
	bool progressed = false;
	bool signal_completion = false;
	for(reqNode *start = reqHead; start != NULL; start = start->next) {
		if(!(start->req->complete)) {
			if(!(start->req->type == MPI_FT_COLLECTIVE_REQUEST) && !(start->req->type == MPI_FT_READ_REQUEST) && !(start->req->type == MPI_FT_WRITE_REQUEST)) {
				if(!(((ptpdata *)(start->req->storeloc))->completecmp) && (*(start->req->reqcmp) != EMPI_REQUEST_NULL)) {
					int flag;
					EMPI_Test(start->req->reqcmp,&flag,&((start->req->status).status));
					(((ptpdata *)(start->req->storeloc))->completecmp) = flag > 0;
					if((((ptpdata *)(start->req->storeloc))->completecmp)) {
						progressed = true;
						if(start->req->type == MPI_FT_RECV_REQUEST) {
							pthread_rwlock_rdlock(&commLock);
							if((start->req->comm->EMPI_COMM_CMP) != EMPI_COMM_NULL) {
								int size;
								int extracount;
								int count;
								EMPI_Type_size(((ptpdata *)(start->req->storeloc))->dt->edatatype,&size);
								if(size >= sizeof(int)) extracount = 1;
								else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
								else extracount = (((int)sizeof(int))/size) + 1;
								EMPI_Get_count(&((start->req->status).status),((ptpdata *)(start->req->storeloc))->dt->edatatype,&count);
								count -= extracount;
								memcpy(start->req->bufloc,((ptpdata *)(start->req->storeloc))->buf,count*size);
								memcpy(&(((ptpdata *)(start->req->storeloc))->id),((((ptpdata *)(start->req->storeloc))->buf) + (((count+extracount) * size) - sizeof(int))),sizeof(int));
								if(((((ptpdata *)(start->req->storeloc))->id) & 0xF0000000) == 0x70000000) {
									if(start->req->cbuf == NULL) parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
									else {
										return_commbuf_node(start->req->cbuf);
										((ptpdata *)(start->req->storeloc))->buf = NULL;
										start->req->cbuf = NULL;
									}
									
									(start->req->status).count = count;
									if((((start->req->status).status).EMPI_TAG) == EMPI_ANY_TAG) (start->req->status).MPI_TAG = MPI_ANY_TAG;
									else (start->req->status).MPI_TAG = ((start->req->status).status).EMPI_TAG;
									(start->req->status).MPI_ERROR = ((start->req->status).status).EMPI_ERROR;
									(start->req->status).MPI_SOURCE = ((start->req->status).status).EMPI_SOURCE;
									
									pthread_mutex_lock(&peertopeerLock);
									((ptpdata *)(start->req->storeloc))->count = count;
									((ptpdata *)(start->req->storeloc))->target = (start->req->status).MPI_SOURCE;
									((ptpdata *)(start->req->storeloc))->tag = (start->req->status).MPI_TAG;
									pthread_mutex_unlock(&peertopeerLock);
									
									if(!(((ptpdata *)(start->req->storeloc))->completerep)) {
										int cmprank;
										EMPI_Comm_rank(start->req->comm->EMPI_COMM_CMP,&cmprank);
										((ptpdata *)(start->req->storeloc))->buf = parep_mpi_malloc(3*sizeof(int));
										((int *)(((ptpdata *)(start->req->storeloc))->buf))[0] = ((ptpdata *)(start->req->storeloc))->target;
										((int *)(((ptpdata *)(start->req->storeloc))->buf))[1] = ((ptpdata *)(start->req->storeloc))->tag;
										((int *)(((ptpdata *)(start->req->storeloc))->buf))[2] = ((ptpdata *)(start->req->storeloc))->id;
										EMPI_Isend(((ptpdata *)(start->req->storeloc))->buf, 3, EMPI_INT, cmpToRepMap[cmprank], MPI_FT_WILDCARD_TAG, start->req->comm->EMPI_CMP_REP_INTERCOMM, start->req->reqrep);
									}
								} else {
									(((ptpdata *)(start->req->storeloc))->completecmp) = false;
									*(start->req->reqcmp) = EMPI_REQUEST_NULL;
								}
								
							} else if((start->req->comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
								int target,tag,id;
								target = ((int *)(((ptpdata *)(start->req->storeloc))->buf))[0];
								tag = ((int *)(((ptpdata *)(start->req->storeloc))->buf))[1];
								id = ((int *)(((ptpdata *)(start->req->storeloc))->buf))[2];
								if((id & 0xF0000000) == 0x70000000) {
									((ptpdata *)(start->req->storeloc))->target = target;
									((ptpdata *)(start->req->storeloc))->tag = tag;
									((ptpdata *)(start->req->storeloc))->id = id;
									
									pthread_mutex_lock(&recvDataListLock);
									recvDataNode *rnode = recvDataListFindWithId(target,tag,id,start->req->comm);
									ptpdata *curargs;
									int size;
									EMPI_Type_size(((ptpdata *)(start->req->storeloc))->dt->edatatype,&size);
									if(rnode != NULL) {
										curargs = rnode->pdata;
										recvDataListDelete(rnode);
										pthread_cond_signal(&recvDataListCond);
										pthread_mutex_unlock(&recvDataListLock);
										
										int recvcount = curargs->count;
										recvcount = recvcount/size;
										
										memcpy(start->req->bufloc,curargs->buf,recvcount*size);
										start->req->status.count = recvcount;
										start->req->status.MPI_TAG = curargs->tag;
										start->req->status.MPI_SOURCE = curargs->target;
										start->req->status.MPI_ERROR = 0;
										
										pthread_mutex_lock(&peertopeerLock);
										parep_mpi_free(curargs->buf);
										curargs->count = recvcount;
										curargs->dt = ((ptpdata *)(start->req->storeloc))->dt;
										parep_mpi_free(curargs->req);
										curargs->req = &(start->req);
										parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
										struct peertopeer_data *deldata = ((ptpdata *)(start->req->storeloc));
										if(deldata == first_peertopeer) {
											first_peertopeer = first_peertopeer->prev;
										}
										if(deldata == last_peertopeer) {
											last_peertopeer = last_peertopeer->next;
										}
										if(deldata->next != NULL) deldata->next->prev = deldata->prev;
										if(deldata->prev != NULL) deldata->prev->next = deldata->next;
										parep_mpi_free(deldata);
										start->req->storeloc = (void *)curargs;
										pthread_mutex_unlock(&peertopeerLock);
									} else {
										pthread_mutex_unlock(&recvDataListLock);
										int count = ((ptpdata *)(start->req->storeloc))->count;
										int size;
										int extracount;
										EMPI_Type_size(((ptpdata *)(start->req->storeloc))->dt->edatatype,&size);
										if(size >= sizeof(int)) extracount = 1;
										else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
										else extracount = (((int)sizeof(int))/size) + 1;
										if((count+extracount)*size < 3*sizeof(int)) {
											parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
											((ptpdata *)(start->req->storeloc))->buf = parep_mpi_malloc((count+extracount)*size);
										}
										if(cmpToRepMap[target] == -1) EMPI_Irecv (((ptpdata *)(start->req->storeloc))->buf, count+extracount, ((ptpdata *)(start->req->storeloc))->dt->edatatype, target, tag, (start->req->comm->EMPI_CMP_REP_INTERCOMM), (start->req->reqrep));
										else EMPI_Irecv (((ptpdata *)(start->req->storeloc))->buf, count+extracount, ((ptpdata *)(start->req->storeloc))->dt->edatatype, cmpToRepMap[target], tag, (start->req->comm->EMPI_COMM_REP), (start->req->reqrep));
									}
								} else {
									(((ptpdata *)(start->req->storeloc))->completecmp) = false;
									*(start->req->reqcmp) = EMPI_REQUEST_NULL;
								}
							}
							pthread_rwlock_unlock(&commLock);
						}
					}
				}
				if(!(((ptpdata *)(start->req->storeloc))->completerep) && (*(start->req->reqrep) != EMPI_REQUEST_NULL)) {
					int flag;
					EMPI_Test(start->req->reqrep,&flag,&((start->req->status).status));
					((ptpdata *)(start->req->storeloc))->completerep = flag > 0;
					if(((ptpdata *)(start->req->storeloc))->completerep) {
						progressed = true;
						if(start->req->type == MPI_FT_RECV_REQUEST) {
							pthread_rwlock_rdlock(&commLock);
							if((start->req->comm->EMPI_COMM_CMP) != EMPI_COMM_NULL) {
								parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
							} else if((start->req->comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
								int size;
								int extracount;
								int count;
								EMPI_Type_size(((ptpdata *)(start->req->storeloc))->dt->edatatype,&size);
								if(size >= sizeof(int)) extracount = 1;
								else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
								else extracount = (((int)sizeof(int))/size) + 1;
								EMPI_Get_count(&((start->req->status).status),((ptpdata *)(start->req->storeloc))->dt->edatatype,&count);
								count -= extracount;
								memcpy(start->req->bufloc,((ptpdata *)(start->req->storeloc))->buf,count*size);
								memcpy(&(((ptpdata *)(start->req->storeloc))->id),((((ptpdata *)(start->req->storeloc))->buf) + (((count+extracount) * size) - sizeof(int))),sizeof(int));
								if(((((ptpdata *)(start->req->storeloc))->id) & 0xF0000000) == 0x70000000) {
									if(start->req->cbuf == NULL) parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
									else {
										return_commbuf_node(start->req->cbuf);
										((ptpdata *)(start->req->storeloc))->buf = NULL;
										start->req->cbuf = NULL;
									}
									
									(start->req->status).count = count;
									if((((start->req->status).status).EMPI_TAG) == EMPI_ANY_TAG) (start->req->status).MPI_TAG = MPI_ANY_TAG;
									else (start->req->status).MPI_TAG = ((start->req->status).status).EMPI_TAG;
									(start->req->status).MPI_ERROR = ((start->req->status).status).EMPI_ERROR;
									(start->req->status).MPI_SOURCE = ((start->req->status).status).EMPI_SOURCE;
									if(cmpToRepMap[((ptpdata *)(start->req->storeloc))->target] != -1) {
										(start->req->status).MPI_SOURCE = repToCmpMap[((start->req->status).status).EMPI_SOURCE];
									}
									
									pthread_mutex_lock(&peertopeerLock);
									((ptpdata *)(start->req->storeloc))->count = count;
									((ptpdata *)(start->req->storeloc))->target = (start->req->status).MPI_SOURCE;
									((ptpdata *)(start->req->storeloc))->tag = (start->req->status).MPI_TAG;
									pthread_mutex_unlock(&peertopeerLock);
								} else {
									(((ptpdata *)(start->req->storeloc))->completerep) = false;
									*(start->req->reqrep) = EMPI_REQUEST_NULL;
								}
							}
							pthread_rwlock_unlock(&commLock);
						}
					}
				}
				start->req->complete = (((ptpdata *)(start->req->storeloc))->completecmp) & (((ptpdata *)(start->req->storeloc))->completerep);
				if(start->req->complete) {
					progressed = true;
					signal_completion = true;
				}
			}
		}/* else {
			signal_completion = true;
		}*/
		if(signal_completion) pthread_cond_signal(&reqListCond);
	}
	return progressed;
}

void test_all_requests_no_lock() {
	bool signal_completion = false;
	for(reqNode *start = reqHead; start != NULL; start = start->next) {
		if(!(start->req->complete)) {
			if(start->req->type == MPI_FT_COLLECTIVE_REQUEST) {
				MPI_Request req = start->req;
				clcdata *cdata = (clcdata *)(req->storeloc);
				if(!(cdata->completecmp)) {
					int flag;
					EMPI_Test(req->reqcmp,&flag,&((req->status).status));
					cdata->completecmp = flag > 0;
					if(cdata->completecmp) {
						bool alt_comms = false;
						if(getenv("ENABLE_ALT_COMMS")) {
							if(!strcmp(getenv("ENABLE_ALT_COMMS"),"1")) {
								alt_comms = true;
							}
						}
						if(alt_comms) {
							if((cdata->type == MPI_FT_BCAST) || (cdata->type == MPI_FT_ALLTOALL)) {
								void *dest_recvbuf = start->req->bufloc;
								int size;
								int myrank;
								if(cdata->type == MPI_FT_BCAST) {
									EMPI_Comm_rank((cdata->args).bcast.comm->EMPI_COMM_CMP,&myrank);
									if(!(cdata->completerep)) EMPI_Isend((cdata->args).bcast.buf,(cdata->args).bcast.count,(cdata->args).bcast.dt->edatatype,cmpToRepMap[myrank],MPI_FT_REDUCE_TAG,(cdata->args).bcast.comm->EMPI_CMP_REP_INTERCOMM,req->reqrep);
								} else if(cdata->type == MPI_FT_ALLTOALL) {
									EMPI_Comm_rank((cdata->args).alltoall.comm->EMPI_COMM_CMP,&myrank);
									if(!(cdata->completerep)) EMPI_Isend (dest_recvbuf, (cdata->args).alltoall.recvcount, (cdata->args).alltoall.recvdt->edatatype, cmpToRepMap[myrank], MPI_FT_ALLREDUCE_TAG, (cdata->args).alltoall.comm->EMPI_CMP_REP_INTERCOMM, req->reqrep);
								}
							}
						} else {
							if((cdata->type == MPI_FT_REDUCE) || (cdata->type == MPI_FT_ALLREDUCE)) {
								void *dest_recvbuf = start->req->bufloc;
								int size;
								int myrank;
								if(cdata->type == MPI_FT_REDUCE) {
									int extracount;
									int dis;
									EMPI_Comm_rank((cdata->args).reduce.comm->EMPI_COMM_CMP,&myrank);
									EMPI_Type_size((cdata->args).reduce.dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									dis = (((cdata->args).reduce.count+extracount)*size) - sizeof(int);
									memcpy((cdata->args).reduce.recvbuf,dest_recvbuf,(cdata->args).reduce.count * size);
									assert(*((int *)((cdata->args).reduce.recvbuf + dis)) == cdata->id);
									if(!(cdata->completerep)) EMPI_Isend((cdata->args).reduce.recvbuf,(cdata->args).reduce.count+extracount,(cdata->args).reduce.dt->edatatype,cmpToRepMap[myrank],MPI_FT_REDUCE_TAG,(cdata->args).reduce.comm->EMPI_CMP_REP_INTERCOMM,req->reqrep);
								} else if (cdata->type == MPI_FT_ALLREDUCE) {
									int extracount;
									int dis;
									EMPI_Comm_rank((cdata->args).allreduce.comm->EMPI_COMM_CMP,&myrank);
									EMPI_Type_size((cdata->args).allreduce.dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									dis = (((cdata->args).allreduce.count+extracount)*size) - sizeof(int);
									memcpy((cdata->args).allreduce.recvbuf,dest_recvbuf,(cdata->args).allreduce.count * size);
									assert(*((int *)((cdata->args).allreduce.recvbuf + dis)) == cdata->id);
									if(!(cdata->completerep)) EMPI_Isend((cdata->args).allreduce.recvbuf,(cdata->args).allreduce.count+extracount,(cdata->args).allreduce.dt->edatatype,cmpToRepMap[myrank],MPI_FT_ALLREDUCE_TAG,(cdata->args).allreduce.comm->EMPI_CMP_REP_INTERCOMM,req->reqrep);
								}
							}
						}
					}
				}
				if(!(cdata->completerep)) {
					int flag;
					if(*(req->reqrep) != EMPI_REQUEST_NULL) EMPI_Test(req->reqrep,&flag,&((req->status).status));
					else flag = 0;
					cdata->completerep = flag > 0;
					if(cdata->completerep) {
						bool alt_comms = false;
						if(getenv("ENABLE_ALT_COMMS")) {
							if(!strcmp(getenv("ENABLE_ALT_COMMS"),"1")) {
								alt_comms = true;
							}
						}
						if(alt_comms) {
						} else {
							if((cdata->type == MPI_FT_REDUCE) || (cdata->type == MPI_FT_ALLREDUCE)) {
								void *dest_recvbuf = start->req->bufloc;
								int size;
								int myrank;
								if(cdata->type == MPI_FT_REDUCE) {
									int extracount;
									int dis;
									EMPI_Type_size((cdata->args).reduce.dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									dis = (((cdata->args).reduce.count+extracount)*size) - sizeof(int);
									if((cdata->args).reduce.comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
										memcpy(dest_recvbuf,(cdata->args).reduce.recvbuf,(cdata->args).reduce.count * size);
										if((*((int *)((cdata->args).reduce.recvbuf + dis)) & 0xF0000000) == 0x70000000) {
											assert(*((int *)((cdata->args).reduce.recvbuf + dis)) == cdata->id);
										} else {
											cdata->completerep = false;
											*(req->reqrep) = EMPI_REQUEST_NULL;
										}
									}
								} else if (cdata->type == MPI_FT_ALLREDUCE) {
									int extracount;
									int dis;
									EMPI_Type_size((cdata->args).allreduce.dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									dis = (((cdata->args).allreduce.count+extracount)*size) - sizeof(int);
									if((cdata->args).allreduce.comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
										memcpy(dest_recvbuf,(cdata->args).allreduce.recvbuf,(cdata->args).allreduce.count * size);
										if((*((int *)((cdata->args).allreduce.recvbuf + dis)) & 0xF0000000) == 0x70000000) {
											assert(*((int *)((cdata->args).allreduce.recvbuf + dis)) == cdata->id);
										} else {
											cdata->completerep = false;
											*(req->reqrep) = EMPI_REQUEST_NULL;
										}
									}
								}
							}
						}
					}
				}
				for(int i = 0; i < cdata->num_colls; i++) {
					if(!(cdata->completecolls[i])) {
						int flag;
						EMPI_Test(req->reqcolls[i],&flag,&((req->status).status));
						cdata->completecolls[i] = flag > 0;
					}
				}
				req->complete = (cdata->completecmp) & (cdata->completerep);
				for(int i = 0; i < cdata->num_colls; i++) {
					req->complete = req->complete & cdata->completecolls[i];
				}
				if(req->complete) {
					bool alt_comms = false;
					if(getenv("ENABLE_ALT_COMMS")) {
						if(!strcmp(getenv("ENABLE_ALT_COMMS"),"1")) {
							alt_comms = true;
						}
					}
					if(alt_comms) {
						if(cdata->type == MPI_FT_ALLREDUCE) {
							pthread_rwlock_rdlock(&commLock);
							if((cdata->args).allreduce.comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
								if((cdata->args).allreduce.dt == MPI_INT) {
									if((cdata->args).allreduce.op == MPI_SUM) {
										int *dest_recvbuf = (int *)start->req->bufloc;
										int *src = (int *)(cdata->args).allreduce.recvbuf;
										for(int i = 0; i < (cdata->args).allreduce.count; i++) {
											dest_recvbuf[i] += src[i];
										}
									} else if((cdata->args).allreduce.op == MPI_MAX) {
										int *dest_recvbuf = (int *)start->req->bufloc;
										int *src = (int *)(cdata->args).allreduce.recvbuf;
										for(int i = 0; i < (cdata->args).allreduce.count; i++) {
											if(src[i] > dest_recvbuf[i]) {
												dest_recvbuf[i] = src[i];
											}
										}
									}
								} else if ((cdata->args).allreduce.dt == MPI_DOUBLE) {
									if((cdata->args).allreduce.op == MPI_SUM) {
										double *dest_recvbuf = (double *)start->req->bufloc;
										double *src = (double *)(cdata->args).allreduce.recvbuf;
										for(int i = 0; i < (cdata->args).allreduce.count; i++) {
											dest_recvbuf[i] += src[i];
										}
									} else if((cdata->args).allreduce.op == MPI_MAX) {
										double *dest_recvbuf = (double *)start->req->bufloc;
										double *src = (double *)(cdata->args).allreduce.recvbuf;
										for(int i = 0; i < (cdata->args).allreduce.count; i++) {
											if(src[i] > dest_recvbuf[i]) {
												dest_recvbuf[i] = src[i];
											}
										}
									}
								}
							}
							pthread_rwlock_unlock(&commLock);
						}
					}
					signal_completion = true;
				}
			} else if((start->req->type == MPI_FT_READ_REQUEST) || (start->req->type == MPI_FT_WRITE_REQUEST)) {
				MPI_Request req = start->req;
				int flag;
				if(*(req->reqcmp) != EMPI_REQUEST_NULL) {
					parep_mpi_empi_entered = true;
					EMPI_Test(req->reqcmp,&flag,&((req->status).status));
					parep_mpi_empi_entered = false;
					if(flag > 0) {
						if(req->iotype == MPI_FT_READ_SHARED_REQUEST) {
							if(req->comm->EMPI_COMM_CMP != EMPI_COMM_NULL) {
								if(*(req->reqrep) == EMPI_REQUEST_NULL) {
									int count;
									EMPI_Get_count(&((req->status).status),EMPI_BYTE,&count);
									(req->status).count = count;
									EMPI_Ibcast(req->storeloc, req->count, req->datatype->edatatype, 0, req->comm->pairComm, req->reqrep);
									*(req->reqcmp) = EMPI_REQUEST_NULL;
								} else {
									*(req->reqcmp) = EMPI_REQUEST_NULL;
									*(req->reqrep) = EMPI_REQUEST_NULL;
								}
							} else if(req->comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
								*(req->reqcmp) = EMPI_REQUEST_NULL;
							}
						} else if (req->iotype == MPI_FT_WRITE_SHARED_REQUEST) {
							if(req->comm->EMPI_COMM_CMP != EMPI_COMM_NULL) {
								int count;
								EMPI_Get_count(&((req->status).status),EMPI_BYTE,&count);
								(req->status).count = count;
								EMPI_Ibarrier(req->comm->pairComm, req->reqrep);
								*(req->reqcmp) = EMPI_REQUEST_NULL;
							}
						} else if(req->iotype == MPI_FT_READ_ALL_REQUEST) {
							int count;
							EMPI_Get_count(&((req->status).status),EMPI_BYTE,&count);
							(req->status).count = count;
							EMPI_Ibarrier(req->comm->pairComm, req->reqrep);
							*(req->reqcmp) = EMPI_REQUEST_NULL;
						} else if(req->iotype == MPI_FT_WRITE_ALL_REQUEST) {
							int count;
							EMPI_Get_count(&((req->status).status),EMPI_BYTE,&count);
							(req->status).count = count;
							//EMPI_Ibarrier(req->comm->pairComm, req->reqrep);
							*(req->reqcmp) = EMPI_REQUEST_NULL;
						} else {
							int count;
							EMPI_Get_count(&((req->status).status),EMPI_BYTE,&count);
							(req->status).count = count;
							*(req->reqcmp) = EMPI_REQUEST_NULL;
						}
					}
				}
				if(*(req->reqrep) != EMPI_REQUEST_NULL) {
					EMPI_Test(req->reqrep,&flag,&((req->status).status));
					if(flag > 0) {
						if(req->iotype == MPI_FT_READ_SHARED_REQUEST) {
							if(req->comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
								(req->status).count = req->count;
								*(req->reqrep) = EMPI_REQUEST_NULL;
							}
							EMPI_Ibarrier(req->comm->pairComm, req->reqcmp);
						} else if(req->iotype == MPI_FT_WRITE_SHARED_REQUEST) {
							if(req->comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
								(req->status).count = req->count;
							}
							*(req->reqrep) = EMPI_REQUEST_NULL;
						} else if(req->iotype == MPI_FT_READ_ALL_REQUEST) {
							*(req->reqrep) = EMPI_REQUEST_NULL;
						} else if(req->iotype == MPI_FT_WRITE_ALL_REQUEST) {
							*(req->reqrep) = EMPI_REQUEST_NULL;
							if(req->comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
								if(!(req->iotransfercomplete)) {
									int count;
									EMPI_Get_count(&((req->status).status),EMPI_BYTE,&count);
									req->iotransfercomplete = true;
									parep_mpi_empi_entered = true;
									EMPI_File_iwrite_all(((MPI_File)(req->bufloc))->repfh, req->cbuf, count, ((MPI_File)(req->bufloc))->real_view.etype->edatatype, req->reqrep);
									parep_mpi_empi_entered = false;
								} else {
									parep_mpi_free(req->cbuf);
								}
							}
						}
					}
				}
				if((*(req->reqcmp) == EMPI_REQUEST_NULL) && (*(req->reqrep) == EMPI_REQUEST_NULL)) {
					req->complete = true;
					signal_completion = true;
				}
			} else {
				if(!(((ptpdata *)(start->req->storeloc))->completecmp) && (*(start->req->reqcmp) != EMPI_REQUEST_NULL)) {
					int flag;
					EMPI_Test(start->req->reqcmp,&flag,&((start->req->status).status));
					(((ptpdata *)(start->req->storeloc))->completecmp) = flag > 0;
					if((((ptpdata *)(start->req->storeloc))->completecmp)) {
						if(start->req->type == MPI_FT_RECV_REQUEST) {
							if((start->req->comm->EMPI_COMM_CMP) != EMPI_COMM_NULL) {
								int size;
								int extracount;
								int count;
								EMPI_Type_size(((ptpdata *)(start->req->storeloc))->dt->edatatype,&size);
								if(size >= sizeof(int)) extracount = 1;
								else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
								else extracount = (((int)sizeof(int))/size) + 1;
								EMPI_Get_count(&((start->req->status).status),((ptpdata *)(start->req->storeloc))->dt->edatatype,&count);
								count -= extracount;
								memcpy(start->req->bufloc,((ptpdata *)(start->req->storeloc))->buf,count*size);
								memcpy(&(((ptpdata *)(start->req->storeloc))->id),((((ptpdata *)(start->req->storeloc))->buf) + (((count+extracount) * size) - sizeof(int))),sizeof(int));
								if(((((ptpdata *)(start->req->storeloc))->id) & 0xF0000000) == 0x70000000) {
									if(start->req->cbuf == NULL) parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
									else {
										return_commbuf_node(start->req->cbuf);
										((ptpdata *)(start->req->storeloc))->buf = NULL;
										start->req->cbuf = NULL;
									}
									
									(start->req->status).count = count;
									if((((start->req->status).status).EMPI_TAG) == EMPI_ANY_TAG) (start->req->status).MPI_TAG = MPI_ANY_TAG;
									else (start->req->status).MPI_TAG = ((start->req->status).status).EMPI_TAG;
									(start->req->status).MPI_ERROR = ((start->req->status).status).EMPI_ERROR;
									(start->req->status).MPI_SOURCE = ((start->req->status).status).EMPI_SOURCE;
									
									((ptpdata *)(start->req->storeloc))->count = count;
									((ptpdata *)(start->req->storeloc))->target = (start->req->status).MPI_SOURCE;
									((ptpdata *)(start->req->storeloc))->tag = (start->req->status).MPI_TAG;
									
									if(!(((ptpdata *)(start->req->storeloc))->completerep)) {
										int cmprank;
										EMPI_Comm_rank(start->req->comm->EMPI_COMM_CMP,&cmprank);
										((ptpdata *)(start->req->storeloc))->buf = parep_mpi_malloc(3*sizeof(int));
										((int *)(((ptpdata *)(start->req->storeloc))->buf))[0] = ((ptpdata *)(start->req->storeloc))->target;
										((int *)(((ptpdata *)(start->req->storeloc))->buf))[1] = ((ptpdata *)(start->req->storeloc))->tag;
										((int *)(((ptpdata *)(start->req->storeloc))->buf))[2] = ((ptpdata *)(start->req->storeloc))->id;
										EMPI_Isend(((ptpdata *)(start->req->storeloc))->buf, 3, EMPI_INT, cmpToRepMap[cmprank], MPI_FT_WILDCARD_TAG, start->req->comm->EMPI_CMP_REP_INTERCOMM, start->req->reqrep);
									}
								} else {
									(((ptpdata *)(start->req->storeloc))->completecmp) = false;
									*(start->req->reqcmp) = EMPI_REQUEST_NULL;
								}
								
							} else if((start->req->comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
								int target,tag,id;
								target = ((int *)(((ptpdata *)(start->req->storeloc))->buf))[0];
								tag = ((int *)(((ptpdata *)(start->req->storeloc))->buf))[1];
								id = ((int *)(((ptpdata *)(start->req->storeloc))->buf))[2];
								if((id & 0xF0000000) == 0x70000000) {
									((ptpdata *)(start->req->storeloc))->target = target;
									((ptpdata *)(start->req->storeloc))->tag = tag;
									((ptpdata *)(start->req->storeloc))->id = id;
									
									pthread_mutex_lock(&recvDataListLock);
									recvDataNode *rnode = recvDataListFindWithId(target,tag,id,start->req->comm);
									ptpdata *curargs;
									int size;
									EMPI_Type_size(((ptpdata *)(start->req->storeloc))->dt->edatatype,&size);
									if(rnode != NULL) {
										curargs = rnode->pdata;
										recvDataListDelete(rnode);
										pthread_cond_signal(&recvDataListCond);
										pthread_mutex_unlock(&recvDataListLock);
										
										int recvcount = curargs->count;
										recvcount = recvcount/size;
										
										memcpy(start->req->bufloc,curargs->buf,recvcount*size);
										start->req->status.count = recvcount;
										start->req->status.MPI_TAG = curargs->tag;
										start->req->status.MPI_SOURCE = curargs->target;
										start->req->status.MPI_ERROR = 0;
										
										parep_mpi_free(curargs->buf);
										curargs->count = recvcount;
										curargs->dt = ((ptpdata *)(start->req->storeloc))->dt;
										parep_mpi_free(curargs->req);
										curargs->req = &(start->req);
										parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
										struct peertopeer_data *deldata = ((ptpdata *)(start->req->storeloc));
										if(deldata == first_peertopeer) {
											first_peertopeer = first_peertopeer->prev;
										}
										if(deldata == last_peertopeer) {
											last_peertopeer = last_peertopeer->next;
										}
										if(deldata->next != NULL) deldata->next->prev = deldata->prev;
										if(deldata->prev != NULL) deldata->prev->next = deldata->next;
										parep_mpi_free(deldata);
										start->req->storeloc = (void *)curargs;
									} else {
										pthread_mutex_unlock(&recvDataListLock);
										int count = ((ptpdata *)(start->req->storeloc))->count;
										int size;
										int extracount;
										EMPI_Type_size(((ptpdata *)(start->req->storeloc))->dt->edatatype,&size);
										if(size >= sizeof(int)) extracount = 1;
										else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
										else extracount = (((int)sizeof(int))/size) + 1;
										if((count+extracount)*size < 3*sizeof(int)) {
											parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
											((ptpdata *)(start->req->storeloc))->buf = parep_mpi_malloc((count+extracount)*size);
										}
										if(cmpToRepMap[target] == -1) EMPI_Irecv (((ptpdata *)(start->req->storeloc))->buf, count+extracount, ((ptpdata *)(start->req->storeloc))->dt->edatatype, target, tag, (start->req->comm->EMPI_CMP_REP_INTERCOMM), (start->req->reqrep));
										else EMPI_Irecv (((ptpdata *)(start->req->storeloc))->buf, count+extracount, ((ptpdata *)(start->req->storeloc))->dt->edatatype, cmpToRepMap[target], tag, (start->req->comm->EMPI_COMM_REP), (start->req->reqrep));
									}
								} else {
									(((ptpdata *)(start->req->storeloc))->completecmp) = false;
									*(start->req->reqcmp) = EMPI_REQUEST_NULL;
								}
							}
						}
					}
				}
				if(!(((ptpdata *)(start->req->storeloc))->completerep) && (*(start->req->reqrep) != EMPI_REQUEST_NULL)) {
					int flag;
					EMPI_Test(start->req->reqrep,&flag,&((start->req->status).status));
					((ptpdata *)(start->req->storeloc))->completerep = flag > 0;
					if(((ptpdata *)(start->req->storeloc))->completerep) {
						if(start->req->type == MPI_FT_RECV_REQUEST) {
							if((start->req->comm->EMPI_COMM_CMP) != EMPI_COMM_NULL) {
								parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
							} else if((start->req->comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
								int size;
								int extracount;
								int count;
								EMPI_Type_size(((ptpdata *)(start->req->storeloc))->dt->edatatype,&size);
								if(size >= sizeof(int)) extracount = 1;
								else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
								else extracount = (((int)sizeof(int))/size) + 1;
								EMPI_Get_count(&((start->req->status).status),((ptpdata *)(start->req->storeloc))->dt->edatatype,&count);
								count -= extracount;
								memcpy(start->req->bufloc,((ptpdata *)(start->req->storeloc))->buf,count*size);
								memcpy(&(((ptpdata *)(start->req->storeloc))->id),((((ptpdata *)(start->req->storeloc))->buf) + (((count+extracount) * size) - sizeof(int))),sizeof(int));
								if(((((ptpdata *)(start->req->storeloc))->id) & 0xF0000000) == 0x70000000) {
									if(start->req->cbuf == NULL) parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
									else {
										return_commbuf_node(start->req->cbuf);
										((ptpdata *)(start->req->storeloc))->buf = NULL;
										start->req->cbuf = NULL;
									}
									
									(start->req->status).count = count;
									if((((start->req->status).status).EMPI_TAG) == EMPI_ANY_TAG) (start->req->status).MPI_TAG = MPI_ANY_TAG;
									else (start->req->status).MPI_TAG = ((start->req->status).status).EMPI_TAG;
									(start->req->status).MPI_ERROR = ((start->req->status).status).EMPI_ERROR;
									(start->req->status).MPI_SOURCE = ((start->req->status).status).EMPI_SOURCE;
									if(cmpToRepMap[((ptpdata *)(start->req->storeloc))->target] != -1) {
										(start->req->status).MPI_SOURCE = repToCmpMap[((start->req->status).status).EMPI_SOURCE];
									}
									
									((ptpdata *)(start->req->storeloc))->count = count;
									((ptpdata *)(start->req->storeloc))->target = (start->req->status).MPI_SOURCE;
									((ptpdata *)(start->req->storeloc))->tag = (start->req->status).MPI_TAG;
								} else {
									(((ptpdata *)(start->req->storeloc))->completerep) = false;
									*(start->req->reqrep) = EMPI_REQUEST_NULL;
								}
							}
						}
					}
				}
				start->req->complete = (((ptpdata *)(start->req->storeloc))->completecmp) & (((ptpdata *)(start->req->storeloc))->completerep);
				if(start->req->complete) {
					signal_completion = true;
				}
			}
		}/* else {
			signal_completion = true;
		}*/
		if(signal_completion) pthread_cond_signal(&reqListCond);
	}
}

bool test_all_requests() {
	bool progressed = false;
	bool signal_completion = false;
	for(reqNode *start = reqHead; start != NULL; start = start->next) {
		if(!(start->req->complete)) {
			if(start->req->type == MPI_FT_COLLECTIVE_REQUEST) {
				MPI_Request req = start->req;
				clcdata *cdata = (clcdata *)(req->storeloc);
				if(!(cdata->completecmp)) {
					int flag;
					EMPI_Test(req->reqcmp,&flag,&((req->status).status));
					cdata->completecmp = flag > 0;
					if(cdata->completecmp) {
						progressed = true;
						bool alt_comms = false;
						if(getenv("ENABLE_ALT_COMMS")) {
							if(!strcmp(getenv("ENABLE_ALT_COMMS"),"1")) {
								alt_comms = true;
							}
						}
						if(alt_comms) {
							if((cdata->type == MPI_FT_BCAST) || (cdata->type == MPI_FT_ALLTOALL)) {
								void *dest_recvbuf = start->req->bufloc;
								int size;
								int myrank;
								if(cdata->type == MPI_FT_BCAST) {
									EMPI_Comm_rank((cdata->args).bcast.comm->EMPI_COMM_CMP,&myrank);
									if(!(cdata->completerep)) EMPI_Isend((cdata->args).bcast.buf,(cdata->args).bcast.count,(cdata->args).bcast.dt->edatatype,cmpToRepMap[myrank],MPI_FT_REDUCE_TAG,(cdata->args).bcast.comm->EMPI_CMP_REP_INTERCOMM,req->reqrep);
								} else if(cdata->type == MPI_FT_ALLTOALL) {
									EMPI_Comm_rank((cdata->args).alltoall.comm->EMPI_COMM_CMP,&myrank);
									if(!(cdata->completerep)) EMPI_Isend (dest_recvbuf, (cdata->args).alltoall.recvcount, (cdata->args).alltoall.recvdt->edatatype, cmpToRepMap[myrank], MPI_FT_ALLREDUCE_TAG, (cdata->args).alltoall.comm->EMPI_CMP_REP_INTERCOMM, req->reqrep);
								}
							}
						} else {
							if((cdata->type == MPI_FT_REDUCE) || (cdata->type == MPI_FT_ALLREDUCE)) {
								void *dest_recvbuf = start->req->bufloc;
								int size;
								int myrank;
								if(cdata->type == MPI_FT_REDUCE) {
									int extracount;
									int dis;
									EMPI_Comm_rank((cdata->args).reduce.comm->EMPI_COMM_CMP,&myrank);
									EMPI_Type_size((cdata->args).reduce.dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									dis = (((cdata->args).reduce.count+extracount)*size) - sizeof(int);
									memcpy((cdata->args).reduce.recvbuf,dest_recvbuf,(cdata->args).reduce.count * size);
									assert(*((int *)((cdata->args).reduce.recvbuf + dis)) == cdata->id);
									if(!(cdata->completerep)) EMPI_Isend((cdata->args).reduce.recvbuf,(cdata->args).reduce.count+extracount,(cdata->args).reduce.dt->edatatype,cmpToRepMap[myrank],MPI_FT_REDUCE_TAG,(cdata->args).reduce.comm->EMPI_CMP_REP_INTERCOMM,req->reqrep);
								} else if (cdata->type == MPI_FT_ALLREDUCE) {
									int extracount;
									int dis;
									EMPI_Comm_rank((cdata->args).allreduce.comm->EMPI_COMM_CMP,&myrank);
									EMPI_Type_size((cdata->args).allreduce.dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									dis = (((cdata->args).allreduce.count+extracount)*size) - sizeof(int);
									memcpy((cdata->args).allreduce.recvbuf,dest_recvbuf,(cdata->args).allreduce.count * size);
									assert(*((int *)((cdata->args).allreduce.recvbuf + dis)) == cdata->id);
									if(!(cdata->completerep)) EMPI_Isend((cdata->args).allreduce.recvbuf,(cdata->args).allreduce.count+extracount,(cdata->args).allreduce.dt->edatatype,cmpToRepMap[myrank],MPI_FT_ALLREDUCE_TAG,(cdata->args).allreduce.comm->EMPI_CMP_REP_INTERCOMM,req->reqrep);
								}
							}
						}
					}
				}
				if(!(cdata->completerep)) {
					int flag;
					if(*(req->reqrep) != EMPI_REQUEST_NULL) EMPI_Test(req->reqrep,&flag,&((req->status).status));
					else flag = 0;
					cdata->completerep = flag > 0;
					if(cdata->completerep) {
						progressed = true;
						bool alt_comms = false;
						if(getenv("ENABLE_ALT_COMMS")) {
							if(!strcmp(getenv("ENABLE_ALT_COMMS"),"1")) {
								alt_comms = true;
							}
						}
						if(alt_comms) {
						} else {
							if((cdata->type == MPI_FT_REDUCE) || (cdata->type == MPI_FT_ALLREDUCE)) {
								void *dest_recvbuf = start->req->bufloc;
								int size;
								int myrank;
								if(cdata->type == MPI_FT_REDUCE) {
									int extracount;
									int dis;
									pthread_rwlock_rdlock(&commLock);
									EMPI_Type_size((cdata->args).reduce.dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									dis = (((cdata->args).reduce.count+extracount)*size) - sizeof(int);
									if((cdata->args).reduce.comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
										memcpy(dest_recvbuf,(cdata->args).reduce.recvbuf,(cdata->args).reduce.count * size);
										if((*((int *)((cdata->args).reduce.recvbuf + dis)) & 0xF0000000) == 0x70000000) {
											assert(*((int *)((cdata->args).reduce.recvbuf + dis)) == cdata->id);
										} else {
											cdata->completerep = false;
											*(req->reqrep) = EMPI_REQUEST_NULL;
										}
									}
									pthread_rwlock_unlock(&commLock);
								} else if (cdata->type == MPI_FT_ALLREDUCE) {
									int extracount;
									int dis;
									pthread_rwlock_rdlock(&commLock);
									EMPI_Type_size((cdata->args).allreduce.dt->edatatype,&size);
									if(size >= sizeof(int)) extracount = 1;
									else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
									else extracount = (((int)sizeof(int))/size) + 1;
									dis = (((cdata->args).allreduce.count+extracount)*size) - sizeof(int);
									if((cdata->args).allreduce.comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
										memcpy(dest_recvbuf,(cdata->args).allreduce.recvbuf,(cdata->args).allreduce.count * size);
										if((*((int *)((cdata->args).allreduce.recvbuf + dis)) & 0xF0000000) == 0x70000000) {
											assert(*((int *)((cdata->args).allreduce.recvbuf + dis)) == cdata->id);
										} else {
											cdata->completerep = false;
											*(req->reqrep) = EMPI_REQUEST_NULL;
										}
									}
									pthread_rwlock_unlock(&commLock);
								}
							}
						}
					}
				}
				for(int i = 0; i < cdata->num_colls; i++) {
					if(!(cdata->completecolls[i])) {
						int flag;
						EMPI_Test(req->reqcolls[i],&flag,&((req->status).status));
						cdata->completecolls[i] = flag > 0;
					}
				}
				req->complete = (cdata->completecmp) & (cdata->completerep);
				for(int i = 0; i < cdata->num_colls; i++) {
					req->complete = req->complete & cdata->completecolls[i];
				}
				if(req->complete) {
					bool alt_comms = false;
					if(getenv("ENABLE_ALT_COMMS")) {
						if(!strcmp(getenv("ENABLE_ALT_COMMS"),"1")) {
							alt_comms = true;
						}
					}
					if(alt_comms) {
						if(cdata->type == MPI_FT_ALLREDUCE) {
							pthread_rwlock_rdlock(&commLock);
							if((cdata->args).allreduce.comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
								if((cdata->args).allreduce.dt == MPI_INT) {
									if((cdata->args).allreduce.op == MPI_SUM) {
										int *dest_recvbuf = (int *)start->req->bufloc;
										int *src = (int *)(cdata->args).allreduce.recvbuf;
										for(int i = 0; i < (cdata->args).allreduce.count; i++) {
											dest_recvbuf[i] += src[i];
										}
									} else if((cdata->args).allreduce.op == MPI_MAX) {
										int *dest_recvbuf = (int *)start->req->bufloc;
										int *src = (int *)(cdata->args).allreduce.recvbuf;
										for(int i = 0; i < (cdata->args).allreduce.count; i++) {
											if(src[i] > dest_recvbuf[i]) {
												dest_recvbuf[i] = src[i];
											}
										}
									}
								} else if ((cdata->args).allreduce.dt == MPI_DOUBLE) {
									if((cdata->args).allreduce.op == MPI_SUM) {
										double *dest_recvbuf = (double *)start->req->bufloc;
										double *src = (double *)(cdata->args).allreduce.recvbuf;
										for(int i = 0; i < (cdata->args).allreduce.count; i++) {
											dest_recvbuf[i] += src[i];
										}
									} else if((cdata->args).allreduce.op == MPI_MAX) {
										double *dest_recvbuf = (double *)start->req->bufloc;
										double *src = (double *)(cdata->args).allreduce.recvbuf;
										for(int i = 0; i < (cdata->args).allreduce.count; i++) {
											if(src[i] > dest_recvbuf[i]) {
												dest_recvbuf[i] = src[i];
											}
										}
									}
								}
							}
							pthread_rwlock_unlock(&commLock);
						}
					}
					progressed = true;
					signal_completion = true;
				}
			} else if((start->req->type == MPI_FT_READ_REQUEST) || (start->req->type == MPI_FT_WRITE_REQUEST)) {
				MPI_Request req = start->req;
				int flag;
				if(*(req->reqcmp) != EMPI_REQUEST_NULL) {
					parep_mpi_empi_entered = true;
					EMPI_Test(req->reqcmp,&flag,&((req->status).status));
					parep_mpi_empi_entered = false;
					if(flag > 0) {
						if(req->iotype == MPI_FT_READ_SHARED_REQUEST) {
							pthread_rwlock_rdlock(&commLock);
							if(req->comm->EMPI_COMM_CMP != EMPI_COMM_NULL) {
								if(*(req->reqrep) == EMPI_REQUEST_NULL) {
									int count;
									EMPI_Get_count(&((req->status).status),EMPI_BYTE,&count);
									(req->status).count = count;
									EMPI_Ibcast(req->storeloc, req->count, req->datatype->edatatype, 0, req->comm->pairComm, req->reqrep);
									*(req->reqcmp) = EMPI_REQUEST_NULL;
								} else {
									*(req->reqcmp) = EMPI_REQUEST_NULL;
									*(req->reqrep) = EMPI_REQUEST_NULL;
								}
							} else if(req->comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
								*(req->reqcmp) = EMPI_REQUEST_NULL;
							}
							pthread_rwlock_unlock(&commLock);
						} else if (req->iotype == MPI_FT_WRITE_SHARED_REQUEST) {
							pthread_rwlock_rdlock(&commLock);
							if(req->comm->EMPI_COMM_CMP != EMPI_COMM_NULL) {
								int count;
								EMPI_Get_count(&((req->status).status),EMPI_BYTE,&count);
								(req->status).count = count;
								EMPI_Ibarrier(req->comm->pairComm, req->reqrep);
								*(req->reqcmp) = EMPI_REQUEST_NULL;
							}
							pthread_rwlock_unlock(&commLock);
						} else if(req->iotype == MPI_FT_READ_ALL_REQUEST) {
							int count;
							EMPI_Get_count(&((req->status).status),EMPI_BYTE,&count);
							(req->status).count = count;
							EMPI_Ibarrier(req->comm->pairComm, req->reqrep);
							*(req->reqcmp) = EMPI_REQUEST_NULL;
						} else if(req->iotype == MPI_FT_WRITE_ALL_REQUEST) {
							int count;
							EMPI_Get_count(&((req->status).status),EMPI_BYTE,&count);
							(req->status).count = count;
							//EMPI_Ibarrier(req->comm->pairComm, req->reqrep);
							*(req->reqcmp) = EMPI_REQUEST_NULL;
						} else {
							int count;
							EMPI_Get_count(&((req->status).status),EMPI_BYTE,&count);
							(req->status).count = count;
							*(req->reqcmp) = EMPI_REQUEST_NULL;
						}
					}
				}
				if(*(req->reqrep) != EMPI_REQUEST_NULL) {
					EMPI_Test(req->reqrep,&flag,&((req->status).status));
					if(flag > 0) {
						if(req->iotype == MPI_FT_READ_SHARED_REQUEST) {
							pthread_rwlock_rdlock(&commLock);
							if(req->comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
								(req->status).count = req->count;
								*(req->reqrep) = EMPI_REQUEST_NULL;
							}
							EMPI_Ibarrier(req->comm->pairComm, req->reqcmp);
							pthread_rwlock_unlock(&commLock);
						} else if(req->iotype == MPI_FT_WRITE_SHARED_REQUEST) {
							pthread_rwlock_rdlock(&commLock);
							if(req->comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
								(req->status).count = req->count;
							}
							pthread_rwlock_unlock(&commLock);
							*(req->reqrep) = EMPI_REQUEST_NULL;
						} else if(req->iotype == MPI_FT_READ_ALL_REQUEST) {
							*(req->reqrep) = EMPI_REQUEST_NULL;
						} else if(req->iotype == MPI_FT_WRITE_ALL_REQUEST) {
							*(req->reqrep) = EMPI_REQUEST_NULL;
							pthread_rwlock_rdlock(&commLock);
							if(req->comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
								if(!(req->iotransfercomplete)) {
									int count;
									EMPI_Get_count(&((req->status).status),EMPI_BYTE,&count);
									req->iotransfercomplete = true;
									parep_mpi_empi_entered = true;
									EMPI_File_iwrite_all(((MPI_File)(req->bufloc))->repfh, req->cbuf, count, ((MPI_File)(req->bufloc))->real_view.etype->edatatype, req->reqrep);
									parep_mpi_empi_entered = false;
								} else {
									parep_mpi_free(req->cbuf);
								}
							}
							pthread_rwlock_unlock(&commLock);
						}
					}
				}
				if((*(req->reqcmp) == EMPI_REQUEST_NULL) && (*(req->reqrep) == EMPI_REQUEST_NULL)) {
					req->complete = true;
					signal_completion = true;
				}
			} else {
				if(!(((ptpdata *)(start->req->storeloc))->completecmp) && (*(start->req->reqcmp) != EMPI_REQUEST_NULL)) {
					int flag;
					EMPI_Test(start->req->reqcmp,&flag,&((start->req->status).status));
					(((ptpdata *)(start->req->storeloc))->completecmp) = flag > 0;
					if((((ptpdata *)(start->req->storeloc))->completecmp)) {
						progressed = true;
						if(start->req->type == MPI_FT_RECV_REQUEST) {
							pthread_rwlock_rdlock(&commLock);
							if((start->req->comm->EMPI_COMM_CMP) != EMPI_COMM_NULL) {
								int size;
								int extracount;
								int count;
								EMPI_Type_size(((ptpdata *)(start->req->storeloc))->dt->edatatype,&size);
								if(size >= sizeof(int)) extracount = 1;
								else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
								else extracount = (((int)sizeof(int))/size) + 1;
								EMPI_Get_count(&((start->req->status).status),((ptpdata *)(start->req->storeloc))->dt->edatatype,&count);
								count -= extracount;
								memcpy(start->req->bufloc,((ptpdata *)(start->req->storeloc))->buf,count*size);
								memcpy(&(((ptpdata *)(start->req->storeloc))->id),((((ptpdata *)(start->req->storeloc))->buf) + (((count+extracount) * size) - sizeof(int))),sizeof(int));
								if(((((ptpdata *)(start->req->storeloc))->id) & 0xF0000000) == 0x70000000) {
									if(start->req->cbuf == NULL) parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
									else {
										return_commbuf_node(start->req->cbuf);
										((ptpdata *)(start->req->storeloc))->buf = NULL;
										start->req->cbuf = NULL;
									}
									
									(start->req->status).count = count;
									if((((start->req->status).status).EMPI_TAG) == EMPI_ANY_TAG) (start->req->status).MPI_TAG = MPI_ANY_TAG;
									else (start->req->status).MPI_TAG = ((start->req->status).status).EMPI_TAG;
									(start->req->status).MPI_ERROR = ((start->req->status).status).EMPI_ERROR;
									(start->req->status).MPI_SOURCE = ((start->req->status).status).EMPI_SOURCE;
									
									pthread_mutex_lock(&peertopeerLock);
									((ptpdata *)(start->req->storeloc))->count = count;
									((ptpdata *)(start->req->storeloc))->target = (start->req->status).MPI_SOURCE;
									((ptpdata *)(start->req->storeloc))->tag = (start->req->status).MPI_TAG;
									pthread_mutex_unlock(&peertopeerLock);
									
									if(!(((ptpdata *)(start->req->storeloc))->completerep)) {
										int cmprank;
										EMPI_Comm_rank(start->req->comm->EMPI_COMM_CMP,&cmprank);
										((ptpdata *)(start->req->storeloc))->buf = parep_mpi_malloc(3*sizeof(int));
										((int *)(((ptpdata *)(start->req->storeloc))->buf))[0] = ((ptpdata *)(start->req->storeloc))->target;
										((int *)(((ptpdata *)(start->req->storeloc))->buf))[1] = ((ptpdata *)(start->req->storeloc))->tag;
										((int *)(((ptpdata *)(start->req->storeloc))->buf))[2] = ((ptpdata *)(start->req->storeloc))->id;
										EMPI_Isend(((ptpdata *)(start->req->storeloc))->buf, 3, EMPI_INT, cmpToRepMap[cmprank], MPI_FT_WILDCARD_TAG, start->req->comm->EMPI_CMP_REP_INTERCOMM, start->req->reqrep);
									}
								} else {
									(((ptpdata *)(start->req->storeloc))->completecmp) = false;
									*(start->req->reqcmp) = EMPI_REQUEST_NULL;
								}
								
							} else if((start->req->comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
								int target,tag,id;
								target = ((int *)(((ptpdata *)(start->req->storeloc))->buf))[0];
								tag = ((int *)(((ptpdata *)(start->req->storeloc))->buf))[1];
								id = ((int *)(((ptpdata *)(start->req->storeloc))->buf))[2];
								if((id & 0xF0000000) == 0x70000000) {
									((ptpdata *)(start->req->storeloc))->target = target;
									((ptpdata *)(start->req->storeloc))->tag = tag;
									((ptpdata *)(start->req->storeloc))->id = id;
									
									pthread_mutex_lock(&recvDataListLock);
									recvDataNode *rnode = recvDataListFindWithId(target,tag,id,start->req->comm);
									ptpdata *curargs;
									int size;
									EMPI_Type_size(((ptpdata *)(start->req->storeloc))->dt->edatatype,&size);
									if(rnode != NULL) {
										curargs = rnode->pdata;
										recvDataListDelete(rnode);
										pthread_cond_signal(&recvDataListCond);
										pthread_mutex_unlock(&recvDataListLock);
										
										int recvcount = curargs->count;
										recvcount = recvcount/size;
										
										memcpy(start->req->bufloc,curargs->buf,recvcount*size);
										start->req->status.count = recvcount;
										start->req->status.MPI_TAG = curargs->tag;
										start->req->status.MPI_SOURCE = curargs->target;
										start->req->status.MPI_ERROR = 0;
										
										pthread_mutex_lock(&peertopeerLock);
										parep_mpi_free(curargs->buf);
										curargs->count = recvcount;
										curargs->dt = ((ptpdata *)(start->req->storeloc))->dt;
										parep_mpi_free(curargs->req);
										curargs->req = &(start->req);
										parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
										struct peertopeer_data *deldata = ((ptpdata *)(start->req->storeloc));
										if(deldata == first_peertopeer) {
											first_peertopeer = first_peertopeer->prev;
										}
										if(deldata == last_peertopeer) {
											last_peertopeer = last_peertopeer->next;
										}
										if(deldata->next != NULL) deldata->next->prev = deldata->prev;
										if(deldata->prev != NULL) deldata->prev->next = deldata->next;
										parep_mpi_free(deldata);
										start->req->storeloc = (void *)curargs;
										pthread_mutex_unlock(&peertopeerLock);
									} else {
										pthread_mutex_unlock(&recvDataListLock);
										int count = ((ptpdata *)(start->req->storeloc))->count;
										int size;
										int extracount;
										EMPI_Type_size(((ptpdata *)(start->req->storeloc))->dt->edatatype,&size);
										if(size >= sizeof(int)) extracount = 1;
										else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
										else extracount = (((int)sizeof(int))/size) + 1;
										if((count+extracount)*size < 3*sizeof(int)) {
											parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
											((ptpdata *)(start->req->storeloc))->buf = parep_mpi_malloc((count+extracount)*size);
										}
										if(cmpToRepMap[target] == -1) EMPI_Irecv (((ptpdata *)(start->req->storeloc))->buf, count+extracount, ((ptpdata *)(start->req->storeloc))->dt->edatatype, target, tag, (start->req->comm->EMPI_CMP_REP_INTERCOMM), (start->req->reqrep));
										else EMPI_Irecv (((ptpdata *)(start->req->storeloc))->buf, count+extracount, ((ptpdata *)(start->req->storeloc))->dt->edatatype, cmpToRepMap[target], tag, (start->req->comm->EMPI_COMM_REP), (start->req->reqrep));
									}
								} else {
									(((ptpdata *)(start->req->storeloc))->completecmp) = false;
									*(start->req->reqcmp) = EMPI_REQUEST_NULL;
								}
							}
							pthread_rwlock_unlock(&commLock);
						}
					}
				}
				if(!(((ptpdata *)(start->req->storeloc))->completerep) && (*(start->req->reqrep) != EMPI_REQUEST_NULL)) {
					int flag;
					EMPI_Test(start->req->reqrep,&flag,&((start->req->status).status));
					((ptpdata *)(start->req->storeloc))->completerep = flag > 0;
					if(((ptpdata *)(start->req->storeloc))->completerep) {
						progressed = true;
						if(start->req->type == MPI_FT_RECV_REQUEST) {
							pthread_rwlock_rdlock(&commLock);
							if((start->req->comm->EMPI_COMM_CMP) != EMPI_COMM_NULL) {
								parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
							} else if((start->req->comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
								int size;
								int extracount;
								int count;
								EMPI_Type_size(((ptpdata *)(start->req->storeloc))->dt->edatatype,&size);
								if(size >= sizeof(int)) extracount = 1;
								else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
								else extracount = (((int)sizeof(int))/size) + 1;
								EMPI_Get_count(&((start->req->status).status),((ptpdata *)(start->req->storeloc))->dt->edatatype,&count);
								count -= extracount;
								memcpy(start->req->bufloc,((ptpdata *)(start->req->storeloc))->buf,count*size);
								memcpy(&(((ptpdata *)(start->req->storeloc))->id),((((ptpdata *)(start->req->storeloc))->buf) + (((count+extracount) * size) - sizeof(int))),sizeof(int));
								if(((((ptpdata *)(start->req->storeloc))->id) & 0xF0000000) == 0x70000000) {
									if(start->req->cbuf == NULL) parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
									else {
										return_commbuf_node(start->req->cbuf);
										((ptpdata *)(start->req->storeloc))->buf = NULL;
										start->req->cbuf = NULL;
									}
									
									(start->req->status).count = count;
									if((((start->req->status).status).EMPI_TAG) == EMPI_ANY_TAG) (start->req->status).MPI_TAG = MPI_ANY_TAG;
									else (start->req->status).MPI_TAG = ((start->req->status).status).EMPI_TAG;
									(start->req->status).MPI_ERROR = ((start->req->status).status).EMPI_ERROR;
									(start->req->status).MPI_SOURCE = ((start->req->status).status).EMPI_SOURCE;
									if(cmpToRepMap[((ptpdata *)(start->req->storeloc))->target] != -1) {
										(start->req->status).MPI_SOURCE = repToCmpMap[((start->req->status).status).EMPI_SOURCE];
									}
									
									pthread_mutex_lock(&peertopeerLock);
									((ptpdata *)(start->req->storeloc))->count = count;
									((ptpdata *)(start->req->storeloc))->target = (start->req->status).MPI_SOURCE;
									((ptpdata *)(start->req->storeloc))->tag = (start->req->status).MPI_TAG;
									pthread_mutex_unlock(&peertopeerLock);
								} else {
									(((ptpdata *)(start->req->storeloc))->completerep) = false;
									*(start->req->reqrep) = EMPI_REQUEST_NULL;
								}
							}
							pthread_rwlock_unlock(&commLock);
						}
					}
				}
				start->req->complete = (((ptpdata *)(start->req->storeloc))->completecmp) & (((ptpdata *)(start->req->storeloc))->completerep);
				if(start->req->complete) {
					bool alt_comms = false;
					if(getenv("ENABLE_ALT_COMMS")) {
						if(!strcmp(getenv("ENABLE_ALT_COMMS"),"1")) {
							alt_comms = true;
						}
					}
					if(alt_comms) {
						if(start->req->type == MPI_FT_RECV_REQUEST) {
							parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
						}
					}
					progressed = true;
					signal_completion = true;
				}
			}
		}/* else {
			signal_completion = true;
		}*/
		if(signal_completion) pthread_cond_signal(&reqListCond);
	}
	return progressed;
}

int probe_msg_from_source(MPI_Comm comm, int src) {
	int myrank;
	bool progressed;
	EMPI_Comm_rank(comm->eworldComm,&myrank);
	if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL) {
		if(src != myrank) {
			int flag = 0;
			EMPI_Status stat;
			do {
				MPI_Comm comm = MPI_COMM_WORLD;
				EMPI_Iprobe(src,EMPI_ANY_TAG,comm->EMPI_COMM_CMP,&flag,&stat);
				if(flag) {
					pthread_rwlock_unlock(&commLock);
					do {
						progressed = test_all_ptp_requests();
					} while(progressed);
					pthread_rwlock_rdlock(&commLock);
					EMPI_Iprobe(src,EMPI_ANY_TAG,comm->EMPI_COMM_CMP,&flag,&stat);
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
					
					EMPI_Recv(curargs->buf,count+extracount,EMPI_BYTE,src,stat.EMPI_TAG,comm->EMPI_COMM_CMP,&stat);
								
					memcpy(&(curargs->id),(curargs->buf) + (count * size),sizeof(int));
					if((curargs->id & 0xF0000000) != 0x70000000) {
						printf("%d: Wrong id probed %p myrank %d src %d\n",getpid(),curargs->id,myrank,src);
						parep_mpi_free(curargs->buf);
						parep_mpi_free(curargs);
					} else {
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
						pthread_cond_signal(&recvDataListCond);
						pthread_mutex_unlock(&recvDataListLock);
					}
				}
			} while(flag != 0);
		}
	} else if(comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
		if((src != myrank) && (src != repToCmpMap[myrank-nC])) {
			int flag = 0;
			EMPI_Status stat;
			do {
				if(src < nC) EMPI_Iprobe(src,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
				else EMPI_Iprobe(src-nC,EMPI_ANY_TAG,comm->EMPI_COMM_REP,&flag,&stat);
				if(flag) {
					pthread_rwlock_unlock(&commLock);
					do {
						progressed = test_all_ptp_requests();
					} while(progressed);
					pthread_rwlock_rdlock(&commLock);
					if(src < nC) EMPI_Iprobe(src,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
					else EMPI_Iprobe(src-nC,EMPI_ANY_TAG,comm->EMPI_COMM_REP,&flag,&stat);
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
					
					if(src < nC) EMPI_Recv(curargs->buf,count+extracount,EMPI_BYTE,src,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
					else EMPI_Recv(curargs->buf,count+extracount,EMPI_BYTE,src-nC,stat.EMPI_TAG,comm->EMPI_COMM_REP,&stat);
								
					memcpy(&(curargs->id),(curargs->buf) + (count * size),sizeof(int));
					if((curargs->id & 0xF0000000) != 0x70000000) {
						printf("%d: Wrong id probed %p myrank %d src %d\n",getpid(),curargs->id,myrank,src);
						parep_mpi_free(curargs->buf);
						parep_mpi_free(curargs);
					} else {
						MPI_Status status;
						memcpy(&(status.status),&(stat),sizeof(EMPI_Status));
						status.count = count;
						if(stat.EMPI_TAG == EMPI_ANY_TAG) status.MPI_TAG = MPI_ANY_TAG;
						else status.MPI_TAG = stat.EMPI_TAG;
						status.MPI_ERROR = stat.EMPI_ERROR;
						status.MPI_SOURCE = stat.EMPI_SOURCE;
						if(src >= nC) status.MPI_SOURCE = repToCmpMap[stat.EMPI_SOURCE];
									
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
						pthread_cond_signal(&recvDataListCond);
						pthread_mutex_unlock(&recvDataListLock);
					}
				}
			} while(flag != 0);
		}
	}
	return 0;
}

void probe_reduce_messages() {
	if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
		MPI_Comm comm = MPI_COMM_WORLD;
		int reprank,cmprank;
		bool progressed;
		do {
			progressed = test_all_requests();
		} while(progressed);
		int flag = 0;
		EMPI_Status stat;
		pthread_rwlock_rdlock(&commLock);
		EMPI_Comm_rank(comm->EMPI_COMM_REP,&reprank);
		cmprank = repToCmpMap[reprank];
		do {
			EMPI_Iprobe(cmprank,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
			if(flag) {
				pthread_rwlock_unlock(&commLock);
				do {
					progressed = test_all_requests();
				} while(progressed);
				pthread_rwlock_rdlock(&commLock);
				EMPI_Iprobe(cmprank,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
				if(flag == 0) {
					flag = 1;
					continue;
				}
				int count;
				int extracount;
				int size;
				int tag = stat.EMPI_TAG;
				assert((tag == MPI_FT_REDUCE_TAG) || (tag == MPI_FT_ALLREDUCE_TAG) || (tag == MPI_FT_WILDCARD_TAG));
				if(tag == MPI_FT_WILDCARD_TAG) {
					ptpdata *curargs;
					curargs = (ptpdata *)parep_mpi_malloc(sizeof(ptpdata));
					EMPI_Get_count(&stat,EMPI_INT,&count);
					assert(count == 3);
					int *tempbuf = (int *)parep_mpi_malloc(3*sizeof(int));
					EMPI_Recv(tempbuf,count,EMPI_INT,cmprank,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
					curargs->target = tempbuf[0];
					curargs->tag = tempbuf[1];
					curargs->id = tempbuf[2];
					curargs->type = MPI_FT_WILDCARD_RECV;
					parep_mpi_free(tempbuf);
					curargs->comm = comm;
					if((curargs->id & 0xF0000000) != 0x70000000) {
						parep_mpi_free(curargs);
					} else {
						pthread_mutex_lock(&peertopeerLock);
						if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
						else first_peertopeer = curargs;
						curargs->prev = NULL;
						curargs->next = last_peertopeer;
						last_peertopeer = curargs;
						pthread_cond_signal(&peertopeerCond);
						pthread_mutex_unlock(&peertopeerLock);
						
						pthread_mutex_lock(&recvDataListLock);
						recvDataListInsert((ptpdata *)curargs);
						pthread_cond_signal(&recvDataListCond);
						pthread_mutex_unlock(&recvDataListLock);
					}
				} else {
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
						EMPI_Recv((curargs->args).reduce.recvbuf,count+extracount,EMPI_BYTE,cmprank,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
						memcpy(&(curargs->id),((curargs->args).reduce.recvbuf) + (count * size),sizeof(int));
						if((curargs->id & 0xF0000000) != 0x70000000) {
							parep_mpi_free((curargs->args).reduce.recvbuf);
						}
						(curargs->args).reduce.comm = comm;
					} else if(tag == MPI_FT_ALLREDUCE_TAG) {
						curargs->type = MPI_FT_ALLREDUCE_TEMP;
						(curargs->args).allreduce.alloc_recvbuf = true;
						(curargs->args).allreduce.recvbuf = parep_mpi_malloc((count+extracount)*size);
						EMPI_Recv((curargs->args).allreduce.recvbuf,count+extracount,EMPI_BYTE,cmprank,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
						memcpy(&(curargs->id),((curargs->args).allreduce.recvbuf) + (count * size),sizeof(int));
						if((curargs->id & 0xF0000000) != 0x70000000) {
							parep_mpi_free((curargs->args).allreduce.recvbuf);
						}
						(curargs->args).allreduce.comm = comm;
					}
					
					if((curargs->id & 0xF0000000) != 0x70000000) {
						parep_mpi_free(curargs->req);
						parep_mpi_free(curargs);
					} else {
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
						pthread_cond_signal(&recvDataListCond);
						pthread_mutex_unlock(&recvDataListLock);
					}
				}
			}
		} while(flag != 0);
		pthread_rwlock_unlock(&commLock);
	}
}

void probe_reduce_messages_with_comm(MPI_Comm comm) {
	if(comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
		int reprank,cmprank;
		bool progressed;
		pthread_rwlock_unlock(&commLock);
		do {
			progressed = test_all_coll_requests();
		} while(progressed);
		pthread_rwlock_rdlock(&commLock);
		int flag = 0;
		EMPI_Status stat;
		EMPI_Comm_rank(comm->EMPI_COMM_REP,&reprank);
		cmprank = repToCmpMap[reprank];
		do {
			EMPI_Iprobe(cmprank,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
			if(flag) {
				pthread_rwlock_unlock(&commLock);
				do {
					progressed = test_all_coll_requests();
				} while(progressed);
				pthread_rwlock_rdlock(&commLock);
				EMPI_Iprobe(cmprank,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
				if(flag == 0) {
					flag = 1;
					continue;
				}
				int count;
				int extracount;
				int size;
				int tag = stat.EMPI_TAG;
				assert((tag == MPI_FT_REDUCE_TAG) || (tag == MPI_FT_ALLREDUCE_TAG) || (tag == MPI_FT_WILDCARD_TAG));
				if(tag == MPI_FT_WILDCARD_TAG) {
					ptpdata *curargs;
					curargs = (ptpdata *)parep_mpi_malloc(sizeof(ptpdata));
					EMPI_Get_count(&stat,EMPI_INT,&count);
					assert(count == 3);
					int *tempbuf = (int *)parep_mpi_malloc(3*sizeof(int));
					EMPI_Recv(tempbuf,count,EMPI_INT,cmprank,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
					curargs->target = tempbuf[0];
					curargs->tag = tempbuf[1];
					curargs->id = tempbuf[2];
					curargs->type = MPI_FT_WILDCARD_RECV;
					parep_mpi_free(tempbuf);
					curargs->comm = comm;
					if((curargs->id & 0xF0000000) != 0x70000000) {
						parep_mpi_free(curargs);
					} else {
						pthread_mutex_lock(&peertopeerLock);
						if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
						else first_peertopeer = curargs;
						curargs->prev = NULL;
						curargs->next = last_peertopeer;
						last_peertopeer = curargs;
						pthread_cond_signal(&peertopeerCond);
						pthread_mutex_unlock(&peertopeerLock);
						
						pthread_mutex_lock(&recvDataListLock);
						recvDataListInsert((ptpdata *)curargs);
						pthread_cond_signal(&recvDataListCond);
						pthread_mutex_unlock(&recvDataListLock);
					}
				} else {
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
						EMPI_Recv((curargs->args).reduce.recvbuf,count+extracount,EMPI_BYTE,cmprank,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
						memcpy(&(curargs->id),((curargs->args).reduce.recvbuf) + (count * size),sizeof(int));
						if((curargs->id & 0xF0000000) != 0x70000000) {
							printf("%d: Wrong reduce id probed %p reprank %d cmprank %d\n",getpid(),curargs->id,reprank,cmprank);
							parep_mpi_free((curargs->args).reduce.recvbuf);
						}
						(curargs->args).reduce.comm = comm;
					} else if(tag == MPI_FT_ALLREDUCE_TAG) {
						curargs->type = MPI_FT_ALLREDUCE_TEMP;
						(curargs->args).allreduce.alloc_recvbuf = true;
						(curargs->args).allreduce.recvbuf = parep_mpi_malloc((count+extracount)*size);
						EMPI_Recv((curargs->args).allreduce.recvbuf,count+extracount,EMPI_BYTE,cmprank,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
						memcpy(&(curargs->id),((curargs->args).allreduce.recvbuf) + (count * size),sizeof(int));
						if((curargs->id & 0xF0000000) != 0x70000000) {
							printf("%d: Wrong allreduce id probed %p reprank %d cmprank %d\n",getpid(),curargs->id,reprank,cmprank);
							parep_mpi_free((curargs->args).allreduce.recvbuf);
						}
						(curargs->args).allreduce.comm = comm;
					}
					
					if((curargs->id & 0xF0000000) != 0x70000000) {
						parep_mpi_free(curargs->req);
						parep_mpi_free(curargs);
					} else {
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
						pthread_cond_signal(&recvDataListCond);
						pthread_mutex_unlock(&recvDataListLock);
					}
				}
			}
		} while(flag != 0);
	}
}

void *handling_requests(void *arg) {
	if(_real_pthread_create == NULL) _real_pthread_create = dlsym(RTLD_NEXT,"pthread_create");
	if(_real_malloc == NULL) _real_malloc = dlsym(RTLD_NEXT,"malloc");
	while(1) {
		pthread_mutex_lock(&reqListLock);
		while(reqListIsEmpty()) {
			pthread_cond_wait(&reqListCond,&reqListLock);
		}
		
		test_all_requests();
		
		pthread_mutex_unlock(&reqListLock);
	}
}

