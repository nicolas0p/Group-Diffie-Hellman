// EPOS ICMP (RFC 792) Protocol Declarations

#ifndef __icmp_h
#define __icmp_h

#include <ip.h>

__BEGIN_SYS

class ICMP: private IP::Observer
{
    friend class Network;

public:
    static const bool connectionless = true;

    // Network to be used by Communicator
    typedef IP Network;

    // Buffers received from IP
    typedef NIC::Buffer Buffer;

    // ICMP Packet Types
    typedef unsigned char Type;
    enum {
        ECHO_REPLY              = 0,
        UNREACHABLE             = 3,
        SOURCE_QUENCH           = 4,
        REDIRECT                = 5,
        ALTERNATE_ADDRESS       = 6,
        ECHO                    = 8,
        ROUTER_ADVERT           = 9,
        ROUTER_SOLIC            = 10,
        TIME_EXCEEDED           = 11,
        PARAMETER_PROBLEM       = 12,
        TIMESTAMP               = 13,
        TIMESTAMP_REPLY         = 14,
        INFO_REQUEST            = 15,
        INFO_REPLY              = 16,
        ADDRESS_MASK_REQ        = 17,
        ADDRESS_MASK_REP        = 18,
        TRACEROUTE              = 30,
        DGRAM_ERROR             = 31,
        MOBILE_HOST_REDIR       = 32,
        IPv6_WHERE_ARE_YOU      = 33,
        IPv6_I_AM_HERE          = 34,
        MOBILE_REG_REQ          = 35,
        MOBILE_REG_REP          = 36,
        DOMAIN_NAME_REQ         = 37,
        DOMAIN_NAME_REP         = 38,
        SKIP                    = 39
    };

    // ICMP Packet Codes
    typedef unsigned char Code;
    enum {
        NETWORK_UNREACHABLE     = 0,
        HOST_UNREACHABLE        = 1,
        PROTOCOL_UNREACHABLE    = 2,
        PORT_UNREACHABLE        = 3,
        FRAGMENTATION_NEEDED    = 4,
        ROUTE_FAILED            = 5,
        NETWORK_UNKNOWN         = 6,
        HOST_UNKNOWN            = 7,
        HOST_ISOLATED           = 8,
        NETWORK_PROHIBITED      = 9,
        HOST_PROHIBITED         = 10,
        NETWORK_TOS_UNREACH     = 11,
        HOST_TOS_UNREACH        = 12,
        ADMIN_PROHIBITED        = 13,
        PRECEDENCE_VIOLATION    = 14,
        PRECEDENCE_CUTOFF       = 15
    };

    // ICMP addresses are ordinary IP addresses, but Type will be used by Communicator as local address
    class Address: public IP::Address
    {
    public:
        typedef Type Local;

    public:
        Address() {}
        Address(const IP::Address & ip): IP::Address(ip) {}

        Local local() const { return 0; }
    };

    typedef Data_Observer<Buffer, Type> Observer;
    typedef Data_Observed<Buffer, Type> Observed;

    class Header
    {
    public:
        Header() {}
        Header(const Type & type, const Code & code): _type(type), _code(code) {}
        Header(const Type & type, const Code & code, unsigned short id, unsigned short seq):
            _type(type), _code(code), _checksum(0), _id(htons(id)), _sequence(htons(seq)) {}

        Type & type() { return _type; }
        Code & code() { return _code; }
        unsigned short checksum() { return _checksum; }

        unsigned short id() { return htons(_id); }
        void id(unsigned short id) { _id = htons(id); }

        unsigned short sequence() { return htons(_sequence); }
        void sequence(unsigned short seq) { _sequence = htons(seq); }

        const Address & ip();
        void ip (const Address & ip);

        friend Debug & operator<<(Debug & db, const Header & h) {
            db << "{typ=" << h._type
               << ",cod=" << h._code
               << ",chk=" << hex << h._checksum << dec
               << ",id=" << h._id
               << ",seq=" << h._sequence
               << "}";
            return db;
        }

    protected:
        unsigned char  _type;
        unsigned char  _code;
        unsigned short _checksum;
        unsigned short _id;
        unsigned short _sequence;
    } __attribute__((packed));


    // ICMP Packet
    static const unsigned int MTU = 56; // to make a traditional 64-byte packet
    static const unsigned int HEADERS_SIZE = sizeof(IP::Header) + sizeof(Header);

    typedef unsigned char Data[MTU];

    class Packet: public Header
    {
    public:
        Packet(){}
        Packet(const Type & type, const Code & code): Header(type, code) {}
        Packet(const Type & type, const Code & code, unsigned short id, unsigned short seq): Header(type, code, id, seq) {}

        Header * header() { return this; }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        void sum() { _checksum = 0; _checksum = htons(IP::checksum(reinterpret_cast<unsigned char *>(this), sizeof(Packet))); }
        bool check() { return (IP::checksum(reinterpret_cast<unsigned char *>(this), sizeof(Packet)) != 0xffff); }

        friend Debug & operator<<(Debug & db, const Packet & p) {
            db << "{head=" << reinterpret_cast<const Header &>(p) << ",data=" << p._data << "}";
            return db;
        }

    private:
        Data _data;
    } __attribute__((packed));

    typedef Packet PDU;

protected:
    ICMP() {
        db<ICMP>(TRC) << "ICMP::ICMP()" << endl;
        IP::attach(this, IP::ICMP);
    }

public:
    ~ICMP() {
        db<ICMP>(TRC) << "ICMP::ICMP()" << endl;
        IP::detach(this, IP::ICMP);
    }

    static int send(const Address::Local & from, const Address & to, const void * data, unsigned int size);
    static int receive(Buffer * buf, Address * from, void * data, unsigned int size);

    static void attach(Observer * obs, const Type & type) { _observed.attach(obs, type); }
    static void detach(Observer * obs, const Type & type) { _observed.detach(obs, type); }
    static bool notify(const Type & type, Buffer * buf) { return _observed.notify(type, buf); }

private:
    void update(IP::Observed * ip, IP::Protocol prot, Buffer * buf);

    static Observed _observed; // Channel protocols are singletons
};

__END_SYS

#endif
