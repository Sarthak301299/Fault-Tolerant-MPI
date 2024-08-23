#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

int main(int argc, char **argv) {
	int i;
	int exepoint;
	int empi_argc = 0;
	char parep_mpi_empi_argc[64];
	char *executable = (char *)malloc(1000*sizeof(char));
	for(i=1; i < argc; i++) {
		if(argv[i][0] == '-') {
			for(int j=0; j < strlen(argv[i]); j++) {
				if(argv[i][j] == '=') {
					i--;
					break;
				}
			}
			i++;
		}
		else {
			break;
		}
	}
	exepoint = i;
	char **new_argv;
	
	new_argv = (char **)malloc((i+2) * sizeof(char *));
	for(int j = 0; j < i; j++) {
		//new_argv[j] = (char *)malloc(strlen(argv[j])+1);
		//strcpy(new_argv[j],argv[j]);
		new_argv[j] = (char *)malloc(strlen(argv[j])+1);
		strcpy(new_argv[j],argv[j]);
	}
	new_argv[i] = (char *)malloc(100*sizeof(char));
	strcpy(new_argv[i],"parep_mpi_daemon");
	new_argv[i+1] = NULL;
	
	char *daemonargs = (char *)malloc(4096*sizeof(char));
	strcpy(daemonargs, argv[i]);
	i++;
	empi_argc++;
	for(; i < argc; i++)
	{
		strcat(daemonargs," ");
		strcat(daemonargs,argv[i]);
		empi_argc++;
	}
	sprintf(parep_mpi_empi_argc,"%d",empi_argc);
	setenv("PAREP_MPI_EMPI_ARGC",parep_mpi_empi_argc,1);
	setenv("PAREP_MPI_EMPI_DAEMON",daemonargs,1);
	unsetenv("LD_PRELOAD");
	sprintf(executable,"%s",getenv("PAREP_MPI_SRUN"));
	fflush(stdout);
	extern char **environ;
	execve(executable,new_argv,environ);
}
