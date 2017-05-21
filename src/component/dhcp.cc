// EPOS DHCP (RFCs 2131 and 2132) Protocol Implementation

#include <system/config.h>
#ifndef __no_networking__

#include <dhcp.h>

__BEGIN_SYS

DHCP::Client::Client(const MAC_Address & mac, IP * ip):
Link<UDP>(68, Link<UDP>::Address(~0UL, 67)), _xid(Random::random())
{
    IP::Address address = IP::Address::NULL;
    IP::Address netmask = IP::Address::NULL;
    IP::Address broadcast = IP::Address::NULL;
    IP::Address gateway = IP::Address::NULL;
    IP::Address nameserver = IP::Address::NULL;

    Packet<> packet;

    db<DHCP>(TRC) << "DHCP::Client()" << endl;

    while(true) {
        Discover * discover = new (&packet) Discover(mac, _xid);
        db<IP>(INF) << "DHCP::Client: sending discover with MAC=" << mac << " and XID=" << _xid << endl;
        send(discover, sizeof(Discover));

        Offer * offer = new (&packet) Offer;
        receive(offer, sizeof(Offer));
        if((offer->type() != OFFER) || (offer->xid() != _xid)) {
            db<IP>(INF) << "DHCP::Client: offer does not belong to me!" << endl;
            continue;
        } else
            db<DHCP>(INF) << "DHCP::Clinet: server " << offer->siaddr() << " offered " << offer->yiaddr() << endl;

        db<IP>(INF) << "DHCP::Client: sending request for " << offer->yiaddr() << " to " << offer->siaddr() << " with MAC=" << mac << " and XID=" << _xid << endl;
        Request * request = new (&packet) Request(mac, _xid, offer->yiaddr(), offer->siaddr());
        send(request, sizeof(Offer));

        Ack * ack = new (&packet) Ack;
        receive(ack, sizeof(Ack));

        if((ack->type() == ACK) && (ack->xid() == _xid)) {
            db<DHCP>(INF) << "DHCP::Client: ack received!" << endl;
            address = ack->yiaddr();
            parse_options(ack, &netmask, &broadcast, &gateway, &nameserver);
            if(!broadcast)
                broadcast = (address & netmask) | ~netmask;
            break;
        }
    }

    db<DHCP>(INF) << "DHCP::Clinet() => {a=" << address << ",m=" << netmask << ",b=" << broadcast << ",g=" << gateway << ",n=" << nameserver << "}" << endl;
    ip->reconfigure(address, netmask, gateway);
}

void DHCP::Client::parse_options(Packet<> * packet, IP::Address * netmask, IP::Address * broadcast, IP::Address * gateway, IP::Address * nameserver)
{
    unsigned char * opt = packet->options();

    for(unsigned int i = 0; i < Packet<>::MAX_OPTIONS_LENGTH; i++) {
        switch(opt[i]) {
        case 0: // padding
            break;
        case 1: // netmask
            ++i;
            if(opt[i] == 4) { // IPv4
                (*netmask)[0] = opt[i+1];
                (*netmask)[1] = opt[i+2];
                (*netmask)[2] = opt[i+3];
                (*netmask)[3] = opt[i+4];
                db<DHCP>(INF) << "DHCP::Client::parse:netmask=" << *netmask << endl;
            }
            i += opt[i];
            break;
        case 3: // gateways
            ++i;
            if(opt[i] >= 4) { // one or more, let's get the first
                (*gateway)[0] = opt[i+1];
                (*gateway)[1] = opt[i+2];
                (*gateway)[2] = opt[i+3];
                (*gateway)[3] = opt[i+4];
                db<DHCP>(INF) << "DHCP::Client::parse:gateway=" << *gateway << endl;
            }
            i += opt[i];
            break;
        case 6: // nameservers
            ++i;
            if(opt[i] >= 4) { // one or more, let's get the first
                (*nameserver)[0] = opt[i+1];
                (*nameserver)[1] = opt[i+2];
                (*nameserver)[2] = opt[i+3];
                (*nameserver)[3] = opt[i+4];
                db<DHCP>(INF) << "DHCP::Client::parse:nameserver=" << *nameserver << endl;
            }
            i += opt[i];
            break;
        case 28: // broadcast address
            ++i;
            if(opt[i] == 4) { // IPv4
                (*broadcast)[0] = opt[i+1];
                (*broadcast)[1] = opt[i+2];
                (*broadcast)[2] = opt[i+3];
                (*broadcast)[3] = opt[i+4];
                db<DHCP>(INF) << "DHCP::Client::parse:broadcast=" << *broadcast << endl;
            }
            i += opt[i];
            break;
        case 51: // lease time in seconds
            ++i;
            if(opt[i] == 4) { // valid!
                _lease_time = ((opt[i+1] << 24) & 0xff000000) | ((opt[i+2] << 16) & 0x00ff0000) |
                              ((opt[i+3] << 8 ) & 0x0000ff00) | (opt[i+4] & 0x000000ff);
                db<DHCP>(INF) << "DHCP::Client::parse:lease time=" << _lease_time << endl;
            }
            i += opt[i];
            break;
        case 255: // end
            i = 255; // get out of the loop
            break;
        default:
            db<DHCP>(INF) << "DHCP::Client::parse:unknown code=" << hex << opt[i] << dec << ",len=" << opt[i+1] << endl;
            i += opt[i+1] + 1;
        }
    }
}

__END_SYS

#endif
