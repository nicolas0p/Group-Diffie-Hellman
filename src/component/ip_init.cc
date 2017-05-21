// EPOS IP Protocol Initialization

#include <system/config.h>
#ifndef __no_networking__

#include <ip.h>

__BEGIN_SYS

template<typename Config>
IP::IP(const NIC & nic, const Config & config)
: _nic(nic), _arp(&_nic, this), _address(Config::ADDRESS), _netmask(Config::NETMASK), _broadcast((_address & _netmask) | ~_netmask), _gateway(Config::GATEWAY)
{
    db<IP>(TRC) << "IP::IP(nic=" << &_nic << ") => " << this << endl;

    _nic.attach(this, NIC::IP);

    if(Config::TYPE == Traits<IP>::MAC)
        config_by_mac();
    else if(Config::TYPE == Traits<IP>::INFO)
        config_by_info();
    else if(Config::TYPE == Traits<IP>::RARP)
        config_by_rarp();
    else if(Config::TYPE == Traits<IP>::DHCP)
        config_by_dhcp();

    _router.insert(&_nic, this, &_arp, _address & _netmask, _address, _netmask);

    if(_gateway) {
        _router.insert(&_nic, this, &_arp, Address::NULL, _gateway, Address::NULL); // Default route must be the last one in table
        _arp.resolve(_gateway);
    }
}

template<unsigned int UNIT>
void IP::init(const NIC & nic)
{
    db<Init, IP>(TRC) << "IP::init(u=" << UNIT << ")" << endl;

    _networks[UNIT] = new (SYSTEM) IP(nic, Traits<IP>::Config<UNIT>());
}

template void IP::init<0>(const NIC & nic);
template void IP::init<1>(const NIC & nic);
template void IP::init<2>(const NIC & nic);
template void IP::init<3>(const NIC & nic);
template void IP::init<4>(const NIC & nic);
template void IP::init<5>(const NIC & nic);
template void IP::init<6>(const NIC & nic);
template void IP::init<7>(const NIC & nic);

__END_SYS

#endif
