// EPOS ICMP Protocol Implementation

#include <system/config.h>
#ifndef __no_networking__

#include <icmp.h>

__BEGIN_SYS

// Class attributes
ICMP::Observed ICMP::_observed;

// Methods
int ICMP::send(const Address::Local & from, const Address & to, const void * data, unsigned int s)
{
    db<ICMP>(TRC) << "ICMP::send(t=" << to << ",d=" << data << ",s=" << s << ")" << endl;

    Buffer * buf = IP::alloc(to, IP::ICMP, sizeof(Header), sizeof(Data));
    if(!buf)
        return 0;

    IP::Packet * dgram = buf->frame()->data<IP::Packet>();
    Packet * packet = dgram->data<Packet>();
    unsigned int size = (s >= sizeof(Packet)) ? sizeof(Packet) : s;

    memcpy(packet, data, size);
    packet->sum();

    IP::send(buf);

    return size;
}

int ICMP::receive(Buffer * buf, Address * from, void * data, unsigned int s)
{
    db<ICMP>(TRC) << "ICMP::receive(buf=" << buf << ",d=" << data << ",s=" << s << ")" << endl;

    IP::Packet * dgram = buf->frame()->data<IP::Packet>();
    Packet * packet = dgram->data<Packet>();
    unsigned int size = (s >= sizeof(Packet)) ? sizeof(Packet) : s;

    if(!packet->check()) {
        db<ICMP>(INF) << "ICMP::update: wrong checksum!" << endl;
        buf->nic()->free(buf);
        return 0;
    }

    *from = dgram->from();
    memcpy(data, packet, size);
    buf->nic()->free(buf);

    return size;
}

void ICMP::update(IP::Observed * obs, IP::Protocol prot, NIC::Buffer * buf)
{
    db<ICMP>(TRC) << "ICMP::update(obs=" << obs << ",prot=" << prot << ",buf=" << buf << ")" << endl;

    if(!notify(0, buf))
        buf->nic()->free(buf);
}

__END_SYS

#endif

