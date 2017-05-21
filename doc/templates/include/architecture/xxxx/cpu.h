// EPOS XXXX CPU Mediator Declarations

#ifndef __xxxx_h
#define __xxxx_h

#include <cpu.h>

__BEGIN_SYS

class XXXX: public CPU_Common
{
    friend class Init_System;

public:
    // CPU Flags
    typedef RegNN Flags;
    enum {
        FLAG_AA		= 0x00000001,
        FLAG_BB		= 0x00000002,
        FLAG_DEFAULTS   = FLAG_AA | FLAG_BB,
        // Mask to clear flags (by ANDing)
        FLAG_CLEAR      = ~(FLAG_AA | FLAG_BB)
    };

    // CPU Exceptions
    typedef RegNN Exceptions;
    enum {
        EXC_DIV0        = 0x00,
    };

    // CPU Context
    class Context
    {
    public:
        Context(Log_Addr entry) : _flags(FLAG_DEFAULTS), _ip(entry) {}

        void save() volatile;
        void load() const volatile;

        friend Debug & operator<<(Debug & db, const Context & c) {
            db << hex
               << "{pc=" << c._pc
               << ",sp=" << c._sp
               << ",lr=" << c._lr
               << ",r0=" << c._r0
               << ",rn=" << c._rn
               << "}" << dec;
            return db;
        }

    private:
        Reg32 _pc;
        Reg32 _sp;
        Reg32 _lr;
        Reg32 _r0;
        Reg32 _rn;
    };

    // I/O ports
    typedef RegNN IO_Port;
    typedef RegNN IO_Irq;

    // Interrupt Service Routines
    typedef void (ISR)();

    // Fault Service Routines
    typedef void (FSR)();

public:
    XXXX() {}

    static Hertz clock() { return _cpu_clock; }
    static Hertz bus_clock() { return _bus_clock; }

    static void int_enable() {
        /* ASM to enable/unmask interrupts within the CPU (not on the devices/busses/ICs) */
        ASM("nop");
    }
    static void int_disable() {
        /* ASM to disable/mask interrupts within the CPU (not on the devices/busses/ICs) */
        ASM("nop");
    }
    static bool int_enabled() {
        return (flags() & FLAG_IF);
    }

    static void halt() {
        /* ASM to halt the CPU (i.e. put it to sleep in the lowest power consumption mode that is still able to wakeup on an interrup */
        ASM("nop");
    }

    static Flags flags() {
        RegNN value;
        /* ASM to read the flags */
        ASM("nop" : "=r"(value) :);
        return value;
    }
    static void flags(const Flags & flags) {
        /* ASM to set the flags */
        ASM("nop" : : "r"(value));
    }

    static RegNN sp() {
        RegNN value;
        /* ASM to read the stack pointer */
        ASM("nop" : "=r"(value) :);
        return value;
    }
    static void sp(const RegNN & value) {
        /* ASM to set the stack pointer */
        ASM("nop" : : "r"(value));
    }

    static RegNN fr() {
        /* ASM to read a function return value according to the current cdecl spec */
        /* Either a register or the "top" of the stack */
        Reg32 value;
        ASM("nop" : "=r"(value) :);
        return value;
    }
    static void fr(const RegNN & value) {
        /* ASM to set a function return value according to the current cdecl spec */
        /* Either a register or the "top" of the stack */
        ASM("nop" : : "r"(value));
    }

    static Log_Addr ip() {
        /* ASM to read the Instruction Pointer register */
        /* Either a register (e.g. LR) ... */
        Log_Addr value;
        ASM("nop" : "=r"(value) :);
        /* or through a fake call like this */
        ASM("          push    %%r0            # save r0                                       \n"
             "          call    1f              # relative call to 1:                           \n"
             "1:        popl    %%r0            # pop the return address (i.e. IP) into r0      \n"
             "          store   %%r0,%0         # store r0 to valeu                             \n"
             "          popl    %%r0            # restore r0                                    \n"
             : "=o"(value)
             : );
        return value;
    }

    static RegNN pdp() { return 0 ; /* if no paging is available/implemented */ }
    static void pdp(const RegNN & pdp) { /* if no paging is available/implemented */ }

    template <typename T>
    static T tsl(volatile T & lock) {
        /* ASM to perform an atomic TSL. Available from the Common Package if not supported natively */
        register T old = 1;
        ASM("nop"
             : "=a"(old) 
             : "a"(old), "m"(lock) 
             : "memory"); 
        return old;
    }

    template <typename T>
    static T finc(volatile T & value) {
        /* ASM to perform an atomic FINC. Available from the Common Package if not supported natively */
        register T old;
        ASM("nop"
             : "=a"(old)
             : "m"(value)
             : "memory"); 
        return old;
    }

    template <typename T>
    static T fdec(volatile T & value) {
        /* ASM to perform an atomic FDEC. Available from the Common Package if not supported natively */
        register T old;
        ASM("nop"
             : "=a"(old)
             : "m"(value)
             : "memory"); 
        return old;
    }

    template <typename T>
    static T cas(volatile T & value, T compare, T replacement) {
        /* ASM to perform an atomic CAS. Available from the Common Package if not supported natively */
        /* Semantics are as follows:
         * int cas(volatile int & value, int compare, int replacement)
         * {
         *     ATOMIC();
         *     int old = value;
         *     if(value == compare)
         *         value = replacement;
         *     END_ATOMIC();
         *     return old;
         * }
         */
        ASM("nop"
             : "=a"(compare) 
             : "a"(compare), "r"(replacement), "m"(value)
             : "memory");
        return compare;
   }

    static Reg32 htonl(Reg32 v)	{
        /* ASM to swap bytes in a little-endian, 32-bit word to make it big-endian */
        /* Null if this architecture is big-endian. Available from the Common Package if not supported natively */
 	ASM("nop" : "=r" (v) : "0" (v), "r" (v));
 	return v;
    }
    static Reg16 htons(Reg16 v)	{
        /* ASM to swap bytes in a little-endian, 16-bit word to make it big-endian */
        /* Null if this architecture is big-endian. Available from the Common Package if not supported natively */
        ASM("nop" : "=r" (v) : "0" (v), "r" (v));
        return v;
    }
    static Reg32 ntohl(Reg32 v)	{ return htonl(v); }
    static Reg16 ntohs(Reg16 v)	{ return htons(v); }

    /* This is a very complex and important function. In order to drastically reduce architecture-dependent
     * code, EPOS factored out over a dozen architectures on the search for a context switch scheme that could
     * operate properly across all of them. The major requirement was a mean to save user-visible CPU registers
     * without touching them. Since many architecture lack OS-specific registers, the solution was to model
     * the context as a floating object on the stack. It will often be implemented in cpu.cc and compile with
     * -fomit-frame-pointer. Further hints there. */
    static void switch_context(Context * volatile * o, Context * volatile n);

    /* The integer left on the stack between a thread's arguments and its context is due to the fact that the
     * thread's function believes it's a normal function that will be invoked through a call and thus pushes the
     * return address on the stack. In this case, a call to Thread::exit(). */
    static Context * init_stack(Log_Addr stack, unsigned int size, void (* exit)(), int (* entry)()) {
        Log_Addr sp = stack + size;
        sp -= sizeof(int); *static_cast<int *>(sp) = Log_Addr(exit);
        sp -= sizeof(Context);
        return new (sp) Context(entry);
    }

    template<typename T1>
    static Context * init_stack(Log_Addr stack, unsigned int size, void (* exit)(), int (* entry)(T1 a1), T1 a1) {
        Log_Addr sp = stack + size;
        sp -= sizeof(T1); *static_cast<T1 *>(sp) = a1;
        sp -= sizeof(int); *static_cast<int *>(sp) = Log_Addr(exit);
        sp -= sizeof(Context);
        return new (sp) Context(entry);
    }

    template<typename T1, typename T2>
    static Context * init_stack(Log_Addr stack, unsigned int size, void (* exit)(), int (* entry)(T1 a1, T2 a2), T1 a1, T2 a2) {
        Log_Addr sp = stack + size;
        sp -= sizeof(T2); *static_cast<T2 *>(sp) = a2;
        sp -= sizeof(T1); *static_cast<T1 *>(sp) = a1;
        sp -= sizeof(int); *static_cast<int *>(sp) = Log_Addr(exit);
        sp -= sizeof(Context);
        return new (sp) Context(entry);
    }

    template<typename T1, typename T2, typename T3>
    static Context * init_stack(Log_Addr stack, unsigned int size, void (* exit)(), int (* entry)(T1 a1, T2 a2, T3 a3), T1 a1, T2 a2, T3 a3) {
        Log_Addr sp = stack + size;
        sp -= sizeof(T3); *static_cast<T3 *>(sp) = a3;
        sp -= sizeof(T2); *static_cast<T2 *>(sp) = a2;
        sp -= sizeof(T1); *static_cast<T1 *>(sp) = a1;
        sp -= sizeof(int); *static_cast<int *>(sp) = Log_Addr(exit);
        sp -= sizeof(Context);
        return new (sp) Context(entry);
    }

    template<typename T1, typename T2, typename T3, typename T4>
    static Context * init_stack(Log_Addr stack, unsigned int size, void (* exit)(), int (* entry)(T1 a1, T2 a2, T3 a3, T4 a4), T1 a1, T2 a2, T3 a3, T4 a4) {
        Log_Addr sp = stack + size;
        sp -= sizeof(T4); *static_cast<T4 *>(sp) = a4;
        sp -= sizeof(T3); *static_cast<T3 *>(sp) = a3;
        sp -= sizeof(T2); *static_cast<T2 *>(sp) = a2;
        sp -= sizeof(T1); *static_cast<T1 *>(sp) = a1;
        sp -= sizeof(int); *static_cast<int *>(sp) = Log_Addr(exit);
        sp -= sizeof(Context);
        return new (sp) Context(entry);
    }

public: // XXXX specific methods

    /* This section is reserved for any additional method that is relevant to abstract a given architecture.
     * However, they should never be used out of a mediators. */

    static Reg8 in8(const IO_Port & port) {
        /* ASM to read a byte from the I/O address space */
        Reg8 value;
        ASM("nop" : "=a"(value) : "r"(port));
        return value;
    }
    static Reg16 in16(const IO_Port & port) {
        /* ASM to read a 16-bit word from the I/O address space */
        Reg16 value;
        ASM("nop" : "=a"(value) : "r"(port));
        return value;
    }
    static Reg32 in32(const IO_Port & port) {
        /* ASM to read a 32-bit word from the I/O address space */
        Reg32 value;
        ASM("nop" : "=a"(value) : "r"(port));
        return value;
    }
    static void out8(const IO_Port & port, const Reg8 & value) {
        /* ASM to write a byte to the I/O address space */
        ASM("nop" : : "r"(port), "a"(value));
    }
    static void out16(const IO_Port & port, const Reg16 & value) {
        /* ASM to write a 16-bit to from the I/O address space */
        ASM("nop" : : "r"(port), "a"(value));
    }
    static void out32(const IO_Port & port, const Reg32 & value) {
        /* ASM to write a 32-bit to from the I/O address space */
        ASM("nop" : : "r"(port), "a"(value));
    }

private:
    static void init();

private:
    static unsigned int _cpu_clock;
    static unsigned int _bus_clock;
};

inline CPU::Reg32 htonl(CPU::Reg32 v) { return CPU::htonl(v); }
inline CPU::Reg16 htons(CPU::Reg16 v) { return CPU::htons(v); }
inline CPU::Reg32 ntohl(CPU::Reg32 v) { return CPU::ntohl(v); }
inline CPU::Reg16 ntohs(CPU::Reg16 v) { return CPU::ntohs(v); }

__END_SYS

#endif
