// EPOS Transmission Control Protocol (RFC 793) Declarations

#ifndef __tcp_h
#define __tcp_h

#include <utility/handler.h>
#include <utility/random.h>
#include <alarm.h>
#include <condition.h>
#include <ip.h>
#include <icmp.h>
#include <udp.h> // TCP::Address == UDP_Address

__BEGIN_SYS

class TCP: private IP::Observer
{
private:
    typedef IP::Packet Packet;

public:
    static const bool connectionless = false;

    static const unsigned int RETRIES = Traits<TCP>::RETRIES;
    static const unsigned int TIMEOUT = Traits<TCP>::TIMEOUT * 1000000;
    static const unsigned int WINDOW = Traits<TCP>::WINDOW;

    typedef IP::Buffer Buffer;

    typedef UDP::Port Port;

    typedef UDP::Address Address;

    typedef Data_Observer<Buffer, unsigned long long> Observer; // Condition = Connection::id()
    typedef Data_Observed<Buffer, unsigned long long> Observed;


    class Header
    {
    public:
        static const unsigned int DO = 5; // size of TCP header in 32-bit words

        typedef unsigned char Flags;
        enum {
            FIN = 0x01,
            SYN = 0x02,
            RST = 0x04,
            PSH = 0x08,
            ACK = 0x10,
            URG = 0x20,
            ECE = 0x40,
            CWR = 0x80
        };

    public:
        Header() {}
        Header(const Port & from, const Port & to, unsigned int sequence, unsigned short window)
        : _from(htons(from)), _to(htons(to)), _sequence(htonl(sequence)), _acknowledgment(0), _data_offset(DO), _flags(0), _window(htons(window)), _checksum(0), _urgent_pointer(0) {}

        Port from() const { return ntohs(_from); }
        Port to() const { return ntohs(_to); }

        unsigned int sequence() const { return ntohl(_sequence); }
        unsigned int acknowledgment() const { return ntohl(_acknowledgment); }
        unsigned int flags() const { return _flags; }
        unsigned int window() const { return ntohs(_window); }

        unsigned short checksum() const { return ntohs(_checksum); }

        friend OStream & operator<<(OStream & db, const Header & h) {
            db << "{sp=" << hex << ntohs(h._from) << ",dp=" << ntohs(h._to)
               << ",seq=" << ntohl(h._sequence) << ",ack=" << ntohl(h._acknowledgment)
               << ",flg=" << ((h._flags & ACK) ? 'A' : '-') << ((h._flags & RST) ? 'R' : '-') << ((h._flags & SYN) ? 'S' : '-') << ((h._flags & FIN) ? 'F' : '-')
               << ",win=" << dec << ntohs(h._window) << ",chk=" << hex << ntohs(h._checksum) << dec << "}";
            return db;
        }

    protected:
        Port            _from;
        Port            _to;
        unsigned int    _sequence;
        unsigned int    _acknowledgment;
        unsigned char   _unused:4;
        unsigned char   _data_offset:4;
        unsigned char   _flags;
        unsigned short  _window;
        unsigned short  _checksum; // Pseudo header checksum (see RFC)
        unsigned short  _urgent_pointer;
    } __attribute__((packed));

    static const unsigned int MTU = IP::MTU - sizeof(Header);
    static const unsigned int MSS = IP::MFS - sizeof(Header);
    static const unsigned int HEADERS_SIZE = sizeof(IP::Header) + sizeof(Header);

    typedef unsigned char Data[MTU];

    class Segment: public Header
    {
    public:
        Segment() {}

        Header * header() { return this; }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        void sum(const IP::Address & from, const IP::Address & to, const void * data, unsigned int length);
        bool check(unsigned int length) { return IP::checksum(this, length) != 0xffff; } // FIXME

        friend Debug & operator<<(Debug & db, const Segment & m) {
            db << "{head=" << reinterpret_cast<const Header &>(m) << ",data=" << m._data << "}";
            return db;
        }

    private:
        Data _data;
    } __attribute__((packed));


    class Connection: public Header, private TCP::Observer, public TCP::Observed
    {
        friend class TCP;

    public:
        enum State {
            LISTENING,
            SYN_SENT,
            SYN_RECEIVED,
            ESTABLISHED,
            FIN_WAIT1,
            FIN_WAIT2,
            CLOSE_WAIT,
            CLOSING,
            LAST_ACK,
            TIME_WAIT,
            CLOSED
        };

        typedef void (Connection:: * State_Handler)();

    public:
        Connection(const Port & from, const Address & to)
        : Header(from, to.port(), Random::random() & 0x00ffffff, WINDOW), _peer(to.ip()), _peer_window(0), _next(ntohl(_sequence)),
          _unacknowledged(_next), _initial(_next), _state(CLOSED), _handler(&Connection::closed), _current(0), _length(0), _valid(false),
          _streaming(false), _retransmiting(false), _timeout_handler(&timeout,this), _alarm(0), _tries(0), _observer(0) {}
        ~Connection() { if(_alarm) delete _alarm; close(); }

        const volatile State & state() const { return _state; }
        const Header * header() const { return this; }

        int send(const void * data, unsigned int size);
        int receive(Buffer * buf, void * data, unsigned int size);

        const IP::Address & peer() const { return _peer; }

        unsigned long long id() const {
            unsigned long long tmp = _peer[0] << 24 | _peer[1] << 16 | _peer[2] << 8 | _peer[3];
            tmp = (tmp << 32) | ((to() << 16) | from());
            return tmp;
        }

        void attach(TCP::Observer * obs) { _observer = obs; }
        void detach(TCP::Observer * obs) { _observer = 0; }
        bool notify(unsigned long long socket, Buffer * buf) {
            if(_observer) {
                _observer->update(this, socket, buf);
                return true;
            } else
                return false;
        }

        static unsigned long long id(const Port & local, const Port & remote, const IP::Address & peer) {
            unsigned long long tmp = peer[0] << 24 | peer[1] << 16 | peer[2] << 8 | peer[3];
            tmp = (tmp << 32) | ((remote << 16) | local);
            return tmp;
        }

        friend Debug & operator<<(Debug & db, const Connection & c) {
            db << *c.header()
               << ",peer=" << c._peer << ",pwin=" << c._peer_window << ",uack=" << ntohl(c._unacknowledged) << ",stat=" << c._state;
            if(c._current)
                db << ",curr=" << c._current << " => " << *c._current << ",len=" << c._length;
            return db;
        }

    private:
        void state(const State & s) {
            _state = s;
            _handler = _handlers[s];
        }

        void update(TCP::Observed * osb, unsigned long long socket, Buffer * buf);

        // State Transition Initiators
        void listen();
        void connect();
        void close();

        // State Handlers
        void listening();
        void syn_sent();
        void syn_received();
        void established();
        void fin_wait1();
        void fin_wait2();
        void close_wait();
        void closing();
        void last_ack();
        void time_wait();
        void closed();

        void fsend(const Flags & flags);
        int dsend(const void * data, unsigned int size);

        bool check_sequence();
        void process_fin();

        static void timeout(Connection * c);
        void set_timeout(const Alarm::Microsecond & time = TIMEOUT);

    private:
        IP::Address _peer;
        unsigned short  _peer_window;   // (host endianness)
        unsigned int _next;             // next regular sequence number to be sent, it tells how far the conversation has gone (host endianness)
        unsigned int _unacknowledged;   // earliest unacknowledged sequence number sent (host endianness)
        const unsigned int _initial;    // initial sequence number (host endianness)

        volatile State _state;
        volatile State_Handler _handler;

        Condition _transition;
        static State_Handler _handlers[];

        Segment * _current;
        unsigned int _length;
        volatile bool _valid;

        // Stream stuff
        volatile bool _streaming;
        volatile bool _retransmiting;
        Condition _stream;

        // Timeout stuff
        Functor_Handler<Connection> _timeout_handler;
        Alarm * _alarm;
        volatile int _tries; // either for close() or open() calls

        TCP::Observer * _observer;
    };

public:
    TCP() {
        db<TCP>(TRC) << "TCP::TCP()" << endl;
        IP::attach(this, IP::TCP);
    }
    ~TCP() {
        db<TCP>(TRC) << "TCP::~TCP()" << endl;
        IP::detach(this, IP::TCP);
    }

    static Connection * attach(Observer * obs, const Port & from, const Address & to) {
        db<TCP>(TRC) << "TCP::attach(obs=" << obs << ",from=" << hex << from << ",to=" << to << ")" << endl;

        Connection * conn = new Connection(from, to);
        unsigned long long id = conn->id();
        conn->attach(obs);
        _observed.attach(conn, id);

        if(to)
            for(unsigned int i = 0; (i < RETRIES) && (conn->state() != Connection::ESTABLISHED); i++)
                conn->connect();
        else
            conn->listen();

        if(conn->state() != Connection::ESTABLISHED) {
            delete conn;
            conn = 0;
        }

        return conn;
    }

    static void detach(Observer * obs, Connection * conn) {
        unsigned long long id = conn->id();
        conn->detach(obs);
        delete conn;
        _observed.detach(conn, id);
    }

private:
    void update(IP::Observed * obs, IP::Protocol prot, Buffer * pool);

    unsigned short mss(Buffer * buf) {
       return buf->nic()->mtu() - sizeof(IP::Header) - sizeof(Header);
    }

private:
    static Observed _observed; // Channel protocols are singletons
};

__END_SYS

#endif
