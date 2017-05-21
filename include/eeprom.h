// EPOS EEPROM Mediator Common Package

#ifndef __eeprom_h
#define __eeprom_h

#include <system/config.h>

__BEGIN_SYS

class EEPROM_Common
{
protected:
    EEPROM_Common() {}

public:
    typedef unsigned int Address;
};

__END_SYS

#ifdef __EEPROM_H
#include __EEPROM_H
#endif

#endif
