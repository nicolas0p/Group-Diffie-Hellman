// EPOS YYYY Timer Mediator Declarations

#ifndef __yyyy_timer_h
#define __yyyy_timer_h

#include <cpu.h>
#include <ic.h>
#include <rtc.h>
#include <timer.h>
#include <machine.h>

__BEGIN_SYS

// CHIP_YYYY_Timer
class CHIP_YYYY_Timer
{
public:
    // The timer's counter
    typedef CPU::Reg16 Count;

    // Clock
    static const int CLOCK = NNNNN;

public:
    CHIP_YYYY_Timer() {}

    static Hertz clock() { return CLOCK; }
};

// YYYY_Timer
class YYYY_Timer: public Timer_Common
{
    friend class YYYY;
    friend class Init_System;

public:
    /* The timer abstraction must feature at least 2 channels. If the machine only has one,
     * then it must be multiplexed as is the case for the PC. The scratch bellow represents this case. */
    typedef int Channel;
    enum {
        SCHEDULER,
        ALARM,
        USER1,
        USER2,
        USERN,
        USER = USER1
    };

protected:
    static const unsigned int CHANNELS = N;
    static const unsigned int FREQUENCY = Traits<YYYY_Timer>::FREQUENCY;

public:
    YYYY_Timer(const Hertz & frequency, const Handler & handler, const Channel & channel, bool retrigger = true)
    : _channel(channel), _initial(FREQUENCY / frequency), _retrigger(retrigger), _handler(handler) {
        db<Timer>(TRC) << "Timer(f=" << frequency
                       << ",h=" << reinterpret_cast<void *>(handler)
                       << ",ch=" << channel
                       << ") => {count=" << _initial << "}" << endl;

        if(_initial && (unsigned(channel) < CHANNELS) && !_channels[channel])
            _channels[channel] = this;
        else
            db<Timer>(WRN) << "Timer not installed!"<< endl;

        for(unsigned int i = 0; i < Traits<Machine>::CPUS; i++)
            _current[i] = _initial;

    }

    ~YYYY_Timer() {
        db<Timer>(TRC) << "~Timer(f=" << frequency()
        	       << ",h=" << reinterpret_cast<void*>(_handler)
        	       << ",ch=" << _channel
        	       << ") => {count=" << _initial << "}" << endl;

        _channels[_channel] = 0;
    }

    Hertz frequency() const { return (FREQUENCY / _initial); }
    void frequency(const Hertz & f) { _initial = FREQUENCY / f; reset(); }

    Tick read() { return _current[Machine::cpu_id()]; }

    int reset() {
        db<Timer>(TRC) << "Timer::reset() => {f=" << frequency()
        	       << ",h=" << reinterpret_cast<void*>(_handler)
        	       << ",count=" << _current[Machine::cpu_id()] << "}" << endl;

        int percentage = _current[Machine::cpu_id()] * 100 / _initial;
        _current[Machine::cpu_id()] = _initial;

        return percentage;
    }

    void handler(const Handler * handler) { _handler = handler; }

    static void enable() { IC::enable(IC::INT_TIMER); }
    static void disable() { IC::enable(IC::INT_TIMER); }

 private:
    static int init();

    static Hertz count2freq(const Count & c) {
        return c ? Engine::clock() / c : 0;
    }

    static Count freq2count(const Hertz & f) { 
        return f ? Engine::clock() / f : 0;
    }

    static void int_handler(unsigned int irq);

protected:
    unsigned int _channel;
    Count _initial;
    bool _retrigger;
    volatile Count _current[Traits<Machine>::CPUS];
    Handler * _handler;

    static YYYY_Timer * _channels[CHANNELS];
};


// Timer used by Thread::Scheduler
class Scheduler_Timer: public YYYY_Timer
{
private:
    typedef RTC::Microsecond Microsecond;

public:
    Scheduler_Timer(const Microsecond & quantum, const Handler & handler): YYYY_Timer(1000000 / quantum, handler, SCHEDULER) {}
};


// Timer used by Alarm
class Alarm_Timer: public YYYY_Timer
{
public:
    static const unsigned int FREQUENCY = Timer::FREQUENCY;

public:
    Alarm_Timer(const Handler & handler): YYYY_Timer(FREQUENCY, handler, ALARM) {}
};


// Timer available for users
class User_Timer: public YYYY_Timer
{
private:
    typedef RTC::Microsecond Microsecond;

public:
    User_Timer(const Microsecond & quantum, const Handler & handler): YYYY_Timer(1000000 / quantum, handler, USER, true) {}
};

__END_SYS

#endif
