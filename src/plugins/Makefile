CC=gcc
PAREP_MPI_WORKDIR=/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi

all: parep_mpi_ckpt_coordinator.c
	$(CC) parep_mpi_ckpt_coordinator.c -o $(PAREP_MPI_WORKDIR)/bin/parep_mpi_ckpt_coordinator -lpthread

clean:
	rm -f $(PAREP_MPI_WORKDIR)/bin/parep_mpi_ckpt_coordinator