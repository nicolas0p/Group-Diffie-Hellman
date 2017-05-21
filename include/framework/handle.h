// EPOS Component Framework - Component Handle

// Handle is the framework entry point. It defines a first component wrapper whose main
// purpose is to ensure the invocation of proper new and delete operators (from scenario) for components,
// independently of how they are declared by the client (static allocations will be forwarded to new).

#ifndef __handle_h
#define __handle_h

#include "stub.h"

__BEGIN_SYS

template<typename Component>
class Handle;

// Handled is used to create a wrapper for components created either by SETUP before the framework came into place or internally by the framework itself
template<typename Component>
class Handled: public Handle<Component>
{
public:
    Handled(): Handle<Component>(Handle<Component>::HANDLED) {
        db<Framework>(TRC) << "Handled(this=" << this << ")" << endl;
    }

    void * operator new(size_t s, void * stub) {
        db<Framework>(TRC) << "Handled::new(stub=" << stub << ")" << endl;
        Framework::Element * el= Framework::_cache.search_key(reinterpret_cast<unsigned int>(stub));
        void * handle;
        if(el) {
            handle = el->object();
            db<Framework>(INF) << "Handled::new(stub=" << stub << ") => " << handle << " (CACHED)" << endl;
        } else {
            handle = new Handle<Component>(reinterpret_cast<typename Handle<Component>::_Stub *>(stub));
            el = new Framework::Element(handle, reinterpret_cast<unsigned int>(stub));  // the handled cache is insert-only; object are intentionally never deleted, since they have been created by SETUP!
            Framework::_cache.insert(el);
        }
        return handle;
    }
};


template<typename Component>
class Handle
{
    template<typename> friend class Handle;
    template<typename> friend class Handled;
    template<typename> friend class Proxy;

private:
    typedef Stub<Component, Traits<Component>::ASPECTS::Length || (Traits<System>::mode == Traits<Build>::KERNEL)> _Stub;

    enum Private_Handle{ HANDLED };

private:
    Handle(const Private_Handle & h) { db<Framework>(TRC) << "Handle(HANDLED) => [stub=" << _stub << "]" << endl; }
    Handle(_Stub * s) { _stub = s; }

public:
    template<typename ... Tn>
    Handle(const Tn & ... an) { _stub = new _Stub(an ...); }

    // Dereferencing handles for Task(cs, ds, ...)
    template<typename ... Tn>
    Handle(Handle<Segment> * cs, Handle<Segment> * ds, const Tn & ... an) { _stub = new _Stub(*cs->_stub, *ds->_stub, an ...); }

    ~Handle() { if(_stub) delete _stub; }

    static Handle<Component> * self() { return new (_Stub::self()) Handled<Component>; }

    // Process management
    int priority() { return _stub->priority(); }
    void priority(int p) { _stub->priority(p); }
    int join() { return _stub->join(); }
    int pass() { return _stub->pass(); }
    void suspend() { _stub->suspend(); }
    void resume() { _stub->resume(); }
    static void yield() { _Stub::yield(); }
    static void exit(int r = 0) { _Stub::exit(r); }
    static volatile bool wait_next() { return _Stub::wait_next(); }

    Handle<Address_Space> * address_space() const { return new (_stub->address_space()) Handled<Address_Space>; }
    Handle<Segment> * code_segment() const { return new (_stub->code_segment()) Handled<Segment>; }
    Handle<Segment> * data_segment() const { return new (_stub->data_segment()) Handled<Segment>; }
    CPU::Log_Addr code() const { return _stub->code(); }
    CPU::Log_Addr data() const { return _stub->data(); }
    Handle<Thread> * main() const { return new (_stub->main()) Handled<Thread>; }

    // Memory Management
    CPU::Phy_Addr pd() { return _stub->pd(); }
    CPU::Log_Addr attach(Handle<Segment> * seg) { return _stub->attach(*seg->_stub); }
    CPU::Log_Addr attach(Handle<Segment> * seg, CPU::Log_Addr addr) { return _stub->attach(*seg->_stub, addr); }
    void detach(Handle<Segment> * seg) { _stub->detach(*seg->_stub); }
    void detach(Handle<Segment> * seg, CPU::Log_Addr addr) { _stub->detach(*seg->_stub, addr); }
    CPU::Phy_Addr physical(const CPU::Log_Addr addr) { return _stub->physical(addr); }

    unsigned int size() const { return _stub->size(); }
    CPU::Phy_Addr phy_address() const { return _stub->phy_address(); }
    int resize(int amount) { return _stub->resize(amount); }

    // Synchronization
    void lock() { _stub->lock(); }
    void unlock() { _stub->unlock(); }

    void p() { _stub->p(); }
    void v() { _stub->v(); }

    void wait() { _stub->wait(); }
    void signal() { _stub->signal(); }
    void broadcast() { _stub->broadcast(); }

    // Timing
    template<typename T>
    static void delay(T t) { _Stub::delay(t); }

    void reset() { _stub->reset(); }
    void start() { _stub->start(); }
    void lap() { _stub->lap(); }
    void stop() { _stub->stop(); }

    int frequency() { return _stub->frequency(); }
    int ticks() { return _stub->ticks(); }
    int read() { return _stub->read(); }

    const RTC::Microsecond period() const { return _stub->period(); }
    void period(const RTC::Microsecond p) { _stub->period(p); }
    static TSC::Hertz alarm_frequency() { return _Stub::alarm_frequency(); }

    // Communication
    template<typename ... Tn>
    int send(Tn ... an) { return _stub->send(an ...);}
    template<typename ... Tn>
    int receive(Tn ... an) { return _stub->receive(an ...);}
    template<typename ... Tn>
    int reply(Tn ... an) { return _stub->reply(an ...);}

    template<typename ... Tn>
    int read(Tn ... an) { return _stub->read(an ...);}
    template<typename ... Tn>
    int write(Tn ... an) { return _stub->write(an ...);}

private:
    _Stub * _stub;
};

__END_SYS

#endif
