#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <pthread.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>

#ifndef __SENDBUF_CACHE_H__
#define __SENDBUF_CACHE_H__

#define PAREP_MPI_COMMBUF_CACHE_ENTRIES 10240
#define PAREP_MPI_COMMBUF_CACHE_ENTRIES_MAX 16384

#define PAREP_MPI_COMMBUF_BIN_COUNT 9
#define PAREP_MPI_COMMBUF_BIN_MAX_IDX (PAREP_MPI_COMMBUF_BIN_COUNT - 1)

#define COMMBUF_BIN_COUNT PAREP_MPI_COMMBUF_BIN_COUNT
#define COMMBUF_BIN_MAX_IDX PAREP_MPI_COMMBUF_BIN_MAX_IDX

typedef struct parep_mpi_commbuf_node_t {
	size_t size;
	void *commbuf;
	struct parep_mpi_commbuf_node_t *next;
	struct parep_mpi_commbuf_node_t *prev;
} commbuf_node_t;

typedef struct {
	commbuf_node_t *head;
	commbuf_node_t *tail;
} commbuf_bin_t;

commbuf_node_t *new_commbuf_node(size_t);
void delete_commbuf_node(commbuf_node_t *);
int commbuf_node_insert(commbuf_node_t *,commbuf_bin_t *);
int commbuf_node_reinsert(commbuf_node_t *,commbuf_bin_t *);
commbuf_node_t *commbuf_node_remove(commbuf_node_t *,commbuf_bin_t *);
commbuf_node_t *commbuf_node_find(size_t size);
commbuf_node_t *commbuf_node_evict(commbuf_bin_t *);

commbuf_node_t *get_commbuf_node(size_t);
void return_commbuf_node(commbuf_node_t *);

#endif