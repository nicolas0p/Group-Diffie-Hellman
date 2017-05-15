// EPOS Cortex Smart Plug Mediator Declarations

#ifndef __cortex_smart_plug_h
#define __cortex_smart_plug_h

#include <adc.h>
#include <pwm.h>
#include <gpio.h>
#include <machine.h>
#include <utility/observer.h>
#include <smart_plug.h>

__BEGIN_SYS

class Power_Meter
{
private:
    static const int SAMP_FREQ = 2400;
    static const int N = 128;
    static const int N_ERR = 11;

    typedef ADC::Channel Channel;

public:
    Power_Meter(Channel v_chan, Channel i_chan, Channel ref_chan):
        _v_chan(v_chan), _i_chan(i_chan), _ref_chan(ref_chan) {}

    ~Power_Meter() {}

    // Calculate the average electrical power of sinusoidal signals using the
    // following equation: integrate (from 0 to T) { v[t]*i[t]*dt }
    unsigned int average() {
        int j;
        long i_avg = 0, ref, //p_avg = 0, v[N], v_shift[N],
             i[N], i_rms = 0;//p[N];

        ref = _ref_chan.read();

        for(j = 0; j < N; j++) {
            i[j] = _i_chan.read() - ref;
            //v[j] = _v_chan.read() - ref;

            // The measured voltage stored in v[] is sinusoidal signal being
            // rectified by a diode. v_shift[] is an approximation of the
            // blocked half signal based on the measured voltage and is used to
            // reconstruct the complete signal.
            //v_shift[j + N_ERR] = v[j];

            i_avg += i[j];

            Machine::delay(1000000/SAMP_FREQ);
        }

        i_avg /= N;

        for(j = 0; j < N; j++) {
            // Rebuild the complete voltage signal
            //v[j] = v[j] - v_shift[j];
            // Remove DC bias from i[]
            i[j] = i[j] - i_avg;
            i_rms += i[j] * i[j];
            // Calculate instant power
            //p[j] = v[j]*i[j];
        }
        i_rms /= N;

        // Remove the 11 first voltage samples, that aren't properly
        // reconstructed
        //for(j = N_ERR; j < N; j++) {
        //    p_avg += p[j];
        //}

        //p_avg = p_avg/(N - N_ERR);

        if(i_rms < 0)
            return 0;
        else
            // Conversion constants to watts
            return i_rms;
        //if(p_avg < 0)
        //    return 0;
        //else
        //    // Conversion constants to watts
        //    return p_avg/3912;
    }

private:
    ADC _v_chan;
    ADC _i_chan;
    ADC _ref_chan;
};

template<bool dimmer = true>
class _Smart_Plug_Actuator {
public:
    _Smart_Plug_Actuator(PWM * pwm) : _pwm(pwm) {}
    _Smart_Plug_Actuator(GPIO * gpio) {}

    void actuate(const Percent & duty_cycle) {
        if((duty_cycle != 0) && (duty_cycle != 1))
        _pwm->duty_cycle(duty_cycle);
    }

private:
    PWM * _pwm;
};

template<>
class _Smart_Plug_Actuator<false> {
public:
    _Smart_Plug_Actuator(PWM * pwm){}
    _Smart_Plug_Actuator(GPIO * gpio) : _gpio(gpio) { _gpio->set(); }

    void actuate(const Percent & data) {
        if(data == 0)
            _gpio->clear();
        else if(data == 1)
            _gpio->set();
    }

private:
    GPIO * _gpio;
};

class Smart_Plug: public Smart_Plug_Common
{
private:
    friend class Machine;
    typedef Power_Meter Engine;

public:
    typedef _UTIL::Observer Observer;
    typedef _UTIL::Observed Observed;

    typedef _Smart_Plug_Actuator<Traits<Smart_Plug>::P1_ACTUATOR == Traits<Smart_Plug>::DIMMER> Actuator0;
    typedef _Smart_Plug_Actuator<Traits<Smart_Plug>::P2_ACTUATOR == Traits<Smart_Plug>::DIMMER> Actuator1;

public:
    Smart_Plug() {}

    static unsigned int current(unsigned int dev) {
        assert(dev < 2);
        return _power_meter[dev]->average();
    }

    static void actuate(unsigned int dev, Percent data) {
        _actuator0->actuate(data);
        _actuator1->actuate(data);
    }

    static void attach(Observer * obs) { _observed.attach(obs); }
    static void detach(Observer * obs) { _observed.detach(obs); }

private:
    static bool notify() { return _observed.notify(); }

    static void init();

private:
    static Observed _observed;
    static Power_Meter * _power_meter[2];
    static Actuator0 * _actuator0;
    static Actuator1 * _actuator1;
};

__END_SYS

#endif
