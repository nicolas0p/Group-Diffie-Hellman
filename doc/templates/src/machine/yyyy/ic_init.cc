// EPOS YYYY Interrupt Controller Initialization

#include <cpu.h>
#include <ic.h>

__BEGIN_SYS

void PC_IC::init()
{
    db<Init, IC>(TRC) << "IC::init()" << endl;

    CPU::int_disable();

    // Set all IDT entries to proper int_dispatch() offsets
    CPU::IDT_Entry * idt = reinterpret_cast<CPU::IDT_Entry *>(Memory_Map<YYYY>::IDT);
    for(unsigned int i = 0; i < CPU::IDT_ENTRIES; i++)
        if(i < INTS)
            idt[i] = CPU::IDT_Entry(CPU::GDT_SYS_CODE, Log_Addr(entry) + i * 16, CPU::SEG_IDT_ENTRY);
        else
            idt[i] = CPU::IDT_Entry(CPU::GDT_SYS_CODE, Log_Addr(entry) + CPU::EXC_LAST * 16, CPU::SEG_IDT_ENTRY);
    
    // Set all interrupt handlers to int_not()
    for(unsigned int i = 0; i < INTS; i++)
 	_int_vector[i] = int_not;

    // Reset some important exception handlers
    _int_vector[CPU::EXC_PF] = reinterpret_cast<Interrupt_Handler>(exc_pf);
    _int_vector[CPU::EXC_DOUBLE] = reinterpret_cast<Interrupt_Handler>(exc_pf);
    _int_vector[CPU::EXC_GPF] = reinterpret_cast<Interrupt_Handler>(exc_gpf);
    _int_vector[CPU::EXC_NODEV] = reinterpret_cast<Interrupt_Handler>(exc_fpu);

    remap();
    disable();

    CPU::int_enable();
}

__END_SYS
