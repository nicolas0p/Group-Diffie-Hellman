// EPOS Cortex Smart Plug Mediator Initialization

#include <system/config.h>

#ifdef __SMART_PLUG_H

#include <machine/cortex/smart_plug.h>
#include <adc.h>

__BEGIN_SYS

void Smart_Plug::init()
{
    db<Init, Smart_Plug>(TRC) << "Smart_Plug::init()" << endl;

    if(Traits<Smart_Plug>::P1_power_meter_enabled)
        _power_meter[0] = new (SYSTEM) Power_Meter(ADC::SINGLE_ENDED_ADC7, ADC::SINGLE_ENDED_ADC5, ADC::GND);
    if(Traits<Smart_Plug>::P2_power_meter_enabled)
        _power_meter[1] = new (SYSTEM) Power_Meter(ADC::SINGLE_ENDED_ADC7, ADC::SINGLE_ENDED_ADC6, ADC::GND);

    switch(Traits<Smart_Plug>::P1_ACTUATOR) {
        case Traits<Smart_Plug>::DIMMER: {
            User_Timer * t = new (SYSTEM) User_Timer(Traits<Smart_Plug>::PWM_TIMER_CHANNEL, Traits<Smart_Plug>::PWM_PERIOD, 0);
            GPIO * g = new (SYSTEM) GPIO('D', 3, GPIO::OUT);
            _actuator0 = new (SYSTEM) Actuator0(new (SYSTEM) PWM(t, g, 50));
        } break;
        case Traits<Smart_Plug>::SWITCH: {
            GPIO * g = new (SYSTEM) GPIO('D', 3, GPIO::OUT);
            _actuator0 = new (SYSTEM) Actuator0(g);
        } break;
        default:
            break;
    }

    switch(Traits<Smart_Plug>::P2_ACTUATOR) {
        case Traits<Smart_Plug>::DIMMER: {
            User_Timer * t = new (SYSTEM) User_Timer(Traits<Smart_Plug>::PWM_TIMER_CHANNEL, Traits<Smart_Plug>::PWM_PERIOD, 0);
            GPIO * g = new (SYSTEM) GPIO('D', 2, GPIO::OUT);
            _actuator1 = new (SYSTEM) Actuator1(new (SYSTEM) PWM(t, g, 50));
        } break;
        case Traits<Smart_Plug>::SWITCH: {
            GPIO * g = new (SYSTEM) GPIO('D', 2, GPIO::OUT);
            _actuator1 = new (SYSTEM) Actuator1(g);
        } break;
        default:
            break;
    }
}

__END_SYS

#endif
