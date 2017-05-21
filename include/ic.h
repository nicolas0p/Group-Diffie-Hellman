// EPOS IC Mediator Common Package

#ifndef __ic_h
#define __ic_h

#include <system/config.h>

__BEGIN_SYS

class IC_Common
{
protected:
    IC_Common() {}

public:
    typedef unsigned int Interrupt_Id;
    typedef void (* Interrupt_Handler)(const Interrupt_Id &);
};

__END_SYS

#ifdef __IC_H
#include __IC_H
#endif

#endif
