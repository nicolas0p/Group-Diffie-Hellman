// EPOS XXXX MMU Mediator Declarations

/* This is actually no template. It is a functional implementation of the MMU HAL API for architectures without logical addressing (i.e. without MMU) */

#ifndef __xxxx_mmu_h
#define __xxxx_mmu_h

#include <system/memory_map.h>
#include <utility/string.h>
#include <utility/list.h>
#include <utility/debug.h>
#include <cpu.h>
#include <mmu.h>

__BEGIN_SYS

class XXXX_MMU: public MMU_Common<10, 10, 12>
{
    friend class XXXX;

private:
    typedef Grouping_List<Frame> List;

    static const unsigned int PHY_MEM = Memory_Map<Machine>::PHY_MEM;

public:
    // Page Flags
    typedef MMU_Common<0, 0, 0>::Flags XXXX_Flags;

    // Page_Table
    class Page_Table {};

    // Chunk (for Segment)
    class Chunk
    {
    public:
        Chunk() {}
        Chunk(unsigned int bytes, Flags flags): _phy_addr(alloc(bytes)), _bytes(bytes), _flags(flags) {}
        Chunk(Phy_Addr phy_addr, unsigned int bytes, Flags flags): _phy_addr(phy_addr), _bytes(bytes), _flags(flags) {}

        ~Chunk() { free(_phy_addr, _bytes); }

        unsigned int pts() const { return 0; }
        Flags flags() const { return _flags; }
        Page_Table * pt() const { return 0; }
        unsigned int size() const { return _bytes; }
        Phy_Addr phy_address() const { return _phy_addr; } // always CT
        int resize(unsigned int amount) { return 0; } // always CT

    private:
        Phy_Addr _phy_addr;
        unsigned int _bytes;
        XXXX_Flags _flags;
    };

    // Page Directory
    typedef Page_Table Page_Directory;

    // Directory (for Address_Space)
    class Directory 
    {
    public:
        Directory() {}
        Directory(Page_Directory * pd) {}
        ~Directory() {}

        Page_Table * pd() const { return 0; }

        void activate() {}

        Log_Addr attach(const Chunk & chunk) { return chunk.phy_address(); }
        Log_Addr attach(const Chunk & chunk, Log_Addr addr) { return (addr == chunk.phy_address())? addr : Log_Addr(false); }
        void detach(const Chunk & chunk) {}
        void detach(const Chunk & chunk, Log_Addr addr) {}

        Phy_Addr physical(Log_Addr addr) { return addr; }
    };

    // DMA_Buffer (straightforward without paging)
    typedef Chunk DMA_Buffer;

public:
    XXXX_MMU() {}

    static Phy_Addr alloc(unsigned int bytes = 1) {
        Phy_Addr phy(false);

        // Free blocks must be large enough to contain a list element
        assert(bytes > sizeof (List::Element));

        if(bytes) {
            List::Element * e = _free.search_decrementing(bytes);
            if(e)
        	phy = e->object() + e->size();
            else
        	db<XXXX_MMU>(WRN) << "XXXX_MMU::alloc() failed!" << endl;
        }

        db<XXXX_MMU>(TRC) << "XXXX_MMU::alloc(bytes=" << bytes << ") => " << phy << endl;

        return phy;
    }

    static Phy_Addr calloc(unsigned int frames = 1) {
        Phy_Addr phy = alloc(frames);

        memset(phy2log(phy), 0, sizeof(Frame) * frames);

        return phy;	
    }

    static void free(Phy_Addr addr, int n = 1) {
        db<XXXX_MMU>(TRC) << "XXXX_MMU::free(addr=" << addr << ",n=" << n << ")" << endl;

        // No unaligned addresses if the CPU doesn't support it
        assert(Traits<CPU>::unaligned_memory_access || !(addr % 4));

        // Free blocks must be large enough to contain a list element
        assert(n > sizeof (List::Element));

        if(addr && n) {
            List::Element * e = new (phy2log(addr)) List::Element(addr, n);
            List::Element * m1, * m2;
            _free.insert_merging(e, &m1, &m2);
        }
    }

    static unsigned int allocable() { return _free.head() ? _free.head()->size() : 0; }

    static Page_Directory * volatile current() { return 0; }

    static Phy_Addr physical(Log_Addr addr) { return addr; }

    static void flush_tlb() {}
    static void flush_tlb(Log_Addr addr) {}

private:
    static void init();

    static Log_Addr phy2log(Phy_Addr phy) { return phy; }

private:
    static List _free;
};

__END_SYS

#endif
