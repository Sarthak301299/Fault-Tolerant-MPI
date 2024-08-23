#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>

#define RET_COMPLETED 101
#define RET_INCOMPLETE 102

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

extern char **environ;
int main(int argc, char **argv) {
	int parep_mpi_size;
	int parep_mpi_ntasks_per_node;
	int parep_mpi_node_num;
	int parep_mpi_node_size;
	int parep_mpi_node_id;
	if(argc < 4) {
		printf("Usage: mpirun -np <num_procs> <executable>");
		return 1;
	}
	
	char job_ckpt_dir[256];
	char nfilepath[256];
	char snfile[256];
	sprintf(job_ckpt_dir,"/scratch/cdsjsar/checkpoint/%s",getenv("SLURM_JOB_ID"));
	{
		int ret;
		do {
			ret = mkdir(job_ckpt_dir,0700);
		} while((ret == -1) && (errno != EEXIST));
	}
	sprintf(nfilepath,"%s/pbs_nodes",job_ckpt_dir);
	
	FILE *source, *target;
	char ch;
	sprintf(snfile,"/scratch/cdsjsar/checkpoint/pbs_nodes%s",getenv("SLURM_JOB_ID"));
	source = fopen(snfile, "r");
	if(source == NULL) {
		printf("Error opening source file.\n");
		return 1;
	}
	
	target = fopen(nfilepath, "w");
	if (target == NULL) {
		fclose(source);
		printf("Error opening target file.\n");
		return 1;
	}
	
	while ((ch = fgetc(source)) != EOF) {
		fputc(ch, target);
	}
	
	fclose(source);
	fclose(target);
	
	for(int i=1; i < argc; i++) {
		if(strcmp(argv[i],"-np") == 0) {
			setenv("PAREP_MPI_SIZE", argv[i+1], 1);
			parep_mpi_size = atoi(argv[i+1]);
			break;
		}
		if(i == (argc-1)) {
			printf("Usage: mpirun -np <num_procs> <executable>");
			return 1;
		}
	}
	
	parep_mpi_node_id = atoi(getenv("SLURM_NODEID"));
	parep_mpi_ntasks_per_node = atoi(getenv("SLURM_NTASKS_PER_NODE"));
	if((parep_mpi_size % parep_mpi_ntasks_per_node) == 0)	parep_mpi_node_num = parep_mpi_size/parep_mpi_ntasks_per_node;
	else parep_mpi_node_num = 1 + ((parep_mpi_size - (parep_mpi_size % parep_mpi_ntasks_per_node))/parep_mpi_ntasks_per_node);
	parep_mpi_node_size = parep_mpi_ntasks_per_node;
	
	char temp[64];
	sprintf(temp,"%d",parep_mpi_node_num);
	setenv("PAREP_MPI_NODE_NUM",temp,1);
	sprintf(temp,"%d",parep_mpi_node_size);
	setenv("PAREP_MPI_NODE_SIZE",temp,1);
	sprintf(temp,"%d",parep_mpi_node_id);
	setenv("PAREP_MPI_NODE_ID",temp,1);
	
	char which_cmd[1024];
	snprintf(which_cmd, sizeof(which_cmd), "which srun");
	char out[1024];
	FILE *fp = popen(which_cmd, "r");
	if (fp && fgets(out, sizeof(out), fp)) {
		out[strlen(out)-1] = '\0';
		setenv("PAREP_MPI_SRUN", out, 1);
	} else {
		perror("which srun");
		return 1;
	}
	pclose(fp);
	
	char path[4096];
	sprintf(path,"/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi/bin/parep_mpi_srun:%s",getenv("PATH"));
	setenv("PATH",path,1);
	sprintf(path,"/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi");
	setenv("PAREP_MPI_PATH",path,1);
	sprintf(path,"/scratch/cdsjsar/checkpoint/%s",getenv("SLURM_JOB_ID"));
	setenv("PAREP_MPI_WORKDIR",path,1);
	sprintf(path,"/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi/lib:%s",getenv("LD_LIBRARY_PATH"));
	setenv("LD_LIBRARY_PATH",path,1);
	
	char host_name[HOST_NAME_MAX+1];
	char *IP_address;
	int thname = gethostname(host_name,sizeof(host_name));
	if(thname == -1) {
		perror("gethostname");
		exit(1);
	}
	
	setenv("PAREP_MPI_HEAD_NODE", host_name, 1);

	struct hostent *entry = gethostbyname(host_name);
	if(entry == NULL) {
		perror("gethostbyname");
		exit(1);
	}

	IP_address = inet_ntoa(*((struct in_addr *)entry->h_addr_list[0]));
	setenv("PAREP_MPI_MAIN_COORDINATOR_IP", IP_address, 1);

	fflush(stdout);
	
	pid_t empi_pid;
	pid_t coordinator_pid;
	pid_t failure_injector_pid;
	
	int completed = RET_INCOMPLETE;
	
	if(getenv("PAREP_MPI_MTBF") != NULL) {
		failure_injector_pid = fork();
		if(failure_injector_pid == -1) {
			perror("fork");
			exit(1);
		}

		if(failure_injector_pid == 0) {
			char exec[1024];
			sprintf(exec,"%s/bin/parep_mpi_failure_injector",getenv("PAREP_MPI_PATH"));
			char **newargv;
			newargv = (char **)malloc(sizeof(char *)*(argc+2));
			newargv[0] = (char *)malloc(sizeof(exec));
			strcpy(newargv[0],exec);
			for(int i = 1; i < argc; i++) {
				newargv[i] = (char *)malloc(strlen(argv[i])+1);
				strcpy(newargv[i],argv[i]);
			}
			char argument[64];
			sprintf(argument,"-ismainserver");
			newargv[argc] = (char *)malloc(sizeof(argument));
			strcpy(newargv[argc],argument);
			newargv[argc+1] = NULL;
			execve(exec,newargv,environ);
		}
	}
	
	do {
		empi_pid = fork();
		if(empi_pid == -1) {
			perror("fork");
			exit(1);
		}
		
		if(empi_pid == 0) { //CHILD PROCESS
			char exec[1024];
			char thresh[8];
			sprintf(thresh,"%d",parep_mpi_size);
			setenv("MV2_ON_DEMAND_THRESHOLD",thresh,1);
			setenv("MV2_ON_DEMAND_UD_INFO_EXCHANGE","0",1);
			setenv("MV2_USE_PMI_IBARRIER","0",1);
			setenv("MV2_USE_PMI_IALLGATHER","0",1);
			sprintf(exec,"/home/phd/21/cdsjsar/MVAPICH2/bin/mpirun");
			char **newargv;
			newargv = (char **)malloc(sizeof(char *)*(argc+1));
			newargv[0] = (char *)malloc(sizeof(exec));
			strcpy(newargv[0],exec);
			int diff = 0;
			int j = 1;
			for(int i = 1; i < argc; i++) {
				if(!strcmp(argv[i],"-ckpt_interval")) {
					i = i+1;
					diff = diff+2;
					continue;
				}
				newargv[j] = (char *)malloc(strlen(argv[i])+1);
				strcpy(newargv[j],argv[i]);
				j++;
			}
			newargv[argc-diff] = NULL;
			execve(exec,newargv,environ);
		} else { //PARENT PROCESS
			pid_t coordinator_pid = fork();
			if(coordinator_pid == -1) {
				perror("fork");
				exit(1);
			}
			
			if(coordinator_pid == 0) { //COORDINATOR PROC
				char exec[1024];
				sprintf(exec,"%s/bin/parep_mpi_ckpt_coordinator",getenv("PAREP_MPI_PATH"));
				char **newargv;
				newargv = (char **)malloc(sizeof(char *)*(argc+2));
				newargv[0] = (char *)malloc(sizeof(exec));
				strcpy(newargv[0],exec);
				for(int i = 1; i < argc; i++) {
					newargv[i] = (char *)malloc(strlen(argv[i])+1);
					strcpy(newargv[i],argv[i]);
				}
				char argument[64];
				sprintf(argument,"-ismainserver");
				newargv[argc] = (char *)malloc(sizeof(argument));
				strcpy(newargv[argc],argument);
				newargv[argc+1] = NULL;
				execve(exec,newargv,environ);
			} else {
				int stat;
				do {
					waitpid(empi_pid,&stat,0);
				} while(!WIFEXITED(stat));
				do {
					waitpid(coordinator_pid,&stat,0);
				} while(!WIFEXITED(stat));
				completed = WEXITSTATUS(stat);
				if(completed == RET_INCOMPLETE) {
					int ret = kill(failure_injector_pid,SIGUSR1);
				}
			}
		}
	} while(completed == RET_INCOMPLETE);
	assert(completed == RET_COMPLETED);
	if(getenv("PAREP_MPI_MTBF") != NULL) kill(failure_injector_pid,SIGKILL);
	return 0;
}