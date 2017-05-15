// EPOS YYYY Mediator Implementation

#include <mach/yyyy/machine.h>

__BEGIN_SYS

// Class attributes
volatile unsigned int YYYY::_n_cpus;

// Class methods
void YYYY::delay(const RTC::Microsecond & time)
{
    TSC::Time_Stamp end = TSC::time_stamp() + time * (TSC::frequency() / 1000000);
    while(end > TSC::time_stamp());
}

void YYYY::panic()
{
    CPU::int_disable(); 
    Display::position(24, 73);
    Display::puts("PANIC!");
    if(Traits<System>::reboot)
        Machine::reboot();
    else
        CPU::halt();
}

void YYYY::reboot()
{
    for(int i = 0; (i < 300) && (CPU::in8(0x64) & 0x02); i++)
        i8255::ms_delay(1);

    // Sending 0xfe to the keyboard controller port causes it to pulse
    // the reset line
    CPU::out8(0x64, 0xfe);

    for(int i = 0; (i < 300) && (CPU::in8(0x64) & 0x02); i++)
        i8255::ms_delay(1);
}

__END_SYS
