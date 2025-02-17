#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <pthread.h>
#include <errno.h>

#ifndef __REQUEST_HANDLER_H__
#define __REQUEST_HANDLER_H__

#include "mpi-internal.h"
#include "ibvctx.h"

reqNode *reqListInsert(MPI_Request);
void reqListDelete(reqNode *);
bool reqListIsEmpty();

bool test_all_coll_requests();
bool test_all_ptp_requests();
void test_all_requests();
void test_all_requests_no_lock();

int probe_msg_from_source(MPI_Comm, int);
void probe_reduce_messages();
void probe_reduce_messages_with_comm(MPI_Comm);

pthread_t request_handler;
void *handling_requests(void *);

#endif