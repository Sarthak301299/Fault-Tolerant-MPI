//#define _GNU_SOURCE
#ifndef _MPI_INTERNAL_H_
#define _MPI_INTERNAL_H_
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
//#include<pmix.h>
//#include<prte.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/un.h>

//#include <sys/mman.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifdef HEAPTRACK

void *__real_malloc(size_t size);

void *__wrap_malloc(size_t size);

#endif
/*struct mpi_ft_comm mpi_ft_comm_null = {OMPI_COMM_NULL,OMPI_COMM_NULL,OMPI_COMM_NULL,OMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL};
struct mpi_ft_op mpi_ft_op_null = {OMPI_OP_NULL,EMPI_OP_NULL};//,NULL};
struct mpi_ft_datatype mpi_ft_datatype_null = {OMPI_DATATYPE_NULL,EMPI_DATATYPE_NULL,-1};
struct mpi_ft_comm mpi_ft_comm_world = {OMPI_COMM_WORLD,OMPI_COMM_NULL,OMPI_COMM_NULL,OMPI_COMM_NULL,EMPI_COMM_WORLD,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL};
struct mpi_ft_comm mpi_ft_comm_self = {OMPI_COMM_SELF,OMPI_COMM_NULL,OMPI_COMM_NULL,OMPI_COMM_NULL,EMPI_COMM_SELF,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL};
struct mpi_ft_datatype mpi_ft_datatype_char = {OMPI_CHAR,EMPI_CHAR,sizeof(char)};
struct mpi_ft_datatype mpi_ft_datatype_signed_char = {OMPI_SIGNED_CHAR,EMPI_SIGNED_CHAR,sizeof(signed char)};
struct mpi_ft_datatype mpi_ft_datatype_unsigned_char = {OMPI_UNSIGNED_CHAR,EMPI_UNSIGNED_CHAR,sizeof(unsigned char)};
struct mpi_ft_datatype mpi_ft_datatype_byte = {OMPI_BYTE,EMPI_BYTE,1};
struct mpi_ft_datatype mpi_ft_datatype_short = {OMPI_SHORT,EMPI_SHORT,sizeof(short)};
struct mpi_ft_datatype mpi_ft_datatype_unsigned_short = {OMPI_UNSIGNED_SHORT,EMPI_UNSIGNED_SHORT,sizeof(unsigned short)};
struct mpi_ft_datatype mpi_ft_datatype_int = {OMPI_INT,EMPI_INT,sizeof(int)};
struct mpi_ft_datatype mpi_ft_datatype_unsigned = {OMPI_UNSIGNED,EMPI_UNSIGNED,sizeof(unsigned int)};
struct mpi_ft_datatype mpi_ft_datatype_long = {OMPI_LONG,EMPI_LONG,sizeof(long)};
struct mpi_ft_datatype mpi_ft_datatype_unsigned_long = {OMPI_UNSIGNED_LONG,EMPI_UNSIGNED_LONG,sizeof(unsigned long)};
struct mpi_ft_datatype mpi_ft_datatype_float = {OMPI_FLOAT,EMPI_FLOAT,sizeof(float)};
struct mpi_ft_datatype mpi_ft_datatype_double = {OMPI_DOUBLE,EMPI_DOUBLE,sizeof(double)};
struct mpi_ft_datatype mpi_ft_datatype_long_double = {OMPI_LONG_DOUBLE,EMPI_LONG_DOUBLE,sizeof(long double)};
struct mpi_ft_datatype mpi_ft_datatype_long_long_int = {OMPI_LONG_LONG_INT,EMPI_LONG_LONG_INT,sizeof(long long int)};
struct mpi_ft_datatype mpi_ft_datatype_unsigned_long_long = {OMPI_UNSIGNED_LONG_LONG,EMPI_UNSIGNED_LONG_LONG,sizeof(unsigned long long)};
struct mpi_ft_op mpi_ft_op_max = {OMPI_MAX,EMPI_MAX};//,mpi_ft_max_op};
struct mpi_ft_op mpi_ft_op_min = {OMPI_MIN,EMPI_MIN};//,mpi_ft_min_op};
struct mpi_ft_op mpi_ft_op_sum = {OMPI_SUM,EMPI_SUM};//,mpi_ft_sum_op};
struct mpi_ft_op mpi_ft_op_prod = {OMPI_PROD,EMPI_PROD};//,mpi_ft_prod_op};
struct mpi_ft_op mpi_ft_op_land = {OMPI_LAND,EMPI_LAND};//,mpi_ft_land_op};
struct mpi_ft_op mpi_ft_op_band = {OMPI_BAND,EMPI_BAND};//,mpi_ft_band_op};
struct mpi_ft_op mpi_ft_op_lor = {OMPI_LOR,EMPI_LOR};//,mpi_ft_lor_op};
struct mpi_ft_op mpi_ft_op_bor = {OMPI_BOR,EMPI_BOR};//,mpi_ft_bor_op};
struct mpi_ft_op mpi_ft_op_lxor = {OMPI_LXOR,EMPI_LXOR};//,mpi_ft_lxor_op};
struct mpi_ft_op mpi_ft_op_bxor = {OMPI_BXOR,EMPI_BXOR};//,mpi_ft_bxor_op};
struct mpi_ft_op mpi_ft_op_minloc = {OMPI_MINLOC,EMPI_MINLOC};//,mpi_ft_minloc_op};
struct mpi_ft_op mpi_ft_op_maxloc = {OMPI_MAXLOC,EMPI_MAXLOC};//,mpi_ft_maxloc_op};
struct mpi_ft_op mpi_ft_op_replace = {OMPI_REPLACE,EMPI_REPLACE};//,mpi_ft_replace_op};
struct mpi_ft_op mpi_ft_op_no_op = {OMPI_NO_OP,EMPI_NO_OP};//,mpi_ft_no_op_op};
*/
/* MPI PROFILER - BEGIN */
#define REP_DEGREE 2
#define CMP_PER_REP 1
static int (*EMPI_Init)(int *, char ***);
static int (*EMPI_Finalize)();
static int (*EMPI_Comm_size)(EMPI_Comm,int *);
static int (*EMPI_Comm_remote_size)(EMPI_Comm,int *);
static int (*EMPI_Comm_rank)(EMPI_Comm,int *);
static int (*EMPI_Send)(void *,int,EMPI_Datatype,int,int,EMPI_Comm);
static int (*EMPI_Recv)(void *,int,EMPI_Datatype,int,int,EMPI_Comm,EMPI_Status *);
static int (*EMPI_Comm_free)(EMPI_Comm *);
static int (*EMPI_Comm_dup)(EMPI_Comm,EMPI_Comm *);
static int (*EMPI_Comm_spawn)(const char *,char **,int,EMPI_Info,int,EMPI_Comm,int *);
static int (*EMPI_Barrier)(EMPI_Comm);
static int (*EMPI_Abort)(EMPI_Comm,int);
static int (*EMPI_Comm_group)(EMPI_Comm,EMPI_Group *);
static int (*EMPI_Group_incl)(EMPI_Group,int,int *,EMPI_Group *);
static int (*EMPI_Comm_create_group)(EMPI_Comm,EMPI_Group,int,EMPI_Comm *);
static int (*EMPI_Comm_split)(EMPI_Comm,int,int,EMPI_Comm *);
static int (*EMPI_Intercomm_create)(EMPI_Comm,int,EMPI_Comm,int,int,EMPI_Comm *);
static int (*EMPI_Comm_set_name)(EMPI_Comm,const char *);
static int (*EMPI_Bcast)(void *,int,EMPI_Datatype,int,EMPI_Comm);
static int (*EMPI_Allgather)(void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm);
static int (*EMPI_Alltoall)(void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm);
static int (*EMPI_Allgatherv)(void *,int,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm);
static int (*EMPI_Allreduce)(void *,void *,int,EMPI_Datatype,EMPI_Op,EMPI_Comm);
static int (*EMPI_Isend)(void *,int,EMPI_Datatype,int,int,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Irecv)(void *,int,EMPI_Datatype,int,int,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Ibcast)(void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Iscatter)(const void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Iscatterv)(const void *,int *,int *,EMPI_Datatype,void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Igather)(const void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Igatherv)(const void *,int,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Ireduce)(void *,void *,int,EMPI_Datatype,EMPI_Op,int,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Iallgather)(const void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Iallgatherv)(const void *,int *,int *,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Ialltoall)(void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Ialltoallv)(void *,int *,int *,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Iallreduce)(void *,void *,int,EMPI_Datatype,EMPI_Op,EMPI_Comm,EMPI_Request *);
static int (*EMPI_Ibarrier)(EMPI_Comm,EMPI_Request *);
static int (*EMPI_Test)(EMPI_Request *,int *,EMPI_Status *);
static int (*EMPI_Cancel)(EMPI_Request *);
static int (*EMPI_Request_free)(EMPI_Request *);
static int (*EMPI_Type_size)(EMPI_Datatype,int *);
static double (*EMPI_Wtime)();

static int (*OMPI_Init)(int *, char ***);
static int (*OMPI_Finalize)();
static int (*OMPI_Comm_size)(OMPI_Comm,int *);
static int (*OMPI_Comm_remote_size)(OMPI_Comm,int *);
static int (*OMPI_Comm_rank)(OMPI_Comm,int *);
static int (*OMPI_Send)(void *,int,OMPI_Datatype,int,int,OMPI_Comm);
static int (*OMPI_Recv)(void *,int,OMPI_Datatype,int,int,OMPI_Comm,OMPI_Status *);
static int (*OMPI_Comm_free)(OMPI_Comm *);
static int (*OMPI_Comm_dup)(OMPI_Comm,OMPI_Comm *);
static int (*OMPI_Comm_create_errhandler)(OMPI_Comm_errhandler_function *,OMPI_Errhandler *);
static int (*OMPI_Comm_set_errhandler)(OMPI_Comm, OMPI_Errhandler);
static int (*OMPI_Barrier)(OMPI_Comm);
static int (*OMPI_Abort)(OMPI_Comm,int);
static int (*OMPI_Error_string)(int, char *,int *);
static int (*OMPI_Error_class)(int,int *);
static int (*OMPIX_Comm_is_revoked)(OMPI_Comm,int *);
static int (*OMPIX_Comm_revoke)(OMPI_Comm);
static int (*OMPIX_Comm_shrink)(OMPI_Comm,OMPI_Comm *);
static int (*OMPI_Group_difference)(OMPI_Group,OMPI_Group,OMPI_Group *);
static int (*OMPI_Group_size)(OMPI_Group,int *);
static int (*OMPI_Group_translate_ranks)(OMPI_Group,int,const int *,OMPI_Group,int *);
static int (*OMPI_Group_incl)(OMPI_Group,int,int *,OMPI_Group *);
static int (*OMPI_Comm_group)(OMPI_Comm,OMPI_Group *);
static int (*OMPI_Comm_remote_group)(OMPI_Comm,OMPI_Group *);
static int (*OMPI_Comm_create)(OMPI_Comm,OMPI_Group,OMPI_Comm *);
static int (*OMPI_Comm_split)(OMPI_Comm,int,int,OMPI_Comm *);
static int (*OMPI_Intercomm_create)(OMPI_Comm,int,OMPI_Comm,int,int,OMPI_Comm *);
static int (*OMPI_Comm_set_name)(OMPI_Comm,const char *);
static int (*OMPI_Comm_test_inter)(OMPI_Comm,int *);
static int (*OMPIX_Comm_failure_ack)(OMPI_Comm);
static int (*OMPIX_Comm_failure_get_acked)(OMPI_Comm,OMPI_Group *);
static int (*OMPI_Bcast)(void *,int,OMPI_Datatype,int,OMPI_Comm);
static int (*OMPI_Allgather)(void *,int,OMPI_Datatype,void *,int,OMPI_Datatype,OMPI_Comm);
static int (*OMPIX_Comm_mark_failed)(OMPI_Comm,int);

static FILE *logfile;
static OMPI_Comm oworldComm;
static EMPI_Comm eworldComm;
static OMPI_Comm OMPI_COMM_CMP, OMPI_COMM_REP, OMPI_CMP_REP_INTERCOMM;
static EMPI_Comm EMPI_COMM_CMP, EMPI_COMM_REP, EMPI_CMP_REP_INTERCOMM;
static int *repToCmpMap = NULL, *cmpToRepMap = NULL;
//extern int *repToCmpMap, *cmpToRepMap;
static char *workDir = "/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi", *repState = "repState";
static int sendid = 0;
//extern char *workDir, *repState;

/* MPI PROFILER - END */

/* REPLICATION MANAGER - BEGIN */

#define CONFIG_FILE cfg
#define CP_FILE ckpt

#define JMP_BUF_SP(tempEnv) (*((long *)(tempEnv)))//(((long*)(tempEnv))[JmpBufSP_Index()])

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

static RAW_ADDR stackPosInit;
static jmp_buf env;
static RAW_ADDR key;
static int repRank = -1, nC, nR;
//extern int repRank, nC, nR;

long array_sum(const int *,int);

struct Barrier_args
{
  MPI_Comm comm;
};

struct Bcast_args
{
  void *buf;
  int count;
  MPI_Datatype dt;
  int root;
  //int reproot;
  MPI_Comm comm;
};

struct Scatter_args
{
  void *sendbuf;
  int sendcount;
	MPI_Datatype senddt;
	void *recvbuf;
	int recvcount;
	MPI_Datatype recvdt;
  int root;
  //int reproot;
  MPI_Comm comm;
};

struct Gather_args
{
  void *sendbuf;
  int sendcount;
	MPI_Datatype senddt;
	void *recvbuf;
	int recvcount;
	MPI_Datatype recvdt;
  int root;
  //int reproot;
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
};

struct Allgather_args
{
  void *sendbuf;
  int sendcount;
	MPI_Datatype senddt;
	void *recvbuf;
	int recvcount;
	MPI_Datatype recvdt;
  MPI_Comm comm;
};

struct Alltoall_args
{
  void *sendbuf;
  int sendcount;
	MPI_Datatype senddt;
	void *recvbuf;
	int recvcount;
	MPI_Datatype recvdt;
  MPI_Comm comm;
};

struct Alltoallv_args
{
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

struct Allreduce_args
{
  void *sendbuf;
  void *recvbuf;
  int count;
  MPI_Datatype dt;
  MPI_Op op;
  //int targetrep;
  MPI_Comm comm;
};

static struct collective_data
{
  int type;
  int id;
	bool completecmp;
	bool completerep;
	EMPI_Request *repreq;
  union
  {
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
} *last_collective = NULL;

static struct peertopeer_data
{
	int id;
	bool sendtype;
	void *buf;
	int count;
	MPI_Datatype dt;
	int target;
	int tag;
	MPI_Comm comm;
	bool completecmp;
	bool completerep;
	MPI_Request *req;
	//MPI_Request *reqrep;
	struct peertopeer_data *next;
	struct peertopeer_data *prev;
} *last_peertopeer = NULL, *first_peertopeer = NULL;

static struct skiplist
{
	int id;
	struct skiplist *next;
} *skipcmplist = NULL, *skipreplist = NULL;

/* MPI_REQ LIST BEGIN */

struct EMPIReqStruct
{
  EMPI_Request *request;
  EMPI_Request *reqRep;
  void *reqRepBuf;
  struct EMPIReqStruct *next;
};
static struct EMPIReqStruct *empiReqHead = NULL, *empiReqTail = NULL;
//extern struct MPIReqStruct *mpiReqHead, *mpiReqTail;
struct EMPIReqStruct *efindMR (EMPI_Request *);
struct EMPIReqStruct *ecreateMR (EMPI_Request *);
void edeleteMR (EMPI_Request *);
int ecountMR (void);

/* MPI_REQ LIST END */

/* HEAP LIST - BEGIN */

struct heapListStruct
{  
  void *ptrAddr;
  void *ptr;
  size_t size;
  struct heapListStruct *next;
};

struct mapStruct
{
  RAW_ADDR addr;
  RAW_ADDR val;
};

static struct heapListStruct *heapListHead = NULL, *heapListTail = NULL;
//extern struct heapListStruct *heapListHead, *heapListTail;

int updateHL (void *, void *, size_t);
void appendHL (void *, void *, size_t);
void deleteHL (void *);
int countHL (void);
void displayHL (void);

/* MALLOC/FREE WRAPPER - BEGIN */

void *myMalloc (void *, size_t);
void myFree (void *);

/* MALLOC/FREE WRAPPER - END */

/* HEAP LIST - END */

static RAW_ADDR dataSegAddr [3]; 
static RAW_ADDR stackSegAddr [2];
static RAW_ADDR textstart;
static RAW_ADDR stackLimit;
static RAW_ADDR heapLimit;
static RAW_ADDR libLimit[2];
static RAW_ADDR replibLimit[2];
//static RAW_ADDR regs [5];

static int *internalranks;
static OMPI_Group oworldGroupFull;
static OMPI_Group failed_group;

static long dataOffset;
static long libOffset;
static bool libUsed;

int JmpBufSP_Index (void);
unsigned long SPFromJmpBuf (jmp_buf);
unsigned long FPFromJmpBuf (jmp_buf);
unsigned long PCFromJmpBuf (jmp_buf);
unsigned long SPFromJmpBufUsingKey (jmp_buf, RAW_ADDR);
unsigned long FPFromJmpBufUsingKey (jmp_buf, RAW_ADDR);
unsigned long PCFromJmpBufUsingKey (jmp_buf, RAW_ADDR);
int replace (RAW_ADDR bottomAddr, long, int, EMPI_Comm);

/* STACK REPLACE - BEGIN */

static void (*savedFunc)();
#define tmpStackSize sizeof(char)*512*1024/sizeof(double)
static double tmpStack [tmpStackSize];
static const long overRunFlag = 0xdeadbeef;
void restoreStack (void);

static long *getOverrunPos (void);
void executeOnTmpStk (void (*)());

/* STACK REPLACE - END */

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

static void HandleErrors(OMPI_Comm *pcomm, int *perr, ... );
static OMPI_Errhandler ehandler;

static int iof_setup(int, int, int);
static void set_handler_local(int);
static int childconnect(int, int, int, int, int, int, int);
static int check_errhandler_conditions(OMPI_Comm, EMPI_Request *, int);
static int check_errhandler_conditions_collective(OMPI_Comm, EMPI_Request *);
static int closest_replica(int);
static void mpi_ft_collective_from_args(struct collective_data *, int, int, int);
static void mpi_ft_free_older_collectives(struct collective_data *);

#ifdef __cplusplus
}
#endif
#endif
