// EPOS Observer Utility Declarations

// Observation about the lack of virtual destructors in the following classes:
// Observed x Observer is used in mediators, so they appear very early in the system.
// To be more precise, they are used in SETUP, where we cannot yet handle a heap.
// Since the purpose of the destructors is only to trace the classes, we accepted to
// declare them as non-virtual. But it must be clear that this is one of the few uses
// for them.

#ifndef __observer_h
#define	__observer_h

#include <utility/list.h>

__BEGIN_UTIL

// Observer x Observed
class Observer;

class Observed
{
    friend class Observer;

private:
    typedef Simple_List<Observer>::Element Element;

public:
    Observed() {
        db<Observers>(TRC) << "Observed() => " << this << endl;
    }
    ~Observed() {
        db<Observers>(TRC) << "~Observed(this=" << this << ")" << endl;
    }

    virtual void attach(Observer * o);
    virtual void detach(Observer * o);
    virtual bool notify();

private:
    Simple_List<Observer> _observers;
};

class Observer
{
    friend class Observed;

protected:
    Observer(): _link(this) {
        db<Observers>(TRC) << "Observer() => " << this << endl;
    }

public:
    ~Observer() {
        db<Observers>(TRC) << "~Observer(this=" << this << ")" << endl;
    }

    virtual void update(Observed * o) = 0;

private:
    Observed::Element _link;
};

inline void Observed::attach(Observer * o)
{
    db<Observers>(TRC) << "Observed::attach(obs=" << o << ")" << endl;

    _observers.insert(&o->_link);
}

inline void Observed::detach(Observer * o)
{
    db<Observers>(TRC) << "Observed::detach(obs=" << o << ")" << endl;

    _observers.remove(&o->_link);
}

inline bool Observed::notify()
{
    bool notified = false;

    db<Observers>(TRC) << "Observed::notify()" << endl;

    for(Element * e = _observers.head(); e; e = e->next()) {
        db<Observers>(INF) << "Observed::notify(this=" << this << ",obs=" << e->object() << ")" << endl;

        e->object()->update(this);
        notified = true;
    }

    return notified;
}


// Conditional Observer x Conditionally Observed
template<typename T = int>
class Conditional_Observer;

template<typename T = int>
class Conditionally_Observed
{
    friend class Conditional_Observer<T>;

private:
    typedef typename Simple_Ordered_List<Conditional_Observer<T>, T>::Element Element;

public:
    typedef T Observing_Condition;

public:
    Conditionally_Observed() {
        db<Observers>(TRC) << "Conditionally_Observed() => " << this << endl;
    }
    ~Conditionally_Observed() {
        db<Observers>(TRC) << "~Conditionally_Observed(this=" << this << ")" << endl;
    }

    virtual void attach(Conditional_Observer<T> * o, T c) {
        db<Observers>(TRC) << "Conditionally_Observed::attach(o=" << o << ",c=" << c << ")" << endl;

        o->_link = Element(o, c);
        _observers.insert(&o->_link);
    }

    virtual void detach(Conditional_Observer<T> * o, T c) {
        db<Observers>(TRC) << "Conditionally_Observed::detach(obs=" << o << ",c=" << c << ")" << endl;

        _observers.remove(&o->_link);
    }

    virtual bool notify(T c) {
        bool notified = false;

        db<Observers>(TRC) << "Conditionally_Observed::notify(cond=" << hex << c << ")" << endl;

        for(Element * e = _observers.head(); e; e = e->next()) {
            if(e->rank() == c) {
                db<Observers>(INF) << "Observed::notify(this=" << this << ",obs=" << e->object() << ")" << endl;
                e->object()->update(this, c);
                notified = true;
            }
        }

        return notified;
    }

private:
    Simple_Ordered_List<Conditional_Observer<T>, T> _observers;
};

template<typename T>
class Conditional_Observer
{
    friend class Conditionally_Observed<T>;

public:
    typedef T Observing_Condition;

protected:
    Conditional_Observer(): _link(this) {
        db<Observers>(TRC) << "Conditionally_Observer() => " << this << endl;
    }

public:
    ~Conditional_Observer() {
        db<Observers>(TRC) << "~Conditionally_Observer(this=" << this << ")" << endl;
    }

    virtual void update(Conditionally_Observed<T> * o, T c) = 0;

private:
    typename Conditionally_Observed<T>::Element _link;
};


// (Conditional) Observer x (Conditionally) Observed with Data
template<typename T1, typename T2 = void>
class Data_Observer;

template<typename T1, typename T2 = void>
class Data_Observed
{
    friend class Data_Observer<T1, T2>;

private:
    typedef Data_Observer<T1, T2> Observer;
    typedef typename Simple_Ordered_List<Data_Observer<T1, T2>, T2>::Element Element;

public:
    typedef T1 Observed_Data;
    typedef T2 Observing_Condition;

public:
    Data_Observed() {
        db<Observers>(TRC) << "Data_Observed<T>() => " << this << endl;
    }

    ~Data_Observed() {
        db<Observers>(TRC) << "~Data_Observed<T>(this=" << this << ")" << endl;
    }

    virtual void attach(Data_Observer<T1, T2> * o, T2 c) {
        db<Observers>(TRC) << "Data_Observed<T>::attach(obs=" << o << ",cond=" << c << ")" << endl;

        o->_link = Element(o, c);
        _observers.insert(&o->_link);
    }

    virtual void detach(Data_Observer<T1, T2> * o, T2 c) {
        db<Observers>(TRC) << "Data_Observed<T>::detach(obs=" << o << ",cond=" << c << ")" << endl;

        _observers.remove(&o->_link);
    }

    virtual bool notify(T2 c, T1 * d) {
        bool notified = false;

        db<Observers>(TRC) << "Data_Observed<T>::notify(this=" << this << ",cond=" << c << ")" << endl;

        for(Element * e = _observers.head(); e; e = e->next()) {
            if(e->rank() == c) {
                db<Observers>(INF) << "Data_Observed<T>::notify(this=" << this << ",obs=" << e->object() << ")" << endl;
                e->object()->update(this, c, d);
                notified = true;
            }
        }

        return notified;
    }

    virtual Observer * observer(T2 c, unsigned int index = 0) {
        Observer * o = 0;
        for(Element * e = _observers.head(); e; e = e->next()) {
            if(e->rank() == c) {
                if(!index)
                    o =  e->object();
                else
                    index--;
            }
        }
        return o;
    }

private:
    Simple_Ordered_List<Data_Observer<T1, T2>, T2> _observers;
};

template<typename T1, typename T2>
class Data_Observer
{
    friend class Data_Observed<T1, T2>;

public:
    typedef T1 Observed_Data;
    typedef T2 Observing_Condition;

protected:
    Data_Observer(): _link(this) {
        db<Observers>(TRC) << "Data_Observer<T>() => " << this << endl;
    }

public:
    ~Data_Observer() {
        db<Observers>(TRC) << "~Data_Observer<T>(this=" << this << ")" << endl;
    }

    virtual void update(Data_Observed<T1, T2> * o, T2 c, T1 * d) = 0;

private:
    typename Data_Observed<T1, T2>::Element _link;
};


// (Unconditional) Observer x (Unconditionally) Observed with Data
template<typename T1>
class Data_Observed<T1, void>
{
    friend class Data_Observer<T1, void>;

private:
    typedef Data_Observer<T1, void> Observer;
    typedef typename Simple_List<Data_Observer<T1, void>>::Element Element;

public:
    typedef T1 Observed_Data;

public:
    Data_Observed() {
        db<Observers>(TRC) << "Data_Observed() => " << this << endl;
    }

    ~Data_Observed() {
        db<Observers>(TRC) << "~Data_Observed(this=" << this << ")" << endl;
    }

    virtual void attach(Data_Observer<T1, void> * o) {
        db<Observers>(TRC) << "Data_Observed::attach(obs=" << o << ")" << endl;

        o->_link = Element(o);
        _observers.insert(&o->_link);
    }

    virtual void detach(Data_Observer<T1, void> * o) {
        db<Observers>(TRC) << "Data_Observed::detach(obs=" << o << ")" << endl;

        _observers.remove(&o->_link);
    }

    virtual bool notify(T1 * d) {
        bool notified = false;

        db<Observers>(TRC) << "Data_Observed::notify(this=" << this << ")" << endl;

        for(Element * e = _observers.head(); e; e = e->next()) {
            db<Observers>(INF) << "Observed::notify(this=" << this << ",obs=" << e->object() << ")" << endl;
            e->object()->update(this, d);
            notified = true;
        }

        return notified;
    }

    virtual Observer * observer(unsigned int index = 0) {
        Observer * o = 0;
        for(Element * e = _observers.head(); e; e = e->next()) {
            if(!index)
                o =  e->object();
            else
                index--;
        }
        return o;
    }

private:
    Simple_List<Data_Observer<T1, void>> _observers;
};

template<typename T1>
class Data_Observer<T1, void>
{
    friend class Data_Observed<T1, void>;

public:
    typedef T1 Observed_Data;

protected:
    Data_Observer(): _link(this) {
        db<Observers>(TRC) << "Data_Observer() => " << this << endl;
    }

public:
    ~Data_Observer() {
        db<Observers>(TRC) << "~Data_Observer(this=" << this << ")" << endl;
    }

    virtual void update(Data_Observed<T1, void> * o, T1 * d) = 0;

private:
    typename Data_Observed<T1, void>::Element _link;
};

__END_UTIL

#endif
