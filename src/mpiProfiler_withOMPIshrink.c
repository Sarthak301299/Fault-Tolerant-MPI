//#include <stddef.h>
//#include "mpiProfilerdefs.h"
//#include "typedefs.h"
//#include "declarations.h"
//#include "ompitypedefs.h"
//#include "ompideclarations.h"
#include "mpi-internal.h"
#include "mpi.h"
#ifdef HEAPTRACK
#include <sys/time.h>
#endif

#if SIZE_MAX == UCHAR_MAX
   #define my_EMPI_SIZE_T EMPI_UNSIGNED_CHAR
   #define my_OMPI_SIZE_T OMPI_UNSIGNED_CHAR
#elif SIZE_MAX == USHRT_MAX
   #define my_EMPI_SIZE_T EMPI_UNSIGNED_SHORT
   #define my_OMPI_SIZE_T OMPI_UNSIGNED_SHORT
#elif SIZE_MAX == UINT_MAX
   #define my_EMPI_SIZE_T EMPI_UNSIGNED
   #define my_OMPI_SIZE_T OMPI_UNSIGNED
#elif SIZE_MAX == ULONG_MAX
   #define my_EMPI_SIZE_T EMPI_UNSIGNED_LONG
   #define my_OMPI_SIZE_T OMPI_UNSIGNED_LONG
#elif SIZE_MAX == ULLONG_MAX
   #define my_EMPI_SIZE_T EMPI_UNSIGNED_LONG_LONG
   #define my_OMPI_SIZE_T OMPI_UNSIGNED_LONG_LONG
#else
   #error "what is happening here?"
#endif

#ifdef HEAPTRACK
size_t total_alloc = 0;
struct timeval tv;
void *__wrap_malloc(size_t size) {
	void *ptr = __real_malloc(size);
	if(getenv("MPI_FT_TRACK_HEAP"))
	{
		if(!strcmp(getenv("MPI_FT_TRACK_HEAP"),"1"))
		{
			gettimeofday(&tv,NULL);
			total_alloc += size;
			fprintf(logfile,"%d %ld\n",total_alloc,1000000 * tv.tv_sec + tv.tv_usec);
		}
	}
	return ptr;
}
#endif
/*
struct mpi_ft_datatype{
  OMPI_Datatype odatatype;
  EMPI_Datatype edatatype;
  int size;
};

struct mpi_ft_comm{
  OMPI_Comm oworldComm;
  OMPI_Comm OMPI_COMM_CMP;
  OMPI_Comm OMPI_COMM_REP;
  OMPI_Comm OMPI_CMP_REP_INTERCOMM;
	OMPI_Comm OMPI_CMP_NO_REP;
	OMPI_Comm OMPI_CMP_NO_REP_INTERCOMM;
  EMPI_Comm eworldComm;
  EMPI_Comm EMPI_COMM_CMP;
  EMPI_Comm EMPI_COMM_REP;
  EMPI_Comm EMPI_CMP_REP_INTERCOMM;
	EMPI_Comm EMPI_CMP_NO_REP;
	EMPI_Comm EMPI_CMP_NO_REP_INTERCOMM;
};

struct mpi_ft_op{
  OMPI_Op oop;
  EMPI_Op eop;
  //void *func;
};

struct mpi_ft_status{
	EMPI_Status status;
	int count;
	//int count_lo;
	//int count_hi_and_cancelled;
	int MPI_SOURCE;
	int MPI_TAG;
	int MPI_ERROR;
};

struct mpi_ft_request{
	EMPI_Request *reqcmp;
	EMPI_Request *reqrep;
	bool complete;
	MPI_Comm comm;
	MPI_Status status;
	int type;
	int backup;
	int *backuploc;
	void *storeloc;
};
*/
void *extLib = NULL;
void *openLib = NULL;

struct mpi_ft_comm mpi_ft_comm_null;
struct mpi_ft_op mpi_ft_op_null;
struct mpi_ft_datatype mpi_ft_datatype_null;
struct mpi_ft_request mpi_ft_request_null;
struct mpi_ft_comm mpi_ft_comm_world;
struct mpi_ft_comm mpi_ft_comm_self;
struct mpi_ft_datatype mpi_ft_datatype_char;
struct mpi_ft_datatype mpi_ft_datatype_signed_char;
struct mpi_ft_datatype mpi_ft_datatype_unsigned_char;
struct mpi_ft_datatype mpi_ft_datatype_byte;
struct mpi_ft_datatype mpi_ft_datatype_short;
struct mpi_ft_datatype mpi_ft_datatype_unsigned_short;
struct mpi_ft_datatype mpi_ft_datatype_int;
struct mpi_ft_datatype mpi_ft_datatype_unsigned;
struct mpi_ft_datatype mpi_ft_datatype_long;
struct mpi_ft_datatype mpi_ft_datatype_unsigned_long;
struct mpi_ft_datatype mpi_ft_datatype_float;
struct mpi_ft_datatype mpi_ft_datatype_double;
struct mpi_ft_datatype mpi_ft_datatype_long_double;
struct mpi_ft_datatype mpi_ft_datatype_long_long_int;
struct mpi_ft_datatype mpi_ft_datatype_unsigned_long_long;

struct mpi_ft_datatype mpi_ft_datatype_packed;
//struct mpi_ft_datatype mpi_ft_datatype_lb;
//struct mpi_ft_datatype mpi_ft_datatype_ub;
struct mpi_ft_datatype mpi_ft_datatype_float_int;
struct mpi_ft_datatype mpi_ft_datatype_double_int;
struct mpi_ft_datatype mpi_ft_datatype_long_int;
struct mpi_ft_datatype mpi_ft_datatype_short_int;
struct mpi_ft_datatype mpi_ft_datatype_2int;
struct mpi_ft_datatype mpi_ft_datatype_long_double_int;
struct mpi_ft_datatype mpi_ft_datatype_complex;
struct mpi_ft_datatype mpi_ft_datatype_double_complex;
struct mpi_ft_datatype mpi_ft_datatype_logical;
struct mpi_ft_datatype mpi_ft_datatype_real;
struct mpi_ft_datatype mpi_ft_datatype_double_precision;
struct mpi_ft_datatype mpi_ft_datatype_integer;
struct mpi_ft_datatype mpi_ft_datatype_2integer;
struct mpi_ft_datatype mpi_ft_datatype_2real;
struct mpi_ft_datatype mpi_ft_datatype_2double_precision;
struct mpi_ft_datatype mpi_ft_datatype_character;
//struct mpi_ft_datatype mpi_ft_datatype_real4;
//struct mpi_ft_datatype mpi_ft_datatype_real8;
//struct mpi_ft_datatype mpi_ft_datatype_real16;
//struct mpi_ft_datatype mpi_ft_datatype_complex8;
//struct mpi_ft_datatype mpi_ft_datatype_complex16;
//struct mpi_ft_datatype mpi_ft_datatype_complex32;
//struct mpi_ft_datatype mpi_ft_datatype_integer1;
//struct mpi_ft_datatype mpi_ft_datatype_integer2;
//struct mpi_ft_datatype mpi_ft_datatype_integer4;
//struct mpi_ft_datatype mpi_ft_datatype_integer8;
//struct mpi_ft_datatype mpi_ft_datatype_integer16;
struct mpi_ft_datatype mpi_ft_datatype_int8_t;
struct mpi_ft_datatype mpi_ft_datatype_int16_t;
struct mpi_ft_datatype mpi_ft_datatype_int32_t;
struct mpi_ft_datatype mpi_ft_datatype_int64_t;
struct mpi_ft_datatype mpi_ft_datatype_uint8_t;
struct mpi_ft_datatype mpi_ft_datatype_uint16_t;
struct mpi_ft_datatype mpi_ft_datatype_uint32_t;
struct mpi_ft_datatype mpi_ft_datatype_uint64_t;
struct mpi_ft_datatype mpi_ft_datatype_c_bool;
struct mpi_ft_datatype mpi_ft_datatype_c_float_complex;
struct mpi_ft_datatype mpi_ft_datatype_c_double_complex;
struct mpi_ft_datatype mpi_ft_datatype_c_long_double_complex;
struct mpi_ft_datatype mpi_ft_datatype_aint;
struct mpi_ft_datatype mpi_ft_datatype_offset;
struct mpi_ft_datatype mpi_ft_datatype_count;
struct mpi_ft_datatype mpi_ft_datatype_cxx_bool;
struct mpi_ft_datatype mpi_ft_datatype_cxx_float_complex;
struct mpi_ft_datatype mpi_ft_datatype_cxx_double_complex;
struct mpi_ft_datatype mpi_ft_datatype_cxx_long_double_complex;

struct mpi_ft_op mpi_ft_op_max;
struct mpi_ft_op mpi_ft_op_min;
struct mpi_ft_op mpi_ft_op_sum;
struct mpi_ft_op mpi_ft_op_prod;
struct mpi_ft_op mpi_ft_op_land;
struct mpi_ft_op mpi_ft_op_band;
struct mpi_ft_op mpi_ft_op_lor;
struct mpi_ft_op mpi_ft_op_bor;
struct mpi_ft_op mpi_ft_op_lxor;
struct mpi_ft_op mpi_ft_op_bxor;
struct mpi_ft_op mpi_ft_op_minloc;
struct mpi_ft_op mpi_ft_op_maxloc;
struct mpi_ft_op mpi_ft_op_replace;
struct mpi_ft_op mpi_ft_op_no_op;


void mpi_ft_datatype_init(OMPI_Datatype odatatype, EMPI_Datatype edatatype, int size, struct mpi_ft_datatype *out)
{
	out->odatatype = odatatype;
	out->edatatype = edatatype;
	out->size = size;
}

void mpi_ft_comm_init(OMPI_Comm oworldComm, OMPI_Comm ocmpComm, OMPI_Comm orepComm, OMPI_Comm ointerComm, OMPI_Comm ocmpnorepComm, OMPI_Comm ocmpnorepinterComm, EMPI_Comm eworldComm, EMPI_Comm ecmpComm, EMPI_Comm erepComm, EMPI_Comm einterComm, EMPI_Comm ecmpnorepComm, EMPI_Comm ecmpnorepinterComm, struct mpi_ft_comm *out)
{
	out->oworldComm = oworldComm;
	out->OMPI_COMM_CMP = ocmpComm;
	out->OMPI_COMM_REP = orepComm;
	out->OMPI_CMP_REP_INTERCOMM = ointerComm;
	out->OMPI_CMP_NO_REP = ocmpnorepComm;
	out->OMPI_CMP_NO_REP_INTERCOMM = ocmpnorepinterComm;
	out->eworldComm = eworldComm;
	out->EMPI_COMM_CMP = ecmpComm;
	out->EMPI_COMM_REP = erepComm;
	out->EMPI_CMP_REP_INTERCOMM = einterComm;
	out->EMPI_CMP_NO_REP = ecmpnorepComm;
	out->EMPI_CMP_NO_REP_INTERCOMM = ecmpnorepinterComm;
}

void mpi_ft_op_init(OMPI_Op oop, EMPI_Op eop, struct mpi_ft_op *out)
{
	out->oop = oop;
	out->eop = eop;
}

void mpi_ft_status_init(EMPI_Status estatus, int count, int source, int tag, int error, struct mpi_ft_status *out)
{
	out->status = estatus;
	out->count = count;
	out->MPI_SOURCE = source;
	out->MPI_TAG = tag;
	out->MPI_ERROR = error;
}

void mpi_ft_request_init(EMPI_Request reqcmp, EMPI_Request reqrep, bool complete, MPI_Comm comm, MPI_Status *status, int type, /*void* backup, int *backuploc*/void *bufloc, void *storeloc, struct mpi_ft_request *out)
{
	out->reqcmp = (EMPI_Request *)malloc(sizeof(EMPI_Request));
	out->reqrep = (EMPI_Request *)malloc(sizeof(EMPI_Request));
	*(out->reqcmp) = reqcmp;
	*(out->reqrep) = reqrep;
	out->complete = complete;
	out->comm = comm;
	if(status != MPI_STATUS_IGNORE) out->status = *status;
	out->type = type;
	//out->backup = backup;
	//out->backuploc = backuploc;
	out->bufloc = bufloc;
	out->storeloc = storeloc;
}

long array_sum(const int *array,int size)
{
	long sum = 0;
	for(int i = 0; i < size; i++)
	{
		sum += array[i];
	}
	return sum;
}

//MPI_Comm MPI_COMM_CMP, MPI_COMM_REP, CMP_REP_INTERCOMM;
//int *repToCmpMap = NULL, *cmpToRepMap = NULL;
//RAW_ADDR stackPosInit;
//jmp_buf env;
//int repRank, nC, nR;
//struct MPIReqStruct *mpiReqHead = NULL, *mpiReqTail = NULL;
//struct heapListStruct *heapListHead = NULL, *heapListTail = NULL;
//RAW_ADDR dataSegAddr [3];
//RAW_ADDR stackSegAddr [2];

/* FUNCTIONS TO KEEP TRACK OF MPI_REQUESTs - BEGIN */

/* In the current implementation, process image replacement shouldn't happen when there are in-flight messages in the system. Pending MPI_Request's indicate the possibility of the presence of in-flight messages. Below set of functions are used to keep track of MPI_Request's using a linked list of the data structure 'struct MPIReqStruct' for the purpose of making sure that there are no requests pending before doing a process replace */

/* Function to create an object of MPIReqStruct using a given request handle and add it to the linked list */
struct EMPIReqStruct *ecreateMR (EMPI_Request *request)
{
  struct EMPIReqStruct *temp = (struct EMPIReqStruct *) myMalloc (&temp, sizeof (struct EMPIReqStruct));
  temp->request = request;
  temp->reqRep = NULL;
  temp->reqRepBuf = NULL;
  temp->next = NULL;

  if (empiReqHead == NULL && empiReqTail == NULL)
  {
    empiReqHead = temp;
    empiReqTail = temp;
  }

  else
  {
    empiReqTail->next = temp;
    empiReqTail = temp;
  }

  return temp;
}

/* Function to find the corresponding object of type 'struct MPIReqStruct' for a given request handle if present in the linked list for tracking MPI_Request's */
struct EMPIReqStruct *findMR (EMPI_Request *request)
{
  struct EMPIReqStruct *temp = empiReqHead;

  while (temp != NULL)
  {
    if (temp->request == request)
      return temp;

    else
      temp = temp->next;
  }

  return NULL;
}

/* Function to delete the object corresponding to the given request handle from the linked list for tracking MPI_Request's */
void edeleteMR (EMPI_Request *request)
{
  struct EMPIReqStruct *temp = empiReqHead;

  if (temp->request == request)
  {
    empiReqHead == temp->next;
    myFree (temp);
    return;
  }

  while (temp->next != NULL)
  {
     if (temp->next->request == request)
     {
       struct EMPIReqStruct *toBeDeleted = temp->next;
       temp->next = temp->next->next;

       if (empiReqTail == toBeDeleted)
         empiReqTail = temp;

       myFree (toBeDeleted);
       break;
     }

     temp = temp->next;
  }
}

/* Function to count the number of pending MPI_Request's currently in the system */
int ecountMR ()
{
  int count = 0;
  struct EMPIReqStruct *temp = empiReqHead;

  while (temp != NULL)
  {
    count ++;
    temp = temp->next;
  }

  return count;
}

/* FUNCTIONS TO KEEP TRACK OF MPI_REQUESTs - END */

/* FUNCTIONS TO KEEP TRACK OF HEAP DATA STRCTURES - BEGIN */

/* Below set of functions are used to keep track of heap data structures in the sytem using a linked list of objects of a data structure 'struct heapListStruct' (hereafter referred to as 'heap list') */

/* Function to modify an entry in the heap list */
int updateHL (void *ptr, void *ptrAddr, size_t size)
{
  struct heapListStruct *temp = heapListHead;

  while (temp != NULL)
  {
    if (temp->ptrAddr == ptrAddr)
    {
      temp->ptr = ptr;
      temp->size = size;
      return 0;
    }

    else
      temp = temp->next;
  }

  return -1;
}

/* Function to add a new entry to the heap list when a new heap allocation is made */
void appendHL (void *ptr, void *ptrAddr, size_t size)
{
  //void *(*libc_malloc) (size_t) = dlsym (RTLD_NEXT, "malloc");

  //struct heapListStruct *temp = (struct heapListStruct *) libc_malloc (sizeof (struct heapListStruct));
  struct heapListStruct *temp = (struct heapListStruct *) malloc (sizeof (struct heapListStruct));

  temp->ptrAddr = ptrAddr;
  temp->ptr = ptr;
  temp->size = size;
  temp->next = NULL;

  if (heapListHead == NULL && heapListTail == NULL)
  {
    heapListHead = temp;
    heapListTail = temp;
  }

  else
  {
    heapListTail->next = temp;
    heapListTail = temp;
  }
}

/* Function to delete an entry from the heap list */
void deleteHL (void *ptr)
{
  //void (*libc_free) (void*) = dlsym (RTLD_NEXT, "free");

  struct heapListStruct *temp = heapListHead;

  if (temp->ptr == ptr)
  {
    heapListHead == temp->next;

    //libc_free (temp);
    free (temp);

    return;
  }

  while (temp->next != NULL)
  {
     if (temp->next->ptr == ptr)
     {
       struct heapListStruct *toBeDeleted = temp->next;
       temp->next = temp->next->next;

       if (heapListTail == toBeDeleted)
         heapListTail = temp;

       //libc_free (toBeDeleted);
       free (toBeDeleted);

       break;
     }

     temp = temp->next;
  }
}

/* Function to count the number of elements in the heap list */
int countHL ()
{
  int count = 0;
  struct heapListStruct *temp = heapListHead;

  while (temp != NULL)
  {
    count ++;
    temp = temp->next;
  }

  return count;
}

/* Function to display the contents of the heap list (for debugging purposes) */
void displayHL ()
{
  struct heapListStruct *temp = heapListHead;

  while (temp != NULL)
  {
    printf ("HL %p\t%p\t%ld\n", temp->ptrAddr, temp->ptr, temp->size);
    temp = temp->next;
  }
}

/* Wrapper function for malloc function, to keep track of heap allocations using heap list */
void *myMalloc (void *pAddr, size_t size)
{
  //void *(*libc_malloc) (size_t) = dlsym (RTLD_NEXT, "malloc");
  //void *p = libc_malloc (size);

  void *p = (void *) malloc (size);

  if (updateHL (p, pAddr, size) != 0)
    appendHL (p, pAddr, size);

  return p;
}

/* Wrapper function for free function, to keep track of heap allocations using heap list */
void myFree (void *p)
{
  //void (*libc_free) (void *) = dlsym (RTLD_NEXT, "free");

  deleteHL (p);

  //libc_free (p);
  free (p);
}

/* FUNCTIONS TO KEEP TRACK OF HEAP DATA STRCTURES - END */

/* Function that returns the index of Stack Pointer in the jmp_buf data structure.
IMPORTANT: This index is system dependent */

int JmpBufSP_Index()
{
  //#if defined(__x86_64__)
    return 0;
  //#endif
}


void SetSPIntoJmpBuf (jmp_buf tempEnv, unsigned long addr)
{
  PTR_ENCRYPT(addr);
  asm ("mov %2, %%rdx\n"
       "mov %1, 0x30(%%rdx)\n"
       "mov %%rdx, %0\n"
          : "=r" (tempEnv)
          : "r" (addr), "r" (tempEnv));
}

void SetFPIntoJmpBuf (jmp_buf tempEnv, unsigned long addr)
{
  PTR_ENCRYPT(addr);
  asm ("mov %2, %%rdx\n"
       "mov %1, 0x8(%%rdx)\n"
       "mov %%rdx, %0\n"
          : "=r" (tempEnv)
          : "r" (addr), "r" (tempEnv));
}

void SetPCIntoJmpBuf (jmp_buf tempEnv, unsigned long addr)
{
  PTR_ENCRYPT(addr);
  asm ("mov %2, %%rdx\n"
       "mov %1, 0x38(%%rdx)\n"
       "mov %%rdx, %0\n"
          : "=r" (tempEnv)
          : "r" (addr), "r" (tempEnv));
}

void SetRBXIntoJmpBuf (jmp_buf tempEnv, unsigned long addr)
{
  asm ("mov %2, %%rdx\n"
       "mov %1, 0x0(%%rdx)\n"
       "mov %%rdx, %0\n"
          : "=r" (tempEnv)
          : "r" (addr), "r" (tempEnv));
}

void SetR12IntoJmpBuf (jmp_buf tempEnv, unsigned long addr)
{
  asm ("mov %2, %%rdx\n"
       "mov %1, 0x10(%%rdx)\n"
       "mov %%rdx, %0\n"
          : "=r" (tempEnv)
          : "r" (addr), "r" (tempEnv));
}


void SetR13IntoJmpBuf (jmp_buf tempEnv, unsigned long addr)
{
  asm ("mov %2, %%rdx\n"
       "mov %1, 0x18(%%rdx)\n"
       "mov %%rdx, %0\n"
          : "=r" (tempEnv)
          : "r" (addr), "r" (tempEnv));
}

void SetR14IntoJmpBuf (jmp_buf tempEnv, unsigned long addr)
{
  asm ("mov %2, %%rdx\n"
       "mov %1, 0x20(%%rdx)\n"
       "mov %%rdx, %0\n"
          : "=r" (tempEnv)
          : "r" (addr), "r" (tempEnv));
}


void SetR15IntoJmpBuf (jmp_buf tempEnv, unsigned long addr)
{
  asm ("mov %2, %%rdx\n"
       "mov %1, 0x28(%%rdx)\n"
       "mov %%rdx, %0\n"
          : "=r" (tempEnv)
          : "r" (addr), "r" (tempEnv));
}

unsigned long RBXFromJmpBuf (jmp_buf tempEnv)
{
  unsigned long addr;

  asm ("mov 0x0(%1), %0"
          : "=r" (addr)
          : "r" (tempEnv));

  return addr;
}

unsigned long R12FromJmpBuf (jmp_buf tempEnv)
{
  unsigned long addr;

  asm ("mov 0x10(%1), %0"
          : "=r" (addr)
          : "r" (tempEnv));

  return addr;
}

unsigned long R13FromJmpBuf (jmp_buf tempEnv)
{
  unsigned long addr;

  asm ("mov 0x18(%1), %0"
          : "=r" (addr)
          : "r" (tempEnv));

  return addr;
}

unsigned long R14FromJmpBuf (jmp_buf tempEnv)
{
  unsigned long addr;

  asm ("mov 0x20(%1), %0"
          : "=r" (addr)
          : "r" (tempEnv));

  return addr;
}

unsigned long R15FromJmpBuf (jmp_buf tempEnv)
{
  unsigned long addr;

  asm ("mov 0x28(%1), %0"
          : "=r" (addr)
          : "r" (tempEnv));

  return addr;
}

/* Function to extract the address of the Stack Pointer from jmp_buf */
unsigned long SPFromJmpBuf (jmp_buf tempEnv)
{
  unsigned long addr;

  asm ("mov 0x30(%1), %0"
          : "=r" (addr)
          : "r" (tempEnv));

  //addr = (long) JMP_BUF_SP(tempEnv);

  /* Whether the below decrypt step (PTR_DECRYPT) is required depends on the implementation of jmp_buf, i.e whether the stack pointer is encrypted before storing it in jmp_buf or not. It may or may not be required. Also, the decrypt implemation is system dependent (refer to mpiProfiler.h for a sample implementation for this) */
  PTR_DECRYPT(addr);

  return addr;
}
unsigned long FPFromJmpBuf (jmp_buf tempEnv)
{
  unsigned long addr;

  asm ("mov 0x8(%1), %0"
          : "=r" (addr)
          : "r" (tempEnv));

  //addr = (long) JMP_BUF_SP(tempEnv);

  /* Whether the below decrypt step (PTR_DECRYPT) is required depends on the implementation of jmp_buf, i.e whether the stack pointer is encrypted before storing it in jmp_buf or not. It may or may not be required. Also, the decrypt implemation is system dependent (refer to mpiProfiler.h for a sample implementation for this) */
  PTR_DECRYPT(addr);

  return addr;
}
unsigned long PCFromJmpBuf (jmp_buf tempEnv)
{
  unsigned long addr;

  asm ("mov 0x38(%1), %0"
          : "=r" (addr)
          : "r" (tempEnv));

  //addr = (long) JMP_BUF_SP(tempEnv);

  /* Whether the below decrypt step (PTR_DECRYPT) is required depends on the implementation of jmp_buf, i.e whether the stack pointer is encrypted before storing it in jmp_buf or not. It may or may not be required. Also, the decrypt implemation is system dependent (refer to mpiProfiler.h for a sample implementation for this) */
  PTR_DECRYPT(addr);

  return addr;
}

/* Function to extract the address of the Stack Pointer from jmp_buf */
unsigned long SPFromJmpBufUsingKey (jmp_buf tempEnv, RAW_ADDR key)
{
  unsigned long addr;

  asm ("mov 0x30(%1), %0"
          : "=r" (addr)
          : "r" (tempEnv));

  //addr = (long) JMP_BUF_SP(tempEnv);

  /* Whether the below decrypt step (PTR_DECRYPT) is required depends on the implementation of jmp_buf, i.e whether the stack pointer is encrypted before storing it in jmp_buf or not. It may or may not be required. Also, the decrypt implemation is system dependent (refer to mpiProfiler.h for a sample implementation for this) */
  PTR_DECRYPT_USING_KEY(addr,key);

  return addr;
}
unsigned long FPFromJmpBufUsingKey (jmp_buf tempEnv, RAW_ADDR key)
{
  unsigned long addr;

  asm ("mov 0x8(%1), %0"
          : "=r" (addr)
          : "r" (tempEnv));

  //addr = (long) JMP_BUF_SP(tempEnv);

  /* Whether the below decrypt step (PTR_DECRYPT) is required depends on the implementation of jmp_buf, i.e whether the stack pointer is encrypted before storing it in jmp_buf or not. It may or may not be required. Also, the decrypt implemation is system dependent (refer to mpiProfiler.h for a sample implementation for this) */
  PTR_DECRYPT_USING_KEY(addr,key);

  return addr;
}
unsigned long PCFromJmpBufUsingKey (jmp_buf tempEnv, RAW_ADDR key)
{
  unsigned long addr;

  asm ("mov 0x38(%1), %0"
          : "=r" (addr)
          : "r" (tempEnv));

  //addr = (long) JMP_BUF_SP(tempEnv);

  /* Whether the below decrypt step (PTR_DECRYPT) is required depends on the implementation of jmp_buf, i.e whether the stack pointer is encrypted before storing it in jmp_buf or not. It may or may not be required. Also, the decrypt implemation is system dependent (refer to mpiProfiler.h for a sample implementation for this) */
  PTR_DECRYPT_USING_KEY(addr,key);

  return addr;
}

/* Function to replace part of the memory (size 'len'), staring from bottomAddr */

int replace (RAW_ADDR bottomAddr, long len, int tag, EMPI_Comm comm)
{
  int rank = repRank, retVal;
  EMPI_Status status;
  
  retVal = EMPI_Recv ((void *) bottomAddr, len, EMPI_BYTE, rank, tag, comm, &status);
  repRank = rank;

  if (retVal == EMPI_SUCCESS)
    return 0;

  else
    return -1;
}

/* Function to replace the stack segment */
void restoreStack ()
{
  RAW_ADDR stackTop = (stackPosInit & PAGE_MASK) + PAGE_SIZE - 1;
  //RAW_ADDR stackTop = stackPosInit;
  //long offset = stackPosInit - stackPosInitRecv;
  long offset = stackTop - stackSegAddr[1];

  /*if(mprotect((void *)(stackTop - (stackSegAddr [1] - stackSegAddr [0])), stackSegAddr [1] - stackSegAddr [0] + 1, PROT_EXEC | PROT_READ | PROT_WRITE) == -1)
  {
    printf("mprotect failed\n");
  }*/

  printf("Receiving Stack Segment from %p to %p of size %d %p\n",stackTop - (stackSegAddr [1] - stackSegAddr [0]), stackTop,stackSegAddr [1] - stackSegAddr [0] + 1, stackPosInit);

  if (replace (stackTop - (stackSegAddr [1] - stackSegAddr [0]), stackSegAddr [1] - stackSegAddr [0] + 1, 61, MPI_COMM_WORLD->eworldComm) < 0)
  {
    printf ("Replacing STACK Segment Unsuccessful!\n");
    EMPI_Abort (eworldComm, 1);
  }

  printf ("Replacing STACK Segment Successful!\n");

  printf ("ENV BEFORE: Stack Pointer %p, Frame Pointer %p, PC %p\n", SPFromJmpBufUsingKey(env,key), FPFromJmpBufUsingKey(env,key), PCFromJmpBufUsingKey(env,key));



  RAW_ADDR SP = SPFromJmpBufUsingKey(env,key);
  RAW_ADDR FP = FPFromJmpBufUsingKey(env,key);
  RAW_ADDR PC = PCFromJmpBufUsingKey(env,key);

  SP = SP + offset;
  FP = FP + offset;
  if(libUsed && (PC >= replibLimit[0]) && (PC <= replibLimit[1])) PC = PC + libOffset;
	else PC = PC + dataOffset;

  SetSPIntoJmpBuf(env,SP);
  SetFPIntoJmpBuf(env,FP);
  SetPCIntoJmpBuf(env,PC);

  RAW_ADDR rbx = RBXFromJmpBuf(env);
  RAW_ADDR r12 = R12FromJmpBuf(env);
  RAW_ADDR r13 = R13FromJmpBuf(env);

  RAW_ADDR r14 = R14FromJmpBuf(env);
  RAW_ADDR r15 = R15FromJmpBuf(env);


  //rbx = rbx + dataOffset;
  r12 = r12 + offset;
  r13 = r13 + dataOffset;
  
  //SetRBXIntoJmpBuf(env,rbx);
  SetR12IntoJmpBuf(env,r12);
  SetR13IntoJmpBuf(env,r13);
  
  //SetRBXIntoJmpBuf(env,regs[0]);
  //SetR12IntoJmpBuf(env,r12+offset);
  //SetR13IntoJmpBuf(env,regs[2]);
  //SetR14IntoJmpBuf(env,regs[3]);
  //SetR15IntoJmpBuf(env,regs[4]);

  //printf("OTHER THINGS %p, %p\n", regs[1], *((long *)regs[1]));

  printf ("ENV AFTER: Stack Pointer %p, Frame Pointer %p, PC %p\n", SPFromJmpBuf(env), FPFromJmpBuf(env), PCFromJmpBuf(env));

  //SPFromJmpBuf(env) = SPFromJmpBuf(env) + offset;

  //printf ("Recv stack pointer after: %p %p %p %p %p %p %p\n", JMP_BUF_SP(env), JMP_BUF_SP(env) + offset, (env)[0].__jmpbuf[0],(((long*)(env))[0]),(((long*)(env))[1]),(((long*)(env))[2]),(((long*)(env))[3]));

  //*((long*) SP) = *((long*) SP) + offset;
  //*((long*) FP) = *((long*) FP) + offset;

  /*RAW_ADDR a = FP;
  while ((*((long *)a) < stackTop)&&(*((long *)a) != NULL))
  {
    printf("BEFORE %p %p\n",*((long *)a),*((long *)(a+8)));
    *((long *)a) = *((long *)a) + offset;
    *((long *)(a+8)) = *((long *)(a+8)) + dataOffset;
    a = *((long *)a);
    printf("%d AFTER %p %p\n",*((long *)a),*((long *)(a+8)));
  }

  if((*((long *)a) > stackTop) && (*((long *)a) != NULL))
  {
    *((long *)a) = *((long *)a) + offset;
  }*/

  for (RAW_ADDR start = SP;  start < stackTop; start = start +8)
  {
    if(((*((long *)start) <= dataSegAddr[2]) && (*((long *)start) >= textstart)))
    {
      *((long *)start) = *((long *)start) + dataOffset;
    }
    else if(((*((long *)start) <= stackSegAddr[1]) && (*((long *)start) >= stackSegAddr[0])))
    {
      *((long *)start) = *((long *)start) + offset;
    }
		else if(((*((long *)start) <= replibLimit[1]) && (*((long *)start) >= replibLimit[0])))
		{
			*((long *)start) = *((long *)start) + libOffset;
		}
    //printf("%p, %p:   %p\n",start, start - offset, *((long *)start));
  }
  //printf("RECV REG %p, %p\n", r12+offset, *((long *)(r12+offset)));
  printf("regs %p %p %p\n",r13-dataOffset,r13,textstart);
  longjmp (env, 1);
}

/* Return the address of the temporary stack to be used while replacing the real stack segment */
static long *getOverrunPos()
{
  return (long*) tmpStack;
}

/* Function to move the execution to the temporary stack before replacing the real stack segment */
void executeOnTmpStk (void (*func)())
{

  jmp_buf tempEnv;
  savedFunc = func;
  unsigned long addr;

  *(getOverrunPos()) = overRunFlag;

  if (setjmp (tempEnv) == 0 )
  {
    addr = (long)(tmpStack + tmpStackSize - 2);

  /* Whether the below encrypt step (PTR_ENCRYPT) is required depends on the implementation of jmp_buf, i.e whether the stack pointer is encrypted before storing it in jmp_buf or not. It may or may not be required. Also, the encrypt implemation is system dependent (refer to mpiProfiler.h for a sample implementation for this) */
    //PTR_ENCRYPT(addr);
    printf ("TempEnv Before: Stack Pointer %p, Frame Pointer %p, PC %p\n", SPFromJmpBuf(tempEnv), FPFromJmpBuf(tempEnv), PCFromJmpBuf(tempEnv));
    SetSPIntoJmpBuf(tempEnv,addr);
    printf ("TempEnv After: Stack Pointer %p, Frame Pointer %p, PC %p\n", SPFromJmpBuf(tempEnv), FPFromJmpBuf(tempEnv), PCFromJmpBuf(tempEnv));
    //printf("TempEnv regs: %p %p %p %x %x %p %d %p\n",RBXFromJmpBuf(tempEnv),R12FromJmpBuf(tempEnv),R13FromJmpBuf(tempEnv),R14FromJmpBuf(tempEnv),R15FromJmpBuf(tempEnv),tmpStack,tmpStackSize,savedFunc);

    longjmp (tempEnv, 1);
  }

  else
    {
    /*if (setjmp (tempEnv) == 0 )
    {
      printf("Current stack pointer : %p\n",JMP_BUF_SP(tempEnv));
      longjmp (tempEnv, 1);
    }
    else*/
      savedFunc();
    }
  if (*(getOverrunPos()) != overRunFlag)
    printf ("Stack overrun in executeOnTmpStack\n");
}

/* Replace Process */

void replaceImage (int srcRank, RAW_ADDR dataBottomAddr, RAW_ADDR dataTopAddr, RAW_ADDR dataEndAddr, RAW_ADDR stackBottomAddr, RAW_ADDR stackTopAddr/*, RAW_ADDR rbx, RAW_ADDR r12, RAW_ADDR r13, RAW_ADDR r14, RAW_ADDR r15*/)
{
  long dataLen;
  int start, x;
  EMPI_Comm eworldComm_dup;
  EMPI_Comm EMPI_COMM_CMP_DUP, EMPI_COMM_REP_DUP, EMPI_CMP_REP_INTERCOMM_DUP, EMPI_CMP_NO_REP_DUP, EMPI_CMP_NO_REP_INTERCOMM_DUP;
	struct mpi_ft_comm mpi_ft_comm_world_dup;
  char *workDirDup,*repStateDup;
  workDirDup = workDir;
  repStateDup = repState;
  void *extLib_dup, *openLib_dup;
  pid_t parentpid_dup;

  repRank = srcRank;

  /* Replace DATA */
  eworldComm_dup = MPI_COMM_WORLD->eworldComm;
  EMPI_COMM_CMP_DUP = MPI_COMM_WORLD->EMPI_COMM_CMP;
  EMPI_COMM_REP_DUP = MPI_COMM_WORLD->EMPI_COMM_REP;
  EMPI_CMP_REP_INTERCOMM_DUP = MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM;
	EMPI_CMP_NO_REP_DUP = MPI_COMM_WORLD->EMPI_CMP_NO_REP;
	EMPI_CMP_NO_REP_INTERCOMM_DUP = MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM;
	
  extLib_dup = extLib;
  openLib_dup = openLib;
  parentpid_dup = parentpid;
  //printf ("%p\t%p\t%p\n", &CMP_REP_INTERCOMM, &CMP_REP_INTERCOMM_DUP, CMP_REP_INTERCOMM);

  struct heapListStruct *heapListHeadBackup = heapListHead, *heapListTailBackup = heapListTail;

  dataLen = dataTopAddr - dataBottomAddr + 1;

  extern RAW_ADDR __data_start, _edata, _end, _etext;
  dataOffset = ((RAW_ADDR) &__data_start & PAGE_MASK) - dataBottomAddr;

  printf("Allocations: data start %p, edata %p, bss end %p\n",(RAW_ADDR) &__data_start & PAGE_MASK,((RAW_ADDR) &_edata & PAGE_MASK) + PAGE_SIZE - 1,((RAW_ADDR) &_end & PAGE_MASK) + PAGE_SIZE - 1);
  printf("function location %p %p %p %p %p %p\n",OMPI_Recv,malloc,&extLib,extLib,&eworldComm,((RAW_ADDR) &_etext & PAGE_MASK) + PAGE_SIZE - 1);

  long dataSegLength = ((RAW_ADDR) &_end & PAGE_MASK) + PAGE_SIZE - 1 - ((RAW_ADDR) &__data_start & PAGE_MASK) + 1;

  if (dataSegLength != dataEndAddr - dataBottomAddr + 1)
  {
    sbrk(dataEndAddr - dataBottomAddr + 1 - dataSegLength);
  }
  
  //for(start = 0; start < 128; start +=8) printf("receiver: %p %p %d %p %p\n",(void *)(dataBottomAddr + dataOffset + start),*((RAW_ADDR *)(dataBottomAddr + dataOffset + start)),start,heapLimit,stackLimit);
  
  int skips[dataLen];
  int k;
  EMPI_Recv (&k, 1, EMPI_INT, repRank, 1234, MPI_COMM_WORLD->eworldComm,EMPI_STATUS_IGNORE);
  EMPI_Recv (skips, k, EMPI_INT, repRank, 12345, MPI_COMM_WORLD->eworldComm,EMPI_STATUS_IGNORE);
  int skipidx = 0;
  for (start=0; start<dataLen; start+=8)
  {
    if (dataLen - start < 8)
      x = dataLen - start;

    else
      x = 8;
      
    if(skipidx < k)
    {
      if( start != skips[skipidx] ) 
      {
    	if (replace (dataBottomAddr + dataOffset + start, x, 51 + start, eworldComm_dup) < 0)
    	{
    	  printf ("Replacing DATA Segment Unsuccessful!\n");
    	  EMPI_Abort (eworldComm_dup, 1);
    	}
      }
      else
      {
    	skipidx++;
      }
    }
    else
    {
      if (replace (dataBottomAddr + dataOffset + start, x, 51 + start, eworldComm_dup) < 0)
      {
        printf ("Replacing DATA Segment Unsuccessful!\n");
      	EMPI_Abort (eworldComm_dup, 1);
      }
    }
    //else printf("receiver skipping %d\n",start);
  }

  workDir = workDirDup;
  repState = repStateDup;
  printf("String After: %p %p %p %p\n",&workDir,&repState,workDir,repState);
  
  /*for(int i = 0; i<k;i++)
  {
    *((RAW_ADDR *) map[i].addr) = map[i].val;
  }*/

  printf ("Replacing DATA Segment Successful!\n");

  MPI_COMM_WORLD->eworldComm = eworldComm_dup;
  MPI_COMM_WORLD->EMPI_COMM_CMP = EMPI_COMM_CMP_DUP;
  MPI_COMM_WORLD->EMPI_COMM_REP = EMPI_COMM_REP_DUP;
  MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM = EMPI_CMP_REP_INTERCOMM_DUP;
	MPI_COMM_WORLD->EMPI_CMP_NO_REP = EMPI_CMP_NO_REP_DUP;
	MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM = EMPI_CMP_NO_REP_INTERCOMM_DUP;
  extLib = extLib_dup;
  openLib = openLib_dup;
  parentpid = parentpid_dup;

  stackSegAddr [0] = stackBottomAddr;
  stackSegAddr [1] = stackTopAddr;
  
  //regs[0] = rbx;
  //regs[1] = r12;
  //regs[2] = r13;
  //regs[3] = r14;
  //regs[4] = r15;

  heapListHead = heapListHeadBackup;
  heapListTail = heapListTailBackup;

  /* Replace HEAP Data Structures */

  struct heapListStruct *heapData = heapListHead;
  struct heapListStruct *prevData = NULL;

  int i = 0;
  int heapListLen;
  EMPI_Recv (&heapListLen, 1, EMPI_INT, repRank, 52, eworldComm_dup, EMPI_STATUS_IGNORE);
  printf("Received original process Heap List Length %d, current size %d\n",heapListLen,countHL());
	
	void *ptrAddr;
  for (i = 0; i < heapListLen; i++)
  {
    printf("Loop: %d, %d\n",i,heapListLen);
    size_t size;
    EMPI_Recv (&size, 1, my_EMPI_SIZE_T, repRank, 52 + i, eworldComm_dup, EMPI_STATUS_IGNORE);
    //printf("Received Size %d, current size %d\n",size,heapData->size);
    EMPI_Recv (&ptrAddr, 8, EMPI_BYTE, repRank, 52 + i, eworldComm_dup, EMPI_STATUS_IGNORE);
    //printf("Received ptrAddr %p, current value %p\n",ptrAddr,heapData->ptrAddr);
    if (heapData == NULL)
    {
      void *p = (void *) malloc (size);
      if(libUsed && ((RAW_ADDR)ptrAddr >= replibLimit[0]) && ((RAW_ADDR)ptrAddr <= replibLimit[1])) appendHL (p, ptrAddr + libOffset, size);
			else appendHL (p, ptrAddr + dataOffset, size);
      heapData = heapListTail;
    }
    else if (size != heapData->size)
    {
      struct heapListStruct *nextData = heapData->next;

      free (heapData->ptr);
      free (prevData->next);

      void *ptr = (void *) malloc (size);
      struct heapListStruct *temp = (struct heapListStruct *) malloc (sizeof (struct heapListStruct));
      temp->ptr = ptr;
      if(libUsed && ((RAW_ADDR)ptrAddr >= replibLimit[0]) && ((RAW_ADDR)ptrAddr <= replibLimit[1])) temp->ptrAddr = ptrAddr + libOffset;
			else temp->ptrAddr = ptrAddr + dataOffset;
      temp->size = size;
      temp->next = nextData;
      prevData->next = temp;
      heapData = temp;
    }
    else
    {
      if(libUsed && ((RAW_ADDR)ptrAddr >= replibLimit[0]) && ((RAW_ADDR)ptrAddr <= replibLimit[1])) heapData->ptrAddr = ptrAddr + libOffset;
			else heapData->ptrAddr = ptrAddr + dataOffset;
    }
    //printf("Receiving Heap Segment at %p of size %d datapos is %p\n",heapData->ptr,heapData->size,heapData->ptrAddr);
    if (replace ((RAW_ADDR) heapData->ptr, heapData->size, 52 + i, eworldComm_dup) < 0)
    {
      printf ("Replacing HEAP Data Structure Unsuccessful!\n");
      EMPI_Abort (eworldComm_dup, 1);
    }

    else
    {
      if( (((RAW_ADDR)(heapData->ptrAddr)) <= heapLimit) ) memcpy ((void *)heapData->ptrAddr, (void *)&heapData->ptr, 8);
      prevData = heapData;
      heapData = heapData->next;
    }
  }
  printf ("Replacing HEAP Segment Successful!\n");

  /* Replace STACK */

  executeOnTmpStk (restoreStack);
}

/* Write Checkpoint */

int sendData (int destRank)
{
  int retVal, start, x;
  long dataLen, stackLen;
  //MPI_Request request;
  EMPI_Status status;

  /* Send DATA Segment */

  dataLen = dataSegAddr [1] - dataSegAddr [0] + 1;
  printf("String Send: %p %p %s %s\n",&workDir,&repState,workDir,repState);

  printf ("Send Len = %d %d\n", dataLen,destRank+nC);
  
  //for(start = 0; start < 128; start +=8) printf("sender: %p %p %d %p %p\n",(void *)(dataSegAddr[0] + start),*((RAW_ADDR *)(dataSegAddr[0] + start)),start,heapLimit,stackLimit);
  int skips[dataLen];
  int k=0;
  for(start = 0; start < dataLen; start+=8)
  {
    if( (((*((RAW_ADDR *)(dataSegAddr[0] + start))) > heapLimit) && ((*((RAW_ADDR *)(dataSegAddr[0] + start))) <= stackLimit)) )
    {
      skips[k] = start;
      k++;
    }
  }
  retVal = EMPI_Send (&k, 1, EMPI_INT, destRank + nC, 1234, MPI_COMM_WORLD->eworldComm);
  retVal = EMPI_Send (skips, k, EMPI_INT, destRank + nC, 12345, MPI_COMM_WORLD->eworldComm);
  int skipidx = 0;
  for (start = 0; start<dataLen; start+=8)
  {
    if (dataLen - start < 8)
      x = dataLen - start;

    else
      x = 8;
    if(skipidx < k)
    {
      if( start != skips[skipidx] ) 
      {
    	retVal = EMPI_Send ((void *) (dataSegAddr [0] + start), x, EMPI_BYTE, destRank + nC, 51+start, MPI_COMM_WORLD->eworldComm);
      }
      else
      {
    	skipidx++;
      }
    }
    else
    {
      retVal = EMPI_Send ((void *) (dataSegAddr [0] + start), x, EMPI_BYTE, destRank + nC, 51+start, MPI_COMM_WORLD->eworldComm);
    }
    //else printf("sender skipping %d\n",start);
  }

  if (retVal != EMPI_SUCCESS)
  {
    printf ("Sending DATA Segment Unsuccessful!\n");
    return -1;
  }

  printf ("Sending DATA Segment Successful!\n");

  //PMPI_Wait (&request, &status);

  /* Send HEAP Data Structures */

  struct heapListStruct *heapData = heapListHead;

  int i = countHL();

  retVal = EMPI_Send (&i, 1, EMPI_INT, destRank + nC, 52, MPI_COMM_WORLD->eworldComm);
  if (retVal != EMPI_SUCCESS)
  {
    printf ("Sending HEAP Data Structure Unsuccessful!\n");
    return -1;
  }

  i = 0;

  while (heapData != NULL)
  {
    retVal = EMPI_Send (&heapData->size, 1, my_EMPI_SIZE_T, destRank + nC, 52 + i, MPI_COMM_WORLD->eworldComm);
    retVal = EMPI_Send (&heapData->ptrAddr, 8, EMPI_BYTE, destRank + nC, 52 + i, MPI_COMM_WORLD->eworldComm);
    retVal = EMPI_Send (heapData->ptr, heapData->size, EMPI_BYTE, destRank + nC, 52 + i++, MPI_COMM_WORLD->eworldComm);

    if (retVal != EMPI_SUCCESS)
    {
      printf ("Sending HEAP Data Structure Unsuccessful!\n");
      return -1;
    }

    else
      heapData = heapData->next;
  }

  printf ("Sending HEAP Segment Successful!\n");

  /* Send STACK Segment */

  stackLen = stackSegAddr [1] - stackSegAddr [0] + 1;


  printf("Sending Stack %p %p, %d\n",stackSegAddr[0], stackSegAddr[1],stackLen);

  //if (mmap((void *)stackSegAddr[0], stackLen, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0) == (void *)-1)
  //{
  //  printf("stack mmap failed\n");
  //  MPI_Abort(MPI_COMM_WORLD,1);
  //}

  retVal = EMPI_Send ((void *) stackSegAddr [0], stackLen, EMPI_BYTE, destRank + nC, 61, MPI_COMM_WORLD->eworldComm);

  if (retVal != EMPI_SUCCESS)
  {
    printf ("Sending STACK Segment Unsuccessful!\n");
    return -1;
  }
  printf ("Sending STACK Segment Successful!\n");

  return 0;
}

/* Checkpoint Process */

void sendImage (int srcRank, int destRank)
{

  extern RAW_ADDR _end, __data_start, _edata, __libc_setlocale_lock;

  if (setjmp (env) == 0 )
  {
    RAW_ADDR rbx = RBXFromJmpBuf(env);
    RAW_ADDR r12 = R12FromJmpBuf(env);
    RAW_ADDR r13 = R13FromJmpBuf(env);
    RAW_ADDR r14 = R14FromJmpBuf(env);
    RAW_ADDR r15 = R15FromJmpBuf(env);
    //printf("Send Side Register vals: %p %p %p %x %x %p\n",rbx,r12,r13,r14,r15,*((long *)r12));
    //for(RAW_ADDR sp = SPFromJmpBuf(env); sp - SPFromJmpBuf(env) <= 40; sp=sp+8)
    //{
      //printf("Send side stack %p %p\n",sp,*((long *)sp));
    //}
    printf ("sendImage 1\n");

    dataSegAddr [0] = ((RAW_ADDR) &__data_start & PAGE_MASK);

    printf ("sendImage 2\n");

    dataSegAddr [1] = ((RAW_ADDR) &_edata & PAGE_MASK) + PAGE_SIZE - 1;
    dataSegAddr [2] = ((RAW_ADDR) &_end & PAGE_MASK) + PAGE_SIZE - 1;

    printf ("sendImage 3, %p %p %p\n",dataSegAddr[0],dataSegAddr[1],dataSegAddr[2]);

    //stackSegAddr [0] = SPFromJmpBuf (env) & PAGE_MASK;
    stackSegAddr [0] = SPFromJmpBuf (env);

    //printf ("stackSegAddr : %p\n", (void *) stackSegAddr [0]);

    printf ("sendImage 4\n");

    stackSegAddr [1] = (stackPosInit & PAGE_MASK) + PAGE_SIZE - 1;
    //stackSegAddr [1] = stackPosInit;

    printf ("sendImage 5 %p %p\n", stackSegAddr[0], stackSegAddr[1]);

    EMPI_Send (dataSegAddr, 3, EMPI_UNSIGNED_LONG, destRank + nC, 37, MPI_COMM_WORLD->eworldComm);
    EMPI_Send (stackSegAddr, 2, EMPI_UNSIGNED_LONG, destRank + nC, 38, MPI_COMM_WORLD->eworldComm);
    EMPI_Send (env, sizeof (jmp_buf), EMPI_BYTE, destRank + nC, 39, MPI_COMM_WORLD->eworldComm);
    asm ("mov %%fs:48, %0"
          : "=r" (key));
    EMPI_Send (&key, 1, EMPI_UNSIGNED_LONG, destRank + nC, 40, MPI_COMM_WORLD->eworldComm);

    //textstart = dataSegAddr [0] & 0xfffffff00000;
    EMPI_Send (&textstart, 1, EMPI_UNSIGNED_LONG, destRank + nC, 41, MPI_COMM_WORLD->eworldComm);
		if(libUsed) EMPI_Send (libLimit, 2, EMPI_UNSIGNED_LONG, destRank + nC, 42, MPI_COMM_WORLD->eworldComm);

    printf ("Send env: Stack Pointer %p, Frame Pointer %p, PC %p\n", SPFromJmpBuf(env), FPFromJmpBuf(env), PCFromJmpBuf(env));

    printf ("ENV Size : %d\n", sizeof (jmp_buf));

    if (sendData (destRank) < 0)
    {
      printf ("RANK: %d. Sending Data Unsuccessful!\n", srcRank);
      EMPI_Abort (eworldComm, 1);
    }

    longjmp (env, 1);

    printf ("You are not supposed to see this send!\n");
  }

  else
  {
    int rank;
    EMPI_Comm_rank (MPI_COMM_WORLD->eworldComm, &rank);
    printf ("%d Checkpoint/Restart Successful!\n",rank);
  }
}

/* Send Image */

void recvImage (int srcRank)
{
  //extern RAW_ADDR _end, __data_start, _edata, __libc_setlocale_lock;
  /*jmp_buf tempEnv;
  if (setjmp (tempEnv) == 0)
  {
    regs[0] = RBXFromJmpBuf(tempEnv);
    regs[1] = R12FromJmpBuf(tempEnv);
    regs[2] = R13FromJmpBuf(tempEnv);
    regs[3] = R14FromJmpBuf(tempEnv);
    regs[4] = R15FromJmpBuf(tempEnv);
    printf("Recv Side Register vals: %p %p %p %x %x %p\n",regs[0],regs[1],regs[2],regs[3],regs[4],*((long *)regs[1]));
    for(RAW_ADDR sp = SPFromJmpBuf(tempEnv); sp - SPFromJmpBuf(tempEnv) <= 40; sp=sp+8)
    {
      printf("Recv side stack %p %p\n",sp,*((long *)sp));
    }
    longjmp (tempEnv, 1);
  }*/
  EMPI_Status status;

  EMPI_Recv (dataSegAddr, 3, EMPI_UNSIGNED_LONG, srcRank, 37, MPI_COMM_WORLD->eworldComm, &status);
  EMPI_Recv (stackSegAddr, 2, EMPI_UNSIGNED_LONG, srcRank, 38, MPI_COMM_WORLD->eworldComm, &status);

  EMPI_Recv (env, sizeof (jmp_buf), EMPI_BYTE, srcRank, 39, MPI_COMM_WORLD->eworldComm, &status);

  EMPI_Recv (&key, 1, EMPI_UNSIGNED_LONG, srcRank, 40, MPI_COMM_WORLD->eworldComm, &status);
  EMPI_Recv (&textstart, 1, EMPI_UNSIGNED_LONG, srcRank, 41, MPI_COMM_WORLD->eworldComm, &status);
	if(libUsed) EMPI_Recv (replibLimit, 2, EMPI_UNSIGNED_LONG, srcRank, 42,MPI_COMM_WORLD->eworldComm, &status);
	
	if(libUsed)
	{
		libOffset = libLimit[0] - replibLimit[0];
	}

  printf ("Recv env: Stack Pointer %p, Frame Pointer %p, PC %p\n", SPFromJmpBufUsingKey(env,key), FPFromJmpBufUsingKey(env,key), PCFromJmpBufUsingKey(env,key));
  replaceImage (srcRank, dataSegAddr [0], dataSegAddr [1], dataSegAddr [2], stackSegAddr [0], stackSegAddr [1]/*, regs[0], regs[1], regs[2], regs[3], regs[4]*/);

  printf ("You are not supposed to see this recv!\n");
}

void repManager ()
{

  /*FILE *p = fopen ("/proc/self/maps", "r");
  char *line = (char *) malloc (1000 * sizeof (char));

  line = fgets (line, 1000, p);
  while (line != NULL)
  {
    printf ("%s", line);
    line = fgets (line, 1000, p);
  }*/
  int i, c, r, localRank, destRank = -1, srcRank = -1;
  char fileName [100];
  sprintf (fileName, "%s/%s", workDir, repState);
  FILE *fp = fopen (fileName, "r");
  if (MPI_COMM_WORLD->EMPI_COMM_REP == EMPI_COMM_NULL)
    EMPI_Comm_rank (MPI_COMM_WORLD->EMPI_COMM_CMP, &localRank);

  else
    EMPI_Comm_rank (MPI_COMM_WORLD->EMPI_COMM_REP, &localRank);
  for (i=0; i<nC; i++)
  {
    if (i == nC - 1)
      fscanf (fp, "%d\t%d", &c, &r);

    else
      fscanf (fp, "%d\t%d\n", &c, &r);

    if (r != -1)
    {
      if (MPI_COMM_WORLD->EMPI_COMM_REP == EMPI_COMM_NULL)
      {
        if (localRank == c && cmpToRepMap [localRank] != r)
          destRank = r;
      }

      else
      {
        if (localRank == r && cmpToRepMap [c] != localRank)
          srcRank = c;
      }
      repToCmpMap [r] = c;
    }

    cmpToRepMap [c] = r;
  }

  fclose (fp);

  //double start_time = MPI_Wtime();
  if (destRank != -1)
  {
    printf ("Sending image to rank %d BEGIN\n", destRank);
    sendImage (localRank, destRank);
    printf ("Sending image to rank %d END\n", destRank);
  }

  else if (srcRank != -1)
  {
    printf ("Receiving image from rank %d BEGIN\n", srcRank);
    recvImage (srcRank);
    printf ("Receiving image from rank %d END\n", srcRank);
  }
  EMPI_Barrier (MPI_COMM_WORLD->eworldComm);

  /*if (localRank == 0 && MPI_COMM_REP == MPI_COMM_NULL)
  {
    double end_time = MPI_Wtime();
    printf ("Replica Change Time (s) = %f\n", end_time - start_time);
    MPI_Abort (MPI_COMM_WORLD, 1);
  }*/
}

int MPI_Finalize(void)
{
	MPI_Barrier(MPI_COMM_WORLD);
	OMPI_Finalize();
	//EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
	mpi_ft_free_older_collectives(last_collective);
	last_collective = NULL;
  //dlclose(extLib);
  //OMPI_Barrier(oworldComm);
  //OMPI_Finalize();
  //dlclose(openLib);
  fclose(logfile);
  printf("after finalize returning\n");
	kill(getpid(),SIGKILL);
  return 0;
}

int iof_setup(int p_stdin,int p_stdout,int p_stderr)
{
  int ret;
  //printf("Iof setup\n");
  //printf("Closed stdouterr\n");
  //printf("disable echo\n");
  struct termios term_attrs;
  if (tcgetattr(p_stdout, &term_attrs) < 0) {
      printf("pipe setup failure\n");
      return -1;
  }
  //printf("Setting flags\n");
  term_attrs.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | ECHONL);
  term_attrs.c_iflag &= ~(ICRNL | INLCR | ISTRIP | INPCK | IXON);
  term_attrs.c_oflag &= ~(
#ifdef OCRNL
                          OCRNL |
#endif
                          ONLCR);
  //printf("tcsetattr\n");
  if (tcsetattr(p_stdout, TCSANOW, &term_attrs) == -1) {
      printf("pipe setup failure 1\n");
      return -1;
  }
  printf("dup2 %d %d %d %d\n",p_stdout,fileno(stdout),fcntl(p_stdout,F_GETFD),fcntl(fileno(stdout),F_GETFD));
  if(p_stdout != fileno(stdout)) {
    ret = dup2(p_stdout, fileno(stdout));
    //printf("dup2 returned\n");
    //printf("dup2 returned %d\n",ret);
    if (ret < 0) {
      printf("pipe setup failure 2\n");
      return -1;
    }
  //printf("closing stdout\n");
    close(p_stdout);
  }
  printf("connecting stdin\n");
  if (p_stdin > 0) {
    if (p_stdin != fileno(stdin)) {
      ret = dup2(p_stdin, fileno(stdin));
      if (ret < 0) {
          printf("stdin dup2 failed\n");
          return -1;
      }
      close(p_stdin);
    }
  } else {
    int fd;
    /* connect input to /dev/null */
    fd = open("/dev/null", O_RDONLY, 0);
    if (fd != fileno(stdin)) {
      ret = dup2(fd, fileno(stdin));
      if (ret < 0) {
          printf("stdin dup2 to null failed\n");
          return -1;
      }
    }
    close(fd);
	  }
  printf("check stderr to filno\n");
  printf("stderr check test %d %p %p %p\n",p_stderr,&p_stderr,stderr,&stderr);

  if (p_stderr != fileno(stderr)) {
    printf("calling stderr dup2 %d %d\n",p_stderr,fileno(stderr));
    ret = dup2(p_stderr, fileno(stderr));
    if (ret < 0) {
      printf("stderr dup2 failed\n");
      return -1;
    }
    close(p_stderr);
  }
  printf("end of iof setup\n");

  return 0;
}

void set_handler_local(int sig)
{
    struct sigaction act;

    act.sa_handler = SIG_DFL;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    sigaction(sig, &act, (struct sigaction *) 0);
}

int childconnect(int pipefd, int pin, int pout, int perr, int pidfd, int spid, int conn)
{
  printf("childconnect start\n");
  int i;
  sigset_t sigs;
  char dir[1024];

  setpgid(0, 0);

  //printf("ptraces access? %d\n",__ptrace_may_access(get_pid_task(pidfd, PIDTYPE_PID),PTRACE_MODE_ATTACH_REALCREDS))
  //ptrace(PTRACE_MODE_ATTACH_REALCREDS, (pid_t)spid,NULL, NULL);
  int p_stdin=-2,p_stdout=-2,p_stderr=-2,write_fd=-2;
  /*if(pidfd > 0)
  {
    write_fd = syscall(SYS_pidfd_getfd, pidfd, pipefd, 0);
    if(write_fd == -1)
    {
      printf("get write_fd error\n");
      return -1;
    }
    p_stdout = syscall(SYS_pidfd_getfd, pidfd, pout, 0);
    if(p_stdout == -1)
    {
      printf("get stdout error\n");
      return -1;
    }
    p_stderr = syscall(SYS_pidfd_getfd, pidfd, perr, 0);
    if(p_stderr == -1)
    {
      printf("get stderr error\n");
      return -1;
    }
    if(pin > 0)
    {
      p_stdin = syscall(SYS_pidfd_getfd, pidfd, pin, 0);
      if(p_stdin == -1)
      {
        printf("get stdin error\n");
        return -1;
      }
    }
    else
    {
      int fd;
      fd = open("/dev/null", O_RDONLY, 0);
      if (fd != fileno(stdin)) {
        dup2(fd, fileno(stdin));
      }
      close(fd);
    }
  }
  else */if(conn > 0)
  {
    struct msghdr mhdr;
    struct iovec iov;
    union
    {
      struct cmsghdr cmhdr;
      char cntrl[CMSG_SPACE(sizeof(int))];
    } cntrl_unix;
    struct cmsghdr *cmhdr;
    
    char tmp;
    iov.iov_base = &tmp;
    iov.iov_len = sizeof(char);
    
    mhdr.msg_name = NULL;
    mhdr.msg_namelen = 0;
    mhdr.msg_iov = &iov;
    mhdr.msg_iovlen = 1;
    mhdr.msg_control = cntrl_unix.cntrl;
    mhdr.msg_controllen = sizeof(cntrl_unix.cntrl);
    
    int size = recvmsg(conn,&mhdr,0);
    if(size == -1)
    {
      printf("Recvmsg failed\n");
      return -1;
    }
    cmhdr = CMSG_FIRSTHDR(&mhdr);
    if(!cmhdr)
    {
      printf("Expected one Recvmsg header with FD, Got zero Headers!\n");
      return -1;
    }
    if(cmhdr->cmsg_level != SOL_SOCKET)
    {
      printf("Invalid cmsg_level %d\n",cmhdr->cmsg_level);
      return -1;
    }
    if(cmhdr->cmsg_type != SCM_RIGHTS)
    {
      printf("Invalid cmsg_type %d\n",cmhdr->cmsg_type);
      return -1;
    }
    write_fd = *((int *) CMSG_DATA(cmhdr));
    
    size = recvmsg(conn,&mhdr,0);
    if(size == -1)
    {
      printf("Recvmsg failed\n");
      return -1;
    }
    cmhdr = CMSG_FIRSTHDR(&mhdr);
    if(!cmhdr)
    {
      printf("Expected one Recvmsg header with FD, Got zero Headers!\n");
      return -1;
    }
    if(cmhdr->cmsg_level != SOL_SOCKET)
    {
      printf("Invalid cmsg_level %d\n",cmhdr->cmsg_level);
      return -1;
    }
    if(cmhdr->cmsg_type != SCM_RIGHTS)
    {
      printf("Invalid cmsg_type %d\n",cmhdr->cmsg_type);
      return -1;
    }
    p_stdout = *((int *) CMSG_DATA(cmhdr));
    
    size = recvmsg(conn,&mhdr,0);
    if(size == -1)
    {
      printf("Recvmsg failed\n");
      return -1;
    }
    cmhdr = CMSG_FIRSTHDR(&mhdr);
    if(!cmhdr)
    {
      printf("Expected one Recvmsg header with FD, Got zero Headers!\n");
      return -1;
    }
    if(cmhdr->cmsg_level != SOL_SOCKET)
    {
      printf("Invalid cmsg_level %d\n",cmhdr->cmsg_level);
      return -1;
    }
    if(cmhdr->cmsg_type != SCM_RIGHTS)
    {
      printf("Invalid cmsg_type %d\n",cmhdr->cmsg_type);
      return -1;
    }
    p_stderr = *((int *) CMSG_DATA(cmhdr));
    
    if(pin > 0)
    {
      size = recvmsg(conn,&mhdr,0);
      if(size == -1)
      {
        printf("Recvmsg failed\n");
        return -1;
      }
      cmhdr = CMSG_FIRSTHDR(&mhdr);
      if(!cmhdr)
      {
        printf("Expected one Recvmsg header with FD, Got zero Headers!\n");
        return -1;
      }
      if(cmhdr->cmsg_level != SOL_SOCKET)
      {
        printf("Invalid cmsg_level %d\n",cmhdr->cmsg_level);
        return -1;
      }
      if(cmhdr->cmsg_type != SCM_RIGHTS)
      {
        printf("Invalid cmsg_type %d\n",cmhdr->cmsg_type);
        return -1;
      }
      p_stdin = *((int *) CMSG_DATA(cmhdr));
    }
    else
    {
      int fd;
      /* connect input to /dev/null */
      fd = open("/dev/null", O_RDONLY, 0);
      if (fd != fileno(stdin)) {
        dup2(fd, fileno(stdin));
      }
      close(fd);
    }
  }
  else
  {
    printf("pidfd %d conn %d\n",pidfd,conn);
    return -1;
  }
  printf("Got fds %d %d %d %d %d %d %d\n", write_fd,p_stdin,p_stdout,p_stderr,fcntl(write_fd,F_GETFD),fcntl(p_stdout,F_GETFD),fcntl(p_stderr,F_GETFD));
  pid_t mypid = getpid();
  prctl(PR_SET_DUMPABLE, 1);
  prctl(PR_SET_PTRACER, spid);
  write(write_fd,&mypid,sizeof(pid_t));
  int ret;
  ret = iof_setup(p_stdin,p_stdout,p_stderr);
  if(ret != 0 )
  {
    printf("Returned iof failed %d\n",ret);
    return -1;
  }
  printf("Setting signal handlers\n");

  set_handler_local(SIGTERM);
  set_handler_local(SIGINT);
  set_handler_local(SIGHUP);
  set_handler_local(SIGPIPE);
  set_handler_local(SIGCHLD);
  set_handler_local(SIGTRAP);
  /* Unblock all signals, for many of the same reasons that we
     set the default handlers, above.  This is noticable on
     Linux where the event library blocks SIGTERM, but we don't
     want that blocked by the launched process. */
  //printf("unblocking sigs\n");
  sigprocmask(0, 0, &sigs);
  sigprocmask(SIG_UNBLOCK, &sigs, 0);
  //printf("closing fd\n");
  close(write_fd);
  //printf("end of child connect\n");
  //jmp_buf tempEnv;
  //if (setjmp (tempEnv) == 0)
  //{
    //printf("end of child connect Stack Pointer %p, Frame Pointer %p, PC %p, Stack pointer data %p\n", SPFromJmpBuf(tempEnv),FPFromJmpBuf(tempEnv),PCFromJmpBuf(tempEnv),*((long *)SPFromJmpBuf(tempEnv)));
    //longjmp (tempEnv, 1);
  //}
  return 0;
}

/* MPI_Init */

int MPI_Init (int *argc, char ***argv)
{
  parentpid = getppid();
  int dmn = daemon(1,1);
  setenv("POLL_HACKED","0",1);
  setenv("UCX_POLL_OVERRIDE","1",1);
  setenv("IBV_POLL_OVERRIDE","1",1);
  //printf("daemon out %d\n", dmn);
  jmp_buf tempEnv;
  if (setjmp (tempEnv) == 0)
  {
    stackPosInit = SPFromJmpBuf (tempEnv);
    printf("Stack Pointer %p, Frame Pointer %p, PC %p \n", stackPosInit,FPFromJmpBuf(tempEnv),PCFromJmpBuf(tempEnv));
    longjmp (tempEnv, 1);
  }
  
  FILE *memfile;
  char *memfname = (char *)malloc(1000*sizeof(char));
  char *buffer = (char *)malloc(1000*sizeof(char));
  sprintf(memfname,"/proc/%d/maps",getpid());
  memfile = fopen(memfname,"r");
  if(memfile == NULL)
  {
  	printf("Error! MEM File");
  	exit(1);
  }
  ssize_t memread;
  size_t memlen = 0;
  char * word;
  //RAW_ADDR temp;
  bool firstline = true;
  heapLimit = -1;
  libLimit[0] = -1;
  libLimit[1] = -1;
  while ((memread = getline(&buffer, &memlen, memfile)) != -1)
  {
    if(firstline)
    {
      char *ptr = strchr(buffer,'-');
      if(ptr!=NULL) *ptr = '\0';
      textstart = strtol(buffer,NULL,16);
      firstline = false;
    }
    if (strstr(buffer, "stack"))
    {
      GetFirstWord(buffer,buffer);
      word = strstr(buffer,"-");
      if(word!=NULL) *word = '\0';
      word++;
      stackPosInit = strtol(word,NULL,16);
      stackPosInit--;
      stackLimit = strtol(buffer,NULL,16);
      printf("stackPosInit: %p, stackLimit: %p\n",stackPosInit,stackLimit);
    }
		else if (strstr(buffer, "/home/phd/21/cdsjsar/Adaptive_Replication/parep-mpi/lib/libmpi.so"))
    {
      GetFirstWord(buffer,buffer);
      word = strstr(buffer,"-");
      if(word!=NULL) *word = '\0';
      word++;
      if(libLimit[1] < strtol(word,NULL,16))
      {
        libLimit[1] = strtol(word,NULL,16);
        printf("lib upper limit updated to %p\n",libLimit[1]);
      }
      libLimit[1]--;
      if((libLimit[0] > strtol(buffer,NULL,16)) || (libLimit[0] == -1))
      {
				libLimit[0] = strtol(buffer,NULL,16);
        printf("lib lower limit updated to %p\n",libLimit[0]);
      }
    }
  }
	if(libLimit[0] >= 0 && libLimit[1] >= 0) libUsed = true;

  printf("starting init\n");

  mpi_ft_comm_init(OMPI_COMM_NULL,OMPI_COMM_NULL,OMPI_COMM_NULL,OMPI_COMM_NULL,OMPI_COMM_NULL,OMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,&mpi_ft_comm_null);
  mpi_ft_op_init(OMPI_OP_NULL,EMPI_OP_NULL,&mpi_ft_op_null);
  mpi_ft_datatype_init(OMPI_DATATYPE_NULL,EMPI_DATATYPE_NULL,-1,&mpi_ft_datatype_null);
  mpi_ft_request_init(EMPI_REQUEST_NULL,EMPI_REQUEST_NULL,false,MPI_COMM_NULL,MPI_STATUS_IGNORE,-1,/*NULL,*/NULL,NULL,&mpi_ft_request_null);
  mpi_ft_comm_init(OMPI_COMM_SELF,OMPI_COMM_NULL,OMPI_COMM_NULL,OMPI_COMM_NULL,OMPI_COMM_NULL,OMPI_COMM_NULL,EMPI_COMM_SELF,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,EMPI_COMM_NULL,&mpi_ft_comm_self);
  
	mpi_ft_datatype_init(OMPI_CHAR,EMPI_CHAR,sizeof(char),&mpi_ft_datatype_char);
	mpi_ft_datatype_init(OMPI_SIGNED_CHAR,EMPI_SIGNED_CHAR,sizeof(signed char),&mpi_ft_datatype_signed_char);
	mpi_ft_datatype_init(OMPI_UNSIGNED_CHAR,EMPI_UNSIGNED_CHAR,sizeof(unsigned char),&mpi_ft_datatype_unsigned_char);
	mpi_ft_datatype_init(OMPI_BYTE,EMPI_BYTE,1,&mpi_ft_datatype_byte);
	mpi_ft_datatype_init(OMPI_SHORT,EMPI_SHORT,sizeof(short),&mpi_ft_datatype_short);
	mpi_ft_datatype_init(OMPI_UNSIGNED_SHORT,EMPI_UNSIGNED_SHORT,sizeof(unsigned short),&mpi_ft_datatype_unsigned_short);
	mpi_ft_datatype_init(OMPI_INT,EMPI_INT,sizeof(int),&mpi_ft_datatype_int);
	mpi_ft_datatype_init(OMPI_UNSIGNED,EMPI_UNSIGNED,sizeof(unsigned int),&mpi_ft_datatype_unsigned);
	mpi_ft_datatype_init(OMPI_LONG,EMPI_LONG,sizeof(long),&mpi_ft_datatype_long);
	mpi_ft_datatype_init(OMPI_UNSIGNED_LONG,EMPI_UNSIGNED_LONG,sizeof(unsigned long),&mpi_ft_datatype_unsigned_long);
	mpi_ft_datatype_init(OMPI_FLOAT,EMPI_FLOAT,sizeof(float),&mpi_ft_datatype_float);
	mpi_ft_datatype_init(OMPI_DOUBLE,EMPI_DOUBLE,sizeof(double),&mpi_ft_datatype_double);
	mpi_ft_datatype_init(OMPI_LONG_DOUBLE,EMPI_LONG_DOUBLE,sizeof(long double),&mpi_ft_datatype_long_double);
	mpi_ft_datatype_init(OMPI_LONG_LONG_INT,EMPI_LONG_LONG_INT,sizeof(long long int),&mpi_ft_datatype_long_long_int);
	mpi_ft_datatype_init(OMPI_UNSIGNED_LONG_LONG,EMPI_UNSIGNED_LONG_LONG,sizeof(unsigned long long),&mpi_ft_datatype_unsigned_long_long);

	mpi_ft_datatype_init(OMPI_PACKED,EMPI_PACKED,1,&mpi_ft_datatype_packed);
	//mpi_ft_datatype_init(OMPI_LB,EMPI_LB,sizeof(int),&mpi_ft_datatype_lb);
	//mpi_ft_datatype_init(OMPI_UB,EMPI_UB,sizeof(int),&mpi_ft_datatype_ub);
	mpi_ft_datatype_init(OMPI_FLOAT_INT,EMPI_FLOAT_INT,sizeof(float)+sizeof(int),&mpi_ft_datatype_float_int);
	mpi_ft_datatype_init(OMPI_DOUBLE_INT,EMPI_DOUBLE_INT,sizeof(double)+sizeof(int),&mpi_ft_datatype_double_int);
	mpi_ft_datatype_init(OMPI_LONG_INT,EMPI_LONG_INT,sizeof(long)+sizeof(int),&mpi_ft_datatype_long_int);
	mpi_ft_datatype_init(OMPI_SHORT_INT,EMPI_SHORT_INT,sizeof(short)+sizeof(int),&mpi_ft_datatype_short_int);
	mpi_ft_datatype_init(OMPI_2INT,EMPI_2INT,sizeof(int)+sizeof(int),&mpi_ft_datatype_2int);
	mpi_ft_datatype_init(OMPI_LONG_DOUBLE_INT,EMPI_LONG_DOUBLE_INT,sizeof(long double)+sizeof(int),&mpi_ft_datatype_long_double_int);
	mpi_ft_datatype_init(OMPI_COMPLEX,EMPI_COMPLEX,8,&mpi_ft_datatype_complex);
	mpi_ft_datatype_init(OMPI_DOUBLE_COMPLEX,EMPI_DOUBLE_COMPLEX,16,&mpi_ft_datatype_double_complex);
	mpi_ft_datatype_init(OMPI_LOGICAL,EMPI_LOGICAL,4,&mpi_ft_datatype_logical);
	mpi_ft_datatype_init(OMPI_REAL,EMPI_REAL,4,&mpi_ft_datatype_real);
	mpi_ft_datatype_init(OMPI_DOUBLE_PRECISION,EMPI_DOUBLE_PRECISION,8,&mpi_ft_datatype_double_precision);
	mpi_ft_datatype_init(OMPI_INTEGER,EMPI_INTEGER,4,&mpi_ft_datatype_integer);
	mpi_ft_datatype_init(OMPI_2INTEGER,EMPI_2INTEGER,8,&mpi_ft_datatype_2integer);
	mpi_ft_datatype_init(OMPI_2REAL,EMPI_2REAL,8,&mpi_ft_datatype_2real);
	mpi_ft_datatype_init(OMPI_2DOUBLE_PRECISION,EMPI_2DOUBLE_PRECISION,16,&mpi_ft_datatype_2double_precision);
	mpi_ft_datatype_init(OMPI_CHARACTER,EMPI_CHARACTER,1,&mpi_ft_datatype_character);
	//mpi_ft_datatype_init(OMPI_REAL4,EMPI_REAL4,4,&mpi_ft_datatype_real4);
	//mpi_ft_datatype_init(OMPI_REAL8,EMPI_REAL8,8,&mpi_ft_datatype_real8);
	//mpi_ft_datatype_init(OMPI_REAL16,EMPI_REAL16,16,&mpi_ft_datatype_real16);
	//mpi_ft_datatype_init(OMPI_COMPLEX8,EMPI_COMPLEX8,8,&mpi_ft_datatype_complex8);
	//mpi_ft_datatype_init(OMPI_COMPLEX16,EMPI_COMPLEX16,16,&mpi_ft_datatype_complex16);
	//mpi_ft_datatype_init(OMPI_COMPLEX32,EMPI_COMPLEX32,32,&mpi_ft_datatype_complex32);
	//mpi_ft_datatype_init(OMPI_INTEGER1,EMPI_INTEGER1,1,&mpi_ft_datatype_integer1);
	//mpi_ft_datatype_init(OMPI_INTEGER2,EMPI_INTEGER2,2,&mpi_ft_datatype_integer2);
	//mpi_ft_datatype_init(OMPI_INTEGER4,EMPI_INTEGER4,4,&mpi_ft_datatype_integer4);
	//mpi_ft_datatype_init(OMPI_INTEGER8,EMPI_INTEGER8,8,&mpi_ft_datatype_integer8);
	//mpi_ft_datatype_init(OMPI_INTEGER16,EMPI_INTEGER16,16,&mpi_ft_datatype_integer16);
	mpi_ft_datatype_init(OMPI_INT8_T,EMPI_INT8_T,sizeof(int8_t),&mpi_ft_datatype_int8_t);
	mpi_ft_datatype_init(OMPI_INT16_T,EMPI_INT16_T,sizeof(int16_t),&mpi_ft_datatype_int16_t);
	mpi_ft_datatype_init(OMPI_INT32_T,EMPI_INT32_T,sizeof(int32_t),&mpi_ft_datatype_int32_t);
	mpi_ft_datatype_init(OMPI_INT64_T,EMPI_INT64_T,sizeof(int64_t),&mpi_ft_datatype_int64_t);
	mpi_ft_datatype_init(OMPI_UINT8_T,EMPI_UINT8_T,sizeof(uint8_t),&mpi_ft_datatype_uint8_t);
	mpi_ft_datatype_init(OMPI_UINT16_T,EMPI_UINT16_T,sizeof(uint16_t),&mpi_ft_datatype_uint16_t);
	mpi_ft_datatype_init(OMPI_UINT32_T,EMPI_UINT32_T,sizeof(uint32_t),&mpi_ft_datatype_uint32_t);
	mpi_ft_datatype_init(OMPI_UINT64_T,EMPI_UINT64_T,sizeof(uint64_t),&mpi_ft_datatype_uint64_t);
	mpi_ft_datatype_init(OMPI_C_BOOL,EMPI_C_BOOL,sizeof(bool),&mpi_ft_datatype_c_bool);
	mpi_ft_datatype_init(OMPI_C_FLOAT_COMPLEX,EMPI_C_FLOAT_COMPLEX,sizeof(float _Complex),&mpi_ft_datatype_c_float_complex);
	mpi_ft_datatype_init(OMPI_C_DOUBLE_COMPLEX,EMPI_C_DOUBLE_COMPLEX,sizeof(double _Complex),&mpi_ft_datatype_c_double_complex);
	mpi_ft_datatype_init(OMPI_C_LONG_DOUBLE_COMPLEX,EMPI_C_LONG_DOUBLE_COMPLEX,sizeof(long double _Complex),&mpi_ft_datatype_c_long_double_complex);
	mpi_ft_datatype_init(OMPI_AINT,EMPI_AINT,4,&mpi_ft_datatype_aint);
	mpi_ft_datatype_init(OMPI_OFFSET,EMPI_OFFSET,4,&mpi_ft_datatype_offset);
	mpi_ft_datatype_init(OMPI_COUNT,EMPI_COUNT,sizeof(int),&mpi_ft_datatype_count);
	mpi_ft_datatype_init(OMPI_CXX_BOOL,EMPI_CXX_BOOL,sizeof(bool),&mpi_ft_datatype_cxx_bool);
	mpi_ft_datatype_init(OMPI_CXX_FLOAT_COMPLEX,EMPI_CXX_FLOAT_COMPLEX,sizeof(float _Complex),&mpi_ft_datatype_cxx_float_complex);
	mpi_ft_datatype_init(OMPI_CXX_DOUBLE_COMPLEX,EMPI_CXX_DOUBLE_COMPLEX,sizeof(double _Complex),&mpi_ft_datatype_cxx_double_complex);
	mpi_ft_datatype_init(OMPI_CXX_LONG_DOUBLE_COMPLEX,EMPI_CXX_LONG_DOUBLE_COMPLEX,sizeof(long double _Complex),&mpi_ft_datatype_cxx_long_double_complex);
	
	mpi_ft_op_init(OMPI_MAX,EMPI_MAX,&mpi_ft_op_max);
  mpi_ft_op_init(OMPI_MIN,EMPI_MIN,&mpi_ft_op_min);
  mpi_ft_op_init(OMPI_SUM,EMPI_SUM,&mpi_ft_op_sum);
  mpi_ft_op_init(OMPI_PROD,EMPI_PROD,&mpi_ft_op_prod);
  mpi_ft_op_init(OMPI_LAND,EMPI_LAND,&mpi_ft_op_land);
  mpi_ft_op_init(OMPI_BAND,EMPI_BAND,&mpi_ft_op_band);
  mpi_ft_op_init(OMPI_LOR,EMPI_LOR,&mpi_ft_op_lor);
  mpi_ft_op_init(OMPI_BOR,EMPI_BOR,&mpi_ft_op_bor);
  mpi_ft_op_init(OMPI_LXOR,EMPI_LXOR,&mpi_ft_op_lxor);
  mpi_ft_op_init(OMPI_BXOR,EMPI_BXOR,&mpi_ft_op_bxor);
  mpi_ft_op_init(OMPI_MINLOC,EMPI_MINLOC,&mpi_ft_op_minloc);
  mpi_ft_op_init(OMPI_MAXLOC,EMPI_MAXLOC,&mpi_ft_op_maxloc);
  mpi_ft_op_init(OMPI_REPLACE,EMPI_REPLACE,&mpi_ft_op_replace);
  mpi_ft_op_init(OMPI_NO_OP,EMPI_NO_OP,&mpi_ft_op_no_op);
  
  extLib = dlopen("/home/phd/21/cdsjsar/MVAPICH2/lib/libmpi.so", RTLD_NOW);
  EMPI_Init = (int(*)(int *,char ***))dlsym(extLib,"MPI_Init");
  EMPI_Finalize = (int(*)())dlsym(extLib,"MPI_Finalize");
  EMPI_Comm_size = (int(*)(EMPI_Comm,int *))dlsym(extLib,"MPI_Comm_size");
  EMPI_Comm_remote_size = (int(*)(EMPI_Comm,int *))dlsym(extLib,"MPI_Comm_remote_size");
  EMPI_Comm_rank = (int(*)(EMPI_Comm,int *))dlsym(extLib,"MPI_Comm_rank");
  EMPI_Send = (int(*)(void *,int,EMPI_Datatype,int,int,EMPI_Comm))dlsym(extLib,"MPI_Send");
  EMPI_Recv = (int(*)(void *,int,EMPI_Datatype,int,int,EMPI_Comm,EMPI_Status *))dlsym(extLib,"MPI_Recv");
  EMPI_Comm_free = (int(*)(EMPI_Comm *))dlsym(extLib,"MPI_Comm_free");
  EMPI_Comm_dup = (int(*)(EMPI_Comm,EMPI_Comm *))dlsym(extLib,"MPI_Comm_dup");
  EMPI_Comm_spawn = (int(*)(const char *,char **,int,EMPI_Info,int,EMPI_Comm,int *))dlsym(extLib,"MPI_Comm_spawn");
  EMPI_Barrier = (int(*)(EMPI_Comm))dlsym(extLib,"MPI_Barrier");
  EMPI_Abort = (int(*)(EMPI_Comm,int))dlsym(extLib,"MPI_Abort");
  EMPI_Comm_group = (int(*)(EMPI_Comm,EMPI_Group *))dlsym(extLib,"MPI_Comm_group");
  EMPI_Group_incl = (int(*)(EMPI_Group,int,int *,EMPI_Group *))dlsym(extLib,"MPI_Group_incl");
  EMPI_Comm_create_group = (int(*)(EMPI_Comm,EMPI_Group,int,EMPI_Comm *))dlsym(extLib,"MPI_Comm_create_group");
  EMPI_Comm_split = (int(*)(EMPI_Comm,int,int,EMPI_Comm *))dlsym(extLib,"MPI_Comm_split");
  EMPI_Intercomm_create = (int(*)(EMPI_Comm,int,EMPI_Comm,int,int,EMPI_Comm *))dlsym(extLib,"MPI_Intercomm_create");
  EMPI_Comm_set_name = (int(*)(EMPI_Comm,const char *))dlsym(extLib,"MPI_Comm_set_name");
  EMPI_Bcast = (int(*)(void *,int,EMPI_Datatype,int,EMPI_Comm))dlsym(extLib,"MPI_Bcast");
  EMPI_Allgather = (int(*)(void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm))dlsym(extLib,"MPI_Allgather");
	EMPI_Alltoall = (int(*)(void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm))dlsym(extLib,"MPI_Alltoall");
  EMPI_Allgatherv = (int(*)(void *,int,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm))dlsym(extLib,"MPI_Allgatherv");
  EMPI_Allreduce = (int(*)(void *,void *,int,EMPI_Datatype,EMPI_Op,EMPI_Comm))dlsym(extLib,"MPI_Allreduce");
  EMPI_Isend = (int(*)(void *,int,EMPI_Datatype,int,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Isend");
  EMPI_Irecv = (int(*)(void *,int,EMPI_Datatype,int,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Irecv");
  EMPI_Ibcast = (int(*)(void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Ibcast");
	EMPI_Iscatter = (int(*)(const void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Iscatter");
	EMPI_Iscatterv = (int(*)(const void *,int *,int *,EMPI_Datatype,void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Iscatterv");
	EMPI_Igather = (int(*)(const void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Igather");
	EMPI_Igatherv = (int(*)(const void *,int,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Igatherv");
	EMPI_Ireduce = (int(*)(void *,void *,int,EMPI_Datatype,EMPI_Op,int,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Ireduce");
	EMPI_Iallgather = (int(*)(const void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Iallgather");
	EMPI_Iallgatherv = (int(*)(const void *,int *,int *,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Iallgatherv");
	EMPI_Ialltoall = (int(*)(void *,int,EMPI_Datatype,void *,int,EMPI_Datatype,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Ialltoall");
	EMPI_Ialltoallv = (int(*)(void *,int *,int *,EMPI_Datatype,void *,int *,int *,EMPI_Datatype,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Ialltoallv");
	EMPI_Iallreduce = (int(*)(void *,void *,int,EMPI_Datatype,EMPI_Op,EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Iallreduce");
  EMPI_Ibarrier = (int(*)(EMPI_Comm,EMPI_Request *))dlsym(extLib,"MPI_Ibarrier");
  EMPI_Test = (int(*)(EMPI_Request *,int *,EMPI_Status *))dlsym(extLib,"MPI_Test");
  EMPI_Cancel = (int(*)(EMPI_Request *))dlsym(extLib,"MPI_Cancel");
  EMPI_Request_free = (int(*)(EMPI_Request *))dlsym(extLib,"MPI_Request_free");
  EMPI_Type_size = (int(*)(EMPI_Datatype,int *))dlsym(extLib,"MPI_Type_size");
	EMPI_Get_count = (int(*)(EMPI_Status *,EMPI_Datatype,int *))dlsym(extLib,"MPI_Get_count");
  EMPI_Wtime = (double(*)())dlsym(extLib,"MPI_Wtime");
  
  openLib = dlopen("/home/phd/21/cdsjsar/openMPI/lib/libmpi.so", RTLD_NOW);
  OMPI_Init = (int(*)(int *,char ***))dlsym(openLib,"MPI_Init");
  OMPI_Finalize = (int(*)())dlsym(openLib,"MPI_Finalize");
  OMPI_Comm_size = (int(*)(OMPI_Comm,int *))dlsym(openLib,"MPI_Comm_size");
  OMPI_Comm_remote_size = (int(*)(OMPI_Comm,int *))dlsym(openLib,"MPI_Comm_remote_size");
  OMPI_Comm_rank = (int(*)(OMPI_Comm,int *))dlsym(openLib,"MPI_Comm_rank");
  OMPI_Send = (int(*)(void *,int,OMPI_Datatype,int,int,OMPI_Comm))dlsym(openLib,"MPI_Send");
  OMPI_Recv = (int(*)(void *,int,OMPI_Datatype,int,int,OMPI_Comm,OMPI_Status *))dlsym(openLib,"MPI_Recv");
  OMPI_Comm_free = (int(*)(OMPI_Comm *))dlsym(openLib,"MPI_Comm_free");
  OMPI_Comm_dup = (int(*)(OMPI_Comm,OMPI_Comm *))dlsym(openLib,"MPI_Comm_dup");
  OMPI_Comm_create_errhandler = (int(*)(OMPI_Comm_errhandler_function *,OMPI_Errhandler *))dlsym(openLib,"MPI_Comm_create_errhandler");
  OMPI_Comm_set_errhandler = (int(*)(OMPI_Comm, OMPI_Errhandler))dlsym(openLib,"MPI_Comm_set_errhandler");
  OMPI_Barrier = (int(*)(OMPI_Comm))dlsym(openLib,"MPI_Barrier");
  OMPI_Abort = (int(*)(OMPI_Comm,int))dlsym(openLib,"MPI_Abort");
  OMPI_Error_string = (int(*)(int, char *,int *))dlsym(openLib,"MPI_Error_string");
  OMPI_Error_class = (int(*)(int,int *))dlsym(openLib,"MPI_Error_class");
  OMPIX_Comm_is_revoked = (int(*)(OMPI_Comm,int *))dlsym(openLib,"MPIX_Comm_is_revoked");
  OMPIX_Comm_revoke = (int(*)(OMPI_Comm))dlsym(openLib,"MPIX_Comm_revoke");
  OMPIX_Comm_shrink = (int(*)(OMPI_Comm,OMPI_Comm *))dlsym(openLib,"MPIX_Comm_shrink");
  OMPI_Group_difference = (int(*)(OMPI_Group,OMPI_Group,OMPI_Group *))dlsym(openLib,"MPI_Group_difference");
  OMPI_Group_size = (int(*)(OMPI_Group,int *))dlsym(openLib,"MPI_Group_size");
  OMPI_Group_translate_ranks = (int(*)(OMPI_Group,int,const int *,OMPI_Group,int *))dlsym(openLib,"MPI_Group_translate_ranks");
  OMPI_Group_incl = (int(*)(OMPI_Group,int,int *,OMPI_Group *))dlsym(openLib,"MPI_Group_incl");
  OMPI_Comm_group = (int(*)(OMPI_Comm,OMPI_Group *))dlsym(openLib,"MPI_Comm_group");
  OMPI_Comm_remote_group = (int(*)(OMPI_Comm,OMPI_Group *))dlsym(openLib,"MPI_Comm_remote_group");
  OMPI_Comm_create = (int(*)(OMPI_Comm,OMPI_Group,OMPI_Comm *))dlsym(openLib,"MPI_Comm_create");
  OMPI_Comm_split = (int(*)(OMPI_Comm,int,int,OMPI_Comm *))dlsym(openLib,"MPI_Comm_split");
  OMPI_Intercomm_create = (int(*)(OMPI_Comm,int,OMPI_Comm,int,int,OMPI_Comm *))dlsym(openLib,"MPI_Intercomm_create");
  OMPI_Comm_set_name = (int(*)(OMPI_Comm,const char *))dlsym(openLib,"MPI_Comm_set_name");
  OMPI_Comm_test_inter = (int(*)(OMPI_Comm,int *))dlsym(openLib,"MPI_Comm_test_inter");
  OMPIX_Comm_failure_ack = (int(*)(OMPI_Comm))dlsym(openLib,"MPIX_Comm_failure_ack");
  OMPIX_Comm_failure_get_acked = (int(*)(OMPI_Comm,OMPI_Group *))dlsym(openLib,"MPIX_Comm_failure_get_acked");
  OMPI_Bcast = (int(*)(void *,int,OMPI_Datatype,int,OMPI_Comm))dlsym(openLib,"MPI_Bcast");
  OMPI_Allgather = (int(*)(void *,int,OMPI_Datatype,void *,int,OMPI_Datatype,OMPI_Comm))dlsym(openLib,"MPI_Allgather");
  OMPIX_Comm_mark_failed = (int(*)(OMPI_Comm,int))dlsym(openLib,"MPIX_Comm_mark_failed");
  printf("Dynamic loading done\n");

  int i, retVal, worldRank, worldSize, color, cmpLeader, repLeader;
  int destRank = -1, srcRank = -1;
  retVal = EMPI_Init (argc, argv);
  printf("EMPI INIT DONE\n");
  
  //int dmn = daemon(1,1);
  //printf("daemon out %d\n", dmn);

  EMPI_Comm_dup(EMPI_COMM_WORLD,&(MPI_COMM_WORLD->eworldComm));
  
  EMPI_Comm_rank (MPI_COMM_WORLD->eworldComm, &worldRank);
  EMPI_Comm_size (MPI_COMM_WORLD->eworldComm, &worldSize);

	if((getenv("CMP_RATIO") != NULL)  &&(getenv("REP_RATIO") != NULL))
	{
		int cr = atoi(getenv("CMP_RATIO"));
		int rr = atoi(getenv("REP_RATIO"));
		nR = (worldSize*rr)/(rr+cr);
	}
	else if(getenv("COMP_PER_REP") != NULL)
	{
		int cpr = atoi(getenv("COMP_PER_REP"));
		if(cpr > 0) nR = worldSize/(1+cpr);
		else nR = 0;
	}
	else
	{
		if(CMP_PER_REP > 0) nR = worldSize/(1+CMP_PER_REP);
		else nR = 0;
	}
  //nR = worldSize - worldSize / REP_DEGREE;
  nC = worldSize - nR;
  
  /*FILE *pidfptr;
  char *pidfname = (char *)malloc(1000*sizeof(char));
  sprintf(pidfname,"./connection/%dpid",worldRank);
  pidfptr = fopen(pidfname,"w+");
  if(pidfptr == NULL)
  {
  	printf("Error! PID File");
  	exit(1);
  }
  fprintf(pidfptr,"%d\n",getpid());
  fprintf(pidfptr,"%d\n",worldRank);
  fprintf(pidfptr,"%d",worldRank < nC);
  fclose(pidfptr);
  free(pidfname);*/
  cmpToRepMap = (int *) myMalloc (&cmpToRepMap, nC * sizeof (int));
  repToCmpMap = (int *) myMalloc (&repToCmpMap, nR * sizeof (int));

  for (i=0; i<nC; i++)
  {
    if (i < nR)
    {
      cmpToRepMap [i] = i;
      repToCmpMap [i] = i;
    }
    else
      cmpToRepMap [i] = -1;
  }
  MPI_COMM_WORLD->EMPI_COMM_CMP = MPI_COMM_WORLD->EMPI_COMM_REP = MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM = MPI_COMM_WORLD->EMPI_CMP_NO_REP = MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM = EMPI_COMM_NULL;
  if (nR > 0)
  {

    /*cmpToRepMap = (int *) myMalloc (&cmpToRepMap, nC * sizeof (int));
    repToCmpMap = (int *) myMalloc (&repToCmpMap, nR * sizeof (int));

    for (i=0; i<nC; i++)
    {
      if (i < nR)
      {
        cmpToRepMap [i] = i;
        repToCmpMap [i] = i;
      }

      else
        cmpToRepMap [i] = -1;
    }*/

    //EMPI_COMM_CMP = EMPI_COMM_REP = EMPI_CMP_REP_INTERCOMM = EMPI_COMM_NULL;

    color = worldRank < nC ? 0 : 1;
    cmpLeader = 0;
    repLeader = nC;
		
		//eworldComm = EMPI_COMM_CMP = EMPI_COMM_REP = EMPI_CMP_REP_INTERCOMM = EMPI_COMM_NULL;
		printf("At EMPI nC %d nR %d color %d\n",nC,nR,color);
    if (color == 0)
    {
      EMPI_Comm_split (MPI_COMM_WORLD->eworldComm, color, 0, &(MPI_COMM_WORLD->EMPI_COMM_CMP));
      EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_CMP, 0, MPI_COMM_WORLD->eworldComm, repLeader, 100, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
			int colorinternal;
			if(cmpToRepMap[worldRank] != -1) colorinternal = EMPI_UNDEFINED;
			else colorinternal = 1;
			EMPI_Comm_split (MPI_COMM_WORLD->EMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
			if(MPI_COMM_WORLD->EMPI_CMP_NO_REP != EMPI_COMM_NULL) EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->eworldComm, repLeader, 101, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
      EMPI_Comm_set_name (MPI_COMM_WORLD->eworldComm,"eworldComm");
      EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_COMM_CMP,"EMPI_COMM_CMP");
      EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,"EMPI_CMP_REP_INTERCOMM");
			MPI_COMM_WORLD->EMPI_COMM_REP = EMPI_COMM_NULL;
    }

    else if (color == 1)
    {
      EMPI_Comm_split (MPI_COMM_WORLD->eworldComm, color, 0, &(MPI_COMM_WORLD->EMPI_COMM_REP));
      EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, cmpLeader, 100, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
			if(nC > nR) EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, nR, 101, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
      EMPI_Comm_set_name (MPI_COMM_WORLD->eworldComm,"eworldComm");
      EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_COMM_REP,"EMPI_COMM_REP");
      EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,"EMPI_CMP_REP_INTERCOMM");
			MPI_COMM_WORLD->EMPI_COMM_CMP = EMPI_COMM_NULL;
    }
    if (MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL && cmpToRepMap[worldRank] != -1)
      destRank = cmpToRepMap[worldRank];
    else if (MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL && repToCmpMap[worldRank - nC] != -1)
      srcRank = repToCmpMap[worldRank - nC];
  }
  else
	{
		EMPI_Comm_dup(MPI_COMM_WORLD->eworldComm,&(MPI_COMM_WORLD->EMPI_COMM_CMP));
		EMPI_Comm_dup(MPI_COMM_WORLD->eworldComm,&(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
	}
  printf("MPI comm world before replication %p\n",MPI_COMM_WORLD);
	
  fseek(memfile,0,SEEK_SET);
  while ((memread = getline(&buffer, &memlen, memfile)) != -1)
  {
    if (strstr(buffer, "heap"))
    {
      GetFirstWord(buffer,buffer);
      word = strstr(buffer,"-");
      word++;
      if(heapLimit < strtol(word,NULL,16))
      {
        heapLimit = strtol(word,NULL,16);
        printf("heapLimit updated to %p\n",heapLimit);
      }
    }
  }
  fclose(memfile);
  free(memfname);
  free(buffer);
  
  if (destRank != -1)
  {
    printf ("Sending image to rank %d BEGIN\n", destRank);
    sendImage (worldRank, destRank);
    int rank;
    EMPI_Comm_rank (MPI_COMM_WORLD->eworldComm, &rank);
    printf ("%d Sending image to rank %d END\n",rank, destRank);
  }

  else if (srcRank != -1)
  {
    printf ("Receiving image from rank %d BEGIN\n", srcRank);
    recvImage (srcRank);
    printf ("Receiving image from rank %d END\n", srcRank);
  }
  printf("MPI comm world after replication %p\n",MPI_COMM_WORLD);
	
	//if(eworldComm != EMPI_COMM_NULL) EMPI_Comm_dup(eworldComm,&(MPI_COMM_WORLD->eworldComm));
	//if(EMPI_COMM_CMP != EMPI_COMM_NULL) EMPI_Comm_dup(EMPI_COMM_CMP,&(MPI_COMM_WORLD->EMPI_COMM_CMP));
	//if(EMPI_COMM_REP != EMPI_COMM_NULL) EMPI_Comm_dup(EMPI_COMM_REP,&(MPI_COMM_WORLD->EMPI_COMM_REP));
	//if(EMPI_CMP_REP_INTERCOMM != EMPI_COMM_NULL) EMPI_Comm_dup(EMPI_CMP_REP_INTERCOMM,&(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));

  int myrank,mysize;
  EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
  EMPI_Comm_size(MPI_COMM_WORLD->eworldComm,&mysize);
  
  EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
  printf("Rank %d starting prte server connection\n",myrank);
  
  FILE *fptr;
  char *fname = (char *)malloc(1000*sizeof(char));
  //sprintf(fname,"./connection/%dpid",r);
  //fptr = fopen(fname,"w+");
  //fprintf(fptr,"%d", getpid());
  //fclose(fptr);
  sprintf(fname,"%s/connection/%dcheck",workDir,myrank);
  fptr = fopen(fname,"r");
  while(fptr == NULL)
  {
    fptr = fopen(fname,"r");
  }
  fclose(fptr);
  remove(fname);
  sprintf(fname,"%s/connection/%d",workDir,myrank);
  printf("fname %s\n", fname);
  fptr = fopen(fname,"r");
  if(fptr == NULL)
  {
    printf("Connection file open failed %d\n",errno);
    exit(1);
  }
  char *line = NULL;
  size_t llen = 0;
  ssize_t read;
  int pipefd=-1,pin=-1,perr=-1,pout=-1,spid=-1;
  char dyncwd[PATH_MAX];
  while ((read = getline(&line, &llen, fptr)) != -1)
  {
    line[strlen(line)-1] = '\0';
    //printf("%d:%s\n",myrank,line);
    if(0 == strncmp(line,"PIPEFD",6))
    {
      char *name,*val;
      name = strtok(line,"=");
      val = strtok(NULL,"=");
      pipefd = atoi(val);
    }
    else if(0 == strncmp(line,"STDOUT",6))
    {
      char *name,*val;
      name = strtok(line,"=");
      val = strtok(NULL,"=");
      pout = atoi(val);
    }
    else if(0 == strncmp(line,"STDERR",6))
    {
      char *name,*val;
      name = strtok(line,"=");
      val = strtok(NULL,"=");
      perr = atoi(val);
    }
    else if(0 == strncmp(line,"STDIN",5))
    {
      char *name,*val;
      name = strtok(line,"=");
      val = strtok(NULL,"=");
      pin = atoi(val);
    }
    else if(0 == strncmp(line,"PID",3))
    {
      char *name,*val;
      name = strtok(line,"=");
      val = strtok(NULL,"=");
      spid = atoi(val);
    }
    else if(0 == strncmp(line,"CWD",3))
    {
      char *name,*val;
      name = strtok(line,"=");
      val = strtok(NULL,"=");
      strcpy(dyncwd,val);
    }
    else
    {
      char *name,*val;
      name = strtok(line,"=");
      val = strtok(NULL,"=");
      setenv(name,val,1);
    }
  }
  printf("%d %d %d %d %d %d %d %d \n", getpid(),getppid(),myrank,pipefd,pin,pout,perr,spid);
  fclose(fptr);
  remove(fname);
  //int pidfd = syscall(SYS_pidfd_open, (pid_t)spid, 0);
  //while(pidfd == -1)
  //{
    //pidfd = syscall(SYS_pidfd_open, (pid_t)spid, 0);
  //}
  /*if(pidfd == -1)
  {
    printf("pidfd open error\n");
    exit(1);
  }*/
  //printf("pidfd opened %d\n", pidfd);
  int pidfd = -2;
  int conn = -2;
  struct sockaddr_un addr;
  
  conn = socket(PF_UNIX,SOCK_STREAM,0);
  if(conn == -1)
  {
  	printf("Socket creation failed %d %d\n", conn,errno);
  	exit(1);
  }
  
  memset(&addr,0,sizeof(addr));
  addr.sun_family = AF_UNIX;
  char hname[50];
  gethostname(hname,50);
  snprintf(addr.sun_path,108,"%s/connection/dus_%s_%s",getenv("PAREP_MPI_WORKDIR"),hname,getenv("OMPI_COMM_WORLD_LOCAL_RANK"));
  printf("Connecting to %s\n",addr.sun_path);
  int ret = connect(conn,(struct sockaddr *)(&addr),sizeof(struct sockaddr_un));
  while(ret != 0)
  {
      ret = connect(conn,(struct sockaddr *)(&addr),sizeof(struct sockaddr_un));
  }
  //if(ret != 0)
  //{
  //	printf("Socket connection failed %d %d\n", ret,errno);
  //	exit(1);
  //}
  
  
  ret = childconnect(pipefd,pin,pout,perr,pidfd,spid,conn);
  if(ret != 0)
  {
    printf("childconnect failed %d\n", ret);
    exit(1);
  }
  printf("childconnect done %d\n",myrank);
  if(!strcmp(getenv("OMPI_COMM_WORLD_LOCAL_RANK"),"0"))
  { 
    sprintf(fname,"%s/tempwpid_%d",getenv("PAREP_MPI_WORKDIR"),parentpid);
    fptr = fopen(fname,"w+");
    if(fptr == NULL)
    {
      printf("Tempwpid file creation failed!\n");
      exit(1);
    }
    fclose(fptr);
  }
  free(fname);
  EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
  printf("Starting OMPI Init\n");
  OMPI_Init (argc, argv);
  printf("OMPI Init done\n");
  OMPI_Comm_dup(OMPI_COMM_WORLD,&(MPI_COMM_WORLD->oworldComm));
  MPI_COMM_WORLD->OMPI_COMM_CMP = MPI_COMM_WORLD->OMPI_COMM_REP = MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM = MPI_COMM_WORLD->OMPI_CMP_NO_REP = MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM = OMPI_COMM_NULL;
  /*if(nR > 0)
  {
    //OMPI_COMM_CMP = OMPI_COMM_REP = OMPI_CMP_REP_INTERCOMM = OMPI_COMM_NULL;
    cmpLeader = 0;
    repLeader = nC;
    color = myrank < nC ? 0 : 1;
		
		//oworldComm = OMPI_COMM_CMP = OMPI_COMM_REP = OMPI_CMP_REP_INTERCOMM = OMPI_COMM_NULL;
    if (color == 0)
    {
      OMPI_Comm_split (MPI_COMM_WORLD->oworldComm, color, 0, &(MPI_COMM_WORLD->OMPI_COMM_CMP));
      OMPI_Intercomm_create (MPI_COMM_WORLD->OMPI_COMM_CMP, 0, MPI_COMM_WORLD->oworldComm, repLeader, 200, &(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM));
			int colorinternal;
			if(cmpToRepMap[myrank] != -1) colorinternal = OMPI_UNDEFINED;
			else colorinternal = 1;
			OMPI_Comm_split (MPI_COMM_WORLD->OMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP));
			if(MPI_COMM_WORLD->OMPI_CMP_NO_REP != OMPI_COMM_NULL) OMPI_Intercomm_create (MPI_COMM_WORLD->OMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->oworldComm, repLeader, 201, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM));
      OMPI_Comm_set_name (MPI_COMM_WORLD->oworldComm,"oworldComm");
      OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_COMM_CMP,"OMPI_COMM_CMP");
      OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM,"OMPI_CMP_REP_INTERCOMM");
			MPI_COMM_WORLD->OMPI_COMM_REP = OMPI_COMM_NULL;
    }

    else if (color == 1)
    {
      OMPI_Comm_split (MPI_COMM_WORLD->oworldComm, color, 0, &(MPI_COMM_WORLD->OMPI_COMM_REP));
      OMPI_Intercomm_create (MPI_COMM_WORLD->OMPI_COMM_REP, 0, MPI_COMM_WORLD->oworldComm, cmpLeader, 200, &(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM));
			if(nC > nR) OMPI_Intercomm_create (MPI_COMM_WORLD->OMPI_COMM_REP, 0, MPI_COMM_WORLD->oworldComm, nR, 201, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM));
      OMPI_Comm_set_name (MPI_COMM_WORLD->oworldComm,"oworldComm");
      OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_COMM_REP,"OMPI_COMM_REP");
      OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM,"OMPI_CMP_REP_INTERCOMM");
			MPI_COMM_WORLD->OMPI_COMM_CMP = OMPI_COMM_NULL;
    }
  }
  else
	{
		MPI_COMM_WORLD->OMPI_COMM_CMP = MPI_COMM_WORLD->oworldComm;
		MPI_COMM_WORLD->OMPI_CMP_NO_REP = MPI_COMM_WORLD->oworldComm;
  }*/
	//if(oworldComm != OMPI_COMM_NULL) OMPI_Comm_dup(oworldComm,&(MPI_COMM_WORLD->oworldComm));
	//if(OMPI_COMM_CMP != OMPI_COMM_NULL) OMPI_Comm_dup(OMPI_COMM_CMP,&(MPI_COMM_WORLD->OMPI_COMM_CMP));
	//if(OMPI_COMM_REP != OMPI_COMM_NULL) OMPI_Comm_dup(OMPI_COMM_REP,&(MPI_COMM_WORLD->OMPI_COMM_REP));
	//if(OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL) OMPI_Comm_dup(OMPI_CMP_REP_INTERCOMM,&(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM));
  printf("OMPI comm creation is done\n");

  FILE *pidfptr;
  char *pidfname = (char *)malloc(1000*sizeof(char));
  sprintf(pidfname,"%s/connection/%dpid",getenv("PAREP_MPI_WORKDIR"),myrank);
  pidfptr = fopen(pidfname,"w+");
  if(pidfptr == NULL)
  {
        printf("Error! PID File");
        exit(1);
  }
  fprintf(pidfptr,"%d\n",getpid());
  fprintf(pidfptr,"%d\n",myrank);
  fprintf(pidfptr,"%d",myrank < nC);
  fclose(pidfptr);
  free(pidfname);

  char logName[200];
  sprintf (logName, "%s/logs/%d", workDir, myrank);
  printf("Logname : %s\n",logName);
  logfile = fopen(logName,"w+");

  if(logfile == NULL)
  {
    printf("Log open problem %d\n",errno);
    exit(1);
  }

  for(int i=0;i<nbuf;i++)
  {
    logbuffer[i] = (char*) malloc(300 * sizeof(char));
  }

  //OMPI_Comm_create_errhandler(HandleErrors,&ehandler);
  //if(OMPI_COMM_CMP != OMPI_COMM_NULL)OMPI_Comm_set_errhandler(OMPI_COMM_CMP,ehandler);
  //if(OMPI_COMM_REP != OMPI_COMM_NULL)OMPI_Comm_set_errhandler(OMPI_COMM_REP,ehandler);
  //if(OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL)OMPI_Comm_set_errhandler(OMPI_CMP_REP_INTERCOMM,ehandler);
  //if(oworldComm != OMPI_COMM_NULL)OMPI_Comm_set_errhandler(oworldComm,ehandler);
  //EMPI_Init (argc, argv);
  
  internalranks = (int *)malloc(nC*sizeof(int));
  //mpiprocidfd[myrank] = getpid();
  //OMPI_Allgather(&mpiprocidfd[myrank],1,OMPI_INT,mpiprocidfd,1,OMPI_INT,oworldComm);
  
  for(int i=0;i<nC;i++)
  {
    internalranks[i] = i;
  }
  //last_collective = (struct collective_data *)malloc(sizeof(struct collective_data));
  //last_collective->id = -1;
	//last_collective->next = NULL;
  OMPI_Comm_group(OMPI_COMM_WORLD,&oworldGroupFull);

  //printf("%d,%d: pidfds generated %d %d %d %d\n",myrank,mysize,mpiprocidfd[0],mpiprocidfd[1],mpiprocidfd[2],mpiprocidfd[3]);
#ifdef HEAPTRACK
  setenv("MPI_FT_TRACK_HEAP","1",1);
#endif
	
  EMPI_Barrier(MPI_COMM_WORLD->eworldComm);
	/*if(mysize >= 256)
	{
		if(myrank >= 256){ MPI_Finalize();exit(0);}
		for(int i = 0; i < 256; i++)
		{
			cmpToRepMap[i] = -1;
		}
		nC = 256;
		nR = 0;
	}*/
  return retVal;
}

/* MPI_Comm_rank */

int MPI_Comm_rank (MPI_Comm comm, int *rank)
{
  int myRank;

  if (comm->EMPI_COMM_REP != EMPI_COMM_NULL)
  {
    EMPI_Comm_rank (comm->EMPI_COMM_REP, &myRank);
    *rank = repToCmpMap [myRank];
    return MPI_SUCCESS;
  }

  else
  {
    return EMPI_Comm_rank (comm->EMPI_COMM_CMP, rank);
  }
}

/* MPI_Comm_size */

int MPI_Comm_size (MPI_Comm comm, int *size)
{
  if (comm->EMPI_COMM_REP != EMPI_COMM_NULL)
  {
    EMPI_Comm_size (comm->eworldComm, size);
    *size = *size - nR;
    return MPI_SUCCESS;
  }
  else
  {
    return EMPI_Comm_size (comm->EMPI_COMM_CMP, size);
  }
}

void GetFirstWord(char *string, char *first)
{
  int i;
  for (i = 0; i <  strlen(string); i++)
  {
    if (string[i] == ' ' || string[i] == '\n' || string[i] == '\0') break;
  }
  strncpy(first,string,i);
  first[i] = '\0';
}

static void HandleErrors(OMPI_Comm *pcomm, int *perr, ... )
{
  OMPI_Comm comm = *pcomm;
  int err = *perr;
  char errstr[OMPI_MAX_ERROR_STRING];
  int i, nf, len, eclass;
  OMPI_Group group_c, group_f, worldGroup, repGroup, cmpGroup;
  EMPI_Group eworldGroup, erepGroup, ecmpGroup;
  int *ranks_gc, *ranks_gf, *ranks_ga;
  OMPI_Error_string(err, errstr, &len);
  OMPI_Comm new_comm, new_intercomm;
  int flag;
  int nonreptol = 0;
  int *worldComm_ranks;
  bool switchingToCmp = false;
	int colorinternal;

  int r;
  OMPI_Comm_rank(MPI_COMM_WORLD->oworldComm, &r);

  OMPI_Error_class(err, &eclass);
	printf("Entered error handler with err %d wrank %d\n",err,r);

  if(eclass == OMPIX_ERR_PROC_FAILED)
  {
    if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
    {
      /*OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_COMM_CMP,&flag);
      if(!flag)
      {
        printf("revoking CMP\n");
        OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_COMM_CMP);
      }
      if(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL)
      {
        OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM,&flag);
        if(!flag)
        {
          printf("revoking INTER\n");
          OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM);
        }
      }
			if(MPI_COMM_WORLD->OMPI_CMP_NO_REP != OMPI_COMM_NULL)
			{
				OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_CMP_NO_REP,&flag);
				if(!flag)
				{
					printf("revoking CMP NO REP\n");
					OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_CMP_NO_REP);
				}
			}
			if(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM != OMPI_COMM_NULL)
			{
				OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM,&flag);
				if(!flag)
				{
					printf("revoking CMP NO REP INTER\n");
					OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM);
				}
			}*/
      OMPIX_Comm_is_revoked(MPI_COMM_WORLD->oworldComm,&flag);
      if(!flag)
      {
        //printf("revoking oworld\n");
        OMPIX_Comm_revoke(MPI_COMM_WORLD->oworldComm);
      }
      printf("before shrink CMP wrank %d\n",r);
      OMPIX_Comm_shrink(MPI_COMM_WORLD->oworldComm,&new_intercomm);
			printf("after shrink CMP wrank %d\n",r);
      OMPI_Comm_group(MPI_COMM_WORLD->oworldComm, &worldGroup);
      OMPI_Comm_group(new_intercomm, &group_c);
      OMPI_Group_difference(worldGroup,group_c,&group_f);

      //PMPIX_Comm_failure_ack(worldComm);
      //PMPIX_Comm_failure_get_acked(worldComm, &group_f);

      OMPI_Group_size(group_f, &nf);
      if(comm == MPI_COMM_WORLD->OMPI_COMM_REP)
      {
        //printf("AT OMPI_COMM_REP: World rank %d: Notified of error %s. %d found dead: { ", r, errstr, nf);
      }
      else if(comm == MPI_COMM_WORLD->oworldComm)
      {
        //printf("AT oworldComm: World rank %d: Notified of error %s. %d found dead: { ", r, errstr, nf);
      }
      else if(comm == MPI_COMM_WORLD->OMPI_COMM_CMP)
      {
        //printf("AT OMPI_COMM_CMP: World rank %d: Notified of error %s. %d found dead: { ", r , errstr, nf);
      }
      else if(comm == MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM)
      {
        //printf("AT OMPI_CMP_REP_INTERCOMM: World rank %d: Notified of error %s. %d found dead: { ", r , errstr, nf);
      }
			else if(comm == MPI_COMM_WORLD->OMPI_CMP_NO_REP)
      {
        //printf("AT OMPI_CMP_REP_INTERCOMM: World rank %d: Notified of error %s. %d found dead: { ", r , errstr, nf);
      }
			else if(comm == MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM)
      {
        //printf("AT OMPI_CMP_REP_INTERCOMM: World rank %d: Notified of error %s. %d found dead: { ", r , errstr, nf);
      }
      ranks_gf = (int*)malloc(nf * sizeof(int));
      ranks_gc = (int*)malloc(nf * sizeof(int));
      ranks_ga = (int*)malloc(nf * sizeof(int));
      //PMPI_Comm_group(worldComm, &worldGroup);
      for(i = 0; i < nf; i++)
        ranks_gf[i] = i;
      OMPI_Group_translate_ranks(group_f, nf, ranks_gf, worldGroup, ranks_gc);
      for(i = 0; i < nf; i++)
      {
        ranks_ga[i] = ranks_gc[i];
        //printf("%d ", ranks_gc[i]);
      }
      //printf("}\n");

      worldComm_ranks = (int*)malloc((nR+nC) * sizeof(int));
      int rankadj[nC+nR];
      int rankadjrep[nC+nR];
      for(i = 0; i < nR+nC; i++)
      {
        rankadjrep[i] = 0;
        rankadj[i] = 0;
        worldComm_ranks[i] = i;
      }
      int nRdiff = 0;
      int nCdiff = 0;
      for (int j = 0; j < nf;j++)
      {
        if(ranks_gc[j] < nC)
        {
          if(cmpToRepMap[ranks_gc[j]] == -1)
          {
            if(!getenv("NON_REP_TOLERANCE") || strcmp(getenv("NON_REP_TOLERANCE"),"1"))
            {
              nonreptol = 0;
              printf("Failed process has no replica... aborting\n");
              OMPI_Abort(comm,err);
            }
            else
            {
              nonreptol = 1;
              nCdiff++;
              worldComm_ranks[ranks_gc[j]] = -1;
              for(i = ranks_gc[j]+1; i < nC+nR; i++)
              {
                if(i < nC) internalranks[i]--;
                rankadj[i]++;
              }
              internalranks[ranks_gc[j]] = -1;
            }
          }
          else
          {
            nonreptol = 0;
            nRdiff++;
            worldComm_ranks[ranks_gc[j]] = worldComm_ranks[cmpToRepMap[ranks_gc[j]]+nC];
            worldComm_ranks[cmpToRepMap[ranks_gc[j]]+nC] = -1;
            for(i = cmpToRepMap[ranks_gc[j]]+nC+1; i < nC+nR; i++)
                rankadjrep[i]++;
            repToCmpMap[cmpToRepMap[ranks_gc[j]]] = -1;
            cmpToRepMap[ranks_gc[j]] = -1;
            ranks_gc[j] = worldComm_ranks[ranks_gc[j]];
          }
        }
        else
        {
          nonreptol = 0;
          nRdiff++;
          worldComm_ranks[ranks_gc[j]] = -1;
          for(i = ranks_gc[j]+1; i < nC+nR; i++)
                rankadjrep[i]++;
          cmpToRepMap[repToCmpMap[ranks_gc[j]-nC]] = -1;
          repToCmpMap[ranks_gc[j]-nC] = -1;
        }
      }
      
      for(i = 0; i < nC+nR; i++)
      {
        if(worldComm_ranks[i] != -1) worldComm_ranks[i - rankadj[i] - rankadjrep[i]] = worldComm_ranks[i];
        if(i < nC) if(cmpToRepMap[i] != -1) cmpToRepMap[i - rankadj[i]] = cmpToRepMap[i] - rankadjrep[cmpToRepMap[i]+nC];
        if(i < nR) if(repToCmpMap[i] != -1) repToCmpMap[i - rankadjrep[i+nC]] = repToCmpMap[i] - rankadj[repToCmpMap[i]];
      }

      nC = nC - nCdiff;
      nR = nR - nRdiff;
      //printf("world comm ranks : ");
      //for(i =0; i<nC+nR;i++) printf("%d ",worldComm_ranks[i]);
      //printf("\n");
			
      OMPI_Group_incl(worldGroup, nC, worldComm_ranks, &cmpGroup);
      OMPI_Group_incl(worldGroup, nR+nC, worldComm_ranks, &worldGroup);
      //PMPIX_Comm_shrink(worldComm,&new_intercomm);
      MPI_COMM_WORLD->oworldComm = new_intercomm;
      //OMPI_Comm_create(MPI_COMM_WORLD->oworldComm, cmpGroup, &(MPI_COMM_WORLD->OMPI_COMM_CMP));
      OMPI_Comm_create(MPI_COMM_WORLD->oworldComm, worldGroup, &(MPI_COMM_WORLD->oworldComm));
      //if(nR > 0) OMPI_Intercomm_create(MPI_COMM_WORLD->OMPI_COMM_CMP, 0, MPI_COMM_WORLD->oworldComm, nC, 200, &(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM));
      //else MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM = OMPI_COMM_NULL;
			//OMPI_Comm_rank(MPI_COMM_WORLD->oworldComm, &r);
			//if(cmpToRepMap[r] != -1) colorinternal = OMPI_UNDEFINED;
			//else colorinternal = 1;
			//OMPI_Comm_split (MPI_COMM_WORLD->OMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP));
			//if(MPI_COMM_WORLD->OMPI_CMP_NO_REP != OMPI_COMM_NULL && nR > 0) OMPI_Intercomm_create (MPI_COMM_WORLD->OMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->oworldComm, nC, 201, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM));
			//else MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM = OMPI_COMM_NULL;
      //OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_COMM_CMP,"OMPI_COMM_CMP");
      OMPI_Comm_set_name (MPI_COMM_WORLD->oworldComm,"oworldComm");
      //if(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL) OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM,"OMPI_CMP_REP_INTERCOMM");
      
      //printf("CMP Starting EMPI comm creation\n");
      EMPI_Comm_group(MPI_COMM_WORLD->eworldComm, &eworldGroup);
      EMPI_Group_incl(eworldGroup, nC, worldComm_ranks, &ecmpGroup);
      EMPI_Group_incl(eworldGroup, nR+nC, worldComm_ranks, &eworldGroup);
      EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm, ecmpGroup,0,&(MPI_COMM_WORLD->EMPI_COMM_CMP));
      EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm, eworldGroup,0,&(MPI_COMM_WORLD->eworldComm));
      if(nR > 0) EMPI_Intercomm_create(MPI_COMM_WORLD->EMPI_COMM_CMP, 0, MPI_COMM_WORLD->eworldComm, nC, 200, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
      else MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM = EMPI_COMM_NULL;
			EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm, &r);
			if(cmpToRepMap[r] != -1) colorinternal = EMPI_UNDEFINED;
			else colorinternal = 1;
			EMPI_Comm_split (MPI_COMM_WORLD->EMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
			if(MPI_COMM_WORLD->EMPI_CMP_NO_REP != EMPI_COMM_NULL) EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->eworldComm, nC, 201, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
			else MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM = EMPI_COMM_NULL;
      EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_COMM_CMP,"EMPI_COMM_CMP");
      EMPI_Comm_set_name (MPI_COMM_WORLD->eworldComm,"eworldComm");
      if(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM != EMPI_COMM_NULL) EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,"EMPI_CMP_REP_INTERCOMM");      
    }
    else if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL)
    {
      /*OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_COMM_REP,&flag);
      if(!flag)
      {
        printf("revoking REP\n");
        OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_COMM_REP);
      }
      if(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL)
      {
        OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM,&flag);
        if(!flag)
        {
          printf("revoking INTER\n");
          OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM);
        }
      }
			if(MPI_COMM_WORLD->OMPI_CMP_NO_REP != OMPI_COMM_NULL)
			{
				OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_CMP_NO_REP,&flag);
				if(!flag)
				{
					printf("revoking CMP NO REP\n");
					OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_CMP_NO_REP);
				}
			}
			if(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM != OMPI_COMM_NULL)
			{
				OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM,&flag);
				if(!flag)
				{
					printf("revoking CMP NO REP INTER\n");
					OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM);
				}
			}*/
      OMPIX_Comm_is_revoked(MPI_COMM_WORLD->oworldComm,&flag);
      if(!flag)
      {
        //printf("revoking oworld\n");
        OMPIX_Comm_revoke(MPI_COMM_WORLD->oworldComm);
      }
      printf("before shrink REP wrank %d\n",r);
      OMPIX_Comm_shrink(MPI_COMM_WORLD->oworldComm,&new_intercomm);
      printf("after shrink REP wrank %d\n",r);
      OMPI_Comm_group(MPI_COMM_WORLD->oworldComm, &worldGroup);
      OMPI_Comm_group(new_intercomm, &group_c);
      OMPI_Group_difference(worldGroup,group_c,&group_f);

      //PMPIX_Comm_failure_ack(worldComm);
      //PMPIX_Comm_failure_get_acked(worldComm, &group_f);

      OMPI_Group_size(group_f, &nf);
      if(comm == MPI_COMM_WORLD->OMPI_COMM_REP)
      {
        //printf("AT OMPI_COMM_REP: World rank %d: Notified of error %s. %d found dead: { ", r, errstr, nf);
      }
      else if(comm == MPI_COMM_WORLD->oworldComm)
      {
        //printf("AT oworldComm: World rank %d: Notified of error %s. %d found dead: { ", r, errstr, nf);
      }
      else if(comm == MPI_COMM_WORLD->OMPI_COMM_CMP)
      {
        //printf("AT OMPI_COMM_CMP: World rank %d: Notified of error %s. %d found dead: { ", r , errstr, nf);
      }
      else if(comm == MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM)
      {
        //printf("AT OMPI_CMP_REP_INTERCOMM: World rank %d: Notified of error %s. %d found dead: { ", r , errstr, nf);
      }
      ranks_gf = (int*)malloc(nf * sizeof(int));
      ranks_gc = (int*)malloc(nf * sizeof(int));
      ranks_ga = (int*)malloc(nf * sizeof(int));
      //PMPI_Comm_group(worldComm, &worldGroup);
      for(i = 0; i < nf; i++)
        ranks_gf[i] = i;
      OMPI_Group_translate_ranks(group_f, nf, ranks_gf, worldGroup, ranks_gc);
      for(i = 0; i < nf; i++)
      {
        ranks_ga[i] = ranks_gc[i];
        //printf("%d ", ranks_gc[i]);
      }
      //printf("}\n");

      int myreprank;
			EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_REP,&myreprank);
      worldComm_ranks = (int*)malloc((nR+nC) * sizeof(int));
      int rankadj[nC+nR];
      int rankadjrep[nC+nR];
      //if(myreprank == cmpToRepMap[ranks_ga[j]]) switchingToCmp = true;
      for(i = 0; i < nR+nC; i++)
      {
        rankadj[i] = 0;
        rankadjrep[i] = 0;
        worldComm_ranks[i] = i;
      }
      int nRdiff = 0;
      int nCdiff = 0;
      for (int j = 0; j < nf;j++)
      {
        if(ranks_gc[j] < nC)
        {
          if(cmpToRepMap[ranks_gc[j]] == -1)
          {
            if(!getenv("NON_REP_TOLERANCE") || strcmp(getenv("NON_REP_TOLERANCE"),"1"))
            {
              nonreptol = 0;
              printf("Failed process has no replica... aborting\n");
              OMPI_Abort(comm,err);
            }
            else
            {
              nonreptol = 1;
              nCdiff++;
              worldComm_ranks[ranks_gc[j]] = -1;
              for(i = ranks_gc[j]+1; i < nC+nR; i++)
              {
                if(i < nC) internalranks[i]--;
                rankadj[i]++;
              }
              internalranks[ranks_gc[j]] = -1;
            }
          }
          else
          {
            nonreptol = 0;
            nRdiff++;
            if(myreprank == cmpToRepMap[ranks_ga[j]]) switchingToCmp = true;
            worldComm_ranks[ranks_gc[j]] = worldComm_ranks[cmpToRepMap[ranks_gc[j]]+nC];
            worldComm_ranks[cmpToRepMap[ranks_gc[j]]+nC] = -1;
            for(i = cmpToRepMap[ranks_gc[j]]+nC+1; i < nC+nR; i++)
                rankadjrep[i]++;
            repToCmpMap[cmpToRepMap[ranks_gc[j]]] = -1;
            cmpToRepMap[ranks_gc[j]] = -1;
            ranks_gc[j] = worldComm_ranks[ranks_gc[j]];
          }
        }
        else
        {
          nonreptol = 0;
          nRdiff++;
          worldComm_ranks[ranks_gc[j]] = -1;
          for(i = ranks_gc[j]+1; i < nC+nR; i++)
                rankadjrep[i]++;
          cmpToRepMap[repToCmpMap[ranks_gc[j]-nC]] = -1;
          repToCmpMap[ranks_gc[j]-nC] = -1;
        }
      }

      for(i = 0; i < nC+nR; i++)
      {
        if(worldComm_ranks[i] != -1) worldComm_ranks[i - rankadj[i] - rankadjrep[i]] = worldComm_ranks[i];
        if(i < nC) if(cmpToRepMap[i] != -1) cmpToRepMap[i - rankadj[i]] = cmpToRepMap[i] - rankadjrep[cmpToRepMap[i]+nC];
        if(i < nR) if(repToCmpMap[i] != -1) repToCmpMap[i - rankadjrep[i+nC]] = repToCmpMap[i] - rankadj[repToCmpMap[i]];
      }

      nC = nC - nCdiff;
      nR = nR - nRdiff;

      if(switchingToCmp) OMPI_Group_incl(worldGroup, nC, worldComm_ranks, &cmpGroup);
      else OMPI_Group_incl(worldGroup, nR, &worldComm_ranks[nC], &repGroup);
      OMPI_Group_incl(worldGroup, nR+nC, worldComm_ranks, &worldGroup);

      //PMPIX_Comm_shrink(worldComm,&new_intercomm);
      MPI_COMM_WORLD->oworldComm = new_intercomm;


      if(switchingToCmp)
      {
        //OMPI_COMM_REP = OMPI_COMM_NULL;
        //OMPI_Comm_free(&(MPI_COMM_WORLD->OMPI_COMM_REP));
        //OMPI_Comm_create(MPI_COMM_WORLD->oworldComm, cmpGroup, &(MPI_COMM_WORLD->OMPI_COMM_CMP));
        OMPI_Comm_create(MPI_COMM_WORLD->oworldComm, worldGroup, &(MPI_COMM_WORLD->oworldComm));
        //if(nR > 0) OMPI_Intercomm_create(MPI_COMM_WORLD->OMPI_COMM_CMP, 0, MPI_COMM_WORLD->oworldComm, nC, 200, &(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM));
        //else MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM = OMPI_COMM_NULL;
				//OMPI_Comm_rank(MPI_COMM_WORLD->oworldComm, &r);
				//if(cmpToRepMap[r] != -1) colorinternal = OMPI_UNDEFINED;
				//else colorinternal = 1;
				//OMPI_Comm_split (MPI_COMM_WORLD->OMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP));
				//if(MPI_COMM_WORLD->OMPI_CMP_NO_REP != OMPI_COMM_NULL && nR > 0) OMPI_Intercomm_create (MPI_COMM_WORLD->OMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->oworldComm, nC, 201, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM));
				//else MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM = OMPI_COMM_NULL;
        //OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_COMM_CMP,"OMPI_COMM_CMP");
        OMPI_Comm_set_name (MPI_COMM_WORLD->oworldComm,"oworldComm");
        //if(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL) OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM,"OMPI_CMP_REP_INTERCOMM");
        
        //printf("REP (now CMP) Starting EMPI comm creation\n");
      	EMPI_Comm_group(MPI_COMM_WORLD->eworldComm, &eworldGroup);
      	EMPI_Group_incl(eworldGroup, nC, worldComm_ranks, &ecmpGroup);
      	EMPI_Group_incl(eworldGroup, nR+nC, worldComm_ranks, &eworldGroup);
        //EMPI_Comm_free(&EMPI_COMM_REP);
      	MPI_COMM_WORLD->EMPI_COMM_REP = EMPI_COMM_NULL;
      	EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm, ecmpGroup,0,&(MPI_COMM_WORLD->EMPI_COMM_CMP));
      	EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm, eworldGroup,0,&(MPI_COMM_WORLD->eworldComm));
      	if(nR > 0) EMPI_Intercomm_create(MPI_COMM_WORLD->EMPI_COMM_CMP, 0, MPI_COMM_WORLD->eworldComm, nC, 200, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
      	else MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM = EMPI_COMM_NULL;
				EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm, &r);
				if(cmpToRepMap[r] != -1) colorinternal = EMPI_UNDEFINED;
				else colorinternal = 1;
				EMPI_Comm_split (MPI_COMM_WORLD->EMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
				if(MPI_COMM_WORLD->EMPI_CMP_NO_REP != EMPI_COMM_NULL) EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->eworldComm, nC, 201, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
				else MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM = EMPI_COMM_NULL;
      	EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_COMM_CMP,"EMPI_COMM_CMP");
      	EMPI_Comm_set_name (MPI_COMM_WORLD->eworldComm,"eworldComm");
      	if(EMPI_CMP_REP_INTERCOMM != EMPI_COMM_NULL) EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,"EMPI_CMP_REP_INTERCOMM");
        //last_collective->ecomm = &EMPI_COMM_CMP;
        //last_collective->ocomm = &OMPI_COMM_CMP;
      }
      else
      {
        //OMPI_Comm_create(MPI_COMM_WORLD->oworldComm, repGroup, &(MPI_COMM_WORLD->OMPI_COMM_REP));
        OMPI_Comm_create(MPI_COMM_WORLD->oworldComm, worldGroup, &(MPI_COMM_WORLD->oworldComm));
        //if(nR > 0) OMPI_Intercomm_create(MPI_COMM_WORLD->OMPI_COMM_REP, 0, MPI_COMM_WORLD->oworldComm, 0, 200, &(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM));
        //else MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM = OMPI_COMM_NULL;
				//if(nR > 0 && nC > nR)
				//{
				//	int rleader = nR;
				//	for(i = 0; i < nC; i++)
				//	{
				//		if(cmpToRepMap[i] == -1)
				//		{
				//			rleader = i;
				//			break;
				//		}
				//	}
				//	OMPI_Intercomm_create (MPI_COMM_WORLD->OMPI_COMM_REP, 0, MPI_COMM_WORLD->oworldComm, rleader, 201, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM));
				//}
				//else MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM = OMPI_COMM_NULL;
        //OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_COMM_REP,"OMPI_COMM_REP");
        OMPI_Comm_set_name (MPI_COMM_WORLD->oworldComm,"oworldComm");
        //if(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL) OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM,"OMPI_CMP_REP_INTERCOMM");
        
        //printf("REP Starting EMPI comm creation\n");
      	EMPI_Comm_group(MPI_COMM_WORLD->eworldComm, &eworldGroup);
      	EMPI_Group_incl(eworldGroup, nR, &worldComm_ranks[nC], &erepGroup);
      	EMPI_Group_incl(eworldGroup, nR+nC, worldComm_ranks, &eworldGroup);
      	EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm, erepGroup,0,&(MPI_COMM_WORLD->EMPI_COMM_REP));
      	EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm, eworldGroup,0,&(MPI_COMM_WORLD->eworldComm));
      	if(nR > 0) EMPI_Intercomm_create(MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, 0, 200, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
      	else MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM = EMPI_COMM_NULL;
				if(nR > 0 && nC > nR)
				{
					int rleader = nR;
					for(i = 0; i < nC; i++)
					{
						if(cmpToRepMap[i] == -1)
						{
							rleader = i;
							break;
						}
					}
					EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, rleader, 201, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
				}
				else MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM = EMPI_COMM_NULL;
      	EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_COMM_REP,"EMPI_COMM_REP");
      	EMPI_Comm_set_name (MPI_COMM_WORLD->eworldComm,"eworldComm");
      	if(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM != EMPI_COMM_NULL) EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,"EMPI_CMP_REP_INTERCOMM");
      }
    }
  }
  else if(eclass == OMPIX_ERR_REVOKED)
  {
    if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
    {
			/*OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_COMM_CMP,&flag);
      if(!flag)
      {
        OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_COMM_CMP);
      }
      if(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL)
      {
        OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM,&flag);
        if(!flag)
        {
          OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM);
        }
      }
			if(MPI_COMM_WORLD->OMPI_CMP_NO_REP != OMPI_COMM_NULL)
			{
				OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_CMP_NO_REP,&flag);
				if(!flag)
				{
					OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_CMP_NO_REP);
				}
			}
			if(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM != OMPI_COMM_NULL)
			{
				OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM,&flag);
				if(!flag)
				{
					OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM);
				}
			}*/
      OMPIX_Comm_is_revoked(MPI_COMM_WORLD->oworldComm,&flag);
      if(!flag)
      {
        OMPIX_Comm_revoke(MPI_COMM_WORLD->oworldComm);
      }
			printf("before shrink CMP wrank %d\n",r);
      OMPIX_Comm_shrink(MPI_COMM_WORLD->oworldComm,&new_intercomm);
			printf("after shrink CMP wrank %d\n",r);
      OMPI_Comm_group(MPI_COMM_WORLD->oworldComm, &worldGroup);
      OMPI_Comm_group(new_intercomm, &group_c);
      OMPI_Group_difference(worldGroup,group_c,&group_f);
      //PMPIX_Comm_failure_ack(worldComm);
      //PMPIX_Comm_failure_get_acked(worldComm, &group_f);
      OMPI_Group_size(group_f, &nf);
      //PMPI_Comm_group(worldComm, &worldGroup);
      //if(comm == MPI_COMM_WORLD->OMPI_COMM_CMP) printf("AT OMPI_COMM_CMP, World rank %d reported: %s, %d processes failed { ",r,errstr,nf);
      //else if(comm == MPI_COMM_WORLD->OMPI_COMM_REP) printf("AT OMPI_COMM_REP, World rank %d reported: %s, %d processes failed { ",r,errstr,nf);
      //else if(comm == MPI_COMM_WORLD->oworldComm) printf("AT oworldComm, World rank %d reported: %s, %d processes failed { ",r,errstr,nf);
      //else if(comm == MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM) printf("AT OMPI_CMP_REP_INTERCOMM, World rank %d reported: %s, %d processes failed { ",r,errstr,nf);
      ranks_gf = (int*)malloc(nf * sizeof(int));
      ranks_gc = (int*)malloc(nf * sizeof(int));
      ranks_ga = (int*)malloc(nf * sizeof(int));
      for(i = 0; i < nf; i++)
        ranks_gf[i] = i;
      OMPI_Group_translate_ranks(group_f, nf, ranks_gf, worldGroup, ranks_gc);
      for(i = 0; i < nf; i++)
      {
        ranks_ga[i] = ranks_gc[i];
        //printf("%d ", ranks_gc[i]);
      }
      //printf("}\n");
      for(i = 0; i < nf; i++)
        ranks_ga[i] = ranks_gc[i];

      worldComm_ranks = (int*)malloc((nR+nC) * sizeof(int));
      int rankadj[nC+nR];
      int rankadjrep[nC+nR];
      for(i = 0; i < nR+nC; i++)
      {
        rankadj[i] = 0;
        rankadjrep[i] = 0;
        worldComm_ranks[i] = i;
      }
      int nRdiff = 0;
      int nCdiff = 0;
      for (int j = 0; j < nf;j++)
      {
        if(ranks_gc[j] < nC)
        {
          if(cmpToRepMap[ranks_gc[j]] == -1)
          {
            if(!getenv("NON_REP_TOLERANCE") || strcmp(getenv("NON_REP_TOLERANCE"),"1"))
            {
              nonreptol = 0;
              //printf("Failed process has no replica... aborting\n");
              OMPI_Abort(comm,err);
            }
            else
            {
              nonreptol = 1;
              nCdiff++;
              worldComm_ranks[ranks_gc[j]] = -1;
              for(i = ranks_gc[j]+1; i < nC+nR; i++)
              {
                if(i < nC) internalranks[i]--;
                rankadj[i]++;
              }
              internalranks[ranks_gc[j]] = -1;
            }
          }
          else
          {
            nonreptol = 0;
            nRdiff++;
            worldComm_ranks[ranks_gc[j]] = worldComm_ranks[cmpToRepMap[ranks_gc[j]]+nC];
            worldComm_ranks[cmpToRepMap[ranks_gc[j]]+nC] = -1;
            for(i = cmpToRepMap[ranks_gc[j]]+nC+1; i < nC+nR; i++)
                rankadjrep[i]++;
            repToCmpMap[cmpToRepMap[ranks_gc[j]]] = -1;
            cmpToRepMap[ranks_gc[j]] = -1;
            ranks_gc[j] = worldComm_ranks[ranks_gc[j]];
          }
        }
        else
        {
          nonreptol = 0;
          nRdiff++;
          worldComm_ranks[ranks_gc[j]] = -1;
          for(i = ranks_gc[j]+1; i < nC+nR; i++)
                rankadjrep[i]++;
          cmpToRepMap[repToCmpMap[ranks_gc[j]-nC]] = -1;
          repToCmpMap[ranks_gc[j]-nC] = -1;
        }
      }

      for(i = 0; i < nC+nR; i++)
      {
        if(worldComm_ranks[i] != -1) worldComm_ranks[i - rankadj[i] - rankadjrep[i]] = worldComm_ranks[i];
        if(i < nC) if(cmpToRepMap[i] != -1) cmpToRepMap[i - rankadj[i]] = cmpToRepMap[i] - rankadjrep[cmpToRepMap[i]+nC];
        if(i < nR) if(repToCmpMap[i] != -1) repToCmpMap[i - rankadjrep[i+nC]] = repToCmpMap[i] - rankadj[repToCmpMap[i]];
      }

      nC = nC - nCdiff;
      nR = nR - nRdiff;

      OMPI_Group_incl(worldGroup, nC, worldComm_ranks, &cmpGroup);
      OMPI_Group_incl(worldGroup, nR+nC, worldComm_ranks, &worldGroup);
      MPI_COMM_WORLD->oworldComm = new_intercomm;

      //OMPI_Comm_create(MPI_COMM_WORLD->oworldComm, cmpGroup, &(MPI_COMM_WORLD->OMPI_COMM_CMP));
      OMPI_Comm_create(MPI_COMM_WORLD->oworldComm, worldGroup, &(MPI_COMM_WORLD->oworldComm));
      //if(nR > 0) OMPI_Intercomm_create(MPI_COMM_WORLD->OMPI_COMM_CMP, 0, MPI_COMM_WORLD->oworldComm, nC, 200, &(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM));
      //else MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM = OMPI_COMM_NULL;
			//OMPI_Comm_rank(MPI_COMM_WORLD->oworldComm, &r);
			//if(cmpToRepMap[r] != -1) colorinternal = OMPI_UNDEFINED;
			//else colorinternal = 1;
			//OMPI_Comm_split (MPI_COMM_WORLD->OMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP));
			//if(MPI_COMM_WORLD->OMPI_CMP_NO_REP != OMPI_COMM_NULL && nR > 0) OMPI_Intercomm_create (MPI_COMM_WORLD->OMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->oworldComm, nC, 201, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM));
			//else MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM = OMPI_COMM_NULL;
     // OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_COMM_CMP,"OMPI_COMM_CMP");
      OMPI_Comm_set_name (MPI_COMM_WORLD->oworldComm,"oworldComm");
      //if(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL) OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM,"OMPI_CMP_REP_INTERCOMM");
      
      //printf("CMP Starting EMPI comm creation\n");
      EMPI_Comm_group(MPI_COMM_WORLD->eworldComm, &eworldGroup);
      EMPI_Group_incl(eworldGroup, nC, worldComm_ranks, &ecmpGroup);
      EMPI_Group_incl(eworldGroup, nR+nC, worldComm_ranks, &eworldGroup);
      EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm, ecmpGroup,0,&(MPI_COMM_WORLD->EMPI_COMM_CMP));
      EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm, eworldGroup,0,&(MPI_COMM_WORLD->eworldComm));
      if(nR > 0) EMPI_Intercomm_create(MPI_COMM_WORLD->EMPI_COMM_CMP, 0, MPI_COMM_WORLD->eworldComm, nC, 200, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
      else MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM = EMPI_COMM_NULL;
			EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm, &r);
			if(cmpToRepMap[r] != -1) colorinternal = EMPI_UNDEFINED;
			else colorinternal = 1;
			EMPI_Comm_split (MPI_COMM_WORLD->EMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
			if(MPI_COMM_WORLD->EMPI_CMP_NO_REP != EMPI_COMM_NULL) EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->eworldComm, nC, 201, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
			else MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM = EMPI_COMM_NULL;
      EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_COMM_CMP,"EMPI_COMM_CMP");
      EMPI_Comm_set_name (MPI_COMM_WORLD->eworldComm,"eworldComm");
      if(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM != EMPI_COMM_NULL) EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,"EMPI_CMP_REP_INTERCOMM");
    }
    else if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL)
    {
			/*OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_COMM_REP,&flag);
      if(!flag)
      {
        OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_COMM_REP);
      }
      if(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL)
      {
        OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM,&flag);
        if(!flag)
        {
          OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM);
        }
      }
			if(MPI_COMM_WORLD->OMPI_CMP_NO_REP != OMPI_COMM_NULL)
			{
				OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_CMP_NO_REP,&flag);
				if(!flag)
				{
					OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_CMP_NO_REP);
				}
			}
			if(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM != OMPI_COMM_NULL)
			{
				OMPIX_Comm_is_revoked(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM,&flag);
				if(!flag)
				{
					OMPIX_Comm_revoke(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM);
				}
			}*/
      OMPIX_Comm_is_revoked(MPI_COMM_WORLD->oworldComm,&flag);
      if(!flag)
      {
        OMPIX_Comm_revoke(MPI_COMM_WORLD->oworldComm);
      }
			printf("before shrink REP wrank %d\n",r);
      OMPIX_Comm_shrink(MPI_COMM_WORLD->oworldComm,&new_intercomm);
			printf("after shrink REP wrank %d\n",r);
      OMPI_Comm_group(MPI_COMM_WORLD->oworldComm, &worldGroup);
      OMPI_Comm_group(new_intercomm, &group_c);
      OMPI_Group_difference(worldGroup,group_c,&group_f);
      //PMPIX_Comm_failure_ack(worldComm);
      //PMPIX_Comm_failure_get_acked(worldComm, &group_f);
      OMPI_Group_size(group_f, &nf);
      //PMPI_Comm_group(worldComm, &worldGroup);
      //if(comm == MPI_COMM_WORLD->OMPI_COMM_CMP) printf("AT OMPI_COMM_CMP, World rank %d reported: %s, %d processes failed { ",r,errstr,nf);
      //else if(comm == MPI_COMM_WORLD->OMPI_COMM_REP) printf("AT OMPI_COMM_REP, World rank %d reported: %s, %d processes failed { ",r,errstr,nf);
      //else if(comm == MPI_COMM_WORLD->oworldComm) printf("AT oworldComm, World rank %d reported: %s, %d processes failed { ",r,errstr,nf);
      //else if(comm == MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM) printf("AT OMPI_CMP_REP_INTERCOMM, World rank %d reported: %s, %d processes failed { ",r,errstr,nf);
      ranks_gf = (int*)malloc(nf * sizeof(int));
      ranks_gc = (int*)malloc(nf * sizeof(int));
      ranks_ga = (int*)malloc(nf * sizeof(int));
      for(i = 0; i < nf; i++)
        ranks_gf[i] = i;
      OMPI_Group_translate_ranks(group_f, nf, ranks_gf, worldGroup, ranks_gc);
      for(i = 0; i < nf; i++)
      {
        ranks_ga[i] = ranks_gc[i];
        //printf("%d ", ranks_gc[i]);
      }
      //printf("}\n");
      for(i = 0; i < nf; i++)
        ranks_ga[i] = ranks_gc[i];
      int myreprank;
      EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_REP,&myreprank);
      worldComm_ranks = (int*)malloc((nR+nC) * sizeof(int));
      int rankadj[nC+nR];
      int rankadjrep[nC+nR];
      for(i = 0; i < nR+nC; i++)
      {
        rankadj[i] = 0;
        rankadjrep[i] = 0;
        worldComm_ranks[i] = i;
      }
      int nRdiff = 0;
      int nCdiff = 0;
      for (int j = 0; j < nf;j++)
      {
        if(ranks_gc[j] < nC)
        {
          if(cmpToRepMap[ranks_gc[j]] == -1)
          {
            if(!getenv("NON_REP_TOLERANCE") || strcmp(getenv("NON_REP_TOLERANCE"),"1"))
            {
              nonreptol = 0;
              printf("Failed process has no replica... aborting\n");
              OMPI_Abort(comm,err);
            }
            else
            {
              nonreptol = 1;
              nCdiff++;
              worldComm_ranks[ranks_gc[j]] = -1;
              for(i = ranks_gc[j]+1; i < nC+nR; i++)
              {
                if(i < nC) internalranks[i]--;
                rankadj[i]++;
              }
              internalranks[ranks_gc[j]] = -1;
            }
          }
          else
          {
            nonreptol = 0;
            nRdiff++;
            if(myreprank == cmpToRepMap[ranks_ga[j]]) switchingToCmp = true;
            worldComm_ranks[ranks_gc[j]] = worldComm_ranks[cmpToRepMap[ranks_gc[j]]+nC];
            worldComm_ranks[cmpToRepMap[ranks_gc[j]]+nC] = -1;
            for(i = cmpToRepMap[ranks_gc[j]]+nC+1; i < nC+nR; i++)
                rankadjrep[i]++;
            repToCmpMap[cmpToRepMap[ranks_gc[j]]] = -1;
            cmpToRepMap[ranks_gc[j]] = -1;
            ranks_gc[j] = worldComm_ranks[ranks_gc[j]];
          }
        }
        else
        {
          nonreptol = 0;
          nRdiff++;
          worldComm_ranks[ranks_gc[j]] = -1;
          for(i = ranks_gc[j]+1; i < nC+nR; i++)
                rankadjrep[i]++;
          cmpToRepMap[repToCmpMap[ranks_gc[j]-nC]] = -1;
          repToCmpMap[ranks_gc[j]-nC] = -1;
        }
      }

      for(i = 0; i < nC+nR; i++)
      {
        if(worldComm_ranks[i] != -1) worldComm_ranks[i - rankadj[i] - rankadjrep[i]] = worldComm_ranks[i];
        if(i < nC) if(cmpToRepMap[i] != -1) cmpToRepMap[i - rankadj[i]] = cmpToRepMap[i] - rankadjrep[cmpToRepMap[i]+nC];
        if(i < nR) if(repToCmpMap[i] != -1) repToCmpMap[i - rankadjrep[i+nC]] = repToCmpMap[i] - rankadj[repToCmpMap[i]];
      }

      nC = nC - nCdiff;
      nR = nR - nRdiff;

      if(switchingToCmp) OMPI_Group_incl(worldGroup, nC, worldComm_ranks, &cmpGroup);
      else OMPI_Group_incl(worldGroup, nR, &worldComm_ranks[nC], &repGroup);
      OMPI_Group_incl(worldGroup, nR+nC, worldComm_ranks, &worldGroup);
      //PMPIX_Comm_shrink(MPI_COMM_REP,&new_comm);
      //MPI_COMM_REP = new_comm;
      //PMPIX_Comm_shrink(worldComm,&new_intercomm);
      MPI_COMM_WORLD->oworldComm = new_intercomm;

      if(switchingToCmp)
      {
        //OMPI_COMM_REP = OMPI_COMM_NULL;
        //OMPI_Comm_free(&(MPI_COMM_WORLD->OMPI_COMM_REP));
        //OMPI_Comm_create(MPI_COMM_WORLD->oworldComm, cmpGroup, &(MPI_COMM_WORLD->OMPI_COMM_CMP));
        OMPI_Comm_create(MPI_COMM_WORLD->oworldComm, worldGroup, &(MPI_COMM_WORLD->oworldComm));
        //if(nR > 0) OMPI_Intercomm_create(MPI_COMM_WORLD->OMPI_COMM_CMP, 0, MPI_COMM_WORLD->oworldComm, nC, 200, &(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM));
        //else MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM = OMPI_COMM_NULL;
				//OMPI_Comm_rank(MPI_COMM_WORLD->oworldComm, &r);
				//if(cmpToRepMap[r] != -1) colorinternal = OMPI_UNDEFINED;
				//else colorinternal = 1;
				//OMPI_Comm_split (MPI_COMM_WORLD->OMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP));
				//if(MPI_COMM_WORLD->OMPI_CMP_NO_REP != OMPI_COMM_NULL && nR > 0) OMPI_Intercomm_create (MPI_COMM_WORLD->OMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->oworldComm, nC, 201, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM));
				//else MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM = OMPI_COMM_NULL;
        //OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_COMM_CMP,"OMPI_COMM_CMP");
        OMPI_Comm_set_name (MPI_COMM_WORLD->oworldComm,"oworldComm");
        //if(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL) OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM,"OMPI_CMP_REP_INTERCOMM");
        
        //printf("REP (now CMP) Starting EMPI comm creation\n");
      	EMPI_Comm_group(MPI_COMM_WORLD->eworldComm, &eworldGroup);
      	EMPI_Group_incl(eworldGroup, nC, worldComm_ranks, &ecmpGroup);
      	EMPI_Group_incl(eworldGroup, nR+nC, worldComm_ranks, &eworldGroup);
      	MPI_COMM_WORLD->EMPI_COMM_REP = EMPI_COMM_NULL;
        //EMPI_Comm_free(&EMPI_COMM_REP);
      	EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm, ecmpGroup,0,&(MPI_COMM_WORLD->EMPI_COMM_CMP));
      	EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm, eworldGroup,0,&(MPI_COMM_WORLD->eworldComm));
      	if(nR > 0) EMPI_Intercomm_create(MPI_COMM_WORLD->EMPI_COMM_CMP, 0, MPI_COMM_WORLD->eworldComm, nC, 200, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
      	else MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM = EMPI_COMM_NULL;
				EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm, &r);
				if(cmpToRepMap[r] != -1) colorinternal = EMPI_UNDEFINED;
				else colorinternal = 1;
				EMPI_Comm_split (MPI_COMM_WORLD->EMPI_COMM_CMP, colorinternal, 0, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP));
				if(MPI_COMM_WORLD->EMPI_CMP_NO_REP != EMPI_COMM_NULL) EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_CMP_NO_REP, 0, MPI_COMM_WORLD->eworldComm, nC, 201, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
				else MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM = EMPI_COMM_NULL;
      	EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_COMM_CMP,"EMPI_COMM_CMP");
      	EMPI_Comm_set_name (MPI_COMM_WORLD->eworldComm,"eworldComm");
      	if(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM != EMPI_COMM_NULL) EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,"EMPI_CMP_REP_INTERCOMM");
        //last_collective->ecomm = &EMPI_COMM_CMP;
        //last_collective->ocomm = &OMPI_COMM_CMP;
      }
      else
      {
        //OMPI_Comm_create(MPI_COMM_WORLD->oworldComm, repGroup, &(MPI_COMM_WORLD->OMPI_COMM_REP));
        OMPI_Comm_create(MPI_COMM_WORLD->oworldComm, worldGroup, &(MPI_COMM_WORLD->oworldComm));
        //if(nR > 0) OMPI_Intercomm_create(MPI_COMM_WORLD->OMPI_COMM_REP, 0, MPI_COMM_WORLD->oworldComm, 0, 200, &(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM));
        //else MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM = OMPI_COMM_NULL;
				//if(nR > 0 && nC > nR)
				//{
				//	int rleader = nR;
				//	for(i = 0; i < nC; i++)
				//	{
				//		if(cmpToRepMap[i] == -1)
				//		{
				//			rleader = i;
				//			break;
				//		}
				//	}
				//	OMPI_Intercomm_create (MPI_COMM_WORLD->OMPI_COMM_REP, 0, MPI_COMM_WORLD->oworldComm, rleader, 201, &(MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM));
				//}
				//else MPI_COMM_WORLD->OMPI_CMP_NO_REP_INTERCOMM = OMPI_COMM_NULL;
        //OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_COMM_REP,"OMPI_COMM_REP");
        OMPI_Comm_set_name (MPI_COMM_WORLD->oworldComm,"oworldComm");
        //if(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL) OMPI_Comm_set_name (MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM,"OMPI_CMP_REP_INTERCOMM");
        
        //printf("REP Starting EMPI comm creation\n");
      	EMPI_Comm_group(MPI_COMM_WORLD->eworldComm, &eworldGroup);
      	EMPI_Group_incl(eworldGroup, nR, &worldComm_ranks[nC], &erepGroup);
      	EMPI_Group_incl(eworldGroup, nR+nC, worldComm_ranks, &eworldGroup);
      	EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm, erepGroup, 0, &(MPI_COMM_WORLD->EMPI_COMM_REP));
      	EMPI_Comm_create_group(MPI_COMM_WORLD->eworldComm, eworldGroup,0,&(MPI_COMM_WORLD->eworldComm));
      	if(nR > 0) EMPI_Intercomm_create(MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, 0, 200, &(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
      	else MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM = EMPI_COMM_NULL;
				if(nR > 0 && nC > nR)
				{
					int rleader = nR;
					for(i = 0; i < nC; i++)
					{
						if(cmpToRepMap[i] == -1)
						{
							rleader = i;
							break;
						}
					}
					EMPI_Intercomm_create (MPI_COMM_WORLD->EMPI_COMM_REP, 0, MPI_COMM_WORLD->eworldComm, rleader, 201, &(MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM));
				}
				else MPI_COMM_WORLD->EMPI_CMP_NO_REP_INTERCOMM = EMPI_COMM_NULL;
      	EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_COMM_REP,"EMPI_COMM_REP");
      	EMPI_Comm_set_name (MPI_COMM_WORLD->eworldComm,"eworldComm");
      	if(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM != EMPI_COMM_NULL) EMPI_Comm_set_name (MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM,"EMPI_CMP_REP_INTERCOMM");
      }
    }
  }
  else
  {
    printf("Aborting due to bad error %d",err);
    OMPI_Abort(comm,err);
  }
	/*printf("non null checks %d %d %d %d\n",eworldComm != EMPI_COMM_NULL,EMPI_COMM_CMP != EMPI_COMM_NULL,EMPI_COMM_REP != EMPI_COMM_NULL,EMPI_CMP_REP_INTERCOMM != EMPI_COMM_NULL);
	if(eworldComm != EMPI_COMM_NULL) EMPI_Comm_dup(eworldComm,&(MPI_COMM_WORLD->eworldComm));
	//else EMPI_Comm_free(&(MPI_COMM_WORLD->eworldComm));
	if(EMPI_COMM_CMP != EMPI_COMM_NULL) EMPI_Comm_dup(EMPI_COMM_CMP,&(MPI_COMM_WORLD->EMPI_COMM_CMP));
	//else EMPI_Comm_free(&(MPI_COMM_WORLD->EMPI_COMM_CMP));
	if(EMPI_COMM_REP != EMPI_COMM_NULL) EMPI_Comm_dup(EMPI_COMM_REP,&(MPI_COMM_WORLD->EMPI_COMM_REP));
	//else if(switchingToCmp && MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) MPI_COMM_WORLD->EMPI_COMM_REP = EMPI_COMM_NULL;
	if(EMPI_CMP_REP_INTERCOMM != EMPI_COMM_NULL) EMPI_Comm_dup(EMPI_CMP_REP_INTERCOMM,&(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM));
	//else if(MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM != EMPI_COMM_NULL) MPI_COMM_WORLD->EMPI_CMP_REP_INTERCOMM = EMPI_COMM_NULL;
	if(oworldComm != OMPI_COMM_NULL) OMPI_Comm_dup(oworldComm,&(MPI_COMM_WORLD->oworldComm));
	if(OMPI_COMM_CMP != OMPI_COMM_NULL) OMPI_Comm_dup(OMPI_COMM_CMP,&(MPI_COMM_WORLD->OMPI_COMM_CMP));
	if(OMPI_COMM_REP != OMPI_COMM_NULL) OMPI_Comm_dup(OMPI_COMM_REP,&(MPI_COMM_WORLD->OMPI_COMM_REP));
	if(OMPI_CMP_REP_INTERCOMM != OMPI_COMM_NULL) OMPI_Comm_dup(OMPI_CMP_REP_INTERCOMM,&(MPI_COMM_WORLD->OMPI_CMP_REP_INTERCOMM));*/
  int myrank,cmprank=-1,reprank=-1;
  EMPI_Comm_rank(MPI_COMM_WORLD->eworldComm,&myrank);
	if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_CMP,&cmprank);
	else if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL) EMPI_Comm_rank(MPI_COMM_WORLD->EMPI_COMM_REP,&reprank);

  int sendnums[nC+nR];
	int recvnums[nC+nR];
	int senttomenums[nC+nR];
	int recvfrommenums[nC+nR];
	int *sendids;
	int *recvids;
	
	for(int i = 0; i<nC+nR;i++)
  {
		sendnums[i] = 0;
		recvnums[i] = 0;
	}
	
	struct peertopeer_data *pdata = first_peertopeer;
	while(pdata != NULL)
	{
		if(pdata->sendtype)
		{
			sendnums[pdata->target]++;
			if(cmpToRepMap[pdata->target] != -1) sendnums[cmpToRepMap[pdata->target]+nC]++;
		}
		else
		{
			if(pdata->completecmp && pdata->completerep)
			{
							recvnums[pdata->target]++;
							if(cmpToRepMap[pdata->target] != -1) recvnums[cmpToRepMap[pdata->target]+nC]++;
			}
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL)
			{
				if(!pdata->completerep || !pdata->completecmp)
				{
					int retVal, retValRep;
					int size;
					EMPI_Type_size(pdata->dt->edatatype,&size);
					int dis = pdata->count*size;
					int extracount;
					if(size >= sizeof(int)) extracount = 1;
					else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
					do
					{
						retValRep = 0;
						if(cmpToRepMap[pdata->target] == -1) retValRep = check_errhandler_conditions((pdata->comm->oworldComm),((*(pdata->req))->reqcmp),pdata->target);
						else retValRep = check_errhandler_conditions((pdata->comm->oworldComm),((*(pdata->req))->reqrep),cmpToRepMap[pdata->target]+nC);
						if(retValRep == 0)
						{
							if(cmpToRepMap[pdata->target] == -1) retValRep = EMPI_Irecv (pdata->buf, pdata->count +extracount, pdata->dt->edatatype, pdata->target, pdata->tag, (pdata->comm->EMPI_CMP_REP_INTERCOMM), ((*(pdata->req))->reqcmp));
							else retValRep = EMPI_Irecv (pdata->buf, pdata->count +extracount, pdata->dt->edatatype, cmpToRepMap[pdata->target], pdata->tag, (pdata->comm->EMPI_COMM_REP), ((*(pdata->req))->reqrep));
						}
						if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
						{
							if(*((*(pdata->req))->reqcmp) != EMPI_REQUEST_NULL) EMPI_Request_free(((*(pdata->req))->reqcmp));
							if(*((*(pdata->req))->reqrep) != EMPI_REQUEST_NULL) EMPI_Request_free(((*(pdata->req))->reqrep));
							goto RESENDandMARKdone;
						}
					} while(retValRep != 0);
					(*(pdata->req))->complete = false;
					pdata->completecmp = (cmpToRepMap[pdata->target] != -1);
					pdata->completerep = !(pdata->completecmp);
					//EMPI_Request req = EMPI_REQUEST_NULL;
					//do
					//{
					//	retVal = 0;
					//	retVal = check_errhandler_conditions_collective((pdata->comm->oworldComm),&req);
					//	if(pdata->comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
					//	{
					//		if(req != EMPI_REQUEST_NULL) EMPI_Request_free(&req);
					//		goto RESENDandMARKdone;
					//	}
					//} while(retVal != 0);
				}
			}
			else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				if(!pdata->completerep || !pdata->completecmp)
				{
					int retVal;
					int size;
					EMPI_Type_size(pdata->dt->edatatype,&size);
					int dis = pdata->count*size;
					int extracount;
					if(size >= sizeof(int)) extracount = 1;
					else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
					do
					{
						retVal = 0;
						retVal = check_errhandler_conditions((pdata->comm->oworldComm), ((*(pdata->req))->reqcmp),pdata->target);
						if(retVal == 0)
						{
							//printf("CMP Irecv from errhandler req %p myrank %d src %d\n",*(pdata->req),myrank,pdata->target);
							retVal = EMPI_Irecv (pdata->buf, pdata->count+extracount, pdata->dt->edatatype, pdata->target, pdata->tag, (pdata->comm->EMPI_COMM_CMP), ((*(pdata->req))->reqcmp));
						}
					} while(retVal != 0);
					(*(pdata->req))->complete = false;
					pdata->completecmp = false;
					pdata->completerep = true;
					//EMPI_Request req = EMPI_REQUEST_NULL;
					//do
					//{
					//	retVal = 0;
					//	retVal = check_errhandler_conditions_collective((pdata->comm->oworldComm),&req);
					//} while(retVal != 0);
				}
			}
		}
		pdata = pdata->prev;
	}


  EMPI_Alltoall(&sendnums,1,EMPI_INT,senttomenums,1,EMPI_INT,MPI_COMM_WORLD->eworldComm);
	EMPI_Alltoall(&recvnums,1,EMPI_INT,recvfrommenums,1,EMPI_INT,MPI_COMM_WORLD->eworldComm);
	
  //Identify items from sendnums that were not in recvfrommenums and resend them
	//Identify items from recvnums that were not in senttomenums and mark them to be skipped
	
	for(int i = 0; i<nC+nR;i++)
	{
		if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
		{
			if(i != cmprank && (cmpToRepMap[cmprank] ==-1 || i != cmpToRepMap[cmprank] + nC))
			{
				if((recvnums[i] != senttomenums[i]) && (i < nC) && (recvnums[i] > 0))
				{
					recvids = (int *)malloc(recvnums[i]*sizeof(int));
					pdata = first_peertopeer;
					int index = 0;
					while(pdata != NULL)
					{
						if(!pdata->sendtype && pdata->target == i && pdata->completecmp && pdata->completerep)
						{
							recvids[index] = pdata->id;
							index++;
						}
						pdata = pdata->prev;
					}
					EMPI_Send(recvids,recvnums[i],EMPI_INT,i,i+myrank,MPI_COMM_WORLD->eworldComm);
					//printf("Successful send of %d to %d myval %d myrank %d first %d last %d\n",recvnums[i],i,senttomenums[i],myrank,recvids[0],recvids[recvnums[i]-1]);
					free(recvids);
				}
				if((sendnums[i] != recvfrommenums[i]) && ((i < nC) || (cmpToRepMap[myrank] == -1)))
				{
					EMPI_Status status;
					sendids = (int *)malloc(recvfrommenums[i]*sizeof(int));
					//printf("Attempting recv of %d from %d myval %d myrank %d\n",recvfrommenums[i],i,sendnums[i],myrank);
					if(recvfrommenums[i] > 0) EMPI_Recv(sendids,recvfrommenums[i],EMPI_INT,i,i+myrank,MPI_COMM_WORLD->eworldComm,&status);
					//printf("Successful recv of %d from %d myval %d myrank %d first %d last %d\n",recvfrommenums[i],i,sendnums[i],myrank,sendids[0],sendids[recvfrommenums[i]-1]);
					
					pdata = first_peertopeer;
					int index = 0;
					while(pdata != NULL)
					{
						if(pdata->sendtype && (pdata->target == i || (cmpToRepMap[pdata->target] + nC) == i))
						{
							bool resend = true;
							for(int j = 0; j<recvfrommenums[i];j++)
							{
								if(sendids[j] == pdata->id)
								{
									sendids[j] = -1;
									resend  = false;
								}
							}
							if(resend)
							{
								//printf("CMP Resending %d data with tag %d to %d my_rank %d\n",(pdata->count)+1,pdata->tag,pdata->target,myrank);
								int retVal = 0;
								if(*(pdata->req) != MPI_REQUEST_NULL) MPI_Request_free(pdata->req);
								EMPI_Request req = EMPI_REQUEST_NULL;
								int rank;
								int extracount;
								int dis;
								//void *backup;
								int size;
								EMPI_Type_size(pdata->dt->edatatype,&size);
								dis = pdata->count*size;
								if(size >= sizeof(int)) extracount = 1;
								else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
								else extracount = (((int)sizeof(int))/size) + 1;
								//backup = malloc(extracount*size);
								//memcpy(backup,(pdata->buf)+dis,extracount*size);
								//backup = *(int *)(pdata->buf+dis);
								//*((int *)(pdata->buf+dis)) = pdata->id;
								do
								{
									retVal = 0;
									EMPI_Comm_rank(pdata->comm->EMPI_COMM_CMP,&rank);
									if(i < nC) retVal = check_errhandler_conditions((pdata->comm->oworldComm),((*(pdata->req))->reqcmp),pdata->target);
									else if(cmpToRepMap[rank] == -1) retVal = check_errhandler_conditions((pdata->comm->oworldComm),((*(pdata->req))->reqrep),cmpToRepMap[pdata->target]+nC);
									else break;
									if(retVal == 0)
									{
										if(i < nC) retVal = EMPI_Isend (pdata->buf, (pdata->count) + extracount, pdata->dt->edatatype, pdata->target, pdata->tag, (pdata->comm->EMPI_COMM_CMP), ((*(pdata->req))->reqcmp));
										else if(cmpToRepMap[rank] == -1) retVal = EMPI_Isend (pdata->buf, (pdata->count) + extracount, pdata->dt->edatatype, cmpToRepMap[pdata->target], pdata->tag, (pdata->comm->EMPI_CMP_REP_INTERCOMM), ((*(pdata->req))->reqrep));
										int flag = 0;
										if((pdata->completecmp && (i < nC)) || (pdata->completerep && (i >= nC) && (cmpToRepMap[rank] == -1)))
										{
											do
											{
												if(i < nC) retVal = check_errhandler_conditions((pdata->comm->oworldComm),((*(pdata->req))->reqcmp),pdata->target);
												else if(cmpToRepMap[rank] == -1) retVal = check_errhandler_conditions((pdata->comm->oworldComm),((*(pdata->req))->reqrep),cmpToRepMap[pdata->target]+nC);
												if(retVal != 0) break;
												if(i < nC) EMPI_Test(((*(pdata->req))->reqcmp),&flag,&status);
												else if(cmpToRepMap[rank] == -1) EMPI_Test(((*(pdata->req))->reqrep),&flag,&status);
											} while(flag == 0);
										}
									}
								} while(retVal != 0);
								//req = EMPI_REQUEST_NULL;
								//do
								//{
								//	retVal = 0;
								//	retVal = check_errhandler_conditions_collective((pdata->comm->oworldComm),&req);
								//} while(retVal != 0);
								//memcpy((pdata->buf)+dis,backup,extracount*size);
								//*(int *)(pdata->buf+dis) = backup;
								//free(backup);
							}
						}
						pdata = pdata->prev;
					}
					for(int j = 0; j<recvfrommenums[i];j++)
					{
						if(sendids[j] != -1)
						{
							struct skiplist *skipitem = (struct skiplist *)malloc(sizeof(struct skiplist));
							skipitem->id = sendids[j];
							//printf("adding %d to skiplist myrank %d\n",skipitem->id,myrank);
							if(i < nC)
							{
								skipitem->next = skipcmplist;
								skipcmplist = skipitem;
							}
							else
							{
								skipitem->next = skipreplist;
								skipreplist = skipitem;
							}
						}
					}
					free(sendids);
				}
			}
		}
		else if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL)
		{
			if(i != repToCmpMap[reprank] &&  i != reprank + nC)
			{
				if((recvnums[i] != senttomenums[i]) && (recvnums[i] > 0) && ((i >= nC) || (cmpToRepMap[i] == -1)))
				{
					recvids = (int *)malloc(recvnums[i]*sizeof(int));
					pdata = first_peertopeer;
					int index = 0;
					//if(pdata == NULL) printf("pdata made null i %d myrank %d\n",i,myrank);
					while(pdata != NULL)
					{
						if(!pdata->sendtype && ((pdata->target == i) || (cmpToRepMap[pdata->target]+nC == i)) && pdata->completecmp && pdata->completerep)
						{
							recvids[index] = pdata->id;
							//printf("pdata id %d index %d recvid %d start %d my_rank %d\n",pdata->id,index,recvids[index],recvids[0],myrank);
							index++;
						}
						pdata = pdata->prev;
					}
					EMPI_Send(recvids,recvnums[i],EMPI_INT,i,i+myrank,MPI_COMM_WORLD->eworldComm);
					//printf("Successful send of %d to %d myval %d myrank %d first %d last %d i %d\n",recvnums[i],i,senttomenums[i],myrank,recvids[0],recvids[recvnums[i]-1],i);
					free(recvids);
				}
				if(sendnums[i] != recvfrommenums[i] && i >= nC)
				{
					EMPI_Status status;
					sendids = (int *)malloc(recvfrommenums[i]*sizeof(int));
					//printf("Attempting recv of %d from %d myval %d myrank %d\n",recvfrommenums[i],i,sendnums[i],myrank);
					if(recvfrommenums[i] > 0) EMPI_Recv(sendids,recvfrommenums[i],EMPI_INT,i,i+myrank,MPI_COMM_WORLD->eworldComm,&status);
					//printf("Successful recv of %d from %d myval %d myrank %d first %d last %d\n",recvfrommenums[i],i,sendnums[i],myrank,sendids[0],sendids[recvfrommenums[i]-1]);
					
					pdata = first_peertopeer;
					int index = 0;
					while(pdata != NULL)
					{
						if(pdata->sendtype && (cmpToRepMap[pdata->target] + nC) == i)
						{
							bool resend = true;
							for(int j = 0; j<recvfrommenums[i];j++)
							{
								if(sendids[j] == pdata->id)
								{
									sendids[j] = -1;
									resend  = false;
								}
							}
							if(resend)
							{
								//printf("REP Resending %d data with tag %d to %d my_rank %d id %d\n",(pdata->count)+1,pdata->tag,pdata->target,myrank,pdata->id);
								int retVal = 0;
								if(*(pdata->req) != MPI_REQUEST_NULL) MPI_Request_free(pdata->req);
								EMPI_Request req = EMPI_REQUEST_NULL;
								int extracount;
								int dis;
								//void *backup;
								int size;
								EMPI_Type_size(pdata->dt->edatatype,&size);
								dis = pdata->count*size;
								if(size >= sizeof(int)) extracount = 1;
								else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
								else extracount = (((int)sizeof(int))/size) + 1;
								//backup = malloc(extracount*size);
								//memcpy(backup,(pdata->buf)+dis,extracount*size);
								//backup = *(int *)(pdata->buf+dis);
								*((int *)(pdata->buf+dis)) = pdata->id;
								do
								{
									retVal = 0;
									retVal = check_errhandler_conditions((pdata->comm->oworldComm),((*(pdata->req))->reqrep),cmpToRepMap[pdata->target]+nC);
									if(retVal == 0)
									{
										retVal = EMPI_Isend (pdata->buf, (pdata->count) + extracount, pdata->dt->edatatype, cmpToRepMap[pdata->target], pdata->tag, (pdata->comm->EMPI_COMM_REP),((*(pdata->req))->reqrep));
										int flag = 0;
										if(pdata->completerep)
										{
											do
											{
												retVal = check_errhandler_conditions((pdata->comm->oworldComm),((*(pdata->req))->reqrep),cmpToRepMap[pdata->target]+nC);
												if(retVal != 0) break;
												EMPI_Test(((*(pdata->req))->reqrep),&flag,&status);
											} while(flag == 0);
										}
										if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
										{
											if(*((*(pdata->req))->reqrep) != EMPI_COMM_NULL) EMPI_Request_free(((*(pdata->req))->reqrep));
											goto RESENDandMARKdone;
										}
									}
								} while(retVal != 0);
								//req = EMPI_REQUEST_NULL;
								//do
								//{
								//	retVal = 0;
								//	retVal = check_errhandler_conditions_collective((pdata->comm->oworldComm),&req);
								//	if(pdata->comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
								//	{
								//		if(req != EMPI_REQUEST_NULL) EMPI_Request_free(&req);
								//		goto RESENDandMARKdone;
								//	}
								//} while(retVal != 0);
								//memcpy((pdata->buf)+dis,backup,extracount*size);
								//*(int *)(pdata->buf+dis) = backup;
								//free(backup);
							}
						}
						pdata = pdata->prev;
					}
					for(int j = 0; j<recvfrommenums[i];j++)
					{
						if(sendids[j] != -1)
						{
							struct skiplist *skipitem = (struct skiplist *)malloc(sizeof(struct skiplist));
							skipitem->id = sendids[j];
							skipitem->next = skipreplist;
							skipreplist = skipitem;
						}
					}
					free(sendids);
				}
			}
		}
	}
	//printf("At res and mark %d\n",myrank);
RESENDandMARKdone:
  free(ranks_gf); free(ranks_gc);
  fclose(logfile);
	
  char logName[200];
  sprintf (logName, "%s/logs/%d", workDir, myrank);
  logfile = fopen(logName,"w+");
  int collids[nC+nR];
	int minid;
  if(last_collective != NULL) EMPI_Allgather(&last_collective->id,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm);
	else
	{
		int temp = -1;
		EMPI_Allgather(&temp,1,EMPI_INT,collids,1,EMPI_INT,MPI_COMM_WORLD->eworldComm);
	}
	minid = collids[0];
	//printf("collids: ");
	//for(int i = 0; i < nC+nR; i++)
	//{
	//	printf("%d ",collids[i]);
	//}
	//printf("\n");
	for(int i = 0; i < nC; i++)
	{
		if(collids[i] < minid)
		{
			minid = collids[i];
		}
	}
	int cmpid = minid;
	int repid = -1;
	int myrepid = -1;
	if(nR > 0)
	{
		repid = collids[nC];
		for(int i = nC; i < nC+nR; i++)
		{
			if(collids[i] < repid)
			{
				repid = collids[i];
			}
		}
		if(repid < minid && repid >= 0) minid = repid;
	}
	if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
	{
		if(cmpToRepMap[myrank] != -1)
		{
			myrepid = collids[cmpToRepMap[myrank]+nC];
		}
	}
	struct collective_data *retry_data = NULL;
	if(last_collective != NULL)
	{
		if(minid < last_collective->id)
		{
			retry_data = last_collective;
			while(retry_data->next != NULL)
			{
				if(minid >= retry_data->next->id) break;
				retry_data = retry_data->next;
			}
		}
	}
	else
	{
		retry_data = NULL;
	}
	//printf("rank %d cmpid %d repid %d retry data id %d %d last coll %d %d\n",myrank,cmpid,repid,(retry_data == NULL) ? -1 : retry_data->id, (retry_data == NULL) ? -1 : retry_data->type, last_collective->id, last_collective->type);
	mpi_ft_collective_from_args(retry_data,cmpid,repid,myrepid);
	//printf("rank %d collective from args done\n",myrank);
}

static void mpi_ft_free_older_collectives(struct collective_data *data)
{
	struct collective_data *current = data;
	while(current != NULL)
	{
		struct collective_data *next = current->next;
		if(current->type == MPI_FT_BCAST)
		{
			free((current->args).bcast.buf);
#ifdef HEAPTRACK
			if(getenv("MPI_FT_TRACK_HEAP"))
			{
				if(!strcmp(getenv("MPI_FT_TRACK_HEAP"),"1"))
				{
					gettimeofday(&tv,NULL);
					int size;
					EMPI_Type_size((current->args).bcast.dt->edatatype,&size);
					total_alloc -= size * (current->args).bcast.count;
					fprintf(logfile,"%d %ld\n",total_alloc,1000000 * tv.tv_sec + tv.tv_usec);
				}
			}
#endif
		}
		else if(current->type == MPI_FT_SCATTER)
		{
			free((current->args).scatter.sendbuf);
			free((current->args).scatter.recvbuf);
		}
		else if(current->type == MPI_FT_GATHER)
		{
			free((current->args).gather.sendbuf);
			free((current->args).gather.recvbuf);
		}
		else if(current->type == MPI_FT_REDUCE)
		{
			free((current->args).reduce.sendbuf);
			free((current->args).reduce.recvbuf);
		}
		else if(current->type == MPI_FT_ALLGATHER)
		{
			free((current->args).allgather.sendbuf);
			free((current->args).allgather.recvbuf);
		}
		else if(current->type == MPI_FT_ALLTOALL)
		{
			free((current->args).alltoall.sendbuf);
			free((current->args).alltoall.recvbuf);
		}
		else if(current->type == MPI_FT_ALLTOALLV)
		{
			free((current->args).alltoallv.sendbuf);
			free((current->args).alltoallv.recvbuf);
			free((current->args).alltoallv.sendcounts);
			free((current->args).alltoallv.recvcounts);
			free((current->args).alltoallv.sdispls);
			free((current->args).alltoallv.rdispls);
		}
		else if(current->type == MPI_FT_ALLREDUCE)
		{
			free((current->args).allreduce.sendbuf);
			free((current->args).allreduce.recvbuf);
		}
		free(current);
		current = next;
#ifdef HEAPTRACK
		if(getenv("MPI_FT_TRACK_HEAP"))
		{
			if(!strcmp(getenv("MPI_FT_TRACK_HEAP"),"1"))
			{
				gettimeofday(&tv,NULL);
				total_alloc -= sizeof(struct collective_data);
				fprintf(logfile,"%d %ld\n",total_alloc,1000000 * tv.tv_sec + tv.tv_usec);
			}
		}
#endif
	}
}

void mpi_ft_collective_from_args(struct collective_data *data, int cmpid, int repid, int myrepid)
{
	while(data != NULL)
	{
		EMPI_Request req = EMPI_REQUEST_NULL;
		EMPI_Request reqinter = EMPI_REQUEST_NULL;
		EMPI_Status status;
		int retVal;
		int rank;
		if(data->type == MPI_FT_BARRIER)
		{
			if(data->id > cmpid)
			{
				do
				{
					retVal = 0;
					retVal = check_errhandler_conditions_collective(((data->args).barrier.comm)->oworldComm,&req);
					
					if(retVal == 0)
					{
						//printf("Recalling barrier for id %d %d %d\n",data->id,(data->args).barrier.comm == MPI_COMM_WORLD,((data->args).barrier.comm)->eworldComm == EMPI_COMM_NULL);
						retVal = EMPI_Ibarrier (((data->args).barrier.comm)->eworldComm, &req);
						int flag = 0;
						do
						{
							retVal = check_errhandler_conditions_collective(((data->args).barrier.comm)->oworldComm,&req);
							if(retVal != 0) break;
							EMPI_Test(&req,&flag,&status);
						} while(flag == 0);
					}
				} while(retVal != 0);
				int rnk;
				EMPI_Comm_rank(((data->args).barrier.comm)->eworldComm,&rnk);
				//printf("%d: Recalling barrier complete for id %d %d %d\n",rnk,data->id,data->type,data->prev == NULL);
			}
		}
		if(data->type == MPI_FT_BCAST)
		{
			if(!data->completerep)
			{
				reqinter = *(data->repreq);
			}
			int retValinter;
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL)
			{
				do
				{
					retVal = 0;
					retValinter = 0;
					if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).bcast.root] == -1) retVal = check_errhandler_conditions_collective(((data->args).bcast.comm)->oworldComm,&reqinter);
					else if(data->id > repid && repid >= 0) retVal = check_errhandler_conditions_collective(((data->args).bcast.comm)->oworldComm,&req);
					
					if(retVal == 0)
					{
						if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).bcast.root] == -1) retVal = EMPI_Ibcast ((data->args).bcast.buf, (data->args).bcast.count, (data->args).bcast.dt->edatatype, (data->args).bcast.root, ((data->args).bcast.comm)->EMPI_CMP_REP_INTERCOMM, &reqinter);
						else if(data->id > repid && repid >= 0) retVal = EMPI_Ibcast ((data->args).bcast.buf, (data->args).bcast.count, (data->args).bcast.dt->edatatype, cmpToRepMap[(data->args).bcast.root], ((data->args).bcast.comm)->EMPI_COMM_REP, &req);
						int flag = !(data->id > repid && repid >= 0);
						do
						{
							if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).bcast.root] == -1) retVal = check_errhandler_conditions_collective(((data->args).bcast.comm)->oworldComm,&reqinter);
							else if(data->id > repid && repid >= 0) retVal = check_errhandler_conditions_collective(((data->args).bcast.comm)->oworldComm,&req);
							if(retVal != 0) break;
							if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).bcast.root] == -1) EMPI_Test(&reqinter,&flag,&status);
							else if(data->id > repid && repid >= 0) EMPI_Test(&req,&flag,&status);
						} while(flag == 0);
					}
				} while(retVal != 0);
			}
			else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{	
				do
				{
					retVal = 0;
					retValinter = 0;
					EMPI_Comm_rank((data->args).bcast.comm->EMPI_COMM_CMP,&rank);
					if(data->id > cmpid) retVal = check_errhandler_conditions_collective(((data->args).bcast.comm)->oworldComm,&req);
					if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).bcast.root] == -1 && nR > 0) retValinter = check_errhandler_conditions_collective(((data->args).bcast.comm)->oworldComm,&reqinter);
					
					if(retVal == 0 && retValinter == 0)
					{
						if(data->id > cmpid) retVal = EMPI_Ibcast ((data->args).bcast.buf, (data->args).bcast.count, (data->args).bcast.dt->edatatype, (data->args).bcast.root, ((data->args).bcast.comm)->EMPI_COMM_CMP, &req);
						if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).bcast.root] == -1 && rank == (data->args).bcast.root && nR > 0) retValinter = EMPI_Ibcast ((data->args).bcast.buf, (data->args).bcast.count, (data->args).bcast.dt->edatatype, EMPI_ROOT, ((data->args).bcast.comm)->EMPI_CMP_REP_INTERCOMM, &reqinter);
						else if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).bcast.root] == -1 && nR > 0) retValinter = EMPI_Ibcast ((data->args).bcast.buf, (data->args).bcast.count, (data->args).bcast.dt->edatatype, EMPI_PROC_NULL, ((data->args).bcast.comm)->EMPI_CMP_REP_INTERCOMM, &reqinter);
						int flag = !(data->id > cmpid);
						int flaginter = !(cmpToRepMap[(data->args).bcast.root]  == -1 && nR > 0 && data->id > repid && repid >= 0 && data->completerep);
						do
						{
							if(data->id > cmpid && flag == 0) retVal = check_errhandler_conditions_collective(((data->args).bcast.comm)->oworldComm,&req);
							if(retVal != 0) break;
							if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).bcast.root] == -1 && nR > 0 && flaginter == 0 && data->completerep) retValinter = check_errhandler_conditions_collective(((data->args).bcast.comm)->oworldComm,&req);
							if(retValinter != 0) break;
							if(data->id > cmpid  && flag == 0) EMPI_Test(&req,&flag,&status);
							if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).bcast.root] == -1 && nR > 0 && flaginter == 0 && data->completerep) EMPI_Test(&reqinter,&flaginter,&status);
						} while(flag == 0 && flaginter == 0);
					}
				} while(retVal != 0 || retValinter != 0);
			}
			if(!data->completerep)
			{
				*(data->repreq) = reqinter;
			}
		}
		else if(data->type == MPI_FT_SCATTER)
		{
			if(!data->completerep)
			{
				reqinter = *(data->repreq);
			}
			int retValinter;
			int *tmpscounts;
			int *tmpsdispls;
			int commnC;
			int commnR;
			int rank;
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL)
			{
				do
				{
					retVal = 0;
					retValinter = 0;
					EMPI_Comm_rank((((data->args).scatter.comm)->EMPI_COMM_REP),&rank);
					EMPI_Comm_size((((data->args).scatter.comm)->EMPI_COMM_REP),&commnR);
					EMPI_Comm_remote_size((((data->args).scatter.comm)->EMPI_CMP_REP_INTERCOMM),&commnC);
					tmpscounts = (int *)malloc(commnR*sizeof(int));
					tmpsdispls = (int *)malloc(commnR*sizeof(int));
					int j = 0;
					for(int i = 0; i < commnC; i++)
					{
						if(cmpToRepMap[i] != -1)
						{
							tmpscounts[j] = (data->args).scatter.sendcount;
							tmpsdispls[j] = i*(data->args).scatter.sendcount;
							j++;
						}
					}
					if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).scatter.root] == -1) retVal = check_errhandler_conditions_collective(((data->args).scatter.comm)->oworldComm,&reqinter);
					else if(data->id > repid && repid >= 0) retVal = check_errhandler_conditions_collective(((data->args).scatter.comm)->oworldComm,&req);
					
					if(retVal == 0)
					{
						if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).scatter.root] == -1) retVal = EMPI_Iscatterv ((data->args).scatter.sendbuf, tmpscounts, tmpsdispls, (data->args).scatter.senddt->edatatype, (data->args).scatter.recvbuf, (data->args).scatter.recvcount, (data->args).scatter.recvdt->edatatype, (data->args).scatter.root, ((data->args).scatter.comm)->EMPI_CMP_REP_INTERCOMM, &reqinter);
						else if(data->id > repid && repid >= 0) retVal = EMPI_Iscatterv ((data->args).scatter.sendbuf, tmpscounts, tmpsdispls, (data->args).scatter.senddt->edatatype, (data->args).scatter.recvbuf, (data->args).scatter.recvcount, (data->args).scatter.recvdt->edatatype, cmpToRepMap[(data->args).scatter.root], ((data->args).scatter.comm)->EMPI_COMM_REP, &req);
						int flag = !(data->id > repid && repid >= 0);
						do
						{
							if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).scatter.root] == -1) retVal = check_errhandler_conditions_collective(((data->args).scatter.comm)->oworldComm,&reqinter);
							else if(data->id > repid && repid >= 0) retVal = check_errhandler_conditions_collective(((data->args).scatter.comm)->oworldComm,&req);
							if(retVal != 0) break;
							if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).scatter.root] == -1) EMPI_Test(&reqinter,&flag,&status);
							else if(data->id > repid && repid >= 0) EMPI_Test(&req,&flag,&status);
						} while(flag == 0);
					}
				} while(retVal != 0);
			}
			else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{	
				do
				{
					retVal = 0;
					retValinter = 0;
					EMPI_Comm_rank((((data->args).scatter.comm)->EMPI_COMM_CMP),&rank);
					EMPI_Comm_size((((data->args).scatter.comm)->EMPI_COMM_CMP),&commnC);
					if((((data->args).scatter.comm)->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((((data->args).scatter.comm)->EMPI_CMP_REP_INTERCOMM),&commnR);
					else commnR = 0;
					tmpscounts = (int *)malloc(commnR*sizeof(int));
					tmpsdispls = (int *)malloc(commnR*sizeof(int));
					int j = 0;
					for(int i = 0; i < commnC; i++)
					{
						if(cmpToRepMap[i] != -1)
						{
							tmpscounts[j] = (data->args).scatter.sendcount;
							tmpsdispls[j] = i*(data->args).scatter.sendcount;
							j++;
						}
					}
					EMPI_Comm_rank((data->args).scatter.comm->EMPI_COMM_CMP,&rank);
					if(data->id > cmpid) retVal = check_errhandler_conditions_collective(((data->args).scatter.comm)->oworldComm,&req);
					if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).scatter.root] == -1 && nR > 0) retValinter = check_errhandler_conditions_collective(((data->args).scatter.comm)->oworldComm,&reqinter);
					
					if(retVal == 0 && retValinter == 0)
					{
						if(data->id > cmpid) retVal = EMPI_Iscatter ((data->args).scatter.sendbuf, (data->args).scatter.sendcount, (data->args).scatter.senddt->edatatype, (data->args).scatter.recvbuf, (data->args).scatter.recvcount, (data->args).scatter.recvdt->edatatype, (data->args).scatter.root, ((data->args).scatter.comm)->EMPI_COMM_CMP, &req);
						if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).scatter.root] == -1 && rank == (data->args).scatter.root && nR > 0) retValinter = EMPI_Iscatterv ((data->args).scatter.sendbuf, tmpscounts, tmpsdispls, (data->args).scatter.senddt->edatatype, (data->args).scatter.recvbuf, (data->args).scatter.recvcount, (data->args).scatter.recvdt->edatatype, EMPI_ROOT, ((data->args).scatter.comm)->EMPI_CMP_REP_INTERCOMM, &reqinter);
						else if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).scatter.root] == -1 && nR > 0) retValinter = EMPI_Iscatterv ((data->args).scatter.sendbuf, tmpscounts, tmpsdispls, (data->args).scatter.senddt->edatatype, (data->args).scatter.recvbuf, (data->args).scatter.recvcount, (data->args).scatter.recvdt->edatatype, EMPI_PROC_NULL, ((data->args).scatter.comm)->EMPI_CMP_REP_INTERCOMM, &reqinter);
						int flag = !(data->id > cmpid);
						int flaginter = !(cmpToRepMap[(data->args).scatter.root] == -1 && nR > 0 && data->id > repid && repid >= 0 && data->completerep);
						do
						{
							if(data->id > cmpid && flag == 0) retVal = check_errhandler_conditions_collective(((data->args).scatter.comm)->oworldComm,&req);
							if(retVal != 0) break;
							if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).scatter.root] == -1 && nR > 0 && flaginter == 0 && data->completerep) retValinter = check_errhandler_conditions_collective(((data->args).scatter.comm)->oworldComm,&reqinter);
							if(retValinter != 0) break;
							if(data->id > cmpid  && flag == 0) EMPI_Test(&req,&flag,&status);
							if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).scatter.root] == -1 && nR > 0 && flaginter == 0 && data->completerep) EMPI_Test(&reqinter,&flaginter,&status);
						} while(flag == 0 && flaginter == 0);
					}
				} while(retVal != 0 || retValinter != 0);
			}
			if(!data->completerep)
			{
				*(data->repreq) = reqinter;
			}
		}
		else if(data->type == MPI_FT_GATHER)
		{
			if(!data->completerep)
			{
				reqinter = *(data->repreq);
			}
			int retValinter;
			int *tmprcounts;
			int *tmprdispls;
			int *tmprcountsinter;
			int *tmprdisplsinter;
			int commnC;
			int commnR;
			int rank;
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL)
			{
				do
				{
					retVal = 0;
					retValinter = 0;
					EMPI_Comm_rank((((data->args).gather.comm)->EMPI_COMM_REP),&rank);
					EMPI_Comm_size((((data->args).gather.comm)->EMPI_COMM_REP),&commnR);
					EMPI_Comm_remote_size((((data->args).gather.comm)->EMPI_CMP_REP_INTERCOMM),&commnC);
					if(data->id > repid && repid >= 0) tmprcounts = (int *)malloc(commnR*sizeof(int));
					if(data->id > repid && repid >= 0) tmprdispls = (int *)malloc(commnR*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid && repid >= 0) tmprcountsinter = (int *)malloc((commnC-commnR)*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid && repid >= 0) tmprdisplsinter = (int *)malloc((commnC-commnR)*sizeof(int));
					if(cmpToRepMap[(data->args).gather.root] != -1 && data->id > repid && repid >= 0)
					{
						int j = 0;
						int k = 0;
						for(int i = 0; i < commnC; i++)
						{
							if(cmpToRepMap[i] != -1)
							{
								tmprcounts[j] = (data->args).gather.recvcount;
								tmprdispls[j] = i*(data->args).gather.recvcount;
								j++;
							}
							else if(commnC > commnR && commnR > 0)
							{
								tmprcountsinter[k] = (data->args).gather.recvcount;
								tmprdisplsinter[k] = i*(data->args).gather.recvcount;
								k++;
							}
						}
						if(commnC > commnR && commnR > 0) retValinter = check_errhandler_conditions_collective(((data->args).gather.comm)->oworldComm,&reqinter);
						retVal = check_errhandler_conditions_collective(((data->args).gather.comm)->oworldComm,&req);
						
						if(retVal == 0 && retValinter == 0)
						{
							if(commnC > commnR && commnR > 0 && rank == cmpToRepMap[(data->args).gather.root]) retValinter = EMPI_Igatherv ((data->args).gather.sendbuf, (data->args).gather.sendcount, (data->args).gather.senddt->edatatype, (data->args).gather.recvbuf, tmprcountsinter, tmprdisplsinter, (data->args).gather.recvdt->edatatype, EMPI_ROOT, ((data->args).gather.comm)->EMPI_CMP_NO_REP_INTERCOMM, &reqinter);
							else if(commnC > commnR && commnR > 0) retValinter = EMPI_Igatherv ((data->args).gather.sendbuf, (data->args).gather.sendcount, (data->args).gather.senddt->edatatype, (data->args).gather.recvbuf, tmprcountsinter, tmprdisplsinter, (data->args).gather.recvdt->edatatype, EMPI_PROC_NULL, ((data->args).gather.comm)->EMPI_CMP_NO_REP_INTERCOMM, &reqinter);
							retVal = EMPI_Igatherv ((data->args).gather.sendbuf, (data->args).gather.sendcount, (data->args).gather.senddt->edatatype, (data->args).gather.recvbuf, tmprcounts, tmprdispls, (data->args).gather.recvdt->edatatype, cmpToRepMap[(data->args).gather.root], ((data->args).gather.comm)->EMPI_COMM_REP, &req);
							int flag = 0;
							int flaginter = !(commnC > commnR && commnR > 0);
							do
							{
								if(flag == 0) retVal = check_errhandler_conditions_collective(((data->args).gather.comm)->oworldComm,&req);
								if(retVal != 0) break;
								if(commnC > commnR && commnR > 0 && flaginter == 0) retValinter = check_errhandler_conditions_collective(((data->args).gather.comm)->oworldComm,&reqinter);
								if(retValinter != 0) break;
								if(flag == 0) EMPI_Test(&req,&flag,&status);
								if(commnC > commnR && commnR > 0 && flaginter == 0) EMPI_Test(&reqinter,&flaginter,&status);
							} while(flag == 0 && flaginter == 0);
						}
					}
				} while(retVal != 0 || retValinter != 0);
			}
			else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{	
				do
				{
					retVal = 0;
					retValinter = 0;
					EMPI_Comm_rank(((data->args).gather.comm->EMPI_COMM_CMP),&rank);
					EMPI_Comm_size(((data->args).gather.comm->EMPI_COMM_CMP),&commnC);
					if(((data->args).gather.comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size(((data->args).gather.comm->EMPI_CMP_REP_INTERCOMM),&commnR);
					else commnR = 0;
					if(cmpToRepMap[(data->args).gather.root] != -1 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0) tmprcountsinter = (int *)malloc((commnC-commnR)*sizeof(int));
					if(cmpToRepMap[(data->args).gather.root] != -1 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0) tmprdisplsinter = (int *)malloc((commnC-commnR)*sizeof(int));
					if(cmpToRepMap[(data->args).gather.root] != -1 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0)
					{
						int j = 0;
						for(int i = 0; i < commnC; i++)
						{
							if(cmpToRepMap[i] != -1)
							{
								tmprcountsinter[j] = (data->args).gather.recvcount;
								tmprdisplsinter[j] = i*(data->args).gather.recvcount;
								j++;
							}
						}
					}
					if(data->id > cmpid) retVal = check_errhandler_conditions_collective(((data->args).gather.comm)->oworldComm,&req);
					if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).gather.root] != -1 && cmpToRepMap[rank] == -1) retValinter = check_errhandler_conditions_collective(((data->args).gather.comm)->oworldComm,&reqinter);
					
					if(retVal == 0 && retValinter == 0)
					{
						if(data->id > cmpid) retVal = EMPI_Igather ((data->args).gather.sendbuf, (data->args).gather.sendcount, (data->args).gather.senddt->edatatype, (data->args).gather.recvbuf, (data->args).gather.recvcount, (data->args).gather.recvdt->edatatype, (data->args).gather.root, ((data->args).gather.comm)->EMPI_COMM_CMP, &req);
						if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).gather.root] != -1 && cmpToRepMap[rank] == -1) retValinter = EMPI_Igatherv ((data->args).gather.sendbuf, (data->args).gather.sendcount, (data->args).gather.senddt->edatatype, (data->args).gather.recvbuf, tmprcountsinter, tmprdisplsinter, (data->args).gather.recvdt->edatatype, EMPI_ROOT, ((data->args).gather.comm)->EMPI_CMP_REP_INTERCOMM, &reqinter);
						int flag = !(data->id > cmpid);
						int flaginter = !(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).gather.root] != -1 && cmpToRepMap[rank] == -1 && data->completerep);
						do
						{
							if(data->id > cmpid && flag == 0) retVal = check_errhandler_conditions_collective(((data->args).gather.comm)->oworldComm,&req);
							if(retVal != 0) break;
							if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).gather.root] != -1 && cmpToRepMap[rank] == -1 && data->completerep) retValinter = check_errhandler_conditions_collective(((data->args).gather.comm)->oworldComm,&req);
							if(retValinter != 0) break;
							if(data->id > cmpid  && flag == 0) EMPI_Test(&req,&flag,&status);
							if(data->id > repid && repid >= 0 && cmpToRepMap[(data->args).gather.root] != -1 && cmpToRepMap[rank] == -1 && data->completerep) EMPI_Test(&reqinter,&flaginter,&status);
						} while(flag == 0 && flaginter == 0);
					}
				} while(retVal != 0 || retValinter != 0);
			}
			if(!data->completerep)
			{
				*(data->repreq) = reqinter;
			}
		}
		else if(data->type == MPI_FT_REDUCE)
		{
			if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				if(data->id > cmpid)
				{
					do
					{
						retVal = 0;
						retVal = check_errhandler_conditions_collective(((data->args).reduce.comm)->oworldComm,&req);
						
						if(retVal == 0)
						{
							retVal = EMPI_Ireduce ((data->args).reduce.sendbuf, (data->args).reduce.recvbuf, (data->args).reduce.count, (data->args).reduce.dt->edatatype, ((data->args).reduce.op)->eop, (data->args).reduce.root, ((data->args).reduce.comm)->EMPI_COMM_CMP, &req);
							int flag = 0;
							do
							{
								retVal = check_errhandler_conditions_collective(((data->args).reduce.comm)->oworldComm,&req);
								if(retVal != 0) break;
								EMPI_Test(&req,&flag,&status);
							} while(flag == 0);
						}
					} while(retVal != 0);
				}
				if(data->id > myrepid && myrepid >= 0)
				{
					do
					{
						retVal = 0;
						EMPI_Comm_rank(((data->args).reduce.comm)->EMPI_COMM_CMP,&rank);
						if(cmpToRepMap[rank] == -1 || rank != (data->args).reduce.root) break;
						retVal = check_errhandler_conditions(((data->args).reduce.comm)->oworldComm,&req,cmpToRepMap[rank]+nC);
						
						if(retVal == 0)
						{
							retVal = EMPI_Isend ((data->args).reduce.recvbuf, (data->args).reduce.count, (data->args).reduce.dt->edatatype, cmpToRepMap[rank], MPI_FT_COLLECTIVE_TAG, ((data->args).reduce.comm)->EMPI_CMP_REP_INTERCOMM, &req);
							int flag = 0;
							do
							{
								retVal = check_errhandler_conditions(((data->args).reduce.comm)->oworldComm,&req,cmpToRepMap[rank]);
								if(retVal != 0) break;
								EMPI_Test(&req,&flag,&status);
							} while(flag == 0);
						}
					} while(retVal != 0);
				}
			}
		}
		else if(data->type == MPI_FT_ALLGATHER)
		{
			int retValinter;
			int *tmpscounts;
			int *tmpsdispls;
			int *tmprcounts;
			int *tmprdispls;
			int *tmprcountsinter;
			int *tmprdisplsinter;
			EMPI_Request *reqs;
			bool *completereq;
			if(!data->completerep)
			{
				reqs = data->repreq;
				free(reqs);
			}
			int commnC;
			int commnR;
			int rank;
			int flagcount;
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL)
			{
				do
				{
					retVal = 0;
					retValinter = 0;
					EMPI_Comm_rank((((data->args).allgather.comm)->EMPI_COMM_REP),&rank);
					EMPI_Comm_size((((data->args).allgather.comm)->EMPI_COMM_REP),&commnR);
					EMPI_Comm_remote_size((((data->args).allgather.comm)->EMPI_CMP_REP_INTERCOMM),&commnC);
					if(data->id > repid && repid >= 0) tmpscounts = (int *)malloc(commnR*sizeof(int));
					if(data->id > repid && repid >= 0) tmpsdispls = (int *)malloc(commnR*sizeof(int));
					if(data->id > repid && repid >= 0) tmprcounts = (int *)malloc(commnR*sizeof(int));
					if(data->id > repid && repid >= 0) tmprdispls = (int *)malloc(commnR*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid) tmprcountsinter = (int *)malloc((commnC-commnR)*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid) tmprdisplsinter = (int *)malloc((commnC-commnR)*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid) reqs = (EMPI_Request *)malloc((commnR)*sizeof(EMPI_Request));
					if(commnC > commnR && commnR > 0 && data->id > repid) completereq = (bool *)malloc((commnR)*sizeof(bool));
					if(data->id > repid && repid >= 0)
					{
						int j = 0;
						int k = 0;
						for(int i = 0; i < commnC; i++)
						{
							if(cmpToRepMap[i] != -1)
							{
								tmpscounts[j] = (data->args).allgather.sendcount;
								tmpsdispls[j] = i*(data->args).allgather.sendcount;
								tmprcounts[j] = (data->args).allgather.recvcount;
								tmprdispls[j] = i*(data->args).allgather.recvcount;
								j++;
							}
							else if(commnC > commnR && commnR > 0)
							{
								tmprcountsinter[k] = (data->args).allgather.recvcount;
								tmprdisplsinter[k] = i*(data->args).allgather.recvcount;
								k++;
							}
						}
						
						if(commnC > commnR && commnR > 0)
						{
							reqs = (EMPI_Request *)malloc(commnR*sizeof(EMPI_Request));
							completereq = (bool *)malloc(commnR*sizeof(bool));
							for(int i = 0; i < commnR; i++)
							{
								completereq[i] = false;
								retValinter = check_errhandler_conditions_collective(((data->args).allgather.comm->oworldComm),&reqs[i]);
								if(retValinter == 0)
								{
									if(rank == i) retValinter = EMPI_Igatherv ((data->args).allgather.sendbuf, (data->args).allgather.sendcount, (data->args).allgather.senddt->edatatype, (data->args).allgather.recvbuf, tmprcountsinter, tmprdisplsinter, (data->args).allgather.recvdt->edatatype, EMPI_ROOT, ((data->args).allgather.comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
									else retValinter = EMPI_Igatherv ((data->args).allgather.sendbuf, (data->args).allgather.sendcount, (data->args).allgather.senddt->edatatype, (data->args).allgather.recvbuf, tmprcountsinter, tmprdisplsinter, (data->args).allgather.recvdt->edatatype, EMPI_PROC_NULL, ((data->args).allgather.comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
								}
								if(retValinter != 0) break;
							}
						}
						if(retValinter != 0)
						{
							if(commnC > commnR && commnR > 0) free(reqs);
							if(commnC > commnR && commnR > 0) free(completereq);
							free(tmpscounts);
							free(tmpsdispls);
							free(tmprcounts);
							free(tmprdispls);
							if(commnC > commnR && commnR > 0) free(tmprcountsinter);
							if(commnC > commnR && commnR > 0) free(tmprdisplsinter);
							continue;
						}
						
						retVal = check_errhandler_conditions_collective(((data->args).allgather.comm)->oworldComm,&req);
						if(retVal == 0)
						{
							retVal = EMPI_Iallgatherv ((data->args).allgather.sendbuf, tmpscounts, tmpsdispls, (data->args).allgather.senddt->edatatype, (data->args).allgather.recvbuf, tmprcounts, tmprdispls, (data->args).allgather.recvdt->edatatype, ((data->args).allgather.comm)->EMPI_COMM_REP, &req);
							int flag = 0;
							do
							{
								retVal = check_errhandler_conditions_collective(((data->args).allgather.comm)->oworldComm,&req);
								if(retVal != 0) break;
								EMPI_Test(&req,&flag,&status);
							} while(flag == 0);
						}
						if(retVal != 0)
						{
							if(commnC > commnR && commnR > 0) free(reqs);
							if(commnC > commnR && commnR > 0) free(completereq);
							free(tmpscounts);
							free(tmpsdispls);
							free(tmprcounts);
							free(tmprdispls);
							if(commnC > commnR && commnR > 0) free(tmprcountsinter);
							if(commnC > commnR && commnR > 0) free(tmprdisplsinter);
							continue;
						}
						
						retValinter = 0;
						if(commnC > commnR && commnR > 0) flagcount = 0;
						else flagcount = commnR;
						int flaginter = 0;
						do
						{
							flaginter = 0;
							if(commnC > commnR && commnR > 0)
							{
								for(int i = 0; i < commnR; i++)
								{
									if(!completereq[i])
									{
										flaginter = 0;
										retValinter = check_errhandler_conditions_collective(((data->args).allgather.comm->oworldComm),&reqs[i]);
										if(retValinter != 0) break;
										EMPI_Test(&reqs[i],&flaginter,&status);
										flagcount += flaginter;
										if(flaginter > 0) completereq[i] = true;
									}
								}
							}
							if(retValinter != 0) break;
						} while(flagcount < commnR);
						if(retValinter != 0)
						{
							if(commnC > commnR && commnR > 0) free(reqs);
							if(commnC > commnR && commnR > 0) free(completereq);
							free(tmpscounts);
							free(tmpsdispls);
							free(tmprcounts);
							free(tmprdispls);
							if(commnC > commnR && commnR > 0) free(tmprcountsinter);
							if(commnC > commnR && commnR > 0) free(tmprdisplsinter);
						}	
					}
				} while(retVal != 0 || retValinter != 0);
			}
			else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{	
				do
				{
					retVal = 0;
					retValinter = 0;
					EMPI_Comm_rank(((data->args).allgather.comm->EMPI_COMM_CMP),&rank);
					EMPI_Comm_size(((data->args).allgather.comm->EMPI_COMM_CMP),&commnC);
					if(((data->args).allgather.comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size(((data->args).allgather.comm->EMPI_CMP_REP_INTERCOMM),&commnR);
					else commnR = 0;
					
					if(data->id > repid && repid >= 0)
					{
						if(commnR > 0 && cmpToRepMap[rank] == -1)
						{
							tmprcountsinter = (int *)malloc((commnC-commnR)*sizeof(int));
							tmprdisplsinter = (int *)malloc((commnC-commnR)*sizeof(int));
							int j = 0;
							for(int i = 0; i < commnC; i++)
							{
								if(cmpToRepMap[i] == -1)
								{
									tmprcountsinter[j] = (data->args).allgather.recvcount;
									tmprdisplsinter[j] = i*(data->args).allgather.recvcount;
									j++;
								}
							}
							reqs = (EMPI_Request *)malloc(commnR*sizeof(EMPI_Request));
							completereq = (bool *)malloc(commnR*sizeof(bool));
							for(int i = 0; i < commnR; i++)
							{
								completereq[i] = false;
								retValinter = check_errhandler_conditions_collective(((data->args).allgather.comm->oworldComm),&reqs[i]);
								if(retValinter == 0)
								{
									retValinter = EMPI_Igatherv ((data->args).allgather.sendbuf, (data->args).allgather.sendcount, (data->args).allgather.senddt->edatatype, (data->args).allgather.recvbuf, tmprcountsinter, tmprdisplsinter, (data->args).allgather.recvdt->edatatype, i, ((data->args).allgather.comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
								}
								if(retValinter != 0) break;
							}
						}
						if(retValinter != 0)
						{
							if(commnR > 0 && cmpToRepMap[rank] == -1)
							{
								free(reqs);
								free(completereq);
								free(tmprcountsinter);
								free(tmprdisplsinter);
							}
							continue;
						}
					}
					
					
					if(data->id > cmpid) retVal = check_errhandler_conditions_collective(((data->args).allgather.comm)->oworldComm,&req);
					if(retVal == 0)
					{
						if(data->id > cmpid) retVal = EMPI_Iallgather ((data->args).allgather.sendbuf, (data->args).allgather.sendcount, (data->args).allgather.senddt->edatatype, (data->args).allgather.recvbuf, (data->args).allgather.recvcount, (data->args).allgather.recvdt->edatatype, ((data->args).allgather.comm)->EMPI_COMM_CMP, &req);
						int flag = !(data->id > cmpid);
						do
						{
							if(data->id > cmpid && flag == 0) retVal = check_errhandler_conditions_collective(((data->args).allgather.comm)->oworldComm,&req);
							if(retVal != 0) break;
							if(data->id > cmpid  && flag == 0) EMPI_Test(&req,&flag,&status);
						} while(flag == 0);
					}
					
					if(retVal != 0)
					{
						if(commnR > 0 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0)
						{
							free(reqs);
							free(completereq);
							free(tmprcountsinter);
							free(tmprdisplsinter);
						}
						continue;
					}
					
					retValinter = 0;
					if(commnR > 0 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0 && data->completerep) flagcount = 0;
					else flagcount = commnR;
					int flaginter = 0;
					do
					{
						flaginter = 0;
						if(commnR > 0 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0 && data->completerep)
						{
							for(int i = 0; i < commnR; i++)
							{
								if(!completereq[i])
								{
									flaginter = 0;
									retValinter = check_errhandler_conditions_collective(((data->args).allgather.comm->oworldComm),&reqs[i]);
									if(retValinter != 0) break;
									EMPI_Test(&reqs[i],&flaginter,&status);
									flagcount += flaginter;
									if(flaginter > 0) completereq[i] = true;
								}
							}
						}
						if(retValinter != 0) break;
					} while(flagcount < commnR);
					if(retValinter != 0)
					{
						if(commnR > 0 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0)
						{
							free(reqs);
							free(completereq);
							free(tmprcountsinter);
							free(tmprdisplsinter);
						}
					}
					
				} while(retVal != 0 || retValinter != 0);
			}
			if(!data->completerep)
			{
				data->repreq = reqs;
			}
		}
		else if(data->type == MPI_FT_ALLTOALL)
		{
			int retValinter;
			int *tmpscounts;
			int *tmpsdispls;
			int *tmprcounts;
			int *tmprdispls;
			int *tmpscountsinter;
			int *tmpsdisplsinter;
			int *intermapping;
			EMPI_Request *reqs;
			bool *completereq;
			if(!data->completerep)
			{
				reqs = data->repreq;
				free(reqs);
			}
			int commnC;
			int commnR;
			int rank;
			int noreprank;
			int flagcount;
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL)
			{
				do
				{
					retVal = 0;
					retValinter = 0;
					EMPI_Comm_rank((((data->args).alltoall.comm)->EMPI_COMM_REP),&rank);
					EMPI_Comm_size((((data->args).alltoall.comm)->EMPI_COMM_REP),&commnR);
					EMPI_Comm_remote_size((((data->args).alltoall.comm)->EMPI_CMP_REP_INTERCOMM),&commnC);
					if(data->id > repid && repid >= 0) tmpscounts = (int *)malloc(commnR*sizeof(int));
					if(data->id > repid && repid >= 0) tmpsdispls = (int *)malloc(commnR*sizeof(int));
					if(data->id > repid && repid >= 0) tmprcounts = (int *)malloc(commnR*sizeof(int));
					if(data->id > repid && repid >= 0) tmprdispls = (int *)malloc(commnR*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid) tmpscountsinter = (int *)malloc((commnR)*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid) tmpsdisplsinter = (int *)malloc((commnR)*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid) intermapping = (int *)malloc((commnC-commnR)*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid) reqs = (EMPI_Request *)malloc((commnC-commnR)*sizeof(EMPI_Request));
					if(commnC > commnR && commnR > 0 && data->id > repid) completereq = (bool *)malloc((commnC-commnR)*sizeof(bool));
					if(data->id > repid && repid >= 0)
					{
						int j = 0;
						int k = 0;
						for(int i = 0; i < commnC; i++)
						{
							if(cmpToRepMap[i] != -1)
							{
								tmpscounts[j] = (data->args).alltoall.sendcount;
								tmpsdispls[j] = i*(data->args).alltoall.sendcount;
								tmprcounts[j] = (data->args).alltoall.recvcount;
								tmprdispls[j] = i*(data->args).alltoall.recvcount;
								if(commnC > commnR && commnR > 0)
								{
									tmpscountsinter[j] = (data->args).alltoall.sendcount;
									tmpsdisplsinter[j] = i*(data->args).alltoall.sendcount;
								}
								j++;
							}
							else if(commnC > commnR && commnR > 0)
							{
								intermapping[k] = i;
								k++;
							}
						}
						
						if(commnC > commnR && commnR > 0)
						{
							reqs = (EMPI_Request *)malloc((commnC-commnR)*sizeof(EMPI_Request));
							completereq = (bool *)malloc((commnC-commnR)*sizeof(bool));
							for(int i = 0; i < commnC-commnR; i++)
							{
								completereq[i] = false;
								retValinter = check_errhandler_conditions_collective(((data->args).alltoall.comm->oworldComm),&reqs[i]);
								if(retValinter == 0)
								{
									retValinter = EMPI_Iscatterv ((data->args).alltoall.sendbuf, tmpscountsinter, tmpsdisplsinter, (data->args).alltoall.senddt->edatatype, (data->args).alltoall.recvbuf + intermapping[i]*(data->args).alltoall.recvcount*(data->args).alltoall.recvdt->size, (data->args).alltoall.recvcount, (data->args).alltoall.recvdt->edatatype, i, ((data->args).alltoall.comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
								}
								if(retValinter != 0) break;
							}
						}
						if(retValinter != 0)
						{
							if(commnC > commnR && commnR > 0) free(reqs);
							if(commnC > commnR && commnR > 0) free(completereq);
							free(tmpscounts);
							free(tmpsdispls);
							free(tmprcounts);
							free(tmprdispls);
							if(commnC > commnR && commnR > 0) free(tmpscountsinter);
							if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
							if(commnC > commnR && commnR > 0) free(intermapping);
							continue;
						}
						
						retVal = check_errhandler_conditions_collective(((data->args).alltoall.comm)->oworldComm,&req);
						if(retVal == 0)
						{
							retVal = EMPI_Ialltoallv ((data->args).alltoall.sendbuf, tmpscounts, tmpsdispls, (data->args).alltoall.senddt->edatatype, (data->args).alltoall.recvbuf, tmprcounts, tmprdispls, (data->args).alltoall.recvdt->edatatype, ((data->args).alltoall.comm)->EMPI_COMM_REP, &req);
							int flag = 0;
							do
							{
								retVal = check_errhandler_conditions_collective(((data->args).alltoall.comm)->oworldComm,&req);
								if(retVal != 0) break;
								EMPI_Test(&req,&flag,&status);
							} while(flag == 0);
						}
						if(retVal != 0)
						{
							if(commnC > commnR && commnR > 0) free(reqs);
							if(commnC > commnR && commnR > 0) free(completereq);
							free(tmpscounts);
							free(tmpsdispls);
							free(tmprcounts);
							free(tmprdispls);
							if(commnC > commnR && commnR > 0) free(tmpscountsinter);
							if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
							if(commnC > commnR && commnR > 0) free(intermapping);
							continue;
						}
						
						retValinter = 0;
						if(commnC > commnR && commnR > 0) flagcount = 0;
						else flagcount = commnC-commnR;
						int flaginter = 0;
						do
						{
							flaginter = 0;
							if(commnC > commnR && commnR > 0)
							{
								for(int i = 0; i < commnC-commnR; i++)
								{
									if(!completereq[i])
									{
										flaginter = 0;
										retValinter = check_errhandler_conditions_collective(((data->args).alltoall.comm->oworldComm),&reqs[i]);
										if(retValinter != 0) break;
										EMPI_Test(&reqs[i],&flaginter,&status);
										flagcount += flaginter;
										if(flaginter > 0) completereq[i] = true;
									}
								}
							}
							if(retValinter != 0) break;
						} while(flagcount < commnC-commnR);
						if(retValinter != 0)
						{
							if(commnC > commnR && commnR > 0) free(reqs);
							if(commnC > commnR && commnR > 0) free(completereq);
							free(tmpscounts);
							free(tmpsdispls);
							free(tmprcounts);
							free(tmprdispls);
							if(commnC > commnR && commnR > 0) free(tmpscountsinter);
							if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
							if(commnC > commnR && commnR > 0) free(intermapping);
						}	
					}
				} while(retVal != 0 || retValinter != 0);
			}
			else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{	
				do
				{
					retVal = 0;
					retValinter = 0;
					EMPI_Comm_rank(((data->args).alltoall.comm->EMPI_COMM_CMP),&rank);
					EMPI_Comm_size(((data->args).alltoall.comm->EMPI_COMM_CMP),&commnC);
					if(((data->args).alltoall.comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size(((data->args).alltoall.comm->EMPI_CMP_REP_INTERCOMM),&commnR);
					else commnR = 0;
					
					if(data->id > repid && repid >= 0)
					{
						if(commnR > 0 && cmpToRepMap[rank] == -1)
						{
							tmpscountsinter = (int *)malloc((commnR)*sizeof(int));
							tmpsdisplsinter = (int *)malloc((commnR)*sizeof(int));
							intermapping = (int *)malloc((commnC-commnR)*sizeof(int));
							int j = 0;
							int k = 0;
							for(int i = 0; i < commnC; i++)
							{
								if(cmpToRepMap[i] != -1)
								{
									tmpscountsinter[j] = (data->args).alltoall.sendcount;
									tmpsdisplsinter[j] = i*(data->args).alltoall.sendcount;
									j++;
								}
								else
								{
									intermapping[k] = i;
									k++;
								}
							}
							reqs = (EMPI_Request *)malloc((commnC-commnR)*sizeof(EMPI_Request));
							completereq = (bool *)malloc((commnC-commnR)*sizeof(bool));
							EMPI_Comm_rank(((data->args).alltoall.comm->EMPI_CMP_NO_REP),&noreprank);
							for(int i = 0; i < commnC-commnR; i++)
							{
								completereq[i] = false;
								retValinter = check_errhandler_conditions_collective(((data->args).alltoall.comm->oworldComm),&reqs[i]);
								if(retValinter == 0)
								{
									if(noreprank == i) retValinter = EMPI_Iscatterv ((data->args).alltoall.sendbuf, tmpscountsinter, tmpsdisplsinter, (data->args).alltoall.senddt->edatatype, (data->args).alltoall.recvbuf + intermapping[i]*(data->args).alltoall.recvcount*(data->args).alltoall.recvdt->size, (data->args).alltoall.recvcount, (data->args).alltoall.recvdt->edatatype, EMPI_ROOT, ((data->args).alltoall.comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
									else retValinter = EMPI_Iscatterv ((data->args).alltoall.sendbuf, tmpscountsinter, tmpsdisplsinter, (data->args).alltoall.senddt->edatatype, (data->args).alltoall.recvbuf + intermapping[i]*(data->args).alltoall.recvcount*(data->args).alltoall.recvdt->size, (data->args).alltoall.recvcount, (data->args).alltoall.recvdt->edatatype, EMPI_PROC_NULL, ((data->args).alltoall.comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
								}
								if(retValinter != 0) break;
							}
						}
						if(retValinter != 0)
						{
							if(commnR > 0 && cmpToRepMap[rank] == -1)
							{
								free(reqs);
								free(completereq);
								free(tmpscountsinter);
								free(tmpsdisplsinter);
								free(intermapping);
							}
							continue;
						}
					}
					
					
					if(data->id > cmpid) retVal = check_errhandler_conditions_collective(((data->args).alltoall.comm)->oworldComm,&req);
					if(retVal == 0)
					{
						if(data->id > cmpid) retVal = EMPI_Ialltoall ((data->args).alltoall.sendbuf, (data->args).alltoall.sendcount, (data->args).alltoall.senddt->edatatype, (data->args).alltoall.recvbuf, (data->args).alltoall.recvcount, (data->args).alltoall.recvdt->edatatype, ((data->args).alltoall.comm)->EMPI_COMM_CMP, &req);
						int flag = !(data->id > cmpid);
						do
						{
							if(data->id > cmpid && flag == 0) retVal = check_errhandler_conditions_collective(((data->args).alltoall.comm)->oworldComm,&req);
							if(retVal != 0) break;
							if(data->id > cmpid  && flag == 0) EMPI_Test(&req,&flag,&status);
						} while(flag == 0);
					}
					
					if(retVal != 0)
					{
						if(commnR > 0 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0)
						{
							free(reqs);
							free(completereq);
							free(tmpscountsinter);
							free(tmpsdisplsinter);
							free(intermapping);
						}
						continue;
					}
					
					retValinter = 0;
					if(commnR > 0 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0 && data->completerep) flagcount = 0;
					else flagcount = commnC-commnR;
					int flaginter = 0;
					do
					{
						flaginter = 0;
						if(commnR > 0 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0 && data->completerep)
						{
							for(int i = 0; i < commnC-commnR; i++)
							{
								if(!completereq[i])
								{
									flaginter = 0;
									retValinter = check_errhandler_conditions_collective(((data->args).alltoall.comm->oworldComm),&reqs[i]);
									if(retValinter != 0) break;
									EMPI_Test(&reqs[i],&flaginter,&status);
									flagcount += flaginter;
									if(flaginter > 0) completereq[i] = true;
								}
							}
						}
						if(retValinter != 0) break;
					} while(flagcount < commnC-commnR);
					if(retValinter != 0)
					{
						if(commnR > 0 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0)
						{
							free(reqs);
							free(completereq);
							free(tmpscountsinter);
							free(tmpsdisplsinter);
							free(intermapping);
						}
					}
				} while(retVal != 0 || retValinter != 0);
			}
			if(!data->completerep)
			{
				data->repreq = reqs;
			}
		}
		else if(data->type == MPI_FT_ALLTOALLV)
		{
			int retValinter;
			int *tmpscounts;
			int *tmpsdispls;
			int *tmpsdisplsintra;
			int *tmprcounts;
			int *tmprdispls;
			int *tmprdisplsintra;
			int *tmpscountsinter;
			int *tmpsdisplsinter;
			int *intermapping;
			EMPI_Request *reqs;
			bool *completereq;
			if(!data->completerep)
			{
				reqs = data->repreq;
				free(reqs);
			}
			int commnC;
			int commnR;
			int rank;
			int noreprank;
			int flagcount;
			if(MPI_COMM_WORLD->EMPI_COMM_REP != EMPI_COMM_NULL)
			{
				do
				{
					retVal = 0;
					retValinter = 0;
					EMPI_Comm_rank((((data->args).alltoallv.comm)->EMPI_COMM_REP),&rank);
					EMPI_Comm_size((((data->args).alltoallv.comm)->EMPI_COMM_REP),&commnR);
					EMPI_Comm_remote_size((((data->args).alltoallv.comm)->EMPI_CMP_REP_INTERCOMM),&commnC);
					if(data->id > repid && repid >= 0) tmpscounts = (int *)malloc(commnR*sizeof(int));
					if(data->id > repid && repid >= 0) tmpsdispls = (int *)malloc(commnR*sizeof(int));
					if(data->id > repid && repid >= 0) tmprcounts = (int *)malloc(commnR*sizeof(int));
					if(data->id > repid && repid >= 0) tmprdispls = (int *)malloc(commnR*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid) tmpscountsinter = (int *)malloc((commnR)*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid) tmpsdisplsinter = (int *)malloc((commnR)*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid) intermapping = (int *)malloc((commnC-commnR)*sizeof(int));
					if(commnC > commnR && commnR > 0 && data->id > repid) reqs = (EMPI_Request *)malloc((commnC-commnR)*sizeof(EMPI_Request));
					if(commnC > commnR && commnR > 0 && data->id > repid) completereq = (bool *)malloc((commnC-commnR)*sizeof(bool));
					if(data->id > repid && repid >= 0)
					{
						int j = 0;
						int k = 0;
						int snumcounts = 0;
						int rnumcounts = 0;
						for(int i = 0; i < commnC; i++)
						{
							if(cmpToRepMap[i] != -1)
							{
								tmpscounts[j] = (data->args).alltoallv.sendcounts[i];
								//tmpsdispls[j] = (data->args).alltoallv.sdispls[i];
								tmpsdispls[j] = snumcounts;
								tmprcounts[j] = (data->args).alltoallv.recvcounts[i];
								//tmprdispls[j] = (data->args).alltoallv.rdispls[i];
								tmprdispls[j] = rnumcounts;
								if(commnC > commnR && commnR > 0)
								{
									tmpscountsinter[j] = (data->args).alltoallv.sendcounts[i];
									//tmpsdisplsinter[j] = (data->args).alltoallv.sdispls[i];
									tmpsdisplsinter[j] = snumcounts;
								}
								j++;
							}
							else if(commnC > commnR && commnR > 0)
							{
								intermapping[k] = i;
								k++;
							}
							snumcounts += (data->args).alltoallv.sendcounts[i];
							rnumcounts += (data->args).alltoallv.recvcounts[i];
						}
						
						if(commnC > commnR && commnR > 0)
						{
							reqs = (EMPI_Request *)malloc((commnC-commnR)*sizeof(EMPI_Request));
							completereq = (bool *)malloc((commnC-commnR)*sizeof(bool));
							for(int i = 0; i < commnC-commnR; i++)
							{
								completereq[i] = false;
								retValinter = check_errhandler_conditions_collective(((data->args).alltoallv.comm->oworldComm),&reqs[i]);
								if(retValinter == 0)
								{
									retValinter = EMPI_Iscatterv ((data->args).alltoallv.sendbuf, tmpscountsinter, tmpsdisplsinter, (data->args).alltoallv.senddt->edatatype, (data->args).alltoallv.recvbuf/* + (data->args).alltoallv.rdispls[intermapping[i]]*(data->args).alltoallv.recvdt->size*/, (data->args).alltoallv.recvcounts[intermapping[i]], (data->args).alltoallv.recvdt->edatatype, i, ((data->args).alltoallv.comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
								}
								if(retValinter != 0) break;
							}
						}
						if(retValinter != 0)
						{
							if(commnC > commnR && commnR > 0) free(reqs);
							if(commnC > commnR && commnR > 0) free(completereq);
							free(tmpscounts);
							free(tmpsdispls);
							free(tmprcounts);
							free(tmprdispls);
							if(commnC > commnR && commnR > 0) free(tmpscountsinter);
							if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
							if(commnC > commnR && commnR > 0) free(intermapping);
							continue;
						}
						
						retVal = check_errhandler_conditions_collective(((data->args).alltoallv.comm)->oworldComm,&req);
						if(retVal == 0)
						{
							retVal = EMPI_Ialltoallv ((data->args).alltoallv.sendbuf, tmpscounts, tmpsdispls, (data->args).alltoallv.senddt->edatatype, (data->args).alltoallv.recvbuf, tmprcounts, tmprdispls, (data->args).alltoallv.recvdt->edatatype, ((data->args).alltoallv.comm)->EMPI_COMM_REP, &req);
							int flag = 0;
							do
							{
								retVal = check_errhandler_conditions_collective(((data->args).alltoallv.comm)->oworldComm,&req);
								if(retVal != 0) break;
								EMPI_Test(&req,&flag,&status);
							} while(flag == 0);
						}
						if(retVal != 0)
						{
							if(commnC > commnR && commnR > 0) free(reqs);
							if(commnC > commnR && commnR > 0) free(completereq);
							free(tmpscounts);
							free(tmpsdispls);
							free(tmprcounts);
							free(tmprdispls);
							if(commnC > commnR && commnR > 0) free(tmpscountsinter);
							if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
							if(commnC > commnR && commnR > 0) free(intermapping);
							continue;
						}
						
						retValinter = 0;
						if(commnC > commnR && commnR > 0) flagcount = 0;
						else flagcount = commnC-commnR;
						int flaginter = 0;
						do
						{
							flaginter = 0;
							if(commnC > commnR && commnR > 0)
							{
								for(int i = 0; i < commnC-commnR; i++)
								{
									if(!completereq[i])
									{
										flaginter = 0;
										retValinter = check_errhandler_conditions_collective(((data->args).alltoallv.comm->oworldComm),&reqs[i]);
										if(retValinter != 0) break;
										EMPI_Test(&reqs[i],&flaginter,&status);
										flagcount += flaginter;
										if(flaginter > 0) completereq[i] = true;
									}
								}
							}
							if(retValinter != 0) break;
						} while(flagcount < commnC-commnR);
						if(retValinter != 0)
						{
							if(commnC > commnR && commnR > 0) free(reqs);
							if(commnC > commnR && commnR > 0) free(completereq);
							free(tmpscounts);
							free(tmpsdispls);
							free(tmprcounts);
							free(tmprdispls);
							if(commnC > commnR && commnR > 0) free(tmpscountsinter);
							if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
							if(commnC > commnR && commnR > 0) free(intermapping);
						}	
					}
				} while(retVal != 0 || retValinter != 0);
			}
			else if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{	
				do
				{
					retVal = 0;
					retValinter = 0;
					EMPI_Comm_rank(((data->args).alltoallv.comm->EMPI_COMM_CMP),&rank);
					EMPI_Comm_size(((data->args).alltoallv.comm->EMPI_COMM_CMP),&commnC);
					if(((data->args).alltoallv.comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size(((data->args).alltoallv.comm->EMPI_CMP_REP_INTERCOMM),&commnR);
					else commnR = 0;
					tmpsdisplsintra = (int *)malloc((commnC)*sizeof(int));
					tmprdisplsintra = (int *)malloc((commnC)*sizeof(int));
					
					if(data->id > repid && repid >= 0)
					{
						if(commnR > 0 && cmpToRepMap[rank] == -1)
						{
							tmpscountsinter = (int *)malloc((commnR)*sizeof(int));
							tmpsdisplsinter = (int *)malloc((commnR)*sizeof(int));
							intermapping = (int *)malloc((commnC-commnR)*sizeof(int));
							int j = 0;
							int k = 0;
							int snumcounts = 0;
							for(int i = 0; i < commnC; i++)
							{
								if(cmpToRepMap[i] != -1)
								{
									tmpscountsinter[j] = (data->args).alltoallv.sendcounts[i];
									//tmpsdisplsinter[j] = (data->args).alltoallv.sdispls[i];
									tmpsdisplsinter[j] = snumcounts;
									j++;
								}
								else
								{
									intermapping[k] = i;
									k++;
								}
								snumcounts += (data->args).alltoallv.sendcounts[i];
							}
							reqs = (EMPI_Request *)malloc((commnC-commnR)*sizeof(EMPI_Request));
							completereq = (bool *)malloc((commnC-commnR)*sizeof(bool));
							EMPI_Comm_rank(((data->args).alltoallv.comm->EMPI_CMP_NO_REP),&noreprank);
							for(int i = 0; i < commnC-commnR; i++)
							{
								completereq[i] = false;
								retValinter = check_errhandler_conditions_collective(((data->args).alltoallv.comm->oworldComm),&reqs[i]);
								if(retValinter == 0)
								{
									if(noreprank == i) retValinter = EMPI_Iscatterv ((data->args).alltoallv.sendbuf, tmpscountsinter, tmpsdisplsinter, (data->args).alltoallv.senddt->edatatype, (data->args).alltoallv.recvbuf/* + (data->args).alltoallv.rdispls[intermapping[i]]*(data->args).alltoallv.recvdt->size*/, (data->args).alltoallv.recvcounts[intermapping[i]], (data->args).alltoallv.recvdt->edatatype, EMPI_ROOT, ((data->args).alltoallv.comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
									else retValinter = EMPI_Iscatterv ((data->args).alltoallv.sendbuf, tmpscountsinter, tmpsdisplsinter, (data->args).alltoallv.senddt->edatatype, (data->args).alltoallv.recvbuf/* + (data->args).alltoallv.rdispls[intermapping[i]]*(data->args).alltoallv.recvdt->size*/, (data->args).alltoallv.recvcounts[intermapping[i]], (data->args).alltoallv.recvdt->edatatype, EMPI_PROC_NULL, ((data->args).alltoallv.comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
								}
								if(retValinter != 0) break;
							}
						}
						if(retValinter != 0)
						{
							if(commnR > 0 && cmpToRepMap[rank] == -1)
							{
								free(reqs);
								free(completereq);
								free(tmpscountsinter);
								free(tmpsdisplsinter);
								free(intermapping);
							}
							continue;
						}
					}
					if(data->id > cmpid)
					{
						int snumcounts = 0;
						int rnumcounts = 0;
						for(int i = 0; i < commnC; i++)
						{
							tmpsdisplsintra[i] = snumcounts;
							tmprdisplsintra[i] = rnumcounts;
							snumcounts += (data->args).alltoallv.sendcounts[i];
							rnumcounts += (data->args).alltoallv.recvcounts[i];
						}
					}
					
					
					if(data->id > cmpid) retVal = check_errhandler_conditions_collective(((data->args).alltoallv.comm)->oworldComm,&req);
					if(retVal == 0)
					{
						if(data->id > cmpid) retVal = EMPI_Ialltoallv ((data->args).alltoallv.sendbuf, (data->args).alltoallv.sendcounts, /*(data->args).alltoallv.sdispls*/tmpsdisplsintra, (data->args).alltoallv.senddt->edatatype, (data->args).alltoallv.recvbuf, (data->args).alltoallv.recvcounts, /*(data->args).alltoallv.rdispls*/tmprdisplsintra, (data->args).alltoallv.recvdt->edatatype, ((data->args).alltoallv.comm)->EMPI_COMM_CMP, &req);
						int flag = !(data->id > cmpid);
						do
						{
							if(data->id > cmpid && flag == 0) retVal = check_errhandler_conditions_collective(((data->args).alltoallv.comm)->oworldComm,&req);
							if(retVal != 0) break;
							if(data->id > cmpid  && flag == 0) EMPI_Test(&req,&flag,&status);
						} while(flag == 0);
					}
					
					if(retVal != 0)
					{
						if(commnR > 0 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0)
						{
							free(reqs);
							free(completereq);
							free(tmpscountsinter);
							free(tmpsdisplsinter);
							free(intermapping);
						}
						continue;
					}
					
					retValinter = 0;
					if(commnR > 0 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0 && data->completerep) flagcount = 0;
					else flagcount = commnC-commnR;
					int flaginter = 0;
					do
					{
						flaginter = 0;
						if(commnR > 0 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0 && data->completerep)
						{
							for(int i = 0; i < commnC-commnR; i++)
							{
								if(!completereq[i])
								{
									flaginter = 0;
									retValinter = check_errhandler_conditions_collective(((data->args).alltoallv.comm->oworldComm),&reqs[i]);
									if(retValinter != 0) break;
									EMPI_Test(&reqs[i],&flaginter,&status);
									flagcount += flaginter;
									if(flaginter > 0) completereq[i] = true;
								}
							}
						}
						if(retValinter != 0) break;
					} while(flagcount < commnC-commnR);
					if(retValinter != 0)
					{
						if(commnR > 0 && cmpToRepMap[rank] == -1 && data->id > repid && repid >= 0)
						{
							free(reqs);
							free(completereq);
							free(tmpscountsinter);
							free(tmpsdisplsinter);
							free(intermapping);
						}
					}
				} while(retVal != 0 || retValinter != 0);
			}
			if(!data->completerep)
			{
				data->repreq = reqs;
			}
		}
		else if(data->type == MPI_FT_ALLREDUCE)
		{
			if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				if(data->id > cmpid)
				{
					do
					{
						retVal = 0;
						retVal = check_errhandler_conditions_collective(((data->args).allreduce.comm)->oworldComm,&req);
						
						if(retVal == 0)
						{
							retVal = EMPI_Iallreduce ((data->args).allreduce.sendbuf, (data->args).allreduce.recvbuf, (data->args).allreduce.count, (data->args).allreduce.dt->edatatype, ((data->args).allreduce.op)->eop, ((data->args).allreduce.comm)->EMPI_COMM_CMP, &req);
							int flag = 0;
							do
							{
								retVal = check_errhandler_conditions_collective(((data->args).allreduce.comm)->oworldComm,&req);
								if(retVal != 0) break;
								EMPI_Test(&req,&flag,&status);
							} while(flag == 0);
						}
					} while(retVal != 0);
				}
				if(data->id > myrepid && myrepid >= 0)
				{
					do
					{
						retVal = 0;
						EMPI_Comm_rank(((data->args).allreduce.comm)->EMPI_COMM_CMP,&rank);
						if(cmpToRepMap[rank] == -1) break;
						retVal = check_errhandler_conditions(((data->args).allreduce.comm)->oworldComm,&req,cmpToRepMap[rank]+nC);
						
						if(retVal == 0)
						{
							retVal = EMPI_Isend ((data->args).allreduce.recvbuf, (data->args).allreduce.count, (data->args).allreduce.dt->edatatype, cmpToRepMap[rank], MPI_FT_COLLECTIVE_TAG, ((data->args).allreduce.comm)->EMPI_CMP_REP_INTERCOMM, &req);
							int flag = 0;
							do
							{
								retVal = check_errhandler_conditions(((data->args).allreduce.comm)->oworldComm,&req,cmpToRepMap[rank]);
								if(retVal != 0) break;
								EMPI_Test(&req,&flag,&status);
							} while(flag == 0);
						}
					} while(retVal != 0);
				}
			}
		}
		data = data->prev;
	}
}

bool IsRecvComplete(int src, int id)
{
  if(ondisk) fseek(logfile, 0, SEEK_SET);
  bool rcomp = false;
  char *mode;
  char *line = NULL;
  size_t llen = 0;
  ssize_t read;
  mode = (char*) malloc(50 * sizeof(char));
  if(ondisk)
  {
    while ((read = getline(&line, &llen, logfile)) != -1)
    {
      GetFirstWord(line,mode);
      if(strcmp (mode,"RECV") == 0)
      {
        int logsrc, logid;
        GetSrcAndId(line, &logid, &logsrc);
        if((logid == id) && (logsrc == src)) rcomp = true;
      }
    }
  }
  for(int i=0; i<nbufentries;i++)
  {
    GetFirstWord(logbuffer[i],mode);
    if(strcmp (mode,"RECV") == 0)
    {
      int logsrc, logid;
      GetSrcAndId(logbuffer[i], &logid, &logsrc);
      if((logid == id) && (logsrc == src)) rcomp = true;
    }
  }
  return rcomp;
}

void GetSrcAndId(char *line, int *id, int *src)
{
  int i;
  int wordnum = 0;
  char *word;
  word = (char*) malloc(100 * sizeof(char));
  int wordlen = 0;
  for (i = 0; i <  strlen(line); i++)
  {
    if (line[i] == ' ' || line[i] == '\n' || line[i] == '\0')
    {
      word[wordlen] = '\0';
      wordlen++;
      if(wordnum == 1)
      {
        *id = atoi(word);
      }
      else if(wordnum == 2)
      {
        *src = atoi(word);
      }
      wordlen = 0;
      wordnum++;
    }
    else
    {
      word[wordlen] = line[i];
      wordlen++;
    }
  }
}

void GetDestAndId(char *line, int *id, int *dest)
{
  int i;
  int wordnum = 0;
  char *word;
  word = (char*) malloc(100 * sizeof(char));
  int wordlen = 0;
  for (i = 0; i <  strlen(line); i++)
  {
    if (line[i] == ' ' || line[i] == '\n' || line[i] == '\0')
    {
      word[wordlen] = '\0';
      wordlen++;
      if(wordnum == 1)
      {
        *id = atoi(word);
      }
      else if(wordnum == 4)
      {
        *dest = atoi(word);
      }
      wordlen = 0;
      wordnum++;
    }
    else
    {
      word[wordlen] = line[i];
      wordlen++;
    }
  }
}

int check_errhandler_conditions_collective(OMPI_Comm ocomm, EMPI_Request *req)
{
  int flag;
  OMPIX_Comm_is_revoked(ocomm,&flag);
  if(flag)
  {
		//if(*req != EMPI_REQUEST_NULL)
		//{
			//EMPI_Cancel(req);
		//	EMPI_Request_free(req);
		//}
    int err = OMPI_ERR_REVOKED;
    //printf("Entering error handler from revoked side\n");
    HandleErrors(&ocomm,&err);
    //printf("exited error handler from revoked side\n");
    return err;
  }
  else
  {
    OMPI_Group failed_group;
    OMPIX_Comm_failure_ack(ocomm);
    OMPIX_Comm_failure_get_acked(ocomm,&failed_group);
    int fsz = 0;
    OMPI_Group_size(failed_group,&fsz);
    if(fsz > 0)
    {
			//if(*req != EMPI_REQUEST_NULL)
			//{
				//EMPI_Cancel(req);
			//	EMPI_Request_free(req);
			//}
      int err = OMPI_ERR_PROC_FAILED;
      HandleErrors(&ocomm,&err);
      return err;
    }
  }
  return 0;
}

int check_errhandler_conditions(OMPI_Comm ocomm, EMPI_Request *req, int rank)
{
  int flag;
  
  //if(ocomm == OMPI_COMM_NULL) printf("Comm is revoked check %d\n",ocomm == OMPI_COMM_NULL);
  OMPIX_Comm_is_revoked(ocomm,&flag);
  //if(OMPI_COMM_REP != OMPI_COMM_NULL) printf("comm revoked? %d %d\n",flag,getpid());
  if(flag)
  {
    //printf("detected revoked communicator %d\n",*req != EMPI_REQUEST_NULL);
    OMPI_Group failed_group;
    OMPIX_Comm_failure_ack(ocomm);
    OMPIX_Comm_failure_get_acked(ocomm,&failed_group);
    int fsz = 0;
    OMPI_Group_size(failed_group,&fsz);
    //printf("failed size %d\n",fsz);
    if(fsz > 0)
    {
      OMPI_Group oGroup;
      int isInter;
      OMPI_Comm_test_inter(ocomm,&isInter);
      if(isInter) OMPI_Comm_remote_group(ocomm,&oGroup);
      else OMPI_Comm_group(ocomm,&oGroup);
      int *ranks_gf,*ranks_gc;
      ranks_gf = (int*)malloc(fsz * sizeof(int));
      ranks_gc = (int*)malloc(fsz * sizeof(int));
      for(int i = 0; i < fsz; i++) ranks_gf[i] = i;
      OMPI_Group_translate_ranks(failed_group, fsz, ranks_gf, oGroup, ranks_gc);
      for(int i = 0; i < fsz; i++)
      {
        if(ranks_gc[i] == rank)
        {
          if(*req != EMPI_REQUEST_NULL)
          {
            EMPI_Cancel(req);
            EMPI_Request_free(req);
          }
          int err = OMPI_ERR_PROC_FAILED;
          HandleErrors(&ocomm,&err);
          return err;
        }
      }
    }
    int err = OMPI_ERR_REVOKED;
    //printf("Entering error handler from revoked side %d %d\n",rank,*req!=EMPI_REQUEST_NULL);
    HandleErrors(&ocomm,&err);
    //printf("exited error handler from revoked side rank was %d\n",rank);
    return err;
  }
  else
  {
    OMPI_Group failed_group;
    OMPIX_Comm_failure_ack(ocomm);
    OMPIX_Comm_failure_get_acked(ocomm,&failed_group);
    int fsz = 0;
    OMPI_Group_size(failed_group,&fsz);
    //printf("failed size %d\n",fsz);
    if(fsz > 0)
    {
      OMPI_Group oGroup;
      int isInter;
      OMPI_Comm_test_inter(ocomm,&isInter);
      if(isInter) OMPI_Comm_remote_group(ocomm,&oGroup);
      else OMPI_Comm_group(ocomm,&oGroup);
      int *ranks_gf,*ranks_gc;
      ranks_gf = (int*)malloc(fsz * sizeof(int));
      ranks_gc = (int*)malloc(fsz * sizeof(int));
      for(int i = 0; i < fsz; i++) ranks_gf[i] = i;
      OMPI_Group_translate_ranks(failed_group, fsz, ranks_gf, oGroup, ranks_gc);
      for(int i = 0; i < fsz; i++)
      {
        if(ranks_gc[i] == rank)
        {
          if(*req != EMPI_REQUEST_NULL)
          {
            EMPI_Cancel(req);
            EMPI_Request_free(req);
          }
					int err = OMPI_ERR_PROC_FAILED;
          HandleErrors(&ocomm,&err);
          return err;
        }
      }
			int err = OMPI_ERR_REVOKED;
			HandleErrors(&ocomm,&err);
			return err;
    }
  }
  return 0;
}
/*
void SendUsingLine(char *line, int *worldComm_ranks)
{
  int i;
  int wordnum = 0;
  char *word;
  word = (char*) malloc(100 * sizeof(char));
  int wordlen = 0;
  int count;
  int id;
  EMPI_Datatype datatype;
  int dest;
  int actualdest;
  int tag;
  void *buf;
  EMPI_Comm comm;
  OMPI_Comm ocomm;
  EMPI_Request req;
  req = EMPI_REQUEST_NULL;
  EMPI_Status status;
  bool iscmpproc;
  for (i = 0; i <  strlen(line); i++)
  {
    if (line[i] == ' ' || line[i] == '\n' || line[i] == '\0')
    {
      word[wordlen] = '\0';
      wordlen++;
      if(wordnum == 1)
      {
        id = atoi(word);
      }
      if(wordnum == 2)
      {
        count = atoi(word);
      }
      else if(wordnum == 3)
      {
        datatype = EMPI_INT;
        if(datatype == EMPI_INT)
        {
          buf = (int*) malloc(count * sizeof(int));
        }
      }
      else if(wordnum == 4)
      {
        dest = atoi(word);
      }
      else if(wordnum == 5)
      {
        tag = atoi(word);
      }
      else if(wordnum == 6)
      {
        *(int *)buf = atoi(word);
      }
      wordlen = 0;
      wordnum++;
    }
    else
    {
      word[wordlen] = line[i];
      wordlen++;
    }
  }
  for(int i = 0; i<nR+nC;i++)
  {
    if(dest == worldComm_ranks[i])
    {
      dest = i;
      break;
    }
  }
  if(EMPI_COMM_CMP != EMPI_COMM_NULL)
  {
    iscmpproc = true;
    if(dest >= nC)
    {
      actualdest = dest-nC;
      comm = EMPI_CMP_REP_INTERCOMM;
      ocomm = OMPI_CMP_REP_INTERCOMM;
    }
    else
    {
      actualdest = dest;
      comm = EMPI_COMM_CMP;
      ocomm = OMPI_COMM_CMP;
    }
  }
  else if(EMPI_COMM_REP != EMPI_COMM_NULL)
  {
    iscmpproc = false;
    if(dest >= nC)
    {
      comm = EMPI_COMM_REP;
      ocomm = OMPI_COMM_CMP;
      actualdest = dest - nC;
    }
    else
    {
      actualdest = dest;
      comm = EMPI_CMP_REP_INTERCOMM;
      ocomm = OMPI_CMP_REP_INTERCOMM;
    }
  }
  int retVal;
  char *logstring;
  logstring = (char*) malloc(300 * sizeof(char));
  int backup;
  int dis;
  if(datatype == EMPI_INT)
  {
    dis = count*sizeof(int);
    backup = *(int *)(buf+dis);
    *((int *)(buf+dis)) = id;
  }
  do
  {
    retVal = 0;
    //retVal = PMPI_Send(&id,1,MPI_INT,actualdest,100,comm);
    //detect_failed_proc(ocomm,actualdest,&req);
    retVal = check_errhandler_conditions(ocomm,&req,actualdest);
    if(retVal == 0)
    {
      retVal = EMPI_Isend(buf,count+1,datatype,actualdest,tag,comm,&req);
    
      int flag = 0;
      do
      {
        //detect_failed_proc(ocomm,actualdest,&req);
        retVal = check_errhandler_conditions(ocomm,&req,actualdest);
        if(retVal != 0) break;
        EMPI_Test(&req,&flag,&status);
      } while(flag == 0);
      //retVal = status.EMPI_ERROR;
    }
    
    if(!iscmpproc && EMPI_COMM_CMP != EMPI_COMM_NULL)
    {
      iscmpproc = true;
      if(dest >= nC)
      {
        actualdest = dest;
        comm = eworldComm;
      }
      else
      {
        actualdest = dest;
        comm = EMPI_COMM_CMP;
      }
    }
    if(retVal==0)
    {
      GenerateSendLog(logstring,id,buf,count,datatype,actualdest,tag,comm);
      if(nbufentries == nbuf)
      {
        for(int i = 0; i<nbuf;i++)
        {
          fputs(logbuffer[i],logfile);
        }
        fputs(logstring,logfile);
        fflush(logfile);
        ondisk = true;
        nbufentries = 0;
      }
      else
      {
        strncpy(logbuffer[nbufentries],logstring,strlen(logstring));
        nbufentries++;
      }
    }
  } while (retVal != 0);
  if(datatype == EMPI_INT)
  {
    *(int *)(buf+dis) = backup;
  }
}

void GenerateSendLog(char *logstr, int id, void *buf, int count, EMPI_Datatype datatype, int dest, int tag, EMPI_Comm comm)
{
  char *bufstr;
  int actualdest;
  if((comm == EMPI_COMM_CMP) || ((comm == EMPI_CMP_REP_INTERCOMM) && (EMPI_COMM_CMP == EMPI_COMM_NULL))) actualdest = dest;
  else if((comm == EMPI_COMM_REP) || ((comm == EMPI_CMP_REP_INTERCOMM) && (EMPI_COMM_REP == EMPI_COMM_NULL))) actualdest = dest + nC;
  bufstr = (char*) malloc(100 * sizeof(char));
  if(datatype == EMPI_INT)
  {
    int *p = (int *)buf;
    sprintf (bufstr, "%d", p[0]);
    for(int i = 1;i < count;i++)
    {
      sprintf (bufstr, "%s %d",bufstr, p[i]);
    }
  }
  sprintf (logstr, "SEND %d %d %d %d %d %s\n", id, count, datatype, actualdest, tag, bufstr);
}*/

/* MPI_Send */

int MPI_Send (void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
	int retVal,retValRep;
	EMPI_Request req = EMPI_REQUEST_NULL;
	EMPI_Request reqRep = EMPI_REQUEST_NULL;
	EMPI_Datatype edatatype;
	EMPI_Status status;
	int rank;
	int extracount;
	int dis;
	//void *backup;
	int size;
	struct peertopeer_data *curargs;
	curargs = (struct peertopeer_data *)malloc(sizeof(struct collective_data));
	EMPI_Type_size(datatype->edatatype,&size);
	dis = count*size;
	if(size >= sizeof(int)) extracount = 1;
	else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
	else extracount = (((int)sizeof(int))/size) + 1;
	curargs->buf = malloc((count+extracount)*size);
	curargs->req = (MPI_Request *)malloc(sizeof(MPI_Request));
	memcpy(curargs->buf,buf,count*size);
	memcpy((curargs->buf) + dis,&sendid,sizeof(int));
	//*((int *)((curargs->buf) + dis)) = sendid;
	//backup = malloc(extracount*size);
	//memcpy(backup,buf+dis,extracount*size);
	//backup = *(int *)(buf+dis);
	//*((int *)(buf+dis)) = sendid;
	bool skiprep, skipcmp;
	struct skiplist *skip,*oldskip;
	if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
	{
		do
		{
			retValRep = 0;
			skiprep = false;
			skip = skipreplist;
			oldskip = NULL;
			while(skip != NULL)
			{
				if(skip->id == sendid)
				{
					skiprep = true;
					if(oldskip != NULL) oldskip->next = skip->next;
					else skipreplist = skip->next;
					free(skip);
					break;
				}
				oldskip = skip;
				skip = skip->next;
			}
			if(!skiprep)
			{
				if(cmpToRepMap[dest] == -1) break;
				retValRep = check_errhandler_conditions((comm->oworldComm),&reqRep,cmpToRepMap[dest]+nC);
				if(retValRep == 0)
				{
					retValRep = EMPI_Isend (curargs->buf, count +extracount, datatype->edatatype, cmpToRepMap[dest], tag, (comm->EMPI_COMM_REP), &reqRep);
					curargs->sendtype = true;
					//curargs->buf = malloc(count*size);
					//memcpy(curargs->buf,buf,count*size);
					curargs->count = count;
					curargs->dt = datatype;
					curargs->target = dest;
					curargs->tag = tag;
					curargs->comm = comm;
					curargs->completecmp = true;
					curargs->completerep = true;
					//curargs->req = (MPI_Request *)malloc(sizeof(MPI_Request));
					*(curargs->req) = MPI_REQUEST_NULL;
					curargs->id = sendid;
					int flag = 0;
					do
					{
						retValRep = check_errhandler_conditions((comm->oworldComm),&reqRep,cmpToRepMap[dest]+nC);
						if(retValRep != 0) break;
						MPI_Request tmpreq;
						if(last_peertopeer != NULL) tmpreq = *(last_peertopeer->req);
						EMPI_Test(&reqRep,&flag,&status);
						if(last_peertopeer != NULL) *(last_peertopeer->req) = tmpreq;
					} while(flag == 0);
				}
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					printf("Converting to cmp at send %d\n",reqRep != EMPI_REQUEST_NULL);
					if(reqRep != EMPI_REQUEST_NULL) EMPI_Request_free(&reqRep);
					goto ConvertToCMPSEND;
				}
			} 
		} while(retValRep != 0);
		req = EMPI_REQUEST_NULL;
		reqRep = EMPI_REQUEST_NULL;
		sendid++;
		if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
		else first_peertopeer = curargs;
		curargs->prev = NULL;
		curargs->next = last_peertopeer;
		last_peertopeer = curargs;
//		do
//		{
//			retVal = 0;
//			retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
//			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
//			{
//				if(reqRep != EMPI_REQUEST_NULL) EMPI_Request_free(&reqRep);
//				goto ConvertToCMPSENDSTORED;
//			}
//		} while(retVal != 0);
	}
	else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
	{
ConvertToCMPSEND:
		do
		{
			retVal = 0;
			retValRep = 0;
			skiprep = false;
			skipcmp = false;
			skip = skipreplist;
			oldskip = NULL;
			while(skip != NULL)
			{
				if(skip->id == sendid)
				{
					skiprep = true;
					if(oldskip != NULL) oldskip->next = skip->next;
					else skipreplist = skip->next;
					free(skip);
					break;
				}
				oldskip = skip;
				skip = skip->next;
			}
			skip = skipcmplist;
			oldskip = NULL;
			while(skip != NULL)
			{
				if(skip->id == sendid)
				{
					skipcmp = true;
					if(oldskip != NULL) oldskip->next = skip->next;
					else skipcmplist = skip->next;
					free(skip);
					break;
				}
				oldskip = skip;
				skip = skip->next;
			}
			if(!skipcmp || !skiprep)
			{
				int rank;
				EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
				if(!skipcmp) retVal = check_errhandler_conditions((comm->oworldComm),&req,dest);
				if(cmpToRepMap[dest] != -1 && cmpToRepMap[rank] == -1 && !skiprep) retValRep = check_errhandler_conditions((comm->oworldComm),&reqRep,cmpToRepMap[dest]+nC);
				if(retVal == 0 && retValRep == 0)
				{
					if(!skipcmp)retVal = EMPI_Isend (curargs->buf, count+extracount, datatype->edatatype, dest, tag, (comm->EMPI_COMM_CMP), &req);
					if(cmpToRepMap[dest] != -1 && cmpToRepMap[rank] == -1 && !skiprep) retValRep = EMPI_Isend (curargs->buf, count+extracount, datatype->edatatype, cmpToRepMap[dest], tag, (comm->EMPI_CMP_REP_INTERCOMM), &reqRep);
					curargs->sendtype = true;
					//curargs->buf = malloc(count*size);
					//memcpy(curargs->buf,buf,count*size);
					curargs->count = count;
					curargs->dt = datatype;
					curargs->target = dest;
					curargs->tag = tag;
					curargs->comm = comm;
					curargs->completecmp = true;
					curargs->completerep = true;
					//curargs->req = (MPI_Request *)malloc(sizeof(MPI_Request));
					*(curargs->req) = MPI_REQUEST_NULL;
					curargs->id = sendid;
					int flag = skipcmp;
					int flagRep = !(cmpToRepMap[dest] != -1 && cmpToRepMap[rank] == -1 && !skiprep);
					do
					{
						if(!skipcmp && flag == 0) retVal = check_errhandler_conditions((comm->oworldComm),&req,dest);
						if(retVal != 0) break;
						if(cmpToRepMap[dest] != -1 && cmpToRepMap[rank] == -1 && !skiprep && flagRep == 0) retValRep = check_errhandler_conditions((comm->oworldComm),&reqRep,cmpToRepMap[dest]+nC);
						if(retValRep != 0) break;
						MPI_Request tmpreq;
						if(last_peertopeer != NULL) tmpreq = *(last_peertopeer->req);
						if(!skipcmp && flag == 0) EMPI_Test(&req,&flag,&status);
						if(cmpToRepMap[dest] != -1 && cmpToRepMap[rank] == -1 && !skiprep && flagRep == 0) EMPI_Test(&reqRep,&flagRep,&status);
						if(last_peertopeer != NULL) *(last_peertopeer->req) = tmpreq;
					} while(flag == 0 || flagRep == 0);
				}
			} 
		} while(retVal != 0 || retValRep != 0);
		req = EMPI_REQUEST_NULL;
		reqRep = EMPI_REQUEST_NULL;
		sendid++;
		if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
		else first_peertopeer = curargs;
		curargs->prev = NULL;
		curargs->next = last_peertopeer;
		last_peertopeer = curargs;
//ConvertToCMPSENDSTORED:
//		if(nR > 0)
//		{
//			do
//			{
//				retValRep = 0;
//				retValRep = check_errhandler_conditions_collective((comm->oworldComm),&reqRep);
//			} while(retValRep != 0);
//		}
	}
	//memcpy(buf+dis,backup,extracount*size);
	//*(int *)(buf+dis) = backup;
	//free(backup);
	return MPI_SUCCESS;
}

/* MPI_Isend */

int MPI_Isend (void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *req)
{
	int retVal,retValRep;
	MPI_Request ftreq;
	ftreq = (MPI_Request)malloc(sizeof(struct mpi_ft_request));
	*req = ftreq;
	(ftreq)->reqcmp = (EMPI_Request *)malloc(sizeof(EMPI_Request));
	(ftreq)->reqrep = (EMPI_Request *)malloc(sizeof(EMPI_Request));
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	EMPI_Status status;
	int rank;
	int extracount;
	int dis;
	//void *backup;
	int size;
	struct peertopeer_data *curargs;
	curargs = (struct peertopeer_data *)malloc(sizeof(struct collective_data));
	EMPI_Type_size(datatype->edatatype,&size);
	dis = count*size;
	if(size >= sizeof(int)) extracount = 1;
	else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
	else extracount = (((int)sizeof(int))/size) + 1;
	//backup = malloc(extracount*size);
	curargs->buf = malloc((count+extracount)*size);
	memcpy(curargs->buf,buf,count*size);
	//memcpy(backup,buf+dis,extracount*size);
	//backup = *(int *)(buf+dis);
	memcpy((curargs->buf) + dis,&sendid,sizeof(int));
	//*((int *)((curargs->buf)+dis)) = sendid;
	//*((int *)(buf+dis)) = sendid;
	//(ftreq)->backup = backup;
	//(ftreq)->backuploc = (void *)(buf+dis);
	(ftreq)->bufloc = buf;
	(ftreq)->status.count = count;
	(ftreq)->status.MPI_SOURCE = dest;
	(ftreq)->status.MPI_TAG = tag;
	(ftreq)->status.MPI_ERROR = 0;
	bool skiprep, skipcmp;
	struct skiplist *skip,*oldskip;
	if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
	{
		do
		{
			retValRep = 0;
			skiprep = false;
			skip = skipreplist;
			oldskip = NULL;
			while(skip != NULL)
			{
				if(skip->id == sendid)
				{
					skiprep = true;
					if(oldskip != NULL) oldskip->next = skip->next;
					else skipreplist = skip->next;
					free(skip);
					break;
				}
				oldskip = skip;
				skip = skip->next;
			}
			if(!skiprep)
			{
				if(cmpToRepMap[dest] == -1) break;
				retValRep = check_errhandler_conditions((comm->oworldComm),((ftreq)->reqrep),cmpToRepMap[dest]+nC);
				if(retValRep == 0)
				{
					retValRep = EMPI_Isend (curargs->buf, count +extracount, datatype->edatatype, cmpToRepMap[dest], tag, (comm->EMPI_COMM_REP), ((ftreq)->reqrep));
				}
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					if(*((ftreq)->reqrep) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqrep));
					goto ConvertToCMPISEND;
				}
			}
		} while(retValRep != 0);
		(ftreq)->complete = false;
		(ftreq)->comm = comm;
		(ftreq)->type = MPI_FT_REQUEST_SEND;
		curargs->sendtype = true;
		//curargs->buf = malloc(count*size);
		//memcpy(curargs->buf,buf,count*size);
		curargs->count = count;
		curargs->dt = datatype;
		curargs->target = dest;
		curargs->tag = tag;
		curargs->comm = comm;
		curargs->completecmp = true;
		curargs->completerep = skiprep || (cmpToRepMap[dest] == -1);
		curargs->req = req;
		curargs->id = sendid;
		sendid++;
		if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
		else first_peertopeer = curargs;
		curargs->prev = NULL;
		curargs->next = last_peertopeer;
		last_peertopeer = curargs;
		(ftreq)->storeloc = curargs;
		do
		{
			retVal = 0;
			retVal = check_errhandler_conditions_collective((comm->oworldComm),((ftreq)->reqcmp));
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				if(*((ftreq)->reqcmp) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqcmp));
				goto ConvertToCMPISENDSTORED;
			}
		} while(retVal != 0);
	}
	else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
	{
ConvertToCMPISEND:
		do
		{
			retVal = 0;
			retValRep = 0;
			skiprep = false;
			skipcmp = false;
			skip = skipreplist;
			oldskip = NULL;
			while(skip != NULL)
			{
				if(skip->id == sendid)
				{
					skiprep = true;
					if(oldskip != NULL) oldskip->next = skip->next;
					else skipreplist = skip->next;
					free(skip);
					break;
				}
				oldskip = skip;
				skip = skip->next;
			}
			skip = skipcmplist;
			oldskip = NULL;
			while(skip != NULL)
			{
				if(skip->id == sendid)
				{
					skipcmp = true;
					if(oldskip != NULL) oldskip->next = skip->next;
					else skipcmplist = skip->next;
					free(skip);
					break;
				}
				oldskip = skip;
				skip = skip->next;
			}
			if(!skipcmp || !skiprep)
			{
				EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
				if(!skipcmp) retVal = check_errhandler_conditions((comm->oworldComm),((ftreq)->reqcmp),dest);
				if(cmpToRepMap[dest] != -1 && cmpToRepMap[rank] == -1 && !skiprep) retValRep = check_errhandler_conditions((comm->oworldComm),((ftreq)->reqrep),cmpToRepMap[dest]+nC);
				if(retVal == 0 && retValRep == 0)
				{
					if(!skipcmp)retVal = EMPI_Isend (curargs->buf, count+extracount, datatype->edatatype, dest, tag, (comm->EMPI_COMM_CMP), ((ftreq)->reqcmp));
					if(cmpToRepMap[dest] != -1 && cmpToRepMap[rank] == -1 && !skiprep) retValRep = EMPI_Isend (curargs->buf, count+extracount, datatype->edatatype, cmpToRepMap[dest], tag, (comm->EMPI_CMP_REP_INTERCOMM), ((ftreq)->reqrep));
				}
			}
		} while(retVal != 0 || retValRep != 0);
		(ftreq)->complete = false;
		(ftreq)->comm = comm;
		(ftreq)->type = MPI_FT_REQUEST_SEND;
		curargs->sendtype = true;
		//curargs->buf = malloc(count*size);
		//memcpy(curargs->buf,buf,count*size);
		curargs->count = count;
		curargs->dt = datatype;
		curargs->target = dest;
		curargs->tag = tag;
		curargs->comm = comm;
		curargs->completecmp = skipcmp;
		curargs->completerep = (cmpToRepMap[dest] == -1) || (cmpToRepMap[rank] != -1) || skiprep;
		curargs->req = req;
		curargs->id = sendid;
		sendid++;
		if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
		else first_peertopeer = curargs;
		curargs->prev = NULL;
		curargs->next = last_peertopeer;
		last_peertopeer = curargs;
		(ftreq)->storeloc = curargs;
ConvertToCMPISENDSTORED:
		if(nR > 0)
		{
			do
			{
				retValRep = 0;
				retValRep = check_errhandler_conditions_collective((comm->oworldComm),((ftreq)->reqrep));
			} while(retValRep != 0);
		}
	}
	*req = ftreq;
	return MPI_SUCCESS;
}

void GenerateRecvLog(char *logstr, int id, int src, int tag, EMPI_Comm comm)
{
  int actualsrc;
  if((comm == EMPI_COMM_CMP) || ((comm == EMPI_CMP_REP_INTERCOMM) && (EMPI_COMM_CMP == EMPI_COMM_NULL))) actualsrc = src;
  else if((comm == EMPI_COMM_REP) || ((comm == EMPI_CMP_REP_INTERCOMM) && (EMPI_COMM_REP == EMPI_COMM_NULL))) actualsrc = src + nC;
  sprintf (logstr, "RECV %d %d %d\n", id, actualsrc, tag);
}

/* MPI_Recv */

int MPI_Recv (void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *stat)
{
  int retVal,retValRep;
	EMPI_Request req = EMPI_REQUEST_NULL;
	EMPI_Request reqRep = EMPI_REQUEST_NULL;
	EMPI_Datatype edatatype;
	EMPI_Status status;
	int rank;
	int extracount;
	int dis;
	//void *backup;
	int size;
	struct peertopeer_data *curargs;
	curargs = (struct peertopeer_data *)malloc(sizeof(struct collective_data));
	EMPI_Type_size(datatype->edatatype,&size);
	dis = count*size;
	if(size >= sizeof(int)) extracount = 1;
	else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
	else extracount = (((int)sizeof(int))/size) + 1;
	//backup = malloc(extracount*size);
	//memcpy(backup,buf+dis,extracount*size);
	//backup = *(int *)(buf+dis);
	curargs->buf = malloc((count+extracount)*size);
	if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
	{
		do
		{
			retValRep = 0;
			if(cmpToRepMap[source] == -1) retValRep = check_errhandler_conditions((comm->oworldComm),&reqRep,source);
			else retValRep = check_errhandler_conditions((comm->oworldComm),&reqRep,cmpToRepMap[source]+nC);
			if(retValRep == 0)
			{
				if(cmpToRepMap[source] == -1) retValRep = EMPI_Irecv (curargs->buf, count +extracount, datatype->edatatype, source, tag, (comm->EMPI_CMP_REP_INTERCOMM), &reqRep);
				else retValRep = EMPI_Irecv (curargs->buf, count +extracount, datatype->edatatype, cmpToRepMap[source], tag, (comm->EMPI_COMM_REP), &reqRep);
				int flag = 0;
				do
				{
					if(cmpToRepMap[source] == -1) retValRep = check_errhandler_conditions((comm->oworldComm),&reqRep,source);
					else retValRep = check_errhandler_conditions((comm->oworldComm),&reqRep,cmpToRepMap[source]+nC);
					if(retValRep != 0) break;
					EMPI_Test(&reqRep,&flag,&status);
				} while(flag == 0);
			}
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				if(reqRep != EMPI_REQUEST_NULL) EMPI_Request_free(&reqRep);
				goto ConvertToCMPRECV;
			}
		} while(retValRep != 0);
		req = EMPI_REQUEST_NULL;
		reqRep = EMPI_REQUEST_NULL;
		memcpy(buf,curargs->buf,count*size);
		curargs->sendtype = false;
		//curargs->buf = malloc(count*size);
		//memcpy(curargs->buf,buf,count*size);
		curargs->count = count;
		curargs->dt = datatype;
		curargs->target = source;
		curargs->tag = tag;
		curargs->comm = comm;
		curargs->completecmp = true;
		curargs->completerep = true;
		curargs->req = (MPI_Request *)malloc(sizeof(MPI_Request));
		*(curargs->req) = MPI_REQUEST_NULL;
		memcpy(&(curargs->id),(curargs->buf) + dis,sizeof(int));
		//curargs->id = *(int *)((curargs->buf)+dis);
		free(curargs->buf);
		if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
		else first_peertopeer = curargs;
		curargs->prev = NULL;
		curargs->next = last_peertopeer;
		last_peertopeer = curargs;
		do
		{
			retVal = 0;
			retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				if(reqRep != EMPI_REQUEST_NULL) EMPI_Request_free(&reqRep);
				goto ConvertToCMPRECVSTORED;
			}
		} while(retVal != 0);
	}
	else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
	{
ConvertToCMPRECV:
		do
		{
			retVal = 0;
			retVal = check_errhandler_conditions((comm->oworldComm),&req,source);
			if(retVal == 0)
			{
				retVal = EMPI_Irecv (curargs->buf, count+extracount, datatype->edatatype, source, tag, (comm->EMPI_COMM_CMP), &req);
				int flag = 0;
				do
				{
					retVal = check_errhandler_conditions((comm->oworldComm),&req,source);
					if(retVal != 0) break;
					EMPI_Test(&req,&flag,&status);
				} while(flag == 0);
			}
		} while(retVal != 0);
		req = EMPI_REQUEST_NULL;
		reqRep = EMPI_REQUEST_NULL;
		memcpy(buf,curargs->buf,count*size);
		curargs->sendtype = false;
		//curargs->buf = malloc(count*size);
		//memcpy(curargs->buf,buf,count*size);
		curargs->count = count;
		curargs->dt = datatype;
		curargs->target = source;
		curargs->tag = tag;
		curargs->comm = comm;
		curargs->completecmp = true;
		curargs->completerep = true;
		curargs->req = (MPI_Request *)malloc(sizeof(MPI_Request));
		*(curargs->req) = MPI_REQUEST_NULL;
		memcpy(&(curargs->id),(curargs->buf) + dis,sizeof(int));
		curargs->id = *(int *)((curargs->buf)+dis);
		free(curargs->buf);
		if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
		else first_peertopeer = curargs;
		curargs->prev = NULL;
		curargs->next = last_peertopeer;
		last_peertopeer = curargs;
ConvertToCMPRECVSTORED:
		if(nR > 0)
		{
			do
			{
				retValRep = 0;
				retValRep = check_errhandler_conditions_collective((comm->oworldComm),&reqRep);
			} while(retValRep != 0);
		}
	}
	//memcpy(buf+dis,backup,extracount*size);
	//*(int *)(buf+dis) = backup;
	//free(backup);
	return MPI_SUCCESS;
}

/* MPI_Irecv */

int MPI_Irecv (void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *req)
{
	int retVal,retValRep;
	MPI_Request ftreq;
	ftreq = (MPI_Request)malloc(sizeof(struct mpi_ft_request));
	*req = ftreq;
	(ftreq)->reqcmp = (EMPI_Request *)malloc(sizeof(EMPI_Request));
	(ftreq)->reqrep = (EMPI_Request *)malloc(sizeof(EMPI_Request));
	*((ftreq)->reqcmp) = EMPI_REQUEST_NULL;
	*((ftreq)->reqrep) = EMPI_REQUEST_NULL;
	EMPI_Datatype edatatype;
	EMPI_Status status;
	int rank;
	int extracount;
	int dis;
	//void *backup;
	int size;
	struct peertopeer_data *curargs;
	curargs = (struct peertopeer_data *)malloc(sizeof(struct collective_data));
	EMPI_Type_size(datatype->edatatype,&size);
	dis = count*size;
	if(size >= sizeof(int)) extracount = 1;
	else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
	else extracount = (((int)sizeof(int))/size) + 1;
	//backup = malloc(extracount*size);
	//memcpy(backup,buf+dis,extracount*size);
	//backup = *(int *)(buf+dis);
	//(ftreq)->backup = backup;
	//(ftreq)->backuploc = (void *)(buf+dis);
	curargs->buf = malloc((count+extracount)*size);
	(ftreq)->bufloc = buf;
	(ftreq)->status.count = count;
	(ftreq)->status.MPI_SOURCE = source;
	(ftreq)->status.MPI_TAG = tag;
	(ftreq)->status.MPI_ERROR = 0;
	if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
	{
		do
		{
			retValRep = 0;
			EMPI_Comm_rank(comm->EMPI_COMM_REP,&rank);
			int wrank;
			EMPI_Comm_rank(comm->eworldComm,&wrank);
			//printf("REP calling erecv at %p %p myrank %d wrank %d source has no rep %d\n",curargs->buf,buf,rank,wrank,cmpToRepMap[source] == -1);
			if(cmpToRepMap[source] == -1) retValRep = check_errhandler_conditions((comm->oworldComm),((ftreq)->reqcmp),source);
			else retValRep = check_errhandler_conditions((comm->oworldComm),((ftreq)->reqrep),cmpToRepMap[source]+nC);
			if(retValRep == 0)
			{
				if(cmpToRepMap[source] == -1) retValRep = EMPI_Irecv (curargs->buf, count +extracount, datatype->edatatype, source, tag, (comm->EMPI_CMP_REP_INTERCOMM), ((ftreq)->reqcmp));
				else retValRep = EMPI_Irecv (curargs->buf, count +extracount, datatype->edatatype, cmpToRepMap[source], tag, (comm->EMPI_COMM_REP), ((ftreq)->reqrep));
			}
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				printf("Conveting to cmp at irecv %d %d\n",*((ftreq)->reqcmp) != EMPI_REQUEST_NULL,*((ftreq)->reqrep) != EMPI_REQUEST_NULL);
				if(*((ftreq)->reqcmp) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqcmp));
				if(*((ftreq)->reqrep) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqrep));
				goto ConvertToCMPIRECV;
			}
			if(retValRep != 0) printf("Retvalrep %d looping bufaddr %p rank %d\n",retValRep,curargs->buf,rank);
		} while(retValRep != 0);
		(ftreq)->complete = false;
		(ftreq)->comm = comm;
		(ftreq)->type = MPI_FT_REQUEST_RECV;
		curargs->sendtype = false;
		//curargs->buf = buf;
		//memcpy(curargs->buf,buf,count*size);
		curargs->count = count;
		curargs->dt = datatype;
		curargs->target = source;
		curargs->tag = tag;
		curargs->comm = comm;
		curargs->completecmp = (cmpToRepMap[source] != -1);
		//curargs->completecmp = false;
		curargs->completerep = !(curargs->completecmp);
		curargs->req = req;
		if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
		else first_peertopeer = curargs;
		curargs->prev = NULL;
		curargs->next = last_peertopeer;
		last_peertopeer = curargs;
		(ftreq)->storeloc = curargs;
//		do
//		{
//			retVal = 0;
//			retVal = check_errhandler_conditions_collective((comm->oworldComm),((ftreq)->reqcmp));
//			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
//			{
//				if(*((ftreq)->reqcmp) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqcmp));
//				goto ConvertToCMPIRECVSTORED;
//			}
//		} while(retVal != 0);
	}
	else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
	{
ConvertToCMPIRECV:
		do
		{
			retVal = 0;
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			//printf("CMP calling erecv at %p %p myrank %d\n",curargs->buf,buf,rank);
			retVal = check_errhandler_conditions((comm->oworldComm),((ftreq)->reqcmp),source);
			if(retVal == 0)
			{
				retVal = EMPI_Irecv (curargs->buf, count+extracount, datatype->edatatype, source, tag, (comm->EMPI_COMM_CMP), ((ftreq)->reqcmp));
			}
			if(retVal != 0) printf("Retval %d looping bufaddr %p rank %d source %d tag %d\n",retVal,curargs->buf,rank,source,tag);
		} while(retVal != 0);
		(ftreq)->complete = false;
		(ftreq)->comm = comm;
		(ftreq)->type = MPI_FT_REQUEST_RECV;
		curargs->sendtype = false;
		//curargs->buf = buf;
		//memcpy(curargs->buf,buf,count*size);
		curargs->count = count;
		curargs->dt = datatype;
		curargs->target = source;
		curargs->tag = tag;
		curargs->comm = comm;
		curargs->completecmp = false;
		curargs->completerep = true;
		curargs->req = req;
		if(last_peertopeer != NULL) last_peertopeer->prev = curargs;
		else first_peertopeer = curargs;
		curargs->prev = NULL;
		curargs->next = last_peertopeer;
		last_peertopeer = curargs;
		(ftreq)->storeloc = curargs;
//ConvertToCMPIRECVSTORED:
//		if(nR > 0)
//		{
//			do
//			{
//				retValRep = 0;
//				retValRep = check_errhandler_conditions_collective((comm->oworldComm),((ftreq)->reqrep));
//			} while(retValRep != 0);
//		}
	}
	*req = ftreq;
	return MPI_SUCCESS;
}

/* MPI_Request_free */

int MPI_Request_free(MPI_Request *req)
{
	MPI_Request ftreq = *req;
	if(ftreq == MPI_REQUEST_NULL) return 0;
	else
	{
		if(*((ftreq)->reqcmp) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqcmp));
		if(*((ftreq)->reqrep) != EMPI_REQUEST_NULL) EMPI_Request_free(((ftreq)->reqrep));
		free(ftreq);
		ftreq = MPI_REQUEST_NULL;
	}
	return 0;
}


/* MPI_Test */

int MPI_Test(MPI_Request *req, int *flag, MPI_Status *status)
{
	MPI_Request ftreq;
	ftreq = *req;
	int tmpflagcmp;
	int tmpflagrep;
	int extracount;
	int count;
	int size;
	EMPI_Status tmpstatuscmp;
	EMPI_Status tmpstatusrep;
	int retVal = 0;
	int retValRep = 0;
	*flag = 0;
	bool cmprecv = true;
	bool reprecv = true;
	if(ftreq == MPI_REQUEST_NULL || (ftreq)->complete)
	{
		*flag = 1;
		if(status != MPI_STATUS_IGNORE) *status = (ftreq)->status;
		return MPI_Request_free(req);
	}
	else
	{
		if((ftreq)->comm->EMPI_COMM_REP != EMPI_COMM_NULL)
		{
			if((ftreq)->type == MPI_FT_REQUEST_SEND)
			{
				int retValRep = 0;
				do
				{
					retValRep = 0;
					retValRep = check_errhandler_conditions(((ftreq)->comm->oworldComm),((ftreq)->reqrep),cmpToRepMap[((struct peertopeer_data*)((ftreq)->storeloc))->target]+nC);
					if((ftreq)->comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
					{
						goto ConvertToCMPTESTSEND;
					}
				} while(retValRep != 0);
				if(!(((struct peertopeer_data*)((ftreq)->storeloc))->completerep)) EMPI_Test(((ftreq)->reqrep),&tmpflagrep,&tmpstatusrep);
				else tmpflagrep = 1;
				if(tmpflagrep)
				{
					(ftreq)->complete = true;
					//EMPI_Type_size(((struct peertopeer_data*)((ftreq)->storeloc))->dt->edatatype,&size);
					//if(size >= sizeof(int)) extracount = 1;
					//else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
					//else extracount = (((int)sizeof(int))/size) + 1;
					//memcpy((ftreq)->backuploc,(ftreq)->backup,extracount*size);
					//*((ftreq)->backuploc) = (ftreq)->backup;
					//free((ftreq)->backup);
					((struct peertopeer_data*)((ftreq)->storeloc))->completerep = true;
					((ftreq)->status).status = tmpstatusrep;
					((ftreq)->status).MPI_TAG = tmpstatusrep.EMPI_TAG;
					((ftreq)->status).MPI_SOURCE = tmpstatusrep.EMPI_SOURCE;
					((ftreq)->status).MPI_ERROR = tmpstatusrep.EMPI_ERROR;
					if(status != MPI_STATUS_IGNORE) *status = (ftreq)->status;
					*flag = 1;
					return MPI_Request_free(req);
				}
//				do
//				{
//					retVal = 0;
//					retVal = check_errhandler_conditions_collective(((ftreq)->comm->oworldComm),((ftreq)->reqcmp));
//					if((ftreq)->comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
//					{
//						goto ConvertToCMPTESTSENDSTORED;
//					}
//				} while(retVal != 0);
			}
			else if((ftreq)->type == MPI_FT_REQUEST_RECV)
			{
				do
				{
					retVal = 0;
					retValRep = 0;
					if(!(((struct peertopeer_data*)((ftreq)->storeloc))->completecmp)) retVal = check_errhandler_conditions(((ftreq)->comm->oworldComm),((ftreq)->reqcmp),((struct peertopeer_data*)((ftreq)->storeloc))->target);
					if((ftreq)->comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
					{
						printf("Converting to cmp at test recv completecmp false current !completerep %d !completecmp %d\n",!(((struct peertopeer_data*)((ftreq)->storeloc))->completerep),!(((struct peertopeer_data*)((ftreq)->storeloc))->completecmp));
						goto ConvertToCMPTESTRECV;
					}
					if(!(((struct peertopeer_data*)((ftreq)->storeloc))->completerep)) retValRep = check_errhandler_conditions(((ftreq)->comm->oworldComm),((ftreq)->reqrep),cmpToRepMap[((struct peertopeer_data*)((ftreq)->storeloc))->target]+nC);
					if((ftreq)->comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
					{
						printf("Converting to cmp at test recv completerep false current !completerep %d !completecmp %d req %p\n",!(((struct peertopeer_data*)((ftreq)->storeloc))->completerep),!(((struct peertopeer_data*)((ftreq)->storeloc))->completecmp),ftreq);
						goto ConvertToCMPTESTRECV;
					}
				} while(retVal != 0 || retValRep != 0);
				if(!(((struct peertopeer_data*)((ftreq)->storeloc))->completecmp))
				{
					MPI_Request tmpreq = ftreq;
					EMPI_Test(((ftreq)->reqcmp),&tmpflagcmp,&tmpstatuscmp);
					ftreq = tmpreq;
				}
				else
				{
					cmprecv = false;
					tmpflagcmp = 1;
				}
				if(!(((struct peertopeer_data*)((ftreq)->storeloc))->completerep))
				{
					MPI_Request tmpreq = ftreq;
					EMPI_Test(((ftreq)->reqrep),&tmpflagrep,&tmpstatusrep);
					ftreq = tmpreq;
				}
				else 
				{
					reprecv = false;
					tmpflagrep = 1;
				}
				if(tmpflagcmp) ((struct peertopeer_data*)((ftreq)->storeloc))->completecmp = true;
				if(tmpflagrep) ((struct peertopeer_data*)((ftreq)->storeloc))->completerep = true;
				if(tmpflagcmp && tmpflagrep)
				{
					(ftreq)->complete = true;
					count = (((struct peertopeer_data *)((ftreq)->storeloc))->count);
					EMPI_Type_size(((struct peertopeer_data*)((ftreq)->storeloc))->dt->edatatype,&size);
					memcpy((ftreq)->bufloc,(((struct peertopeer_data *)((ftreq)->storeloc))->buf),count*size);
					memcpy(&(((struct peertopeer_data *)((ftreq)->storeloc))->id),((((struct peertopeer_data *)((ftreq)->storeloc))->buf) + (count * size)),sizeof(int));
					int rank;
					EMPI_Comm_rank(((ftreq)->comm->eworldComm),&rank);
					//printf("REP recv test complete rank %d target %d id %d\n",rank,((struct peertopeer_data*)((ftreq)->storeloc))->target,(((struct peertopeer_data *)((ftreq)->storeloc))->id));
					//((struct peertopeer_data *)((ftreq)->storeloc))->id = *((int *)((((struct peertopeer_data *)((ftreq)->storeloc))->buf) + (count * size)));
					free((((struct peertopeer_data *)((ftreq)->storeloc))->buf));
					//EMPI_Type_size(((struct peertopeer_data*)((ftreq)->storeloc))->dt->edatatype,&size);
					if(size >= sizeof(int)) extracount = 1;
					else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
					else extracount = (((int)sizeof(int))/size) + 1;
					//memcpy((ftreq)->backuploc,(ftreq)->backup,extracount*size);
					//*((ftreq)->backuploc) = (ftreq)->backup;
					//free((ftreq)->backup);
					if(reprecv)
					{
						((ftreq)->status).status = tmpstatusrep;
						EMPI_Get_count(&tmpstatusrep,((struct peertopeer_data*)((ftreq)->storeloc))->dt->edatatype,&(((ftreq)->status).count));
						((ftreq)->status).count -= extracount;
						((ftreq)->status).MPI_TAG = tmpstatusrep.EMPI_TAG;
						((ftreq)->status).MPI_SOURCE = tmpstatusrep.EMPI_SOURCE;
						((ftreq)->status).MPI_ERROR = tmpstatusrep.EMPI_ERROR;
					}
					else if(cmprecv)
					{
						((ftreq)->status).status = tmpstatuscmp;
						EMPI_Get_count(&tmpstatuscmp,((struct peertopeer_data*)((ftreq)->storeloc))->dt->edatatype,&(((ftreq)->status).count));
						((ftreq)->status).count -= extracount;
						((ftreq)->status).MPI_TAG = tmpstatuscmp.EMPI_TAG;
						((ftreq)->status).MPI_SOURCE = tmpstatuscmp.EMPI_SOURCE;
						((ftreq)->status).MPI_ERROR = tmpstatuscmp.EMPI_ERROR;
					}
					if(status != MPI_STATUS_IGNORE) *status = (ftreq)->status;
					*flag = 1;
					return MPI_Request_free(req);
				}
				//do
				//{
				//	retVal = 0;
				//	retVal = check_errhandler_conditions_collective(((ftreq)->comm->oworldComm),((ftreq)->reqcmp));
				//	if((ftreq)->comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				//	{
				//		goto ConvertToCMPTESTRECVSTORED;
				//	}
				//} while(retVal != 0);
			}
		}
		else if((ftreq)->comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
		{
			if((ftreq)->type == MPI_FT_REQUEST_SEND)
			{
ConvertToCMPTESTSEND:
				do
				{
					retVal = 0;
					retValRep = 0;
					if(!(((struct peertopeer_data*)((ftreq)->storeloc))->completecmp)) retVal = check_errhandler_conditions(((ftreq)->comm->oworldComm),((ftreq)->reqcmp),((struct peertopeer_data*)((ftreq)->storeloc))->target);
					if(!(((struct peertopeer_data*)((ftreq)->storeloc))->completerep)) retValRep = check_errhandler_conditions(((ftreq)->comm->oworldComm),((ftreq)->reqrep),cmpToRepMap[((struct peertopeer_data*)((ftreq)->storeloc))->target]+nC);
				} while(retVal != 0 || retValRep != 0);
				if(!(((struct peertopeer_data*)((ftreq)->storeloc))->completecmp)) EMPI_Test(((ftreq)->reqcmp),&tmpflagcmp,&tmpstatuscmp);
				else tmpflagcmp = 1;
				if(!(((struct peertopeer_data*)((ftreq)->storeloc))->completerep)) EMPI_Test(((ftreq)->reqrep),&tmpflagrep,&tmpstatusrep);
				else tmpflagrep = 1;
				if(tmpflagcmp) ((struct peertopeer_data*)((ftreq)->storeloc))->completecmp = true;
				if(tmpflagrep) ((struct peertopeer_data*)((ftreq)->storeloc))->completerep = true;
				if(tmpflagcmp && tmpflagrep)
				{
					(ftreq)->complete = true;
					//EMPI_Type_size(((struct peertopeer_data*)((ftreq)->storeloc))->dt->edatatype,&size);
					//if(size >= sizeof(int)) extracount = 1;
					//else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
					//else extracount = (((int)sizeof(int))/size) + 1;
					//memcpy((ftreq)->backuploc,(ftreq)->backup,extracount*size);
					//*((ftreq)->backuploc) = (ftreq)->backup;
					//free((ftreq)->backup);
					((ftreq)->status).status = tmpstatuscmp;
					((ftreq)->status).MPI_TAG = tmpstatuscmp.EMPI_TAG;
					((ftreq)->status).MPI_SOURCE = tmpstatuscmp.EMPI_SOURCE;
					((ftreq)->status).MPI_ERROR = tmpstatuscmp.EMPI_ERROR;
					if(status != MPI_STATUS_IGNORE) *status = (ftreq)->status;
					*flag = 1;
					return MPI_Request_free(req);
				}
//ConvertToCMPTESTSENDSTORED:
//				if(nR > 0)
//				{
//					do
//					{
//						retValRep = 0;
//						retValRep = check_errhandler_conditions_collective(((ftreq)->comm->oworldComm),((ftreq)->reqrep));
//					} while(retValRep != 0);
//				}
			}
			else if((ftreq)->type == MPI_FT_REQUEST_RECV)
			{
ConvertToCMPTESTRECV:
				do
				{
					retVal = 0;
					retVal = check_errhandler_conditions(((ftreq)->comm->oworldComm),((ftreq)->reqcmp),((struct peertopeer_data*)((ftreq)->storeloc))->target);
				} while(retVal != 0);
				if(!(((struct peertopeer_data*)((ftreq)->storeloc))->completecmp))
				{
					MPI_Request tmpreq = ftreq;
					EMPI_Test(((ftreq)->reqcmp),&tmpflagcmp,&tmpstatuscmp);
					ftreq = tmpreq;
				}
				else tmpflagcmp = 1;
				if(tmpflagcmp)
				{
					(ftreq)->complete = true;
					count = (((struct peertopeer_data *)((ftreq)->storeloc))->count);
					EMPI_Type_size(((struct peertopeer_data*)((ftreq)->storeloc))->dt->edatatype,&size);
					memcpy((ftreq)->bufloc,(((struct peertopeer_data *)((ftreq)->storeloc))->buf),count*size);
					memcpy(&(((struct peertopeer_data *)((ftreq)->storeloc))->id),((((struct peertopeer_data *)((ftreq)->storeloc))->buf) + (count * size)),sizeof(int));
					int rank;
					EMPI_Comm_rank(((ftreq)->comm->eworldComm),&rank);
					//printf("CMP recv test complete rank %d target %d id %d\n",rank,((struct peertopeer_data*)((ftreq)->storeloc))->target,(((struct peertopeer_data *)((ftreq)->storeloc))->id));
					//((struct peertopeer_data *)((ftreq)->storeloc))->id = *((int *)((((struct peertopeer_data *)((ftreq)->storeloc))->buf) + (count * size)));
					free((((struct peertopeer_data *)((ftreq)->storeloc))->buf));
					//((struct peertopeer_data *)((ftreq)->storeloc))->id = *(int *)((ftreq)->backuploc);
					EMPI_Type_size(((struct peertopeer_data*)((ftreq)->storeloc))->dt->edatatype,&size);
					if(size >= sizeof(int)) extracount = 1;
					else if(((int)sizeof(int)) % size == 0) extracount = (((int)sizeof(int))/size);
					else extracount = (((int)sizeof(int))/size) + 1;
					//memcpy((ftreq)->backuploc,(ftreq)->backup,extracount*size);
					//*((ftreq)->backuploc) = (ftreq)->backup;
					//free((ftreq)->backup);
					((struct peertopeer_data*)((ftreq)->storeloc))->completecmp = true;
					((ftreq)->status).status = tmpstatuscmp;
					EMPI_Get_count(&tmpstatuscmp,((struct peertopeer_data*)((ftreq)->storeloc))->dt->edatatype,&(((ftreq)->status).count));
					((ftreq)->status).count -= extracount;
					((ftreq)->status).MPI_TAG = tmpstatuscmp.EMPI_TAG;
					((ftreq)->status).MPI_SOURCE = tmpstatuscmp.EMPI_SOURCE;
					((ftreq)->status).MPI_ERROR = tmpstatuscmp.EMPI_ERROR;
					if(status != MPI_STATUS_IGNORE) *status = (ftreq)->status;
					*flag = 1;
					return MPI_Request_free(req);
				}
//ConvertToCMPTESTRECVSTORED:
//				if(nR > 0)
//				{
//					do
//					{
//						retValRep = 0;
//						retValRep = check_errhandler_conditions_collective(((ftreq)->comm->oworldComm),((ftreq)->reqrep));
//					} while(retValRep != 0);
//				}
			}
		}
	}
	return 0;
}

/* MPI_Wait */

int MPI_Wait (MPI_Request *request, MPI_Status *status)
{
	int flag;
	int ret;
	MPI_Request ftreq;
	do
	{
		ret = MPI_Test(request,&flag,status);
	} while(flag == 0);
	return 0;
}

/* MPI_Reduce */

int MPI_Reduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
  int retVal;
  EMPI_Request req = EMPI_REQUEST_NULL;
  EMPI_Request sreq = EMPI_REQUEST_NULL;
  EMPI_Request rreq = EMPI_REQUEST_NULL;
  EMPI_Status status;
  int oldsize = 0;
  int targetrep;
  int rank;
	int size;
	struct collective_data *curargs;
  if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
  {
    /*tmpbuf = malloc(count * datatype->size);
    EMPI_Comm_rank((comm->EMPI_COMM_REP), &rank);
    int start = 0;
    int end = nC;
    if(rank != 0) start = repToCmpMap[rank-1]+1;
    if(rank != nR-1) end = repToCmpMap[rank+1];
    for(int i = start; i < end; i++)
    {
      int targetrep = closest_replica(i);
      if(rank == targetrep && targetrep != -1)
      {
        if(repToCmpMap[rank] != i && !haveData[i])
        {
          do
          {
            retVal = 0;
            retVal = check_errhandler_conditions((comm->oworldComm),&req,i);

            if(retVal == 0)
            {
              retVal = EMPI_Irecv (tmpbuf, count, edatatype, i, MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &req);
              int flag = 0;
              do
              {
                retVal = check_errhandler_conditions((comm->oworldComm),&req,i);
                if(retVal != 0) break;
                EMPI_Test(&req,&flag,&status);
              } while(flag == 0);
            }
            if((comm->EMPI_COMM_CMP) != (comm->EMPI_COMM_NULL))
            {
              if(req != EMPI_REQUEST_NULL) EMPI_Request_free(&req);
              goto ConvertToCMPALLREDUCE;
            }
          } while(retVal != 0);
		  haveData[i] = true;
		  
        }
      }
    }
   
    req = EMPI_REQUEST_NULL;
    do
    { 
      retVal = 0;
      retVal = check_errhandler_conditions_collective(oworldComm,&req);
    
      if(retVal == 0)
      {
        retVal = EMPI_Ibcast (buffer, count, edatatype, reproot, EMPI_COMM_REP, &req);
        int flag = 0;
        do
        {
          retVal = check_errhandler_conditions_collective(oworldComm,&req);
          if(retVal != 0) break;
          EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
      if(EMPI_COMM_CMP != EMPI_COMM_NULL)
      {
        if(req != EMPI_REQUEST_NULL) EMPI_Request_free(&req);
        goto ConvertToCMPBCAST;
      }
      if(retVal != 0)
      {
        bool haveData = (rank == reproot);
        reproot = closest_replica(root);
        EMPI_Comm_rank(EMPI_COMM_REP, &rank);
        if(reproot != -1 && rank == reproot)
        {
          if(!haveData)
          {
            goto RootToReprootRECV;
          }
        }
      }
    } while(retVal != 0);
    if(last_collective->id != -1)
    {
      EMPI_Type_size(last_collective->dt,&oldsize);
    }
    int size;
    EMPI_Type_size(edatatype,&size);
    //if(last_collective->id != -1) printf("Size comp %d %d %d %d\n",count,size,last_collective->count,oldsize);
    if(count*size != oldsize*last_collective->count)
    {
      if(last_collective->id != -1) free(last_collective->buf);
      last_collective->buf = malloc(count*size);
    }
    memcpy(last_collective->buf,buffer,count*size);
    last_collective->count = count;
    last_collective->dt = datatype;
    last_collective->root = root;
    last_collective->reproot = reproot;
    last_collective->ecomm = &EMPI_COMM_REP;
    last_collective->ocomm = &oworldComm;
    last_collective->id = bcastid;
    bcastid++;
    req = EMPI_REQUEST_NULL;*/
    //char ssync = '0';
    //char rsync;
    do
    {
      retVal = 0;
      EMPI_Comm_rank((comm->EMPI_COMM_REP),&rank);
			if(rank != cmpToRepMap[root]) break;
      retVal = check_errhandler_conditions((comm->oworldComm),&rreq,repToCmpMap[rank]);

      if(retVal == 0)
      {
        //retVal = EMPI_Isend (&ssync, 1, EMPI_CHAR, repToCmpMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &sreq);
				retVal = EMPI_Irecv (recvbuf, count, datatype->edatatype, repToCmpMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &rreq);
				//else retVal = EMPI_Irecv (&rsync, 1, EMPI_CHAR, repToCmpMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &rreq);
        //int sflag = 0;
				
				size = datatype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				(curargs->args).reduce.sendbuf = malloc(count*size);
				memcpy((curargs->args).reduce.sendbuf,sendbuf,count*size);
				(curargs->args).reduce.recvbuf = malloc(count*size);
				(curargs->args).reduce.count = count;
				(curargs->args).reduce.dt = datatype;
				(curargs->args).reduce.op = op;
				(curargs->args).reduce.root = root;
				(curargs->args).reduce.comm = comm;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_REDUCE;
				
        int rflag = 0;
        do
        {
          retVal = check_errhandler_conditions((comm->oworldComm),&rreq,repToCmpMap[rank]);
          if(retVal != 0) break;
          //EMPI_Test(&sreq,&sflag,&status);
          EMPI_Test(&rreq,&rflag,&status);
        } while(rflag == 0);
      }
      if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
      {
        //if(sreq != EMPI_REQUEST_NULL) EMPI_Request_free(&sreq);
        if(rreq != EMPI_REQUEST_NULL) EMPI_Request_free(&rreq);
				free((curargs->args).reduce.recvbuf);
				free((curargs->args).reduce.sendbuf);
				free(curargs);
        goto ConvertToCMPREDUCE;
      }
			if(retVal != 0)
			{
				free((curargs->args).reduce.recvbuf);
				free((curargs->args).reduce.sendbuf);
				free(curargs);
			}
    } while(retVal != 0);
		
		if(rank == cmpToRepMap[root]) memcpy((curargs->args).reduce.recvbuf,recvbuf,count*size);
		else
		{
			size = datatype->size;
			curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
			(curargs->args).reduce.sendbuf = malloc(count*size);
			memcpy((curargs->args).reduce.sendbuf,sendbuf,count*size);
			(curargs->args).reduce.recvbuf = malloc(count*size);
			(curargs->args).reduce.count = count;
			(curargs->args).reduce.dt = datatype;
			(curargs->args).reduce.op = op;
			(curargs->args).reduce.root = root;
			(curargs->args).reduce.comm = comm;
			if(last_collective == NULL) curargs->id = 0;
			else curargs->id = last_collective->id + 1;
			curargs->type = MPI_FT_REDUCE;
		}
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
  }
  else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
  {
ConvertToCMPREDUCE:
    /*EMPI_Comm_rank((comm->EMPI_COMM_CMP), &rank);
    targetrep = closest_replica(rank);
    if(targetrep != -1)
    {
      if(cmpToRepMap[rank] != targetrep)
      {
        do
        {
          retVal = 0;
          retVal = check_errhandler_conditions((comm->oworldComm),&req,targetrep);

          if(retVal == 0)
          {
            retVal = EMPI_Isend (sendbuf, count, edatatype, targetrep, MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &req);
            int flag = 0;
            do
            {
              retVal = check_errhandler_conditions((comm->oworldComm),&req,targetrep);
              if(retVal != 0) break;
              EMPI_Test(&req,&flag,&status);
            } while(flag == 0);
          }
          if(retVal != 0)
          {
            EMPI_Comm_rank((comm->EMPI_COMM_CMP),&rank);
            targetrep = closest_replica(rank);
            if(targetrep == -1 || cmpToRepMap[rank] == targetrep) retVal = 0;
          }
        } while(retVal != 0);
      }
    }
    req = EMPI_REQUEST_NULL;*/
    do
    {
      retVal = 0;
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);

      if(retVal == 0)
      {
        retVal = EMPI_Ireduce (sendbuf, recvbuf, count, datatype->edatatype, op->eop, root, (comm->EMPI_COMM_CMP), &req);
				
				size = datatype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				(curargs->args).reduce.sendbuf = malloc(count*size);
				memcpy((curargs->args).reduce.sendbuf,sendbuf,count*size);
				(curargs->args).reduce.recvbuf = malloc(count*size);
				(curargs->args).reduce.count = count;
				(curargs->args).reduce.dt = datatype;
				(curargs->args).reduce.op = op;
				(curargs->args).reduce.root = root;
				(curargs->args).reduce.comm = comm;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_REDUCE;
				
        int flag = 0;
        do
        {
          retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
          EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
			if(retVal != 0)
			{
				free((curargs->args).reduce.recvbuf);
				free((curargs->args).reduce.sendbuf);
				free(curargs);
			}
    } while(retVal != 0);

		if(rank == root) memcpy((curargs->args).reduce.recvbuf,recvbuf,count*size);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
//ConvertToCMPALLREDUCESYNC:
    req = EMPI_REQUEST_NULL;
    //char ssync = '0';
    //char rsync;
    do
    {
      retVal = 0;
      EMPI_Comm_rank((comm->EMPI_COMM_CMP),&rank);
      if(cmpToRepMap[rank] == -1 || rank != root) break;
      retVal = check_errhandler_conditions((comm->oworldComm),&sreq,cmpToRepMap[rank]+nC);

      if(retVal == 0)
      {
        retVal = EMPI_Isend (recvbuf, count, datatype->edatatype, cmpToRepMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &sreq);
				//else retVal = EMPI_Isend (&ssync, 1, EMPI_CHAR, cmpToRepMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &sreq);
        //retVal = EMPI_Irecv (&rsync, 1, EMPI_CHAR, cmpToRepMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &rreq);
        int sflag = 0;
        //int rflag = 0;
        do
        {
          retVal = check_errhandler_conditions((comm->oworldComm),&sreq,cmpToRepMap[rank]+nC);
          if(retVal != 0) break;
          EMPI_Test(&sreq,&sflag,&status);
          //EMPI_Test(&rreq,&rflag,&status);
        } while(sflag == 0);
      }
    } while(retVal != 0);
  }
  return MPI_SUCCESS;
}

/* MPI_Allreduce */
int MPI_Allreduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
  int retVal;
  EMPI_Request req = EMPI_REQUEST_NULL;
  EMPI_Request sreq = EMPI_REQUEST_NULL;
  EMPI_Request rreq = EMPI_REQUEST_NULL;
  EMPI_Status status;
  int oldsize = 0;
  int targetrep;
  int rank;
	int size;
	struct collective_data *curargs;
  if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
  {
    do
    {
      retVal = 0;
      EMPI_Comm_rank((comm->EMPI_COMM_REP),&rank);
      retVal = check_errhandler_conditions((comm->oworldComm),&rreq,repToCmpMap[rank]);

      if(retVal == 0)
      {
        //retVal = EMPI_Isend (&ssync, 1, EMPI_CHAR, repToCmpMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &sreq);
				retVal = EMPI_Irecv (recvbuf, count, datatype->edatatype, repToCmpMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &rreq);
				//else retVal = EMPI_Irecv (&rsync, 1, EMPI_CHAR, repToCmpMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &rreq);
        //int sflag = 0;
				
				size = datatype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				(curargs->args).allreduce.sendbuf = malloc(count*size);
				memcpy((curargs->args).allreduce.sendbuf,sendbuf,count*size);
				(curargs->args).allreduce.recvbuf = malloc(count*size);
				(curargs->args).allreduce.count = count;
				(curargs->args).allreduce.dt = datatype;
				(curargs->args).allreduce.op = op;
				(curargs->args).allreduce.comm = comm;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_ALLREDUCE;
				
        int rflag = 0;
        do
        {
          retVal = check_errhandler_conditions((comm->oworldComm),&rreq,repToCmpMap[rank]);
          if(retVal != 0) break;
          //EMPI_Test(&sreq,&sflag,&status);
          EMPI_Test(&rreq,&rflag,&status);
        } while(rflag == 0);
      }
      if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
      {
        //if(sreq != EMPI_REQUEST_NULL) EMPI_Request_free(&sreq);
        if(rreq != EMPI_REQUEST_NULL) EMPI_Request_free(&rreq);
				free((curargs->args).allreduce.recvbuf);
				free((curargs->args).allreduce.sendbuf);
				free(curargs);
        goto ConvertToCMPALLREDUCE;
      }
			if(retVal != 0)
			{
				free((curargs->args).allreduce.recvbuf);
				free((curargs->args).allreduce.sendbuf);
				free(curargs);
			}
    } while(retVal != 0);
    
		memcpy((curargs->args).allreduce.recvbuf,recvbuf,count*size);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
  }
  else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
  {
ConvertToCMPALLREDUCE:
    do
    {
      retVal = 0;
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);

      if(retVal == 0)
      {
        retVal = EMPI_Iallreduce (sendbuf, recvbuf, count, datatype->edatatype, op->eop, (comm->EMPI_COMM_CMP), &req);
				
				size = datatype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				(curargs->args).allreduce.sendbuf = malloc(count*size);
				memcpy((curargs->args).allreduce.sendbuf,sendbuf,count*size);
				(curargs->args).allreduce.recvbuf = malloc(count*size);
				(curargs->args).allreduce.count = count;
				(curargs->args).allreduce.dt = datatype;
				(curargs->args).allreduce.op = op;
				(curargs->args).allreduce.comm = comm;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_ALLREDUCE;
				
        int flag = 0;
        do
        {
          retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
          EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
			if(retVal != 0)
			{
				free((curargs->args).allreduce.recvbuf);
				free((curargs->args).allreduce.sendbuf);
				free(curargs);
			}
    } while(retVal != 0);

		memcpy((curargs->args).allreduce.recvbuf,recvbuf,count*size);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
//ConvertToCMPALLREDUCESYNC:
    req = EMPI_REQUEST_NULL;
    //char ssync = '0';
    //char rsync;
    do
    {
      retVal = 0;
      EMPI_Comm_rank((comm->EMPI_COMM_CMP),&rank);
      if(cmpToRepMap[rank] == -1) break;
      retVal = check_errhandler_conditions((comm->oworldComm),&sreq,cmpToRepMap[rank]+nC);

      if(retVal == 0)
      {
        retVal = EMPI_Isend (recvbuf, count, datatype->edatatype, cmpToRepMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &sreq);
				//else retVal = EMPI_Isend (&ssync, 1, EMPI_CHAR, cmpToRepMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &sreq);
        //retVal = EMPI_Irecv (&rsync, 1, EMPI_CHAR, cmpToRepMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &rreq);
        int sflag = 0;
        //int rflag = 0;
        do
        {
          retVal = check_errhandler_conditions((comm->oworldComm),&sreq,cmpToRepMap[rank]+nC);
          if(retVal != 0) break;
          EMPI_Test(&sreq,&sflag,&status);
          //EMPI_Test(&rreq,&rflag,&status);
        } while(sflag == 0);
      }
    } while(retVal != 0);
  }
  return MPI_SUCCESS;
}

/*int MPI_Allreduce(void *sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
	int retVal;
	int retValinter;
	int retValintra;
	EMPI_Request reqinter = EMPI_REQUEST_NULL;
  EMPI_Request req = EMPI_REQUEST_NULL;
	EMPI_Request reqintra = EMPI_REQUEST_NULL;
  EMPI_Datatype edatatype;
  EMPI_Status status;
  int oldsize = 0;
  int targetrep;
  int rank;
	int flagcount;
	int commnC;
	int commnR;
	int size;
	struct collective_data *curargs;
	void *tmpbuf;
  if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
  {
		do
    {
			retVal = 0;
			retValinter = 0;
			retValintra = 0;
			EMPI_Comm_rank((comm->EMPI_COMM_REP),&rank);
			EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
			EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
			if(commnC > commnR && commnR > 0)
			{
				tmpbuf = malloc(count*datatype->size);
				retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);
				if(retValinter == 0)
				{
					if(rank == 0) retValinter = EMPI_Ireduce (sendbuf, tmpbuf, count, datatype->edatatype, op->eop, EMPI_ROOT, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqinter);
					else retValinter = EMPI_Ireduce (sendbuf, tmpbuf, count, datatype->edatatype, op->eop, EMPI_PROC_NULL, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqinter);
				}
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					if(reqinter != EMPI_REQUEST_NULL) EMPI_Request_free(&reqinter);
					free(tmpbuf);
					goto ConvertToCMPALLREDUCE;
				}
			}
			if(retValinter != 0)
			{
				if(commnC > commnR && commnR > 0) free(tmpbuf);
				continue;
			}

      retVal = 0;
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
      if(retVal == 0)
      {
        retVal = EMPI_Ireduce (sendbuf, recvbuf, count, datatype->edatatype, op->eop, 0, (comm->EMPI_COMM_REP), &req);
			}
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				if(commnC > commnR && commnR > 0) free(tmpbuf);
				goto ConvertToCMPALLREDUCE;
			}
			if(retVal != 0)
			{
				if(commnC > commnR && commnR > 0) free(tmpbuf);
				continue;
			}
			
			if(rank == 0)
			{
				size = datatype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				(curargs->args).allreduce.sendbuf = malloc(count*size);
				memcpy((curargs->args).allreduce.sendbuf,sendbuf,count*size);
				(curargs->args).allreduce.recvbuf = malloc(count*size);
				(curargs->args).allreduce.count = count;
				(curargs->args).allreduce.dt = datatype;
				(curargs->args).allreduce.op = op;
				(curargs->args).allreduce.comm = comm;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_ALLREDUCE;
				
				int flag = 0;
				int flaginter = !(commnC > commnR && commnR > 0);
				do
				{
					if(flag == 0) retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
					if(retVal != 0) break;
					if(commnC > commnR && commnR > 0 && flaginter == 0) retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);
					if(retValinter != 0) break;
					if(flag == 0) EMPI_Test(&req,&flag,&status);
					if(commnC > commnR && commnR > 0 && flaginter == 0) EMPI_Test(&reqinter,&flaginter,&status);
				} while(flag == 0 || flaginter == 0);
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					if(commnC > commnR && commnR > 0) free(tmpbuf);
					free((curargs->args).allreduce.recvbuf);
					free((curargs->args).allreduce.sendbuf);
					free(curargs);
					goto ConvertToCMPALLREDUCE;
				}
				if(retVal != 0)
				{
					if(commnC > commnR && commnR > 0) free(tmpbuf);
					free((curargs->args).allreduce.recvbuf);
					free((curargs->args).allreduce.sendbuf);
					free(curargs);
					continue;
				}
				if(commnC > commnR && commnR > 0)
				{
					for(int i = 0; i < count; i++)
					{
						*((int *)(recvbuf + (i*datatype->size))) = *((int *)(recvbuf + (i*datatype->size))) + *((int *)(tmpbuf + (i*datatype->size)));
					}
				}
			}
			
			retValintra = 0;
      retValintra = check_errhandler_conditions_collective((comm->oworldComm),&reqintra);
      if(retVal == 0)
      {
        retValintra = EMPI_Ibcast (recvbuf, count, datatype->edatatype, 0, (comm->EMPI_COMM_REP), &reqintra);
			}
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				goto ConvertToCMPALLREDUCE;
			}
			if(retValintra != 0)
			{
				continue;
			}
			
			if(rank != 0)
			{
				size = datatype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				(curargs->args).allreduce.sendbuf = malloc(count*size);
				memcpy((curargs->args).allreduce.sendbuf,sendbuf,count*size);
				(curargs->args).allreduce.recvbuf = malloc(count*size);
				(curargs->args).allreduce.count = count;
				(curargs->args).allreduce.dt = datatype;
				(curargs->args).allreduce.op = op;
				(curargs->args).allreduce.comm = comm;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_ALLREDUCE;
				
				int flag = 0;
				int flaginter = !(commnC > commnR && commnR > 0);
				int flagintra = 0;
				do
				{
					if(flag == 0) retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
					if(retVal != 0) break;
					if(commnC > commnR && commnR > 0 && flaginter == 0) retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);
					if(retValinter != 0) break;
					if(flagintra == 0) retValintra = check_errhandler_conditions_collective((comm->oworldComm),&reqintra);
					if(retValintra != 0) break;
					if(flag == 0) EMPI_Test(&req,&flag,&status);
					if(commnC > commnR && commnR > 0 && flaginter == 0) EMPI_Test(&reqinter,&flaginter,&status);
					if(flagintra == 0) EMPI_Test(&reqintra,&flagintra,&status);
				} while(flag == 0 || flaginter == 0 || flagintra == 0);
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					if(commnC > commnR && commnR > 0) free(tmpbuf);
					free((curargs->args).allreduce.recvbuf);
					free((curargs->args).allreduce.sendbuf);
					free(curargs);
					goto ConvertToCMPALLREDUCE;
				}
				if(retVal != 0)
				{
					if(commnC > commnR && commnR > 0) free(tmpbuf);
					free((curargs->args).allreduce.recvbuf);
					free((curargs->args).allreduce.sendbuf);
					free(curargs);
					continue;
				}
			}
			else
			{
				int flagintra = 0;
				do
				{
					retValintra = check_errhandler_conditions_collective((comm->oworldComm),&reqintra);
					if(retValintra != 0) break;
					EMPI_Test(&reqintra,&flagintra,&status);
				} while(flagintra == 0);
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					if(commnC > commnR && commnR > 0) free(tmpbuf);
					free((curargs->args).allreduce.recvbuf);
					free((curargs->args).allreduce.sendbuf);
					free(curargs);
					goto ConvertToCMPALLREDUCE;
				}
				if(retVal != 0)
				{
					if(commnC > commnR && commnR > 0) free(tmpbuf);
					free((curargs->args).allreduce.recvbuf);
					free((curargs->args).allreduce.sendbuf);
					free(curargs);
					continue;
				}
			}
			
    } while(retVal != 0 || retValinter != 0 || retValintra != 0);
    
		//memcpy((curargs->args).allgather.recvbuf,recvbuf,commnC*recvcount*rsize);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		
		if(commnC > commnR && commnR > 0) free(tmpbuf);
		req = EMPI_REQUEST_NULL;
		if(commnR > 0 && commnC == commnR)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					if(req != EMPI_REQUEST_NULL) EMPI_Request_free(&req);
					goto ConvertToCMPALLREDUCESTORED;
				}
			} while(retVal != 0);
		}
  }
  else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
 {
ConvertToCMPALLREDUCE:
		do
    {
      retVal = 0;
			retValinter = 0;
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
			else commnR = 0;

      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			if(commnR > 0 && cmpToRepMap[rank] == -1) tmpbuf = malloc(count * datatype->size);
			if(commnR > 0 && cmpToRepMap[rank] == -1) retValinter = check_errhandler_conditions_collective((comm->oworldComm),&req);
      if(retVal == 0 && retValinter == 0)
      {
        retVal = EMPI_Iallreduce (sendbuf, recvbuf, count, datatype->edatatype, op->eop, (comm->EMPI_COMM_CMP), &req);
				if(commnR > 0 && cmpToRepMap[rank] == -1) retValinter = EMPI_Ireduce (sendbuf, tmpbuf,count, datatype->edatatype, op->eop, 0, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqinter);
			}
			
			if(retVal != 0 || retValinter != 0)
			{
				if(commnR > 0 && cmpToRepMap[rank] == -1) free(tmpbuf);
				continue;
			}
			
			size = datatype->size;
			curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
			(curargs->args).allreduce.sendbuf = malloc(count*size);
			memcpy((curargs->args).allreduce.sendbuf,sendbuf,count*size);
			(curargs->args).allreduce.recvbuf = malloc(count*size);
			(curargs->args).allreduce.count = count;
			(curargs->args).allreduce.dt = datatype;
			(curargs->args).allreduce.op = op;
			(curargs->args).allreduce.comm = comm;
			if(last_collective == NULL) curargs->id = 0;
			else curargs->id = last_collective->id + 1;
			curargs->type = MPI_FT_ALLREDUCE;
			curargs->repreq = &reqinter;
			
			retVal = 0;
			retValinter = 0;
			int flag = 0;
      do
      {
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
				if(retVal != 0) break;
				EMPI_Test(&req,&flag,&status);
			} while(flag == 0);
			
			if(retVal != 0 || retValinter != 0)
			{
				if(commnR > 0 && cmpToRepMap[rank] == -1)
				{
					free(tmpbuf);
				}
				free((curargs->args).allreduce.recvbuf);
				free((curargs->args).allreduce.sendbuf);
				free(curargs);
			}
		} while(retVal != 0 || retValinter != 0);
    
		//memcpy((curargs->args).gather.recvbuf,recvbuf,commnC*recvcount*rsize);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		do
    {
			retValinter = 0;
			reqinter =  *(curargs->repreq);
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			int flaginter = !(commnR > 0 && cmpToRepMap[rank] == -1);
      do
      {
				if(flaginter == 0 && commnR > 0 != -1 && cmpToRepMap[rank] == -1) retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);
				if(retValinter != 0) break;
				if(flaginter == 0 && commnR > 0 != -1 && cmpToRepMap[rank] == -1) EMPI_Test(&reqinter,&flaginter,&status);
			} while(flaginter == 0);
		} while(retValinter != 0);
		//last_collective->completerep = true;
		if(commnR > 0 != -1 && cmpToRepMap[rank] == -1) free(tmpbuf);
ConvertToCMPALLREDUCESTORED:
    req = EMPI_REQUEST_NULL;
		if(commnR > 0 && commnC == commnR)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			} while(retVal != 0);
		}
  }
  return MPI_SUCCESS;
}*/

/*int MPI_Allreduce(void *sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
	int retVal;
	int retValinter;
	EMPI_Request *reqs;
	bool *completereq;
  EMPI_Request req = EMPI_REQUEST_NULL;
  EMPI_Datatype edatatype;
  EMPI_Status status;
  int oldsize = 0;
  int targetrep;
  int rank;
	int flagcount;
	int commnC;
	int commnR;
	int size;
	struct collective_data *curargs;
	void *tmpbuf;
  if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
  {
		do
    {
			retVal = 0;
			retValinter = 0;
			EMPI_Comm_rank((comm->EMPI_COMM_REP),&rank);
			EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
			EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
			if(commnC > commnR && commnR > 0) tmpbuf = malloc(count * datatype->size);
			
			if(commnC > commnR && commnR > 0)
			{
				reqs = (EMPI_Request *)malloc(commnR*sizeof(EMPI_Request));
				completereq = (bool *)malloc(commnR*sizeof(bool));
				for(int i = 0; i < commnR; i++)
				{
					completereq[i] = false;
					retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
					if(retValinter == 0)
					{
						if(rank == i) retValinter = EMPI_Ireduce (sendbuf, tmpbuf, count, datatype->edatatype, op->eop, EMPI_ROOT, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
						else retValinter = EMPI_Ireduce (sendbuf, tmpbuf, count, datatype->edatatype, op->eop, EMPI_PROC_NULL, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
					}
					if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
					{
						for(int j = 0; j <= i; j++)
						{
							if(reqs[j] != EMPI_REQUEST_NULL) EMPI_Request_free(&reqs[j]);
						}
						if(commnC > commnR && commnR > 0) free(reqs);
						if(commnC > commnR && commnR > 0) free(completereq);
						if(commnC > commnR && commnR > 0) free(tmpbuf);
						goto ConvertToCMPALLREDUCE;
					}
					if(retValinter != 0) break;
				}
			}
			if(retValinter != 0)
			{
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				if(commnC > commnR && commnR > 0) free(tmpbuf);
				continue;
			}

      retVal = 0;
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnC);
			EMPI_Comm_size(comm->EMPI_COMM_REP,&commnR);
      if(retVal == 0)
      {
        retVal = EMPI_Iallreduce (sendbuf, recvbuf, count, datatype->edatatype, op->eop, (comm->EMPI_COMM_REP), &req);
				
				size = datatype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				(curargs->args).allreduce.sendbuf = malloc(count*size);
				memcpy((curargs->args).allreduce.sendbuf,sendbuf,count*size);
				(curargs->args).allreduce.recvbuf = malloc(count*size);
				(curargs->args).allreduce.count = count;
				(curargs->args).allreduce.dt = datatype;
				(curargs->args).allreduce.op = op;
				(curargs->args).allreduce.comm = comm;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_ALLREDUCE;
				curargs->repreq = reqs;
				
        int flag = 0;
        do
        {
          retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
          EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				for(int j = 0; j <= commnR; j++)
				{
					if(reqs[j] != EMPI_REQUEST_NULL && !completereq[j]) EMPI_Request_free(&reqs[j]);
				}
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				if(commnC > commnR && commnR > 0) free(tmpbuf);
				free((curargs->args).allgather.recvbuf);
				free((curargs->args).allgather.sendbuf);
				free(curargs);
				goto ConvertToCMPALLREDUCE;
			}
			if(retVal != 0)
			{
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				if(commnC > commnR && commnR > 0) free(tmpbuf);
				free((curargs->args).allgather.recvbuf);
				free((curargs->args).allgather.sendbuf);
				free(curargs);
				continue;
			}
			
			retValinter = 0;
			if(commnC > commnR && commnR > 0) flagcount = 0;
			else flagcount = commnR;
			int flaginter = 0;
			do
			{
				flaginter = 0;
				if(commnC > commnR && commnR > 0)
				{
					for(int i = 0; i < commnR; i++)
					{
						if(!completereq[i])
						{
							flaginter = 0;
							retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
							if(retValinter != 0) break;
							EMPI_Test(&reqs[i],&flaginter,&status);
							flagcount += flaginter;
							if(flaginter > 0) completereq[i] = true;
						}
					}
					if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
					{
						for(int j = 0; j <= commnR; j++)
						{
							if(reqs[j] != EMPI_REQUEST_NULL && !completereq[j]) EMPI_Request_free(&reqs[j]);
						}
						if(commnC > commnR && commnR > 0) free(reqs);
						if(commnC > commnR && commnR > 0) free(completereq);
						if(commnC > commnR && commnR > 0) free(tmpbuf);
						free((curargs->args).allgather.recvbuf);
						free((curargs->args).allgather.sendbuf);
						free(curargs);
						goto ConvertToCMPALLREDUCE;
					}
				}
				if(retValinter != 0) break;
			} while(flagcount < commnR);
			if(retValinter != 0)
			{
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				if(commnC > commnR && commnR > 0) free(tmpbuf);
				free((curargs->args).allgather.recvbuf);
				free((curargs->args).allgather.sendbuf);
				free(curargs);
			}
    } while(retVal != 0 || retValinter != 0);
    
		if(commnC > commnR && commnR > 0)
		{
			for(int i = 0; i < count; i++)
			{
				*((int *)(recvbuf + (i*datatype->size))) = *((int *)(recvbuf + (i*datatype->size))) + *((int *)(tmpbuf + (i*datatype->size)));
			}
		}
		
		//memcpy((curargs->args).allreduce.recvbuf,recvbuf,count*size);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		if(commnC > commnR && commnR > 0) free(reqs);
		if(commnC > commnR && commnR > 0) free(completereq);
		if(commnC > commnR && commnR > 0) free(tmpbuf);
		req = EMPI_REQUEST_NULL;
		if(commnR > 0 && commnC == commnR)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					if(req != EMPI_REQUEST_NULL) EMPI_Request_free(&req);
					goto ConvertToCMPALLREDUCESTORED;
				}
			} while(retVal != 0);
		}
  }
  else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
  {
ConvertToCMPALLREDUCE:
		do
    {
      retVal = 0;
			retValinter = 0;
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
			else commnR = 0;
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			if(commnR > 0 && cmpToRepMap[rank] == -1)
			{
				tmpbuf = malloc(count * datatype->size);
				reqs = (EMPI_Request *)malloc(commnR*sizeof(EMPI_Request));
				completereq = (bool *)malloc(commnR*sizeof(bool));
				for(int i = 0; i < commnR; i++)
				{
					completereq[i] = false;
					retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
					if(retValinter == 0)
					{
						retValinter = EMPI_Ireduce (sendbuf, tmpbuf, count, datatype->edatatype, op->eop, i, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
					}
					if(retValinter != 0) break;
				}
			}
			if(retValinter != 0)
			{
				if(commnR > 0 && cmpToRepMap[rank] == -1)
				{
					free(reqs);
					free(completereq);
					free(tmpbuf);
				}
				continue;
			}
			
      retVal = 0;
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);

      if(retVal == 0)
      {
        retVal = EMPI_Iallreduce (sendbuf, recvbuf, count, datatype->edatatype, op->eop, (comm->EMPI_COMM_CMP), &req);
				
				size = datatype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				(curargs->args).allreduce.sendbuf = malloc(count*size);
				memcpy((curargs->args).allreduce.sendbuf,sendbuf,count*size);
				(curargs->args).allreduce.recvbuf = malloc(count*size);
				(curargs->args).allreduce.count = count;
				(curargs->args).allreduce.dt = datatype;
				(curargs->args).allreduce.op = op;
				(curargs->args).allreduce.comm = comm;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_ALLREDUCE;
				curargs->repreq = reqs;
		
        int flag = 0;
        do
        {
          retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
          EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
			if(retVal != 0)
			{
				if(commnR > 0 && cmpToRepMap[rank] == -1)
				{
					free(reqs);
					free(completereq);
					free(tmpbuf);
				}
				free((curargs->args).allgather.recvbuf);
				free((curargs->args).allgather.sendbuf);
				free(curargs);
			}
    } while(retVal != 0 || retValinter != 0);
		
		//memcpy((curargs->args).allgather.recvbuf,recvbuf,commnC*recvcount*rsize);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		do
    {
			retValinter = 0;
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
			else commnR = 0;
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			if(commnR > 0 && cmpToRepMap[rank] == -1) flagcount = 0;
			else flagcount = commnR;
			reqs =  (curargs->repreq);
			int flaginter = 0;
			do
			{
				flaginter = 0;
				if(commnR > 0 && cmpToRepMap[rank] == -1)
				{
					for(int i = 0; i < commnR; i++)
					{
						if(!completereq[i])
						{
							flaginter = 0;
							retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
							if(retValinter != 0) break;
							EMPI_Test(&reqs[i],&flaginter,&status);
							flagcount += flaginter;
							if(flaginter > 0) completereq[i] = true;
						}
					}
				}
				if(retValinter != 0) break;
			} while(flagcount < commnR);
		} while(retValinter != 0);
		last_collective->completerep = true;
		if(commnR > 0 && cmpToRepMap[rank] == -1)
		{
			free(reqs);
			free(completereq);
			free(tmpbuf);
		}
ConvertToCMPALLREDUCESTORED:
		req = EMPI_REQUEST_NULL;
		if(commnR > 0 && commnC == commnR)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			} while(retVal != 0);
		}
  }
  return MPI_SUCCESS;
}*/

int closest_replica(int cmpRank)
{
  if(nR <= 0) return -1;
  if(cmpToRepMap[cmpRank] != -1) return cmpToRepMap[cmpRank];
  else
  {
    for(int i = (cmpRank+1); i != nC; i++)
    {
      if(cmpToRepMap[i] == 0) return 0;
      else if(cmpToRepMap[i] > 0)
      {
        int fdist = i - cmpRank;
        for(int j = (cmpRank-1); j != 0; j--)
        {
          if(cmpRank - j >= fdist) return cmpToRepMap[i];
          else if(cmpToRepMap[j] >= 0) return cmpToRepMap[j];
        }
      }
    }
    
    return nR-1;
  }
}

/*int MPI_Bcast (void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
  int retVal;
  EMPI_Request req = EMPI_REQUEST_NULL;
  EMPI_Request sreq = EMPI_REQUEST_NULL;
  EMPI_Request rreq = EMPI_REQUEST_NULL;
  EMPI_Status status;
  int oldsize = 0;
  int targetrep;
  int rank;
	int size;
	struct collective_data *curargs;
  if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
  {
    do
    {
      retVal = 0;
      EMPI_Comm_rank((comm->EMPI_COMM_REP),&rank);
      retVal = check_errhandler_conditions((comm->oworldComm),&rreq,repToCmpMap[rank]);

      if(retVal == 0)
      {
				retVal = EMPI_Irecv (recvbuf, count, datatype->edatatype, repToCmpMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &rreq);
				
        int rflag = 0;
        do
        {
          retVal = check_errhandler_conditions((comm->oworldComm),&rreq,repToCmpMap[rank]);
          if(retVal != 0) break;
          EMPI_Test(&rreq,&rflag,&status);
        } while(rflag == 0);
      }
      if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
      {
        //if(sreq != EMPI_REQUEST_NULL) EMPI_Request_free(&sreq);
        if(rreq != EMPI_REQUEST_NULL) EMPI_Request_free(&rreq);
        goto ConvertToCMPBCAST;
      }
    } while(retVal != 0);
    
		size = datatype->size;
		curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
		(curargs->args).bcast.buf = malloc(count*size);
		if(cmpToRepMap[root] == rank) memcpy((curargs->args).bcast.buf,buffer,count*size);
		(curargs->args).bcast.count = count;
		(curargs->args).bcast.dt = datatype;
		(curargs->args).bcast.root = root;
		(curargs->args).bcast.comm = comm;
		curargs->completecmp = true;
		curargs->completerep = true;
		if(last_collective == NULL) curargs->id = 0;
		else curargs->id = last_collective->id + 1;
		curargs->type = MPI_FT_BCAST;
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
  }
  else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
  {
ConvertToCMPBCAST:
    do
    {
      retVal = 0;
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);

      if(retVal == 0)
      {
        retVal = EMPI_Ibcast (buffer, count, datatype->edatatype, root, (comm->EMPI_COMM_CMP), &req);
				
        int flag = 0;
        do
        {
          retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
          EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
    } while(retVal != 0);

		size = datatype->size;
		curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
		(curargs->args).bcast.buf = malloc(count*size);
		if(cmpToRepMap[root] == rank) memcpy((curargs->args).bcast.buf,buffer,count*size);
		(curargs->args).bcast.count = count;
		(curargs->args).bcast.dt = datatype;
		(curargs->args).bcast.root = root;
		(curargs->args).bcast.comm = comm;
		curargs->completecmp = true;
		curargs->completerep = true;
		if(last_collective == NULL) curargs->id = 0;
		else curargs->id = last_collective->id + 1;
		curargs->type = MPI_FT_BCAST;
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
//ConvertToCMPALLREDUCESYNC:
    req = EMPI_REQUEST_NULL;
    do
    {
      retVal = 0;
      EMPI_Comm_rank((comm->EMPI_COMM_CMP),&rank);
      if(cmpToRepMap[rank] == -1) break;
      retVal = check_errhandler_conditions((comm->oworldComm),&sreq,cmpToRepMap[rank]);

      if(retVal == 0)
      {
        retVal = EMPI_Isend (buffer, count, datatype->edatatype, cmpToRepMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &sreq);
				//else retVal = EMPI_Isend (&ssync, 1, EMPI_CHAR, cmpToRepMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &sreq);
        //retVal = EMPI_Irecv (&rsync, 1, EMPI_CHAR, cmpToRepMap[rank], MPI_FT_COLLECTIVE_TAG, (comm->EMPI_CMP_REP_INTERCOMM), &rreq);
        int sflag = 0;
        //int rflag = 0;
        do
        {
          retVal = check_errhandler_conditions((comm->oworldComm),&sreq,cmpToRepMap[rank]);
          if(retVal != 0) break;
          EMPI_Test(&sreq,&sflag,&status);
        } while(sflag == 0);
      }
    } while(retVal != 0);
  }
  return MPI_SUCCESS;
}*/

int MPI_Bcast (void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
  //return EMPI_Bcast(buffer,count,datatype->edatatype,root,comm->eworldComm);
  int retVal;
	int retValinter;
  EMPI_Request req = EMPI_REQUEST_NULL;
	EMPI_Request reqinter = EMPI_REQUEST_NULL;
  EMPI_Request sreq = EMPI_REQUEST_NULL;
  EMPI_Request rreq = EMPI_REQUEST_NULL;
  EMPI_Datatype edatatype;
  EMPI_Status status;
  int oldsize = 0;
  int targetrep;
  int rank;
  edatatype = datatype->edatatype;
	int size;
	struct collective_data *curargs;
  if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
  {		
		do
    {
      retVal = 0;
			EMPI_Comm_rank(comm->EMPI_COMM_REP,&rank);
			if(cmpToRepMap[root] == -1) retVal = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);
			else retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);

      if(retVal == 0)
      {
				if(cmpToRepMap[root] == -1) EMPI_Ibcast (buffer, count, edatatype, root, (comm->EMPI_CMP_REP_INTERCOMM), &reqinter);
        else retVal = EMPI_Ibcast (buffer, count, edatatype, cmpToRepMap[root], (comm->EMPI_COMM_REP), &req);
				
				size = datatype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				(curargs->args).bcast.buf = malloc(count*size);
				if(cmpToRepMap[root] == rank) memcpy((curargs->args).bcast.buf,buffer,count*size);
				(curargs->args).bcast.count = count;
				(curargs->args).bcast.dt = datatype;
				(curargs->args).bcast.root = root;
				(curargs->args).bcast.comm = comm;
				curargs->completecmp = true;
				curargs->completerep = true;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_BCAST;
				curargs->repreq = &reqinter;
				
        int flag = 0;
        do
        {
          if(cmpToRepMap[root] == -1) retVal = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);
					else retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
					if(cmpToRepMap[root] == -1) EMPI_Test(&reqinter,&flag,&status);
          else EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				if(cmpToRepMap[root] == -1 && reqinter != EMPI_REQUEST_NULL) EMPI_Request_free(&reqinter);
				else if(req != EMPI_REQUEST_NULL) EMPI_Request_free(&req);
				free((curargs->args).bcast.buf);
				free(curargs);
				goto ConvertToCMPBCAST;
			}
			if(retVal != 0)
			{
				free((curargs->args).bcast.buf);
				free(curargs);
			}
    } while(retVal != 0);
		
		if(cmpToRepMap[root] != rank) memcpy((curargs->args).bcast.buf,buffer,count*size);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		req = EMPI_REQUEST_NULL;
		do
		{
			retVal = 0;
			retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				goto ConvertToCMPBCASTSTORED;
			}
		} while(retVal != 0);
  }
  else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
  {
ConvertToCMPBCAST:
    do
    {
      retVal = 0;
			retValinter = 0;
			EMPI_Comm_rank((comm->EMPI_COMM_CMP),&rank);
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			if(cmpToRepMap[root] == -1 && nR > 0) retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);

      if(retVal == 0 && retValinter == 0)
      {
        retVal = EMPI_Ibcast (buffer, count, edatatype, root, (comm->EMPI_COMM_CMP), &req);
				if(cmpToRepMap[root] == -1 && rank == root && nR > 0) retValinter = EMPI_Ibcast (buffer, count, edatatype, EMPI_ROOT, (comm->EMPI_CMP_REP_INTERCOMM), &reqinter);
				else if(cmpToRepMap[root] == -1 && nR > 0) retValinter = EMPI_Ibcast (buffer, count, edatatype, EMPI_PROC_NULL, (comm->EMPI_CMP_REP_INTERCOMM), &reqinter);
			}
			
			if(retVal != 0 || retValinter != 0)
			{
				continue;
			}
			
			size = datatype->size;
			curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
			(curargs->args).bcast.buf = malloc(count*size);
			if(rank == root) memcpy((curargs->args).bcast.buf,buffer,count*size);
			(curargs->args).bcast.count = count;
			(curargs->args).bcast.dt = datatype;
			(curargs->args).bcast.root = root;
			(curargs->args).bcast.comm = comm;
			curargs->completecmp = true;
			curargs->completerep = false;
			if(last_collective == NULL) curargs->id = 0;
			else curargs->id = last_collective->id + 1;
			curargs->type = MPI_FT_BCAST;
			curargs->repreq = &reqinter;
			
			retVal = 0;
			retValinter = 0;
			int flag = 0;
      do
      {
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
				if(retVal != 0) break;
				EMPI_Test(&req,&flag,&status);
			} while(flag == 0);
			
			if(retVal != 0)
			{
				free((curargs->args).bcast.buf);
				free(curargs);
			}
		} while(retVal != 0 || retValinter != 0);
		
		if(rank != root) memcpy((curargs->args).bcast.buf,buffer,count*size);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		do
    {
			retValinter = 0;
			reqinter =  *(curargs->repreq);
			int flaginter = !(cmpToRepMap[root] == -1 && nR > 0);
      do
      {
				if(cmpToRepMap[root] == -1 && flaginter == 0 && nR > 0) retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);
				if(retValinter != 0) break;
				if(cmpToRepMap[root] == -1 && flaginter == 0 && nR > 0) EMPI_Test(&reqinter,&flaginter,&status);
			} while(flaginter == 0);
		} while(retValinter != 0);
		last_collective->completerep = true;
ConvertToCMPBCASTSTORED:
    req = EMPI_REQUEST_NULL;
		if(cmpToRepMap[root] != -1 && nR > 0)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			} while(retVal != 0);
		}
  }
  return MPI_SUCCESS;
}

int MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm)
{
  int retVal;
	int retValinter;
  EMPI_Request req = EMPI_REQUEST_NULL;
	EMPI_Request reqinter = EMPI_REQUEST_NULL;
  EMPI_Request sreq = EMPI_REQUEST_NULL;
  EMPI_Status status;
  int oldsize = 0;
  int targetrep;
  int rank;
	int *tmpscounts;
	int *tmpsdispls;
	int commnC;
	int commnR;
	int ssize,rsize;
	struct collective_data *curargs;
  if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
  {
    do
    {
      retVal = 0;
			EMPI_Comm_rank((comm->EMPI_COMM_REP),&rank);
			EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
			EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
			tmpscounts = (int *)malloc(commnR*sizeof(int));
			tmpsdispls = (int *)malloc(commnR*sizeof(int));
			int j = 0;
			for(int i = 0; i < commnC; i++)
			{
				if(cmpToRepMap[i] != -1)
				{
					tmpscounts[j] = sendcount;
					tmpsdispls[j] = i*sendcount;
					j++;
				}
			}
			if(cmpToRepMap[root] == -1) retVal = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);
			else retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
      if(retVal == 0)
      {
				if(cmpToRepMap[root] == -1) EMPI_Iscatterv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, root, (comm->EMPI_CMP_REP_INTERCOMM), &reqinter);
        else retVal = EMPI_Iscatterv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, cmpToRepMap[root], (comm->EMPI_COMM_REP), &req);
				
				ssize = sendtype->size;
				rsize = sendtype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnC);
				(curargs->args).scatter.sendbuf = malloc(commnC*sendcount*ssize);
				memcpy((curargs->args).scatter.sendbuf,sendbuf,commnC*sendcount*ssize);
				(curargs->args).scatter.recvbuf = malloc(recvcount*rsize);
				(curargs->args).scatter.sendcount = sendcount;
				(curargs->args).scatter.senddt = sendtype;
				(curargs->args).scatter.recvcount = recvcount;
				(curargs->args).scatter.recvdt = recvtype;
				(curargs->args).scatter.root = root;
				(curargs->args).scatter.comm = comm;
				curargs->completecmp = true;
				curargs->completerep = true;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_SCATTER;
				curargs->repreq = &reqinter;
				
        int flag = 0;
        do
        {
          if(cmpToRepMap[root] == -1) retVal = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);
					else retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
					if(cmpToRepMap[root] == -1) EMPI_Test(&reqinter,&flag,&status);
          else EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				if(cmpToRepMap[root] == -1 && reqinter != EMPI_REQUEST_NULL) EMPI_Request_free(&reqinter);
				else if(req != EMPI_REQUEST_NULL) EMPI_Request_free(&req);
				free(tmpscounts);
				free(tmpsdispls);
				free((curargs->args).scatter.recvbuf);
				free((curargs->args).scatter.sendbuf);
				free(curargs);
				goto ConvertToCMPSCATTER;
			}
			if(retVal != 0)
			{
				free(tmpscounts);
				free(tmpsdispls);
				free((curargs->args).scatter.recvbuf);
				free((curargs->args).scatter.sendbuf);
				free(curargs);
			}
    } while(retVal != 0);
		
		free(tmpscounts);
		free(tmpsdispls);
		
		//memcpy((curargs->args).scatter.recvbuf,recvbuf,recvcount*rsize);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		req = EMPI_REQUEST_NULL;
		do
		{
			retVal = 0;
			retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				goto ConvertToCMPSCATTERSTORED;
			}
		} while(retVal != 0);
  }
  else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
  {
ConvertToCMPSCATTER:
    do
    {
      retVal = 0;
			retValinter = 0;
			EMPI_Comm_rank((comm->EMPI_COMM_CMP),&rank);
			EMPI_Comm_size((comm->EMPI_COMM_CMP),&commnC);
			if(comm->EMPI_CMP_REP_INTERCOMM != EMPI_COMM_NULL) EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnR);
			else commnR = 0;
			if(cmpToRepMap[root] == -1 && nR > 0)
			{
				tmpscounts = (int *)malloc(commnR*sizeof(int));
				tmpsdispls = (int *)malloc(commnR*sizeof(int));
				int j = 0;
				for(int i = 0; i < commnC; i++)
				{
					if(cmpToRepMap[i] != -1)
					{
						tmpscounts[j] = sendcount;
						tmpsdispls[j] = i*sendcount;
						j++;
					}
				}
			}
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			if(cmpToRepMap[root] == -1 && nR > 0) retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);

      if(retVal == 0 && retValinter == 0)
      {
        retVal = EMPI_Iscatter (sendbuf, sendcount, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, root, (comm->EMPI_COMM_CMP), &req);
				if(cmpToRepMap[root] == -1 && rank == root && nR > 0) retValinter = EMPI_Iscatterv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, EMPI_ROOT, (comm->EMPI_CMP_REP_INTERCOMM), &reqinter);
				else if(cmpToRepMap[root] == -1 && nR > 0) retValinter = EMPI_Iscatterv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, EMPI_PROC_NULL, (comm->EMPI_CMP_REP_INTERCOMM), &reqinter);
			}
			
			if(retVal != 0 || retValinter != 0)
			{
				if(cmpToRepMap[root] == -1 && nR > 0)
				{
					free(tmpscounts);
					free(tmpsdispls);
				}
				continue;
			}
			
			ssize = sendtype->size;
			rsize = recvtype->size;
			curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			(curargs->args).scatter.sendbuf = malloc(commnC*sendcount*ssize);
			memcpy((curargs->args).scatter.sendbuf,sendbuf,commnC*sendcount*ssize);
			(curargs->args).scatter.recvbuf = malloc(recvcount*rsize);
			(curargs->args).scatter.sendcount = sendcount;
			(curargs->args).scatter.senddt = sendtype;
			(curargs->args).scatter.recvcount = recvcount;
			(curargs->args).scatter.recvdt = recvtype;
			(curargs->args).scatter.root = root;
			(curargs->args).scatter.comm = comm;
			curargs->completecmp = true;
			curargs->completerep = false;
			if(last_collective == NULL) curargs->id = 0;
			else curargs->id = last_collective->id + 1;
			curargs->type = MPI_FT_SCATTER;
			curargs->repreq = &reqinter;
			
			retVal = 0;
			retValinter = 0;
			int flag = 0;
      do
      {
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
				if(retVal != 0) break;
				EMPI_Test(&req,&flag,&status);
			} while(flag == 0);
			if(retVal != 0 || retValinter != 0)
			{
				if(cmpToRepMap[root] == -1 && nR > 0)
				{
					free(tmpscounts);
					free(tmpsdispls);
				}
				free((curargs->args).scatter.recvbuf);
				free((curargs->args).scatter.sendbuf);
				free(curargs);
			}
		} while(retVal != 0 || retValinter != 0);
		
		if(cmpToRepMap[root] == -1 && rank == root && nR > 0)
		{
			free(tmpscounts);
			free(tmpsdispls);
		}
		
		//memcpy((curargs->args).scatter.recvbuf,recvbuf,recvcount*rsize);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		do
    {
			retValinter = 0;
			reqinter =  *(curargs->repreq);
			int flaginter = !(cmpToRepMap[root] == -1 && nR > 0);
      do
      {
				if(cmpToRepMap[root] == -1 && flaginter == 0 && nR > 0) retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);
				if(retValinter != 0) break;
				if(cmpToRepMap[root] == -1 && flaginter == 0 && nR > 0) EMPI_Test(&reqinter,&flaginter,&status);
			} while(flaginter == 0);
		} while(retValinter != 0);
		last_collective->completerep = true;
ConvertToCMPSCATTERSTORED:
    req = EMPI_REQUEST_NULL;
		if(cmpToRepMap[root] != -1 && nR > 0)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			} while(retVal != 0);
		}
  }
  return MPI_SUCCESS;
}

int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm)
{
  int retVal;
	int retValinter;
  EMPI_Request req = EMPI_REQUEST_NULL;
  EMPI_Request reqinter = EMPI_REQUEST_NULL;
  EMPI_Status status;
  int oldsize = 0;
  int targetrep;
  int rank;
	int *tmprcounts;
	int *tmprdispls;
	int *tmprcountsinter;
	int *tmprdisplsinter;
	int flagcount;
	int commnC;
	int commnR;
	int ssize,rsize;
	struct collective_data *curargs;
  if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
  {
		do
    {
      retVal = 0;
			retValinter = 0;
			EMPI_Comm_rank((comm->EMPI_COMM_REP),&rank);
			EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
			EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
			tmprcounts = (int *)malloc(commnR*sizeof(int));
			tmprdispls = (int *)malloc(commnR*sizeof(int));
			if(commnC > commnR && commnR > 0) tmprcountsinter = (int *)malloc((commnC-commnR)*sizeof(int));
			if(commnC > commnR && commnR > 0) tmprdisplsinter = (int *)malloc((commnC-commnR)*sizeof(int));
			if(cmpToRepMap[root] != -1)
			{
				int j = 0;
				int k = 0;
				for(int i = 0; i < commnC; i++)
				{
					if(cmpToRepMap[i] != -1)
					{
						tmprcounts[j] = recvcount;
						tmprdispls[j] = i*recvcount;
						j++;
					}
					else if(commnC > commnR && commnR > 0)
					{
						tmprcountsinter[k] = recvcount;
						tmprdisplsinter[k] = i*recvcount;
						k++;
					}
				}
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
				if(commnC > commnR && commnR > 0) retValinter = check_errhandler_conditions_collective((comm->oworldComm),&req);
				if(retVal == 0 && retValinter == 0)
				{
					retVal = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcounts, tmprdispls, recvtype->edatatype, cmpToRepMap[root], (comm->EMPI_COMM_REP), &req);
					if(rank == cmpToRepMap[root] && commnC > commnR && commnR > 0) retValinter = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcountsinter, tmprdisplsinter, recvtype->edatatype, EMPI_ROOT, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqinter);
					else if(commnC > commnR && commnR > 0) retValinter = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcountsinter, tmprdisplsinter, recvtype->edatatype, EMPI_PROC_NULL, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqinter);
					
					ssize = sendtype->size;
					rsize = recvtype->size;
					curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
					EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnC);
					(curargs->args).gather.sendbuf = malloc(sendcount*ssize);
					memcpy((curargs->args).gather.sendbuf,sendbuf,sendcount*ssize);
					(curargs->args).gather.recvbuf = malloc(commnC*recvcount*rsize);
					(curargs->args).gather.sendcount = sendcount;
					(curargs->args).gather.senddt = sendtype;
					(curargs->args).gather.recvcount = recvcount;
					(curargs->args).gather.recvdt = recvtype;
					(curargs->args).gather.root = root;
					(curargs->args).gather.comm = comm;
					curargs->completecmp = true;
					curargs->completerep = true;
					if(last_collective == NULL) curargs->id = 0;
					else curargs->id = last_collective->id + 1;
					curargs->type = MPI_FT_GATHER;
					curargs->repreq = &reqinter;
					
					int flag = 0;
					int flaginter = !(commnC > commnR && commnR > 0);
					do
					{
						if(flag == 0) retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
						if(retVal != 0) break;
						if(flag == 0) EMPI_Test(&req,&flag,&status);
						if(commnC > commnR && commnR > 0 && flaginter == 0) retValinter = check_errhandler_conditions_collective((comm->oworldComm),&req);
						if(retValinter != 0) break;
						if(commnC > commnR && commnR > 0 && flaginter == 0)EMPI_Test(&reqinter,&flaginter,&status);
					} while(flag == 0 || flaginter == 0);
				}
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					free(tmprcounts);
					free(tmprdispls);
					if(commnC > commnR && commnR > 0) free(tmprcountsinter);
					if(commnC > commnR && commnR > 0) free(tmprdisplsinter);
					goto ConvertToCMPGATHER;
				}
			}
			else
			{
				ssize = sendtype->size;
				rsize = recvtype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnC);
				(curargs->args).gather.sendbuf = malloc(sendcount*ssize);
				memcpy((curargs->args).gather.sendbuf,sendbuf,sendcount*ssize);
				(curargs->args).gather.recvbuf = malloc(commnC*recvcount*rsize);
				(curargs->args).gather.sendcount = sendcount;
				(curargs->args).gather.senddt = sendtype;
				(curargs->args).gather.recvcount = recvcount;
				(curargs->args).gather.recvdt = recvtype;
				(curargs->args).gather.root = root;
				(curargs->args).gather.comm = comm;
				curargs->completecmp = true;
				curargs->completerep = true;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_GATHER;
				curargs->repreq = &reqinter;
			}
			if(retVal != 0 || retValinter != 0)
			{
				free((curargs->args).gather.recvbuf);
				free((curargs->args).gather.sendbuf);
				free(curargs);
			}
    } while(retVal != 0 || retValinter != 0);
		
		free(tmprcounts);
		free(tmprdispls);
		if(commnC > commnR && commnR > 0) free(tmprcountsinter);
		if(commnC > commnR && commnR > 0) free(tmprdisplsinter);
    
		//memcpy((curargs->args).gather.recvbuf,recvbuf,commnC*recvcount*rsize);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		req = EMPI_REQUEST_NULL;
		if(commnR > 0 && commnC == commnR)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					if(req != EMPI_REQUEST_NULL) EMPI_Request_free(&req);
					goto ConvertToCMPGATHERSTORED;
				}
			} while(retVal != 0);
		}
  }
  else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
  {
ConvertToCMPGATHER:
		do
    {
      retVal = 0;
			retValinter = 0;
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
			else commnR = 0;
			if(cmpToRepMap[root] != -1 && cmpToRepMap[rank] == -1)
			{
				tmprcountsinter = (int *)malloc((commnC-commnR)*sizeof(int));
				tmprdisplsinter = (int *)malloc((commnC-commnR)*sizeof(int));
				int j = 0;
				for(int i = 0; i < commnC; i++)
				{
					if(cmpToRepMap[i] == -1)
					{
						tmprcountsinter[j] = recvcount;
						tmprdisplsinter[j] = i*recvcount;
						j++;
					}
				}
			}
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			if(cmpToRepMap[root] != -1 && cmpToRepMap[rank] == -1) retValinter = check_errhandler_conditions_collective((comm->oworldComm),&req);
      if(retVal == 0 && retValinter == 0)
      {
        retVal = EMPI_Igather (sendbuf, sendcount, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, root, (comm->EMPI_COMM_CMP), &req);
				if(cmpToRepMap[root] != -1 && cmpToRepMap[rank] == -1) retValinter = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcountsinter, tmprdisplsinter, recvtype->edatatype, cmpToRepMap[root], (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqinter);
			}
			
			if(retVal != 0 || retValinter != 0)
			{
				if(cmpToRepMap[root] != -1 && cmpToRepMap[rank] == -1)
				{
					free(tmprcountsinter);
					free(tmprdisplsinter);
				}
				continue;
			}
			
			ssize = sendtype->size;
			rsize = recvtype->size;
			curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			(curargs->args).gather.sendbuf = malloc(sendcount*ssize);
			memcpy((curargs->args).gather.sendbuf,sendbuf,sendcount*ssize);
			(curargs->args).gather.recvbuf = malloc(commnC*recvcount*rsize);
			(curargs->args).gather.sendcount = sendcount;
			(curargs->args).gather.senddt = sendtype;
			(curargs->args).gather.recvcount = recvcount;
			(curargs->args).gather.recvdt = recvtype;
			(curargs->args).gather.root = root;
			(curargs->args).gather.comm = comm;
			curargs->completecmp = true;
			curargs->completerep = false;
			if(last_collective == NULL) curargs->id = 0;
			else curargs->id = last_collective->id + 1;
			curargs->type = MPI_FT_GATHER;
			curargs->repreq = &reqinter;
			
			retVal = 0;
			retValinter = 0;
			int flag = 0;
      do
      {
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
				if(retVal != 0) break;
				EMPI_Test(&req,&flag,&status);
			} while(flag == 0);
			
			if(retVal != 0 || retValinter != 0)
			{
				if(cmpToRepMap[root] != -1 && cmpToRepMap[rank] == -1)
				{
					free(tmprcountsinter);
					free(tmprdisplsinter);
				}
				free((curargs->args).gather.recvbuf);
				free((curargs->args).gather.sendbuf);
				free(curargs);
			}
		} while(retVal != 0 || retValinter != 0);
		
		if(cmpToRepMap[root] != -1 && cmpToRepMap[rank] == -1) free(tmprcountsinter);
		if(cmpToRepMap[root] != -1 && cmpToRepMap[rank] == -1) free(tmprdisplsinter);
    
		//memcpy((curargs->args).gather.recvbuf,recvbuf,commnC*recvcount*rsize);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		do
    {
			retValinter = 0;
			reqinter =  *(curargs->repreq);
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			int flaginter = !(cmpToRepMap[root] != -1 && cmpToRepMap[rank] == -1);
      do
      {
				if(flaginter == 0 && cmpToRepMap[root] != -1 && cmpToRepMap[rank] == -1) retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqinter);
				if(retValinter != 0) break;
				if(flaginter == 0 && cmpToRepMap[root] != -1 && cmpToRepMap[rank] == -1) EMPI_Test(&reqinter,&flaginter,&status);
			} while(flaginter == 0);
		} while(retValinter != 0);
		last_collective->completerep = true;
ConvertToCMPGATHERSTORED:
    req = EMPI_REQUEST_NULL;
		if(commnR > 0 && commnC == commnR)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			} while(retVal != 0);
		}
  }
  return MPI_SUCCESS;
}

int MPI_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
	int retVal;
	int retValinter;
	EMPI_Request *reqs;
	bool *completereq;
  EMPI_Request req = EMPI_REQUEST_NULL;
  EMPI_Datatype edatatype;
  EMPI_Status status;
  int oldsize = 0;
  int targetrep;
  int rank;
	int flagcount;
	int commnC;
	int commnR;
	int *tmpscounts;
	int *tmpsdispls;
	int *tmprcounts;
	int *tmprdispls;
	int *tmprcountsinter;
	int *tmprdisplsinter;
	int ssize,rsize;
	struct collective_data *curargs;
  if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
  {
		do
    {
			retVal = 0;
			retValinter = 0;
			EMPI_Comm_rank((comm->EMPI_COMM_REP),&rank);
			EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
			EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
			tmpscounts = (int *)malloc(commnR*sizeof(int));
			tmpsdispls = (int *)malloc(commnR*sizeof(int));
			tmprcounts = (int *)malloc(commnR*sizeof(int));
			tmprdispls = (int *)malloc(commnR*sizeof(int));
			if(commnC > commnR && commnR > 0) tmprcountsinter = (int *)malloc((commnC-commnR)*sizeof(int));
			if(commnC > commnR && commnR > 0) tmprdisplsinter = (int *)malloc((commnC-commnR)*sizeof(int));
			int j = 0;
			int k = 0;
			for(int i = 0; i < commnC; i++)
			{
				if(cmpToRepMap[i] != -1)
				{
					tmpscounts[j] = sendcount;
					tmpsdispls[j] = i*sendcount;
					tmprcounts[j] = recvcount;
					tmprdispls[j] = i*recvcount;
					j++;
				}
				else if(commnC > commnR && commnR > 0)
				{
					tmprcountsinter[k] = recvcount;
					tmprdisplsinter[k] = i*recvcount;
					k++;
				}
			}
			if(commnC > commnR && commnR > 0)
			{
				reqs = (EMPI_Request *)malloc(commnR*sizeof(EMPI_Request));
				completereq = (bool *)malloc(commnR*sizeof(bool));
				for(int i = 0; i < commnR; i++)
				{
					completereq[i] = false;
					retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
					if(retValinter == 0)
					{
						if(rank == i) retValinter = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcountsinter, tmprdisplsinter, recvtype->edatatype, EMPI_ROOT, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
						else retValinter = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcountsinter, tmprdisplsinter, recvtype->edatatype, EMPI_PROC_NULL, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
					}
					if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
					{
						for(int j = 0; j <= i; j++)
						{
							if(reqs[j] != EMPI_REQUEST_NULL) EMPI_Request_free(&reqs[j]);
						}
						if(commnC > commnR && commnR > 0) free(reqs);
						if(commnC > commnR && commnR > 0) free(completereq);
						free(tmpscounts);
						free(tmpsdispls);
						free(tmprcounts);
						free(tmprdispls);
						if(commnC > commnR && commnR > 0) free(tmprcountsinter);
						if(commnC > commnR && commnR > 0) free(tmprdisplsinter);
						goto ConvertToCMPALLGATHER;
					}
					if(retValinter != 0) break;
				}
			}
			if(retValinter != 0)
			{
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				free(tmpscounts);
				free(tmpsdispls);
				free(tmprcounts);
				free(tmprdispls);
				if(commnC > commnR && commnR > 0) free(tmprcountsinter);
				if(commnC > commnR && commnR > 0) free(tmprdisplsinter);
				continue;
			}

      retVal = 0;
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnC);
			EMPI_Comm_size(comm->EMPI_COMM_REP,&commnR);
      if(retVal == 0)
      {
        retVal = EMPI_Iallgatherv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, tmprcounts, tmprdispls, recvtype->edatatype, (comm->EMPI_COMM_REP), &req);
				
				ssize = sendtype->size;
				rsize = recvtype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnC);
				(curargs->args).allgather.sendbuf = malloc(sendcount*ssize);
				memcpy((curargs->args).allgather.sendbuf,sendbuf,sendcount*ssize);
				(curargs->args).allgather.recvbuf = malloc(commnC*recvcount*rsize);
				(curargs->args).allgather.sendcount = sendcount;
				(curargs->args).allgather.senddt = sendtype;
				(curargs->args).allgather.recvcount = recvcount;
				(curargs->args).allgather.recvdt = recvtype;
				(curargs->args).allgather.comm = comm;
				curargs->completecmp = true;
				curargs->completerep = true;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_ALLGATHER;
				curargs->repreq = reqs;
				
        int flag = 0;
        do
        {
          retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
          EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				for(int j = 0; j < commnR; j++)
				{
					if(reqs[j] != EMPI_REQUEST_NULL && !completereq[j]) EMPI_Request_free(&reqs[j]);
				}
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				free(tmpscounts);
				free(tmpsdispls);
				free(tmprcounts);
				free(tmprdispls);
				if(commnC > commnR && commnR > 0) free(tmprcountsinter);
				if(commnC > commnR && commnR > 0) free(tmprdisplsinter);
				free((curargs->args).allgather.recvbuf);
				free((curargs->args).allgather.sendbuf);
				free(curargs);
				goto ConvertToCMPALLGATHER;
			}
			if(retVal != 0)
			{
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				free(tmpscounts);
				free(tmpsdispls);
				free(tmprcounts);
				free(tmprdispls);
				if(commnC > commnR && commnR > 0) free(tmprcountsinter);
				if(commnC > commnR && commnR > 0) free(tmprdisplsinter);
				free((curargs->args).allgather.recvbuf);
				free((curargs->args).allgather.sendbuf);
				free(curargs);
				continue;
			}
			
			retValinter = 0;
			if(commnC > commnR && commnR > 0) flagcount = 0;
			else flagcount = commnR;
			int flaginter = 0;
			do
			{
				flaginter = 0;
				if(commnC > commnR && commnR > 0)
				{
					for(int i = 0; i < commnR; i++)
					{
						if(!completereq[i])
						{
							flaginter = 0;
							retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
							if(retValinter != 0) break;
							EMPI_Test(&reqs[i],&flaginter,&status);
							flagcount += flaginter;
							if(flaginter > 0) completereq[i] = true;
						}
					}
					if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
					{
						for(int j = 0; j < commnR; j++)
						{
							if(reqs[j] != EMPI_REQUEST_NULL && !completereq[j]) EMPI_Request_free(&reqs[j]);
						}
						if(commnC > commnR && commnR > 0) free(reqs);
						if(commnC > commnR && commnR > 0) free(completereq);
						free(tmpscounts);
						free(tmpsdispls);
						free(tmprcounts);
						free(tmprdispls);
						if(commnC > commnR && commnR > 0) free(tmprcountsinter);
						if(commnC > commnR && commnR > 0) free(tmprdisplsinter);
						free((curargs->args).allgather.recvbuf);
						free((curargs->args).allgather.sendbuf);
						free(curargs);
						goto ConvertToCMPALLGATHER;
					}
				}
				if(retValinter != 0) break;
			} while(flagcount < commnR);
			if(retValinter != 0)
			{
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				free(tmpscounts);
				free(tmpsdispls);
				free(tmprcounts);
				free(tmprdispls);
				if(commnC > commnR && commnR > 0) free(tmprcountsinter);
				if(commnC > commnR && commnR > 0) free(tmprdisplsinter);
				free((curargs->args).allgather.recvbuf);
				free((curargs->args).allgather.sendbuf);
				free(curargs);
			}
    } while(retVal != 0 || retValinter != 0);
    
		//memcpy((curargs->args).allgather.recvbuf,recvbuf,commnC*recvcount*rsize);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		if(commnC > commnR && commnR > 0) free(reqs);
		if(commnC > commnR && commnR > 0) free(completereq);
		free(tmpscounts);
		free(tmpsdispls);
		free(tmprcounts);
		free(tmprdispls);
		if(commnC > commnR && commnR > 0) free(tmprcountsinter);
		if(commnC > commnR && commnR > 0) free(tmprdisplsinter);
		req = EMPI_REQUEST_NULL;
		if(commnR > 0 && commnC == commnR)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					if(req != EMPI_REQUEST_NULL) EMPI_Request_free(&req);
					goto ConvertToCMPALLGATHERSTORED;
				}
			} while(retVal != 0);
		}
  }
  else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
  {
ConvertToCMPALLGATHER:
		do
    {
      retVal = 0;
			retValinter = 0;
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
			else commnR = 0;
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			if(commnR > 0 && cmpToRepMap[rank] == -1)
			{
				tmprcountsinter = (int *)malloc((commnC-commnR)*sizeof(int));
				tmprdisplsinter = (int *)malloc((commnC-commnR)*sizeof(int));
				int j = 0;
				for(int i = 0; i < commnC; i++)
				{
					if(cmpToRepMap[i] == -1)
					{
						tmprcountsinter[j] = recvcount;
						tmprdisplsinter[j] = i*recvcount;
						j++;
					}
				}
				reqs = (EMPI_Request *)malloc(commnR*sizeof(EMPI_Request));
				completereq = (bool *)malloc(commnR*sizeof(bool));
				for(int i = 0; i < commnR; i++)
				{
					completereq[i] = false;
					retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
					if(retValinter == 0)
					{
						retValinter = EMPI_Igatherv (sendbuf, sendcount, sendtype->edatatype, recvbuf, tmprcountsinter, tmprdisplsinter, recvtype->edatatype, i, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
					}
					if(retValinter != 0) break;
				}
			}
			if(retValinter != 0)
			{
				if(commnR > 0 && cmpToRepMap[rank] == -1)
				{
					free(reqs);
					free(completereq);
					free(tmprcountsinter);
					free(tmprdisplsinter);
				}
				continue;
			}
			
      retVal = 0;
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);

      if(retVal == 0)
      {
        retVal = EMPI_Iallgather (sendbuf, sendcount, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, (comm->EMPI_COMM_CMP), &req);
				
				ssize = sendtype->size;
				rsize = recvtype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
				(curargs->args).allgather.sendbuf = malloc(sendcount*ssize);
				memcpy((curargs->args).allgather.sendbuf,sendbuf,sendcount*ssize);
				(curargs->args).allgather.recvbuf = malloc(commnC*recvcount*rsize);
				(curargs->args).allgather.sendcount = sendcount;
				(curargs->args).allgather.senddt = sendtype;
				(curargs->args).allgather.recvcount = recvcount;
				(curargs->args).allgather.recvdt = recvtype;
				(curargs->args).allgather.comm = comm;
				curargs->completecmp = true;
				curargs->completerep = false;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_ALLGATHER;
				curargs->repreq = reqs;
		
        int flag = 0;
        do
        {
          retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
          EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
			if(retVal != 0)
			{
				if(commnR > 0 && cmpToRepMap[rank] == -1)
				{
					free(reqs);
					free(completereq);
					free(tmprcountsinter);
					free(tmprdisplsinter);
				}
				free((curargs->args).allgather.recvbuf);
				free((curargs->args).allgather.sendbuf);
				free(curargs);
			}
    } while(retVal != 0 || retValinter != 0);
		
		//memcpy((curargs->args).allgather.recvbuf,recvbuf,commnC*recvcount*rsize);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		do
    {
			retValinter = 0;
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
			else commnR = 0;
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			if(commnR > 0 && cmpToRepMap[rank] == -1) flagcount = 0;
			else flagcount = commnR;
			reqs =  (curargs->repreq);
			int flaginter = 0;
			do
			{
				flaginter = 0;
				if(commnR > 0 && cmpToRepMap[rank] == -1)
				{
					for(int i = 0; i < commnR; i++)
					{
						if(!completereq[i])
						{
							flaginter = 0;
							retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
							if(retValinter != 0) break;
							EMPI_Test(&reqs[i],&flaginter,&status);
							flagcount += flaginter;
							if(flaginter > 0) completereq[i] = true;
						}
					}
				}
				if(retValinter != 0) break;
			} while(flagcount < commnR);
		} while(retValinter != 0);
		last_collective->completerep = true;
		if(commnR > 0 && cmpToRepMap[rank] == -1)
		{
			free(reqs);
			free(completereq);
			free(tmprcountsinter);
			free(tmprdisplsinter);
		}
ConvertToCMPALLGATHERSTORED:
		req = EMPI_REQUEST_NULL;
		if(commnR > 0 && commnC == commnR)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			} while(retVal != 0);
		}
  }
  return MPI_SUCCESS;
}

/* MPI_Alltoall */
int MPI_Alltoall (void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
	int retVal;
	int retValinter;
	EMPI_Request *reqs;
	bool *completereq;
  EMPI_Request req = EMPI_REQUEST_NULL;
  EMPI_Datatype edatatype;
  EMPI_Status status;
  int oldsize = 0;
  int targetrep;
  int rank;
	int flagcount;
	int noreprank;
	int commnC;
	int commnR;
	int *tmpscounts;
	int *tmpsdispls;
	int *tmprcounts;
	int *tmprdispls;
	int *tmpscountsinter;
	int *tmpsdisplsinter;
	int *intermapping;
	int ssize,rsize;
	struct collective_data *curargs;
  if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
  {
		do
    {
			retVal = 0;
			retValinter = 0;
			EMPI_Comm_rank((comm->EMPI_COMM_REP),&rank);
			EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
			EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
			tmpscounts = (int *)malloc(commnR*sizeof(int));
			tmpsdispls = (int *)malloc(commnR*sizeof(int));
			tmprcounts = (int *)malloc(commnR*sizeof(int));
			tmprdispls = (int *)malloc(commnR*sizeof(int));
			if(commnC > commnR && commnR > 0) tmpscountsinter = (int *)malloc((commnR)*sizeof(int));
			if(commnC > commnR && commnR > 0) tmpsdisplsinter = (int *)malloc((commnR)*sizeof(int));
			if(commnC > commnR && commnR > 0) intermapping = (int *)malloc((commnC-commnR)*sizeof(int));
			int j = 0;
			int k = 0;
			for(int i = 0; i < commnC; i++)
			{
				if(cmpToRepMap[i] != -1)
				{
					tmpscounts[j] = sendcount;
					tmpsdispls[j] = i*sendcount;
					tmprcounts[j] = recvcount;
					tmprdispls[j] = i*recvcount;
					if(commnC > commnR && commnR > 0)
					{
						tmpscountsinter[j] = sendcount;
						tmpsdisplsinter[j] = i*sendcount;
					}
					j++;
				}
				else if(commnC > commnR && commnR > 0)
				{
					intermapping[k] = i;
					k++;
				}
			}
			if(commnC > commnR && commnR > 0)
			{
				reqs = (EMPI_Request *)malloc((commnC-commnR)*sizeof(EMPI_Request));
				completereq = (bool *)malloc((commnC-commnR)*sizeof(bool));
				for(int i = 0; i < commnC-commnR; i++)
				{
					completereq[i] = false;
					retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
					if(retValinter == 0)
					{
						retValinter = EMPI_Iscatterv (sendbuf, tmpscountsinter, tmpsdisplsinter, sendtype->edatatype, recvbuf + intermapping[i]*recvcount*recvtype->size, recvcount, recvtype->edatatype, i, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
					}
					if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
					{
						for(int j = 0; j <= i; j++)
						{
							if(reqs[j] != EMPI_REQUEST_NULL) EMPI_Request_free(&reqs[j]);
						}
						if(commnC > commnR && commnR > 0) free(reqs);
						if(commnC > commnR && commnR > 0) free(completereq);
						free(tmpscounts);
						free(tmpsdispls);
						free(tmprcounts);
						free(tmprdispls);
						if(commnC > commnR && commnR > 0) free(tmpscountsinter);
						if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
						if(commnC > commnR && commnR > 0) free(intermapping);
						goto ConvertToCMPALLTOALL;
					}
					if(retValinter != 0) break;
				}
			}
			if(retValinter != 0)
			{
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				free(tmpscounts);
				free(tmpsdispls);
				free(tmprcounts);
				free(tmprdispls);
				if(commnC > commnR && commnR > 0) free(tmpscountsinter);
				if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
				if(commnC > commnR && commnR > 0) free(intermapping);
				continue;
			}

      retVal = 0;
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnC);
			EMPI_Comm_size(comm->EMPI_COMM_REP,&commnR);
      if(retVal == 0)
      {
        retVal = EMPI_Ialltoallv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, tmprcounts, tmprdispls, recvtype->edatatype, (comm->EMPI_COMM_REP), &req);
				
				ssize = sendtype->size;
				rsize = recvtype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnC);
				(curargs->args).alltoall.sendbuf = malloc(commnC*sendcount*ssize);
				memcpy((curargs->args).alltoall.sendbuf,sendbuf,commnC*sendcount*ssize);
				(curargs->args).alltoall.recvbuf = malloc(commnC*recvcount*rsize);
				(curargs->args).alltoall.sendcount = sendcount;
				(curargs->args).alltoall.senddt = sendtype;
				(curargs->args).alltoall.recvcount = recvcount;
				(curargs->args).alltoall.recvdt = recvtype;
				(curargs->args).alltoall.comm = comm;
				curargs->completecmp = true;
				curargs->completerep = true;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_ALLTOALL;
				curargs->repreq = reqs;
				
				int flag = 0;
        do
        {
          retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
          EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				for(int j = 0; j < commnC-commnR; j++)
				{
					if(reqs[j] != EMPI_REQUEST_NULL && !completereq[j]) EMPI_Request_free(&reqs[j]);
				}
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				free(tmpscounts);
				free(tmpsdispls);
				free(tmprcounts);
				free(tmprdispls);
				if(commnC > commnR && commnR > 0) free(tmpscountsinter);
				if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
				if(commnC > commnR && commnR > 0) free(intermapping);
				free((curargs->args).alltoall.recvbuf);
				free((curargs->args).alltoall.sendbuf);
				free(curargs);
				goto ConvertToCMPALLTOALL;
			}
			if(retVal != 0)
			{
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				free(tmpscounts);
				free(tmpsdispls);
				free(tmprcounts);
				free(tmprdispls);
				if(commnC > commnR && commnR > 0) free(tmpscountsinter);
				if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
				if(commnC > commnR && commnR > 0) free(intermapping);
				free((curargs->args).alltoall.recvbuf);
				free((curargs->args).alltoall.sendbuf);
				free(curargs);
				continue;
			}
			
			retValinter = 0;
			if(commnC > commnR && commnR > 0) flagcount = 0;
			else flagcount = commnC-commnR;
			int flaginter = 0;
			do
			{
				flaginter = 0;
				if(commnC > commnR && commnR > 0)
				{
					for(int i = 0; i < commnC-commnR; i++)
					{
						if(!completereq[i])
						{
							flaginter = 0;
							retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
							if(retValinter != 0) break;
							EMPI_Test(&reqs[i],&flaginter,&status);
							flagcount += flaginter;
							if(flaginter > 0) completereq[i] = true;
						}
					}
					if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
					{
						for(int j = 0; j < commnC-commnR; j++)
						{
							if(reqs[j] != EMPI_REQUEST_NULL && !completereq[j]) EMPI_Request_free(&reqs[j]);
						}
						if(commnC > commnR && commnR > 0) free(reqs);
						if(commnC > commnR && commnR > 0) free(completereq);
						free(tmpscounts);
						free(tmpsdispls);
						free(tmprcounts);
						free(tmprdispls);
						if(commnC > commnR && commnR > 0) free(tmpscountsinter);
						if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
						if(commnC > commnR && commnR > 0) free(intermapping);
						free((curargs->args).alltoall.recvbuf);
						free((curargs->args).alltoall.sendbuf);
						free(curargs);
						goto ConvertToCMPALLTOALL;
					}
				}
				if(retValinter != 0) break;
			} while(flagcount < commnC-commnR);
			if(retVal != 0)
			{
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				free(tmpscounts);
				free(tmpsdispls);
				free(tmprcounts);
				free(tmprdispls);
				if(commnC > commnR && commnR > 0) free(tmpscountsinter);
				if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
				if(commnC > commnR && commnR > 0) free(intermapping);
				free((curargs->args).alltoall.recvbuf);
				free((curargs->args).alltoall.sendbuf);
				free(curargs);
			}
    } while(retVal != 0 || retValinter != 0);
		
		//memcpy((curargs->args).alltoall.recvbuf,recvbuf,commnC*recvcount*rsize);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		if(commnC > commnR && commnR > 0) free(reqs);
		if(commnC > commnR && commnR > 0) free(completereq);
		free(tmpscounts);
		free(tmpsdispls);
		free(tmprcounts);
		free(tmprdispls);
		if(commnC > commnR && commnR > 0) free(tmpscountsinter);
		if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
		if(commnC > commnR && commnR > 0) free(intermapping);
		req = EMPI_REQUEST_NULL;
		if(commnR > 0 && commnC == commnR)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					if(req != EMPI_REQUEST_NULL) EMPI_Request_free(&req);
					goto ConvertToCMPALLTOALLSTORED;
				}
			} while(retVal != 0);
		}
  }
  else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
  {
ConvertToCMPALLTOALL:
		do
    {
      retVal = 0;
			retValinter = 0;
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
			else commnR = 0;
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			if(commnR > 0 && cmpToRepMap[rank] == -1)
			{
				tmpscountsinter = (int *)malloc((commnR)*sizeof(int));
				tmpsdisplsinter = (int *)malloc((commnR)*sizeof(int));
				intermapping = (int *)malloc((commnC-commnR)*sizeof(int));
				int j = 0;
				int k = 0;
				for(int i = 0; i < commnC; i++)
				{
					if(cmpToRepMap[i] != -1)
					{
						tmpscountsinter[j] = sendcount;
						tmpsdisplsinter[j] = i*sendcount;
						j++;
					}
					else
					{
						intermapping[k] = i;
						k++;
					}	
				}
				reqs = (EMPI_Request *)malloc((commnC-commnR)*sizeof(EMPI_Request));
				completereq = (bool *)malloc((commnC-commnR)*sizeof(bool));
				EMPI_Comm_rank(comm->EMPI_CMP_NO_REP,&noreprank);
				for(int i = 0; i < commnC-commnR; i++)
				{
					completereq[i] = false;
					retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
					if(retValinter == 0)
					{
						if(i == noreprank) retValinter = EMPI_Iscatterv (sendbuf, tmpscountsinter, tmpsdisplsinter, sendtype->edatatype, recvbuf + intermapping[i]*recvcount*recvtype->size, recvcount, recvtype->edatatype, EMPI_ROOT, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
						else retValinter = EMPI_Iscatterv (sendbuf, tmpscountsinter, tmpsdisplsinter, sendtype->edatatype, recvbuf + intermapping[i]*recvcount*recvtype->size, recvcount, recvtype->edatatype, EMPI_PROC_NULL, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
					}
					if(retValinter != 0) break;
				}
			}
			if(retValinter != 0)
			{
				if(commnR > 0 && cmpToRepMap[rank] == -1)
				{
					free(reqs);
					free(completereq);
					free(tmpscountsinter);
					free(tmpsdisplsinter);
					free(intermapping);
				}
				continue;
			}
			
      retVal = 0;
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);

      if(retVal == 0)
      {
        retVal = EMPI_Ialltoall (sendbuf, sendcount, sendtype->edatatype, recvbuf, recvcount, recvtype->edatatype, (comm->EMPI_COMM_CMP), &req);
        
				ssize = sendtype->size;
				rsize = recvtype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
				(curargs->args).alltoall.sendbuf = malloc(commnC*sendcount*ssize);
				memcpy((curargs->args).alltoall.sendbuf,sendbuf,commnC*sendcount*ssize);
				(curargs->args).alltoall.recvbuf = malloc(commnC*recvcount*rsize);
				(curargs->args).alltoall.sendcount = sendcount;
				(curargs->args).alltoall.senddt = sendtype;
				(curargs->args).alltoall.recvcount = recvcount;
				(curargs->args).alltoall.recvdt = recvtype;
				(curargs->args).alltoall.comm = comm;
				curargs->completecmp = true;
				curargs->completerep = false;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_ALLTOALL;
				curargs->repreq = reqs;
				
				int flag = 0;
        do
        {
          retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
          EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
			if(retVal != 0)
			{
				free((curargs->args).alltoall.recvbuf);
				free((curargs->args).alltoall.sendbuf);
				free(curargs);
			}
    } while(retVal != 0 || retValinter != 0);
		
		//memcpy((curargs->args).alltoall.recvbuf,recvbuf,commnC*recvcount*rsize);
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		do
    {
			retValinter = 0;
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
			else commnR = 0;
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			if(commnR > 0 && cmpToRepMap[rank] == -1) flagcount = 0;
			else flagcount = commnC-commnR;
			reqs =  (curargs->repreq);
			int flaginter = 0;
			do
			{
				flaginter = 0;
				if(commnR > 0 && cmpToRepMap[rank] == -1)
				{
					for(int i = 0; i < commnC-commnR; i++)
					{
						if(!completereq[i])
						{
							flaginter = 0;
							retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
							if(retValinter != 0) break;
							EMPI_Test(&reqs[i],&flaginter,&status);
							flagcount += flaginter;
							if(flaginter > 0) completereq[i] = true;
						}
					}
				}
				if(retValinter != 0) break;
			} while(flagcount < commnC-commnR);
		} while(retValinter != 0);
		last_collective->completerep = true;
		if(commnR > 0 && cmpToRepMap[rank] == -1)
		{
			free(reqs);
			free(completereq);
			free(tmpscountsinter);
			free(tmpsdisplsinter);
			free(intermapping);
		}
ConvertToCMPALLTOALLSTORED:
		req = EMPI_REQUEST_NULL;
		if(commnR > 0 && commnC == commnR)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			} while(retVal != 0);
		}
  }
  return MPI_SUCCESS;
}
/* MPI_Alltoallv */
int MPI_Alltoallv(void *sendbuf, int *sendcounts, int *sdispls, MPI_Datatype sendtype, void *recvbuf, int *recvcounts, int *rdispls, MPI_Datatype recvtype, MPI_Comm comm)
{
  int retVal;
	int retValinter;
	EMPI_Request *reqs;
	bool *completereq;
  EMPI_Request req = EMPI_REQUEST_NULL;
  EMPI_Datatype edatatype;
  EMPI_Status status;
  int oldsize = 0;
  int targetrep;
  int rank;
	int flagcount;
	int noreprank;
	int commnC;
	int commnR;
	int *tmpscounts;
	int *tmpsdispls;
	int *tmprcounts;
	int *tmprdispls;
	int *tmpscountsinter;
	int *tmpsdisplsinter;
	int *intermapping;
	long scount,rcount;
	int ssize,rsize;
	struct collective_data *curargs;
  if((comm->EMPI_COMM_REP) != EMPI_COMM_NULL)
  {
		do
    {
			retVal = 0;
			retValinter = 0;
			EMPI_Comm_rank((comm->EMPI_COMM_REP),&rank);
			EMPI_Comm_size((comm->EMPI_COMM_REP),&commnR);
			EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnC);
			tmpscounts = (int *)malloc(commnR*sizeof(int));
			tmpsdispls = (int *)malloc(commnR*sizeof(int));
			tmprcounts = (int *)malloc(commnR*sizeof(int));
			tmprdispls = (int *)malloc(commnR*sizeof(int));
			if(commnC > commnR && commnR > 0) tmpscountsinter = (int *)malloc((commnR)*sizeof(int));
			if(commnC > commnR && commnR > 0) tmpsdisplsinter = (int *)malloc((commnR)*sizeof(int));
			if(commnC > commnR && commnR > 0) intermapping = (int *)malloc((commnC-commnR)*sizeof(int));
			int j = 0;
			int k = 0;
			for(int i = 0; i < commnC; i++)
			{
				if(cmpToRepMap[i] != -1)
				{
					tmpscounts[j] = sendcounts[i];
					tmpsdispls[j] = sdispls[i];
					tmprcounts[j] = recvcounts[i];
					tmprdispls[j] = rdispls[i];
					if(commnC > commnR && commnR > 0)
					{
						tmpscountsinter[j] = sendcounts[i];
						tmpsdisplsinter[j] = sdispls[i];
					}
					j++;
				}
				else if(commnC > commnR && commnR > 0)
				{
					intermapping[k] = i;
					k++;
				}
			}
			if(commnC > commnR && commnR > 0)
			{
				reqs = (EMPI_Request *)malloc((commnC-commnR)*sizeof(EMPI_Request));
				completereq = (bool *)malloc((commnC-commnR)*sizeof(bool));
				for(int i = 0; i < commnC-commnR; i++)
				{
					completereq[i] = false;
					retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
					if(retValinter == 0)
					{
						retValinter = EMPI_Iscatterv (sendbuf, tmpscountsinter, tmpsdisplsinter, sendtype->edatatype, recvbuf + (rdispls[intermapping[i]]*recvtype->size), recvcounts[intermapping[i]], recvtype->edatatype, i, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
					}
					if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
					{
						//printf("Rep to cmp conversion iscatter\n");
						for(int j = 0; j <= i; j++)
						{
							if(reqs[j] != EMPI_REQUEST_NULL) EMPI_Request_free(&reqs[j]);
						}
						if(commnC > commnR && commnR > 0) free(reqs);
						if(commnC > commnR && commnR > 0) free(completereq);
						free(tmpscounts);
						free(tmpsdispls);
						free(tmprcounts);
						free(tmprdispls);
						if(commnC > commnR && commnR > 0) free(tmpscountsinter);
						if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
						if(commnC > commnR && commnR > 0) free(intermapping);
						goto ConvertToCMPALLTOALLV;
					}
					if(retValinter != 0) break;
				}
			}
			if(retValinter != 0)
			{
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				free(tmpscounts);
				free(tmpsdispls);
				free(tmprcounts);
				free(tmprdispls);
				if(commnC > commnR && commnR > 0) free(tmpscountsinter);
				if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
				if(commnC > commnR && commnR > 0) free(intermapping);
				continue;
			}

      retVal = 0;
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnC);
			EMPI_Comm_size(comm->EMPI_COMM_REP,&commnR);
      if(retVal == 0)
      {
        retVal = EMPI_Ialltoallv (sendbuf, tmpscounts, tmpsdispls, sendtype->edatatype, recvbuf, tmprcounts, tmprdispls, recvtype->edatatype, (comm->EMPI_COMM_REP), &req);
				ssize = sendtype->size;
				rsize = recvtype->size;
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
				EMPI_Comm_remote_size(comm->EMPI_CMP_REP_INTERCOMM,&commnC);
				EMPI_Comm_size(comm->EMPI_COMM_REP,&commnR);
				rcount = array_sum(recvcounts,commnC);
				scount = array_sum(sendcounts,commnC);
				(curargs->args).alltoallv.sendbuf = malloc(scount*ssize);
				long logdispl = 0;
				//memcpy((curargs->args).alltoallv.sendbuf, sendbuf + (sdispls[0]*ssize),(sendcounts[0]*ssize));
				for(int i = 0; i < commnC; i++)
				{
					memcpy((curargs->args).alltoallv.sendbuf + logdispl,sendbuf + (sdispls[i]*ssize),(sendcounts[i]*ssize));
					logdispl += (sendcounts[i]*ssize);
				}
				(curargs->args).alltoallv.recvbuf = malloc(rcount*rsize);
				(curargs->args).alltoallv.sendcounts = (int *)malloc(commnC*sizeof(int));
				memcpy((curargs->args).alltoallv.sendcounts,sendcounts,commnC*sizeof(int));
				(curargs->args).alltoallv.sdispls = (int *)malloc(commnC*sizeof(int));
				memcpy((curargs->args).alltoallv.sdispls,sdispls,commnC*sizeof(int));
				(curargs->args).alltoallv.senddt = sendtype;
				(curargs->args).alltoallv.recvcounts = (int *)malloc(commnC*sizeof(int));
				memcpy((curargs->args).alltoallv.recvcounts,recvcounts,commnC*sizeof(int));
				(curargs->args).alltoallv.rdispls = (int *)malloc(commnC*sizeof(int));
				memcpy((curargs->args).alltoallv.rdispls,rdispls,commnC*sizeof(int));
				(curargs->args).alltoallv.recvdt = recvtype;
				(curargs->args).alltoallv.comm = comm;
				curargs->completecmp = true;
				curargs->completerep = true;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_ALLTOALLV;
				curargs->repreq = reqs;
        int flag = 0;
        do
        {
          retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
          EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
      }
			if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
			{
				//printf("Rep to cmp conversion ialltoallv\n");
				for(int j = 0; j < commnC-commnR; j++)
				{
					if(reqs[j] != EMPI_REQUEST_NULL && !completereq[j]) EMPI_Request_free(&reqs[j]);
				}
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				free(tmpscounts);
				free(tmpsdispls);
				free(tmprcounts);
				free(tmprdispls);
				if(commnC > commnR && commnR > 0) free(tmpscountsinter);
				if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
				if(commnC > commnR && commnR > 0) free(intermapping);
				free((curargs->args).alltoallv.rdispls);
				free((curargs->args).alltoallv.recvcounts);
				free((curargs->args).alltoallv.sdispls);
				free((curargs->args).alltoallv.sendcounts);
				free((curargs->args).alltoallv.recvbuf);
				free((curargs->args).alltoallv.sendbuf);
				free(curargs);
				goto ConvertToCMPALLTOALLV;
			}
			if(retVal != 0)
			{
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				free(tmpscounts);
				free(tmpsdispls);
				free(tmprcounts);
				free(tmprdispls);
				if(commnC > commnR && commnR > 0) free(tmpscountsinter);
				if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
				if(commnC > commnR && commnR > 0) free(intermapping);
				free((curargs->args).alltoallv.rdispls);
				free((curargs->args).alltoallv.recvcounts);
				free((curargs->args).alltoallv.sdispls);
				free((curargs->args).alltoallv.sendcounts);
				free((curargs->args).alltoallv.recvbuf);
				free((curargs->args).alltoallv.sendbuf);
				free(curargs);
				continue;
			}
			
			retValinter = 0;
			if(commnC > commnR && commnR > 0) flagcount = 0;
			else flagcount = commnC-commnR;
			int flaginter = 0;
			do
			{
				flaginter = 0;
				if(commnC > commnR && commnR > 0)
				{
					for(int i = 0; i < commnC-commnR; i++)
					{
						if(!completereq[i])
						{
							flaginter = 0;
							retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
							if(retValinter != 0) break;
							EMPI_Test(&reqs[i],&flaginter,&status);
							flagcount += flaginter;
							if(flaginter > 0) completereq[i] = true;
						}
					}
					if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
					{
						//printf("Rep to cmp conversion iscatter test\n");
						for(int j = 0; j < commnC-commnR; j++)
						{
							if(reqs[j] != EMPI_REQUEST_NULL && !completereq[j]) EMPI_Request_free(&reqs[j]);
						}
						if(commnC > commnR && commnR > 0) free(reqs);
						if(commnC > commnR && commnR > 0) free(completereq);
						free(tmpscounts);
						free(tmpsdispls);
						free(tmprcounts);
						free(tmprdispls);
						if(commnC > commnR && commnR > 0) free(tmpscountsinter);
						if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
						if(commnC > commnR && commnR > 0) free(intermapping);
						free((curargs->args).alltoallv.rdispls);
						free((curargs->args).alltoallv.recvcounts);
						free((curargs->args).alltoallv.sdispls);
						free((curargs->args).alltoallv.sendcounts);
						free((curargs->args).alltoallv.recvbuf);
						free((curargs->args).alltoallv.sendbuf);
						free(curargs);
						goto ConvertToCMPALLTOALLV;
					}
				}
				if(retValinter != 0) break;
			} while(flagcount < commnC-commnR);
			if(retValinter != 0)
			{
				if(commnC > commnR && commnR > 0) free(reqs);
				if(commnC > commnR && commnR > 0) free(completereq);
				free(tmpscounts);
				free(tmpsdispls);
				free(tmprcounts);
				free(tmprdispls);
				if(commnC > commnR && commnR > 0) free(tmpscountsinter);
				if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
				if(commnC > commnR && commnR > 0) free(intermapping);
				free((curargs->args).alltoallv.rdispls);
				free((curargs->args).alltoallv.recvcounts);
				free((curargs->args).alltoallv.sdispls);
				free((curargs->args).alltoallv.sendcounts);
				free((curargs->args).alltoallv.recvbuf);
				free((curargs->args).alltoallv.sendbuf);
				free(curargs);
			}
    } while(retVal != 0 || retValinter != 0);
		//memcpy((curargs->args).alltoallv.recvbuf, recvbuf + (rdispls[0]*rsize),(recvcounts[0]*rsize));
		//for(int i = 1; i < commnC; i++)
		//{
		//	memcpy((curargs->args).alltoallv.recvbuf + (recvcounts[i-1]*rsize),recvbuf + (rdispls[i]*rsize),(recvcounts[i]*rsize));
		//}
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		if(commnC > commnR && commnR > 0) free(reqs);
		if(commnC > commnR && commnR > 0) free(completereq);
		free(tmpscounts);
		free(tmpsdispls);
		free(tmprcounts);
		free(tmprdispls);
		if(commnC > commnR && commnR > 0) free(tmpscountsinter);
		if(commnC > commnR && commnR > 0) free(tmpsdisplsinter);
		if(commnC > commnR && commnR > 0) free(intermapping);
		req = EMPI_REQUEST_NULL;
		if(commnR > 0 && commnC == commnR)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
				if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL)
				{
					if(req != EMPI_REQUEST_NULL) EMPI_Request_free(&req);
					goto ConvertToCMPALLTOALLVSTORED;
				}
			} while(retVal != 0);
		}
  }
  else if((comm->EMPI_COMM_CMP)!= EMPI_COMM_NULL)
  {
ConvertToCMPALLTOALLV:
		do
    {
      retVal = 0;
			retValinter = 0;
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
			else commnR = 0;
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			if(commnR > 0 && cmpToRepMap[rank] == -1)
			{
				tmpscountsinter = (int *)malloc((commnR)*sizeof(int));
				tmpsdisplsinter = (int *)malloc((commnR)*sizeof(int));
				intermapping = (int *)malloc((commnC-commnR)*sizeof(int));
				int j = 0;
				int k = 0;
				for(int i = 0; i < commnC; i++)
				{
					if(cmpToRepMap[i] != -1)
					{
						tmpscountsinter[j] = sendcounts[i];
						tmpsdisplsinter[j] = sdispls[i];
						j++;
					}
					else
					{
						intermapping[k] = i;
						k++;
					}
				}
				reqs = (EMPI_Request *)malloc((commnC-commnR)*sizeof(EMPI_Request));
				completereq = (bool *)malloc((commnC-commnR)*sizeof(bool));
				EMPI_Comm_rank(comm->EMPI_CMP_NO_REP,&noreprank);
				for(int i = 0; i < commnC-commnR; i++)
				{
					completereq[i] = false;
					retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
					if(retValinter == 0)
					{
						if(i == noreprank) retValinter = EMPI_Iscatterv (sendbuf, tmpscountsinter, tmpsdisplsinter, sendtype->edatatype, recvbuf + (rdispls[intermapping[i]]*recvtype->size), recvcounts[intermapping[i]], recvtype->edatatype, EMPI_ROOT, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
						else retValinter = EMPI_Iscatterv (sendbuf, tmpscountsinter, tmpsdisplsinter, sendtype->edatatype, recvbuf + (rdispls[intermapping[i]]*recvtype->size), recvcounts[intermapping[i]], recvtype->edatatype, EMPI_PROC_NULL, (comm->EMPI_CMP_NO_REP_INTERCOMM), &reqs[i]);
					}
					if(retValinter != 0) break;
				}
			}
			if(retValinter != 0)
			{
				if(commnR > 0 && cmpToRepMap[rank] == -1)
				{
					free(reqs);
					free(completereq);
					free(tmpscountsinter);
					free(tmpsdisplsinter);
					free(intermapping);
				}
				continue;
			}
			
      retVal = 0;
      retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			
      if(retVal == 0)
      {
        retVal = EMPI_Ialltoallv (sendbuf, sendcounts, sdispls, sendtype->edatatype, recvbuf, recvcounts, rdispls, recvtype->edatatype, (comm->EMPI_COMM_CMP), &req);
				
				ssize = sendtype->size;
				rsize = recvtype->size;
				EMPI_Type_size(sendtype->edatatype,&ssize);
				EMPI_Type_size(recvtype->edatatype,&rsize);
				curargs = (struct collective_data *)malloc(sizeof(struct collective_data));

				EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
				rcount = array_sum(recvcounts,commnC);
				scount = array_sum(sendcounts,commnC);
				(curargs->args).alltoallv.sendbuf = malloc(scount*ssize);
				long logdispl = 0;
				//memcpy((curargs->args).alltoallv.sendbuf, sendbuf + (sdispls[0]*ssize),(sendcounts[0]*ssize));
				for(int i = 0; i < commnC; i++)
				{
					memcpy((curargs->args).alltoallv.sendbuf + logdispl,sendbuf + (sdispls[i]*ssize),(sendcounts[i]*ssize));
					logdispl += (sendcounts[i]*ssize);
				}
				(curargs->args).alltoallv.recvbuf = malloc(rcount*rsize);
				(curargs->args).alltoallv.sendcounts = (int *)malloc(commnC*sizeof(int));
				memcpy((curargs->args).alltoallv.sendcounts,sendcounts,commnC*sizeof(int));
				(curargs->args).alltoallv.sdispls = (int *)malloc(commnC*sizeof(int));
				memcpy((curargs->args).alltoallv.sdispls,sdispls,commnC*sizeof(int));
				(curargs->args).alltoallv.senddt = sendtype;
				(curargs->args).alltoallv.recvcounts = (int *)malloc(commnC*sizeof(int));
				memcpy((curargs->args).alltoallv.recvcounts,recvcounts,commnC*sizeof(int));
				(curargs->args).alltoallv.rdispls = (int *)malloc(commnC*sizeof(int));
				memcpy((curargs->args).alltoallv.rdispls,rdispls,commnC*sizeof(int));
				(curargs->args).alltoallv.recvdt = recvtype;
				(curargs->args).alltoallv.comm = comm;
				curargs->completecmp = true;
				curargs->completerep = false;
				if(last_collective == NULL) curargs->id = 0;
				else curargs->id = last_collective->id + 1;
				curargs->type = MPI_FT_ALLTOALLV;
				curargs->repreq = reqs;
				
        int flag = 0;
        do
        {
          retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
          if(retVal != 0) break;
          EMPI_Test(&req,&flag,&status);
        } while(flag == 0);
				if(retVal != 0)
				{
					if(commnR > 0 && cmpToRepMap[rank] == -1)
					{
						free(reqs);
						free(completereq);
						free(tmpscountsinter);
						free(tmpsdisplsinter);
						free(intermapping);
					}
					free((curargs->args).alltoallv.rdispls);
					free((curargs->args).alltoallv.recvcounts);
					free((curargs->args).alltoallv.sdispls);
					free((curargs->args).alltoallv.sendcounts);
					free((curargs->args).alltoallv.recvbuf);
					free((curargs->args).alltoallv.sendbuf);
					free(curargs);
				}
      }
    } while(retVal != 0 || retValinter != 0);
		
		//memcpy((curargs->args).alltoallv.recvbuf, recvbuf + (rdispls[0]*rsize),(recvcounts[0]*rsize));
		//for(int i = 1; i < commnC; i++)
		//{
		//	memcpy((curargs->args).alltoallv.recvbuf + (recvcounts[i-1]*rsize),recvbuf + (rdispls[i]*rsize),(recvcounts[i]*rsize));
		//}
		if(last_collective != NULL) last_collective->prev = curargs;
		curargs->prev = NULL;
		curargs->next = last_collective;
		last_collective = curargs;
		do
    {
			retValinter = 0;
			EMPI_Comm_size(comm->EMPI_COMM_CMP,&commnC);
			if((comm->EMPI_CMP_REP_INTERCOMM) != EMPI_COMM_NULL) EMPI_Comm_remote_size((comm->EMPI_CMP_REP_INTERCOMM),&commnR);
			else commnR = 0;
			EMPI_Comm_rank(comm->EMPI_COMM_CMP,&rank);
			if(commnR > 0 && cmpToRepMap[rank] == -1) flagcount = 0;
			else flagcount = commnC-commnR;
			reqs =  (curargs->repreq);
			int flaginter = 0;
			do
			{
				flaginter = 0;
				if(commnR > 0 && cmpToRepMap[rank] == -1)
				{
					for(int i = 0; i < commnC-commnR; i++)
					{
						if(!completereq[i])
						{
							flaginter = 0;
							retValinter = check_errhandler_conditions_collective((comm->oworldComm),&reqs[i]);
							if(retValinter != 0) break;
							EMPI_Test(&reqs[i],&flaginter,&status);
							flagcount += flaginter;
							if(flaginter > 0) completereq[i] = true;
						}
					}
				}
				if(retValinter != 0) break;
			} while(flagcount < commnC-commnR);
		} while(retValinter != 0);
		last_collective->completerep = true;
		if(commnR > 0 && cmpToRepMap[rank] == -1)
		{
			free(reqs);
			free(completereq);
			free(tmpscountsinter);
			free(tmpsdisplsinter);
			free(intermapping);
		}
ConvertToCMPALLTOALLVSTORED:
    req = EMPI_REQUEST_NULL;
		if(commnR > 0 && commnC == commnR)
		{
			do
			{
				retVal = 0;
				retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
			} while(retVal != 0);
		}
		
  }
  return MPI_SUCCESS;
}

/*
int MPI_Gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, int root, MPI_Comm comm)
{
  int rank, size, i;
  MPI_Status *stats;
  MPI_Aint sdextent, rdextent;

  if (nR > 0 && comm == MPI_COMM_WORLD)
  {
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    MPI_Aint lb;
    MPI_Type_get_extent (recvtype, &lb, &rdextent);

    stats = (MPI_Status *) myMalloc (&stats, size * sizeof (MPI_Status));

    for (i=0; i<size; i++)
    {
      MPI_Send (sendbuf, sendcount, sendtype, root, 500 + i, comm);
      if (rank == root)
      {
        MPI_Recv ((void *) recvbuf + displs [i] * rdextent, recvcounts[i], recvtype, i, 500 + i, comm, &stats[i]);
      }
    }
    myFree (stats);
    return (MPI_SUCCESS);
  }
  else
    return PMPI_Gatherv (sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm);
}
*/
/*
int MPI_Scatterv(const void *sendbuf, const int sendcounts[], const int displs[], MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm)
{
  int rank, size, i;
  MPI_Status status;
  MPI_Aint sdextent, rdextent;

  if (nR > 0 && comm == MPI_COMM_WORLD)
  {
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    MPI_Aint lb;
    MPI_Type_get_extent (sendtype, &lb, &sdextent);

    for (i=0; i<size; i++)
    {
      if (rank == root)
      {
        MPI_Send ((void *) sendbuf + displs[i] * sdextent, sendcounts[i], sendtype, i, 600 + i, comm);
      }
      MPI_Recv (recvbuf, recvcount, recvtype, root, 600 + rank, comm, &status);
    }
    return (MPI_SUCCESS);
  }

  else
    return PMPI_Scatterv (sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm);
}
*/
int MPI_Barrier(MPI_Comm comm)
{
  //return EMPI_Barrier(comm->eworldComm);
  int retVal;
  EMPI_Status status;
  EMPI_Request req = EMPI_REQUEST_NULL;
  do
  {
    retVal = 0;
    retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);

    if(retVal == 0)
    {
			//if(last_collective!=NULL)printf("Calling barrier for id %d %d\n",last_collective->id + 1,comm == MPI_COMM_WORLD);
      retVal = EMPI_Ibarrier ((comm->eworldComm), &req);
			//if(comm->EMPI_COMM_CMP != EMPI_COMM_NULL) retVal = EMPI_Ibarrier ((comm->EMPI_COMM_CMP), &req);
			//else if(comm->EMPI_COMM_REP != EMPI_COMM_NULL) retVal = EMPI_Ibarrier ((comm->EMPI_COMM_REP), &req);
      int flag = 0;
      do
      {
        retVal = check_errhandler_conditions_collective((comm->oworldComm),&req);
        if(retVal != 0) break;
        EMPI_Test(&req,&flag,&status);
      } while(flag == 0);
    }
    if(retVal != 0)
    {
			req = EMPI_REQUEST_NULL;
      //int sz,rnk;
      //EMPI_Comm_size(comm->eworldComm,&sz);
			//EMPI_Comm_rank(comm->eworldComm,&rnk);
      //if(last_collective!=NULL)printf("%d: Barrier failed for id %d new size %d %d\n",rnk,last_collective->id + 1,sz,comm == MPI_COMM_WORLD);
    }
  } while(retVal != 0);
	//if(last_collective!=NULL)printf("Calling barrier complete for id %d\n",last_collective->id + 1);
	//mpi_ft_free_older_collectives(last_collective);
	last_collective = NULL;
	struct collective_data *curargs;
	curargs = (struct collective_data *)malloc(sizeof(struct collective_data));
  (curargs->args).barrier.comm = comm;
  if(last_collective == NULL) curargs->id = 0;
	else curargs->id = last_collective->id + 1;
	curargs->type = MPI_FT_BARRIER;
	if(last_collective != NULL) last_collective->prev = curargs;
	curargs->prev = NULL;
	curargs->next = last_collective;
	last_collective = curargs;
  return MPI_SUCCESS;
}

double MPI_Wtime()
{
  return EMPI_Wtime();
}

int MPI_Abort(MPI_Comm comm, int errorcode)
{
	return OMPI_Abort(comm->oworldComm,errorcode);
}
