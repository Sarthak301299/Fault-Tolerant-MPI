CC=gcc
LIBDIR=$(PAREP_MPI_PATH)/lib
INCLPATH=-I$(PAREP_MPI_LIBUNWIND_PATH)/include
LIBPATH=-L$(PAREP_MPI_LIBUNWIND_PATH)/lib

HEADERS = mpi-internal.h mpi.h declarations.h full_context.h ibvctx.h mmanager.h mpiProfilerdefs.h ompideclarations.h ompitypedefs.h typedefs.h parep_mpi_assembly.h list.h ibv_internal.h ibvidentifier.h debug.h wrappers.h client_thread.h request_handler.h heap_allocator.h commbuf_cache.h
LIBOBJS = mpiProfiler.o full_context.o mmanager.o parep_mpi_assembly.o infinibandwrappers.o ibvctx.o list.o wrappers.o client_thread.o request_handler.o heap_allocator.o commbuf_cache.o

lib: ${LIBOBJS}
	$(CC) $(LIBOBJS) -shared -o ${LIBDIR}/libmpi.so $(LIBPATH) -ldl -libverbs -lpthread -lunwind -lstdc++
	
get_qp_from_pointer.ic: get_XX_from_pointer.ic
	sed -e 's%XX%qp%g' get_XX_from_pointer.ic > get_qp_from_pointer.ic
get_cq_from_pointer.ic: get_XX_from_pointer.ic
	sed -e 's%XX%cq%g' get_XX_from_pointer.ic > get_cq_from_pointer.ic
get_srq_from_pointer.ic: get_XX_from_pointer.ic
	sed -e 's%XX%srq%g' get_XX_from_pointer.ic > get_srq_from_pointer.ic
ibv_wr_ops_send.ic: ibv_wr_ops.ic
	sed -e 's%SENDRECV%send%g' ibv_wr_ops.ic > ibv_wr_ops_send.ic
ibv_wr_ops_recv.ic: ibv_wr_ops.ic
	sed -e 's%SENDRECV%recv%g' ibv_wr_ops.ic > ibv_wr_ops_recv.ic
ibvctx.c: ibv_wr_ops_send.ic ibv_wr_ops_recv.ic get_qp_from_pointer.ic get_cq_from_pointer.ic get_srq_from_pointer.ic

%.o : %.c ${HEADERS}
	$(CC) $(INCLPATH) -fPIC -c -o $@ $<

%.o : %.S ${HEADERS}
	$(CC) $(INCLPATH) -fPIC -c -o $@ $<

clean:
	rm -f ${LIBOBJS} ${LIBDIR}/libmpi.so