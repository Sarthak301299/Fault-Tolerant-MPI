#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define REAL_SSH "/usr/bin/ssh"
#define PROXY_PATH "/home/sarthakjoshi/MVAPICH2/bin/hydra_pmi_proxy"
#define INTERCEPT_PATH "/home/sarthakjoshi/fthp-mpi/bin/parep_mpi_srun/hydra_pmi_proxy"

/* List the environment variables you want to "forward" */
const char* vars_to_forward[] = {"SLURM_JOB_ID", "SLURM_JOB_NAME", "SLURM_SUBMIT_DIR", "SLURM_NTASKS", "SLURM_JOB_NUM_NODES", "SLURM_NTASKS_PER_NODE", "MV2_ENABLE_AFFINITY", "MV2_LIBIBUMAD_PATH",
																"MV2_LIBRDMACM_PATH", "PAREP_MPI_MANUAL_CKPT_START", "COMP_PER_REP", "CMP_RATIO", "REP_RATIO", "PAREP_MPI_MTBF", "PAREP_MPI_SIM_LOGS", "PAREP_MPI_SIM_NODEKILL",
																"PAREP_MPI_PREDICT_RECALL", "PAREP_MPI_PREDICT_PRECISION", "PAREP_MPI_COMPUTE_CKPT", "PAREP_MPI_CKPT_TIME", "PAREP_MPI_SIM_PREDICT", "PAREP_MPI_SIZE", "PAREP_MPI_EMPI_DAEMON_EXEC", "PAREP_MPI_NODE_NUM",
																"PAREP_MPI_NODE_SIZE", "PAREP_MPI_WORKDIR", "PAREP_MPI_HEAD_NODE", "PAREP_MPI_MAIN_COORDINATOR_IP", "PAREP_MPI_CKPT_INTERVAL", "MV2_UD_RETRY_COUNT", "LD_PRELOAD", "PAREP_MPI_PROXY_HACKED",
																"MV2_USE_CUDA", "MV2_USE_ZE", "HWLOC_COMPONENTS", "FI_PROVIDER", "FI_CXI_DISABLE_HOST_REGISTER", "FI_CXI_DEFAULT_CQ_SIZE", "PAREP_MPI_MAX_IO_CHUNK_SIZE"};
const int vars_count = 39;

int main(int argc, char *argv[]) {
	// 1. Build the injection string from current environment
	char env_inject[4096] = {0}; 
	for (int i = 0; i < vars_count; i++) {
		char* val = getenv(vars_to_forward[i]);
		if (val) {
			char buffer[1024];
			// Format: export NAME='value'; 
			snprintf(buffer, sizeof(buffer), "export %s='%s'; ", vars_to_forward[i], val);
			strncat(env_inject, buffer, sizeof(env_inject) - strlen(env_inject) - 1);
		}
	}

	// 2. Prepare new argument list
	char **new_argv = malloc((argc + 1) * sizeof(char *));

	for (int i = 0; i < argc; i++) {
		if (strstr(argv[i], PROXY_PATH) != NULL) {
			// Allocate space for: [injected vars] + [original command] + null terminator
			size_t new_len = strlen(env_inject) + strlen(INTERCEPT_PATH) + 1;
			char *modified_cmd = malloc(new_len);
			snprintf(modified_cmd, new_len, "%s%s", env_inject, INTERCEPT_PATH);
			new_argv[i] = modified_cmd;
		} else {
			new_argv[i] = argv[i];
		}
	}
	new_argv[argc] = NULL;
	
	unsetenv("LD_PRELOAD");

	// 3. Execute the real SSH
	// Note: new_argv[0] should technically be the path to the executable
	execv(REAL_SSH, new_argv);

	perror("execv failed");
	return 1;
}
