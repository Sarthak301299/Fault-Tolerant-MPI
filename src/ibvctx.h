#define _GNU_SOURCE
#include <dlfcn.h>
#include <infiniband/verbs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __IBVCTX_H__
#define __IBVCTX_H__

// libibverbs.so includes two versions for each symbol, 1.1 for new one, which
// is the default, and 1.0 for the old. We use dlvsym to call the new one.
#define NEXT_IBV_FNC(func)                                            \
  ({                                                                  \
    static __typeof__(&func)_real_ ## func = (__typeof__(&func)) - 1; \
    if (_real_ ## func == (__typeof__(&func)) - 1) {                  \
      _real_ ## func =                                                \
        (__typeof__(&func))dlvsym(RTLD_NEXT, # func, "IBVERBS_1.1");  \
    }                                                                 \
    _real_ ## func;                                                   \
  })
	
#define NEXT_IBV_FNC_1_8(func)                                            \
  ({                                                                  \
    static __typeof__(&func)_real_ ## func = (__typeof__(&func)) - 1; \
    if (_real_ ## func == (__typeof__(&func)) - 1) {                  \
      _real_ ## func =                                                \
        (__typeof__(&func))dlvsym(RTLD_NEXT, # func, "IBVERBS_1.8");  \
    }                                                                 \
    _real_ ## func;                                                   \
  })

// For some reason, ibv_create_comp_channel() and ibv_destroy_comp_channel()
// have version 1.0 only
#define NEXT_IBV_COMP_CHANNEL(func)                                   \
  ({                                                                  \
    static __typeof__(&func)_real_ ## func = (__typeof__(&func)) - 1; \
    if (_real_ ## func == (__typeof__(&func)) - 1) {                  \
      _real_ ## func =                                                \
        (__typeof__(&func))dlvsym(RTLD_NEXT, # func, "IBVERBS_1.0");  \
    }                                                                 \
    _real_ ## func;                                                   \
  })
	
#define NEXT_FNC(func)                                            \
  ({                                                                  \
    static __typeof__(&func)_real_ ## func = (__typeof__(&func)) - 1; \
    if (_real_ ## func == (__typeof__(&func)) - 1) {                  \
      _real_ ## func =                                                \
        (__typeof__(&func))dlsym(RTLD_NEXT, # func);  \
    }                                                                 \
    _real_ ## func;                                                   \
  })
	

#define COORDINATOR_DPORT 2579
#define DYN_COORDINATOR_PORT 2582
#define BUFFER_SIZE 256
#define CMD_STORE 0
#define CMD_QUERY 1
#define CMD_GET_PD_ID 2
#define CMD_GET_RKEY 3
#define CMD_CLEAR_STORE 4
#define CMD_EXIT 5
#define CMD_INFORM_PROC_FAILED 10
#define CMD_CKPT_CREATED 17
#define CMD_MPI_INITIALIZED 18
#define CMD_BARRIER 19
#define CMD_GET_ALL_RKEYS 22
#define CMD_STORE_MULTIPLE 24
#define CMD_STORE_MULTIPLE_NO_PROP 25
#define CMD_GET_ALL_QPS 26
#define CMD_GET_ALL_LIDS 27
#define CMD_INFORM_FINALIZE_REACHED 29

#define PAREP_IB_KILL_COORDINATOR 0
#define PAREP_IB_PRECHECKPOINT 1
#define PAREP_IB_POSTRESTART 2
#define PAREP_IB_CLEAN_COORDINATOR 3
#define PAREP_IB_PRE_SHRINK 4
#define PAREP_IB_POST_SHRINK 5
#define PAREP_IB_CKPT_CREATED 6
#define PAREP_IB_EMPI_INITIALIZED 7
#define PAREP_IB_BARRIER 8
#define PAREP_IB_REACHED_FINALIZE 9
/*
#define VERBS_OPS_NUM (sizeof(struct verbs_context_ops) / sizeof(void *))

#define BITS_PER_LONG	   (8 * sizeof(long))
#define BITS_PER_LONG_LONG (8 * sizeof(long long))
#define BITS_TO_LONGS(nr)  (((nr) + BITS_PER_LONG - 1) / BITS_PER_LONG)

#define container_off(containing_type, member)	\
	offsetof(containing_type, member)

#define check_types_match(expr1, expr2)		\
	((typeof(expr1) *)0 != (typeof(expr2) *)0)

#ifndef container_of
#define container_of(member_ptr, containing_type, member)		\
	 ((containing_type *)						\
	  ((char *)(member_ptr)						\
	   - container_off(containing_type, member))			\
	  + check_types_match(*(member_ptr), ((containing_type *)0)->member))
#endif


	
struct verbs_context_ops {
	int (*advise_mr)(struct ibv_pd *pd,
			 enum ibv_advise_mr_advice advice,
			 uint32_t flags,
			 struct ibv_sge *sg_list,
			 uint32_t num_sges);
	struct ibv_dm *(*alloc_dm)(struct ibv_context *context,
				   struct ibv_alloc_dm_attr *attr);
	struct ibv_mw *(*alloc_mw)(struct ibv_pd *pd, enum ibv_mw_type type);
	struct ibv_mr *(*alloc_null_mr)(struct ibv_pd *pd);
	struct ibv_pd *(*alloc_parent_domain)(
		struct ibv_context *context,
		struct ibv_parent_domain_init_attr *attr);
	struct ibv_pd *(*alloc_pd)(struct ibv_context *context);
	struct ibv_td *(*alloc_td)(struct ibv_context *context,
				   struct ibv_td_init_attr *init_attr);
	void (*async_event)(struct ibv_context *context, struct ibv_async_event *event);
	int (*attach_counters_point_flow)(struct ibv_counters *counters,
					  struct ibv_counter_attach_attr *attr,
					  struct ibv_flow *flow);
	int (*attach_mcast)(struct ibv_qp *qp, const union ibv_gid *gid,
			    uint16_t lid);
	int (*bind_mw)(struct ibv_qp *qp, struct ibv_mw *mw,
		       struct ibv_mw_bind *mw_bind);
	int (*close_xrcd)(struct ibv_xrcd *xrcd);
	void (*cq_event)(struct ibv_cq *cq);
	struct ibv_ah *(*create_ah)(struct ibv_pd *pd,
				    struct ibv_ah_attr *attr);
	struct ibv_counters *(*create_counters)(struct ibv_context *context,
						struct ibv_counters_init_attr *init_attr);
	struct ibv_cq *(*create_cq)(struct ibv_context *context, int cqe,
				    struct ibv_comp_channel *channel,
				    int comp_vector);
	struct ibv_cq_ex *(*create_cq_ex)(
		struct ibv_context *context,
		struct ibv_cq_init_attr_ex *init_attr);
	struct ibv_flow *(*create_flow)(struct ibv_qp *qp,
					struct ibv_flow_attr *flow_attr);
	struct ibv_flow_action *(*create_flow_action_esp)(struct ibv_context *context,
							  struct ibv_flow_action_esp_attr *attr);
	struct ibv_qp *(*create_qp)(struct ibv_pd *pd,
				    struct ibv_qp_init_attr *attr);
	struct ibv_qp *(*create_qp_ex)(
		struct ibv_context *context,
		struct ibv_qp_init_attr_ex *qp_init_attr_ex);
	struct ibv_rwq_ind_table *(*create_rwq_ind_table)(
		struct ibv_context *context,
		struct ibv_rwq_ind_table_init_attr *init_attr);
	struct ibv_srq *(*create_srq)(struct ibv_pd *pd,
				      struct ibv_srq_init_attr *srq_init_attr);
	struct ibv_srq *(*create_srq_ex)(
		struct ibv_context *context,
		struct ibv_srq_init_attr_ex *srq_init_attr_ex);
	struct ibv_wq *(*create_wq)(struct ibv_context *context,
				    struct ibv_wq_init_attr *wq_init_attr);
	int (*dealloc_mw)(struct ibv_mw *mw);
	int (*dealloc_pd)(struct ibv_pd *pd);
	int (*dealloc_td)(struct ibv_td *td);
	int (*dereg_mr)(struct verbs_mr *vmr);
	int (*destroy_ah)(struct ibv_ah *ah);
	int (*destroy_counters)(struct ibv_counters *counters);
	int (*destroy_cq)(struct ibv_cq *cq);
	int (*destroy_flow)(struct ibv_flow *flow);
	int (*destroy_flow_action)(struct ibv_flow_action *action);
	int (*destroy_qp)(struct ibv_qp *qp);
	int (*destroy_rwq_ind_table)(struct ibv_rwq_ind_table *rwq_ind_table);
	int (*destroy_srq)(struct ibv_srq *srq);
	int (*destroy_wq)(struct ibv_wq *wq);
	int (*detach_mcast)(struct ibv_qp *qp, const union ibv_gid *gid,
			    uint16_t lid);
	void (*free_context)(struct ibv_context *context);
	int (*free_dm)(struct ibv_dm *dm);
	int (*get_srq_num)(struct ibv_srq *srq, uint32_t *srq_num);
	struct ibv_dm *(*import_dm)(struct ibv_context *context,
				    uint32_t dm_handle);
	struct ibv_mr *(*import_mr)(struct ibv_pd *pd,
				    uint32_t mr_handle);
	struct ibv_pd *(*import_pd)(struct ibv_context *context,
				    uint32_t pd_handle);
	int (*modify_cq)(struct ibv_cq *cq, struct ibv_modify_cq_attr *attr);
	int (*modify_flow_action_esp)(struct ibv_flow_action *action,
				      struct ibv_flow_action_esp_attr *attr);
	int (*modify_qp)(struct ibv_qp *qp, struct ibv_qp_attr *attr,
			 int attr_mask);
	int (*modify_qp_rate_limit)(struct ibv_qp *qp,
				    struct ibv_qp_rate_limit_attr *attr);
	int (*modify_srq)(struct ibv_srq *srq, struct ibv_srq_attr *srq_attr,
			  int srq_attr_mask);
	int (*modify_wq)(struct ibv_wq *wq, struct ibv_wq_attr *wq_attr);
	struct ibv_qp *(*open_qp)(struct ibv_context *context,
				  struct ibv_qp_open_attr *attr);
	struct ibv_xrcd *(*open_xrcd)(
		struct ibv_context *context,
		struct ibv_xrcd_init_attr *xrcd_init_attr);
	int (*poll_cq)(struct ibv_cq *cq, int num_entries, struct ibv_wc *wc);
	int (*post_recv)(struct ibv_qp *qp, struct ibv_recv_wr *wr,
			 struct ibv_recv_wr **bad_wr);
	int (*post_send)(struct ibv_qp *qp, struct ibv_send_wr *wr,
			 struct ibv_send_wr **bad_wr);
	int (*post_srq_ops)(struct ibv_srq *srq, struct ibv_ops_wr *op,
			    struct ibv_ops_wr **bad_op);
	int (*post_srq_recv)(struct ibv_srq *srq, struct ibv_recv_wr *recv_wr,
			     struct ibv_recv_wr **bad_recv_wr);
	int (*query_device_ex)(struct ibv_context *context,
			       const struct ibv_query_device_ex_input *input,
			       struct ibv_device_attr_ex *attr,
			       size_t attr_size);
	int (*query_ece)(struct ibv_qp *qp, struct ibv_ece *ece);
	int (*query_port)(struct ibv_context *context, uint8_t port_num,
			  struct ibv_port_attr *port_attr);
	int (*query_qp)(struct ibv_qp *qp, struct ibv_qp_attr *attr,
			int attr_mask, struct ibv_qp_init_attr *init_attr);
	int (*query_qp_data_in_order)(struct ibv_qp *qp, enum ibv_wr_opcode op,
				      uint32_t flags);
	int (*query_rt_values)(struct ibv_context *context,
			       struct ibv_values_ex *values);
	int (*query_srq)(struct ibv_srq *srq, struct ibv_srq_attr *srq_attr);
	int (*read_counters)(struct ibv_counters *counters,
			     uint64_t *counters_value,
			     uint32_t ncounters,
			     uint32_t flags);
	struct ibv_mr *(*reg_dm_mr)(struct ibv_pd *pd, struct ibv_dm *dm,
				    uint64_t dm_offset, size_t length,
				    unsigned int access);
	struct ibv_mr *(*reg_dmabuf_mr)(struct ibv_pd *pd, uint64_t offset,
					size_t length, uint64_t iova,
					int fd, int access);
	struct ibv_mr *(*reg_mr)(struct ibv_pd *pd, void *addr, size_t length,
				 uint64_t hca_va, int access);
	int (*req_notify_cq)(struct ibv_cq *cq, int solicited_only);
	int (*rereg_mr)(struct verbs_mr *vmr, int flags, struct ibv_pd *pd,
			void *addr, size_t length, int access);
	int (*resize_cq)(struct ibv_cq *cq, int cqe);
	int (*set_ece)(struct ibv_qp *qp, struct ibv_ece *ece);
	void (*unimport_dm)(struct ibv_dm *dm);
	void (*unimport_mr)(struct ibv_mr *mr);
	void (*unimport_pd)(struct ibv_pd *pd);
};

struct verbs_ex_private {
	unsigned long (unsupported_ioctls)[BITS_TO_LONGS((VERBS_OPS_NUM))]
	uint32_t driver_id;
	bool use_ioctl_write;
	struct verbs_context_ops ops;
	bool imported;
};

static inline struct verbs_ex_private *get_priv(struct ibv_context *ctx)
{
	return container_of(ctx, struct verbs_context, context)->priv;
}

static inline const struct verbs_context_ops *get_ops(struct ibv_context *ctx)
{
	return &get_priv(ctx)->ops;
}*/

void parep_infiniband_cmd(int cmd);

int _fork_init(void);
struct ibv_device **_get_device_list(int *num_devices);
const char *_get_device_name(struct ibv_device *device);
void _free_device_list(struct ibv_device **list);
struct ibv_context *_open_device(struct ibv_device *device);
int _query_device(struct ibv_context *context,
                  struct ibv_device_attr *device_attr);
int _query_port(struct ibv_context *context,
                uint8_t port_num,
                struct ibv_port_attr *port_attr);
int _query_pkey(struct ibv_context *context,
                uint8_t port_num,
                int index,
                uint16_t *pkey);
int _query_gid(struct ibv_context *context,
               uint8_t port_num,
               int index,
               union ibv_gid *gid);
uint64_t _get_device_guid(struct ibv_device *dev);
struct ibv_comp_channel *_create_comp_channel(struct ibv_context *context);
int _destroy_comp_channel(struct ibv_comp_channel *channel);
int _close_device(struct ibv_context *ctx);
int _req_notify_cq(struct ibv_cq *cq, int solicited_only);
int _get_cq_event(struct ibv_comp_channel *channel,
                  struct ibv_cq **cq,
                  void **cq_context);
int _get_async_event(struct ibv_context *context,
                     struct ibv_async_event *event);
void _ack_async_event(struct ibv_async_event *event);
struct ibv_pd *_alloc_pd(struct ibv_context *context);
struct ibv_mr *_reg_mr(struct ibv_pd *pd, void *addr, size_t length, int flag);
struct ibv_mr *_reg_mr_iova(struct ibv_pd *pd, void *addr, size_t length, uint64_t iova, int flag);
struct ibv_mr *_reg_mr_iova2(struct ibv_pd *pd, void *addr, size_t length, uint64_t iova, unsigned int flag);
struct ibv_cq *_create_cq(struct ibv_context *context,
                          int cqe,
                          void *cq_context,
                          struct ibv_comp_channel *channel,
                          int comp_vector);
struct ibv_srq *_create_srq(struct ibv_pd *pd,
                            struct ibv_srq_init_attr *srq_init_attr);
int _modify_srq(struct ibv_srq *srq, struct ibv_srq_attr *attr, int attr_mask);
int _query_srq(struct ibv_srq *srq, struct ibv_srq_attr *srq_attr);

struct ibv_qp *_create_qp(struct ibv_pd *pd,
                          struct ibv_qp_init_attr *qp_init_attr);
int _modify_qp(struct ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask);
int _resize_cq(struct ibv_cq *cq, int cqe);
int _query_qp(struct ibv_qp *qp,
              struct ibv_qp_attr *attr,
              int attr_mask,
              struct ibv_qp_init_attr *init_attr);
int _post_recv(struct ibv_qp *qp,
               struct ibv_recv_wr *wr,
               struct ibv_recv_wr **bad_wr);
int _post_srq_recv(struct ibv_srq *srq,
                   struct ibv_recv_wr *wr,
                   struct ibv_recv_wr **bad_wr);
int _post_send(struct ibv_qp *qp,
               struct ibv_send_wr *wr,
               struct ibv_send_wr **bad_wr);
int _poll_cq(struct ibv_cq *cq, int num_entries, struct ibv_wc *wc);
int _destroy_cq(struct ibv_cq *cq);
int _destroy_srq(struct ibv_srq *srq);
int _destroy_qp(struct ibv_qp *qp);
int _dereg_mr(struct ibv_mr *mr);
int _dealloc_pd(struct ibv_pd *pd);
void _ack_cq_events(struct ibv_cq *cq, unsigned int nevents);
struct ibv_ah *_create_ah(struct ibv_pd *pd, struct ibv_ah_attr *attr);
int _destroy_ah(struct ibv_ah *ah);
struct ibv_mw *_alloc_mw(struct ibv_pd *pd, enum ibv_mw_type type);
int _bind_mw(struct ibv_qp *qp, struct ibv_mw *mw, struct ibv_mw_bind *mw_bind);
int _dealloc_mw(struct ibv_mw *mw);

#endif