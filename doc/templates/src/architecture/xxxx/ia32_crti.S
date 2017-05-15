        .file   "crti.s"

	.section .init
	.weak	__epos_library_app_entry
        .type   __epos_library_app_entry,@function
__epos_library_app_entry:
        .globl  _init
        .type   _init,@function
_init:
	push   %ebp
	mov    %esp,%ebp
	sub    $0x8,%esp
	
        .section .fini
        .globl  _fini
        .type   _fini,@function
_fini:
	push   %ebp
	mov    %esp,%ebp

