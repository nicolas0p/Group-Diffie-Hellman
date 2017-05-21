#ifndef __traits_h
#define __traits_h

#include <system/config.h>

__BEGIN_SYS

// Global Configuration
template<typename T>
struct Traits
{
    static const bool enabled = true;
    static const bool debugged = false;
    static const bool hysterically_debugged = false;
    typedef TLIST<> ASPECTS;
};

template<> struct Traits<Build>
{
    enum {LIBRARY, BUILTIN, KERNEL};
    static const unsigned int MODE = LIBRARY;

    enum {IA32, ARMv7};
    static const unsigned int ARCHITECTURE = ARMv7;

    enum {PC, Cortex_M, Cortex_A};
    static const unsigned int MACHINE = Cortex_M;

    enum {Legacy, eMote3, LM3S811};
    static const unsigned int MODEL = eMote3;

    static const unsigned int ID_SIZE = 2;
    // Default value initialized at init_system.cc. The application can overwrite it.
    static const char ID[ID_SIZE];

    static const unsigned int CPUS = 1;
    static const unsigned int NODES = 2; // > 1 => NETWORKING
};


// Utilities
template<> struct Traits<Debug>
{
    static const bool error   = false;
    static const bool warning = false;
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
template <> struct Traits<Bignum> : public Traits<void>
{
    static const bool enabled = false;
    // You can edit these values
    typedef unsigned int digit;
    typedef unsigned long long double_digit;
    static const unsigned int word = 4;

    // You shouldn't edit these
    static const unsigned int sz_digit = sizeof(digit);
    static const unsigned int sz_word = sz_digit * word;
    static const unsigned int double_word = 2 * word;
    static const unsigned int bits_in_digit = sz_digit * 8;
};

template <> struct Traits<AES> : public Traits<void>
{
    static const bool enabled = false;
    // The number of columns comprising a state in AES. This is a constant in AES. Value=4
    static const unsigned int Nb = 4;
    // The number of 32 bit words in a key.
    static const unsigned int Nk = 4;
    // The number of rounds in AES _Cipher.
    static const unsigned int Nr = 10;
    // Key length in bytes [128 bit]
    static const unsigned int KEYLEN = 16;
};

template <> struct Traits<Diffie_Hellman> : public Traits<void>
{
    static const bool enabled = false;
    // Don't edit these, unless you really know what you're doing
    static const unsigned int SECRET_SIZE = Traits<Bignum>::sz_word;
    static const unsigned int PUBLIC_KEY_SIZE = Traits<Bignum>::sz_word * 2;
};

template <> struct Traits<Secure_NIC> : public Traits<void>
{
    static const bool enabled = false;
    static const int PROTOCOL_ID = 46;
    static const unsigned long long TIME_WINDOW = 100000000U; // In Microseconds
    static const bool ALLOW_MULTIPLE_NODES_WITH_SAME_ID = true;

    static const unsigned int MAX_PEERS = 8;
    static const bool USE_FLASH = false;
    static const unsigned int FLASH_ADDRESS = 0x50000;
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
    static const bool enabled = false;
    static const int COLUMNS = 80;
    static const int LINES = 24;
    static const int TAB_SIZE = 8;
};

__END_SYS

#include __ARCH_TRAITS_H
#include __MACH_TRAITS_H
#include __MACH_CONFIG_H

__BEGIN_SYS

// Abstractions
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

    static const bool reboot = true;

    static const unsigned int STACK_SIZE = Traits<Machine>::STACK_SIZE;
    static const unsigned int HEAP_SIZE = (Traits<Application>::MAX_THREADS + 1) * Traits<Application>::STACK_SIZE;
};

template<> struct Traits<Task>: public Traits<void>
{
    static const bool enabled = false;
};

template<> struct Traits<Thread>: public Traits<void>
{
    static const bool enabled = false;
    static const bool smp = Traits<System>::multicore;

    typedef Scheduling_Criteria::RR Criterion;
    static const unsigned int QUANTUM = 10000; // us

    static const bool trace_idle = hysterically_debugged;
};

template<> struct Traits<Scheduler<Thread> >: public Traits<void>
{
    static const bool debugged = Traits<Thread>::trace_idle || hysterically_debugged;
};

template<> struct Traits<Periodic_Thread>: public Traits<void>
{
    static const bool enabled = false;
    static const bool simulate_capacity = false;
};

template<> struct Traits<Address_Space>: public Traits<void>
{
    static const bool enabled = false;
};

template<> struct Traits<Segment>: public Traits<void>
{
    static const bool enabled = false;
};

template<> struct Traits<Alarm>: public Traits<void>
{
    static const bool enabled = false;
    static const bool visible = hysterically_debugged;
};

template<> struct Traits<Synchronizer>: public Traits<void>
{
    static const bool enabled = Traits<System>::multithread;
};

template<> struct Traits<Network>: public Traits<void>
{
    static const bool enabled = false;

    static const unsigned int RETRIES = 3;
    static const unsigned int TIMEOUT = 10; // s

    // This list is positional, with one network for each NIC in traits<NIC>::NICS
    typedef LIST<IP> NETWORKS;
};

template<> struct Traits<Modbus_ASCII>: public Traits<void>
{
    static const bool enabled = false;
    static const unsigned int PROTOCOL_ID = 83;
    static const unsigned int MSG_LEN = 96;
};

template<> struct Traits<TSTP>: public Traits<void>
{
    static const bool enabled = false;//(Traits<Build>::NODES > 1);

    static const unsigned int PROTOCOL_ID = 84;
    static const unsigned int MAX_INTERESTS = 8;
    static const unsigned int MAX_SENSORS = 8;
};

template<> struct Traits<IP>: public Traits<Network>
{
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
