// EPOS IA32 MMU Mediator Declarations

#ifndef __ia32_mmu_h
#define __ia32_mmu_h

#include <system/memory_map.h>
#include <utility/string.h>
#include <utility/list.h>
#include <utility/debug.h>
#include <cpu.h>
#include <mmu.h>

__BEGIN_SYS

class MMU: public MMU_Common<10, 10, 12>
{
    friend class CPU;

private:
    typedef Grouping_List<Frame> List;

    static const bool colorful = Traits<MMU>::colorful;
    static const unsigned int COLORS = Traits<MMU>::COLORS;
    static const unsigned int PHY_MEM = Memory_Map::PHY_MEM;

public:
    // Page Flags
    class IA32_Flags
    {
    public:
        enum {
            PRE  = 0x001, // Presence (0=not-present, 1=present)
            RW   = 0x002, // Write (0=read-only, 1=read-write)
            USR  = 0x004, // Access Control (0=supervisor, 1=user)
            PWT  = 0x008, // Cache Mode (0=write-back, 1=write-through)
            PCD  = 0x010, // Cache Disable (0=cacheable, 1=non-cacheable)
            ACC  = 0x020, // Accessed (0=not-accessed, 1=accessed
            DRT  = 0x040, // Dirty (only for PTEs, 0=clean, 1=dirty)
            PS   = 0x080, // Page Size (for PDEs, 0=4KBytes, 1=4MBytes)
            GLB  = 0X100, // Global Page (0=local, 1=global)
            EX   = 0x200, // User Def. (0=non-executable, 1=executable)
            CT   = 0x400, // User Def. (0=non-contiguous, 1=contiguous)
            IO   = 0x800, // User Def. (0=memory, 1=I/O)
            APP  = (PRE | RW  | ACC | USR),
            SYS  = (PRE | RW  | ACC),
            PCI  = (SYS | PCD | IO),
            APIC = (SYS | PCD),
            VGA = (SYS | PCD),
            DMA  = (SYS | PCD | CT),
        };

    public:
        IA32_Flags() {}
        IA32_Flags(const IA32_Flags & f) : _flags(f._flags) {}
        IA32_Flags(unsigned int f) : _flags(f) {}
        IA32_Flags(const Flags & f) : _flags(PRE | ACC |
                                             ((f & Flags::RW)  ? RW  : 0) |
                                             ((f & Flags::USR) ? USR : 0) |
                                             ((f & Flags::CWT) ? PWT : 0) |
                                             ((f & Flags::CD)  ? PCD : 0) |
                                             ((f & Flags::CT)  ? CT  : 0) |
                                             ((f & Flags::IO)  ? PCI : 0) ) {}

        operator unsigned int() const { return _flags; }

        friend Debug & operator<<(Debug & db, const IA32_Flags & f) { db << hex << f._flags; return db; }

    private:
        unsigned int _flags;
    };

    // Page_Table
    class Page_Table
    {
    public:
        Page_Table() {}

        PT_Entry & operator[](unsigned int i) { return _entry[i]; }

        void map(int from, int to, const IA32_Flags & flags, const Color & color) {
            Phy_Addr * addr = alloc(to - from, color);
            if(addr)
                remap(addr, from, to, flags);
            else
                for( ; from < to; from++) {
                    Log_Addr * tmp = phy2log(&_entry[from]);
                    *tmp = alloc(1, color) | flags;
                }
        }

        void map_contiguous(int from, int to, const IA32_Flags & flags, const Color & color) {
            remap(alloc(to - from, color), from, to, flags);
        }

        void remap(Phy_Addr addr, int from, int to, const IA32_Flags & flags) {
            addr = align_page(addr);
            for( ; from < to; from++) {
                Log_Addr * tmp = phy2log(&_entry[from]);
                *tmp = addr | flags;
                addr += sizeof(Page);
            }
        }

        void unmap(int from, int to) {
            for( ; from < to; from++) {
                free(_entry[from]);
                Log_Addr * tmp = phy2log(&_entry[from]);
                *tmp = 0;
            }
        }

        friend Debug & operator<<(Debug & db, Page_Table & pt) {
            db << "{\n";
            int brk = 0;
            for(unsigned int i = 0; i < PT_ENTRIES; i++)
                if(pt[i]) {
                    db << "[" << i << "]=" << pt[i] << "  ";
                    if(!(++brk % 4))
                        db << "\n";
                }
            db << "\n}";
            return db;
        }

    private:
        PT_Entry _entry[PT_ENTRIES];
    };

    // Chunk (for Segment)
    class Chunk
    {
    public:
        Chunk() {}

        Chunk(unsigned int bytes, const Flags & flags, const Color & color = WHITE)
        : _from(0), _to(pages(bytes)), _pts(page_tables(_to - _from)), _flags(IA32_Flags(flags)), _pt(calloc(_pts, WHITE)) {
            if(flags & IA32_Flags::CT)
                _pt->map_contiguous(_from, _to, _flags, color);
            else
                _pt->map(_from, _to, _flags, color);
        }

        Chunk(const Phy_Addr & phy_addr, unsigned int bytes, const Flags & flags)
        : _from(0), _to(pages(bytes)), _pts(page_tables(_to - _from)), _flags(IA32_Flags(flags)), _pt(calloc(_pts, WHITE)) {
            _pt->remap(phy_addr, _from, _to, flags);
        }

        ~Chunk() {
            if(!(_flags & IA32_Flags::IO)) {
                if(_flags & IA32_Flags::CT)
                    free((*static_cast<Page_Table *>(phy2log(_pt)))[_from], _to - _from);
                else
                    for( ; _from < _to; _from++)
                        free((*static_cast<Page_Table *>(phy2log(_pt)))[_from]);
            }
            free(_pt, _pts);
        }

        unsigned int pts() const { return _pts; }
        IA32_Flags flags() const { return _flags; }
        Page_Table * pt() const { return _pt; }
        unsigned int size() const { return (_to - _from) * sizeof(Page); }

        Phy_Addr phy_address() const {
            return (_flags & IA32_Flags::CT) ? Phy_Addr(indexes((*_pt)[_from])) : Phy_Addr(false);
        }

        int resize(unsigned int amount) {
            if(_flags & IA32_Flags::CT)
                return 0;

            unsigned int pgs = pages(amount);

            Color color = colorful ? phy2color(_pt) : WHITE;

            unsigned int free_pgs = _pts * PT_ENTRIES - _to;
            if(free_pgs < pgs) { // resize _pt
                unsigned int pts = _pts + page_tables(pgs - free_pgs);
                Page_Table * pt = calloc(pts, color);
                memcpy(pt, _pt, _pts * sizeof(Page));
                free(_pt, _pts);
                _pt = pt;
                _pts = pts;
            }

            _pt->map(_to, _to + pgs, _flags, color);
            _to += pgs;

            return pgs * sizeof(Page);
        }

    private:
        unsigned int _from;
        unsigned int _to;
        unsigned int _pts;
        IA32_Flags _flags;
        Page_Table * _pt;
    };

    // Page Directory
    typedef Page_Table Page_Directory;

    // Directory (for Address_Space)
    class Directory
    {
    public:
        Directory() : _pd(calloc(1, WHITE)), _free(true) {
            for(unsigned int i = directory(PHY_MEM); i < PD_ENTRIES; i++)
                (*_pd)[i] = (*_master)[i];
        }

        Directory(Page_Directory * pd) : _pd(pd), _free(false) {}

        ~Directory() { if(_free) free(_pd); }

        Phy_Addr pd() const { return _pd; }

        void activate() const { CPU::pdp(reinterpret_cast<CPU::Reg32>(_pd)); }

        Log_Addr attach(const Chunk & chunk, unsigned int from = 0) {
            for(unsigned int i = from; i < PD_ENTRIES; i++)
                if(attach(i, chunk.pt(), chunk.pts(), chunk.flags()))
                    return i << DIRECTORY_SHIFT;
            return false;
        }

        Log_Addr attach(const Chunk & chunk, const Log_Addr & addr) {
            unsigned int from = directory(addr);
            if(!attach(from, chunk.pt(), chunk.pts(), chunk.flags()))
                return Log_Addr(false);
            return from << DIRECTORY_SHIFT;
        }

        void detach(const Chunk & chunk) {
            for(unsigned int i = 0; i < PD_ENTRIES; i++)
                if(indexes((*_pd)[i]) == indexes(chunk.pt())) {
                    detach(i, chunk.pt(), chunk.pts());
                return;
            }
            db<MMU>(WRN) << "MMU::Directory::detach(pt=" << chunk.pt() << ") failed!" << endl;
        }

        void detach(const Chunk & chunk, const Log_Addr & addr) {
            unsigned int from = directory(addr);
            if(indexes((*static_cast<Log_Addr *>(phy2log(_pd)))[from]) != indexes(chunk.pt())) {
                db<MMU>(WRN) << "MMU::Directory::detach(pt=" << chunk.pt() << ",addr=" << addr << ") failed!" << endl;
                return;
            }
            detach(from, chunk.pt(), chunk.pts());
        }

        Phy_Addr physical(const Log_Addr & addr) {
            Page_Table * pt = reinterpret_cast<Page_Table *>((void *)(*_pd)[directory(addr)]);
            return (*pt)[page(addr)] | offset(addr);
        }

    private:
        bool attach(unsigned int from, const Page_Table * pt, unsigned int n, IA32_Flags flags) {
            for(unsigned int i = from; i < from + n; i++)
                if((*static_cast<Page_Directory *>(phy2log(_pd)))[i])
                    return false;
            for(unsigned int i = from; i < from + n; i++, pt++)
                (*static_cast<Page_Directory *>(phy2log(_pd)))[i] = Phy_Addr(pt) | flags;
            return true;
        }

        void detach(unsigned int from, const Page_Table * pt, unsigned int n) {
            for(unsigned int i = from; i < from + n; i++)
                (*static_cast<Page_Directory *>(phy2log(_pd)))[i] = 0;
        }

    private:
        Page_Directory * _pd;
        bool _free;
    };

    // DMA_Buffer
    class DMA_Buffer: public Chunk
    {
    public:
        DMA_Buffer(unsigned int s) : Chunk(s, IA32_Flags::DMA) {
            Directory dir(current());
            _log_addr = dir.attach(*this, MMU::directory(PHY_MEM));
            db<MMU>(TRC) << "MMU::DMA_Buffer() => " << *this << endl;
        }

        DMA_Buffer(unsigned int s, const Log_Addr & d): Chunk(s, IA32_Flags::DMA) {
            Directory dir(current());
            _log_addr = dir.attach(*this);
            memcpy(_log_addr, d, s);
            db<MMU>(TRC) << "MMU::DMA_Buffer(phy=" << *this << " <= " << d << endl;
        }

        Log_Addr log_address() const { return _log_addr; }

        friend Debug & operator<<(Debug & db, const DMA_Buffer & b) {
            db << "{phy=" << b.phy_address()
               << ",log=" << b.log_address()
               << ",size=" << b.size()
               << ",flags=" << b.flags() << "}";
            return db;
        }

    private:
        Log_Addr _log_addr;
    };

public:
    MMU() {}

    static Phy_Addr alloc(unsigned int frames = 1, const Color & color = WHITE) {
        Phy_Addr phy(false);

        if(frames) {
            List::Element * e = _free[color].search_decrementing(frames);
            if(e) {
                phy = e->object() + e->size();
                db<MMU>(TRC) << "MMU::alloc(frames=" << frames << ",color=" << color << ") => " << phy << endl;
            } else
                db<MMU>(WRN) << "MMU::alloc(frames=" << frames << ",color=" << color << ") => failed!" << endl;
        }

        return phy;
    }

    static Phy_Addr calloc(unsigned int frames = 1, const Color & color = WHITE) {
        Phy_Addr phy = alloc(frames, color);

        memset(phy2log(phy), 0, sizeof(Frame) * frames);

        return phy;
    }

    static void free(Phy_Addr frame, int n = 1) {
        // Clean up MMU flags in frame address
        frame = indexes(frame);
        Color color = colorful ? phy2color(frame) : WHITE;

        db<MMU>(TRC) << "MMU::free(frame=" << frame << ",color=" << color << ",n=" << n << ")" << endl;

        if(frame && n) {
            List::Element * e = new (phy2log(frame)) List::Element(frame, n);
            List::Element * m1, * m2;
            _free[color].insert_merging(e, &m1, &m2);
        }
    }

    static void white_free(Phy_Addr frame, int n) {
        // Clean up MMU flags in frame address
        frame = indexes(frame);

        db<MMU>(TRC) << "MMU::free(frame=" << frame << ",color=" << WHITE << ",n=" << n << ")" << endl;

        if(frame && n) {
            List::Element * e = new (phy2log(frame)) List::Element(frame, n);
            List::Element * m1, * m2;
            _free[WHITE].insert_merging(e, &m1, &m2);
        }
    }

    static unsigned int allocable(const Color & color = WHITE) { return _free[color].head() ? _free[color].head()->size() : 0; }

    static Page_Directory * volatile current() {
        return reinterpret_cast<Page_Directory * volatile>(CPU::pdp());
    }

    static Phy_Addr physical(const Log_Addr & addr) {
        Page_Directory * pd = current();
        Page_Table * pt = (*pd)[directory(addr)];
        return (*pt)[page(addr)] | offset(addr);
    }

    static void flush_tlb() {
        ASM("movl %cr3,%eax");
        ASM("movl %eax,%cr3");
    }
    static void flush_tlb(const Log_Addr & addr) {
        ASM("invlpg %0" : : "m"(addr));
    }

private:
    static void init();

    static Log_Addr phy2log(const Phy_Addr & phy) { return phy | PHY_MEM; }

    static Color phy2color(const Phy_Addr & phy) { return static_cast<Color>(colorful ? ((phy >> PAGE_SHIFT) & 0x7f) % COLORS : WHITE); } // TODO: what is 0x7f

    static Color log2color(const Log_Addr & log) {
        if(colorful) {
            Page_Directory * pd = current();
            Page_Table * pt = (*pd)[directory(log)];
            Phy_Addr phy = (*pt)[page(log)] | offset(log);
            return static_cast<Color>(((phy >> PAGE_SHIFT) & 0x7f) % COLORS);
        } else
            return WHITE;
    }

private:
    static List _free[colorful * COLORS + 1]; // +1 for WHITE
    static Page_Directory * _master;
};

__END_SYS

#endif
