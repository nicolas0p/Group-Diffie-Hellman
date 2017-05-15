// EPOS ARMv7 PMU Mediator Implementation

#include <architecture/armv7/pmu.h>

#ifdef __mmod_zynq__

__BEGIN_SYS

// Class attributes
const CPU::Reg32 ARMv7_A_PMU::_events[EVENTS] = {
                         /* CLOCK              */ CYCLE,
                         /* DVS_CLOCK          */ 0,
                         /* INSTRUCTION        */ ISSUE_CORE_RENAMING,
                         /* BRANCH             */ BRANCHES_ARCHITECTURALLY_EXECUTED,
                         /* BRANCH_MISS        */ MISPREDICTED_BRANCH,
                         /* L1_HIT             */ L1D_ACCESS,
                         /* L2_HIT             */ 0,
                         /* L3_HIT             */ 0,
                         /* L1_MISS            */ L1D_REFILL,
                         /* L2_MISS            */ 0,
                         /* L3_MISS            */ 0,
};

__END_SYS

#endif
