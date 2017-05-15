// EPOS Active Object Component Declarations

#ifndef __active_h
#define __active_h

#include <thread.h>

__BEGIN_SYS

class Active: public Thread
{
public:
    Active(): Thread(Configuration(Thread::SUSPENDED), &entry, this) {}
    virtual ~Active() {}

    virtual int run() = 0;

    void start() { resume(); }

private:
    static int entry(Active * runnable) { return runnable->run(); }
};

__END_SYS

#endif
