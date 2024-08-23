//#define _GNU_SOURCE
#ifndef _MPI_INTERNAL_H_
#define _MPI_INTERNAL_H_
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include<dlfcn.h>
#include<unistd.h>
#include<signal.h>
#include<stddef.h>
#include<stdbool.h>

#include <stdint.h>
#include <limits.h>

#include "mpiProfilerdefs.h"

#include <setjmp.h>
#include <errno.h>
#include <syscall.h>
#include <string.h>
#include <sys/types.h>

#include<poll.h>
#include <assert.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <asm/prctl.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <link.h>
#include <ucontext.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

//#include <sys/mman.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifdef HEAPTRACK

void *__real_malloc(size_t size);

void *__wrap_malloc(size_t size);

#endif

#define MAX_CKPT 100
#define REP_DEGREE 2
#define CMP_PER_REP 1

#define PAREP_MPI_SAFE_FD 820

int (*EMPI_Init)(int *, char ***);
int (*EMPI_Finalize)();
int (*EMPI_Comm_size)(EMPI_Comm,int *);
int (*EMPI_Comm_remote_size)(EMPI_Comm,int *);
int (*EMPI_Comm_rank)(EMPI_Comm,int *);
int (*EMPI_Send)(void *,int,EMPI_Datatype,int,int,EMPI_Comm);
int (*EMPI_Recv)(void *,int,EMPI_Datatype,int,int,EMPI_Comm,EMPI_Status *);
int (*EMPI_Comm_free)(EMPI_Comm *);
int (*EMPI_Comm_dup)(EMPI_Comm,EMPI_Comm *);
int (*EMPI_Comm_spawn)(const char *,char **,int,EMPI_Info,int,EMPI_Comm,int *);
int (*EMPI_Barrier)(EMPI_Comm);
int (*EMPI_Abort)(EMPI_Comm,int);
int (*EMPI_Comm_group)(EMPI_Comm,EMPI_Group *);
int (*EMPI_Group_size)(EMPI_Group,int *);
int (*EMPI_Group_incl)(EMPI_Group,int,int *,EMPI_Group *);
int (*EMPI_Group_union)(EMPI_Group,EMPI_Group,EMPI_Group *);
int (*EMPI_Group_intersection)(EMPI_Group,EMPI_Group,EMPI_Group *);
int (*EMPI_Group_difference)(EMPI_Group,EMPI_Group,EMPI_Group *);
int (*EMPI_Group_translate_ranks)(EMPI_Group,int,int *,EMPI_Group,int *);
int (*EMPI_Comm_create_group)(EMPI_Comm,EMPI_Group,int,EMPI_Comm *);
int (*EMPI_Comm_split)(EMPI_Comm,int,int,EMPI_Comm *);
int (*EMPI_Intercomm_create)(EMPI_Comm,int,EMPI_Comm,int,int,EMPI_Comm *);
int (*EMPI_Comm_set_name)(EMPI_Comm,const char *);
int (*EMPI_Bcast)(void *,int,EMPI_Datatype,int,EMPI_Comm);
int (*EMPI_Allgather)(void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm);
int (*EMPI_Alltoall)(void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm);
int (*EMPI_Allgatherv)(void *,int,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm);
int (*EMPI_Alltoallv)(void *,int *,int *,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm);
int (*EMPI_Allreduce)(void *,void *,int,EMPI_Datatype,EMPI_Op,EMPI_Comm);
int (*EMPI_Isend)(void *,int,EMPI_Datatype,int,int,EMPI_Comm,EMPI_Request *);
int (*EMPI_Irecv)(void *,int,EMPI_Datatype,int,int,EMPI_Comm,EMPI_Request *);
int (*EMPI_Ibcast)(void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *);
int (*EMPI_Iscatter)(const void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *);
int (*EMPI_Iscatterv)(const void *,int *,int *,EMPI_Datatype,void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *);
int (*EMPI_Igather)(const void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *);
int (*EMPI_Igatherv)(const void *,int,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *);
int (*EMPI_Ireduce)(void *,void *,int,EMPI_Datatype,EMPI_Op,int,EMPI_Comm,EMPI_Request *);
int (*EMPI_Iallgather)(const void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm,EMPI_Request *);
int (*EMPI_Iallgatherv)(const void *,int *,int *,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm,EMPI_Request *);
int (*EMPI_Ialltoall)(void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm,EMPI_Request *);
int (*EMPI_Ialltoallv)(void *,int *,int *,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm,EMPI_Request *);
int (*EMPI_Iallreduce)(void *,void *,int,EMPI_Datatype,EMPI_Op,EMPI_Comm,EMPI_Request *);
int (*EMPI_Ibarrier)(EMPI_Comm,EMPI_Request *);
int (*EMPI_Test)(EMPI_Request *,int *,EMPI_Status *);
int (*EMPI_Wait)(EMPI_Request *,EMPI_Status *);
int (*EMPI_Cancel)(EMPI_Request *);
int (*EMPI_Probe)(int,int,EMPI_Comm,EMPI_Status *);
int (*EMPI_Iprobe)(int,int,EMPI_Comm,int *,EMPI_Status *);
int (*EMPI_Request_free)(EMPI_Request *);
int (*EMPI_Type_size)(EMPI_Datatype,int *);
int (*EMPI_Get_count)(EMPI_Status *,EMPI_Datatype,int *);
double (*EMPI_Wtime)();

static FILE *logfile;
static EMPI_Comm eworldComm;
static EMPI_Comm EMPI_COMM_CMP, EMPI_COMM_REP, EMPI_CMP_REP_INTERCOMM;
int *repToCmpMap, *cmpToRepMap;
//extern int *repToCmpMap, *cmpToRepMap;
static char *workDir = "/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi", *repState = "repState";
static char *extLibstr = "/home/phd/21/cdsjsar/MVAPICH2/lib/libmpi.so";

pid_t parep_mpi_coordinator_pid;

#define CONFIG_FILE cfg
#define CP_FILE ckpt

#if MACHTYPE==ppc64 

  #define PTR_ENCRYPT(variable) \
          asm ("xorq %%fs:48, %0\n" \
               "rolq $17, %0" \
               : "=r" (variable) \
               : "0" (variable))

  #define PTR_DECRYPT(variable) \
          asm ("rorq $17, %0\n" \
               "xorq %%fs:48, %0" \
               : "=r" (variable) \
               : "0" (variable))
  #define PTR_DECRYPT_USING_KEY(variable,key) \
          asm ("rorq $17, %0\n" \
               "xorq %2, %0" \
               : "=r" (variable) \
               : "r" (variable), "r" (key))
#else
  #error "Please determine how to deal with PTR_ENCRYPT/PTR_DECRYPT."
#endif

#define PAGE_SIZE sysconf(_SC_PAGESIZE) //getpagesize()
#define PAGE_MASK (~(PAGE_SIZE - 1))

typedef long RAW_ADDR;

static int repRank = -1;
int nC,nR;

static long array_sum(const int *array,int size) {
	long sum = 0;
	for(int i = 0; i < size; i++) {
		sum += array[i];
	}
	return sum;
}

struct Barrier_args
{
  MPI_Comm comm;
};

struct Bcast_args {
  void *buf;
  int count;
  MPI_Datatype dt;
  int root;
  MPI_Comm comm;
};

struct Scatter_args {
  void *sendbuf;
  int sendcount;
	MPI_Datatype senddt;
	void *recvbuf;
	int recvcount;
	MPI_Datatype recvdt;
  int root;
  MPI_Comm comm;
};

struct Gather_args {
  void *sendbuf;
  int sendcount;
	MPI_Datatype senddt;
	void *recvbuf;
	int recvcount;
	MPI_Datatype recvdt;
  int root;
  MPI_Comm comm;
};

struct Reduce_args
{
  void *sendbuf;
  int count;
	MPI_Datatype dt;
	void *recvbuf;
  MPI_Op op;
	int root;
  //int reproot;
  MPI_Comm comm;
	bool alloc_recvbuf;
};

struct Allgather_args {
  void *sendbuf;
  int sendcount;
	MPI_Datatype senddt;
	void *recvbuf;
	int recvcount;
	MPI_Datatype recvdt;
  MPI_Comm comm;
};

struct Alltoall_args {
  void *sendbuf;
  int sendcount;
	MPI_Datatype senddt;
	void *recvbuf;
	int recvcount;
	MPI_Datatype recvdt;
  MPI_Comm comm;
};

struct Alltoallv_args {
  void *sendbuf;
  int *sendcounts;
	int *sdispls;
	MPI_Datatype senddt;
	void *recvbuf;
	int *recvcounts;
	int *rdispls;
	MPI_Datatype recvdt;
  MPI_Comm comm;
};

struct Allreduce_args {
	void *sendbuf;
	void *recvbuf;
	int count;
	MPI_Datatype dt;
	MPI_Op op;
	MPI_Comm comm;
	bool alloc_recvbuf;
};

enum comm_type {
	MPI_FT_SEND,
	MPI_FT_RECV,
	MPI_FT_BARRIER,
	MPI_FT_BCAST,
	MPI_FT_SCATTER,
	MPI_FT_GATHER,
	MPI_FT_REDUCE,
	MPI_FT_ALLGATHER,
	MPI_FT_ALLTOALL,
	MPI_FT_ALLREDUCE,
	MPI_FT_ALLTOALLV,
	MPI_FT_REDUCE_TEMP,
	MPI_FT_ALLREDUCE_TEMP
};

struct collective_data {
	int type;
	int id;
	bool completecmp;
	bool completerep;
	bool *completecolls;
	int num_colls;
	MPI_Request *req;
	union {
		struct Barrier_args barrier;
		struct Bcast_args bcast;
		struct Scatter_args scatter;
		struct Gather_args gather;
		struct Reduce_args reduce;
		struct Allgather_args allgather;
		struct Alltoall_args alltoall;
		struct Allreduce_args allreduce;
		struct Alltoallv_args alltoallv;
	} args;
	struct collective_data *next;
	struct collective_data *prev;
};

struct peertopeer_data {
	int id;
	int type;
	void *buf;
	int count;
	MPI_Datatype dt;
	int target;
	int tag;
	MPI_Comm comm;
	bool completecmp;
	bool completerep;
	bool markdelcmp;
	bool markdelrep;
	MPI_Request *req;
	struct peertopeer_data *next;
	struct peertopeer_data *prev;
};

typedef struct peertopeer_data ptpdata;
typedef struct collective_data clcdata;

static struct skiplist
{
	int id;
	struct skiplist *next;
} *skipcmplist = NULL, *skipreplist = NULL, *skipredlist = NULL;

/* MPI_REQ LIST BEGIN */

static int *internalranks;

static long dataOffset;
static long libOffset;
static bool libUsed;

static bool parep_mpi_rem_logs = true;
static bool parep_mpi_make_logs = true;

int num_failures;
double time_in_err_handler;

extern void parep_mpi_longjmp(jmp_buf,int) __attribute__((visibility("hidden")));

int parep_mpi_checkpoint_init(int);
void parep_mpi_checkpoint_restore();
void parep_mpi_restore_messages();

void replaceImage (int, RAW_ADDR, RAW_ADDR, RAW_ADDR, RAW_ADDR, RAW_ADDR/*, RAW_ADDR, RAW_ADDR, RAW_ADDR, RAW_ADDR, RAW_ADDR*/);
int sendData (int);
void sendImage (int, int);
void recvImage (int);
void repManager (void);

static char *logbuffer[100];
static int nbuf = 0;
static int nbufentries = 0;
static bool ondisk = false;
static int parentpid;

void GenerateSendLog(char *, int, void *, int, EMPI_Datatype, int, int, EMPI_Comm);
void GenerateRecvLog(char *, int, int, int, EMPI_Comm);

void GetFirstWord(char *string, char *first);
//void SendUsingLine(char *line, int *worldComm_ranks);
void GetDestAndId(char *line, int *id, int *dest);
void GetSrcAndId(char *line, int *id, int *src);
bool IsRecvComplete(int src, int id);

//static int closest_replica(int);
void mpi_ft_collective_from_args(struct collective_data *, int, int, int, int *, int *, int *);
void mpi_ft_free_older_collectives(struct collective_data *);

enum mpi_ft_request_type {
	MPI_FT_SEND_REQUEST,
	MPI_FT_RECV_REQUEST,
	MPI_FT_COLLECTIVE_REQUEST
};

void parep_mpi_send_replication_data(int);
void parep_mpi_recv_replication_data(int);

EMPI_Comm parep_mpi_original_comm;

/*typedef struct mpi_ft_comm_ptr {
	struct mpi_ft_comm *p;
} mpi_ft_comm_ptr;

typedef struct mpi_ft_datatype_ptr {
	struct mpi_ft_datatype *p;
} mpi_ft_datatype_ptr;

typedef struct mpi_ft_op_ptr {
	struct mpi_ft_op *p;
} mpi_ft_op_ptr;

mpi_ft_comm_ptr commarr[1024];
mpi_ft_datatype_ptr dtarr[1024];
mpi_ft_op_ptr oparr[1024];*/
MPI_Comm commarr[1024];
MPI_Datatype dtarr[1024];
MPI_Op oparr[1024];
MPI_Request reqarr[1024];
bool reqinuse[1024];

void initialize_mpi_variables()
int empi_comm_creation(int *,int *,int *,char ***,bool);
void initialize_common_heap_and_stack(int);

#define MPI_FT_COLLECTIVE_TAG 102536
#define MPI_FT_REDUCE_TAG MPI_FT_COLLECTIVE_TAG+1
#define MPI_FT_ALLREDUCE_TAG MPI_FT_COLLECTIVE_TAG+2

#define PAREP_MPI_COORDINATOR_SOCK_NUM 500
#define PAREP_MPI_IB_SHMEM_FD 501
#define PAREP_MPI_IB_POOL_FD 502
#define PAREP_MPI_VALIDATOR_RD_FD 503
#define PAREP_MPI_VALIDATOR_WR_FD 504
#define PAREP_MPI_PMI_FD 505

#define PAREP_MPI_EXT_FILE_FD 512

#ifdef __cplusplus
}
#endif
#endif