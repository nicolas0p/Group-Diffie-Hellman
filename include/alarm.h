// EPOS Alarm Component Declarations

#ifndef __alarm_h
#define __alarm_h

#include <utility/queue.h>
#include <utility/handler.h>
#include <tsc.h>
#include <rtc.h>
#include <ic.h>
#include <timer.h>
#include <semaphore.h>

__BEGIN_SYS

class Alarm
{
    friend class System;
    friend class Alarm_Chronometer;
    friend class Periodic_Thread;
    friend class RT_Thread;
    friend class Scheduling_Criteria::FCFS;
    friend class Scheduling_Criteria::EDF;

private:
    typedef TSC::Hertz Hertz;
    typedef Timer::Tick Tick;

    typedef Relative_Queue<Alarm, Tick> Queue;

public:
    typedef RTC::Microsecond Microsecond;

    // Infinite times (for alarms)
    enum { INFINITE = RTC::INFINITE };

public:
    Alarm(const Microsecond & time, Handler * handler, int times = 1);
    ~Alarm();

    const Microsecond & period() const { return _time; }
    void period(const Microsecond & p);

    static Hertz frequency() { return _timer->frequency(); }

    static void delay(const Microsecond & time);

private:
    static void init();

    static Microsecond timer_period() {
        return 1000000 / frequency();
    }

    static Tick ticks(const Microsecond & time) {
        return (time + timer_period() / 2) / timer_period();
    }

    static void lock() { Thread::lock(); }
    static void unlock() { Thread::unlock(); }

    static void handler(const IC::Interrupt_Id & i);

private:
    Microsecond _time;
    Handler * _handler;
    int _times;
    Tick _ticks;
    Queue::Element _link;

    static Alarm_Timer * _timer;
    static volatile Tick _elapsed;
    static Queue _request;
};


class Delay
{
private:
    typedef RTC::Microsecond Microsecond;

public:
    Delay(const Microsecond & time): _time(time)  { Alarm::delay(_time); }

private:
    Microsecond _time;
};


// The following Scheduling Criteria depend on Alarm, which is not yet available at scheduler.h
namespace Scheduling_Criteria {
    inline FCFS::FCFS(int p): Priority((p == IDLE) ? IDLE : Alarm::_elapsed) {}

    inline EDF::EDF(const Microsecond & d, const Microsecond & p, const Microsecond & c, int): RT_Common(Alarm::ticks(d), Alarm::ticks(d), p, c) {}

    inline void EDF::update() {
        if((_priority > PERIODIC) && (_priority < APERIODIC))
            _priority = Alarm::_elapsed + _deadline;
    }
};

__END_SYS

#endif
