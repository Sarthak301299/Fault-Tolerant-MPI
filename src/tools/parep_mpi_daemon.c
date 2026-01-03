#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

extern char **environ;
int main(int argc, char **argv) {
	char path[4096];
	char node_file[4096];
	bool isHeadNode;
	sprintf(node_file,"%s/checkpoint/pbs_nodes%s",getenv("PAREP_MPI_BASE_WORKDIR"),getenv("SLURM_JOB_ID"));
	int parep_mpi_size = atoi(getenv("PAREP_MPI_SIZE"));
	int parep_mpi_node_num = atoi(getenv("PAREP_MPI_NODE_NUM"));
	int parep_mpi_node_size = atoi(getenv("PAREP_MPI_NODE_SIZE"));
	int parep_mpi_node_id = 0;
	
	char host_name[256];
	int thname = gethostname(host_name,sizeof(host_name));
	if(thname == -1) {
		perror("gethostname");
		exit(1);
	}
	
	FILE *nf = fopen(node_file,"r");
	assert(nf != NULL);
	char prevline[256];
	prevline[0] = '\0';
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;
	while ((nread = getline(&line, &len, nf)) != -1) {
		if(strcmp(line,prevline)) {
			if (nread > 0 && line[nread-1] == '\n') {
				line[nread-1] = '\0';
				if(!strcmp(line,host_name)) {
					break;
				}
				line[nread-1] = '\n';
			} else if(!strcmp(line,host_name)) {
				break;
			}
			parep_mpi_node_id++;
		}
		strcpy(prevline,line);
	}
	free(line);
	line = NULL;
	fclose(nf);
	assert(parep_mpi_node_id < parep_mpi_node_num);
	
	if(parep_mpi_node_id == (parep_mpi_node_num-1)) {
		if(parep_mpi_size < (parep_mpi_node_size*parep_mpi_node_num)) {
			parep_mpi_node_size = parep_mpi_node_size - ((parep_mpi_node_size*parep_mpi_node_num) - parep_mpi_size);
			char temp[64];
			sprintf(temp,"%d",parep_mpi_node_size);
			setenv("PAREP_MPI_NODE_SIZE",temp,1);
		}
	}
	char temp[64];
	sprintf(temp,"%d",parep_mpi_node_id);
	setenv("PAREP_MPI_NODE_ID",temp,1);
	//sprintf(path,"/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi");
	//setenv("PAREP_MPI_PATH",path,1);
	//sprintf(path,"/scratch/cdsjsar/checkpoint/%s",getenv("SLURM_JOB_ID"));
	//setenv("PAREP_MPI_WORKDIR",path,1);
	
	if(strcmp(host_name,getenv("PAREP_MPI_HEAD_NODE")) == 0) {
		assert(parep_mpi_node_id == 0);
		isHeadNode = true;
	} else {
		assert(parep_mpi_node_id != 0);
		isHeadNode = false;
	}
	
	fflush(stdout);
	
	if(isHeadNode) {
		char parep_mpi_empi_daemon[4096];
		int empi_argc = atoi(getenv("PAREP_MPI_EMPI_ARGC"));
		strcpy(parep_mpi_empi_daemon,getenv("PAREP_MPI_EMPI_DAEMON"));
		char **newargv;
		newargv = (char **)malloc(sizeof(char *)*(empi_argc+1));
		char *token;
		int word_count = 0;
		token = strtok(parep_mpi_empi_daemon," \t\n");
		while(token != NULL && word_count < empi_argc) {
			newargv[word_count] = strdup(token);
			token = strtok(NULL, " \t\n");
			word_count++;
		}
		newargv[empi_argc] = NULL;
		char exec[1024];
		setenv("PAREP_MPI_EMPI_DAEMON_EXEC",newargv[0],1);
		free(newargv[0]);
		newargv[0] = (char *)malloc(sizeof(char) * 64);
		sprintf(newargv[0],"%s/bin/parep_mpi_empi_daemon",getenv("PAREP_MPI_PATH"));
		strcpy(exec,newargv[0]);
		execve(exec,newargv,environ);
	} else {
		pid_t childpid = fork();
		if(childpid == -1) {
			perror("fork");
			exit(1);
		}
		
		if(childpid == 0) { //CHILD PROCESS
			char parep_mpi_empi_daemon[4096];
			int empi_argc = atoi(getenv("PAREP_MPI_EMPI_ARGC"));
			strcpy(parep_mpi_empi_daemon,getenv("PAREP_MPI_EMPI_DAEMON"));
			char **newargv;
			newargv = (char **)malloc(sizeof(char *)*(empi_argc+1));
			char *token;
			int word_count = 0;
			token = strtok(parep_mpi_empi_daemon," \t\n");
			while(token != NULL && word_count < empi_argc) {
				newargv[word_count] = strdup(token);
				token = strtok(NULL, " \t\n");
				word_count++;
			}
			newargv[empi_argc] = NULL;
			char exec[1024];
			setenv("PAREP_MPI_EMPI_DAEMON_EXEC",newargv[0],1);
			free(newargv[0]);
			newargv[0] = (char *)malloc(sizeof(char) * 64);
			sprintf(newargv[0],"%s/bin/parep_mpi_empi_daemon",getenv("PAREP_MPI_PATH"));
			strcpy(exec,newargv[0]);
			execve(exec,newargv,environ);
		} else { //PARENT PROCESS
			char exec[1024];
			sprintf(exec,"%s/bin/parep_mpi_ckpt_coordinator",getenv("PAREP_MPI_PATH"));
			char **newargv;
			newargv = (char **)malloc(sizeof(char *)*(2));
			newargv[0] = (char *)malloc(sizeof(exec));
			strcpy(newargv[0],exec);
			newargv[1] = NULL;
			execve(exec,newargv,environ);
		}
	}
	
	return 0;
}