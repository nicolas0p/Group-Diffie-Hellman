// EPOS Component Framework

#ifndef __framework_h
#define __framework_h

#include <utility/hash.h>

__BEGIN_SYS

class Framework
{
    template<typename> friend class Handled;
    template<typename> friend class Proxied;

private:
    typedef Simple_Hash<void, 5, unsigned int> Cache; // TODO: a real cache, with operator >> instead of % would improve performance
    typedef Cache::Element Element;

public:
    Framework() {}

private:
    static Cache _cache;
};

__END_SYS

#include <cpu.h>
#include <mmu.h>
#include <system.h>
#include <thread.h>
#include <task.h>
#include <alarm.h>
#include <address_space.h>
#include <segment.h>
#include <mutex.h>
#include <semaphore.h>
#include <condition.h>
#include <communicator.h>

#include "handle.h"

#define BIND(X) typedef _SYS::IF<(_SYS::Traits<_SYS::X>::ASPECTS::Length || (_SYS::Traits<_SYS::Build>::MODE == _SYS::Traits<_SYS::Build>::KERNEL)), _SYS::Handle<_SYS::X>, _SYS::X>::Result X;
#define EXPORT(X) typedef _SYS::X X;

__BEGIN_API

__USING_UTIL

EXPORT(CPU);
EXPORT(Handler);
EXPORT(Function_Handler);

EXPORT(System);
EXPORT(Application);

BIND(Thread);
BIND(Active);
BIND(Periodic_Thread);
BIND(RT_Thread);
BIND(Task);

BIND(Address_Space);
BIND(Segment);

BIND(Mutex);
BIND(Semaphore);
BIND(Condition);

BIND(Clock);
BIND(Chronometer);
BIND(Alarm);
BIND(Delay);

BIND(Network);
EXPORT(IPC);
EXPORT(IP);
EXPORT(ICMP);
EXPORT(UDP);
EXPORT(TCP);
EXPORT(DHCP);

template<typename Channel, bool connectionless = Channel::connectionless>
class Link: public _SYS::Handle<_SYS::Link<Channel, connectionless>>
{
private:
    typedef typename _SYS::Handle<_SYS::Link<Channel, connectionless>> Base;

public:
    template<typename ... Tn>
    Link(const Tn & ... an): Base(an ...) {};
};

template<typename Channel, bool connectionless = Channel::connectionless>
class Port: public _SYS::Handle<_SYS::Port<Channel, connectionless>>
{
private:
    typedef typename _SYS::Handle<_SYS::Port<Channel, connectionless>> Base;

public:
    template<typename ... Tn>
    Port(const Tn & ... an): Base(an ...) {};
};

__END_API

#endif
