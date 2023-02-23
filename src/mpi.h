//#define _GNU_SOURCE
#ifndef _MPI_H_
#define _MPI_H_
 
#include "mpiProfilerdefs.h"
//#include <sys/mman.h>
#ifdef __cplusplus
extern "C" {
#endif

int MPI_Finalize(void);
int MPI_Init (int *, char ***);

int MPI_Comm_rank (MPI_Comm, int *);
int MPI_Comm_size (MPI_Comm, int *);

int MPI_Send (void *, int, MPI_Datatype, int, int, MPI_Comm);
int MPI_Isend (void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);

int MPI_Recv (void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
int MPI_Irecv (void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);

int MPI_Request_free(MPI_Request *);
int MPI_Test(MPI_Request *, int *, MPI_Status *);
int MPI_Wait(MPI_Request *, MPI_Status *);

int MPI_Bcast (void *, int, MPI_Datatype, int, MPI_Comm);
int MPI_Scatter (const void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm);
int MPI_Gather (const void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm);
int MPI_Reduce (void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm);
int MPI_Allgather (const void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm);
int MPI_Alltoall (void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm);
int MPI_Allreduce (void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm);
int MPI_Alltoallv(void *sendbuf, int *sendcounts, int *sdispls, MPI_Datatype sendtype, void *recvbuf, int *recvcounts, int *rdispls, MPI_Datatype recvtype, MPI_Comm comm);
double MPI_Wtime();
int MPI_Barrier(MPI_Comm);

int MPI_Abort(MPI_Comm, int);
#ifdef __cplusplus
}
#endif
#endif
