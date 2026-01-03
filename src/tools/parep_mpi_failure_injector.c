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
#include <sys/file.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>

#define MAX_FILENAME_LEN 256
#define DYN_COORDINATOR_PORT 2582
#define CMD_INFORM_PREDICT 30
#define CMD_INFORM_PREDICT_NODE 34

#define ACT_PRED 0
#define ACT_KILL 1
#define ACT_TRIG 2
#define ACT_NODE_KILL 3
#define ACT_NODE_RESTART 4
#define ACT_NODE_PRED 5

#define SKIP_COUNT 0

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
	double dt;
	double origtrig;
	double origdt;
	struct ActionList *next;
};

typedef struct ActionList ActList;

ActList *actListHead = NULL;
ActList *actListTail = NULL;

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
volatile IntStringPair *node_pairs;
volatile IntStringPair *killed_pairs;
int num_killed = 0;
double *time_between_failures;
int N, node_num;
int node_kill_mode = 0;
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
bool uselogs = false;

double shape,scale;
int failindexstart = 0;
int failindexend = -1;

volatile sig_atomic_t sigusr1_recvd = 0;
volatile sig_atomic_t sigusr2_recvd = 0;
volatile sig_atomic_t sigstate = 0;

void inform_server(ActList *act) {
	if(getenv("PAREP_MPI_SIM_PREDICT") != NULL) {
		if(!strcmp(getenv("PAREP_MPI_SIM_PREDICT"),"1")) {
			if(act->acttype == ACT_PRED) {
				int dyn_sock;
				check((dyn_sock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int ret;
				do {
					ret = connect(dyn_sock,(struct sockaddr *)(&saddr),sizeof(saddr));
				} while(ret != 0);
				int cmd = CMD_INFORM_PREDICT;
				write(dyn_sock,&cmd,sizeof(int));
				write(dyn_sock,&(act->relpair->first),sizeof(pid_t));
				write(dyn_sock,(act->relpair->second),MAX_FILENAME_LEN);
				close(dyn_sock);
			} else if(act->acttype == ACT_NODE_PRED) {
				int dyn_sock;
				check((dyn_sock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket");
				int ret;
				do {
					ret = connect(dyn_sock,(struct sockaddr *)(&saddr),sizeof(saddr));
				} while(ret != 0);
				int cmd = CMD_INFORM_PREDICT_NODE;
				write(dyn_sock,&cmd,sizeof(int));
				write(dyn_sock,(act->relpair->second),MAX_FILENAME_LEN);
				close(dyn_sock);
			}
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

void create_node_kill_acts(IntStringPair *array, int size) {
	assert(actListHead == NULL);
	if(!uselogs) {
		ActList *temp = actListHead;
		for(int i = 0; i < size; i++) {
			ActList *act = (ActList *)malloc(sizeof(ActList));
			act->relpair = &(array[i]);
			act->acttype = ACT_NODE_KILL;
			act->trigger = act->relpair->ttk;
			act->dt = -1;
			act->next = NULL;
			if(temp == NULL) {
				actListHead = act;
			} else {
				temp->next = act;
			}
			temp = act;
		}
	} else {
		ActList *temp = actListHead;
		FILE *logfile;
		char logname[4096];
		char line[4096];
		sprintf(logname,"%s/logs.txt",getenv("PAREP_MPI_LOGS_SIM_PATH"));
		logfile = fopen(logname,"r");
		while(fgets(line,4096,logfile) != NULL) {
			char indexstr[4096];
			double tbf;
			double downtime;
			sscanf(line,"%[^;];%lf;%lf",indexstr,&tbf,&downtime);
			if(downtime < 0) downtime = 0;
			char *token;
			char delimiter = ',';
			bool loopedonce = false;
			token = strtok(indexstr,&delimiter);
			while(token != NULL) {
				ActList *act = (ActList *)malloc(sizeof(ActList));
				int index = atoi(token) % size;
				//if(index == 0) index++;
				act->relpair = &(array[index]);
				if(!loopedonce) {
					act->trigger = tbf;
					act->origtrig = tbf;
				}
				else {
					act->trigger = 0;
					act->origtrig = 0;
				}
				act->acttype = ACT_NODE_KILL;
				act->dt = downtime;
				act->origdt = downtime;
				loopedonce = true;
				act->next = NULL;
				if(temp == NULL) {
					actListHead = act;
				} else {
					temp->next = act;
				}
				temp = act;
				token = strtok(NULL,&delimiter);
			}
		}
		fclose(logfile);
		for(int i = 0; i < SKIP_COUNT; i++) {
			actListHead = actListHead->next;
		}
	}
}

void create_node_pred_acts(IntStringPair *array) {
	int tp = 0;
	int fp = 0;
	int nfail = 0;
	double tottime = 0;
	if(uselogs) {
		ActList *curact = actListHead;
		ActList *prev = NULL;
		while(curact != NULL) {
			nfail++;
			tottime += curact->trigger;
			bool predict = probabCheck(recall);
			if(predict) {
				double lead_time = (double)uni_rand(60, 120);
				ActList *act = (ActList *)malloc(sizeof(ActList));
				act->relpair = curact->relpair;
				act->acttype = ACT_NODE_PRED;
				act->trigger = curact->trigger - lead_time;
				if(act->trigger <= 0) act->trigger = 30;
				act->origtrig = act->trigger;
				curact->trigger = lead_time;
				act->dt = -1;
				act->next = curact;
				if(prev == NULL) actListHead = act;
				else prev->next = act;
				tp++;
			}
			prev = curact;
			curact = curact->next;
		}
		double fracfp = (((double)tp)/precision) - (double)tp;
		double fracprobab = fracfp - ((double)(floor(fracfp)));
		bool roundup = probabCheck(fracprobab);
		if(roundup) fp = ceil(fracfp);
		else fp = floor(fracfp);
		for(int fpcount=0; fpcount < fp; fpcount++) {
			double fptime = uni_rand(0,tottime);
			double curtime = 0;
			ActList *curact = actListHead;
			ActList *prev = NULL;
			while(curact != NULL) {
				if(fptime > curact->trigger) fptime -= curact->trigger;
				else {
					ActList *act = (ActList *)malloc(sizeof(ActList));
					int index = ((int)(floor(uni_rand(0,node_num-1))));
					act->relpair = &(array[index]);
					act->acttype = ACT_NODE_PRED;
					act->trigger = fptime;
					act->origtrig = act->trigger;
					curact->trigger = curact->trigger - fptime;
					if(act->trigger <= 0) act->trigger = 30;
					act->dt = -1;
					act->next = curact;
					if(prev == NULL) actListHead = act;
					else prev->next = act;
				}
				curtime += curact->trigger;
				prev = curact;
				curact = curact->next;
			}
		}
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

int delete_matching_lines_inplace(const char *filepath, const char *match)
{
	FILE *fp = fopen(filepath, "r+");
	if (!fp) {
		perror("fopen");
		return -1;
	}
	
	int lockret;
	do {
		lockret = flock(fileno(fp), LOCK_EX);
	} while(lockret != 0);
	
	size_t match_len = strlen(match);
	long removed_bytes = 0;
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;
	long pos = 0;
	while ((nread = getline(&line, &len, fp)) != -1) {
		if (nread > 0 && line[nread-1] == '\n') {
			line[nread-1] = '\0';
			if(strcmp(line, match) == 0) {
				removed_bytes += nread;
			}
			line[nread-1] = '\n';
		} else if (strcmp(line, match) == 0) {
			removed_bytes += nread;
		}
		pos += nread;
	}
	free(line);
	line = NULL;
	
	if (ferror(fp)) { perror("getline"); goto error; }
	if (removed_bytes == 0) {
		fclose(fp);
		return 0;
	}
	
	rewind(fp);
	long read_pos  = 0;
	long write_pos = 0;
	while ((nread = getline(&line, &len, fp)) != -1) {
		int keep = 1;
		if (nread > 0 && line[nread-1] == '\n') {
			line[nread-1] = '\0';
			if (strcmp(line, match) == 0) keep = 0;
			line[nread-1] = '\n';
		} else if (strcmp(line, match) == 0) {
			keep = 0;
		}
		
		if (keep) {
			if (read_pos > write_pos) {
				if (fseek(fp, write_pos, SEEK_SET) != 0) { perror("fseek(write)"); goto error; }
				if (fwrite(line, 1, nread, fp) != (size_t)nread) { perror("fwrite"); goto error; }
				if (fseek(fp, read_pos, SEEK_SET) != 0) { perror("fseek(reset)"); goto error; }
			}
			write_pos += nread;
		}
		read_pos += nread;
	}
	free(line);
	line = NULL;
	
	if (ferror(fp)) { perror("getline/fwrite"); goto error; }
	if (fflush(fp) != 0) { perror("fflush"); goto error; }
	if (ftruncate(fileno(fp), write_pos) != 0) { perror("ftruncate"); goto error; }
	
	do {
		lockret = flock(fileno(fp), LOCK_UN);
	} while(lockret != 0);
	
	fclose(fp);
	return 0;
error:
	if (line) free(line);
	
	do {
		lockret = flock(fileno(fp), LOCK_UN);
	} while(lockret != 0);
	
	fclose(fp);
	return -1;
}

void sigusr2_handler(int signum) {
	if(sigusr2_recvd == 1) return;
	sigusr2_recvd = 1;
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR2);
	sigprocmask(SIG_BLOCK, &mask, NULL);
	printf("%d: Failure injector got SIGUSR2 sigusr2_recvd %d\n",getpid(),sigusr2_recvd);
	fflush(stdout);
	ActList *curact = actListHead;
	ActList *prev = NULL;
	while(curact != NULL) {
		double tasktime;
		if(curact->acttype == ACT_PRED) tasktime = curact->relpair->ttp;
		else if((curact->acttype == ACT_KILL)) tasktime = curact->relpair->ttk;
		else if((curact->acttype == ACT_TRIG) || (curact->acttype == ACT_NODE_RESTART) || (curact->acttype == ACT_NODE_KILL) || (curact->acttype == ACT_NODE_PRED)) tasktime = curact->trigger;
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
		pout = ppoll(&pfd,1,&wtime,NULL);
		assert(pout == 0);
		if(curact->acttype == ACT_NODE_RESTART) {
			char downpath[MAX_FILENAME_LEN];
			sprintf(downpath,"%s/checkpoint/downfile%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
			if(!uselogs) curact->trigger = weibull_distribution(shape, scale);
			if(curact->trigger < 150 && !uselogs) curact->relpair->ttk = 150;
			ActList *temp = curact;
			ActList *act = (ActList *)malloc(sizeof(ActList));
			act->relpair = curact->relpair;
			act->acttype = ACT_NODE_KILL;
			act->trigger = curact->origtrig;
			act->dt = curact->origdt;
			act->origtrig = curact->origtrig;
			act->origdt = curact->origdt;
			act->next = NULL;
			while(temp->next != NULL) {
				temp = temp->next;
			}
			temp->next = act;
			int delret = delete_matching_lines_inplace(downpath, curact->relpair->second);
			assert(delret == 0);
			if(curact == actListHead) actListHead = actListHead->next;
			else prev->next = curact->next;
			free(curact);
			if(prev == NULL) curact = actListHead;
			else curact = prev->next;
			sigprocmask(SIG_UNBLOCK, &mask, NULL);
			sigusr2_recvd = 0;
			return;
		} else if(curact->acttype == ACT_NODE_KILL) {
			curact->dt = curact->origdt;
			curact->trigger = curact->origtrig;
			ActList *temp = curact;
			while(temp->next != NULL) {
				temp = temp->next;
			}
			temp->next = curact;
			if(curact == actListHead) actListHead = actListHead->next;
			else prev->next = curact->next;
			curact->next = NULL;
			if(prev == NULL) curact = actListHead;
			else curact = prev->next;
		} else if(curact->acttype == ACT_NODE_PRED) {
			curact->trigger = curact->origtrig;
			ActList *temp = curact;
			while(temp->next != NULL) {
				temp = temp->next;
			}
			temp->next = curact;
			if(curact == actListHead) actListHead = actListHead->next;
			else prev->next = curact->next;
			curact->next = NULL;
			if(prev == NULL) curact = actListHead;
			else curact = prev->next;
		}
	}
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	sigusr2_recvd = 0;
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
	if(sigusr1_recvd == 1) {
		sigprocmask(SIG_UNBLOCK, &mask, NULL);
		return;
	}
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
		if(pid_node_pairs[i].ttk < 150 && !uselogs) pid_node_pairs[i].ttk = 150;
		pid_node_pairs[i].ttp = -1;
	}
	fclose(file);
	remove(file_name);
	remove(dfile_name);
	
	if(node_kill_mode) {
	} else {
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
	}
	
	sigusr1_recvd = 0;
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	assert(sigstate == 3);
	struct sigaction sa = {0};
	sa.sa_handler = sigusr1_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR1, &sa, NULL);
	sigstate = 0;
}

int main(int argc, char **argv) {
	N = atoi(getenv("PAREP_MPI_SIZE")); // number of processes
	alive = N; // number of alive processes
	gauss_stddev = 1;
	gauss_var = gauss_stddev*gauss_stddev;
	precision = 1;
	recall = 1;
	
	if(getenv("PAREP_MPI_SIM_LOGS") != NULL) {
		if(!strcmp(getenv("PAREP_MPI_SIM_LOGS"),"1")) {
			uselogs = true;
		}
	}
	
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

	struct sigaction sa = {0};
	sa.sa_handler = sigusr1_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR1, &sa, NULL);
	
	struct sigaction sa2 = {0};
	sa2.sa_handler = sigusr2_handler;
	sa2.sa_flags = 0;
	sigemptyset(&sa2.sa_mask);
	sigaction(SIGUSR2, &sa2, NULL);
	
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
	if(getenv("PAREP_MPI_SIM_NODEKILL") != NULL) {
		if(!strcmp(getenv("PAREP_MPI_SIM_NODEKILL"),"1")) {
			node_kill_mode = 1;
			node_num = atoi(getenv("PAREP_MPI_NODE_NUM"));
			node_pairs = (IntStringPair *)malloc(node_num*sizeof(IntStringPair));
			for(int i = 0; i < node_num; i++) {
				node_pairs[i].second[0] = '\0';
			}
		}
	}
	
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
		if(pid_node_pairs[i].ttk < 150 && !uselogs) pid_node_pairs[i].ttk = 150;
		pid_node_pairs[i].ttp = -1;
	}
	fclose(file);
	fclose(node_file);
	remove(file_name);
	remove(dfile_name);
	if(getenv("PAREP_MPI_SIM_NODEKILL") != NULL) {
		if(!strcmp(getenv("PAREP_MPI_SIM_NODEKILL"),"1")) {
			char file_name[MAX_FILENAME_LEN];
			sprintf(file_name,"%s/checkpoint/nodecopy%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
			FILE *file = fopen(file_name, "r");
			int index = 0;
			char first_node[MAX_FILENAME_LEN];
			first_node[0] = '\0';
			for (int i = 0; i < N; i++) {
				fscanf(file, "%s\n", node_name);
				if(first_node[0] == '\0') {
					strcpy(first_node,node_name);
				} else if(!strcmp(first_node,node_name)) {
				} else {
					int already_obt = 0;
					for(int j = 0; j < node_num; j++) {
						if(!strcmp((char *)(node_pairs[j].second),node_name)) {
							already_obt = 1;
							break;
						}
					}
					if(!already_obt) {
						node_pairs[index].first = -1;
						strcpy((char *)(node_pairs[index].second), node_name);
						node_pairs[index].ttk = weibull_distribution(shape, scale);
						if((node_pairs[index].ttk < 150) && !uselogs) node_pairs[index].ttk = 150;
						node_pairs[index].ttp = -1;
						index++;
					}
				}
			}
			fclose(file);
		}
	}
	

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
	if(node_kill_mode) {
		fisher_yates_shuffle((IntStringPair *)node_pairs, node_num-1);
		create_node_kill_acts((IntStringPair *)node_pairs, node_num-1);
		if(getenv("PAREP_MPI_SIM_PREDICT") != NULL) {
			if(!strcmp(getenv("PAREP_MPI_SIM_PREDICT"),"1")) {
				create_node_pred_acts((IntStringPair *)node_pairs);
			}
		}
	} else {
		create_kill_acts((IntStringPair *)shuffled_pairs, N);
		if(getenv("PAREP_MPI_SIM_PREDICT") != NULL) {
			if(!strcmp(getenv("PAREP_MPI_SIM_PREDICT"),"1")) {
				create_pred_acts((IntStringPair *)shuffled_pairs, N);
			}
		}
	}

	
	if(node_kill_mode) {
		ActList *curact = actListHead;
		while(curact != NULL) {
			double tasktime;
			if(curact->acttype == ACT_PRED) tasktime = curact->relpair->ttp;
			else if((curact->acttype == ACT_KILL)) tasktime = curact->relpair->ttk;
			else if((curact->acttype == ACT_TRIG) || (curact->acttype == ACT_NODE_RESTART) || (curact->acttype == ACT_NODE_KILL) || (curact->acttype == ACT_NODE_PRED)) tasktime = curact->trigger;
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
			
			printf("Starting timer for kill node %s: type %d tasktime %f\n",curact->relpair->second,curact->acttype,tasktime);
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
			if(curact->acttype == ACT_NODE_KILL) {
				double downtime = 0;
				if(curact->dt >= 0) downtime = curact->dt;
				ActList *act = (ActList *)malloc(sizeof(ActList));
				act->relpair = curact->relpair;
				act->acttype = ACT_NODE_RESTART;
				act->dt = curact->trigger;
				act->trigger = downtime;
				act->origtrig = curact->origtrig;
				act->origdt = curact->origdt;
				ActList *prevact = actListHead;
				ActList *thisact = actListHead->next;
				while(thisact != NULL) {
					double ttime;
					if(thisact->acttype == ACT_PRED) ttime = thisact->relpair->ttp;
					else if((thisact->acttype == ACT_KILL)) ttime = thisact->relpair->ttk;
					else if((thisact->acttype == ACT_TRIG) || (thisact->acttype == ACT_NODE_RESTART) || (thisact->acttype == ACT_NODE_KILL) || (thisact->acttype == ACT_NODE_PRED)) ttime = thisact->trigger;
					if(ttime > act->trigger) break;
					else {
						act->trigger -= ttime;
						prevact = thisact;
						thisact = thisact->next;
					}
				}
				prevact->next = act;
				act->next = thisact;
				if(thisact != NULL) {
					if(thisact->acttype == ACT_PRED) thisact->relpair->ttp -= act->trigger;
					else if((thisact->acttype == ACT_KILL)) thisact->relpair->ttk -= act->trigger;
					else if((thisact->acttype == ACT_TRIG) || (thisact->acttype == ACT_NODE_RESTART) || (thisact->acttype == ACT_NODE_KILL) || (thisact->acttype == ACT_NODE_PRED)) thisact->trigger -= act->trigger;
				}
				
				if(downtime > 0) {
					char downpath[MAX_FILENAME_LEN];
					sprintf(downpath,"%s/checkpoint/downfile%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
					FILE *downfile = fopen(downpath, "a");
					int lockret;
					do {
						lockret = flock(fileno(downfile), LOCK_EX);
					} while(lockret != 0);
					assert(downfile != NULL);
					fprintf(downfile,"%s\n",curact->relpair->second);
					do {
						lockret = flock(fileno(downfile), LOCK_UN);
					} while(lockret != 0);
					fclose(downfile);
				}
				
				char kill_command[MAX_FILENAME_LEN];
				int ret;
				printf("Killing node %s: Kill TTK %f\n",curact->relpair->second,curact->relpair->ttk);
				fflush(stdout);
				sprintf(kill_command, "ssh %s pkill -9 -u %s", curact->relpair->second, "cdsjsar");
				do {
					ret = system(kill_command);
					printf("Failure injector system returned %d\n",ret);
				} while(ret == -1);
			} else if(curact->acttype == ACT_NODE_RESTART) {
				char downpath[MAX_FILENAME_LEN];
				sprintf(downpath,"%s/checkpoint/downfile%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
				if(!uselogs) curact->trigger = weibull_distribution(shape, scale);
				if((curact->trigger < 150) && !uselogs) curact->relpair->ttk = 150;
				ActList *temp = curact;
				ActList *act = (ActList *)malloc(sizeof(ActList));
				act->relpair = curact->relpair;
				act->acttype = ACT_NODE_KILL;
				act->trigger = curact->origtrig;
				act->dt = curact->origdt;
				act->origtrig = curact->origtrig;
				act->origdt = curact->origdt;
				act->next = NULL;
				while(temp->next != NULL) {
					temp = temp->next;
				}
				temp->next = act;
				int delret = delete_matching_lines_inplace(downpath, curact->relpair->second);
				assert(delret == 0);
			} else if(curact->acttype == ACT_NODE_PRED) {
				sigstate = 1;
				printf("Node %s: Predict TTP %f\n",curact->relpair->second,curact->trigger);
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
			}
			actListHead = actListHead->next;
			free(curact);
			curact = actListHead;
		}
	} else {
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
	}
	return 0;
}
