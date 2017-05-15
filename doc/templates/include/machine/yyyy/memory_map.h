// EPOS YYYY Memory Map

#ifndef __yyyy_memory_map_h
#define __yyyy_memory_map_h

#include <system/memory_map.h>

__BEGIN_SYS

template <>
struct Memory_Map<YYYY>
{
    // Physical Memory
    enum {
        MEM_BASE =	Traits<YYYY>::MEM_BASE,
        MEM_TOP =	Traits<YYYY>::MEM_TOP
    };

    // Logical Address Space
    enum {
        APP_LOW =       Traits<YYYY>::APP_LOW,
        APP_CODE =      Traits<YYYY>::APP_CODE,
        APP_DATA =      Traits<YYYY>::APP_DATA,
        APP_HIGH =      Traits<YYYY>::APP_HIGH,

        PHY_MEM =       Traits<YYYY>::PHY_MEM,
        IO =            Traits<YYYY>::IO_BASE,
        SYS =           Traits<YYYY>::SYS
    };
};

__END_SYS

#endif
