// EPOS ARM Cortex NIC Mediator Declarations

#include <nic.h>

#ifndef __cortex_nic_h
#define __cortex_nic_h

#include <system.h>
#include "machine.h"

#ifdef __mmod_zynq__

#include <ethernet.h>
#include "gem.h"

__BEGIN_SYS

class NIC: public Ethernet
{
    friend class Machine;

private:
    typedef Traits<NIC>::NICS NICS;
    typedef IF<NICS::Polymorphic, NIC_Base<Ethernet>, NICS::Get<0>::Result>::Result Device;
    static const unsigned int UNITS = NICS::Length;

public:
    typedef Data_Observer<Buffer, Protocol> Observer;
    typedef Data_Observed<Buffer, Protocol> Observed;

private:
    NIC(Device * dev) {
        _dev = dev;
    }

public:
    template<unsigned int UNIT = 0>
    NIC(unsigned int u = UNIT) {
        _dev = reinterpret_cast<Device *>(NICS::Get<UNIT>::Result::get(u));
        db<NIC>(TRC) << "NIC::NIC(u=" << UNIT << ",d=" << _dev << ") => " << this << endl;
    }

    template<unsigned int UNIT>
    static NIC nic() {
        typedef typename Traits<NIC>::NICS::template Get<UNIT>::Result NIC_Type;
        static const unsigned int OFFSET = Traits<NIC>::NICS::template Find<NIC_Type>::Result;
        return NIC(reinterpret_cast<Device *>(NICS::Get<UNIT>::Result::get(UNIT - OFFSET))); // An error at this line might mean that the number of NETWORKS in Traits<Network> mismatches the number of NICS in Traits<NIC>
    }

    int send(const Address & dst, const Protocol & prot, const void * data, unsigned int size) { return _dev->send(dst, prot, data, size); }
    int receive(Address * src, Protocol * prot, void * data, unsigned int size) { return _dev->receive(src, prot, data, size); }

    Buffer * alloc(const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload) { return _dev->alloc(this, dst, prot, once, always, payload); }
    int send(Buffer * buf) { return _dev->send(buf); }
    void free(Buffer * buf) { _dev->free(buf); }

    const Address & address() { return _dev->address(); }
    void address(const Address & address) { _dev->address(address); }

    const Statistics & statistics() { return _dev->statistics(); }

    void reset() { _dev->reset(); }

    void attach(Observer * obs, const Protocol & prot) { _dev->Ethernet::Observed::attach(obs, prot); }
    void detach(Observer * obs, const Protocol & prot) { _dev->Ethernet::Observed::detach(obs, prot); }
    void notify(const Protocol & prot, Buffer * buf) { _dev->Ethernet::Observed::notify(prot, buf); }

private:
    static void init();

private:
    Device * _dev;
};

__END_SYS

#else

#include <ieee802_15_4.h>
#include "cc2538.h"
#include "m95.h"

__BEGIN_SYS

class NIC: public IEEE802_15_4
{
    friend class Machine;

private:
    typedef Traits<NIC>::NICS NICS;
    typedef IF<NICS::Polymorphic, NIC_Base<IEEE802_15_4>, NICS::Get<0>::Result>::Result Device;

    static const unsigned int UNITS = NICS::Length;

public:
    typedef Data_Observer<Buffer, Protocol> Observer;
    typedef Data_Observed<Buffer, Protocol> Observed;

private:
    NIC(Device * dev) {
        _dev = dev;
    }

public:
    template<unsigned int UNIT = 0>
    NIC(unsigned int u = UNIT) {
        _dev = reinterpret_cast<Device *>(NICS::Get<UNIT>::Result::get(u));
        db<NIC>(TRC) << "NIC::NIC(u=" << UNIT << ",d=" << _dev << ") => " << this << endl;
    }

    template<unsigned int UNIT>
    static NIC nic() {
        typedef typename Traits<NIC>::NICS::template Get<UNIT>::Result NIC_Type;
        static const unsigned int OFFSET = Traits<NIC>::NICS::template Find<NIC_Type>::Result;
        return NIC(reinterpret_cast<Device *>(NICS::Get<UNIT>::Result::get(UNIT - OFFSET))); // An error at this line might mean that the number of NETWORKS in Traits<Network> mismatches the number of NICS in Traits<NIC>
    }

    Buffer * alloc(NIC * nic, const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload) { return _dev->alloc(nic, dst, prot, once, always, payload); }
    Buffer * alloc(const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload) { return _dev->alloc(this, dst, prot, once, always, payload); }
    int send(Buffer * buf) { return _dev->send(buf); }
    void free(Buffer * buf) { _dev->free(buf); }

    int send(const Address & dst, const Protocol & prot, const void * data, unsigned int size) { return _dev->send(dst, prot, data, size); }
    int receive(Address * src, Protocol * prot, void * data, unsigned int size) { return _dev->receive(src, prot, data, size); }

    const unsigned int mtu() const { return _dev->mtu(); }
    const Address broadcast() const { return _dev->broadcast(); }

    const Address & address() { return _dev->address(); }
    void address(const Address & address) { _dev->address(address); }

    const Statistics & statistics() { return _dev->statistics(); }

    void reset() { _dev->reset(); }

    void attach(Observer * obs, const Protocol & prot) { _dev->attach(obs, prot); }
    void detach(Observer * obs, const Protocol & prot) { _dev->detach(obs, prot); }
    void notify(const Protocol & prot, Buffer * buf) { _dev->notify(prot, buf); }

private:
    static void init();

private:
    Device * _dev;
};

__END_SYS

#endif

#endif
