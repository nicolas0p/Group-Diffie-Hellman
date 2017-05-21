// EPOS Task Component Declarations

#include <thread.h>

#ifndef __task_h
#define __task_h

#include <utility/malloc.h>
#include <address_space.h>
#include <segment.h>

__BEGIN_SYS

class Task
{
    friend class Init_First;
    friend class System;
    friend class Thread;

private:
    static const bool multitask = Traits<System>::multitask;

    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Context Context;
    typedef Thread::Queue Queue;

protected:
    // This constructor is only used by Init_First
    template<typename ... Tn>
    Task(Address_Space * as, Segment * cs, Segment * ds, int (* entry)(Tn ...), const Log_Addr & code, const Log_Addr & data, Tn ... an)
    : _as(as), _cs(cs), _ds(ds), _entry(entry), _code(code), _data(data) {
        db<Task, Init>(TRC) << "Task(as=" << _as << ",cs=" << _cs << ",ds=" << _ds << ",entry=" << _entry << ",code=" << _code << ",data=" << _data << ") => " << this << endl;

        _current = this;
        activate();
        _main = new (SYSTEM) Thread(Thread::Configuration(Thread::RUNNING, Thread::MAIN, WHITE, this, 0), entry, an ...);
    }

public:
    template<typename ... Tn>
    Task(Segment * cs, Segment * ds, int (* entry)(Tn ...), Tn ... an)
    : _as (new (SYSTEM) Address_Space), _cs(cs), _ds(ds), _entry(entry), _code(_as->attach(_cs)), _data(_as->attach(_ds)) {
        db<Task>(TRC) << "Task(as=" << _as << ",cs=" << _cs << ",ds=" << _ds << ",entry=" << _entry << ",code=" << _code << ",data=" << _data << ") => " << this << endl;

        _main = new (SYSTEM) Thread(Thread::Configuration(Thread::READY, Thread::MAIN, WHITE, this, 0), entry, an ...);
    }
    template<typename ... Tn>
    Task(const Thread::Configuration & conf, Segment * cs, Segment * ds, int (* entry)(Tn ...), Tn ... an)
    : _as (new (SYSTEM) Address_Space), _cs(cs), _ds(ds), _entry(entry), _code(_as->attach(_cs)), _data(_as->attach(_ds)) {
        db<Task>(TRC) << "Task(as=" << _as << ",cs=" << _cs << ",ds=" << _ds << ",entry=" << _entry << ",code=" << _code << ",data=" << _data << ") => " << this << endl;

        _main = new (SYSTEM) Thread(Thread::Configuration(conf.state, conf.criterion, this, 0), entry, an ...);
    }
    ~Task();

    Address_Space * address_space() const { return _as; }

    Segment * code_segment() const { return _cs; }
    Segment * data_segment() const { return _ds; }

    Log_Addr code() const { return _code; }
    Log_Addr data() const { return _data; }

    Thread * main() const { return _main; }

    static Task * volatile self() { return current(); }

private:
    void activate() const { _as->activate(); }

    void insert(Thread * t) { _threads.insert(new (SYSTEM) Queue::Element(t)); }
    void remove(Thread * t) { Queue::Element * el = _threads.remove(t); if(el) delete el; }

    static Task * volatile current() { return _current; }
    static void current(Task * t) { _current = t; }

    static void init();

private:
    Address_Space * _as;
    Segment * _cs;
    Segment * _ds;
    Log_Addr _entry;
    Log_Addr _code;
    Log_Addr _data;
    Thread * _main;
    Queue _threads;

    static Task * volatile _current;
};

__END_SYS

#endif
