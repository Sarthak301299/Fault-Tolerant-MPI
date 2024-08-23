#ifndef _MPIPROFILERDEFS_H_
#define _MPIPROFILERDEFS_H_

#include "typedefs.h"
#include "declarations.h"
#include "commbuf_cache.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void *extLib;

struct mpi_ft_datatype {
	EMPI_Datatype edatatype;
	int size;
};
//struct mpi_ft_datatype;
typedef struct mpi_ft_datatype *MPI_Datatype;

struct mpi_ft_comm {
	EMPI_Comm eworldComm;
	EMPI_Comm EMPI_COMM_CMP;
	EMPI_Comm EMPI_COMM_REP;
	EMPI_Comm EMPI_CMP_REP_INTERCOMM;
	EMPI_Comm EMPI_CMP_NO_REP;
	EMPI_Comm EMPI_CMP_NO_REP_INTERCOMM;
};
//struct mpi_ft_comm;
typedef struct mpi_ft_comm *MPI_Comm;

struct mpi_ft_op {
	EMPI_Op eop;
};
//struct mpi_ft_op;
typedef struct mpi_ft_op *MPI_Op;

typedef int MPI_Group;
typedef int MPI_Win;
typedef struct ADIOI_FileD *MPI_File;

typedef enum MPIR_Win_flavor {MPI_WIN_FLAVOR_CREATE = 1,    MPI_WIN_FLAVOR_ALLOCATE = 2,    MPI_WIN_FLAVOR_DYNAMIC = 3,    MPI_WIN_FLAVOR_SHARED = 4} MPIR_Win_flavor_t;
typedef enum MPIR_Win_model {MPI_WIN_SEPARATE = 1,    MPI_WIN_UNIFIED = 2} MPIR_Win_model_t;
typedef enum MPIR_Topo_type {MPI_GRAPH=1, MPI_CART=2, MPI_DIST_GRAPH=3} MPIR_Topo_type;
typedef void (MPI_Handler_function) ( MPI_Comm *, int *, ... );
typedef int (MPI_Comm_copy_attr_function)(MPI_Comm, int, void *, void *,        void *, int *);
typedef int (MPI_Comm_delete_attr_function)(MPI_Comm, int, void *, void *);
typedef int (MPI_Type_copy_attr_function)(MPI_Datatype, int, void *, void *,        void *, int *);
typedef int (MPI_Type_delete_attr_function)(MPI_Datatype, int, void *, void *);
typedef int (MPI_Win_copy_attr_function)(MPI_Win, int, void *, void *, void *,       int *);
typedef int (MPI_Win_delete_attr_function)(MPI_Win, int, void *, void *);
typedef void (MPI_Comm_errhandler_function)(MPI_Comm *, int *, ...);
typedef void (MPI_File_errhandler_function)(MPI_File *, int *, ...);
typedef void (MPI_Win_errhandler_function)(MPI_Win *, int *, ...);
typedef MPI_Comm_errhandler_function MPI_Comm_errhandler_fn;
typedef MPI_File_errhandler_function MPI_File_errhandler_fn;
typedef MPI_Win_errhandler_function MPI_Win_errhandler_fn;
typedef int MPI_Errhandler;

struct mpi_ft_status {
	EMPI_Status status;
	int count;
	//int count_lo;
	//int count_hi_and_cancelled;
	int MPI_SOURCE;
	int MPI_TAG;
	int MPI_ERROR;
};
//struct mpi_ft_status;
typedef struct mpi_ft_status MPI_Status;

struct mpi_ft_request;

struct parep_mpi_request_list_node {
	struct mpi_ft_request *req;
	struct parep_mpi_request_list_node *next;
	struct parep_mpi_request_list_node *prev;
};
typedef struct parep_mpi_request_list_node reqNode;

struct mpi_ft_request {
	EMPI_Request *reqcmp;
	EMPI_Request *reqrep;
	EMPI_Request **reqcolls;
	int num_reqcolls;
	bool complete;
	MPI_Comm comm;
	MPI_Status status;
	int type;
	void *bufloc;
	void *storeloc;
	reqNode *rnode;
	commbuf_node_t *cbuf;
};
//struct mpi_ft_request;
typedef struct mpi_ft_request *MPI_Request;

typedef int MPI_Message;
typedef void (MPI_User_function) ( void *, void *, int *, MPI_Datatype * );
typedef int (MPI_Copy_function) ( MPI_Comm, int, void *, void *, void *, int * );
typedef int (MPI_Delete_function) ( MPI_Comm, int, void *, void * );
enum MPIR_Combiner_enum {MPI_COMBINER_NAMED = 1,    MPI_COMBINER_DUP = 2,    MPI_COMBINER_CONTIGUOUS = 3,    MPI_COMBINER_VECTOR = 4,    MPI_COMBINER_HVECTOR_INTEGER = 5,    MPI_COMBINER_HVECTOR = 6,    MPI_COMBINER_INDEXED = 7,    MPI_COMBINER_HINDEXED_INTEGER = 8,    MPI_COMBINER_HINDEXED = 9,    MPI_COMBINER_INDEXED_BLOCK = 10,    MPI_COMBINER_STRUCT_INTEGER = 11,    MPI_COMBINER_STRUCT = 12,    MPI_COMBINER_SUBARRAY = 13,    MPI_COMBINER_DARRAY = 14,    MPI_COMBINER_F90_REAL = 15,    MPI_COMBINER_F90_COMPLEX = 16,    MPI_COMBINER_F90_INTEGER = 17,    MPI_COMBINER_RESIZED = 18,    MPI_COMBINER_HINDEXED_BLOCK = 19};
typedef int MPI_Info;
typedef long MPI_Aint;
typedef int MPI_Fint;
typedef long long MPI_Count;
typedef long long MPI_Offset;
typedef struct MPIR_T_enum_s * MPI_T_enum;
typedef struct MPIR_T_cvar_handle_s * MPI_T_cvar_handle;
typedef struct MPIR_T_pvar_handle_s * MPI_T_pvar_handle;
typedef struct MPIR_T_pvar_session_s * MPI_T_pvar_session;
typedef enum MPIR_T_verbosity_t {MPIX_T_VERBOSITY_INVALID = 0,    MPI_T_VERBOSITY_USER_BASIC = 221,    MPI_T_VERBOSITY_USER_DETAIL,    MPI_T_VERBOSITY_USER_ALL,    MPI_T_VERBOSITY_TUNER_BASIC,    MPI_T_VERBOSITY_TUNER_DETAIL,    MPI_T_VERBOSITY_TUNER_ALL,    MPI_T_VERBOSITY_MPIDEV_BASIC,    MPI_T_VERBOSITY_MPIDEV_DETAIL,    MPI_T_VERBOSITY_MPIDEV_ALL} MPIR_T_verbosity_t;
typedef enum MPIR_T_bind_t {MPIX_T_BIND_INVALID = 0,    MPI_T_BIND_NO_OBJECT = 9700,    MPI_T_BIND_MPI_COMM,    MPI_T_BIND_MPI_DATATYPE,    MPI_T_BIND_MPI_ERRHANDLER,    MPI_T_BIND_MPI_FILE,    MPI_T_BIND_MPI_GROUP,    MPI_T_BIND_MPI_OP,    MPI_T_BIND_MPI_REQUEST,    MPI_T_BIND_MPI_WIN,    MPI_T_BIND_MPI_MESSAGE,    MPI_T_BIND_MPI_INFO} MPIR_T_bind_t;
typedef enum MPIR_T_scope_t {MPIX_T_SCOPE_INVALID = 0,    MPI_T_SCOPE_CONSTANT = 60438,    MPI_T_SCOPE_READONLY,    MPI_T_SCOPE_LOCAL,    MPI_T_SCOPE_GROUP,    MPI_T_SCOPE_GROUP_EQ,    MPI_T_SCOPE_ALL,    MPI_T_SCOPE_ALL_EQ} MPIR_T_scope_t;
typedef enum MPIR_T_pvar_class_t {MPIX_T_PVAR_CLASS_INVALID = 0,    MPIR_T_PVAR_CLASS_FIRST = 240,    MPI_T_PVAR_CLASS_STATE = MPIR_T_PVAR_CLASS_FIRST,    MPI_T_PVAR_CLASS_LEVEL,    MPI_T_PVAR_CLASS_SIZE,    MPI_T_PVAR_CLASS_PERCENTAGE,    MPI_T_PVAR_CLASS_HIGHWATERMARK,    MPI_T_PVAR_CLASS_LOWWATERMARK,    MPI_T_PVAR_CLASS_COUNTER,    MPI_T_PVAR_CLASS_AGGREGATE,    MPI_T_PVAR_CLASS_TIMER,    MPI_T_PVAR_CLASS_GENERIC,    MPIR_T_PVAR_CLASS_LAST,    MPIR_T_PVAR_CLASS_NUMBER = MPIR_T_PVAR_CLASS_LAST - MPIR_T_PVAR_CLASS_FIRST} MPIR_T_pvar_class_t;
typedef struct {MPI_Fint count_lo;    MPI_Fint count_hi_and_cancelled;    MPI_Fint MPI_SOURCE;    MPI_Fint MPI_TAG;    MPI_Fint MPI_ERROR;} MPI_F08_Status;
typedef int (MPI_Grequest_cancel_function)(void *, int);
typedef int (MPI_Grequest_free_function)(void *);
typedef int (MPI_Grequest_query_function)(void *, MPI_Status *);
typedef int (MPIX_Grequest_poll_function)(void *, MPI_Status *);
typedef int (MPIX_Grequest_wait_function)(int, void **, double, MPI_Status *);
typedef int (MPI_Datarep_conversion_function)(void *, MPI_Datatype, int,              void *, MPI_Offset, void *);
typedef int (MPI_Datarep_extent_function)(MPI_Datatype datatype, MPI_Aint *,                       void *);
typedef int MPIX_Grequest_class;
typedef struct mpixi_mutex_s * MPIX_Mutex;
#define MPI_INCLUDED 
#undef MPICH_DEFINE_ATTR_TYPE_TYPES
#define MPICH_ATTR_POINTER_WITH_TYPE_TAG(buffer_idx,type_idx) 
#define MPICH_ATTR_TYPE_TAG(type) 
#define MPICH_ATTR_TYPE_TAG_LAYOUT_COMPATIBLE(type) 
#define MPICH_ATTR_TYPE_TAG_MUST_BE_NULL() 
#define MPICH_ATTR_TYPE_TAG_STDINT(type) MPICH_ATTR_TYPE_TAG(type)
#define MPICH_ATTR_TYPE_TAG_C99(type) MPICH_ATTR_TYPE_TAG(type)
#define MPICH_ATTR_TYPE_TAG_CXX(type)

extern struct mpi_ft_comm mpi_ft_comm_null;
extern struct mpi_ft_op mpi_ft_op_null;
extern struct mpi_ft_datatype mpi_ft_datatype_null;
extern struct mpi_ft_request mpi_ft_request_null;
//extern struct mpi_ft_status mpi_ft_status_ignore;
extern struct mpi_ft_comm mpi_ft_comm_world;
extern struct mpi_ft_comm mpi_ft_comm_self;
extern struct mpi_ft_datatype mpi_ft_datatype_char;
extern struct mpi_ft_datatype mpi_ft_datatype_signed_char;
extern struct mpi_ft_datatype mpi_ft_datatype_unsigned_char;
extern struct mpi_ft_datatype mpi_ft_datatype_byte;
extern struct mpi_ft_datatype mpi_ft_datatype_short;
extern struct mpi_ft_datatype mpi_ft_datatype_unsigned_short;
extern struct mpi_ft_datatype mpi_ft_datatype_int;
extern struct mpi_ft_datatype mpi_ft_datatype_unsigned;
extern struct mpi_ft_datatype mpi_ft_datatype_long;
extern struct mpi_ft_datatype mpi_ft_datatype_unsigned_long;
extern struct mpi_ft_datatype mpi_ft_datatype_float;
extern struct mpi_ft_datatype mpi_ft_datatype_double;
extern struct mpi_ft_datatype mpi_ft_datatype_long_double;
extern struct mpi_ft_datatype mpi_ft_datatype_long_long_int;
extern struct mpi_ft_datatype mpi_ft_datatype_unsigned_long_long;

extern struct mpi_ft_datatype mpi_ft_datatype_packed;
//extern struct mpi_ft_datatype mpi_ft_datatype_lb;
//extern struct mpi_ft_datatype mpi_ft_datatype_ub;
extern struct mpi_ft_datatype mpi_ft_datatype_float_int;
extern struct mpi_ft_datatype mpi_ft_datatype_double_int;
extern struct mpi_ft_datatype mpi_ft_datatype_long_int;
extern struct mpi_ft_datatype mpi_ft_datatype_short_int;
extern struct mpi_ft_datatype mpi_ft_datatype_2int;
extern struct mpi_ft_datatype mpi_ft_datatype_long_double_int;
extern struct mpi_ft_datatype mpi_ft_datatype_complex;
extern struct mpi_ft_datatype mpi_ft_datatype_double_complex;
extern struct mpi_ft_datatype mpi_ft_datatype_logical;
extern struct mpi_ft_datatype mpi_ft_datatype_real;
extern struct mpi_ft_datatype mpi_ft_datatype_double_precision;
extern struct mpi_ft_datatype mpi_ft_datatype_integer;
extern struct mpi_ft_datatype mpi_ft_datatype_2integer;
extern struct mpi_ft_datatype mpi_ft_datatype_2real;
extern struct mpi_ft_datatype mpi_ft_datatype_2double_precision;
extern struct mpi_ft_datatype mpi_ft_datatype_character;
//extern struct mpi_ft_datatype mpi_ft_datatype_real4;
//extern struct mpi_ft_datatype mpi_ft_datatype_real8;
//extern struct mpi_ft_datatype mpi_ft_datatype_real16;
//extern struct mpi_ft_datatype mpi_ft_datatype_complex8;
//extern struct mpi_ft_datatype mpi_ft_datatype_complex16;
//extern struct mpi_ft_datatype mpi_ft_datatype_complex32;
//extern struct mpi_ft_datatype mpi_ft_datatype_integer1;
//extern struct mpi_ft_datatype mpi_ft_datatype_integer2;
//extern struct mpi_ft_datatype mpi_ft_datatype_integer4;
//extern struct mpi_ft_datatype mpi_ft_datatype_integer8;
//extern struct mpi_ft_datatype mpi_ft_datatype_integer16;
extern struct mpi_ft_datatype mpi_ft_datatype_int8_t;
extern struct mpi_ft_datatype mpi_ft_datatype_int16_t;
extern struct mpi_ft_datatype mpi_ft_datatype_int32_t;
extern struct mpi_ft_datatype mpi_ft_datatype_int64_t;
extern struct mpi_ft_datatype mpi_ft_datatype_uint8_t;
extern struct mpi_ft_datatype mpi_ft_datatype_uint16_t;
extern struct mpi_ft_datatype mpi_ft_datatype_uint32_t;
extern struct mpi_ft_datatype mpi_ft_datatype_uint64_t;
extern struct mpi_ft_datatype mpi_ft_datatype_c_bool;
extern struct mpi_ft_datatype mpi_ft_datatype_c_float_complex;
extern struct mpi_ft_datatype mpi_ft_datatype_c_double_complex;
extern struct mpi_ft_datatype mpi_ft_datatype_c_long_double_complex;
extern struct mpi_ft_datatype mpi_ft_datatype_aint;
extern struct mpi_ft_datatype mpi_ft_datatype_offset;
extern struct mpi_ft_datatype mpi_ft_datatype_count;
extern struct mpi_ft_datatype mpi_ft_datatype_cxx_bool;
extern struct mpi_ft_datatype mpi_ft_datatype_cxx_float_complex;
extern struct mpi_ft_datatype mpi_ft_datatype_cxx_double_complex;
extern struct mpi_ft_datatype mpi_ft_datatype_cxx_long_double_complex;

extern struct mpi_ft_op mpi_ft_op_max;
extern struct mpi_ft_op mpi_ft_op_min;
extern struct mpi_ft_op mpi_ft_op_sum;
extern struct mpi_ft_op mpi_ft_op_prod;
extern struct mpi_ft_op mpi_ft_op_land;
extern struct mpi_ft_op mpi_ft_op_band;
extern struct mpi_ft_op mpi_ft_op_lor;
extern struct mpi_ft_op mpi_ft_op_bor;
extern struct mpi_ft_op mpi_ft_op_lxor;
extern struct mpi_ft_op mpi_ft_op_bxor;
extern struct mpi_ft_op mpi_ft_op_minloc;
extern struct mpi_ft_op mpi_ft_op_maxloc;
extern struct mpi_ft_op mpi_ft_op_replace;
extern struct mpi_ft_op mpi_ft_op_no_op;

#define MPI_FT_PREDEFINED_GLOBAL(type,global) ((type) ((void *) &(global)))

#define MPI_COMM_NULL MPI_FT_PREDEFINED_GLOBAL(MPI_Comm,mpi_ft_comm_null)
#define MPI_OP_NULL MPI_FT_PREDEFINED_GLOBAL(MPI_Op, mpi_ft_op_null)
#define MPI_GROUP_NULL ((MPI_Group)0x08000000)
#define MPI_DATATYPE_NULL MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_null)
#define MPI_REQUEST_NULL MPI_FT_PREDEFINED_GLOBAL(MPI_Request, mpi_ft_request_null)
#define MPI_ERRHANDLER_NULL ((MPI_Errhandler)0x14000000)
#define MPI_MESSAGE_NULL ((MPI_Message)0x2c000000)
#define MPI_MESSAGE_NO_PROC ((MPI_Message)0x6c000000)
#define MPI_IDENT 0
#define MPI_CONGRUENT 1
#define MPI_SIMILAR 2
#define MPI_UNEQUAL 3
#define MPI_CHAR MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_char)
#define MPI_SIGNED_CHAR MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_signed_char)
#define MPI_UNSIGNED_CHAR MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_unsigned_char)
#define MPI_BYTE MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_byte)
//#define MPI_WCHAR ((MPI_Datatype)0x4c00040e)
#define MPI_SHORT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_short)
#define MPI_UNSIGNED_SHORT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_unsigned_short)
#define MPI_INT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_int)
#define MPI_UNSIGNED MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_unsigned)
#define MPI_LONG MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_long)
#define MPI_UNSIGNED_LONG MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_unsigned_long)
#define MPI_FLOAT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_float)
#define MPI_DOUBLE MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_double)
#define MPI_LONG_DOUBLE MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_long_double)
#define MPI_LONG_LONG_INT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_long_long_int)
#define MPI_UNSIGNED_LONG_LONG MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_unsigned_long_long)
#define MPI_LONG_LONG MPI_LONG_LONG_INT

#define MPI_PACKED MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_packed)
//#define MPI_LB MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_lb)
//#define MPI_UB MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_ub)
#define MPI_FLOAT_INT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_float_int)
#define MPI_DOUBLE_INT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_double_int)
#define MPI_LONG_INT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_long_int)
#define MPI_SHORT_INT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_short_int)
#define MPI_2INT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_2int)
#define MPI_LONG_DOUBLE_INT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_long_double_int)
#define MPI_COMPLEX MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_complex)
#define MPI_DOUBLE_COMPLEX MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_double_complex)
#define MPI_LOGICAL MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_logical)
#define MPI_REAL MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_real)
#define MPI_DOUBLE_PRECISION MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_double_precision)
#define MPI_INTEGER MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_integer)
#define MPI_2INTEGER MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_2integer)
#define MPI_2REAL MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_2real)
#define MPI_2DOUBLE_PRECISION MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_2double_precision)
#define MPI_CHARACTER MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_character)
//#define MPI_REAL4 MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_real4)
//#define MPI_REAL8 MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_real8)
//#define MPI_REAL16 MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_real16)
//#define MPI_COMPLEX8 MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_complex8)
//#define MPI_COMPLEX16 MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_complex16)
//#define MPI_COMPLEX32 MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_complex32)
//#define MPI_INTEGER1 MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_integer1)
//#define MPI_INTEGER2 MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_integer2)
//#define MPI_INTEGER4 MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_integer4)
//#define MPI_INTEGER8 MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_integer8)
//#define MPI_INTEGER16 MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_integer16)
#define MPI_INT8_T MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_int8_t)
#define MPI_INT16_T MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_int16_t)
#define MPI_INT32_T MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_int32_t)
#define MPI_INT64_T MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_int64_t)
#define MPI_UINT8_T MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_uint8_t)
#define MPI_UINT16_T MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_uint16_t)
#define MPI_UINT32_T MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_uint32_t)
#define MPI_UINT64_T MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_uint64_t)
#define MPI_C_BOOL MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_c_bool)
#define MPI_C_FLOAT_COMPLEX MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_c_float_complex)
#define MPI_C_COMPLEX MPI_C_FLOAT_COMPLEX
#define MPI_C_DOUBLE_COMPLEX MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_c_double_complex)
#define MPI_C_LONG_DOUBLE_COMPLEX MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_c_long_double_complex)
#define MPI_AINT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_aint)
#define MPI_OFFSET MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_offset)
#define MPI_COUNT MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_count)
#define MPI_CXX_BOOL MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_cxx_bool)
#define MPI_CXX_FLOAT_COMPLEX MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_cxx_float_complex)
#define MPI_CXX_DOUBLE_COMPLEX MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_cxx_double_complex)
#define MPI_CXX_LONG_DOUBLE_COMPLEX MPI_FT_PREDEFINED_GLOBAL(MPI_Datatype, mpi_ft_datatype_cxx_long_double_complex)

#define MPI_TYPECLASS_REAL 1
#define MPI_TYPECLASS_INTEGER 2
#define MPI_TYPECLASS_COMPLEX 3
#define MPI_COMM_WORLD MPI_FT_PREDEFINED_GLOBAL(MPI_Comm,mpi_ft_comm_world)
#define MPI_COMM_SELF MPI_FT_PREDEFINED_GLOBAL(MPI_Comm,mpi_ft_comm_self)
#define MPI_GROUP_EMPTY ((MPI_Group)0x48000000)
#define MPI_WIN_NULL ((MPI_Win)0x20000000)
#define MPI_FILE_DEFINED 
#define MPI_FILE_NULL ((MPI_File)0)

#define MPI_MAX MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_max)
#define MPI_MIN MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_min)
#define MPI_SUM MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_sum)
#define MPI_PROD MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_prod)
#define MPI_LAND MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_land)
#define MPI_BAND MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_band)
#define MPI_LOR MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_lor)
#define MPI_BOR MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_bor)
#define MPI_LXOR MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_lxor)
#define MPI_BXOR MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_bxor)
#define MPI_MINLOC MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_minloc)
#define MPI_MAXLOC MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_maxloc)
#define MPI_REPLACE MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_replace)
#define MPI_NO_OP MPI_FT_PREDEFINED_GLOBAL(MPI_Op,mpi_ft_op_no_op)
#define MPI_TAG_UB 0x64400001
#define MPI_HOST 0x64400003
#define MPI_IO 0x64400005
#define MPI_WTIME_IS_GLOBAL 0x64400007
#define MPI_UNIVERSE_SIZE 0x64400009
#define MPI_LASTUSEDCODE 0x6440000b
#define MPI_APPNUM 0x6440000d
#define MPI_WIN_BASE 0x66000001
#define MPI_WIN_SIZE 0x66000003
#define MPI_WIN_DISP_UNIT 0x66000005
#define MPI_WIN_CREATE_FLAVOR 0x66000007
#define MPI_WIN_MODEL 0x66000009
#define MPI_MAX_PROCESSOR_NAME 128
#define MPI_MAX_LIBRARY_VERSION_STRING 8192
#define MPI_MAX_ERROR_STRING 512
#define MPI_MAX_PORT_NAME 256
#define MPI_MAX_OBJECT_NAME 128
#define MPI_UNDEFINED (-32766)
#define MPI_KEYVAL_INVALID 0x24000000
#define MPI_BSEND_OVERHEAD 96
#define MPI_BOTTOM (void *)0
#define MPI_PROC_NULL (-1)
#define MPI_ANY_SOURCE (-2)
#define MPI_ROOT (-3)
#define MPI_ANY_TAG (-1)
#define MPI_LOCK_EXCLUSIVE 234
#define MPI_LOCK_SHARED 235
#define MPI_ERRORS_ARE_FATAL ((MPI_Errhandler)0x54000000)
#define MPI_ERRORS_RETURN ((MPI_Errhandler)0x54000001)
#define MPIR_ERRORS_THROW_EXCEPTIONS ((MPI_Errhandler)0x54000002)
#define MPI_NULL_COPY_FN ((MPI_Copy_function *)0)
#define MPI_NULL_DELETE_FN ((MPI_Delete_function *)0)
#define MPI_DUP_FN MPIR_Dup_fn
#define MPI_COMM_NULL_COPY_FN ((MPI_Comm_copy_attr_function*)0)
#define MPI_COMM_NULL_DELETE_FN ((MPI_Comm_delete_attr_function*)0)
#define MPI_COMM_DUP_FN ((MPI_Comm_copy_attr_function *)MPI_DUP_FN)
#define MPI_WIN_NULL_COPY_FN ((MPI_Win_copy_attr_function*)0)
#define MPI_WIN_NULL_DELETE_FN ((MPI_Win_delete_attr_function*)0)
#define MPI_WIN_DUP_FN ((MPI_Win_copy_attr_function*)MPI_DUP_FN)
#define MPI_TYPE_NULL_COPY_FN ((MPI_Type_copy_attr_function*)0)
#define MPI_TYPE_NULL_DELETE_FN ((MPI_Type_delete_attr_function*)0)
#define MPI_TYPE_DUP_FN ((MPI_Type_copy_attr_function*)MPI_DUP_FN)
#define MPI_VERSION 3
#define MPI_SUBVERSION 1
#define MPICH_NAME 3
#define MPICH 1
#define MPICH_HAS_C2F 1
#define MPICH_VERSION "3.2.1"
#define MPICH_NUMVERSION 30201300
#define MPICH_RELEASE_TYPE_ALPHA 0
#define MPICH_RELEASE_TYPE_BETA 1
#define MPICH_RELEASE_TYPE_RC 2
#define MPICH_RELEASE_TYPE_PATCH 3
#define MPICH_CALC_VERSION(MAJOR,MINOR,REVISION,TYPE,PATCH) (((MAJOR) * 10000000) + ((MINOR) * 100000) + ((REVISION) * 1000) + ((TYPE) * 100) + (PATCH))
#define MPI_INFO_NULL ((MPI_Info)0x1c000000)
#define MPI_INFO_ENV ((MPI_Info)0x5c000001)
#define MPI_MAX_INFO_KEY 255
#define MPI_MAX_INFO_VAL 1024
#define MPI_ORDER_C 56
#define MPI_ORDER_FORTRAN 57
#define MPI_DISTRIBUTE_BLOCK 121
#define MPI_DISTRIBUTE_CYCLIC 122
#define MPI_DISTRIBUTE_NONE 123
#define MPI_DISTRIBUTE_DFLT_DARG -49767
#define MPI_IN_PLACE (void *) -1
#define MPI_MODE_NOCHECK 1024
#define MPI_MODE_NOSTORE 2048
#define MPI_MODE_NOPUT 4096
#define MPI_MODE_NOPRECEDE 8192
#define MPI_MODE_NOSUCCEED 16384
#define MPI_COMM_TYPE_SHARED 1
#define MPI_AINT_FMT_DEC_SPEC "%ld"
#define MPI_AINT_FMT_HEX_SPEC "%lx"
#define HAVE_MPI_OFFSET 
#define MPI_T_ENUM_NULL ((MPI_T_enum)NULL)
#define MPI_T_CVAR_HANDLE_NULL ((MPI_T_cvar_handle)NULL)
#define MPI_T_PVAR_HANDLE_NULL ((MPI_T_pvar_handle)NULL)
#define MPI_T_PVAR_SESSION_NULL ((MPI_T_pvar_session)NULL)
#define MPI_Comm_c2f(comm) (MPI_Fint)(comm)
#define MPI_Comm_f2c(comm) (MPI_Comm)(comm)
#define MPI_Type_c2f(datatype) (MPI_Fint)(datatype)
#define MPI_Type_f2c(datatype) (MPI_Datatype)(datatype)
#define MPI_Group_c2f(group) (MPI_Fint)(group)
#define MPI_Group_f2c(group) (MPI_Group)(group)
#define MPI_Info_c2f(info) (MPI_Fint)(info)
#define MPI_Info_f2c(info) (MPI_Info)(info)
#define MPI_Request_f2c(request) (MPI_Request)(request)
#define MPI_Request_c2f(request) (MPI_Fint)(request)
#define MPI_Op_c2f(op) (MPI_Fint)(op)
#define MPI_Op_f2c(op) (MPI_Op)(op)
#define MPI_Errhandler_c2f(errhandler) (MPI_Fint)(errhandler)
#define MPI_Errhandler_f2c(errhandler) (MPI_Errhandler)(errhandler)
#define MPI_Win_c2f(win) (MPI_Fint)(win)
#define MPI_Win_f2c(win) (MPI_Win)(win)
#define MPI_Message_c2f(msg) ((MPI_Fint)(msg))
#define MPI_Message_f2c(msg) ((MPI_Message)(msg))
#define PMPI_Comm_c2f(comm) (MPI_Fint)(comm)
#define PMPI_Comm_f2c(comm) (MPI_Comm)(comm)
#define PMPI_Type_c2f(datatype) (MPI_Fint)(datatype)
#define PMPI_Type_f2c(datatype) (MPI_Datatype)(datatype)
#define PMPI_Group_c2f(group) (MPI_Fint)(group)
#define PMPI_Group_f2c(group) (MPI_Group)(group)
#define PMPI_Info_c2f(info) (MPI_Fint)(info)
#define PMPI_Info_f2c(info) (MPI_Info)(info)
#define PMPI_Request_f2c(request) (MPI_Request)(request)
#define PMPI_Request_c2f(request) (MPI_Fint)(request)
#define PMPI_Op_c2f(op) (MPI_Fint)(op)
#define PMPI_Op_f2c(op) (MPI_Op)(op)
#define PMPI_Errhandler_c2f(errhandler) (MPI_Fint)(errhandler)
#define PMPI_Errhandler_f2c(errhandler) (MPI_Errhandler)(errhandler)
#define PMPI_Win_c2f(win) (MPI_Fint)(win)
#define PMPI_Win_f2c(win) (MPI_Win)(win)
#define PMPI_Message_c2f(msg) ((MPI_Fint)(msg))
#define PMPI_Message_f2c(msg) ((MPI_Message)(msg))
//#define MPI_STATUS_IGNORE MPI_FT_PREDEFINED_GLOBAL(MPI_Status *, mpi_ft_status_ignore)
#define MPI_STATUS_IGNORE (MPI_Status *)1
#define MPI_STATUSES_IGNORE (MPI_Status *)1
#define MPI_ERRCODES_IGNORE (int *)0
#define MPIU_DLL_SPEC 
#define MPI_ARGV_NULL (char **)0
#define MPI_ARGVS_NULL (char ***)0
#define MPI_THREAD_SINGLE 0
#define MPI_THREAD_FUNNELED 1
#define MPI_THREAD_SERIALIZED 2
#define MPI_THREAD_MULTIPLE 3
#define MPI_SUCCESS 0
#define MPI_ERR_BUFFER 1
#define MPI_ERR_COUNT 2
#define MPI_ERR_TYPE 3
#define MPI_ERR_TAG 4
#define MPI_ERR_COMM 5
#define MPI_ERR_RANK 6
#define MPI_ERR_ROOT 7
#define MPI_ERR_TRUNCATE 14
#define MPI_ERR_GROUP 8
#define MPI_ERR_OP 9
#define MPI_ERR_REQUEST 19
#define MPI_ERR_TOPOLOGY 10
#define MPI_ERR_DIMS 11
#define MPI_ERR_ARG 12
#define MPI_ERR_OTHER 15
#define MPI_ERR_UNKNOWN 13
#define MPI_ERR_INTERN 16
#define MPI_ERR_IN_STATUS 17
#define MPI_ERR_PENDING 18
#define MPI_ERR_ACCESS 20
#define MPI_ERR_AMODE 21
#define MPI_ERR_BAD_FILE 22
#define MPI_ERR_CONVERSION 23
#define MPI_ERR_DUP_DATAREP 24
#define MPI_ERR_FILE_EXISTS 25
#define MPI_ERR_FILE_IN_USE 26
#define MPI_ERR_FILE 27
#define MPI_ERR_IO 32
#define MPI_ERR_NO_SPACE 36
#define MPI_ERR_NO_SUCH_FILE 37
#define MPI_ERR_READ_ONLY 40
#define MPI_ERR_UNSUPPORTED_DATAREP 43
#define MPI_ERR_INFO 28
#define MPI_ERR_INFO_KEY 29
#define MPI_ERR_INFO_VALUE 30
#define MPI_ERR_INFO_NOKEY 31
#define MPI_ERR_NAME 33
#define MPI_ERR_NO_MEM 34
#define MPI_ERR_NOT_SAME 35
#define MPI_ERR_PORT 38
#define MPI_ERR_QUOTA 39
#define MPI_ERR_SERVICE 41
#define MPI_ERR_SPAWN 42
#define MPI_ERR_UNSUPPORTED_OPERATION 44
#define MPI_ERR_WIN 45
#define MPI_ERR_BASE 46
#define MPI_ERR_LOCKTYPE 47
#define MPI_ERR_KEYVAL 48
#define MPI_ERR_RMA_CONFLICT 49
#define MPI_ERR_RMA_SYNC 50
#define MPI_ERR_SIZE 51
#define MPI_ERR_DISP 52
#define MPI_ERR_ASSERT 53
#define MPI_ERR_RMA_RANGE 55
#define MPI_ERR_RMA_ATTACH 56
#define MPI_ERR_RMA_SHARED 57
#define MPI_ERR_RMA_FLAVOR 58
#define MPI_T_ERR_MEMORY 59
#define MPI_T_ERR_NOT_INITIALIZED 60
#define MPI_T_ERR_CANNOT_INIT 61
#define MPI_T_ERR_INVALID_INDEX 62
#define MPI_T_ERR_INVALID_ITEM 63
#define MPI_T_ERR_INVALID_HANDLE 64
#define MPI_T_ERR_OUT_OF_HANDLES 65
#define MPI_T_ERR_OUT_OF_SESSIONS 66
#define MPI_T_ERR_INVALID_SESSION 67
#define MPI_T_ERR_CVAR_SET_NOT_NOW 68
#define MPI_T_ERR_CVAR_SET_NEVER 69
#define MPI_T_ERR_PVAR_NO_STARTSTOP 70
#define MPI_T_ERR_PVAR_NO_WRITE 71
#define MPI_T_ERR_PVAR_NO_ATOMIC 72
#define MPI_T_ERR_INVALID_NAME 73
#define MPI_T_ERR_INVALID 74
#define MPI_ERR_LASTCODE 0x3fffffff
#define MPICH_ERR_LAST_CLASS 74
#define MPICH_ERR_FIRST_MPIX 100
#define MPIX_ERR_PROC_FAILED MPICH_ERR_FIRST_MPIX+1
#define MPIX_ERR_PROC_FAILED_PENDING MPICH_ERR_FIRST_MPIX+2
#define MPIX_ERR_REVOKED MPICH_ERR_FIRST_MPIX+3
#define MPICH_ERR_LAST_MPIX MPICH_ERR_FIRST_MPIX+3
#define MPI_CONVERSION_FN_NULL ((MPI_Datarep_conversion_function *)0)
#define MPIIMPL_ADVERTISES_FEATURES 1
#define MPIIMPL_HAVE_MPI_INFO 1
#define MPIIMPL_HAVE_MPI_COMBINER_DARRAY 1
#define MPIIMPL_HAVE_MPI_TYPE_CREATE_DARRAY 1
#define MPIIMPL_HAVE_MPI_COMBINER_SUBARRAY 1
#define MPIIMPL_HAVE_MPI_TYPE_CREATE_DARRAY 1
#define MPIIMPL_HAVE_MPI_COMBINER_DUP 1
#define MPIIMPL_HAVE_MPI_GREQUEST 1
#define MPIIMPL_HAVE_STATUS_SET_BYTES 1
#define MPIIMPL_HAVE_STATUS_SET_INFO 1
#define MPIO_INCLUDE 
#define HAVE_MPI_GREQUEST 1
#define MPIO_Request MPI_Request
#define MPIO_USES_MPI_REQUEST 
#define MPIO_Wait MPI_Wait
#define MPIO_Test MPI_Test
#define PMPIO_Wait PMPI_Wait
#define PMPIO_Test PMPI_Test
#define MPIO_REQUEST_DEFINED 
#define HAVE_MPI_INFO 
#define MPI_MODE_RDONLY 2
#define MPI_MODE_RDWR 8
#define MPI_MODE_WRONLY 4
#define MPI_MODE_CREATE 1
#define MPI_MODE_EXCL 64
#define MPI_MODE_DELETE_ON_CLOSE 16
#define MPI_MODE_UNIQUE_OPEN 32
#define MPI_MODE_APPEND 128
#define MPI_MODE_SEQUENTIAL 256
#define MPI_DISPLACEMENT_CURRENT -54278278
#define MPIO_REQUEST_NULL ((MPIO_Request) 0)
#define MPI_SEEK_SET 600
#define MPI_SEEK_CUR 602
#define MPI_SEEK_END 604
#define MPI_MAX_DATAREP_STRING 128
#define HAVE_MPI_DARRAY_SUBARRAY

#ifdef __cplusplus
}
#endif

#endif
