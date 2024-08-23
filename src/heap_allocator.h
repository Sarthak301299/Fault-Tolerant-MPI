#define _GNU_SOURCE
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

#include "mpi-internal.h"

#ifndef __HEAP_ALLOCATOR_H__
#define __HEAP_ALLOCATOR_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>

//#define PAREP_MPI_HEAP_SIZE 0x8000000
#define PAREP_MPI_HEAP_SIZE 0x40000
//#define PAREP_MPI_HEAP_MAX_SIZE 0x8000000
#define PAREP_MPI_HEAP_MAX_SIZE 0x400000000

#define MIN_ALLOC_SZ 8

#define BIN_COUNT 9
#define BIN_MAX_IDX (BIN_COUNT - 1)

#define FIRST_BIT_MASK (uint64_t)(1ULL << 63)
#define SECOND_BIT_MASK (uint64_t)(1ULL << 62)
#define REMAINING_BITS_MASK ((uint64_t)~(FIRST_BIT_MASK | SECOND_BIT_MASK))

#define GET_HOLE(x) (((x) & FIRST_BIT_MASK) >> 63)
#define SET_HOLE(x,value) ((x) = ((x) & ~FIRST_BIT_MASK) | (((uint64_t)(value) & 1) << 63))

#define GET_FAST(x) (((x) & SECOND_BIT_MASK) >> 62)
#define SET_FAST(x,value) ((x) = ((x) & ~SECOND_BIT_MASK) | (((uint64_t)(value) & 1) << 62))

#define GET_SIZE(x) ((x) & REMAINING_BITS_MASK)
#define SET_SIZE(x, value) ((x) = ((x) & (FIRST_BIT_MASK | SECOND_BIT_MASK)) | ((uint64_t)(value) & REMAINING_BITS_MASK))

#define FASTBIN_SIZE_THRESHOLD 64
#define FASTBIN_COUNT 9
#define FASTBIN_MAX_IDX (FASTBIN_COUNT - 1)

#define MIN_LARGE_SIZE 512
#define FASTBIN_CONSOLIDATION_THRESHOLD  (65536UL)

typedef uint64_t address;

typedef struct parep_mpi_heap_node_t {
	uint64_t hole_and_size;
	//unsigned int hole;
	//unsigned int size;
	//unsigned long size;
	struct parep_mpi_heap_node_t *next;
	struct parep_mpi_heap_node_t *prev;
} heap_node_t;

typedef struct {
	heap_node_t *header;
} footer_t;

typedef struct {
	heap_node_t *head;
} bin_t;

typedef struct {
	address start;
	address end;
	bin_t *bins[BIN_COUNT];
	bin_t *fastbins[FASTBIN_COUNT];
} heap_t;

static size_t overhead = sizeof(footer_t) + sizeof(heap_node_t);

int expand_heap(size_t);

footer_t *get_foot(heap_node_t *);

void parep_mpi_init_heap(address);
void *parep_mpi_malloc(size_t);
void parep_mpi_free(void *);
void *parep_mpi_realloc(void *,size_t);
void *parep_mpi_calloc(size_t,size_t);
char *parep_mpi_strdup(const char *);
char *parep_mpi_strndup(const char *, size_t);
char *parep_mpi_realpath(const char *, char *);

void parep_mpi_malloc_consolidate(void);

#endif