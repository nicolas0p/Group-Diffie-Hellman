	.file	"crtbegin.s"
	
	.section	.ctors,"aw",@progbits
	.align 4
	.type	__CTOR_LIST__, @object
__CTOR_LIST__:
	.long	-1
	
	.section	.dtors,"aw",@progbits
	.align 4
	.type	__DTOR_LIST__, @object
__DTOR_LIST__:
	.long	-1
	
	.section	.jcr,"aw",@progbits
	.align 4
	.type	__JCR_LIST__, @object
__JCR_LIST__:
	.hidden	__dso_handle
	.globl	__dso_handle

	.data
	.align	4
	.type	__dso_handle, @object
__dso_handle:
	.long	0
	.align	4
	.type	p.0, @object
p.0:
	.long	__DTOR_LIST__+4
	.local	completed.1
	.comm	completed.1,1,1

	.text
	.p2align 4,,15
	.type	__do_global_dtors_aux, @function
__do_global_dtors_aux:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	cmpb	$0, completed.1
	je	.L8
	jmp	.L1
	.p2align 4,,7
.L10:
	addl	$4, %eax
	movl	%eax, p.0
	call	*%edx
.L8:
	movl	p.0, %eax
	movl	(%eax), %edx
	testl	%edx, %edx
	jne	.L10
	movb	$1, completed.1
.L1:
	leave
	ret

	.section	.fini
	call __do_global_dtors_aux

	.text
	.p2align 4,,15
	.type	frame_dummy, @function
frame_dummy:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	__JCR_LIST__, %eax
	testl	%eax, %eax
	je	.L11
	movl	$_Jv_RegisterClasses, %eax
	testl	%eax, %eax
	je	.L11
	movl	$__JCR_LIST__, (%esp)
	call	_Jv_RegisterClasses
	.p2align 4,,15
.L11:
	leave
	ret

	.section	.init
	call frame_dummy

	.text
	.weak	_Jv_RegisterClasses
	.section	.note.GNU-stack,"",@progbits
