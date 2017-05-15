// EPOS Component Authentication Aspect Program

#ifndef __authenticated_h
#define __authenticated_h

#include <system/config.h>

__BEGIN_SYS

template<typename Component>
class Authenticated
{
protected:
    Authenticated() {}

public:
    void enter() { db<Aspect>(TRC) << "Authenticated::enter()" << endl; }
    void leave() { db<Aspect>(TRC) << "Authenticated::leave()" << endl; }

    static void static_enter() { db<Aspect>(TRC) << "Authenticated::static_enter()" << endl; }
    static void static_leave() { db<Aspect>(TRC) << "Authenticated::static_leave()" << endl; }
};

__END_SYS

#endif
