// EPOS IP Protocol Declarations

#ifndef __ip_h
#define __ip_h

#include <utility/bitmap.h>
#include <nic.h>
#include <system.h>
#include <arp.h>

__BEGIN_SYS

class IP: private NIC::Observer
{
    friend class System;
    template<int unit> friend void call_init();

public:
    static const unsigned int TIMEOUT = Traits<IP>::TIMEOUT * 1000000;

    // IP Protocol Id
    static const unsigned int PROTOCOL = NIC::IP;

    // Addresses
    typedef NIC::Address MAC_Address;
    typedef NIC_Common::Address<4> Address;

    // RFC 1700 Protocols
    typedef unsigned char Protocol;
    enum {
        ICMP    = 0x01,
        TCP     = 0x06,
        UDP     = 0x11,
        RDP     = 0x1b,
        TTP     = 0x54
    };

    // Buffers received by the NIC, eventually linked into a list
    typedef NIC::Buffer Buffer;

    // IP and NIC observer/d
    typedef Data_Observer<Buffer, Protocol> Observer;
    typedef Data_Observed<Buffer, Protocol> Observed;

    // IP Header
    class Header
    {
    public:
        static const unsigned int VER = 4;
        static const unsigned int IHL = 5; // 20 bytes
        static const unsigned int TOS = 0;
        static const unsigned int TTL = Traits<IP>::TTL;

        // Flags
        enum {
            MF = 1, // More Fragments
            DF = 2  // Don't Fragment
        };

    public:
        Header() {}
        Header(const Address & from, const Address & to, const Protocol & prot, unsigned int size) :
            _ihl(IHL), _version(VER), _tos(TOS), _length(htons(size)), _id(htons(_next_id++)),
            _offset(0), _flags(0), _ttl(TTL), _protocol(prot), _checksum(0), _from(from), _to(to) {}

        unsigned short length() const { return ntohs(_length); }
        void length(unsigned short length) { _length = htons(length); }

        unsigned short id() const { return ntohs(_id); }

        // Offsets in bytes that are converted to 8-byte unities
        unsigned short offset() const { return (ntohs((_flags << 13) | _offset) & 0x1fff) << 3; }
        void offset(unsigned short off) {
            unsigned short tmp = htons((flags() << 13) | (off >> 3));
            _offset = tmp & 0x1fff;
            _flags  = tmp >> 13;
        }

        unsigned short flags() const { return ntohs((_flags << 13) | _offset) >> 13; }
        void flags(unsigned short flg) {
            unsigned short tmp = htons((flg << 13) | (offset() >> 3));
            _offset = tmp & 0x1fff;
            _flags  = tmp >> 13;
        }

        unsigned char ttl() { return _ttl; }

        const Protocol & protocol() const { return _protocol; }

        unsigned short checksum() const { return ntohs(_checksum); }

        void sum() { _checksum = 0; _checksum = htons(IP::checksum(reinterpret_cast<unsigned char *>(this), _ihl * 4)); }
        bool check() { return (IP::checksum(reinterpret_cast<unsigned char *>(this), _ihl * 4) != 0xffff); }

        const Address & from() const { return _from; }
        void from(const Address & from){ _from = from; }

        const Address & to() const { return _to; }
        void to(const Address & to){ _to = to; }

        friend Debug & operator<<(Debug & db, const Header & h) {
            db << "{ver=" << h._version
               << ",ihl=" << h._ihl
               << ",tos=" << h._tos
               << ",len=" << h.length()
               << ",id="  << h.id()
               << ",off=" << h.offset()
               << ",flg=" << ((h.flags() & DF) ? 'D' : '-') << ((h.flags() & MF) ? 'M' : '-')
               << ",ttl=" << h._ttl
               << ",pro=" << h._protocol
               << ",chk=" << hex << h._checksum << dec
               << ",from=" << h._from
               << ",to=" << h._to
               << "}";
            return db;
        }

    private:
        unsigned char   _ihl:4;         // IP Header Length (in 32-bit words)
        unsigned char   _version:4;     // IP Version
        unsigned char    _tos;          // Type Of Service (not used -> 0)
        unsigned short  _length;        // Length of datagram in bytes (header + data)
        unsigned short  _id;            // Datagram id
        unsigned short  _offset:13;     // Fragment offset (x 8 bytes)
        unsigned short  _flags:3;       // Flags (DF, MF)
        unsigned char   _ttl;           // Time To Live
        Protocol        _protocol;      // Payload Protocol (RFC 1700)
        unsigned short  _checksum;      // Header checksum (RFC 1071)
        Address         _from;          // Source IP address
        Address         _to;            // Destination IP address

        static unsigned short _next_id;
    } __attribute__((packed));


    // IP Pseudo Header for checksum calculations
    class Pseudo_Header
    {
    public:
        Pseudo_Header(const Address & from, const Address & to, const Protocol & prot, unsigned int size) :
            _from(from), _to(to), _zero(0), _protocol(prot), _length(htons(size)) {};

    private:
        IP::Address _from;
        IP::Address _to;
        unsigned char _zero;
        unsigned char _protocol;
        unsigned short _length;
    };


    // IP Packet
    static const unsigned int MFS = (sizeof(NIC::Data) - sizeof(Header)) & (~7u); // Maximum Fragment Size (1480 for Ethernet). Must be multiple of 8, since offset in Header ignores the 3 LSbits.
    static const unsigned int MTU = 65535 - sizeof(Header);
    typedef unsigned char Data[MTU];

    class Packet: public Header
    {
    public:
        Packet() {}

        Header * header() { return this; }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        friend Debug & operator<<(Debug & db, const Packet & p) {
            db << "{head=" << reinterpret_cast<const Header &>(p) << ",data=" << p._data << "}";
            return db;
        }

    private:
        Data _data;
    } __attribute__((packed));

    typedef Packet PDU;

private:
    // Fragment key = f(from, id) = (from & ~_netmask) << 16 | id (fragmentation can only happen on localnet)
    typedef unsigned long Key;

    // List to hold received Buffers containing datagram fragments
    class Fragmented;
    typedef Simple_Hash<Fragmented, Traits<Build>::NODES, Key> Reassembling;

    class Fragmented
    {
        friend class IP;

    private:
        static const unsigned int MAX_FRAGMENTS = (MTU + MFS - 1) / MFS; // 45 for Ethernet
        typedef Reassembling::Element Element;

    public:
        Fragmented(const Key & key): _frags(MAX_FRAGMENTS), _handler(&timeout, this), _alarm(TIMEOUT, &_handler), _link(this, key) {}

        void insert(Buffer * buf) {
            Packet * packet = buf->frame()->data<Packet>();

            if(_bitmap.set(packet->offset() / MFS)) // if not a dup, then insert
                _list.insert(buf->link());

            if(!(packet->flags() & Header::MF))
                _frags = (packet->offset() + MFS) / MFS;

            db<IP>(TRC) << "IP::Fragmented::insert(frags=" << _frags << ",buf=" << buf << ") => " << *packet << endl;
        }

        bool reassembled() const { return _bitmap.full(_frags); }

        void reorder();

        Buffer * pool() { return _list.head()->object(); }

        Element * link() { return &_link; }

    private:
        static void timeout(Fragmented * frag) {
            frag->pool()->nic()->free(frag->pool());
            delete frag;
        }

    private:
        unsigned int _frags;
        Bitmap<MAX_FRAGMENTS> _bitmap;
        Buffer::List _list;
        Functor_Handler<Fragmented> _handler;
        Alarm _alarm;
        Element _link;
    };


    class Router;

    class Route
    {
        friend class Router;

    private:
        typedef Simple_List<Route> Table;
        typedef Table::Element Element;

    public:
        Route(NIC * nic, IP * ip, ARP<NIC, IP> * arp, const Address & d, const Address & g, const Address & m, unsigned int t = 0, unsigned int w = 0):
            _destination(d), _gateway(g), _genmask(m), _flags(t), _metric(w), _nic(nic), _ip(ip), _arp(arp), _link(this) {}

        const Address & gateway() const { return _gateway; }
        NIC * nic() { return _nic; }
        IP * ip() { return _ip; }
        ARP<NIC, IP> * arp() { return _arp; }

        friend Debug & operator<<(Debug & db, const Route & r) {
            db << "{d=" << r._destination
                << ",g=" << r._gateway
                << ",m=" << r._genmask
                << ",f=" << r._flags
                << ",w=" << r._metric
                << ",nic=" << r._nic
                << ",ip=" << r._ip
                << "}";
            return db;
        }

    private:
        Address _destination;
        Address _gateway;
        Address _genmask;
        unsigned int _flags;
        unsigned int _metric;
        NIC * _nic;
        IP * _ip;
        ARP<NIC, IP> * _arp;

        Element _link;
    };


    class Router
    {
    private:
        typedef Route::Table::Element Element;

    public:
        void insert(NIC * nic, IP * ip, ARP<NIC, IP> * arp, const Address & d, const Address & g, const Address & m, unsigned int t = 0, unsigned int w = 0) {
            Route * route = new (SYSTEM) Route(nic, ip, arp, d, g, m, t, w);

            db<IP>(TRC) << "IP::Router::insert() => " << *route << endl;

            _table.insert(&route->_link);
        }

        void remove(const Address & to) {
            db<IP>(TRC) << "IP::Router::remove(to=" << to << ")" << endl;

            Element * e = _table.head();
            for(; e && ((to & e->object()->_genmask) != e->object()->_destination); e = e->next());
            if(e) {
                db<IP>(INF) << "IP::Router::remove: removing and deleting " << *e->object() << endl;

                _table.remove(e);
                delete e->object();
            }
        }

        Route * search(Address to) {
            // Assume default (0.0.0.0) to have genmask 0.0.0.0 and be the last route in table
            // Assume all routes to be network routes (i.g. flags "G")
            db<IP>(TRC) << "IP::Route::search(to=" << to << ")" << endl;

            Element * e = _table.head();
            for(; e && ((to & e->object()->_genmask) != e->object()->_destination); e = e->next());
            if(e) {
                db<IP>(INF) << "IP::Route::search: found route to " << to << " => " << *e->object() << endl;
                return e->object();
            } else
                return 0;
        }

    private:
        Route::Table _table;
    };


protected:
    template<typename Config>
    IP(const NIC & nic, const Config & config);

public:
    ~IP();

    void reconfigure(const Address & a, const Address & m, const Address & g) {
        _address = a;
        _netmask = m;
        _broadcast = (a & m) | ~m;
        _gateway = g;
    }

    NIC * nic() { return &_nic; }
    ARP<NIC, IP> * arp() { return &_arp; }
    Router * router() { return &_router; }

    const Address & address() const { return _address; }
    const Address & broadcast() const { return _broadcast; }
    const Address & gateway() const { return _gateway; }
    const Address & netmask() const { return _netmask; }

    static IP * get_by_nic(unsigned int unit) {
        if(unit >= Traits<NIC>::UNITS) {
            db<IP>(WRN) << "IP::get_by_nic: requested unit (" << unit << ") does not exist!" << endl;
            return 0;
        } else
            return _networks[unit];
    }

    static Route * route(const Address & to) { return _router.search(to); }

    static Buffer * alloc(const Address & to, const Protocol & prot, unsigned int once, unsigned int payload);
    static int send(Buffer * buf);

    static const unsigned int mtu() { return MTU; }

    static unsigned short checksum(const void * data, unsigned int size);

    static void attach(Observer * obs, const Protocol & prot) { _observed.attach(obs, prot); }
    static void detach(Observer * obs, const Protocol & prot) { _observed.detach(obs, prot); }

    friend Debug & operator<<(Debug & db, const IP & ip) {
        db << "{a=" << ip._address
           << ",m=" << ip._netmask
           << ",b=" << ip._broadcast
           << ",g=" << ip._gateway
           << ",nic=" << &ip._nic
           << "}";
        return db;
    }

private:
    void config_by_mac() { _address[sizeof(Address) -1] = _nic.address()[sizeof(MAC_Address) - 1]; }
    void config_by_info();
    void config_by_rarp();
    void config_by_dhcp();

    void update(NIC::Observed * obs, NIC::Protocol prot, Buffer * buf);

    static bool notify(const Protocol & prot, Buffer * buf) { return _observed.notify(prot, buf); }

    template<unsigned int UNIT>
    static void init(const NIC & nic);

protected:
    NIC _nic;
    ARP<NIC, IP> _arp;

    Address _address;
    Address _netmask;
    Address _broadcast;
    Address _gateway;

    static IP * _networks[Traits<NIC>::UNITS];
    static Router _router;
    static Reassembling _reassembling;
    static Observed _observed; // shared by all IP instances, so the default for binding on a port is for all IPs
};

__END_SYS

#include <icmp.h>

#endif
