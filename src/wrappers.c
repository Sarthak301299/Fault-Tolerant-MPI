#define _GNU_SOURCE

#include <dlfcn.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <execinfo.h>

#include "mpi-internal.h"
#include "full_context.h"
#include "client_thread.h"
#include "request_handler.h"
#include "wrappers.h"
#include "heap_allocator.h"
#include "mpi.h"

extern int ___ckpt_counter;

extern int parep_mpi_pmi_fd;
extern char parep_mpi_kvs_name[256];

extern volatile sig_atomic_t parep_mpi_restore;
extern volatile sig_atomic_t parep_mpi_restore_wait;
extern volatile sig_atomic_t parep_mpi_ckpt_wait;

extern bool __parep_mpi_ckpt_triggered;
extern bool __parep_mpi_ckpt_restore_triggered;
extern bool __parep_mpi_ckpt_disable_triggered;

extern void parep_mpi_sigusr1_hook(int,bool,int);

extern int num_threads;
extern pthread_mutex_t num_thread_mutex;
extern pthread_cond_t num_thread_cond;
extern pthread_t thread_tid[10];
extern bool signal_bcasted;

extern int parep_mpi_size;
extern int parep_mpi_rank;
extern int parep_mpi_node_rank;
extern int parep_mpi_node_id;
extern int parep_mpi_node_num;
extern int parep_mpi_node_size;

pthread_rwlock_t wrapperLock = PTHREAD_RWLOCK_INITIALIZER;
int nreaders = 0;
int nwriters = 0;
bool threadholdswrlock = false;

extern pid_t kernel_tid[10];

extern bool entered_sighandler[10];
extern pthread_mutex_t entered_sighandler_mutex;
extern pthread_cond_t entered_sighandler_cond;

extern void *parep_mpi_ext_heap_mapping;
extern size_t parep_mpi_ext_heap_size;

extern void test_all_requests();

extern pthread_rwlock_t commLock;
extern pthread_rwlock_t fgroupLock;

extern reqNode *reqHead;
extern reqNode *reqTail;

extern pthread_mutex_t reqListLock;
extern pthread_cond_t reqListCond;

extern recvDataNode *recvDataHead;
extern recvDataNode *recvDataTail;

extern pthread_mutex_t recvDataListLock;
extern pthread_cond_t recvDataListCond;

extern struct peertopeer_data *first_peertopeer;
extern struct peertopeer_data *last_peertopeer;
extern struct collective_data *last_collective;

extern long parep_mpi_store_buf_sz;
extern pthread_mutex_t parep_mpi_store_buf_sz_mutex;
extern pthread_cond_t parep_mpi_store_buf_sz_cond;

extern pthread_mutex_t peertopeerLock;
extern pthread_cond_t peertopeerCond;

extern pthread_mutex_t collectiveLock;
extern pthread_cond_t collectiveCond;

extern int parep_mpi_validator_pipe_check;

extern bool parep_mpi_initialized;
extern int parep_mpi_completed;

extern volatile sig_atomic_t parep_mpi_swap_replicas;
extern volatile sig_atomic_t parep_mpi_restart_replicas;

extern int *ctrmap;
extern int *rtcmap;
extern pthread_mutex_t procmaplock;

bool use_common_heap = false;

bool daemon_poller_free = false;
pthread_mutex_t daemon_poller_free_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t daemon_poller_free_cond = PTHREAD_COND_INITIALIZER;

extern volatile sig_atomic_t parep_mpi_sighandling_state;

extern int rem_recv_msg_sent;
extern pthread_mutex_t rem_recv_msg_sent_mutex;

volatile int end_wake_thread = 0;

extern volatile sig_atomic_t parep_mpi_failed_proc_recv;

void *(*_real_dlopen)(const char *,int) = NULL;

int (*_real_open)(const char *pathname, int flags, ...) = NULL;
FILE *(*_real_fopen)(const char *__restrict__filename, const char *__restrict__mode) = NULL;
FILE *(*_real_fopen64)(const char *__restrict__filename, const char *__restrict__mode) = NULL;
int (*_real_fclose)(FILE *stream) = NULL;

void (*_real_free)(void *) = NULL;
void *(*_real_malloc)(size_t) = NULL;
void *(*_real_calloc)(size_t,size_t) = NULL;
void *(*_real_realloc)(void *,size_t) = NULL;
char *(*_real_strdup)(const char *) = NULL;
char *(*_real_strndup)(const char *,size_t) = NULL;
char *(*_real_realpath)(const char *,char *) = NULL;
struct mallinfo (*_real_mallinfo)(void) = NULL;

void (*_ext_free)(void *) = NULL;
void *(*_ext_malloc)(size_t) = NULL;
void *(*_ext_calloc)(size_t,size_t) = NULL;
void *(*_ext_realloc)(void *,size_t) = NULL;
struct mallinfo (*_ext_mallinfo)(void) = NULL;

void *(*_real_memalign)(size_t,size_t) = NULL;
int (*_real_posix_memalign)(void **,size_t,size_t) = NULL;
void *(*_real_valloc)(size_t) = NULL;
int (*_real_pthread_create)(pthread_t *restrict,const pthread_attr_t *restrict,void *(*)(void *),void *restrict) = NULL;

void *(*_ext_valloc)(size_t) = NULL;
void *(*_ext_memalign)(size_t,size_t) = NULL;

int (*_real_pipe)(int pipefd[2]) = NULL;
int (*_real_pipe2)(int pipefd[2],int flags) = NULL;

ssize_t (*_real_process_vm_readv)(pid_t pid, const struct iovec *local_iov, unsigned long liovcnt, const struct iovec *remote_iov, unsigned long riovcnt,unsigned long flags) = NULL;

extern bool parep_mpi_internal_call;
openFileNode *openFileHead = NULL;
openFileNode *openFileTail = NULL;

pthread_mutex_t openFileListLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t openFileListCond = PTHREAD_COND_INITIALIZER;

void openFileListInsert(int fd, char *fname, char *mode, FILE *file, bool use64) {
	openFileNode *newnode = parep_mpi_malloc(sizeof(openFileNode));
	newnode->fd  = fd;
	strcpy(newnode->fname,fname);
	strcpy(newnode->mode,mode);
	newnode->file = file;
	newnode->use64 = use64;
	newnode->next = NULL;
	newnode->prev = NULL;
	if(openFileTail == NULL) {
		openFileHead = newnode;
	} else {
		newnode->prev = openFileTail;
		openFileTail->next = newnode;
	}
	openFileTail = newnode;
}

void openFileListDelete(openFileNode *onode) {
	if(onode->prev != NULL) onode->prev->next = onode->next;
	else openFileHead = onode->next;
	if(onode->next != NULL) onode->next->prev = onode->prev;
	else openFileTail = onode->prev;
	parep_mpi_free(onode);
}

bool openFileListIsEmpty() {
	return openFileHead == NULL;
}

openFileNode *openFileListFind(int fd) {
	openFileNode *out = openFileHead;
	while(out != NULL) {
		if(out->fd == fd) break;
		out = out->next;
	}
	return out;
}

bool parep_mpi_disable_ckpt() {
	if(threadholdswrlock) return false;
	else { 
		int ret = pthread_rwlock_rdlock(&wrapperLock);
		return (ret==0);
	}
}

void parep_mpi_enable_ckpt() {
	pthread_rwlock_unlock(&wrapperLock);
}

ssize_t process_vm_readv(pid_t pid, const struct iovec *local_iov, unsigned long liovcnt, const struct iovec *remote_iov, unsigned long riovcnt,unsigned long flags) {
	if(_real_process_vm_readv == NULL) _real_process_vm_readv = dlsym(RTLD_NEXT,"process_vm_readv");
	ssize_t result;
	int old_errno = errno;
	result = _real_process_vm_readv(pid,local_iov,liovcnt,remote_iov,riovcnt,flags);
	if((result == -1) && (errno == ESRCH)) {
		errno = old_errno;
		result = local_iov->iov_len;
	}
	return result;
}

int pipe(int pipefd[2]) {
	if(_real_pipe == NULL) _real_pipe = dlsym(RTLD_NEXT,"pipe");
	int result;
	result = _real_pipe(pipefd);
	if(parep_mpi_validator_pipe_check) {
		int temp = fcntl(PAREP_MPI_VALIDATOR_RD_FD,F_GETFD);
		int rdvalid = (temp != -1) || (errno != EBADF);
		temp = fcntl(PAREP_MPI_VALIDATOR_WR_FD,F_GETFD);
		int wrvalid = (temp != -1) || (errno != EBADF);
		if(!rdvalid && !wrvalid) {
			if(pipefd[0] != PAREP_MPI_VALIDATOR_RD_FD) {
				dup2(pipefd[0],PAREP_MPI_VALIDATOR_RD_FD);
				close(pipefd[0]);
			}
			if(pipefd[1] != PAREP_MPI_VALIDATOR_WR_FD) {
				dup2(pipefd[1],PAREP_MPI_VALIDATOR_WR_FD);
				close(pipefd[1]);
			}
		}
	}
	return result;
}

int pipe2(int pipefd[2], int flags) {
	if(_real_pipe2 == NULL) _real_pipe2 = dlsym(RTLD_NEXT,"pipe2");
	int result;
	result = _real_pipe2(pipefd, flags);
	if(parep_mpi_validator_pipe_check) {
		int temp = fcntl(PAREP_MPI_VALIDATOR_RD_FD,F_GETFD);
		int rdvalid = (temp != -1) || (errno != EBADF);
		temp = fcntl(PAREP_MPI_VALIDATOR_WR_FD,F_GETFD);
		int wrvalid = (temp != -1) || (errno != EBADF);
		if(!rdvalid && !wrvalid) {
			if(pipefd[0] != PAREP_MPI_VALIDATOR_RD_FD) {
				int fdflags, flflags;
				fdflags = fcntl(pipefd[0],F_GETFD);
				assert(fdflags != -1);
				flflags = fcntl(pipefd[0],F_GETFL);
				assert(flflags != -1);
				dup2(pipefd[0],PAREP_MPI_VALIDATOR_RD_FD);
				close(pipefd[0]);
				pipefd[0] = PAREP_MPI_VALIDATOR_RD_FD;
				temp = fcntl(pipefd[0],F_SETFD,fdflags);
				assert(temp != -1);
				temp = fcntl(pipefd[0],F_SETFL,flflags);
				assert(temp != -1);
			}
			if(pipefd[1] != PAREP_MPI_VALIDATOR_WR_FD) {
				int fdflags, flflags;
				fdflags = fcntl(pipefd[1],F_GETFD);
				assert(fdflags != -1);
				flflags = fcntl(pipefd[1],F_GETFL);
				assert(flflags != -1);
				dup2(pipefd[1],PAREP_MPI_VALIDATOR_WR_FD);
				close(pipefd[1]);
				pipefd[1] = PAREP_MPI_VALIDATOR_WR_FD;
				temp = fcntl(pipefd[1],F_SETFD,fdflags);
				assert(temp != -1);
				temp = fcntl(pipefd[1],F_SETFL,flflags);
				assert(temp != -1);
			}
		}
	}
	return result;
}

int fclose(FILE *stream) {
	if(_real_fclose == NULL) _real_fclose = dlsym(RTLD_NEXT,"fclose");
	int result;
	if(parep_mpi_initialized) {
		pthread_mutex_lock(&openFileListLock);
		openFileNode *onode = openFileListFind(stream->_fileno);
		openFileListDelete(onode);
		pthread_mutex_unlock(&openFileListLock);
	}
	//if(parep_mpi_initialized) use_common_heap = true;
	result = _real_fclose(stream);
	//if(parep_mpi_initialized) use_common_heap = false;
	return result;
}

FILE *fopen(const char *__restrict__filename, const char *__restrict__mode) {
	if(_real_fopen == NULL) _real_fopen = dlsym(RTLD_NEXT,"fopen");
	FILE *result;
	//if(parep_mpi_initialized) use_common_heap = true;
	result = _real_fopen(__restrict__filename,__restrict__mode);
	//if(parep_mpi_initialized) use_common_heap = false;
	if(result == NULL) return result;
	if(parep_mpi_initialized) {
		int targfd = PAREP_MPI_EXT_FILE_FD - 1;
		bool fd_open = false;
		do {
			fd_open = false;
			targfd++;
			errno = 0;
			int fcntlret = fcntl(targfd,F_GETFD);
			if(fcntlret != -1 || errno != EBADF) fd_open = true;
		} while(fd_open);
		dup2(result->_fileno,targfd);
		close(result->_fileno);
		result->_fileno = targfd;
		
		pthread_mutex_lock(&openFileListLock);
		openFileListInsert(result->_fileno,(char *)__restrict__filename,(char *)__restrict__mode,result,false);
		pthread_mutex_unlock(&openFileListLock);
	}
	return result;
}

FILE *fopen64(const char *__restrict__filename, const char *__restrict__mode) {
	if(_real_fopen64 == NULL) _real_fopen64 = dlsym(RTLD_NEXT,"fopen64");
	FILE *result;
	//if(parep_mpi_initialized) use_common_heap = true;
	result = _real_fopen64(__restrict__filename,__restrict__mode);
	//if(parep_mpi_initialized) use_common_heap = false;
	if(result == NULL) return result;
	if(parep_mpi_initialized) {
		int targfd = PAREP_MPI_EXT_FILE_FD - 1;
		bool fd_open = false;
		do {
			fd_open = false;
			targfd++;
			errno = 0;
			int fcntlret = fcntl(targfd,F_GETFD);
			if(fcntlret != -1 || errno != EBADF) fd_open = true;
		} while(fd_open);
		dup2(result->_fileno,targfd);
		close(result->_fileno);
		result->_fileno = targfd;
		
		pthread_mutex_lock(&openFileListLock);
		openFileListInsert(result->_fileno,(char *)__restrict__filename,(char *)__restrict__mode,result,true);
		pthread_mutex_unlock(&openFileListLock);
	}
	return result;
}

int open(const char *pathname, int flags, ...) {
	if(_real_open == NULL) _real_open = dlsym(RTLD_NEXT,"open");
	int result;
	va_list args;
	va_start(args,flags);
	if(flags & O_CREAT) {
		mode_t mode = va_arg(args,mode_t);
		result = _real_open(pathname,flags,mode);
	} else {
		result = _real_open(pathname,flags);
	}
	va_end(args);
	
	if(strstr(pathname,"ib_shmem")) {
		dup2(result,PAREP_MPI_IB_SHMEM_FD);
		close(result);
		result = PAREP_MPI_IB_SHMEM_FD;
	} else if(strstr(pathname,"ib_pool")) {
		dup2(result,PAREP_MPI_IB_POOL_FD);
		close(result);
		result = PAREP_MPI_IB_POOL_FD;
	}
	
	return result;
}

struct mallinfo mallinfo(void) {
	if(_real_mallinfo == NULL) _real_mallinfo = dlsym(RTLD_NEXT,"mallinfo");
	struct mallinfo retval;
	PAREP_MPI_DISABLE_CKPT();
	if(_ext_mallinfo == NULL) retval = _real_mallinfo();
	else retval = _ext_mallinfo();
	PAREP_MPI_ENABLE_CKPT();
	return retval;
}

void free(void *ptr) {
	if(_real_free == NULL) _real_free = dlsym(RTLD_NEXT,"free");
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) parep_mpi_sighandling_state = 1;
	PAREP_MPI_DISABLE_CKPT();
	if(parep_mpi_completed) return;
	if(((parep_mpi_internal_call) || (pthread_self() != thread_tid[0]) || (ptr < parep_mpi_ext_heap_mapping) || (ptr >= (parep_mpi_ext_heap_mapping + parep_mpi_ext_heap_size))) && (!use_common_heap)) {
		if(_ext_free == NULL) _real_free(ptr);
		else _ext_free(ptr);
	} else {
		parep_mpi_free(ptr);
	}
	PAREP_MPI_ENABLE_CKPT();
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) {
		if(parep_mpi_sighandling_state == 2) {
			parep_mpi_sighandling_state = 0;
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
		}
		parep_mpi_sighandling_state = 0;
	}
}

void *malloc(size_t size) {
	if(_real_malloc == NULL) _real_malloc = dlsym(RTLD_NEXT,"malloc");
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) parep_mpi_sighandling_state = 1;
	PAREP_MPI_DISABLE_CKPT();
	void *retval;
	if(((parep_mpi_internal_call) || (pthread_self() != thread_tid[0]))  && (!use_common_heap)) {
		if(_ext_malloc == NULL) retval = _real_malloc(size);
		else retval = _ext_malloc(size);
	} else {
		retval = parep_mpi_malloc(size);
	}
	PAREP_MPI_ENABLE_CKPT();
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) {
		if(parep_mpi_sighandling_state == 2) {
			parep_mpi_sighandling_state = 0;
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
		}
		parep_mpi_sighandling_state = 0;
	}
	return retval;
}

static bool dlsym_active = false;
static unsigned char calloc_buf[8192];

void *calloc(size_t nmemb, size_t size) {
	if(_real_calloc == NULL) {
		if(dlsym_active) {
			return calloc_buf;
		} else {
			dlsym_active = true;
			_real_calloc = dlsym(RTLD_NEXT,"calloc");
			dlsym_active = false;
		}
	}
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) parep_mpi_sighandling_state = 1;
	PAREP_MPI_DISABLE_CKPT();
	void *retval;
	if(((parep_mpi_internal_call) || (pthread_self() != thread_tid[0])) && (!use_common_heap)) {
		if(_ext_calloc == NULL) retval = _real_calloc(nmemb, size);
		else retval = _ext_calloc(nmemb, size);
	} else {
		retval = parep_mpi_calloc(nmemb, size);
	}
	PAREP_MPI_ENABLE_CKPT();
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) {
		if(parep_mpi_sighandling_state == 2) {
			parep_mpi_sighandling_state = 0;
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
		}
		parep_mpi_sighandling_state = 0;
	}
	return retval;
}

void *realloc(void *ptr, size_t size) {
	if(_real_realloc == NULL) _real_realloc = dlsym(RTLD_NEXT,"realloc");
	PAREP_MPI_DISABLE_CKPT();
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) parep_mpi_sighandling_state = 1;
	void *retval;
	if(((parep_mpi_internal_call) || (pthread_self() != thread_tid[0]) || (ptr < parep_mpi_ext_heap_mapping) || (ptr >= (parep_mpi_ext_heap_mapping + parep_mpi_ext_heap_size)))&& (!use_common_heap)) {
		if(_ext_realloc == NULL) retval = _real_realloc(ptr, size);
		else retval = _ext_realloc(ptr, size);
	} else {
		retval = parep_mpi_realloc(ptr, size);
	}
	PAREP_MPI_ENABLE_CKPT();
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) {
		if(parep_mpi_sighandling_state == 2) {
			parep_mpi_sighandling_state = 0;
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
		}
		parep_mpi_sighandling_state = 0;
	}
	return retval;
}

char *strdup(const char *str) {
	if(_real_strdup == NULL) _real_strdup = dlsym(RTLD_NEXT,"strdup");
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) parep_mpi_sighandling_state = 1;
	PAREP_MPI_DISABLE_CKPT();
	char *retval;
	if(((parep_mpi_internal_call) || (pthread_self() != thread_tid[0])) && (!use_common_heap)) {
		retval = _real_strdup(str);
	} else {
		retval = parep_mpi_strdup(str);
	}
	PAREP_MPI_ENABLE_CKPT();
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) {
		if(parep_mpi_sighandling_state == 2) {
			parep_mpi_sighandling_state = 0;
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
		}
		parep_mpi_sighandling_state = 0;
	}
	return retval;
}

char *strndup(const char *str, size_t size) {
	if(_real_strndup == NULL) _real_strndup = dlsym(RTLD_NEXT,"strndup");
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) parep_mpi_sighandling_state = 1;
	PAREP_MPI_DISABLE_CKPT();
	char *retval;
	if(((parep_mpi_internal_call) || (pthread_self() != thread_tid[0])) && (!use_common_heap)) {
		retval = _real_strndup(str,size);
	} else {
		retval = parep_mpi_strndup(str,size);
	}
	PAREP_MPI_ENABLE_CKPT();
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) {
		if(parep_mpi_sighandling_state == 2) {
			parep_mpi_sighandling_state = 0;
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
		}
		parep_mpi_sighandling_state = 0;
	}
	return retval;
}

char *realpath(const char *path, char *resolved_path) {
	if(_real_realpath == NULL) _real_realpath = dlsym(RTLD_NEXT,"realpath");
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) parep_mpi_sighandling_state = 1;
	PAREP_MPI_DISABLE_CKPT();
	char *retval;
	if(((parep_mpi_internal_call) || (pthread_self() != thread_tid[0])) && (!use_common_heap)) {
		retval = _real_realpath(path,resolved_path);
	} else {
		retval = parep_mpi_realpath(path,resolved_path);
	}
	PAREP_MPI_ENABLE_CKPT();
	if((pthread_self() == thread_tid[0]) && (!parep_mpi_internal_call)) {
		if(parep_mpi_sighandling_state == 2) {
			parep_mpi_sighandling_state = 0;
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
		}
		parep_mpi_sighandling_state = 0;
	}
	return retval;
}

void *memalign(size_t boundary, size_t size)
{
	if(_real_memalign == NULL) _real_memalign = dlsym(RTLD_NEXT,"memalign");
	PAREP_MPI_DISABLE_CKPT();
	void *retval;
	if(_ext_memalign == NULL) retval = _real_memalign(boundary, size);
	else retval = _ext_memalign(boundary, size);
	PAREP_MPI_ENABLE_CKPT();
	return retval;
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	if(_real_posix_memalign == NULL) _real_posix_memalign = dlsym(RTLD_NEXT,"posix_memalign");
	PAREP_MPI_DISABLE_CKPT();
	int retval = _real_posix_memalign(memptr, alignment, size);
	PAREP_MPI_ENABLE_CKPT();
	return retval;
}

void *valloc(size_t size)
{
	if(_real_valloc == NULL) _real_valloc = dlsym(RTLD_NEXT,"valloc");
	PAREP_MPI_DISABLE_CKPT();
	void *retval;
	if(_ext_valloc == NULL) retval = _real_valloc(size);
	else retval = _ext_valloc(size);
	PAREP_MPI_ENABLE_CKPT();
	return retval;
}

void *dlopen(const char *filename, int flags) {
	void *handle;
	if(_real_dlopen == NULL) _real_dlopen = dlsym(RTLD_NEXT,"dlopen");
	if(filename) {
		if (strstr(filename, "libibverbs.so")) {
			char mylib[500];
			sprintf(mylib,"%s/lib/libmpi.so",getenv("PAREP_MPI_PATH"));
			//PAREP_MPI_DISABLE_CKPT_DL();
			handle = _real_dlopen(mylib,flags);
			//PAREP_MPI_ENABLE_CKPT_DL();
			return handle;
		}
	}
	//PAREP_MPI_DISABLE_CKPT_DL();
	handle = _real_dlopen(filename,flags);
	//PAREP_MPI_ENABLE_CKPT_DL();
	return handle;
}

void *wake_from_wait(void *arg) {
	while(end_wake_thread == 0) {
		pthread_mutex_lock(&reqListLock);
		pthread_cond_signal(&reqListCond);
		pthread_mutex_unlock(&reqListLock);
	}
	end_wake_thread = 0;
	return NULL;
}

void signal_handler(int signum, siginfo_t *siginfo, void *context) {
	bool int_call;
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	int tid_index;
	int restore = false;
	bool bcast = false;
	bool wrlockheld = false;
	pthread_mutex_lock(&num_thread_mutex);
	if(!signal_bcasted) {
		assert(pthread_self() == thread_tid[0]);
		if(parep_mpi_sighandling_state == 1) {
			parep_mpi_sighandling_state = 2;
			pthread_mutex_unlock(&num_thread_mutex);
			if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
			return;
		} else if(parep_mpi_sighandling_state == 3) {
			parep_mpi_sighandling_state = 2;
			pthread_mutex_unlock(&num_thread_mutex);
			//_real_pthread_create(&waitWaker,NULL,wake_from_wait,NULL);
			if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
			return;
		}
		
		int myrank;
		if(___ckpt_counter < 0) myrank = parep_mpi_rank;
		else EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
		
		EMPI_Request req;
		int ret = 0;
		/*do {
			ret = 0;
			EMPI_Ibarrier(MPI_COMM_WORLD->eworldComm,&req);
			int flag = 0;
			do {
				if(parep_mpi_failed_proc_recv) {
					EMPI_Cancel(&req);
					EMPI_Request_free(&req);
					
					while(parep_mpi_failed_proc_recv);
					
					EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
					ret = 1;
					break;
				}
				EMPI_Test(&req,&flag,EMPI_STATUS_IGNORE);
			} while(flag == 0);
		} while(ret == 1);*/
		
		if(!parep_mpi_swap_replicas) pthread_rwlock_wrlock(&wrapperLock);
		
		pthread_mutex_lock(&reqListLock);
		
		if(!parep_mpi_swap_replicas) pthread_rwlock_unlock(&wrapperLock);
		//1. Test all current requests if complete
		if(___ckpt_counter >= 0) test_all_requests();
		
		bool progressed;
		
		pthread_rwlock_rdlock(&commLock);
		if(___ckpt_counter >= 0) {
			//2. Probe for any receives. CMP from only CMP. REP from everywhere.
			if(myrank >= nC) {
				for(int j = 0; j < nC+nR; j++) {
					if((j != myrank) && (j != repToCmpMap[myrank-nC])) {
						int flag = 0;
						EMPI_Status stat;
						do {
							MPI_Comm comm = MPI_COMM_WORLD;
							if(j < nC) EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
							else EMPI_Iprobe(j-nC,EMPI_ANY_TAG,comm->EMPI_COMM_REP,&flag,&stat);
							if(flag) {
								pthread_rwlock_unlock(&commLock);
								do {
									progressed = test_all_ptp_requests();
								} while(progressed);
								pthread_rwlock_rdlock(&commLock);
								if(j < nC) EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_CMP_REP_INTERCOMM,&flag,&stat);
								else EMPI_Iprobe(j-nC,EMPI_ANY_TAG,comm->EMPI_COMM_REP,&flag,&stat);
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
								
								if(j < nC) EMPI_Recv(curargs->buf,count+extracount,EMPI_BYTE,j,stat.EMPI_TAG,comm->EMPI_CMP_REP_INTERCOMM,&stat);
								else EMPI_Recv(curargs->buf,count+extracount,EMPI_BYTE,j-nC,stat.EMPI_TAG,comm->EMPI_COMM_REP,&stat);
								
								memcpy(&(curargs->id),(curargs->buf) + (count * size),sizeof(int));
								if((curargs->id & 0xF0000000) != 0x70000000) {
									printf("%d: Wrong id probed at sighandler %p myrank %d src %d\n",getpid(),curargs->id,myrank,j);
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
									if(j >= nC) status.MPI_SOURCE = repToCmpMap[stat.EMPI_SOURCE];
									
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
			} else {
				for(int j = 0; j < nC; j++) {
					if(j != myrank) {
						int flag = 0;
						EMPI_Status stat;
						do {
							MPI_Comm comm = MPI_COMM_WORLD;
							EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_COMM_CMP,&flag,&stat);
							if(flag) {
								pthread_rwlock_unlock(&commLock);
								do {
									progressed = test_all_ptp_requests();
								} while(progressed);
								pthread_rwlock_rdlock(&commLock);
								EMPI_Iprobe(j,EMPI_ANY_TAG,comm->EMPI_COMM_CMP,&flag,&stat);
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
								
								EMPI_Recv(curargs->buf,count+extracount,EMPI_BYTE,j,stat.EMPI_TAG,comm->EMPI_COMM_CMP,&stat);
								
								memcpy(&(curargs->id),(curargs->buf) + (count * size),sizeof(int));
								if((curargs->id & 0xF0000000) != 0x70000000) {
									printf("%d: Wrong id probed at sighandler %p myrank %d src %d\n",getpid(),curargs->id,myrank,j);
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
				}
			}
			
			pthread_rwlock_unlock(&commLock);
			probe_reduce_messages();
			pthread_rwlock_rdlock(&commLock);
			
			do {
				ret = 0;
				EMPI_Ibarrier(MPI_COMM_WORLD->eworldComm,&req);
				int flag = 0;
				do {
					if(parep_mpi_failed_proc_recv) {
						//pthread_rwlock_unlock(&wrapperLock);
						pthread_rwlock_unlock(&commLock);
						pthread_mutex_unlock(&reqListLock);
						
						//EMPI_Cancel(&req);
						//EMPI_Request_free(&req);
						
						while(parep_mpi_failed_proc_recv);
						
						//pthread_rwlock_wrlock(&wrapperLock);
						pthread_mutex_lock(&reqListLock);
						test_all_requests();
						EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
						pthread_rwlock_rdlock(&commLock);
						ret = 1;
						break;
					}
					EMPI_Test(&req,&flag,EMPI_STATUS_IGNORE);
				} while(flag == 0);
			} while(ret == 1);
			
			pthread_mutex_lock(&peertopeerLock);
			pthread_mutex_lock(&collectiveLock);
			//handle_rem_recv();
			handle_rem_recv_no_poll();
			pthread_cond_signal(&collectiveCond);
			pthread_cond_signal(&peertopeerCond);
			pthread_mutex_unlock(&collectiveLock);
			pthread_mutex_unlock(&peertopeerLock);
			
			do {
				ret = 0;
				EMPI_Ibarrier(MPI_COMM_WORLD->eworldComm,&req);
				int flag = 0;
				do {
					if(parep_mpi_failed_proc_recv) {
						//pthread_rwlock_unlock(&wrapperLock);
						pthread_rwlock_unlock(&commLock);
						pthread_mutex_unlock(&reqListLock);
						
						//EMPI_Cancel(&req);
						//EMPI_Request_free(&req);
						
						while(parep_mpi_failed_proc_recv);
						
						//pthread_rwlock_wrlock(&wrapperLock);
						pthread_mutex_lock(&reqListLock);
						test_all_requests();
						EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
						pthread_rwlock_rdlock(&commLock);
						ret = 1;
						break;
					}
					EMPI_Test(&req,&flag,EMPI_STATUS_IGNORE);
				} while(flag == 0);
			} while(ret == 1);
			
			pthread_mutex_lock(&peertopeerLock);
			pthread_mutex_lock(&collectiveLock);
			ptpdata *pdata = first_peertopeer;
			while(pdata != NULL) {
				if(pdata->type == MPI_FT_RECV) {
					if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
						if(pdata->markdelcmp && pdata->markdelrep) {
							struct peertopeer_data *deldata = pdata;
							pdata = pdata->prev;
							
							if(deldata == first_peertopeer) {
								first_peertopeer = first_peertopeer->prev;
							}
							if(deldata == last_peertopeer) {
								last_peertopeer = last_peertopeer->next;
							}
							
							if(deldata->next != NULL) deldata->next->prev = deldata->prev;
							if(deldata->prev != NULL) deldata->prev->next = deldata->next;
							
							parep_mpi_free(deldata);
							
							continue;
						}
					}
				} else if(pdata->type == MPI_FT_SEND) {
					if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
						if(pdata->markdelcmp && pdata->markdelrep) {
							struct peertopeer_data *deldata = pdata;
							pdata = pdata->prev;
							
							if(deldata == first_peertopeer) {
								first_peertopeer = first_peertopeer->prev;
							}
							if(deldata == last_peertopeer) {
								last_peertopeer = last_peertopeer->next;
							}
							
							if(deldata->next != NULL) deldata->next->prev = deldata->prev;
							if(deldata->prev != NULL) deldata->prev->next = deldata->next;
							
							int size;
							int count, extracount;
							EMPI_Type_size(deldata->dt->edatatype,&size);
							if(size >= sizeof(int)) extracount = 1;
							else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
							else extracount = (((int)sizeof(int))/size) + 1;
							count = deldata->count;
							
							long delsize = (count+extracount)*size;
							
							if(deldata->buf != NULL) {
								parep_mpi_free(deldata->buf);
								deldata->buf = NULL;
								pthread_mutex_lock(&parep_mpi_store_buf_sz_mutex);
								parep_mpi_store_buf_sz = parep_mpi_store_buf_sz - delsize;
								pthread_cond_signal(&parep_mpi_store_buf_sz_cond);
								pthread_mutex_unlock(&parep_mpi_store_buf_sz_mutex);
							}
							
							parep_mpi_free(deldata);
							
							continue;
						}
					}
				}
				pdata = pdata->prev;
			}
			
			clcdata *cdata = last_collective;
			while(cdata != NULL) {
				if(cdata->id == -1) {
					struct collective_data *deldata = cdata;
					cdata = cdata->next;
					
					if(deldata == last_collective) {
						last_collective = last_collective->next;
					}
					
					if(deldata->next != NULL) deldata->next->prev = deldata->prev;
					if(deldata->prev != NULL) deldata->prev->next = deldata->next;
					
					if(deldata->type == MPI_FT_BCAST) {
						if((deldata->args).bcast.buf != NULL) parep_mpi_free((deldata->args).bcast.buf);
					} else if(deldata->type == MPI_FT_SCATTER) {
						if((deldata->args).scatter.sendbuf != NULL) parep_mpi_free((deldata->args).scatter.sendbuf);
						//parep_mpi_free((deldata->args).scatter.recvbuf);
					} else if(deldata->type == MPI_FT_GATHER) {
						parep_mpi_free((deldata->args).gather.sendbuf);
						//parep_mpi_free((deldata->args).gather.recvbuf);
					} else if(deldata->type == MPI_FT_REDUCE) {
						parep_mpi_free((deldata->args).reduce.sendbuf);
						if((deldata->args).reduce.alloc_recvbuf) parep_mpi_free((deldata->args).reduce.recvbuf);
					} else if(deldata->type == MPI_FT_ALLGATHER) {
						parep_mpi_free((deldata->args).allgather.sendbuf);
						//parep_mpi_free((deldata->args).allgather.recvbuf);
					} else if(deldata->type == MPI_FT_ALLTOALL) {
						parep_mpi_free((deldata->args).alltoall.sendbuf);
						//parep_mpi_free((deldata->args).alltoall.recvbuf);
					} else if(deldata->type == MPI_FT_ALLTOALLV) {
						parep_mpi_free((deldata->args).alltoallv.sendbuf);
						//parep_mpi_free((deldata->args).alltoallv.recvbuf);
						parep_mpi_free((deldata->args).alltoallv.sendcounts);
						parep_mpi_free((deldata->args).alltoallv.recvcounts);
						parep_mpi_free((deldata->args).alltoallv.sdispls);
						parep_mpi_free((deldata->args).alltoallv.rdispls);
					} else if(deldata->type == MPI_FT_ALLREDUCE) {
						int size;
						EMPI_Type_size((deldata->args).allreduce.dt->edatatype,&size);
						long delsize = 2 * (deldata->args).allreduce.count * size;
						parep_mpi_free((deldata->args).allreduce.sendbuf);
						if((deldata->args).allreduce.alloc_recvbuf) parep_mpi_free((deldata->args).allreduce.recvbuf);
						pthread_mutex_lock(&parep_mpi_store_buf_sz_mutex);
						parep_mpi_store_buf_sz = parep_mpi_store_buf_sz - delsize;
						pthread_cond_signal(&parep_mpi_store_buf_sz_cond);
						pthread_mutex_unlock(&parep_mpi_store_buf_sz_mutex);
					}
					
					parep_mpi_free(deldata);
					
					continue;
				}
				cdata = cdata->next;
			}
			pthread_cond_signal(&collectiveCond);
			pthread_cond_signal(&peertopeerCond);
			pthread_mutex_unlock(&collectiveLock);
			pthread_mutex_unlock(&peertopeerLock);
		}
		if(parep_mpi_swap_replicas) {
			parep_mpi_swap_replicas = 0;
			pthread_rwlock_unlock(&commLock);
			pthread_mutex_unlock(&reqListLock);
			pthread_mutex_lock(&procmaplock);
			MPI_Replica_rearrange(ctrmap,rtcmap);
			_real_free(ctrmap);
			_real_free(rtcmap);
			pthread_mutex_unlock(&procmaplock);
			pthread_mutex_lock(&reqListLock);
			pthread_rwlock_rdlock(&commLock);
			pthread_mutex_unlock(&num_thread_mutex);
			pthread_rwlock_unlock(&commLock);
			pthread_mutex_unlock(&reqListLock);
			if(parep_mpi_ckpt_wait) parep_mpi_ckpt_wait = 0;
			if(parep_mpi_restore) parep_mpi_restore = 0;
			if(parep_mpi_restore_wait) parep_mpi_restore_wait = 0;
			if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
			return;
		}/* else if (parep_mpi_restart_replicas) {
			parep_mpi_restart_replicas = 0;
			pthread_mutex_unlock(&num_thread_mutex);
			pthread_rwlock_unlock(&commLock);
			pthread_mutex_unlock(&reqListLock);
			if(parep_mpi_ckpt_wait) parep_mpi_ckpt_wait = 0;
			if(parep_mpi_restore) parep_mpi_restore = 0;
			if(parep_mpi_restore_wait) parep_mpi_restore_wait = 0;
			if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
			return;
		}*/
		signal_bcasted = true;
		bcast = true;
		pthread_rwlock_wrlock(&wrapperLock);
		wrlockheld = true;
	}
	pthread_mutex_unlock(&num_thread_mutex);
	
	for(int i = 0; i < num_threads; i++) {
		if(pthread_self() != thread_tid[i]) {
			if(bcast) {
				pthread_kill(thread_tid[i],signum);
			}
		} else {
			tid_index = i;
		}
	}
	
	restore = parep_mpi_restore;
	
	if(pthread_self() == thread_tid[0]) {
		pthread_mutex_lock(&entered_sighandler_mutex);
		for(int i = 1; i < num_threads; i++) {
			while(!entered_sighandler[i]) {
				pthread_cond_wait(&entered_sighandler_cond,&entered_sighandler_mutex);
			}
		}
		pthread_mutex_unlock(&entered_sighandler_mutex);
		for(int i = 1; i < num_threads; i++) {
			entered_sighandler[i] = false;
		}
		threadholdswrlock = true;
	} else {
		pthread_mutex_lock(&entered_sighandler_mutex);
		entered_sighandler[tid_index] = true;
		pthread_cond_signal(&entered_sighandler_cond);
		pthread_mutex_unlock(&entered_sighandler_mutex);
	}
	kernel_tid[tid_index] = *((pid_t *)(pthread_self() + 0x2d0));
	
	openFileNode *onode = openFileHead;
	while(onode != NULL) {
		fsync(onode->fd);
		onode = onode->next;
	}
	
	if(pthread_self() == thread_tid[0]) parep_mpi_sigusr1_hook(tid_index,true,restore);
	else parep_mpi_sigusr1_hook(tid_index,false,restore);
	
	if(wrlockheld) {
		pthread_rwlock_unlock(&wrapperLock);
		wrlockheld = false;
	}
	threadholdswrlock = false;
	*((pid_t *)(((address)(pthread_self())) + ((address)(0x2d0)))) = kernel_tid[tid_index];
	syscall(SYS_set_tid_address,((int *)(pthread_self() + 0x2d0)));
	
	address fsbase;
	//pid_t fsbasepid;
	syscall(SYS_arch_prctl,ARCH_GET_FS,&fsbase);
	*((pid_t *)(fsbase+0x2d4)) = kernel_tid[0];
	//printf("%d: Pid loc check %d kernel_tid %d\n",getpid(),fsbasepid,kernel_tid[tid_index]);
	
	if(pthread_self() == thread_tid[0]) {
		bool restore_case = parep_mpi_restore > 0;
		if(restore_case) parep_mpi_restore = 0;
		pthread_mutex_lock(&entered_sighandler_mutex);
		for(int i = 1; i < num_threads; i++) {
			if((daemon_poller == thread_tid[i]) || (!restore_case)) {
				while(!entered_sighandler[i]) {
					pthread_cond_wait(&entered_sighandler_cond,&entered_sighandler_mutex);
				}
			}
		}
		pthread_mutex_unlock(&entered_sighandler_mutex);
		signal_bcasted = false;
		for(int i = 1; i < num_threads; i++) {
			entered_sighandler[i] = false;
		}
		
		pthread_mutex_lock(&daemon_poller_free_mutex);
		daemon_poller_free = true;
		pthread_cond_signal(&daemon_poller_free_cond);
		pthread_mutex_unlock(&daemon_poller_free_mutex);
		
		pthread_rwlock_unlock(&commLock);
		pthread_mutex_unlock(&reqListLock);
	} else {
		pthread_mutex_lock(&entered_sighandler_mutex);
		entered_sighandler[tid_index] = true;
		pthread_cond_signal(&entered_sighandler_cond);
		pthread_mutex_unlock(&entered_sighandler_mutex);
		
		if(pthread_self() == daemon_poller) {
			pthread_mutex_lock(&daemon_poller_free_mutex);
			while(!daemon_poller_free) {
				pthread_cond_wait(&daemon_poller_free_cond,&daemon_poller_free_mutex);
			}
			daemon_poller_free = false;
			pthread_mutex_unlock(&daemon_poller_free_mutex);
		}
		
	}
	
	set_signal_handler(SIGUSR1);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
}

void sigsegv_handler(int signum, siginfo_t *siginfo, void *context) {
	printf("%d: sigsegv at %p parep_mpi_store_buf_sz %p\n",getpid(),siginfo->si_addr,parep_mpi_store_buf_sz);
	void *bt[1024];
	int bt_size;
	char **bt_syms;
	int i;
	bt_size = backtrace(bt, 1024);
	bt_syms = backtrace_symbols(bt, bt_size);
	for (i = 0; i < bt_size; i++) {
		printf("%d: %s\n",getpid(),bt_syms[i]);
	}
	free(bt_syms);
}

void set_signal_handler(int signal) {
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, signal);
	pthread_sigmask(SIG_UNBLOCK, &mask, NULL);

	struct sigaction action;
	action.sa_sigaction = signal_handler;
	action.sa_flags = SA_SIGINFO;
	sigaction(signal, &action, NULL);
	
	action.sa_sigaction = sigsegv_handler;
	sigaction(SIGSEGV, &action, NULL);
}

void *start_routine_intercept(void *argument) {
	int tid_index;
	pthread_mutex_lock(&num_thread_mutex);
	tid_index = num_threads;
	num_threads++;
	pthread_mutex_unlock(&num_thread_mutex);
	thread_tid[tid_index] = pthread_self();
	pcin_arg_t *args = (pcin_arg_t *)argument;
	void *(*real_start_routine)(void *) = args->start_routine;
	void *restrict real_arg = args->arg;
	free(args);
	set_signal_handler(SIGUSR1);
	return real_start_routine(real_arg);
}

int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg) {
	int retval;
	if(_real_pthread_create == NULL) _real_pthread_create = dlsym(RTLD_NEXT,"pthread_create");
	if(_real_malloc == NULL) _real_malloc = dlsym(RTLD_NEXT,"malloc");
	pcin_arg_t *args_intercept = _real_malloc(sizeof(pcin_arg_t));
	args_intercept->start_routine = start_routine;
	args_intercept->arg = arg;
	retval = _real_pthread_create(thread,attr,start_routine_intercept,args_intercept);
	return retval;
}

/*int dlclose(void *handle)
{
	//PAREP_MPI_DISABLE_CKPT_DL();
	int ret = _real_dlclose(handle);
	//PAREP_MPI_ENABLE_CKPT_DL();
	return ret;
}*/