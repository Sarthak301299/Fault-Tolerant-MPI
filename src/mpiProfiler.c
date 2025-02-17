#include "wrappers.h"
#include "heap_allocator.h"
#include "commbuf_cache.h"
#include "client_thread.h"
#include "request_handler.h"
#include "ibvctx.h"
#include "mmanager.h"
#include "full_context.h"
#include "mpi-internal.h"
#include "mpi.h"
#ifdef HEAPTRACK
#include <sys/time.h>
#endif

#if SIZE_MAX == UCHAR_MAX
   #define my_EMPI_SIZE_T EMPI_UNSIGNED_CHAR
#elif SIZE_MAX == USHRT_MAX
   #define my_EMPI_SIZE_T EMPI_UNSIGNED_SHORT
#elif SIZE_MAX == UINT_MAX
   #define my_EMPI_SIZE_T EMPI_UNSIGNED
#elif SIZE_MAX == ULONG_MAX
   #define my_EMPI_SIZE_T EMPI_UNSIGNED_LONG
#elif SIZE_MAX == ULLONG_MAX
   #define my_EMPI_SIZE_T EMPI_UNSIGNED_LONG_LONG
#else
   #error "what is happening here?"
#endif

double ___ckpt_time[MAX_CKPT];
int ___ckpt_counter = -1;

void *extLib = NULL;
void *openLib = NULL;

double timespec_to_double(struct timespec ts) {
	return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

struct timespec mpi_ft_start_time;
struct timespec mpi_ft_end_time;
struct timespec mpi_ft_ckpt_start_time;
struct timespec mpi_ft_ckpt_end_time;
struct timespec mpi_ft_replica_rearrange_start_time;
struct timespec mpi_ft_replica_rearrange_end_time;

struct mpi_ft_comm mpi_ft_comm_null;
struct mpi_ft_op mpi_ft_op_null;
struct mpi_ft_datatype mpi_ft_datatype_null;
struct mpi_ft_request mpi_ft_request_null;
struct mpi_ft_comm mpi_ft_comm_world;
struct mpi_ft_comm mpi_ft_comm_self;
struct mpi_ft_datatype mpi_ft_datatype_char;
struct mpi_ft_datatype mpi_ft_datatype_signed_char;
struct mpi_ft_datatype mpi_ft_datatype_unsigned_char;
struct mpi_ft_datatype mpi_ft_datatype_byte;
struct mpi_ft_datatype mpi_ft_datatype_short;
struct mpi_ft_datatype mpi_ft_datatype_unsigned_short;
struct mpi_ft_datatype mpi_ft_datatype_int;
struct mpi_ft_datatype mpi_ft_datatype_unsigned;
struct mpi_ft_datatype mpi_ft_datatype_long;
struct mpi_ft_datatype mpi_ft_datatype_unsigned_long;
struct mpi_ft_datatype mpi_ft_datatype_float;
struct mpi_ft_datatype mpi_ft_datatype_double;
struct mpi_ft_datatype mpi_ft_datatype_long_double;
struct mpi_ft_datatype mpi_ft_datatype_long_long_int;
struct mpi_ft_datatype mpi_ft_datatype_unsigned_long_long;

struct mpi_ft_datatype mpi_ft_datatype_packed;
//struct mpi_ft_datatype mpi_ft_datatype_lb;
//struct mpi_ft_datatype mpi_ft_datatype_ub;
struct mpi_ft_datatype mpi_ft_datatype_float_int;
struct mpi_ft_datatype mpi_ft_datatype_double_int;
struct mpi_ft_datatype mpi_ft_datatype_long_int;
struct mpi_ft_datatype mpi_ft_datatype_short_int;
struct mpi_ft_datatype mpi_ft_datatype_2int;
struct mpi_ft_datatype mpi_ft_datatype_long_double_int;
struct mpi_ft_datatype mpi_ft_datatype_complex;
struct mpi_ft_datatype mpi_ft_datatype_double_complex;
struct mpi_ft_datatype mpi_ft_datatype_logical;
struct mpi_ft_datatype mpi_ft_datatype_real;
struct mpi_ft_datatype mpi_ft_datatype_double_precision;
struct mpi_ft_datatype mpi_ft_datatype_integer;
struct mpi_ft_datatype mpi_ft_datatype_2integer;
struct mpi_ft_datatype mpi_ft_datatype_2real;
struct mpi_ft_datatype mpi_ft_datatype_2double_precision;
struct mpi_ft_datatype mpi_ft_datatype_character;
//struct mpi_ft_datatype mpi_ft_datatype_real4;
//struct mpi_ft_datatype mpi_ft_datatype_real8;
//struct mpi_ft_datatype mpi_ft_datatype_real16;
//struct mpi_ft_datatype mpi_ft_datatype_complex8;
//struct mpi_ft_datatype mpi_ft_datatype_complex16;
//struct mpi_ft_datatype mpi_ft_datatype_complex32;
//struct mpi_ft_datatype mpi_ft_datatype_integer1;
//struct mpi_ft_datatype mpi_ft_datatype_integer2;
//struct mpi_ft_datatype mpi_ft_datatype_integer4;
//struct mpi_ft_datatype mpi_ft_datatype_integer8;
//struct mpi_ft_datatype mpi_ft_datatype_integer16;
struct mpi_ft_datatype mpi_ft_datatype_int8_t;
struct mpi_ft_datatype mpi_ft_datatype_int16_t;
struct mpi_ft_datatype mpi_ft_datatype_int32_t;
struct mpi_ft_datatype mpi_ft_datatype_int64_t;
struct mpi_ft_datatype mpi_ft_datatype_uint8_t;
struct mpi_ft_datatype mpi_ft_datatype_uint16_t;
struct mpi_ft_datatype mpi_ft_datatype_uint32_t;
struct mpi_ft_datatype mpi_ft_datatype_uint64_t;
struct mpi_ft_datatype mpi_ft_datatype_c_bool;
struct mpi_ft_datatype mpi_ft_datatype_c_float_complex;
struct mpi_ft_datatype mpi_ft_datatype_c_double_complex;
struct mpi_ft_datatype mpi_ft_datatype_c_long_double_complex;
struct mpi_ft_datatype mpi_ft_datatype_aint;
struct mpi_ft_datatype mpi_ft_datatype_offset;
struct mpi_ft_datatype mpi_ft_datatype_count;
struct mpi_ft_datatype mpi_ft_datatype_cxx_bool;
struct mpi_ft_datatype mpi_ft_datatype_cxx_float_complex;
struct mpi_ft_datatype mpi_ft_datatype_cxx_double_complex;
struct mpi_ft_datatype mpi_ft_datatype_cxx_long_double_complex;

struct mpi_ft_op mpi_ft_op_max;
struct mpi_ft_op mpi_ft_op_min;
struct mpi_ft_op mpi_ft_op_sum;
struct mpi_ft_op mpi_ft_op_prod;
struct mpi_ft_op mpi_ft_op_land;
struct mpi_ft_op mpi_ft_op_band;
struct mpi_ft_op mpi_ft_op_lor;
struct mpi_ft_op mpi_ft_op_bor;
struct mpi_ft_op mpi_ft_op_lxor;
struct mpi_ft_op mpi_ft_op_bxor;
struct mpi_ft_op mpi_ft_op_minloc;
struct mpi_ft_op mpi_ft_op_maxloc;
struct mpi_ft_op mpi_ft_op_replace;
struct mpi_ft_op mpi_ft_op_no_op;

struct collective_data *last_collective = NULL;
struct peertopeer_data *first_peertopeer = NULL;
struct peertopeer_data *last_peertopeer = NULL;

pthread_mutex_t peertopeerLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t peertopeerCond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t collectiveLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t collectiveCond = PTHREAD_COND_INITIALIZER;

/*void peertopeerInsert(ptpdata *newnode) {
	if(last_peertopeer == NULL) {
		first_peertopeer = newnode;
	} else {
		newnode->prev = last_peertopeer;
		last_peertopeer->next = newnode;
	}
	last_peertopeer = newnode;
}

void peertopeerDelete(ptpdata *rnode) {
	if(rnode->prev != NULL) rnode->prev->next = rnode->next;
	else first_peertopeer = rnode->next;
	if(rnode->next != NULL) rnode->next->prev = rnode->prev;
	else last_peertopeer = rnode->prev;
}

bool peertopeerIsEmpty() {
	return first_peertopeer == NULL;
}*/

int parep_mpi_sendid = (int)0x70000000;
int parep_mpi_collective_id = (int)0x70000000;

int tagbase = 0;

volatile sig_atomic_t parep_mpi_sighandling_state = 0;

volatile sig_atomic_t parep_mpi_wait_block = 0;
volatile sig_atomic_t parep_mpi_failed_proc_recv = 0;

char *newStack;
int related_map;

char *tempStack;
char *tempMap;
address parep_mpi_temp_pc[10];
int parep_mpi_jumped[10];

jmp_buf context;
address stackHigherAddress, stackLowerAddress;
extern int unmap_num;
extern int remap_num;
extern address unmap_addr[100][2];
extern address remap_addr[MAX_MAPPINGS][3];

int parep_mpi_size;
int parep_mpi_rank;
int parep_mpi_node_rank;
int parep_mpi_node_id;
int parep_mpi_node_num;
int parep_mpi_node_size;
int parep_mpi_original_rank;

pid_t parep_mpi_pid;

int parep_mpi_leader_rank = 0;
pthread_mutex_t parep_mpi_leader_rank_mutex = PTHREAD_MUTEX_INITIALIZER;

volatile sig_atomic_t parep_mpi_restore = 0;
volatile sig_atomic_t parep_mpi_restore_wait = 0;
volatile sig_atomic_t parep_mpi_ckpt_wait = 0;

extern volatile int end_wake_thread;

int parep_mpi_coordinator_socket;
struct sockaddr_un parep_mpi_coordinator_addr;

int shrinkpipe[2];
pthread_t shrinkthread;
EMPI_Group shrinkgroup;
EMPI_Comm shrinkcomm;

struct sockaddr_in parep_mpi_dyn_coordinator_addr;

bool __parep_mpi_ckpt_triggered = false;
bool __parep_mpi_ckpt_restore_triggered = false;
bool __parep_mpi_ckpt_disable_triggered = false;

bool newStack_ready = false;
pthread_mutex_t newStack_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t newStack_cond_var = PTHREAD_COND_INITIALIZER;

bool threadcontext_write_ready[10] = {false,false,false,false,false,false,false,false,false,false};
pthread_mutex_t threadcontext_write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t threadcontext_write_cond_var = PTHREAD_COND_INITIALIZER;

bool precheckpoint_ready[10] = {false,false,false,false,false,false,false,false,false,false};
pthread_mutex_t precheckpoint_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t precheckpoint_cond_var = PTHREAD_COND_INITIALIZER;

int writing_ckpt_futex = 0;
char writing_ckpt_check;
long writing_ckpt_ret;
int reached_temp_futex[10] = {0,0,0,0,0,0,0,0,0,0};
long reached_temp_ret[10];

bool thread_ready[10] = {false,false,false,false,false,false,false,false,false,false};
pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t thread_cond_var = PTHREAD_COND_INITIALIZER;

bool ib_restored_ready[10] = {false,false,false,false,false,false,false,false,false,false};
pthread_mutex_t ib_restored_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ib_restored_cond_var = PTHREAD_COND_INITIALIZER;

bool imm_return[10] = {false,false,false,false,false,false,false,false,false,false};
bool imm_return_ready[10] = {false,false,false,false,false,false,false,false,false,false};
pthread_mutex_t imm_return_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t imm_return_cond_var = PTHREAD_COND_INITIALIZER;

void *threadfsbase[10];
jmp_buf threadContext[10];
Context threadprocContext[10];

int num_threads = 0;
pthread_mutex_t num_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t num_thread_cond = PTHREAD_COND_INITIALIZER;
pthread_t thread_tid[10];
pid_t kernel_tid[10];
bool signal_bcasted = false;
bool entered_sighandler[10] = {false,false,false,false,false,false,false,false,false,false};
pthread_mutex_t entered_sighandler_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t entered_sighandler_cond = PTHREAD_COND_INITIALIZER;

void *parep_mpi_ext_heap_mapping;
size_t parep_mpi_ext_heap_size;

void *parep_mpi_new_stack;
size_t parep_mpi_new_stack_size;
jmp_buf parep_mpi_stack_switcher;
address parep_mpi_new_stack_start;
address parep_mpi_new_stack_end;
address parep_mpi_target_sp,parep_mpi_target_bp,parep_mpi_target_pc;
bool parep_mpi_initialized = false;

bool parep_mpi_empi_initialized = false;
pthread_mutex_t parep_mpi_empi_initialized_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t parep_mpi_empi_initialized_cond = PTHREAD_COND_INITIALIZER;

jmp_buf parep_mpi_replication_initializer;
jmp_buf parep_mpi_replication_stack_switcher;

bool parep_mpi_internal_call = true;

pthread_rwlock_t commLock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t fgroupLock = PTHREAD_RWLOCK_INITIALIZER;

extern pthread_mutex_t reqListLock;
extern pthread_cond_t reqListCond;

extern pthread_mutex_t recvDataListLock;
extern pthread_cond_t recvDataListCond;

extern recvDataNode *recvDataHead;
extern recvDataNode *recvDataTail;

extern recvDataNode *recvDataRedHead;
extern recvDataNode *recvDataRedTail;

extern reqNode *reqHead;
extern reqNode *reqTail;

extern pthread_mutex_t openFileListLock;
extern pthread_cond_t openFileListCond;

extern openFileNode *openFileHead;
extern openFileNode *openFileTail;

extern int (*_real_fclose)(FILE *);
extern FILE *(*_real_fopen)(const char *, const char *);
extern FILE *(*_real_fopen64)(const char *, const char *);

extern int (*_real_pthread_create)(pthread_t *restrict,const pthread_attr_t *restrict,void *(*)(void *),void *restrict);

extern bin_t parep_mpi_bins[BIN_COUNT];
extern bin_t parep_mpi_fastbins[FASTBIN_COUNT];
extern pthread_mutex_t heap_free_list_mutex;
extern heap_t parep_mpi_heap;

extern commbuf_bin_t parep_mpi_commbuf_bins[COMMBUF_BIN_COUNT];

extern pthread_rwlock_t wrapperLock;

extern void (*_real_free)(void *);
extern void *(*_real_malloc)(size_t);

extern void (*_ext_free)(void *);
extern void *(*_ext_malloc)(size_t);
extern void *(*_ext_calloc)(size_t,size_t);
extern void *(*_ext_realloc)(void *,size_t);
extern void *(*_ext_valloc)(size_t);
extern void *(*_ext_memalign)(size_t,size_t);
extern struct mallinfo (*_ext_mallinfo)(void);

extern volatile sig_atomic_t parep_mpi_polling_started;

address parep_mpi_reg_vals[64][15];
int parep_mpi_num_reg_vals = 0;

int parep_mpi_completed = 0;

int rem_recv_msg_sent = 0;
pthread_mutex_t rem_recv_msg_sent_mutex = PTHREAD_MUTEX_INITIALIZER;

long parep_mpi_store_buf_sz = 0;
pthread_mutex_t parep_mpi_store_buf_sz_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t parep_mpi_store_buf_sz_cond = PTHREAD_COND_INITIALIZER;

int parep_mpi_pmi_fd;
char parep_mpi_kvs_name[256];

int parep_mpi_fortran_binding_used = 0;
address parep_mpi_fortran_sp;
address parep_mpi_fortran_bp;
address parep_mpi_fortran_pc;

int parep_mpi_validator_pipe_check = 0;

int parep_mpi_manual_restart = 0;

int parep_mpi_argc;
char **parep_mpi_argv;
extern int main(int,char **);

void mpi_ft_datatype_init(EMPI_Datatype edatatype, int size, struct mpi_ft_datatype *out) {
	out->edatatype = edatatype;
	out->size = size;
}

void mpi_ft_comm_init(EMPI_Comm eworldComm, EMPI_Comm ecmpComm, EMPI_Comm erepComm, EMPI_Comm einterComm, EMPI_Comm ecmpnorepComm, EMPI_Comm ecmpnorepinterComm, struct mpi_ft_comm *out) {
	out->eworldComm = eworldComm;
	out->EMPI_COMM_CMP = ecmpComm;
	out->EMPI_COMM_REP = erepComm;
	out->EMPI_CMP_REP_INTERCOMM = einterComm;
	out->EMPI_CMP_NO_REP = ecmpnorepComm;
	out->EMPI_CMP_NO_REP_INTERCOMM = ecmpnorepinterComm;
}

void mpi_ft_op_init(EMPI_Op eop, struct mpi_ft_op *out)
{
	out->eop = eop;
}

void mpi_ft_status_init(EMPI_Status estatus, int count, int source, int tag, int error, struct mpi_ft_status *out) {
	out->status = estatus;
	out->count = count;
	out->MPI_SOURCE = source;
	out->MPI_TAG = tag;
	out->MPI_ERROR = error;
}

void mpi_ft_request_init(EMPI_Request reqcmp, EMPI_Request reqrep, bool complete, MPI_Comm comm, MPI_Status *status, int type, /*void* backup, int *backuploc*/void *bufloc, void *storeloc, struct mpi_ft_request *out) {
	out->reqcmp = (EMPI_Request *)libc_malloc(sizeof(EMPI_Request));
	out->reqrep = (EMPI_Request *)libc_malloc(sizeof(EMPI_Request));
	*(out->reqcmp) = reqcmp;
	*(out->reqrep) = reqrep;
	out->complete = complete;
	out->comm = comm;
	if(status != MPI_STATUS_IGNORE) out->status = *status;
	out->type = type;
	//out->backup = backup;
	//out->backuploc = backuploc;
	out->bufloc = bufloc;
	out->storeloc = storeloc;
}

void parep_mpi_fortran_preinit() {
	if(!parep_mpi_initialized) {
		parep_mpi_validator_pipe_check = 1;
		unw_set_caching_policy(unw_local_addr_space,UNW_CACHE_NONE);
		unw_cursor_t cursor; unw_context_t uc;
		unw_word_t ip, sp, bp;
		
		extern address __executable_start;
		extern address _etext;

		unw_getcontext(&uc);
		unw_init_local(&cursor, &uc);
		while (unw_step(&cursor) > 0) {
			unw_get_reg(&cursor, UNW_REG_IP, &ip);
			unw_get_reg(&cursor, UNW_REG_SP, &sp);
			unw_get_reg(&cursor, UNW_X86_64_RBP, &bp);
			if(((long)ip > (long)(&(_etext))) || ((long)ip < (long)(&(__executable_start)))) {
				break;
			}
			//printf("%d: Preinit ip = %p, sp = %p bp = %p\n", getpid(), (long) ip, (long) sp, (long) bp);
			parep_mpi_fortran_sp = (long)sp;
			parep_mpi_fortran_bp = (long)bp;
			parep_mpi_fortran_pc = (long)ip;
		}
		parep_mpi_validator_pipe_check = 0;
	}
}

void parep_mpi_fortran_postfinalize() {
	parep_mpi_validator_pipe_check = 1;
	unw_cursor_t cursor; unw_context_t uc;
	unw_word_t ip, sp, bp;

	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);
	while (unw_step(&cursor) > 0) {
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);
		unw_get_reg(&cursor, UNW_X86_64_RBP, &bp);
		//printf("%d: Postfinalize ip = %p, sp = %p bp = %p\n", getpid(), (long) ip, (long) sp, (long) bp);
		if(parep_mpi_fortran_pc == (long)ip) break;
	}
	parep_mpi_validator_pipe_check = 0;
	if(setjmp(parep_mpi_stack_switcher) == 0) {
		setRSP(parep_mpi_stack_switcher,parep_mpi_fortran_sp);
		setRBP(parep_mpi_stack_switcher,parep_mpi_fortran_bp);
		setPC(parep_mpi_stack_switcher,parep_mpi_fortran_pc);
		parep_mpi_longjmp(parep_mpi_stack_switcher,1);
	}
}

int MPI_Finalize(void) {
	parep_mpi_internal_call = true;
	void *fpointer;
	asm volatile ("mov %%rbp, %0" : "=r"(fpointer));
	parep_mpi_target_pc = *((address *)(fpointer+8));
	
	parep_infiniband_cmd(PAREP_IB_REACHED_FINALIZE);
	
	int myrank;
	EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
	int rank;
	int size;
	int d, dst, src, is_pof2, N2_prev, surfeit;
	int ret = 0;
	char s = 's';
	char r = 'r';

	//printf("%d: Before finalize barrier first ptp %p\n",getpid(),first_peertopeer);
	pthread_mutex_lock(&reqListLock);
	do {
		ret = 0;
		EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
		rank = myrank;
		EMPI_Comm_size(MPI_COMM_WORLD->eworldComm,&size);

		is_pof2 = (size & (size - 1)) ? 0 : 1;
		if(is_pof2) {
			N2_prev = size;
		} else {
			int v = 1;
			int old_v = 1;
			while(v < size) {
				old_v = v;
				v = v << 1;
			}
			N2_prev = old_v;
		}
		surfeit = size - N2_prev;

		if(rank < N2_prev) {
			if (rank < surfeit) {
				EMPI_Request rreq = EMPI_REQUEST_NULL;
				int rflag = 0;
				dst = N2_prev + rank;
				EMPI_Irecv(&r,1,EMPI_BYTE,dst,639012,MPI_COMM_WORLD->eworldComm,&rreq);
				do {
					if(parep_mpi_failed_proc_recv) {
						pthread_mutex_unlock(&reqListLock);
						if(rreq != EMPI_REQUEST_NULL) {
							//EMPI_Cancel(&rreq);
							EMPI_Request_free(&rreq);
						}
						while(parep_mpi_failed_proc_recv);
						pthread_mutex_lock(&reqListLock);
						ret = 1;
						break;
					}
					EMPI_Test(&rreq,&rflag,EMPI_STATUS_IGNORE);
				} while(rflag == 0);
				if(ret == 1) continue;
			}
			for(d = 1; d < N2_prev; d <<= 1) {
				dst = (rank ^ d);
				EMPI_Request rreq = EMPI_REQUEST_NULL;
				EMPI_Request sreq = EMPI_REQUEST_NULL;
				int rflag = 0;
				int sflag = 0;
				EMPI_Irecv(&r,1,EMPI_BYTE,dst,639012,MPI_COMM_WORLD->eworldComm,&rreq);
				EMPI_Isend(&s,1,EMPI_BYTE,dst,639012,MPI_COMM_WORLD->eworldComm,&sreq);
				//if(d == 64) printf("%d: Last iteration dst %d rank %d\n",getpid(),dst,rank);
				do {
					if(parep_mpi_failed_proc_recv) {
						pthread_mutex_unlock(&reqListLock);
						if(sreq != EMPI_REQUEST_NULL) {
							//EMPI_Cancel(&sreq);
							EMPI_Request_free(&sreq);
						}
						if(rreq != EMPI_REQUEST_NULL) {
							//EMPI_Cancel(&rreq);
							EMPI_Request_free(&rreq);
						}
						while(parep_mpi_failed_proc_recv);
						pthread_mutex_lock(&reqListLock);
						ret = 1;
						break;
					}
					EMPI_Test(&sreq,&sflag,EMPI_STATUS_IGNORE);
				} while(sflag == 0);
				//if(d == 64) printf("%d: Last iteration dst %d rank %d send waited sflag %d ret %d\n",getpid(),dst,rank,sflag,ret);
				if(ret == 1) break;
				do {
					if(parep_mpi_failed_proc_recv) {
						pthread_mutex_unlock(&reqListLock);
						if(rreq != EMPI_REQUEST_NULL) {
							//EMPI_Cancel(&rreq);
							EMPI_Request_free(&rreq);
						}
						while(parep_mpi_failed_proc_recv);
						pthread_mutex_lock(&reqListLock);
						ret = 1;
						break;
					}
					EMPI_Test(&rreq,&rflag,EMPI_STATUS_IGNORE);
				} while(rflag == 0);
				if(ret == 1) break;
				//if(d == 64) printf("%d: Last iteration dst %d rank %d recv waited sflag %d ret %d\n",getpid(),dst,rank,rflag,ret);
			}
			if(ret == 1) continue;
			if (rank < surfeit) {
				EMPI_Request sreq = EMPI_REQUEST_NULL;
				int sflag = 0;
				src = N2_prev + rank;
				EMPI_Isend(&s,1,EMPI_BYTE,src,639012,MPI_COMM_WORLD->eworldComm,&sreq);
				do {
					if(parep_mpi_failed_proc_recv) {
						pthread_mutex_unlock(&reqListLock);
						if(sreq != EMPI_REQUEST_NULL) {
							//EMPI_Cancel(&sreq);
							EMPI_Request_free(&sreq);
						}
						while(parep_mpi_failed_proc_recv);
						pthread_mutex_lock(&reqListLock);
						ret = 1;
						break;
					}
					EMPI_Test(&sreq,&sflag,EMPI_STATUS_IGNORE);
				} while(sflag == 0);
				if(ret == 1) continue;
			}
		} else {
			src = rank - N2_prev;
			EMPI_Request rreq = EMPI_REQUEST_NULL;
			EMPI_Request sreq = EMPI_REQUEST_NULL;
			int rflag = 0;
			int sflag = 0;
			EMPI_Irecv(&r,1,EMPI_BYTE,src,639012,MPI_COMM_WORLD->eworldComm,&rreq);
			EMPI_Isend(&s,1,EMPI_BYTE,src,639012,MPI_COMM_WORLD->eworldComm,&sreq);
			do {
				if(parep_mpi_failed_proc_recv) {
					pthread_mutex_unlock(&reqListLock);
					if(sreq != EMPI_REQUEST_NULL) {
						//EMPI_Cancel(&sreq);
						EMPI_Request_free(&sreq);
					}
					if(rreq != EMPI_REQUEST_NULL) {
						//EMPI_Cancel(&rreq);
						EMPI_Request_free(&rreq);
					}
					while(parep_mpi_failed_proc_recv);
					pthread_mutex_lock(&reqListLock);
					ret = 1;
					break;
				}
				EMPI_Test(&sreq,&sflag,EMPI_STATUS_IGNORE);
			} while(sflag == 0);
			if(ret == 1) continue;
			do {
				if(parep_mpi_failed_proc_recv) {
					pthread_mutex_unlock(&reqListLock);
					if(rreq != EMPI_REQUEST_NULL) {
						//EMPI_Cancel(&rreq);
						EMPI_Request_free(&rreq);
					}
					while(parep_mpi_failed_proc_recv);
					pthread_mutex_lock(&reqListLock);
					ret = 1;
					break;
				}
				EMPI_Test(&rreq,&rflag,EMPI_STATUS_IGNORE);
			} while(rflag == 0);
		}
	} while(ret == 1);
	pthread_mutex_unlock(&reqListLock);
	//MPI_Barrier(MPI_COMM_WORLD);
	clock_gettime(CLOCK_REALTIME,&mpi_ft_end_time);
	if(myrank == 0) {
		printf("FT Time taken = %f\n",(timespec_to_double(mpi_ft_end_time)-timespec_to_double(mpi_ft_start_time)));
	}
	//mpi_ft_free_older_collectives(last_collective);
	last_collective = NULL;
	printf("after finalize returning targ sp %p bp %p pc %p\n",parep_mpi_target_sp,parep_mpi_target_bp,parep_mpi_target_pc);
	pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
	parep_mpi_completed = 1;
	if(myrank == parep_mpi_leader_rank) {
		parep_infiniband_cmd(PAREP_IB_KILL_COORDINATOR);
	}
	pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
	
	while(1);
	
	pthread_join(daemon_poller,NULL);
	
	dlclose(extLib);
	close(parep_mpi_coordinator_socket);
	//munmap(parep_mpi_ext_heap_mapping,parep_mpi_ext_heap_size);
	
	if(parep_mpi_fortran_binding_used) return MPI_SUCCESS;
	
	if(setjmp(parep_mpi_stack_switcher) == 0) {
		setRSP(parep_mpi_stack_switcher,parep_mpi_target_sp);
		setRBP(parep_mpi_stack_switcher,parep_mpi_target_bp);
		setPC(parep_mpi_stack_switcher,parep_mpi_target_pc);
		exit(0);
		parep_mpi_longjmp(parep_mpi_stack_switcher,1);
	}
	return 0;
}

__attribute__((optimize(0)))
int parep_mpi_thread_idle(int tindex, int restore) {
	int tid_index = tindex;
	int res = restore;
	pthread_mutex_lock(&imm_return_mutex);
	while(!imm_return_ready[tid_index]) {
		pthread_cond_wait(&imm_return_cond_var, &imm_return_mutex);
	}
	pthread_mutex_unlock(&imm_return_mutex);
	imm_return_ready[tid_index] = false;
	
	if(imm_return[tid_index]) return 0;
	
	pthread_mutex_lock(&precheckpoint_mutex);
	precheckpoint_ready[tid_index] = true;
	pthread_cond_signal(&precheckpoint_cond_var);
	pthread_mutex_unlock(&precheckpoint_mutex);
	
	int setjmp_status = setjmp(threadContext[tid_index]);
	if(setjmp_status == 0) {
		jmp_buf copy_context;
		copy_jmp_buf(threadContext[tid_index], copy_context);
		
		pthread_mutex_lock(&newStack_mutex);
		while(!newStack_ready) {
			pthread_cond_wait(&newStack_cond_var, &newStack_mutex);
		}
		pthread_mutex_unlock(&newStack_mutex);
		setRSP(copy_context, ((address)(newStack)) + (TEMP_STACK_SIZE/2) - (tid_index*TEMP_STACK_SIZE/20) - TEMP_STACK_BOTTOM_OFFSET);
		parep_mpi_longjmp(copy_context, 1);
	} else if(setjmp_status == 1) {
		parep_mpi_thread_ckpt_wait(tid_index,res);
	} else if(setjmp_status == 2) {
		thread_tid[tid_index] = pthread_self();
		pthread_mutex_lock(&thread_mutex);
		thread_ready[tid_index] = true;
		pthread_cond_signal(&thread_cond_var);
		pthread_mutex_unlock(&thread_mutex);
		
		pthread_mutex_lock(&ib_restored_mutex);
		while(!ib_restored_ready[tid_index]) {
			pthread_cond_wait(&ib_restored_cond_var, &ib_restored_mutex);
		}
		pthread_mutex_unlock(&ib_restored_mutex);
		ib_restored_ready[tid_index] = false;
		return 1;
	}
}

void parep_mpi_sigusr1_hook(int tid_index, bool is_main_thread,int restore) {
	//printf("%d: At parep_mpi_sigusr1_hook %ld main thread %d\n",getpid(),pthread_self(),is_main_thread);
	if(is_main_thread) {
		assert(tid_index == 0);
		if(parep_mpi_restore) {
			if(!__parep_mpi_ckpt_disable_triggered) {
				parep_mpi_checkpoint_restore();
			} else {
				__parep_mpi_ckpt_restore_triggered = true;
			}
			return;
		}
		if(!__parep_mpi_ckpt_disable_triggered) {
			int ret = parep_mpi_checkpoint_init(___ckpt_counter + 1);
		} else {
			__parep_mpi_ckpt_triggered = true;
		}
		return;
	} else {
		int ret = parep_mpi_thread_idle(tid_index,restore);
		return;
	}
}

void handle_sigterm(int sig) {
	pthread_exit(NULL);
}

void *shrink_caller(void *arg) {
	struct sigaction sa;
	sa.sa_handler = handle_sigterm;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGTERM,&sa,NULL);
	int ret = EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm,shrinkgroup,1234,&shrinkcomm);
	printf("%d: EMPI_Comm_create_group ret %d shrinkgroup %p shrinkcomm %p\n",getpid(),ret,shrinkgroup,shrinkcomm);
	int msg = 1234;
	write(shrinkpipe[1],&msg,sizeof(int));
	return NULL;
}

int parep_mpi_checkpoint_init(int ckpt_num) {
	parep_mpi_ckpt_wait = 0;
	___ckpt_counter = ckpt_num;
	char ckpt_name[100];
	int rank;
	bool is_baseline = true;
	struct stat st = {0};
	clock_gettime(CLOCK_REALTIME,&mpi_ft_ckpt_start_time);
	if(___ckpt_counter <= 0) rank = parep_mpi_rank;
	else EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&rank);
	sprintf(ckpt_name,"%s/%d",getenv("PAREP_MPI_WORKDIR"),rank);
	{
		int ret;
		do {
			ret = mkdir(ckpt_name,0700);
		} while((ret == -1) && (errno != EEXIST));
	}
	
	if(ckpt_num >= 3) {
		sprintf(ckpt_name,"%s/%d/%d",getenv("PAREP_MPI_WORKDIR"),rank,ckpt_num-2);
		int ret;
		do {
			ret = remove(ckpt_name);
		} while(ret == 0);
	}
	
	sprintf(ckpt_name,"%s/%d/%d",getenv("PAREP_MPI_WORKDIR"),rank,ckpt_num);
	if(ckpt_num > 0) is_baseline = false;
	pthread_mutex_lock(&imm_return_mutex);
	for(int i = 0; i < num_threads; i++) {
		imm_return[i] = false;
		imm_return_ready[i] = true;
	}
	pthread_cond_broadcast(&imm_return_cond_var);
	pthread_mutex_unlock(&imm_return_mutex);
	
	pthread_mutex_lock(&precheckpoint_mutex);
	for(int i = 1; i < num_threads; i++) {
		while(!precheckpoint_ready[i]) {
			pthread_cond_wait(&precheckpoint_cond_var, &precheckpoint_mutex);
		}
	}
	pthread_mutex_unlock(&precheckpoint_mutex);
	
	//parep_infiniband_cmd(PAREP_IB_PRECHECKPOINT);
	
	prstat.cur_brk = (address)sbrk(0);
	parseStatFile();
	parseMapsFile();
	
	int setjmp_status = setjmp(context);
	if(setjmp_status == 0) {
		jmp_buf copy_context;
		copy_jmp_buf(context, copy_context);

		// Create some space for new temp stack.
//#undef malloc
//#undef free
//		newStack = libc_malloc(sizeof(char) * TEMP_STACK_SIZE);
//#define malloc(size) ft_malloc(size)
//#define free(p) ft_free(p)
		pthread_mutex_lock(&newStack_mutex);
		newStack = mmap(NULL,TEMP_STACK_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
		newStack_ready = true;
		writing_ckpt_futex = 1;
		pthread_cond_broadcast(&newStack_cond_var);
		pthread_mutex_unlock(&newStack_mutex);
		
		address cur_pc = getPC(copy_context);
		char pc_map[100];
		related_map = -1;
		strcpy(pc_map,"NULL");
		for(int i = 0; i < num_mappings; i++) {
			if((cur_pc >= mappings[i].start) && (cur_pc < mappings[i].end)) {
				prstat.restore_start = mappings[i].start;
				prstat.restore_start_map = i;
				strcpy(pc_map,mappings[i].pathname);
			} else if(strcmp(pc_map,"NULL")) {
				if((!strcmp(pc_map,mappings[i].pathname)) && (mappings[i].prot & PROT_READ) && (mappings[i].prot & PROT_WRITE) && (related_map == -1)) {
					prstat.restore_end = mappings[i].end;
					/*for(address start = mappings[i].start; start < mappings[i].end; start += 8) {
						if(*((address *)start) != (address)NULL) {
							for(int k = 0; k < num_mappings; k++) {
								if(!strcmp(mappings[k].pathname,"")) {
									if((*((address *)start) >= mappings[k].start) && (*((address *)start) < mappings[k].end)) {
										related_map = k;
										prstat.related_map = related_map;
										break;
									}
								}
							}
						}
						if(related_map > 0) break;
					}*/
					prstat.related_map = i+1;
					related_map = i+1;
					assert(mappings[i+1].start == mappings[i].end);
					prstat.restore_full_end = mappings[i+1].end;
				}
			}
		}

		// Stack starts from higher address.
		setRSP(copy_context, ((address)(newStack)) + TEMP_STACK_SIZE - TEMP_STACK_BOTTOM_OFFSET);

		// Start execution on new temp stack.
		parep_mpi_longjmp(copy_context, 1);
	} else if(setjmp_status == 1) {
		pthread_mutex_lock(&threadcontext_write_mutex);
		for(int i = 1; i < num_threads; i++) {
			while(!threadcontext_write_ready[i]) {
				pthread_cond_wait(&threadcontext_write_cond_var, &threadcontext_write_mutex);
			}
		}
		pthread_mutex_unlock(&threadcontext_write_mutex);
		
		init_ckpt(ckpt_name, is_baseline);
		//asm volatile ("lock cmpxchgl %3, %0; setz %1" : "+m"(writing_ckpt_futex), "=q"(writing_ckpt_check) : "a"(1), "r"(0) : "memory", "rax");
		if(__sync_bool_compare_and_swap(&writing_ckpt_futex,1,0)) {
			asm volatile ("mov %1, %%rax\n\t"
									"mov %2, %%rdi\n\t"
									"mov %3, %%rsi\n\t"
									"mov %4, %%rdx\n\t"
									"mov %5, %%r10\n\t"
									"mov %6, %%r8\n\t"
									"mov %7, %%r9\n\t"
									"syscall\n\t"
									"mov %%rax, %0"
									: "=r" (writing_ckpt_ret)
									: "i" (SYS_futex),
									"r" (&writing_ckpt_futex),
									"i" (FUTEX_WAKE),
									"r" ((long)INT_MAX),
									"r" ((long)NULL),
									"r" ((long)NULL),
									"r" ((long)0)
									: "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory");
		}
		parep_mpi_longjmp(context,2);
	} else if(setjmp_status == 2) {
		pthread_mutex_lock(&thread_mutex);
		for(int i = 1; i < num_threads; i++) {
			while(!thread_ready[i]) {
				pthread_cond_wait(&thread_cond_var, &thread_mutex);
			}
		}
		pthread_mutex_unlock(&thread_mutex);
		
		newStack_ready = false;
		for(int i = 0; i < num_threads; i++) {
			threadcontext_write_ready[i] = false;
			precheckpoint_ready[i] = false;
			thread_ready[i] = false;
		}
		munmap(newStack,TEMP_STACK_SIZE);
		
		pthread_mutex_lock(&ib_restored_mutex);
		for(int i = 1; i < num_threads; i++) {
			ib_restored_ready[i] = true;
		}
		pthread_cond_broadcast(&ib_restored_cond_var);
		pthread_mutex_unlock(&ib_restored_mutex);
		
		bool checkpoint_written = false;
PAREP_MPI_WRITE_CKPT_START:
		if(___ckpt_counter > 0) {
			int dyn_sock = socket(AF_INET, SOCK_STREAM, 0);
			if(dyn_sock == -1) {
				perror("Failed to create socket");
				exit(1);
			}
			int ret;
			ssize_t bytes_written;
			int msgsize = 0;
			do {
				ret = connect(dyn_sock,(struct sockaddr *)(&parep_mpi_dyn_coordinator_addr),sizeof(parep_mpi_dyn_coordinator_addr));
			} while(ret != 0);

			int cmd = CMD_BARRIER;
			
			while((bytes_written = write(dyn_sock,(&cmd)+msgsize, sizeof(int)-msgsize)) > 0) {
				msgsize += bytes_written;
				if(msgsize >= (sizeof(int))) break;
			}
			if(bytes_written < 0) printf("%d: Write to server returned %d errno %d dyn_sock %d\n",getpid(),bytes_written,errno,dyn_sock);
			msgsize = 0;
			while((bytes_written = write(dyn_sock,(&parep_mpi_rank)+msgsize, sizeof(int)-msgsize)) > 0) {
				msgsize += bytes_written;
				if(msgsize >= (sizeof(int))) break;
			}
			if(bytes_written < 0) printf("%d: Write to server rank returned %d errno %d dyn_sock %d\n",getpid(),bytes_written,errno,dyn_sock);
			close(dyn_sock);
			
			if(!checkpoint_written) {
				parep_mpi_validator_pipe_check = 1;
				unw_cursor_t cursor;
				unw_context_t uc;
				unw_word_t ip,ax,bx,cx,dx,di,si,r8,r9,r10,r11,r12,r13,r14,r15;
				unw_getcontext(&uc);
				unw_init_local(&cursor, &uc);
				parep_mpi_num_reg_vals = 0;
				while (unw_step(&cursor) > 0) {
					unw_get_reg(&cursor, UNW_REG_IP, &ip);
					unw_get_reg(&cursor, UNW_X86_64_RAX, &ax);
					unw_get_reg(&cursor, UNW_X86_64_RBX, &bx);
					unw_get_reg(&cursor, UNW_X86_64_RCX, &cx);
					unw_get_reg(&cursor, UNW_X86_64_RDX, &dx);
					unw_get_reg(&cursor, UNW_X86_64_RDI, &di);
					unw_get_reg(&cursor, UNW_X86_64_RSI, &si);
					unw_get_reg(&cursor, UNW_X86_64_R8, &r8);
					unw_get_reg(&cursor, UNW_X86_64_R9, &r9);
					unw_get_reg(&cursor, UNW_X86_64_R10, &r10);
					unw_get_reg(&cursor, UNW_X86_64_R11, &r11);
					unw_get_reg(&cursor, UNW_X86_64_R12, &r12);
					unw_get_reg(&cursor, UNW_X86_64_R13, &r13);
					unw_get_reg(&cursor, UNW_X86_64_R14, &r14);
					unw_get_reg(&cursor, UNW_X86_64_R15, &r15);
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][0] = (address)ip;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][1] = (address)ax;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][2] = (address)bx;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][3] = (address)cx;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][4] = (address)dx;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][5] = (address)di;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][6] = (address)si;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][7] = (address)r8;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][8] = (address)r9;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][9] = (address)r10;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][10] = (address)r11;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][11] = (address)r12;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][12] = (address)r13;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][13] = (address)r14;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][14] = (address)r15;
					parep_mpi_num_reg_vals++;
				}
				parep_mpi_validator_pipe_check = 0;
				int setjmp_status = setjmp(parep_mpi_replication_initializer);
				if(setjmp_status == 0) {
					jmp_buf copy_context;
					copy_jmp_buf(parep_mpi_replication_initializer, copy_context);
					newStack = mmap(NULL,TEMP_STACK_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
					address cur_pc = getPC(copy_context);
					// Stack starts from higher address.
					setRSP(copy_context, ((address)(newStack)) + TEMP_STACK_SIZE - TEMP_STACK_BOTTOM_OFFSET);
					// Start execution on new temp stack.
					parep_mpi_longjmp(copy_context, 1);
				} else if(setjmp_status == 1) {
					int targ_rank;
					if(rank < nC) targ_rank = rank;
					else targ_rank = repToCmpMap[rank-nC];
					//Get Data Heap and Stack
					sprintf(ckpt_name,"%s/%d/%d",getenv("PAREP_MPI_WORKDIR"),targ_rank,___ckpt_counter);
					write_heap_and_stack(ckpt_name);
					parep_mpi_longjmp(parep_mpi_replication_initializer,2);
				} else if(setjmp_status == 2) {
					if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) checkpoint_written = true;
					printf("Longjmped to HEAPSTACKRESTORE SIDE after ckpt rank %d myrank %d\n",parep_mpi_rank,rank);
					munmap(newStack,TEMP_STACK_SIZE);
				}
			}
			
			struct pollfd pfd;
			pfd.fd = parep_mpi_coordinator_socket;
			pfd.events = POLLIN;
			nfds_t nfds = 1;
			int pollret = -1;
			do {
				pollret = poll(&pfd,nfds,-1);
			} while((pollret == -1) && (errno == EINTR));
			assert(pollret > 0);
			if (pfd.revents != 0) {
				if(pfd.revents & POLLIN) {
					int out;
					int msgsize = 0;
					size_t bytes_read;
					while((bytes_read = read(pfd.fd,(&out)+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					assert(out == CMD_BARRIER);
					int fail_ready = 0;
					msgsize = 0;
					while((bytes_read = read(pfd.fd,(&fail_ready)+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					if(fail_ready) {
						do {
							pollret = poll(&pfd,nfds,-1);
						} while((pollret == -1) && (errno == EINTR));
					}
				}
			}
		}
		
		struct pollfd pfd;
		pfd.fd = parep_mpi_coordinator_socket;
		pfd.events = POLLIN;
		nfds_t nfds = 1;
		int pollret = -1;
		int loop = 0;
		bool redoshrink = false;
		int num_failed_procs;
		int *abs_failed_ranks;
		int *failed_ranks;
		
		do {
			pollret = poll(&pfd,nfds,0);
			if(pollret > 0) {
				if(pfd.revents & POLLIN) {
					int cmd;
					int msgsize = 0;
					size_t bytes_read;
					while((bytes_read = read(pfd.fd,(&cmd)+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					assert(cmd == CMD_INFORM_PROC_FAILED);
					if(cmd == CMD_INFORM_PROC_FAILED) {
						int newnfp;
						if(redoshrink) {
							msgsize = 0;
							while((bytes_read = read(pfd.fd,(&newnfp)+msgsize, sizeof(int)-msgsize)) > 0) {
								msgsize += bytes_read;
								if(msgsize >= (sizeof(int))) break;
							}
							printf("%d: newnfp %d num_failed_procs %d\n",getpid(),newnfp,num_failed_procs);
							num_failed_procs += newnfp;
							_real_free(failed_ranks);
							failed_ranks = (int *)_real_malloc(sizeof(int) * (num_failed_procs));
							int *tempranks = (int *)_real_malloc(sizeof(int) * (num_failed_procs));
							
							for(int k = 0; k < num_failed_procs - newnfp; k++) {
								tempranks[k] = abs_failed_ranks[k];
							}
							_real_free(abs_failed_ranks);
							abs_failed_ranks = tempranks;
							
							msgsize = 0;
							while((bytes_read = read(pfd.fd,&(abs_failed_ranks[num_failed_procs-newnfp])+msgsize, (sizeof(int) * newnfp)-msgsize)) > 0) {
								msgsize += bytes_read;
								if(msgsize >= (sizeof(int) * newnfp)) break;
							}
							printf("%d: myrank %d absfailranks : ",getpid(),parep_mpi_rank);
							for(int i = 0; i < num_failed_procs; i++) printf("%d ",abs_failed_ranks[i]);
							printf("\n");
						} else {
							msgsize = 0;
							while((bytes_read = read(pfd.fd,(&num_failed_procs)+msgsize, sizeof(int)-msgsize)) > 0) {
								msgsize += bytes_read;
								if(msgsize >= (sizeof(int))) break;
							}
							abs_failed_ranks = (int *)_real_malloc(sizeof(int) * (num_failed_procs));
							failed_ranks = (int *)_real_malloc(sizeof(int) * (num_failed_procs));
							msgsize = 0;
							while((bytes_read = read(pfd.fd,((void *)abs_failed_ranks)+msgsize, (sizeof(int) * num_failed_procs)-msgsize)) > 0) {
								msgsize += bytes_read;
								if(msgsize >= (sizeof(int) * num_failed_procs)) break;
							}
						}
						int myrank;
						EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
						int *worldComm_ranks = (int *)_real_malloc(sizeof(int) * (nC+nR));
						int *rankadj = (int *)_real_malloc(sizeof(int) * (nC+nR));
						int *rankadjrep = (int *)_real_malloc(sizeof(int) * (nC+nR));
						int i;
						int num_failed_procs_current;
						
						EMPI_Group current_group;
						EMPI_Group current_failed_group;
						EMPI_Group current_alive_group;
						EMPI_Comm_group(MPI_COMM_WORLD->eworldComm,&current_group);
						
						pthread_rwlock_wrlock(&fgroupLock);
						EMPI_Group failed_group;
						if(parep_mpi_failed_group == EMPI_GROUP_EMPTY) {
							EMPI_Group_incl(parep_mpi_original_group, num_failed_procs, abs_failed_ranks, &parep_mpi_failed_group);
						} else {
							EMPI_Group_incl(parep_mpi_original_group, num_failed_procs, abs_failed_ranks, &failed_group);
							EMPI_Group_union(parep_mpi_failed_group,failed_group,&parep_mpi_failed_group);
						}
						pthread_rwlock_unlock(&fgroupLock);
						
						EMPI_Group_intersection(current_group,parep_mpi_failed_group,&current_failed_group);
						EMPI_Group_size(current_failed_group,&num_failed_procs_current);
						
						for(int j = 0; j < num_failed_procs_current; j++) abs_failed_ranks[j] = j;
						
						EMPI_Group_translate_ranks(current_failed_group,num_failed_procs_current,abs_failed_ranks,current_group,failed_ranks);
						
						for(i = 0; i < nR+nC; i++) {
							rankadjrep[i] = 0;
							rankadj[i] = 0;
							worldComm_ranks[i] = i;
						}
						int nRdiff = 0;
						int nCdiff = 0;
						bool aborting = false;
						bool switching_to_cmp = false;
						for (int j = 0; j < num_failed_procs_current; j++) {
							if(myrank >= nC) {
								if (repToCmpMap[myrank-nC] == failed_ranks[j]) switching_to_cmp = true;
							}
							if(failed_ranks[j] < nC) {
								if(cmpToRepMap[failed_ranks[j]] == -1) {
									//NO REPLICA ABORT
									printf("No Replica exists aborting after ckpt\n");
									aborting = true;
									break;
								} else {
									nRdiff++;
									worldComm_ranks[failed_ranks[j]] = worldComm_ranks[cmpToRepMap[failed_ranks[j]]+nC];
									worldComm_ranks[cmpToRepMap[failed_ranks[j]]+nC] = -1;
									for(i = cmpToRepMap[failed_ranks[j]]+nC+1; i < nC+nR; i++) rankadjrep[i]++;
									repToCmpMap[cmpToRepMap[failed_ranks[j]]] = -1;
									cmpToRepMap[failed_ranks[j]] = -1;
									//failed_ranks[j] = worldComm_ranks[failed_ranks[j]];
								}
							} else {
								nRdiff++;
								worldComm_ranks[failed_ranks[j]] = -1;
								for(i = failed_ranks[j]+1; i < nC+nR; i++) rankadjrep[i]++;
								cmpToRepMap[repToCmpMap[failed_ranks[j]-nC]] = -1;
								repToCmpMap[failed_ranks[j]-nC] = -1;
							}
						}
						if(!aborting) {
							for(i = 0; i < nC+nR; i++) {
								if(worldComm_ranks[i] != -1) worldComm_ranks[i - rankadj[i] - rankadjrep[i]] = worldComm_ranks[i];
								if(i < nC) if(cmpToRepMap[i] != -1) cmpToRepMap[i - rankadj[i]] = cmpToRepMap[i] - rankadjrep[cmpToRepMap[i]+nC];
								if(i < nR) if(repToCmpMap[i] != -1) repToCmpMap[i - rankadjrep[i+nC]] = repToCmpMap[i] - rankadj[repToCmpMap[i]];
							}
							
							nC = nC - nCdiff;
							nR = nR - nRdiff;
						}
						
						if(!aborting) {
							EMPI_Group_incl(current_group, nR+nC, worldComm_ranks, &current_alive_group);
						}
						else {
							EMPI_Group_difference(current_group,current_failed_group,&current_alive_group);
						}
						
						redoshrink = false;
						
						_real_free(worldComm_ranks);
						_real_free(rankadj);
						_real_free(rankadjrep);
						
						EMPI_Comm parep_mpi_new_comm;
						
						if(pipe(shrinkpipe) == -1) {
							printf("%d: Pipe Failed\n",getpid());
							exit(0);
						}
						
						struct pollfd shrinkfds[2];
						shrinkfds[0].fd = parep_mpi_coordinator_socket;
						shrinkfds[0].events = POLLIN;
						shrinkfds[0].revents = 0;
						shrinkfds[1].fd = shrinkpipe[0];
						shrinkfds[1].events = POLLIN;
						shrinkfds[1].revents = 0;
						memcpy(&shrinkgroup,&current_alive_group,sizeof(EMPI_Group));
						
						pthread_rwlock_unlock(&wrapperLock);
						
						_real_pthread_create(&shrinkthread,NULL,shrink_caller,NULL);
						
						pollret = poll(shrinkfds,2,-1);
						assert(pollret > 0);
						if(shrinkfds[1].revents & POLLIN) {
							int msg;
							int msgsize = 0;
							size_t bytes_read;
							while((bytes_read = read(shrinkfds[1].fd,(&msg)+msgsize, sizeof(int)-msgsize)) > 0) {
								msgsize += bytes_read;
								if(msgsize >= (sizeof(int))) break;
							}
							assert(msg == 1234);
							close(shrinkpipe[0]);
							close(shrinkpipe[1]);
							pthread_join(shrinkthread,NULL);
						} else if(shrinkfds[0].revents & POLLIN) {
							if(pollret >= 2) pollret = 1;
							redoshrink = true;
							pthread_kill(shrinkthread,SIGTERM);
							pthread_join(shrinkthread,NULL);
							close(shrinkpipe[0]);
							close(shrinkpipe[1]);
						}
						
						pthread_rwlock_wrlock(&wrapperLock);
						
						if(redoshrink) continue;
						
						printf("%d: Past redo shrink shrinkcomm %p current_alive_group %p shrinkgroup %p\n",getpid(),shrinkcomm,current_alive_group,shrinkgroup);
						
						_real_free(abs_failed_ranks);
						_real_free(failed_ranks);
						
						//EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm,current_alive_group,1234,&parep_mpi_new_comm);
						
						memcpy(&parep_mpi_new_comm,&shrinkcomm,sizeof(EMPI_Comm));
						
						memcpy(&(MPI_COMM_WORLD->eworldComm),&parep_mpi_new_comm,sizeof(EMPI_Comm));
						int newrank;
						EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&newrank);
						int *node_ids = (int *)_real_malloc(sizeof(int) * (nC+nR));
						node_ids[newrank] = parep_mpi_node_id;
						EMPI_Allgather(&parep_mpi_node_id,1,EMPI_INT,node_ids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm);
						for(int j = 0; j < nC+nR; j++) {
							if(node_ids[j] == 0) {
								pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
								parep_mpi_leader_rank = j;
								pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
								break;
							}
						}
						_real_free(node_ids);
						
						if(aborting) {
							EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
							pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
							if(newrank == parep_mpi_leader_rank) {
								parep_infiniband_cmd(PAREP_IB_KILL_COORDINATOR);
							}
							pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
							dlclose(extLib);
							
							while(1);
							
							pollret = poll(&pfd,1,-1);
							int cmd;
							int msgsize = 0;
							size_t bytes_read;
							while((bytes_read = read(pfd.fd,(&cmd)+msgsize, sizeof(int)-msgsize)) > 0) {
								msgsize += bytes_read;
								if(msgsize >= (sizeof(int))) break;
							}
							close(parep_mpi_coordinator_socket);							
							assert(cmd == CMD_EXIT_CALLED);
							exit(1);
						}
						
						if (nR > 0) {
							int color = (newrank < nC) ? 0 : 1;
							int cmpLeader = 0;
							int repLeader = nC;
							
							if (color == 0) {
								EMPI_Comm_split(MPI_COMM_WORLD->eworldComm, color, 0, &(MPI_COMM_WORLD->EMPI_COMM_CMP));
								EMPI_Intercomm_create(MPI_COMM_WORLD->EMPI_COMM_CMP, 0, MPI_COMM_WORLD->eworldComm, repLeader, 100, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
								int colorinternal;
								if(cmpToRepMap[newrank] != -1) colorinternal = EMPI_UNDEFINED;
								else colorinternal = 1;
								EMPI_Comm_split (MPI_COMM_WORLD->EMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
								if(MPI_COMM_WORLD->EMPI_CMP_NO_REP != EMPI_COMM_NULL) EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->eworldComm, repLeader, 101, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
								MPI_COMM_WORLD->EMPI_COMM_REP = EMPI_COMM_NULL;
							} else if (color == 1) {
								EMPI_Comm_split (MPI_COMM_WORLD->eworldComm, color, 0, &(MPI_COMM_WORLD->EMPI_COMM_REP));
								EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, cmpLeader, 100, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
								if(nC > nR) {
									int rleader = nR;
									for(int j = 0; j < nC; j++) {
										if(cmpToRepMap[j] == -1) {
											rleader = j;
											break;
										}
									}
									EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, rleader, 101, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
								}
								MPI_COMM_WORLD->EMPI_COMM_CMP = EMPI_COMM_NULL;
							}
						} else {
							EMPI_Comm_dup(MPI_COMM_WORLD->eworldComm,&(MPI_COMM_WORLD->EMPI_COMM_CMP));
							EMPI_Comm_dup(MPI_COMM_WORLD->eworldComm,&(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
						}
						
						printf("Shrink completed newrank %d parep_mpi_rank %d new leader rank %d loop %d\n",newrank,parep_mpi_rank,parep_mpi_leader_rank,loop);
						pthread_mutex_lock(&peertopeerLock);
						pthread_mutex_lock(&collectiveLock);
						parep_mpi_restore_messages();
						pthread_cond_signal(&collectiveCond);
						pthread_cond_signal(&peertopeerCond);
						pthread_cond_signal(&reqListCond);
						pthread_mutex_unlock(&collectiveLock);
						pthread_mutex_unlock(&peertopeerLock);
						
						goto PAREP_MPI_WRITE_CKPT_START;
					}
				}
			}
			
			if(loop == 0) parep_infiniband_cmd(PAREP_IB_BARRIER);
			loop++;
		} while(loop < 2);
		
		//EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
		
		pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
		if(rank == parep_mpi_leader_rank) {
			FILE *fptr;
			char file[100];
			sprintf(file,"%s/latest_ckpt",getenv("PAREP_MPI_WORKDIR"));
			fptr = _real_fopen(file, "wb");
			fwrite(&___ckpt_counter, sizeof(int), 1, fptr);
			fflush(fptr);
			_real_fclose(fptr);
			if(___ckpt_counter == 0) {
				/*if(getenv("PAREP_MPI_MANUAL_CKPT_START") != NULL) {
					if(strcmp(getenv("PAREP_MPI_MANUAL_CKPT_START"),"1") != 0) parep_infiniband_cmd(PAREP_IB_CKPT_CREATED);
				} else parep_infiniband_cmd(PAREP_IB_CKPT_CREATED);*/
			} else parep_infiniband_cmd(PAREP_IB_CKPT_CREATED);
		}
		clock_gettime(CLOCK_REALTIME,&mpi_ft_ckpt_end_time);
		printf("%d: Rank %d Checkpoint time %f\n",getpid(),rank,timespec_to_double(mpi_ft_ckpt_end_time)-timespec_to_double(mpi_ft_ckpt_start_time));
		pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
		return 1;
	} else {
		clock_gettime(CLOCK_REALTIME,&mpi_ft_ckpt_end_time);
		printf("%d: Rank %d Baseline Restore I/O time %f\n",getpid(),rank,timespec_to_double(mpi_ft_ckpt_end_time)-timespec_to_double(mpi_ft_ckpt_start_time));
		
		pthread_mutex_lock(&thread_mutex);
		for(int i = 1; i < num_threads; i++) {
			while(!thread_ready[i]) {
				pthread_cond_wait(&thread_cond_var, &thread_mutex);
			}
		}
		pthread_mutex_unlock(&thread_mutex);
		
		writing_ckpt_futex = 0;
		
		newStack_ready = false;
		for(int i = 0; i < num_threads; i++) {
			threadcontext_write_ready[i] = false;
			thread_ready[i] = false;
		}
		
		/*newStack = (char *)unmap_addr[0][0];
		for(int i = 1; i < unmap_num; i++) {
			syscall(SYS_munmap,(void *)unmap_addr[i][0],unmap_addr[i][1]-unmap_addr[i][0]);
		}
		munmap(newStack,TEMP_STACK_SIZE);
		
		for(int i = 0; i < remap_num; i++) {
			void *ret = mremap((void *)remap_addr[i][0],remap_addr[i][1],remap_addr[i][1],MREMAP_FIXED|MREMAP_MAYMOVE,(void *)remap_addr[i][2]);
			if(ret == MAP_FAILED) {
				printf("%d: mremap failed addr %p sz %p naddr %p errno %d\n",getpid(),remap_addr[i][0],remap_addr[i][1],remap_addr[i][2],errno);
				while(1);
			}
		}*/
		
		struct pollfd pfd;
		pfd.fd = parep_mpi_coordinator_socket;
		pfd.events = POLLIN;
		nfds_t nfds = 1;
		int pollret = -1;
		do {
			pollret = poll(&pfd,nfds,-1);
		} while((pollret == -1) && (errno == EINTR));
		assert(pollret > 0);
		if (pfd.revents != 0) {
			if(pfd.revents & POLLIN) {
				int out;
				int msgsize = 0;
				size_t bytes_read;
				while((bytes_read = read(pfd.fd,(&out)+msgsize, sizeof(int)-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int))) break;
				}
				if(out != CMD_BARRIER) {
					printf("%d: Failed proc during restore barrier immediate out %d bytes_read %d errno %d\n",getpid(),out,bytes_read,errno);
					//assert(out == CMD_INFORM_PROC_FAILED);
					pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
					if(parep_mpi_rank == parep_mpi_leader_rank) {
						parep_infiniband_cmd(PAREP_IB_KILL_COORDINATOR);
					}
					pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
					while(1);
				}
				assert(out == CMD_BARRIER);
				int fail_ready = 0;
				msgsize = 0;
				while((bytes_read = read(pfd.fd,(&fail_ready)+msgsize, sizeof(int)-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int))) break;
				}
				if(fail_ready) {
					do {
						pollret = poll(&pfd,nfds,-1);
					} while((pollret == -1) && (errno == EINTR));
				}
			}
		}
		
		pollret = -1;
		int loop = 0;
		do {
			pollret = poll(&pfd,nfds,0);
			if(pollret > 0) {
				if(pfd.revents & POLLIN) {
					int cmd;
					int msgsize = 0;
					size_t bytes_read;
					while((bytes_read = read(pfd.fd,(&cmd)+msgsize, sizeof(int)-msgsize)) > 0) {
						msgsize += bytes_read;
						if(msgsize >= (sizeof(int))) break;
					}
					assert(cmd == CMD_INFORM_PROC_FAILED);
					if(cmd == CMD_INFORM_PROC_FAILED) {
						int num_failed_procs;
						msgsize = 0;
						while((bytes_read = read(pfd.fd,(&num_failed_procs)+msgsize, sizeof(int)-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(int))) break;
						}
						int *abs_failed_ranks = (int *)_real_malloc(sizeof(int) * (num_failed_procs));
						msgsize = 0;
						while((bytes_read = read(pfd.fd,((void *)abs_failed_ranks)+msgsize, (sizeof(int) * num_failed_procs)-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(int) * num_failed_procs)) break;
						}
						
						if(num_failed_procs > 0) printf("%d: Rank %d num %d failed during baseline restore aborting myrank %d\n",getpid(),abs_failed_ranks[0],num_failed_procs,rank);
						
						_real_free(abs_failed_ranks);

						pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
						if(rank == parep_mpi_leader_rank) {
							parep_infiniband_cmd(PAREP_IB_KILL_COORDINATOR);
						}
						pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
						dlclose(extLib);
						
						while(1);
						
						pollret = poll(&pfd,1,-1);
						int cmd;
						int msgsize = 0;
						size_t bytes_read;
						while((bytes_read = read(pfd.fd,(&cmd)+msgsize, sizeof(int)-msgsize)) > 0) {
							msgsize += bytes_read;
							if(msgsize >= (sizeof(int))) break;
						}
						close(parep_mpi_coordinator_socket);							
						assert(cmd == CMD_EXIT_CALLED);
						exit(1);
					}
				}
			}
			
			if(loop == 0) parep_infiniband_cmd(PAREP_IB_BARRIER);
			loop++;
		} while(loop < 2);
		
		//parep_infiniband_cmd(PAREP_IB_POSTRESTART);
		
		parep_infiniband_cmd(PAREP_IB_BARRIER);
		
		munmap(tempMap, (prstat.restore_full_end - prstat.restore_start) + TEMP_STACK_SIZE);
		
		pthread_mutex_lock(&ib_restored_mutex);
		for(int i = 1; i < num_threads; i++) {
			ib_restored_ready[i] = true;
		}
		pthread_cond_broadcast(&ib_restored_cond_var);
		pthread_mutex_unlock(&ib_restored_mutex);
		
		if(___ckpt_counter > 0) {
			pthread_rwlock_unlock(&wrapperLock);
			address fsbase;
			syscall(SYS_arch_prctl,ARCH_GET_FS,&fsbase);
			*((pid_t *)(fsbase+0x2d4)) = kernel_tid[0];
			initialize_mpi_variables();
			int myrank,mysize;
			int retVal = empi_comm_creation(&myrank,&mysize,&parep_mpi_argc,&parep_mpi_argv,false);
			initialize_common_heap_and_stack(myrank);
			pthread_rwlock_wrlock(&wrapperLock);
			rank = myrank;

			unw_cursor_t cursor; unw_context_t uc;
			unw_word_t ip;
			int setjmp_status = setjmp(parep_mpi_replication_initializer);
			if(setjmp_status == 0) {
				jmp_buf copy_context;
				copy_jmp_buf(parep_mpi_replication_initializer, copy_context);
				newStack = mmap(NULL,TEMP_STACK_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
				address cur_pc = getPC(copy_context);
				// Stack starts from higher address.
				setRSP(copy_context, ((address)(newStack)) + TEMP_STACK_SIZE - TEMP_STACK_BOTTOM_OFFSET);
				// Start execution on new temp stack.
				parep_mpi_longjmp(copy_context, 1);
			} else if(setjmp_status == 1) {
				int targ_rank;
				if(rank < nC) targ_rank = rank;
				else targ_rank = repToCmpMap[rank-nC];
				//Get Data Heap and Stack
				sprintf(ckpt_name,"%s/%d/%d",getenv("PAREP_MPI_WORKDIR"),targ_rank,___ckpt_counter);
				read_heap_and_stack(ckpt_name);
				parep_mpi_longjmp(parep_mpi_replication_initializer,2);
			} else if(setjmp_status == 2) {
				EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&rank);
				printf("Longjmped to HEAPSTACKRESTORE SIDE after restore rank %d myrank %d\n",parep_mpi_rank,rank);
				munmap(newStack,TEMP_STACK_SIZE);
			}
			
			parep_infiniband_cmd(PAREP_IB_BARRIER);
			
			parep_mpi_validator_pipe_check = 1;
			unw_getcontext(&uc);
			unw_init_local(&cursor, &uc);
			int step = 0;
			while (unw_step(&cursor) > 0) {
				unw_set_reg(&cursor, UNW_REG_IP, parep_mpi_reg_vals[step][0]);
				unw_set_reg(&cursor, UNW_X86_64_RAX, parep_mpi_reg_vals[step][1]);
				unw_set_reg(&cursor, UNW_X86_64_RBX, parep_mpi_reg_vals[step][2]);
				unw_set_reg(&cursor, UNW_X86_64_RCX, parep_mpi_reg_vals[step][3]);
				unw_set_reg(&cursor, UNW_X86_64_RDX, parep_mpi_reg_vals[step][4]);
				unw_set_reg(&cursor, UNW_X86_64_RDI, parep_mpi_reg_vals[step][5]);
				unw_set_reg(&cursor, UNW_X86_64_RSI, parep_mpi_reg_vals[step][6]);
				unw_set_reg(&cursor, UNW_X86_64_R8, parep_mpi_reg_vals[step][7]);
				unw_set_reg(&cursor, UNW_X86_64_R9, parep_mpi_reg_vals[step][8]);
				unw_set_reg(&cursor, UNW_X86_64_R10, parep_mpi_reg_vals[step][9]);
				unw_set_reg(&cursor, UNW_X86_64_R11, parep_mpi_reg_vals[step][10]);
				unw_set_reg(&cursor, UNW_X86_64_R12, parep_mpi_reg_vals[step][11]);
				unw_set_reg(&cursor, UNW_X86_64_R13, parep_mpi_reg_vals[step][12]);
				unw_set_reg(&cursor, UNW_X86_64_R14, parep_mpi_reg_vals[step][13]);
				unw_set_reg(&cursor, UNW_X86_64_R15, parep_mpi_reg_vals[step][14]);
				step++;
			}
			parep_mpi_validator_pipe_check = 0;
			
			openFileNode *onode = openFileHead;
			while(onode != NULL) {
				FILE *file;
				char mode[8];
				if(strstr(onode->mode,"r")) {
					strcpy(mode,onode->mode);
				} else {
					if(strstr(onode->mode,"+")) {
						strcpy(mode,"a+");
					} else {
						strcpy(mode,"a");
					}
				}
				if(onode->use64) file = _real_fopen64(onode->fname,mode);
				else file = _real_fopen(onode->fname,mode);
				dup2(file->_fileno,onode->fd);
				close(file->_fileno);
				file->_fileno = onode->fd;
				memcpy(onode->file,file,sizeof(FILE));
				
				onode = onode->next;
			}
			
			parep_mpi_failed_proc_recv = 0;
			
			pthread_rwlock_unlock(&wrapperLock);
			pthread_mutex_lock(&peertopeerLock);
			pthread_mutex_lock(&collectiveLock);
			
			ptpdata *pdata = first_peertopeer;
			while(pdata != NULL) {
				if(pdata->type == MPI_FT_RECV) {
					if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
						pdata->markdelrep = pdata->markdelcmp;
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
			
			handle_rem_recv();
			
			EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
			
			pdata = first_peertopeer;
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
			
			cdata = last_collective;
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
			
			parep_mpi_restore_messages();
			pthread_cond_signal(&collectiveCond);
			pthread_cond_signal(&peertopeerCond);
			pthread_cond_signal(&reqListCond);
			pthread_mutex_unlock(&collectiveLock);
			pthread_mutex_unlock(&peertopeerLock);
			pthread_rwlock_wrlock(&wrapperLock);
		}
		
		if(___ckpt_counter == 0) {
			/*if(getenv("PAREP_MPI_MANUAL_CKPT_START") != NULL) {
				if(strcmp(getenv("PAREP_MPI_MANUAL_CKPT_START"),"1") != 0) {
					pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
					if(rank == parep_mpi_leader_rank) parep_infiniband_cmd(PAREP_IB_CKPT_CREATED);
					pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
				}
			} else {
				pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
				if(rank == parep_mpi_leader_rank) parep_infiniband_cmd(PAREP_IB_CKPT_CREATED);
				pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
			}*/
		} else {
			pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
			if(rank == parep_mpi_leader_rank) parep_infiniband_cmd(PAREP_IB_CKPT_CREATED);
			pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
		}
		
		pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
		clock_gettime(CLOCK_REALTIME,&mpi_ft_ckpt_end_time);
		if(rank == parep_mpi_leader_rank) printf("%d: Rank %d Restore time %f\n",getpid(),rank,timespec_to_double(mpi_ft_ckpt_end_time)-timespec_to_double(mpi_ft_ckpt_start_time));
		pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
		parep_mpi_restore = 1;
		return 2;
	}
}

int MPI_Checkpoint_start() {
	bool int_call;
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	parep_mpi_ckpt_wait = 1;
	pthread_kill(pthread_self(),SIGUSR1);
	while(parep_mpi_ckpt_wait) {;}
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	/*if(getenv("PAREP_MPI_MANUAL_CKPT_START") != NULL) {
		if(strcmp(getenv("PAREP_MPI_MANUAL_CKPT_START"),"1") == 0) {
			pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
			if(parep_mpi_rank == parep_mpi_leader_rank) parep_infiniband_cmd(PAREP_IB_CKPT_CREATED);
			pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
		}
	}*/
}

void parep_mpi_checkpoint_restore() {
	parep_mpi_restore = 0;
	parep_mpi_restore_wait = 0;
	char ckpt_name[100];
	int ckpt_num;
	int rank;
	bool ckpt_restore = true;
	clock_gettime(CLOCK_REALTIME,&mpi_ft_ckpt_start_time);
	if(___ckpt_counter <= 0) rank = parep_mpi_rank;
	else EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&rank);
	FILE *fptr;
	char file[100];
	sprintf(file,"%s/latest_ckpt",getenv("PAREP_MPI_WORKDIR"));
	fptr = _real_fopen(file, "rb");
	if(fptr == NULL) {
		pthread_mutex_lock(&imm_return_mutex);
		for(int i = 0; i < num_threads; i++) {
			imm_return[i] = true;
			imm_return_ready[i] = true;
		}
		pthread_cond_broadcast(&imm_return_cond_var);
		pthread_mutex_unlock(&imm_return_mutex);
		return;
	} else {
		pthread_mutex_lock(&imm_return_mutex);
		for(int i = 0; i < num_threads; i++) {
			imm_return[i] = false;
			imm_return_ready[i] = true;
		}
		pthread_cond_broadcast(&imm_return_cond_var);
		pthread_mutex_unlock(&imm_return_mutex);
	}
	
	fread(&ckpt_num, sizeof(int), 1, fptr);
	_real_fclose(fptr);
	___ckpt_counter = ckpt_num;
	ckpt_num = 0;
	
	pthread_mutex_lock(&precheckpoint_mutex);
	for(int i = 1; i < num_threads; i++) {
		while(!precheckpoint_ready[i]) {
			pthread_cond_wait(&precheckpoint_cond_var, &precheckpoint_mutex);
		}
	}
	pthread_mutex_unlock(&precheckpoint_mutex);
	
	sprintf(ckpt_name,"%s/%d/%d",getenv("PAREP_MPI_WORKDIR"),rank,ckpt_num);
	
	int dyn_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(dyn_sock == -1) {
		perror("Failed to create socket");
		exit(1);
	}
	int ret;
	do {
		ret = connect(dyn_sock,(struct sockaddr *)(&parep_mpi_dyn_coordinator_addr),sizeof(parep_mpi_dyn_coordinator_addr));
	} while(ret != 0);
	int cmd = CMD_BARRIER;
	write(dyn_sock,&cmd,sizeof(int));
	write(dyn_sock,&parep_mpi_rank,sizeof(int));
	close(dyn_sock);
	
	prstat.cur_brk = (address)sbrk(0);
	parseStatFile();
	parseMapsFile();
	
	int setjmp_status = setjmp(context);
	if(setjmp_status == 0) {
		jmp_buf copy_context;
		copy_jmp_buf(context, copy_context);
		pthread_mutex_lock(&newStack_mutex);
		newStack = mmap(NULL,TEMP_STACK_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
		newStack_ready = true;
		writing_ckpt_futex = 1;
		pthread_cond_broadcast(&newStack_cond_var);
		pthread_mutex_unlock(&newStack_mutex);
		
		address cur_pc = getPC(copy_context);
		char pc_map[100];
		related_map = -1;
		strcpy(pc_map,"NULL");
		for(int i = 0; i < num_mappings; i++) {
			if((cur_pc >= mappings[i].start) && (cur_pc < mappings[i].end)) {
				prstat.restore_start = mappings[i].start;
				prstat.restore_start_map = i;
				strcpy(pc_map,mappings[i].pathname);
			}
			else if(strcmp(pc_map,"NULL")) {
				if((!strcmp(pc_map,mappings[i].pathname)) && (mappings[i].prot & PROT_READ) && (mappings[i].prot & PROT_WRITE) && (related_map == -1)) {
					prstat.restore_end = mappings[i].end;
					/*for(address start = mappings[i].start; start < mappings[i].end; start += 8) {
						if(*((address *)start) != (address)NULL) {
							for(int k = 0; k < num_mappings; k++) {
								if(!strcmp(mappings[k].pathname,"")) {
									if((*((address *)start) >= mappings[k].start) && (*((address *)start) < mappings[k].end)) {
										related_map = k;
										prstat.related_map = related_map;
										break;
									}
								}
							}
						}
						if(related_map > 0) break;
					}*/
					prstat.related_map = i+1;
					related_map = i+1;
					assert(mappings[i+1].start == mappings[i].end);
					prstat.restore_full_end = mappings[i+1].end;
				}
			}
		}
		
		setRSP(copy_context, ((address)(newStack)) + TEMP_STACK_SIZE - TEMP_STACK_BOTTOM_OFFSET);
		parep_mpi_longjmp(copy_context, 1);
	} else if(setjmp_status == 1) {
		unmap_num = 0;
		unmap_addr[unmap_num][0] = (address)newStack;
		unmap_num++;
		pthread_mutex_lock(&threadcontext_write_mutex);
		for(int i = 1; i < num_threads; i++) {
			while(!threadcontext_write_ready[i]) {
				pthread_cond_wait(&threadcontext_write_cond_var, &threadcontext_write_mutex);
			}
		}
		pthread_mutex_unlock(&threadcontext_write_mutex);
		
		//parep_infiniband_cmd(PAREP_IB_CLEAN_COORDINATOR);
		
		init_ckpt_restore(ckpt_name);
		//asm volatile ("lock cmpxchgl %3, %0; setz %1" : "+m"(writing_ckpt_futex), "=q"(writing_ckpt_check) : "a"(1), "r"(0) : "memory", "rax");
		/*if(__sync_bool_compare_and_swap(&writing_ckpt_futex,1,0)) {
			asm volatile ("mov %1, %%rax\n\t"
									"mov %2, %%rdi\n\t"
									"mov %3, %%rsi\n\t"
									"mov %4, %%rdx\n\t"
									"mov %5, %%r10\n\t"
									"mov %6, %%r8\n\t"
									"mov %7, %%r9\n\t"
									"syscall\n\t"
									"mov %%rax, %0"
									: "=r" (writing_ckpt_ret)
									: "i" (SYS_futex),
									"r" (&writing_ckpt_futex),
									"i" (FUTEX_WAKE),
									"r" ((long)INT_MAX),
									"r" ((long)NULL),
									"r" ((long)NULL),
									"r" ((long)0)
									: "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory");
		}
		parep_mpi_longjmp(context,3);*/
	} else {
		printf("Longjmped into restore itself ERROR!!!!\n");
		raise(SIGKILL);
	}
}

void parep_mpi_restore_messages() {
	int myrank,cmprank=-1,reprank=-1;
	EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
	if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_CMP,&cmprank);
	else if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_REP,&reprank);

	int *sendnums = (int *)_real_malloc((nC+nR)*sizeof(int));
	int *recvnums = (int *)_real_malloc((nC+nR)*sizeof(int));
	int *recvnumsdispls = (int *)_real_malloc((nC+nR)*sizeof(int));
	int *senttomenums = (int *)_real_malloc((nC+nR)*sizeof(int));
	int *recvfrommenums = (int *)_real_malloc((nC+nR)*sizeof(int));
	int *recvfrommenumsdispls = (int *)_real_malloc((nC+nR)*sizeof(int));
	
	int *sendids;
	int *recvids;
	int *allrecvids;
	int *allsendids;
	int *rids;
	int *redids;
	int *reddataids;
	int *reddispls;
	
	int *collids = (int *)_real_malloc((nC+nR)*sizeof(int));
	int minid;
	int num_reddataids = 0;
	bool completed = false;
	clcdata *last_completed_collective = last_collective;
	
	if(last_completed_collective == NULL) {
		int temp = -1;
		EMPI_Allgather(&temp,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm);
	} else {
		while(last_completed_collective != NULL) {
			completed = last_completed_collective->completecmp & last_completed_collective->completerep;
			for (int i = 0; i < last_completed_collective->num_colls; i++) {
				completed = completed & last_completed_collective->completecolls[i];
			}
			if(completed) break;
			last_completed_collective = last_completed_collective->next;
		}
		if(last_completed_collective == NULL) {
			int temp = -1;
			EMPI_Allgather(&temp,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm);
		} else {
			EMPI_Allgather(&last_completed_collective->id,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm);
		}
	}
	
	minid = collids[0];
	
	redids = (int *)_real_malloc((nC+nR)*sizeof(int));
	reddispls = (int *)_real_malloc((nC+nR)*sizeof(int));
	clcdata *tempreddata = last_completed_collective;
	while(tempreddata != NULL) {
		if((tempreddata->type == MPI_FT_REDUCE_TEMP) || (tempreddata->type == MPI_FT_ALLREDUCE_TEMP)) {
			num_reddataids++;
		}
		tempreddata = tempreddata->prev;
	}
	EMPI_Allgather(&num_reddataids,1,EMPI_INT,redids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm);
	rids = (int *)_real_malloc(num_reddataids*sizeof(int));
	if(array_sum(redids,nC+nR) > 0) reddataids = (int *)_real_malloc(array_sum(redids,nC+nR)*sizeof(int));
	else reddataids = NULL;
	reddispls[0] = 0;
	for(int i = 1; i < nC+nR; i++) {
		reddispls[i] = reddispls[i-1] + redids[i-1];
	}
	
	tempreddata = last_completed_collective;
	int index = 0;
	while(tempreddata != NULL) {
		if((tempreddata->type == MPI_FT_REDUCE_TEMP) || (tempreddata->type == MPI_FT_ALLREDUCE_TEMP)) {
			rids[index] = tempreddata->id;
			index++;
		}
		tempreddata = tempreddata->prev;
	}
	
	EMPI_Allgatherv(rids,num_reddataids,EMPI_INT,reddataids,redids,reddispls,EMPI_INT,MPI_COMM_WORLD->eworldComm);
	
	_real_free(rids);
	_real_free(reddispls);
	
	for(int i = 0; i<nC+nR;i++) {
		sendnums[i] = 0;
		recvnums[i] = 0;
	}
	
	struct peertopeer_data *pdata = first_peertopeer;
	while(pdata != NULL) {
		if(pdata->type == MPI_FT_SEND) {
			sendnums[pdata->target]++;
			if(cmpToRepMap[pdata->target] != -1) sendnums[cmpToRepMap[pdata->target]+nC]++;
			if(cmpToRepMap[pdata->target] == -1) {
				if(!pdata->completerep) {
					pdata->completerep = true;
					*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
				}
				//New code Take Care!!!
				if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
					if(!pdata->completecmp) {
						pdata->completecmp = true;
						*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
					}
				}
			}
		} else if(pdata->type == MPI_FT_RECV) {
			if(pdata->completecmp && pdata->completerep) {
				recvnums[pdata->target]++;
				if(cmpToRepMap[pdata->target] != -1) recvnums[cmpToRepMap[pdata->target]+nC]++;
			}
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				if(!pdata->completerep || !pdata->completecmp) {
					int retVal, retValRep;
					int size;
					EMPI_Type_size(pdata->dt->edatatype,&size);
					int dis = pdata->count*size;
					int extracount;
					if(size >= sizeof(int)) extracount = 1;
					else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
					else extracount = (((int)sizeof(int))/size) + 1;
					
					*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
					*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
					
					if(cmpToRepMap[pdata->target] == -1) retValRep = EMPI_Irecv (pdata->buf, pdata->count+extracount, pdata->dt->edatatype, pdata->target, pdata->tag, (pdata->comm->EMPI_CMP_REP_INTERCOMM), ((*(pdata->req))->reqrep));
					else retValRep = EMPI_Irecv (pdata->buf, pdata->count+extracount, pdata->dt->edatatype, cmpToRepMap[pdata->target], pdata->tag, (pdata->comm->EMPI_COMM_REP), ((*(pdata->req))->reqrep));
					
					(*(pdata->req))->complete = false;
					pdata->completecmp = true;
					pdata->completerep = false;
				}
			} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				if(!pdata->completecmp || !pdata->completerep) {
					int retVal;
					int size;
					EMPI_Type_size(pdata->dt->edatatype,&size);
					int dis = pdata->count*size;
					int extracount;
					if(size >= sizeof(int)) extracount = 1;
					else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
					else extracount = (((int)sizeof(int))/size) + 1;
					
					*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
					*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
					
					retVal = EMPI_Irecv (pdata->buf, pdata->count+extracount, pdata->dt->edatatype, pdata->target, pdata->tag, (pdata->comm->EMPI_COMM_CMP), ((*(pdata->req))->reqcmp));
					
					(*(pdata->req))->complete = false;
					pdata->completecmp = false;
					pdata->completerep = true;
				}
			}
		}
		pdata = pdata->prev;
	}

	EMPI_Alltoall(recvnums,1,EMPI_INT,recvfrommenums,1,EMPI_INT,MPI_COMM_WORLD->eworldComm);
	
  //Identify items from sendnums that were not in recvfrommenums and resend them
	//Identify items from recvnums that were not in senttomenums and mark them to be skipped
	allrecvids = (int *)_real_malloc(array_sum(recvnums,nC+nR)*sizeof(int));
	allsendids = (int *)_real_malloc(array_sum(recvfrommenums,nC+nR)*sizeof(int));
	
	recvnumsdispls[0] = 0;
	recvfrommenumsdispls[0] = 0;
	index = 0;
	int *indices = (int *)_real_malloc((nC+nR)*sizeof(int));
	for(int i = 0; i<nC+nR;i++) {
		indices[i] = 0;
		if(i > 0) recvnumsdispls[i] = recvnumsdispls[i-1] + recvnums[i-1];
		if(i > 0) recvfrommenumsdispls[i] = recvfrommenumsdispls[i-1] + recvfrommenums[i-1];
	}
	
	pdata = first_peertopeer;
	while(pdata != NULL) {
		if((pdata->type == MPI_FT_RECV) && pdata->completecmp && pdata->completerep) {
			int cmptarg = pdata->target;
			int reptarg = cmpToRepMap[pdata->target] + nC;
			index = recvnumsdispls[cmptarg];
			allrecvids[index + indices[cmptarg]] = pdata->id;
			indices[cmptarg]++;
			if(cmpToRepMap[pdata->target] != -1) {
				index = recvnumsdispls[reptarg];
				allrecvids[index + indices[reptarg]] = pdata->id;
				indices[reptarg]++;
			}
		}
		pdata = pdata->prev;
	}
	_real_free(indices);
	
	/*for(int i = 0; i<nC+nR;i++) {
		pdata = first_peertopeer;
		while(pdata != NULL) {
			if((pdata->type == MPI_FT_RECV) && ((pdata->target == i) || ((cmpToRepMap[pdata->target] != -1) && ((cmpToRepMap[pdata->target]+nC) == i))) && pdata->completecmp && pdata->completerep) {
				allrecvids[index] = pdata->id;
				index++;
			}
			pdata = pdata->prev;
		}
		if(i > 0) recvnumsdispls[i] = recvnumsdispls[i-1] + recvnums[i-1];
		if(i > 0) recvfrommenumsdispls[i] = recvfrommenumsdispls[i-1] + recvfrommenums[i-1];
	}*/
	
	EMPI_Alltoallv(allrecvids,recvnums,recvnumsdispls,EMPI_INT,allsendids,recvfrommenums,recvfrommenumsdispls,EMPI_INT,MPI_COMM_WORLD->eworldComm);
	
	index = 0;
	if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
		pdata = first_peertopeer;
		while(pdata != NULL) {
			if((pdata->type == MPI_FT_SEND)) {
				int cmptarg = pdata->target;
				int reptarg = cmpToRepMap[pdata->target]+nC;
				bool cmpresend = true;
				bool represend = (cmpToRepMap[pdata->target] != -1) && (cmpToRepMap[cmprank] == -1);
				EMPI_Status status;
				index = recvfrommenumsdispls[cmptarg];
				for(int j = 0; j<recvfrommenums[cmptarg];j++) {
					if(allsendids[index+j] == pdata->id) {
						pdata->completecmp = true;
						if(pdata->req != NULL) {
							if((*(pdata->req) != NULL) && (*(pdata->req) != MPI_REQUEST_NULL)) {
								*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
							}
						}
						allsendids[index+j] = -1;
						cmpresend  = false;
					}
				}
				if(represend) {
					index = recvfrommenumsdispls[reptarg];
					for(int j = 0; j<recvfrommenums[reptarg];j++) {
						if(allsendids[index+j] == pdata->id) {
							pdata->completerep = true;
							if(pdata->req != NULL) {
								if((*(pdata->req) != NULL) && (*(pdata->req) != MPI_REQUEST_NULL)) {
									*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
								}
							}
							allsendids[index+j] = -1;
							represend  = false;
						}
					}
				}
				if(cmpresend) {
					int retVal = 0;
					bool no_req_exists;
					void *tmpbufloc;
					if(!(pdata->completecmp)) {
						tmpbufloc = (*(pdata->req))->bufloc;
					}
					
					no_req_exists = (pdata->req == NULL);
					if(no_req_exists) {
						pdata->req = (MPI_Request *)parep_mpi_malloc(sizeof(MPI_Request));
						*(pdata->req) = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
						(*(pdata->req))->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
						(*(pdata->req))->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
						*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
						*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
					} else {
						*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
						pdata->completecmp = false;
					}
					
					int extracount;
					int dis;
					int size;
					EMPI_Type_size(pdata->dt->edatatype,&size);
					dis = pdata->count*size;
					if(size >= sizeof(int)) extracount = 1;
					else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
					else extracount = (((int)sizeof(int))/size) + 1;
					
					retVal = EMPI_Isend (pdata->buf, (pdata->count) + extracount, pdata->dt->edatatype, pdata->target, pdata->tag, (pdata->comm->EMPI_COMM_CMP), ((*(pdata->req))->reqcmp));
					int flag = 0;
					if(pdata->completecmp) {
						do {
							EMPI_Test(((*(pdata->req))->reqcmp),&flag,&status);
						} while(flag == 0);
						if(no_req_exists) {
							parep_mpi_free((*(pdata->req))->reqcmp);
							parep_mpi_free((*(pdata->req))->reqrep);
							parep_mpi_free(*(pdata->req));
							*(pdata->req) = MPI_REQUEST_NULL;
							parep_mpi_free(pdata->req);
							pdata->req = NULL;
						}
					} else {
						(*(pdata->req))->bufloc = tmpbufloc;
						(*(pdata->req))->status.count = (pdata->count);
						(*(pdata->req))->status.MPI_SOURCE = pdata->target;
						(*(pdata->req))->status.MPI_TAG = pdata->tag;
						(*(pdata->req))->status.MPI_ERROR = 0;
						(*(pdata->req))->complete = false;
						(*(pdata->req))->comm = pdata->comm;
						(*(pdata->req))->type = MPI_FT_SEND_REQUEST;
						(*(pdata->req))->storeloc = pdata; 
					}
				}
				if(represend) {
					int retVal = 0;
					bool no_req_exists;
					void *tmpbufloc;
					if(!(pdata->completerep && (cmpToRepMap[myrank] == -1))) {
						tmpbufloc = (*(pdata->req))->bufloc;
					}
					
					no_req_exists = (pdata->req == NULL);
					if(no_req_exists) {
						pdata->req = (MPI_Request *)parep_mpi_malloc(sizeof(MPI_Request));
						*(pdata->req) = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
						(*(pdata->req))->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
						(*(pdata->req))->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
						*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
						*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
					} else {
						*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
						if(cmpToRepMap[myrank] == -1) pdata->completerep = false;
						else pdata->completerep = true;
					}
					
					int extracount;
					int dis;
					int size;
					EMPI_Type_size(pdata->dt->edatatype,&size);
					dis = pdata->count*size;
					if(size >= sizeof(int)) extracount = 1;
					else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
					else extracount = (((int)sizeof(int))/size) + 1;
					
					if(cmpToRepMap[myrank] == -1) retVal = EMPI_Isend (pdata->buf, (pdata->count) + extracount, pdata->dt->edatatype, cmpToRepMap[pdata->target], pdata->tag, (pdata->comm->EMPI_CMP_REP_INTERCOMM), ((*(pdata->req))->reqrep));
					int flag = (cmpToRepMap[myrank] != -1);
					if((pdata->completerep && (cmpToRepMap[myrank] == -1))) {
						do {
							if(cmpToRepMap[myrank] == -1) EMPI_Test(((*(pdata->req))->reqrep),&flag,&status);
						} while(flag == 0);
						if(no_req_exists) {
							parep_mpi_free((*(pdata->req))->reqcmp);
							parep_mpi_free((*(pdata->req))->reqrep);
							parep_mpi_free(*(pdata->req));
							*(pdata->req) = MPI_REQUEST_NULL;
							parep_mpi_free(pdata->req);
							pdata->req = NULL;
						}
					} else {
						(*(pdata->req))->bufloc = tmpbufloc;
						(*(pdata->req))->status.count = (pdata->count);
						(*(pdata->req))->status.MPI_SOURCE = pdata->target;
						(*(pdata->req))->status.MPI_TAG = pdata->tag;
						(*(pdata->req))->status.MPI_ERROR = 0;
						(*(pdata->req))->complete = false;
						(*(pdata->req))->comm = pdata->comm;
						(*(pdata->req))->type = MPI_FT_SEND_REQUEST;
						(*(pdata->req))->storeloc = pdata; 
					}
				}
			}
			pdata = pdata->prev;
		}
		index = 0;
		for(int i = 0; i < nC+nR; i++) {
			if(i != cmprank && (cmpToRepMap[cmprank] == -1 || i != cmpToRepMap[cmprank] + nC)) {
				if(((i < nC) || (cmpToRepMap[cmprank] == -1))) {
					for(int j = 0; j<recvfrommenums[i];j++) {
						if(allsendids[index+j] != -1) {
							if(allsendids[index+j] >= parep_mpi_sendid) {
								struct skiplist *skipitem = (struct skiplist *)parep_mpi_malloc(sizeof(struct skiplist));
								skipitem->id = allsendids[index+j];
								if(i < nC) {
									skipitem->next = skipcmplist;
									skipcmplist = skipitem;
								} else {
									skipitem->next = skipreplist;
									skipreplist = skipitem;
								}
							}
						}
					}
				}
			}
			index = index + recvfrommenums[i];
		}
	} else if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
		pdata = first_peertopeer;
		while(pdata != NULL) {
			if((pdata->type == MPI_FT_SEND) && (cmpToRepMap[pdata->target] != -1)) {
				int cmptarg = pdata->target;
				int reptarg = cmpToRepMap[pdata->target]+nC;
				bool resend = true;
				EMPI_Status status;
				index = recvfrommenumsdispls[reptarg];
				for(int j = 0; j<recvfrommenums[reptarg];j++) {
					if(allsendids[index+j] == pdata->id) {
						pdata->completecmp = true;
						if(pdata->req != NULL) {
							if((*(pdata->req) != NULL) && (*(pdata->req) != MPI_REQUEST_NULL)) {
								*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
							}
						}
						pdata->completerep = true;
						if(pdata->req != NULL) {
							if((*(pdata->req) != NULL) && (*(pdata->req) != MPI_REQUEST_NULL)) {
								*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
							}
						}
						allsendids[index+j] = -1;
						resend  = false;
					}
				}
				if(resend) {
					int retVal = 0;
					bool no_req_exists;
					void *tmpbufloc;
					if(!pdata->completecmp) {
						pdata->completerep = false;
						pdata->completecmp = true;
					}
					if(!(pdata->completerep)) {
						tmpbufloc = (*(pdata->req))->bufloc;
					}
					
					no_req_exists = (pdata->req == NULL);
					if(no_req_exists) {
						pdata->req = (MPI_Request *)parep_mpi_malloc(sizeof(MPI_Request));
						*(pdata->req) = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
						(*(pdata->req))->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
						(*(pdata->req))->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
						*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
						*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
					} else {
						*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
						*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
					}
					
					int extracount;
					int dis;
					int size;
					EMPI_Type_size(pdata->dt->edatatype,&size);
					dis = pdata->count*size;
					if(size >= sizeof(int)) extracount = 1;
					else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
					else extracount = (((int)sizeof(int))/size) + 1;
					retVal = EMPI_Isend(pdata->buf, (pdata->count) + extracount, pdata->dt->edatatype, cmpToRepMap[pdata->target], pdata->tag, (pdata->comm->EMPI_COMM_REP),((*(pdata->req))->reqrep));
					int flag = 0;
					if(pdata->completerep) {
						do {
							EMPI_Test(((*(pdata->req))->reqrep),&flag,&status);
						} while(flag == 0);
						if(no_req_exists) {
							parep_mpi_free((*(pdata->req))->reqcmp);
							parep_mpi_free((*(pdata->req))->reqrep);
							parep_mpi_free(*(pdata->req));
							*(pdata->req) = MPI_REQUEST_NULL;
							parep_mpi_free(pdata->req);
							pdata->req = NULL;
						}
					} else {
						(*(pdata->req))->bufloc = tmpbufloc;
						(*(pdata->req))->status.count = (pdata->count);
						(*(pdata->req))->status.MPI_SOURCE = pdata->target;
						(*(pdata->req))->status.MPI_TAG = pdata->tag;
						(*(pdata->req))->status.MPI_ERROR = 0;
						(*(pdata->req))->complete = false;
						(*(pdata->req))->comm = pdata->comm;
						(*(pdata->req))->type = MPI_FT_SEND_REQUEST;
						(*(pdata->req))->storeloc = pdata;
					}
				}
			}
			pdata = pdata->prev;
		}
		index = 0;
		for(int i = 0; i < nC+nR; i++) {
			if(i != repToCmpMap[reprank] &&  i != reprank + nC) {
				if(i >= nC) {
					for(int j = 0; j<recvfrommenums[i];j++) {
						if(allsendids[index+j] != -1) {
							if(allsendids[index+j] >= parep_mpi_sendid) {
								struct skiplist *skipitem = (struct skiplist *)parep_mpi_malloc(sizeof(struct skiplist));
								skipitem->id = allsendids[index+j];
								skipitem->next = skipreplist;
								skipreplist = skipitem;
							}
						}
					}
				}
			}
			index = index + recvfrommenums[i];
		}
	}
	
	/*index = 0;
	for(int i = 0; i<nC+nR;i++) {
		if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
			if(i != cmprank && (cmpToRepMap[cmprank] == -1 || i != cmpToRepMap[cmprank] + nC)) {
				if(((i < nC) || (cmpToRepMap[cmprank] == -1))) {
					EMPI_Status status;
					//sendids = allsendids + offset;
					
					pdata = first_peertopeer;
					while(pdata != NULL) {
						if((pdata->type == MPI_FT_SEND) && (pdata->target == i || ((cmpToRepMap[pdata->target] != -1) && (cmpToRepMap[pdata->target] + nC) == i))) {
							bool resend = true;
							for(int j = 0; j<recvfrommenums[i];j++) {
								if(allsendids[index+j] == pdata->id) {
									if(i < nC) {
										pdata->completecmp = true;
										if(pdata->req != NULL) {
											if((*(pdata->req) != NULL) && (*(pdata->req) != MPI_REQUEST_NULL)) {
												*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
											}
										}
									} else if(cmpToRepMap[cmprank] == -1) {
										pdata->completerep = true;
										if(pdata->req != NULL) {
											if((*(pdata->req) != NULL) && (*(pdata->req) != MPI_REQUEST_NULL)) {
												*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
											}
										}
									}
									allsendids[index+j] = -1;
									resend  = false;
								}
							}
							if(resend) {
								//if((i < nC) || (cmpToRepMap[myrank] == -1)) printf("CMP Resending %d data with tag %d to %d my_rank %d i %d id %d sendnums %d recvfrommenums %d completecmp %d completerep %d\n",(pdata->count)+1,pdata->tag,pdata->target,myrank,i,pdata->id,sendnums[i],recvfrommenums[i],(pdata->completecmp),(pdata->completerep));
								int retVal = 0;
								bool no_req_exists;
								void *tmpbufloc;
								if(!((pdata->completecmp && (i < nC)) || (pdata->completerep && (i >= nC) && (cmpToRepMap[myrank] == -1)))) {
									tmpbufloc = (*(pdata->req))->bufloc;
								}
								
								no_req_exists = (pdata->req == NULL);
								if(no_req_exists) {
									pdata->req = (MPI_Request *)parep_mpi_malloc(sizeof(MPI_Request));
									*(pdata->req) = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
									(*(pdata->req))->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
									(*(pdata->req))->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
									*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
									*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
								} else {
									*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
									*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
									if(i < nC) pdata->completecmp = false;
									else if(cmpToRepMap[myrank] == -1) pdata->completerep = false;
									else pdata->completerep = true;
								}
								
								int extracount;
								int dis;
								int size;
								EMPI_Type_size(pdata->dt->edatatype,&size);
								dis = pdata->count*size;
								if(size >= sizeof(int)) extracount = 1;
								else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
								else extracount = (((int)sizeof(int))/size) + 1;
								
								//pdata->completecmp = false;
								//pdata->completerep = false;
								
								if(i < nC) retVal = EMPI_Isend (pdata->buf, (pdata->count) + extracount, pdata->dt->edatatype, pdata->target, pdata->tag, (pdata->comm->EMPI_COMM_CMP), ((*(pdata->req))->reqcmp));
								//else pdata->completecmp = true;
								if((i >= nC) && (cmpToRepMap[myrank] == -1)) retVal = EMPI_Isend (pdata->buf, (pdata->count) + extracount, pdata->dt->edatatype, cmpToRepMap[pdata->target], pdata->tag, (pdata->comm->EMPI_CMP_REP_INTERCOMM), ((*(pdata->req))->reqrep));
								//else pdata->completerep = true;
								int flag = (i >= nC) && (cmpToRepMap[myrank] != -1);
								if((pdata->completecmp && (i < nC)) || (pdata->completerep && (i >= nC) && (cmpToRepMap[myrank] == -1))) {
									do {
										if(i < nC) EMPI_Test(((*(pdata->req))->reqcmp),&flag,&status);
										else if(cmpToRepMap[myrank] == -1) EMPI_Test(((*(pdata->req))->reqrep),&flag,&status);
									} while(flag == 0);
									if(no_req_exists) {
										parep_mpi_free((*(pdata->req))->reqcmp);
										parep_mpi_free((*(pdata->req))->reqrep);
										parep_mpi_free(*(pdata->req));
										*(pdata->req) = MPI_REQUEST_NULL;
										parep_mpi_free(pdata->req);
										pdata->req = NULL;
									}
								} else {
									(*(pdata->req))->bufloc = tmpbufloc;
									(*(pdata->req))->status.count = (pdata->count);
									(*(pdata->req))->status.MPI_SOURCE = pdata->target;
									(*(pdata->req))->status.MPI_TAG = pdata->tag;
									(*(pdata->req))->status.MPI_ERROR = 0;
									(*(pdata->req))->complete = false;
									(*(pdata->req))->comm = pdata->comm;
									(*(pdata->req))->type = MPI_FT_SEND_REQUEST;
									(*(pdata->req))->storeloc = pdata; 
								}
							}
						}
						pdata = pdata->prev;
					}
					for(int j = 0; j<recvfrommenums[i];j++) {
						if(allsendids[index+j] != -1) {
							if(allsendids[index+j] >= parep_mpi_sendid) {
								struct skiplist *skipitem = (struct skiplist *)parep_mpi_malloc(sizeof(struct skiplist));
								skipitem->id = allsendids[index+j];
								if(i < nC) {
									skipitem->next = skipcmplist;
									skipcmplist = skipitem;
								}
								else {
									skipitem->next = skipreplist;
									skipreplist = skipitem;
								}
							}
						}
					}
				}
			}
		}
		else if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
			if(i != repToCmpMap[reprank] &&  i != reprank + nC) {
				if(i >= nC) {
					EMPI_Status status;
					
					pdata = first_peertopeer;
					while(pdata != NULL) {
						if((pdata->type == MPI_FT_SEND) && ((cmpToRepMap[pdata->target] != -1) && (cmpToRepMap[pdata->target] + nC) == i)) {
							bool resend = true;
							for(int j = 0; j<recvfrommenums[i];j++) {
								if(allsendids[index+j] == pdata->id) {
									pdata->completecmp = true;
									if(pdata->req != NULL) {
										if((*(pdata->req) != NULL) && (*(pdata->req) != MPI_REQUEST_NULL)) {
											*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
										}
									}
									pdata->completerep = true;
									if(pdata->req != NULL) {
										if((*(pdata->req) != NULL) && (*(pdata->req) != MPI_REQUEST_NULL)) {
											*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
										}
									}
									allsendids[index+j] = -1;
									resend  = false;
								}
							}
							if(resend) {
								int retVal = 0;
								bool no_req_exists;
								void *tmpbufloc;
								if(!pdata->completecmp) {
									pdata->completerep = false;
									pdata->completecmp = true;
								}
								if(!(pdata->completerep)) {
									tmpbufloc = (*(pdata->req))->bufloc;
								}
								
								no_req_exists = (pdata->req == NULL);
								if(no_req_exists) {
									pdata->req = (MPI_Request *)parep_mpi_malloc(sizeof(MPI_Request));
									*(pdata->req) = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
									(*(pdata->req))->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
									(*(pdata->req))->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
									*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
									*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
								} else {
									*((*(pdata->req))->reqcmp) = EMPI_REQUEST_NULL;
									*((*(pdata->req))->reqrep) = EMPI_REQUEST_NULL;
								}
								
								int extracount;
								int dis;
								//void *backup;
								int size;
								EMPI_Type_size(pdata->dt->edatatype,&size);
								dis = pdata->count*size;
								if(size >= sizeof(int)) extracount = 1;
								else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
								else extracount = (((int)sizeof(int))/size) + 1;
								//*((int *)(pdata->buf+dis)) = pdata->id;
								retVal = EMPI_Isend(pdata->buf, (pdata->count) + extracount, pdata->dt->edatatype, cmpToRepMap[pdata->target], pdata->tag, (pdata->comm->EMPI_COMM_REP),((*(pdata->req))->reqrep));
								int flag = 0;
								if(pdata->completerep) {
									do {
										EMPI_Test(((*(pdata->req))->reqrep),&flag,&status);
									} while(flag == 0);
									if(no_req_exists) {
										parep_mpi_free((*(pdata->req))->reqcmp);
										parep_mpi_free((*(pdata->req))->reqrep);
										parep_mpi_free(*(pdata->req));
										*(pdata->req) = MPI_REQUEST_NULL;
										parep_mpi_free(pdata->req);
										pdata->req = NULL;
									}
								} else {
									(*(pdata->req))->bufloc = tmpbufloc;
									(*(pdata->req))->status.count = (pdata->count);
									(*(pdata->req))->status.MPI_SOURCE = pdata->target;
									(*(pdata->req))->status.MPI_TAG = pdata->tag;
									(*(pdata->req))->status.MPI_ERROR = 0;
									(*(pdata->req))->complete = false;
									(*(pdata->req))->comm = pdata->comm;
									(*(pdata->req))->type = MPI_FT_SEND_REQUEST;
									(*(pdata->req))->storeloc = pdata;
								}
							}
						}
						pdata = pdata->prev;
					}
					for(int j = 0; j<recvfrommenums[i];j++) {
						if(allsendids[index+j] != -1) {
							if(allsendids[index+j] >= parep_mpi_sendid) {
								struct skiplist *skipitem = (struct skiplist *)parep_mpi_malloc(sizeof(struct skiplist));
								skipitem->id = allsendids[index+j];
								skipitem->next = skipreplist;
								skipreplist = skipitem;
							}
						}
					}
				}
			}
		}
		index = index + recvfrommenums[i];
	}*/
	_real_free(allrecvids);
	_real_free(allsendids);
	
	pthread_mutex_unlock(&peertopeerLock);
	
	int barflag = 0;
	EMPI_Request barreq;
	EMPI_Ibarrier(MPI_COMM_WORLD->eworldComm,&barreq);
	pthread_rwlock_unlock(&commLock);
	
	do {
		bool progressed;
		do {
			progressed = test_all_ptp_requests();
		} while(progressed);
		pthread_rwlock_rdlock(&commLock);
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
								printf("%d: Wrong id probed at restore_messages %p myrank %d src %d\n",getpid(),curargs->id,myrank,j);
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
								printf("%d: Wrong id probed at restore_messages %p myrank %d src %d\n",getpid(),curargs->id,myrank,j);
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
		EMPI_Test(&barreq,&barflag,EMPI_STATUS_IGNORE);
	} while(barflag == 0);
	pthread_rwlock_rdlock(&commLock);
	pthread_mutex_lock(&peertopeerLock);
	
	for(int i = 0; i < nC; i++) {
		if(collids[i] < minid) {
			minid = collids[i];
		}
	}
	int cmpid = minid;
	int repid = -1;
	int myrepid = -1;
	if(nR > 0) {
		repid = collids[nC];
		for(int i = nC; i < nC+nR; i++) {
			if(collids[i] < repid) {
				repid = collids[i];
			}
		}
		if(repid < minid && repid >= 0) minid = repid;
	}
	if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
		if(cmpToRepMap[myrank] != -1) {
			myrepid = collids[cmpToRepMap[myrank]+nC];
		}
	}
	struct collective_data *retry_data = NULL;
	if(last_collective != NULL) {
		if(minid < last_collective->id) {
			retry_data = last_collective;
			while(retry_data->next != NULL) {
				if(minid >= retry_data->next->id) break;
				retry_data = retry_data->next;
			}
		}
	} else {
		retry_data = NULL;
	}
	
	_real_free(sendnums);
	_real_free(recvnums);
	_real_free(recvnumsdispls);
	_real_free(senttomenums);
	_real_free(recvfrommenums);
	_real_free(recvfrommenumsdispls);
	
	if(last_completed_collective != NULL) {
		if(last_completed_collective->req != NULL) {
			if(*(last_completed_collective->req) != MPI_REQUEST_NULL) {
				(*(last_completed_collective->req))->complete = false;
			}
		}
	}
	
	mpi_ft_collective_from_args(retry_data,cmpid,repid,myrepid,redids,reddataids,collids);
	
	pthread_mutex_unlock(&collectiveLock);
	pthread_rwlock_unlock(&commLock);
	probe_reduce_messages();
	pthread_rwlock_rdlock(&commLock);
	pthread_mutex_lock(&collectiveLock);
}

#define PAREP_MPI_REPLICATION_TAG 1234

void parep_mpi_send_replication_data(int dest_rank) {
	extern address __data_start;
	extern address _edata;
	extern address __bss_start;
	extern address _end;
	tagbase = 0;
	address my_req_null = (address)MPI_REQUEST_NULL;
	
	address offset;
	char mapname[256];
	address fsbasekey;
	//asm volatile("\t rdfsbase %0" : "=r"(fsbasekey));
	syscall(SYS_arch_prctl,ARCH_GET_FS,&fsbasekey);
	fsbasekey = *((address *)(fsbasekey+0x28));
	
	Context processContext;
	processContext.rip = getPC(parep_mpi_replication_initializer);
	processContext.rsp = getRSP(parep_mpi_replication_initializer);
	processContext.rbp = getRBP(parep_mpi_replication_initializer);
	
	EMPI_Send(&fsbasekey,sizeof(address),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&processContext,sizeof(Context),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&parep_mpi_replication_initializer,sizeof(jmp_buf),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	
	EMPI_Send(&parep_mpi_num_reg_vals,1,EMPI_INT,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	
	for(int i = 0; i < parep_mpi_num_reg_vals; i++) {
		for(int k = 0; k < 15; k++) {
			offset = 0;
			mapname[0] = '\0';
			if((parep_mpi_reg_vals[i][k] == (address)NULL) || (parep_mpi_reg_vals[i][k] <= (address)(&(_end))) || (((address)parep_mpi_ext_heap_mapping <= parep_mpi_reg_vals[i][k]) && (((address)parep_mpi_ext_heap_mapping + parep_mpi_ext_heap_size) > parep_mpi_reg_vals[i][k])) || ((parep_mpi_new_stack_start <= parep_mpi_reg_vals[i][k]) && (parep_mpi_new_stack_end > parep_mpi_reg_vals[i][k]))) {
				offset = parep_mpi_reg_vals[i][k];
				EMPI_Send(mapname,MAX_PATH_LEN,EMPI_CHAR,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
				EMPI_Send(&offset,sizeof(address),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
			} else {
				bool found = false;
				for(int j = 0; j < num_mappings; j++) {
					if((mappings[j].start <= parep_mpi_reg_vals[i][k]) && (mappings[j].end > parep_mpi_reg_vals[i][k])) {
						if(mappings[j].pathname[0] == '\0') {
							while(mappings[j].pathname[0] == '\0') j--;
						}
						strcpy(mapname,mappings[j].pathname);
						while(!strcmp(mapname,mappings[j].pathname)) j--;
						j++;
						offset = parep_mpi_reg_vals[i][k] - mappings[j].start;
						found = true;
						break;
					}
				}
				if(!found) {
					mapname[0] = '\0';
					offset = parep_mpi_reg_vals[i][k];
				}
				EMPI_Send(mapname,MAX_PATH_LEN,EMPI_CHAR,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
				EMPI_Send(&offset,sizeof(address),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
			}
		}
	}
	
	int init_data_seg_size = (((char *)(&_edata)) - ((char *)(&__data_start)));
	int uinit_data_seg_size = (((char *)(&_end)) - ((char *)(&__bss_start)));
	
	struct mpi_ft_comm mpi_ft_comm_world_dup,mpi_ft_comm_self_dup;
	memcpy(&mpi_ft_comm_world_dup,&mpi_ft_comm_world,sizeof(struct mpi_ft_comm));
	memcpy(&mpi_ft_comm_self_dup,&mpi_ft_comm_self,sizeof(struct mpi_ft_comm));
	
	//SEND DATA
	EMPI_Send(&init_data_seg_size,1,EMPI_INT,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&uinit_data_seg_size,1,EMPI_INT,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	
	EMPI_Send(&__data_start,init_data_seg_size,EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(reqarr,1024*sizeof(MPI_Request),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(reqinuse,1024*sizeof(bool),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	if(parep_mpi_fortran_binding_used) {
		EMPI_Send(&__bss_start,uinit_data_seg_size,EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);
		tagbase++;
	}
	
	memcpy(&mpi_ft_comm_world,&mpi_ft_comm_world_dup,sizeof(struct mpi_ft_comm));
	memcpy(&mpi_ft_comm_self,&mpi_ft_comm_self_dup,sizeof(struct mpi_ft_comm));
	
	//SEND HEAP
	EMPI_Send(&parep_mpi_ext_heap_size,sizeof(size_t),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	pthread_mutex_lock(&heap_free_list_mutex);
	EMPI_Send(parep_mpi_bins,sizeof(bin_t)*BIN_COUNT,EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(parep_mpi_fastbins,sizeof(bin_t)*FASTBIN_COUNT,EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	heap_node_t *headnode = (heap_node_t *)parep_mpi_ext_heap_mapping;
	footer_t *footnode;
	
	for(int k = 0; k < parep_mpi_ext_heap_size/0x20000000; k++) {
		int size = 0x20000000;
		if(((long)k*(long)0x20000000) + size > (parep_mpi_ext_heap_size)) size = parep_mpi_ext_heap_size - ((long)k*(long)0x20000000);
		EMPI_Send(parep_mpi_ext_heap_mapping + ((long)k*(long)0x20000000),size,EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	}
	/*while((address)headnode < (address)(parep_mpi_ext_heap_mapping+parep_mpi_ext_heap_size)) {
		footnode = get_foot(headnode);
		EMPI_Send(headnode,sizeof(heap_node_t),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
		if((!GET_HOLE(headnode->hole_and_size)) && (!GET_FAST(headnode->hole_and_size))) {
			if(((address)footnode) - ((address)headnode) - sizeof(heap_node_t) > 0) {
				EMPI_Send((void *)(((address)headnode) + sizeof(heap_node_t)),((address)footnode) - ((address)headnode) - sizeof(heap_node_t),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);
				tagbase++;
			}
		}
		EMPI_Send(footnode,sizeof(footer_t),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
		headnode = (heap_node_t *)(((address)footnode) + sizeof(footer_t));
	}*/
	pthread_mutex_unlock(&heap_free_list_mutex);
	
	//SEND STACK
	EMPI_Send((void *)parep_mpi_new_stack_start,parep_mpi_new_stack_size,EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	
	EMPI_Send(&my_req_null,sizeof(address),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&first_peertopeer,sizeof(ptpdata *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&last_peertopeer,sizeof(ptpdata *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&last_collective,sizeof(clcdata *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	
	EMPI_Send(parep_mpi_commbuf_bins,sizeof(commbuf_bin_t)*COMMBUF_BIN_COUNT,EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	
	EMPI_Send(&recvDataHead,sizeof(recvDataNode *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&recvDataTail,sizeof(recvDataNode *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&recvDataRedHead,sizeof(recvDataNode *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&recvDataRedTail,sizeof(recvDataNode *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&skipcmplist,sizeof(struct skiplist *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&skipreplist,sizeof(struct skiplist *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&skipredlist,sizeof(struct skiplist *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&reqHead,sizeof(reqNode *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&reqTail,sizeof(reqNode *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&openFileHead,sizeof(openFileNode *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&openFileTail,sizeof(openFileNode *),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	
	openFileNode *onode = openFileHead;
	while(onode != NULL) {
		address vtableptr = *((address *)((((address)(onode->file)) + (onode->file->_vtable_offset)) + sizeof(FILE)));
		offset = 0;
		mapname[0] = '\0';
		bool found = false;
		for(int j = 0; j < num_mappings; j++) {
			if((mappings[j].start <= vtableptr) && (mappings[j].end > vtableptr)) {
				if(mappings[j].pathname[0] == '\0') {
					while(mappings[j].pathname[0] == '\0') j--;
				}
				strcpy(mapname,mappings[j].pathname);
				while(!strcmp(mapname,mappings[j].pathname)) j--;
				j++;
				offset = vtableptr - mappings[j].start;
				found = true;
				break;
			}
		}
		if(!found) {
			mapname[0] = '\0';
			offset = vtableptr;
		}
		EMPI_Send(mapname,MAX_PATH_LEN,EMPI_CHAR,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
		EMPI_Send(&offset,sizeof(address),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
		onode = onode->next;
	}
	
	EMPI_Send(&parep_mpi_store_buf_sz,sizeof(long),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&parep_mpi_sendid,sizeof(int),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
	EMPI_Send(&parep_mpi_collective_id,sizeof(int),EMPI_BYTE,dest_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM);tagbase++;
}

void parep_mpi_recv_replication_data(int src_rank) {
	extern address __data_start;
	extern address _edata;
	extern address __bss_start;
	extern address _end;
	tagbase = 0;
	address offset;
	address fsbasekey;
	address curfsbase;
	address src_req_null;
	char mapname[256];
	ptpdata *pdata;
	clcdata *cdata;
	heap_node_t *headnode;
	footer_t *footnode;
	
	address temprip,temprsp,temprbp;
	temprip = getPC(parep_mpi_replication_initializer);
	
	size_t hpsize,stksize;
	int init_data_seg_size;
	int uinit_data_seg_size;
	
	Context processContext;
	EMPI_Recv(&fsbasekey,sizeof(address),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&processContext,sizeof(Context),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&parep_mpi_replication_initializer,sizeof(jmp_buf),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	
	EMPI_Recv(&parep_mpi_num_reg_vals,1,EMPI_INT,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	
	for(int i = 0; i < parep_mpi_num_reg_vals; i++) {
		for(int k = 0; k < 15; k++) {
			EMPI_Recv(mapname,MAX_PATH_LEN,EMPI_CHAR,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
			EMPI_Recv(&offset,sizeof(address),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
			if(mapname[0] == '\0') parep_mpi_reg_vals[i][k] = offset;
			else {
				for(int j = 0; j < num_mappings; j++) {
					if((!strcmp(mappings[j].pathname,mapname)) || (strstr(mappings[j].pathname,"ib_shmem-kvs") && strstr(mapname,"ib_shmem-kvs"))) {
						parep_mpi_reg_vals[i][k] = offset + mappings[j].start;
						break;
					}
				}
			}
		}
	}
	
	struct mpi_ft_comm mpi_ft_comm_dup[3];
	struct mpi_ft_datatype mpi_ft_datatype_dup[64];
	struct mpi_ft_request mpi_ft_request_dup[1];
	struct mpi_ft_op mpi_ft_op_dup[32];
	
	address stdout_dup = (address)stdout;
	char _ZSt4cout_buf[0x110];
	memcpy(_ZSt4cout_buf,&_ZSt4cout,0x110);
	
	memcpy(&mpi_ft_comm_dup[0],&mpi_ft_comm_null,sizeof(struct mpi_ft_comm));
	memcpy(&mpi_ft_comm_dup[1],&mpi_ft_comm_world,sizeof(struct mpi_ft_comm));
	memcpy(&mpi_ft_comm_dup[2],&mpi_ft_comm_self,sizeof(struct mpi_ft_comm));
	
	memcpy(&mpi_ft_datatype_dup[0],&mpi_ft_datatype_null,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[1],&mpi_ft_datatype_char,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[2],&mpi_ft_datatype_signed_char,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[3],&mpi_ft_datatype_unsigned_char,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[4],&mpi_ft_datatype_byte,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[5],&mpi_ft_datatype_short,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[6],&mpi_ft_datatype_unsigned_short,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[7],&mpi_ft_datatype_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[8],&mpi_ft_datatype_unsigned,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[9],&mpi_ft_datatype_long,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[10],&mpi_ft_datatype_unsigned_long,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[11],&mpi_ft_datatype_float,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[12],&mpi_ft_datatype_double,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[13],&mpi_ft_datatype_long_double,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[14],&mpi_ft_datatype_long_long_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[15],&mpi_ft_datatype_unsigned_long_long,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[16],&mpi_ft_datatype_packed,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[17],&mpi_ft_datatype_float_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[18],&mpi_ft_datatype_double_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[19],&mpi_ft_datatype_long_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[20],&mpi_ft_datatype_short_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[21],&mpi_ft_datatype_2int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[22],&mpi_ft_datatype_long_double_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[23],&mpi_ft_datatype_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[24],&mpi_ft_datatype_double_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[25],&mpi_ft_datatype_logical,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[26],&mpi_ft_datatype_real,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[27],&mpi_ft_datatype_double_precision,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[28],&mpi_ft_datatype_integer,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[29],&mpi_ft_datatype_2integer,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[30],&mpi_ft_datatype_2real,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[31],&mpi_ft_datatype_2double_precision,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[32],&mpi_ft_datatype_character,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[33],&mpi_ft_datatype_int8_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[34],&mpi_ft_datatype_int16_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[35],&mpi_ft_datatype_int32_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[36],&mpi_ft_datatype_int64_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[37],&mpi_ft_datatype_uint8_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[38],&mpi_ft_datatype_uint16_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[39],&mpi_ft_datatype_uint32_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[40],&mpi_ft_datatype_uint64_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[41],&mpi_ft_datatype_c_bool,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[42],&mpi_ft_datatype_c_float_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[43],&mpi_ft_datatype_c_double_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[44],&mpi_ft_datatype_c_long_double_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[45],&mpi_ft_datatype_aint,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[46],&mpi_ft_datatype_offset,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[47],&mpi_ft_datatype_count,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[48],&mpi_ft_datatype_cxx_bool,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[49],&mpi_ft_datatype_cxx_float_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[50],&mpi_ft_datatype_cxx_double_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[51],&mpi_ft_datatype_cxx_long_double_complex,sizeof(struct mpi_ft_datatype));
	
	memcpy(&mpi_ft_request_dup[0],&mpi_ft_request_null,sizeof(struct mpi_ft_request));
	
	memcpy(&mpi_ft_op_dup[0],&mpi_ft_op_null,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[1],&mpi_ft_op_max,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[2],&mpi_ft_op_min,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[3],&mpi_ft_op_sum,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[4],&mpi_ft_op_prod,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[5],&mpi_ft_op_land,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[6],&mpi_ft_op_band,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[7],&mpi_ft_op_lor,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[8],&mpi_ft_op_bor,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[9],&mpi_ft_op_lxor,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[10],&mpi_ft_op_bxor,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[11],&mpi_ft_op_minloc,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[12],&mpi_ft_op_maxloc,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[13],&mpi_ft_op_replace,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[14],&mpi_ft_op_no_op,sizeof(struct mpi_ft_op));
	
	//RECV DATA
	EMPI_Recv(&init_data_seg_size,1,EMPI_INT,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),mpi_ft_comm_dup[1].EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&uinit_data_seg_size,1,EMPI_INT,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),mpi_ft_comm_dup[1].EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	
	EMPI_Recv(&__data_start,init_data_seg_size,EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),mpi_ft_comm_dup[1].EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(reqarr,1024*sizeof(MPI_Request),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),mpi_ft_comm_dup[1].EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(reqinuse,1024*sizeof(bool),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),mpi_ft_comm_dup[1].EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	if(parep_mpi_fortran_binding_used) {
		EMPI_Recv(&__bss_start,uinit_data_seg_size,EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),mpi_ft_comm_dup[1].EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);
		tagbase++;
	}
	
	stdout = (FILE *)stdout_dup;
	memcpy(&_ZSt4cout,_ZSt4cout_buf,0x110);
	
	memcpy(&mpi_ft_comm_null,&mpi_ft_comm_dup[0],sizeof(struct mpi_ft_comm));
	memcpy(&mpi_ft_comm_world,&mpi_ft_comm_dup[1],sizeof(struct mpi_ft_comm));
	memcpy(&mpi_ft_comm_self,&mpi_ft_comm_dup[2],sizeof(struct mpi_ft_comm));

	memcpy(&mpi_ft_datatype_null,&mpi_ft_datatype_dup[0],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_char,&mpi_ft_datatype_dup[1],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_signed_char,&mpi_ft_datatype_dup[2],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_unsigned_char,&mpi_ft_datatype_dup[3],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_byte,&mpi_ft_datatype_dup[4],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_short,&mpi_ft_datatype_dup[5],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_unsigned_short,&mpi_ft_datatype_dup[6],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_int,&mpi_ft_datatype_dup[7],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_unsigned,&mpi_ft_datatype_dup[8],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_long,&mpi_ft_datatype_dup[9],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_unsigned_long,&mpi_ft_datatype_dup[10],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_float,&mpi_ft_datatype_dup[11],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_double,&mpi_ft_datatype_dup[12],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_long_double,&mpi_ft_datatype_dup[13],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_long_long_int,&mpi_ft_datatype_dup[14],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_unsigned_long_long,&mpi_ft_datatype_dup[15],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_packed,&mpi_ft_datatype_dup[16],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_float_int,&mpi_ft_datatype_dup[17],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_double_int,&mpi_ft_datatype_dup[18],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_long_int,&mpi_ft_datatype_dup[19],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_short_int,&mpi_ft_datatype_dup[20],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_2int,&mpi_ft_datatype_dup[21],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_long_double_int,&mpi_ft_datatype_dup[22],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_complex,&mpi_ft_datatype_dup[23],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_double_complex,&mpi_ft_datatype_dup[24],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_logical,&mpi_ft_datatype_dup[25],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_real,&mpi_ft_datatype_dup[26],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_double_precision,&mpi_ft_datatype_dup[27],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_integer,&mpi_ft_datatype_dup[28],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_2integer,&mpi_ft_datatype_dup[29],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_2real,&mpi_ft_datatype_dup[30],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_2double_precision,&mpi_ft_datatype_dup[31],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_character,&mpi_ft_datatype_dup[32],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_int8_t,&mpi_ft_datatype_dup[33],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_int16_t,&mpi_ft_datatype_dup[34],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_int32_t,&mpi_ft_datatype_dup[35],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_int64_t,&mpi_ft_datatype_dup[36],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_uint8_t,&mpi_ft_datatype_dup[37],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_uint16_t,&mpi_ft_datatype_dup[38],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_uint32_t,&mpi_ft_datatype_dup[39],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_uint64_t,&mpi_ft_datatype_dup[40],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_c_bool,&mpi_ft_datatype_dup[41],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_c_float_complex,&mpi_ft_datatype_dup[42],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_c_double_complex,&mpi_ft_datatype_dup[43],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_c_long_double_complex,&mpi_ft_datatype_dup[44],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_aint,&mpi_ft_datatype_dup[45],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_offset,&mpi_ft_datatype_dup[46],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_count,&mpi_ft_datatype_dup[47],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_cxx_bool,&mpi_ft_datatype_dup[48],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_cxx_float_complex,&mpi_ft_datatype_dup[49],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_cxx_double_complex,&mpi_ft_datatype_dup[50],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_cxx_long_double_complex,&mpi_ft_datatype_dup[51],sizeof(struct mpi_ft_datatype));
	
	memcpy(&mpi_ft_request_null,&mpi_ft_request_dup[0],sizeof(struct mpi_ft_request));
	
	memcpy(&mpi_ft_op_null,&mpi_ft_op_dup[0],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_max,&mpi_ft_op_dup[1],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_min,&mpi_ft_op_dup[2],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_sum,&mpi_ft_op_dup[3],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_prod,&mpi_ft_op_dup[4],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_land,&mpi_ft_op_dup[5],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_band,&mpi_ft_op_dup[6],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_lor,&mpi_ft_op_dup[7],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_bor,&mpi_ft_op_dup[8],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_lxor,&mpi_ft_op_dup[9],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_bxor,&mpi_ft_op_dup[10],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_minloc,&mpi_ft_op_dup[11],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_maxloc,&mpi_ft_op_dup[12],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_replace,&mpi_ft_op_dup[13],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_no_op,&mpi_ft_op_dup[14],sizeof(struct mpi_ft_op));
	
	//RECV HEAP
	EMPI_Recv(&hpsize,sizeof(size_t),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	pthread_mutex_lock(&heap_free_list_mutex);
	if(hpsize != parep_mpi_ext_heap_size) {
		if(hpsize < parep_mpi_ext_heap_size) {
			void *ctr_heap = mremap((void *)parep_mpi_ext_heap_mapping,parep_mpi_ext_heap_size,hpsize,0);
			if(ctr_heap == MAP_FAILED) {
				printf("%d: Heap shrink failed\n",getpid());
				exit(0);
			}
			assert((address)ctr_heap == (address)parep_mpi_ext_heap_mapping);
			assert(parep_mpi_heap.start == (address)parep_mpi_ext_heap_mapping);
			parep_mpi_ext_heap_size = hpsize;
			parep_mpi_heap.end = (address)parep_mpi_ext_heap_mapping + (address)parep_mpi_ext_heap_size;
		}
		//assert((hpsize >= parep_mpi_ext_heap_size) && (hpsize % parep_mpi_ext_heap_size == 0));
		expand_heap(hpsize - parep_mpi_ext_heap_size);
	}
	EMPI_Recv(parep_mpi_bins,sizeof(bin_t)*BIN_COUNT,EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(parep_mpi_fastbins,sizeof(bin_t)*FASTBIN_COUNT,EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	
	headnode = (heap_node_t *)parep_mpi_ext_heap_mapping;
	for(int k = 0; k < parep_mpi_ext_heap_size/0x20000000; k++) {
		int size = 0x20000000;
		if(((long)k*(long)0x20000000) + size > (parep_mpi_ext_heap_size)) size = parep_mpi_ext_heap_size - ((long)k*(long)0x20000000);
		EMPI_Recv(parep_mpi_ext_heap_mapping + ((long)k*(long)0x20000000),size,EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	}
	/*while((address)headnode < (address)(parep_mpi_ext_heap_mapping+parep_mpi_ext_heap_size)) {
		EMPI_Recv(headnode,sizeof(heap_node_t),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
		footnode = get_foot(headnode);
		if((!GET_HOLE(headnode->hole_and_size)) && (!GET_FAST(headnode->hole_and_size)) && (GET_SIZE(headnode->hole_and_size) > 0)) {
			if(((address)footnode) - ((address)headnode) - sizeof(heap_node_t) > 0) {
				EMPI_Recv((void *)(((address)headnode) + sizeof(heap_node_t)),((address)footnode) - ((address)headnode) - sizeof(heap_node_t),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);
				tagbase++;
			}
		} else {
			if((GET_SIZE(headnode->hole_and_size) > 0)) assert((((address)headnode + GET_SIZE(headnode->hole_and_size) + sizeof(heap_node_t)) == (address)footnode));
			else assert((GET_SIZE(headnode->hole_and_size) == 0) && (((address)headnode + sizeof(heap_node_t)) == (address)footnode));
		}
		EMPI_Recv(footnode,sizeof(footer_t),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
		headnode = (heap_node_t *)(((address)footnode) + sizeof(footer_t));
	}*/
	pthread_mutex_unlock(&heap_free_list_mutex);

	//RECV STACK
	EMPI_Recv((void *)parep_mpi_new_stack_start,parep_mpi_new_stack_size,EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	
	EMPI_Recv(&src_req_null,sizeof(address),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&first_peertopeer,sizeof(ptpdata *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&last_peertopeer,sizeof(ptpdata *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&last_collective,sizeof(clcdata *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	
	EMPI_Recv(parep_mpi_commbuf_bins,sizeof(commbuf_bin_t)*COMMBUF_BIN_COUNT,EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	
	EMPI_Recv(&recvDataHead,sizeof(recvDataNode *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&recvDataTail,sizeof(recvDataNode *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&recvDataRedHead,sizeof(recvDataNode *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&recvDataRedTail,sizeof(recvDataNode *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&skipcmplist,sizeof(struct skiplist *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&skipreplist,sizeof(struct skiplist *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&skipredlist,sizeof(struct skiplist *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&reqHead,sizeof(reqNode *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&reqTail,sizeof(reqNode *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&openFileHead,sizeof(reqNode *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&openFileTail,sizeof(reqNode *),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	
	openFileNode *onode = openFileHead;
	while(onode != NULL) {
		address *vtableptraddr = ((address *)((((address)(onode->file)) + (onode->file->_vtable_offset)) + sizeof(FILE)));
		EMPI_Recv(mapname,MAX_PATH_LEN,EMPI_CHAR,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
		EMPI_Recv(&offset,sizeof(address),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
		if(mapname[0] == '\0') *vtableptraddr = offset;
		else {
			for(int j = 0; j < num_mappings; j++) {
				if(!strcmp(mappings[j].pathname,mapname)) {
					*vtableptraddr = offset + mappings[j].start;
					break;
				}
			}
		}
		onode = onode->next;
	}
	
	EMPI_Recv(&parep_mpi_store_buf_sz,sizeof(long),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&parep_mpi_sendid,sizeof(int),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	EMPI_Recv(&parep_mpi_collective_id,sizeof(int),EMPI_BYTE,src_rank,(int)((long)((long)(PAREP_MPI_REPLICATION_TAG) + ((long)tagbase)) % INT_MAX),MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,EMPI_STATUS_IGNORE);tagbase++;
	
	pdata = first_peertopeer;
	while(pdata != NULL) {
		if(pdata->req != NULL) {
			if(*((address *)(pdata->req)) == src_req_null) *(pdata->req) = MPI_REQUEST_NULL;
		}
		pdata = pdata->prev;
	}
	
	cdata = last_collective;
	while(cdata != NULL) {
		if(cdata->req != NULL) {
			if(*((address *)(cdata->req)) == src_req_null) *(cdata->req) = MPI_REQUEST_NULL;
		}
		cdata = cdata->next;
	}
	
	temprsp = processContext.rsp;
	temprbp = processContext.rbp;
	
	PTR_ENCRYPT(temprip);
	parep_mpi_replication_initializer[0].__jmpbuf[JB_PC] = temprip;
	PTR_ENCRYPT(temprsp);
	parep_mpi_replication_initializer[0].__jmpbuf[JB_RSP] = temprsp;
	PTR_ENCRYPT(temprbp);
	parep_mpi_replication_initializer[0].__jmpbuf[JB_RBP] = temprbp;
	
	//asm volatile("\t rdfsbase %0" : "=r"(curfsbase));
	syscall(SYS_arch_prctl,ARCH_GET_FS,&curfsbase);
	*((address *)(curfsbase+0x28)) = fsbasekey;
	
	IMB;
}

void parep_mpi_main_trampoline() {
	main(parep_mpi_argc,parep_mpi_argv);
}

void initialize_mpi_variables() {
	mpi_ft_comm_init(EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,&mpi_ft_comm_null);
	mpi_ft_op_init(EMPI_OP_NULL,&mpi_ft_op_null);
	mpi_ft_datatype_init(EMPI_DATATYPE_NULL,-1,&mpi_ft_datatype_null);
	mpi_ft_request_init(EMPI_REQUEST_NULL,EMPI_REQUEST_NULL,false,MPI_COMM_NULL,MPI_STATUS_IGNORE,-1,/*NULL,*/NULL,NULL,&mpi_ft_request_null);
	mpi_ft_comm_init(EMPI_COMM_SELF,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,&mpi_ft_comm_self);

	mpi_ft_datatype_init(EMPI_CHAR,sizeof(char),&mpi_ft_datatype_char);
	mpi_ft_datatype_init(EMPI_SIGNED_CHAR,sizeof(signed char),&mpi_ft_datatype_signed_char);
	mpi_ft_datatype_init(EMPI_UNSIGNED_CHAR,sizeof(unsigned char),&mpi_ft_datatype_unsigned_char);
	mpi_ft_datatype_init(EMPI_BYTE,1,&mpi_ft_datatype_byte);
	mpi_ft_datatype_init(EMPI_SHORT,sizeof(short),&mpi_ft_datatype_short);
	mpi_ft_datatype_init(EMPI_UNSIGNED_SHORT,sizeof(unsigned short),&mpi_ft_datatype_unsigned_short);
	mpi_ft_datatype_init(EMPI_INT,sizeof(int),&mpi_ft_datatype_int);
	mpi_ft_datatype_init(EMPI_UNSIGNED,sizeof(unsigned int),&mpi_ft_datatype_unsigned);
	mpi_ft_datatype_init(EMPI_LONG,sizeof(long),&mpi_ft_datatype_long);
	mpi_ft_datatype_init(EMPI_UNSIGNED_LONG,sizeof(unsigned long),&mpi_ft_datatype_unsigned_long);
	mpi_ft_datatype_init(EMPI_FLOAT,sizeof(float),&mpi_ft_datatype_float);
	mpi_ft_datatype_init(EMPI_DOUBLE,sizeof(double),&mpi_ft_datatype_double);
	mpi_ft_datatype_init(EMPI_LONG_DOUBLE,sizeof(long double),&mpi_ft_datatype_long_double);
	mpi_ft_datatype_init(EMPI_LONG_LONG_INT,sizeof(long long int),&mpi_ft_datatype_long_long_int);
	mpi_ft_datatype_init(EMPI_UNSIGNED_LONG_LONG,sizeof(unsigned long long),&mpi_ft_datatype_unsigned_long_long);

	mpi_ft_datatype_init(EMPI_PACKED,1,&mpi_ft_datatype_packed);
	//mpi_ft_datatype_init(EMPI_LB,sizeof(int),&mpi_ft_datatype_lb);
	//mpi_ft_datatype_init(EMPI_UB,sizeof(int),&mpi_ft_datatype_ub);
	mpi_ft_datatype_init(EMPI_FLOAT_INT,sizeof(float)+sizeof(int),&mpi_ft_datatype_float_int);
	mpi_ft_datatype_init(EMPI_DOUBLE_INT,sizeof(double)+sizeof(int),&mpi_ft_datatype_double_int);
	mpi_ft_datatype_init(EMPI_LONG_INT,sizeof(long)+sizeof(int),&mpi_ft_datatype_long_int);
	mpi_ft_datatype_init(EMPI_SHORT_INT,sizeof(short)+sizeof(int),&mpi_ft_datatype_short_int);
	mpi_ft_datatype_init(EMPI_2INT,sizeof(int)+sizeof(int),&mpi_ft_datatype_2int);
	mpi_ft_datatype_init(EMPI_LONG_DOUBLE_INT,sizeof(long double)+sizeof(int),&mpi_ft_datatype_long_double_int);
	mpi_ft_datatype_init(EMPI_COMPLEX,8,&mpi_ft_datatype_complex);
	mpi_ft_datatype_init(EMPI_DOUBLE_COMPLEX,16,&mpi_ft_datatype_double_complex);
	mpi_ft_datatype_init(EMPI_LOGICAL,4,&mpi_ft_datatype_logical);
	mpi_ft_datatype_init(EMPI_REAL,4,&mpi_ft_datatype_real);
	mpi_ft_datatype_init(EMPI_DOUBLE_PRECISION,8,&mpi_ft_datatype_double_precision);
	mpi_ft_datatype_init(EMPI_INTEGER,4,&mpi_ft_datatype_integer);
	mpi_ft_datatype_init(EMPI_2INTEGER,8,&mpi_ft_datatype_2integer);
	mpi_ft_datatype_init(EMPI_2REAL,8,&mpi_ft_datatype_2real);
	mpi_ft_datatype_init(EMPI_2DOUBLE_PRECISION,16,&mpi_ft_datatype_2double_precision);
	mpi_ft_datatype_init(EMPI_CHARACTER,1,&mpi_ft_datatype_character);
	//mpi_ft_datatype_init(EMPI_REAL4,4,&mpi_ft_datatype_real4);
	//mpi_ft_datatype_init(EMPI_REAL8,8,&mpi_ft_datatype_real8);
	//mpi_ft_datatype_init(EMPI_REAL16,16,&mpi_ft_datatype_real16);
	//mpi_ft_datatype_init(EMPI_COMPLEX8,8,&mpi_ft_datatype_complex8);
	//mpi_ft_datatype_init(EMPI_COMPLEX16,16,&mpi_ft_datatype_complex16);
	//mpi_ft_datatype_init(EMPI_COMPLEX32,32,&mpi_ft_datatype_complex32);
	//mpi_ft_datatype_init(EMPI_INTEGER1,1,&mpi_ft_datatype_integer1);
	//mpi_ft_datatype_init(EMPI_INTEGER2,2,&mpi_ft_datatype_integer2);
	//mpi_ft_datatype_init(EMPI_INTEGER4,4,&mpi_ft_datatype_integer4);
	//mpi_ft_datatype_init(EMPI_INTEGER8,8,&mpi_ft_datatype_integer8);
	//mpi_ft_datatype_init(EMPI_INTEGER16,16,&mpi_ft_datatype_integer16);
	mpi_ft_datatype_init(EMPI_INT8_T,sizeof(int8_t),&mpi_ft_datatype_int8_t);
	mpi_ft_datatype_init(EMPI_INT16_T,sizeof(int16_t),&mpi_ft_datatype_int16_t);
	mpi_ft_datatype_init(EMPI_INT32_T,sizeof(int32_t),&mpi_ft_datatype_int32_t);
	mpi_ft_datatype_init(EMPI_INT64_T,sizeof(int64_t),&mpi_ft_datatype_int64_t);
	mpi_ft_datatype_init(EMPI_UINT8_T,sizeof(uint8_t),&mpi_ft_datatype_uint8_t);
	mpi_ft_datatype_init(EMPI_UINT16_T,sizeof(uint16_t),&mpi_ft_datatype_uint16_t);
	mpi_ft_datatype_init(EMPI_UINT32_T,sizeof(uint32_t),&mpi_ft_datatype_uint32_t);
	mpi_ft_datatype_init(EMPI_UINT64_T,sizeof(uint64_t),&mpi_ft_datatype_uint64_t);
	mpi_ft_datatype_init(EMPI_C_BOOL,sizeof(bool),&mpi_ft_datatype_c_bool);
	mpi_ft_datatype_init(EMPI_C_FLOAT_COMPLEX,sizeof(float _Complex),&mpi_ft_datatype_c_float_complex);
	mpi_ft_datatype_init(EMPI_C_DOUBLE_COMPLEX,sizeof(double _Complex),&mpi_ft_datatype_c_double_complex);
	mpi_ft_datatype_init(EMPI_C_LONG_DOUBLE_COMPLEX,sizeof(long double _Complex),&mpi_ft_datatype_c_long_double_complex);
	mpi_ft_datatype_init(EMPI_AINT,4,&mpi_ft_datatype_aint);
	mpi_ft_datatype_init(EMPI_OFFSET,4,&mpi_ft_datatype_offset);
	mpi_ft_datatype_init(EMPI_COUNT,sizeof(int),&mpi_ft_datatype_count);
	mpi_ft_datatype_init(EMPI_CXX_BOOL,sizeof(bool),&mpi_ft_datatype_cxx_bool);
	mpi_ft_datatype_init(EMPI_CXX_FLOAT_COMPLEX,sizeof(float _Complex),&mpi_ft_datatype_cxx_float_complex);
	mpi_ft_datatype_init(EMPI_CXX_DOUBLE_COMPLEX,sizeof(double _Complex),&mpi_ft_datatype_cxx_double_complex);
	mpi_ft_datatype_init(EMPI_CXX_LONG_DOUBLE_COMPLEX,sizeof(long double _Complex),&mpi_ft_datatype_cxx_long_double_complex);

	mpi_ft_op_init(EMPI_MAX,&mpi_ft_op_max);
	mpi_ft_op_init(EMPI_MIN,&mpi_ft_op_min);
	mpi_ft_op_init(EMPI_SUM,&mpi_ft_op_sum);
	mpi_ft_op_init(EMPI_PROD,&mpi_ft_op_prod);
	mpi_ft_op_init(EMPI_LAND,&mpi_ft_op_land);
	mpi_ft_op_init(EMPI_BAND,&mpi_ft_op_band);
	mpi_ft_op_init(EMPI_LOR,&mpi_ft_op_lor);
	mpi_ft_op_init(EMPI_BOR,&mpi_ft_op_bor);
	mpi_ft_op_init(EMPI_LXOR,&mpi_ft_op_lxor);
	mpi_ft_op_init(EMPI_BXOR,&mpi_ft_op_bxor);
	mpi_ft_op_init(EMPI_MINLOC,&mpi_ft_op_minloc);
	mpi_ft_op_init(EMPI_MAXLOC,&mpi_ft_op_maxloc);
	mpi_ft_op_init(EMPI_REPLACE,&mpi_ft_op_replace);
	mpi_ft_op_init(EMPI_NO_OP,&mpi_ft_op_no_op);
	
	sprintf(extLibstr,"%s/lib/libmpi.so",getenv("PAREP_MPI_EMPI_PATH"));
	extLib = dlopen(extLibstr, RTLD_NOW);
	EMPI_Init = (int(*)(int *,char ***))dlsym(extLib,"MPI_Init");
	EMPI_Finalize = (int(*)())dlsym(extLib,"MPI_Finalize");
	EMPI_Comm_size = (int(*)(EMPI_Comm,int *))dlsym(extLib,"MPI_Comm_size");
	EMPI_Comm_remote_size = (int(*)(EMPI_Comm,int *))dlsym(extLib,"MPI_Comm_remote_size");
	EMPI_Comm_rank = (int(*)(EMPI_Comm,int *))dlsym(extLib,"MPI_Comm_rank");
	EMPI_Send = (int(*)(void *,int,EMPI_Datatype,int,int,EMPI_Comm))dlsym(extLib,"MPI_Send");
	EMPI_Recv = (int(*)(void *,int,EMPI_Datatype,int,int,EMPI_Comm,EMPI_Status *))dlsym(extLib,"MPI_Recv");
	EMPI_Comm_free = (int(*)(EMPI_Comm *))dlsym(extLib,"MPI_Comm_free");
	EMPI_Comm_dup = (int(*)(EMPI_Comm,EMPI_Comm *))dlsym(extLib,"MPI_Comm_dup");
	EMPI_Comm_spawn = (int(*)(const char *,char **,int,EMPI_Info,int,EMPI_Comm,int *))dlsym(extLib,"MPI_Comm_spawn");
	EMPI_Barrier = (int(*)(EMPI_Comm))dlsym(extLib,"MPI_Barrier");
	EMPI_Abort = (int(*)(EMPI_Comm,int))dlsym(extLib,"MPI_Abort");
	EMPI_Comm_group = (int(*)(EMPI_Comm,EMPI_Group *))dlsym(extLib,"MPI_Comm_group");
	EMPI_Group_size = (int(*)(EMPI_Group,int *))dlsym(extLib,"MPI_Group_size");
	EMPI_Group_incl = (int(*)(EMPI_Group,int,int *,EMPI_Group *))dlsym(extLib,"MPI_Group_incl");
	EMPI_Group_union = (int(*)(EMPI_Group,EMPI_Group,EMPI_Group *))dlsym(extLib,"MPI_Group_union");
	EMPI_Group_intersection = (int(*)(EMPI_Group,EMPI_Group,EMPI_Group *))dlsym(extLib,"MPI_Group_intersection");
	EMPI_Group_difference = (int(*)(EMPI_Group,EMPI_Group,EMPI_Group *))dlsym(extLib,"MPI_Group_difference");
	EMPI_Group_translate_ranks = (int(*)(EMPI_Group,int,int *,EMPI_Group,int *))dlsym(extLib,"MPI_Group_translate_ranks");
	EMPI_Comm_create_group = (int(*)(EMPI_Comm,EMPI_Group,int,EMPI_Comm *))dlsym(extLib,"MPI_Comm_create_group");
	EMPI_Comm_split = (int(*)(EMPI_Comm,int,int,EMPI_Comm *))dlsym(extLib,"MPI_Comm_split");
	EMPI_Intercomm_create = (int(*)(EMPI_Comm,int,EMPI_Comm,int,int,EMPI_Comm *))dlsym(extLib,"MPI_Intercomm_create");
	EMPI_Comm_set_name = (int(*)(EMPI_Comm,const char *))dlsym(extLib,"MPI_Comm_set_name");
	EMPI_Bcast = (int(*)(void *,int,EMPI_Datatype,int,EMPI_Comm))dlsym(extLib,"MPI_Bcast");
	EMPI_Allgather = (int(*)(void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm))dlsym(extLib,"MPI_Allgather");
	EMPI_Alltoall = (int(*)(void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm))dlsym(extLib,"MPI_Alltoall");
	EMPI_Allgatherv = (int(*)(void *,int,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm))dlsym(extLib,"MPI_Allgatherv");
	EMPI_Alltoallv = (int(*)(void *,int *,int *,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm))dlsym(extLib,"MPI_Alltoallv");
	EMPI_Allreduce = (int(*)(void *,void *,int,EMPI_Datatype,EMPI_Op,EMPI_Comm))dlsym(extLib,"MPI_Allreduce");
	EMPI_Isend = (int(*)(void *,int,EMPI_Datatype,int,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Isend");
	EMPI_Irecv = (int(*)(void *,int,EMPI_Datatype,int,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Irecv");
	EMPI_Ibcast = (int(*)(void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Ibcast");
	EMPI_Iscatter = (int(*)(const void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Iscatter");
	EMPI_Iscatterv = (int(*)(const void *,int *,int *,EMPI_Datatype,void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Iscatterv");
	EMPI_Igather = (int(*)(const void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Igather");
	EMPI_Igatherv = (int(*)(const void *,int,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Igatherv");
	EMPI_Ireduce = (int(*)(void *,void *,int,EMPI_Datatype,EMPI_Op,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Ireduce");
	EMPI_Iallgather = (int(*)(const void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Iallgather");
	EMPI_Iallgatherv = (int(*)(const void *,int *,int *,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Iallgatherv");
	EMPI_Ialltoall = (int(*)(void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Ialltoall");
	EMPI_Ialltoallv = (int(*)(void *,int *,int *,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Ialltoallv");
	EMPI_Iallreduce = (int(*)(void *,void *,int,EMPI_Datatype,EMPI_Op,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Iallreduce");
	EMPI_Ibarrier = (int(*)(EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Ibarrier");
	EMPI_Test = (int(*)(EMPI_Request *,int *,EMPI_Status *))dlsym(extLib,"MPI_Test");
	EMPI_Cancel = (int(*)(EMPI_Request *))dlsym(extLib,"MPI_Cancel");
	EMPI_Probe = (int(*)(int,int,EMPI_Comm,EMPI_Status *))dlsym(extLib,"MPI_Probe");
	EMPI_Iprobe = (int(*)(int,int,EMPI_Comm,int *,EMPI_Status *))dlsym(extLib,"MPI_Iprobe");
	EMPI_Request_free = (int(*)(EMPI_Request *))dlsym(extLib,"MPI_Request_free");
	EMPI_Type_size = (int(*)(EMPI_Datatype,int *))dlsym(extLib,"MPI_Type_size");
	EMPI_Get_count = (int(*)(EMPI_Status *,EMPI_Datatype,int *))dlsym(extLib,"MPI_Get_count");
	EMPI_Wtime = (double(*)())dlsym(extLib,"MPI_Wtime");
	
	/*_ext_free = (void(*)(void *))dlsym(extLib,"free");
	_ext_malloc = (void *(*)(size_t))dlsym(extLib,"malloc");
	_ext_calloc = (void *(*)(size_t,size_t))dlsym(extLib,"calloc");
	_ext_realloc = (void *(*)(void *,size_t))dlsym(extLib,"realloc");
	_ext_valloc = (void *(*)(size_t))dlsym(extLib,"valloc");
	_ext_memalign = (void *(*)(size_t,size_t))dlsym(extLib,"memalign");
	_ext_mallinfo = (struct mallinfo(*)(void))dlsym(extLib,"mallinfo");

	struct mallinfo parep_mpi_mallinfo = mallinfo();*/
}

int empi_comm_creation(int *rank, int *size, int *argc, char ***argv, bool poll_thread_active) {
	int i, retVal, worldRank, worldSize, color, cmpLeader, repLeader;
	int destRank = -1, srcRank = -1;
	retVal = EMPI_Init (argc, argv);
	if(parep_mpi_rank == parep_mpi_leader_rank) printf("EMPI INIT DONE\n");
	EMPI_Comm_rank(EMPI_COMM_WORLD,&parep_mpi_original_rank);
	parep_mpi_pid = getpid();
	
	if(parep_mpi_rank == parep_mpi_leader_rank) {
		parep_infiniband_cmd(PAREP_IB_EMPI_INITIALIZED);
	}
	
	if(poll_thread_active) {
		pthread_mutex_lock(&parep_mpi_empi_initialized_mutex);
		while(!parep_mpi_empi_initialized) {
			pthread_cond_wait(&parep_mpi_empi_initialized_cond,&parep_mpi_empi_initialized_mutex);
		}
		pthread_mutex_unlock(&parep_mpi_empi_initialized_mutex);
	} else {
		struct pollfd pfd;
		pfd.fd = parep_mpi_coordinator_socket;
		pfd.events = POLLIN;
		nfds_t nfds = 1;
		int pollret = -1;
		do {
			pollret = poll(&pfd,nfds,-1);
		} while((pollret == -1) && (errno == EINTR));
		assert(pollret > 0);
		if (pfd.revents != 0) {
			if(pfd.revents & POLLIN) {
				int out;
				int msgsize = 0;
				size_t bytes_read;
				while((bytes_read = read(pfd.fd,(&out)+msgsize, sizeof(int)-msgsize)) > 0) {
					msgsize += bytes_read;
					if(msgsize >= (sizeof(int))) break;
				}
				assert(out == CMD_MPI_INITIALIZED);
				pthread_mutex_lock(&parep_mpi_empi_initialized_mutex);
				parep_mpi_empi_initialized = true;
				pthread_cond_signal(&parep_mpi_empi_initialized_cond);
				pthread_mutex_unlock(&parep_mpi_empi_initialized_mutex);
			}
		}
	}
	
	EMPI_Comm_split(EMPI_COMM_WORLD,0,parep_mpi_rank,&parep_mpi_original_comm);
	EMPI_Comm_dup(parep_mpi_original_comm,&(MPI_COMM_WORLD->eworldComm));
	EMPI_Comm_group(MPI_COMM_WORLD->eworldComm,&parep_mpi_original_group);
	
	EMPI_Comm_rank (MPI_COMM_WORLD->eworldComm, &worldRank);
	EMPI_Comm_size (MPI_COMM_WORLD->eworldComm, &worldSize);

	if((getenv("CMP_RATIO") != NULL)  &&(getenv("REP_RATIO") != NULL)) {
		int cr = atoi(getenv("CMP_RATIO"));
		int rr = atoi(getenv("REP_RATIO"));
		nR = (worldSize*rr)/(rr+cr);
	}
	else if(getenv("COMP_PER_REP") != NULL) {
		int cpr = atoi(getenv("COMP_PER_REP"));
		if(cpr > 0) nR = worldSize/(1+cpr);
		else nR = 0;
	}
	else {
		if(CMP_PER_REP > 0) nR = worldSize/(1+CMP_PER_REP);
		else nR = 0;
	}
	//nR = worldSize - worldSize / REP_DEGREE;
	nC = worldSize - nR;
	
	cmpToRepMap = (int *) libc_malloc (nC * sizeof (int));
	repToCmpMap = (int *) libc_malloc (nR * sizeof (int));

	for (i=0; i<nC; i++) {
		if (i < nR) {
			cmpToRepMap[i] = i;
			repToCmpMap[i] = i;
		} else cmpToRepMap[i] = -1;
	}
	MPI_COMM_WORLD->EMPI_COMM_CMP = MPI_COMM_WORLD->EMPI_COMM_REP = MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM = MPI_COMM_WORLD->EMPI_CMP_NO_REP = MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM = EMPI_COMM_NULL;
	if (nR > 0) {
		color = worldRank < nC ? 0 : 1;
		cmpLeader = 0;
		repLeader = nC;

		printf("At EMPI nC %d nR %d color %d\n",nC,nR,color);
		if (color == 0) {
			EMPI_Comm_split (MPI_COMM_WORLD->eworldComm, color, 0, &(MPI_COMM_WORLD->EMPI_COMM_CMP));
			EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_CMP, 0, MPI_COMM_WORLD->eworldComm, repLeader, 100, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
			int colorinternal;
			if(cmpToRepMap[worldRank] != -1) colorinternal = EMPI_UNDEFINED;
			else colorinternal = 1;
			EMPI_Comm_split (MPI_COMM_WORLD->EMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
			if(MPI_COMM_WORLD->EMPI_CMP_NO_REP != EMPI_COMM_NULL) EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->eworldComm, repLeader, 101, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
			MPI_COMM_WORLD->EMPI_COMM_REP = EMPI_COMM_NULL;
		}

		else if (color == 1) {
			EMPI_Comm_split (MPI_COMM_WORLD->eworldComm, color, 0, &(MPI_COMM_WORLD->EMPI_COMM_REP));
			EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, cmpLeader, 100, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
			if(nC > nR) EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, nR, 101, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
			MPI_COMM_WORLD->EMPI_COMM_CMP = EMPI_COMM_NULL;
		}
		if (MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL && cmpToRepMap[worldRank] != -1) destRank = cmpToRepMap[worldRank];
		else if (MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL && repToCmpMap[worldRank - nC] != -1) srcRank = repToCmpMap[worldRank - nC];
	} else {
		EMPI_Comm_dup(MPI_COMM_WORLD->eworldComm,&(MPI_COMM_WORLD->EMPI_COMM_CMP));
		EMPI_Comm_dup(MPI_COMM_WORLD->eworldComm,&(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
	}

	int myrank,mysize;
	EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
	EMPI_Comm_size(MPI_COMM_WORLD->eworldComm,&mysize);
	
	parep_mpi_size = mysize;
	
	*rank = myrank;
	*size = mysize;

	EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
	
	internalranks = (int *)libc_malloc(nC*sizeof(int));
	
	for(int i=0;i<nC;i++) {
		internalranks[i] = i;
	}
	return retVal;
}

void initialize_common_heap_and_stack(int myrank) {
	parep_mpi_ext_heap_size = PAREP_MPI_HEAP_SIZE;
	parep_mpi_ext_heap_mapping = (void *)(0x300000000000);
	int andresp = 0;
	int resp = 0;
	do {
		void *base = parep_mpi_ext_heap_mapping;
		if(myrank == parep_mpi_leader_rank) {
			do {
				base = parep_mpi_ext_heap_mapping;
				parep_mpi_ext_heap_mapping = mmap(parep_mpi_ext_heap_mapping,parep_mpi_ext_heap_size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS,-1,0);
				if(parep_mpi_ext_heap_mapping == MAP_FAILED) parep_mpi_ext_heap_mapping = (void *)(((address)base) + ((address)0x10000000000));
			} while(parep_mpi_ext_heap_mapping == MAP_FAILED);
			EMPI_Bcast(&parep_mpi_ext_heap_mapping,sizeof(address),EMPI_BYTE,parep_mpi_leader_rank,MPI_COMM_WORLD->eworldComm);
			resp = 1;
			EMPI_Allreduce(&resp,&andresp,1,EMPI_INT,EMPI_BAND,MPI_COMM_WORLD->eworldComm);
		} else {
			EMPI_Bcast(&parep_mpi_ext_heap_mapping,sizeof(address),EMPI_BYTE,parep_mpi_leader_rank,MPI_COMM_WORLD->eworldComm);
			parep_mpi_ext_heap_mapping = mmap(parep_mpi_ext_heap_mapping,parep_mpi_ext_heap_size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS,-1,0);
			resp = (parep_mpi_ext_heap_mapping != MAP_FAILED);
			EMPI_Allreduce(&resp,&andresp,1,EMPI_INT,EMPI_BAND,MPI_COMM_WORLD->eworldComm);
		}
		if(andresp == 0) {
			if(resp) {
				munmap(parep_mpi_ext_heap_mapping,parep_mpi_ext_heap_size);
			}
			parep_mpi_ext_heap_mapping = (void *)(((address)base) + ((address)0x10000000000));
		}
	} while(andresp == 0);
	
	parep_mpi_init_heap((address)parep_mpi_ext_heap_mapping);
	
	for(int i = 0; i < COMMBUF_BIN_COUNT; i++) {
		parep_mpi_commbuf_bins[i].head = NULL;
		parep_mpi_commbuf_bins[i].tail = NULL;
	}
	
	parep_mpi_new_stack_size = 256*1024;
	parep_mpi_new_stack = (void *)((address)parep_mpi_ext_heap_mapping + (address)(0x100000000000));
	andresp = 0;
	resp = 0;
	do {
		void *base = parep_mpi_new_stack;
		if(myrank == parep_mpi_leader_rank) {
			do {
				base = parep_mpi_new_stack;
				parep_mpi_new_stack = mmap(parep_mpi_new_stack,parep_mpi_new_stack_size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS,-1,0);
				if(parep_mpi_new_stack == MAP_FAILED) parep_mpi_new_stack = (void *)(((address)base) + ((address)0x10000000000));
			} while(parep_mpi_new_stack == MAP_FAILED);
			EMPI_Bcast(&parep_mpi_new_stack,sizeof(address),EMPI_BYTE,parep_mpi_leader_rank,MPI_COMM_WORLD->eworldComm);
			resp = 1;
			EMPI_Allreduce(&resp,&andresp,1,EMPI_INT,EMPI_BAND,MPI_COMM_WORLD->eworldComm);
		} else {
			EMPI_Bcast(&parep_mpi_new_stack,sizeof(address),EMPI_BYTE,parep_mpi_leader_rank,MPI_COMM_WORLD->eworldComm);
			parep_mpi_new_stack = mmap(parep_mpi_new_stack,parep_mpi_ext_heap_size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS,-1,0);
			resp = (parep_mpi_new_stack != MAP_FAILED);
			EMPI_Allreduce(&resp,&andresp,1,EMPI_INT,EMPI_BAND,MPI_COMM_WORLD->eworldComm);
		}
		if(andresp == 0) {
			if(resp) {
				munmap(parep_mpi_new_stack,parep_mpi_ext_heap_size);
			}
			parep_mpi_new_stack = (void *)((address)base + (address)(0x10000000000));
		}
	} while(andresp == 0);
	
	parep_mpi_new_stack_start = (address)parep_mpi_new_stack;
	parep_mpi_new_stack_end = parep_mpi_new_stack_start + parep_mpi_new_stack_size;
	parep_mpi_new_stack = (void *)(parep_mpi_new_stack_start + parep_mpi_new_stack_size - 256 - 8);
	
	EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
}

void checkpoint_and_restart() {
	bool int_call;
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	parep_mpi_manual_restart = 1;
	parep_mpi_ckpt_wait = 1;
	pthread_kill(pthread_self(),SIGUSR1);
	while(parep_mpi_ckpt_wait) {;}

	if(parep_mpi_manual_restart) {
		parep_mpi_manual_restart = 0;
		int newrank;
		pthread_mutex_lock(&reqListLock);
		pthread_rwlock_rdlock(&commLock);
		EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&newrank);
		EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
		pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
		if(newrank == parep_mpi_leader_rank) {
			parep_infiniband_cmd(PAREP_IB_KILL_COORDINATOR);
		}
		pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
		dlclose(extLib);
		while(1);
		pthread_rwlock_unlock(&commLock);
		pthread_mutex_unlock(&reqListLock);
	}

	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
}

void transfer_to_replicas(int *argc, char ***argv) {
	bool int_call;
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	pthread_mutex_lock(&reqListLock);
	pthread_rwlock_wrlock(&commLock);
	
	EMPI_Group current_alive_group;
	EMPI_Comm parep_mpi_new_comm,parep_mpi_old_comm;
	memcpy(&parep_mpi_old_comm,&(MPI_COMM_WORLD->eworldComm),sizeof(EMPI_Comm));
	EMPI_Comm_group(MPI_COMM_WORLD->eworldComm,&current_alive_group);
	int ret = EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm,current_alive_group,1234,&parep_mpi_new_comm);
	memcpy(&(MPI_COMM_WORLD->eworldComm),&parep_mpi_new_comm,sizeof(EMPI_Comm));
	EMPI_Comm_free(&parep_mpi_old_comm);
	
	int newrank;
	EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&newrank);
	if (nR > 0) {
		int color = (newrank < nC) ? 0 : 1;
		int cmpLeader = 0;
		int repLeader = nC;

		if (color == 0) {
			EMPI_Comm_split(MPI_COMM_WORLD->eworldComm, color, 0, &(MPI_COMM_WORLD->EMPI_COMM_CMP));
			EMPI_Intercomm_create(MPI_COMM_WORLD->EMPI_COMM_CMP, 0, MPI_COMM_WORLD->eworldComm, repLeader, 100, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
			int colorinternal;
			if(cmpToRepMap[newrank] != -1) colorinternal = EMPI_UNDEFINED;
			else colorinternal = 1;
			EMPI_Comm_split (MPI_COMM_WORLD->EMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
			if(MPI_COMM_WORLD->EMPI_CMP_NO_REP != EMPI_COMM_NULL) EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->eworldComm, repLeader, 101, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
			MPI_COMM_WORLD->EMPI_COMM_REP = EMPI_COMM_NULL;
		} else if (color == 1) {
			EMPI_Comm_split (MPI_COMM_WORLD->eworldComm, color, 0, &(MPI_COMM_WORLD->EMPI_COMM_REP));
			EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, cmpLeader, 100, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
			if(nC > nR) {
				int rleader = nR;
				for(int j = 0; j < nC; j++) {
						if(cmpToRepMap[j] == -1) {
						rleader = j;
						break;
					}
				}
				EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, rleader, 101, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
			}
			MPI_COMM_WORLD->EMPI_COMM_CMP = EMPI_COMM_NULL;
		}
	} else {
		EMPI_Comm_dup(MPI_COMM_WORLD->eworldComm,&(MPI_COMM_WORLD->EMPI_COMM_CMP));
		EMPI_Comm_dup(MPI_COMM_WORLD->eworldComm,&(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
	}
	
	parseStatFile();
	parseMapsFile();
	
	int myrank;
	EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
	
	if (nR > 0) {
		if(myrank < nC) {
			if(cmpToRepMap[myrank] != -1) {
				parep_mpi_validator_pipe_check = 1;
				unw_cursor_t cursor; unw_context_t uc;
				unw_word_t ip,ax,bx,cx,dx,di,si,r8,r9,r10,r11,r12,r13,r14,r15;
				unw_getcontext(&uc);
				unw_init_local(&cursor, &uc);
				parep_mpi_num_reg_vals = 0;
				while (unw_step(&cursor) > 0) {
					unw_get_reg(&cursor, UNW_REG_IP, &ip);
					unw_get_reg(&cursor, UNW_X86_64_RAX, &ax);
					unw_get_reg(&cursor, UNW_X86_64_RBX, &bx);
					unw_get_reg(&cursor, UNW_X86_64_RCX, &cx);
					unw_get_reg(&cursor, UNW_X86_64_RDX, &dx);
					unw_get_reg(&cursor, UNW_X86_64_RDI, &di);
					unw_get_reg(&cursor, UNW_X86_64_RSI, &si);
					unw_get_reg(&cursor, UNW_X86_64_R8, &r8);
					unw_get_reg(&cursor, UNW_X86_64_R9, &r9);
					unw_get_reg(&cursor, UNW_X86_64_R10, &r10);
					unw_get_reg(&cursor, UNW_X86_64_R11, &r11);
					unw_get_reg(&cursor, UNW_X86_64_R12, &r12);
					unw_get_reg(&cursor, UNW_X86_64_R13, &r13);
					unw_get_reg(&cursor, UNW_X86_64_R14, &r14);
					unw_get_reg(&cursor, UNW_X86_64_R15, &r15);
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][0] = (address)ip;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][1] = (address)ax;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][2] = (address)bx;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][3] = (address)cx;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][4] = (address)dx;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][5] = (address)di;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][6] = (address)si;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][7] = (address)r8;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][8] = (address)r9;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][9] = (address)r10;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][10] = (address)r11;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][11] = (address)r12;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][12] = (address)r13;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][13] = (address)r14;
					parep_mpi_reg_vals[parep_mpi_num_reg_vals][14] = (address)r15;
					parep_mpi_num_reg_vals++;
				}
				parep_mpi_validator_pipe_check = 0;
				int setjmp_status = setjmp(parep_mpi_replication_initializer);
				if(setjmp_status == 0) {
					jmp_buf copy_context;
					copy_jmp_buf(parep_mpi_replication_initializer, copy_context);
					newStack = mmap(NULL,TEMP_STACK_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
					address cur_pc = getPC(copy_context);
					// Stack starts from higher address.
					setRSP(copy_context, ((address)(newStack)) + TEMP_STACK_SIZE - TEMP_STACK_BOTTOM_OFFSET);
					// Start execution on new temp stack.
					parep_mpi_longjmp(copy_context, 1);
				} else if(setjmp_status == 1) {
					int dest_rank = cmpToRepMap[myrank];
					//Send Data Heap and Stack
					parep_mpi_send_replication_data(dest_rank);
					parep_mpi_longjmp(parep_mpi_replication_initializer,2);
				} else if(setjmp_status == 2) {
					if(argv != NULL) *argv = parep_mpi_argv;
					printf("%d: Longjmped to CMP SIDE rank %d myrank %d\n",getpid(),parep_mpi_rank,myrank);
					munmap(newStack,TEMP_STACK_SIZE);
				}
			}
		} else {
			unw_cursor_t cursor; unw_context_t uc;
			unw_word_t ip;
			int setjmp_status = setjmp(parep_mpi_replication_initializer);
			if(setjmp_status == 0) {
				jmp_buf copy_context;
				copy_jmp_buf(parep_mpi_replication_initializer, copy_context);
				newStack = mmap(NULL,TEMP_STACK_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
				address cur_pc = getPC(copy_context);
				// Stack starts from higher address.
				setRSP(copy_context, ((address)(newStack)) + TEMP_STACK_SIZE - TEMP_STACK_BOTTOM_OFFSET);
				// Start execution on new temp stack.
				parep_mpi_longjmp(copy_context, 1);
			} else if(setjmp_status == 1) {
				int src_rank = repToCmpMap[myrank-nC];
				//Recv Data Heap and Stack
				parep_mpi_recv_replication_data(src_rank);
				parep_mpi_longjmp(parep_mpi_replication_initializer,2);
			} else if(setjmp_status == 2) {
				if(argv != NULL) *argv = parep_mpi_argv;
				printf("%d: Longjmped to REP SIDE rank %d myrank %d\n",getpid(),parep_mpi_rank,myrank);
				munmap(newStack,TEMP_STACK_SIZE);
			}
			parep_mpi_validator_pipe_check = 1;
			unw_getcontext(&uc);
			unw_init_local(&cursor, &uc);
			int step = 0;
			while (unw_step(&cursor) > 0) {
				unw_set_reg(&cursor, UNW_REG_IP, parep_mpi_reg_vals[step][0]);
				unw_set_reg(&cursor, UNW_X86_64_RAX, parep_mpi_reg_vals[step][1]);
				unw_set_reg(&cursor, UNW_X86_64_RBX, parep_mpi_reg_vals[step][2]);
				unw_set_reg(&cursor, UNW_X86_64_RCX, parep_mpi_reg_vals[step][3]);
				unw_set_reg(&cursor, UNW_X86_64_RDX, parep_mpi_reg_vals[step][4]);
				unw_set_reg(&cursor, UNW_X86_64_RDI, parep_mpi_reg_vals[step][5]);
				unw_set_reg(&cursor, UNW_X86_64_RSI, parep_mpi_reg_vals[step][6]);
				unw_set_reg(&cursor, UNW_X86_64_R8, parep_mpi_reg_vals[step][7]);
				unw_set_reg(&cursor, UNW_X86_64_R9, parep_mpi_reg_vals[step][8]);
				unw_set_reg(&cursor, UNW_X86_64_R10, parep_mpi_reg_vals[step][9]);
				unw_set_reg(&cursor, UNW_X86_64_R11, parep_mpi_reg_vals[step][10]);
				unw_set_reg(&cursor, UNW_X86_64_R12, parep_mpi_reg_vals[step][11]);
				unw_set_reg(&cursor, UNW_X86_64_R13, parep_mpi_reg_vals[step][12]);
				unw_set_reg(&cursor, UNW_X86_64_R14, parep_mpi_reg_vals[step][13]);
				unw_set_reg(&cursor, UNW_X86_64_R15, parep_mpi_reg_vals[step][14]);
				step++;
			}
			parep_mpi_validator_pipe_check = 0;
		}
		
		pthread_mutex_lock(&peertopeerLock);
		pthread_mutex_lock(&collectiveLock);
		
		ptpdata *pdata = first_peertopeer;
		while(pdata != NULL) {
			if(pdata->type == MPI_FT_RECV) {
				if(pdata->completecmp && pdata->completerep && (pdata->req == NULL)) {
					pdata->markdelrep = pdata->markdelcmp;
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
			//New code Take Care!!!
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				if((cdata->type == MPI_FT_REDUCE) || (cdata->type == MPI_FT_ALLREDUCE)) {
					cdata->completerep = cdata->completecmp;
					cdata->completecmp = true;
				}
			} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				if((cdata->type == MPI_FT_REDUCE) || (cdata->type == MPI_FT_ALLREDUCE)) {
					if((!cdata->completecmp) && (cdata->completerep)) {
						int myrank;
						EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_CMP,&myrank);
						if(cdata->type == MPI_FT_REDUCE) {
							if((cmpToRepMap[myrank] != -1) && ((cdata->args).reduce.root == myrank)) cdata->completerep = false;
						} else if(cdata->type == MPI_FT_ALLREDUCE) {
							if(cmpToRepMap[myrank] != -1) cdata->completerep = false;
						}
					}
				}
			}
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
				} else if(deldata->type == MPI_FT_GATHER) {
					parep_mpi_free((deldata->args).gather.sendbuf);
				} else if(deldata->type == MPI_FT_REDUCE) {
					parep_mpi_free((deldata->args).reduce.sendbuf);
					if((deldata->args).reduce.alloc_recvbuf) parep_mpi_free((deldata->args).reduce.recvbuf);
				} else if(deldata->type == MPI_FT_ALLGATHER) {
					parep_mpi_free((deldata->args).allgather.sendbuf);
				} else if(deldata->type == MPI_FT_ALLTOALL) {
					parep_mpi_free((deldata->args).alltoall.sendbuf);
				} else if(deldata->type == MPI_FT_ALLTOALLV) {
					parep_mpi_free((deldata->args).alltoallv.sendbuf);
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
		
		handle_rem_recv_no_poll();
		
		EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
		
		pdata = first_peertopeer;
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
		
		cdata = last_collective;
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
				} else if(deldata->type == MPI_FT_GATHER) {
					parep_mpi_free((deldata->args).gather.sendbuf);
				} else if(deldata->type == MPI_FT_REDUCE) {
					parep_mpi_free((deldata->args).reduce.sendbuf);
					if((deldata->args).reduce.alloc_recvbuf) parep_mpi_free((deldata->args).reduce.recvbuf);
				} else if(deldata->type == MPI_FT_ALLGATHER) {
					parep_mpi_free((deldata->args).allgather.sendbuf);
				} else if(deldata->type == MPI_FT_ALLTOALL) {
					parep_mpi_free((deldata->args).alltoall.sendbuf);
				} else if(deldata->type == MPI_FT_ALLTOALLV) {
					parep_mpi_free((deldata->args).alltoallv.sendbuf);
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
		parep_mpi_restore_messages();
		pthread_mutex_unlock(&collectiveLock);
		pthread_mutex_unlock(&peertopeerLock);
	}
	EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
	pthread_rwlock_unlock(&commLock);
	pthread_mutex_unlock(&reqListLock);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
}

int MPI_Replica_rearrange(int *ctrmap, int *rtcmap) {
	clock_gettime(CLOCK_REALTIME,&mpi_ft_replica_rearrange_start_time);
	for (int i=0; i<nC; i++) {
		cmpToRepMap[i] = ctrmap[i];
		if (i < nR) repToCmpMap[i] = rtcmap[i];
	}
	transfer_to_replicas(NULL,NULL);
	clock_gettime(CLOCK_REALTIME,&mpi_ft_replica_rearrange_end_time);
	printf("%d: Replica Rearrange time = %f\n",getpid(),(timespec_to_double(mpi_ft_replica_rearrange_end_time)-timespec_to_double(mpi_ft_replica_rearrange_start_time)));
	return MPI_SUCCESS;
}

/* MPI_Init */
int MPI_Init (int *argc, char ***argv) {
	parep_mpi_argc = *argc;
	parep_mpi_argv = *argv;
	if(parep_mpi_initialized) {
		parep_mpi_internal_call = true;
		//parep_mpi_ckpt_wait = 1;
		
		parep_mpi_validator_pipe_check = 1;
		unw_set_caching_policy(unw_local_addr_space,UNW_CACHE_NONE);
		parep_mpi_validator_pipe_check = 0;
		
		//mpi_ft_start_time = EMPI_Wtime();

		/*pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}*/
		transfer_to_replicas(argc,argv);
		int myrank;
		EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
		if(getenv("PAREP_MPI_MANUAL_CKPT_START") != NULL) {
			if(strcmp(getenv("PAREP_MPI_MANUAL_CKPT_START"),"1") != 0) {
				pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
				if(myrank == parep_mpi_leader_rank) parep_infiniband_cmd(PAREP_IB_CKPT_CREATED);
				pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
			}
		} else {
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
			pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
			if(myrank == parep_mpi_leader_rank) parep_infiniband_cmd(PAREP_IB_CKPT_CREATED);
			pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
		}
		//pthread_mutex_lock(&parep_mpi_leader_rank_mutex);
		//if(myrank == parep_mpi_leader_rank) parep_infiniband_cmd(PAREP_IB_CKPT_CREATED);
		//pthread_mutex_unlock(&parep_mpi_leader_rank_mutex);
		parep_mpi_internal_call = false;
		return MPI_SUCCESS;
	}
	void* spointer,*fpointer;
	asm volatile ("mov %%rsp, %0" : "=r"(spointer));
	asm volatile ("mov %%rbp, %0" : "=r"(fpointer));
	parep_mpi_target_sp = *((address *)spointer);
	parep_mpi_target_bp = *((address *)fpointer);
	setenv("PAREP_MPI_PROXY_HACKED","0",1);
	
	if(_real_free == NULL) _real_free = dlsym(RTLD_NEXT,"free");
	if(_real_malloc == NULL) _real_malloc = dlsym(RTLD_NEXT,"malloc");
	
	num_threads = 0;
	
	int tid_index;
	pthread_mutex_lock(&num_thread_mutex);
	tid_index = num_threads;
	num_threads++;
	pthread_mutex_unlock(&num_thread_mutex);
	thread_tid[tid_index] = pthread_self();
	set_signal_handler(SIGUSR1);
	parep_mpi_restore = 1;
	parep_mpi_restore_wait = 1;
	
	num_failures = 0;
	
	char host_name[HOST_NAME_MAX+1];
	char *IP_address;
	int thname = gethostname(host_name,sizeof(host_name));
	if(thname == -1) {
		perror("gethostname");
		exit(1);
	}

	struct hostent *entry = gethostbyname(host_name);
	if(entry == NULL) {
		perror("gethostbyname");
		exit(1);
	}

	IP_address = inet_ntoa(*((struct in_addr *)entry->h_addr_list[0]));
	
	parep_mpi_coordinator_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if(parep_mpi_coordinator_socket == -1) {
		perror("parep_mpi_coordinator_socket");
		exit(1);
	}
	
	if(parep_mpi_coordinator_socket != PAREP_MPI_COORDINATOR_SOCK_NUM) {
		dup2(parep_mpi_coordinator_socket,PAREP_MPI_COORDINATOR_SOCK_NUM);
		close(parep_mpi_coordinator_socket);
		parep_mpi_coordinator_socket = PAREP_MPI_COORDINATOR_SOCK_NUM;
	}
	
	/*parep_mpi_coordinator_addr.sin_family = AF_INET;
	parep_mpi_coordinator_addr.sin_port = htons(COORDINATOR_DPORT);
	parep_mpi_coordinator_addr.sin_addr.s_addr = inet_addr(IP_address);*/
	memset(&parep_mpi_coordinator_addr,0,sizeof(parep_mpi_coordinator_addr));
	parep_mpi_coordinator_addr.sun_family = AF_UNIX;
	strcpy(parep_mpi_coordinator_addr.sun_path,"#LocalServer");
	parep_mpi_coordinator_addr.sun_path[0] = 0;
	
	parep_mpi_dyn_coordinator_addr.sin_family = AF_INET;
	parep_mpi_dyn_coordinator_addr.sin_port = htons(DYN_COORDINATOR_PORT);
	parep_mpi_dyn_coordinator_addr.sin_addr.s_addr = inet_addr(IP_address);
	
	int ret = connect(parep_mpi_coordinator_socket,(struct sockaddr *)(&parep_mpi_coordinator_addr),sizeof(parep_mpi_coordinator_addr));
	while(ret != 0) {
		ret = connect(parep_mpi_coordinator_socket,(struct sockaddr *)(&parep_mpi_coordinator_addr),sizeof(parep_mpi_coordinator_addr));
	}
	
	pid_t mypid = getpid();
	write(parep_mpi_coordinator_socket,&mypid,sizeof(pid_t));
	
	size_t bytes_read;
	int msgsize = 0;
	char buffer[BUFFER_SIZE];
	while((bytes_read = read(parep_mpi_coordinator_socket,buffer+msgsize, ((5*sizeof(int)) + sizeof(pid_t)) - msgsize)) > 0) {
		msgsize += bytes_read;
		if(msgsize >= ((5*sizeof(int)) + sizeof(pid_t))) break;
	}
	
	parep_mpi_rank = *((int *)buffer);
	parep_mpi_node_rank = *((int *)(buffer + sizeof(int)));
	parep_mpi_node_id =	*((int *)(buffer + (2*sizeof(int))));
	parep_mpi_node_num = *((int *)(buffer + (3*sizeof(int))));
	parep_mpi_node_size = *((int *)(buffer + (4*sizeof(int))));
	parep_mpi_coordinator_pid = *((pid_t *)(buffer + (5*sizeof(int))));
	
	struct msghdr mhdr;
	struct iovec iov;
	union {
		struct cmsghdr cmhdr;
		char cntrl[CMSG_SPACE(sizeof(int))];
	} cntrl_unix;
	struct cmsghdr *cmhdr;

	char tmp;
	iov.iov_base = &tmp;
	iov.iov_len = sizeof(char);

	mhdr.msg_name = NULL;
	mhdr.msg_namelen = 0;
	mhdr.msg_iov = &iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = cntrl_unix.cntrl;
	mhdr.msg_controllen = sizeof(cntrl_unix.cntrl);
	
	int p_stdout,p_stderr,p_stdin;

	int size = recvmsg(parep_mpi_coordinator_socket,&mhdr,0);
	if(size == -1) {
		printf("Recvmsg failed\n");
		return -1;
	}
	cmhdr = CMSG_FIRSTHDR(&mhdr);
	if(!cmhdr) {
		printf("Expected one Recvmsg header with FD, Got zero Headers!\n");
		return -1;
	}
	if(cmhdr->cmsg_level != SOL_SOCKET) {
		printf("Invalid cmsg_level %d\n",cmhdr->cmsg_level);
		return -1;
	}
	if(cmhdr->cmsg_type != SCM_RIGHTS) {
		printf("Invalid cmsg_type %d\n",cmhdr->cmsg_type);
		return -1;
	}
	p_stdout = *((int *) CMSG_DATA(cmhdr));

	size = recvmsg(parep_mpi_coordinator_socket,&mhdr,0);
	if(size == -1) {
		printf("Recvmsg failed\n");
		return -1;
	}
	cmhdr = CMSG_FIRSTHDR(&mhdr);
	if(!cmhdr) {
		printf("Expected one Recvmsg header with FD, Got zero Headers!\n");
		return -1;
	}
	if(cmhdr->cmsg_level != SOL_SOCKET) {
		printf("Invalid cmsg_level %d\n",cmhdr->cmsg_level);
		return -1;
	}
	if(cmhdr->cmsg_type != SCM_RIGHTS) {
		printf("Invalid cmsg_type %d\n",cmhdr->cmsg_type);
		return -1;
	}
	p_stderr = *((int *) CMSG_DATA(cmhdr));
	
	if(parep_mpi_rank == 0) {
		size = recvmsg(parep_mpi_coordinator_socket,&mhdr,0);
		if(size == -1) {
			printf("Recvmsg failed\n");
			return -1;
		}
		cmhdr = CMSG_FIRSTHDR(&mhdr);
		if(!cmhdr) {
			printf("Expected one Recvmsg header with FD, Got zero Headers!\n");
			return -1;
		}
		if(cmhdr->cmsg_level != SOL_SOCKET) {
			printf("Invalid cmsg_level %d\n",cmhdr->cmsg_level);
			return -1;
		}
		if(cmhdr->cmsg_type != SCM_RIGHTS) {
			printf("Invalid cmsg_type %d\n",cmhdr->cmsg_type);
			return -1;
		}
		p_stdin = *((int *) CMSG_DATA(cmhdr));
	}
	else {
		int fd;
		/* connect input to /dev/null */
		fd = open("/dev/null", O_RDONLY, 0);
		if (fd != fileno(stdin)) {
			dup2(fd, fileno(stdin));
		}
		close(fd);
	}
	
	/*struct termios term_attrs;
	if (tcgetattr(p_stdout, &term_attrs) < 0) {
		printf("pipe setup failure errno %d\n",errno);
		return -1;
	}
	term_attrs.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | ECHONL);
	term_attrs.c_iflag &= ~(ICRNL | INLCR | ISTRIP | INPCK | IXON);
	term_attrs.c_oflag &= ~(
#ifdef OCRNL
													OCRNL |
#endif
													ONLCR);
	if (tcsetattr(p_stdout, TCSANOW, &term_attrs) == -1) {
		printf("pipe setup failure 1\n");
		return -1;
	}*/
	if(p_stdout != fileno(stdout)) {
		ret = dup2(p_stdout, fileno(stdout));
		if (ret < 0) {
			printf("pipe setup failure 2\n");
			return -1;
		}
		close(p_stdout);
	}
	
	if (p_stderr != fileno(stderr)) {
		ret = dup2(p_stderr, fileno(stderr));
		if (ret < 0) {
			printf("stderr dup2 failed\n");
			return -1;
		}
		close(p_stderr);
	}
	
	if(parep_mpi_rank == 0) {
		if (p_stdin != fileno(stdin)) {
			ret = dup2(p_stdin, fileno(stdin));
			if (ret < 0) {
				printf("stdin dup2 failed\n");
				return -1;
			}
			close(p_stdin);
		}
	}
	
	parep_mpi_leader_rank = 0;
	
	pthread_create(&daemon_poller,NULL,polling_daemon,NULL);
	
	while(parep_mpi_polling_started == 0);
	
	parep_mpi_pmi_fd = (getenv("PMI_FD") == NULL) ? -1 : atoi(getenv("PMI_FD"));
	if(parep_mpi_pmi_fd != -1) {
		if(parep_mpi_pmi_fd != PAREP_MPI_PMI_FD) {
			dup2(parep_mpi_pmi_fd,PAREP_MPI_PMI_FD);
			parep_mpi_pmi_fd = PAREP_MPI_PMI_FD;
			char pmi_fd_str[64];
			sprintf(pmi_fd_str,"%d",parep_mpi_pmi_fd);
			setenv("PMI_FD",pmi_fd_str,1);
		}
	}
	parep_mpi_kvs_name[0] = '\0';
	
	//parseSockInfo();
	parep_mpi_validator_pipe_check = 1;
	unw_set_caching_policy(unw_local_addr_space,UNW_CACHE_NONE);
	parep_mpi_validator_pipe_check = 0;
	
	if(_real_fclose == NULL) _real_fclose = dlsym(RTLD_NEXT,"fclose");
	if(_real_fopen == NULL) _real_fopen = dlsym(RTLD_NEXT,"fopen");
	if(_real_fopen64 == NULL) _real_fopen64 = dlsym(RTLD_NEXT,"fopen64");
	
	pthread_kill(pthread_self(),SIGUSR1);
	while(parep_mpi_restore_wait) {;}
	
	clock_gettime(CLOCK_REALTIME,&mpi_ft_start_time);
	
	parep_mpi_ckpt_wait = 1;
	pthread_kill(pthread_self(),SIGUSR1);
	while(parep_mpi_ckpt_wait) {;}
	
	initialize_mpi_variables();
	int myrank,mysize;
	int retVal = empi_comm_creation(&myrank,&mysize,argc,argv,true);
	initialize_common_heap_and_stack(myrank);
	
	parep_mpi_initialized = true;
	
	if(setjmp(parep_mpi_stack_switcher) == 0) {
		setRSP(parep_mpi_stack_switcher,(address)parep_mpi_new_stack);
		setRBP(parep_mpi_stack_switcher,*((address *)getRBP(parep_mpi_stack_switcher)));
		setPC(parep_mpi_stack_switcher,(address)parep_mpi_main_trampoline);
		parep_mpi_internal_call = false;
		parep_mpi_longjmp(parep_mpi_stack_switcher,1);
	}
	
	return retVal;
}

/* MPI_Comm_rank */

int MPI_Comm_rank (MPI_Comm comm, int *rank) {
	bool int_call;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	parep_mpi_sighandling_state = 1;
	int myRank;
	pthread_rwlock_rdlock(&commLock);
	if (comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
		EMPI_Comm_rank (comm->EMPI_COMM_REP, &myRank);
		*rank = repToCmpMap [myRank];
		pthread_rwlock_unlock(&commLock);
		if(parep_mpi_sighandling_state == 2) {
			parep_mpi_sighandling_state = 0;
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
		}
		parep_mpi_sighandling_state = 0;
		if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
		
		return MPI_SUCCESS;
	} else {
		EMPI_Comm_rank (comm->EMPI_COMM_CMP, rank);
		pthread_rwlock_unlock(&commLock);
		if(parep_mpi_sighandling_state == 2) {
			parep_mpi_sighandling_state = 0;
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
		}
		parep_mpi_sighandling_state = 0;
		if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
		
		return MPI_SUCCESS;
	}
}

/* MPI_Comm_size */

int MPI_Comm_size (MPI_Comm comm, int *size) {
	bool int_call;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	parep_mpi_sighandling_state = 1;
	pthread_rwlock_rdlock(&commLock);
	if (comm->EMPI_COMM_REP != EMPI_COMM_NULL) {
		EMPI_Comm_size (comm->eworldComm, size);
		*size = *size - nR;
		pthread_rwlock_unlock(&commLock);
		if(parep_mpi_sighandling_state == 2) {
			parep_mpi_sighandling_state = 0;
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
		}
		parep_mpi_sighandling_state = 0;
		if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
		
		return MPI_SUCCESS;
	} else {
		EMPI_Comm_size (comm->EMPI_COMM_CMP, size);
		pthread_rwlock_unlock(&commLock);
		if(parep_mpi_sighandling_state == 2) {
			parep_mpi_sighandling_state = 0;
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
		}
		parep_mpi_sighandling_state = 0;
		if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
		
		return MPI_SUCCESS;
	}
}

void GetFirstWord(char *string, char *first) {
	int i;
	for (i = 0; i <  strlen(string); i++) {
		if (string[i] == ' ' || string[i] == '\n' || string[i] == '\0') break;
	}
	strncpy(first,string,i);
	first[i] = '\0';
}

void mpi_ft_free_older_collectives(struct collective_data *data) {
	struct collective_data *current = data;
	while(current != NULL) {
		struct collective_data *next = current->next;
		if(current->type == MPI_FT_BCAST) {
			parep_mpi_free((current->args).bcast.buf);
#ifdef HEAPTRACK
			if(getenv("MPI_FT_TRACK_HEAP"))
			{
				if(!strcmp(getenv("MPI_FT_TRACK_HEAP"),"1"))
				{
					gettimeofday(&tv,NULL);
					int size;
					EMPI_Type_size((current->args).bcast.dt->edatatype,&size);
					total_alloc -= size * (current->args).bcast.count;
					fprintf(logfile,"%d %ld\n",total_alloc,1000000 * tv.tv_sec + tv.tv_usec);
				}
			}
#endif
		}
		else if(current->type == MPI_FT_SCATTER) {
			parep_mpi_free((current->args).scatter.sendbuf);
			//free((current->args).scatter.recvbuf);
		}
		else if(current->type == MPI_FT_GATHER) {
			parep_mpi_free((current->args).gather.sendbuf);
			//free((current->args).gather.recvbuf);
		}
		else if(current->type == MPI_FT_REDUCE) {
			parep_mpi_free((current->args).reduce.sendbuf);
			//if((current->args).reduce.recvbuf != NULL) free((current->args).reduce.recvbuf);
		}
		else if(current->type == MPI_FT_ALLGATHER) {
			parep_mpi_free((current->args).allgather.sendbuf);
			//free((current->args).allgather.recvbuf);
		}
		else if(current->type == MPI_FT_ALLTOALL) {
			parep_mpi_free((current->args).alltoall.sendbuf);
			//free((current->args).alltoall.recvbuf);
		}
		else if(current->type == MPI_FT_ALLTOALLV) {
			parep_mpi_free((current->args).alltoallv.sendbuf);
			//free((current->args).alltoallv.recvbuf);
			parep_mpi_free((current->args).alltoallv.sendcounts);
			parep_mpi_free((current->args).alltoallv.recvcounts);
			parep_mpi_free((current->args).alltoallv.sdispls);
			parep_mpi_free((current->args).alltoallv.rdispls);
		}
		else if(current->type == MPI_FT_ALLREDUCE) {
			parep_mpi_free((current->args).allreduce.sendbuf);
			//if((current->args).allreduce.recvbuf != NULL) free((current->args).allreduce.recvbuf);
		}
		parep_mpi_free(current);
		current = next;
#ifdef HEAPTRACK
		if(getenv("MPI_FT_TRACK_HEAP"))
		{
			if(!strcmp(getenv("MPI_FT_TRACK_HEAP"),"1"))
			{
				gettimeofday(&tv,NULL);
				total_alloc -= sizeof(struct collective_data);
				fprintf(logfile,"%d %ld\n",total_alloc,1000000 * tv.tv_sec + tv.tv_usec);
			}
		}
#endif
	}
}

void mpi_ft_collective_from_args(struct collective_data *data, int cmpid, int repid, int myrepid, int *redids, int *reddataids, int *collids) {
	while(data != NULL) {
		bool no_req_exists = (data->req == NULL);
		int retVal;
		int myrank;
		int realrank;
		int commnC;
		int commnR;
		void *recv_dest = NULL;
		void *send_src = NULL;
		MPI_Comm targ_comm;
		EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&realrank);
		if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
			int reprank;
			EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_REP,&reprank);
			myrank = repToCmpMap[reprank];
		} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
			EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_CMP,&myrank);
		}
		bool completerep = false;
		bool completecmp = false;
		if(no_req_exists) {
			data->req = (MPI_Request *)parep_mpi_malloc(sizeof(MPI_Request));
			*(data->req) = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
			(*(data->req))->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
			(*(data->req))->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
			*((*(data->req))->reqcmp) = EMPI_REQUEST_NULL;
			*((*(data->req))->reqrep) = EMPI_REQUEST_NULL;
			((*(data->req))->reqcolls) = NULL;
			((*(data->req))->num_reqcolls) = 0;
		} else {
			*((*(data->req))->reqcmp) = EMPI_REQUEST_NULL;
			*((*(data->req))->reqrep) = EMPI_REQUEST_NULL;
			for(int i = 0; i < data->num_colls; i++) {
				*((*(data->req))->reqcolls[i]) = EMPI_REQUEST_NULL;
			}
		}
		
		if(data->type == MPI_FT_BARRIER) {
			data->num_colls = 0;
			data->completecolls = NULL;
			targ_comm = (data->args).barrier.comm;
			retVal = EMPI_Ibarrier (((data->args).barrier.comm)->eworldComm, ((*(data->req))->reqcmp));
			completerep = true;
		} else if(data->type == MPI_FT_BCAST) {
			data->num_colls = 0;
			data->completecolls = NULL;
			targ_comm = (data->args).bcast.comm;
			int size;
			EMPI_Type_size((data->args).bcast.dt->edatatype,&size);
			if((data->args).bcast.buf == NULL) {
				send_src = _real_malloc((data->args).bcast.count * size);
			} else {
				send_src = _real_malloc((data->args).bcast.count * size);
				memcpy(send_src, (data->args).bcast.buf, (data->args).bcast.count * size);
			}
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				if(data->id > repid && repid >= 0) {
					completecmp = true;
					if(cmpToRepMap[(data->args).bcast.root] == -1) retVal = EMPI_Ibcast (send_src, (data->args).bcast.count, (data->args).bcast.dt->edatatype, (data->args).bcast.root, ((data->args).bcast.comm)->EMPI_CMP_REP_INTERCOMM, ((*(data->req))->reqrep));
					else retVal = EMPI_Ibcast (send_src, (data->args).bcast.count, (data->args).bcast.dt->edatatype, cmpToRepMap[(data->args).bcast.root], ((data->args).bcast.comm)->EMPI_COMM_REP, ((*(data->req))->reqrep));
				} else {
					completecmp = true;
					completerep = true;
				}
			} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				if(data->id > cmpid && cmpid >= 0) retVal = EMPI_Ibcast (send_src, (data->args).bcast.count, (data->args).bcast.dt->edatatype, (data->args).bcast.root, ((data->args).bcast.comm)->EMPI_COMM_CMP, ((*(data->req))->reqcmp));
				else completecmp = true;
				if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).bcast.root] == -1 && myrank == (data->args).bcast.root && nR > 0) retVal = EMPI_Ibcast (send_src, (data->args).bcast.count, (data->args).bcast.dt->edatatype, EMPI_ROOT, ((data->args).bcast.comm)->EMPI_CMP_REP_INTERCOMM, ((*(data->req))->reqrep));
				else if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).bcast.root] == -1 && nR > 0) retVal = EMPI_Ibcast (send_src, (data->args).bcast.count, (data->args).bcast.dt->edatatype, EMPI_PROC_NULL, ((data->args).bcast.comm)->EMPI_CMP_REP_INTERCOMM, ((*(data->req))->reqrep));
				else completerep = true;
			}
		} else if(data->type == MPI_FT_SCATTER) {
			data->num_colls = 0;
			data->completecolls = NULL;
			targ_comm = (data->args).scatter.comm;
			int *tmpscounts;
			int *tmpsdispls;
			int rsize;
			int ssize;
			EMPI_Type_size((data->args).scatter.recvdt->edatatype,&rsize);
			EMPI_Type_size((data->args).scatter.senddt->edatatype,&ssize);
			if(no_req_exists) {
				recv_dest = _real_malloc((data->args).scatter.recvcount * rsize);
			} else {
				recv_dest = (*(data->req))->bufloc;
			}
			if((data->args).scatter.sendbuf == NULL) {
				send_src = _real_malloc((data->args).scatter.sendcount * ssize);
			} else {
				send_src = _real_malloc((data->args).scatter.sendcount * ssize);
				memcpy(send_src, (data->args).scatter.sendbuf, (data->args).scatter.sendcount * ssize);
			}
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				EMPI_Comm_size((((data->args).scatter.comm)->EMPI_COMM_REP),&commnR);
				EMPI_Comm_remote_size((((data->args).scatter.comm)->EMPI_CMP_REP_INTERCOMM),&commnC);
					
				tmpscounts = (int *)_real_malloc(commnR*sizeof(int));
				tmpsdispls = (int *)_real_malloc(commnR*sizeof(int));
				int j = 0;
				for(int i = 0; i < commnC; i++) {
					if(cmpToRepMap[i] != -1) {
						tmpscounts[j] = (data->args).scatter.sendcount;
						tmpsdispls[j] = i*(data->args).scatter.sendcount;
						j++;
					}
				}
				completecmp = true;
				if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).scatter.root] == -1) retVal = EMPI_Iscatterv (send_src, tmpscounts, tmpsdispls, (data->args).scatter.senddt->edatatype, recv_dest, (data->args).scatter.recvcount, (data->args).scatter.recvdt->edatatype, (data->args).scatter.root, ((data->args).scatter.comm)->EMPI_CMP_REP_INTERCOMM, ((*(data->req))->reqrep));
				else if(data->id > repid && repid >= 0) retVal = EMPI_Iscatterv (send_src, tmpscounts, tmpsdispls, (data->args).scatter.senddt->edatatype, recv_dest, (data->args).scatter.recvcount, (data->args).scatter.recvdt->edatatype, cmpToRepMap[(data->args).scatter.root], ((data->args).scatter.comm)->EMPI_COMM_REP, ((*(data->req))->reqrep));
				else completerep = true;
				
				_real_free(tmpscounts);
				_real_free(tmpsdispls);
			} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				EMPI_Comm_size((((data->args).scatter.comm)->EMPI_COMM_CMP),&commnC);
				if((((data->args).scatter.comm)->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((((data->args).scatter.comm)->EMPI_CMP_REP_INTERCOMM),&commnR);
				else commnR = 0;
				
				tmpscounts = (int *)_real_malloc(commnR*sizeof(int));
				tmpsdispls = (int *)_real_malloc(commnR*sizeof(int));
				int j = 0;
				for(int i = 0; i < commnC; i++) {
					if(cmpToRepMap[i] != -1) {
						tmpscounts[j] = (data->args).scatter.sendcount;
						tmpsdispls[j] = i*(data->args).scatter.sendcount;
						j++;
					}
				}
				if(data->id > cmpid && cmpid >= 0) retVal = EMPI_Iscatter (send_src, (data->args).scatter.sendcount, (data->args).scatter.senddt->edatatype, recv_dest, (data->args).scatter.recvcount, (data->args).scatter.recvdt->edatatype, (data->args).scatter.root, ((data->args).scatter.comm)->EMPI_COMM_CMP, ((*(data->req))->reqcmp));
				else completecmp = true;
				if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).scatter.root] == -1 && myrank == (data->args).scatter.root && nR > 0) retVal = EMPI_Iscatterv (send_src, tmpscounts, tmpsdispls, (data->args).scatter.senddt->edatatype, recv_dest, (data->args).scatter.recvcount, (data->args).scatter.recvdt->edatatype, EMPI_ROOT, ((data->args).scatter.comm)->EMPI_CMP_REP_INTERCOMM, ((*(data->req))->reqrep));
				else if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).scatter.root] == -1 && nR > 0) retVal = EMPI_Iscatterv (send_src, tmpscounts, tmpsdispls, (data->args).scatter.senddt->edatatype, recv_dest, (data->args).scatter.recvcount, (data->args).scatter.recvdt->edatatype, EMPI_PROC_NULL, ((data->args).scatter.comm)->EMPI_CMP_REP_INTERCOMM, ((*(data->req))->reqrep));
				else completerep = true;
				
				_real_free(tmpscounts);
				_real_free(tmpsdispls);
			}
		} else if(data->type == MPI_FT_GATHER) {
			data->num_colls = 0;
			data->completecolls = NULL;
			targ_comm = (data->args).gather.comm;
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				EMPI_Comm_size((((data->args).gather.comm)->EMPI_COMM_REP),&commnR);
				EMPI_Comm_remote_size((((data->args).gather.comm)->EMPI_CMP_REP_INTERCOMM),&commnC);
			} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				EMPI_Comm_size((((data->args).gather.comm)->EMPI_COMM_CMP),&commnC);
				if((((data->args).gather.comm)->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((((data->args).gather.comm)->EMPI_CMP_REP_INTERCOMM),&commnR);
				else commnR = 0;
			}
			int *tmprcounts;
			int *tmprdispls;
			int *tmprcountsinter;
			int *tmprdisplsinter;
			int rsize;
			EMPI_Type_size((data->args).gather.recvdt->edatatype,&rsize);
			if(no_req_exists) {
				recv_dest = _real_malloc((data->args).gather.recvcount * rsize * commnC);
			} else {
				recv_dest = (*(data->req))->bufloc;
			}
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				if(data->id > repid && repid >= 0) {
					tmprcounts = (int *)_real_malloc(commnR*sizeof(int));
					tmprdispls = (int *)_real_malloc(commnR*sizeof(int));
				}
				if(commnC > commnR && commnR > 0 && data->id > repid && repid >= 0) {
					tmprcountsinter = (int *)_real_malloc((commnC-commnR)*sizeof(int));
					tmprdisplsinter = (int *)_real_malloc((commnC-commnR)*sizeof(int));
				}
				if(cmpToRepMap[(data->args).gather.root] != -1 && data->id > repid && repid >= 0) {
					int j = 0;
					int k = 0;
					for(int i = 0; i < commnC; i++) {
						if(cmpToRepMap[i] != -1) {
							tmprcounts[j] = (data->args).gather.recvcount;
							tmprdispls[j] = i*(data->args).gather.recvcount;
							j++;
						}
						else if(commnC > commnR && commnR > 0) {
							tmprcountsinter[k] = (data->args).gather.recvcount;
							tmprdisplsinter[k] = i*(data->args).gather.recvcount;
							k++;
						}
					}
					if(commnC > commnR && commnR > 0 && myrank == (data->args).gather.root) retVal = EMPI_Igatherv ((data->args).gather.sendbuf, (data->args).gather.sendcount, (data->args).gather.senddt->edatatype, recv_dest, tmprcountsinter, tmprdisplsinter, (data->args).gather.recvdt->edatatype, EMPI_ROOT, ((data->args).gather.comm)->EMPI_CMP_NO_REP_INTERCOMM, ((*(data->req))->reqcmp));
					else if(commnC > commnR && commnR > 0) retVal = EMPI_Igatherv ((data->args).gather.sendbuf, (data->args).gather.sendcount, (data->args).gather.senddt->edatatype, recv_dest, tmprcountsinter, tmprdisplsinter, (data->args).gather.recvdt->edatatype, EMPI_PROC_NULL, ((data->args).gather.comm)->EMPI_CMP_NO_REP_INTERCOMM, ((*(data->req))->reqcmp));
					else completecmp = true;
					retVal = EMPI_Igatherv ((data->args).gather.sendbuf, (data->args).gather.sendcount, (data->args).gather.senddt->edatatype, recv_dest, tmprcounts, tmprdispls, (data->args).gather.recvdt->edatatype, cmpToRepMap[(data->args).gather.root], ((data->args).gather.comm)->EMPI_COMM_REP, ((*(data->req))->reqrep));
					
					_real_free(tmprcounts);
					_real_free(tmprdispls);
					if(commnC > commnR && commnR > 0) {
						_real_free(tmprcountsinter);
						_real_free(tmprdisplsinter);
					}
				} else {
					completecmp = true;
					completerep = true;
				}
			} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				if(cmpToRepMap[(data->args).gather.root] != -1 && cmpToRepMap[myrank] == -1 && data->id > repid && repid >= 0) {
					tmprcountsinter = (int *)_real_malloc((commnC-commnR)*sizeof(int));
					tmprdisplsinter = (int *)_real_malloc((commnC-commnR)*sizeof(int));
				}
				if(cmpToRepMap[(data->args).gather.root] != -1 && cmpToRepMap[myrank] == -1 && data->id > repid && repid >= 0) {
					int j = 0;
					for(int i = 0; i < commnC; i++) {
						if(cmpToRepMap[i] != -1) {
							tmprcountsinter[j] = (data->args).gather.recvcount;
							tmprdisplsinter[j] = i*(data->args).gather.recvcount;
							j++;
						}
					}
				}
				
				if(data->id > cmpid && cmpid >= 0) retVal = EMPI_Igather ((data->args).gather.sendbuf, (data->args).gather.sendcount, (data->args).gather.senddt->edatatype, recv_dest, (data->args).gather.recvcount, (data->args).gather.recvdt->edatatype, (data->args).gather.root, ((data->args).gather.comm)->EMPI_COMM_CMP, ((*(data->req))->reqcmp));
				else completecmp = true;
				if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).gather.root] != -1 && cmpToRepMap[myrank] == -1) retVal = EMPI_Igatherv ((data->args).gather.sendbuf, (data->args).gather.sendcount, (data->args).gather.senddt->edatatype, recv_dest, tmprcountsinter, tmprdisplsinter, (data->args).gather.recvdt->edatatype, cmpToRepMap[(data->args).gather.root], ((data->args).gather.comm)->EMPI_CMP_REP_INTERCOMM, ((*(data->req))->reqrep));
				else completerep = true;
				
				if(cmpToRepMap[(data->args).gather.root] != -1 && cmpToRepMap[myrank] == -1 && data->id > repid && repid >= 0) {
					_real_free(tmprcountsinter);
					_real_free(tmprdisplsinter);
				}
			}
		} else if(data->type == MPI_FT_ALLGATHER) {
			targ_comm = (data->args).allgather.comm;
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				EMPI_Comm_size((((data->args).allgather.comm)->EMPI_COMM_REP),&commnR);
				EMPI_Comm_remote_size((((data->args).allgather.comm)->EMPI_CMP_REP_INTERCOMM),&commnC);
			} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				EMPI_Comm_size((((data->args).allgather.comm)->EMPI_COMM_CMP),&commnC);
				if((((data->args).allgather.comm)->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((((data->args).allgather.comm)->EMPI_CMP_REP_INTERCOMM),&commnR);
				else commnR = 0;
			}
			int *tmpscounts;
			int *tmpsdispls;
			int *tmprcounts;
			int *tmprdispls;
			int *tmprcountsinter;
			int *tmprdisplsinter;
			int rsize;
			EMPI_Type_size((data->args).allgather.recvdt->edatatype,&rsize);
			if(no_req_exists) {
				recv_dest = _real_malloc((data->args).allgather.recvcount * rsize * commnC);
			} else {
				recv_dest = (*(data->req))->bufloc;
			}
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				if(data->id > repid && repid >= 0) {
					tmpscounts = (int *)_real_malloc(commnR*sizeof(int));
					tmpsdispls = (int *)_real_malloc(commnR*sizeof(int));
					tmprcounts = (int *)_real_malloc(commnR*sizeof(int));
					tmprdispls = (int *)_real_malloc(commnR*sizeof(int));
					if(commnC > commnR && commnR > 0) {
						tmprcountsinter = (int *)_real_malloc((commnC-commnR)*sizeof(int));
						tmprdisplsinter = (int *)_real_malloc((commnC-commnR)*sizeof(int));
						if(data->num_colls > 0) parep_mpi_free(data->completecolls);
						if(no_req_exists) {
							(*(data->req))->reqcolls = (EMPI_Request **)parep_mpi_malloc(sizeof(EMPI_Request *) * commnR);
							(*(data->req))->num_reqcolls = commnR;
						} else {
							for(int i = 0; i < (*(data->req))->num_reqcolls; i++) parep_mpi_free(((*(data->req))->reqcolls[i]));
							(*(data->req))->reqcolls = (EMPI_Request **)parep_mpi_realloc((*(data->req))->reqcolls,sizeof(EMPI_Request *) * commnR);
							(*(data->req))->num_reqcolls = commnR;
						}
						for(int i = 0; i < (*(data->req))->num_reqcolls; i++) *((*(data->req))->reqcolls[i]) = EMPI_REQUEST_NULL;
						data->completecolls = (bool *)parep_mpi_malloc(sizeof(bool) * commnR);
						data->num_colls = commnR;
					} else {
						if(data->num_colls > 0) parep_mpi_free(data->completecolls);
						if(no_req_exists) {
							(*(data->req))->reqcolls = NULL;
							(*(data->req))->num_reqcolls = 0;
						} else {
							for(int i = 0; i < (*(data->req))->num_reqcolls; i++) parep_mpi_free(((*(data->req))->reqcolls[i]));
							if((*(data->req))->num_reqcolls > 0) parep_mpi_free((*(data->req))->reqcolls);
							(*(data->req))->reqcolls = NULL;
							(*(data->req))->num_reqcolls = 0;
						}
						data->completecolls = NULL;
						data->num_colls = 0;
					}
					
					int j = 0;
					int k = 0;
					for(int i = 0; i < commnC; i++) {
						if(cmpToRepMap[i] != -1) {
							tmpscounts[j] = (data->args).allgather.sendcount;
							tmpsdispls[j] = i*(data->args).allgather.sendcount;
							tmprcounts[j] = (data->args).allgather.recvcount;
							tmprdispls[j] = i*(data->args).allgather.recvcount;
							j++;
						}
						else if(commnC > commnR && commnR > 0) {
							tmprcountsinter[k] = (data->args).allgather.recvcount;
							tmprdisplsinter[k] = i*(data->args).allgather.recvcount;
							k++;
						}
					}
					
					completecmp = true;
					if(commnC > commnR && commnR > 0) {
						for(int i = 0; i < commnR; i++) {
							((*(data->req))->reqcolls[i]) = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
							data->completecolls[i] = false;
							if(cmpToRepMap[myrank] == i) retVal = EMPI_Igatherv ((data->args).allgather.sendbuf, (data->args).allgather.sendcount, (data->args).allgather.senddt->edatatype, recv_dest, tmprcountsinter, tmprdisplsinter, (data->args).allgather.recvdt->edatatype, EMPI_ROOT, ((data->args).allgather.comm->EMPI_CMP_NO_REP_INTERCOMM), ((*(data->req))->reqcolls[i]));
							else retVal = EMPI_Igatherv ((data->args).allgather.sendbuf, (data->args).allgather.sendcount, (data->args).allgather.senddt->edatatype, recv_dest, tmprcountsinter, tmprdisplsinter, (data->args).allgather.recvdt->edatatype, EMPI_PROC_NULL, ((data->args).allgather.comm->EMPI_CMP_NO_REP_INTERCOMM), ((*(data->req))->reqcolls[i]));
						}
					}
					retVal = EMPI_Iallgatherv ((data->args).allgather.sendbuf, tmpscounts, tmpsdispls, (data->args).allgather.senddt->edatatype, recv_dest, tmprcounts, tmprdispls, (data->args).allgather.recvdt->edatatype, ((data->args).allgather.comm)->EMPI_COMM_REP, ((*(data->req))->reqrep));
						
					_real_free(tmpscounts);
					_real_free(tmpsdispls);
					_real_free(tmprcounts);
					_real_free(tmprdispls);
					if(commnC > commnR && commnR > 0) {
						_real_free(tmprcountsinter);
						_real_free(tmprdisplsinter);
					}
				} else {
					completecmp = true;
					completerep = true;
					for(int i = 0; i < data->num_colls; i++) {
						data->completecolls[i] = true;
					}
				}
			} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				if(data->id > cmpid && cmpid >= 0) retVal = EMPI_Iallgather ((data->args).allgather.sendbuf, (data->args).allgather.sendcount, (data->args).allgather.senddt->edatatype, recv_dest, (data->args).allgather.recvcount, (data->args).allgather.recvdt->edatatype, ((data->args).allgather.comm)->EMPI_COMM_CMP, ((*(data->req))->reqcmp));
				else completecmp = true;
				if(data->id > repid && repid >= 0) {
					if(commnR > 0 && cmpToRepMap[myrank] == -1) {
						tmprcountsinter = (int *)_real_malloc((commnC-commnR)*sizeof(int));
						tmprdisplsinter = (int *)_real_malloc((commnC-commnR)*sizeof(int));
						int j = 0;
						for(int i = 0; i < commnC; i++) {
							if(cmpToRepMap[i] == -1) {
								tmprcountsinter[j] = (data->args).allgather.recvcount;
								tmprdisplsinter[j] = i*(data->args).allgather.recvcount;
								j++;
							}
						}
						
						if(data->num_colls > 0) parep_mpi_free(data->completecolls);
						if(no_req_exists) {
							(*(data->req))->reqcolls = (EMPI_Request **)parep_mpi_malloc(sizeof(EMPI_Request *) * commnR);
							(*(data->req))->num_reqcolls = commnR;
						} else {
							for(int i = 0; i < (*(data->req))->num_reqcolls; i++) parep_mpi_free(((*(data->req))->reqcolls[i]));
							(*(data->req))->reqcolls = (EMPI_Request **)parep_mpi_realloc((*(data->req))->reqcolls,sizeof(EMPI_Request *) * commnR);
							(*(data->req))->num_reqcolls = commnR;
						}
						for(int i = 0; i < (*(data->req))->num_reqcolls; i++) *((*(data->req))->reqcolls[i]) = EMPI_REQUEST_NULL;
						data->completecolls = (bool *)parep_mpi_malloc(sizeof(bool) * commnR);
						data->num_colls = commnR;
						
						for(int i = 0; i < commnR; i++) {
							((*(data->req))->reqcolls[i]) = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
							data->completecolls[i] = false;
							retVal = EMPI_Igatherv ((data->args).allgather.sendbuf, (data->args).allgather.sendcount, (data->args).allgather.senddt->edatatype, recv_dest, tmprcountsinter, tmprdisplsinter, (data->args).allgather.recvdt->edatatype, i, ((data->args).allgather.comm->EMPI_CMP_NO_REP_INTERCOMM), ((*(data->req))->reqcolls[i]));
						}
					} else {
						if(data->num_colls > 0) parep_mpi_free(data->completecolls);
						if(no_req_exists) {
							(*(data->req))->reqcolls = NULL;
							(*(data->req))->num_reqcolls = 0;
						} else {
							for(int i = 0; i < (*(data->req))->num_reqcolls; i++) parep_mpi_free(((*(data->req))->reqcolls[i]));
							if((*(data->req))->num_reqcolls > 0) parep_mpi_free((*(data->req))->reqcolls);
							(*(data->req))->reqcolls = NULL;
							(*(data->req))->num_reqcolls = 0;
						}
						data->completecolls = NULL;
						data->num_colls = 0;
					}
					completerep = true;
					
					if (commnR > 0 && cmpToRepMap[myrank] == -1) {
						free(tmprcountsinter);
						free(tmprdisplsinter);
					}
				} else {
					completerep = true;
					for(int i = 0; i < data->num_colls; i++) {
						data->completecolls[i] = true;
					}
				}
			}
		} else if(data->type == MPI_FT_ALLTOALL) {
			targ_comm = (data->args).alltoall.comm;
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				EMPI_Comm_size((((data->args).alltoall.comm)->EMPI_COMM_REP),&commnR);
				EMPI_Comm_remote_size((((data->args).alltoall.comm)->EMPI_CMP_REP_INTERCOMM),&commnC);
			} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				EMPI_Comm_size((((data->args).alltoall.comm)->EMPI_COMM_CMP),&commnC);
				if((((data->args).alltoall.comm)->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((((data->args).alltoall.comm)->EMPI_CMP_REP_INTERCOMM),&commnR);
				else commnR = 0;
			}
			int *tmpscounts;
			int *tmpsdispls;
			int *tmprcounts;
			int *tmprdispls;
			int *tmpscountsinter;
			int *tmpsdisplsinter;
			int *intermapping;
			int noreprank;
			int rsize;
			EMPI_Type_size((data->args).alltoall.recvdt->edatatype,&rsize);
			if(no_req_exists) {
				recv_dest = _real_malloc((data->args).alltoall.recvcount * rsize * commnC);
			} else {
				recv_dest = (*(data->req))->bufloc;
			}
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				if(data->id > repid && repid >= 0) {
					tmpscounts = (int *)_real_malloc(commnR*sizeof(int));
					tmpsdispls = (int *)_real_malloc(commnR*sizeof(int));
					tmprcounts = (int *)_real_malloc(commnR*sizeof(int));
					tmprdispls = (int *)_real_malloc(commnR*sizeof(int));
						
					if(commnC > commnR && commnR > 0) {
						tmpscountsinter = (int *)_real_malloc((commnR)*sizeof(int));
						tmpsdisplsinter = (int *)_real_malloc((commnR)*sizeof(int));
						intermapping = (int *)_real_malloc((commnC-commnR)*sizeof(int));
						if(data->num_colls > 0) parep_mpi_free(data->completecolls);
						if(no_req_exists) {
							(*(data->req))->reqcolls = (EMPI_Request **)parep_mpi_malloc(sizeof(EMPI_Request *) * (commnC-commnR));
							(*(data->req))->num_reqcolls = (commnC-commnR);
						} else {
							for(int i = 0; i < (*(data->req))->num_reqcolls; i++) parep_mpi_free(((*(data->req))->reqcolls[i]));
							(*(data->req))->reqcolls = (EMPI_Request **)parep_mpi_realloc((*(data->req))->reqcolls,sizeof(EMPI_Request *) * (commnC-commnR));
							(*(data->req))->num_reqcolls = (commnC-commnR);
						}
						for(int i = 0; i < (*(data->req))->num_reqcolls; i++) *((*(data->req))->reqcolls[i]) = EMPI_REQUEST_NULL;
						data->completecolls = (bool *)parep_mpi_malloc(sizeof(bool) * (commnC-commnR));
						data->num_colls = (commnC-commnR);
					} else {
						if(data->num_colls > 0) parep_mpi_free(data->completecolls);
						if(no_req_exists) {
							(*(data->req))->reqcolls = NULL;
							(*(data->req))->num_reqcolls = 0;
						} else {
							for(int i = 0; i < (*(data->req))->num_reqcolls; i++) parep_mpi_free(((*(data->req))->reqcolls[i]));
							if((*(data->req))->num_reqcolls > 0) parep_mpi_free((*(data->req))->reqcolls);
							(*(data->req))->reqcolls = NULL;
							(*(data->req))->num_reqcolls = 0;
						}
						data->completecolls = NULL;
						data->num_colls = 0;
					}
						
					int j = 0;
					int k = 0;
					for(int i = 0; i < commnC; i++) {
						if(cmpToRepMap[i] != -1) {
							tmpscounts[j] = (data->args).alltoall.sendcount;
							tmpsdispls[j] = i*(data->args).alltoall.sendcount;
							tmprcounts[j] = (data->args).alltoall.recvcount;
							tmprdispls[j] = i*(data->args).alltoall.recvcount;
							if(commnC > commnR && commnR > 0) {
								tmpscountsinter[j] = (data->args).alltoall.sendcount;
								tmpsdisplsinter[j] = i*(data->args).alltoall.sendcount;
							}
							j++;
						} else if(commnC > commnR && commnR > 0) {
							intermapping[k] = i;
							k++;
						}
					}
					
					completecmp = true;
					if(commnC > commnR && commnR > 0) {
						for(int i = 0; i < commnC-commnR; i++) {
							((*(data->req))->reqcolls[i]) = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
							data->completecolls[i] = false;
							retVal = EMPI_Iscatterv ((data->args).alltoall.sendbuf, tmpscountsinter, tmpsdisplsinter, (data->args).alltoall.senddt->edatatype, recv_dest + intermapping[i]*(data->args).alltoall.recvcount*rsize, (data->args).alltoall.recvcount, (data->args).alltoall.recvdt->edatatype, i, ((data->args).alltoall.comm->EMPI_CMP_NO_REP_INTERCOMM), ((*(data->req))->reqcolls[i]));
						}
					}
					retVal = EMPI_Ialltoallv ((data->args).alltoall.sendbuf, tmpscounts, tmpsdispls, (data->args).alltoall.senddt->edatatype, recv_dest, tmprcounts, tmprdispls, (data->args).alltoall.recvdt->edatatype, ((data->args).alltoall.comm)->EMPI_COMM_REP, ((*(data->req))->reqrep));

					_real_free(tmpscounts);
					_real_free(tmpsdispls);
					_real_free(tmprcounts);
					_real_free(tmprdispls);
					if(commnC > commnR && commnR > 0) {
						_real_free(tmpscountsinter);
						_real_free(tmpsdisplsinter);
						_real_free(intermapping);
					}
				} else {
					completecmp = true;
					completerep = true;
					for(int i = 0; i < data->num_colls; i++) {
						data->completecolls[i] = true;
					}
				}
			} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				if(data->id > cmpid && cmpid >= 0) retVal = EMPI_Ialltoall ((data->args).alltoall.sendbuf, (data->args).alltoall.sendcount, (data->args).alltoall.senddt->edatatype, recv_dest, (data->args).alltoall.recvcount, (data->args).alltoall.recvdt->edatatype, ((data->args).alltoall.comm)->EMPI_COMM_CMP, ((*(data->req))->reqcmp));
				else completecmp = true;
				if(data->id > repid && repid >= 0) {
					if(commnR > 0 && cmpToRepMap[myrank] == -1) {
						tmpscountsinter = (int *)_real_malloc((commnR)*sizeof(int));
						tmpsdisplsinter = (int *)_real_malloc((commnR)*sizeof(int));
						intermapping = (int *)_real_malloc((commnC-commnR)*sizeof(int));
						int j = 0;
						int k = 0;
						for(int i = 0; i < commnC; i++) {
							if(cmpToRepMap[i] != -1) {
								tmpscountsinter[j] = (data->args).alltoall.sendcount;
								tmpsdisplsinter[j] = i*(data->args).alltoall.sendcount;
								j++;
							} else {
								intermapping[k] = i;
								k++;
							}
						}
						
						if(data->num_colls > 0) parep_mpi_free(data->completecolls);
						if(no_req_exists) {
							(*(data->req))->reqcolls = (EMPI_Request **)parep_mpi_malloc(sizeof(EMPI_Request *) * (commnC-commnR));
							(*(data->req))->num_reqcolls = (commnC-commnR);
						} else {
							for(int i = 0; i < (*(data->req))->num_reqcolls; i++) parep_mpi_free(((*(data->req))->reqcolls[i]));
							(*(data->req))->reqcolls = (EMPI_Request **)parep_mpi_realloc((*(data->req))->reqcolls,sizeof(EMPI_Request *) * (commnC-commnR));
							(*(data->req))->num_reqcolls = (commnC-commnR);
						}
						for(int i = 0; i < (*(data->req))->num_reqcolls; i++) *((*(data->req))->reqcolls[i]) = EMPI_REQUEST_NULL;
						data->completecolls = (bool *)parep_mpi_malloc(sizeof(bool) * (commnC-commnR));
						data->num_colls = (commnC-commnR);
						
						EMPI_Comm_rank(((data->args).alltoall.comm->EMPI_CMP_NO_REP),&noreprank);
						for(int i = 0; i < commnC-commnR; i++) {
							((*(data->req))->reqcolls[i]) = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
							data->completecolls[i] = false;
							if(noreprank == i) retVal = EMPI_Iscatterv ((data->args).alltoall.sendbuf, tmpscountsinter, tmpsdisplsinter, (data->args).alltoall.senddt->edatatype, recv_dest + intermapping[i]*(data->args).alltoall.recvcount*rsize, (data->args).alltoall.recvcount, (data->args).alltoall.recvdt->edatatype, EMPI_ROOT, ((data->args).alltoall.comm->EMPI_CMP_NO_REP_INTERCOMM), ((*(data->req))->reqcolls[i]));
							else retVal = EMPI_Iscatterv ((data->args).alltoall.sendbuf, tmpscountsinter, tmpsdisplsinter, (data->args).alltoall.senddt->edatatype, (data->args).alltoall.recvbuf + intermapping[i]*(data->args).alltoall.recvcount*rsize, (data->args).alltoall.recvcount, (data->args).alltoall.recvdt->edatatype, EMPI_PROC_NULL, ((data->args).alltoall.comm->EMPI_CMP_NO_REP_INTERCOMM), ((*(data->req))->reqcolls[i]));
						}
					} else {
						if(data->num_colls > 0) parep_mpi_free(data->completecolls);
						if(no_req_exists) {
							(*(data->req))->reqcolls = NULL;
							(*(data->req))->num_reqcolls = 0;
						} else {
							for(int i = 0; i < (*(data->req))->num_reqcolls; i++) parep_mpi_free(((*(data->req))->reqcolls[i]));
							if((*(data->req))->num_reqcolls > 0) parep_mpi_free((*(data->req))->reqcolls);
							(*(data->req))->reqcolls = NULL;
							(*(data->req))->num_reqcolls = 0;
						}
						data->completecolls = NULL;
						data->num_colls = 0;
					}
					completerep = true;
					
					if (commnR > 0 && cmpToRepMap[myrank] == -1) {
						_real_free(tmpscountsinter);
						_real_free(tmpsdisplsinter);
						_real_free(intermapping);
					}
				} else {
					completerep = true;
					for(int i = 0; i < data->num_colls; i++) {
						data->completecolls[i] = true;
					}
				}
			}
		} else if(data->type == MPI_FT_ALLTOALLV) {
			targ_comm = (data->args).alltoallv.comm;
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				EMPI_Comm_size((((data->args).alltoallv.comm)->EMPI_COMM_REP),&commnR);
				EMPI_Comm_remote_size((((data->args).alltoallv.comm)->EMPI_CMP_REP_INTERCOMM),&commnC);
			} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				EMPI_Comm_size((((data->args).alltoallv.comm)->EMPI_COMM_CMP),&commnC);
				if((((data->args).alltoallv.comm)->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((((data->args).alltoallv.comm)->EMPI_CMP_REP_INTERCOMM),&commnR);
				else commnR = 0;
			}
			int *tmpscounts;
			int *tmpsdispls;
			int *tmpsdisplsintra;
			int *tmprcounts;
			int *tmprdispls;
			int *tmprdisplsintra;
			int *tmpscountsinter;
			int *tmpsdisplsinter;
			int *intermapping;
			int noreprank;
			int rsize;
			long rcount;
			EMPI_Type_size((data->args).alltoallv.recvdt->edatatype,&rsize);
			rcount = array_sum((data->args).alltoallv.recvcounts,commnC);
			if(no_req_exists) {
				recv_dest = _real_malloc(rcount * rsize);
			} else {
				recv_dest = (*(data->req))->bufloc;
			}
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				if(data->id > repid && repid >= 0) {
					tmpscounts = (int *)_real_malloc(commnR*sizeof(int));
					tmpsdispls = (int *)_real_malloc(commnR*sizeof(int));
					tmprcounts = (int *)_real_malloc(commnR*sizeof(int));
					tmprdispls = (int *)_real_malloc(commnR*sizeof(int));
					if(commnC > commnR && commnR > 0) {
						tmpscountsinter = (int *)_real_malloc((commnR)*sizeof(int));
						tmpsdisplsinter = (int *)_real_malloc((commnR)*sizeof(int));
						intermapping = (int *)_real_malloc((commnC-commnR)*sizeof(int));
						if(data->num_colls > 0) parep_mpi_free(data->completecolls);
						if(no_req_exists) {
							(*(data->req))->reqcolls = (EMPI_Request **)parep_mpi_malloc(sizeof(EMPI_Request *) * (commnC-commnR));
							(*(data->req))->num_reqcolls = (commnC-commnR);
						} else {
							for(int i = 0; i < (*(data->req))->num_reqcolls; i++) parep_mpi_free(((*(data->req))->reqcolls[i]));
							(*(data->req))->reqcolls = (EMPI_Request **)parep_mpi_realloc((*(data->req))->reqcolls,sizeof(EMPI_Request *) * (commnC-commnR));
							(*(data->req))->num_reqcolls = (commnC-commnR);
						}
						for(int i = 0; i < (*(data->req))->num_reqcolls; i++) *((*(data->req))->reqcolls[i]) = EMPI_REQUEST_NULL;
						data->completecolls = (bool *)parep_mpi_malloc(sizeof(bool) * (commnC-commnR));
						data->num_colls = (commnC-commnR);
					} else {
						if(data->num_colls > 0) parep_mpi_free(data->completecolls);
						if(no_req_exists) {
							(*(data->req))->reqcolls = NULL;
							(*(data->req))->num_reqcolls = 0;
						} else {
							for(int i = 0; i < (*(data->req))->num_reqcolls; i++) parep_mpi_free(((*(data->req))->reqcolls[i]));
							if((*(data->req))->num_reqcolls > 0) parep_mpi_free((*(data->req))->reqcolls);
							(*(data->req))->reqcolls = NULL;
							(*(data->req))->num_reqcolls = 0;
						}
						data->completecolls = NULL;
						data->num_colls = 0;
					}

					int j = 0;
					int k = 0;
					int snumcounts = 0;
					int rnumcounts = 0;
					for(int i = 0; i < commnC; i++) {
						if(cmpToRepMap[i] != -1) {
							tmpscounts[j] = (data->args).alltoallv.sendcounts[i];
							//tmpsdispls[j] = (data->args).alltoallv.sdispls[i];
							tmpsdispls[j] = snumcounts;
							tmprcounts[j] = (data->args).alltoallv.recvcounts[i];
							//tmprdispls[j] = (data->args).alltoallv.rdispls[i];
							tmprdispls[j] = rnumcounts;
							if(commnC > commnR && commnR > 0) {
								tmpscountsinter[j] = (data->args).alltoallv.sendcounts[i];
								//tmpsdisplsinter[j] = (data->args).alltoallv.sdispls[i];
								tmpsdisplsinter[j] = snumcounts;
							}
							j++;
						} else if(commnC > commnR && commnR > 0) {
							intermapping[k] = i;
							k++;
						}
						snumcounts += (data->args).alltoallv.sendcounts[i];
						rnumcounts += (data->args).alltoallv.recvcounts[i];
					}
					
					completecmp = true;
					if(commnC > commnR && commnR > 0) {
						for(int i = 0; i < commnC-commnR; i++) {
							((*(data->req))->reqcolls[i]) = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
							data->completecolls[i] = false;
							retVal = EMPI_Iscatterv ((data->args).alltoallv.sendbuf, tmpscountsinter, tmpsdisplsinter, (data->args).alltoallv.senddt->edatatype, recv_dest, (data->args).alltoallv.recvcounts[intermapping[i]], (data->args).alltoallv.recvdt->edatatype, i, ((data->args).alltoallv.comm->EMPI_CMP_NO_REP_INTERCOMM), ((*(data->req))->reqcolls[i]));
						}
					}
					retVal = EMPI_Ialltoallv ((data->args).alltoallv.sendbuf, tmpscounts, tmpsdispls, (data->args).alltoallv.senddt->edatatype, recv_dest, tmprcounts, tmprdispls, (data->args).alltoallv.recvdt->edatatype, ((data->args).alltoallv.comm)->EMPI_COMM_REP, ((*(data->req))->reqrep));
					
					_real_free(tmpscounts);
					_real_free(tmpsdispls);
					_real_free(tmprcounts);
					_real_free(tmprdispls);
					if(commnC > commnR && commnR > 0) {
						_real_free(tmpscountsinter);
						_real_free(tmpsdisplsinter);
						_real_free(intermapping);
					}
				} else {
					completecmp = true;
					completerep = true;
					for(int i = 0; i < data->num_colls; i++) {
						data->completecolls[i] = true;
					}
				}
			} else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				tmpsdisplsintra = (int *)_real_malloc((commnC)*sizeof(int));
				tmprdisplsintra = (int *)_real_malloc((commnC)*sizeof(int));
				if(data->id > cmpid && cmpid >= 0) {
					int snumcounts = 0;
					int rnumcounts = 0;
					for(int i = 0; i < commnC; i++) {
						tmpsdisplsintra[i] = snumcounts;
						tmprdisplsintra[i] = rnumcounts;
						snumcounts += (data->args).alltoallv.sendcounts[i];
						rnumcounts += (data->args).alltoallv.recvcounts[i];
					}
					retVal = EMPI_Ialltoallv ((data->args).alltoallv.sendbuf, (data->args).alltoallv.sendcounts, tmpsdisplsintra, (data->args).alltoallv.senddt->edatatype, recv_dest, (data->args).alltoallv.recvcounts,tmprdisplsintra, (data->args).alltoallv.recvdt->edatatype, ((data->args).alltoallv.comm)->EMPI_COMM_CMP, ((*(data->req))->reqcmp));
				} else completecmp = true;
				if(data->id > repid && repid >= 0) {
					if(commnR > 0 && cmpToRepMap[myrank] == -1) {
						tmpscountsinter = (int *)_real_malloc((commnR)*sizeof(int));
						tmpsdisplsinter = (int *)_real_malloc((commnR)*sizeof(int));
						intermapping = (int *)_real_malloc((commnC-commnR)*sizeof(int));
						int j = 0;
						int k = 0;
						int snumcounts = 0;
						for(int i = 0; i < commnC; i++) {
							if(cmpToRepMap[i] != -1) {
								tmpscountsinter[j] = (data->args).alltoallv.sendcounts[i];
								//tmpsdisplsinter[j] = (data->args).alltoallv.sdispls[i];
								tmpsdisplsinter[j] = snumcounts;
								j++;
							} else {
								intermapping[k] = i;
								k++;
							}
							snumcounts += (data->args).alltoallv.sendcounts[i];
						}
						
						if(data->num_colls > 0) parep_mpi_free(data->completecolls);
						if(no_req_exists) {
							(*(data->req))->reqcolls = (EMPI_Request **)parep_mpi_malloc(sizeof(EMPI_Request *) * (commnC-commnR));
							(*(data->req))->num_reqcolls = (commnC-commnR);
						} else {
							for(int i = 0; i < (*(data->req))->num_reqcolls; i++) parep_mpi_free(((*(data->req))->reqcolls[i]));
							(*(data->req))->reqcolls = (EMPI_Request **)parep_mpi_realloc((*(data->req))->reqcolls,sizeof(EMPI_Request *) * (commnC-commnR));
							(*(data->req))->num_reqcolls = (commnC-commnR);
						}
						for(int i = 0; i < (*(data->req))->num_reqcolls; i++) *((*(data->req))->reqcolls[i]) = EMPI_REQUEST_NULL;
						data->completecolls = (bool *)parep_mpi_malloc(sizeof(bool) * (commnC-commnR));
						data->num_colls = (commnC-commnR);
						
						EMPI_Comm_rank(((data->args).alltoallv.comm->EMPI_CMP_NO_REP),&noreprank);
						for(int i = 0; i < commnC-commnR; i++) {
							((*(data->req))->reqcolls[i]) = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
							data->completecolls[i] = false;
							if(noreprank == i) retVal = EMPI_Iscatterv ((data->args).alltoallv.sendbuf, tmpscountsinter, tmpsdisplsinter, (data->args).alltoallv.senddt->edatatype, recv_dest, (data->args).alltoallv.recvcounts[intermapping[i]], (data->args).alltoallv.recvdt->edatatype, EMPI_ROOT, ((data->args).alltoallv.comm->EMPI_CMP_NO_REP_INTERCOMM), ((*(data->req))->reqcolls[i]));
							else retVal = EMPI_Iscatterv ((data->args).alltoallv.sendbuf, tmpscountsinter, tmpsdisplsinter, (data->args).alltoallv.senddt->edatatype, recv_dest, (data->args).alltoallv.recvcounts[intermapping[i]], (data->args).alltoallv.recvdt->edatatype, EMPI_PROC_NULL, ((data->args).alltoallv.comm->EMPI_CMP_NO_REP_INTERCOMM), ((*(data->req))->reqcolls[i]));
						}
					} else {
						if(data->num_colls > 0) parep_mpi_free(data->completecolls);
						if(no_req_exists) {
							(*(data->req))->reqcolls = NULL;
							(*(data->req))->num_reqcolls = 0;
						} else {
							for(int i = 0; i < (*(data->req))->num_reqcolls; i++) parep_mpi_free(((*(data->req))->reqcolls[i]));
							if((*(data->req))->num_reqcolls > 0) parep_mpi_free((*(data->req))->reqcolls);
							(*(data->req))->reqcolls = NULL;
							(*(data->req))->num_reqcolls = 0;
						}
						data->completecolls = NULL;
						data->num_colls = 0;
					}
					completerep = true;
					
					if (commnR > 0 && cmpToRepMap[myrank] == -1) {
						_real_free(tmpscountsinter);
						_real_free(tmpsdisplsinter);
						_real_free(intermapping);
					}
				} else {
					completerep = true;
					for(int i = 0; i < data->num_colls; i++) {
						data->completecolls[i] = true;
					}
				}
				_real_free(tmpsdisplsintra);
				_real_free(tmprdisplsintra);
			}
		} else if(data->type == MPI_FT_REDUCE) {
			data->num_colls = 0;
			data->completecolls = NULL;
			targ_comm = (data->args).reduce.comm;
			bool cmpstate = data->completecmp;
			bool repstate = data->completerep;
			int size;
			EMPI_Type_size((data->args).reduce.dt->edatatype,&size);
			if(no_req_exists) {
				recv_dest = _real_malloc((data->args).reduce.count * size);
			} else {
				recv_dest = (*(data->req))->bufloc;
			}
			int extracount,dis;
			if(size >= sizeof(int)) extracount = 1;
			else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
			else extracount = (((int)sizeof(int))/size) + 1;
			dis = (((data->args).reduce.count+extracount)*size) - sizeof(int);
			if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				bool completed_in_all_cmps = true;
				bool completed_in_my_rep;
				if(cmpToRepMap[myrank] == -1) completed_in_my_rep = true;
				else if((data->id <= collids[cmpToRepMap[myrank] + nC]) || (myrank != (data->args).reduce.root)) completed_in_my_rep = true;
				else completed_in_my_rep = false;
				int index = 0;
				for(int i = 0; i < nC+nR; i++) {
					if(i < nC) {
						if(data->id > collids[i]) {
							completed_in_all_cmps = false;
							for(int j = 0; j < redids[i]; j++) {
								if(reddataids[index+j] == data->id) {
									completed_in_all_cmps = true;
									break;
								}
							}
						}
					} else if((cmpToRepMap[myrank] != -1) && (i == cmpToRepMap[myrank] + nC) && (myrank == (data->args).reduce.root)) {
						for(int j = 0; j < redids[i]; j++) {
							if(reddataids[index+j] == data->id) {
								completed_in_my_rep = true;
								break;
							}
						}
					}
					index = index + redids[i];
				}
				if(data->id > cmpid && !completed_in_all_cmps/* && cmpid >= 0*/) {
					retVal = EMPI_Ireduce ((data->args).reduce.sendbuf, recv_dest, (data->args).reduce.count, (data->args).reduce.dt->edatatype, ((data->args).reduce.op)->eop, (data->args).reduce.root, ((data->args).reduce.comm)->EMPI_COMM_CMP, ((*(data->req))->reqcmp));
				} else completecmp = true;
				if(data->id > myrepid/* && myrepid >= 0*/ && cmpToRepMap[myrank] != -1 && myrank == (data->args).reduce.root && cmpstate && !completed_in_my_rep) {
					retVal = EMPI_Isend ((data->args).reduce.recvbuf, (data->args).reduce.count+extracount, (data->args).reduce.dt->edatatype, cmpToRepMap[myrank], MPI_FT_REDUCE_TAG, ((data->args).reduce.comm)->EMPI_CMP_REP_INTERCOMM, ((*(data->req))->reqrep));
				} else if((cmpToRepMap[myrank] == -1) || (data->id <= myrepid) || (repstate) || (myrank != (data->args).reduce.root) || completed_in_my_rep) completerep = true;
			} else if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				completecmp = true;
				if(!repstate) retVal = EMPI_Irecv ((data->args).reduce.recvbuf, (data->args).reduce.count+extracount, (data->args).reduce.dt->edatatype, myrank, MPI_FT_REDUCE_TAG, ((data->args).reduce.comm)->EMPI_CMP_REP_INTERCOMM, ((*(data->req))->reqrep));
				else completerep = true;
			}
		}
		else if(data->type == MPI_FT_ALLREDUCE) {
			data->num_colls = 0;
			data->completecolls = NULL;
			targ_comm = (data->args).allreduce.comm;
			bool cmpstate = data->completecmp;
			bool repstate = data->completerep;
			int size;
			EMPI_Type_size((data->args).allreduce.dt->edatatype,&size);
			if(no_req_exists) {
				recv_dest = _real_malloc((data->args).allreduce.count * size);
			} else {
				recv_dest = (*(data->req))->bufloc;
			}
			int extracount,dis;
			if(size >= sizeof(int)) extracount = 1;
			else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
			else extracount = (((int)sizeof(int))/size) + 1;
			dis = (((data->args).allreduce.count+extracount)*size) - sizeof(int);
			if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
				bool completed_in_all_cmps = true;
				bool completed_in_my_rep;
				if(cmpToRepMap[myrank] == -1) completed_in_my_rep = true;
				else if(data->id <= collids[cmpToRepMap[myrank] + nC]) completed_in_my_rep = true;
				else completed_in_my_rep = false;
				int index = 0;
				for(int i = 0; i < nC+nR; i++) {
					if(i < nC) {
						if(data->id > collids[i]) {
							completed_in_all_cmps = false;
							for(int j = 0; j < redids[i]; j++) {
								if(reddataids[index+j] == data->id) {
									completed_in_all_cmps = true;
									break;
								}
							}
						}
					} else if((cmpToRepMap[myrank] != -1) && (i == cmpToRepMap[myrank] + nC)) {
						for(int j = 0; j < redids[i]; j++) {
							if(reddataids[index+j] == data->id) {
								completed_in_my_rep = true;
								break;
							}
						}
					}
					index = index + redids[i];
				}
				if(data->id > cmpid && !completed_in_all_cmps/* && cmpid >= 0*/) {
					retVal = EMPI_Iallreduce ((data->args).allreduce.sendbuf, recv_dest, (data->args).allreduce.count, (data->args).allreduce.dt->edatatype, ((data->args).allreduce.op)->eop, ((data->args).allreduce.comm)->EMPI_COMM_CMP, ((*(data->req))->reqcmp));
				} else completecmp = true;
				if(data->id > myrepid/* && myrepid >= 0 */&& cmpToRepMap[myrank] != -1 && cmpstate && !completed_in_my_rep) {
					retVal = EMPI_Isend ((data->args).allreduce.recvbuf, (data->args).allreduce.count+extracount, (data->args).allreduce.dt->edatatype, cmpToRepMap[myrank], MPI_FT_ALLREDUCE_TAG, ((data->args).allreduce.comm)->EMPI_CMP_REP_INTERCOMM, ((*(data->req))->reqrep));
				} else if((cmpToRepMap[myrank] == -1) || (data->id <= myrepid) || (repstate) || completed_in_my_rep) completerep = true;
			} else if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) {
				completecmp = true;
				if(!repstate) retVal = EMPI_Irecv ((data->args).allreduce.recvbuf, (data->args).allreduce.count+extracount, (data->args).allreduce.dt->edatatype, myrank, MPI_FT_ALLREDUCE_TAG, ((data->args).allreduce.comm)->EMPI_CMP_REP_INTERCOMM, ((*(data->req))->reqrep));
				else completerep = true;
			}
		}
		
		if(no_req_exists) {
			bool complete = false;
			while(!complete) {
				if(!(completecmp)) {
					int flag;
					EMPI_Test((*(data->req))->reqcmp,&flag,&(((*(data->req))->status).status));
					completecmp = flag > 0;
				}
				if(!(completerep)) {
					int flag;
					if(*((*(data->req))->reqrep) != EMPI_REQUEST_NULL) EMPI_Test((*(data->req))->reqrep,&flag,&(((*(data->req))->status).status));
					else flag = 0;
					completerep = flag > 0;
				}
				for(int i = 0; i < data->num_colls; i++) {
					if(!(data->completecolls[i])) {
						int flag;
						EMPI_Test((*(data->req))->reqcolls[i],&flag,&(((*(data->req))->status).status));
						data->completecolls[i] = flag > 0;
					}
				}
				complete = (completecmp) & (completerep);
				for(int i = 0; i < data->num_colls; i++) {
					complete = complete & data->completecolls[i];
				}
			}
			
			if(recv_dest != NULL) _real_free(recv_dest);
			parep_mpi_free((*(data->req))->reqcmp);
			parep_mpi_free((*(data->req))->reqrep);
			for(int i = 0; i < (*(data->req))->num_reqcolls; i++) {
				parep_mpi_free((*(data->req))->reqcolls[i]);
			}
			if((*(data->req))->num_reqcolls > 0) parep_mpi_free((*(data->req))->reqcolls);
			parep_mpi_free(*(data->req));
			*(data->req) = MPI_REQUEST_NULL;
			parep_mpi_free(data->req);
			data->req = NULL;
		} else {
			data->completecmp = completecmp;
			data->completerep = completerep;
			(*(data->req))->status.MPI_ERROR = 0;
			(*(data->req))->complete = false;
			(*(data->req))->comm = targ_comm;
			(*(data->req))->type = MPI_FT_COLLECTIVE_REQUEST;
			(*(data->req))->storeloc = data;
		}
		
		if(send_src != NULL) _real_free(send_src);
		
		data = data->prev;
	}
	if(redids != NULL) _real_free(redids);
	if(reddataids != NULL) _real_free(reddataids);
	if(collids != NULL) _real_free(collids);
}

/*bool IsRecvComplete(int src, int id) {
  if(ondisk) fseek(logfile, 0, SEEK_SET);
  bool rcomp = false;
  char *mode;
  char *line = NULL;
  size_t llen = 0;
  ssize_t read;
  mode = (char*) malloc(50 * sizeof(char));
  if(ondisk)
  {
    while ((read = getline(&line, &llen, logfile)) != -1)
    {
      GetFirstWord(line,mode);
      if(strcmp (mode,"RECV") == 0)
      {
        int logsrc, logid;
        GetSrcAndId(line, &logid, &logsrc);
        if((logid == id) && (logsrc == src)) rcomp = true;
      }
    }
  }
  for(int i=0; i<nbufentries;i++)
  {
    GetFirstWord(logbuffer[i],mode);
    if(strcmp (mode,"RECV") == 0)
    {
      int logsrc, logid;
      GetSrcAndId(logbuffer[i], &logid, &logsrc);
      if((logid == id) && (logsrc == src)) rcomp = true;
    }
  }
  return rcomp;
}

void GetSrcAndId(char *line, int *id, int *src) {
  int i;
  int wordnum = 0;
  char *word;
  word = (char*) malloc(100 * sizeof(char));
  int wordlen = 0;
  for (i = 0; i <  strlen(line); i++)
  {
    if (line[i] == ' ' || line[i] == '\n' || line[i] == '\0')
    {
      word[wordlen] = '\0';
      wordlen++;
      if(wordnum == 1)
      {
        *id = atoi(word);
      }
      else if(wordnum == 2)
      {
        *src = atoi(word);
      }
      wordlen = 0;
      wordnum++;
    }
    else
    {
      word[wordlen] = line[i];
      wordlen++;
    }
  }
}

void GetDestAndId(char *line, int *id, int *dest) {
  int i;
  int wordnum = 0;
  char *word;
  word = (char*) malloc(100 * sizeof(char));
  int wordlen = 0;
  for (i = 0; i <  strlen(line); i++)
  {
    if (line[i] == ' ' || line[i] == '\n' || line[i] == '\0')
    {
      word[wordlen] = '\0';
      wordlen++;
      if(wordnum == 1)
      {
        *id = atoi(word);
      }
      else if(wordnum == 4)
      {
        *dest = atoi(word);
      }
      wordlen = 0;
      wordnum++;
    }
    else
    {
      word[wordlen] = line[i];
      wordlen++;
    }
  }
}*/

int MPI_Send (void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}

	MPI_Request req;
	retVal = MPI_Isend(buf,count,datatype,dest,tag,comm,&req);
	retVal = MPI_Wait(&req,MPI_STATUS_IGNORE);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return retVal;
}

int MPI_Isend (void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *req) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	MPI_Request ftreq;
	int myrank;
	void *sendbuf;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	parep_mpi_sighandling_state = 1;
	ftreq = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
	(ftreq)->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	ftreq->reqcolls = NULL;
	ftreq->num_reqcolls = 0;
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	ftreq->complete = false;
	ftreq->status.count = count;
	ftreq->status.MPI_SOURCE = dest;
	ftreq->status.MPI_TAG = tag;
	ftreq->status.MPI_ERROR = 0;
	ftreq->type = MPI_FT_SEND_REQUEST;
	ftreq->bufloc = buf;
	int extracount;
	int dis;
	int size;
	ptpdata *curargs;
	curargs = (ptpdata *)parep_mpi_malloc(sizeof(ptpdata));
	curargs->markdelcmp = false;
	curargs->markdelrep = false;
	EMPI_Type_size(datatype->edatatype,&size);
	if(size >= sizeof(int)) extracount = 1;
	else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
	else extracount = (((int)sizeof(int))/size) + 1;
	dis = ((count+extracount)*size) - sizeof(int);
	
	pthread_mutex_lock(&parep_mpi_store_buf_sz_mutex);
	if(parep_mpi_store_buf_sz + ((count+extracount)*size) >= 0x40000000) {
		pthread_mutex_lock(&rem_recv_msg_sent_mutex);
		if(rem_recv_msg_sent == 0) {
			int dyn_sock;
			int ret;
			dyn_sock = socket(AF_INET, SOCK_STREAM, 0);
			do {
				ret = connect(dyn_sock,(struct sockaddr *)(&parep_mpi_dyn_coordinator_addr),sizeof(parep_mpi_dyn_coordinator_addr));
			} while(ret != 0);
			int cmd = CMD_REM_RECV;
			write(dyn_sock,&cmd,sizeof(int));
			close(dyn_sock);
			rem_recv_msg_sent = 1;
		}
		pthread_mutex_unlock(&rem_recv_msg_sent_mutex);
	}
	while(parep_mpi_store_buf_sz + ((count+extracount)*size) >= 0x80000000) {
		pthread_mutex_unlock(&parep_mpi_store_buf_sz_mutex);
		pthread_mutex_lock(&reqListLock);
		pthread_rwlock_rdlock(&commLock);
		for(int source = 0; source < nC; source++) {
			if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
				if(cmpToRepMap[source] == -1) probe_msg_from_source(comm, source);
				else probe_msg_from_source(comm, cmpToRepMap[source]+nC);
			} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
				probe_msg_from_source(comm, source);
			}
		}
		probe_reduce_messages_with_comm(comm);
		pthread_rwlock_unlock(&commLock);
		pthread_mutex_unlock(&reqListLock);
		pthread_mutex_lock(&parep_mpi_store_buf_sz_mutex);
		//pthread_cond_wait(&parep_mpi_store_buf_sz_cond,&parep_mpi_store_buf_sz_mutex);
	}
	parep_mpi_store_buf_sz = parep_mpi_store_buf_sz + ((count+extracount)*size);
	curargs->buf = parep_mpi_malloc((count+extracount)*size);
	pthread_mutex_unlock(&parep_mpi_store_buf_sz_mutex);
	memcpy(curargs->buf,buf,count*size);
	memcpy((curargs->buf) + dis,&parep_mpi_sendid,sizeof(int));
	bool skiprep, skipcmp;
	struct skiplist *skip,*oldskip;
	
	pthread_mutex_lock(&reqListLock);
	skiprep = false;
	skipcmp = false;
	skip = skipreplist;
	oldskip = NULL;
	while(skip != NULL) {
		if(skip->id == parep_mpi_sendid) {
			skiprep = true;
			if(oldskip != NULL) oldskip->next = skip->next;
			else skipreplist = skip->next;
			parep_mpi_free(skip);
			break;
		}
		oldskip = skip;
		skip = skip->next;
	}
	skip = skipcmplist;
	oldskip = NULL;
	while(skip != NULL) {
		if(skip->id == parep_mpi_sendid) {
			skipcmp = true;
			if(oldskip != NULL) oldskip->next = skip->next;
			else skipcmplist = skip->next;
			parep_mpi_free(skip);
			break;
		}
		oldskip = skip;
		skip = skip->next;
	}
	
	curargs->completecmp = false;
	curargs->completerep = false;
	
	//commbuf_node_t *cbuf = get_commbuf_node((count+extracount)*size);
	//sendbuf = cbuf->commbuf;
	//memcpy(sendbuf,curargs->buf,(count+extracount)*size);
	//ftreq->cbuf = cbuf;
	sendbuf = curargs->buf;
	
	pthread_rwlock_rdlock(&commLock);
	ftreq->comm = comm;
	
	if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
		if(cmpToRepMap[dest] == -1) {
			curargs->completecmp = true;
			curargs->completerep = true;
		} else {
			curargs->completecmp = true;
			if(!skiprep) retVal = EMPI_Isend(sendbuf, count+extracount, datatype->edatatype, cmpToRepMap[dest], tag, (comm->EMPI_COMM_REP), ((ftreq)->reqrep));
			else curargs->completerep = true;
		}
	} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
		if(!skipcmp) retVal = EMPI_Isend (sendbuf, count+extracount, datatype->edatatype, dest, tag, (comm->EMPI_COMM_CMP), ((ftreq)->reqcmp));
		else curargs->completecmp = true;
		if((cmpToRepMap[dest] != -1) && (cmpToRepMap[myrank] == -1) && (!skiprep)) retVal = EMPI_Isend (sendbuf, count+extracount, datatype->edatatype, cmpToRepMap[dest], tag, (comm->EMPI_CMP_REP_INTERCOMM), ((ftreq)->reqrep));
		else curargs->completerep = true;
	}
	pthread_rwlock_unlock(&commLock);
	
	curargs->type = MPI_FT_SEND;
	curargs->count = count;
	curargs->dt = datatype;
	curargs->target = dest;
	curargs->tag = tag;
	curargs->comm = comm;
	curargs->req = req;
	curargs->id = parep_mpi_sendid;
	parep_mpi_sendid++;
	
	pthread_mutex_lock(&peertopeerLock);
	if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
	else first_peertopeer = curargs;
	curargs->prev = NULL;
	curargs->next = last_peertopeer;
	last_peertopeer = curargs;
	pthread_cond_signal(&peertopeerCond);
	pthread_mutex_unlock(&peertopeerLock);
	
	(ftreq)->storeloc = curargs;
	*req = ftreq;
	(*req)->rnode = reqListInsert(*req);
	
	pthread_cond_signal(&reqListCond);
	pthread_mutex_unlock(&reqListLock);
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return MPI_SUCCESS;
}

int MPI_Recv (void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *stat) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}

	MPI_Request req;
	retVal = MPI_Irecv(buf,count,datatype,source,tag,comm,&req);
	retVal = MPI_Wait(&req,stat);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return retVal;
}

int MPI_Irecv (void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *req) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	MPI_Request ftreq;
	int myrank;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	parep_mpi_sighandling_state = 1;
	ftreq = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
	
	ftreq->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	ftreq->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	ftreq->reqcolls = NULL;
	ftreq->num_reqcolls = 0;
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	ftreq->complete = false;
	ftreq->status.count = count;
	ftreq->status.MPI_SOURCE = source;
	ftreq->status.MPI_TAG = tag;
	ftreq->status.MPI_ERROR = 0;
	ftreq->type = MPI_FT_RECV_REQUEST;
	ftreq->bufloc = buf;
	
	pthread_mutex_lock(&reqListLock);
	/*pthread_rwlock_rdlock(&commLock);
	retVal = 0;
	do {
		if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
			if(cmpToRepMap[source] == -1) retVal = probe_msg_from_source(comm, source);
			else retVal = probe_msg_from_source(comm, cmpToRepMap[source]+nC);
		} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
			retVal = probe_msg_from_source(comm, source);
		}
	} while(retVal != 0);
	pthread_rwlock_unlock(&commLock);*/
	
	pthread_mutex_lock(&recvDataListLock);
	recvDataNode *rnode = recvDataListFind(source,tag,comm);
	
	ptpdata *curargs;
	int size;
	EMPI_Type_size(datatype->edatatype,&size);
	if(rnode != NULL) {
		curargs = rnode->pdata;
		recvDataListDelete(rnode);
		pthread_cond_signal(&recvDataListCond);
		pthread_mutex_unlock(&recvDataListLock);
		
		int recvcount = curargs->count;
		recvcount = recvcount/size;
		
		memcpy(ftreq->bufloc,curargs->buf,recvcount*size);
		ftreq->status.count = recvcount;
		ftreq->status.MPI_TAG = curargs->tag;
		ftreq->status.MPI_SOURCE = curargs->target;
		ftreq->status.MPI_ERROR = 0;
		
		pthread_mutex_lock(&peertopeerLock);
		parep_mpi_free(curargs->buf);
		curargs->count = recvcount;
		curargs->dt = datatype;
		parep_mpi_free(curargs->req);
		curargs->req = req;
		pthread_mutex_unlock(&peertopeerLock);
	} else {
		pthread_mutex_unlock(&recvDataListLock);
		int extracount;
		int dis;
		curargs = (ptpdata *)parep_mpi_malloc(sizeof(ptpdata));
		curargs->markdelcmp = false;
		curargs->markdelrep = false;
		dis = count*size;
		if(size >= sizeof(int)) extracount = 1;
		else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
		else extracount = (((int)sizeof(int))/size) + 1;
		curargs->buf = parep_mpi_malloc((count+extracount)*size);
		
		curargs->completecmp = false;
		curargs->completerep = false;
		
		pthread_rwlock_rdlock(&commLock);
		ftreq->comm = comm;
		if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
			curargs->completecmp = true;
			if(cmpToRepMap[source] == -1) retVal = EMPI_Irecv (curargs->buf, count+extracount, datatype->edatatype, source, tag, (comm->EMPI_CMP_REP_INTERCOMM), ((ftreq)->reqrep));
			else retVal = EMPI_Irecv (curargs->buf, count+extracount, datatype->edatatype, cmpToRepMap[source], tag, (comm->EMPI_COMM_REP), ((ftreq)->reqrep));
		} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
			retVal = EMPI_Irecv (curargs->buf, count+extracount, datatype->edatatype, source, tag, (comm->EMPI_COMM_CMP), ((ftreq)->reqcmp));
			curargs->completerep = true;
		}
		pthread_rwlock_unlock(&commLock);
		
		curargs->type = MPI_FT_RECV;
		curargs->count = count;
		curargs->dt = datatype;
		curargs->target = source;
		curargs->tag = tag;
		curargs->comm = comm;
		curargs->req = req;
		
		pthread_mutex_lock(&peertopeerLock);
		if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
		else first_peertopeer = curargs;
		curargs->prev = NULL;
		curargs->next = last_peertopeer;
		last_peertopeer = curargs;
		pthread_cond_signal(&peertopeerCond);
		pthread_mutex_unlock(&peertopeerLock);
	}
	
	(ftreq)->storeloc = curargs;
	*req = ftreq;
	(*req)->rnode = reqListInsert(*req);
	pthread_cond_signal(&reqListCond);	
	pthread_mutex_unlock(&reqListLock);
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return MPI_SUCCESS;
}

int MPI_Request_free(MPI_Request *req) {
	bool int_call;
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	MPI_Request ftreq = *req;
	parep_mpi_sighandling_state = 1;
	if(ftreq == MPI_REQUEST_NULL) return MPI_SUCCESS;
	else {
		pthread_mutex_lock(&reqListLock);
		reqListDelete(ftreq->rnode);
		pthread_mutex_unlock(&reqListLock);
		if(*((ftreq)->reqcmp) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqcmp));
		if(*((ftreq)->reqrep) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqrep));
		parep_mpi_free(ftreq->reqcmp);
		parep_mpi_free(ftreq->reqrep);
		if(ftreq->type == MPI_FT_COLLECTIVE_REQUEST) {
			for(int i = 0; i < ftreq->num_reqcolls; i++) {
				if(*((ftreq)->reqcolls[i]) != EMPI_REQUEST_NULL) EMPI_Request_free(ftreq->reqcolls[i]);
				parep_mpi_free(ftreq->reqcolls[i]);
			}
			if(ftreq->num_reqcolls > 0) parep_mpi_free(ftreq->reqcolls);
			pthread_mutex_lock(&collectiveLock);
			*(((clcdata *)(ftreq->storeloc))->req) = MPI_REQUEST_NULL;
			(((clcdata *)(ftreq->storeloc))->req) = NULL;
			pthread_mutex_unlock(&collectiveLock);
		} else {
			pthread_mutex_lock(&peertopeerLock);
			*(((ptpdata *)(ftreq->storeloc))->req) = MPI_REQUEST_NULL;
			(((ptpdata *)(ftreq->storeloc))->req) = NULL;
			pthread_mutex_unlock(&peertopeerLock);
		}
		
		/*if(ftreq->type == MPI_FT_SEND_REQUEST) {
			return_commbuf_node(ftreq->cbuf);
		}*/
		
		parep_mpi_free(ftreq);
		ftreq = MPI_REQUEST_NULL;
		*req = ftreq;
	}
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	return MPI_SUCCESS;
}

int MPI_Test (MPI_Request *req, int *flag, MPI_Status *status) {
	bool int_call;
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	MPI_Request ftreq = *req;
	parep_mpi_sighandling_state = 1;
	pthread_mutex_lock(&reqListLock);
	test_all_requests();
	*flag = ftreq->complete;
	if(*flag) {
		if(status != MPI_STATUS_IGNORE) {
			*status = (ftreq)->status;
		}
		reqListDelete(ftreq->rnode);
		if(*((ftreq)->reqcmp) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqcmp));
		if(*((ftreq)->reqrep) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqrep));
		parep_mpi_free(ftreq->reqcmp);
		parep_mpi_free(ftreq->reqrep);
		if(ftreq->type == MPI_FT_COLLECTIVE_REQUEST) {
			for(int i = 0; i < ftreq->num_reqcolls; i++) {
				if(*((ftreq)->reqcolls[i]) != EMPI_REQUEST_NULL) EMPI_Request_free(ftreq->reqcolls[i]);
				parep_mpi_free(ftreq->reqcolls[i]);
			}
			if(ftreq->num_reqcolls > 0) parep_mpi_free(ftreq->reqcolls);
			pthread_mutex_lock(&collectiveLock);
			*(((clcdata *)(ftreq->storeloc))->req) = MPI_REQUEST_NULL;
			(((clcdata *)(ftreq->storeloc))->req) = NULL;
			pthread_mutex_unlock(&collectiveLock);
		} else {
			pthread_mutex_lock(&peertopeerLock);
			*(((ptpdata *)(ftreq->storeloc))->req) = MPI_REQUEST_NULL;
			(((ptpdata *)(ftreq->storeloc))->req) = NULL;
			pthread_mutex_unlock(&peertopeerLock);
		}
		
		/*if(ftreq->type == MPI_FT_SEND_REQUEST) {
			return_commbuf_node(ftreq->cbuf);
		}*/
		
		parep_mpi_free(ftreq);
		ftreq = MPI_REQUEST_NULL;
		*req = ftreq;
	}
	pthread_mutex_unlock(&reqListLock);
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	return MPI_SUCCESS;
}

int MPI_Wait (MPI_Request *req, MPI_Status *status) {
	bool int_call;
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	MPI_Request ftreq = *req;
	//struct timespec waittime;
	//waittime.tv_sec = 1;
	//waittime.tv_nsec = 0;
	parep_mpi_sighandling_state = 3;
	pthread_mutex_lock(&reqListLock);
	while(!(ftreq->complete)) {
		if(parep_mpi_wait_block == 1) {
			pthread_mutex_unlock(&reqListLock);
			pthread_mutex_lock(&reqListLock);
		}
		if(parep_mpi_sighandling_state == 2) {
			parep_mpi_sighandling_state = 0;
			//end_wake_thread = 1;
			pthread_mutex_unlock(&reqListLock);
			//pthread_join(waitWaker,NULL);
			parep_mpi_ckpt_wait = 1;
			pthread_kill(pthread_self(),SIGUSR1);
			while(parep_mpi_ckpt_wait) {;}
			pthread_mutex_lock(&reqListLock);
			parep_mpi_sighandling_state = 3;
		}
		test_all_requests();
		//pthread_cond_wait(&reqListCond,&reqListLock);
		//pthread_cond_timedwait(&reqListCond,&reqListLock,&waittime);
	}
	if(parep_mpi_sighandling_state == 3) parep_mpi_sighandling_state = 1;
	/*else if(parep_mpi_sighandling_state == 2) {
		end_wake_thread = 1;
		pthread_mutex_unlock(&reqListLock);
		pthread_join(waitWaker,NULL);
		pthread_mutex_lock(&reqListLock);
	}*/
	if(status != MPI_STATUS_IGNORE) {
		*status = (ftreq)->status;
	}
	reqListDelete(ftreq->rnode);
	if(*((ftreq)->reqcmp) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqcmp));
	if(*((ftreq)->reqrep) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqrep));
	parep_mpi_free(ftreq->reqcmp);
	parep_mpi_free(ftreq->reqrep);
	if(ftreq->type == MPI_FT_COLLECTIVE_REQUEST) {
		for(int i = 0; i < ftreq->num_reqcolls; i++) {
			if(*((ftreq)->reqcolls[i]) != EMPI_REQUEST_NULL) EMPI_Request_free(ftreq->reqcolls[i]);
			parep_mpi_free(ftreq->reqcolls[i]);
		}
		if(ftreq->num_reqcolls > 0) parep_mpi_free(ftreq->reqcolls);
		pthread_mutex_lock(&collectiveLock);
		*(((clcdata *)(ftreq->storeloc))->req) = MPI_REQUEST_NULL;
		(((clcdata *)(ftreq->storeloc))->req) = NULL;
		pthread_mutex_unlock(&collectiveLock);
	} else {
		pthread_mutex_lock(&peertopeerLock);
		*(((ptpdata *)(ftreq->storeloc))->req) = MPI_REQUEST_NULL;
		(((ptpdata *)(ftreq->storeloc))->req) = NULL;
		pthread_mutex_unlock(&peertopeerLock);
	}
	
	/*if(ftreq->type == MPI_FT_SEND_REQUEST) {
		return_commbuf_node(ftreq->cbuf);
	}*/
	
	parep_mpi_free(ftreq);
	ftreq = MPI_REQUEST_NULL;
	*req = ftreq;
	pthread_mutex_unlock(&reqListLock);
	
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	return MPI_SUCCESS;
}

int closest_replica(int cmpRank)
{
  if(nR <= 0) return -1;
  if(cmpToRepMap[cmpRank] != -1) return cmpToRepMap[cmpRank];
  else
  {
    for(int i = (cmpRank+1); i != nC; i++)
    {
      if(cmpToRepMap[i] == 0) return 0;
      else if(cmpToRepMap[i] > 0)
      {
        int fdist = i - cmpRank;
        for(int j = (cmpRank-1); j != 0; j--)
        {
          if(cmpRank - j >= fdist) return cmpToRepMap[i];
          else if(cmpToRepMap[j] >= 0) return cmpToRepMap[j];
        }
      }
    }
    
    return nR-1;
  }
}

int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}

	MPI_Request req;
	retVal = MPI_Ibcast(buffer,count,datatype,root,comm,&req);
	retVal = MPI_Wait(&req,MPI_STATUS_IGNORE);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return retVal;
}

int MPI_Ibcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm, MPI_Request *req) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	MPI_Request ftreq;
	int myrank;
	int size;
	struct collective_data *curargs;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	parep_mpi_sighandling_state = 1;
	ftreq = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
	(ftreq)->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqcolls = NULL;
	(ftreq)->num_reqcolls = 0;
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	ftreq->bufloc = buffer;
	ftreq->complete = false;
	ftreq->status.count = count;
	ftreq->status.MPI_SOURCE = root;
	ftreq->status.MPI_ERROR = 0;
	ftreq->type = MPI_FT_COLLECTIVE_REQUEST;
	
	bool completecmp = false;
	bool completerep = false;
	
	pthread_mutex_lock(&reqListLock);
	
	pthread_rwlock_rdlock(&commLock);
	ftreq->comm = comm;
	
	if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
		completecmp = true;
		if(cmpToRepMap[root] == -1) EMPI_Ibcast (buffer, count, datatype->edatatype, root, (comm->EMPI_CMP_REP_INTERCOMM), (ftreq)->reqrep);
		else retVal = EMPI_Ibcast (buffer, count, datatype->edatatype, cmpToRepMap[root], (comm->EMPI_COMM_REP), (ftreq)->reqrep);
	} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
		retVal = EMPI_Ibcast (buffer, count, datatype->edatatype, root, (comm->EMPI_COMM_CMP), (ftreq)->reqcmp);
		if(cmpToRepMap[root] == -1 && myrank == root && nR > 0) retVal = EMPI_Ibcast (buffer, count, datatype->edatatype, EMPI_ROOT, (comm->EMPI_CMP_REP_INTERCOMM), (ftreq)->reqrep);
		else if(cmpToRepMap[root] == -1 && nR > 0) retVal = EMPI_Ibcast (buffer, count, datatype->edatatype, EMPI_PROC_NULL, (comm->EMPI_CMP_REP_INTERCOMM), (ftreq)->reqrep);
		else completerep = true;
	}
	pthread_rwlock_unlock(&commLock);
	
	EMPI_Type_size(datatype->edatatype,&size);
	curargs = (struct collective_data *)parep_mpi_malloc(sizeof(struct collective_data));
	if(myrank == root) {
		(curargs->args).bcast.buf = parep_mpi_malloc(count*size);
		memcpy((curargs->args).bcast.buf,buffer,count*size);
	} else (curargs->args).bcast.buf = NULL;
	(curargs->args).bcast.count = count;
	(curargs->args).bcast.dt = datatype;
	(curargs->args).bcast.root = root;
	(curargs->args).bcast.comm = comm;
	curargs->req = req;
	curargs->type = MPI_FT_BCAST;
	curargs->completecmp = completecmp;
	curargs->completerep = completerep;
	curargs->completecolls = NULL;
	curargs->num_colls = 0;
	
	pthread_mutex_lock(&collectiveLock);
	curargs->id = parep_mpi_collective_id;
	if(last_collective == NULL) {
		curargs->prev = NULL;
		curargs->next = NULL;
		last_collective = curargs;
	} else {
		clcdata *temp = last_collective;
		while((curargs->id <= temp->id) && (temp->next != NULL)) {
			temp = temp->next;
		}
		if(curargs->id > temp->id) {
			curargs->prev = temp->prev;
			curargs->next = temp;
			if(temp->prev != NULL) temp->prev->next = curargs;
			else last_collective = curargs;
			temp->prev = curargs;
		}
		else {
			curargs->prev = temp;
			curargs->next = NULL;
			temp->next = curargs;
		}
	}
	parep_mpi_collective_id++;
	pthread_cond_signal(&collectiveCond);
	pthread_mutex_unlock(&collectiveLock);
	
	(ftreq)->storeloc = curargs;
	*req = ftreq;
	(*req)->rnode = reqListInsert(*req);
	
	pthread_cond_signal(&reqListCond);
	pthread_mutex_unlock(&reqListLock);
	
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return MPI_SUCCESS;
}

int MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}

	MPI_Request req;
	retVal = MPI_Iscatter(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,root,comm,&req);
	retVal = MPI_Wait(&req,MPI_STATUS_IGNORE);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return retVal;
}

int MPI_Iscatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request *req) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	MPI_Request ftreq;
	int myrank;
	int ssize,rsize;
	int commnC,commnR;
	int *tmpscounts;
	int *tmpsdispls;
	struct collective_data *curargs;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	parep_mpi_sighandling_state = 1;
	ftreq = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
	(ftreq)->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqcolls = NULL;
	(ftreq)->num_reqcolls = 0;
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	ftreq->bufloc = recvbuf;
	ftreq->complete = false;
	ftreq->status.count = sendcount;
	ftreq->status.MPI_SOURCE = root;
	ftreq->status.MPI_ERROR = 0;
	ftreq->type = MPI_FT_COLLECTIVE_REQUEST;
	
	bool completecmp = false;
	bool completerep = false;

	pthread_mutex_lock(&reqListLock);
	
	pthread_rwlock_rdlock(&commLock);
	ftreq->comm = comm;
	
	if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
		EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnC);
		EMPI_Comm_size(comm->EMPI_COMM_REP,&commnR);
		
		tmpscounts = (int *)malloc(commnR*sizeof(int));
		tmpsdispls = (int *)malloc(commnR*sizeof(int));
		int j = 0;
		for(int i = 0; i < commnC; i++) {
			if(cmpToRepMap[i] != -1) {
				tmpscounts[j] = sendcount;
				tmpsdispls[j] = i*sendcount;
				j++;
			}
		}
		
		completecmp = true;
		if(cmpToRepMap[root] == -1) EMPI_Iscatterv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, root, (comm->EMPI_CMP_REP_INTERCOMM), (ftreq)->reqrep);
		else retVal = EMPI_Iscatterv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, cmpToRepMap[root], (comm->EMPI_COMM_REP), (ftreq)->reqrep);
		
		free(tmpscounts);
		free(tmpsdispls);
	} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
		EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
		if(comm->EMPI_CMP_REP_INTERCOMM != EMPI_COMM_NULL) EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnR);
		else commnR = 0;
		if((cmpToRepMap[root] == -1) && (nR > 0)) {
			tmpscounts = (int *)malloc(commnR*sizeof(int));
			tmpsdispls = (int *)malloc(commnR*sizeof(int));
			int j = 0;
			for(int i = 0; i < commnC; i++) {
				if(cmpToRepMap[i] != -1) {
					tmpscounts[j] = sendcount;
					tmpsdispls[j] = i*sendcount;
					j++;
				}
			}
		}
		
		retVal = EMPI_Iscatter (sendbuf, sendcount, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, root, (comm->EMPI_COMM_CMP), (ftreq)->reqcmp);
		if(cmpToRepMap[root] == -1 && myrank == root && nR > 0) retVal = EMPI_Iscatterv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, EMPI_ROOT, (comm->EMPI_CMP_REP_INTERCOMM), (ftreq)->reqrep);
		else if(cmpToRepMap[root] == -1 && nR > 0) retVal = EMPI_Iscatterv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, EMPI_PROC_NULL, (comm->EMPI_CMP_REP_INTERCOMM), (ftreq)->reqrep);
		else completerep = true;
		
		if((cmpToRepMap[root] == -1) && (nR > 0)) {
			free(tmpscounts);
			free(tmpsdispls);
		}
	}
	pthread_rwlock_unlock(&commLock);
	
	EMPI_Type_size(sendtype->edatatype,&ssize);
	EMPI_Type_size(recvtype->edatatype,&rsize);
	curargs = (struct collective_data *)parep_mpi_malloc(sizeof(struct collective_data));
	if(myrank == root) {
		(curargs->args).scatter.sendbuf = parep_mpi_malloc(commnC*sendcount*ssize);
		memcpy((curargs->args).scatter.sendbuf,sendbuf,commnC*sendcount*ssize);
	} else (curargs->args).scatter.sendbuf = NULL;
	(curargs->args).scatter.sendcount = sendcount;
	(curargs->args).scatter.senddt = sendtype;
	(curargs->args).scatter.recvcount = recvcount;
	(curargs->args).scatter.recvdt = recvtype;
	(curargs->args).scatter.root = root;
	(curargs->args).scatter.comm = comm;
	curargs->req = req;
	curargs->type = MPI_FT_SCATTER;
	curargs->completecmp = completecmp;
	curargs->completerep = completerep;
	curargs->completecolls = NULL;
	curargs->num_colls = 0;
	
	pthread_mutex_lock(&collectiveLock);
	curargs->id = parep_mpi_collective_id;
	if(last_collective == NULL) {
		curargs->prev = NULL;
		curargs->next = NULL;
		last_collective = curargs;
	} else {
		clcdata *temp = last_collective;
		while((curargs->id <= temp->id) && (temp->next != NULL)) {
			temp = temp->next;
		}
		if(curargs->id > temp->id) {
			curargs->prev = temp->prev;
			curargs->next = temp;
			if(temp->prev != NULL) temp->prev->next = curargs;
			else last_collective = curargs;
			temp->prev = curargs;
		}
		else {
			curargs->prev = temp;
			curargs->next = NULL;
			temp->next = curargs;
		}
	}
	parep_mpi_collective_id++;
	pthread_cond_signal(&collectiveCond);
	pthread_mutex_unlock(&collectiveLock);
	
	(ftreq)->storeloc = curargs;
	*req = ftreq;
	(*req)->rnode = reqListInsert(*req);
	
	pthread_cond_signal(&reqListCond);
	pthread_mutex_unlock(&reqListLock);
	
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return MPI_SUCCESS;
}

int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}

	MPI_Request req;
	retVal = MPI_Igather(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,root,comm,&req);
	retVal = MPI_Wait(&req,MPI_STATUS_IGNORE);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return retVal;
}

int MPI_Igather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request *req) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	MPI_Request ftreq;
	int myrank;
	int ssize,rsize;
	int commnC,commnR;
	int *tmprcounts;
	int *tmprdispls;
	int *tmprcountsinter;
	int *tmprdisplsinter;
	struct collective_data *curargs;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	parep_mpi_sighandling_state = 1;
	ftreq = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
	(ftreq)->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqcolls = NULL;
	(ftreq)->num_reqcolls = 0;
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	ftreq->bufloc = recvbuf;
	ftreq->complete = false;
	ftreq->status.count = sendcount;
	ftreq->status.MPI_SOURCE = root;
	ftreq->status.MPI_ERROR = 0;
	ftreq->type = MPI_FT_COLLECTIVE_REQUEST;
	
	bool completecmp = false;
	bool completerep = false;
	
	pthread_mutex_lock(&reqListLock);
	
	pthread_rwlock_rdlock(&commLock);
	ftreq->comm = comm;
	
	if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
		EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
		EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
		
		if(cmpToRepMap[root] != -1) {
			tmprcounts = (int *)malloc(commnR*sizeof(int));
			tmprdispls = (int *)malloc(commnR*sizeof(int));
			if(commnC > commnR && commnR > 0) {
				tmprcountsinter = (int *)malloc((commnC-commnR)*sizeof(int));
				tmprdisplsinter = (int *)malloc((commnC-commnR)*sizeof(int));
			}
			int j = 0;
			int k = 0;
			for(int i = 0; i < commnC; i++) {
				if(cmpToRepMap[i] != -1) {
					tmprcounts[j] = recvcount;
					tmprdispls[j] = i*recvcount;
					j++;
				} else if(commnC > commnR && commnR > 0) {
					tmprcountsinter[k] = recvcount;
					tmprdisplsinter[k] = i*recvcount;
					k++;
				}
			}
			
			if(myrank == root && commnC > commnR && commnR > 0) retVal = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcountsinter, tmprdisplsinter, recvtype->edatatype, EMPI_ROOT, (comm->EMPI_CMP_NO_REP_INTERCOMM), (ftreq)->reqcmp);
			else if(commnC > commnR && commnR > 0) retVal = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcountsinter, tmprdisplsinter, recvtype->edatatype, EMPI_PROC_NULL, (comm->EMPI_CMP_NO_REP_INTERCOMM), (ftreq)->reqcmp);
			else completecmp = true;
			retVal = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcounts, tmprdispls, recvtype->edatatype, cmpToRepMap[root], (comm->EMPI_COMM_REP), (ftreq)->reqrep);
			
			free(tmprcounts);
			free(tmprdispls);
			if(commnC > commnR && commnR > 0) {
				free(tmprcountsinter);
				free(tmprdisplsinter);
			}
		} else {
			completecmp = true;
			completerep = true;
		}
	} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
		EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
		if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
		else commnR = 0;
			
		if(cmpToRepMap[root] != -1 && cmpToRepMap[myrank] == -1) {
			tmprcountsinter = (int *)malloc((commnC-commnR)*sizeof(int));
			tmprdisplsinter = (int *)malloc((commnC-commnR)*sizeof(int));
			int j = 0;
			for(int i = 0; i < commnC; i++) {
				if(cmpToRepMap[i] == -1) {
					tmprcountsinter[j] = recvcount;
					tmprdisplsinter[j] = i*recvcount;
					j++;
				}
			}
		}
		
		retVal = EMPI_Igather (sendbuf, sendcount, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, root, (comm->EMPI_COMM_CMP), (ftreq)->reqcmp);
		if(cmpToRepMap[root] != -1 && cmpToRepMap[myrank] == -1) retVal = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcountsinter, tmprdisplsinter, recvtype->edatatype, cmpToRepMap[root], (comm->EMPI_CMP_NO_REP_INTERCOMM), (ftreq)->reqrep);
		else completerep = true;
		
		if(cmpToRepMap[root] != -1 && cmpToRepMap[myrank] == -1) {
			free(tmprcountsinter);
			free(tmprdisplsinter);
		}
	}
	pthread_rwlock_unlock(&commLock);
	
	EMPI_Type_size(sendtype->edatatype,&ssize);
	EMPI_Type_size(recvtype->edatatype,&rsize);
	curargs = (struct collective_data *)parep_mpi_malloc(sizeof(struct collective_data));
	(curargs->args).gather.sendbuf = parep_mpi_malloc(sendcount*ssize);
	memcpy((curargs->args).gather.sendbuf,sendbuf,sendcount*ssize);
	(curargs->args).gather.sendcount = sendcount;
	(curargs->args).gather.senddt = sendtype;
	(curargs->args).gather.recvcount = recvcount;
	(curargs->args).gather.recvdt = recvtype;
	(curargs->args).gather.root = root;
	(curargs->args).gather.comm = comm;
	curargs->req = req;
	curargs->type = MPI_FT_GATHER;
	curargs->completecmp = completecmp;
	curargs->completerep = completerep;
	curargs->completecolls = NULL;
	curargs->num_colls = 0;
	
	pthread_mutex_lock(&collectiveLock);
	curargs->id = parep_mpi_collective_id;
	if(last_collective == NULL) {
		curargs->prev = NULL;
		curargs->next = NULL;
		last_collective = curargs;
	} else {
		clcdata *temp = last_collective;
		while((curargs->id <= temp->id) && (temp->next != NULL)) {
			temp = temp->next;
		}
		if(curargs->id > temp->id) {
			curargs->prev = temp->prev;
			curargs->next = temp;
			if(temp->prev != NULL) temp->prev->next = curargs;
			else last_collective = curargs;
			temp->prev = curargs;
		}
		else {
			curargs->prev = temp;
			curargs->next = NULL;
			temp->next = curargs;
		}
	}
	parep_mpi_collective_id++;
	pthread_cond_signal(&collectiveCond);
	pthread_mutex_unlock(&collectiveLock);
	
	(ftreq)->storeloc = curargs;
	*req = ftreq;
	(*req)->rnode = reqListInsert(*req);
	
	pthread_cond_signal(&reqListCond);
	pthread_mutex_unlock(&reqListLock);
	
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return MPI_SUCCESS;
}

int MPI_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}

	MPI_Request req;
	retVal = MPI_Iallgather(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,comm,&req);
	retVal = MPI_Wait(&req,MPI_STATUS_IGNORE);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return retVal;
}

int MPI_Iallgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *req) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	MPI_Request ftreq;
	int myrank;
	int ssize,rsize;
	int commnC,commnR;
	int *tmpscounts;
	int *tmpsdispls;
	int *tmprcounts;
	int *tmprdispls;
	int *tmprcountsinter;
	int *tmprdisplsinter;
	struct collective_data *curargs;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	parep_mpi_sighandling_state = 1;
	ftreq = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
	(ftreq)->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	ftreq->bufloc = recvbuf;
	ftreq->complete = false;
	ftreq->status.count = sendcount;
	ftreq->status.MPI_SOURCE = myrank;
	ftreq->status.MPI_ERROR = 0;
	ftreq->type = MPI_FT_COLLECTIVE_REQUEST;
	
	curargs = (struct collective_data *)parep_mpi_malloc(sizeof(struct collective_data));
	
	bool completecmp = false;
	bool completerep = false;
	
	pthread_mutex_lock(&reqListLock);
	
	pthread_rwlock_rdlock(&commLock);
	ftreq->comm = comm;
	
	if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
		EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
		EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
		
		tmpscounts = (int *)malloc(commnR*sizeof(int));
		tmpsdispls = (int *)malloc(commnR*sizeof(int));
		tmprcounts = (int *)malloc(commnR*sizeof(int));
		tmprdispls = (int *)malloc(commnR*sizeof(int));
		if(commnC > commnR && commnR > 0) {
			tmprcountsinter = (int *)malloc((commnC-commnR)*sizeof(int));
			tmprdisplsinter = (int *)malloc((commnC-commnR)*sizeof(int));
		}
		int j = 0;
		int k = 0;
		for(int i = 0; i < commnC; i++) {
			if(cmpToRepMap[i] != -1) {
				tmpscounts[j] = sendcount;
				tmpsdispls[j] = i*sendcount;
				tmprcounts[j] = recvcount;
				tmprdispls[j] = i*recvcount;
				j++;
			} else if(commnC > commnR && commnR > 0) {
				tmprcountsinter[k] = recvcount;
				tmprdisplsinter[k] = i*recvcount;
				k++;
			}
		}
		
		completecmp = true;
		if(commnC > commnR && commnR > 0) {
			ftreq->reqcolls = (EMPI_Request **)parep_mpi_malloc(commnR*sizeof(EMPI_Request *));
			ftreq->num_reqcolls = commnR;
			curargs->completecolls = (bool *)parep_mpi_malloc(commnR*sizeof(bool));
			curargs->num_colls = commnR;
			for(int i = 0; i < commnR; i++) {
				ftreq->reqcolls[i] = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
				curargs->completecolls[i] = false;
				if(cmpToRepMap[myrank] == i) retVal = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcountsinter, tmprdisplsinter, recvtype->edatatype, EMPI_ROOT, (comm->EMPI_CMP_NO_REP_INTERCOMM), ftreq->reqcolls[i]);
				else retVal = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcountsinter, tmprdisplsinter, recvtype->edatatype, EMPI_PROC_NULL, (comm->EMPI_CMP_NO_REP_INTERCOMM), ftreq->reqcolls[i]);
			}
		} else {
			ftreq->reqcolls = NULL;
			ftreq->num_reqcolls = 0;
			curargs->completecolls = NULL;
			curargs->num_colls = 0;
		}
		retVal = EMPI_Iallgatherv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, tmprcounts, tmprdispls, recvtype->edatatype, (comm->EMPI_COMM_REP), ftreq->reqrep);
		
		free(tmpscounts);
		free(tmpsdispls);
		free(tmprcounts);
		free(tmprdispls);
		if(commnC > commnR && commnR > 0) {
			free(tmprcountsinter);
			free(tmprdisplsinter);
		}
	} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
		EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
		if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
		else commnR = 0;
		
		retVal = EMPI_Iallgather (sendbuf, sendcount, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, (comm->EMPI_COMM_CMP), ftreq->reqcmp);
		if(commnR > 0 && cmpToRepMap[myrank] == -1) {
			tmprcountsinter = (int *)malloc((commnC-commnR)*sizeof(int));
			tmprdisplsinter = (int *)malloc((commnC-commnR)*sizeof(int));
			int j = 0;
			for(int i = 0; i < commnC; i++) {
				if(cmpToRepMap[i] == -1) {
					tmprcountsinter[j] = recvcount;
					tmprdisplsinter[j] = i*recvcount;
					j++;
				}
			}
			ftreq->reqcolls = (EMPI_Request **)parep_mpi_malloc(commnR*sizeof(EMPI_Request *));
			ftreq->num_reqcolls = commnR;
			curargs->completecolls = (bool *)parep_mpi_malloc(commnR*sizeof(bool));
			curargs->num_colls = commnR;
			for(int i = 0; i < commnR; i++) {
				ftreq->reqcolls[i] = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
				curargs->completecolls[i] = false;
				retVal = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcountsinter, tmprdisplsinter, recvtype->edatatype, i, (comm->EMPI_CMP_NO_REP_INTERCOMM), ftreq->reqcolls[i]);
			}
		} else {
			ftreq->reqcolls = NULL;
			ftreq->num_reqcolls = 0;
			curargs->completecolls = NULL;
			curargs->num_colls = 0;
		}
		completerep = true;
		
		if(commnR > 0 && cmpToRepMap[myrank] == -1) {
			free(tmprcountsinter);
			free(tmprdisplsinter);
		}
	}
	pthread_rwlock_unlock(&commLock);
	
	EMPI_Type_size(sendtype->edatatype,&ssize);
	EMPI_Type_size(recvtype->edatatype,&rsize);

	(curargs->args).allgather.sendbuf = parep_mpi_malloc(sendcount*ssize);
	memcpy((curargs->args).allgather.sendbuf,sendbuf,sendcount*ssize);
	(curargs->args).allgather.sendcount = sendcount;
	(curargs->args).allgather.senddt = sendtype;
	(curargs->args).allgather.recvcount = recvcount;
	(curargs->args).allgather.recvdt = recvtype;
	(curargs->args).allgather.comm = comm;
	curargs->req = req;
	curargs->type = MPI_FT_ALLGATHER;
	curargs->completecmp = completecmp;
	curargs->completerep = completerep;
	
	pthread_mutex_lock(&collectiveLock);
	curargs->id = parep_mpi_collective_id;
	if(last_collective == NULL) {
		curargs->prev = NULL;
		curargs->next = NULL;
		last_collective = curargs;
	} else {
		clcdata *temp = last_collective;
		while((curargs->id <= temp->id) && (temp->next != NULL)) {
			temp = temp->next;
		}
		if(curargs->id > temp->id) {
			curargs->prev = temp->prev;
			curargs->next = temp;
			if(temp->prev != NULL) temp->prev->next = curargs;
			else last_collective = curargs;
			temp->prev = curargs;
		}
		else {
			curargs->prev = temp;
			curargs->next = NULL;
			temp->next = curargs;
		}
	}
	parep_mpi_collective_id++;
	pthread_cond_signal(&collectiveCond);
	pthread_mutex_unlock(&collectiveLock);
	
	(ftreq)->storeloc = curargs;
	*req = ftreq;
	(*req)->rnode = reqListInsert(*req);
	
	pthread_cond_signal(&reqListCond);
	pthread_mutex_unlock(&reqListLock);
	
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return MPI_SUCCESS;
}

/* MPI_Alltoall */
int MPI_Alltoall (void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}

	MPI_Request req;
	retVal = MPI_Ialltoall(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,comm,&req);
	retVal = MPI_Wait(&req,MPI_STATUS_IGNORE);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return retVal;
}

int MPI_Ialltoall (void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *req) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	MPI_Request ftreq;
	int myrank;
	int ssize,rsize;
	int commnC,commnR;
	int noreprank;
	int *tmpscounts;
	int *tmpsdispls;
	int *tmprcounts;
	int *tmprdispls;
	int *tmpscountsinter;
	int *tmpsdisplsinter;
	int *intermapping;
	struct collective_data *curargs;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	parep_mpi_sighandling_state = 1;
	ftreq = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
	(ftreq)->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	ftreq->bufloc = recvbuf;
	ftreq->complete = false;
	ftreq->status.count = sendcount;
	ftreq->status.MPI_SOURCE = myrank;
	ftreq->status.MPI_ERROR = 0;
	ftreq->type = MPI_FT_COLLECTIVE_REQUEST;
	
	curargs = (struct collective_data *)parep_mpi_malloc(sizeof(struct collective_data));
	
	bool completecmp = false;
	bool completerep = false;
	
	EMPI_Type_size(sendtype->edatatype,&ssize);
	EMPI_Type_size(recvtype->edatatype,&rsize);
	
	pthread_mutex_lock(&reqListLock);
	
	pthread_rwlock_rdlock(&commLock);
	ftreq->comm = comm;
	
	if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
		EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
		EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
		
		tmpscounts = (int *)malloc(commnR*sizeof(int));
		tmpsdispls = (int *)malloc(commnR*sizeof(int));
		tmprcounts = (int *)malloc(commnR*sizeof(int));
		tmprdispls = (int *)malloc(commnR*sizeof(int));
		if(commnC > commnR && commnR > 0) {
			tmpscountsinter = (int *)malloc((commnR)*sizeof(int));
			tmpsdisplsinter = (int *)malloc((commnR)*sizeof(int));
			intermapping = (int *)malloc((commnC-commnR)*sizeof(int));
		}
		int j = 0;
		int k = 0;
		for(int i = 0; i < commnC; i++) {
			if(cmpToRepMap[i] != -1) {
				tmpscounts[j] = sendcount;
				tmpsdispls[j] = i*sendcount;
				tmprcounts[j] = recvcount;
				tmprdispls[j] = i*recvcount;
				if(commnC > commnR && commnR > 0) {
					tmpscountsinter[j] = sendcount;
					tmpsdisplsinter[j] = i*sendcount;
				}
				j++;
			}
			else if(commnC > commnR && commnR > 0) {
				intermapping[k] = i;
				k++;
			}
		}
		
		completecmp = true;
		if(commnC > commnR && commnR > 0) {
			ftreq->reqcolls = (EMPI_Request **)parep_mpi_malloc((commnC-commnR)*sizeof(EMPI_Request *));
			ftreq->num_reqcolls = commnC-commnR;
			curargs->completecolls = (bool *)parep_mpi_malloc((commnC-commnR)*sizeof(bool));
			curargs->num_colls = commnC-commnR;
			for(int i = 0; i < commnC-commnR; i++) {
				ftreq->reqcolls[i] = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
				curargs->completecolls[i] = false;
				retVal = EMPI_Iscatterv (sendbuf, tmpscountsinter, tmpsdisplsinter, sendtype->edatatype, recvbuf + intermapping[i]*recvcount*rsize, recvcount, recvtype->edatatype, i, (comm->EMPI_CMP_NO_REP_INTERCOMM), ftreq->reqcolls[i]);
			}
		} else {
			ftreq->reqcolls = NULL;
			ftreq->num_reqcolls = 0;
			curargs->completecolls = NULL;
			curargs->num_colls = 0;
		}
		retVal = EMPI_Ialltoallv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, tmprcounts, tmprdispls, recvtype->edatatype, (comm->EMPI_COMM_REP), ftreq->reqrep);
		
		free(tmpscounts);
		free(tmpsdispls);
		free(tmprcounts);
		free(tmprdispls);
		if(commnC > commnR && commnR > 0) {
			free(tmpscountsinter);
			free(tmpsdisplsinter);
			free(intermapping);
		}
	} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
		EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
		if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
		else commnR = 0;
			
		retVal = EMPI_Ialltoall (sendbuf, sendcount, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, (comm->EMPI_COMM_CMP), ftreq->reqcmp);
		if(commnR > 0 && cmpToRepMap[myrank] == -1) {
			tmpscountsinter = (int *)malloc((commnR)*sizeof(int));
			tmpsdisplsinter = (int *)malloc((commnR)*sizeof(int));
			intermapping = (int *)malloc((commnC-commnR)*sizeof(int));
			int j = 0;
			int k = 0;
			for(int i = 0; i < commnC; i++) {
				if(cmpToRepMap[i] != -1) {
					tmpscountsinter[j] = sendcount;
					tmpsdisplsinter[j] = i*sendcount;
					j++;
				}
				else {
					intermapping[k] = i;
					k++;
				}	
			}
			ftreq->reqcolls = (EMPI_Request **)parep_mpi_malloc((commnC-commnR)*sizeof(EMPI_Request *));
			ftreq->num_reqcolls = commnC-commnR;
			curargs->completecolls = (bool *)parep_mpi_malloc((commnC-commnR)*sizeof(bool));
			curargs->num_colls = commnC-commnR;
			EMPI_Comm_rank(comm->EMPI_CMP_NO_REP,&noreprank);
			for(int i = 0; i < commnC-commnR; i++) {
				ftreq->reqcolls[i] = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
				
				curargs->completecolls[i] = false;
				if(i == noreprank) retVal = EMPI_Iscatterv (sendbuf, tmpscountsinter, tmpsdisplsinter, sendtype->edatatype, recvbuf + intermapping[i]*recvcount*rsize, recvcount, recvtype->edatatype, EMPI_ROOT, (comm->EMPI_CMP_NO_REP_INTERCOMM), ftreq->reqcolls[i]);
				else retVal = EMPI_Iscatterv (sendbuf, tmpscountsinter, tmpsdisplsinter, sendtype->edatatype, recvbuf + intermapping[i]*recvcount*rsize, recvcount, recvtype->edatatype, EMPI_PROC_NULL, (comm->EMPI_CMP_NO_REP_INTERCOMM), ftreq->reqcolls[i]);
			}
		} else {
			ftreq->reqcolls = NULL;
			ftreq->num_reqcolls = 0;
			curargs->completecolls = NULL;
			curargs->num_colls = 0;
		}
		completerep = true;
		
		if(commnR > 0 && cmpToRepMap[myrank] == -1) {
			free(tmpscountsinter);
			free(tmpsdisplsinter);
			free(intermapping);
		}
	}
	pthread_rwlock_unlock(&commLock);
	
	(curargs->args).alltoall.sendbuf = parep_mpi_malloc(commnC*sendcount*ssize);
	memcpy((curargs->args).alltoall.sendbuf,sendbuf,commnC*sendcount*ssize);
	(curargs->args).alltoall.sendcount = sendcount;
	(curargs->args).alltoall.senddt = sendtype;
	(curargs->args).alltoall.recvcount = recvcount;
	(curargs->args).alltoall.recvdt = recvtype;
	(curargs->args).alltoall.comm = comm;
	curargs->req = req;
	curargs->type = MPI_FT_ALLTOALL;
	curargs->completecmp = completecmp;
	curargs->completerep = completerep;
	
	pthread_mutex_lock(&collectiveLock);
	curargs->id = parep_mpi_collective_id;
	if(last_collective == NULL) {
		curargs->prev = NULL;
		curargs->next = NULL;
		last_collective = curargs;
	} else {
		clcdata *temp = last_collective;
		while((curargs->id <= temp->id) && (temp->next != NULL)) {
			temp = temp->next;
		}
		if(curargs->id > temp->id) {
			curargs->prev = temp->prev;
			curargs->next = temp;
			if(temp->prev != NULL) temp->prev->next = curargs;
			else last_collective = curargs;
			temp->prev = curargs;
		}
		else {
			curargs->prev = temp;
			curargs->next = NULL;
			temp->next = curargs;
		}
	}
	parep_mpi_collective_id++;
	pthread_cond_signal(&collectiveCond);
	pthread_mutex_unlock(&collectiveLock);
	
	(ftreq)->storeloc = curargs;
	*req = ftreq;
	(*req)->rnode = reqListInsert(*req);
	
	pthread_cond_signal(&reqListCond);
	pthread_mutex_unlock(&reqListLock);
	
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return MPI_SUCCESS;
}

/* MPI_Alltoallv */
int MPI_Alltoallv(void *sendbuf, int *sendcounts, int *sdispls, MPI_Datatype sendtype, void *recvbuf, int *recvcounts, int *rdispls, MPI_Datatype recvtype, MPI_Comm comm) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}

	MPI_Request req;
	retVal = MPI_Ialltoallv(sendbuf,sendcounts,sdispls,sendtype,recvbuf,recvcounts,rdispls,recvtype,comm,&req);
	retVal = MPI_Wait(&req,MPI_STATUS_IGNORE);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return retVal;
}

int MPI_Ialltoallv(void *sendbuf, int *sendcounts, int *sdispls, MPI_Datatype sendtype, void *recvbuf, int *recvcounts, int *rdispls, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *req) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	MPI_Request ftreq;
	int myrank;
	int ssize,rsize;
	int commnC,commnR;
	int noreprank;
	int *tmpscounts;
	int *tmpsdispls;
	int *tmprcounts;
	int *tmprdispls;
	int *tmpscountsinter;
	int *tmpsdisplsinter;
	int *intermapping;
	long scount,rcount;
	struct collective_data *curargs;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	parep_mpi_sighandling_state = 1;
	ftreq = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
	(ftreq)->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	ftreq->bufloc = recvbuf;
	ftreq->complete = false;
	ftreq->status.MPI_SOURCE = myrank;
	ftreq->status.MPI_ERROR = 0;
	ftreq->type = MPI_FT_COLLECTIVE_REQUEST;
	
	curargs = (struct collective_data *)parep_mpi_malloc(sizeof(struct collective_data));
	
	bool completecmp = false;
	bool completerep = false;
	
	EMPI_Type_size(sendtype->edatatype,&ssize);
	EMPI_Type_size(recvtype->edatatype,&rsize);
	
	pthread_mutex_lock(&reqListLock);
	
	pthread_rwlock_rdlock(&commLock);
	ftreq->comm = comm;
	
	if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
		EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
		EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
			
		tmpscounts = (int *)malloc(commnR*sizeof(int));
		tmpsdispls = (int *)malloc(commnR*sizeof(int));
		tmprcounts = (int *)malloc(commnR*sizeof(int));
		tmprdispls = (int *)malloc(commnR*sizeof(int));
		if(commnC > commnR && commnR > 0) {
			tmpscountsinter = (int *)malloc((commnR)*sizeof(int));
			tmpsdisplsinter = (int *)malloc((commnR)*sizeof(int));
			intermapping = (int *)malloc((commnC-commnR)*sizeof(int));
		}
		int j = 0;
		int k = 0;
		for(int i = 0; i < commnC; i++) {
			if(cmpToRepMap[i] != -1) {
				tmpscounts[j] = sendcounts[i];
				tmpsdispls[j] = sdispls[i];
				tmprcounts[j] = recvcounts[i];
				tmprdispls[j] = rdispls[i];
				if(commnC > commnR && commnR > 0) {
					tmpscountsinter[j] = sendcounts[i];
					tmpsdisplsinter[j] = sdispls[i];
				}
				j++;
			}
			else if(commnC > commnR && commnR > 0) {
				intermapping[k] = i;
				k++;
			}
		}
		
		completecmp = true;
		if(commnC > commnR && commnR > 0) {
			ftreq->reqcolls = (EMPI_Request **)parep_mpi_malloc((commnC-commnR)*sizeof(EMPI_Request *));
			ftreq->num_reqcolls = commnC-commnR;
			curargs->completecolls = (bool *)parep_mpi_malloc((commnC-commnR)*sizeof(bool));
			curargs->num_colls = commnC-commnR;
			for(int i = 0; i < commnC-commnR; i++) {
				ftreq->reqcolls[i] = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
				curargs->completecolls[i] = false;
				retVal = EMPI_Iscatterv (sendbuf, tmpscountsinter, tmpsdisplsinter, sendtype->edatatype, recvbuf + (rdispls[intermapping[i]]*rsize), recvcounts[intermapping[i]], recvtype->edatatype, i, (comm->EMPI_CMP_NO_REP_INTERCOMM), ftreq->reqcolls[i]);
			}
		} else {
			ftreq->reqcolls = NULL;
			ftreq->num_reqcolls = 0;
			curargs->completecolls = NULL;
			curargs->num_colls = 0;
		}
		retVal = EMPI_Ialltoallv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, tmprcounts, tmprdispls, recvtype->edatatype, (comm->EMPI_COMM_REP), ftreq->reqrep);
		
		free(tmpscounts);
		free(tmpsdispls);
		free(tmprcounts);
		free(tmprdispls);
		if(commnC > commnR && commnR > 0) {
			free(tmpscountsinter);
			free(tmpsdisplsinter);
			free(intermapping);
		}
	} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
		EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
		if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
		else commnR = 0;
		
		retVal = EMPI_Ialltoallv (sendbuf, sendcounts, sdispls, sendtype->edatatype, recvbuf, recvcounts, rdispls, recvtype->edatatype, (comm->EMPI_COMM_CMP), ftreq->reqcmp);
		if(commnR > 0 && cmpToRepMap[myrank] == -1) {
			tmpscountsinter = (int *)malloc((commnR)*sizeof(int));
			tmpsdisplsinter = (int *)malloc((commnR)*sizeof(int));
			intermapping = (int *)malloc((commnC-commnR)*sizeof(int));
			int j = 0;
			int k = 0;
			for(int i = 0; i < commnC; i++) {
				if(cmpToRepMap[i] != -1) {
					tmpscountsinter[j] = sendcounts[i];
					tmpsdisplsinter[j] = sdispls[i];
					j++;
				}
				else {
					intermapping[k] = i;
					k++;
				}
			}
			
			ftreq->reqcolls = (EMPI_Request **)parep_mpi_malloc((commnC-commnR)*sizeof(EMPI_Request *));
			ftreq->num_reqcolls = commnC-commnR;
			curargs->completecolls = (bool *)parep_mpi_malloc((commnC-commnR)*sizeof(bool));
			curargs->num_colls = commnC-commnR;
			EMPI_Comm_rank(comm->EMPI_CMP_NO_REP,&noreprank);
			for(int i = 0; i < commnC-commnR; i++) {
				ftreq->reqcolls[i] = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
				curargs->completecolls[i] = false;
				if(i == noreprank) retVal = EMPI_Iscatterv (sendbuf, tmpscountsinter, tmpsdisplsinter, sendtype->edatatype, recvbuf + (rdispls[intermapping[i]]*rsize), recvcounts[intermapping[i]], recvtype->edatatype, EMPI_ROOT, (comm->EMPI_CMP_NO_REP_INTERCOMM), ftreq->reqcolls[i]);
				else retVal = EMPI_Iscatterv (sendbuf, tmpscountsinter, tmpsdisplsinter, sendtype->edatatype, recvbuf + (rdispls[intermapping[i]]*rsize), recvcounts[intermapping[i]], recvtype->edatatype, EMPI_PROC_NULL, (comm->EMPI_CMP_NO_REP_INTERCOMM), ftreq->reqcolls[i]);
			}
		} else {
			ftreq->reqcolls = NULL;
			ftreq->num_reqcolls = 0;
			curargs->completecolls = NULL;
			curargs->num_colls = 0;
		}
		completerep = true;
		
		if(commnR > 0 && cmpToRepMap[myrank] == -1) {
			free(tmpscountsinter);
			free(tmpsdisplsinter);
			free(intermapping);
		}
	}
	pthread_rwlock_unlock(&commLock);
	
	rcount = array_sum(recvcounts,commnC);
	scount = array_sum(sendcounts,commnC);
	(curargs->args).alltoallv.sendbuf = parep_mpi_malloc(scount*ssize);
	long logdispl = 0;
	for(int i = 0; i < commnC; i++) {
		memcpy((curargs->args).alltoallv.sendbuf + logdispl,sendbuf + (sdispls[i]*ssize),(sendcounts[i]*ssize));
		logdispl += (sendcounts[i]*ssize);
	}
	(curargs->args).alltoallv.sendcounts = (int *)parep_mpi_malloc(commnC*sizeof(int));
	memcpy((curargs->args).alltoallv.sendcounts,sendcounts,commnC*sizeof(int));
	(curargs->args).alltoallv.sdispls = (int *)parep_mpi_malloc(commnC*sizeof(int));
	memcpy((curargs->args).alltoallv.sdispls,sdispls,commnC*sizeof(int));
	(curargs->args).alltoallv.senddt = sendtype;
	(curargs->args).alltoallv.recvcounts = (int *)parep_mpi_malloc(commnC*sizeof(int));
	memcpy((curargs->args).alltoallv.recvcounts,recvcounts,commnC*sizeof(int));
	(curargs->args).alltoallv.rdispls = (int *)parep_mpi_malloc(commnC*sizeof(int));
	memcpy((curargs->args).alltoallv.rdispls,rdispls,commnC*sizeof(int));
	(curargs->args).alltoallv.recvdt = recvtype;
	(curargs->args).alltoallv.comm = comm;
	curargs->req = req;
	curargs->type = MPI_FT_ALLTOALLV;
	curargs->completecmp = completecmp;
	curargs->completerep = completerep;
	
	pthread_mutex_lock(&collectiveLock);
	curargs->id = parep_mpi_collective_id;
	if(last_collective == NULL) {
		curargs->prev = NULL;
		curargs->next = NULL;
		last_collective = curargs;
	} else {
		clcdata *temp = last_collective;
		while((curargs->id <= temp->id) && (temp->next != NULL)) {
			temp = temp->next;
		}
		if(curargs->id > temp->id) {
			curargs->prev = temp->prev;
			curargs->next = temp;
			if(temp->prev != NULL) temp->prev->next = curargs;
			else last_collective = curargs;
			temp->prev = curargs;
		}
		else {
			curargs->prev = temp;
			curargs->next = NULL;
			temp->next = curargs;
		}
	}
	parep_mpi_collective_id++;
	pthread_cond_signal(&collectiveCond);
	pthread_mutex_unlock(&collectiveLock);
	
	(ftreq)->storeloc = curargs;
	*req = ftreq;
	(*req)->rnode = reqListInsert(*req);
	
	pthread_cond_signal(&reqListCond);
	pthread_mutex_unlock(&reqListLock);
	
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return MPI_SUCCESS;
}

/* MPI_Reduce */
int MPI_Reduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}

	MPI_Request req;
	retVal = MPI_Ireduce(sendbuf,recvbuf,count,datatype,op,root,comm,&req);
	retVal = MPI_Wait(&req,MPI_STATUS_IGNORE);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return retVal;
}

int MPI_Ireduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm, MPI_Request *req) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	MPI_Request ftreq;
	int myrank;
	int size;
	int commnC,commnR;
	struct collective_data *curargs;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	parep_mpi_sighandling_state = 1;
	ftreq = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
	(ftreq)->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqcolls = NULL;
	(ftreq)->num_reqcolls = 0;
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	ftreq->bufloc = recvbuf;
	ftreq->complete = false;
	ftreq->status.count = count;
	ftreq->status.MPI_SOURCE = root;
	ftreq->status.MPI_ERROR = 0;
	ftreq->type = MPI_FT_COLLECTIVE_REQUEST;
	
	bool completecmp = false;
	bool completerep = false;
	
	int extracount;
	int dis;
	EMPI_Type_size(datatype->edatatype,&size);
	if(size >= sizeof(int)) extracount = 1;
	else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
	else extracount = (((int)sizeof(int))/size) + 1;
	dis = ((count+extracount)*size) - sizeof(int);
	
	pthread_mutex_lock(&reqListLock);
	
	pthread_mutex_lock(&recvDataListLock);
	recvDataNode *rnode = recvDataRedListFind(parep_mpi_collective_id,MPI_FT_REDUCE_TEMP);
	if(rnode != NULL) {
		curargs = (clcdata *)rnode->pdata;
		recvDataRedListDelete(rnode);
		pthread_cond_signal(&recvDataListCond);
		pthread_mutex_unlock(&recvDataListLock);
		memcpy(ftreq->bufloc,curargs->args.reduce.recvbuf,count*size);
		pthread_mutex_lock(&collectiveLock);
		curargs->type = MPI_FT_REDUCE;
		curargs->completecmp = true;
		curargs->completerep = true;
		(curargs->args).reduce.sendbuf = parep_mpi_malloc(count*size);
		memcpy((curargs->args).reduce.sendbuf,sendbuf,count*size);
		(curargs->args).reduce.count = count;
		(curargs->args).reduce.dt = datatype;
		(curargs->args).reduce.op = op;
		(curargs->args).reduce.root = root;
		(curargs->args).reduce.comm = comm;
		if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL) {
			assert((myrank == root) && (cmpToRepMap[myrank] == -1));
		}
		parep_mpi_free(curargs->req);
		curargs->req = req;
		parep_mpi_collective_id++;
		pthread_mutex_unlock(&collectiveLock);
	} else {
		pthread_mutex_unlock(&recvDataListLock);
		curargs = (struct collective_data *)parep_mpi_malloc(sizeof(struct collective_data));
		(curargs->args).reduce.alloc_recvbuf = true;
		(curargs->args).reduce.recvbuf = parep_mpi_malloc((count+extracount)*size);
		memcpy((curargs->args).reduce.recvbuf + dis,&parep_mpi_collective_id,sizeof(int));
	
		pthread_rwlock_rdlock(&commLock);
		ftreq->comm = comm;
		
		if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
			EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
			EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
			
			if(myrank == root) {
				completecmp = true;
				retVal = EMPI_Irecv ((curargs->args).reduce.recvbuf, count+extracount, datatype->edatatype, myrank, MPI_FT_REDUCE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), ftreq->reqrep);
			} else {
				completecmp = true;
				completerep = true;
			}
		} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
			else commnR = 0;
			
			retVal = EMPI_Ireduce (sendbuf, recvbuf, count, datatype->edatatype, op->eop, root, (comm->EMPI_COMM_CMP), ftreq->reqcmp);
			if((cmpToRepMap[myrank] == -1) || (myrank != root)) completerep = true;
		}
		pthread_rwlock_unlock(&commLock);
		
		(curargs->args).reduce.sendbuf = parep_mpi_malloc(count*size);
		memcpy((curargs->args).reduce.sendbuf,sendbuf,count*size);
		(curargs->args).reduce.count = count;
		(curargs->args).reduce.dt = datatype;
		(curargs->args).reduce.op = op;
		(curargs->args).reduce.root = root;
		(curargs->args).reduce.comm = comm;
		curargs->req = req;
		curargs->type = MPI_FT_REDUCE;
		curargs->completecmp = completecmp;
		curargs->completerep = completerep;
		curargs->completecolls = NULL;
		curargs->num_colls = 0;
		
		pthread_mutex_lock(&collectiveLock);
		curargs->id = parep_mpi_collective_id;
		if(last_collective == NULL) {
			curargs->prev = NULL;
			curargs->next = NULL;
			last_collective = curargs;
		} else {
			clcdata *temp = last_collective;
			while((curargs->id <= temp->id) && (temp->next != NULL)) {
				temp = temp->next;
			}
			if(curargs->id > temp->id) {
				curargs->prev = temp->prev;
				curargs->next = temp;
				if(temp->prev != NULL) temp->prev->next = curargs;
				else last_collective = curargs;
				temp->prev = curargs;
			}
			else {
				curargs->prev = temp;
				curargs->next = NULL;
				temp->next = curargs;
			}
		}
		parep_mpi_collective_id++;
		pthread_cond_signal(&collectiveCond);
		pthread_mutex_unlock(&collectiveLock);
	}
	(ftreq)->storeloc = curargs;
	*req = ftreq;
	(*req)->rnode = reqListInsert(*req);
	
	pthread_cond_signal(&reqListCond);
	pthread_mutex_unlock(&reqListLock);
	
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return MPI_SUCCESS;
}

/* MPI_Allreduce */
int MPI_Allreduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}

	MPI_Request req;
	retVal = MPI_Iallreduce(sendbuf,recvbuf,count,datatype,op,comm,&req);
	retVal = MPI_Wait(&req,MPI_STATUS_IGNORE);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return retVal;
}

int MPI_Iallreduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *req) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	MPI_Request ftreq;
	int myrank;
	int size;
	int commnC,commnR;
	struct collective_data *curargs;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	parep_mpi_sighandling_state = 1;
	ftreq = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
	(ftreq)->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqcolls = NULL;
	(ftreq)->num_reqcolls = 0;
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	ftreq->bufloc = recvbuf;
	ftreq->complete = false;
	ftreq->status.count = count;
	ftreq->status.MPI_SOURCE = myrank;
	ftreq->status.MPI_ERROR = 0;
	ftreq->type = MPI_FT_COLLECTIVE_REQUEST;
	
	bool completecmp = false;
	bool completerep = false;
	
	int extracount;
	int dis;
	EMPI_Type_size(datatype->edatatype,&size);
	if(size >= sizeof(int)) extracount = 1;
	else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
	else extracount = (((int)sizeof(int))/size) + 1;
	dis = ((count+extracount)*size) - sizeof(int);
	
	pthread_mutex_lock(&parep_mpi_store_buf_sz_mutex);
	if(parep_mpi_store_buf_sz + (2*count*size) >= 0x40000000) {
		pthread_mutex_lock(&rem_recv_msg_sent_mutex);
		if(rem_recv_msg_sent == 0) {
			int dyn_sock;
			int ret;
			dyn_sock = socket(AF_INET, SOCK_STREAM, 0);
			do {
				ret = connect(dyn_sock,(struct sockaddr *)(&parep_mpi_dyn_coordinator_addr),sizeof(parep_mpi_dyn_coordinator_addr));
			} while(ret != 0);
			int cmd = CMD_REM_RECV;
			write(dyn_sock,&cmd,sizeof(int));
			close(dyn_sock);
			rem_recv_msg_sent = 1;
		}
		pthread_mutex_unlock(&rem_recv_msg_sent_mutex);
	}
	while(parep_mpi_store_buf_sz + (2*count*size) >= 0x80000000) {
		pthread_mutex_unlock(&parep_mpi_store_buf_sz_mutex);
		pthread_mutex_lock(&reqListLock);
		pthread_rwlock_rdlock(&commLock);
		for(int source = 0; source < nC; source++) {
			if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
				if(cmpToRepMap[source] == -1) probe_msg_from_source(comm, source);
				else probe_msg_from_source(comm, cmpToRepMap[source]+nC);
			} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
				probe_msg_from_source(comm, source);
			}
		}
		probe_reduce_messages_with_comm(comm);
		pthread_rwlock_unlock(&commLock);
		pthread_mutex_unlock(&reqListLock);
		pthread_mutex_lock(&parep_mpi_store_buf_sz_mutex);
		//pthread_cond_wait(&parep_mpi_store_buf_sz_cond,&parep_mpi_store_buf_sz_mutex);
	}
	pthread_mutex_unlock(&parep_mpi_store_buf_sz_mutex);
	
	pthread_mutex_lock(&reqListLock);
	
	/*pthread_rwlock_rdlock(&commLock);
	probe_reduce_messages_with_comm(comm);
	pthread_rwlock_unlock(&commLock);*/
	
	pthread_mutex_lock(&recvDataListLock);
	recvDataNode *rnode = recvDataRedListFind(parep_mpi_collective_id,MPI_FT_ALLREDUCE_TEMP);
	if(rnode != NULL) {
		curargs = (clcdata *)rnode->pdata;
		recvDataRedListDelete(rnode);
		pthread_cond_signal(&recvDataListCond);
		pthread_mutex_unlock(&recvDataListLock);
		memcpy(ftreq->bufloc,curargs->args.allreduce.recvbuf,count*size);
		pthread_mutex_lock(&parep_mpi_store_buf_sz_mutex);
		parep_mpi_store_buf_sz = parep_mpi_store_buf_sz + (2*count*size);
		pthread_mutex_unlock(&parep_mpi_store_buf_sz_mutex);
		pthread_mutex_lock(&collectiveLock);
		curargs->type = MPI_FT_ALLREDUCE;
		curargs->completecmp = true;
		curargs->completerep = true;
		(curargs->args).allreduce.sendbuf = parep_mpi_malloc(count*size);
		memcpy((curargs->args).allreduce.sendbuf,sendbuf,count*size);
		(curargs->args).allreduce.count = count;
		(curargs->args).allreduce.dt = datatype;
		(curargs->args).allreduce.op = op;
		(curargs->args).allreduce.comm = comm;
		if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL) {
			assert((cmpToRepMap[myrank] == -1));
		}
		parep_mpi_free(curargs->req);
		curargs->req = req;
		parep_mpi_collective_id++;
		pthread_mutex_unlock(&collectiveLock);
	} else {
		pthread_mutex_unlock(&recvDataListLock);
		curargs = (struct collective_data *)parep_mpi_malloc(sizeof(struct collective_data));
		(curargs->args).allreduce.alloc_recvbuf = true;
		(curargs->args).allreduce.recvbuf = parep_mpi_malloc((count+extracount)*size);
		memcpy((curargs->args).allreduce.recvbuf + dis,&parep_mpi_collective_id,sizeof(int));
		
		pthread_rwlock_rdlock(&commLock);
		ftreq->comm = comm;
		
		if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL) {
			EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
			EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
			completecmp = true;
			retVal = EMPI_Irecv ((curargs->args).allreduce.recvbuf, count+extracount, datatype->edatatype, myrank, MPI_FT_ALLREDUCE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), ftreq->reqrep);
		} else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL) {
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
			else commnR = 0;
			
			retVal = EMPI_Iallreduce (sendbuf, recvbuf, count, datatype->edatatype, op->eop, (comm->EMPI_COMM_CMP), ftreq->reqcmp);
			if(cmpToRepMap[myrank] == -1) completerep = true;
		}
		pthread_rwlock_unlock(&commLock);
		
		(curargs->args).allreduce.sendbuf = parep_mpi_malloc(count*size);
		memcpy((curargs->args).allreduce.sendbuf,sendbuf,count*size);
		(curargs->args).allreduce.count = count;
		(curargs->args).allreduce.dt = datatype;
		(curargs->args).allreduce.op = op;
		(curargs->args).allreduce.comm = comm;
		curargs->req = req;
		curargs->type = MPI_FT_ALLREDUCE;
		curargs->completecmp = completecmp;
		curargs->completerep = completerep;
		curargs->completecolls = NULL;
		curargs->num_colls = 0;
		
		pthread_mutex_lock(&parep_mpi_store_buf_sz_mutex);
		parep_mpi_store_buf_sz = parep_mpi_store_buf_sz + (2*count*size);
		pthread_mutex_unlock(&parep_mpi_store_buf_sz_mutex);
		
		pthread_mutex_lock(&collectiveLock);
		curargs->id = parep_mpi_collective_id;
		if(last_collective == NULL) {
			curargs->prev = NULL;
			curargs->next = NULL;
			last_collective = curargs;
		} else {
			clcdata *temp = last_collective;
			while((curargs->id <= temp->id) && (temp->next != NULL)) {
				temp = temp->next;
			}
			if(curargs->id > temp->id) {
				curargs->prev = temp->prev;
				curargs->next = temp;
				if(temp->prev != NULL) temp->prev->next = curargs;
				else last_collective = curargs;
				temp->prev = curargs;
			}
			else {
				curargs->prev = temp;
				curargs->next = NULL;
				temp->next = curargs;
			}
		}
		parep_mpi_collective_id++;
		pthread_cond_signal(&collectiveCond);
		pthread_mutex_unlock(&collectiveLock);
	}
	(ftreq)->storeloc = curargs;
	*req = ftreq;
	(*req)->rnode = reqListInsert(*req);
	
	pthread_cond_signal(&reqListCond);
	pthread_mutex_unlock(&reqListLock);
	
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return MPI_SUCCESS;
}

int MPI_Barrier(MPI_Comm comm) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}

	MPI_Request req;
	retVal = MPI_Ibarrier(comm,&req);
	retVal = MPI_Wait(&req,MPI_STATUS_IGNORE);
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return retVal;
}

int MPI_Ibarrier(MPI_Comm comm, MPI_Request *req) {
	bool int_call;
	int retVal;
	
	if(pthread_self() == thread_tid[0]) {
		int_call = parep_mpi_internal_call;
		parep_mpi_internal_call = true;
	}
	
	MPI_Request ftreq;
	int myrank;
	int size;
	int commnC,commnR;
	struct collective_data *curargs;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	parep_mpi_sighandling_state = 1;
	ftreq = (MPI_Request)parep_mpi_malloc(sizeof(struct mpi_ft_request));
	(ftreq)->reqcmp = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqrep = (EMPI_Request *)parep_mpi_malloc(sizeof(EMPI_Request));
	(ftreq)->reqcolls = NULL;
	(ftreq)->num_reqcolls = 0;
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	ftreq->complete = false;
	ftreq->status.MPI_SOURCE = myrank;
	ftreq->status.MPI_ERROR = 0;
	ftreq->type = MPI_FT_COLLECTIVE_REQUEST;
	
	bool completecmp = false;
	bool completerep = false;
	
	pthread_mutex_lock(&reqListLock);
	
	pthread_rwlock_rdlock(&commLock);
	ftreq->comm = comm;
	
	retVal = EMPI_Ibarrier ((comm->eworldComm), ftreq->reqcmp);
	completerep = true;
	pthread_rwlock_unlock(&commLock);
	
	curargs = (struct collective_data *)parep_mpi_malloc(sizeof(struct collective_data));
	(curargs->args).barrier.comm = comm;
	curargs->req = req;
	curargs->type = MPI_FT_BARRIER;
	curargs->completecmp = completecmp;
	curargs->completerep = completerep;
	curargs->completecolls = NULL;
	curargs->num_colls = 0;
	
	pthread_mutex_lock(&collectiveLock);
	curargs->id = parep_mpi_collective_id;
	if(last_collective == NULL) {
		curargs->prev = NULL;
		curargs->next = NULL;
		last_collective = curargs;
	} else {
		clcdata *temp = last_collective;
		while((curargs->id <= temp->id) && (temp->next != NULL)) {
			temp = temp->next;
		}
		if(curargs->id > temp->id) {
			curargs->prev = temp->prev;
			curargs->next = temp;
			if(temp->prev != NULL) temp->prev->next = curargs;
			else last_collective = curargs;
			temp->prev = curargs;
		}
		else {
			curargs->prev = temp;
			curargs->next = NULL;
			temp->next = curargs;
		}
	}
	parep_mpi_collective_id++;
	pthread_cond_signal(&collectiveCond);
	pthread_mutex_unlock(&collectiveLock);
	
	(ftreq)->storeloc = curargs;
	*req = ftreq;
	(*req)->rnode = reqListInsert(*req);
	
	pthread_cond_signal(&reqListCond);
	pthread_mutex_unlock(&reqListLock);
	
	if(parep_mpi_sighandling_state == 2) {
		parep_mpi_sighandling_state = 0;
		parep_mpi_ckpt_wait = 1;
		pthread_kill(pthread_self(),SIGUSR1);
		while(parep_mpi_ckpt_wait) {;}
	}
	parep_mpi_sighandling_state = 0;
	
	if(pthread_self() == thread_tid[0]) parep_mpi_internal_call = int_call;
	
	return MPI_SUCCESS;
}

double MPI_Wtime() {
  return EMPI_Wtime();
}

int MPI_Abort(MPI_Comm comm, int errorcode) {
	return EMPI_Abort(comm->eworldComm,errorcode);
}