// EPOS PC NIC Mediator Initialization

#include <machine/pc/nic.h>

__BEGIN_SYS

template<int unit>
inline static void call_init()
{
    typedef typename Traits<NIC>::NICS::template Get<unit>::Result NIC;
    static const unsigned int OFFSET = Traits<NIC>::NICS::template Find<NIC>::Result;

    if(Traits<NIC>::enabled && (unit < Traits<Network>::NETWORKS::Length))
        NIC::init(unit - OFFSET);

    call_init<unit + 1>();
};

template<>
inline void call_init<Traits<NIC>::NICS::Length>()
{
};

void NIC::init()
{
    call_init<0>();
}

__END_SYS
