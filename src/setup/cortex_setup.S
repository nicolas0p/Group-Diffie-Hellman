#ifdef zynq
// EPOS Cortex-A SETUP

        .file "cortex_a_setup.S"

        // Interrupt Vector Table
        .section .init
        .type _vector_table, object
_vector_table:
        ldr pc, _start_addr
        ldr pc, _undefined_instruction_addr
        ldr pc, _swi_addr
        ldr pc, _prefetch_abort_addr
        ldr pc, _data_abort_addr
        ldr pc, _reserved_addr
        ldr pc, _irq_addr
        ldr pc, _fiq_addr

_start_addr:
        .word _start
_undefined_instruction_addr:
        .word _undefined_instruction
_swi_addr:
        .word _software_interrupt
_prefetch_abort_addr:
        .word _prefetch_abort
_data_abort_addr:
        .word _data_abort
_reserved_addr:
        .word _reserved
_irq_addr:
        .word _int_entry
_fiq_addr:
        .word _fiq
#else
// EPOS Cortex-M SETUP

        .file "cortex_m_setup.S"

        // Interrupt Vector Table
        .section .init
        .type   _vector_table, object
_vector_table:
        .word   MEM_TOP  + 1            // Stack pointer at reset (set by sed in the makefile)
        .word   _start + 1              // Reset  (lsb must be 1 in thumb mode)
        .word   _int_entry + 1          // NMI
        .word   _int_entry + 1          // Hard fault
        .word   _int_entry + 1          // Memory management fault
        .word   _int_entry + 1          // Bus fault
        .word   _int_entry + 1          // Usage fault
        .word   _int_entry + 1          // Reserved
        .word   _int_entry + 1          // Reserved
        .word   _int_entry + 1          // Reserved
        .word   _int_entry + 1          // Reserved
        .word   _svc_handler + 1        // SVCall
        .word   _int_entry + 1          // Reserved
        .word   _int_entry + 1          // Reserved
        .word   _int_entry + 1          // PendSV
        .word   _int_entry + 1          // Systick
        .word   _int_entry + 1          // IRQ0
        .word   _int_entry + 1          // IRQ1
        .word   _int_entry + 1          // IRQ2
        .word   _int_entry + 1          // IRQ3
        .word   _int_entry + 1          // IRQ4
        .word   _int_entry + 1          // IRQ5
        .word   _int_entry + 1          // IRQ6
        .word   _int_entry + 1          // IRQ7
        .word   _int_entry + 1          // IRQ8
        .word   _int_entry + 1          // IRQ9
        .word   _int_entry + 1          // IRQ10
        .word   _int_entry + 1          // IRQ11
        .word   _int_entry + 1          // IRQ12
        .word   _int_entry + 1          // IRQ13
        .word   _int_entry + 1          // IRQ14
        .word   _int_entry + 1          // IRQ15
        .word   _int_entry + 1          // IRQ16
        .word   _int_entry + 1          // IRQ17
        .word   _int_entry + 1          // IRQ18
        .word   _int_entry + 1          // IRQ19
        .word   _int_entry + 1          // IRQ20
        .word   _int_entry + 1          // IRQ21
        .word   _int_entry + 1          // IRQ22
        .word   _int_entry + 1          // IRQ23
        .word   _int_entry + 1          // IRQ24
        .word   _int_entry + 1          // IRQ25
        .word   _int_entry + 1          // IRQ26
        .word   _int_entry + 1          // IRQ27
        .word   _int_entry + 1          // IRQ28
        .word   _int_entry + 1          // IRQ29
        .word   _int_entry + 1          // IRQ30
        .word   _int_entry + 1          // IRQ31
        .word   _int_entry + 1          // IRQ32
        .word   _int_entry + 1          // IRQ33
        .word   _int_entry + 1          // IRQ34
        .word   _int_entry + 1          // IRQ35
        .word   _int_entry + 1          // IRQ36
        .word   _int_entry + 1          // IRQ37
        .word   _int_entry + 1          // IRQ38
        .word   _int_entry + 1          // IRQ39
        .word   _int_entry + 1          // IRQ40
        .word   _int_entry + 1          // IRQ41
        .word   _int_entry + 1          // IRQ42
        .word   _int_entry + 1          // IRQ43
        .word   _int_entry + 1          // IRQ44
        .word   _int_entry + 1          // IRQ45
#endif
