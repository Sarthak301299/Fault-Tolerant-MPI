CC=gcc
PAREP_MPI_WORKDIR=/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi

all: proxy_hack.c proxy_loader.c
	$(CC) -fPIC -shared -o $(PAREP_MPI_WORKDIR)/lib/proxy_hack.so proxy_hack.c -ldl
	$(CC) proxy_loader.c -o $(PAREP_MPI_WORKDIR)/bin/parep_mpi_empi_daemon

clean:
	rm -f $(PAREP_MPI_WORKDIR)/lib/proxy_hack.so
	rm -f $(PAREP_MPI_WORKDIR)/bin/parep_mpi_empi_daemon