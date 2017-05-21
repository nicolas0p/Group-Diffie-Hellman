// EPOS Network Component Initialization

#include <system/config.h>
#ifndef __no_networking__

#include <network.h>
#include <elp.h>
#include <icmp.h>
#include <udp.h>
#include <tcp.h>
#include <tstp.h>
#include <http.h> // Quectel HTTP implements the whole stack, so it can be used as a Network

__BEGIN_SYS

template <int unit>
inline static void call_init()
{
    typedef typename Traits<Network>::NETWORKS::template Get<unit>::Result NET;

    if(Traits<NET>::enabled)
        NET::template init<unit>(NIC::template nic<unit>());

    call_init<unit + 1>();
};

template <>
inline void call_init<Traits<Network>::NETWORKS::Length>() {};

void Network::init()
{
    db<Init, Network>(TRC) << "Network::init()" << endl;

    call_init<0>();

    // If IP was initialized, initialize also the rest of the stack
    if(Traits<Network>::NETWORKS::Count<IP>::Result) {
        if(Traits<ICMP>::enabled)
            new (SYSTEM) ICMP;
        if(Traits<UDP>::enabled)
            new (SYSTEM) UDP;
        if(Traits<TCP>::enabled)
            new (SYSTEM) TCP;
    }

    // If TSTP was initialized, initialize also the rest of the stack
    if(Traits<Network>::NETWORKS::Count<TSTP>::Result) {
//        if(Traits<TSTP>::enabled)
//            new (SYSTEM) TSTP;
    }

}

__END_SYS

#endif
