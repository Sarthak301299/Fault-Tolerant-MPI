#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <malloc.h>

#ifndef __WRAPPERS_H__
#define __WRAPPERS_H__

//#define _real_dlopen	NEXT_FNC(dlopen)
//#define _real_dlclose	NEXT_FNC(dlclose)

//int __parep_mpi_ckpt_disabled_dl = false;
bool parep_mpi_disable_ckpt();
void parep_mpi_enable_ckpt();

pthread_t waitWaker;

//void parep_mpi_disable_ckpt_dl();
//void parep_mpi_enable_ckpt_dl();

#define PAREP_MPI_DISABLE_CKPT() bool __parep_mpi_ckpt_disabled = parep_mpi_disable_ckpt()
#define PAREP_MPI_ENABLE_CKPT() if(__parep_mpi_ckpt_disabled) parep_mpi_enable_ckpt()

//#define PAREP_MPI_DISABLE_CKPT_DL() bool __parep_mpi_ckpt_disabled = parep_mpi_disable_ckpt()
//#define PAREP_MPI_ENABLE_CKPT_DL() if(__parep_mpi_ckpt_disabled) parep_mpi_enable_ckpt()

struct parep_mpi_open_file_node {
	int fd;
	char fname[256];
	char mode[8];
	FILE *file;
	bool use64;
	struct parep_mpi_open_file_node *next;
	struct parep_mpi_open_file_node *prev;
};
typedef struct parep_mpi_open_file_node openFileNode;

void openFileListInsert(int,char *,char *,FILE *,bool);
void openFileListDelete(openFileNode *);
openFileNode *openFileListFind(int);

void set_signal_handler(int);

typedef struct pthread_create_intercept_arg {
	void *(*start_routine)(void *);
	void *restrict arg;
} pcin_arg_t;

#endif