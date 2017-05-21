// EPOS PC Timer Mediator Implementation

#include <machine.h>
#include <ic.h>
#include <machine/pc/timer.h>

__BEGIN_SYS

// Class attributes
Timer * Timer::_channels[CHANNELS];

// Class methods
void Timer::int_handler(const Interrupt_Id & i)
{
    if(_channels[SCHEDULER] && (--_channels[SCHEDULER]->_current[Machine::cpu_id()] <= 0)) {
        _channels[SCHEDULER]->_current[Machine::cpu_id()] = _channels[SCHEDULER]->_initial;
        _channels[SCHEDULER]->_handler(i);
    }

    if((!Traits<System>::multicore || (Traits<System>::multicore && (Machine::cpu_id() == 0))) && _channels[ALARM]) {
        _channels[ALARM]->_current[0] = _channels[ALARM]->_initial;
        _channels[ALARM]->_handler(i);
    }

    if((!Traits<System>::multicore || (Traits<System>::multicore && (Machine::cpu_id() == 0))) && _channels[USER]) {
        if(_channels[USER]->_retrigger)
            _channels[USER]->_current[0] = _channels[USER]->_initial;
        _channels[USER]->_handler(i);
    }
}

__END_SYS
