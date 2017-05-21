#ifndef __traits_h
#define __traits_h

#include <system/config.h>

__BEGIN_SYS

// Global Configuration
template<typename T>
struct Traits
{
    static const bool enabled = true;
    static const bool debugged = true;
    static const bool hysterically_debugged = false;
    typedef TLIST<> ASPECTS;
};

template<> struct Traits<Build>
{
    enum {LIBRARY, BUILTIN, KERNEL};
    static const unsigned int MODE = LIBRARY;

    enum {IA32};
    static const unsigned int ARCHITECTURE = IA32;

    enum {PC};
    static const unsigned int MACHINE = PC;

    enum {Legacy_PC};
    static const unsigned int MODEL = Legacy_PC;

    static const unsigned int CPUS = 4;
    static const unsigned int NODES = 1; // > 1 => NETWORKING
};


// Utilities
template<> struct Traits<Debug>
{
    static const bool error   = true;
    static const bool warning = true;
    static const bool info    = false;
    static const bool trace   = false;
};

template<> struct Traits<Lists>: public Traits<void>
{
    static const bool debugged = hysterically_debugged;
};

template<> struct Traits<Spin>: public Traits<void>
{
    static const bool debugged = hysterically_debugged;
};

template<> struct Traits<Heaps>: public Traits<void>
{
    static const bool debugged = hysterically_debugged;
};


// System Parts (mostly to fine control debugging)
template<> struct Traits<Boot>: public Traits<void>
{
};

template<> struct Traits<Setup>: public Traits<void>
{
};

template<> struct Traits<Init>: public Traits<void>
{
};


// Mediators
template<> struct Traits<Serial_Display>: public Traits<void>
{
    static const bool enabled = true;
    enum {UART, USB};
    static const int ENGINE = UART;
    static const int COLUMNS = 80;
    static const int LINES = 24;
    static const int TAB_SIZE = 8;
};

template<> template <unsigned int S> struct Traits<Software_AES<S>>: public Traits<void>
{
    static const bool enabled = true;
    static const unsigned int KEY_SIZE = 16;
};

template<> struct Traits<CPU>: public Traits<void>
{
    enum {LITTLE, BIG};
    static const unsigned int ENDIANESS         = LITTLE;
    static const unsigned int WORD_SIZE         = 32;
    static const unsigned int CLOCK             = 2000000000;
    static const bool unaligned_memory_access   = true;
};

template<> struct Traits<TSC>: public Traits<void>
{
};

template<> struct Traits<MMU>: public Traits<void>
{
    static const bool colorful = true;
    static const unsigned int COLORS = 8;
};

template<> struct Traits<FPU>: public Traits<void>
{
    static const bool enabled = false;
};

template<> struct Traits<PMU>: public Traits<void>
{
    static const bool enabled = true;
    enum { V1, V2, V3, DUO, MICRO, ATOM, NEHALEN, NETBURST, SANDY_BRIDGE };
    static const unsigned int VERSION = V2;
};

class Machine_Common;
template<> struct Traits<Machine_Common>: public Traits<void>
{
    static const bool debugged = Traits<void>::debugged;
};

template<> struct Traits<Machine>: public Traits<Machine_Common>
{
    static const unsigned int CPUS = Traits<Build>::CPUS;

    // Boot Image
    static const unsigned int BOOT_LENGTH_MIN   = 512;
    static const unsigned int BOOT_LENGTH_MAX   = 512;
    static const unsigned int BOOT_IMAGE_ADDR   = 0x00008000;
    static const unsigned int RAMDISK           = 0x0fa28000; // MEMDISK-dependent
    static const unsigned int RAMDISK_SIZE      = 0x003c0000;


    // Physical Memory
    static const unsigned int MEM_BASE  = 0x00000000;
    static const unsigned int MEM_TOP   = 0x01000000; // 256 MB (MAX for 32-bit is 0x70000000 / 1792 MB)

    // Logical Memory Map
    static const unsigned int BOOT      = 0x00007c00;
    static const unsigned int SETUP     = 0x00100000; // 1 MB
    static const unsigned int INIT      = 0x00200000; // 2 MB

    static const unsigned int APP_LOW   = 0x00000000;
    static const unsigned int APP_CODE  = 0x00000000;
    static const unsigned int APP_DATA  = 0x00400000; // 4 MB
    static const unsigned int APP_HIGH  = 0x00ffffff; // 256 MB

    static const unsigned int PHY_MEM   = 0x80000000; // 2 GB
    static const unsigned int IO_BASE   = 0xf0000000; // 4 GB - 256 MB
    static const unsigned int IO_TOP    = 0xff400000; // 4 GB - 12 MB

    static const unsigned int SYS       = IO_TOP;     // 4 GB - 12 MB
    static const unsigned int SYS_CODE  = 0xff700000;
    static const unsigned int SYS_DATA  = 0xff740000;

    // Default Sizes and Quantities
    static const unsigned int STACK_SIZE = 16 * 1024;
    static const unsigned int HEAP_SIZE = 256 * 1024;
    static const unsigned int MAX_THREADS = 16;
};

template<> struct Traits<PCI>: public Traits<Machine_Common>
{
    static const int MAX_BUS = 16;
    static const int MAX_DEV_FN = 0xff;
    static const unsigned int MAX_REGION_SIZE = 0x04000000; // 64 MB
};

template<> struct Traits<IC>: public Traits<Machine_Common>
{
    static const bool debugged = hysterically_debugged;
};

template<> struct Traits<Timer>: public Traits<Machine_Common>
{
    static const bool debugged = hysterically_debugged;

    // Meaningful values for the PC's timer frequency range from 100 to
    // 10000 Hz. The choice must respect the scheduler time-slice, i. e.,
    // it must be higher than the scheduler invocation frequency.
    static const int FREQUENCY = 1000; // Hz
};

template<> struct Traits<RTC>: public Traits<Machine_Common>
{
    static const unsigned int EPOCH_DAY = 1;
    static const unsigned int EPOCH_MONTH = 1;
    static const unsigned int EPOCH_YEAR = 1970;
    static const unsigned int EPOCH_DAYS = 719499;
};

template<> struct Traits<EEPROM>: public Traits<Machine_Common>
{
};

template<> struct Traits<UART>: public Traits<Machine_Common>
{
    static const unsigned int UNITS = 2;

    static const unsigned int CLOCK = 1843200; // 1.8432 MHz

    static const unsigned int DEF_BAUD_RATE = 115200;
    static const unsigned int DEF_DATA_BITS = 8;
    static const unsigned int DEF_PARITY = 0; // none
    static const unsigned int DEF_STOP_BITS = 1;

    static const unsigned int COM1 = 0x3f8; // to 0x3ff, IRQ4
    static const unsigned int COM2 = 0x2f8; // to 0x2ff, IRQ3
    static const unsigned int COM3 = 0x3e8; // to 0x3ef, no IRQ
    static const unsigned int COM4 = 0x2e8; // to 0x2ef, no IRQ
};

template<> struct Traits<Display>: public Traits<Machine_Common>
{
    static const bool enabled = !Traits<Serial_Display>::enabled;
    static const int COLUMNS = 80;
    static const int LINES = 25;
    static const int TAB_SIZE = 8;
};

template<> struct Traits<Keyboard>: public Traits<Machine_Common>
{
    static const bool enabled = !Traits<Serial_Keyboard>::enabled;
};

template<> struct Traits<Scratchpad>: public Traits<Machine_Common>
{
    static const bool enabled = false;
    static const unsigned int ADDRESS = 0xa0000; // VGA Graphic mode frame buffer
    static const unsigned int SIZE = 96 * 1024;
};

template<> struct Traits<NIC>: public Traits<Machine_Common>
{
    static const bool enabled = (Traits<Build>::NODES > 1);

    typedef LIST<PCNet32> NICS;
    static const unsigned int UNITS = NICS::Length;
};

template<> struct Traits<PCNet32>: public Traits<NIC>
{
    static const unsigned int UNITS = NICS::Count<PCNet32>::Result;
    static const unsigned int SEND_BUFFERS = 64; // per unit
    static const unsigned int RECEIVE_BUFFERS = 256; // per unit

    static const bool promiscuous = false;
};

template<> struct Traits<E100>: public Traits<NIC>
{
    static const unsigned int UNITS = NICS::Count<E100>::Result;
    static const unsigned int SEND_BUFFERS = 64; // per unit
    static const unsigned int RECEIVE_BUFFERS = 64; // per unit

    static const bool promiscuous = false;
};

template<> struct Traits<C905>: public Traits<NIC>
{
    static const unsigned int UNITS = NICS::Count<C905>::Result;
    static const unsigned int SEND_BUFFERS = 64; // per unit
    static const unsigned int RECEIVE_BUFFERS = 64; // per unit

    static const bool promiscuous = false;
};

template<> struct Traits<FPGA>: public Traits<Machine_Common>
{
    static const bool enabled = false;

    static const unsigned int DMA_BUFFER_SIZE = 64 * 1024; // 64 KB
};


// Components
template<> struct Traits<Application>: public Traits<void>
{
    static const unsigned int STACK_SIZE = Traits<Machine>::STACK_SIZE;
    static const unsigned int HEAP_SIZE = Traits<Machine>::HEAP_SIZE;
    static const unsigned int MAX_THREADS = Traits<Machine>::MAX_THREADS;
};

template<> struct Traits<System>: public Traits<void>
{
    static const unsigned int mode = Traits<Build>::MODE;
    static const bool multithread = (Traits<Application>::MAX_THREADS > 1);
    static const bool multitask = (mode != Traits<Build>::LIBRARY);
    static const bool multicore = (Traits<Build>::CPUS > 1) && multithread;
    static const bool multiheap = (mode != Traits<Build>::LIBRARY) || Traits<Scratchpad>::enabled;

    enum {FOREVER = 0, SECOND = 1, MINUTE = 60, HOUR = 3600, DAY = 86400, WEEK = 604800, MONTH = 2592000, YEAR = 31536000};
    static const unsigned long LIFE_SPAN = 1 * HOUR; // in seconds
    static const unsigned int DUTY_CYCLE = 10000; // in ppm

    static const bool reboot = true;

    static const unsigned int STACK_SIZE = Traits<Machine>::STACK_SIZE;
    static const unsigned int HEAP_SIZE = (Traits<Application>::MAX_THREADS + 1) * Traits<Application>::STACK_SIZE;
};

template<> struct Traits<Task>: public Traits<void>
{
    static const bool enabled = Traits<System>::multitask;
};

template<> struct Traits<Thread>: public Traits<void>
{
    static const bool smp = Traits<System>::multicore;

    typedef Scheduling_Criteria::PEDF Criterion;
    static const unsigned int QUANTUM = 10000; // us

    static const bool trace_idle = hysterically_debugged;
};

template<> struct Traits<Scheduler<Thread> >: public Traits<void>
{
    static const bool debugged = Traits<Thread>::trace_idle || hysterically_debugged;
};

template<> struct Traits<Periodic_Thread>: public Traits<void>
{
    static const bool simulate_capacity = false;
};

template<> struct Traits<Address_Space>: public Traits<void>
{
    static const bool enabled = Traits<System>::multiheap;
};

template<> struct Traits<Segment>: public Traits<void>
{
    static const bool enabled = Traits<System>::multiheap;
};

template<> struct Traits<Alarm>: public Traits<void>
{
    static const bool visible = hysterically_debugged;
};

template<> struct Traits<Synchronizer>: public Traits<void>
{
    static const bool enabled = Traits<System>::multithread;
};

template<> struct Traits<Network>: public Traits<void>
{
    static const bool enabled = (Traits<Build>::NODES > 1);

    static const unsigned int RETRIES = 3;
    static const unsigned int TIMEOUT = 10; // s

    // This list is positional, with one network for each NIC in Traits<NIC>::NICS
    typedef LIST<IP> NETWORKS;
};

template<> struct Traits<ELP>: public Traits<Network>
{
    static const bool enabled = NETWORKS::Count<ELP>::Result;

    static const bool acknowledged = true;
};

template<> struct Traits<TSTP>: public Traits<Network>
{
    static const bool enabled = NETWORKS::Count<TSTP>::Result;
    static const bool sink = false;
};

template<> template <typename S> struct Traits<Smart_Data<S>>: public Traits<Network>
{
    static const bool enabled = NETWORKS::Count<TSTP>::Result;
};

template<> struct Traits<IP>: public Traits<Network>
{
    static const bool enabled = NETWORKS::Count<IP>::Result;

    enum {STATIC, MAC, INFO, RARP, DHCP};

    struct Default_Config {
        static const unsigned int  TYPE    = DHCP;
        static const unsigned long ADDRESS = 0;
        static const unsigned long NETMASK = 0;
        static const unsigned long GATEWAY = 0;
    };

    template<unsigned int UNIT>
    struct Config: public Default_Config {};

    static const unsigned int TTL  = 0x40; // Time-to-live
};

template<> struct Traits<IP>::Config<0> //: public Traits<IP>::Default_Config
{
    static const unsigned int  TYPE      = MAC;
    static const unsigned long ADDRESS   = 0x0a000100;  // 10.0.1.x x=MAC[5]
    static const unsigned long NETMASK   = 0xffffff00;  // 255.255.255.0
    static const unsigned long GATEWAY   = 0;           // 10.0.1.1
};

template<> struct Traits<IP>::Config<1>: public Traits<IP>::Default_Config
{
};

template<> struct Traits<UDP>: public Traits<Network>
{
    static const bool checksum = true;
};

template<> struct Traits<TCP>: public Traits<Network>
{
    static const unsigned int WINDOW = 4096;
};

template<> struct Traits<DHCP>: public Traits<Network>
{
};

__END_SYS

#endif
