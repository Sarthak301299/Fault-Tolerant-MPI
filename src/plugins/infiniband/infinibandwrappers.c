#define _GNU_SOURCE

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <infiniband/verbs.h>
#include "ibvctx.h"

int
ibv_fork_init(void)
{
  int rslt = _fork_init();
  
  return rslt;
}

struct ibv_device **
ibv_get_device_list(int *num_devices)
{

  struct ibv_device **result = _get_device_list(num_devices);

  
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

  struct ibv_context *user_copy = _open_device(dev);

  
  return user_copy;
}

int
ibv_query_device(struct ibv_context *context,
                 struct ibv_device_attr *device_attr)
{

  int rslt = _query_device(context, device_attr);

  
  return rslt;
}

// ibv_query_port is defined as a macro in verbs.h
#undef ibv_query_port
int ibv_query_port(struct ibv_context *context, uint8_t port_num,
                   struct _compat_ibv_port_attr *port_attr)
{
  

  int rslt = _query_port(context, port_num, (struct ibv_port_attr *)port_attr);

  
  return rslt;
}

int
ibv_query_pkey(struct ibv_context *context,
               uint8_t port_num,
               int index,
               uint16_t *pkey)
{

  int rslt = _query_pkey(context, port_num, index, pkey);

  
  return rslt;
}

int
ibv_query_gid(struct ibv_context *context,
              uint8_t port_num,
              int index,
              union ibv_gid *gid)
{

  int rslt = _query_gid(context, port_num, index, gid);

  
  return rslt;
}

__be64
ibv_get_device_guid(struct ibv_device *device)
{

  uint64_t rslt = _get_device_guid(device);

  
  return rslt;
}

struct ibv_comp_channel *
ibv_create_comp_channel(struct ibv_context *context)
{

  struct ibv_comp_channel *rslt = _create_comp_channel(context);

  
  return rslt;
}

int
ibv_destroy_comp_channel(struct ibv_comp_channel *channel)
{

  int rslt = _destroy_comp_channel(channel);

  
  return rslt;
}

struct ibv_pd *
ibv_alloc_pd(struct ibv_context *context)
{

  struct ibv_pd *user_copy = _alloc_pd(context);

  
  return user_copy;
}

#undef ibv_reg_mr
struct ibv_mr *
ibv_reg_mr(struct ibv_pd *pd, void *addr, size_t length, int access) // int
                                                                     // access)
{

  struct ibv_mr *user_copy = _reg_mr(pd, addr, length, access);

  
  return user_copy;
}

struct ibv_cq *
ibv_create_cq(struct ibv_context *context,
              int cqe,
              void *cq_context,
              struct ibv_comp_channel *channel,
              int comp_vector)
{

  struct ibv_cq *user_copy = _create_cq(context, cqe, cq_context,
                                        channel, comp_vector);

  
  return user_copy;
}

struct ibv_srq *
ibv_create_srq(struct ibv_pd *pd, struct ibv_srq_init_attr *srq_init_attr)
{
  struct ibv_srq *user_copy = _create_srq(pd, srq_init_attr);

  
  return user_copy;
}

int
ibv_modify_srq(struct ibv_srq *srq,
               struct ibv_srq_attr *srq_attr,
               int srq_attr_mask)
{
  int rslt = _modify_srq(srq, srq_attr, srq_attr_mask);
  
  return rslt;
}

int
ibv_query_srq(struct ibv_srq *srq, struct ibv_srq_attr *srq_attr)
{
  int rslt = _query_srq(srq, srq_attr);
  
  return rslt;
}

int
ibv_destroy_srq(struct ibv_srq *srq)
{
  int rslt = _destroy_srq(srq);
  
  return rslt;
}

struct ibv_qp *
ibv_create_qp(struct ibv_pd *pd, struct ibv_qp_init_attr *qp_init_attr)
{

  struct ibv_qp *user_copy = _create_qp(pd, qp_init_attr);

  
  return user_copy;
}

int
ibv_modify_qp(struct ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask) // int
                                                                          // attr_mask)
{

  int rslt = _modify_qp(qp, attr, attr_mask);

  
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

  int rslt = _query_qp(qp, attr, attr_mask, init_attr);

  
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

  _ack_async_event(event);

  
}

int
ibv_resize_cq(struct ibv_cq *cq, int cqe)
{

  int rslt = _resize_cq(cq, cqe);

  
  return rslt;
}

int
ibv_destroy_cq(struct ibv_cq *cq)
{

  int rslt = _destroy_cq(cq);

  
  return rslt;
}

int
ibv_destroy_qp(struct ibv_qp *qp)
{

  int rslt = _destroy_qp(qp);

  
  return rslt;
}

int
ibv_dereg_mr(struct ibv_mr *mr)
{

  int rslt = _dereg_mr(mr);

  
  return rslt;
}

int
ibv_dealloc_pd(struct ibv_pd *pd)
{

  int rslt = _dealloc_pd(pd);

  
  return rslt;
}

int
ibv_close_device(struct ibv_context *context)
{

  int rslt = _close_device(context);

  
  return rslt;
}

void
ibv_free_device_list(struct ibv_device **list)
{

  _free_device_list(list);

  
}

void
ibv_ack_cq_events(struct ibv_cq *cq, unsigned int nevents)
{

  _ack_cq_events(cq, nevents);

  
}

struct ibv_ah *
ibv_create_ah(struct ibv_pd *pd, struct ibv_ah_attr *attr)
{

  struct ibv_ah *rslt = _create_ah(pd, attr);

  
  return rslt;
}

int
ibv_destroy_ah(struct ibv_ah *ah)
{
  int rslt;
  

  rslt = _destroy_ah(ah);

  

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
