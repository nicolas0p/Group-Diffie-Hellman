// EPOS XXXX CPU Mediator Initialization

#include <cpu.h>
#include <mmu.h>
#include <system.h>
#include <system/info.h>

extern "C" { void __epos_library_app_entry(void); }

__BEGIN_SYS

void XXXX::init()
{
    db<Init, CPU>(TRC) << "CPU::init()" << endl;

    _cpu_clock = System::info()->tm.cpu_clock;
    _bus_clock = System::info()->tm.bus_clock;

    // Initialize the MMU
    if(Traits<MMU>::enabled)
        MMU::init();
    else
        db<Init, MMU>(WRN) << "MMU is disabled!" << endl;
}

__END_SYS
