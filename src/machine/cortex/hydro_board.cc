// EPOS Cortex Hydrology Board Mediator Implementation

#include <system/config.h>

#ifdef __HYDRO_BOARD_H

#include <machine/cortex/hydro_board.h>

__BEGIN_SYS

// Class attributes
Observed Hydro_Board::_observed;
GPIO * Hydro_Board::_pulses;
GPIO * Hydro_Board::_relay[4];
ADC * Hydro_Board::_adc[4];
volatile unsigned int Hydro_Board::_pulse_count;
volatile TSC::Time_Stamp Hydro_Board::_last_interrupt;

// Class methods

__END_SYS

#endif

