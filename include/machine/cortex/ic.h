// EPOS ARM Cortex IC Mediator Declarations

#ifndef __cortex_ic_h
#define __cortex_ic_h

#include <cpu.h>
#include <ic.h>
#include __MODEL_H

__BEGIN_SYS

class NVIC;
class GIC;

#ifdef __mmod_zynq__

class GIC: public IC_Common, protected Machine_Model
{
public:
    // IRQs
    static const unsigned int IRQS = Machine_Model::IRQS;
    typedef Interrupt_Id IRQ;
    enum {
        IRQ_SOFTWARE0           = 0,
        IRQ_SOFTWARE1           = 1,
        IRQ_SOFTWARE2           = 2,
        IRQ_SOFTWARE3           = 3,
        IRQ_SOFTWARE4           = 4,
        IRQ_SOFTWARE5           = 5,
        IRQ_SOFTWARE6           = 6,
        IRQ_SOFTWARE7           = 7,
        IRQ_SOFTWARE8           = 8,
        IRQ_SOFTWARE9           = 9,
        IRQ_SOFTWARE10          = 10,
        IRQ_SOFTWARE11          = 11,
        IRQ_SOFTWARE12          = 12,
        IRQ_SOFTWARE13          = 13,
        IRQ_SOFTWARE14          = 14,
        IRQ_SOFTWARE15          = 15,
        IRQ_GLOBAL_TIMER        = 27,
        IRQ_NFIQ                = 28,
        IRQ_PRIVATE_TIMER       = 29,
        IRQ_AWDT                = 30,
        IRQ_NIRQ                = 31,
        IRQ_APU0                = 32,
        IRQ_APU1                = 33,
        IRQ_L2                  = 34,
        IRQ_OCM                 = 35,
        IRQ_PMU0                = 37,
        IRQ_PMU1                = 38,
        IRQ_XADC                = 39,
        IRQ_DEVC                = 40,
        IRQ_SWDT                = 41,
        IRQ_TTC0_0              = 42,
        IRQ_TTC0_1              = 43,
        IRQ_TTC0_2              = 44,
        IRQ_DMAC_ABORT          = 45,
        IRQ_DMAC0               = 46,
        IRQ_DMAC1               = 47,
        IRQ_DMAC2               = 48,
        IRQ_DMAC3               = 49,
        IRQ_SMC                 = 50,
        IRQ_QSPI                = 51,
        IRQ_GPIO                = 52,
        IRQ_USB0                = 53,
        IRQ_ETHERNET0           = 54,
        IRQ_ETHERNET0_WAKEUP    = 55,
        IRQ_SDIO0               = 56,
        IRQ_I2C0                = 57,
        IRQ_SPI0                = 58,
        IRQ_UART0               = 59,
        IRQ_CAN0                = 60,
        IRQ_PL0                 = 61,
        IRQ_PL1                 = 62,
        IRQ_PL2                 = 63,
        IRQ_PL3                 = 64,
        IRQ_PL4                 = 65,
        IRQ_PL5                 = 66,
        IRQ_PL6                 = 67,
        IRQ_PL7                 = 68,
        IRQ_TTC1_0              = 69,
        IRQ_TTC1_1              = 70,
        IRQ_TTC1_2              = 71,
        IRQ_DMAC4               = 72,
        IRQ_DMAC5               = 73,
        IRQ_DMAC6               = 74,
        IRQ_DMAC7               = 75,
        IRQ_USB1                = 76,
        IRQ_ETHERNET1           = 76,
        IRQ_ETHERNET1_WAKEUP    = 78,
        IRQ_SDIO1               = 79,
        IRQ_I2C1                = 80,
        IRQ_SPI1                = 81,
        IRQ_UART1               = 82,
        IRQ_CAN1                = 83,
        IRQ_PL8                 = 84,
        IRQ_PL9                 = 85,
        IRQ_PL10                = 86,
        IRQ_PL11                = 87,
        IRQ_PL12                = 88,
        IRQ_PL13                = 89,
        IRQ_PL14                = 90,
        IRQ_PL15                = 91,
        IRQ_PARITY              = 92,
    };

    // Interrupts
    static const unsigned int INTS = 93 + 1;
    static const unsigned int EXC_INT = 0; // Not mapped by IC. Exceptions are hard configured by SETUP.
    static const unsigned int HARD_INT = 16;
    static const unsigned int SOFT_INT = 0;
    enum {
        INT_TIMER       = IRQ_PRIVATE_TIMER,
        INT_USER_TIMER0 = IRQ_GLOBAL_TIMER,
        INT_USER_TIMER1 = 0,
        INT_USER_TIMER2 = 0,
        INT_USER_TIMER3 = 0,
        INT_USB0         = IRQ_USB0,
        INT_GPIOA       = IRQ_GPIO,
        INT_GPIOB       = IRQ_GPIO,
        INT_GPIOC       = IRQ_GPIO,
        INT_GPIOD       = IRQ_GPIO,
        INT_NIC0_RX     = IRQ_ETHERNET0,
        INT_NIC0_TX     = IRQ_ETHERNET0,
        INT_NIC0_ERR    = IRQ_ETHERNET0,
        INT_NIC0_TIMER  = 0,
        INT_FIRST_HARD  = HARD_INT,
        INT_LAST_HARD   = IRQ_PARITY,
        INT_RESCHEDULER = IRQ_SOFTWARE0
    };

public:
    GIC() {}

    static int irq2int(int i) { return i; }
    static int int2irq(int i) { return i; }

    static void enable() {
        dist(ICDISER0) = ~0;
        dist(ICDISER1) = ~0;
        dist(ICDISER2) = ~0;
    }

    static void enable(int i) { dist(ICDISER0 + (i/32)*4) = 1 << (i%32); }

    static void disable() {
        dist(ICDICER0) = ~0;
        dist(ICDICER1) = ~0;
        dist(ICDICER1) = ~0;
    }

    static void disable(int i) { dist(ICDICER0 + (i/32)*4) = 1 << (i%32); }

    static Interrupt_Id int_id() {
        Reg32 icciar = cpu_itf(ICCIAR) & INT_ID_MASK;

        // For every read of a valid interrupt id from the ICCIAR, the ISR must
        // perform a matching write to the ICCEOIR
        cpu_itf(ICCEOIR) = icciar;
        return icciar;
    }

    static void init(void) {
        // Enable distributor
        dist(ICDDCR) = DIST_EN_S;

        // Mask no interrupt
        cpu_itf(ICCPMR) = 0xF0;

        // Enable interrupts signaling by the CPU interfaces to the connected
        // processors
        cpu_itf(ICCICR) = ACK_CTL | ITF_EN_NS | ITF_EN_S;
    }

protected:
    static const unsigned int INT_ID_MASK = 0x3FF;
};

#else

class NVIC: public IC_Common, protected Machine_Model
{
public:
    // IRQs
    static const unsigned int IRQS = Machine_Model::IRQS;
    typedef Interrupt_Id IRQ;
    enum {
        IRQ_GPIOA       = 0,
        IRQ_GPIOB       = 1,
        IRQ_GPIOC       = 2,
        IRQ_GPIOD       = 3,
        IRQ_GPIOE       = 4,
        IRQ_UART0       = 5,
        IRQ_UART1       = 6,
        IRQ_SSI0        = 7,
        IRQ_I2C         = 8,
        IRQ_ADC         = 14,
        IRQ_WATCHDOG    = 18,
        IRQ_GPT0A       = 19,
        IRQ_GPT0B       = 20,
        IRQ_GPT1A       = 21,
        IRQ_GPT1B       = 22,
        IRQ_GPT2A       = 23,
        IRQ_GPT2B       = 24,
        IRQ_AC          = 25,
        IRQ_RFTXRX      = 26,
        IRQ_RFERR       = 27,
        IRQ_SC          = 28,
        IRQ_FC          = 29,
        IRQ_AES         = 30,
        IRQ_PKA         = 31,
        IRQ_SMT         = 32,
        IRQ_MACTIMER    = 33,
        IRQ_SSI1        = 34,
        IRQ_GPT3A       = 35,
        IRQ_GPT3B       = 36,
        IRQ_USB         = 44, // Using alternate interrupt mapping
        IRQ_UDMASW      = 46,
        IRQ_UDMAERR     = 47,
        IRQ_LAST        = IRQ_UDMAERR
    };

    // Interrupts
    static const unsigned int INTS = 64;
    static const unsigned int EXC_INT = 0;
    static const unsigned int HARD_INT = 16;
    static const unsigned int SOFT_INT = HARD_INT + IRQS;
    enum {
        INT_HARD_FAULT  = ARMv7_M::EXC_HARD,
        INT_TIMER       = ARMv7_M::EXC_SYSTICK,
        INT_FIRST_HARD  = HARD_INT,
        INT_USER_TIMER0 = HARD_INT + IRQ_GPT0A,
        INT_USER_TIMER1 = HARD_INT + IRQ_GPT1A,
        INT_USER_TIMER2 = HARD_INT + IRQ_GPT2A,
        INT_USER_TIMER3 = HARD_INT + IRQ_GPT3A,
        INT_MACTIMER    = HARD_INT + IRQ_MACTIMER,
        INT_GPIOA       = HARD_INT + IRQ_GPIOA,
        INT_GPIOB       = HARD_INT + IRQ_GPIOB,
        INT_GPIOC       = HARD_INT + IRQ_GPIOC,
        INT_GPIOD       = HARD_INT + IRQ_GPIOD,
        INT_NIC0_RX     = HARD_INT + IRQ_RFTXRX,
        INT_NIC0_TX     = HARD_INT + IRQ_RFTXRX,
        INT_NIC0_ERR    = HARD_INT + IRQ_RFERR,
        INT_NIC0_TIMER  = HARD_INT + IRQ_MACTIMER,
        INT_USB0        = HARD_INT + IRQ_USB,
        INT_LAST_HARD   = HARD_INT + IRQS,
        INT_RESCHEDULER = SOFT_INT
    };

public:
    NVIC() {}

    static int irq2int(int i) { return i + HARD_INT; }
    static int int2irq(int i) { return i - HARD_INT; }

    static void enable() {
        db<IC>(TRC) << "IC::enable()" << endl;
        scs(IRQ_ENABLE0) = ~0;
        if(IRQS > 32) scs(IRQ_ENABLE1) = ~0;
        if(IRQS > 64) scs(IRQ_ENABLE2) = ~0;
    }

    static void enable(const Interrupt_Id & id) {
        if(id < HARD_INT)
            return;
        IRQ i = int2irq(id);
        db<IC>(TRC) << "IC::enable(irq=" << i << ")" << endl;
        assert(i < IRQS);
        if(i < 32) scs(IRQ_ENABLE0) = 1 << i;
        else if((IRQS > 32) && (i < 64)) scs(IRQ_ENABLE1) = 1 << (i - 32);
        else if(IRQS > 64) scs(IRQ_ENABLE2) = 1 << (i - 64);
    }

    static void disable() {
        db<IC>(TRC) << "IC::disable()" << endl;
        scs(IRQ_DISABLE0) = ~0;
        if(IRQS > 32) scs(IRQ_DISABLE1) = ~0;
        if(IRQS > 64) scs(IRQ_DISABLE2) = ~0;
    }

    static void disable(const Interrupt_Id & id) {
        if(id < HARD_INT)
            return;
        IRQ i = int2irq(id);
        db<IC>(TRC) << "IC::disable(irq=" << i << ")" << endl;
        assert(i < IRQS);
        if(i < 32) scs(IRQ_DISABLE0) = 1 << i;
        else if((IRQS > 32) && (i < 64)) scs(IRQ_DISABLE1) = 1 << (i - 32);
        else if(IRQS > 64) scs(IRQ_DISABLE2) = 1 << (i - 64);
        unpend(i);
    }

    // Only works in handler mode (inside IC::entry())
    static Interrupt_Id int_id() { return CPU::flags() & 0x3f; }

    static void init(void) {};

private:
    static void unpend() {
        db<IC>(TRC) << "IC::unpend()" << endl;
        scs(IRQ_UNPEND0) = ~0;
        scs(IRQ_UNPEND1) = ~0;
        scs(IRQ_UNPEND2) = ~0;
    }

    static void unpend(const IRQ & i) {
        db<IC>(TRC) << "IC::unpend(irq=" << i << ")" << endl;
        assert(i < IRQS);
        if(i < 32) scs(IRQ_UNPEND0) = 1 << i;
        else if((IRQS > 32) && (i < 64)) scs(IRQ_UNPEND1) = 1 << (i - 32);
        else if(IRQS > 64) scs(IRQ_UNPEND2) = 1 << (i - 64);
    }
};

#endif


class IC: private IF<Traits<Build>::MODEL == Traits<Build>::Zynq, GIC, NVIC>::Result
{
    friend class Machine;

private:
    typedef IF<Traits<Build>::MODEL == Traits<Build>::Zynq, GIC, NVIC>::Result Engine;

public:
    using IC_Common::Interrupt_Id;
    using IC_Common::Interrupt_Handler;
    using Engine::INT_TIMER;
    using Engine::INT_USER_TIMER0;
    using Engine::INT_USER_TIMER1;
    using Engine::INT_USER_TIMER2;
    using Engine::INT_USER_TIMER3;
    using Engine::INT_GPIOA;
    using Engine::INT_GPIOB;
    using Engine::INT_GPIOC;
    using Engine::INT_GPIOD;
    using Engine::INT_USB0;
    using Engine::INT_NIC0_RX;
    using Engine::INT_NIC0_TX;
    using Engine::INT_NIC0_ERR;
    using Engine::INT_NIC0_TIMER;
    using Engine::INT_RESCHEDULER;

public:
    IC() {}

    static Interrupt_Handler int_vector(const Interrupt_Id & i) {
        assert(i < INTS);
        return _int_vector[i];
    }

    static void int_vector(const Interrupt_Id & i, const Interrupt_Handler & h) {
        db<IC>(TRC) << "IC::int_vector(int=" << i << ",h=" << reinterpret_cast<void *>(h) <<")" << endl;
        assert(i < INTS);
        _int_vector[i] = h;
    }

    static void enable() {
        db<IC>(TRC) << "IC::enable()" << endl;
        Engine::enable();
    }

    static void enable(const Interrupt_Id & i) {
        db<IC>(TRC) << "IC::enable(int=" << i << ")" << endl;
        assert(i < INTS);
        Engine::enable(i);
    }

    static void disable() {
        db<IC>(TRC) << "IC::disable()" << endl;
        Engine::disable();
    }

    static void disable(const Interrupt_Id & i) {
        db<IC>(TRC) << "IC::disable(int=" << i << ")" << endl;
        assert(i < INTS);
        Engine::disable(i);
    }

    using Engine::irq2int;

    static void ipi_send(unsigned int cpu, Interrupt_Id int_id) {}

    void undefined_instruction();
    void software_interrupt();
    void prefetch_abort();
    void data_abort();
    void reserved();
    void fiq();

private:
    static void dispatch(unsigned int i);
    static void eoi(unsigned int i);

    // Logical handlers
    static void int_not(const Interrupt_Id & i);
    static void hard_fault(const Interrupt_Id & i);

    // Physical handler
    static void entry();

    static void init();

private:
    static Interrupt_Handler _int_vector[INTS];
    static Interrupt_Handler _eoi_vector[INTS];
};

__END_SYS

#endif
