// EPOS Watchdog Mediator Common Package

#ifndef __watchdog_h
#define __watchdog_h

#include <system/config.h>
#include <machine.h>
#include <utility/handler.h>

__BEGIN_SYS

class Watchdog_Common
{
protected:
    Watchdog_Common() {}
};

__END_SYS

#ifdef __WATCHDOG_H
#include __WATCHDOG_H
#else
__BEGIN_SYS
class Watchdog {
public:
    static void enable() {}
    static void kick() {}
};
__END_SYS
#endif

#endif
