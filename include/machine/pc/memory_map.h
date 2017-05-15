// EPOS PC Memory Map

#ifndef __pc_memory_map_h
#define __pc_memory_map_h

#include <system/memory_map.h>

__BEGIN_SYS

struct Memory_Map
{
    // Physical Memory
    enum {
        MEM_BASE      = Traits<Machine>::MEM_BASE,
        MEM_TOP       = Traits<Machine>::MEM_TOP
    };

    // Logical Address Space
    enum {
        APP_LOW       = Traits<Machine>::APP_LOW,
        APP_CODE      = Traits<Machine>::APP_CODE,
        APP_DATA      = Traits<Machine>::APP_DATA,
        APP_HIGH      = Traits<Machine>::APP_HIGH,

        PHY_MEM       = Traits<Machine>::PHY_MEM,
        IO            = Traits<Machine>::IO_BASE,
        APIC          = IO,
        IO_APIC       = APIC    +  4 * 1024,
        VGA           = IO_APIC +  4 * 1024,
        PCI           = VGA + 32 * 1024, // VGA text mode

        SYS           = Traits<Machine>::SYS,
        IDT           = SYS + 0x00000000,
        GDT           = SYS + 0x00001000,
        SYS_PT        = SYS + 0x00002000,
        SYS_PD        = SYS + 0x00003000,
        SYS_INFO      = SYS + 0x00004000,
        TSS0          = SYS + 0x00005000,
        SYS_CODE      = SYS + 0x00300000,
        SYS_DATA      = SYS + 0x00340000,
        SYS_STACK     = SYS + 0x003c0000,
        SYS_HEAP      = SYS + 0x00400000
    };
};

__END_SYS

#endif
