#ifndef _MPIF_H_
#define _MPIF_H_ 

module mpif
        use iso_c_binding
        implicit none
				
				real(c_double), bind(C) :: mpi_ft_start_time
				real(c_double), bind(C) :: mpi_ft_end_time
				
				integer(c_int), bind(C) :: parep_mpi_fortran_binding_used
				
				integer,parameter :: MPI_Status_size = 5
				integer,parameter,dimension(MPI_Status_size) :: MPI_STATUS_IGNORE = (/-1,-1,-1,-1,-1/)
				
				integer,parameter :: MPI_OP_NULL = 1
				integer,parameter :: MPI_DATATYPE_NULL = 1
				integer,parameter :: MPI_REQUEST_NULL = 1
				integer,parameter :: MPI_COMM_NULL = 1
				integer,parameter :: MPI_COMM_WORLD = 2
				integer,parameter :: MPI_COMM_SELF = 3
				
				integer,parameter :: MPI_CHAR = 2
				integer,parameter :: MPI_SIGNED_CHAR = 3
				integer,parameter :: MPI_UNSIGNED_CHAR = 4
				integer,parameter :: MPI_BYTE = 5
				integer,parameter :: MPI_SHORT = 6
				integer,parameter :: MPI_UNSIGNED_SHORT = 7
				integer,parameter :: MPI_INT = 8
				integer,parameter :: MPI_UNSIGNED = 9
				integer,parameter :: MPI_LONG = 10
				integer,parameter :: MPI_UNSIGNED_LONG = 11
				integer,parameter :: MPI_FLOAT = 12
				integer,parameter :: MPI_DOUBLE = 13
				integer,parameter :: MPI_LONG_DOUBLE = 14
				integer,parameter :: MPI_LONG_LONG_INT = 15
				integer,parameter :: MPI_LONG_LONG = 15
				integer,parameter :: MPI_UNSIGNED_LONG_LONG = 16
				
				integer,parameter :: MPI_PACKED = 17
				!integer,parameter :: MPI_LB = 18
				!integer,parameter :: MPI_UB = 19
				integer,parameter :: MPI_FLOAT_INT = 20
				integer,parameter :: MPI_DOUBLE_INT = 21
				integer,parameter :: MPI_LONG_INT = 22
				integer,parameter :: MPI_SHORT_INT = 23
				integer,parameter :: MPI_2INT = 24
				integer,parameter :: MPI_LONG_DOUBLE_INT = 25
				integer,parameter :: MPI_COMPLEX = 26
				integer,parameter :: MPI_DOUBLE_COMPLEX = 27
				integer,parameter :: MPI_LOGICAL = 28
				!integer,parameter :: MPI_REAL = 29
				integer,parameter :: MPI_REAL = 12
				!integer,parameter :: MPI_DOUBLE_PRECISION = 30
				integer,parameter :: MPI_DOUBLE_PRECISION = 13
				!integer,parameter :: MPI_INTEGER = 31
				integer,parameter :: MPI_INTEGER = 8
				integer,parameter :: MPI_2INTEGER = 32
				integer,parameter :: MPI_2REAL = 33
				integer,parameter :: MPI_2DOUBLE_PRECISION = 34
				integer,parameter :: MPI_CHARACTER = 35
				!integer,parameter :: MPI_REAL4 = 36
				!integer,parameter :: MPI_REAL8 = 37
				!integer,parameter :: MPI_REAL16 = 38
				!integer,parameter :: MPI_COMPLEX8 = 39
				!integer,parameter :: MPI_COMPLEX16 = 40
				!integer,parameter :: MPI_COMPLEX32 = 41
				!integer,parameter :: MPI_INTEGER1 = 42
				!integer,parameter :: MPI_INTEGER2 = 43
				!integer,parameter :: MPI_INTEGER4 = 44
				!integer,parameter :: MPI_INTEGER8 = 45
				!integer,parameter :: MPI_INTEGER16 = 46
				integer,parameter :: MPI_INT8_T = 47
				integer,parameter :: MPI_INT16_T = 48
				integer,parameter :: MPI_INT32_T = 49
				integer,parameter :: MPI_INT64_T = 50
				integer,parameter :: MPI_UINT8_T = 51
				integer,parameter :: MPI_UINT16_T = 52
				integer,parameter :: MPI_UINT32_T = 53
				integer,parameter :: MPI_UINT64_T = 54
				integer,parameter :: MPI_C_BOOL = 55
				integer,parameter :: MPI_C_FLOAT_COMPLEX = 56
				integer,parameter :: MPI_C_COMPLEX = 56
				integer,parameter :: MPI_C_DOUBLE_COMPLEX = 57
				integer,parameter :: MPI_C_LONG_DOUBLE_COMPLEX = 58
				integer,parameter :: MPI_AINT = 59
				integer,parameter :: MPI_OFFSET = 60
				integer,parameter :: MPI_COUNT = 61
				integer,parameter :: MPI_CXX_BOOL = 62
				integer,parameter :: MPI_CXX_FLOAT_COMPLEX = 63
				integer,parameter :: MPI_CXX_DOUBLE_COMPLEX = 64
				integer,parameter :: MPI_CXX_LONG_DOUBLE_COMPLEX = 65
				
				integer,parameter :: MPI_MAX = 2
				integer,parameter :: MPI_MIN = 3
				integer,parameter :: MPI_SUM = 4
				integer,parameter :: MPI_PROD = 5
				integer,parameter :: MPI_LAND = 6
				integer,parameter :: MPI_BAND = 7
				integer,parameter :: MPI_LOR = 8
				integer,parameter :: MPI_BOR = 9
				integer,parameter :: MPI_LXOR = 10
				integer,parameter :: MPI_BXOR = 11
				integer,parameter :: MPI_MINLOC = 12
				integer,parameter :: MPI_MAXLOC = 13
				integer,parameter :: MPI_REPLACE = 14
				integer,parameter :: MPI_NO_OP = 15
				
				integer,parameter :: MPI_ERR_OTHER = 15
				
				logical :: initialized
				
				type, bind(C) :: mpi_ft_comm
					type(c_ptr) :: eworldComm
					type(c_ptr) :: EMPI_COMM_CMP
					type(c_ptr) :: EMPI_COMM_REP
					type(c_ptr) :: EMPI_CMP_REP_INTERCOMM
					type(c_ptr) :: EMPI_CMP_NO_REP
					type(c_ptr) :: EMPI_CMP_NO_REP_INTERCOMM
				end type mpi_ft_comm
				type, bind(C) :: mpi_ft_datatype
					type(c_ptr) :: edatatype
					integer(c_int) :: size
				end type mpi_ft_datatype
				type, bind(C) :: mpi_ft_op
					type(c_ptr) :: eop
				end type mpi_ft_op
				type, bind(C) :: EMPI_Status
					integer(c_int) :: count_lo
					integer(c_int) :: count_hi_and_cancelled
					integer(c_int) :: MPI_SOURCE
					integer(c_int) :: MPI_TAG
					integer(c_int) :: MPI_ERROR
				end type EMPI_Status
				type, bind(C) :: mpi_ft_status
					type(EMPI_Status) :: status
					integer(c_int) :: count
					integer(c_int) :: MPI_SOURCE
					integer(c_int) :: MPI_TAG
					integer(c_int) :: MPI_ERROR
				end type mpi_ft_status
				type, bind(C) :: mpi_ft_request
					type(c_ptr) :: reqcmp
					type(c_ptr) :: reqrep
					type(c_ptr) :: reqcolls
					integer(c_int) :: num_reqcolls
					logical(c_bool) :: complete
					type(mpi_ft_comm) :: comm
					type(mpi_ft_status) :: status
					integer(c_int) :: type
					type(c_ptr) :: bufloc
					type(c_ptr) :: storeloc
					type(c_ptr) :: rnode
				end type mpi_ft_request
				
				type :: mpi_ft_request_ptr
					type(c_ptr), pointer :: p
				end type mpi_ft_request_ptr
				
				!type(c_ptr),target, dimension(1024) :: reqarr
				type(c_ptr),bind(C),target,dimension(1024) :: reqarr
				!logical, dimension(1024) :: reqinuse
				logical(c_bool),bind(C),dimension(1024) :: reqinuse
				
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_null
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_null
				type(mpi_ft_request),bind(C),target :: mpi_ft_request_null
				type(mpi_ft_comm),bind(C),target :: mpi_ft_comm_null
				type(mpi_ft_comm),bind(C),target :: mpi_ft_comm_world
				type(mpi_ft_comm),bind(C),target :: mpi_ft_comm_self
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_char
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_signed_char
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_unsigned_char
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_byte
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_short
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_unsigned_short
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_int
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_unsigned
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_long
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_unsigned_long
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_float
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_double
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_long_double
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_long_long_int
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_unsigned_long_long
				
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_packed
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_lb
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_ub
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_float_int
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_double_int
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_long_int
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_short_int
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_2int
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_long_double_int
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_complex
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_double_complex
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_logical
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_real
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_double_precision
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_integer
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_2integer
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_2real
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_2double_precision
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_character
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_real4
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_real8
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_real16
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_complex8
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_complex16
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_complex32
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_integer1
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_integer2
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_integer4
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_integer8
				!type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_integer16
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_int8_t
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_int16_t
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_int32_t
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_int64_t
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_uint8_t
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_uint16_t
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_uint32_t
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_uint64_t
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_c_bool
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_c_float_complex
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_c_double_complex
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_c_long_double_complex
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_aint
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_offset
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_count
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_cxx_bool
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_cxx_float_complex
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_cxx_double_complex
				type(mpi_ft_datatype),bind(C),target :: mpi_ft_datatype_cxx_long_double_complex
				
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_max
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_min
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_sum
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_prod
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_land
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_band
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_lor
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_bor
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_lxor
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_bxor
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_minloc
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_maxloc
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_replace
				type(mpi_ft_op),bind(C),target :: mpi_ft_op_no_op
				
				!type,bind(C) :: mpi_ft_comm_ptr
					!type(mpi_ft_datatype) :: p
				!end type mpi_ft_comm_ptr
				!type,bind(C) :: mpi_ft_datatype_ptr
					!type(mpi_ft_datatype), pointer :: p
				!end type mpi_ft_datatype_ptr
				!type,bind(C) :: mpi_ft_op_ptr
					!type(mpi_ft_op), pointer :: p
				!end type mpi_ft_op_ptr
				
				!type(mpi_ft_comm_ptr), dimension(1024) :: commarr
				!type(mpi_ft_datatype_ptr), dimension(1024) :: dtarr
				!type(mpi_ft_op_ptr), dimension(1024) :: oparr
				
				!type(mpi_ft_comm_ptr),bind(C),dimension(1024) :: commarr
				!type(mpi_ft_datatype_ptr),bind(C),dimension(1024) :: dtarr
				!type(mpi_ft_op_ptr),bind(C),dimension(1024) :: oparr
				
				type(c_ptr),bind(C),target,dimension(1024) :: commarr
				type(c_ptr),bind(C),target,dimension(1024) :: dtarr
				type(c_ptr),bind(C),target,dimension(1024) :: oparr
				
        interface mpi_finalize_interface
                function mpi_finalize_c() bind(C,name="MPI_Finalize")
                        import :: c_int
                        integer(c_int) :: mpi_finalize_c
                end function mpi_finalize_c
        end interface mpi_finalize_interface
        interface mpi_init_interface
                function mpi_init_c(argc, argv) bind(C,name="MPI_Init")
                        import :: c_int,c_ptr
                        integer(c_int) :: argc
                        type(c_ptr) :: argv
                        integer(c_int) :: mpi_init_c
                end function mpi_init_c
        end interface mpi_init_interface

        interface mpi_comm_size_interface
                function mpi_comm_size_c(comm, size) bind(C,name="MPI_Comm_size")
                        import :: c_int,c_ptr,mpi_ft_comm
                        type(c_ptr), value, intent(in) :: comm
                        integer(c_int), intent(inout) :: size
                        integer(c_int) :: mpi_comm_size_c
                end function mpi_comm_size_c
        end interface mpi_comm_size_interface

        interface mpi_comm_rank_interface
                function mpi_comm_rank_c(comm, rank) bind(C,name="MPI_Comm_rank")
                        import :: c_int,c_ptr,mpi_ft_comm
                        type(c_ptr), value, intent(in) :: comm
                        integer(c_int), intent(inout) :: rank
                        integer(c_int) :: mpi_comm_rank_c
                end function mpi_comm_rank_c
        end interface mpi_comm_rank_interface
				
				interface mpi_send_interface
                function mpi_send_c(buf,count,datatype,dest,tag,comm) bind(C,name="MPI_Send")
                        import :: c_int,c_ptr,mpi_ft_datatype,mpi_ft_comm
												type(c_ptr), value, intent(in) :: buf
												integer(c_int), intent(in), value :: count
												type(c_ptr), value, intent(in) :: datatype
												integer(c_int), intent(in), value :: dest
												integer(c_int), intent(in), value :: tag
                        type(c_ptr), value, intent(in) :: comm
												integer(c_int) :: mpi_send_c
                end function mpi_send_c
        end interface mpi_send_interface
				
				interface mpi_isend_interface
                function mpi_isend_c(buf,count,datatype,dest,tag,comm,request) bind(C,name="MPI_Isend")
                        import :: c_int,c_ptr,mpi_ft_datatype,mpi_ft_comm,mpi_ft_request
												type(c_ptr), value, intent(in) :: buf
												integer(c_int), intent(in), value :: count
												type(c_ptr), value, intent(in) :: datatype
												integer(c_int), intent(in), value :: dest
												integer(c_int), intent(in), value :: tag
                        type(c_ptr), value, intent(in) :: comm
												type(c_ptr),value :: request
												integer(c_int) :: mpi_isend_c
                end function mpi_isend_c
        end interface mpi_isend_interface
				
				interface mpi_recv_interface
                function mpi_recv_c(buf,count,datatype,src,tag,comm,status) bind(C,name="MPI_Recv")
                        import :: c_int,c_ptr,mpi_ft_datatype,mpi_ft_comm,mpi_ft_status
												type(c_ptr), value :: buf
												integer(c_int), intent(in), value :: count
												type(c_ptr), value, intent(in) :: datatype
												integer(c_int), intent(in), value :: src
												integer(c_int), intent(in), value :: tag
												type(c_ptr), value, intent(in) :: comm
												type(mpi_ft_status) :: status
												integer(c_int) :: mpi_recv_c
                end function mpi_recv_c
        end interface mpi_recv_interface
				
				interface mpi_irecv_interface
                function mpi_irecv_c(buf,count,datatype,src,tag,comm,request) bind(C,name="MPI_Irecv")
                        import :: c_int,c_ptr,mpi_ft_datatype,mpi_ft_comm,mpi_ft_request
												type(c_ptr), value :: buf
												integer(c_int), intent(in), value :: count
												type(c_ptr), value, intent(in) :: datatype
												integer(c_int), intent(in), value :: src
												integer(c_int), intent(in), value :: tag
                        type(c_ptr), value, intent(in) :: comm
												type(c_ptr),value :: request
												integer(c_int) :: mpi_irecv_c
                end function mpi_irecv_c
        end interface mpi_irecv_interface
				
				interface mpi_request_free_interface
                function mpi_request_free_c(request) bind(C,name="MPI_Request_free")
                        import :: c_int,c_ptr
												type(c_ptr),value :: request
												integer(c_int) :: mpi_request_free_c
                end function mpi_request_free_c
        end interface mpi_request_free_interface
				
				interface mpi_test_interface
                function mpi_test_c(request,flag,status) bind(C,name="MPI_Test")
                        import :: c_int,c_ptr,mpi_ft_status
												type(c_ptr),value :: request
												integer(c_int) :: flag
												type(mpi_ft_status) :: status
												integer(c_int) :: mpi_test_c
                end function mpi_test_c
        end interface mpi_test_interface
				
				interface mpi_wait_interface
                function mpi_wait_c(request,status) bind(C,name="MPI_Wait")
                        import :: c_int,c_ptr,mpi_ft_request,mpi_ft_status
												type(c_ptr),value :: request
												type(mpi_ft_status) :: status
												integer(c_int) :: mpi_wait_c
                end function mpi_wait_c
        end interface mpi_wait_interface
				
				interface mpi_bcast_interface
                function mpi_bcast_c(buf,count,datatype,root,comm) bind(C,name="MPI_Bcast")
                        import :: c_int,c_ptr,mpi_ft_datatype,mpi_ft_comm
												type(c_ptr), value :: buf
												integer(c_int), intent(in), value :: count
												type(c_ptr), value, intent(in) :: datatype
												integer(c_int), intent(in), value :: root
                        type(c_ptr), value, intent(in) :: comm
												integer(c_int) :: mpi_bcast_c
                end function mpi_bcast_c
        end interface mpi_bcast_interface
				
				interface mpi_scatter_interface
                function mpi_scatter_c(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,root,comm) bind(C,name="MPI_Scatter")
                        import :: c_int,c_ptr,mpi_ft_datatype,mpi_ft_comm
												type(c_ptr), value :: sendbuf,recvbuf
												integer(c_int), intent(in), value :: sendcount,recvcount
												type(c_ptr), value, intent(in) :: sendtype,recvtype
												integer(c_int), intent(in), value :: root
                        type(c_ptr), value, intent(in) :: comm
												integer(c_int) :: mpi_scatter_c
                end function mpi_scatter_c
        end interface mpi_scatter_interface
				
				interface mpi_gather_interface
                function mpi_gather_c(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,root,comm) bind(C,name="MPI_Gather")
                        import :: c_int,c_ptr,mpi_ft_datatype,mpi_ft_comm
												type(c_ptr), value :: sendbuf,recvbuf
												integer(c_int), intent(in), value :: sendcount,recvcount
												type(c_ptr), value, intent(in) :: sendtype,recvtype
												integer(c_int), intent(in), value :: root
                        type(c_ptr), value, intent(in) :: comm
												integer(c_int) :: mpi_gather_c
                end function mpi_gather_c
        end interface mpi_gather_interface
				
				interface mpi_reduce_interface
                function mpi_reduce_c(sendbuf,recvbuf,count,datatype,op,root,comm) bind(C,name="MPI_Reduce")
                        import :: c_int,c_ptr,mpi_ft_datatype,mpi_ft_comm,mpi_ft_op
												type(c_ptr), value :: sendbuf,recvbuf
												integer(c_int), intent(in), value :: count
												type(c_ptr), value, intent(in) :: datatype
												type(c_ptr), value, intent(in) :: op
												integer(c_int), intent(in), value :: root
                        type(c_ptr), value, intent(in) :: comm
												integer(c_int) :: mpi_reduce_c
                end function mpi_reduce_c
        end interface mpi_reduce_interface
				
				interface mpi_allgather_interface
                function mpi_allgather_c(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,comm) bind(C,name="MPI_Allgather")
                        import :: c_int,c_ptr,mpi_ft_datatype,mpi_ft_comm
												type(c_ptr), value :: sendbuf,recvbuf
												integer(c_int), intent(in), value :: sendcount,recvcount
												type(c_ptr), value, intent(in) :: sendtype,recvtype
                        type(c_ptr), value, intent(in) :: comm
												integer(c_int) :: mpi_allgather_c
                end function mpi_allgather_c
        end interface mpi_allgather_interface
				
				interface mpi_alltoall_interface
                function mpi_alltoall_c(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,comm) bind(C,name="MPI_Alltoall")
                        import :: c_int,c_ptr,mpi_ft_datatype,mpi_ft_comm
												type(c_ptr), value :: sendbuf,recvbuf
												integer(c_int), intent(in), value :: sendcount,recvcount
												type(c_ptr), value, intent(in) :: sendtype,recvtype
                        type(c_ptr), value,intent(in) :: comm
												integer(c_int) :: mpi_alltoall_c
                end function mpi_alltoall_c
        end interface mpi_alltoall_interface
				
				interface mpi_allreduce_interface
                function mpi_allreduce_c(sendbuf,recvbuf,count,datatype,op,comm) bind(C,name="MPI_Allreduce")
                        import :: c_int,c_ptr,mpi_ft_datatype,mpi_ft_comm,mpi_ft_op
												type(c_ptr), value :: sendbuf,recvbuf
												integer(c_int), intent(in), value :: count
												type(c_ptr), value, intent(in) :: datatype
												type(c_ptr), value, intent(in) :: op
                        type(c_ptr), value, intent(in) :: comm
												integer(c_int) :: mpi_allreduce_c
                end function mpi_allreduce_c
        end interface mpi_allreduce_interface
				
				interface mpi_alltoallv_interface
                function mpi_alltoallv_c(sendbuf,sendcounts,sdispls,sendtype,&
																				recvbuf,recvcounts,rdispls,recvtype,comm)&
																				bind(C,name="MPI_Alltoallv")
                        import :: c_int,c_ptr,mpi_ft_datatype,mpi_ft_comm
												type(c_ptr), value :: sendbuf,recvbuf
												integer(c_int), intent(in) :: sendcounts,recvcounts,sdispls,rdispls
												type(c_ptr), value, intent(in) :: sendtype,recvtype
                        type(c_ptr), value, intent(in) :: comm
												integer(c_int) :: mpi_alltoallv_c
                end function mpi_alltoallv_c
        end interface mpi_alltoallv_interface
				
				interface mpi_barrier_interface
                function mpi_barrier_c(comm) bind(C,name="MPI_Barrier")
                        import :: c_int,c_ptr,mpi_ft_comm
                        type(c_ptr), value, intent(in) :: comm
                        integer(c_int) :: mpi_barrier_c
                end function mpi_barrier_c
        end interface mpi_barrier_interface
				
				interface mpi_abort_interface
                function mpi_abort_c(comm, errorcode) bind(C,name="MPI_Abort")
                        import :: c_int,c_ptr,mpi_ft_comm
                        type(c_ptr), value, intent(in) :: comm
                        integer(c_int), value :: errorcode
                        integer(c_int) :: mpi_abort_c
                end function mpi_abort_c
        end interface mpi_abort_interface
				
				interface mpi_wtime_interface
                function mpi_wtime_c() bind(C,name="MPI_Wtime")
                        import :: c_double
                        real(c_double) :: mpi_wtime_c
                end function mpi_wtime_c
        end interface mpi_wtime_interface
				
				interface parep_mpi_fortran_preinit_interface
					subroutine parep_mpi_fortran_preinit() bind(C,name="parep_mpi_fortran_preinit")
					end subroutine parep_mpi_fortran_preinit
				end interface parep_mpi_fortran_preinit_interface
					
				interface parep_mpi_fortran_postfinalize_interface
					subroutine parep_mpi_fortran_postfinalize() bind(C,name="parep_mpi_fortran_postfinalize")
					end subroutine parep_mpi_fortran_postfinalize
				end interface parep_mpi_fortran_postfinalize_interface
				
	contains
	
	subroutine mpi_finalize(ierror)
		integer, intent(out) :: ierror
		ierror = mpi_finalize_c()
		initialized = .false.
		call parep_mpi_fortran_postfinalize()
	end subroutine mpi_finalize
	
	subroutine mpi_init(ierror)
		integer, intent(out) :: ierror
		integer i
		type(c_ptr) :: null_req
		call parep_mpi_fortran_preinit()
		
		parep_mpi_fortran_binding_used = 1
		ierror = mpi_init_c(0,c_null_ptr)
		commarr(1) = c_loc(mpi_ft_comm_null)
		commarr(2) = c_loc(mpi_ft_comm_world)
		commarr(3) = c_loc(mpi_ft_comm_self)

		dtarr(1) = c_loc(mpi_ft_datatype_null)
		dtarr(2) = c_loc(mpi_ft_datatype_char)
		dtarr(3) = c_loc(mpi_ft_datatype_signed_char)
		dtarr(4) = c_loc(mpi_ft_datatype_unsigned_char)
		dtarr(5) = c_loc(mpi_ft_datatype_byte)
		dtarr(6) = c_loc(mpi_ft_datatype_short)
		dtarr(7) = c_loc(mpi_ft_datatype_unsigned_short)
		dtarr(8) = c_loc(mpi_ft_datatype_int)
		dtarr(9) = c_loc(mpi_ft_datatype_unsigned)
		dtarr(10) = c_loc(mpi_ft_datatype_long)
		dtarr(11) = c_loc(mpi_ft_datatype_unsigned_long)
		dtarr(12) = c_loc(mpi_ft_datatype_float)
		dtarr(13) = c_loc(mpi_ft_datatype_double)
		dtarr(14) = c_loc(mpi_ft_datatype_long_double)
		dtarr(15) = c_loc(mpi_ft_datatype_long_long_int)
		dtarr(16) = c_loc(mpi_ft_datatype_unsigned_long_long)
		
		dtarr(17) = c_loc(mpi_ft_datatype_packed)
		!dtarr(18) = c_loc(mpi_ft_datatype_lb)
		!dtarr(19) = c_loc(mpi_ft_datatype_ub)
		dtarr(20) = c_loc(mpi_ft_datatype_float_int)
		dtarr(21) = c_loc(mpi_ft_datatype_double_int)
		dtarr(22) = c_loc(mpi_ft_datatype_long_int)
		dtarr(23) = c_loc(mpi_ft_datatype_short_int)
		dtarr(24) = c_loc(mpi_ft_datatype_2int)
		dtarr(25) = c_loc(mpi_ft_datatype_long_double_int)
		dtarr(26) = c_loc(mpi_ft_datatype_complex)
		dtarr(27) = c_loc(mpi_ft_datatype_double_complex)
		dtarr(28) = c_loc(mpi_ft_datatype_logical)
		dtarr(29) = c_loc(mpi_ft_datatype_real)
		dtarr(30) = c_loc(mpi_ft_datatype_double_precision)
		dtarr(31) = c_loc(mpi_ft_datatype_integer)
		dtarr(32) = c_loc(mpi_ft_datatype_2integer)
		dtarr(33) = c_loc(mpi_ft_datatype_2real)
		dtarr(34) = c_loc(mpi_ft_datatype_2double_precision)
		dtarr(35) = c_loc(mpi_ft_datatype_character)
		!dtarr(36) = c_loc(mpi_ft_datatype_real4)
		!dtarr(37) = c_loc(mpi_ft_datatype_real8)
		!dtarr(38) = c_loc(mpi_ft_datatype_real16)
		!dtarr(39) = c_loc(mpi_ft_datatype_complex8)
		!dtarr(40) = c_loc(mpi_ft_datatype_complex16)
		!dtarr(41) = c_loc(mpi_ft_datatype_complex32)
		!dtarr(42) = c_loc(mpi_ft_datatype_integer1)
		!dtarr(43) = c_loc(mpi_ft_datatype_integer2)
		!dtarr(44) = c_loc(mpi_ft_datatype_integer4)
		!dtarr(45) = c_loc(mpi_ft_datatype_integer8)
		!dtarr(46) = c_loc(mpi_ft_datatype_integer16)
		dtarr(47) = c_loc(mpi_ft_datatype_int8_t)
		dtarr(48) = c_loc(mpi_ft_datatype_int16_t)
		dtarr(49) = c_loc(mpi_ft_datatype_int32_t)
		dtarr(50) = c_loc(mpi_ft_datatype_int64_t)
		dtarr(51) = c_loc(mpi_ft_datatype_uint8_t)
		dtarr(52) = c_loc(mpi_ft_datatype_uint16_t)
		dtarr(53) = c_loc(mpi_ft_datatype_uint32_t)
		dtarr(54) = c_loc(mpi_ft_datatype_uint64_t)
		dtarr(55) = c_loc(mpi_ft_datatype_c_bool)
		dtarr(56) = c_loc(mpi_ft_datatype_c_float_complex)
		dtarr(57) = c_loc(mpi_ft_datatype_c_double_complex)
		dtarr(58) = c_loc(mpi_ft_datatype_c_long_double_complex)
		dtarr(59) = c_loc(mpi_ft_datatype_aint)
		dtarr(60) = c_loc(mpi_ft_datatype_offset)
		dtarr(61) = c_loc(mpi_ft_datatype_count)
		dtarr(62) = c_loc(mpi_ft_datatype_cxx_bool)
		dtarr(63) = c_loc(mpi_ft_datatype_cxx_float_complex)
		dtarr(64) = c_loc(mpi_ft_datatype_cxx_double_complex)
		dtarr(65) = c_loc(mpi_ft_datatype_cxx_long_double_complex)
		
		oparr(1) = c_loc(mpi_ft_op_null)
		oparr(2) = c_loc(mpi_ft_op_max)
		oparr(3) = c_loc(mpi_ft_op_min)
		oparr(4) = c_loc(mpi_ft_op_sum)
		oparr(5) = c_loc(mpi_ft_op_prod)
		oparr(6) = c_loc(mpi_ft_op_land)
		oparr(7) = c_loc(mpi_ft_op_band)
		oparr(8) = c_loc(mpi_ft_op_lor)
		oparr(9) = c_loc(mpi_ft_op_bor)
		oparr(10) = c_loc(mpi_ft_op_lxor)
		oparr(11) = c_loc(mpi_ft_op_bxor)
		oparr(12) = c_loc(mpi_ft_op_minloc)
		oparr(13) = c_loc(mpi_ft_op_maxloc)
		oparr(14) = c_loc(mpi_ft_op_replace)
		oparr(15) = c_loc(mpi_ft_op_no_op)
		null_req = c_loc(mpi_ft_request_null)
		reqarr(1) = null_req
		reqinuse(1) = .true.
		do i = 2, 1024
			reqinuse(i) = .false.
		end do
		initialized = .true.
	end subroutine mpi_init
	
	subroutine mpi_initialized(flag,ierror)
		integer, intent(out) :: ierror
		logical :: flag
		flag = initialized
		ierror = 0
	end subroutine mpi_initialized
	
	subroutine mpi_comm_size(comm,size,ierror)
		integer, intent(out) :: ierror
		integer :: comm
		integer(c_int), intent(inout) :: size
		type(c_ptr) :: ccomm
		ccomm = commarr(comm)
		ierror = mpi_comm_size_c(ccomm,size)
	end subroutine mpi_comm_size
	
	subroutine mpi_comm_rank(comm,rank,ierror)
		integer, intent(out) :: ierror
		integer :: comm
		integer(c_int), intent(inout) :: rank
		type(c_ptr) :: ccomm
		ccomm = commarr(comm)
		ierror = mpi_comm_rank_c(ccomm,rank)
	end subroutine mpi_comm_rank
	
	subroutine mpi_send(buf,count,datatype,dest,tag,comm,ierror)
		type(*), dimension(..), target, intent(in) :: buf
		integer(c_int), intent(in), value :: count
		type(c_ptr) :: cdatatype
		integer(c_int), intent(in), value :: dest
		integer(c_int), intent(in), value :: tag
		type(c_ptr) :: ccomm
		integer :: comm
		integer :: datatype
		integer, intent(out) :: ierror
		ccomm = commarr(comm)
		cdatatype = dtarr(datatype)
		ierror = mpi_send_c(c_loc(buf),count,cdatatype,dest,tag,ccomm)
	end subroutine mpi_send
	
	subroutine mpi_isend(buf,count,datatype,dest,tag,comm,request,ierror)
		type(*), dimension(..), target, intent(in) :: buf
		integer(c_int), intent(in), value :: count
		type(c_ptr) :: cdatatype
		integer(c_int), intent(in), value :: dest
		integer(c_int), intent(in), value :: tag
		type(c_ptr) :: ccomm
		integer :: request
		integer :: comm,datatype
		integer i
		integer, intent(out) :: ierror
		type(c_ptr), target :: req
		ccomm = commarr(comm)
		cdatatype = dtarr(datatype)
		do i = 2, 1024
			if (.not.(reqinuse(i))) then
				req = c_loc(reqarr(i))
				request = i
				reqinuse(i) = .true.
        			exit
			end if
		end do
		ierror = mpi_isend_c(c_loc(buf),count,cdatatype,dest,tag,ccomm,req)
	end subroutine mpi_isend
	
	subroutine mpi_recv(buf,count,datatype,src,tag,comm,status,ierror)
		type(*), dimension(..), target :: buf
		integer(c_int), intent(in), value :: count
		type(c_ptr) :: cdatatype
		integer(c_int), intent(in), value :: src
		integer(c_int), intent(in), value :: tag
		type(c_ptr) :: ccomm
		integer, dimension(MPI_Status_size) :: status
		integer, intent(out) :: ierror
		type(mpi_ft_status), target :: stat
		integer :: comm,datatype
		ccomm = commarr(comm)
		cdatatype = dtarr(datatype)
		ierror = mpi_recv_c(c_loc(buf),count,cdatatype,src,tag,ccomm,stat)
		if(status(1) /= -1) then
			status(1) = stat%count
			status(2) = stat%count
			status(3) = stat%MPI_SOURCE
			status(4) = stat%MPI_TAG
			status(5) = stat%MPI_ERROR
		end if
	end subroutine mpi_recv

	subroutine mpi_irecv(buf,count,datatype,src,tag,comm,request,ierror)
		type(*), dimension(..), target :: buf
		integer(c_int), intent(in), value :: count
		type(c_ptr) :: cdatatype
		integer(c_int), intent(in), value :: src
		integer(c_int), intent(in), value :: tag
		type(c_ptr),target :: ccomm
		integer :: request
		integer, intent(out) :: ierror
		type(c_ptr), target :: req
		integer :: comm,datatype
		integer i
		ccomm = commarr(comm)
		cdatatype = dtarr(datatype)
		do i = 2, 1024
			if (.not.(reqinuse(i))) then
				req = c_loc(reqarr(i))
				request = i
				reqinuse(i) = .true.
				exit
			end if
		end do
		!print *,"Fortran Irecv req loc ",c_loc(req)
		ierror = mpi_irecv_c(c_loc(buf),count,cdatatype,src,tag,ccomm,req)
	end subroutine mpi_irecv
	
	subroutine mpi_request_free(request,ierror)
		integer, intent(out) :: ierror
		integer :: request
		type(c_ptr), target :: req
		req = c_loc(reqarr(request))
		ierror = mpi_request_free_c(req)
		reqinuse(request) = .false.
	end subroutine mpi_request_free
	
	subroutine mpi_test(request,flag,status,ierror)
		integer :: request
		integer(c_int), intent(inout) :: flag
		integer, dimension(MPI_Status_size) :: status
		integer, intent(out) :: ierror
		type(c_ptr), target :: req
		type(mpi_ft_status), target :: stat
		req = c_loc(reqarr(request))
		ierror = mpi_test_c(req,flag,stat)
		if(status(1) /= -1) then
			status(1) = stat%count
			status(2) = stat%count
			status(3) = stat%MPI_SOURCE
			status(4) = stat%MPI_TAG
			status(5) = stat%MPI_ERROR
		end if
		if(flag == 1) then
			reqinuse(request) = .false.
		end if
	end subroutine mpi_test
	
	subroutine mpi_wait(request,status,ierror)
		integer, intent(out) :: ierror
		integer :: request
		integer, dimension(MPI_Status_size) :: status
		type(c_ptr), target :: req
		type(mpi_ft_status), target :: stat
		req = c_loc(reqarr(request))
		!print *,"Fortran req loc ",c_loc(req)," from reqarr loc ",c_loc(reqarr(request))
		ierror = mpi_wait_c(req,stat)
		if(status(1) /= -1) then
			status(1) = stat%count
			status(2) = stat%count
			status(3) = stat%MPI_SOURCE
			status(4) = stat%MPI_TAG
			status(5) = stat%MPI_ERROR
		end if
		reqinuse(request) = .false.
	end subroutine mpi_wait
	
	subroutine mpi_waitall(count,requests,statuses,ierror)
		integer, intent(out) :: ierror
		integer :: count
		integer, dimension(count) :: requests
		integer, dimension(count,MPI_Status_size) :: statuses
		integer i
		type(c_ptr), dimension(count), target :: req
		type(mpi_ft_status), dimension(count), target :: stat
		do i = 1,count
			req(i) = c_loc(reqarr(requests(i)))
			ierror = mpi_wait_c(req(i),stat(i))
			if(statuses(i,1) /= -1) then
				statuses(i,1) = stat(i)%count
				statuses(i,2) = stat(i)%count
				statuses(i,3) = stat(i)%MPI_SOURCE
				statuses(i,4) = stat(i)%MPI_TAG
				statuses(i,5) = stat(i)%MPI_ERROR
			end if
			reqinuse(requests(i)) = .false.
		end do
	end subroutine mpi_waitall
	
	subroutine mpi_get_count(status,datatype,count,ierror)
		integer, dimension(MPI_Status_size) :: status
		integer :: datatype
		integer :: count
		integer, intent(out) :: ierror
		count = status(1)
		ierror = 0
	end subroutine mpi_get_count
	
	subroutine mpi_bcast(buf,count,datatype,root,comm,ierror)
		type(*), dimension(..), target :: buf
		integer(c_int), intent(in), value :: count
		type(c_ptr) :: cdatatype
		integer(c_int), intent(in), value :: root
		type(c_ptr) :: ccomm
		integer, intent(out) :: ierror
		integer :: comm,datatype
		ccomm = commarr(comm)
		cdatatype = dtarr(datatype)
		ierror = mpi_bcast_c(c_loc(buf),count,cdatatype,root,ccomm)
	end subroutine mpi_bcast
	
	subroutine mpi_scatter(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,root,comm,ierror)
		type(*), dimension(..), target :: sendbuf,recvbuf
		integer(c_int), intent(in), value :: sendcount,recvcount
		type(c_ptr) :: csendtype,crecvtype
		integer(c_int), intent(in), value :: root
		type(c_ptr) :: ccomm
		integer, intent(out) :: ierror
		integer :: comm,sendtype,recvtype
		ccomm = commarr(comm)
		csendtype = dtarr(sendtype)
		crecvtype = dtarr(recvtype)
		ierror = mpi_scatter_c(c_loc(sendbuf),sendcount,csendtype,c_loc(recvbuf),recvcount,crecvtype,root,ccomm)
	end subroutine mpi_scatter
	
	subroutine mpi_gather(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,root,comm,ierror)
		type(*), dimension(..), target :: sendbuf,recvbuf
		integer(c_int), intent(in), value :: sendcount,recvcount
		type(c_ptr) :: csendtype,crecvtype
		integer(c_int), intent(in), value :: root
		type(c_ptr) :: ccomm
		integer, intent(out) :: ierror
		integer :: comm,sendtype,recvtype
		ccomm = commarr(comm)
		csendtype = dtarr(sendtype)
		crecvtype = dtarr(recvtype)
		ierror = mpi_gather_c(c_loc(sendbuf),sendcount,csendtype,c_loc(recvbuf),recvcount,crecvtype,root,ccomm)
	end subroutine mpi_gather
	
	subroutine mpi_reduce(sendbuf,recvbuf,count,datatype,op,root,comm,ierror)
		type(*), dimension(..), target :: sendbuf,recvbuf
		integer(c_int), intent(in), value :: count
		type(c_ptr) :: cdatatype
		type(c_ptr) :: cop
		integer(c_int), intent(in), value :: root
		type(c_ptr) :: ccomm
		integer, intent(out) :: ierror
		integer :: comm,datatype,op
		ccomm = commarr(comm)
		cdatatype = dtarr(datatype)
		cop = oparr(op)
		ierror = mpi_reduce_c(c_loc(sendbuf),c_loc(recvbuf),count,cdatatype,cop,root,ccomm)
	end subroutine mpi_reduce
	
	subroutine mpi_allgather(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,comm,ierror)
		type(*), dimension(..), target :: sendbuf,recvbuf
		integer(c_int), intent(in), value :: sendcount,recvcount
		type(c_ptr):: csendtype,crecvtype
		type(c_ptr) :: ccomm
		integer, intent(out) :: ierror
		integer :: comm,sendtype,recvtype
		ccomm = commarr(comm)
		csendtype = dtarr(sendtype)
		crecvtype = dtarr(recvtype)
		ierror = mpi_allgather_c(c_loc(sendbuf),sendcount,csendtype,c_loc(recvbuf),recvcount,crecvtype,ccomm)
	end subroutine mpi_allgather
	
	subroutine mpi_alltoall(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,comm,ierror)
		type(*), dimension(..), target :: sendbuf,recvbuf
		integer(c_int), intent(in), value :: sendcount,recvcount
		type(c_ptr) :: csendtype,crecvtype
		type(c_ptr) :: ccomm
		integer, intent(out) :: ierror
		integer :: comm,sendtype,recvtype
		ccomm = commarr(comm)
		csendtype = dtarr(sendtype)
		crecvtype = dtarr(recvtype)
		ierror = mpi_alltoall_c(c_loc(sendbuf),sendcount,csendtype,c_loc(recvbuf),recvcount,crecvtype,ccomm)
	end subroutine mpi_alltoall
	
	subroutine mpi_allreduce(sendbuf,recvbuf,count,datatype,op,comm,ierror)
		type(*), dimension(..), target :: sendbuf,recvbuf
		integer(c_int), intent(in), value :: count
		type(c_ptr) :: cdatatype
		type(c_ptr) :: cop
		type(c_ptr) :: ccomm
		integer, intent(out) :: ierror
		integer :: comm,datatype,op
		ccomm = commarr(comm)
		cdatatype = dtarr(datatype)
		cop = oparr(op)
		ierror = mpi_allreduce_c(c_loc(sendbuf),c_loc(recvbuf),count,cdatatype,cop,ccomm)
	end subroutine mpi_allreduce
	
	subroutine mpi_alltoallv(sendbuf,sendcounts,sdispls,sendtype,recvbuf,recvcounts,rdispls,recvtype,comm,ierror)
		type(*), dimension(..), target :: sendbuf,recvbuf
		integer(c_int), intent(in) :: sendcounts,recvcounts,sdispls,rdispls
		type(c_ptr) :: csendtype,crecvtype
		type(c_ptr) :: ccomm
		integer, intent(out) :: ierror
		integer :: comm,sendtype,recvtype
		ccomm = commarr(comm)
		csendtype = dtarr(sendtype)
		crecvtype = dtarr(recvtype)
		ierror = mpi_alltoallv_c(c_loc(sendbuf),sendcounts,sdispls,csendtype,c_loc(recvbuf),recvcounts,rdispls,crecvtype,ccomm)
	end subroutine mpi_alltoallv
	
	subroutine mpi_barrier(comm,ierror)
		integer, intent(out) :: ierror
		type(c_ptr) :: ccomm
		integer :: comm
		ccomm = commarr(comm)
		ierror = mpi_barrier_c(ccomm)
	end subroutine mpi_barrier
	
	subroutine mpi_abort(comm,errorcode,ierror)
		integer, intent(out) :: ierror
		type(c_ptr) :: ccomm
		integer(c_int), value :: errorcode
		integer :: comm
		ccomm = commarr(comm)
		ierror = mpi_abort_c(ccomm,errorcode)
	end subroutine mpi_abort
	
	function mpi_wtime()
		double precision :: mpi_wtime
		mpi_wtime = mpi_wtime_c()
	end function mpi_wtime
	
end module mpif

#endif
