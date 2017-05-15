// EPOS LM3S811 (ARM Cortex-M3) MCU Metainfo and Configuration

#ifndef __machine_traits_h
#define __machine_traits_h

#include <system/config.h>

__BEGIN_SYS

class Machine_Common;
template<> struct Traits<Machine_Common>: public Traits<void>
{
    static const bool debugged = Traits<void>::debugged;
};

template<> struct Traits<Machine>: public Traits<Machine_Common>
{
    static const unsigned int CPUS = Traits<Build>::CPUS;

    // Physical Memory
    static const unsigned int MEM_BASE  = 0x20000000;
    static const unsigned int MEM_TOP   = 0x20001fff; // 8 KB (MAX for 32-bit is 0x70000000 / 1792 MB)

    // Logical Memory Map
    static const unsigned int APP_LOW   = 0x20000000;
    static const unsigned int APP_CODE  = 0x00000000;
    static const unsigned int APP_DATA  = 0x20000000;
    static const unsigned int APP_HIGH  = 0x20001fff; // 8 KB

    static const unsigned int PHY_MEM   = 0x20000000;
    static const unsigned int IO_BASE   = 0x40000000;
    static const unsigned int IO_TOP    = 0x440067ff;

    static const unsigned int SYS       = 0x00200000;
    static const unsigned int SYS_CODE  = 0x00200000; // Library mode only => APP + SYS
    static const unsigned int SYS_DATA  = 0x20000000; // Library mode only => APP + SYS

    // Default Sizes and Quantities
    static const unsigned int STACK_SIZE = 512;
    static const unsigned int HEAP_SIZE = 512;
    static const unsigned int MAX_THREADS = 5;
};

template<> struct Traits<IC>: public Traits<Machine_Common>
{
    static const bool debugged = hysterically_debugged;
};

template<> struct Traits<Timer>: public Traits<Machine_Common>
{
    static const bool debugged = hysterically_debugged;

    // Meaningful values for the timer frequency range from 100 to
    // 10000 Hz. The choice must respect the scheduler time-slice, i. e.,
    // it must be higher than the scheduler invocation frequency.
    static const int FREQUENCY = 1000; // Hz
};

template<> struct Traits<UART>: public Traits<Machine_Common>
{
    static const unsigned int UNITS = 2;

    static const unsigned int CLOCK = Traits<CPU>::CLOCK;

    static const unsigned int DEF_UNIT = 0;
    static const unsigned int DEF_BAUD_RATE = 115200;
    static const unsigned int DEF_DATA_BITS = 8;
    static const unsigned int DEF_PARITY = 0; // none
    static const unsigned int DEF_STOP_BITS = 1;
};

template<> struct Traits<USB>: public Traits<Machine_Common>
{
    static const bool enabled = false;
    static const unsigned int UNITS = 0;
    static const bool blocking = true;
};

template<> struct Traits<Scratchpad>: public Traits<Machine_Common>
{
    static const bool enabled = false;
};

template<> struct Traits<NIC>: public Traits<Machine_Common>
{
    static const bool enabled = (Traits<Build>::NODES > 1);

    // NICS that don't have a network in Traits<Network>::NETWORKS will not be enabled
    typedef LIST<CC2538> NICS;
    static const unsigned int UNITS = NICS::Length;
    static const bool promiscuous = false;
};

template<> struct Traits<CC2538>: public Traits<NIC>
{
    static const unsigned int UNITS = NICS::Count<CC2538>::Result;
    static const unsigned int RECEIVE_BUFFERS = 20; // per unit
    static const bool gpio_debug = false;
    static const bool reset_backdoor = false;
};

__END_SYS

#endif
