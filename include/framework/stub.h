// EPOS Component Framework - Component Stub

// Stub selectively binds the Handle either to the component's scenario Adapter or to its Proxy.
// Proxies are used in the kernel mode or when a component is subject to the Remote Aspect program.

#ifndef __stub_h
#define __stub_h

#include "adapter.h"
#include "proxy.h"

__BEGIN_SYS

template<typename Component, bool remote>
class Stub: public Adapter<Component>
{
public:
    template<typename ... Tn>
    Stub(const Tn & ... an): Adapter<Component>(an ...) {}
    ~Stub() {}
};

template<typename Component>
class Stub<Component, true>: public Proxy<Component>
{
public:
    template<typename ... Tn>
    Stub(const Tn & ... an): Proxy<Component>(an ...) {}

    // Dereferencing stubs for Task(cs, ds, ...)
    template<typename ... Tn>
    Stub(const Stub<Segment, true> & cs, const Stub<Segment, true> & ds, const Tn & ... an): Proxy<Component>(cs.id().unit(), ds.id().unit(), an ...) {}

    ~Stub() {}
};

__END_SYS

#endif
