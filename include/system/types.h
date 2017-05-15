// EPOS Internal Type Management System

typedef __SIZE_TYPE__ size_t;

#ifndef __types_h
#define __types_h

// Memory allocators
__BEGIN_API

enum System_Allocator { SYSTEM };
enum Scratchpad_Allocator { SCRATCHPAD };
enum Color {
    COLOR_0,  COLOR_1,  COLOR_2,  COLOR_3,  COLOR_4,  COLOR_5,  COLOR_6,  COLOR_7,
    COLOR_8,  COLOR_9,  COLOR_10, COLOR_11, COLOR_12, COLOR_13, COLOR_14, COLOR_15,
    COLOR_16, COLOR_17, COLOR_18, COLOR_19, COLOR_20, COLOR_21, COLOR_22, COLOR_23,
    COLOR_24, COLOR_25, COLOR_26, COLOR_27, COLOR_28, COLOR_29, COLOR_30, COLOR_31,
    WHITE = COLOR_0
};

__END_API

extern "C"
{
    void * malloc(size_t);
    void free(void *);
}

inline void * operator new(size_t s, void * a) { return a; }
inline void * operator new[](size_t s, void * a) { return a; }

void * operator new(size_t, const EPOS::System_Allocator &);
void * operator new[](size_t, const EPOS::System_Allocator &);

void * operator new(size_t, const EPOS::Scratchpad_Allocator &);
void * operator new[](size_t, const EPOS::Scratchpad_Allocator &);

void * operator new(size_t, const EPOS::Color &);
void * operator new[](size_t, const EPOS::Color &);

// Power Management Modes
enum Power_Mode
{
    FULL,
    LIGHT,
    SLEEP,
    OFF
};

// Utilities
__BEGIN_UTIL
typedef unsigned char Percent;
class Dummy {};
class Bitmaps;
class CRC;
class ELF;
class Handler;
class Hashes;
class Heaps;
class Debug;
class Lists;
class Observers;
class Observeds;
class OStream;
class Queues;
class Random;
class Spin;
class SREC;
class Vectors;
__END_UTIL

__BEGIN_SYS

// System parts
class Build;
class Boot;
class Setup;
class Init;
class Utility;

// Architecture Hardware Mediators
class CPU;
class TSC;
class MMU;
class FPU;
class PMU;

// Machine Hardware Mediators
class Machine;
class PCI;
class IC;
class Timer;
class RTC;
class UART;
class USB;
class Smart_Plug;
class Hydro_Board;
class EEPROM;
class Display;
class Serial_Display;
class Keyboard;
class Serial_Keyboard;
class Scratchpad;
class Persistent_Storage;
class Cipher;
class Watchdog;
template<unsigned int KEY_SIZE>
class Software_AES;
class GPIO;
class I2C;
class ADC;
class FPGA;
class NIC;
class Ethernet;
class IEEE802_15_4;
class PCNet32;
class C905;
class E100;
class CC2538;
class M95;
class AT86RF;
class GEM;
class SPI;
class RFID_Reader;

// Components
class System;
class Application;

class Thread;
class Active;
class Periodic_Thread;
class RT_Thread;
class Task;

template<typename> class Scheduler;
namespace Scheduling_Criteria
{
    class Priority;
    class FCFS;
    class RR;
    class RM;
    class DM;
    class EDF;
    class GRR;
    class CPU_Affinity;
    class GEDF;
    class PEDF;
    class CEDF;
    class PRM;
};

class Address_Space;
class Segment;

class Synchronizer;
class Mutex;
class Semaphore;
class Condition;

class Clock;
class Chronometer;
class Alarm;
class Delay;

class Diffie_Hellman;
class Poly1305;

class Network;

class ELP;

class TSTP;

template<typename NIC, typename Network, unsigned int HTYPE>
class ARP;
class IP;
class ICMP;
class UDP;
class TCP;
class DHCP;
class Quectel_HTTP;

class IPC;

template<typename Channel, bool connectionless = Channel::connectionless>
class Link;
template<typename Channel, bool connectionless = Channel::connectionless>
class Port;

template<typename S>
class Smart_Data;

// Framework
class Framework;
template<typename Component> class Handle;
template<typename Component, bool remote> class Stub;
template<typename Component> class Proxy;
template<typename Component> class Adapter;
template<typename Component> class Scenario;
class Agent;

// Aspects
class Aspect;
template<typename Component> class Authenticated;
template<typename Component> class Shared;
template<typename Component> class Remote;

// System Components IDs
// The order in this enumeration defines many things in the system (e.g. init)
typedef unsigned int Type_Id;
enum
{
    CPU_ID = 100,
    TSC_ID,
    MMU_ID,
    FPU_ID,
    PMU_ID,

    MACHINE_ID = 200,
    PCI_ID,
    IC_ID,
    TIMER_ID,
    RTC_ID,
    EEPROM_ID,
    SCRATCHPAD_ID,
    UART_ID,
    DISPLAY_ID,
    KEYBOARD_ID,
    NIC_ID,

    THREAD_ID = 0,
    TASK_ID,
    ACTIVE_ID,

    ADDRESS_SPACE_ID,
    SEGMENT_ID,

    MUTEX_ID,
    SEMAPHORE_ID,
    CONDITION_ID,

    CLOCK_ID,
    ALARM_ID,
    CHRONOMETER_ID,

    IPC_COMMUNICATOR_ID,

    UTILITY_ID,

    LAST_TYPE_ID,

    UNKNOWN_TYPE_ID = 0xffff
};

// Type IDs for system components
template<typename T> struct Type { static const Type_Id ID = UNKNOWN_TYPE_ID; };

template<> struct Type<CPU> { static const Type_Id ID = CPU_ID; };
template<> struct Type<TSC> { static const Type_Id ID = TSC_ID; };
template<> struct Type<MMU> { static const Type_Id ID = MMU_ID; };
template<> struct Type<FPU> { static const Type_Id ID = FPU_ID; };
template<> struct Type<PMU> { static const Type_Id ID = PMU_ID; };

template<> struct Type<Machine> { static const Type_Id ID = MACHINE_ID; };
template<> struct Type<IC> { static const Type_Id ID = IC_ID; };
template<> struct Type<Timer> { static const Type_Id ID = TIMER_ID; };
template<> struct Type<UART> { static const Type_Id ID = UART_ID; };
template<> struct Type<RTC> { static const Type_Id ID = RTC_ID; };
template<> struct Type<PCI> { static const Type_Id ID = PCI_ID; };
template<> struct Type<Display> { static const Type_Id ID = DISPLAY_ID; };
template<> struct Type<Keyboard> { static const Type_Id ID = KEYBOARD_ID; };
template<> struct Type<Scratchpad> { static const Type_Id ID = SCRATCHPAD_ID; };
template<> struct Type<Ethernet> { static const Type_Id ID = NIC_ID; };
template<> struct Type<IEEE802_15_4> { static const Type_Id ID = NIC_ID; };

template<> struct Type<Thread> { static const Type_Id ID = THREAD_ID; };
template<> struct Type<Periodic_Thread> { static const Type_Id ID = THREAD_ID; };
template<> struct Type<RT_Thread> { static const Type_Id ID = THREAD_ID; };
template<> struct Type<Active> { static const Type_Id ID = ACTIVE_ID; };
template<> struct Type<Task> { static const Type_Id ID = TASK_ID; };

template<> struct Type<Address_Space> { static const Type_Id ID = ADDRESS_SPACE_ID; };
template<> struct Type<Segment> { static const Type_Id ID = SEGMENT_ID; };

template<> struct Type<Mutex> { static const Type_Id ID = MUTEX_ID; };
template<> struct Type<Semaphore> { static const Type_Id ID = SEMAPHORE_ID; };
template<> struct Type<Condition> { static const Type_Id ID = CONDITION_ID; };

template<> struct Type<Clock> { static const Type_Id ID = CLOCK_ID; };
template<> struct Type<Chronometer> { static const Type_Id ID = CHRONOMETER_ID; };
template<> struct Type<Alarm> { static const Type_Id ID = ALARM_ID; };
template<> struct Type<Delay> { static const Type_Id ID = ALARM_ID; };

template<> struct Type<Utility> { static const Type_Id ID = UTILITY_ID; };

// Type IDs for system components whose parameters are themselves components are defined where they are declared.

__END_SYS

#endif
