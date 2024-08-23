#ifndef _MMANAGER_H_
#define _MMANAGER_H_

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>

#include "full_context.h"

#define MAX_SOCKETS 20
#define MAX_PATH_LEN 256
#define MAX_MAPPINGS 1000
#define MAX_MAPS_LINE_LEN 1024
#define SIGNATURE "CKPT_HEADER_v2.2\n"
#define SIGNATURE_LEN 32

// Structure to hold information about memory mappings
typedef struct MemoryMapping {
	address start;
	address end;
	int prot;
	int flags;
	address offset;
	int dev_major;
	int dev_minor;
	int inode;
	char pathname[MAX_PATH_LEN];
	size_t wrtSize;
} Mapping;

// Structure to hold information about process status
typedef struct ProcessStatus {
	int pid;
	char comm[MAX_PATH_LEN];
	char state;
	int ppid;
	int pgrp;
	int session;
	int tty_nr;
	int tpgid;
	unsigned int flags;
	unsigned long minflt;
	unsigned long cminflt;
	unsigned long majflt;
	unsigned long cmajflt;
	unsigned long utime;
	unsigned long stime;
	long cutime;
	long cstime;
	long priority;
	long nice;
	long num_threads;
	long itrealvalue;
	unsigned long long starttime;
	unsigned long vsize;
	long rss;
	unsigned long rsslim;
	unsigned long startcode;
	unsigned long endcode;
	unsigned long startstack;
	unsigned long kstkesp;
	unsigned long kstkeip;
	unsigned long signal;
	unsigned long blocked;
	unsigned long sigignore;
	unsigned long sigcatch;
	unsigned long wchan;
	unsigned long nswap;
	unsigned long cnswap;
	int exit_signal;
	int processor;
	unsigned int rt_priority;
	unsigned int policy;
	unsigned long long delayacct_blkio_ticks;
	unsigned long guest_time;
	long cguest_time;
	address start_data;
	address end_data;
	address start_brk;
	address arg_start;
	address arg_end;
	address env_start;
	address env_end;
	int exit_code;
	address cur_brk;
	address vsyscallStart;
	address vsyscallEnd;
	address vdsoStart;
	address vdsoEnd;
	address vvarStart;
	address vvarEnd;
	address restore_start;
	address restore_end;
	int related_map;
	int restore_start_map;
	address restore_full_end;
} ProcessStat;


typedef struct SocketInfo {
	int fd;
	struct sockaddr addr;
	socklen_t len;
} SockInfo;

ProcessStat prstat;
Mapping mappings[MAX_MAPPINGS];
SockInfo sockinfo[MAX_SOCKETS];
address base_brk;

int num_socks;

int num_mappings;
int num_heapmaps;
int num_stackmaps;
int num_anonmaps;

address actual_brk;
address actual_stack_start;
address actual_stack_end;

void parseStatFile();
void parseSockInfo();
void parseMapsFile();

#endif