// EPOS Cortex Hydrology Board Mediator Initialization

#include <system/config.h>

#ifdef __HYDRO_BOARD_H

#include <machine/cortex/hydro_board.h>

__BEGIN_SYS

void Hydro_Board::init()
{
    db<Init, Hydro_Board>(TRC) << "Hydro_Board::init()" << endl;

    if(Traits<Hydro_Board>::P3_enabled) {
        _relay[0] = new (SYSTEM) GPIO('B', 0, GPIO::OUT);
        _adc[0] = new (SYSTEM) ADC(ADC::SINGLE_ENDED_ADC2);
        _relay[0]->clear();
    }
    if(Traits<Hydro_Board>::P4_enabled) {
        _relay[1] = new (SYSTEM) GPIO('B', 1, GPIO::OUT);
        _adc[1] = new (SYSTEM) ADC(ADC::SINGLE_ENDED_ADC3);
        _relay[1]->clear();
    }
    if(Traits<Hydro_Board>::P5_enabled) {
        _relay[2] = new (SYSTEM) GPIO('B', 2, GPIO::OUT);
        _adc[2] = new (SYSTEM) ADC(ADC::SINGLE_ENDED_ADC4);
        _relay[2]->clear();
    }
    if(Traits<Hydro_Board>::P6_enabled) {
        _relay[3] = new (SYSTEM) GPIO('B', 3, GPIO::OUT);
        _adc[3] = new (SYSTEM) ADC(ADC::SINGLE_ENDED_ADC5);
        _relay[3]->clear();
    }
    if(Traits<Hydro_Board>::P7_enabled) {
        _last_interrupt = 0;
        _pulse_count = 0;
        _pulses = new (SYSTEM) GPIO('B', 4, GPIO::IN, GPIO::UP, &pulse_int, GPIO::FALLING);
    }
}

__END_SYS

#endif
