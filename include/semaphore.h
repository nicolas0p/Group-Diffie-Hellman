// EPOS Semaphore Component Declarations

#ifndef __semaphore_h
#define __semaphore_h

#include <utility/handler.h>
#include <synchronizer.h>

__BEGIN_SYS

class Semaphore: protected Synchronizer_Common
{
public:
    Semaphore(int v = 1);
    ~Semaphore();

    void p();
    void v();

private:
    volatile int _value;
};


// An event handler that triggers a semaphore (see handler.h)
class Semaphore_Handler: public Handler
{
public:
    Semaphore_Handler(Semaphore * h) : _handler(h) {}
    ~Semaphore_Handler() {}

    void operator()() { _handler->v(); }

private:
    Semaphore * _handler;
};


// Conditional Observer x Conditionally Observed with Data decoupled by a Semaphore
template<typename D, typename C = int>
class Semaphore_Observer;

template<typename D, typename C = int>
class Semaphore_Observed
{
    friend class Semaphore_Observer<D, C>;

private:
    typedef Semaphore_Observer<D, C> Observer;
    typedef typename Simple_Ordered_List<Semaphore_Observer<D, C>, C>::Element Element;

public:
    typedef C Observing_Condition;

public:
    Semaphore_Observed() {
        db<Observeds, Semaphore>(TRC) << "Semaphore_Observed() => " << this << endl;
    }

    ~Semaphore_Observed() {
        db<Observeds, Semaphore>(TRC) << "~Semaphore_Observed(this=" << this << ")" << endl;
    }

    void attach(Semaphore_Observer<D, C> * o, C c) {
        db<Observeds, Semaphore>(TRC) << "Semaphore_Observed::attach(obs=" << o << ",cond=" << c << ")" << endl;

        o->_link = Element(o, c);
        _observers.insert(&o->_link);
    }

    void detach(Semaphore_Observer<D, C> * o, C c) {
        db<Observeds, Semaphore>(TRC) << "Semaphore_Observed::detach(obs=" << o << ",cond=" << c << ")" << endl;

        _observers.remove(&o->_link);
    }

    bool notify(C c, D * d) {
        bool notified = false;

        db<Observeds, Semaphore>(TRC) << "Semaphore_Observed::notify(this=" << this << ",cond=" << c << ")" << endl;

        for(Element * e = _observers.head(); e; e = e->next()) {
            if(e->rank() == c) {
                db<Observeds, Semaphore>(INF) << "Observed::notify(this=" << this << ",obs=" << e->object() << ")" << endl;
                e->object()->update(c, d);
                notified = true;
            }
        }

        return notified;
    }

private:
    Simple_Ordered_List<Semaphore_Observer<D, C>, C> _observers;
};

template<typename D, typename C>
class Semaphore_Observer
{
    friend class Semaphore_Observed<D, C>;

public:
    typedef D Observed_Data;
    typedef C Observing_Condition;

public:
    Semaphore_Observer(): _sem(0), _link(this) {
        db<Observers>(TRC) << "Observer() => " << this << endl;
    }

    ~Semaphore_Observer() {
        db<Observers>(TRC) << "~Observer(this=" << this << ")" << endl;
    }

    void update(C c, D * d) {
        _list.insert(d->lext());
        _sem.v();
    }

    D * updated() {
        _sem.p();
        return _list.remove()->object();
    }

private:
    Semaphore _sem;
    Simple_List<D> _list;
    typename Semaphore_Observed<D, C>::Element _link;
};

__END_SYS

#endif
