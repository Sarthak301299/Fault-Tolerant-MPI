#include "client_thread.h"
#include "request_handler.h"
#include "heap_allocator.h"
#include "commbuf_cache.h"
#include "mmanager.h"
#include "wrappers.h"
#include "full_context.h"

#define MAX_RELATED_ANON 50

FILE *ckpt_file;
int ckpt_fd;

extern void (*_real_free)(void *);
extern void *(*_real_malloc)(size_t);

extern jmp_buf context;
extern jmp_buf parep_mpi_replication_initializer;

extern address __data_start;		// Initialized Data Segment 	: Start
extern address _edata;				// Initialized Data Segment		: End

extern address __bss_start;			// Uninitialized Data Segment	: Start
extern address _end;				// Uninitialized Data Segment	: End

extern double mpi_ft_ckpt_start_time;
extern double mpi_ft_ckpt_end_time;

extern char *tempStack;
extern char *tempMap;
extern address parep_mpi_temp_pc[10];
extern int parep_mpi_jumped[10];

extern int writing_ckpt_futex;
extern long writing_ckpt_ret;
extern int reached_temp_futex[10];
extern long reached_temp_ret[10];

extern char *newStack;
extern int parep_mpi_size;
extern int parep_mpi_rank;
extern int parep_mpi_node_rank;
extern int parep_mpi_node_id;
extern int parep_mpi_node_num;
extern int parep_mpi_node_size;
extern int parep_mpi_original_rank;
extern pid_t parep_mpi_pid;

extern struct collective_data *last_collective;
extern struct peertopeer_data *first_peertopeer;
extern struct peertopeer_data *last_peertopeer;

extern address parep_mpi_reg_vals[64][15];
extern int parep_mpi_num_reg_vals;
extern bin_t parep_mpi_bins[BIN_COUNT];
extern bin_t parep_mpi_fastbins[FASTBIN_COUNT];
extern pthread_mutex_t heap_free_list_mutex;

extern commbuf_bin_t parep_mpi_commbuf_bins[COMMBUF_BIN_COUNT];

extern recvDataNode *recvDataHead;
extern recvDataNode *recvDataTail;

extern recvDataNode *recvDataRedHead;
extern recvDataNode *recvDataRedTail;

extern reqNode *reqHead;
extern reqNode *reqTail;

extern openFileNode *openFileHead;
extern openFileNode *openFileTail;

extern int related_map;
address llimit[2];
address hlimit[2];
address olimit[2];
address elimit[2];
address levclimit[2];
address levplimit[2];
address lpallimit[2];
address lclimit[2];
address oesbase, oesbasedata;
address libOff, openOff, extOff, libevcOff, libevpOff, libpalOff, libcOff;
address slower, shigher;
address sOff;
address aqs;
int naqs;
ProcessStat pstat;
int nmaps,hmaps,smaps,amaps;
Mapping amapping[100];
int num_relatedanon;
int relatedanon[MAX_RELATED_ANON];
int libctoheap;
address libctoheapptr[5000];
address libctoheapdat[5000];
int anontoheap[MAX_RELATED_ANON];
address anontoheapptr[MAX_RELATED_ANON][5000];
address anontoheapdat[MAX_RELATED_ANON][5000];

address ldLim[2];
address ld2Lim[2];
address ldlimit[2];
address ld2limit[2];
int ldtoheap;
address ldtoheapptr[5000];
address ldtoheapdat[5000];
address ldOff,ld2Off;

Context processContext;
int unmap_num;
int remap_num;
address unmap_addr[100][2];
address remap_addr[MAX_MAPPINGS][3];
bool remap_mark[MAX_MAPPINGS];

address tempremapaddr;

extern int num_threads;
extern void *threadfsbase[10];
extern jmp_buf threadContext[10];
extern Context threadprocContext[10];

extern pid_t kernel_tid[10];

extern struct collective_data *last_collective;
extern struct peertopeer_data *first_peertopeer;
extern struct peertopeer_data *last_peertopeer;

size_t total_malloc_allocation_size = 0;
int malloc_number_of_allocations = 0;
Malloc_list *head = NULL;
Malloc_list *tail = NULL;

extern void *parep_mpi_ext_heap_mapping;
extern size_t parep_mpi_ext_heap_size;

extern void *parep_mpi_new_stack;
extern size_t parep_mpi_new_stack_size;
extern address parep_mpi_new_stack_start;
extern address parep_mpi_new_stack_end;

extern double ___ckpt_time[MAX_CKPT];
extern int ___ckpt_counter;

extern long parep_mpi_store_buf_sz;
extern pthread_mutex_t parep_mpi_store_buf_sz_mutex;
extern pthread_cond_t parep_mpi_store_buf_sz_cond;

extern bool threadcontext_write_ready[10];
extern pthread_mutex_t threadcontext_write_mutex;
extern pthread_cond_t threadcontext_write_cond_var;

extern int parep_mpi_sendid;
extern int parep_mpi_collective_id;

extern int parep_mpi_pmi_fd;
extern char parep_mpi_kvs_name[256];

extern int parep_mpi_fortran_binding_used;

__attribute__((optimize(0), __noinline__))
void *getCurrentPC() { return __builtin_return_address(0); }

NO_OPTIMIZE
static int parep_mpi_open(const char *pathname, int flags, mode_t mode) {
	int ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"mov %3, %%rsi\n\t"
								"mov %4, %%rdx\n\t"
								"syscall\n\t"
								"mov %%eax, %0"
								: "=r" (ret)
								: "i" (SYS_open),
									"r" (pathname),
									"r" ((long)flags),
									"r" ((long)mode)
								: "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static int parep_mpi_close(int fd) {
	int ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"syscall\n\t"
								"mov %%eax, %0"
								: "=r" (ret)
								: "i" (SYS_close),
									"r" ((long)fd)
								: "rax", "rdi", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static ssize_t parep_mpi_read(int fd, void *buf, size_t count) {
	ssize_t ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"mov %3, %%rsi\n\t"
								"mov %4, %%rdx\n\t"
								"syscall\n\t"
								"mov %%rax, %0"
								: "=r" (ret)
								: "i" (SYS_read),
									"r" ((long)fd),
									"r" (buf),
									"r" (count)
								: "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static ssize_t parep_mpi_write(int fd, void *buf, size_t count) {
	ssize_t ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"mov %3, %%rsi\n\t"
								"mov %4, %%rdx\n\t"
								"syscall\n\t"
								"mov %%rax, %0"
								: "=r" (ret)
								: "i" (SYS_write),
									"r" ((long)fd),
									"r" (buf),
									"r" (count)
								: "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static off_t parep_mpi_lseek(int fd, off_t offset, int whence) {
	off_t ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"mov %3, %%rsi\n\t"
								"mov %4, %%rdx\n\t"
								"syscall\n\t"
								"mov %%rax, %0"
								: "=r" (ret)
								: "i" (SYS_lseek),
									"r" ((long)fd),
									"r" (offset),
									"r" ((long)whence)
								: "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static void *parep_mpi_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
	void *ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"mov %3, %%rsi\n\t"
								"mov %4, %%rdx\n\t"
								"mov %5, %%r10\n\t"
								"mov %6, %%r8\n\t"
								"mov %7, %%r9\n\t"
								"syscall\n\t"
								"mov %%rax, %0"
								: "=r" (ret)
								: "i" (SYS_mmap),
									"r" (addr),
									"r" (length),
									"r" ((long)prot),
									"r" ((long)flags),
									"r" ((long)fd),
									"r" (offset)
								: "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory");
	return ret;
}

NO_OPTIMIZE
static int parep_mpi_munmap(void *addr, size_t length) {
	int ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"mov %3, %%rsi\n\t"
								"syscall\n\t"
								"mov %%eax, %0"
								: "=r" (ret)
								: "i" (SYS_munmap),
									"r" (addr),
									"r" (length)
								: "rax", "rdi", "rsi", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static void *parep_mpi_mremap(void *old_addr, size_t old_size, size_t new_size, int flags, void *new_addr) {
	void *ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"mov %3, %%rsi\n\t"
								"mov %4, %%rdx\n\t"
								"mov %5, %%r10\n\t"
								"mov %6, %%r8\n\t"
								"syscall\n\t"
								"mov %%rax, %0"
								: "=r" (ret)
								: "i" (SYS_mremap),
									"r" (old_addr),
									"r" (old_size),
									"r" (new_size),
									"r" ((long)flags),
									"r" (new_addr)
								: "rax", "rdi", "rsi", "rdx", "r10", "r8", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static int parep_mpi_mprotect(void *addr, size_t len, int prot) {
	int ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"mov %3, %%rsi\n\t"
								"mov %4, %%rdx\n\t"
								"syscall\n\t"
								"mov %%eax, %0"
								: "=r" (ret)
								: "i" (SYS_mprotect),
									"r" (addr),
									"r" (len),
									"r" ((long)prot)
								: "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static void *parep_mpi_brk(void *addr) {
	void *ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"syscall\n\t"
								"mov %%rax, %0"
								: "=r" (ret)
								: "i" (SYS_brk),
									"r" (addr)
								: "rax", "rdi", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static int parep_mpi_prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5) {
	int ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"mov %3, %%rsi\n\t"
								"mov %4, %%rdx\n\t"
								"mov %5, %%r10\n\t"
								"mov %6, %%r8\n\t"
								"syscall\n\t"
								"mov %%eax, %0"
								: "=r" (ret)
								: "i" (SYS_prctl),
									"r" ((long)option),
									"r" (arg2),
									"r" (arg3),
									"r" (arg4),
									"r" (arg5)
								: "rax", "rdi", "rsi", "rdx", "r10", "r8", "rcx", "r11", "memory");
	return ret;
}

NO_OPTIMIZE
static int parep_mpi_arch_prctl(int code, unsigned long addr) {
	int ret;
	asm volatile ("mov %1, %%rax\n\t"
								"mov %2, %%rdi\n\t"
								"mov %3, %%rsi\n\t"
								"syscall\n\t"
								"mov %%eax, %0"
								: "=r" (ret)
								: "i" (SYS_arch_prctl),
									"r" ((long)code),
									"r" (addr)
								: "rax", "rdi", "rsi", "rcx", "r11", "memory");
	return ret;
}

void copy_jmp_buf(jmp_buf source, jmp_buf dest) {
	for(int i = 0; i<8; i++) {
		dest[0].__jmpbuf[i] = source[0].__jmpbuf[i];
	}
	dest[0].__mask_was_saved = source[0].__mask_was_saved;
}

address getPC(jmp_buf context) {
	address rip = context[0].__jmpbuf[JB_PC];
	PTR_DECRYPT(rip);
	return rip;
}

address getRBP(jmp_buf context) {
	address rbp = context[0].__jmpbuf[JB_RBP];
	PTR_DECRYPT(rbp);
	return rbp;
}

address getRSP(jmp_buf context) {
	address rsp = context[0].__jmpbuf[JB_RSP];
	PTR_DECRYPT(rsp);
	return rsp;
}

int setPC(jmp_buf context, address pc) {
	PTR_ENCRYPT(pc);
	context[0].__jmpbuf[JB_PC] = pc;
	return 1;
}

int setRBP(jmp_buf context, address rbp) {
	PTR_ENCRYPT(rbp);
	context[0].__jmpbuf[JB_RBP] = rbp;
	return 1;
}

int setRSP(jmp_buf context, address rsp) {
	PTR_ENCRYPT(rsp);
	context[0].__jmpbuf[JB_RSP] = rsp;
	return 1;
}

void rep_append(Malloc_container container) {
	Malloc_list *element = libc_malloc(sizeof(Malloc_list));

	element->next = NULL;
	element->prev = tail;
	
	(element->container).container_address = container.container_address;
	(element->container).linked_address = container.linked_address;
	(element->container).allocated_address = container.allocated_address;
	(element->container).size = container.size;

	if(head == NULL) {
		head = element;
	}
	else {
		tail->next = element;
	}
	tail = element;
}

void rep_remove(void *node_add) {
	Malloc_list *element = (Malloc_list *)node_add;
	Malloc_list *prev = element->prev;

	if(element -> next == NULL && element -> prev == NULL) {
		libc_free(element);
		head = NULL;
		tail = NULL;
		return;
	}

	if(element -> next != NULL) {
		(element->next)->prev = prev;
	}
	else {
		tail = prev;
	}

	if(prev != NULL) {
		prev->next = element->next;
	}
	else {
		head = element->next;
	}
	libc_free(element);
}

void rep_update_cont_addr(address alloc, address cont)
{
	Malloc_list *temp = head;
	while(temp != NULL) {
		if(((temp->container).container_address == (address)NULL) && ((temp->container).allocated_address == alloc)) {
			(temp->container).container_address = cont;
			break;
		}
		temp = temp->next;
	}
}

void rep_update_cont_and_link_addr(address alloc, address cont, address link)
{
	Malloc_list *temp = head;
	while(temp != NULL) {
		if(((temp->container).container_address == (address)NULL) && ((temp->container).linked_address == (address)NULL) && ((temp->container).allocated_address == alloc)) {
			(temp->container).container_address = cont;
			(temp->container).linked_address = link;
			break;
		}
		temp = temp->next;
	}
}

Malloc_container *rep_get_linked_node(void *node)
{
	Malloc_container *element = (Malloc_container *)node;
	Malloc_list *temp = tail;
	while(temp != NULL) {
		if(element->allocated_address == (temp->container).linked_address) {
			return &(temp->container);
		}
		temp = temp->prev;
	}
	return (Malloc_container *)NULL;
}

void rep_clear() {
	
	address *ptr;
	ptr = (address *)((head->container).allocated_address);
	
	free(ptr);
	Malloc_list *temp = head;
	while(temp != NULL) {
		head = temp;
		temp = temp->next;
		free(head);
	}
	head = NULL;
	tail = NULL;
	malloc_number_of_allocations = 0;
}

void rep_clear_discontiguous() {
	Malloc_list *temp = head;
	address *ptr;
	

	while(temp != NULL) {

		head = temp;
		temp = temp->next;

		ptr = ((address *)(head->container).allocated_address);
		free(ptr);
		free(head);
	}
	head = NULL;
	tail = NULL;
	malloc_number_of_allocations = 0;
}

void rep_assign_malloc_context(const void **source, const void **dest) {
	Malloc_container cont;
	cont.container_address = (address)dest;
	cont.linked_address = (address)source;
	cont.size = 0;

	cont.allocated_address = ((address)(*source));

	malloc_number_of_allocations++;

	rep_append(cont);

	*dest = *source;
}

void rep_malloc(void **container, size_t size) {
	*container = libc_malloc(size);

	Malloc_container cont;
	cont.container_address = (address)container;
	cont.linked_address = (address)NULL;
	cont.allocated_address = (address)*container;
	cont.size = size;

	total_malloc_allocation_size += size;
	malloc_number_of_allocations++;

	rep_append(cont);
}

void *ft_malloc(size_t size)
{
	Malloc_container cont;
	cont.container_address = (address)NULL;
	cont.linked_address = (address)NULL;
	cont.allocated_address = (address)libc_malloc(size);
	cont.size = size;

	total_malloc_allocation_size += size;
	malloc_number_of_allocations++;

	rep_append(cont);
	return (void *)cont.allocated_address;
}

void ft_free(void *p)
{
	Malloc_list *temp = head;

	// Empty linkedlist
	while(temp != NULL) {
		if((temp->container).allocated_address == (address)p) {
			address tt = (temp->container).allocated_address;
			libc_free(((address *)((temp->container).allocated_address)));
			rep_remove(temp);
			malloc_number_of_allocations--;
			break;
		}
		temp = temp->next;
	}
}

void rep_free(void **cont_add) {
	Malloc_list *temp = head;

	// Empty linkedlist
	while(temp != NULL) {
		if(((temp->container).container_address == ((address)(cont_add))) || ((temp->container).allocated_address == *((address *)(cont_add)))) {
			address tt = (temp->container).allocated_address;
			libc_free(((address *)((temp->container).allocated_address)));
			rep_remove(temp);
			malloc_number_of_allocations--;
			break;
		}
		temp = temp->next;
	}
}

void rep_free_by_alloc(void *alloc_add) {
	Malloc_list *temp = head;

	// Empty linkedlist
	while(temp != NULL) {
		if((temp->container).allocated_address == ((address)(alloc_add))) {
			address tt = (temp->container).allocated_address;
			libc_free(((address *)((temp->container).allocated_address)));
			rep_remove(temp);
			malloc_number_of_allocations--;
			break;
		}
		temp = temp->next;
	}
}

void bubbleSort(int arr[], int n) {
	int temp;
	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < n - i - 1; j++) {
			if (arr[j] > arr[j + 1]) {
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
			}
		}
	}
}

int removeDuplicates(int arr[], int n) {
	if (n == 0 || n == 1) {
		return n;
	}

	int temp[n];
	int j = 0;

	for (int i = 0; i < n - 1; i++) {
		if (arr[i] != arr[i + 1]) {
			temp[j++] = arr[i];
		}
	}

	temp[j++] = arr[n - 1];

	for (int i = 0; i < j; i++) {
		arr[i] = temp[i];
	}

	return j;
}

NO_OPTIMIZE
static size_t parep_mpi_strlen(const char *s) {
	size_t len = 0;
	while (*s++ != '\0') {
		len++;
	}
	return len;
}

NO_OPTIMIZE
static int parep_mpi_strcmp (const char *s1, const char *s2) {
	size_t n = parep_mpi_strlen(s2);
	unsigned char c1 = (unsigned char) *s1;
	unsigned char c2 = '\0';

	while (n > 0) {
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		if (c1 == '\0' || c1 != c2)
		return c1 - c2;
		n--;
	}
	return c1 - c2;
}

NO_OPTIMIZE
static int parep_mpi_strncmp (const char *s1, const char *s2, size_t n) {
  unsigned char c1 = '\0';
  unsigned char c2 = '\0';

  while (n > 0) {
    c1 = (unsigned char) *s1++;
    c2 = (unsigned char) *s2++;
    if (c1 == '\0' || c1 != c2)
      return c1 - c2;
    n--;
  }
  return c1 - c2;
}

NO_OPTIMIZE
static void parep_mpi_strncpy(char *dest, const char *src, size_t n) {
	size_t i;

	for (i = 0; i < n && src[i] != '\0'; i++)
		dest[i] = src[i];
	if (i < n) {
		dest[i] = '\0';
	}
}

NO_OPTIMIZE
static void parep_mpi_strcpy(char *dest, const char *src) {
	while (*src != '\0') {
		*dest++ = *src++;
	}
	*dest = '\0';
}

NO_OPTIMIZE
static char *parep_mpi_strchr(const char *s, int c) {
	for (; *s != (char)'\0'; s++)
		if (*s == (char)c)
			return (char *)s;
	return NULL;
}

NO_OPTIMIZE
static char *parep_mpi_strrchr(const char *s, int c) {
	char *rc = NULL;
	char *suffix = (char *)s;
	while (1) {
		if (*suffix == '\0') { break; }
		if (*suffix == c) { rc = suffix; }
		suffix++;
	}
	return rc;
}

NO_OPTIMIZE
static const void *parep_mpi_strstr(const char *string, const char *substring)
{
	for ( ; *string != '\0' ; string++) {
		const char *ptr1, *ptr2;
		for (ptr1 = string, ptr2 = substring; *ptr1 == *ptr2 && *ptr2 != '\0'; ptr1++, ptr2++) ;
		if (*ptr2 == '\0') return string;
	}
	return NULL;
}

NO_OPTIMIZE
static int parep_mpi_atoi(const char *str) {
	int result = 0;
	int sign = 1;
	int i = 0;
	if(str[0] == '-') {
		sign = -1;
		i = 1;
	}
	while(str[i] != '\0') {
		int digit = str[i] - '0';
		result = (result*10) + digit;
		i++;
	}
	return sign*result;
}

NO_OPTIMIZE
static char *parep_mpi_ltoa(long num, char *str, int base) {
	int index = 0;
	int isNegative = 0;
	if (base == 10 && num < 0) {
		isNegative = 1;
		num = -num;
	}
	
	do {
		int remainder = num % base;
		if(remainder < 10) {
			str[index++] = '0' + remainder;
		} else {
			str[index++] = 'A' + (remainder - 10);
		}
		num = num / base;
	} while(num > 0);
	
	if(isNegative) {
		str[index++] = '-';
	}
	
	str[index] = '\0';
	
	int start = 0;
	int end = index-1;
	while(start < end) {
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		start++;
		end--;
	}
	
	return str;
}

NO_OPTIMIZE
static char *parep_mpi_itoa(int num, char *str, int base) {
	int isNegative = 0;
	int i = 0;
	if(num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}
	if(num < 0) {
		isNegative = 1;
		num = -num;
	}
	while(num != 0) {
		int digit = num % 10;
		str[i++] = '0' + digit;
		num = num / 10;
	}
	if(isNegative) {
		str[i++] = '-';
	}
	str[i] = '\0';
	int start = 0;
	int end = i-1;
	while(start < end) {
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		start++;
		end--;
	}
	return str;
}

NO_OPTIMIZE
static void *parep_mpi_memset(void *s, int c, size_t n) {
	char *p = s;
	while (n-- > 0) {
		*p++ = (char)c;
	}
	return s;
}

NO_OPTIMIZE
static bool strStartsWith(const char *str, const char *pattern)
{
	if (str == NULL || pattern == NULL) {
		return false;
	}
	int len1 = parep_mpi_strlen(str);
	int len2 = parep_mpi_strlen(pattern);
	if (len1 >= len2) {
		return (parep_mpi_strncmp(str, pattern, len2) == 0);
	}
	return false;
}

NO_OPTIMIZE
static void getDirectory(const char *fullPath, char *directory) {
	const char *lastSlash = parep_mpi_strrchr(fullPath,'/');
	if(lastSlash == NULL) {
		directory = NULL;
	}
	else {
		size_t length = lastSlash - fullPath;
		parep_mpi_strncpy(directory,fullPath,length);
		directory[length] = '\0';
	}
}

NO_OPTIMIZE
static bool isNscdArea(Mapping *map) {
	if (strStartsWith(map->pathname, "/run/nscd") ||
			strStartsWith(map->pathname, "/var/run/nscd") ||
			strStartsWith(map->pathname, "/var/cache/nscd") ||
			strStartsWith(map->pathname, "/var/db/nscd") ||
			strStartsWith(map->pathname, "/ram/var/run/nscd")) {
		return true;
	}
	return false;
}

NO_OPTIMIZE
static bool isIBShmArea(Mapping *map) {
	return strStartsWith(map->pathname,"/dev/infiniband/uverbs");
}

NO_OPTIMIZE
static bool FileExists(const char *str) {
	struct stat st;
	if(!stat(str, &st)) {
		return true;
	}
	else {
		return false;
	}
}

NO_OPTIMIZE
static bool isTmpfsFile(const char *filename) {
	return strStartsWith(filename,"/dev/shm/");
}

NO_OPTIMIZE
static int doAreasOverlap(char *addr1, size_t size1, char *addr2, size_t size2)
{
	char *end1 = (char *)addr1 + size1;
	char *end2 = (char *)addr2 + size2;
	return (size1 > 0 && addr1 >= addr2 && addr1 < end2) || (size2 > 0 && addr2 >= addr1 && addr2 < end1);
}

NO_OPTIMIZE
static int doAreasOverlap2(char *addr, int length, char *vdsoStart, char *vdsoEnd, char *vvarStart, char *vvarEnd) {
	return doAreasOverlap(addr, length, vdsoStart, vdsoEnd - vdsoStart) || doAreasOverlap(addr, length, vvarStart, vvarEnd - vvarStart);
}

NO_OPTIMIZE
static int parep_mpi_perform_read(int fd, void * buf, size_t size) {
	ssize_t rc;
	size_t ar = 0;
#if __arm__ || __aarch64__
	WMB;
#endif
	while(ar != size) {
		rc = parep_mpi_read(fd, buf + ar, size - ar);
		if (rc == 0) {
			return ar;
		}
		ar += rc;
	}
#if __arm__ || __aarch64__
	WMB;
	IMB;
#endif
	return ar;
}

NO_OPTIMIZE
static int parep_mpi_perform_write(int fd, void * buf, size_t count) {
	size_t num_written = 0;
	do {
		ssize_t rc = parep_mpi_write(fd, buf + num_written, count - num_written);
		num_written += rc;
	} while (num_written != count);
	return num_written;
}

off_t compute_ckpt_size() {
	off_t ckpt_size = 0;
	
	ckpt_size += sizeof(Context);
	ckpt_size += sizeof(jmp_buf);
	ckpt_size += sizeof(address);
	
	ckpt_size += sizeof(int);
	
	for(int i = 1; i < num_threads; i++) {
		ckpt_size += sizeof(Context);
		ckpt_size += sizeof(jmp_buf);
		ckpt_size += sizeof(address);
	}
	
	ckpt_size += sizeof(ProcessStat);
	
	for(int i = 0; i < num_mappings; i++) {
		if((mappings[i].flags & MAP_SHARED) && (!isIBShmArea(&(mappings[i])))) {
			ckpt_size += sizeof(Mapping);
			if(parep_mpi_strstr(mappings[i].pathname,"sm_segment")) {
				int rank_from_str = parep_mpi_atoi(parep_mpi_strrchr(mappings[i].pathname,'.') + 1);
				if(rank_from_str == parep_mpi_node_rank) {
					ckpt_size += (mappings[i].end - mappings[i].start);
				}
			} else if(parep_mpi_strstr(mappings[i].pathname,"ib_shmem-kvs")) {
				ckpt_size += sizeof(int);
				address base_start = mappings[i].start + (sizeof(pid_t)*parep_mpi_node_size);
				address base_end = mappings[i].end;
				address start = base_start + (parep_mpi_node_rank * ((base_end - base_start) / parep_mpi_node_size));
				address end = base_start + ((parep_mpi_node_rank+1) * ((base_end - base_start) / parep_mpi_node_size));
				if(end > base_end) end = base_end;
				//ckpt_size += (end-start);
				//ckpt_size += (mappings[i].end - mappings[i].start);
			}
			continue;
		}
		if((mappings[i].end - mappings[i].start) == 0) {
			continue;
		}
		else if((!strcmp(mappings[i].pathname,"[vsyscall]")) || (!strcmp(mappings[i].pathname,"[vectors]")) || (!strcmp(mappings[i].pathname,"[vvar]")) || (!strcmp(mappings[i].pathname,"[vdso]"))) {
			continue;
		}
		else if(strStartsWith(mappings[i].pathname,"/dev/zero (deleted)") || strStartsWith(mappings[i].pathname,"/dev/null (deleted)")) {
		}
		else if(strStartsWith(mappings[i].pathname,"/SYSV")) {
		}
		else if(isNscdArea(&(mappings[i]))) {
			ckpt_size += sizeof(Mapping);
			continue;
		}
		else if(isIBShmArea(&(mappings[i]))) {
			ckpt_size += sizeof(Mapping);
			continue;
		}
		
		if((mappings[i].flags & MAP_ANONYMOUS) != 0) {
			ckpt_size += sizeof(Mapping);
			ckpt_size += (mappings[i].end - mappings[i].start);
		}
		else if(!FileExists(mappings[i].pathname)) {
			ckpt_size += sizeof(Mapping);
			ckpt_size += (mappings[i].end - mappings[i].start);
		}
		else {
			struct stat statbuf = {0};
			if(stat(mappings[i].pathname, &statbuf) == 0) {
				if((mappings[i].prot & PROT_WRITE) || ((statbuf.st_size - mappings[i].offset) > (mappings[i].end - mappings[i].start))) {
					mappings[i].wrtSize = mappings[i].end - mappings[i].start;
				}
				else {
					mappings[i].wrtSize = statbuf.st_size - mappings[i].offset;
				}
			}
			ckpt_size += sizeof(Mapping);
			
			if(mappings[i].wrtSize > 0) {
				if(mappings[i].pathname[0] == '/') {
					if((!(mappings[i].prot & PROT_EXEC)) && (mappings[i].prot != PROT_NONE)) {
						ckpt_size += mappings[i].wrtSize;
					}
				}
				else {
					ckpt_size += mappings[i].wrtSize;
				}
			}
			else {
				ckpt_size += (mappings[i].end - mappings[i].start);
			}
		}
	}
	ckpt_size += sizeof(Mapping);
	
	return ckpt_size;
}

void init_ckpt(char *file_name, bool is_baseline) {
	if(is_baseline) {
		char file[100];
		strcpy(file,file_name);
		
		compute_ckpt_size();
		
		ckpt_fd = parep_mpi_open(file,O_CREAT | O_TRUNC | O_WRONLY,0600);
		//ftruncate(ckpt_fd,compute_ckpt_size());
		
		Context processContext;

		processContext.rip = getPC(context);
		processContext.rsp = getRSP(context);
		processContext.rbp = getRBP(context);
		
		void *fsbase;
		//asm volatile("\t rdfsbase %0" : "=r"(fsbase));
		parep_mpi_arch_prctl(ARCH_GET_FS,(unsigned long)(&fsbase));
		//syscall(SYS_arch_prctl,ARCH_GET_FS,&fsbase);
		
		parep_mpi_perform_write(ckpt_fd,&processContext,sizeof(Context));
		parep_mpi_perform_write(ckpt_fd,&context,sizeof(jmp_buf));
		parep_mpi_perform_write(ckpt_fd,&fsbase,sizeof(address));
		
		parep_mpi_perform_write(ckpt_fd,&num_threads,sizeof(int));
		for(int i = 1; i < num_threads; i++) {
			parep_mpi_perform_write(ckpt_fd,&threadprocContext[i],sizeof(Context));
			parep_mpi_perform_write(ckpt_fd,&threadContext[i],sizeof(jmp_buf));
			parep_mpi_perform_write(ckpt_fd,&threadfsbase[i],sizeof(address));
		}
		
		parep_mpi_perform_write(ckpt_fd,&prstat,sizeof(ProcessStat));
		
		for(int i = 0; i < num_mappings; i++) {
			if((mappings[i].flags & MAP_SHARED) && (!isIBShmArea(&(mappings[i])))) {
				parep_mpi_perform_write(ckpt_fd,&(mappings[i]),sizeof(Mapping));
				if(parep_mpi_strstr(mappings[i].pathname,"sm_segment")) {
					int rank_from_str = parep_mpi_atoi(parep_mpi_strrchr(mappings[i].pathname,'.') + 1);
					if(rank_from_str == parep_mpi_node_rank) {
						if((mappings[i].prot & PROT_READ) == 0) {
							mprotect((void *)mappings[i].start, mappings[i].end - mappings[i].start, mappings[i].prot | PROT_READ);
						}
						parep_mpi_perform_write(ckpt_fd,(void *)mappings[i].start,mappings[i].end - mappings[i].start);
						if((mappings[i].prot & PROT_READ) == 0) {
							mprotect((void *)mappings[i].start, mappings[i].end - mappings[i].start, mappings[i].prot);
						}
					}
				} else if(parep_mpi_strstr(mappings[i].pathname,"ib_shmem-kvs")) {
					if((mappings[i].prot & PROT_READ) == 0) {
						mprotect((void *)mappings[i].start, mappings[i].end - mappings[i].start, mappings[i].prot | PROT_READ);
					}
					
					parep_mpi_perform_write(ckpt_fd,&parep_mpi_original_rank,sizeof(int));
					address base_start = mappings[i].start + (sizeof(pid_t)*parep_mpi_node_size);
					address base_end = mappings[i].end;
					address start = base_start + (parep_mpi_node_rank * ((base_end - base_start) / parep_mpi_node_size));
					address end = base_start + ((parep_mpi_node_rank+1) * ((base_end - base_start) / parep_mpi_node_size));
					if(end > base_end) end = base_end;

					//parep_mpi_perform_write(ckpt_fd,(void *)start,end - start);
					//parep_mpi_perform_write(ckpt_fd,(void *)mappings[i].start,mappings[i].end - mappings[i].start);
					if((mappings[i].prot & PROT_READ) == 0) {
						mprotect((void *)mappings[i].start, mappings[i].end - mappings[i].start, mappings[i].prot);
					}
				}
				continue;
			}
			if((mappings[i].end - mappings[i].start) == 0) {
				continue;
			}
			else if((!strcmp(mappings[i].pathname,"[vsyscall]")) || (!strcmp(mappings[i].pathname,"[vectors]")) || (!strcmp(mappings[i].pathname,"[vvar]")) || (!strcmp(mappings[i].pathname,"[vdso]"))) {
				continue;
			}
			else if(strStartsWith(mappings[i].pathname,"/dev/zero (deleted)") || strStartsWith(mappings[i].pathname,"/dev/null (deleted)")) {
				mappings[i].flags = MAP_PRIVATE|MAP_ANONYMOUS;
				mappings[i].pathname[0] = '\0';
			}
			else if(strStartsWith(mappings[i].pathname,"/SYSV")) {
				mappings[i].flags = MAP_PRIVATE|MAP_ANONYMOUS;
				mappings[i].pathname[0] = '\0';
			}
			else if(isNscdArea(&(mappings[i]))) {
				mappings[i].prot = PROT_READ | PROT_WRITE;
				mappings[i].flags = MAP_PRIVATE | MAP_ANONYMOUS;
				parep_mpi_perform_write(ckpt_fd,&(mappings[i]),sizeof(Mapping));
				continue;
			}
			else if(isIBShmArea(&(mappings[i]))) {
				parep_mpi_perform_write(ckpt_fd,&(mappings[i]),sizeof(Mapping));
				continue;
			}
			
			if((mappings[i].prot & PROT_READ) == 0) {
				mprotect((void *)mappings[i].start, mappings[i].end - mappings[i].start, mappings[i].prot | PROT_READ);
			}
			
			if(mappings[i].pathname[0] == '\0') {
				address brk = (address)sbrk(0);
				if((brk > mappings[i].start) && (brk <= mappings[i].end)) {
					strcpy(mappings[i].pathname,"[heap]");
				}
			}
			
			if((mappings[i].flags & MAP_ANONYMOUS) != 0) {
				parep_mpi_perform_write(ckpt_fd,&(mappings[i]),sizeof(Mapping));
				parep_mpi_perform_write(ckpt_fd,(void *)mappings[i].start,mappings[i].end - mappings[i].start);
			}
			/*else if(!FileExists(mappings[i].pathname)) {
				parep_mpi_perform_write(ckpt_fd,&(mappings[i]),sizeof(Mapping));
				parep_mpi_perform_write(ckpt_fd,(void *)mappings[i].start,mappings[i].end - mappings[i].start);
			}
			*/else {
				/*struct stat statbuf = {0};
				if(stat(mappings[i].pathname, &statbuf) == 0) {
					if((mappings[i].prot & PROT_WRITE) || ((statbuf.st_size - mappings[i].offset) > (mappings[i].end - mappings[i].start))) {
						mappings[i].wrtSize = mappings[i].end - mappings[i].start;
					}
					else {
						mappings[i].wrtSize = statbuf.st_size - mappings[i].offset;
					}
				}*/
				parep_mpi_perform_write(ckpt_fd,&(mappings[i]),sizeof(Mapping));
				
				if(mappings[i].wrtSize != (long)-1) {
					if(mappings[i].pathname[0] == '/') {
						if((!(mappings[i].prot & PROT_EXEC)) && (mappings[i].prot != PROT_NONE)) {
							parep_mpi_perform_write(ckpt_fd,(void *)mappings[i].start,mappings[i].wrtSize);
						}
					}
					else {
						parep_mpi_perform_write(ckpt_fd,(void *)mappings[i].start,mappings[i].wrtSize);
					}
				}
				else {
					parep_mpi_perform_write(ckpt_fd,(void *)mappings[i].start,mappings[i].end - mappings[i].start);
				}
			}
			
			if((mappings[i].prot & PROT_READ) == 0) {
				mprotect((void *)mappings[i].start, mappings[i].end - mappings[i].start, mappings[i].prot);
			}
		}
		
		Mapping last;
		last.start = (address)NULL;
		last.end = (address)NULL;
		
		parep_mpi_perform_write(ckpt_fd,&(last),sizeof(Mapping));
		
		parep_mpi_close(ckpt_fd);
	}
}

void write_heap_and_stack(char *file_name) {
	if(MPI_COMM_WORLD->EMPI_COMM_CMP != EMPI_COMM_NULL) {
		char file[100];
		extern address __data_start;
		extern address _edata;
		extern address __bss_start;
		extern address _end;
		address my_req_null = (address)MPI_REQUEST_NULL;
		
		address offset;
		char mapname[256];
		
		address fsbasekey;
		//asm volatile("\t rdfsbase %0" : "=r"(fsbasekey));
		parep_mpi_arch_prctl(ARCH_GET_FS,(unsigned long)(&fsbasekey));
		//syscall(SYS_arch_prctl,ARCH_GET_FS,&fsbasekey);
		fsbasekey = *((address *)(fsbasekey+0x28));
		strcpy(file,file_name);
		ckpt_fd = parep_mpi_open(file,O_CREAT | O_TRUNC | O_WRONLY,0600);
		
		Context processContext;
		processContext.rip = getPC(parep_mpi_replication_initializer);
		processContext.rsp = getRSP(parep_mpi_replication_initializer);
		processContext.rbp = getRBP(parep_mpi_replication_initializer);
		
		parep_mpi_perform_write(ckpt_fd,&fsbasekey,sizeof(address));
		parep_mpi_perform_write(ckpt_fd,&processContext,sizeof(Context));
		parep_mpi_perform_write(ckpt_fd,&parep_mpi_replication_initializer,sizeof(jmp_buf));
		
		parep_mpi_perform_write(ckpt_fd,&parep_mpi_num_reg_vals,sizeof(int));
		
		for(int i = 0; i < parep_mpi_num_reg_vals; i++) {
			for(int k = 0; k < 15; k++) {
				offset = 0;
				mapname[0] = '\0';
				if((parep_mpi_reg_vals[i][k] == (address)NULL) || (parep_mpi_reg_vals[i][k] <= (address)(&(_end))) || (((address)parep_mpi_ext_heap_mapping <= parep_mpi_reg_vals[i][k]) && (((address)parep_mpi_ext_heap_mapping + parep_mpi_ext_heap_size) > parep_mpi_reg_vals[i][k])) || ((parep_mpi_new_stack_start <= parep_mpi_reg_vals[i][k]) && (parep_mpi_new_stack_end > parep_mpi_reg_vals[i][k]))) {
					offset = parep_mpi_reg_vals[i][k];
					parep_mpi_perform_write(ckpt_fd,mapname,MAX_PATH_LEN);
					parep_mpi_perform_write(ckpt_fd,&offset,sizeof(address));
				} else {
					bool found = false;
					for(int j = 0; j < num_mappings; j++) {
						if((mappings[j].start <= parep_mpi_reg_vals[i][k]) && (mappings[j].end > parep_mpi_reg_vals[i][k])) {
							if(mappings[j].pathname[0] == '\0') {
								while(mappings[j].pathname[0] == '\0') j--;
							}
							parep_mpi_strcpy(mapname,mappings[j].pathname);
							while(!parep_mpi_strcmp(mapname,mappings[j].pathname)) j--;
							j++;
							offset = parep_mpi_reg_vals[i][k] - mappings[j].start;
							found = true;
							break;
						}
					}
					if(!found) {
						mapname[0] = '\0';
						offset = parep_mpi_reg_vals[i][k];
					}
					parep_mpi_perform_write(ckpt_fd,mapname,MAX_PATH_LEN);
					parep_mpi_perform_write(ckpt_fd,&offset,sizeof(address));
				}
			}
		}
		
		int init_data_seg_size = (((char *)(&_edata)) - ((char *)(&__data_start)));
		int uinit_data_seg_size = (((char *)(&_end)) - ((char *)(&__bss_start)));
		
		parep_mpi_perform_write(ckpt_fd,&init_data_seg_size,sizeof(int));
		parep_mpi_perform_write(ckpt_fd,&uinit_data_seg_size,sizeof(int));
		
		parep_mpi_perform_write(ckpt_fd,&__data_start,init_data_seg_size);
		parep_mpi_perform_write(ckpt_fd,reqarr,1024*sizeof(MPI_Request));
		parep_mpi_perform_write(ckpt_fd,reqinuse,1024*sizeof(bool));
		if(parep_mpi_fortran_binding_used) parep_mpi_perform_write(ckpt_fd,&__bss_start,uinit_data_seg_size);
		
		parep_mpi_perform_write(ckpt_fd,&parep_mpi_ext_heap_size,sizeof(size_t));
		pthread_mutex_lock(&heap_free_list_mutex);
		parep_mpi_perform_write(ckpt_fd,parep_mpi_bins,sizeof(bin_t)*BIN_COUNT);
		parep_mpi_perform_write(ckpt_fd,parep_mpi_fastbins,sizeof(bin_t)*FASTBIN_COUNT);
		
		heap_node_t *headnode = (heap_node_t *)parep_mpi_ext_heap_mapping;
		footer_t *footnode;
		
		while((address)headnode < (address)(parep_mpi_ext_heap_mapping+parep_mpi_ext_heap_size)) {
			footnode = get_foot(headnode);
			parep_mpi_perform_write(ckpt_fd,headnode,sizeof(heap_node_t));
			if((!GET_HOLE(headnode->hole_and_size)) && (!GET_FAST(headnode->hole_and_size))) parep_mpi_perform_write(ckpt_fd,(void *)(((address)headnode) + sizeof(heap_node_t)),((address)footnode) - ((address)headnode) - sizeof(heap_node_t));
			parep_mpi_perform_write(ckpt_fd,footnode,sizeof(footer_t));
			headnode = (heap_node_t *)(((address)footnode) + sizeof(footer_t));
		}
		
		//parep_mpi_perform_write(ckpt_fd,parep_mpi_ext_heap_mapping,parep_mpi_ext_heap_size);
		pthread_mutex_unlock(&heap_free_list_mutex);
		
		parep_mpi_perform_write(ckpt_fd,&parep_mpi_new_stack_size,sizeof(size_t));
		parep_mpi_perform_write(ckpt_fd,(void *)parep_mpi_new_stack_start,parep_mpi_new_stack_size);

		parep_mpi_perform_write(ckpt_fd,&my_req_null,sizeof(address));
		parep_mpi_perform_write(ckpt_fd,&first_peertopeer,sizeof(ptpdata *));
		parep_mpi_perform_write(ckpt_fd,&last_peertopeer,sizeof(ptpdata *));
		parep_mpi_perform_write(ckpt_fd,&last_collective,sizeof(clcdata *));
		
		parep_mpi_perform_write(ckpt_fd,parep_mpi_commbuf_bins,sizeof(commbuf_bin_t)*COMMBUF_BIN_COUNT);
		
		parep_mpi_perform_write(ckpt_fd,&recvDataHead,sizeof(recvDataNode *));
		parep_mpi_perform_write(ckpt_fd,&recvDataTail,sizeof(recvDataNode *));
		parep_mpi_perform_write(ckpt_fd,&recvDataRedHead,sizeof(recvDataNode *));
		parep_mpi_perform_write(ckpt_fd,&recvDataRedTail,sizeof(recvDataNode *));
		parep_mpi_perform_write(ckpt_fd,&skipcmplist,sizeof(struct skiplist *));
		parep_mpi_perform_write(ckpt_fd,&skipreplist,sizeof(struct skiplist *));
		parep_mpi_perform_write(ckpt_fd,&skipredlist,sizeof(struct skiplist *));
		parep_mpi_perform_write(ckpt_fd,&reqHead,sizeof(reqNode *));
		parep_mpi_perform_write(ckpt_fd,&reqTail,sizeof(reqNode *));
		parep_mpi_perform_write(ckpt_fd,&openFileHead,sizeof(openFileNode *));
		parep_mpi_perform_write(ckpt_fd,&openFileTail,sizeof(openFileNode *));
		
		parep_mpi_perform_write(ckpt_fd,&parep_mpi_store_buf_sz,sizeof(long));
		
		parep_mpi_perform_write(ckpt_fd,&parep_mpi_sendid,sizeof(int));
		parep_mpi_perform_write(ckpt_fd,&parep_mpi_collective_id,sizeof(int));
		
		parep_mpi_close(ckpt_fd);
	}
}

NO_OPTIMIZE
static void restore_brk(address saved_brk, address restore_begin, address restore_end) {
	address current_brk, new_brk;
	current_brk = (address)parep_mpi_brk(NULL);
	if((current_brk > pstat.restore_start) && (saved_brk < pstat.restore_end)) {
		printf("current_brk %p, saved_brk %p, restore_begin %p, restore_end %p\n", current_brk,saved_brk,pstat.restore_start,pstat.restore_end);
		exit(1);
	}
	
	if(current_brk <= saved_brk) {
		new_brk = (address)parep_mpi_brk((void *)saved_brk);
		//int ret = parep_mpi_prctl(PR_SET_MM,PR_SET_MM_START_BRK,pstat.start_brk,0,0);
		pstat.cur_brk = (address)NULL;
	}
	else {
		new_brk = saved_brk;
		return;
	}
	if(new_brk == (address)-1) {
		printf("brk %p errno %d (bad heap\n)",saved_brk,errno);
		exit(1);
	}
	else if(new_brk > current_brk) {
		parep_mpi_munmap((void *)current_brk,new_brk-current_brk);
	}
}

NO_OPTIMIZE
static void *parep_mpi_memcpy(void *dstpp, const void *srcpp, size_t len) {
	char *dst = (char*) dstpp;
	const char *src = (const char*) srcpp;
	while(len > 0) {
		*dst++ = *src++;
		len--;
	}
	return dstpp;
}

NO_OPTIMIZE
static void unmap_memory_areas_and_restore_vdso(ProcessStat *procStat) {
	Mapping map;
	address vdsoStart = (address)NULL;
	address vdsoEnd = (address)NULL;
	address vvarStart = (address)NULL;
	address vvarEnd = (address)NULL;
	address vsyscallStart = (address)NULL;
	address vsyscallEnd = (address)NULL;
	
	for(int i = 0; i < num_mappings; i++) {
		parep_mpi_memcpy(&map,&(mappings[i]),sizeof(Mapping));
		if(((map.start >= prstat.restore_start) && (map.end <= prstat.restore_end)) || (i == prstat.related_map)) {
			unmap_addr[unmap_num][0] = map.start;
			unmap_addr[unmap_num][1] = map.end;
			unmap_num++;
		}
		else if(map.start == prstat.vdsoStart) {
			vdsoStart = map.start;
			vdsoEnd = map.end;
		}
		else if(map.start == prstat.vdsoStart) {
			vdsoStart = map.start;
			vdsoEnd = map.end;
		}
		else if(map.start == prstat.vvarStart) {
			vvarStart = map.start;
			vvarEnd = map.end;
		}
		else if(map.start == prstat.vsyscallStart) {
			vsyscallStart = map.start;
			vsyscallEnd = map.end;
		}
		else if(strStartsWith(map.pathname,"/SYSV")) {
		}
		else if(isNscdArea(&map)) {
		}
		else if(isIBShmArea(&map)) {
		}
		else if(map.flags & MAP_SHARED) {
		}
		else if((map.end - map.start) > 0) {
			parep_mpi_munmap((void *)map.start,map.end - map.start);
		}
	}
	
	void *stagingAddr = NULL;
	int stagingSize = 0;
	void *stagingVdsoStart = NULL;
	void *stagingVvarStart = NULL;
	if(vdsoStart != (address)NULL) {
		stagingSize += vdsoEnd - vdsoStart;
	}
	if(vvarStart != (address)NULL) {
		stagingSize += vvarEnd - vvarStart;
	}
	if(stagingSize > 0) {
		void *stagingAddrA = NULL;
		void *stagingAddrB = NULL;
		void *stagingAddrC = NULL;
		stagingAddrA = (void *)parep_mpi_mmap(NULL,3*stagingSize,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
		stagingAddr = stagingAddrA;
		if (doAreasOverlap2(stagingAddrA + stagingSize, stagingSize, (char *)procStat->vdsoStart, (char *)procStat->vdsoEnd, (char *)procStat->vvarStart, (char *)procStat->vvarEnd)) {
			stagingAddrB = (void *)parep_mpi_mmap(NULL, 3*stagingSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			stagingAddr = stagingAddrB;
    }
		if (doAreasOverlap2(stagingAddrB + stagingSize, stagingSize, (char *)procStat->vdsoStart, (char *)procStat->vdsoEnd, (char *)procStat->vvarStart, (char *)procStat->vvarEnd)) {
			stagingAddrC = (void *)parep_mpi_mmap(NULL, 3*stagingSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			stagingAddr = stagingAddrC;
    }
		if(stagingAddrA != NULL && stagingAddrA != stagingAddr) {
			parep_mpi_munmap(stagingAddrA,3*stagingSize);
		}
		if(stagingAddrB != NULL && stagingAddrB != stagingAddr) {
			parep_mpi_munmap(stagingAddrB,3*stagingSize);
		}
		
		stagingAddr = stagingAddr + stagingSize;
		parep_mpi_munmap(stagingAddr-stagingSize,stagingSize);
		parep_mpi_munmap(stagingAddr+stagingSize,stagingSize);
		
		stagingVdsoStart = stagingVvarStart = stagingAddr;
		if(vdsoStart != (address)NULL) {
			parep_mpi_mremap((void *)vdsoStart,vdsoEnd-vdsoStart,vdsoEnd-vdsoStart,MREMAP_FIXED|MREMAP_MAYMOVE,stagingVdsoStart);
			stagingVvarStart = (char *)stagingVdsoStart + (vdsoEnd - vdsoStart);
		}
		if(vvarStart != (address)NULL) {
			parep_mpi_mremap((void *)vvarStart,vvarEnd-vvarStart,vvarEnd-vvarStart,MREMAP_FIXED|MREMAP_MAYMOVE,stagingVvarStart);
		}
	}
	
	if(vvarStart != (address)NULL) {
		parep_mpi_mremap((void *)stagingVvarStart,vvarEnd-vvarStart,vvarEnd-vvarStart,MREMAP_FIXED|MREMAP_MAYMOVE,(void *)procStat->vvarStart);
#if defined(__i386__)
		void *vvar = parep_mpi_mmap(stagingVvarStart,vvarEnd-vvarStart,PROT_EXEC|PROT_WRITE|PROT_READ,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0);
		parep_mpi_memcpy(vvarStart,procStat->vvarStart,4096);
#endif
	}
	if(vdsoStart != (address)NULL) {
		parep_mpi_mremap((void *)stagingVdsoStart,vdsoEnd-vdsoStart,vdsoEnd-vdsoStart,MREMAP_FIXED|MREMAP_MAYMOVE,(void *)procStat->vdsoStart);
#if defined(__i386__)
		void *vdso = parep_mpi_mmap(stagingVdsoStart,vdsoEnd-vdsoStart,PROT_EXEC|PROT_WRITE|PROT_READ,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0);
		parep_mpi_memcpy(vdsoStart,procStat->vdsoStart,4096);
#endif
	}
	if(stagingSize > 0) {
		parep_mpi_munmap(stagingAddr,stagingSize);
	}
}

NO_OPTIMIZE
static int read_one_memory_area(int fd, address endOfStack) {
	int imagefd;
	void *mmappedat;
	Mapping map;
	bool map_to_temp = false;
	address targaddr;
	
	parep_mpi_perform_read(fd,&map,sizeof(map));
	if(map.start == (address)NULL) {
		return -1;
	}
	
	if ((map.pathname[0] && map.pathname[0] != '/' && parep_mpi_strstr(map.pathname, "stack")) || (map.end == endOfStack)) {
		map.flags = map.flags | MAP_GROWSDOWN;
  }
	
	if(map.flags & MAP_SHARED) {
		char mydir[256];
		char origdir[256];
		
		int targremap = -1;
		
		getDirectory(map.pathname,origdir);
		
		for(int i = 0; i < num_mappings; i++) {
			if(mappings[i].flags & MAP_SHARED) {
				if((mappings[i].end - mappings[i].start) == (map.end - map.start)) {
					getDirectory(mappings[i].pathname,mydir);
					if(!parep_mpi_strcmp(origdir,mydir)) {
						if(parep_mpi_strstr(mappings[i].pathname,"sm_segment") && parep_mpi_strstr(map.pathname,"sm_segment")) {
							targremap = i;
						}
						else if(parep_mpi_strstr(mappings[i].pathname,"slot_shmem") && parep_mpi_strstr(map.pathname,"slot_shmem")) {
							targremap = i;
						}
						else if(parep_mpi_strstr(mappings[i].pathname,"ib_shmem-coll-kvs") && parep_mpi_strstr(map.pathname,"ib_shmem-coll-kvs")) {
							targremap = i;
						}
						else if(parep_mpi_strstr(mappings[i].pathname,"ib_shmem_coll-kvs") && parep_mpi_strstr(map.pathname,"ib_shmem_coll-kvs")) {
							targremap = i;
						}
						else if(parep_mpi_strstr(mappings[i].pathname,"ib_shmem-kvs") && parep_mpi_strstr(map.pathname,"ib_shmem-kvs")) {
							targremap = i;
						}
						else if(strStartsWith(mappings[i].pathname,"/dev/infiniband/uverbs") && strStartsWith(map.pathname,"/dev/infiniband/uverbs")) {
							targremap = i;
						}
						if(targremap > 0) {
							if(!remap_mark[i]) {
								remap_mark[i] = true;
								break;
							}
							else targremap = -1;
						}
					}
					else if(strStartsWith(mappings[i].pathname,"/tmp/prte") && strStartsWith(map.pathname,"/tmp/prte")) {
						if(parep_mpi_strstr(mappings[i].pathname,"smdataseg") && parep_mpi_strstr(map.pathname,"smdataseg")) {
							targremap = i;
						}
						else if(parep_mpi_strstr(mappings[i].pathname,"smseg") && parep_mpi_strstr(map.pathname,"smseg")) {
							targremap = i;
						}
						else if(parep_mpi_strstr(mappings[i].pathname,"initial-pmix") && parep_mpi_strstr(map.pathname,"initial-pmix")) {
							targremap = i;
						}
						else if(parep_mpi_strstr(mappings[i].pathname,"smlockseg") && parep_mpi_strstr(map.pathname,"smlockseg")) {
							targremap = i;
						}
						else if(parep_mpi_strstr(mappings[i].pathname,"dstore_sm.lock") && parep_mpi_strstr(map.pathname,"dstore_sm.lock")) {
							targremap = i;
						}
						else if(parep_mpi_strstr(mappings[i].pathname,"hwloc") && parep_mpi_strstr(map.pathname,"hwloc")) {
							targremap = i;
						}
						else if(parep_mpi_strstr(mappings[i].pathname,"dvm") && parep_mpi_strstr(map.pathname,"dvm")) {
							targremap = i;
						}
						if(targremap > 0) {
							if(!remap_mark[i]) {
								remap_mark[i] = true;
								break;
							}
							else targremap = -1;
						}
					}
				}
			}
		}
		
		if(targremap > 0) {
			parep_mpi_mprotect((void *)mappings[targremap].start,mappings[targremap].end-mappings[targremap].start,mappings[targremap].prot|PROT_READ|PROT_WRITE);
			void *ret = parep_mpi_mremap((void *)mappings[targremap].start,mappings[targremap].end-mappings[targremap].start,map.end-map.start,MREMAP_FIXED|MREMAP_MAYMOVE,(void *)tempremapaddr);
			if((unsigned long)(ret) >= -4095UL) {
				parep_mpi_perform_write(1,"MREMAP FAILED\n",14);
				unsigned long err = (unsigned long)(ret);
				err = -err;
				if(err == EAGAIN) parep_mpi_perform_write(1,"ERR EAGAIN\n",11);
				if(err == EFAULT) {
					parep_mpi_perform_write(1,"ERR EFAULT\n",11);
					char oldmap[20];
					char oldsize[20];
					char newmap[20];
					char newsize[20];
					parep_mpi_ltoa((long)mappings[targremap].start,oldmap,16);
					parep_mpi_ltoa((long)(mappings[targremap].end - mappings[targremap].start),oldsize,16);
					parep_mpi_ltoa((long)tempremapaddr,newmap,16);
					parep_mpi_ltoa((long)(map.end - map.start),newsize,16);
					parep_mpi_perform_write(1,oldmap,parep_mpi_strlen(oldmap));
					parep_mpi_perform_write(1,oldsize,parep_mpi_strlen(oldsize));
					parep_mpi_perform_write(1,newmap,parep_mpi_strlen(newmap));
					parep_mpi_perform_write(1,newsize,parep_mpi_strlen(newsize));
				}
				if(err == EINVAL) parep_mpi_perform_write(1,"ERR EINVAL\n",11);
				if(err == ENOMEM) parep_mpi_perform_write(1,"ERR ENOMEM\n",11);
				while(1);
			}
			
			if(parep_mpi_strstr(map.pathname,"sm_segment")) {
				int rank_from_str = parep_mpi_atoi(parep_mpi_strrchr(map.pathname,'.') + 1);
				if(rank_from_str == parep_mpi_node_rank) {
					parep_mpi_perform_read(fd,(void *)tempremapaddr,map.end-map.start);
				}
			} else if(parep_mpi_strstr(map.pathname,"ib_shmem-kvs")) {
				int rem_original_rank, rem_original_node_rank;
				parep_mpi_perform_read(ckpt_fd,&rem_original_rank,sizeof(int));
				rem_original_node_rank = rem_original_rank % parep_mpi_node_size;
				address base_start = tempremapaddr + (sizeof(pid_t)*parep_mpi_node_size);
				address base_end = tempremapaddr + map.end - map.start;
				address start = base_start + (parep_mpi_node_rank * ((base_end - base_start) / parep_mpi_node_size));
				address end = base_start + ((parep_mpi_node_rank+1) * ((base_end - base_start) / parep_mpi_node_size));
				if(end > base_end) end = base_end;
				
				//parep_mpi_perform_read(fd,(void *)start,end-start);
				parep_mpi_memset((void *)(start+128),0,end-start-128);
				pid_t *pid_arr = (pid_t *)tempremapaddr;
				pid_arr[rem_original_node_rank] = parep_mpi_pid;
			}
			parep_mpi_mprotect((void *)tempremapaddr,map.end-map.start,map.prot);
			
			mappings[targremap].start = tempremapaddr;
			mappings[targremap].end = tempremapaddr + (map.end-map.start);
			remap_addr[remap_num][0] = tempremapaddr;
			remap_addr[remap_num][1] = (map.end-map.start);
			remap_addr[remap_num][2] = map.start;
			remap_num++;
			tempremapaddr += (map.end-map.start);
		}
		return 0;
	}
	
	if(strStartsWith(map.pathname,"/dev/zero (deleted)") || strStartsWith(map.pathname,"/dev/null (deleted)")) {
		if(!(map.prot & PROT_WRITE)) {
			parep_mpi_mprotect((void *)map.start,map.end-map.start,map.prot);
		}
	}
	else {
		imagefd = -1;
		if(map.pathname[0] == '/') {
			imagefd = parep_mpi_open(map.pathname,O_RDONLY,0);
			if(imagefd >= 0) {
				off_t curr_size = parep_mpi_lseek(imagefd,0,SEEK_END);
				if((curr_size < (map.offset + (map.end - map.start))) && (map.prot & PROT_WRITE)) {
					parep_mpi_close(imagefd);
					imagefd = -1;
					map.offset = 0;
					map.flags |= MAP_ANONYMOUS;
				}
			}
		}
		
		if((imagefd == -1) && (map.flags & MAP_PRIVATE)) {
			map.flags |= MAP_ANONYMOUS;
		}
		
		if((!((map.start >= pstat.restore_start) && (map.end <= pstat.restore_end))) && (!(map.start == pstat.restore_end))) {
			if(doAreasOverlap((char *)map.start,map.end-map.start,(char *)prstat.restore_start,prstat.restore_end-prstat.restore_start)) {
				parep_mpi_perform_write(1,"PRSTAT OVERLAP\n",15);
				if(map.pathname[0] != '\0') parep_mpi_perform_write(1,map.pathname,parep_mpi_strlen(map.pathname) + 1);
				map_to_temp = true;
			}
			if(!map_to_temp) {
				if(doAreasOverlap((char *)map.start,map.end-map.start,(char *)mappings[prstat.related_map].start,mappings[prstat.related_map].end-mappings[prstat.related_map].start)) {
					parep_mpi_perform_write(1,"RELMAP OVERLAP\n",15);
					if(map.pathname[0] != '\0') parep_mpi_perform_write(1,map.pathname,parep_mpi_strlen(map.pathname) + 1);
					map_to_temp = true;
				}
			}
			if(!map_to_temp) {
				if(doAreasOverlap((char *)map.start,map.end-map.start,(char *)newStack,TEMP_STACK_SIZE)) {
					parep_mpi_perform_write(1,"NSTACK OVERLAP\n",15);
					if(map.pathname[0] != '\0') parep_mpi_perform_write(1,map.pathname,parep_mpi_strlen(map.pathname) + 1);
					map_to_temp = true;
				}
			}
			if(!map_to_temp) {
				for(int i = 0; i < num_mappings; i++) {
					if(doAreasOverlap((char *)map.start,map.end-map.start,(char *)mappings[i].start,mappings[i].end-mappings[i].start)) {
						if(mappings[i].flags & MAP_SHARED || isIBShmArea(&(mappings[i]))) {
							parep_mpi_perform_write(1,"SHM MAP OVERLAP\n",16);
							if(map.pathname[0] != '\0') parep_mpi_perform_write(1,map.pathname,parep_mpi_strlen(map.pathname) + 1);
							map_to_temp = true;
							break;
						}
					}
				}
			}
		} else {
			if(doAreasOverlap((char *)map.start,map.end-map.start,(char *)prstat.restore_start,prstat.restore_end-prstat.restore_start)) {
				parep_mpi_perform_write(1,"PRSTAT RES OVERLAP\n",19);
				if(map.pathname[0] != '\0') parep_mpi_perform_write(1,map.pathname,parep_mpi_strlen(map.pathname) + 1);
				map_to_temp = true;
			}
			if(!map_to_temp) {
				if(doAreasOverlap((char *)map.start,map.end-map.start,(char *)mappings[prstat.related_map].start,mappings[prstat.related_map].end-mappings[prstat.related_map].start)) {
					parep_mpi_perform_write(1,"RELMAP RES OVERLAP\n",19);
					if(map.pathname[0] != '\0') parep_mpi_perform_write(1,map.pathname,parep_mpi_strlen(map.pathname) + 1);
					map_to_temp = true;
				}
			}
			if(!map_to_temp) {
				if(doAreasOverlap((char *)map.start,map.end-map.start,(char *)newStack,TEMP_STACK_SIZE)) {
					parep_mpi_perform_write(1,"NSTACK RES OVERLAP\n",19);
					if(map.pathname[0] != '\0') parep_mpi_perform_write(1,map.pathname,parep_mpi_strlen(map.pathname) + 1);
					map_to_temp = true;
				}
			}
			if(!map_to_temp) {
				for(int i = 0; i < num_mappings; i++) {
					if(doAreasOverlap((char *)map.start,map.end-map.start,(char *)mappings[i].start,mappings[i].end-mappings[i].start)) {
						if(mappings[i].flags & MAP_SHARED || isIBShmArea(&(mappings[i]))) {
							parep_mpi_perform_write(1,"SHM MAP RES OVERLAP\n",20);
							if(map.pathname[0] != '\0') parep_mpi_perform_write(1,map.pathname,parep_mpi_strlen(map.pathname) + 1);
							map_to_temp = true;
							break;
						}
					}
				}
			}
		}
		
		targaddr = map.start;
		if(map_to_temp) {
			targaddr = tempremapaddr;
		}
		
		
		mmappedat = (void *)parep_mpi_mmap((void *)targaddr,map.end-map.start,map.prot|PROT_WRITE,map.flags|MAP_FIXED,imagefd,map.offset);
		if((unsigned long)(mmappedat) >= -4095UL) {
			parep_mpi_perform_write(1,"MMAP FAILED\n",12);
			unsigned long err = (unsigned long)(mmappedat);
			err = -err;
			if(err == EACCES) parep_mpi_perform_write(1,"ERR EACCES\n",11);
			if(err == EAGAIN) parep_mpi_perform_write(1,"ERR EAGAIN\n",11);
			if(err == EBADF) parep_mpi_perform_write(1,"ERR EBADF\n",10);
			if(err == EEXIST) parep_mpi_perform_write(1,"ERR EEXIST\n",11);
			if(err == ENFILE) parep_mpi_perform_write(1,"ERR ENFILE\n",11);
			if(err == ENODEV) parep_mpi_perform_write(1,"ERR ENODEV\n",11);
			if(err == EPERM) parep_mpi_perform_write(1,"ERR EPERM\n",10);
			if(err == EFAULT) parep_mpi_perform_write(1,"ERR EFAULT\n",11);
			if(err == EINVAL) parep_mpi_perform_write(1,"ERR EINVAL\n",11);
			if(err == ENOMEM) parep_mpi_perform_write(1,"ERR ENOMEM\n",11);
			while(1);
		}
		
		if(imagefd >= 0) {
			parep_mpi_close(imagefd);
		}
		
		if((map.wrtSize != (long)-1) && (map.pathname[0] == '/')) {
			if((!(map.prot & PROT_EXEC)) && (map.prot != PROT_NONE)) {
				parep_mpi_perform_read(fd,(void *)targaddr,map.wrtSize);
			}
		}
		else {
			parep_mpi_perform_read(fd,(void *)targaddr,map.end-map.start);
		}
		
		if(map_to_temp) {
			parep_mpi_perform_write(1,"MARKING FOR REMAP\n",18);
			if(map.pathname[0] != '\0') parep_mpi_perform_write(1,map.pathname,parep_mpi_strlen(map.pathname) + 1);
			remap_addr[remap_num][0] = tempremapaddr;
			remap_addr[remap_num][1] = (map.end-map.start);
			remap_addr[remap_num][2] = map.start;
			remap_num++;
			tempremapaddr += (map.end-map.start);
		}
		
		if(!(map.prot & PROT_WRITE)) {
			parep_mpi_mprotect((void *)targaddr,map.end-map.start,map.prot);
		}
	}
	return 0;
}

NO_OPTIMIZE
static void readmemoryareas(int fd, address endOfStack) {
	for(int i = 0; i < num_mappings; i++) {
		remap_mark[i] = false;
	}
	while(1) {
		if(read_one_memory_area(fd,endOfStack) == -1) {
			break;
		}
	}
#if defined(__arm__) || defined(__aarch64__)
	WMB;
#endif
}

NO_OPTIMIZE
void read_heap_and_stack(char *file_name) {
	char file[100];
	extern address __data_start;
	extern address _edata;
	extern address __bss_start;
	extern address _end;
	address offset;
	address fsbasekey;
	address curfsbase;
	address src_req_null;
	char mapname[256];
	ptpdata *pdata;
	clcdata *cdata;
	heap_node_t *headnode;
	footer_t *footnode;
	strcpy(file,file_name);
	ckpt_fd = parep_mpi_open(file,O_RDONLY,0777);
	
	size_t hpsize,stksize;
	int init_data_seg_size;
	int uinit_data_seg_size;
	
	address temprip,temprsp,temprbp;
	temprip = getPC(parep_mpi_replication_initializer);
	
	Context processContext;
	parep_mpi_perform_read(ckpt_fd,&fsbasekey,sizeof(address));
	parep_mpi_perform_read(ckpt_fd,&processContext,sizeof(Context));
	parep_mpi_perform_read(ckpt_fd,&parep_mpi_replication_initializer,sizeof(jmp_buf));
	
	parep_mpi_perform_read(ckpt_fd,&parep_mpi_num_reg_vals,sizeof(int));
	
	for(int i = 0; i < parep_mpi_num_reg_vals; i++) {
		for(int k = 0; k < 15; k++) {
			parep_mpi_perform_read(ckpt_fd,mapname,MAX_PATH_LEN);
			parep_mpi_perform_read(ckpt_fd,&offset,sizeof(address));
			if(mapname[0] == '\0') parep_mpi_reg_vals[i][k] = offset;
			else {
				for(int j = 0; j < num_mappings; j++) {
					if((!parep_mpi_strcmp(mappings[j].pathname,mapname)) || (parep_mpi_strstr(mappings[j].pathname,"ib_shmem-kvs") && parep_mpi_strstr(mapname,"ib_shmem-kvs"))) {
						parep_mpi_reg_vals[i][k] = offset + mappings[j].start;
						break;
					}
				}
			}
		}
	}
	
	struct mpi_ft_comm mpi_ft_comm_dup[3];
	struct mpi_ft_datatype mpi_ft_datatype_dup[64];
	struct mpi_ft_request mpi_ft_request_dup[1];
	struct mpi_ft_op mpi_ft_op_dup[32];
	
	address stdout_dup = (address)stdout;
	char _ZSt4cout_buf[0x110];
	memcpy(_ZSt4cout_buf,&_ZSt4cout,0x110);
	
	memcpy(&mpi_ft_comm_dup[0],&mpi_ft_comm_null,sizeof(struct mpi_ft_comm));
	memcpy(&mpi_ft_comm_dup[1],&mpi_ft_comm_world,sizeof(struct mpi_ft_comm));
	memcpy(&mpi_ft_comm_dup[2],&mpi_ft_comm_self,sizeof(struct mpi_ft_comm));
	
	memcpy(&mpi_ft_datatype_dup[0],&mpi_ft_datatype_null,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[1],&mpi_ft_datatype_char,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[2],&mpi_ft_datatype_signed_char,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[3],&mpi_ft_datatype_unsigned_char,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[4],&mpi_ft_datatype_byte,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[5],&mpi_ft_datatype_short,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[6],&mpi_ft_datatype_unsigned_short,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[7],&mpi_ft_datatype_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[8],&mpi_ft_datatype_unsigned,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[9],&mpi_ft_datatype_long,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[10],&mpi_ft_datatype_unsigned_long,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[11],&mpi_ft_datatype_float,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[12],&mpi_ft_datatype_double,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[13],&mpi_ft_datatype_long_double,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[14],&mpi_ft_datatype_long_long_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[15],&mpi_ft_datatype_unsigned_long_long,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[16],&mpi_ft_datatype_packed,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[17],&mpi_ft_datatype_float_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[18],&mpi_ft_datatype_double_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[19],&mpi_ft_datatype_long_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[20],&mpi_ft_datatype_short_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[21],&mpi_ft_datatype_2int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[22],&mpi_ft_datatype_long_double_int,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[23],&mpi_ft_datatype_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[24],&mpi_ft_datatype_double_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[25],&mpi_ft_datatype_logical,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[26],&mpi_ft_datatype_real,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[27],&mpi_ft_datatype_double_precision,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[28],&mpi_ft_datatype_integer,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[29],&mpi_ft_datatype_2integer,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[30],&mpi_ft_datatype_2real,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[31],&mpi_ft_datatype_2double_precision,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[32],&mpi_ft_datatype_character,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[33],&mpi_ft_datatype_int8_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[34],&mpi_ft_datatype_int16_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[35],&mpi_ft_datatype_int32_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[36],&mpi_ft_datatype_int64_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[37],&mpi_ft_datatype_uint8_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[38],&mpi_ft_datatype_uint16_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[39],&mpi_ft_datatype_uint32_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[40],&mpi_ft_datatype_uint64_t,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[41],&mpi_ft_datatype_c_bool,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[42],&mpi_ft_datatype_c_float_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[43],&mpi_ft_datatype_c_double_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[44],&mpi_ft_datatype_c_long_double_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[45],&mpi_ft_datatype_aint,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[46],&mpi_ft_datatype_offset,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[47],&mpi_ft_datatype_count,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[48],&mpi_ft_datatype_cxx_bool,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[49],&mpi_ft_datatype_cxx_float_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[50],&mpi_ft_datatype_cxx_double_complex,sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_dup[51],&mpi_ft_datatype_cxx_long_double_complex,sizeof(struct mpi_ft_datatype));
	
	memcpy(&mpi_ft_request_dup[0],&mpi_ft_request_null,sizeof(struct mpi_ft_request));
	
	memcpy(&mpi_ft_op_dup[0],&mpi_ft_op_null,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[1],&mpi_ft_op_max,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[2],&mpi_ft_op_min,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[3],&mpi_ft_op_sum,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[4],&mpi_ft_op_prod,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[5],&mpi_ft_op_land,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[6],&mpi_ft_op_band,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[7],&mpi_ft_op_lor,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[8],&mpi_ft_op_bor,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[9],&mpi_ft_op_lxor,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[10],&mpi_ft_op_bxor,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[11],&mpi_ft_op_minloc,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[12],&mpi_ft_op_maxloc,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[13],&mpi_ft_op_replace,sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_dup[14],&mpi_ft_op_no_op,sizeof(struct mpi_ft_op));
		
	parep_mpi_perform_read(ckpt_fd,&init_data_seg_size,sizeof(int));
	parep_mpi_perform_read(ckpt_fd,&uinit_data_seg_size,sizeof(int));
	
	parep_mpi_perform_read(ckpt_fd,&__data_start,init_data_seg_size);
	parep_mpi_perform_read(ckpt_fd,reqarr,1024*sizeof(MPI_Request));
	parep_mpi_perform_read(ckpt_fd,reqinuse,1024*sizeof(bool));
	if(parep_mpi_fortran_binding_used) parep_mpi_perform_read(ckpt_fd,&__bss_start,uinit_data_seg_size);
	
	stdout = (FILE *)stdout_dup;
	memcpy(&_ZSt4cout,_ZSt4cout_buf,0x110);
	
	memcpy(&mpi_ft_comm_null,&mpi_ft_comm_dup[0],sizeof(struct mpi_ft_comm));
	memcpy(&mpi_ft_comm_world,&mpi_ft_comm_dup[1],sizeof(struct mpi_ft_comm));
	memcpy(&mpi_ft_comm_self,&mpi_ft_comm_dup[2],sizeof(struct mpi_ft_comm));

	memcpy(&mpi_ft_datatype_null,&mpi_ft_datatype_dup[0],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_char,&mpi_ft_datatype_dup[1],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_signed_char,&mpi_ft_datatype_dup[2],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_unsigned_char,&mpi_ft_datatype_dup[3],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_byte,&mpi_ft_datatype_dup[4],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_short,&mpi_ft_datatype_dup[5],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_unsigned_short,&mpi_ft_datatype_dup[6],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_int,&mpi_ft_datatype_dup[7],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_unsigned,&mpi_ft_datatype_dup[8],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_long,&mpi_ft_datatype_dup[9],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_unsigned_long,&mpi_ft_datatype_dup[10],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_float,&mpi_ft_datatype_dup[11],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_double,&mpi_ft_datatype_dup[12],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_long_double,&mpi_ft_datatype_dup[13],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_long_long_int,&mpi_ft_datatype_dup[14],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_unsigned_long_long,&mpi_ft_datatype_dup[15],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_packed,&mpi_ft_datatype_dup[16],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_float_int,&mpi_ft_datatype_dup[17],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_double_int,&mpi_ft_datatype_dup[18],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_long_int,&mpi_ft_datatype_dup[19],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_short_int,&mpi_ft_datatype_dup[20],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_2int,&mpi_ft_datatype_dup[21],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_long_double_int,&mpi_ft_datatype_dup[22],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_complex,&mpi_ft_datatype_dup[23],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_double_complex,&mpi_ft_datatype_dup[24],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_logical,&mpi_ft_datatype_dup[25],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_real,&mpi_ft_datatype_dup[26],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_double_precision,&mpi_ft_datatype_dup[27],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_integer,&mpi_ft_datatype_dup[28],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_2integer,&mpi_ft_datatype_dup[29],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_2real,&mpi_ft_datatype_dup[30],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_2double_precision,&mpi_ft_datatype_dup[31],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_character,&mpi_ft_datatype_dup[32],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_int8_t,&mpi_ft_datatype_dup[33],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_int16_t,&mpi_ft_datatype_dup[34],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_int32_t,&mpi_ft_datatype_dup[35],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_int64_t,&mpi_ft_datatype_dup[36],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_uint8_t,&mpi_ft_datatype_dup[37],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_uint16_t,&mpi_ft_datatype_dup[38],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_uint32_t,&mpi_ft_datatype_dup[39],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_uint64_t,&mpi_ft_datatype_dup[40],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_c_bool,&mpi_ft_datatype_dup[41],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_c_float_complex,&mpi_ft_datatype_dup[42],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_c_double_complex,&mpi_ft_datatype_dup[43],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_c_long_double_complex,&mpi_ft_datatype_dup[44],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_aint,&mpi_ft_datatype_dup[45],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_offset,&mpi_ft_datatype_dup[46],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_count,&mpi_ft_datatype_dup[47],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_cxx_bool,&mpi_ft_datatype_dup[48],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_cxx_float_complex,&mpi_ft_datatype_dup[49],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_cxx_double_complex,&mpi_ft_datatype_dup[50],sizeof(struct mpi_ft_datatype));
	memcpy(&mpi_ft_datatype_cxx_long_double_complex,&mpi_ft_datatype_dup[51],sizeof(struct mpi_ft_datatype));
	
	memcpy(&mpi_ft_request_null,&mpi_ft_request_dup[0],sizeof(struct mpi_ft_request));
	
	memcpy(&mpi_ft_op_null,&mpi_ft_op_dup[0],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_max,&mpi_ft_op_dup[1],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_min,&mpi_ft_op_dup[2],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_sum,&mpi_ft_op_dup[3],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_prod,&mpi_ft_op_dup[4],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_land,&mpi_ft_op_dup[5],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_band,&mpi_ft_op_dup[6],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_lor,&mpi_ft_op_dup[7],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_bor,&mpi_ft_op_dup[8],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_lxor,&mpi_ft_op_dup[9],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_bxor,&mpi_ft_op_dup[10],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_minloc,&mpi_ft_op_dup[11],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_maxloc,&mpi_ft_op_dup[12],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_replace,&mpi_ft_op_dup[13],sizeof(struct mpi_ft_op));
	memcpy(&mpi_ft_op_no_op,&mpi_ft_op_dup[14],sizeof(struct mpi_ft_op));
	
	parep_mpi_perform_read(ckpt_fd,&hpsize,sizeof(size_t));
	if(hpsize != parep_mpi_ext_heap_size) {
		assert((hpsize > parep_mpi_ext_heap_size) && (hpsize % parep_mpi_ext_heap_size == 0));
		expand_heap(hpsize - parep_mpi_ext_heap_size);
	}
	
	pthread_mutex_lock(&heap_free_list_mutex);
	parep_mpi_perform_read(ckpt_fd,parep_mpi_bins,sizeof(bin_t)*BIN_COUNT);
	parep_mpi_perform_read(ckpt_fd,parep_mpi_fastbins,sizeof(bin_t)*FASTBIN_COUNT);
	
	headnode = (heap_node_t *)parep_mpi_ext_heap_mapping;
	while((address)headnode < (address)(parep_mpi_ext_heap_mapping+parep_mpi_ext_heap_size)) {
		parep_mpi_perform_read(ckpt_fd,headnode,sizeof(heap_node_t));
		footnode = get_foot(headnode);
		if((!GET_HOLE(headnode->hole_and_size)) && (!GET_FAST(headnode->hole_and_size)) && (GET_SIZE(headnode->hole_and_size) > 0)) parep_mpi_perform_read(ckpt_fd,(void *)(((address)headnode) + sizeof(heap_node_t)),((address)footnode) - ((address)headnode) - sizeof(heap_node_t));
		else {
			if((GET_SIZE(headnode->hole_and_size) > 0)) assert((((address)headnode + GET_SIZE(headnode->hole_and_size) + sizeof(heap_node_t)) == (address)footnode));
			else assert((GET_SIZE(headnode->hole_and_size) == 0) && (((address)headnode + sizeof(heap_node_t)) == (address)footnode));
		}
		parep_mpi_perform_read(ckpt_fd,footnode,sizeof(footer_t));
		headnode = (heap_node_t *)(((address)footnode) + sizeof(footer_t));
	}
	
	//parep_mpi_perform_read(ckpt_fd,parep_mpi_ext_heap_mapping,hpsize);
	pthread_mutex_unlock(&heap_free_list_mutex);
	
	parep_mpi_perform_read(ckpt_fd,&stksize,sizeof(size_t));
	assert(stksize == parep_mpi_new_stack_size);
	parep_mpi_perform_read(ckpt_fd,(void *)parep_mpi_new_stack_start,stksize);
	
	parep_mpi_perform_read(ckpt_fd,&src_req_null,sizeof(address));
	parep_mpi_perform_read(ckpt_fd,&first_peertopeer,sizeof(ptpdata *));
	parep_mpi_perform_read(ckpt_fd,&last_peertopeer,sizeof(ptpdata *));
	parep_mpi_perform_read(ckpt_fd,&last_collective,sizeof(clcdata *));
	
	parep_mpi_perform_read(ckpt_fd,parep_mpi_commbuf_bins,sizeof(commbuf_bin_t)*COMMBUF_BIN_COUNT);
	
	parep_mpi_perform_read(ckpt_fd,&recvDataHead,sizeof(recvDataNode *));
	parep_mpi_perform_read(ckpt_fd,&recvDataTail,sizeof(recvDataNode *));
	parep_mpi_perform_read(ckpt_fd,&recvDataRedHead,sizeof(recvDataNode *));
	parep_mpi_perform_read(ckpt_fd,&recvDataRedTail,sizeof(recvDataNode *));
	parep_mpi_perform_read(ckpt_fd,&skipcmplist,sizeof(struct skiplist *));
	parep_mpi_perform_read(ckpt_fd,&skipreplist,sizeof(struct skiplist *));
	parep_mpi_perform_read(ckpt_fd,&skipredlist,sizeof(struct skiplist *));
	parep_mpi_perform_read(ckpt_fd,&reqHead,sizeof(reqNode *));
	parep_mpi_perform_read(ckpt_fd,&reqTail,sizeof(reqNode *));
	parep_mpi_perform_read(ckpt_fd,&openFileHead,sizeof(openFileNode *));
	parep_mpi_perform_read(ckpt_fd,&openFileTail,sizeof(openFileNode *));
	
	parep_mpi_perform_read(ckpt_fd,&parep_mpi_store_buf_sz,sizeof(long));
	
	parep_mpi_perform_read(ckpt_fd,&parep_mpi_sendid,sizeof(int));
	parep_mpi_perform_read(ckpt_fd,&parep_mpi_collective_id,sizeof(int));
	
	parep_mpi_close(ckpt_fd);
	
	pdata = first_peertopeer;
	while(pdata != NULL) {
		if(pdata->req != NULL) {
			if(*((address *)(pdata->req)) == src_req_null) *(pdata->req) = MPI_REQUEST_NULL;
		}
		pdata = pdata->prev;
	}
	
	cdata = last_collective;
	while(cdata != NULL) {
		if(cdata->req != NULL) {
			if(*((address *)(cdata->req)) == src_req_null) *(cdata->req) = MPI_REQUEST_NULL;
		}
		cdata = cdata->next;
	}
	
	temprsp = processContext.rsp;
	temprbp = processContext.rbp;
	
	PTR_ENCRYPT(temprip);
	parep_mpi_replication_initializer[0].__jmpbuf[JB_PC] = temprip;
	PTR_ENCRYPT(temprsp);
	parep_mpi_replication_initializer[0].__jmpbuf[JB_RSP] = temprsp;
	PTR_ENCRYPT(temprbp);
	parep_mpi_replication_initializer[0].__jmpbuf[JB_RBP] = temprbp;
	
	//asm volatile("\t rdfsbase %0" : "=r"(curfsbase));
	parep_mpi_arch_prctl(ARCH_GET_FS,(unsigned long)(&curfsbase));
	//syscall(SYS_arch_prctl,ARCH_GET_FS,&curfsbase);
	*((address *)(curfsbase+0x28)) = fsbasekey;
	
	IMB;
}

NO_OPTIMIZE
static void remap_and_cleanup() {
	long tid_index;
	void *rem_fsbase;
	asm volatile("mov %%rdi, %0" : "=r"(tid_index) : : "rdi","memory");
	asm volatile("mov %%rsi, %0" : "=r"(rem_fsbase) : : "rsi","memory");
	if(tid_index == 0) {
		for(int i = 1; i < num_threads; i++) {
			while(reached_temp_futex[i] == 0) {
				if(__sync_bool_compare_and_swap(&reached_temp_futex[i],1,1)) continue;
					asm volatile ("mov %1, %%rax\n\t"
												"mov %2, %%rdi\n\t"
												"mov %3, %%rsi\n\t"
												"mov %4, %%rdx\n\t"
												"mov %5, %%r10\n\t"
												"mov %6, %%r8\n\t"
												"mov %7, %%r9\n\t"
												"syscall\n\t"
												"mov %%rax, %0"
												: "=r" (reached_temp_ret[i])
												: "i" (SYS_futex),
												"r" (&reached_temp_futex[i]),
												"i" (FUTEX_WAIT),
												"r" ((long)0),
												"r" ((long)NULL),
												"r" ((long)NULL),
												"r" ((long)0)
												: "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory");
			}
		}
		
		parep_mpi_arch_prctl(ARCH_SET_FS,(unsigned long)(rem_fsbase));
		
		address temprip,temprsp,temprbp;
		temprip = processContext.rip;
		temprsp = processContext.rsp;
		temprbp = processContext.rbp;
		
		PTR_ENCRYPT(temprip);
		context[0].__jmpbuf[JB_PC] = temprip;
		PTR_ENCRYPT(temprsp);
		context[0].__jmpbuf[JB_RSP] = temprsp;
		PTR_ENCRYPT(temprbp);
		context[0].__jmpbuf[JB_RBP] = temprbp;
		
		IMB;
		
		newStack = (char *)(unmap_addr[0][0]);
		long unmapret;
		for(int i = 1; i < unmap_num; i++) {
			if((unmap_addr[i][1] - unmap_addr[i][0]) == 0) {
				parep_mpi_perform_write(1,"UNMAP SIZE 0\n",13);
			}
			unmapret = parep_mpi_munmap((void *)(unmap_addr[i][0]),unmap_addr[i][1]-unmap_addr[i][0]);
			if((unsigned long)(unmapret) >= -4095UL) {
				parep_mpi_perform_write(1,"MUNMAP TEMP FAILED\n",19);
				unsigned long err = (unsigned long)(unmapret);
				err = -err;
				if(err == EINVAL) parep_mpi_perform_write(1,"ERR EINVAL\n",11);
				char address[20];
				parep_mpi_ltoa((long)unmap_addr[i][0],address,16);
				parep_mpi_perform_write(1,address,parep_mpi_strlen(address));
				while(1);
			}
		}
		unmapret = parep_mpi_munmap(newStack,TEMP_STACK_SIZE);
		if((unsigned long)(unmapret) >= -4095UL) {
			parep_mpi_perform_write(1,"MUNMAP NTMP FAILED\n",19);
			unsigned long err = (unsigned long)(unmapret);
			err = -err;
			if(err == EINVAL) parep_mpi_perform_write(1,"ERR EINVAL\n",11);
			char address[20];
			parep_mpi_ltoa((long)newStack,address,16);
			parep_mpi_perform_write(1,address,parep_mpi_strlen(address));
			while(1);
		}
		
		for(int i = 0; i < remap_num; i++) {
			void *ret = parep_mpi_mremap((void *)remap_addr[i][0],remap_addr[i][1],remap_addr[i][1],MREMAP_FIXED|MREMAP_MAYMOVE,(void *)remap_addr[i][2]);
			if((unsigned long)(ret) >= -4095UL) {
				parep_mpi_perform_write(1,"MREMAP TEMP FAILED\n",19);
				unsigned long err = (unsigned long)(ret);
				err = -err;
				if(err == EAGAIN) parep_mpi_perform_write(1,"ERR EAGAIN\n",11);
				if(err == EFAULT) parep_mpi_perform_write(1,"ERR EFAULT\n",11);
				if(err == EINVAL) parep_mpi_perform_write(1,"ERR EINVAL\n",11);
				if(err == ENOMEM) parep_mpi_perform_write(1,"ERR ENOMEM\n",11);
				while(1);
			}
		}
		
		*((char **)(((address)(&tempMap)) - (address)tempMap + pstat.restore_start)) = tempMap;
		
		*((int *)(((address)(&___ckpt_counter)) - (address)tempMap + pstat.restore_start)) = ___ckpt_counter;
		
		*((int *)(((address)(&parep_mpi_pmi_fd)) - (address)tempMap + pstat.restore_start)) = parep_mpi_pmi_fd;
		
		*((double *)(((address)(&mpi_ft_ckpt_start_time)) - (address)tempMap + pstat.restore_start)) = mpi_ft_ckpt_start_time;
		
		parep_mpi_memcpy((void *)(((address)(parep_mpi_kvs_name)) -  (address)tempMap + pstat.restore_start),parep_mpi_kvs_name,256);
		
		*((int *)(((address)(&num_socks)) - (address)tempMap + pstat.restore_start)) = num_socks;
		for(int i = 0; i < num_socks; i++) {
			*((SockInfo *)(((address)(&sockinfo[i])) - (address)tempMap + pstat.restore_start)) = sockinfo[i];
		}
		
		for(int i = 0; i < num_threads; i++) {
			*((pid_t *)(((address)(&kernel_tid[i])) - (address)tempMap + pstat.restore_start)) = kernel_tid[i];
		}
		
		if(__sync_bool_compare_and_swap(&writing_ckpt_futex,0,1)) {
			asm volatile ("mov %1, %%rax\n\t"
										"mov %2, %%rdi\n\t"
										"mov %3, %%rsi\n\t"
										"mov %4, %%rdx\n\t"
										"mov %5, %%r10\n\t"
										"mov %6, %%r8\n\t"
										"mov %7, %%r9\n\t"
										"syscall\n\t"
										"mov %%rax, %0"
										: "=r" (writing_ckpt_ret)
										: "i" (SYS_futex),
										"r" (&writing_ckpt_futex),
										"i" (FUTEX_WAKE),
										"r" ((long)INT_MAX),
										"r" ((long)NULL),
										"r" ((long)NULL),
										"r" ((long)0)
										: "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory");
		}
		parep_mpi_longjmp(context,3);
	} else {
		if(__sync_bool_compare_and_swap(&reached_temp_futex[tid_index],0,1)) {
			asm volatile ("mov %1, %%rax\n\t"
										"mov %2, %%rdi\n\t"
										"mov %3, %%rsi\n\t"
										"mov %4, %%rdx\n\t"
										"mov %5, %%r10\n\t"
										"mov %6, %%r8\n\t"
										"mov %7, %%r9\n\t"
										"syscall\n\t"
										"mov %%rax, %0"
										: "=r" (reached_temp_ret[tid_index])
										: "i" (SYS_futex),
										"r" (&reached_temp_futex[tid_index]),
										"i" (FUTEX_WAKE),
										"r" ((long)1),
										"r" ((long)NULL),
										"r" ((long)NULL),
										"r" ((long)0)
										: "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory");
		}
		while(writing_ckpt_futex == 0) {
			if(__sync_bool_compare_and_swap(&writing_ckpt_futex,1,1)) continue;
			asm volatile ("mov %1, %%rax\n\t"
										"mov %2, %%rdi\n\t"
										"mov %3, %%rsi\n\t"
										"mov %4, %%rdx\n\t"
										"mov %5, %%r10\n\t"
										"mov %6, %%r8\n\t"
										"mov %7, %%r9\n\t"
										"syscall\n\t"
										"mov %%rax, %0"
										: "=r" (writing_ckpt_ret)
										: "i" (SYS_futex),
										"r" (&writing_ckpt_futex),
										"i" (FUTEX_WAIT),
										"r" ((long)0),
										"r" ((long)NULL),
										"r" ((long)NULL),
										"r" ((long)0)
										: "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory");
		}
		
		parep_mpi_arch_prctl(ARCH_SET_FS,(unsigned long)(rem_fsbase));
		
		address temprip,temprsp,temprbp;
		temprip = threadprocContext[tid_index].rip;
		temprsp = threadprocContext[tid_index].rsp;
		temprbp = threadprocContext[tid_index].rbp;

		PTR_ENCRYPT(temprip);
		threadContext[tid_index][0].__jmpbuf[JB_PC] = temprip;
		PTR_ENCRYPT(temprsp);
		threadContext[tid_index][0].__jmpbuf[JB_RSP] = temprsp;
		PTR_ENCRYPT(temprbp);
		threadContext[tid_index][0].__jmpbuf[JB_RBP] = temprbp;

		IMB;
		parep_mpi_longjmp(threadContext[tid_index],2);
	}
}

NO_OPTIMIZE
void parep_mpi_thread_ckpt_wait(int tindex, int restore) {
	int tid_index = tindex;
	int res = restore;
	char check;
	long ret;
	char *jmpstackaddr;
	address cpc;
	address temprip,temprsp,temprbp;
	threadprocContext[tid_index].rip = getPC(threadContext[tid_index]);
	threadprocContext[tid_index].rsp = getRSP(threadContext[tid_index]);
	threadprocContext[tid_index].rbp = getRBP(threadContext[tid_index]);
	
	//asm volatile("\t rdfsbase %0" : "=r"(threadfsbase[tid_index]));
	asm volatile ("mov %1, %%rax\n\t"
									"mov %2, %%rdi\n\t"
									"mov %3, %%rsi\n\t"
									"syscall\n\t"
									"mov %%rax, %0"
									: "=r" (ret)
									: "i" (SYS_arch_prctl),
									"i" (ARCH_GET_FS),
									"r" (&threadfsbase[tid_index])
									: "rax", "rdi", "rsi", "rcx", "r11", "memory");
	//syscall(SYS_arch_prctl,ARCH_GET_FS,&threadfsbase[tid_index]);
	
	pthread_mutex_lock(&threadcontext_write_mutex);
	threadcontext_write_ready[tid_index] = true;
	pthread_cond_signal(&threadcontext_write_cond_var);
	pthread_mutex_unlock(&threadcontext_write_mutex);

	while(writing_ckpt_futex == 1) {
		//asm volatile ("lock cmpxchg %3, %0; setz %1" : "+m"(writing_ckpt_futex), "=q"(check) : "a"(0), "r"(0) : "memory", "rax");
		if(__sync_bool_compare_and_swap(&writing_ckpt_futex,0,0)) continue;
		asm volatile ("mov %1, %%rax\n\t"
									"mov %2, %%rdi\n\t"
									"mov %3, %%rsi\n\t"
									"mov %4, %%rdx\n\t"
									"mov %5, %%r10\n\t"
									"mov %6, %%r8\n\t"
									"mov %7, %%r9\n\t"
									"syscall\n\t"
									"mov %%rax, %0"
									: "=r" (ret)
									: "i" (SYS_futex),
									"r" (&writing_ckpt_futex),
									"i" (FUTEX_WAIT),
									"r" ((long)1),
									"r" ((long)NULL),
									"r" ((long)NULL),
									"r" ((long)0)
									: "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory");
	}
	
	if(res) {
		jmpstackaddr = (char *)(((address)(tempStack)) + (TEMP_STACK_SIZE/2) - (tid_index*TEMP_STACK_SIZE/20) - TEMP_STACK_BOTTOM_OFFSET);
		*((int *)(((address)(&(parep_mpi_jumped[tid_index]))) - prstat.restore_start + (address)tempMap)) = 1;
		
		asm volatile("lea (%%rip), %0" : "=r"(cpc) : : "memory");
		parep_mpi_temp_pc[tid_index] = cpc - prstat.restore_start + (address)tempMap;
		if(parep_mpi_jumped[tid_index] == 0) {
			asm volatile("mov %0, %%rsp" : : "r"(jmpstackaddr) : "memory");
			asm volatile("mov %0, (%%rsp)" : : "r"((long)tid_index) : "memory");
			asm volatile("mov %0, -0x8(%%rsp)" : : "r"((long)threadfsbase[tid_index]) : "memory");
			asm volatile("jmpq *%0" : : "r" (parep_mpi_temp_pc[tid_index]) : "memory");
		} else {
			asm volatile("mov (%%rsp), %%rdi" : : : "rdi","memory");
			asm volatile("mov -0x8(%%rsp), %%rsi" : : : "rsi","memory");
			remap_and_cleanup();
		}
	} else {
		//asm volatile("\t wrfsbase %0" : : "r"(threadfsbase[tid_index]));
		asm volatile ("mov %1, %%rax\n\t"
									"mov %2, %%rdi\n\t"
									"mov %3, %%rsi\n\t"
									"syscall\n\t"
									"mov %%rax, %0"
									: "=r" (ret)
									: "i" (SYS_arch_prctl),
									"i" (ARCH_SET_FS),
									"r" (threadfsbase[tid_index])
									: "rax", "rdi", "rsi", "rcx", "r11", "memory");
		//syscall(SYS_arch_prctl,ARCH_SET_FS,threadfsbase[tid_index]);

		temprip = threadprocContext[tid_index].rip;
		temprsp = threadprocContext[tid_index].rsp;
		temprbp = threadprocContext[tid_index].rbp;

		PTR_ENCRYPT(temprip);
		threadContext[tid_index][0].__jmpbuf[JB_PC] = temprip;
		PTR_ENCRYPT(temprsp);
		threadContext[tid_index][0].__jmpbuf[JB_RSP] = temprsp;
		PTR_ENCRYPT(temprbp);
		threadContext[tid_index][0].__jmpbuf[JB_RBP] = temprbp;

		IMB;
		parep_mpi_longjmp(threadContext[tid_index],2);
	}
}

NO_OPTIMIZE
void init_ckpt_restore(char *file_name) {
	char file[100];
	
	strcpy(file,file_name);
	ckpt_fd = parep_mpi_open(file,O_RDONLY,0777);
	
	void *fsbase, *rem_fsbase;
	char *jmpstackaddr;
	int nthreads;
	char *copyaddr;
	address cpc;
	address curaddr;
	remap_num = 0;
	tempremapaddr = (address)(0x500000000000);
	//asm volatile("\t rdfsbase %0" : "=r"(fsbase));
	parep_mpi_arch_prctl(ARCH_GET_FS,(unsigned long)(&fsbase));
	//syscall(SYS_arch_prctl,ARCH_GET_FS,&fsbase);

	parep_mpi_perform_read(ckpt_fd,&processContext,sizeof(Context));
	parep_mpi_perform_read(ckpt_fd,&context,sizeof(jmp_buf));
	parep_mpi_perform_read(ckpt_fd,&rem_fsbase,sizeof(address));
	
	parep_mpi_perform_read(ckpt_fd,&nthreads,sizeof(int));
	assert(num_threads == nthreads);
	for(int i = 1; i < nthreads; i++) {
		parep_mpi_perform_read(ckpt_fd,&threadprocContext[i],sizeof(Context));
		parep_mpi_perform_read(ckpt_fd,&threadContext[i],sizeof(jmp_buf));
		parep_mpi_perform_read(ckpt_fd,&threadfsbase[i],sizeof(address));
	}
	parep_mpi_perform_read(ckpt_fd,&pstat,sizeof(ProcessStat));
	
	restore_brk(pstat.cur_brk,pstat.restore_start,pstat.restore_end);
	
	ProcessStat procStat;
	parep_mpi_memcpy(&procStat,&pstat,sizeof(ProcessStat));
	
	if(pstat.cur_brk != (address)NULL) {
		//int ret = parep_mpi_prctl(PR_SET_MM,PR_SET_MM_START_BRK,pstat.start_brk,0,0);
		parep_mpi_brk((void *)pstat.cur_brk);
	}
	
#if defined(__i386__) || defined(__x86_64__)
	asm volatile (CLEAN_FOR_64_BIT(xor %%eax, %%eax; movw %%ax, %%fs) : : : CLEAN_FOR_64_BIT(eax));
#elif defined(__arm__)
	mtcp_sys_kernel_set_tls(0);
#elif defined(__aarch64__)
#warning __FUNCTION__ "TODO: Implementation for ARM64"
#endif
	
	unmap_memory_areas_and_restore_vdso(&procStat);
	readmemoryareas(ckpt_fd,procStat.startstack);
	
	parep_mpi_close(ckpt_fd);
	
	tempMap = parep_mpi_mmap((void *)0x600000000000, (prstat.restore_full_end - prstat.restore_start) + TEMP_STACK_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
	if((unsigned long)(tempMap) >= -4095UL) {
		parep_mpi_perform_write(1,"MMAP TEMP FAILED\n",17);
		unsigned long err = (unsigned long)(tempMap);
		err = -err;
		if(err == EACCES) parep_mpi_perform_write(1,"ERR EACCES\n",11);
		if(err == EAGAIN) parep_mpi_perform_write(1,"ERR EAGAIN\n",11);
		if(err == EBADF) parep_mpi_perform_write(1,"ERR EBADF\n",10);
		if(err == EEXIST) parep_mpi_perform_write(1,"ERR EEXIST\n",11);
		if(err == ENFILE) parep_mpi_perform_write(1,"ERR ENFILE\n",11);
		if(err == ENODEV) parep_mpi_perform_write(1,"ERR ENODEV\n",11);
		if(err == EPERM) parep_mpi_perform_write(1,"ERR EPERM\n",10);
		if(err == EFAULT) parep_mpi_perform_write(1,"ERR EFAULT\n",11);
		if(err == EINVAL) parep_mpi_perform_write(1,"ERR EINVAL\n",11);
		if(err == ENOMEM) parep_mpi_perform_write(1,"ERR ENOMEM\n",11);
		while(1);
	}
	tempStack = (char *)((address)tempMap + prstat.restore_full_end - prstat.restore_start);
	jmpstackaddr = tempStack + TEMP_STACK_SIZE - TEMP_STACK_BOTTOM_OFFSET;
	for(int i = 0; i < 10; i++) parep_mpi_jumped[i] = 0;
	
	copyaddr = tempMap;
	for(int i = prstat.restore_start_map; i <= prstat.related_map; i++) {
		if(mappings[i].prot != PROT_NONE) {
			parep_mpi_mprotect((void *)mappings[i].start,mappings[i].end-mappings[i].start,mappings[i].prot|PROT_READ);
			parep_mpi_memcpy(copyaddr,(void *)mappings[i].start,mappings[i].end-mappings[i].start);
			if(!(mappings[i].prot & PROT_WRITE) && (mappings[i].prot & PROT_READ) && !(mappings[i].prot & PROT_EXEC)) {
				for(curaddr = (address)copyaddr; curaddr < (address)copyaddr + mappings[i].end-mappings[i].start; curaddr += 8) {
					if((*((address *)curaddr) >= prstat.restore_start) && (*((address *)curaddr) <= prstat.restore_full_end)) {
						*((address *)curaddr) = *((address *)curaddr) - prstat.restore_start + (address)tempMap;
					}
				}
			}
			parep_mpi_mprotect((void *)mappings[i].start,mappings[i].end-mappings[i].start,mappings[i].prot);
		}
		copyaddr = (char *)((address)copyaddr + (mappings[i].end-mappings[i].start));
	}
	//*((address *)(((address)(&parep_mpi_jumped)) - prstat.restore_start + (address)tempMap)) = (address)parep_mpi_jumped - prstat.restore_start + (address)tempMap;
	*((int *)(((address)(&(parep_mpi_jumped[0]))) - prstat.restore_start + (address)tempMap)) = 1;
	
	if(__sync_bool_compare_and_swap(&writing_ckpt_futex,1,0)) {
		asm volatile ("mov %1, %%rax\n\t"
									"mov %2, %%rdi\n\t"
									"mov %3, %%rsi\n\t"
									"mov %4, %%rdx\n\t"
									"mov %5, %%r10\n\t"
									"mov %6, %%r8\n\t"
									"mov %7, %%r9\n\t"
									"syscall\n\t"
									"mov %%rax, %0"
									: "=r" (writing_ckpt_ret)
									: "i" (SYS_futex),
									"r" (&writing_ckpt_futex),
									"i" (FUTEX_WAKE),
									"r" ((long)INT_MAX),
									"r" ((long)NULL),
									"r" ((long)NULL),
									"r" ((long)0)
									: "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory");
	}
	
	asm volatile("lea (%%rip), %0" : "=r"(cpc) : : "memory");
	
	parep_mpi_temp_pc[0] = cpc - prstat.restore_start + (address)tempMap;
	if(parep_mpi_jumped[0] == 0) {
		asm volatile("mov %0, %%rsp" : : "r"(jmpstackaddr) : "memory");
		asm volatile("mov %0, (%%rsp)" : : "r"((long)0) : "memory");
		asm volatile("mov %0, -0x8(%%rsp)" : : "r"((long)rem_fsbase) : "memory");
		asm volatile("jmpq *%0" : : "r" (parep_mpi_temp_pc[0]) : "memory");
	} else {
		asm volatile("mov (%%rsp), %%rdi" : : : "rdi","memory");
		asm volatile("mov -0x8(%%rsp), %%rsi" : : : "rsi","memory");
		remap_and_cleanup();
	}
	
	/*//asm volatile("\t wrfsbase %0" : : "r"(rem_fsbase));
	parep_mpi_arch_prctl(ARCH_SET_FS,(unsigned long)(rem_fsbase));
	//syscall(SYS_arch_prctl,ARCH_SET_FS,rem_fsbase);
	
	address temprip,temprsp,temprbp;
	temprip = processContext.rip;
	temprsp = processContext.rsp;
	temprbp = processContext.rbp;
	
	PTR_ENCRYPT(temprip);
	context[0].__jmpbuf[JB_PC] = temprip;
	PTR_ENCRYPT(temprsp);
	context[0].__jmpbuf[JB_RSP] = temprsp;
	PTR_ENCRYPT(temprbp);
	context[0].__jmpbuf[JB_RBP] = temprbp;
	
	IMB;*/
}

int does_ckpt_file_exists(char *file_name) {
	char file[100];
	strcpy(file,file_name);

	if(access(file, F_OK) != -1) {
		printf("Ckpt file exist: YES");
		return 1;
	}
	else {
		printf("Ckpt file does not exist: NO");
		return 0;
	}
}