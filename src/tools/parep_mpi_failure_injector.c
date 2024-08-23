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

#define MAX_FILENAME_LEN 256

typedef struct {
	int first;
	char second[MAX_FILENAME_LEN];
} IntStringPair;

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

IntStringPair *pid_node_pairs;
volatile IntStringPair *shuffled_pairs;
double *time_between_failures;
int N;
volatile int targpid;
volatile char targnname[MAX_FILENAME_LEN];

volatile sig_atomic_t sigusr1_recvd = 0;

void sigusr1_handler(int signum) {
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

	for (int i = 0; i < N; i++) {
		shuffled_pairs[i].first = pid_node_pairs[i].first;
		strcpy((char *)shuffled_pairs[i].second, pid_node_pairs[i].second);
	}
	fisher_yates_shuffle((IntStringPair *)shuffled_pairs, N);
	sigusr1_recvd = 0;
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

int main(int argc, char **argv) {
	N = atoi(getenv("PAREP_MPI_SIZE")); // number of processes
	double mean = -1; // mean time between failure in seconds
	if(getenv("PAREP_MPI_MTBF")) mean = atof(getenv("PAREP_MPI_MTBF"));
	//double shape = 1; // shape parameter for Weibull distribution
	double shape = 1;
	double scale;
	if(mean != -1) scale = mean / (tgamma(1 + (1/shape))); // scale parameter for Weibull distribution

	struct timespec start_time;
	struct timespec end_time;
	struct timespec elap_time;

	struct sigaction sa;
	sa.sa_handler = sigusr1_handler;
	sigaction(SIGUSR1, &sa, NULL);

	srand(time(NULL));

	// Read PIDs and node names from files
	pid_node_pairs = (IntStringPair *)malloc(N*sizeof(IntStringPair));
	shuffled_pairs = (IntStringPair *)malloc(N*sizeof(IntStringPair));
	//time_between_failures = (double *)malloc(N*sizeof(double));
	char directory[MAX_FILENAME_LEN];
	strcpy(directory, getenv("PAREP_MPI_WORKDIR"));
	char nfile_name[MAX_FILENAME_LEN];
	sprintf(nfile_name,"%s/pbs_nodes",directory);

	FILE *node_file = fopen(nfile_name, "r");
	char node_name[MAX_FILENAME_LEN];
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

	// Kill processes after generated time has passed
	int i = 0;
	while(1) {
		targpid = shuffled_pairs[i].first;
		strcpy((char *)targnname, (char *)shuffled_pairs[i].second);
		double ktime = weibull_distribution(shape, scale);
		if(ktime < 150) ktime = 150;
		printf("Process %d on node %s: Time before failure = %f seconds\n", targpid, targnname, ktime);
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
		//usleep(time_between_failures[i] * 1000000); // sleep for time before failure in microseconds

		// Execute kill command (uncomment and modify as needed)
		char kill_command[MAX_FILENAME_LEN];
		sprintf(kill_command, "ssh %s kill -9 %d", targnname, targpid);
		int ret;
		do {
			ret = system(kill_command);
			printf("Failure injector system returned %d\n",ret);
			fflush(stdout);
		} while(ret != 0);

		i++;
		if (i == N) i = 0;
	}

	return 0;
}
