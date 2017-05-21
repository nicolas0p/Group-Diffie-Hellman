// EPOS PC Scratchpad Memory Mediator Declarations

#include <scratchpad.h>

#ifndef __pc_scratchpad_h
#define __pc_scratchpad_h

__BEGIN_SYS

class Scratchpad: public Scratchpad_Common
{
    friend class Machine;

private:
    static const unsigned int ADDRESS = Traits<Scratchpad>::ADDRESS;
    static const unsigned int SIZE = Traits<Scratchpad>::SIZE;

public:
    Scratchpad() {}

private:
    static void init();
};

__END_SYS

#endif
