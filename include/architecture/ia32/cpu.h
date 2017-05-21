// EPOS IA32 CPU Mediator Declarations

#ifndef __ia32_h
#define __ia32_h

#include <cpu.h>

__BEGIN_SYS

class CPU: public CPU_Common
{
    friend class Init_System;

public:
    // CPU Native Data Types
    using CPU_Common::Reg8;
    using CPU_Common::Reg16;
    using CPU_Common::Reg32;
    using CPU_Common::Log_Addr;
    using CPU_Common::Phy_Addr;

    // CPU Flags
    typedef Reg32 Flags;
    enum {
        FLAG_CF     = 0x00000001,
        FLAG_PF     = 0x00000004,
        FLAG_AF     = 0x00000010,
        FLAG_ZF     = 0x00000040,
        FLAG_SF     = 0x00000080,
        FLAG_TF     = 0x00000100,
        FLAG_IF     = 0x00000200,
        FLAG_DF     = 0x00000400,
        FLAG_OF     = 0x00000800,
        FLAG_IOPL1  = 0x00001000,
        FLAG_IOPL2  = 0x00002000,
        FLAG_NT     = 0x00004000,
        FLAG_RF     = 0x00010000,
        FLAG_VM     = 0x00020000,
        FLAG_AC     = 0x00040000,
        FLAG_VIF    = 0x00080000,
        FLAG_VIP    = 0x00100000,
        FLAG_ID     = 0x00200000,
        FLAG_DEFAULTS   = FLAG_IF,
        // Mask to clear flags (by ANDing)
        FLAG_CLEAR      = ~(FLAG_TF | FLAG_IOPL1 | FLAG_IOPL2 | FLAG_NT | FLAG_RF | FLAG_VM | FLAG_AC)
    };

    // CPU Exceptions
    typedef Reg32 Exceptions;
    enum {
        EXC_BASE    = 0x00,
        EXC_DIV0    = 0x00,
        EXC_DEBUG   = 0x01,
        EXC_NMI     = 0x02,
        EXC_BP      = 0x03,
        EXC_OVFLOW  = 0x04,
        EXC_BOUND   = 0x05,
        EXC_INVOP   = 0x06,
        EXC_NODEV   = 0x07,
        EXC_DOUBLE  = 0x08,
        EXC_FPU_OR  = 0x09,
        EXC_INVTSS  = 0x0a,
        EXC_NOTPRE  = 0x0b,
        EXC_STACK   = 0x0c,
        EXC_GPF     = 0x0d,
        EXC_PF      = 0x0e,
        EXC_RESERV  = 0x0f,
        EXC_FPU     = 0x10,
        EXC_ALIGN   = 0x11,
        EXC_BUS     = 0x12,
        EXC_LAST    = 0x1f
    };

    // CR0 Flags
    enum {
        CR0_PE      = 0x00000001,
        CR0_MP      = 0x00000002,
        CR0_EM      = 0x00000004,
        CR0_TS      = 0x00000008,
        CR0_ET      = 0x00000010,
        CR0_NE      = 0x00000020,
        CR0_WP      = 0x00010000,
        CR0_AM      = 0x00040000,
        CR0_NW      = 0x20000000,
        CR0_CD      = 0x40000000,
        CR0_PG      = 0x80000000,
        CR0_CLEAR       = (CR0_PE | CR0_EM | CR0_WP),   // Mask to clear flags (by ANDing)
        CR0_SET         = (CR0_PE | CR0_PG)             // Mask to set flags (by ORing)
    };

    // CR4 Flags
    enum {
        CR4_PSE     = 1 << 8    // CR4 Performance Counter Enable
    };

    // Segment Flags
    enum {
        SEG_ACC         = 0x01,
        SEG_RW          = 0x02,
        SEG_CONF        = 0x04,
        SEG_CODE        = 0x08,
        SEG_NOSYS       = 0x10,
        SEG_DPL1        = 0x20,
        SEG_DPL2        = 0x40,
        SEG_PRE         = 0x80,
        SEG_TSS         = 0x09,
        SEG_INT         = 0x0e,
        SEG_TRAP        = 0x0f,
        SEG_32          = 0x40,
        SEG_4K          = 0x80,
        SEG_FLT_CODE    = (SEG_PRE  | SEG_NOSYS | SEG_CODE | SEG_RW   | SEG_ACC  ),
        SEG_FLT_DATA    = (SEG_PRE  | SEG_NOSYS | SEG_RW   | SEG_ACC  ),
        SEG_SYS_CODE    = (SEG_PRE  | SEG_NOSYS | SEG_CODE | SEG_RW   | SEG_ACC  ),
        SEG_SYS_DATA    = (SEG_PRE  | SEG_NOSYS | SEG_RW   | SEG_ACC  ),
        SEG_APP_CODE    = (SEG_PRE  | SEG_NOSYS | SEG_DPL2 | SEG_DPL1 | SEG_CODE | SEG_RW   | SEG_ACC),   // P, DPL=3, S, C, W, A
        SEG_APP_DATA    = (SEG_PRE  | SEG_NOSYS | SEG_DPL2 | SEG_DPL1 | SEG_RW   | SEG_ACC  ),   // P, DPL=3, S,    W, A
        SEG_IDT_ENTRY   = (SEG_PRE  | SEG_INT   | SEG_DPL2 | SEG_DPL1 ),
        SEG_TSS0        = (SEG_PRE  | SEG_TSS   | SEG_DPL2 | SEG_DPL1 )
    };

    // DPL/RPL for application (user) and system (supervisor) modes
    enum {
        PL_APP = 3, // GDT, RPL=3
        PL_SYS = 0  // GDT, RPL=0
    };

    // GDT Layout
    enum GDT_Layout { // GCC BUG (anonymous enum in templates)
        GDT_NULL      = 0,
        GDT_FLT_CODE  = 1,
        GDT_FLT_DATA  = 2,
        GDT_SYS_CODE  = GDT_FLT_CODE,
        GDT_SYS_DATA  = GDT_FLT_DATA,
        GDT_APP_CODE  = 3,
        GDT_APP_DATA  = 4,
        GDT_TSS0      = 5
    };

    // GDT Selectors
    enum {
        SEL_FLT_CODE  = (GDT_FLT_CODE << 3)  | PL_SYS,
        SEL_FLT_DATA  = (GDT_FLT_DATA << 3)  | PL_SYS,
        SEL_SYS_CODE  = (GDT_SYS_CODE << 3)  | PL_SYS,
        SEL_SYS_DATA  = (GDT_SYS_DATA << 3)  | PL_SYS,
        SEL_APP_CODE  = (GDT_APP_CODE << 3)  | PL_APP,
        SEL_APP_DATA  = (GDT_APP_DATA << 3)  | PL_APP,
        SEL_TSS0      = (GDT_TSS0     << 3)  | PL_SYS
    };

    // GDT Entry
    class GDT_Entry {
    public:
        GDT_Entry() {}
        GDT_Entry(Reg32 b, Reg32 l, Reg8 f)
        : limit_15_00((Reg16)l), base_15_00((Reg16)b), base_23_16((Reg8)(b >> 16)), p_dpl_s_type(f),
          g_d_0_a_limit_19_16(((f & SEG_NOSYS) ? (SEG_4K | SEG_32) : 0) | ((Reg8)(l >> 16))), base_31_24((Reg8)(b >> 24)) {}

        friend Debug & operator<<(Debug & db, const GDT_Entry & g) {
            db << "{bas=" << (void *)((g.base_31_24 << 24) | (g.base_23_16 << 16) | g.base_15_00)
               << ",lim=" << (void *)(((g.g_d_0_a_limit_19_16 & 0xf) << 16) | g.limit_15_00)
               << ",p=" << (g.p_dpl_s_type >> 7)
               << ",dpl=" << ((g.p_dpl_s_type >> 5) & 0x3)
               << ",s=" << ((g.p_dpl_s_type >> 4) & 0x1)
               << ",typ=" << (g.p_dpl_s_type & 0xf)
               << ",g=" << (g.g_d_0_a_limit_19_16 >> 7)
               << ",d=" << ((g.g_d_0_a_limit_19_16 >> 6) & 0x1)
               << ",a=" << ((g.g_d_0_a_limit_19_16 >> 4) & 0x1) << "}";
            return db;
        }

    private:
        Reg16 limit_15_00;
        Reg16 base_15_00;
        Reg8  base_23_16;
        Reg8  p_dpl_s_type;
        Reg8  g_d_0_a_limit_19_16;
        Reg8  base_31_24;
    };

    // IDT Entry
    class IDT_Entry {
    public:
        IDT_Entry() {}
        IDT_Entry(Reg16 s, Reg32 o, Reg16 f)
        : offset_15_00((Reg16)o), selector(s), zero(0), p_dpl_0_d_1_1_0(f), offset_31_16((Reg16)(o >> 16)) {}

        Reg32 offset() const { return (offset_31_16 << 16) | offset_15_00; }

        friend Debug & operator<<(Debug & db, const IDT_Entry & i) {
            db << "{sel=" << i.selector
               << ",off=" << (void *)i.offset()
               << ",p=" << (i.p_dpl_0_d_1_1_0 >> 7)
               << ",dpl=" << ((i.p_dpl_0_d_1_1_0 >> 5) & 0x3)
               << ",d=" << ((i.p_dpl_0_d_1_1_0 >> 4) & 0x1) << "}";
            return db;
        }

    private:
        Reg16 offset_15_00;
        Reg16 selector;
        Reg8  zero;
        Reg8  p_dpl_0_d_1_1_0;
        Reg16 offset_31_16;
    };
    static const unsigned int IDT_ENTRIES = 256;

    // TSS no longer used, since software context switch is faster
    // it's left here for reference
    struct TSS {
        Reg16 back_link;
        Reg16 zero1;
        Reg32 esp0;
        Reg16 ss0;
        Reg16 zero2;
        Reg32 esp1;
        Reg16 ss1;
        Reg16 zero3;
        Reg32 esp2;
        Reg16 ss2;
        Reg16 zero4;
        Reg32 pdbr;
        Reg32 eip;
        Reg32 eflags;
        Reg32 eax;
        Reg32 ecx;
        Reg32 edx;
        Reg32 ebx;
        Reg32 esp;
        Reg32 ebp;
        Reg32 esi;
        Reg32 edi;
        Reg16 es;
        Reg16 zero5;
        Reg16 cs;
        Reg16 zero6;
        Reg16 ss;
        Reg16 zero7;
        Reg16 ds;
        Reg16 zero8;
        Reg16 fs;
        Reg16 zero9;
        Reg16 gs;
        Reg16 zero10;
        Reg16 ldt;
        Reg16 zero11;
        Reg16 zero12;
        Reg16 io_bmp;
    };

    // CPU Context
    class Context
    {
    public:
        Context(const Log_Addr & usp, const Log_Addr & entry): _esp3(usp), _eip(entry), _cs(((Traits<Build>::MODE == Traits<Build>::KERNEL) && usp)? SEL_APP_CODE : SEL_SYS_CODE), _eflags(FLAG_DEFAULTS) {}

        void save() volatile;
        void load() const volatile;

        friend Debug & operator<<(Debug & db, const Context & c) {
            db << hex
               << "{eflags=" << c._eflags
               << ",eax=" << c._eax
               << ",ebx=" << c._ebx
               << ",ecx=" << c._ecx
               << ",edx=" << c._edx
               << ",esi=" << c._esi
               << ",edi=" << c._edi
               << ",ebp=" << reinterpret_cast<void *>(c._ebp)
               << ",esp=" << &c
               << ",eip=" << reinterpret_cast<void *>(c._eip)
               << ",esp3="<< c._esp3
               << ",cs="  << c._cs
               << ",ccs=" << cs()
               << ",cds=" << ds()
               << ",ces=" << es()
               << ",cfs=" << fs()
               << ",cgs=" << gs()
               << ",css=" << ss()
               << ",cr3=" << reinterpret_cast<void *>(pdp())
               << "}"     << dec;
            return db;
        }

    private:
        Reg32 _esp3; // only used in multitasking environments
        Reg32 _edi;
        Reg32 _esi;
        Reg32 _ebp;
        Reg32 _esp; // redundant (=this); see cpu.cc for details
        Reg32 _ebx;
        Reg32 _edx;
        Reg32 _ecx;
        Reg32 _eax;
        Reg32 _eip;
        Reg32 _cs;
        Reg32 _eflags;
    };

    // I/O ports
    typedef Reg16 IO_Port;
    typedef Reg16 IO_Irq;

    // Interrupt Service Routines
    typedef void (ISR)();

    // Fault Service Routines (exception handlers)
    typedef void (FSR)(Reg32 error, Reg32 eip, Reg32 cs, Reg32 eflags);

public:
    CPU() {}

    static Hertz clock() { return _cpu_clock; }
    static Hertz bus_clock() { return _bus_clock; }

    static void int_enable() { ASM("sti"); }
    static void int_disable() { ASM("cli"); }
    static bool int_enabled() { return (flags() & FLAG_IF); }
    static bool int_disabled() { return !int_enabled(); }

    static void halt() { ASM("hlt"); }

    static void switch_context(Context * volatile * o, Context * volatile n);

    static void syscall(void * message);
    static void syscalled();

    static Flags flags() { return eflags(); }
    static void flags(const Flags flags) { eflags(flags); }

    static Reg32 sp() { return esp(); }
    static void sp(const Reg32 sp) { esp(sp); }

    static Reg32 fr() { return eax(); }
    static void fr(const Reg32 sp) { eax(sp); }

    static Log_Addr ip() { return eip(); }

    static Reg32 pdp() { return cr3() ; }
    static void pdp(const Reg32 pdp) { cr3(pdp); }

    template<typename T>
    static T tsl(volatile T & lock) {
        register T old = 1;
        ASM("lock xchg %0, %2" : "=a"(old) : "a"(old), "m"(lock) : "memory");
        return old;
    }

    template<typename T>
    static T finc(volatile T & value) {
        register T old = 1;
        ASM("lock xadd %0, %2" : "=a"(old) : "a"(old), "m"(value) : "memory");
        return old;
    }

    template<typename T>
    static T fdec(volatile T & value) {
        register T old = -1;
        ASM("lock xadd %0, %2" : "=a"(old) : "a"(old), "m"(value) : "memory");
        return old;
    }

    template<typename T>
    static T cas(volatile T & value, T compare, T replacement) {
        ASM("lock cmpxchgl %2, %3\n" : "=a"(compare) : "a"(compare), "r"(replacement), "m"(value) : "memory");
        return compare;
   }

    static Reg32 htolel(Reg32 v) { return v; }
    static Reg16 htoles(Reg16 v) { return v; }
    static Reg32 letohl(Reg32 v) { return v; }
    static Reg16 letohs(Reg16 v) { return v; }
    static Reg32 htonl(Reg32 v) { ASM("bswap %0" : "=r"(v) : "0"(v), "r"(v)); return v; }
    static Reg16 htons(Reg16 v) { return swap16(v); }
    static Reg32 ntohl(Reg32 v) { return htonl(v); }
    static Reg16 ntohs(Reg16 v) { return htons(v); }

    template<typename ... Tn>
    static Context * init_stack(const Log_Addr & usp, Log_Addr sp, void (* exit)(), int (* entry)(Tn ...), Tn ... an) {
        // IA32 first decrements the stack pointer and then writes into the stack
        sp -= SIZEOF<Tn ... >::Result;
        init_stack_helper(sp, an ...);
        sp -= sizeof(int *);
        *static_cast<int *>(sp) = Log_Addr(exit);
        if(usp) {
            sp -= sizeof(int *);
            *static_cast<int *>(sp) = Log_Addr(SEL_APP_DATA);
            sp -= sizeof(int *);
            *static_cast<int *>(sp) = usp;
        }
        sp -= sizeof(Context);
        return new (sp) Context(usp, entry);
    }
    template<typename ... Tn>
    static Log_Addr init_user_stack(Log_Addr sp, void (* exit)(), Tn ... an) {
        // IA32 first decrements the stack pointer and then writes into the stack
        sp -= SIZEOF<Tn ... >::Result;
        init_stack_helper(sp, an ...);
        if(exit) {
            sp -= sizeof(int *);
            *static_cast<int *>(sp) = Log_Addr(exit);
        }
        return sp;
    }

public:
    // IA32 specific methods
    static Flags eflags() {
        Reg32 value; ASM("pushfl");
        ASM("popl %0" : "=r"(value) :); return value;
    }
    static void eflags(const Flags value) {
         ASM("pushl %0" : : "r"(value)); ASM("popfl");
    }

    static Reg32 esp() {
        Reg32 value; ASM("movl %%esp,%0" : "=r"(value) :); return value;
    }
    static void esp(const Reg32 value) {
        ASM("movl %0, %%esp" : : "r"(value));
    }

    static Reg32 eax() {
        Reg32 value; ASM("movl %%eax,%0" : "=r"(value) :); return value;
    }
    static void eax(const Reg32 value) {
        ASM("movl %0, %%eax" : : "r"(value));
    }

    static Log_Addr eip() {
        Log_Addr value;
        ASM("       push    %%eax                           \n"
            "       call    1f                              \n"
            "1:     popl    %%eax       # ret. addr.        \n"
            "       movl    %%eax,%0                        \n"
            "       popl    %%eax                           \n" : "=o"(value) : : "%eax");
        return value;
    }

    static void cpuid(Reg32 op, Reg32 * eax, Reg32 * ebx, Reg32 * ecx, Reg32 * edx) {
        *eax = op;
        ASM("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "0"(*eax), "2"(*ecx));
    }

    static Reg32 cr0() {
        Reg32 value; ASM("movl %%cr0, %0" : "=r"(value) :); return value;
    }
    static void cr0(const Reg32 value) {
        ASM("movl %0, %%cr0" : : "r"(value));
    }

    static Reg32 cr2() {
        Reg32 value; ASM("movl %%cr2, %0" : "=r"(value) :); return value;
    }

    static Reg32 cr3() {
        Reg32 value; ASM("movl %%cr3, %0" : "=r"(value) :); return value;
    }
    static void cr3(const Reg32 value) {
        ASM("movl %0, %%cr3" : : "r"(value));
    }

    static Reg32 cr4() {
        Reg32 value; ASM("movl %%cr4, %0" : "=r"(value) :); return value;
    }
    static void cr4(const Reg32 value) {
        ASM("movl %0, %%cr4" : : "r"(value));
    }

    static void gdtr(Reg16 * limit, Reg32 * base) {
        volatile Reg8 aux[6];
        volatile Reg16 * l = reinterpret_cast<volatile Reg16 *>(&aux[0]);
        volatile Reg32 * b = reinterpret_cast<volatile Reg32 *>(&aux[2]);

        ASM("sgdt %0" : "=m"(aux[0]) :);
        *limit = *l;
        *base = *b;
    }
    static void gdtr(const Reg16 limit, const Reg32 base) {
        volatile Reg8 aux[6];
        volatile Reg16 * l = reinterpret_cast<volatile Reg16 *>(&aux[0]);
        volatile Reg32 * b = reinterpret_cast<volatile Reg32 *>(&aux[2]);

        *l = limit;
        *b = base;
        ASM("lgdt %0" : : "m"(aux[0]));
    }

    static void idtr(Reg16 * limit, Reg32 * base) {
        volatile Reg8 aux[6];
        volatile Reg16 * l = reinterpret_cast<volatile Reg16 *>(&aux[0]);
        volatile Reg32 * b = reinterpret_cast<volatile Reg32 *>(&aux[2]);

        ASM("sidt %0" : "=m"(aux[0]) :);
        *limit = *l;
        *base = *b;
    }
    static void idtr(const Reg16 limit, const Reg32 base) {
        volatile Reg8 aux[6];
        volatile Reg16 * l = reinterpret_cast<volatile Reg16 *>(&aux[0]);
        volatile Reg32 * b = reinterpret_cast<volatile Reg32 *>(&aux[2]);

        *l = limit;
        *b = base;
        ASM("lidt %0" : : "m" (aux[0]));
    }

    static Reg16 cs() {
        Reg16 value; ASM("mov %%cs,%0" : "=r"(value) :); return value;
    }
    static Reg16 ds() {
        Reg16 value; ASM("mov %%ds,%0" : "=r"(value) :); return value;
    }
    static Reg16 es() {
        Reg16 value; ASM("mov %%es,%0" : "=r"(value) :); return value;
    }
    static Reg16 ss() {
        Reg16 value; ASM("mov %%ss,%0" : "=r"(value) :); return value;
    }
    static Reg16 fs() {
        Reg16 value; ASM("mov %%fs,%0" : "=r"(value) :); return value;
    }
    static Reg16 gs() {
        Reg16 value; ASM("mov %%gs,%0" : "=r"(value) :); return value;
    }

    static Reg16 tr() {
        Reg16 tr;
        ASM("str %0" : "=r"(tr) :);
        return tr;
    }
    static void tr(Reg16 tr) {
        ASM("ltr %0" : : "r"(tr));
    }

    static void bts(Log_Addr addr, const int bit) {
        ASM("bts %1,%0" : "=m"(addr) : "r"(bit));
    }
    static void btr(Log_Addr addr, const int bit) {
        ASM("btr %1,%0" : "=m"(addr) : "r"(bit));
    }

    static int bsf(Log_Addr addr) {
        register unsigned int pos;
        ASM("bsf %1,%0" : "=a"(pos) : "m"(addr) : );
        return pos;
    }
    static int bsr(Log_Addr addr) {
        register int pos = -1;
        ASM("bsr %1, %0" : "=a"(pos) : "m"(addr) : );
        return pos;
    }

    static Reg64 rdmsr(Reg32 msr) {
        Reg64 v;
        ASM("rdmsr" : "=A"(v) : "c"(msr));
        return v;
    }
    static void wrmsr(Reg32 msr, Reg64 v) {
        ASM("wrmsr" : : "c"(msr), "A"(v));
    }

    static Reg8 in8(const IO_Port & port) {
        Reg8 value;
        ASM("inb %1,%0" : "=a"(value) : "d"(port));
        return value;
    }
    static Reg16 in16(const IO_Port & port) {
        Reg16 value;
        ASM("inw %1,%0" : "=a"(value) : "d"(port));
        return value;
    }
    static Reg32 in32(const IO_Port & port) {
        Reg32 value;
        ASM("inl %1,%0" : "=a"(value) : "d"(port));
        return value;
    }
    static void out8(const IO_Port & port, const Reg8 & value) {
        ASM("outb %1,%0" : : "d"(port), "a"(value));
    }
    static void out16(const IO_Port & port, const Reg16 & value) {
        ASM("outw %1,%0" : : "d"(port), "a"(value));
    }
    static void out32(const IO_Port & port, const Reg32 & value) {
        ASM("outl %1,%0" : : "d"(port), "a"(value));
    }

    static void switch_tss(const Reg32 & selector) {
        struct {
            Reg32 offset;
            Reg32 selector;
        } address;

        address.offset   = 0;
        address.selector = selector;

        ASM("ljmp *%0" : "=o" (address));
    }

private:
    template<typename Head, typename ... Tail>
    static void init_stack_helper(Log_Addr sp, Head head, Tail ... tail) {
        *static_cast<Head *>(sp) = head;
        init_stack_helper(sp + sizeof(Head), tail ...);
    }
    static void init_stack_helper(Log_Addr sp) {}

    static void init();

private:
    static unsigned int _cpu_clock;
    static unsigned int _bus_clock;
};

inline CPU::Reg32 htolel(CPU::Reg32 v) { return CPU::htolel(v); }
inline CPU::Reg16 htoles(CPU::Reg16 v) { return CPU::htoles(v); }
inline CPU::Reg32 letohl(CPU::Reg32 v) { return CPU::letohl(v); }
inline CPU::Reg16 letohs(CPU::Reg16 v) { return CPU::letohs(v); }
inline CPU::Reg32 htonl(CPU::Reg32 v) { return CPU::htonl(v); }
inline CPU::Reg16 htons(CPU::Reg16 v) { return CPU::htons(v); }
inline CPU::Reg32 ntohl(CPU::Reg32 v) { return CPU::ntohl(v); }
inline CPU::Reg16 ntohs(CPU::Reg16 v) { return CPU::ntohs(v); }

__END_SYS

#endif
