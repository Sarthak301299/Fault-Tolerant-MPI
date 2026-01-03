#ifndef PAREP_MPI_CKPT_COORDINATOR
#define PAREP_MPI_CKPT_COORDINATOR

#include <sys/time.h>
#include <sys/resource.h>

#define BUFFER_SIZE 256
#define CMD_STORE 0
#define CMD_QUERY 1
#define CMD_GET_PD_ID 2
#define CMD_GET_RKEY 3
#define CMD_CLEAR_STORE 4
#define CMD_EXIT 5
#define CMD_STORE_NO_PROP 6
#define CMD_WAITPID_OUTPUT 7
#define CMD_INFORM_EXIT 8
#define CMD_PROC_STATE_UPDATE 9
#define CMD_INFORM_PROC_FAILED 10
#define CMD_RPROC_TERM_ACK 11
#define CMD_EXIT_CALLED 12
#define CMD_SHRINK_PERFORMED 13
#define CMD_WAITPID_DATA 14
#define CMD_MISC_PROC_FAILED 15
#define CMD_CREATE_CKPT 16
#define CMD_CKPT_CREATED 17
#define CMD_MPI_INITIALIZED 18
#define CMD_BARRIER 19
#define CMD_REM_RECV 20
#define CMD_REM_RECV_FINISHED 21
#define CMD_GET_ALL_RKEYS 22
#define CMD_REMOTE_BARRIER 23
#define CMD_STORE_MULTIPLE 24
#define CMD_STORE_MULTIPLE_NO_PROP 25
#define CMD_GET_ALL_QPS 26
#define CMD_GET_ALL_LIDS 27
#define CMD_INFORM_BARRIER_RUNNING 28
#define CMD_INFORM_FINALIZE_REACHED 29
#define CMD_INFORM_PREDICT 30
#define CMD_BLOCK_PREDICT 31
#define CMD_CHECK_NODE_FAIL 32
#define CMD_GET_NGROUP_LEADER 33
#define CMD_INFORM_PREDICT_NODE 34

int PAREP_MPI_NODE_GROUP_SIZE_MAX = 32;

#define RET_COMPLETED 101
#define RET_INCOMPLETE 102

struct node {
	struct node *next;
	int *client_socket;
};
typedef struct node node_t;

struct qnode {
	struct qnode *next;
	int *client_socket;
};
typedef struct qnode qnode_t;

struct propnode {
	struct propnode *next;
	void *buffer;
	size_t size;
};
typedef struct propnode propnode_t;

struct lid_kval_pair {
	uint16_t virt_lid;
	uint16_t real_lid;
	struct lid_kval_pair *next;
};
typedef struct lid_kval_pair lid_pair_t;

struct rkey_kval_pair {
	uint32_t virt_rkey;
	uint32_t pd_id;
	uint32_t real_rkey;
	struct rkey_kval_pair *next;
};
typedef struct rkey_kval_pair rkey_pair_t;

struct qp_kval_pair {
	uint32_t virt_qp_num;
	uint32_t real_qp_num;
	uint32_t pd_id;
	struct qp_kval_pair *next;
};
typedef struct qp_kval_pair qp_pair_t;

struct rank_limits {
	int start;
	int end;
};
typedef struct rank_limits rlims;

enum proc_state {
	PROC_UNINITIALIZED,
	PROC_RUNNING,
	PROC_TERMINATED
};

//#define SERVER_BACKLOG 128
#define SERVER_BACKLOG 512
#define COORDINATOR_DPORT 2579
#define COORDINATOR_PORT 2580
#define EMPI_PORT 2581
#define DYN_COORDINATOR_PORT 2582
#define COORDINATOR_GROUP_PORT 2583
#define SOCKETERROR (-1)

#define THREAD_POOL_SIZE 8
#define QTHREAD_POOL_SIZE 0
#define PROPTHREAD_POOL_SIZE 0

#define LID_HASH_KEYS 10
#define RKEY_HASH_KEYS 20
#define QP_HASH_KEYS 10

uint16_t lid_lims[2];
uint32_t qp_num_lims[2];
uint32_t pd_id_lims[2];
uint32_t rkey_lims[2];

uint16_t current_lid = 0;
uint32_t current_qp_num = 0;
uint32_t current_pd_id = 0;
uint32_t current_rkey = 0;

node_t *head = NULL;
node_t *tail = NULL;

qnode_t *qhead = NULL;
qnode_t *qtail = NULL;

propnode_t *prophead = NULL;
propnode_t *proptail = NULL;

lid_pair_t *lid_map[LID_HASH_KEYS];
rkey_pair_t *rkey_map[RKEY_HASH_KEYS];
qp_pair_t *qp_map[QP_HASH_KEYS];

pthread_t main_thread;
pthread_t empi_thread;
pthread_t empi_exec_thread;
pthread_t exit_thread;
pthread_t server_poller;
pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_t qthread_pool[QTHREAD_POOL_SIZE];
pthread_t propthread_pool[PROPTHREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

pthread_mutex_t qmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t qcondition_var = PTHREAD_COND_INITIALIZER;

pthread_mutex_t propmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t propcondition_var = PTHREAD_COND_INITIALIZER;

pthread_mutex_t lid_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qp_num_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pd_id_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rkey_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t lid_pair_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rkey_pair_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qp_pair_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t lid_pair_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t rkey_pair_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t qp_pair_cond = PTHREAD_COND_INITIALIZER;

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void enqueue (int *);
void qenqueue(int *);
void propenqueue(void *,size_t);
int *dequeue();
qnode_t *qdequeue();
propnode_t *propdequeue();

int lid_hash(uint16_t);
int rkey_hash(uint32_t, uint32_t);
int qp_hash(uint32_t);

void parep_mpi_ckpt_coordinator_init();

void *polling_server(void *);

void perform_propagate(void *, size_t);

void *handle_connection(void *p_client_socket);
void handle_waitpid_output(pid_t *, int);
int check(int,const char *);
void *thread_function(void *);
void *empi_thread_function(void *);
void *qthread_function(void *);
void *propthread_function(void *);
void *ethread_function(void *);

void get_rank_from_pid(pid_t, int *, int *);

uint16_t lid_add_pair(uint16_t, uint16_t);
uint16_t *lid_add_pair_multiple(uint16_t *, uint16_t *, int);
uint16_t lid_get_pair(uint16_t);
void lid_clean_maps();

uint32_t rkey_add_pair(uint32_t, uint32_t, uint32_t);
uint32_t rkey_get_pair(uint32_t, uint32_t);
void rkey_clean_maps();

uint32_t qp_add_pair(uint32_t, uint32_t, uint32_t);
uint32_t *qp_add_pair_multiple(uint32_t *, uint32_t *, uint32_t *, int);
uint32_t qp_get_pair(uint32_t, uint32_t *);
void qp_clean_maps();

int perform_local_barrier(int, bool *);

void handle_store_cmd(int, bool);
void handle_store_multiple_cmd(int, bool);
void handle_query_cmd(int);
void handle_get_pd_id(int);
void handle_get_rkey(int);
void handle_clear_store(int);
void wait_process_end();
void perform_cleanup();
void handle_inform_exit_cmd(int);
void handle_exit_cmd(int);
void handle_proc_state_update(int);
void handle_rproc_term_ack(int);
void handle_shrink_performed(int);
void handle_ckpt_created(int);
void handle_barrier(int);
void handle_remote_barrier(int);
void handle_rem_recv(int);
void handle_rem_recv_finished(int);
void handle_get_all_rkeys(int);
void handle_get_all_qps(int);
void handle_get_all_lids(int);
void handle_inform_barrier_running(int);
void handle_inform_finalize_reached(int);
void handle_inform_predict(int);
void handle_block_predict(int);

SA_IN srvaddr;
int parep_mpi_size;
int parep_mpi_node_id;
int parep_mpi_node_num;
int parep_mpi_node_size;
int parep_mpi_node_group_num;
int parep_mpi_node_group_id;
int parep_mpi_node_group_nodeid;
int parep_mpi_node_group_size;
int parep_mpi_node_group_nodesize;
int *parep_mpi_node_sizes;
int *parep_mpi_node_group_sizes;
int *parep_mpi_node_group_ids;
int *parep_mpi_node_group_nodeids;
int *parep_mpi_node_group_nodesizes;
pid_t *parep_mpi_pids;
pid_t *parep_mpi_global_pids;
int *parep_mpi_ranks;
int *parep_mpi_all_ranks;

pid_t parep_mpi_empi_pid;
pid_t parep_mpi_empi_exec_pid = (pid_t)-1;

rlims *rank_lims_all;
rlims rank_lims;

int dyn_server_sock;
int server_socket;
int server_group_socket;
int *daemon_socket;
int daemon_client_socket;
int daemon_server_socket;
int *client_socket;
int empi_socket;
int empi_client_socket;
int empi_exec_socket;
int empi_exec_client_socket;

pthread_mutex_t client_sock_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t daemon_sock_mutex = PTHREAD_MUTEX_INITIALIZER;

int parep_mpi_reconf_ngroup = 0;
pthread_mutex_t proc_state_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t proc_state_cond = PTHREAD_COND_INITIALIZER;
int *local_proc_state;
int *daemon_state;
int *group_daemon_state;
int *global_proc_state;

pthread_mutex_t empi_waitpid_safe_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empi_waitpid_safe_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t empi_waitpid_safe_informed_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empi_waitpid_safe_informed_cond = PTHREAD_COND_INITIALIZER;
int empi_waitpid_safe = 0;
int empi_waitpid_safe_informed = 0;

pthread_mutex_t local_procs_term_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t local_procs_term_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t exit_cmd_recvd_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t exit_cmd_recvd_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t remote_procs_term_ack_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t remote_procs_term_ack_cond = PTHREAD_COND_INITIALIZER;
int local_procs_term = 0;
int exit_cmd_recvd = 0;
int remote_procs_term_ack = 0;

pthread_mutex_t ethread_created_mutex = PTHREAD_MUTEX_INITIALIZER;
int ethread_created = 0;

SA_IN *coordinator_addr;
SA_IN main_coordinator_addr;

SA_IN *group_coordinator_addr;
SA_IN group_main_coordinator_addr;

char **coordinator_name;
char *my_coordinator_name;

int stdin_pipe[2];
int **stdout_pipe;
int **stderr_pipe;

pthread_t iopoller;
void *poll_iopipes(void *);

pthread_t ckpt_thread;
void *run_ckpt_timer(void *);
int ckpt_active = 0;
pthread_mutex_t ckpt_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ckpt_cond = PTHREAD_COND_INITIALIZER;

int parep_mpi_initialized = 0;
pthread_mutex_t parep_mpi_initialized_mutex = PTHREAD_MUTEX_INITIALIZER;

int parep_mpi_fpid_created = 0;
pthread_mutex_t parep_mpi_fpid_created_mutex = PTHREAD_MUTEX_INITIALIZER;

bool isMainServer;

struct rlimit rlim_nofile;

int ckpt_interval;

int parep_mpi_completed = 0;

int parep_mpi_propagating_checking = 0;
int parep_mpi_propagating = 0;
pthread_mutex_t parep_mpi_propagating_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t parep_mpi_propagating_cond = PTHREAD_COND_INITIALIZER;

int global_failed_proc_reached_barrier = -1;
int parep_mpi_group_barrier_count = 0;
int parep_mpi_barrier_count = 0;
int parep_mpi_num_failed = 0;
pthread_mutex_t parep_mpi_barrier_count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t parep_mpi_barrier_count_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t parep_mpi_num_failed_mutex = PTHREAD_MUTEX_INITIALIZER;

bool *recv_rank;

pthread_mutex_t parep_mpi_inform_failed_mutex = PTHREAD_MUTEX_INITIALIZER;
int parep_mpi_num_inform_failed = 0;
int parep_mpi_node_group_leader_nodeid = 0;
int *parep_mpi_num_inform_failed_node;
int *parep_mpi_num_inform_failed_node_group;
int *parep_mpi_node_group_leader_nodeids;
bool *parep_mpi_inform_failed_check;
int parep_mpi_performing_barrier = 0;

int parep_mpi_barrier_running = 0;
pthread_mutex_t parep_mpi_barrier_running_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t parep_mpi_barrier_running_cond = PTHREAD_COND_INITIALIZER;

int parep_mpi_finalize_reached = 0;
pthread_mutex_t parep_mpi_finalize_reached_mutex = PTHREAD_MUTEX_INITIALIZER;

int parep_mpi_fail_ready = 0;
pthread_mutex_t parep_mpi_fail_ready_mutex = PTHREAD_MUTEX_INITIALIZER;

#define PAREP_MPI_MAX_DYN_SOCKS 512
int parep_mpi_num_dyn_socks = 0;
pthread_mutex_t parep_mpi_num_dyn_socks_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t parep_mpi_num_dyn_socks_cond = PTHREAD_COND_INITIALIZER;

int rem_recv_running = 0;
int rem_recv_recvd = 0;
pthread_mutex_t rem_recv_running_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t rem_recv_running_cond = PTHREAD_COND_INITIALIZER;

int parep_mpi_block_predict = 0;
pthread_mutex_t parep_mpi_block_predict_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t parep_mpi_block_predict_cond = PTHREAD_COND_INITIALIZER;

int parep_mpi_node_group_checked = 0;
int parep_mpi_num_nodes_failed = 0;
bool waiting_for_node_fail_detect = false;
pthread_mutex_t parep_mpi_node_group_checked_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t parep_mpi_node_group_checked_cond = PTHREAD_COND_INITIALIZER;

#endif
