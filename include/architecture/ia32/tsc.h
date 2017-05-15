// EPOS IA32 Time-Stamp Counter Mediator Declarations

#ifndef __ia32_tsc_h
#define __ia32_tsc_h

#include <cpu.h>
#include <tsc.h>

__BEGIN_SYS

class TSC: private TSC_Common
{
public:
    using TSC_Common::Hertz;
    using TSC_Common::Time_Stamp;

public:
    TSC() {}

    static Hertz frequency() { return CPU::clock(); }

    static Time_Stamp time_stamp() {
        Time_Stamp ts;
        ASM("rdtsc" : "=A" (ts) : ); // must be volatile!
        return ts;
    }
};

__END_SYS

#endif
