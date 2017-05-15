// EPOS Zynq Gigabit Ethernet Controller NIC Mediator Declarations

#ifndef __gem_h
#define __gem_h

#include <ethernet.h>

__BEGIN_SYS

class GEM: public Ethernet::NIC_Base<Ethernet, Traits<NIC>::NICS::Polymorphic> //, private Engine
{
    template <typename Type, int unit> friend void call_init();

protected:
    GEM(){}

public:
    ~GEM() {}

    int send(const Address & dst, const Protocol & prot, const void * data, unsigned int size) { return 0; }
    int receive(Address * src, Protocol * prot, void * data, unsigned int size) { return 0; }


    Buffer * alloc(NIC * nic, const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload);
    void free(Buffer * buf) {}
    int send(Buffer * buf) { return 0; }

    const Address & address() { Address * a = new (SYSTEM) Address; return *a; }
    void address(const Address & address) {}

    const Statistics & statistics() { Statistics * s = new (SYSTEM) Statistics; return *s; }

    void reset() {}

    static GEM * get(unsigned int unit = 0) { GEM * g = new (SYSTEM) GEM; return g; }

private:
    static void init(unsigned int unit);
};

__END_SYS

#endif
