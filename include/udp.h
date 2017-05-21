// EPOS UDP (RFC 768) Protocol Declarations

#ifndef __udp_h
#define __udp_h

#include <ip.h>

__BEGIN_SYS

class UDP: private IP::Observer
{
    friend class Network;
    friend class TCP;

private:
    typedef IP::Packet Packet;

public:
    static const bool connectionless = true;

    typedef IP::Buffer Buffer;

    typedef unsigned short Port;

    class Address
    {
    public:
        typedef Port Local;

        enum Null { NULL = IP::Address::NULL };

    public:
        Address() {}
        Address(const Null &): _ip(NULL), _port(0) {}
        Address(const IP::Address & ip, const Port & port): _ip(ip), _port(port) {}
        Address(const char * addr): _ip(addr) { // a.b.c.d:port
            char * token = strchr(addr, ':');
            _port = token ? atol(++token) : 0;
        }

        const IP::Address & ip() const { return _ip; }
        const Port & port() const { return _port; }
        const Local & local() const { return _port; }

        operator bool() const {
            return (_ip || _port);
        }

        bool operator==(const Address & a) {
            return (_ip == a._ip) && (_port == a._port);
        }

        friend OStream & operator<<(OStream & db, const Address & a) {
            db << a._ip << ":" << hex << a._port;
            return db;
        }

    private:
        IP::Address _ip;
        Port _port;
    };

    typedef Data_Observer<Buffer, Port> Observer;
    typedef Data_Observed<Buffer, Port> Observed;


    class Header
    {
    public:
        Header() {}
        Header(const Port & from, const Port & to, unsigned int size):
            _from(htons(from)), _to(htons(to)), _length(htons((size > sizeof(Data) ? sizeof(Data) : size) + sizeof(Header))) {}

        Port from() const { return ntohs(_from); }
        Port to() const { return ntohs(_to); }

        unsigned short length() const { return ntohs(_length); }

        unsigned short checksum() const { return ntohs(_checksum); }

        friend OStream & operator<<(OStream & db, const Header & h) {
            db << "{sp=" << ntohs(h._from) << ",dp=" << ntohs(h._to)
               << ",len=" << ntohs(h._length) << ",chk=" << hex << ntohs(h._checksum) << dec << "}";
            return db;
        }

    protected:
        Port _from;
        Port _to;
        unsigned short _length;   // Length of datagram (header + data) in bytes
        unsigned short _checksum; // Pseudo header checksum (see RFC)
    } __attribute__((packed));

    static const unsigned int MTU = IP::MTU - sizeof(Header);
    static const unsigned int HEADERS_SIZE = sizeof(IP::Header) + sizeof(Header);

    typedef unsigned char Data[MTU];

    class Message: public Header
    {
    public:
        Message() {}

        Header * header() { return this; }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        void sum_header(const IP::Address & from, const IP::Address & to);
        void sum_data(const void * data, unsigned int size);
        void sum_trailer();
        bool check() { return Traits<UDP>::checksum ? (IP::checksum(this, length()) != 0xffff) : true; }

        friend Debug & operator<<(Debug & db, const Message & m) {
            db << "{head=" << reinterpret_cast<const Header &>(m) << ",data=" << m._data << "}";
            return db;
        }

    private:
        Data _data;
    } __attribute__((packed));

protected:
    UDP() {
        db<UDP>(TRC) << "UDP::UDP()" << endl;
        IP::attach(this, IP::UDP);
    }

public:
    ~UDP() {
        db<UDP>(TRC) << "UDP::~UDP()" << endl;
        IP::detach(this, IP::UDP);
    }

    static int send(const Port & from, const Address & to, const void * data, unsigned int size);
    static int receive(Buffer * buf, void * data, unsigned int size);

    static void attach(Observer * obs, const Port & port) { _observed.attach(obs, port); }
    static void detach(Observer * obs, const Port & port) { _observed.detach(obs, port); }
    static bool notify(const Port & port, Buffer * buf) { return _observed.notify(port, buf); }

private:
    void update(IP::Observed * obs, IP::Protocol prot, Buffer * buf);

    static Observed _observed; // Channel protocols are singletons
};

__END_SYS

#endif
