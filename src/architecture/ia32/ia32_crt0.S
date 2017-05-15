	.file	"ia32_crt0.S"

	.section .text
	.align	4
	.globl	_start
	.type	_start,@function
_start:
	call	_init
	.align	4	// the following label must be aligned for some architectures
	.globl	__epos_app_entry
        .type   __epos_app_entry,@function
__epos_app_entry:
        call    __pre_main
	call	main
	push	%eax	// save main's return value to be used by exit()
	call	_fini
	call	_exit
