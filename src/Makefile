CC=gcc
LIBDIR=/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi/lib

lib: mpiProfiler.o
	$(CC) mpiProfiler.o -shared -o ${LIBDIR}/libmpi.so

mpiProfiler.o: mpiProfiler.c
	$(CC) -o mpiProfiler.o -fPIC -c mpiProfiler.c

clean:
	rm -f mpiProfiler.o ${LIBDIR}/libmpi.so
