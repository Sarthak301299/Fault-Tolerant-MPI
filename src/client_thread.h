#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <pthread.h>
#include <errno.h>

#ifndef __CLIENT_THREAD_H__
#define __CLIENT_THREAD_H__

#include "mpi-internal.h"
#include "ibvctx.h"

#define CMD_INFORM_PROC_FAILED 10
#define CMD_EXIT_CALLED 12
#define CMD_SHRINK_PERFORMED 13
#define CMD_REM_RECV 20
#define CMD_REM_RECV_FINISHED 21
#define CMD_INFORM_PREDICT 30
#define CMD_BLOCK_PREDICT 31
#define CMD_INFORM_PREDICT_NODE 34

struct parep_mpi_recv_data_list_node {
	ptpdata *pdata;
	struct parep_mpi_recv_data_list_node *next;
	struct parep_mpi_recv_data_list_node *prev;
};
typedef struct parep_mpi_recv_data_list_node recvDataNode;

pthread_t daemon_poller;
pthread_t comm_shrinker;

static pthread_mutex_t performing_shrink_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t performing_shrink_cond = PTHREAD_COND_INITIALIZER;
static int performing_shrink = 0;

static pthread_mutex_t thread_active_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t thread_active_cond = PTHREAD_COND_INITIALIZER;
static int thread_active = 0;

static int waiting_for_resp = 0;

void *comm_shrink(void *);
void *polling_daemon(void *);

int handle_rem_recv();
int handle_rem_recv_no_poll();

EMPI_Group parep_mpi_original_group;
static EMPI_Group parep_mpi_failed_group = EMPI_GROUP_EMPTY;

void recvDataListInsert(ptpdata *);
void recvDataListDelete(recvDataNode *);
recvDataNode *recvDataListFind(int,int,MPI_Comm);
recvDataNode *recvDataListFindWithId(int,int,int,MPI_Comm);
recvDataNode *recvDataListFindWildCard(int,MPI_Comm);

void recvDataRedListInsert(ptpdata *);
void recvDataRedListDelete(recvDataNode *);
recvDataNode *recvDataRedListFind(int,int);

#endif