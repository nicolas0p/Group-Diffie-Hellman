// EPOS Component Framework - Component Proxy

// Proxies and Agents handle RMI within EPOS component framework

#ifndef __proxy_h
#define __proxy_h

#include "message.h"
#include "ipc.h"

__BEGIN_SYS

template<typename Component>
class Proxy;

// Proxied is used to create a Proxy for components created either by SETUP before the framework came into place or internally by the framework itself
template<typename Component>
class Proxied: public Proxy<Component>
{
public:
    Proxied(): Proxy<Component>(Proxy<Component>::PROXIED) {
        db<Framework>(TRC) << "Proxied(this=" << this << ")" << endl;
    }

    void * operator new(size_t s, void * adapter) {
        db<Framework>(TRC) << "Proxied::new(adapter=" << adapter << ")" << endl;
        Framework::Element * el= Framework::_cache.search_key(reinterpret_cast<unsigned int>(adapter));
        void * proxy;
        if(el) {
            proxy = el->object();
            db<Framework>(INF) << "Proxied::new(adapter=" << adapter << ") => " << proxy << " (CACHED)" << endl;
        } else {
            proxy = new Proxy<Component>(Id(Type<Component>::ID, reinterpret_cast<Id::Unit_Id>(adapter)));
            el = new Framework::Element(proxy, reinterpret_cast<unsigned int>(adapter));  // the proxied cache is insert-only; object are intentionally never deleted, since they have been created by SETUP!
            Framework::_cache.insert(el);
        }
        return proxy;
    }
};

template<typename Component>
class Proxy: public Message
{
    template<typename> friend class Proxy;
    template<typename> friend class Proxied;

private:
    enum Private_Proxied{ PROXIED };

private:
    Proxy(const Id & id): Message(id) {} // for Proxied::operator new()
    Proxy(const Private_Proxied & p) { db<Framework>(TRC) << "Proxy(PROXIED) => [id=" << Proxy<Component>::id() << "]" << endl; } // for Proxied

public:
    template<typename ... Tn>
    Proxy(const Tn & ... an): Message(Id(Type<Component>::ID, 0)) { invoke(CREATE + sizeof ... (Tn), an ...); }
    ~Proxy() { invoke(DESTROY); }

    static Proxy<Component> * self() { return new (reinterpret_cast<void *>(static_invoke(SELF))) Proxied<Component>; }

    // Process management
    int state() { return invoke(THREAD_STATE); }
    int priority() { return invoke(THREAD_PRIORITY); }
    void priority(int p) { invoke(THREAD_PRIORITY1, p); }
    int join() { return invoke(THREAD_JOIN); }
    int pass() { return invoke(THREAD_PASS); }
    void suspend() { invoke(THREAD_SUSPEND); }
    void resume() { invoke(THREAD_RESUME); }
    static int yield() { return static_invoke(THREAD_YIELD); }
    static void exit(int r) { static_invoke(THREAD_EXIT, r); }
    static volatile bool wait_next() { return static_invoke(THREAD_WAIT_NEXT); }

    Proxy<Address_Space> * address_space() { return new (reinterpret_cast<Adapter<Address_Space> *>(invoke(TASK_ADDRESS_SPACE))) Proxied<Address_Space>; }
    Proxy<Segment> * code_segment() { return new (reinterpret_cast<Adapter<Segment> *>(invoke(TASK_CODE_SEGMENT))) Proxied<Segment>; }
    Proxy<Segment> * data_segment() { return new (reinterpret_cast<Adapter<Segment> *>(invoke(TASK_DATA_SEGMENT))) Proxied<Segment>; }
    CPU::Log_Addr code() { return invoke(TASK_CODE); }
    CPU::Log_Addr data() { return invoke(TASK_DATA); }
    Proxy<Thread> * main() { return new (reinterpret_cast<Adapter<Thread> *>(invoke(TASK_MAIN))) Proxied<Thread>; }

    // Memory management
    CPU::Phy_Addr pd() { return invoke(ADDRESS_SPACE_PD); }
    CPU::Log_Addr attach(const Proxy<Segment> & seg) { return invoke(ADDRESS_SPACE_ATTACH1, seg.id().unit()); }
    CPU::Log_Addr attach(const Proxy<Segment> & seg, CPU::Log_Addr addr) { return invoke(ADDRESS_SPACE_ATTACH2, seg.id().unit(), addr); }
    void detach(const Proxy<Segment> & seg) { invoke(ADDRESS_SPACE_DETACH1, seg.id().unit());}
    void detach(const Proxy<Segment> & seg, CPU::Log_Addr addr) { invoke(ADDRESS_SPACE_DETACH2, seg.id().unit(), addr); }
    CPU::Phy_Addr physical(const CPU::Log_Addr addr) { return invoke(ADDRESS_SPACE_PHYSICAL, addr); }

    unsigned int size() { return invoke(SEGMENT_SIZE); }
    CPU::Phy_Addr phy_address() { return invoke(SEGMENT_PHY_ADDRESS); }
    int resize(int amount) { return invoke(SEGMENT_RESIZE, amount); }

    // Synchronization
    void lock() { invoke(SYNCHRONIZER_LOCK); }
    void unlock() { invoke(SYNCHRONIZER_UNLOCK); }

    void p() { invoke(SYNCHRONIZER_P); }
    void v() { invoke(SYNCHRONIZER_V); }

    void wait() { invoke(SYNCHRONIZER_WAIT); }
    void signal() { invoke(SYNCHRONIZER_SIGNAL); }
    void broadcast() { invoke(SYNCHRONIZER_BROADCAST); }

    // Timing
    template<typename T>
    static void delay(T t) { static_invoke(ALARM_DELAY, t); }

    // Communication
    template<typename ... Tn>
    int send(Tn ... an) { return invoke(COMMUNICATOR_SEND, an ...); }
    template<typename ... Tn>
    int receive(Tn ... an) { return invoke(COMMUNICATOR_RECEIVE, an ...); }
    template<typename ... Tn>
    int reply(Tn ... an) { return invoke(COMMUNICATOR_REPLY, an ...); }

    template<typename ... Tn>
    int read(Tn ... an) { return receive(an ...); }
    template<typename ... Tn>
    int write(Tn ... an) { return send(an ...); }

 private:
    template<typename ... Tn>
    Result invoke(const Method & m, const Tn & ... an) {
        method(m);
        out(an ...);
        act();
        return result();
    }

    template<typename ... Tn>
    static Result static_invoke(const Method & m, const Tn & ... an) {
        Message msg(Id(Type<Component>::ID, 0)); // avoid calling ~Proxy()
        msg.method(m);
        msg.out(an ...);
        msg.act();
        return (m == SELF) ? msg.id().unit() : msg.result();
    }
};

__END_SYS

#endif
