// EPOS YYYY Scratchpad Memory Mediator Declarations

#include <scratchpad.h>

#ifndef __yyyy_scratchpad_h
#define __yyyy_scratchpad_h

__BEGIN_SYS

// CHIP_YYYY_Scratchpad
class CHIP_YYYY_Scratchpad
{
};

class YYYY_Scratchpad: public Scratchpad_Common, private CHIP_YYYY_Scratchpad
{
    friend class YYYY;

private:
    static const unsigned int ADDRESS = Traits<YYYY_Scratchpad>::ADDRESS;
    static const unsigned int SIZE = Traits<YYYY_Scratchpad>::SIZE;

public:
    YYYY_Scratchpad() {}

private:
    static void init();
};

__END_SYS

#endif
