// EPOS DHCP (RFCs 2131 and 2132) Protocol Declarations

#ifndef __dhdcp_h
#define __dhdcp_h

#include <utility/random.h>
#include <utility/string.h>
#include <communicator.h>

__BEGIN_SYS

class DHCP
{
public:
    typedef NIC::Address MAC_Address;
    typedef IP::Address Address;

    // DHCP Message Types
    enum {
        DISCOVER = 1,
        OFFER,
        REQUEST,
        DECLINE,
        ACK,
        NAK,
        RELEASE
    };

    template<unsigned int OPTIONS = 306>
    class Packet
    {
    public:
        static const unsigned int MAX_OPTIONS_LENGTH = 306;

    public:
        Packet() {
            memset(this, 0, sizeof(Packet));
            _magic[0] = 0x63;
            _magic[1] = 0x82;
            _magic[2] = 0x53;
            _magic[3] = 0x63;
            _end = 255;
        }

        unsigned char op() const { return _op; }
        unsigned long xid() const { return _xid; }
        unsigned short secs() const { return ntohs(_secs); }
        const Address & yiaddr() const { return _yiaddr; }
        const Address & siaddr() const { return _siaddr; }
        unsigned char type() { return _options[2]; }
        unsigned char * options() { return _options; }

    protected:
        unsigned char  _op;             // Message type: 1 = Request, 2 = Reply
        unsigned char  _htype;          // Hardware address type: 1 = Ethernet
        unsigned char  _hlen;           // Hardware address length in bytes
        unsigned char  _hops;           // Hops: 0 for clients
        unsigned long  _xid;            // Transaction id (random)
        unsigned short _secs;           // Seconds elapsed since client started trying to boot
        unsigned short _flags;          // Bit 0: broadcast, 1-15: reserved
        Address        _ciaddr;         // Client IP address for Request
        Address        _yiaddr;         // Client IP address offered by Server
        Address        _siaddr;         // Server IP address
        Address        _giaddr;         // Relay Server IP address
        unsigned char  _chaddr[16];     // Client MAC address
        unsigned char  _server[64];     // Server name (C string)
        unsigned char  _file[128];      // Boot file name (C string)
        unsigned char  _magic[4];       // BOOTP Magic Cookie
        unsigned char  _options[OPTIONS];
        unsigned char  _end;            // 255 -> end of options
        unsigned char  _padding[312 - 5 - OPTIONS]; // MAX (312) - _magic - _end - _options
    } __attribute__((packed));;

    class Discover: public Packet<3>
    {
    public:
        Discover(const NIC::Address & mac, unsigned long xid) {
            _op = 1;
            _htype = 1;
            _hlen = sizeof(NIC::Address);
            _xid = xid;
            memcpy(_chaddr, &mac, sizeof(NIC::Address));
            _options[0] = 53; // Code: 53 -> DHCP Message Type
            _options[1] = 1;  // Size
            _options[2] = DISCOVER;
        }
    };

    class Offer: public Packet<> {};

    class Request: public Packet<8>
    {
    public:
        Request(const NIC::Address & mac, unsigned long xid, const Address & yiaddr, const Address & siaddr) {
            _op = 1;
            _htype = 1;
            _hlen = sizeof(MAC_Address);
            _xid = xid;
            _ciaddr = yiaddr;
            _siaddr = siaddr;
            memcpy(_chaddr, &mac, sizeof(MAC_Address));
            _options[0] = 53; // Code: 53 -> DHCP Message Type
            _options[1] = 1;  // Size
            _options[2] = REQUEST;
            _options[3] = 55; // Code: 55 -> Parameter Request List
            _options[4] = 3;  // Size
            _options[5] = 1;  // Code: 1 -> Netmask
            _options[6] = 3;  // Code: 3 -> Gateway
            _options[7] = 6;  // Code: 6 -> DNS
        }
    };

    class Ack: public Packet<> {};

    class Client: private Link<UDP>
    {
    public:
        Client(const MAC_Address & mac, IP * ip);
        ~Client() { /* release() only if renew() active and the client is a thread */ }

        void renew();
        void release();

    private:
        void parse_options(Packet<> * packet, IP::Address * a, IP::Address * m, IP::Address * g, IP::Address * n);

    protected:
        unsigned long _xid;
        unsigned long _lease_time;
    };
};

__END_SYS

#endif
