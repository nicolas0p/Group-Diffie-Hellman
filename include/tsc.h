// EPOS TSC Mediator Common Package

#ifndef __tsc_h
#define __tsc_h

#include <system/config.h>

__BEGIN_SYS

class TSC_Common
{
protected:
    TSC_Common() {}

public:
    typedef unsigned long Hertz;
    typedef unsigned long long Time_Stamp;
};

__END_SYS

#ifdef __TSC_H
#include __TSC_H
#endif

#endif
