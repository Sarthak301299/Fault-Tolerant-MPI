PLUGIN=infiniband
CC=gcc
PAREP_MPI_WORKDIR=/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi

LIBNAME=$(PAREP_MPI_WORKDIR)/lib/libparepmpi_${PLUGIN}.so

PAREP_MPI_BIN = $(PAREP_MPI_WORKDIR)/bin
PAREP_MPI_INCLUDE_PATH = $(PAREP_MPI_WORKDIR)/src/plugins

INCLUDES=-I$(PAREP_MPI_INCLUDE_PATH)
LIB_LDFLAGS=-libverbs -shared

CFLAGS=-fPIC
CXXFLAGS=-fPIC

COMPILE=$(CC) $(INCLUDES) $(CFLAGS) -c -o $@
LINK=$(CC) $(CFLAGS) -o $@

HEADERS = $(PAREP_MPI_INCLUDE_PATH)/plugin-manager.h lib/list.h ibvctx.h ibv_internal.h ibvidentifier.h debug.h

LIBOBJS = infinibandwrappers.o ibvctx.o lib/list.o
TRACEOBJS = infinibandtrace.o

all: default

default: ${LIBNAME}

libibtrace.so: ${TRACEOBJS}
	${CC} -shared -fPIC -o $@ $^

${LIBNAME}: ${LIBOBJS}
	@rm -f ${LIBNAME} libinfiniband.so
	$(LINK) $(LIBOBJS) $(LIB_LDFLAGS)
	ln -sf ${LIBNAME} libinfiniband.so

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

%.o: %.c ${HEADERS}
	$(COMPILE) $<

clean:
	rm -f ${LIBOBJS} ${LIBNAME} libinfiniband.so
	rm -f ${TRACEOBJS} libibtrace.so
	rm -f ibv_wr_ops_send.ic ibv_wr_ops_recv.ic get_srq_from_pointer.ic get_qp_from_pointer.ic get_cq_from_pointer.ic

.PHONY: default all build clean