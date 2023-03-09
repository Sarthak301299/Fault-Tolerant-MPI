#ifndef _EMPI_DECLARATION_H_
#define _EMPI_DECLARATION_H_
#define EMPI_INCLUDED 
#undef EMPICH_DEFINE_ATTR_TYPE_TYPES
#define EMPICH_ATTR_POINTER_WITH_TYPE_TAG(buffer_idx,type_idx) 
#define EMPICH_ATTR_TYPE_TAG(type) 
#define EMPICH_ATTR_TYPE_TAG_LAYOUT_COMPATIBLE(type) 
#define EMPICH_ATTR_TYPE_TAG_MUST_BE_NULL() 
#define EMPICH_ATTR_TYPE_TAG_STDINT(type) 
#define EMPICH_ATTR_TYPE_TAG_C99(type) 
#define EMPICH_ATTR_TYPE_TAG_CXX(type) 
#define EMPI_COMM_NULL ((EMPI_Comm)0x04000000)
#define EMPI_OP_NULL ((EMPI_Op)0x18000000)
#define EMPI_GROUP_NULL ((EMPI_Group)0x08000000)
#define EMPI_DATATYPE_NULL ((EMPI_Datatype)0x0c000000)
#define EMPI_REQUEST_NULL ((EMPI_Request)0x2c000000)
#define EMPI_ERRHANDLER_NULL ((EMPI_Errhandler)0x14000000)
#define EMPI_MESSAGE_NULL ((EMPI_Message)0x2c000000)
#define EMPI_MESSAGE_NO_PROC ((EMPI_Message)0x6c000000)
#define EMPI_IDENT 0
#define EMPI_CONGRUENT 1
#define EMPI_SIMILAR 2
#define EMPI_UNEQUAL 3
#define EMPI_CHAR ((EMPI_Datatype)0x4c000101)
#define EMPI_SIGNED_CHAR ((EMPI_Datatype)0x4c000118)
#define EMPI_UNSIGNED_CHAR ((EMPI_Datatype)0x4c000102)
#define EMPI_BYTE ((EMPI_Datatype)0x4c00010d)
#define EMPI_WCHAR ((EMPI_Datatype)0x4c00040e)
#define EMPI_SHORT ((EMPI_Datatype)0x4c000203)
#define EMPI_UNSIGNED_SHORT ((EMPI_Datatype)0x4c000204)
#define EMPI_INT ((EMPI_Datatype)0x4c000405)
#define EMPI_UNSIGNED ((EMPI_Datatype)0x4c000406)
#define EMPI_LONG ((EMPI_Datatype)0x4c000807)
#define EMPI_UNSIGNED_LONG ((EMPI_Datatype)0x4c000808)
#define EMPI_FLOAT ((EMPI_Datatype)0x4c00040a)
#define EMPI_DOUBLE ((EMPI_Datatype)0x4c00080b)
#define EMPI_LONG_DOUBLE ((EMPI_Datatype)0x4c00100c)
#define EMPI_LONG_LONG_INT ((EMPI_Datatype)0x4c000809)
#define EMPI_UNSIGNED_LONG_LONG ((EMPI_Datatype)0x4c000819)
#define EMPI_LONG_LONG EMPI_LONG_LONG_INT
#define EMPI_PACKED ((EMPI_Datatype)0x4c00010f)
#define EMPI_LB ((EMPI_Datatype)0x4c000010)
#define EMPI_UB ((EMPI_Datatype)0x4c000011)
#define EMPI_FLOAT_INT ((EMPI_Datatype)0x8c000000)
#define EMPI_DOUBLE_INT ((EMPI_Datatype)0x8c000001)
#define EMPI_LONG_INT ((EMPI_Datatype)0x8c000002)
#define EMPI_SHORT_INT ((EMPI_Datatype)0x8c000003)
#define EMPI_2INT ((EMPI_Datatype)0x4c000816)
#define EMPI_LONG_DOUBLE_INT ((EMPI_Datatype)0x8c000004)
#define EMPI_COMPLEX 1275070494
#define EMPI_DOUBLE_COMPLEX 1275072546
#define EMPI_LOGICAL 1275069469
#define EMPI_REAL 1275069468
#define EMPI_DOUBLE_PRECISION 1275070495
#define EMPI_INTEGER 1275069467
#define EMPI_2INTEGER 1275070496
#define EMPI_2REAL 1275070497
#define EMPI_2DOUBLE_PRECISION 1275072547
#define EMPI_CHARACTER 1275068698
#define EMPI_REAL4 ((EMPI_Datatype)0x4c000427)
#define EMPI_REAL8 ((EMPI_Datatype)0x4c000829)
#define EMPI_REAL16 ((EMPI_Datatype)0x4c00102b)
#define EMPI_COMPLEX8 ((EMPI_Datatype)0x4c000828)
#define EMPI_COMPLEX16 ((EMPI_Datatype)0x4c00102a)
#define EMPI_COMPLEX32 ((EMPI_Datatype)0x4c00202c)
#define EMPI_INTEGER1 ((EMPI_Datatype)0x4c00012d)
#define EMPI_INTEGER2 ((EMPI_Datatype)0x4c00022f)
#define EMPI_INTEGER4 ((EMPI_Datatype)0x4c000430)
#define EMPI_INTEGER8 ((EMPI_Datatype)0x4c000831)
#define EMPI_INTEGER16 ((EMPI_Datatype)EMPI_DATATYPE_NULL)
#define EMPI_INT8_T ((EMPI_Datatype)0x4c000137)
#define EMPI_INT16_T ((EMPI_Datatype)0x4c000238)
#define EMPI_INT32_T ((EMPI_Datatype)0x4c000439)
#define EMPI_INT64_T ((EMPI_Datatype)0x4c00083a)
#define EMPI_UINT8_T ((EMPI_Datatype)0x4c00013b)
#define EMPI_UINT16_T ((EMPI_Datatype)0x4c00023c)
#define EMPI_UINT32_T ((EMPI_Datatype)0x4c00043d)
#define EMPI_UINT64_T ((EMPI_Datatype)0x4c00083e)
#define EMPI_C_BOOL ((EMPI_Datatype)0x4c00013f)
#define EMPI_C_FLOAT_COMPLEX ((EMPI_Datatype)0x4c000840)
#define EMPI_C_COMPLEX EMPI_C_FLOAT_COMPLEX
#define EMPI_C_DOUBLE_COMPLEX ((EMPI_Datatype)0x4c001041)
#define EMPI_C_LONG_DOUBLE_COMPLEX ((EMPI_Datatype)0x4c002042)
#define EMPI_AINT ((EMPI_Datatype)0x4c000843)
#define EMPI_OFFSET ((EMPI_Datatype)0x4c000844)
#define EMPI_COUNT ((EMPI_Datatype)0x4c000845)
#define EMPI_CXX_BOOL ((EMPI_Datatype)0x4c000133)
#define EMPI_CXX_FLOAT_COMPLEX ((EMPI_Datatype)0x4c000834)
#define EMPI_CXX_DOUBLE_COMPLEX ((EMPI_Datatype)0x4c001035)
#define EMPI_CXX_LONG_DOUBLE_COMPLEX ((EMPI_Datatype)0x4c002036)
#define EMPI_TYPECLASS_REAL 1
#define EMPI_TYPECLASS_INTEGER 2
#define EMPI_TYPECLASS_COMPLEX 3
#define EMPI_COMM_WORLD ((EMPI_Comm)0x44000000)
#define EMPI_COMM_SELF ((EMPI_Comm)0x44000001)
#define EMPI_GROUP_EMPTY ((EMPI_Group)0x48000000)
#define EMPI_WIN_NULL ((EMPI_Win)0x20000000)
#define EMPI_FILE_DEFINED 
#define EMPI_FILE_NULL ((EMPI_File)0)
#define EMPI_MAX (EMPI_Op)(0x58000001)
#define EMPI_MIN (EMPI_Op)(0x58000002)
#define EMPI_SUM (EMPI_Op)(0x58000003)
#define EMPI_PROD (EMPI_Op)(0x58000004)
#define EMPI_LAND (EMPI_Op)(0x58000005)
#define EMPI_BAND (EMPI_Op)(0x58000006)
#define EMPI_LOR (EMPI_Op)(0x58000007)
#define EMPI_BOR (EMPI_Op)(0x58000008)
#define EMPI_LXOR (EMPI_Op)(0x58000009)
#define EMPI_BXOR (EMPI_Op)(0x5800000a)
#define EMPI_MINLOC (EMPI_Op)(0x5800000b)
#define EMPI_MAXLOC (EMPI_Op)(0x5800000c)
#define EMPI_REPLACE (EMPI_Op)(0x5800000d)
#define EMPI_NO_OP (EMPI_Op)(0x5800000e)
#define EMPI_TAG_UB 0x64400001
#define EMPI_HOST 0x64400003
#define EMPI_IO 0x64400005
#define EMPI_WTIME_IS_GLOBAL 0x64400007
#define EMPI_UNIVERSE_SIZE 0x64400009
#define EMPI_LASTUSEDCODE 0x6440000b
#define EMPI_APPNUM 0x6440000d
#define EMPI_WIN_BASE 0x66000001
#define EMPI_WIN_SIZE 0x66000003
#define EMPI_WIN_DISP_UNIT 0x66000005
#define EMPI_WIN_CREATE_FLAVOR 0x66000007
#define EMPI_WIN_MODEL 0x66000009
#define EMPI_MAX_PROCESSOR_NAME 128
#define EMPI_MAX_LIBRARY_VERSION_STRING 8192
#define EMPI_MAX_ERROR_STRING 512
#define EMPI_MAX_PORT_NAME 256
#define EMPI_MAX_OBJECT_NAME 128
#define EMPI_UNDEFINED (-32766)
#define EMPI_KEYVAL_INVALID 0x24000000
#define EMPI_BSEND_OVERHEAD 96
#define EMPI_BOTTOM (void *)0
#define EMPI_PROC_NULL (-1)
#define EMPI_ANY_SOURCE (-2)
#define EMPI_ROOT (-3)
#define EMPI_ANY_TAG (-1)
#define EMPI_LOCK_EXCLUSIVE 234
#define EMPI_LOCK_SHARED 235
#define EMPI_ERRORS_ARE_FATAL ((EMPI_Errhandler)0x54000000)
#define EMPI_ERRORS_RETURN ((EMPI_Errhandler)0x54000001)
#define EMPIR_ERRORS_THROW_EXCEPTIONS ((EMPI_Errhandler)0x54000002)
#define EMPI_NULL_COPY_FN ((EMPI_Copy_function *)0)
#define EMPI_NULL_DELETE_FN ((EMPI_Delete_function *)0)
#define EMPI_DUP_FN EMPIR_Dup_fn
#define EMPI_COMM_NULL_COPY_FN ((EMPI_Comm_copy_attr_function*)0)
#define EMPI_COMM_NULL_DELETE_FN ((EMPI_Comm_delete_attr_function*)0)
#define EMPI_COMM_DUP_FN ((EMPI_Comm_copy_attr_function *)EMPI_DUP_FN)
#define EMPI_WIN_NULL_COPY_FN ((EMPI_Win_copy_attr_function*)0)
#define EMPI_WIN_NULL_DELETE_FN ((EMPI_Win_delete_attr_function*)0)
#define EMPI_WIN_DUP_FN ((EMPI_Win_copy_attr_function*)EMPI_DUP_FN)
#define EMPI_TYPE_NULL_COPY_FN ((EMPI_Type_copy_attr_function*)0)
#define EMPI_TYPE_NULL_DELETE_FN ((EMPI_Type_delete_attr_function*)0)
#define EMPI_TYPE_DUP_FN ((EMPI_Type_copy_attr_function*)EMPI_DUP_FN)
#define EMPI_VERSION 3
#define EMPI_SUBVERSION 1
#define EMPICH_NAME 3
#define EMPICH 1
#define EMPICH_HAS_C2F 1
#define EMPICH_VERSION "3.2.1"
#define EMPICH_NUMVERSION 30201300
#define EMPICH_RELEASE_TYPE_ALPHA 0
#define EMPICH_RELEASE_TYPE_BETA 1
#define EMPICH_RELEASE_TYPE_RC 2
#define EMPICH_RELEASE_TYPE_PATCH 3
#define EMPICH_CALC_VERSION(MAJOR,MINOR,REVISION,TYPE,PATCH) (((MAJOR) * 10000000) + ((MINOR) * 100000) + ((REVISION) * 1000) + ((TYPE) * 100) + (PATCH))
#define EMPI_INFO_NULL ((EMPI_Info)0x1c000000)
#define EMPI_INFO_ENV ((EMPI_Info)0x5c000001)
#define EMPI_MAX_INFO_KEY 255
#define EMPI_MAX_INFO_VAL 1024
#define EMPI_ORDER_C 56
#define EMPI_ORDER_FORTRAN 57
#define EMPI_DISTRIBUTE_BLOCK 121
#define EMPI_DISTRIBUTE_CYCLIC 122
#define EMPI_DISTRIBUTE_NONE 123
#define EMPI_DISTRIBUTE_DFLT_DARG -49767
#define EMPI_IN_PLACE (void *) -1
#define EMPI_MODE_NOCHECK 1024
#define EMPI_MODE_NOSTORE 2048
#define EMPI_MODE_NOPUT 4096
#define EMPI_MODE_NOPRECEDE 8192
#define EMPI_MODE_NOSUCCEED 16384
#define EMPI_COMM_TYPE_SHARED 1
#define EMPI_AINT_FMT_DEC_SPEC "%ld"
#define EMPI_AINT_FMT_HEX_SPEC "%lx"
#define HAVE_EMPI_OFFSET 
#define EMPI_T_ENUM_NULL ((EMPI_T_enum)NULL)
#define EMPI_T_CVAR_HANDLE_NULL ((EMPI_T_cvar_handle)NULL)
#define EMPI_T_PVAR_HANDLE_NULL ((EMPI_T_pvar_handle)NULL)
#define EMPI_T_PVAR_SESSION_NULL ((EMPI_T_pvar_session)NULL)
#define EMPI_Comm_c2f(comm) (EMPI_Fint)(comm)
#define EMPI_Comm_f2c(comm) (EMPI_Comm)(comm)
#define EMPI_Type_c2f(datatype) (EMPI_Fint)(datatype)
#define EMPI_Type_f2c(datatype) (EMPI_Datatype)(datatype)
#define EMPI_Group_c2f(group) (EMPI_Fint)(group)
#define EMPI_Group_f2c(group) (EMPI_Group)(group)
#define EMPI_Info_c2f(info) (EMPI_Fint)(info)
#define EMPI_Info_f2c(info) (EMPI_Info)(info)
#define EMPI_Request_f2c(request) (EMPI_Request)(request)
#define EMPI_Request_c2f(request) (EMPI_Fint)(request)
#define EMPI_Op_c2f(op) (EMPI_Fint)(op)
#define EMPI_Op_f2c(op) (EMPI_Op)(op)
#define EMPI_Errhandler_c2f(errhandler) (EMPI_Fint)(errhandler)
#define EMPI_Errhandler_f2c(errhandler) (EMPI_Errhandler)(errhandler)
#define EMPI_Win_c2f(win) (EMPI_Fint)(win)
#define EMPI_Win_f2c(win) (EMPI_Win)(win)
#define EMPI_Message_c2f(msg) ((EMPI_Fint)(msg))
#define EMPI_Message_f2c(msg) ((EMPI_Message)(msg))
#define PEMPI_Comm_c2f(comm) (EMPI_Fint)(comm)
#define PEMPI_Comm_f2c(comm) (EMPI_Comm)(comm)
#define PEMPI_Type_c2f(datatype) (EMPI_Fint)(datatype)
#define PEMPI_Type_f2c(datatype) (EMPI_Datatype)(datatype)
#define PEMPI_Group_c2f(group) (EMPI_Fint)(group)
#define PEMPI_Group_f2c(group) (EMPI_Group)(group)
#define PEMPI_Info_c2f(info) (EMPI_Fint)(info)
#define PEMPI_Info_f2c(info) (EMPI_Info)(info)
#define PEMPI_Request_f2c(request) (EMPI_Request)(request)
#define PEMPI_Request_c2f(request) (EMPI_Fint)(request)
#define PEMPI_Op_c2f(op) (EMPI_Fint)(op)
#define PEMPI_Op_f2c(op) (EMPI_Op)(op)
#define PEMPI_Errhandler_c2f(errhandler) (EMPI_Fint)(errhandler)
#define PEMPI_Errhandler_f2c(errhandler) (EMPI_Errhandler)(errhandler)
#define PEMPI_Win_c2f(win) (EMPI_Fint)(win)
#define PEMPI_Win_f2c(win) (EMPI_Win)(win)
#define PEMPI_Message_c2f(msg) ((EMPI_Fint)(msg))
#define PEMPI_Message_f2c(msg) ((EMPI_Message)(msg))
#define EMPI_STATUS_IGNORE (EMPI_Status *)1
#define EMPI_STATUSES_IGNORE (EMPI_Status *)1
#define EMPI_ERRCODES_IGNORE (int *)0
#define EMPIU_DLL_SPEC 
#define EMPI_ARGV_NULL (char **)0
#define EMPI_ARGVS_NULL (char ***)0
#define EMPI_THREAD_SINGLE 0
#define EMPI_THREAD_FUNNELED 1
#define EMPI_THREAD_SERIALIZED 2
#define EMPI_THREAD_MULTIPLE 3
#define EMPI_SUCCESS 0
#define EMPI_ERR_BUFFER 1
#define EMPI_ERR_COUNT 2
#define EMPI_ERR_TYPE 3
#define EMPI_ERR_TAG 4
#define EMPI_ERR_COMM 5
#define EMPI_ERR_RANK 6
#define EMPI_ERR_ROOT 7
#define EMPI_ERR_TRUNCATE 14
#define EMPI_ERR_GROUP 8
#define EMPI_ERR_OP 9
#define EMPI_ERR_REQUEST 19
#define EMPI_ERR_TOPOLOGY 10
#define EMPI_ERR_DIMS 11
#define EMPI_ERR_ARG 12
#define EMPI_ERR_OTHER 15
#define EMPI_ERR_UNKNOWN 13
#define EMPI_ERR_INTERN 16
#define EMPI_ERR_IN_STATUS 17
#define EMPI_ERR_PENDING 18
#define EMPI_ERR_ACCESS 20
#define EMPI_ERR_AMODE 21
#define EMPI_ERR_BAD_FILE 22
#define EMPI_ERR_CONVERSION 23
#define EMPI_ERR_DUP_DATAREP 24
#define EMPI_ERR_FILE_EXISTS 25
#define EMPI_ERR_FILE_IN_USE 26
#define EMPI_ERR_FILE 27
#define EMPI_ERR_IO 32
#define EMPI_ERR_NO_SPACE 36
#define EMPI_ERR_NO_SUCH_FILE 37
#define EMPI_ERR_READ_ONLY 40
#define EMPI_ERR_UNSUPPORTED_DATAREP 43
#define EMPI_ERR_INFO 28
#define EMPI_ERR_INFO_KEY 29
#define EMPI_ERR_INFO_VALUE 30
#define EMPI_ERR_INFO_NOKEY 31
#define EMPI_ERR_NAME 33
#define EMPI_ERR_NO_MEM 34
#define EMPI_ERR_NOT_SAME 35
#define EMPI_ERR_PORT 38
#define EMPI_ERR_QUOTA 39
#define EMPI_ERR_SERVICE 41
#define EMPI_ERR_SPAWN 42
#define EMPI_ERR_UNSUPPORTED_OPERATION 44
#define EMPI_ERR_WIN 45
#define EMPI_ERR_BASE 46
#define EMPI_ERR_LOCKTYPE 47
#define EMPI_ERR_KEYVAL 48
#define EMPI_ERR_RMA_CONFLICT 49
#define EMPI_ERR_RMA_SYNC 50
#define EMPI_ERR_SIZE 51
#define EMPI_ERR_DISP 52
#define EMPI_ERR_ASSERT 53
#define EMPI_ERR_RMA_RANGE 55
#define EMPI_ERR_RMA_ATTACH 56
#define EMPI_ERR_RMA_SHARED 57
#define EMPI_ERR_RMA_FLAVOR 58
#define EMPI_T_ERR_MEMORY 59
#define EMPI_T_ERR_NOT_INITIALIZED 60
#define EMPI_T_ERR_CANNOT_INIT 61
#define EMPI_T_ERR_INVALID_INDEX 62
#define EMPI_T_ERR_INVALID_ITEM 63
#define EMPI_T_ERR_INVALID_HANDLE 64
#define EMPI_T_ERR_OUT_OF_HANDLES 65
#define EMPI_T_ERR_OUT_OF_SESSIONS 66
#define EMPI_T_ERR_INVALID_SESSION 67
#define EMPI_T_ERR_CVAR_SET_NOT_NOW 68
#define EMPI_T_ERR_CVAR_SET_NEVER 69
#define EMPI_T_ERR_PVAR_NO_STARTSTOP 70
#define EMPI_T_ERR_PVAR_NO_WRITE 71
#define EMPI_T_ERR_PVAR_NO_ATOMIC 72
#define EMPI_T_ERR_INVALID_NAME 73
#define EMPI_T_ERR_INVALID 74
#define EMPI_ERR_LASTCODE 0x3fffffff
#define EMPICH_ERR_LAST_CLASS 74
#define EMPICH_ERR_FIRST_EMPIX 100
#define EMPIX_ERR_PROC_FAILED EMPICH_ERR_FIRST_EMPIX+1
#define EMPIX_ERR_PROC_FAILED_PENDING EMPICH_ERR_FIRST_EMPIX+2
#define EMPIX_ERR_REVOKED EMPICH_ERR_FIRST_EMPIX+3
#define EMPICH_ERR_LAST_EMPIX EMPICH_ERR_FIRST_EMPIX+3
#define EMPI_CONVERSION_FN_NULL ((EMPI_Datarep_conversion_function *)0)
#define EMPIIMPL_ADVERTISES_FEATURES 1
#define EMPIIMPL_HAVE_EMPI_INFO 1
#define EMPIIMPL_HAVE_EMPI_COMBINER_DARRAY 1
#define EMPIIMPL_HAVE_EMPI_TYPE_CREATE_DARRAY 1
#define EMPIIMPL_HAVE_EMPI_COMBINER_SUBARRAY 1
#define EMPIIMPL_HAVE_EMPI_TYPE_CREATE_DARRAY 1
#define EMPIIMPL_HAVE_EMPI_COMBINER_DUP 1
#define EMPIIMPL_HAVE_EMPI_GREQUEST 1
#define EMPIIMPL_HAVE_STATUS_SET_BYTES 1
#define EMPIIMPL_HAVE_STATUS_SET_INFO 1
#define EMPIO_INCLUDE 
#define HAVE_EMPI_GREQUEST 1
#define EMPIO_Request EMPI_Request
#define EMPIO_USES_EMPI_REQUEST 
#define EMPIO_Wait EMPI_Wait
#define EMPIO_Test EMPI_Test
#define PEMPIO_Wait PEMPI_Wait
#define PEMPIO_Test PEMPI_Test
#define EMPIO_REQUEST_DEFINED 
#define HAVE_EMPI_INFO 
#define EMPI_MODE_RDONLY 2
#define EMPI_MODE_RDWR 8
#define EMPI_MODE_WRONLY 4
#define EMPI_MODE_CREATE 1
#define EMPI_MODE_EXCL 64
#define EMPI_MODE_DELETE_ON_CLOSE 16
#define EMPI_MODE_UNIQUE_OPEN 32
#define EMPI_MODE_APPEND 128
#define EMPI_MODE_SEQUENTIAL 256
#define EMPI_DISPLACEMENT_CURRENT -54278278
#define EMPIO_REQUEST_NULL ((EMPIO_Request) 0)
#define EMPI_SEEK_SET 600
#define EMPI_SEEK_CUR 602
#define EMPI_SEEK_END 604
#define EMPI_MAX_DATAREP_STRING 128
#define HAVE_EMPI_DARRAY_SUBARRAY 
#endif
