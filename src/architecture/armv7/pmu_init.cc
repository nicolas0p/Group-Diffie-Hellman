// EPOS ARMv7 PMU Mediator Initialization

#include <pmu.h>

#ifdef __mmod_zynq__

__BEGIN_SYS

void ARMv7_A_PMU::init()
{
    db<Init, PMU>(TRC) << "PMU::init()" << endl;

    // Set global enable, reset all counters including cycle counter, and
    // export the events
    pmcr(pmcr() | PMCR_E | PMCR_P | PMCR_C | PMCR_X);

    // Clear all overflows:
    pmovsr(~0);

    // Enable cycle counter
    pmcntenset(pmcntenset() | PMCNTENSET_C);
}

__END_SYS

#endif
