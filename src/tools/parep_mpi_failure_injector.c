#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>

#define MAX_FILENAME_LEN 256
#define DYN_COORDINATOR_PORT 2582
#define CMD_INFORM_PREDICT 30

#define ACT_PRED 0
#define ACT_KILL 1
#define ACT_TRIG 2

typedef struct {
	pid_t first;
	char second[MAX_FILENAME_LEN];
	double ttk;
	double ttp;
} IntStringPair;

void copyIntStringPair(IntStringPair *dest, IntStringPair *src) {
	dest->first = src->first;
	strcpy(dest->second,src->second);
	dest->ttk = src->ttk;
	dest->ttp = src->ttp;
}

struct ActionList{
	IntStringPair *relpair;
	int acttype;
	double trigger;
	struct ActionList *next;
};

typedef struct ActionList ActList;

ActList *actListHead = NULL;

void clearActList() {
	while(actListHead != NULL) {
		ActList *temp = actListHead;
		actListHead = actListHead->next;
		free(temp);
	}
}

typedef struct {
	int value;
	double probability;
} IntProbabilityPair;

double normal_pdf(int x, double mean, double variance) {
	double stddev = sqrt(variance);
	double coeff = 1.0 / (stddev * sqrt(2 * M_PI));
	double exponent = exp(-pow(x - mean, 2) / (2 * variance));
	return coeff * exponent;
}

int check(int exp, const char *msg) {
	if (exp == -1) {
		perror(msg);
		exit(1);
	}
	return exp;
}

int wrap_around(int x, int N) {
	if (x < 0) {
		return (x + N) % (N);  // Wrap negatives to the upper part
	}
	return x % (N);  // Wrap around when exceeding N
}

int compare(const void *a, const void *b) {
	IntProbabilityPair *pairA = (IntProbabilityPair *)a;
	IntProbabilityPair *pairB = (IntProbabilityPair *)b;
	if (pairA->probability < pairB->probability) return 1;
	else if (pairA->probability > pairB->probability) return -1;
	else return 0;
}

double generate_normal_random(double mean, double variance) {
	double u1 = rand() / (double)RAND_MAX;
	double u2 = rand() / (double)RAND_MAX;

	// Box-Muller transform
	double z0 = sqrt(-2.0 * log(u1)) * cos(2 * M_PI * u2);
	double stddev = sqrt(variance);
	return z0 * stddev + mean;
}

int generate_random_int(double mean, double variance) {
	return (int)round(generate_normal_random(mean, variance));
}

void fisher_yates_shuffle(IntStringPair *array, int size) {
	for (int i = size - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		// Swap array[i] and array[j]
		IntStringPair temp;
		copyIntStringPair(&temp,&(array[i]));
		copyIntStringPair(&(array[i]),&(array[j]));
		copyIntStringPair(&(array[j]),&temp);
	}
}

double weibull_distribution(double shape, double scale) {
	double rand_num = (double)rand() / RAND_MAX;
	return scale * pow(-log(1 - rand_num), 1 / shape);
}

int uni_rand(int min, int max) {
	return min + (rand() % (max - min));
}

bool probabCheck(double probability) {
	if (probability >= 1.0) return true;
	if (probability <= 0.0) return false;
	return ((double)rand() / RAND_MAX) < probability;
}

IntStringPair *pid_node_pairs;
volatile IntStringPair *shuffled_pairs;
volatile IntStringPair *killed_pairs;
int num_killed = 0;
double *time_between_failures;
int N;
int alive;
volatile int targpid;
volatile char targnname[MAX_FILENAME_LEN];
double gauss_mean;
double gauss_var;
double gauss_stddev;
double precision;
double recall;
IntProbabilityPair *probvals;
bool meanchanged = false;
struct sockaddr_in saddr;

double shape,scale;
int failindexstart = 0;
int failindexend = -1;

volatile sig_atomic_t sigusr1_recvd = 0;
volatile sig_atomic_t sigstate = 0;

void inform_server(ActList *act) {
	if(getenv("PAREP_MPI_SIM_PREDICT") != NULL) {
		if(!strcmp(getenv("PAREP_MPI_SIM_PREDICT"),"1")) {
			int dyn_sock;
			check((dyn_sock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
			int ret;
			do {
				ret = connect(dyn_sock,(struct sockaddr *)(&saddr),sizeof(saddr));
			} while(ret != 0);
			int cmd = CMD_INFORM_PREDICT;
			write(dyn_sock,&cmd,sizeof(int));
			write(dyn_sock,&(act->relpair->first),sizeof(pid_t));
			write(dyn_sock,&(act->relpair->second),MAX_FILENAME_LEN);
			close(dyn_sock);
		}
	}
}

void create_kill_acts(IntStringPair *array, int size) {
	assert(actListHead == NULL);
	ActList *temp = actListHead;
	for(int i = 0; i < size; i++) {
		ActList *act = (ActList *)malloc(sizeof(ActList));
		act->relpair = &(array[i]);
		act->acttype = ACT_KILL;
		act->trigger = -1;
		act->next = NULL;
		if(temp == NULL) {
			actListHead = act;
		} else {
			temp->next = act;
		}
		temp = act;
	}
}

void create_pred_acts(IntStringPair *array, int size) {
	failindexstart = (failindexend+1);
	int index = failindexstart;
	double tottime = 0;
	int tp = 0;
	int fp = 0;
	int nfail = 0;
	tottime += array[index].ttk;
	while(tottime < 1800) {
		nfail++;
		bool predict = probabCheck(recall);
		if(predict) {
			double lead_time = (double)uni_rand(60, 120);
			if(array[index].ttk - lead_time >= 0) {
				array[index].ttp = array[index].ttk - lead_time;
				array[index].ttk = lead_time;
			} else {
				array[index].ttp = 0;
				array[index].ttk = lead_time;
			}
			tp++;
			index++;
			tottime += (array[index].ttp + array[index].ttk);
		} else {
			index++;
			tottime += array[index].ttk;
		}
	}
	failindexend = index-1;
	double fracfp = (((double)tp)/precision) - (double)tp;
	double fracprobab = fracfp - ((double)(floor(fracfp)));
	bool roundup = probabCheck(fracprobab);
	if(roundup) fp = ceil(fracfp);
	else fp = floor(fracfp);
	for(index = failindexstart; index <= failindexend; index++) {
		if(array[index].ttp >= 0) {
			ActList *temp = actListHead;
			ActList *prev = NULL;
			while(temp != NULL) {
				if((temp->relpair) == &(array[index])) {
					break;
				}
				prev = temp;
				temp = temp->next;
			}
			assert(temp != NULL);
			ActList *act = (ActList *)malloc(sizeof(ActList));
			act->relpair = &(array[index]);
			act->acttype = ACT_PRED;
			act->trigger = -1;
			if(prev != NULL) {
				prev->next = act;
			} else {
				actListHead = act;
			}
			act->next = temp;
		}
	}
	for(int fpcount=0; fpcount < fp; fpcount++) {
		int targindex = uni_rand(failindexend+1,size);
		array[targindex].ttp = uni_rand(0,1800);
		ActList *temp = actListHead;
		ActList *prev = NULL;
		while(temp != NULL) {
			double tasktime;
			if(temp->acttype == ACT_PRED) tasktime = temp->relpair->ttp;
			else if(temp->acttype == ACT_KILL) tasktime = temp->relpair->ttk;
			if(array[targindex].ttp < tasktime) {
				break;
			} else {
				array[targindex].ttp -= tasktime;
			}
			prev = temp;
			temp = temp->next;
		}
		ActList *act = (ActList *)malloc(sizeof(ActList));
		act->relpair = &(array[targindex]);
		act->acttype = ACT_PRED;
		act->trigger = -1;
		if(prev != NULL) {
			prev->next = act;
		} else {
			actListHead = act;
		}
		act->next = temp;
		if(temp != NULL) {
			if(temp->acttype == ACT_PRED) temp->relpair->ttp -= array[targindex].ttp;
			else if(temp->acttype == ACT_KILL) temp->relpair->ttk -= array[targindex].ttp;
		}
	}
	double trigtime = 1800;
	ActList *temp = actListHead;
	ActList *prev = NULL;
	while(temp != NULL) {
		double tasktime;
		if(temp->acttype == ACT_PRED) tasktime = temp->relpair->ttp;
		else if(temp->acttype == ACT_KILL) tasktime = temp->relpair->ttk;
		if(trigtime < tasktime) {
			break;
		} else {
			trigtime -= tasktime;
		}
		prev = temp;
		temp = temp->next;
	}
	ActList *act = (ActList *)malloc(sizeof(ActList));
	act->relpair = NULL;
	act->acttype = ACT_TRIG;
	act->trigger = trigtime;
	if(prev != NULL) {
		prev->next = act;
	} else {
		actListHead = act;
	}
	act->next = temp;
	if(temp != NULL) {
		if(temp->acttype == ACT_PRED) temp->relpair->ttp -= trigtime;
		else if(temp->acttype == ACT_KILL) temp->relpair->ttk -= trigtime;
	}
	printf("Act list prepared nfail %d tp %d fp %d precision %f recall %f\n", nfail,tp,fp,precision,recall);
	fflush(stdout);
}

void sigusr1_handler(int signum) {
	if(sigstate == 1) {
		sigstate = 2;
		return;
	}
	if(sigstate == 0) {
		sigstate = 3;
	} else {
		return;
	}
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &mask, NULL);
	printf("%d: Failure injector got SIGUSR1 sigusr1_recvd %d\n",getpid(),sigusr1_recvd);
	fflush(stdout);
	if(sigusr1_recvd == 1) return;
	sigusr1_recvd = 1;
	char directory[MAX_FILENAME_LEN];
	strcpy(directory, getenv("PAREP_MPI_WORKDIR"));
	char file_name[MAX_FILENAME_LEN];
	char dfile_name[MAX_FILENAME_LEN];
	sprintf(dfile_name,"%s/parep_mpi_pids_done",directory);
	sprintf(file_name,"%s/parep_mpi_pids",directory);
	while (access(dfile_name, F_OK) == -1) {
		usleep(100000); // sleep for 100 milliseconds
	}
	int pid;
	FILE *file = fopen(file_name, "r");
	for (int i = 0; i < N; i++) {
		fscanf(file, "%d\n", &pid);
		pid_node_pairs[i].first = pid;
		pid_node_pairs[i].ttk = weibull_distribution(shape, scale);
		if(pid_node_pairs[i].ttk < 150) pid_node_pairs[i].ttk = 150;
		pid_node_pairs[i].ttp = -1;
	}
	fclose(file);
	remove(file_name);
	remove(dfile_name);
	
	failindexstart = 0;
	failindexend = -1;
	for (int i = 0; i < N; i++) {
		copyIntStringPair((IntStringPair *)(&(shuffled_pairs[i])),(IntStringPair *)(&(pid_node_pairs[i])));
	}
	fisher_yates_shuffle((IntStringPair *)shuffled_pairs, N);
	clearActList();
	create_kill_acts((IntStringPair *)shuffled_pairs, N);
	if(getenv("PAREP_MPI_SIM_PREDICT") != NULL) {
		if(!strcmp(getenv("PAREP_MPI_SIM_PREDICT"),"1")) {
			create_pred_acts((IntStringPair *)shuffled_pairs, N);
		}
	}
	
	sigusr1_recvd = 0;
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	assert(sigstate == 3);
	sigstate = 0;
}

int main(int argc, char **argv) {
	N = atoi(getenv("PAREP_MPI_SIZE")); // number of processes
	alive = N; // number of alive processes
	gauss_stddev = 1;
	gauss_var = gauss_stddev*gauss_stddev;
	precision = 1;
	recall = 1;
	if(getenv("PAREP_MPI_PREDICT_PRECISION") != NULL) {
		double pre = atof(getenv("PAREP_MPI_PREDICT_PRECISION"));
		precision = pre;
	}
	if(getenv("PAREP_MPI_PREDICT_RECALL") != NULL) {
		double rec = atof(getenv("PAREP_MPI_PREDICT_RECALL"));
		recall = rec;
	}
	double mean = -1; // mean time between failure in seconds
	if(getenv("PAREP_MPI_MTBF")) mean = atof(getenv("PAREP_MPI_MTBF"));
	//double shape = 1; // shape parameter for Weibull distribution
	shape = 0.7;
	double lead_time;
	if(mean != -1) scale = mean / (tgamma(1 + (1/shape))); // scale parameter for Weibull distribution

	struct timespec start_time;
	struct timespec end_time;
	struct timespec elap_time;

	struct sigaction sa;
	sa.sa_handler = sigusr1_handler;
	sigaction(SIGUSR1, &sa, NULL);
	
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
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(DYN_COORDINATOR_PORT);
	saddr.sin_addr.s_addr = inet_addr(IP_address);

	srand(time(NULL));

	// Read PIDs and node names from files
	pid_node_pairs = (IntStringPair *)malloc(N*sizeof(IntStringPair));
	shuffled_pairs = (IntStringPair *)malloc(N*sizeof(IntStringPair));
	
	char directory[MAX_FILENAME_LEN];
	strcpy(directory, getenv("PAREP_MPI_WORKDIR"));
	char nfile_name[MAX_FILENAME_LEN];
	sprintf(nfile_name,"%s/parep_mpi_nodes",directory);
	char node_name[MAX_FILENAME_LEN];
	char file_name[MAX_FILENAME_LEN];
	char dfile_name[MAX_FILENAME_LEN];
	sprintf(dfile_name,"%s/parep_mpi_pids_done",directory);
	sprintf(file_name,"%s/parep_mpi_pids",directory);
	while (access(dfile_name, F_OK) == -1) {
		usleep(100000); // sleep for 100 milliseconds
	}
	int pid;
	FILE *node_file = fopen(nfile_name, "r");
	FILE *file = fopen(file_name, "r");
	for (int i = 0; i < N; i++) {
		fscanf(file, "%d\n", &pid);
		fscanf(node_file, "%s\n", node_name);
		pid_node_pairs[i].first = pid;
		strcpy(pid_node_pairs[i].second, node_name);
		pid_node_pairs[i].ttk = weibull_distribution(shape, scale);
		if(pid_node_pairs[i].ttk < 150) pid_node_pairs[i].ttk = 150;
		pid_node_pairs[i].ttp = -1;
	}
	fclose(file);
	fclose(node_file);
	remove(file_name);
	remove(dfile_name);

	if(mean == -1) {
		pause();
	}

	// Shuffle pids and node names
	for (int i = 0; i < N; i++) {
		copyIntStringPair((IntStringPair *)(&(shuffled_pairs[i])),(IntStringPair *)(&(pid_node_pairs[i])));
	}

	// Shuffle using Fisher-Yates algorithm
	fisher_yates_shuffle((IntStringPair *)shuffled_pairs, N);
	
	clearActList();
	create_kill_acts((IntStringPair *)shuffled_pairs, N);
	if(getenv("PAREP_MPI_SIM_PREDICT") != NULL) {
		if(!strcmp(getenv("PAREP_MPI_SIM_PREDICT"),"1")) {
			create_pred_acts((IntStringPair *)shuffled_pairs, N);
		}
	}
	
	ActList *curact = actListHead;
	while(curact != NULL) {
		double tasktime;
		if(curact->acttype == ACT_PRED) tasktime = curact->relpair->ttp;
		else if(curact->acttype == ACT_KILL) tasktime = curact->relpair->ttk;
		else if(curact->acttype == ACT_TRIG) tasktime = curact->trigger;
		struct pollfd pfd;
		pfd.fd = -1;
		pfd.events = 0;
		pfd.revents = 0;
		struct timespec wtime;
		double fractpart, intpart;
		fractpart = modf(tasktime,&intpart);
		wtime.tv_sec = (long)intpart;
		wtime.tv_nsec = (long)(fractpart*1000000000);
		int pout = -1;
		if(sigstate == 2) {
			sigstate = 0;
			sigusr1_recvd = 2;
			raise(SIGUSR1);
			while(sigusr1_recvd);
			curact = actListHead;
			continue;
		}
		
		if((curact->acttype == ACT_PRED) || (curact->acttype == ACT_KILL)) printf("Starting timer for process %d on node %s: type %d tasktime %f\n",curact->relpair->first,curact->relpair->second,curact->acttype,tasktime);
		else printf("Starting timer for Trigger: type %d tasktime %f\n",curact->acttype,tasktime);
		fflush(stdout);
		
		sigstate = 0;
		bool restarted = false;
		do {
			clock_gettime(CLOCK_MONOTONIC, &start_time);
			pout = ppoll(&pfd,1,&wtime,NULL);
			if(pout == -1 && errno == EINTR) {
				restarted = true;
				break;
			}
		} while(pout == -1 && errno == EINTR);
		if(restarted) {
			curact = actListHead;
			continue;
		}
		assert(pout == 0);
		int trigger = 0;
		if(curact->acttype == ACT_KILL) {
			char kill_command[MAX_FILENAME_LEN];
			int ret;
			printf("Process %d on node %s: Kill TTK %f\n",curact->relpair->first,curact->relpair->second,curact->relpair->ttk);
			fflush(stdout);
			sprintf(kill_command, "ssh %s kill -9 %d", curact->relpair->second, curact->relpair->first);
			do {
				ret = system(kill_command);
				printf("Failure injector system returned %d\n",ret);
				fflush(stdout);
			} while(ret != 0);
		} else if(curact->acttype == ACT_PRED) {
			sigstate = 1;
			printf("Process %d on node %s: Predict TTP %f\n",curact->relpair->first,curact->relpair->second,curact->relpair->ttp);
			fflush(stdout);
			inform_server(curact);
			if(sigstate == 2) {
				sigstate = 0;
				sigusr1_recvd = 2;
				raise(SIGUSR1);
				while(sigusr1_recvd);
				curact = actListHead;
				continue;
			}
			sigstate = 0;
		} else if(curact->acttype == ACT_TRIG) {
			trigger = 1;
		}
		actListHead = actListHead->next;
		free(curact);
		if(trigger) {
			create_pred_acts((IntStringPair *)shuffled_pairs, N);
		}
		curact = actListHead;
	}
	return 0;
}
