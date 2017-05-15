// EPOS Keyboard Mediator Common Package

#ifndef __keyboard_h
#define __keyboard_h

#include <utility/observer.h>
#include <display.h>

__BEGIN_SYS

class Keyboard_Common
{
protected:
    Keyboard_Common() {}
};

class Serial_Keyboard: public Keyboard_Common
{
    friend class PC_Setup;
    friend class Machine;

public:
    typedef _UTIL::Observer Observer;
    typedef _UTIL::Observed Observed;

public:
    Serial_Keyboard() {}

    static char get() { return Serial_Display::_engine.get(); }
    static bool ready_to_get() { return Serial_Display::_engine.ready_to_get(); }

    static void attach(Observer * obs) { _observed.attach(obs); }
    static void detach(Observer * obs) { _observed.detach(obs); }

private:
    static void init() {}

private:
    static Observed _observed;
};

__END_SYS

#ifdef __KEYBOARD_H
#include __KEYBOARD_H
#else
__BEGIN_SYS
class Keyboard: public IF<Traits<Serial_Keyboard>::enabled, Serial_Keyboard, Dummy>::Result {};
__END_SYS
#endif

#endif
