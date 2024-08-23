#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <pthread.h>

#include "mpi-internal.h"
#include "mpi.h"

#ifndef __FULL_CONTEXT_H__
#define __FULL_CONTEXT_H__

#if defined(__i386__) || defined(__x86_64__)
#if defined(__i386__) && defined(__PIC__)
#define RMB asm volatile ("lfence" : : : "eax", "ecx", "edx", "memory")
#define WMB asm volatile ("sfence" : : : "eax", "ecx", "edx", "memory")
#define IMB
#else
#define RMB asm volatile ("xorl %%eax,%%eax ; cpuid" : : : "eax", "ebx", "ecx", "edx", "memory")
#define WMB asm volatile ("xorl %%eax,%%eax ; cpuid" : : : "eax", "ebx", "ecx", "edx", "memory")
#define IMB
#endif
#elif defined(__arm__)
#define RMB asm volatile (".arch armv7-a \n\t dsb ; dmb" : : : "memory")
#define WMB asm volatile (".arch armv7-a \n\t dsb ; dmb" : : : "memory")
#define IMB asm volatile (".arch armv7-a \n\t isb" : : : "memory")
#elif defined(__aarch64__)
#define RMB asm volatile (".arch armv8.1-a \n\t dsb sy\n" : : : "memory")
#define WMB asm volatile (".arch armv8.1-a \n\t dsb sy\n" : : : "memory")
#define IMB asm volatile (".arch armv8.1-a \n\t isb" : : : "memory")
#else
#error "instruction architecture not implemented"
#endif

#define NO_OPTIMIZE __attribute__((optimize(0)))

#define JB_RBP	1
#define JB_RSP	6
#define JB_PC	7

#define TEMP_STACK_SIZE 262144
//#define TEMP_STACK_SIZE 1048576
#define TEMP_STACK_BOTTOM_OFFSET 256

#ifndef HIGHEST_VA
#ifdef __x86_64__
#define HIGHEST_VA ((address)0x7f00000000)
#else
#define HIGHEST_VA ((address)0xC0000000)
#endif
#endif

#ifdef __x86_64__
#define eax rax
#define ebx rbx
#define ecx rcx
#define edx rax
#define ebp rbp
#define esi rsi
#define edi rdi
#define esp rsp
#define CLEAN_FOR_64_BIT(args ...)	CLEAN_FOR_64_BIT_HELPER(args)
#define CLEAN_FOR_64_BIT_HELPER(args ...)	#args
#elif __i386__
#define CLEAN_FOR_64_BIT(args ...)	#args
#else
#define CLEAN_FOR_64_BIT(args ...)	"CLEAN_FOR_64_BIT_undefined"
#endif

typedef uint64_t address;

typedef struct ContextContainer {
	address rip;
	address rsp;
	address rbp;
} Context;

typedef struct {
	address container_address;
	address linked_address;
	address allocated_address;	// irrelevant on replica node.
	size_t size;
} Malloc_container;

typedef struct Malloc_list {
	Malloc_container container;
	struct Malloc_list *next;
	struct Malloc_list *prev;
} Malloc_list;

void copy_jmp_buf(jmp_buf, jmp_buf);

address getRSP(jmp_buf);
address getRBP(jmp_buf);
address getPC(jmp_buf);
int setRSP(jmp_buf, address);
int setRBP(jmp_buf, address);
int setPC(jmp_buf, address);

void init_ckpt(char *, bool);
void save_data_seg();
void save_stack_seg();
void save_heap_seg();

void parep_mpi_thread_ckpt_wait(int,int);
void *getCurrentPC();

void init_ckpt_restore(char *);
void restore_data_seg();
void restore_stack_seg();
void restore_heap_seg();

void write_heap_and_stack(char *);
void read_heap_and_stack(char *);

int does_ckpt_file_exists(char *);

#endif