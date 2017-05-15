// EPOS YYYY EEPROM Mediator Common Declarations

#ifndef __yyyy_eeprom_h
#define __yyyy_eeprom_h

#include <eeprom.h>

__BEGIN_SYS

// YYYY Physical EEPROM Controller
class CHIP_YYYY_EEPROM
{
};

class YYYY_EEPROM: public EEPROM_Common, private CHIP_YYYY_EEPROM
{
public:
    typedef EEPROM_Common::Address Address;

public:
    YYYY_EEPROM() {};

    unsigned char read(const Address & a) { return CHIP_YYYY_EEPROM::read(a); }
    void write(const Address & a, unsigned char d) { CHIP_YYYY_EEPROM::write(a, d); }

    int size() const { return CHIP_YYYY_EEPROM::size(); }
};

__END_SYS

#endif
