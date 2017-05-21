// EPOS XXXX Time-Stamp Counter Mediator Declarations

#ifndef __xxxx_tsc_h
#define __xxxx_tsc_h

#include <cpu.h>
#include <tsc.h>

__BEGIN_SYS

class XXXX_TSC: public TSC_Common
{
public:
    XXXX_TSC() {}

    static Hertz frequency() { return CPU::clock(); }

    static Time_Stamp time_stamp() {
        Time_Stamp ts;
        /* ASM to read the clock counter in the CPU */
        ASM("nop" : "=r" (ts) : );
        return ts;
    }
};

__END_SYS

#endif
