//#define _GNU_SOURCE
#ifndef _MPI_H_
#define _MPI_H_

#define PAREP_MPI_DEFINED 1
 
#include "mpiProfilerdefs.h"
//#include <sys/mman.h>
extern long _ZSt4cout;
#ifdef __cplusplus
extern "C" {
#endif

#define libc_malloc(size) _real_malloc(size)
#define libc_free(p) _real_free(p)

//#define malloc(size) ft_malloc(size)
//#define free(p) ft_free(p)

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

int MPI_Ibcast (void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request *);
int MPI_Iscatter (const void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request *);
int MPI_Igather (const void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request *);
int MPI_Ireduce (void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm, MPI_Request *);
int MPI_Iallgather (const void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm, MPI_Request *);
int MPI_Ialltoall (void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm, MPI_Request *);
int MPI_Iallreduce (void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm, MPI_Request *);
int MPI_Ialltoallv(void *sendbuf, int *sendcounts, int *sdispls, MPI_Datatype sendtype, void *recvbuf, int *recvcounts, int *rdispls, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *);

double MPI_Wtime();
int MPI_Barrier(MPI_Comm);
int MPI_Ibarrier(MPI_Comm, MPI_Request *);

int MPI_File_open(MPI_Comm, const char *, int, MPI_Info, MPI_File *);
int MPI_File_close(MPI_File *);
int MPI_File_set_view(MPI_File, MPI_Offset, MPI_Datatype, MPI_Datatype, const char *, MPI_Info);
int MPI_File_get_view(MPI_File, MPI_Offset *, MPI_Datatype *, MPI_Datatype *, char *);
int MPI_File_seek(MPI_File, MPI_Offset, int);
int MPI_File_seek_shared(MPI_File, MPI_Offset, int);
int MPI_File_read_at(MPI_File, MPI_Offset, void *, int, MPI_Datatype, MPI_Status *);
int MPI_File_iread_at(MPI_File, MPI_Offset, void *, int, MPI_Datatype, MPI_Request *);
int MPI_File_write_at(MPI_File, MPI_Offset, const void *, int, MPI_Datatype, MPI_Status *);
int MPI_File_iwrite_at(MPI_File, MPI_Offset, const void *, int, MPI_Datatype, MPI_Request *);
int MPI_File_read(MPI_File, void *, int, MPI_Datatype, MPI_Status *);
int MPI_File_iread(MPI_File, void *, int, MPI_Datatype, MPI_Request *);
int MPI_File_write(MPI_File, const void *, int, MPI_Datatype, MPI_Status *);
int MPI_File_iwrite(MPI_File, const void *, int, MPI_Datatype, MPI_Request *);
int MPI_File_read_shared(MPI_File, void *, int, MPI_Datatype, MPI_Status *);
int MPI_File_iread_shared(MPI_File, void *, int, MPI_Datatype, MPI_Request *);
int MPI_File_write_shared(MPI_File, const void *, int, MPI_Datatype, MPI_Status *);
int MPI_File_iwrite_shared(MPI_File, const void *, int, MPI_Datatype, MPI_Request *);
int MPI_File_read_at_all(MPI_File, MPI_Offset, void *, int, MPI_Datatype, MPI_Status *);
int MPI_File_iread_at_all(MPI_File, MPI_Offset, void *, int, MPI_Datatype, MPI_Request *);
int MPI_File_write_at_all(MPI_File, MPI_Offset, const void *, int, MPI_Datatype, MPI_Status *);
int MPI_File_iwrite_at_all(MPI_File, MPI_Offset, const void *, int, MPI_Datatype, MPI_Request *);
int MPI_File_read_all(MPI_File, void *, int, MPI_Datatype, MPI_Status *);
int MPI_File_iread_all(MPI_File, void *, int, MPI_Datatype, MPI_Request *);
int MPI_File_write_all(MPI_File, const void *, int, MPI_Datatype, MPI_Status *);
int MPI_File_iwrite_all(MPI_File, const void *, int, MPI_Datatype, MPI_Request *);
int MPI_File_iwrite_all_even(MPI_File, const void *, int, MPI_Datatype, MPI_Request *);
int MPI_File_read_ordered(MPI_File, void *, int, MPI_Datatype, MPI_Status *);
int MPI_File_read_ordered_begin(MPI_File, void *, int, MPI_Datatype);
int MPI_File_read_ordered_end(MPI_File, void *, MPI_Status *);
int MPI_File_write_ordered(MPI_File, const void *, int, MPI_Datatype, MPI_Status *);
int MPI_File_write_ordered_begin(MPI_File, const void *, int, MPI_Datatype);
int MPI_File_write_ordered_end(MPI_File, const void *, MPI_Status *);
int MPI_File_read_all_begin(MPI_File, void *, int, MPI_Datatype);
int MPI_File_read_all_end(MPI_File, void *, MPI_Status *);
int MPI_File_write_all_begin(MPI_File, const void *, int, MPI_Datatype);
int MPI_File_write_all_end(MPI_File, const void *, MPI_Status *);
int MPI_File_read_at_all_begin(MPI_File, MPI_Offset, void *, int, MPI_Datatype);
int MPI_File_read_at_all_end(MPI_File, void *, MPI_Status *);
int MPI_File_write_at_all_begin(MPI_File, MPI_Offset, const void *, int, MPI_Datatype);
int MPI_File_write_at_all_end(MPI_File, const void *, MPI_Status *);
int MPI_File_get_position(MPI_File, MPI_Offset *);
int MPI_File_get_position_shared(MPI_File, MPI_Offset *);

int MPI_Type_commit(MPI_Datatype *);
int MPI_Type_free(MPI_Datatype *);
int MPI_Type_contiguous(int, MPI_Datatype, MPI_Datatype *);
int MPI_Type_vector(int, int, int, MPI_Datatype, MPI_Datatype *);
int MPI_Type_create_subarray(int, const int *, const int *, const int *, int, MPI_Datatype, MPI_Datatype *);

int MPI_File_sync(MPI_File fh);
int MPI_File_delete(const char *, MPI_Info);

int MPI_Get_processor_name(char *, int *);
int MPI_Info_create(MPI_Info *);

int MPI_Type_size(MPI_Datatype, int *);
int MPI_Type_get_extent(MPI_Datatype, MPI_Aint *, MPI_Aint *);

int MPI_Abort(MPI_Comm, int);

int MPI_Checkpoint_start();

int MPI_Replica_rearrange(int *, int *);
#ifdef __cplusplus
}
#endif
#endif
