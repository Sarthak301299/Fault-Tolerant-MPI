#ifndef _TYPE_DEFINITION_H_
#define _TYPE_DEFINITION_H_
typedef ptrdiff_t OMPI_Aint;
typedef long long OMPI_Offset;
typedef long long OMPI_Count;
typedef struct ompi_communicator_t *OMPI_Comm;
typedef struct ompi_datatype_t *OMPI_Datatype;
typedef struct ompi_errhandler_t *OMPI_Errhandler;
typedef struct ompi_file_t *OMPI_File;
typedef struct ompi_group_t *OMPI_Group;
typedef struct ompi_info_t *OMPI_Info;
typedef struct ompi_op_t *OMPI_Op;
typedef struct ompi_request_t *OMPI_Request;
typedef struct ompi_message_t *OMPI_Message;
typedef struct ompi_status_public_t OMPI_Status;
typedef struct ompi_f08_status_public_t OMPI_F08_status;
typedef struct ompi_win_t *OMPI_Win;
typedef struct mca_base_var_enum_t *OMPI_T_enum;
typedef struct ompi_mpit_cvar_handle_t *OMPI_T_cvar_handle;
typedef struct mca_base_pvar_handle_t *OMPI_T_pvar_handle;
typedef struct mca_base_pvar_session_t *OMPI_T_pvar_session;
typedef struct ompi_instance_t *OMPI_Session;
typedef struct ompi_status_public_t ompi_status_public_t;
typedef struct ompi_f08_status_public_t ompi_f08_status_public_t;
typedef int (OMPI_Datarep_extent_function)(OMPI_Datatype, OMPI_Aint *, void *);
typedef int (OMPI_Datarep_conversion_function)(void *, OMPI_Datatype,                                               int, void *, OMPI_Offset, void *);
typedef void (OMPI_Comm_errhandler_function)(OMPI_Comm *, int *, ...);
typedef void (OMPI_Session_errhandler_function) (OMPI_Session *, int *, ...);
typedef void (ompi_file_errhandler_function)(OMPI_File *, int *, ...);
typedef void (OMPI_Win_errhandler_function)(OMPI_Win *, int *, ...);
typedef void (OMPI_User_function)(void *, void *, int *, OMPI_Datatype *);
typedef int (OMPI_Comm_copy_attr_function)(OMPI_Comm, int, void *,                                             void *, void *, int *);
typedef int (OMPI_Comm_delete_attr_function)(OMPI_Comm, int, void *, void *);
typedef int (OMPI_Type_copy_attr_function)(OMPI_Datatype, int, void *,                                             void *, void *, int *);
typedef int (OMPI_Type_delete_attr_function)(OMPI_Datatype, int,                                               void *, void *);
typedef int (OMPI_Win_copy_attr_function)(OMPI_Win, int, void *,                                            void *, void *, int *);
typedef int (OMPI_Win_delete_attr_function)(OMPI_Win, int, void *, void *);
typedef int (OMPI_Session_delete_attr_function)(OMPI_Session, int, void *, void *);
typedef int (OMPI_Grequest_query_function)(void *, OMPI_Status *);
typedef int (OMPI_Grequest_free_function)(void *);
typedef int (OMPI_Grequest_cancel_function)(void *, int);
typedef OMPI_Comm_errhandler_function OMPI_Comm_errhandler_fn         __attribute__((__deprecated__("OMPI_Comm_errhandler_fn was deprecated in OMPI-2.2; use OMPI_Comm_errhandler_function instead")));
typedef ompi_file_errhandler_function OMPI_File_errhandler_fn         __attribute__((__deprecated__("OMPI_File_errhandler_fn was deprecated in OMPI-2.2; use OMPI_File_errhandler_function instead")));
typedef ompi_file_errhandler_function OMPI_File_errhandler_function;
typedef OMPI_Win_errhandler_function OMPI_Win_errhandler_fn         __attribute__((__deprecated__("OMPI_Win_errhandler_fn was deprecated in OMPI-2.2; use OMPI_Win_errhandler_function instead")));
enum {OMPI_TAG_UB,    OMPI_HOST,    OMPI_IO,    OMPI_WTIME_IS_GLOBAL,    OMPI_APPNUM,    OMPI_LASTUSEDCODE,    OMPI_UNIVERSE_SIZE,    OMPI_WIN_BASE,    OMPI_WIN_SIZE,    OMPI_WIN_DISP_UNIT,    OMPI_WIN_CREATE_FLAVOR,    OMPI_WIN_MODEL,    OMPI_FT,    OMPI_ATTR_PREDEFINED_KEY_MAX,};
enum {OMPI_IDENT,  OMPI_CONGRUENT,  OMPI_SIMILAR,  OMPI_UNEQUAL};
enum {OMPI_THREAD_SINGLE,  OMPI_THREAD_FUNNELED,  OMPI_THREAD_SERIALIZED,  OMPI_THREAD_MULTIPLE};
enum {OMPI_COMBINER_NAMED,  OMPI_COMBINER_DUP,  OMPI_COMBINER_CONTIGUOUS,  OMPI_COMBINER_VECTOR,  OMPI_COMBINER_HVECTOR_INTEGER,  OMPI_COMBINER_HVECTOR,  OMPI_COMBINER_INDEXED,  OMPI_COMBINER_HINDEXED_INTEGER,  OMPI_COMBINER_HINDEXED,  OMPI_COMBINER_INDEXED_BLOCK,  OMPI_COMBINER_STRUCT_INTEGER,  OMPI_COMBINER_STRUCT,  OMPI_COMBINER_SUBARRAY,  OMPI_COMBINER_DARRAY,  OMPI_COMBINER_F90_REAL,  OMPI_COMBINER_F90_COMPLEX,  OMPI_COMBINER_F90_INTEGER,  OMPI_COMBINER_RESIZED,  OMPI_COMBINER_HINDEXED_BLOCK};
enum {OMPI_COMM_TYPE_SHARED,  OOMPI_COMM_TYPE_HWTHREAD,  OOMPI_COMM_TYPE_CORE,  OOMPI_COMM_TYPE_L1CACHE,  OOMPI_COMM_TYPE_L2CACHE,  OOMPI_COMM_TYPE_L3CACHE,  OOMPI_COMM_TYPE_SOCKET,  OOMPI_COMM_TYPE_NUMA,  OOMPI_COMM_TYPE_BOARD,  OOMPI_COMM_TYPE_HOST,  OOMPI_COMM_TYPE_CU,  OOMPI_COMM_TYPE_CLUSTER};
enum {OMPI_T_VERBOSITY_USER_BASIC,  OMPI_T_VERBOSITY_USER_DETAIL,  OMPI_T_VERBOSITY_USER_ALL,  OMPI_T_VERBOSITY_TUNER_BASIC,  OMPI_T_VERBOSITY_TUNER_DETAIL,  OMPI_T_VERBOSITY_TUNER_ALL,  OMPI_T_VERBOSITY_OMPIDEV_BASIC,  OMPI_T_VERBOSITY_OMPIDEV_DETAIL,  OMPI_T_VERBOSITY_OMPIDEV_ALL};
enum {OMPI_T_SCOPE_CONSTANT,  OMPI_T_SCOPE_READONLY,  OMPI_T_SCOPE_LOCAL,  OMPI_T_SCOPE_GROUP,  OMPI_T_SCOPE_GROUP_EQ,  OMPI_T_SCOPE_ALL,  OMPI_T_SCOPE_ALL_EQ};
enum {OMPI_T_BIND_NO_OBJECT,  OMPI_T_BIND_OMPI_COMM,  OMPI_T_BIND_OMPI_DATATYPE,  OMPI_T_BIND_OMPI_ERRHANDLER,  OMPI_T_BIND_OMPI_FILE,  OMPI_T_BIND_OMPI_GROUP,  OMPI_T_BIND_OMPI_OP,  OMPI_T_BIND_OMPI_REQUEST,  OMPI_T_BIND_OMPI_WIN,  OMPI_T_BIND_OMPI_MESSAGE,  OMPI_T_BIND_OMPI_INFO};
enum {OMPI_T_PVAR_CLASS_STATE,  OMPI_T_PVAR_CLASS_LEVEL,  OMPI_T_PVAR_CLASS_SIZE,  OMPI_T_PVAR_CLASS_PERCENTAGE,  OMPI_T_PVAR_CLASS_HIGHWATERMARK,  OMPI_T_PVAR_CLASS_LOWWATERMARK,  OMPI_T_PVAR_CLASS_COUNTER,  OMPI_T_PVAR_CLASS_AGGREGATE,  OMPI_T_PVAR_CLASS_TIMER,  OMPI_T_PVAR_CLASS_GENERIC};
#define ompi_mpi_comm_world *(long int *)dlsym(openLib,"ompi_mpi_comm_world")
#define ompi_mpi_comm_self *(long int *)dlsym(openLib,"ompi_mpi_comm_self")
#define ompi_mpi_comm_null *(long int *)dlsym(openLib,"ompi_mpi_comm_null")
#define ompi_mpi_group_empty *(long int *)dlsym(openLib,"ompi_mpi_group_empty")
#define ompi_mpi_group_null *(long int *)dlsym(openLib,"ompi_mpi_group_null")
#define ompi_mpi_instance_null *(long int *)dlsym(openLib,"ompi_mpi_instance_null")
#define ompi_request_null *(long int *)dlsym(openLib,"ompi_request_null")
#define ompi_message_null *(long int *)dlsym(openLib,"ompi_message_null")
#define ompi_message_no_proc *(long int *)dlsym(openLib,"ompi_message_no_proc")
#define ompi_mpi_op_null *(long int *)dlsym(openLib,"ompi_mpi_op_null")
#define ompi_mpi_op_min *(long int *)dlsym(openLib,"ompi_mpi_op_min")
#define ompi_mpi_op_max *(long int *)dlsym(openLib,"ompi_mpi_op_max")
#define ompi_mpi_op_sum *(long int *)dlsym(openLib,"ompi_mpi_op_sum")
#define ompi_mpi_op_prod *(long int *)dlsym(openLib,"ompi_mpi_op_prod")
#define ompi_mpi_op_land *(long int *)dlsym(openLib,"ompi_mpi_op_land")
#define ompi_mpi_op_band *(long int *)dlsym(openLib,"ompi_mpi_op_band")
#define ompi_mpi_op_lor *(long int *)dlsym(openLib,"ompi_mpi_op_lor")
#define ompi_mpi_op_bor *(long int *)dlsym(openLib,"ompi_mpi_op_bor")
#define ompi_mpi_op_lxor *(long int *)dlsym(openLib,"ompi_mpi_op_lxor")
#define ompi_mpi_op_bxor *(long int *)dlsym(openLib,"ompi_mpi_op_bxor")
#define ompi_mpi_op_maxloc *(long int *)dlsym(openLib,"ompi_mpi_op_maxloc")
#define ompi_mpi_op_minloc *(long int *)dlsym(openLib,"ompi_mpi_op_minloc")
#define ompi_mpi_op_replace *(long int *)dlsym(openLib,"ompi_mpi_op_replace")
#define ompi_mpi_op_no_op *(long int *)dlsym(openLib,"ompi_mpi_op_no_op")
#define ompi_mpi_datatype_null *(long int *)dlsym(openLib,"ompi_mpi_datatype_null")
#define ompi_mpi_char *(long int *)dlsym(openLib,"ompi_mpi_char")
#define ompi_mpi_signed_char *(long int *)dlsym(openLib,"ompi_mpi_signed_char")
#define ompi_mpi_unsigned_char *(long int *)dlsym(openLib,"ompi_mpi_unsigned_char")
#define ompi_mpi_byte *(long int *)dlsym(openLib,"ompi_mpi_byte")
#define ompi_mpi_short *(long int *)dlsym(openLib,"ompi_mpi_short")
#define ompi_mpi_unsigned_short *(long int *)dlsym(openLib,"ompi_mpi_unsigned_short")
#define ompi_mpi_int *(long int *)dlsym(openLib,"ompi_mpi_int")
#define ompi_mpi_unsigned *(long int *)dlsym(openLib,"ompi_mpi_unsigned")
#define ompi_mpi_long *(long int *)dlsym(openLib,"ompi_mpi_long")
#define ompi_mpi_unsigned_long *(long int *)dlsym(openLib,"ompi_mpi_unsigned_long")
#define ompi_mpi_long_long_int *(long int *)dlsym(openLib,"ompi_mpi_long_long_int")
#define ompi_mpi_unsigned_long_long *(long int *)dlsym(openLib,"ompi_mpi_unsigned_long_long")
#define ompi_mpi_float *(long int *)dlsym(openLib,"ompi_mpi_float")
#define ompi_mpi_double *(long int *)dlsym(openLib,"ompi_mpi_double")
#define ompi_mpi_long_double *(long int *)dlsym(openLib,"ompi_mpi_long_double")
#define ompi_mpi_wchar *(long int *)dlsym(openLib,"ompi_mpi_wchar")
#define ompi_mpi_packed *(long int *)dlsym(openLib,"ompi_mpi_packed")
#define ompi_mpi_cxx_bool *(long int *)dlsym(openLib,"ompi_mpi_cxx_bool")
#define ompi_mpi_cxx_cplex *(long int *)dlsym(openLib,"ompi_mpi_cxx_cplex")
#define ompi_mpi_cxx_dblcplex *(long int *)dlsym(openLib,"ompi_mpi_cxx_dblcplex")
#define ompi_mpi_cxx_ldblcplex *(long int *)dlsym(openLib,"ompi_mpi_cxx_ldblcplex")
#define ompi_mpi_logical *(long int *)dlsym(openLib,"ompi_mpi_logical")
#define ompi_mpi_character *(long int *)dlsym(openLib,"ompi_mpi_character")
#define ompi_mpi_integer *(long int *)dlsym(openLib,"ompi_mpi_integer")
#define ompi_mpi_real *(long int *)dlsym(openLib,"ompi_mpi_real")
#define ompi_mpi_dblprec *(long int *)dlsym(openLib,"ompi_mpi_dblprec")
#define ompi_mpi_cplex *(long int *)dlsym(openLib,"ompi_mpi_cplex")
#define ompi_mpi_dblcplex *(long int *)dlsym(openLib,"ompi_mpi_dblcplex")
#define ompi_mpi_ldblcplex *(long int *)dlsym(openLib,"ompi_mpi_ldblcplex")
#define ompi_mpi_2int *(long int *)dlsym(openLib,"ompi_mpi_2int")
#define ompi_mpi_2integer *(long int *)dlsym(openLib,"ompi_mpi_2integer")
#define ompi_mpi_2real *(long int *)dlsym(openLib,"ompi_mpi_2real")
#define ompi_mpi_2dblprec *(long int *)dlsym(openLib,"ompi_mpi_2dblprec")
#define ompi_mpi_2cplex *(long int *)dlsym(openLib,"ompi_mpi_2cplex")
#define ompi_mpi_2dblcplex *(long int *)dlsym(openLib,"ompi_mpi_2dblcplex")
#define ompi_mpi_float_int *(long int *)dlsym(openLib,"ompi_mpi_float_int")
#define ompi_mpi_double_int *(long int *)dlsym(openLib,"ompi_mpi_double_int")
#define ompi_mpi_longdbl_int *(long int *)dlsym(openLib,"ompi_mpi_longdbl_int")
#define ompi_mpi_short_int *(long int *)dlsym(openLib,"ompi_mpi_short_int")
#define ompi_mpi_long_int *(long int *)dlsym(openLib,"ompi_mpi_long_int")
#define ompi_mpi_logical1 *(long int *)dlsym(openLib,"ompi_mpi_logical1")
#define ompi_mpi_logical2 *(long int *)dlsym(openLib,"ompi_mpi_logical2")
#define ompi_mpi_logical4 *(long int *)dlsym(openLib,"ompi_mpi_logical4")
#define ompi_mpi_logical8 *(long int *)dlsym(openLib,"ompi_mpi_logical8")
#define ompi_mpi_integer1 *(long int *)dlsym(openLib,"ompi_mpi_integer1")
#define ompi_mpi_integer2 *(long int *)dlsym(openLib,"ompi_mpi_integer2")
#define ompi_mpi_integer4 *(long int *)dlsym(openLib,"ompi_mpi_integer4")
#define ompi_mpi_integer8 *(long int *)dlsym(openLib,"ompi_mpi_integer8")
#define ompi_mpi_integer16 *(long int *)dlsym(openLib,"ompi_mpi_integer16")
#define ompi_mpi_real2 *(long int *)dlsym(openLib,"ompi_mpi_real2")
#define ompi_mpi_real4 *(long int *)dlsym(openLib,"ompi_mpi_real4")
#define ompi_mpi_real8 *(long int *)dlsym(openLib,"ompi_mpi_real8")
#define ompi_mpi_real16 *(long int *)dlsym(openLib,"ompi_mpi_real16")
#define ompi_mpi_complex4 *(long int *)dlsym(openLib,"ompi_mpi_complex4")
#define ompi_mpi_complex8 *(long int *)dlsym(openLib,"ompi_mpi_complex8")
#define ompi_mpi_complex16 *(long int *)dlsym(openLib,"ompi_mpi_complex16")
#define ompi_mpi_complex32 *(long int *)dlsym(openLib,"ompi_mpi_complex32")
#define ompi_mpi_int8_t *(long int *)dlsym(openLib,"ompi_mpi_int8_t")
#define ompi_mpi_uint8_t *(long int *)dlsym(openLib,"ompi_mpi_uint8_t")
#define ompi_mpi_int16_t *(long int *)dlsym(openLib,"ompi_mpi_int16_t")
#define ompi_mpi_uint16_t *(long int *)dlsym(openLib,"ompi_mpi_uint16_t")
#define ompi_mpi_int32_t *(long int *)dlsym(openLib,"ompi_mpi_int32_t")
#define ompi_mpi_uint32_t *(long int *)dlsym(openLib,"ompi_mpi_uint32_t")
#define ompi_mpi_int64_t *(long int *)dlsym(openLib,"ompi_mpi_int64_t")
#define ompi_mpi_uint64_t *(long int *)dlsym(openLib,"ompi_mpi_uint64_t")
#define ompi_mpi_aint *(long int *)dlsym(openLib,"ompi_mpi_aint")
#define ompi_mpi_offset *(long int *)dlsym(openLib,"ompi_mpi_offset")
#define ompi_mpi_count *(long int *)dlsym(openLib,"ompi_mpi_count")
#define ompi_mpi_c_bool *(long int *)dlsym(openLib,"ompi_mpi_c_bool")
#define ompi_mpi_c_float_complex *(long int *)dlsym(openLib,"ompi_mpi_c_float_complex")
#define ompi_mpi_c_double_complex *(long int *)dlsym(openLib,"ompi_mpi_c_double_complex")
#define ompi_mpi_c_long_double_complex *(long int *)dlsym(openLib,"ompi_mpi_c_long_double_complex")
#define ompi_mpi_errhandler_null *(long int *)dlsym(openLib,"ompi_mpi_errhandler_null")
#define ompi_mpi_errors_are_fatal *(long int *)dlsym(openLib,"ompi_mpi_errors_are_fatal")
#define ompi_mpi_errors_abort *(long int *)dlsym(openLib,"ompi_mpi_errors_abort")
#define ompi_mpi_errors_return *(long int *)dlsym(openLib,"ompi_mpi_errors_return")
#define ompi_mpi_win_null *(long int *)dlsym(openLib,"ompi_mpi_win_null")
#define ompi_mpi_file_null *(long int *)dlsym(openLib,"ompi_mpi_file_null")
#define ompi_mpi_info_null *(long int *)dlsym(openLib,"ompi_mpi_info_null")
#define ompi_mpi_info_env *(long int *)dlsym(openLib,"ompi_mpi_info_env")
#define ompi_mpi_lb *(long int *)dlsym(openLib,"ompi_mpi_lb")
#define ompi_mpi_ub *(long int *)dlsym(openLib,"ompi_mpi_ub")
typedef int (OMPI_Copy_function)(OMPI_Comm, int, void *,                                 void *, void *, int *);
typedef int (OMPI_Delete_function)(OMPI_Comm, int, void *, void *);
typedef void (OMPI_Handler_function)(OMPI_Comm *, int *, ...);
typedef enum ompi_affinity_fmt {OOMPI_AFFINITY_RSRC_STRING_FMT,    OOMPI_AFFINITY_LAYOUT_FMT} ompi_affinity_fmt_t;
#endif
