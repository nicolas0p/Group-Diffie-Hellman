// EPOS ARMv7 CPU Mediator Implementation

#include <architecture/armv7/cpu.h>
#include <system.h>

__BEGIN_SYS

// Class attributes
unsigned int CPU::_cpu_clock;
unsigned int CPU::_bus_clock;

/*
Regarding the need to preserve r12 (a.k.a IP):

The ARM Procedure Call Standard [1] defines rules for r12 only in special cases, leaving it as a scratch register in general cases.
An objdump reveals that ip is in fact used in several places, and even the processor's interrupt stack frame cares to save its value along with the other scratch registers {r0-r3}.

The only rules defined for IP are [1]:

* At each call where the control transfer instruction is subject to a BL-type relocation at static link time, rules on
the use of IP are observed (5.3.1.1 Use of IP by the linker).

* Register r12 (IP) may be used by a linker as a scratch register between a routine and any subroutine it calls (for
details, see 5.3.1.1, Use of IP by the linker). It can also be used within a routine to hold intermediate values
between subroutine calls.

5.3.1.1 Use of IP by the linker
Both the ARM- and Thumb-state BL instructions are unable to address the full 32-bit address space, so it may be
necessary for the linker to insert a veneer between the calling routine and the called subroutine. Veneers may
also be needed to support ARM-Thumb inter-working or dynamic linking. Any veneer inserted must preserve the
contents of all registers except IP (r12) and the condition code flags; a conforming program must assume that a
veneer that alters IP may be inserted at any branch instruction that is exposed to a relocation that supports inter-
working or long branches.

[1] Procedure Call Standard for the ARM (R) Architecture
    Document number: ARM IHI 0042F, current through ABI release 2.10
    Date of Issue: 24 November 2015
*/

// Class methods
void CPU::Context::save() volatile
{
    ASM("       str     r12, [sp,#-68]          \n"
        "       mov     r12, pc                 \n");
    if(thumb)
        ASM("       orr r12, #1                     \n");

    ASM("       push    {r12}                   \n"
        "       ldr     r12, [sp,#-64]          \n"
        "       push    {r0-r12, lr}            \n");
    mrs12();
    ASM("       push    {r12}                   \n"
        "       sub     sp, #4                  \n"
        "       pop     {r12}                   \n"
        "       str     sp, [%0]                \n" : : "r"(this));
}

void CPU::Context::load() const volatile
{
    System::_heap->free(reinterpret_cast<void *>(Memory_Map::SYS_STACK), Traits<System>::STACK_SIZE);
    ASM("       mov     sp, %0                  \n"
        "       isb                             \n" // serialize the pipeline so that SP gets updated before the pop
        "       pop     {r12}                   \n" : : "r"(this));
    msr12();
    ASM("       pop     {r0-r12, lr}            \n"
        "       pop     {pc}                    \n");
}

void CPU::switch_context(Context * volatile * o, Context * volatile n)
{
    ASM("       sub     sp, #4                  \n"
        "       push    {r12}                   \n"
        "       adr     r12, .ret               \n");
    if(thumb)
        ASM("       orr r12, #1                     \n");
    ASM("       str     r12, [sp,#4]            \n"
        "       pop     {r12}                   \n"
        "       push    {r0-r12, lr}            \n");
    mrs12();
    ASM("       push    {r12}                   \n"
        "       str     sp, [%0]                \n"
        "       mov     sp, %1                  \n"
        "       isb                             \n" // serialize the pipeline so that SP gets updated before the pop
        "       pop     {r12}                   \n" : : "r"(o), "r"(n));
    msr12();
    ASM("       pop     {r0-r12, lr}            \n");
    if(Traits<Build>::MODEL != Traits<Build>::Zynq)
        int_enable();
    ASM("       pop     {pc}                    \n"
        ".ret:  bx      lr                      \n");
}

__END_SYS
