// EPOS Component Framework - Scenario Adapter

// Scenario adapters are the heart of EPOS component framework.
// They collect component-specific Aspect programs to build a scenario for it to run.
// Scenario features are enforced by wrapping all and any method invocation (event creation and destruction)
// within the enter() and leave() methods.

#ifndef __adapter_h
#define __adapter_h

#include "scenario.h"

__BEGIN_SYS

template<typename Component>
class Adapter: public Component, public Scenario<Component>
{
    using Scenario<Component>::enter;
    using Scenario<Component>::leave;
    using Scenario<Component>::static_enter;
    using Scenario<Component>::static_leave;

public:
    typedef Component _Component; // used by Message

public:
    template<typename ... Tn>
    Adapter(const Tn & ... an): Component(an ...) { static_leave(); }
    ~Adapter() { static_enter(); }

    void * operator new(size_t bytes) {
        static_enter();
        return Scenario<Component>::operator new(bytes);
    }
    void operator delete(void * adapter) {
        Scenario<Component>::operator delete(adapter);
        static_leave();
    }

    static const Adapter<Component> * self() { static_enter(); const Adapter<Component> * res = reinterpret_cast<const Adapter<Component> *>(Component::self()); return res; }

    // Process management
    void suspend() { enter(); Component::suspend(); leave(); }
    void resume() { enter(); Component::resume(); leave(); }
    int join() { enter(); int res = Component::join(); leave(); return res; }
    void pass() { enter(); Component::pass(); leave(); }
    static void yield() { static_enter(); Component::yield(); static_leave(); }
    static void exit(int status) { static_enter(); Component::exit(status); static_leave(); }

    Address_Space * address_space() { enter(); Address_Space * res = Component::address_space(); leave(); return res; }
    Segment * code_segment() { enter(); Segment * res = Component::code_segment(); leave(); return res; }
    Segment * data_segment() { enter(); Segment * res = Component::data_segment(); leave(); return res; }
    CPU::Log_Addr code() { enter(); CPU::Log_Addr res = Component::code(); leave(); return res; }
    CPU::Log_Addr data() { enter(); CPU::Log_Addr res = Component::data(); leave(); return res; }
    Thread * main() { enter(); Thread * res = Component::main(); leave(); return res; }

    // Memory management
    CPU::Phy_Addr pd() { enter(); CPU::Phy_Addr res = Component::pd(); leave(); return res; }
    CPU::Log_Addr attach(Segment * seg) { enter(); CPU::Log_Addr res = Component::attach(seg); leave(); return res; }
    CPU::Log_Addr attach(Segment * seg, const CPU::Log_Addr & addr) { enter(); CPU::Log_Addr res = Component::attach(seg, addr); leave(); return res; }
    void detach(Segment * seg) { enter(); Component::detach(seg); leave(); }
    void detach(Segment * seg, const CPU::Log_Addr & addr) { enter(); Component::detach(seg, addr); leave(); }
    CPU::Phy_Addr physical(const CPU::Log_Addr & addr) { enter(); CPU::Phy_Addr res = Component::physical(addr); leave(); return res; }

    unsigned int size() { enter(); unsigned int res = Component::size(); leave(); return res; }
    CPU::Phy_Addr phy_address() { enter(); CPU::Phy_Addr res = Component::phy_address(); leave(); return res; }
    int resize(int amount) { enter(); int res = Component::resize(amount); leave(); return res; }

    // Synchronization
    void lock() { enter(); Component::lock(); leave(); }
    void unlock() { enter(); Component::unlock(); leave(); }
    void p() { enter(); Component::p(); leave(); }
    void v() { enter(); Component::v(); leave(); }
    void wait() { enter(); Component::wait(); leave(); }
    void signal() { enter(); Component::signal(); leave(); }
    void broadcast() { enter(); Component::broadcast(); leave(); }

    // Timing
    static void delay(const RTC::Microsecond & time) { static_enter(); Component::delay(time); static_leave(); }
    void reset() { enter(); Component::reset(); leave(); }
    void start() { enter(); Component::start(); leave(); }
    void lap() { enter(); Component::lap(); leave(); }
    void stop() { enter(); Component::stop(); leave(); }
    int frequency() { enter(); int res = Component::frequency(); leave(); return res; }
    int ticks() { enter(); int res = Component::ticks(); leave(); return res; }
    int read() { enter(); int res = Component::read(); leave(); return res; }
    const RTC::Microsecond period() { enter(); RTC::Microsecond res = Component::period(); leave(); return res; }
    void period(const RTC::Microsecond p) { enter(); Component::period(p); leave(); }
    static TSC::Hertz alarm_frequency() { static_enter(); TSC::Hertz res = Component::frequency(); static_leave(); return res; }

    // Communication
    template<typename ... Tn>
    int send(Tn ... an) {
        enter();
        int res = Component::send(an ...);
        leave();
        return res;
    }
    template<typename ... Tn>
    int receive(Tn ... an) {
        enter();
        int res = Component::receive(an ...);
        leave();
        return res;
    }

    template<typename ... Tn>
    int read(Tn ... an) { return receive(an ...);}
    template<typename ... Tn>
    int write(Tn ... an) { return send(an ...);}
};

__END_SYS

#endif
