#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
	char argc_str[16];
	sprintf(argc_str, "%d", argc);
	if(setenv("PAREP_MPI_EMPI_ARGC", argc_str, 1) != 0) {
		perror("setenv PAREP_MPI_EMPI_ARGC failed");
		return 1;
	}
	const char* parep_mpi_path = getenv("PAREP_MPI_PATH");
	if (parep_mpi_path == NULL) {
		fprintf(stderr, "Error: PAREP_MPI_PATH not set in environment.\n");
		return 1;
	}
	char daemon_exec_path[1024];
	sprintf(daemon_exec_path, "/home/sarthakjoshi/MVAPICH2/bin/hydra_pmi_proxy");
	if (setenv("PAREP_MPI_EMPI_DAEMON_EXEC", daemon_exec_path, 1) != 0) {
		perror("setenv PAREP_MPI_EMPI_DAEMON_EXEC failed");
		return 1;
	}
	
	char *daemon_command = strdup(daemon_exec_path);
	size_t current_len = strlen(daemon_exec_path);
	for (int i = 1; i < argc; i++) {
		size_t arg_len = strlen(argv[i]);
		daemon_command = realloc(daemon_command, current_len + arg_len + 2);
		if (daemon_command == NULL) {
			perror("realloc failed");
			return 1;
		}
		strcat(daemon_command, " ");
		strcat(daemon_command, argv[i]);
		current_len += arg_len + 1;
	}
	
	if (setenv("PAREP_MPI_EMPI_DAEMON", daemon_command, 1) != 0) {
		perror("setenv PAREP_MPI_EMPI_DAEMON failed");
		free(daemon_command);
		return 1;
	}
	free(daemon_command);
	
	printf("LD_PRELOAD before %s\n", getenv("LD_PRELOAD") ? getenv("LD_PRELOAD") : "");
	printf("PAREP_MPI_EMPI_DAEMON %s\n", getenv("PAREP_MPI_EMPI_DAEMON"));
	
	if (unsetenv("LD_PRELOAD") != 0) {
		perror("unsetenv LD_PRELOAD failed");
	}
	
	printf("LD_PRELOAD after %s\n", getenv("LD_PRELOAD") ? getenv("LD_PRELOAD") : "");
	printf("PAREP_MPI_EMPI_DAEMON %s\n", getenv("PAREP_MPI_EMPI_DAEMON"));
	
	execlp("parep_mpi_daemon", "parep_mpi_daemon", (char *) NULL);
	perror("execlp failed");
	return 1;
}