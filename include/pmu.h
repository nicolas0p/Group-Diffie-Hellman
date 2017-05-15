// EPOS PMU Mediator Common Package

#ifndef __pmu_h
#define __pmu_h

#include <system/config.h>

__BEGIN_SYS

class PMU_Common
{
public:
    typedef unsigned int Channel;
    typedef long long int Count;

    enum Event {
        CLOCK,
        DVS_CLOCK,
        INSTRUCTION,
        BRANCH,
        BRANCH_MISS,
        L1_HIT,
        L2_HIT,
        L3_HIT,
        LLC_HIT = L3_HIT,
        CACHE_HIT = LLC_HIT,
        L1_MISS,
        L2_MISS,
        L3_MISS,
        LLC_MISS = L3_MISS,
        CACHE_MISS = LLC_MISS,
        LLC_HITM,
        EVENTS
    };

    enum Flags {
        NONE,
        INT
    };

protected:
    PMU_Common() {}
};

__END_SYS

#ifdef __PMU_H
#include __PMU_H
#endif

#endif
