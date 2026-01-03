#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <math.h>

#define RET_COMPLETED 101
#define RET_INCOMPLETE 102

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

double prod_pred(int b, int N, int j, double R) {
	double temp = 1.0;
	for (int k = 0; k < j; k++) {
		temp = temp * ((2 * (b - k)) + (R * (N - (2 * b) + k))) / (N - k);
	}
	return temp;
}

double sum_pred(int b, int N, double R) {
	double temp = 1.0;
	for (int j = 1; j <= b; j++) {
		temp = temp + prod_pred(b, N, j, R);
	}
	return temp;
}

static char *parep_mpi_itoa(int num, char *str, int base) {
	int isNegative = 0;
	int i = 0;
	if(num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}
	if(num < 0) {
		isNegative = 1;
		num = -num;
	}
	while(num != 0) {
		int digit = num % 10;
		str[i++] = '0' + digit;
		num = num / 10;
	}
	if(isNegative) {
		str[i++] = '-';
	}
	str[i] = '\0';
	int start = 0;
	int end = i-1;
	while(start < end) {
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		start++;
		end--;
	}
	return str;
}

extern char **environ;
int main(int argc, char **argv) {
	int down_procs = 0;
	double ckpt_time,mtbf,recall,a,b;
	recall = 0;
	char **down_nodes;
	int parep_mpi_size;
	int nC, nR;
	int parep_mpi_ntasks_per_node;
	int parep_mpi_node_num;
	int parep_mpi_node_size;
	int parep_mpi_node_id;
	if(argc < 4) {
		printf("Usage: mpirun -np <num_procs> <executable>");
		return 1;
	}
	
	if(getenv("PAREP_MPI_PREDICT_RECALL") != NULL) {
		double rec = atof(getenv("PAREP_MPI_PREDICT_RECALL"));
		recall = rec;
	}
	
	if(getenv("PAREP_MPI_CKPT_TIME") != NULL) {
		double c = atof(getenv("PAREP_MPI_CKPT_TIME"));
		ckpt_time = c;
	}
	
	if(getenv("PAREP_MPI_MTBF") != NULL) {
		double m = atof(getenv("PAREP_MPI_MTBF"));
		mtbf = m;
	}
	
	char job_ckpt_dir[256];
	char nfilepath[256];
	char snfile[256];
	char cppath[256];
	sprintf(job_ckpt_dir,"%s/checkpoint/%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
	{
		int ret;
		do {
			ret = mkdir(job_ckpt_dir,0700);
		} while((ret == -1) && (errno != EEXIST));
	}
	sprintf(nfilepath,"%s/pbs_nodes",getenv("PAREP_MPI_BASE_WORKDIR"),job_ckpt_dir);
	
	FILE *source, *target, *cpfile;
	char ch;
	sprintf(snfile,"%s/checkpoint/pbs_nodes%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
	source = fopen(snfile, "r");
	if(source == NULL) {
		printf("Error opening source file.\n");
		return 1;
	}
	
	sprintf(cppath,"%s/checkpoint/nodecopy%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
	cpfile = fopen(cppath, "w+");
	if(cpfile == NULL) {
		fclose(source);
		printf("Error opening cpfile file.\n");
		return 1;
	}
	
	target = fopen(nfilepath, "w");
	if (target == NULL) {
		fclose(cpfile);
		fclose(source);
		printf("Error opening target file.\n");
		return 1;
	}
	
	while ((ch = fgetc(source)) != EOF) {
		fputc(ch, target);
		fputc(ch, cpfile);
	}
	
	fclose(source);
	fclose(target);
	fclose(cpfile);
	
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
	
	if((getenv("CMP_RATIO") != NULL)  &&(getenv("REP_RATIO") != NULL)) {
		int cr = atoi(getenv("CMP_RATIO"));
		int rr = atoi(getenv("REP_RATIO"));
		nR = (parep_mpi_size*rr)/(rr+cr);
	} else if(getenv("COMP_PER_REP") != NULL) {
		int cpr = atoi(getenv("COMP_PER_REP"));
		if(cpr > 0) nR = parep_mpi_size/(1+cpr);
		else nR = 0;
	}
	nC = parep_mpi_size - nR;
	
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
	sprintf(path,"%s/bin/parep_mpi_srun:%s",getenv("PAREP_MPI_PATH"),getenv("PATH"));
	setenv("PATH",path,1);
	//sprintf(path,"/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi");
	//setenv("PAREP_MPI_PATH",path,1);
	sprintf(path,"%s/checkpoint/%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
	setenv("PAREP_MPI_WORKDIR",path,1);
	sprintf(path,"%s/lib:%s",getenv("PAREP_MPI_PATH"),getenv("LD_LIBRARY_PATH"));
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
	} else {
		failure_injector_pid = (pid_t)-1;
	}
	
	do {
		for(int i=1; i < argc; i++) {
			if(strcmp(argv[i],"-np") == 0) {
				FILE *downfile;
				char downpath[256];
				int nprocs;
				char buf[20];
				do {
					down_nodes = NULL;
					down_procs = 0;
					sprintf(downpath,"%s/checkpoint/downfile%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
					downfile = fopen(downpath, "r");
					if((downfile != NULL) || ((downfile == NULL) && (errno == ENOENT))) {
						char line[4096];
						down_nodes = (char **)malloc(sizeof(char *)*parep_mpi_size);
						for(int k=0; k < parep_mpi_size; k++) down_nodes[k] = NULL;
						if(downfile != NULL) {
							int lockret;
							do {
								lockret = flock(fileno(downfile), LOCK_EX);
							} while(lockret != 0);
							int index = 0;
							while (fgets(line, sizeof(line), downfile)) {
								down_nodes[index] = strdup(line);
								if((down_nodes[index][0] != '\0') && (down_nodes[index][0] != '\n')) down_procs += parep_mpi_ntasks_per_node;
								index++;
							}
							do {
								lockret = flock(fileno(downfile), LOCK_UN);
							} while(lockret != 0);
							fclose(downfile);
						}
					}
					nprocs = parep_mpi_size - down_procs;
					parep_mpi_itoa(nprocs, buf, 10);
					strcpy(argv[i+1],buf);
					nR = parep_mpi_size - nC - down_procs;
					sprintf(temp,"%d",parep_mpi_node_num - (down_procs/parep_mpi_ntasks_per_node));
					setenv("PAREP_MPI_NODE_NUM",temp,1);
					sprintf(temp,"%d",parep_mpi_size - down_procs);
					setenv("PAREP_MPI_SIZE",temp,1);
					if(nR < 0) {
						if(failure_injector_pid != -1) {
							int ret = kill(failure_injector_pid,SIGUSR2);
						}
						sleep(30);
					} else {
						unsetenv("COMP_PER_REP");
						unsetenv("CMP_RATIO");
						unsetenv("REP_RATIO");
						if (nR > 0) {
							parep_mpi_itoa(nC, buf, 10);
							setenv("CMP_RATIO", buf, 1);
							parep_mpi_itoa(nR, buf, 10);
							setenv("REP_RATIO", buf, 1);
							if(getenv("PAREP_MPI_COMPUTE_CKPT") != NULL) {
								if(!strcmp(getenv("PAREP_MPI_COMPUTE_CKPT"),"1")) {
									b = nR;
									a = nC + nR - (2*b);
									double Eab = sum_pred(b,nC+nR,recall);
									double Mintr = mtbf*Eab;
									double Ckpt = ckpt_time*((double)(a+b))/((double)(nC+nR));
									double Topt = sqrt(2*Mintr*Ckpt);
									parep_mpi_itoa(((int)(floor(Topt))), buf, 10);
									setenv("PAREP_MPI_CKPT_INTERVAL", buf, 1);
									printf("%d: Computed ckpt interval %f\n",getpid(),Topt);
									fflush(stdout);
								}
							}
						} else {
							sprintf(buf,"%d",-1);
							setenv("COMP_PER_REP", buf, 1);
							if(getenv("PAREP_MPI_COMPUTE_CKPT") != NULL) {
								if(!strcmp(getenv("PAREP_MPI_COMPUTE_CKPT"),"1")) {
									b = nR;
									a = nC + nR - (2*b);
									double Eab = sum_pred(b,nC+nR,recall);
									double Mintr = mtbf*Eab;
									double Ckpt = ckpt_time*((double)(a+b))/((double)(nC+nR));
									double Topt = sqrt(2*Mintr*Ckpt);
									parep_mpi_itoa(((int)(floor(Topt))), buf, 10);
									setenv("PAREP_MPI_CKPT_INTERVAL", buf, 1);
									printf("%d: Computed ckpt interval %f\n",getpid(),Topt);
									fflush(stdout);
								}
							}
						}
					}
				} while (nR < 0);
					
				if(down_procs > 0) {
					FILE *source, *target;
					sprintf(snfile,"%s/checkpoint/pbs_nodes%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
					target = fopen(snfile, "w");
					source = fopen(cppath, "r");
					
					char line[4096];
					
					while (fgets(line, sizeof(line), source)) {
						int isdown = 0;
						for(int k = 0; k < parep_mpi_size; k++) {
							if(down_nodes[k] != NULL) {
								if(!strcmp(down_nodes[k],line)) {
									isdown = 1;
									break;
								}
							}
						}
						if(!isdown) fputs(line, target);
					}
					
					fclose(source);
					fclose(target);
				}
				if(down_nodes != NULL) free(down_nodes);
				break;
			}
		}
		empi_pid = fork();
		if(empi_pid == -1) {
			perror("fork");
			exit(1);
		}
		
		if(empi_pid == 0) { //CHILD PROCESS
			char exec[1024];
			char preloadlib[1024];
			if(getenv("COMP_PER_REP") != NULL) {
				if(strcmp(getenv("COMP_PER_REP"),"-1")) {
					setenv("MV2_UD_RETRY_COUNT","32768",1);
				}
			} else if(getenv("CMP_RATIO") != NULL) {
				setenv("MV2_UD_RETRY_COUNT","32768",1);
			}
			//setenv("MV2_USE_UD_HYBRID","0",1);
			sprintf(preloadlib,"%s/lib/empi_hack.so",getenv("PAREP_MPI_PATH"));
			setenv("LD_PRELOAD",preloadlib,1);
			setenv("PAREP_MPI_PROXY_HACKED","1",1);
			sprintf(exec,"%s/bin/mpirun",getenv("PAREP_MPI_EMPI_PATH"));
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
					int waitpid_ret = waitpid(empi_pid,&stat,0);
					if(waitpid_ret < 0) {
						if(errno == ECHILD) {
							break;
						}
					}
				} while(!WIFEXITED(stat));
				do {
					int waitpid_ret = waitpid(coordinator_pid,&stat,0);
					if(waitpid_ret < 0) {
						if(errno == ECHILD) {
							break;
						}
					}
				} while(!WIFEXITED(stat));
				completed = WEXITSTATUS(stat);
				if(completed == RET_INCOMPLETE) {
					if(failure_injector_pid != -1) {
						int ret = kill(failure_injector_pid,SIGUSR1);
					}
				}
			}
		}
	} while(completed == RET_INCOMPLETE);
	assert(completed == RET_COMPLETED);
	if(getenv("PAREP_MPI_MTBF") != NULL) kill(failure_injector_pid,SIGKILL);
	
	sprintf(snfile,"%s/checkpoint/pbs_nodes%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
	source = fopen(snfile, "w");
	if(source == NULL) {
		printf("Error opening source file.\n");
		return 1;
	}
	
	sprintf(cppath,"%s/checkpoint/nodecopy%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
	cpfile = fopen(cppath, "r");
	if(cpfile == NULL) {
		fclose(source);
		printf("Error opening cpfile file.\n");
		return 1;
	}
	
	while ((ch = fgetc(cpfile)) != EOF) {
		fputc(ch, source);
	}
	
	fclose(source);
	fclose(cpfile);
	
	int rmret;
	do {
		rmret = remove(cppath);
	} while((rmret != 0) && (errno != ENOENT));
	char downpath[256];
	sprintf(downpath,"%s/checkpoint/downfile%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
	do {
		rmret = remove(downpath);
	} while((rmret != 0) && (errno != ENOENT));
	return 0;
}