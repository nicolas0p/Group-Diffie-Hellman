// EPOS Component Framework - Execution Scenario

// Scenario is simply an ordered collection of Aspect programs that are
// to be applied to a given component through its scenario adapter.

#ifndef __scenario_h
#define __scenario_h

#include <aspect/shared.h>
#include <aspect/authenticated.h>

__BEGIN_SYS

template<typename Component>
class Scenario: public Traits<Component>::ASPECTS::template Recur<Component>
{
protected:
    Scenario() {}

public:
    ~Scenario() {}

    void * operator new(size_t bytes) { return ::operator new(bytes, SYSTEM); }
    void operator delete(void * ptr) { ::operator delete(ptr); }
};

__END_SYS

#endif
