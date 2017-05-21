// EPOS IA32 PMU Mediator Implementation

#include <architecture/ia32/pmu.h>

__BEGIN_SYS

// Class attributes
const CPU::Reg32 Intel_PMU_V1::_events[EVENTS] = {
                         /* CLOCK              */ UNHALTED_CORE_CYCLES,
                         /* DVS_CLOCK          */ UNHALTED_REFERENCE_CYCLES,
                         /* INSTRUCTIONS       */ INSTRUCTIONS_RETIRED,
                         /* BRANCHES           */ BRANCH_INSTRUCTIONS_RETIRED,
                         /* BRANCH_MISSES      */ BRANCH_MISSES_RETIRED,
                         /* L1_HIT             */ 0,
                         /* L2_HIT             */ 0,
                         /* L3_HIT             */ LLC_REFERENCES,
                         /* L1_MISS            */ 0,
                         /* L2_MISS            */ 0,
                         /* L3_MISS            */ LLC_MISSES,
};

__END_SYS
