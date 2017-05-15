// EPOS Hypertext Transfer Protocol Initialization

#include <system/config.h>
#ifndef __no_networking__

#include <http.h>

__BEGIN_SYS

template<unsigned int UNIT>
void Quectel_HTTP::init(const NIC & nic)
{
    db<Init, Quectel_HTTP>(TRC) << "Quectel_HTTP::init(U=" << UNIT << ")" << endl;

    _networks[UNIT] = new (SYSTEM) Quectel_HTTP(nic);
}

template void Quectel_HTTP::init<0>(const NIC & nic);
template void Quectel_HTTP::init<1>(const NIC & nic);
template void Quectel_HTTP::init<2>(const NIC & nic);
template void Quectel_HTTP::init<3>(const NIC & nic);
template void Quectel_HTTP::init<4>(const NIC & nic);
template void Quectel_HTTP::init<5>(const NIC & nic);
template void Quectel_HTTP::init<6>(const NIC & nic);
template void Quectel_HTTP::init<7>(const NIC & nic);

__END_SYS

#endif
