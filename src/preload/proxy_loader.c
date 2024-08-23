#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
void main(int argc, char **argv) {
	char preloadlib[1000];
	char executable[1000];
	strcpy(preloadlib,"/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi/lib/proxy_hack.so");
	if(getenv("PAREP_MPI_EMPI_DAEMON_EXEC") != NULL) strcpy(executable,getenv("PAREP_MPI_EMPI_DAEMON_EXEC"));
	setenv("LD_PRELOAD",preloadlib,1);
	if(getenv("PAREP_MPI_EMPI_DAEMON_EXEC") != NULL) setenv("PAREP_MPI_PROXY_HACKED","1",1);
	else setenv("PAREP_MPI_PROXY_HACKED","0",1);
	char **newargv;
	newargv = (char **)malloc(sizeof(char *)*(argc+1));
	for(int i = 0; i < argc; i++) {
		newargv[i] = (char *)malloc(strlen(argv[i])+1);
		strcpy(newargv[i],argv[i]);
	}
	newargv[argc] = NULL;
	free(newargv[0]);
	newargv[0] = (char *)malloc(sizeof(executable));
	strcpy(newargv[0],executable);
	char path[4096];
	sprintf(path,"/home/phd/21/cdsjsar/MVAPICH2/bin:%s",getenv("PATH"));
	setenv("PATH",path,1);
	extern char **environ;
	execve(executable,newargv,environ);
	printf("Execve failed\n");
}