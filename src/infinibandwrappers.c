#define _GNU_SOURCE

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <infiniband/verbs.h>
#include "wrappers.h"
#include "ibvctx.h"

int
ibv_fork_init(void)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _fork_init();
  PAREP_MPI_ENABLE_CKPT();
  return rslt;
}

struct ibv_device **
ibv_get_device_list(int *num_devices)
{
	PAREP_MPI_DISABLE_CKPT();
  struct ibv_device **result = _get_device_list(num_devices);
	PAREP_MPI_ENABLE_CKPT();
  
  return result;
}

const char *
ibv_get_device_name(struct ibv_device *dev)
{
  const char *rslt = _get_device_name(dev);
  // 
  return rslt;
}

struct ibv_context *
ibv_open_device(struct ibv_device *dev)
{
	PAREP_MPI_DISABLE_CKPT();
  struct ibv_context *user_copy = _open_device(dev);
	PAREP_MPI_ENABLE_CKPT();
  
  return user_copy;
}

int
ibv_query_device(struct ibv_context *context,
                 struct ibv_device_attr *device_attr)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _query_device(context, device_attr);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

// ibv_query_port is defined as a macro in verbs.h
#undef ibv_query_port
int ibv_query_port(struct ibv_context *context, uint8_t port_num,
                   struct _compat_ibv_port_attr *port_attr)
{
  PAREP_MPI_DISABLE_CKPT();
  int rslt = _query_port(context, port_num, (struct ibv_port_attr *)port_attr);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

int
ibv_query_pkey(struct ibv_context *context,
               uint8_t port_num,
               int index,
               uint16_t *pkey)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _query_pkey(context, port_num, index, pkey);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

int
ibv_query_gid(struct ibv_context *context,
              uint8_t port_num,
              int index,
              union ibv_gid *gid)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _query_gid(context, port_num, index, gid);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

__be64
ibv_get_device_guid(struct ibv_device *device)
{
	PAREP_MPI_DISABLE_CKPT();
  uint64_t rslt = _get_device_guid(device);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

struct ibv_comp_channel *
ibv_create_comp_channel(struct ibv_context *context)
{
	PAREP_MPI_DISABLE_CKPT();
  struct ibv_comp_channel *rslt = _create_comp_channel(context);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

int
ibv_destroy_comp_channel(struct ibv_comp_channel *channel)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _destroy_comp_channel(channel);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

struct ibv_pd *
ibv_alloc_pd(struct ibv_context *context)
{
	PAREP_MPI_DISABLE_CKPT();
  struct ibv_pd *user_copy = _alloc_pd(context);
	PAREP_MPI_ENABLE_CKPT();
  
  return user_copy;
}

#undef ibv_reg_mr
struct ibv_mr *
ibv_reg_mr(struct ibv_pd *pd, void *addr, size_t length, int access) // int
                                                                     // access)
{
	PAREP_MPI_DISABLE_CKPT();
  struct ibv_mr *user_copy = _reg_mr(pd, addr, length, access);
	PAREP_MPI_ENABLE_CKPT();
  
  return user_copy;
}

#undef ibv_reg_mr_iova
struct ibv_mr *
ibv_reg_mr_iova(struct ibv_pd *pd, void *addr, size_t length, uint64_t iova, int access) // int
                                                                     // access)
{
	PAREP_MPI_DISABLE_CKPT();
  struct ibv_mr *user_copy = _reg_mr_iova(pd, addr, length, iova, access);
	PAREP_MPI_ENABLE_CKPT();
  
  return user_copy;
}

struct ibv_mr *
ibv_reg_mr_iova2(struct ibv_pd *pd, void *addr, size_t length, uint64_t iova, unsigned int access) // int
                                                                     // access)
{
	PAREP_MPI_DISABLE_CKPT();
  struct ibv_mr *user_copy = _reg_mr_iova2(pd, addr, length, iova, access);
	PAREP_MPI_ENABLE_CKPT();
  
  return user_copy;
}

struct ibv_cq *
ibv_create_cq(struct ibv_context *context,
              int cqe,
              void *cq_context,
              struct ibv_comp_channel *channel,
              int comp_vector)
{
	PAREP_MPI_DISABLE_CKPT();
  struct ibv_cq *user_copy = _create_cq(context, cqe, cq_context,
                                        channel, comp_vector);
	PAREP_MPI_ENABLE_CKPT();
  
  return user_copy;
}

struct ibv_srq *
ibv_create_srq(struct ibv_pd *pd, struct ibv_srq_init_attr *srq_init_attr)
{
	PAREP_MPI_DISABLE_CKPT();
  struct ibv_srq *user_copy = _create_srq(pd, srq_init_attr);
	PAREP_MPI_ENABLE_CKPT();
  
  return user_copy;
}

int
ibv_modify_srq(struct ibv_srq *srq,
               struct ibv_srq_attr *srq_attr,
               int srq_attr_mask)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _modify_srq(srq, srq_attr, srq_attr_mask);
	PAREP_MPI_ENABLE_CKPT();
  return rslt;
}

int
ibv_query_srq(struct ibv_srq *srq, struct ibv_srq_attr *srq_attr)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _query_srq(srq, srq_attr);
	PAREP_MPI_ENABLE_CKPT();
  return rslt;
}

int
ibv_destroy_srq(struct ibv_srq *srq)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _destroy_srq(srq);
	PAREP_MPI_ENABLE_CKPT();
  return rslt;
}

struct ibv_qp *
ibv_create_qp(struct ibv_pd *pd, struct ibv_qp_init_attr *qp_init_attr)
{
	PAREP_MPI_DISABLE_CKPT();
  struct ibv_qp *user_copy = _create_qp(pd, qp_init_attr);
	PAREP_MPI_ENABLE_CKPT();
  
  return user_copy;
}

int
ibv_modify_qp(struct ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask) // int
                                                                          // attr_mask)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _modify_qp(qp, attr, attr_mask);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

int
ibv_get_cq_event(struct ibv_comp_channel *channel,
                 struct ibv_cq **cq,
                 void **cq_context)
{
  int rslt = _get_cq_event(channel, cq, cq_context);

  return rslt;
}

int
ibv_query_qp(struct ibv_qp *qp,
             struct ibv_qp_attr *attr,
             int attr_mask,
             struct ibv_qp_init_attr *init_attr)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _query_qp(qp, attr, attr_mask, init_attr);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

int
ibv_get_async_event(struct ibv_context *context, struct ibv_async_event *event)
{
  int rslt = _get_async_event(context, event);

  return rslt;
}

void
ibv_ack_async_event(struct ibv_async_event *event)
{
	PAREP_MPI_DISABLE_CKPT();
  _ack_async_event(event);
	PAREP_MPI_ENABLE_CKPT();
  
}

int
ibv_resize_cq(struct ibv_cq *cq, int cqe)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _resize_cq(cq, cqe);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

int
ibv_destroy_cq(struct ibv_cq *cq)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _destroy_cq(cq);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

int
ibv_destroy_qp(struct ibv_qp *qp)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _destroy_qp(qp);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

int
ibv_dereg_mr(struct ibv_mr *mr)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _dereg_mr(mr);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

int
ibv_dealloc_pd(struct ibv_pd *pd)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _dealloc_pd(pd);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

int
ibv_close_device(struct ibv_context *context)
{
	PAREP_MPI_DISABLE_CKPT();
  int rslt = _close_device(context);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

void
ibv_free_device_list(struct ibv_device **list)
{
	PAREP_MPI_DISABLE_CKPT();
  _free_device_list(list);
	PAREP_MPI_ENABLE_CKPT();
  
}

void
ibv_ack_cq_events(struct ibv_cq *cq, unsigned int nevents)
{
	PAREP_MPI_DISABLE_CKPT();
  _ack_cq_events(cq, nevents);
	PAREP_MPI_ENABLE_CKPT();
  
}

struct ibv_ah *
ibv_create_ah(struct ibv_pd *pd, struct ibv_ah_attr *attr)
{
	PAREP_MPI_DISABLE_CKPT();
  struct ibv_ah *rslt = _create_ah(pd, attr);
	PAREP_MPI_ENABLE_CKPT();
  
  return rslt;
}

int
ibv_destroy_ah(struct ibv_ah *ah)
{
  int rslt;
	PAREP_MPI_DISABLE_CKPT();
  rslt = _destroy_ah(ah);
	PAREP_MPI_ENABLE_CKPT();
  

  return rslt;
}

/*
 * The following are some unimplemented functionalities, including:
 *
 * Reregistering memory regions
 *
 * Multicast support
 *
 * TODO: Adding XRC (eXtended Reliable Connected) functionalities
 *
 */

int ibv_rereg_mr(struct ibv_mr *mr, int flags,
                 struct ibv_pd *pd, void *addr,
                 size_t length, int access)
{
  return NEXT_IBV_FNC(ibv_rereg_mr)(mr, flags,
                                    pd, addr,
                                    length, access);
}

int ibv_attach_mcast(struct ibv_qp *qp, const union ibv_gid *gid,
                     uint16_t lid)
{
  return NEXT_IBV_FNC(ibv_attach_mcast)(qp, gid, lid);
}

int ibv_detach_mcast(struct ibv_qp *qp, const union ibv_gid *gid,
                     uint16_t lid)
{
  return NEXT_IBV_FNC(ibv_detach_mcast)(qp, gid, lid);
}
