// EPOS Task Component Implementation

#include <task.h>

__BEGIN_SYS

// Class attributes
Task * volatile Task::_current;


// Methods
Task::~Task()
{
    db<Task>(TRC) << "~Task(this=" << this << ")" << endl;

    while(!_threads.empty())
        delete _threads.remove()->object();

    delete _as;
}

__END_SYS
