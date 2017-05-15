// EPOS YYYY Interrupt Controller Mediator Declarations

#ifndef __yyyy_ic_h
#define __yyyy_ic_h

#include <cpu.h>
#include <ic.h>
#include <mach/yyyy/memory_map.h>

__BEGIN_SYS

// YYYY Interrupt Controller
class CHIP_YYYY_IC
{
private:
    static const unsigned int INTS = 64;
    static const unsigned int IRQS = 16;
    static const unsigned int HARD_INT = 32;
    static const unsigned int SOFT_INT = HARD_INT + IRQS;

public:
    CHIP_YYYY_IC() {}
};

class YYYY_IC: public IC_Common, private CHIP_YYYY_IC
{
    friend class YYYY;

private:
    typedef CPU::Reg32 Reg32;
    typedef CPU::Log_Addr Log_Addr;

public:
    using CHIP_YYYY_IC::INT_TIMER;
    using CHIP_YYYY_IC::INT_RESCHEDULER;

public:
    YYYY_IC() {}

    static Interrupt_Handler int_vector(Interrupt_Id i) {
        return (i < INTS) ? _int_vector[i] : 0;
    }

    static void int_vector(Interrupt_Id i, Interrupt_Handler h) {
        db<IC>(TRC) << "IC::int_vector(int=" << i << ",h=" << reinterpret_cast<void *>(h) <<")" << endl;
        if(i < INTS)
            _int_vector[i] = h;
    }

    static void enable() {
        db<IC>(TRC) << "IC::enable()" << endl;
        CHIP_YYYY_IC::enable();
    }

    static void enable(int i) {
        db<IC>(TRC) << "IC::enable(int=" << i << ")" << endl;
        CHIP_YYYY_IC::enable(i);
    }

    static void disable() {
        db<IC>(TRC) << "IC::disable()" << endl;
        CHIP_YYYY_IC::disable();
    }

    static void disable(int i) {
        db<IC>(TRC) << "IC::disable(int=" << i << ")" << endl;
        CHIP_YYYY_IC::disable(i);
    }

    static bool eoi(unsigned int i) {
        /* This method is very important. It acknowledges interrupts, which for EPOS is done before invoking the ISR, thus enabling
         * reentrance. If an interrupt is detected as spurious, then the method must return false, therefore avoiding ISR dispatching */
    }

    static int irq2int(int i) { return i + HARD_INT; }
    static int int2irq(int i) { return i - HARD_INT; }


private:
    static void dispatch(unsigned int i) {
        bool not_spurious = true;
        if((i >= INT_FIRST_HARD) && (i <= INT_LAST_HARD))
            not_spurious = eoi(i);
        if(not_spurious) {
            if((i != INT_TIMER) || Traits<IC>::hysterically_debugged)
                db<IC>(TRC) << "IC::dispatch(i=" << i << ")" << endl;
            _int_vector[i](i);
        } else
            db<IC>(TRC) << "IC::spurious interrupt (" << i << ")" << endl;
    }

    /* This method is the one pointed to by the interrupt vector. It either expands dispatch(i) directly if interrupt IDs are
     * available to the CPU on an internal register or used to find out the ID before invoking dispatch(i).
     * It is often implemented in a separate .cc file and compiled with -fomit-frame-pointer. */
    static void entry();

    static void int_not(Interrupt_Id i) {
        /* This method will be called for unassigned interrupts */
    }
    static void exc_not(Interrupt_Id i) {
        /* This method will be called for unhandled CPU exceptions */
    }

    static void init();

private:
    static Interrupt_Handler _int_vector[INTS];
};

__END_SYS

#endif
