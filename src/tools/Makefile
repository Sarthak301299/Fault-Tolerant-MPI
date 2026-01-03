CC=gcc

all: parep_mpi_ckpt_coordinator.c parep_mpi_ckpt_coordinator.h mpirun.c parep_mpi_daemon.c srun.c
	$(CC) parep_mpi_ckpt_coordinator.c -o $(PAREP_MPI_PATH)/bin/parep_mpi_ckpt_coordinator -lpthread
	$(CC) mpirun.c -o $(PAREP_MPI_PATH)/bin/mpirun -lm
	$(CC) parep_mpi_failure_injector.c -o $(PAREP_MPI_PATH)/bin/parep_mpi_failure_injector -lm
	$(CC) parep_mpi_daemon.c -o $(PAREP_MPI_PATH)/bin/parep_mpi_daemon
	$(CC) srun.c -o $(PAREP_MPI_PATH)/bin/parep_mpi_srun/srun

clean:
	rm -f $(PAREP_MPI_PATH)/bin/parep_mpi_ckpt_coordinator
	rm -f $(PAREP_MPI_PATH)/bin/mpirun
	rm -f $(PAREP_MPI_PATH)/bin/parep_mpi_daemon
	rm -f $(PAREP_MPI_PATH)/bin/srun