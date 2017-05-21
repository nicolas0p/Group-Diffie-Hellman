// EPOS YYYY PCI Mediator

#ifndef __yyyy_pci_h
#define __yyyy_pci_h

#include <bus.h>
#include "memory_map.h"

__BEGIN_SYS

class YYYY_Bus: public BUS_Common
{
    friend class YYYY;

private:
    static const unsigned long LOG_IO_MEM = Memory_Map<YYYY>::IO;

public:
    YYYY_BUS() {}
    
private:
    static void init();

private:
};

__END_SYS

#endif
