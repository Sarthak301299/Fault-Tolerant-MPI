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
				if(!(cdata->completerep)) {
					int flag;
					if(*(req->reqrep) != EMPI_REQUEST_NULL) EMPI_Test(req->reqrep,&flag,&((req->status).status));
					else flag = 0;
					cdata->completerep = flag > 0;
					if(cdata->completerep) {
						progressed = true;
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
			if(!(start->req->type == MPI_FT_COLLECTIVE_REQUEST)) {
				if(!(((ptpdata *)(start->req->storeloc))->completecmp) && (*(start->req->reqcmp) != EMPI_REQUEST_NULL)) {
					int flag;
					EMPI_Test(start->req->reqcmp,&flag,&((start->req->status).status));
					(((ptpdata *)(start->req->storeloc))->completecmp) = flag > 0;
					if((((ptpdata *)(start->req->storeloc))->completecmp)) {
						progressed = true;
						if(start->req->type == MPI_FT_RECV_REQUEST) {
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
								parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
								
								(start->req->status).count = count;
								if((((start->req->status).status).EMPI_TAG) == EMPI_ANY_TAG) (start->req->status).MPI_TAG = MPI_ANY_TAG;
								else (start->req->status).MPI_TAG = ((start->req->status).status).EMPI_TAG;
								(start->req->status).MPI_ERROR = ((start->req->status).status).EMPI_ERROR;
								(start->req->status).MPI_SOURCE = ((start->req->status).status).EMPI_SOURCE;
								pthread_rwlock_rdlock(&commLock);
								assert((start->req->comm->EMPI_COMM_CMP) != EMPI_COMM_NULL);
								pthread_rwlock_unlock(&commLock);
								
								pthread_mutex_lock(&peertopeerLock);
								((ptpdata *)(start->req->storeloc))->count = count;
								((ptpdata *)(start->req->storeloc))->target = (start->req->status).MPI_SOURCE;
								((ptpdata *)(start->req->storeloc))->tag = (start->req->status).MPI_TAG;
								pthread_mutex_unlock(&peertopeerLock);
							} else {
								(((ptpdata *)(start->req->storeloc))->completecmp) = false;
								*(start->req->reqcmp) = EMPI_REQUEST_NULL;
							}
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
								parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
								
								(start->req->status).count = count;
								if((((start->req->status).status).EMPI_TAG) == EMPI_ANY_TAG) (start->req->status).MPI_TAG = MPI_ANY_TAG;
								else (start->req->status).MPI_TAG = ((start->req->status).status).EMPI_TAG;
								(start->req->status).MPI_ERROR = ((start->req->status).status).EMPI_ERROR;
								(start->req->status).MPI_SOURCE = ((start->req->status).status).EMPI_SOURCE;
								pthread_rwlock_rdlock(&commLock);
								assert((start->req->comm->EMPI_COMM_REP) != EMPI_COMM_NULL);
								if(cmpToRepMap[((ptpdata *)(start->req->storeloc))->target] != -1) {
									(start->req->status).MPI_SOURCE = repToCmpMap[((start->req->status).status).EMPI_SOURCE];
								}
								pthread_rwlock_unlock(&commLock);
								
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
				if(!(cdata->completerep)) {
					int flag;
					if(*(req->reqrep) != EMPI_REQUEST_NULL) EMPI_Test(req->reqrep,&flag,&((req->status).status));
					else flag = 0;
					cdata->completerep = flag > 0;
					if(cdata->completerep) {
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
					signal_completion = true;
				}
			} else {
				if(!(((ptpdata *)(start->req->storeloc))->completecmp) && (*(start->req->reqcmp) != EMPI_REQUEST_NULL)) {
					int flag;
					EMPI_Test(start->req->reqcmp,&flag,&((start->req->status).status));
					(((ptpdata *)(start->req->storeloc))->completecmp) = flag > 0;
					if((((ptpdata *)(start->req->storeloc))->completecmp)) {
						if(start->req->type == MPI_FT_RECV_REQUEST) {
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
								parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
								
								(start->req->status).count = count;
								if((((start->req->status).status).EMPI_TAG) == EMPI_ANY_TAG) (start->req->status).MPI_TAG = MPI_ANY_TAG;
								else (start->req->status).MPI_TAG = ((start->req->status).status).EMPI_TAG;
								(start->req->status).MPI_ERROR = ((start->req->status).status).EMPI_ERROR;
								(start->req->status).MPI_SOURCE = ((start->req->status).status).EMPI_SOURCE;
								assert((start->req->comm->EMPI_COMM_CMP) != EMPI_COMM_NULL);
								
								((ptpdata *)(start->req->storeloc))->count = count;
								((ptpdata *)(start->req->storeloc))->target = (start->req->status).MPI_SOURCE;
								((ptpdata *)(start->req->storeloc))->tag = (start->req->status).MPI_TAG;
							} else {
								(((ptpdata *)(start->req->storeloc))->completecmp) = false;
								*(start->req->reqcmp) = EMPI_REQUEST_NULL;
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
								parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
								
								(start->req->status).count = count;
								if((((start->req->status).status).EMPI_TAG) == EMPI_ANY_TAG) (start->req->status).MPI_TAG = MPI_ANY_TAG;
								else (start->req->status).MPI_TAG = ((start->req->status).status).EMPI_TAG;
								(start->req->status).MPI_ERROR = ((start->req->status).status).EMPI_ERROR;
								(start->req->status).MPI_SOURCE = ((start->req->status).status).EMPI_SOURCE;
								assert((start->req->comm->EMPI_COMM_REP) != EMPI_COMM_NULL);
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

void test_all_requests() {
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
				if(!(cdata->completerep)) {
					int flag;
					if(*(req->reqrep) != EMPI_REQUEST_NULL) EMPI_Test(req->reqrep,&flag,&((req->status).status));
					else flag = 0;
					cdata->completerep = flag > 0;
					if(cdata->completerep) {
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
					signal_completion = true;
				}
			} else {
				if(!(((ptpdata *)(start->req->storeloc))->completecmp) && (*(start->req->reqcmp) != EMPI_REQUEST_NULL)) {
					int flag;
					EMPI_Test(start->req->reqcmp,&flag,&((start->req->status).status));
					(((ptpdata *)(start->req->storeloc))->completecmp) = flag > 0;
					if((((ptpdata *)(start->req->storeloc))->completecmp)) {
						if(start->req->type == MPI_FT_RECV_REQUEST) {
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
								parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
								
								(start->req->status).count = count;
								if((((start->req->status).status).EMPI_TAG) == EMPI_ANY_TAG) (start->req->status).MPI_TAG = MPI_ANY_TAG;
								else (start->req->status).MPI_TAG = ((start->req->status).status).EMPI_TAG;
								(start->req->status).MPI_ERROR = ((start->req->status).status).EMPI_ERROR;
								(start->req->status).MPI_SOURCE = ((start->req->status).status).EMPI_SOURCE;
								pthread_rwlock_rdlock(&commLock);
								assert((start->req->comm->EMPI_COMM_CMP) != EMPI_COMM_NULL);
								pthread_rwlock_unlock(&commLock);
								
								pthread_mutex_lock(&peertopeerLock);
								((ptpdata *)(start->req->storeloc))->count = count;
								((ptpdata *)(start->req->storeloc))->target = (start->req->status).MPI_SOURCE;
								((ptpdata *)(start->req->storeloc))->tag = (start->req->status).MPI_TAG;
								pthread_mutex_unlock(&peertopeerLock);
							} else {
								(((ptpdata *)(start->req->storeloc))->completecmp) = false;
								*(start->req->reqcmp) = EMPI_REQUEST_NULL;
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
								parep_mpi_free(((ptpdata *)(start->req->storeloc))->buf);
								
								(start->req->status).count = count;
								if((((start->req->status).status).EMPI_TAG) == EMPI_ANY_TAG) (start->req->status).MPI_TAG = MPI_ANY_TAG;
								else (start->req->status).MPI_TAG = ((start->req->status).status).EMPI_TAG;
								(start->req->status).MPI_ERROR = ((start->req->status).status).EMPI_ERROR;
								(start->req->status).MPI_SOURCE = ((start->req->status).status).EMPI_SOURCE;
								pthread_rwlock_rdlock(&commLock);
								assert((start->req->comm->EMPI_COMM_REP) != EMPI_COMM_NULL);
								if(cmpToRepMap[((ptpdata *)(start->req->storeloc))->target] != -1) {
									(start->req->status).MPI_SOURCE = repToCmpMap[((start->req->status).status).EMPI_SOURCE];
								}
								pthread_rwlock_unlock(&commLock);
								
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
						pthread_cond_signal(&reqListCond);
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
						pthread_cond_signal(&reqListCond);
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
			progressed = test_all_coll_requests();
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
					pthread_cond_signal(&reqListCond);
					pthread_mutex_unlock(&recvDataListLock);
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
					pthread_cond_signal(&reqListCond);
					pthread_mutex_unlock(&recvDataListLock);
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

