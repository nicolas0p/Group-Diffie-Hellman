// EPOS YYYY Timer Mediator Implementation

#include <machine.h>
#include <timer.h>

__BEGIN_SYS

// Class attributes
PC_Timer * PC_Timer::_channels[CHANNELS];

// Class methods
void PC_Timer::int_handler(unsigned int i)
{
    if(_channels[SCHEDULER] && (--_channels[SCHEDULER]->_current[Machine::cpu_id()] <= 0)) {
        _channels[SCHEDULER]->_current[Machine::cpu_id()] = _channels[SCHEDULER]->_initial;
        _channels[SCHEDULER]->_handler();
    }

    if((!Traits<System>::multicore || (Traits<System>::multicore && (Machine::cpu_id() == 0))) && _channels[ALARM]) {
        _channels[ALARM]->_current[0] = _channels[ALARM]->_initial;
        _channels[ALARM]->_handler();
    }

    if((!Traits<System>::multicore || (Traits<System>::multicore && (Machine::cpu_id() == 0))) && _channels[USER]) {
        if(_channels[USER]->_retrigger)
            _channels[USER]->_current[0] = _channels[USER]->_initial;
        _channels[USER]->_handler();
    }
}

__END_SYS
