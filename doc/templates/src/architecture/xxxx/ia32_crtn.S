        .file   "crtn.s"

        .section .init
        .globl  _init
        .type   _init,@function
	leave
	ret

        .section .fini
        .globl  _fini
        .type   _fini,@function
	leave
	ret
