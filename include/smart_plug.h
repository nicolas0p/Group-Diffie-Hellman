// EPOS Smart Plug Mediator Common Package

#ifndef __smart_plug_h
#define __smart_plug_h

#include <system/config.h>

__BEGIN_SYS

class Smart_Plug_Common
{
protected:
    Smart_Plug_Common() {}
};

__END_SYS

#ifdef __SMART_PLUG_H
#include __SMART_PLUG_H
#else
__BEGIN_SYS
class Smart_Plug: public Dummy {};
__END_SYS
#endif

#endif
