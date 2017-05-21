// EPOS Component Sharing Control Aspect Program

#ifndef __shared_h
#define __shared_h

#include <system/config.h>

__BEGIN_SYS

template<typename Component>
class Shared
{
protected:
    Shared() {}

public:
    void enter() { db<Aspect>(TRC) << "Shared::enter()" << endl; }
    void leave() { db<Aspect>(TRC) << "Shared::leave()" << endl; }

    static void static_enter() { db<Aspect>(TRC) << "Shared::static_enter()" << endl; }
    static void static_leave() { db<Aspect>(TRC) << "Shared::static_leave()" << endl; }
};

__END_SYS

#endif
