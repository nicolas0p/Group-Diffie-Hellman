// EPOS MMU Mediator Common Package

#ifndef __mmu_h
#define __mmu_h

#include <cpu.h>

__BEGIN_SYS

template<unsigned int DIRECTORY_BITS, unsigned int PAGE_BITS, unsigned int OFFSET_BITS>
class MMU_Common
{
protected:
    MMU_Common() {}

protected:
    // CPU imports
    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Phy_Addr Phy_Addr;

    // Page constants
    static const unsigned int PAGE_SHIFT = OFFSET_BITS;
    static const unsigned int DIRECTORY_SHIFT = OFFSET_BITS + PAGE_BITS;
    static const unsigned int PAGE_SIZE = 1 << PAGE_SHIFT;

public:
    // Memory page
    typedef unsigned char Page[PAGE_SIZE];
    typedef Page Frame;

    // Page_Table and Page_Directory entries
    typedef Phy_Addr PT_Entry;
    typedef Phy_Addr PD_Entry;

    // Page Flags
    class Flags
    {
    public:
        enum {
            PRE = 0x001, // Presence (0=not-present, 1=present)
            RW  = 0x002, // Write (0=read-only, 1=read-write)
            USR = 0x004, // Access Control (0=supervisor, 1=user)
            CWT = 0x008, // Cache Mode (0=write-back, 1=write-through)
            CD  = 0x010, // Cache Disable (0=cacheable, 1=non-cacheable)
            CT  = 0x020, // Contiguous (0=non-contiguous, 1=contiguous)
            IO  = 0x040, // Memory Mapped I/O (0=memory, 1=I/O)
            SYS = (PRE | RW ),
            APP = (PRE | RW | USR)
        };

    public:
        Flags() {}
        Flags(const Flags & f) : _flags(f._flags) {}
        Flags(unsigned int f) : _flags(f) {}

        operator unsigned int() const { return _flags; }

        friend Debug & operator<<(Debug & db, Flags f) { db << (void *)f._flags; return db; }

    private:
        unsigned int _flags;
    };

    // Number of entries in Page_Table and Page_Directory
    static const unsigned int PT_ENTRIES = sizeof(Page) / sizeof(PT_Entry);
    static const unsigned int PD_ENTRIES = PT_ENTRIES;

public:
    static unsigned int pages(unsigned int bytes) { return (bytes + sizeof(Page) - 1) / sizeof(Page); }
    static unsigned int page_tables(unsigned int pages) { return (pages + PT_ENTRIES - 1) / PT_ENTRIES; }

    static unsigned int offset(const Log_Addr & addr) { return addr & (sizeof(Page) - 1); }
    static unsigned int indexes(const Log_Addr & addr) { return addr & ~(sizeof(Page) - 1); }

    static unsigned int page(const Log_Addr & addr) { return (addr >> PAGE_SHIFT) & (PT_ENTRIES - 1); }
    static unsigned int directory(const Log_Addr & addr) { return addr >> DIRECTORY_SHIFT; }

    static Log_Addr align_page(const Log_Addr & addr) { return (addr + sizeof(Page) - 1) & ~(sizeof(Page) - 1); }
    static Log_Addr align_directory(const Log_Addr & addr) { return (addr + sizeof(Page) * sizeof(Page) - 1) &  ~(sizeof(Page) * sizeof(Page) - 1); }
};

__END_SYS

#ifdef __MMU_H
#include __MMU_H
#endif

#endif
