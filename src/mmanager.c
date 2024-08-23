#include "mmanager.h"
/*
void prepareHeader(CkptHeader *ckptHdr) {
	memset(ckptHdr, 0, sizeof(*ckptHdr));
	strncpy(ckptHdr->signature, SIGNATURE, strlen(SIGNATURE) + 1);
	ckptHdr->saved_brk = sbrk(0);
	ckptHdr->end_of_stack = 
	mtcpHdr->restore_addr = 
	mtcpHdr->restore_size = 

	mtcpHdr->vdsoStart = 
	mtcpHdr->vdsoEnd = 
	mtcpHdr->vvarStart = 
	mtcpHdr->vvarEnd = 

	mtcpHdr->post_restart = &ThreadList::postRestart;
}
*/

extern address parep_mpi_new_stack_start;
extern address parep_mpi_new_stack_end;

extern int (*_real_fclose)(FILE *);
extern FILE *(*_real_fopen)(const char *, const char *);

#define NO_OPTIMIZE __attribute__((optimize(0)))

NO_OPTIMIZE
static int parep_mpi_open(const char *pathname, int flags, mode_t mode) {
	int ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"mov %3, %%rsi\n\t"
								"mov %4, %%rdx\n\t"
								"syscall\n\t"
								"mov %%eax, %0"
								: "=r" (ret)
								: "i" (SYS_open),
									"r" (pathname),
									"r" ((long)flags),
									"r" ((long)mode)
								: "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static int parep_mpi_close(int fd) {
	int ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"syscall\n\t"
								"mov %%eax, %0"
								: "=r" (ret)
								: "i" (SYS_close),
									"r" ((long)fd)
								: "rax", "rdi", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static ssize_t parep_mpi_read(int fd, void *buf, size_t count) {
	ssize_t ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"mov %3, %%rsi\n\t"
								"mov %4, %%rdx\n\t"
								"syscall\n\t"
								"mov %%rax, %0"
								: "=r" (ret)
								: "i" (SYS_read),
									"r" ((long)fd),
									"r" (buf),
									"r" (count)
								: "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static char *parep_mpi_custom_fgets(char *buf, int size, int fd) {
	if(buf == NULL || size <= 0 || fd < 0) {
		return NULL;
	}
	
	int bytesRead = 0;
	char ch;
	
	while(bytesRead < size -1 && parep_mpi_read(fd,&ch,1) == 1) {
		buf[bytesRead++] = ch;
		if(ch == '\n' || ch == EOF) {
			break;
		}
	}
	buf[bytesRead] = '\0';
	return (bytesRead > 0) ? buf : NULL;
}

uint8_t getProtBmapFromString(char str[5]) {
	uint8_t out = 0x0;
	if(str[0] == 'r') out = out | PROT_READ;
	if(str[1] == 'w') out = out | PROT_WRITE;
	if(str[2] == 'x') out = out | PROT_EXEC;
	return out;
}

int getFlagBmapFromString(char str[5]) {
	int out = 0x0;
	if(str[3] == 'p') out = out | MAP_PRIVATE;
	else if(str[3] == 's') out = out | MAP_SHARED;
	return out;
}

void parseStatFile() {
	int pid = getpid();
	
	char stat_path[MAX_PATH_LEN];
	snprintf(stat_path, MAX_PATH_LEN, "/proc/%d/stat", pid);
	
	FILE* stat_file = _real_fopen(stat_path, "r");
	if (stat_file == NULL) {
		perror("Error opening stat file");
		exit(EXIT_FAILURE);
	}
	
	char line[1024];
	fscanf(stat_file, "%d %255s %c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %d",
				&prstat.pid, prstat.comm, &prstat.state, &prstat.ppid, &prstat.pgrp, &prstat.session, &prstat.tty_nr, &prstat.tpgid, &prstat.flags, &prstat.minflt, &prstat.cminflt, &prstat.majflt, &prstat.cmajflt, &prstat.utime, &prstat.stime, &prstat.cutime,
				&prstat.cstime, &prstat.priority, &prstat.nice, &prstat.num_threads, &prstat.itrealvalue, &prstat.starttime, &prstat.vsize, &prstat.rss, &prstat.rsslim, &prstat.startcode, &prstat.endcode, &prstat.startstack, &prstat.kstkesp, &prstat.kstkeip,
				&prstat.signal, &prstat.blocked, &prstat.sigignore, &prstat.sigcatch, &prstat.wchan, &prstat.nswap, &prstat.cnswap, &prstat.exit_signal, &prstat.processor, &prstat.rt_priority, &prstat.policy, &prstat.delayacct_blkio_ticks, &prstat.guest_time,
				&prstat.cguest_time, &prstat.start_data, &prstat.end_data, &prstat.start_brk, &prstat.arg_start, &prstat.arg_end, &prstat.env_start, &prstat.env_end, &prstat.exit_code);
	_real_fclose(stat_file);
}

void parseSockInfo() {
	num_socks = 0;
	int max_fd = getdtablesize();
	struct stat fd_stat;

	for (int fd = 5; fd < max_fd; fd++) {
		if (fstat(fd, &fd_stat) == 0) {
			mode_t mode = fd_stat.st_mode;

			if (S_ISSOCK(mode)) {
				struct sockaddr_in addr;
				socklen_t addrlen = sizeof(addr);
				getsockname(fd, (struct sockaddr *restrict) &addr, &addrlen);
				if(addr.sin_family == AF_INET) {
					sockinfo[num_socks].fd = fd;
					memcpy(&sockinfo[num_socks].addr,&addr,sizeof(struct sockaddr));
					memcpy(&sockinfo[num_socks].len,&addrlen,sizeof(socklen_t));
					num_socks++;
				}
			}
		}
	}
}

NO_OPTIMIZE
static unsigned long int readdec(char *data, int *dataIdx) {
	unsigned long int v = 0;
	while(1) {
		char c = data[*dataIdx];
		if((c >= '0') && (c <= '9')) {
			c -= '0';
		} else {
			break;
		}
		v = (v*10) + c;
		(*dataIdx)++;
	}
	return v;
}

NO_OPTIMIZE
static unsigned long int readhex(char *data, int *dataIdx) {
	unsigned long int v = 0;
	while(1) {
		char c = data[*dataIdx];
		if((c >= '0') && (c <= '9')) {
			c -= '0';
		} else if((c >= 'a') && (c <= 'f')) {
			c -= 'a' - 10;
		} else if ((c >= 'A') && (c <= 'F')) {
			c -= 'A' - 10;
		} else {
			break;
		}
		v = (v*16) + c;
		(*dataIdx)++;
	}
	return v;
}

NO_OPTIMIZE
static void parep_mpi_assert(int exp) {
	if(!exp) {
		printf("parep_mpi_assert failed\n");
		abort();
	}
}

// Function to parse /proc/<pid>/maps and store mappings
void parseMapsFile() {
	
	int pid = getpid();
	address addr_brk;
	
	char maps_path[MAX_PATH_LEN];
	snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

	//FILE* maps_file = fopen(maps_path, "r");
	int maps_file = parep_mpi_open(maps_path,O_RDONLY,0777);
	//if (maps_file == NULL) {
	if(maps_file == -1) {
		perror("Error opening maps file");
		exit(EXIT_FAILURE);
	}
	
	if((address)sbrk(0) != (address)-1) {
		actual_brk = (address)sbrk(0);
		addr_brk = actual_brk;
	} else {
		addr_brk = actual_brk;
	}
	
	num_mappings = 0;
	num_heapmaps = 0;
	num_stackmaps = 0;
	num_anonmaps = 0;
	char line[256];
	char tempperm[5];
	prstat.vdsoStart = (address)NULL;
	prstat.vdsoEnd = (address)NULL;
	prstat.vvarStart = (address)NULL;
	prstat.vvarEnd = (address)NULL;
	prstat.vsyscallStart = (address)NULL;
	prstat.vsyscallEnd = (address)NULL;
	
	
	
	//while (fgets(line, sizeof(line), maps_file)) {
	while (parep_mpi_custom_fgets(line,sizeof(line),maps_file)) {
		if (num_mappings >= MAX_MAPPINGS) {
			printf("Too many mappings, increase MAX_MAPPINGS\n");
			break;
		}
		
		int idx = 0;
		
		mappings[num_mappings].start = readhex(line,&idx);
		parep_mpi_assert(line[idx++] == '-');
		
		mappings[num_mappings].end = readhex(line,&idx);
		parep_mpi_assert(line[idx++] == ' ');
		
		tempperm[0] = line[idx++];
		tempperm[1] = line[idx++];
		tempperm[2] = line[idx++];
		tempperm[3] = line[idx++];
		tempperm[4] = '\0';
		
		parep_mpi_assert(line[idx++] == ' ');
		
		mappings[num_mappings].offset = readhex(line,&idx);
		parep_mpi_assert(line[idx++] == ' ');
		
		mappings[num_mappings].dev_major = readhex(line,&idx);
		parep_mpi_assert(line[idx++] == ':');
		
		mappings[num_mappings].dev_minor = readhex(line,&idx);
		parep_mpi_assert(line[idx++] == ' ');
		
		mappings[num_mappings].inode = readdec(line,&idx);
		
		while(line[idx] == ' ') {
			idx++;
		}
		
		mappings[num_mappings].pathname[0] = '\0';
		if(line[idx] == '/' || line[idx] == '[' || line[idx] == '(') {
			size_t i = 0;
			while(line[idx] != '\n') {
				mappings[num_mappings].pathname[i++] = line[idx++];
				parep_mpi_assert(i < MAX_PATH_LEN);
			}
			mappings[num_mappings].pathname[i] = '\0';
		}
		parep_mpi_assert(line[idx++] == '\n');
		
		//int rnum =sscanf(line, "%lx-%lx %4s %lx %x:%x %d %s",&mappings[num_mappings].start, &mappings[num_mappings].end,
		//					tempperm, &mappings[num_mappings].offset, &mappings[num_mappings].dev_major,
		//					&mappings[num_mappings].dev_minor, &mappings[num_mappings].inode, &mappings[num_mappings].pathname);
						
		//if(rnum < 7) printf("sscanf returned %d items!!!!\n",rnum);
		//if(rnum == 7) mappings[num_mappings].pathname[0] = '\0';
		
		mappings[num_mappings].prot = getProtBmapFromString(tempperm);
		mappings[num_mappings].flags = 0x0;
		mappings[num_mappings].flags |= getFlagBmapFromString(tempperm);
		
		if(mappings[num_mappings].pathname[0] == '\0') {
			if((addr_brk > mappings[num_mappings].start) && ((addr_brk <= mappings[num_mappings].end))) {
				strcpy(mappings[num_mappings].pathname,"[heap]");
				num_heapmaps++;
			} else if((actual_stack_start == mappings[num_mappings].start) && (actual_stack_end == mappings[num_mappings].end)) {
				strcpy(mappings[num_mappings].pathname,"[stack]");
				mappings[num_mappings].flags |= MAP_GROWSDOWN;
				num_stackmaps++;
			} else if((parep_mpi_new_stack_start == mappings[num_mappings].start) && (parep_mpi_new_stack_end == mappings[num_mappings].end)) {
				mappings[num_mappings].flags |= MAP_GROWSDOWN;
				num_stackmaps++;
			}
			else {
				mappings[num_mappings].flags |= MAP_ANONYMOUS;
				num_anonmaps++;
			}
		}
		
		else if(!strcmp(mappings[num_mappings].pathname,"[heap]")) {
			num_heapmaps++;
		}
		
		else if(!strcmp(mappings[num_mappings].pathname,"[stack]")) {
			if((actual_stack_start > mappings[num_mappings].start) || (actual_stack_start == (address)-1)) actual_stack_start = mappings[num_mappings].start;
			if((actual_stack_end < mappings[num_mappings].end) || (actual_stack_end == (address)-1)) actual_stack_end = mappings[num_mappings].end;
			mappings[num_mappings].flags |= MAP_GROWSDOWN;
			num_stackmaps++;
		}
		
		else if(!strcmp(mappings[num_mappings].pathname,"[vdso]")) {
			if((mappings[num_mappings].start < prstat.vdsoStart) || (prstat.vdsoStart == (address)NULL)) prstat.vdsoStart = mappings[num_mappings].start;
			if((mappings[num_mappings].end > prstat.vdsoEnd) || (prstat.vdsoEnd == (address)NULL)) prstat.vdsoEnd = mappings[num_mappings].end;
		}
		
		else if(!strcmp(mappings[num_mappings].pathname,"[vvar]")) {
			if((mappings[num_mappings].start < prstat.vvarStart) || (prstat.vvarStart == (address)NULL)) prstat.vvarStart = mappings[num_mappings].start;
			if((mappings[num_mappings].end > prstat.vvarEnd) || (prstat.vvarEnd == (address)NULL)) prstat.vvarEnd = mappings[num_mappings].end;
		}
		
		else if(!strcmp(mappings[num_mappings].pathname,"[vsyscall]")) {
			if((mappings[num_mappings].start < prstat.vsyscallStart) || (prstat.vsyscallStart == (address)NULL)) prstat.vsyscallStart = mappings[num_mappings].start;
			if((mappings[num_mappings].end > prstat.vsyscallEnd) || (prstat.vsyscallEnd == (address)NULL)) prstat.vsyscallEnd = mappings[num_mappings].end;
		}
		
		if((mappings[num_mappings].pathname[0] == '\0') || (mappings[num_mappings].pathname[0] == '[')) {
			mappings[num_mappings].flags |= MAP_ANONYMOUS;
		}
		
		mappings[num_mappings].wrtSize = (long)-1;
		
//		if(strstr(mappings[num_mappings].pathname, "[heap]") || strstr(mappings[num_mappings].pathname, "[stack]"))printf("MAPinfo %p-%p %d %d %p %x:%x %d %s\n", mappings[num_mappings].start, mappings[num_mappings].end,
//						mappings[num_mappings].prot, mappings[num_mappings].flags, mappings[num_mappings].offset, mappings[num_mappings].dev_major,
//						mappings[num_mappings].dev_minor, mappings[num_mappings].inode, mappings[num_mappings].pathname);
		num_mappings++;
	}
	
	//fclose(maps_file);
	parep_mpi_close(maps_file);
	
}