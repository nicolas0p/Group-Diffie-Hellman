// EPOS IA32 CPU Mediator Implementation

#include <architecture/ia32/cpu.h>
#include <thread.h>

extern "C" { void _exec(void *); }

__BEGIN_SYS

// Class attributes
unsigned int CPU::_cpu_clock;
unsigned int CPU::_bus_clock;

// Class methods
void CPU::Context::save() volatile
{
    // Save the running thread context into its own stack (mostly for debugging)
    ASM("        push    %ebp                                            \n"
        "        mov     %esp, %ebp                                      \n"
        "        mov     8(%ebp), %esp   # sp = this                     \n"
        "        add     $40, %esp       # sp += sizeof(Context)         \n"
        "        push    4(%ebp)         # push eip                      \n"
        "        pushf                                                   \n"
        "        push    %eax                                            \n"
        "        push    %ecx                                            \n"
        "        push    %edx                                            \n"
        "        push    %ebx                                            \n"
        "        push    %ebp            # push esp                      \n"
        "        push    (%ebp)          # push ebp                      \n"
        "        push    %esi                                            \n"
        "        push    %edi                                            \n"
        "        mov     %ebp, %esp                                      \n"
        "        pop     %ebp                                            \n");
}

void CPU::Context::load() const volatile
{
    // Reload Segment Registers with user-level selectors
    if(Traits<System>::multitask)
        ASM("        mov     %0, %%ds                                        \n"
            "        mov     %0, %%es                                        \n"
            "        mov     %0, %%fs                                        \n"
            "        mov     %0, %%gs                                        \n" : : "r"(SEL_APP_DATA));

    // The thread's context in on its stack
    ASM("        mov     4(%esp), %esp         # sp = this               \n");

    // Adjust the user-level stack pointer in the dummy TSS (what for?)
    ASM("        pop     %0                                              \n" : "=m"(reinterpret_cast<TSS *>(Memory_Map::TSS0 + Machine::cpu_id() * sizeof(MMU::Page))->esp) : );

    // Adjust the system-level stack pointer in the dummy TSS (that will be used by system calls and interrupts) for this Thread
    if(Traits<System>::multitask)
        ASM("        mov     %%esp, %%eax                                    \n"
            "        add     $52, %%eax                                      \n"
            "        movl    %%eax, %0                                       \n" : "=m"(reinterpret_cast<TSS *>(Memory_Map::TSS0 + Machine::cpu_id() * sizeof(MMU::Page))->esp0) : : "eax");

    // Perform a possibly cross-level return (from kernel to user-level)
    // Stack contents depend on the CPL in CS, either ss, esp, eflags, cs, eip (for cross-level)
    // or eflags, cs, eip (for same-level)
    ASM("        popa                                                    \n"
        "        iret                                                    \n");
}

void CPU::switch_context(Context * volatile * o, Context * volatile n)
{
    // Recover the return address from the stack and
    // save the previously running thread context ("o") into its stack
    // PUSHA saves an extra SP (which is always "this"), but saves several instruction fetches
    ASM("        pop     %esi                    # eip                   \n"
        "        pushf                                                   \n"
        "        push    %cs                                             \n"
        "        push    %esi                    # eip                   \n"
        "        pusha                                                   \n");
    ASM("        push    %0                                              \n" : : "m"(reinterpret_cast<TSS *>(Memory_Map::TSS0 + Machine::cpu_id() * sizeof(MMU::Page))->esp));
    ASM("        mov     48(%esp), %eax          # old                   \n"
        "        mov     %esp, (%eax)                                    \n");

    // Restore the next thread context ("n") from its stack (and the user-level stack pointer, updating the dummy TSS)
    ASM("        mov     52(%esp), %esp          # new	                 \n");
    ASM("        pop     %0                                              \n" : "=m"(reinterpret_cast<TSS *>(Memory_Map::TSS0 + Machine::cpu_id() * sizeof(MMU::Page))->esp) : );

    // Adjust the system-level stack pointer in the dummy TSS (that will be used by system calls and interrupts) for this Thread
    if(Traits<System>::multitask)
        ASM("        mov     %%esp, %%eax                                    \n"
            "        add     $52, %%eax                                      \n"
            "        movl    %%eax, %0                                       \n" : "=m"(reinterpret_cast<TSS *>(Memory_Map::TSS0 + Machine::cpu_id() * sizeof(MMU::Page))->esp0) : : "eax");

    // Change context through the IRET, will pop FLAGS, CS, and IP
    ASM("        popa                                                    \n"
        "        iret                                                    \n");
}

void CPU::syscalled()
{
    // We get here when an APP triggers INT_SYSCALL with the message address in CX
    // The CPU saves the user-level stack pointer in the stack and restores the system-level stack pointer also from the TSS
    // Stack contents at this point are always: ss, esp, eflags, cs, eip
    // CX holds the pointer to the message

    if(Traits<Build>::MODE == Traits<Build>::KERNEL) {
        // Do the system call by calling _exec with the message pointed by ecx
        ASM("        push    %ecx                # msg                       \n"
            "        call    _exec                                           \n"
            "        pop     %ecx                # clean up                  \n");

        // Return to user-level
        ASM("        iret                                                    \n");
    }
}

__END_SYS
