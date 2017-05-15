// EPOS YYYY Timer Mediator Initialization

#include <timer.h>
#include <ic.h>

__BEGIN_SYS

int PC_Timer::init()
{
    db<Init, Timer>(TRC) << "Timer::init()" << endl;

    CPU::int_disable();
    
    Engine::config(0, Engine::clock() / FREQUENCY);

    IC::int_vector(IC::INT_TIMER, int_handler);
    IC::enable(IC::INT_TIMER);

    CPU::int_enable();

    return 0;
}

__END_SYS
