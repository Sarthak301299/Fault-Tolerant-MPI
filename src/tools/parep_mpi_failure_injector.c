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

typedef struct {
	int first;
	char second[MAX_FILENAME_LEN];
} IntStringPair;

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
		IntStringPair temp = array[i];
		array[i] = array[j];
		array[j] = temp;
	}
}

double weibull_distribution(double shape, double scale) {
	double rand_num = (double)rand() / RAND_MAX;
	return scale * pow(-log(1 - rand_num), 1 / shape);
}

int uni_rand(int min, int max) {
	return min + (rand() % (max - min));
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
IntProbabilityPair *probvals;
bool meanchanged = false;
struct sockaddr_in saddr;

volatile sig_atomic_t sigusr1_recvd = 0;
volatile sig_atomic_t sigstate = 0;

void inform_server() {
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
			close(dyn_sock);
			
		}
	}
}

void dump_probs_and_sim_predict() {
	if(getenv("PAREP_MPI_SIM_PREDICT") != NULL) {
		if(!strcmp(getenv("PAREP_MPI_SIM_PREDICT"),"1")) {
			FILE *probfile;
			char directory[MAX_FILENAME_LEN];
			strcpy(directory, getenv("PAREP_MPI_WORKDIR"));
			char probfile_name[MAX_FILENAME_LEN];
			sprintf(probfile_name,"%s/probfile",directory);
			probfile = fopen(probfile_name, "w+");
			for(int i = 0; i < alive; i++) {
				int index;
				index = probvals[i].value;
				fprintf(probfile,"%d : %s : %f\n",shuffled_pairs[index].first,shuffled_pairs[index].second,probvals[i].probability);
			}
			fclose(probfile);
		}
	}
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
	}
	fclose(file);
	remove(file_name);
	remove(dfile_name);
	
	num_killed = 0;
	for (int i = 0; i < N; i++) {
		shuffled_pairs[i].first = pid_node_pairs[i].first;
		strcpy((char *)shuffled_pairs[i].second, pid_node_pairs[i].second);
	}
	fisher_yates_shuffle((IntStringPair *)shuffled_pairs, N);
	dump_probs_and_sim_predict();
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
	double mean = -1; // mean time between failure in seconds
	if(getenv("PAREP_MPI_MTBF")) mean = atof(getenv("PAREP_MPI_MTBF"));
	//double shape = 1; // shape parameter for Weibull distribution
	double shape = 0.7;
	double scale;
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
	killed_pairs = (IntStringPair *)malloc(N*sizeof(IntStringPair));
	probvals = (IntProbabilityPair *)malloc(N*sizeof(IntProbabilityPair));
	//time_between_failures = (double *)malloc(N*sizeof(double));
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
	}
	fclose(file);
	fclose(node_file);
	remove(file_name);
	remove(dfile_name);

	if(mean == -1) {
		pause();
	}

	// Generate time between failures
	//for (int i = 0; i < N; i++) {
	//time_between_failures[i] = weibull_distribution(shape, scale);
	//}

	// Shuffle pids and node names
	for (int i = 0; i < N; i++) {
		shuffled_pairs[i].first = pid_node_pairs[i].first;
		strcpy((char *)shuffled_pairs[i].second, pid_node_pairs[i].second);
	}

	// Shuffle using Fisher-Yates algorithm
	fisher_yates_shuffle((IntStringPair *)shuffled_pairs, N);
	
	//Generate a mean using uniform distribution for the alive processes
	meanchanged = true;
	gauss_mean = (double)uni_rand(0,alive);
	for (int j = (gauss_mean - (alive/2)); j < (gauss_mean + (alive/2)); j++) {
		int wrapped_j = wrap_around(j, alive); // Wrap the integer within range [0, N]
		probvals[wrapped_j].value = wrapped_j;
		probvals[wrapped_j].probability = normal_pdf(j, gauss_mean, gauss_var);
	}
	qsort(probvals,alive,sizeof(IntProbabilityPair),compare);
	if(meanchanged) {
		meanchanged = false;
		dump_probs_and_sim_predict();
	}
	// Kill processes after generated time has passed
	int i = wrap_around(generate_random_int(gauss_mean,gauss_var),alive);
	while(1) {
		char kill_command[MAX_FILENAME_LEN];
		int ret;
		bool proc_killed = false;
		for(int k = 0; k < num_killed; k++) {
			if((shuffled_pairs[i].first == killed_pairs[k].first) && (!strcmp((char *)shuffled_pairs[i].second,(char *)killed_pairs[k].second))) {
				proc_killed = true;
				break;
			}
		}
		while (proc_killed) {
			if(rand() % 10 == 0) { // Change mean by 10% chance
				meanchanged = true;
				gauss_mean = (double)uni_rand(0,alive);
				for (int j = (gauss_mean - (alive/2)); j < (gauss_mean + (alive/2)); j++) {
					int wrapped_j = wrap_around(j, alive);
					probvals[wrapped_j].value = wrapped_j;
					probvals[wrapped_j].probability = normal_pdf(j, gauss_mean, gauss_var);
				}
				qsort(probvals,alive,sizeof(IntProbabilityPair),compare);
			}
			i = wrap_around(generate_random_int(gauss_mean,gauss_var),alive);
			proc_killed = false;
			for(int k = 0; k < num_killed; k++) {
				if((shuffled_pairs[i].first == killed_pairs[k].first) && (!strcmp((char *)shuffled_pairs[i].second,(char *)killed_pairs[k].second))) {
					proc_killed = true;
					break;
				}
			}
		}
		if(meanchanged) {
			meanchanged = false;
			dump_probs_and_sim_predict();
		}
		targpid = shuffled_pairs[i].first;
		strcpy((char *)targnname, (char *)shuffled_pairs[i].second);
		double ktime = weibull_distribution(shape, scale);
		if(ktime < 150) ktime = 150;
		int findex = -1;
		for (int k = 0; k < alive; k++) {
			int index;
			index = probvals[k].value;
			if((shuffled_pairs[index].first == targpid) && (!strcmp((char *)shuffled_pairs[index].second,(char *)targnname))) {
				findex = k;
				break;
			}
		}
		printf("Process %d on node %s: Time before failure = %f seconds. Failure index = %d\n", targpid, targnname, ktime, findex);
		fflush(stdout);
		struct pollfd pfd;
		pfd.fd = -1;
		pfd.events = 0;
		pfd.revents = 0;
		struct timespec wtime;
		double fractpart, intpart;
		fractpart = modf(ktime,&intpart);
		wtime.tv_sec = (long)intpart;
		wtime.tv_nsec = (long)(fractpart*1000000000);
		int pout = -1;
		if(sigstate == 2) {
			sigstate = 0;
			sigusr1_recvd = 2;
			raise(SIGUSR1);
			while(sigusr1_recvd);
			targpid = shuffled_pairs[i].first;
			strcpy((char *)targnname, (char *)shuffled_pairs[i].second);
			printf("Updated Process %d on node %s: Time before failure = %ld seconds %ld nsec\n", targpid, targnname, wtime.tv_sec,wtime.tv_nsec);
			fflush(stdout);
		}
		
		if(getenv("PAREP_MPI_SIM_PREDICT") != NULL) {
			if(!strcmp(getenv("PAREP_MPI_SIM_PREDICT"),"1")) {
				lead_time = 60 + (((double)rand() / RAND_MAX)*(60));
				wtime.tv_sec = wtime.tv_sec - lead_time;
			}
		}
		
		sigstate = 0;
		do {
			clock_gettime(CLOCK_MONOTONIC, &start_time);
			pout = ppoll(&pfd,1,&wtime,NULL);
			if(pout == -1 && errno == EINTR) {
				clock_gettime(CLOCK_MONOTONIC, &end_time);
				elap_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
				elap_time.tv_nsec = end_time.tv_nsec - start_time.tv_nsec;
				while (elap_time.tv_nsec < 0) {
					elap_time.tv_sec--;
					elap_time.tv_nsec += 1000000000;
				}
				if(elap_time.tv_sec < 0) {
					elap_time.tv_sec = 0;
					elap_time.tv_nsec = 0;
				}

				wtime.tv_sec = wtime.tv_sec - elap_time.tv_sec;
				wtime.tv_nsec = wtime.tv_nsec - elap_time.tv_nsec;
				while (wtime.tv_nsec < 0) {
					wtime.tv_sec--;
					wtime.tv_nsec += 1000000000;
				}
				if(wtime.tv_sec < 0) {
					wtime.tv_sec = 0;
					wtime.tv_nsec = 0;
				}
				targpid = shuffled_pairs[i].first;
				strcpy((char *)targnname, (char *)shuffled_pairs[i].second);
				printf("Updated Process %d on node %s: Time before failure = %ld seconds %ld nsec\n", targpid, targnname, wtime.tv_sec,wtime.tv_nsec);
				fflush(stdout);
			}
		} while(pout == -1 && errno == EINTR);
		assert(pout == 0);
		
		if(getenv("PAREP_MPI_SIM_PREDICT") != NULL) {
			if(!strcmp(getenv("PAREP_MPI_SIM_PREDICT"),"1")) {
				sigstate = 1;
				inform_server();
				wtime.tv_sec = lead_time;
				wtime.tv_nsec = 0;
				pout = -1;
				if(sigstate == 2) {
					sigstate = 0;
					sigusr1_recvd = 2;
					raise(SIGUSR1);
					while(sigusr1_recvd);
				}
				sigstate = 0;
				do {
					clock_gettime(CLOCK_MONOTONIC, &start_time);
					pout = ppoll(&pfd,1,&wtime,NULL);
					if(pout == -1 && errno == EINTR) {
						clock_gettime(CLOCK_MONOTONIC, &end_time);
						elap_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
						elap_time.tv_nsec = end_time.tv_nsec - start_time.tv_nsec;
						while (elap_time.tv_nsec < 0) {
							elap_time.tv_sec--;
							elap_time.tv_nsec += 1000000000;
						}
						if(elap_time.tv_sec < 0) {
							elap_time.tv_sec = 0;
							elap_time.tv_nsec = 0;
						}

						wtime.tv_sec = wtime.tv_sec - elap_time.tv_sec;
						wtime.tv_nsec = wtime.tv_nsec - elap_time.tv_nsec;
						while (wtime.tv_nsec < 0) {
							wtime.tv_sec--;
							wtime.tv_nsec += 1000000000;
						}
						if(wtime.tv_sec < 0) {
							wtime.tv_sec = 0;
							wtime.tv_nsec = 0;
						}
						if(wtime.tv_sec < 30) {
							wtime.tv_sec = 30;
							wtime.tv_nsec = 0;
						}
						targpid = shuffled_pairs[i].first;
						strcpy((char *)targnname, (char *)shuffled_pairs[i].second);
						printf("Updated Process %d on node %s: Time before failure = %ld seconds %ld nsec\n", targpid, targnname, wtime.tv_sec,wtime.tv_nsec);
						fflush(stdout);
						inform_server();
					}
				} while(pout == -1 && errno == EINTR);
				assert(pout == 0);
			}
		}
		//usleep(time_between_failures[i] * 1000000); // sleep for time before failure in microseconds
		//Execute kill command (uncomment and modify as needed)
		sprintf(kill_command, "ssh %s kill -9 %d", targnname, targpid);
		do {
			ret = system(kill_command);
			printf("Failure injector system returned %d\n",ret);
			fflush(stdout);
		} while(ret != 0);
		sigstate = 1;
		killed_pairs[num_killed].first = targpid;
		strcpy((char *)killed_pairs[num_killed].second,(char *)targnname);
		num_killed++;
		
		if(rand() % 4 == 0) { // Change mean by 25% chance
			meanchanged = true;
			gauss_mean = (double)uni_rand(0,alive);
			for (int j = (gauss_mean - (alive/2)); j < (gauss_mean + (alive/2)); j++) {
				int wrapped_j = wrap_around(j, alive);
				probvals[wrapped_j].value = wrapped_j;
				probvals[wrapped_j].probability = normal_pdf(j, gauss_mean, gauss_var);
			}
			qsort(probvals,alive,sizeof(IntProbabilityPair),compare);
		}
		i = wrap_around(generate_random_int(gauss_mean,gauss_var),alive);
	}

	return 0;
}
