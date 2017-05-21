// EPOS Cortex Hydrology Board Mediator Declarations

#ifndef __cortex_hydro_board_h
#define __cortex_hydro_board_h

#include <tsc.h>
#include <gpio.h>
#include <adc.h>
#include <machine.h>
#include <utility/observer.h>
#include <hydro_board.h>

__BEGIN_SYS

class Hydro_Board: public Hydro_Board_Common
{
private:
    friend class Machine;

    static const TSC::Time_Stamp INTERRUPT_DEBOUNCE_TIME = Traits<Hydro_Board>::INTERRUPT_DEBOUNCE_TIME * (TSC::FREQUENCY / 1000000);

public:
    typedef _UTIL::Observer Observer;
    typedef _UTIL::Observed Observed;

public:
    static int rain() { return pulses(); }

    static int water_flow() { return pulses(); }

    static int turbidity(unsigned int dev) {
        unsigned int infrared_dev = dev == 0 ? 3 : dev - 1;
        on(dev);
        // Filter daylight as directed by sensor manufacturer
        Machine::delay(3250000);
        int daylight = read(dev);
        on(infrared_dev);
        Machine::delay(450000);
        int mixed = read(dev);
        off(infrared_dev);
        off(dev);
        return mixed - daylight;
    }

    static int level(unsigned int dev) {
        on(dev);
        Machine::delay(3000000);
        int ret = read(dev);
        off(dev);
        return ret;
    }

    static void attach(Observer * obs) { _observed.attach(obs); }
    static void detach(Observer * obs) { _observed.detach(obs); }

private:
    static unsigned int pulses() {
        unsigned int ret = _pulse_count;
        _pulse_count = 0;
        return ret;
    }
    static void on(unsigned int dev) {
        assert(dev < 4);
        _relay[dev]->set();
    }
    static void off(unsigned int dev) {
        assert(dev < 4);
        _relay[dev]->clear();
    }
    static int read(unsigned int dev) {
        assert(dev < 4);
        return _adc[dev]->read();
    }

private:
    static void pulse_int(const IC::Interrupt_Id & id) {
        TSC::Time_Stamp now;
        if(((now = TSC::time_stamp()) - _last_interrupt) < INTERRUPT_DEBOUNCE_TIME)
            return;
        _last_interrupt = now;
        _pulse_count++;
        notify();
    }

    static bool notify() { return _observed.notify(); }

    static void init();

private:
    static Observed _observed;

    static GPIO * _pulses;
    static GPIO * _relay[4];
    static ADC * _adc[4];

    static volatile unsigned int _pulse_count;
    static volatile TSC::Time_Stamp _last_interrupt;
};

__END_SYS

#endif
