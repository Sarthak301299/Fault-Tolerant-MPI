CC=gcc

all: proxy_hack.c proxy_loader.c empi_hack.c hydra_pmi_proxy.c
	$(CC) -fPIC -shared -o $(PAREP_MPI_PATH)/lib/proxy_hack.so proxy_hack.c -ldl
	$(CC) -fPIC -shared -o $(PAREP_MPI_PATH)/lib/empi_hack.so empi_hack.c -ldl
	$(CC) proxy_loader.c -o $(PAREP_MPI_PATH)/bin/parep_mpi_empi_daemon
	$(CC) hydra_pmi_proxy.c -o $(PAREP_MPI_PATH)/bin/parep_mpi_srun/hydra_pmi_proxy
	$(CC) ssh.c -o $(PAREP_MPI_PATH)/bin/ssh

clean:
	rm -f $(PAREP_MPI_PATH)/lib/proxy_hack.so
	rm -f $(PAREP_MPI_PATH)/lib/empi_hack.so
	rm -f $(PAREP_MPI_PATH)/bin/parep_mpi_empi_daemon
	rm -f $(PAREP_MPI_PATH)/bin/parep_mpi_srun/hydra_pmi_proxy
	rm -f $(PAREP_MPI_PATH)/bin/ssh