	.file	"crt1.s"

	.text
	.align	4
	.globl	_start
	.type	_start,@function
_start:
	call	_init
	.align	4
	.globl	__epos_library_app_entry
__epos_library_app_entry:
	call	main
	push	%eax
	call	_fini
	call	_exit
