.section .text
.globl parep_mpi_longjmp
.type parep_mpi_longjmp, @function

parep_mpi_longjmp:
	mov 0x30(%rdi), %r8
	mov 0x8(%rdi), %r9
	mov 0x38(%rdi), %rdx
	ror $0x11, %r8
	xor %fs:0x30, %r8
	ror $0x11, %r9
	xor %fs:0x30, %r9
	ror $0x11, %rdx
	xor %fs:0x30, %rdx
#ifdef __CET_SUPPORTED
	testl $0x2, %fs:0x48
	je ljmpend
	rdsspq %rax
	sub 0x58(%rdi), %rax
	je ljmpend
	neg %rax
	shr $0x3, %rax
	add $0x1, %rax
	mov $0xff, %ebx

ljmpmid:
	cmp %rbx, %rax
	cmovb %rax, %rbx
	incsspq %rbx
	sub %rbx, %rax
	ja ljmpmid

ljmpend:
#endif
	nop
	
	mov (%rdi), %rbx
	mov 0x10(%rdi), %r12
	mov 0x18(%rdi), %r13
	mov 0x20(%rdi), %r14
	mov 0x28(%rdi), %r15

	mov %esi, %eax
	mov %r8, %rsp
	mov %r9, %rbp

	nop
	
	jmpq *%rdx

	ret
