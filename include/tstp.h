// EPOS Trustful SpaceTime Protocol Declarations

#include <ieee802_15_4.h>

#ifndef __tstp_common_h
#define __tstp_common_h

#include <utility/geometry.h>
#include <rtc.h>

__BEGIN_SYS

class TSTP_Common: public IEEE802_15_4
{
protected:
    static const unsigned int RADIO_RANGE = 1700; // Approximated radio range of nodes, in centimeters
    static const bool drop_expired = true;

public:
    static const unsigned int PAN = 10; // Nodes
    static const unsigned int SAN = 100; // Nodes
    static const unsigned int LAN = 10000; // Nodes
    static const unsigned int NODES = Traits<Build>::NODES;

    // Version
    // This field is packed first and matches the Frame Type field in the Frame Control in IEEE 802.15.4 MAC.
    // A version number above 4 renders TSTP into the reserved frame type zone and should avoid interference.
    enum Version {
        V0 = 4
    };

    // MAC definitions
    typedef CPU::Reg16 Frame_ID;
    typedef CPU::Reg32 Hint;
    typedef NIC_Common::CRC16 CRC;
    typedef CPU::Reg16 MF_Count;

    // Packet Types
    typedef unsigned char Type;
    enum {
        INTEREST  = 0,
        RESPONSE  = 1,
        COMMAND   = 2,
        CONTROL   = 3
    };

    // Control packet subtypes
    typedef unsigned char Subtype;
    enum {
        DH_REQUEST   = 0,
        DH_RESPONSE  = 1,
        AUTH_REQUEST = 2,
        AUTH_GRANTED = 3,
        REPORT = 4,
        KEEP_ALIVE = 5,
        EPOCH = 6,
    };

    // Scale for local network's geographic coordinates
    enum Scale {
        CMx50_8  = 0,
        CM_16    = 1,
        CMx25_16 = 2,
        CM_32    = 3
    };
    static const Scale SCALE = (NODES <= PAN) ? CMx50_8 : (NODES <= SAN) ? CM_16 : (NODES <= LAN) ? CMx25_16 : CM_32;

    // Time
    typedef RTC::Microsecond Microsecond;
    typedef unsigned long long Time;
    typedef long Time_Offset;

    // Geographic Coordinates
    template<Scale S>
    struct _Coordinates: public Point<
                         typename IF<S == CMx50_8, char,
                         typename IF<S == CM_16, short,
                         typename IF<S == CMx25_16, short,
                         typename IF<S == CM_32, int, void
                         >::Result>::Result>::Result>::Result, 3>
    {
        typedef typename IF<S == CMx50_8, char,
                typename IF<S == CM_16, short,
                typename IF<S == CMx25_16, short,
                typename IF<S == CM_32, int, void
                    >::Result>::Result>::Result>::Result Number;

        _Coordinates(Number x = 0, Number y = 0, Number z = 0): Point<Number, 3>(x, y, z) {}
        _Coordinates(const Point<Number, 3> & p): Point<Number, 3>(p) {}
    } __attribute__((packed));
    typedef _Coordinates<SCALE> Coordinates;
    typedef _Coordinates<CM_32> Global_Coordinates;

    // Geographic Region in a time interval (not exactly Spacetime, but ...)
    template<Scale S>
    struct _Region: public Sphere<typename _Coordinates<S>::Number>
    {
        typedef typename _Coordinates<S>::Number Number;
        typedef Sphere<Number> Space;
        typedef typename Space::Center Center;

        _Region(const Center & c, const Number & r, const Time & _t0, const Time & _t1)
        : Space(c, r), t0(_t0), t1(_t1) {}

        bool contains(const Center & c, const Time & t) const {
            return ((t >= t0) && (t <= t1)) && Space::contains(c);
        }

        friend Debug & operator<<(Debug & db, const _Region & r) {
            db << "{" << reinterpret_cast<const Space &>(r) << ",t0=" << r.t0 << ",t1=" << r.t1 << "}";
            return db;
        }

        friend OStream & operator<<(OStream & db, const _Region & r) {
            db << "{" << reinterpret_cast<const Space &>(r) << ",t0=" << r.t0 << ",t1=" << r.t1 << "}";
            return db;
        }

        Time t0;
        Time t1;
    } __attribute__((packed));
    typedef _Region<SCALE> Region;

    // MAC Preamble Microframe
    class Microframe
    {
        // Format
        // Bit 0            1      12   24     56    72
        //     +------------+-------+----+------+-----+
        //     | all listen | count | id | hint | crc |
        //     +------------+-------+----+------+-----+
        // Bits       1        11     12    32    16
    public:
        Microframe() {}

        Microframe(bool all_listen, const Frame_ID & id, const MF_Count & count, const Hint & hint = 0)
        : _al_count_id_hintl(htolel(all_listen | ((count & 0x07ff) << 1) | ((id & 0x0fff) << 12) | ((hint & 0x0ff) << 24))), _hinth_crcl(htolel((hint & 0xffffff00) >> 8)), _crch(0) {}

        MF_Count count() const { return (letohl(_al_count_id_hintl) & 0x0ffe) >> 1; }

        MF_Count dec_count() {
            MF_Count c = count();
            count(c - 1);
            return c;
        }

        void count(const MF_Count & c) {
            _al_count_id_hintl = htolel((letohl(_al_count_id_hintl) & ~0x0ffe) | (c << 1));
        }

        Frame_ID id() const { return ((letohl(_al_count_id_hintl) & 0x00fff000) >> 12); }

        void id(const Frame_ID & id) {
            _al_count_id_hintl = htolel((letohl(_al_count_id_hintl) & ~0x00fff000) | ((id & 0x00000fff) << 12));
        }

        void all_listen(bool all_listen) {
            if(all_listen)
                _al_count_id_hintl = htolel((letohl(_al_count_id_hintl) | 0x01));
            else
                _al_count_id_hintl = htolel((letohl(_al_count_id_hintl) & ~0x01));
        }

        bool all_listen() const { return letohl(_al_count_id_hintl & 0x01); }

        Hint hint() const { return ((letohl(_al_count_id_hintl) & 0xff000000) >> 24) + ((letohl(_hinth_crcl) & 0x00ffffff) << 8); }
        void hint(const Hint & h) {
            _al_count_id_hintl = htolel((letohl(_al_count_id_hintl) & ~0xff000000) | ((h & 0x0ff) << 24));
            _hinth_crcl = htolel((letohl(_hinth_crcl) & ~0x00ffffff) | ((static_cast<unsigned int>(h & 0xffffff00)) >> 8));
        }

        friend Debug & operator<<(Debug & db, const Microframe & m) {
            db << "{al=" << m.all_listen() << ",c=" << m.count() << ",id=" << m.id() << ",h=" << m.hint() << "}";
            return db;
        }
        friend OStream & operator<<(OStream & db, const Microframe & m) {
            db << "{al=" << m.all_listen() << ",c=" << m.count() << ",id=" << m.id() << ",h=" << m.hint()<< "}";
            return db;
        }

    private:
        unsigned int _al_count_id_hintl; // All listen, Count, ID, Hint LSB
        unsigned int _hinth_crcl; // Hint MSBs, CRC LSB
        unsigned char _crch; // CRC MSB
    } __attribute__((packed));

    // Packet Header
    template<Scale S>
    class _Header
    {
        // Format
        // Bit 0      3    5  6    0                0         0         0         0         0         0         0         0
        //     +------+----+--+----+----------------+--- ~ ---+--- ~ ---+--- ~ ---+--- ~ ---+--- ~ ---+--- ~ ---+--- ~ ---+--- ~ ---+
        //     | ver  |type|tr|scal|   confidence   |   o.t   |   o.x   |   o.y   |   o.z   |   l.t   |   l.x   |   l.y   |   l.z   |
        //     +------+----+--+----+----------------+--- ~ ---+--- ~ ---+--- ~ ---+--- ~ ---+--- ~ ---+--- ~ ---+--- ~ ---+--- ~ ---+
        // Bits          8                  8            64     8/16/32   8/16/32   8/16/32      64     8/16/32   8/16/32   8/16/32

    public:
        _Header(const Type & t, bool tr = false, unsigned char c = 0, const Time & ot = 0, const Coordinates & o = 0, const Coordinates & l = 0, const Version & v = V0)
        : _config((S & 0x03) << 6 | tr << 5 | (t & 0x03) << 3 | (v & 0x07)), _confidence(c), _time(ot), _origin(o), _last_hop(l) {}

        Version version() const { return static_cast<Version>(_config & 0x07); }
        void version(const Version & v) { _config = (_config & 0xf8) | (v & 0x07); }

        Type type() const { return static_cast<Type>((_config >> 3) & 0x03); }
        void type(const Type & t) { _config = (_config & 0xe7) | ((t & 0x03) << 3); }

        bool time_request() const { return (_config >> 5) & 0x01; }
        void time_request(bool tr) { _config = (_config & 0xdf) | (tr << 5); }

        Scale scale() const { return static_cast<Scale>((_config >> 6) & 0x03); }
        void scale(const Scale & s) { _config = (_config & 0x3f) | (s & 0x03) << 6; }

        const Coordinates & origin() const { return _origin; }
        void origin(const Coordinates & c) { _origin = c; }

        const Coordinates & last_hop() const { return _last_hop; }
        void last_hop(const Coordinates & c) { _last_hop = c; }

        Time time() const { return _time; }
        void time(const Time & t) { _time = t; }

        Time last_hop_time() const { return _last_hop_time; }
        void last_hop_time(const Time & t) { _last_hop_time = t; }

        unsigned char confidence() const { return _confidence; }
        void confidence(unsigned char c) { _confidence = c; }

        friend Debug & operator<<(Debug & db, const _Header & h) {
            db << "{v=" << h.version() - V0 << ",t=" << ((h.type() == INTEREST) ? 'I' :  (h.type() == RESPONSE) ? 'R' : (h.type() == COMMAND) ? 'C' : 'T') << ",co=" << h._confidence << ",tr=" << h.time_request() << ",s=" << h.scale() << ",ot=" << h._time << ",o=" << h._origin << ",lt=" << h._last_hop_time << ",l=" << h._last_hop << "}";
            return db;
        }
        friend OStream & operator<<(OStream & db, const _Header & h) {
            db << "{v=" << h.version() - V0 << ",t=" << ((h.type() == INTEREST) ? 'I' :  (h.type() == RESPONSE) ? 'R' : (h.type() == COMMAND) ? 'C' : 'T') << ",co=" << h._confidence << ",tr=" << h.time_request() << ",s=" << h.scale() << ",ot=" << h._time << ",o=" << h._origin << ",lt=" << h._last_hop_time << ",l=" << h._last_hop << "}";
            return db;
        }

    protected:
        unsigned char _config;
        unsigned char _confidence;
        Time _time;
        Coordinates _origin;
        Time _last_hop_time; // TODO: change to Time_Offset
        Coordinates _last_hop;
    } __attribute__((packed));
    typedef _Header<SCALE> Header;

    // Control Message extended Header
    class Control: public Header
    {
    protected:
        Control(const Subtype & st, bool tr = false, unsigned char c = 0, const Time & ot = 0, const Coordinates & o = 0, const Coordinates & l = 0, const Version & v = V0)
        : Header(CONTROL, tr, c, ot, o, l, v), _subtype(st) {}

    public:
        const Subtype & subtype() const { return _subtype; }

        friend Debug & operator<<(Debug & db, const Control & m) {
            db << reinterpret_cast<const Header &>(m) << ",st=" << m._subtype;
            return db;
        }

    private:
        Subtype _subtype;
    } __attribute__((packed));

    // Frame
    template<Scale S>
    class _Frame: public Header
    {
    public:
        static const unsigned int MTU = TSTP_Common::MTU;
        typedef unsigned char Data[MTU];

    public:
        _Frame() {}
        _Frame(const Type & type, const Address & src, const Address & dst): Header(type) {} // Just for NIC compatibility
        _Frame(const Type & type, const Address & src, const Address & dst, const void * data, unsigned int size): Header(type) { memcpy(_data, data, size); }

        Header * header() { return this; }

        Reg8 length() const { return MTU; } // Fixme: placeholder

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        friend Debug & operator<<(Debug & db, const _Frame & p) {
            db << "{h=" << reinterpret_cast<const Header &>(p) << ",d=" << p._data << "}";
            return db;
        }

    private:
        Data _data;
    } __attribute__((packed, may_alias));
    typedef _Frame<SCALE> Frame;

    typedef Frame PDU;


    // TSTP encodes SI Units similarly to IEEE 1451 TEDs
    class Unit
    {
    public:
        // Formats
        // Bit       31                                 16                                     0
        //         +--+----------------------------------+-------------------------------------+
        // Digital |0 | type                             | dev                                 |
        //         +--+----------------------------------+-------------------------------------+

        // Bit       31   29   27     24     21     18     15     12      9      6      3      0
        //         +--+----+----+------+------+------+------+------+------+------+------+------+
        // SI      |1 |NUM |MOD |sr+4  |rad+4 |m+4   |kg+4  |s+4   |A+4   |K+4   |mol+4 |cd+4  |
        //         +--+----+----+------+------+------+------+------+------+------+------+------+
        // Bits     1   2    2     3      3      3      3      3      3      3      3      3


        // Valid values for field SI
        enum {
            DIGITAL = 0 << 31, // The Unit is plain digital data. Subsequent 15 bits designate the data type. Lower 16 bits are application-specific, usually a device selector.
            SI      = 1 << 31  // The Unit is SI. Remaining bits are interpreted as specified here.
        };

        // Valid values for field NUM
        enum {
            I32 = 0 << 29, // Value is an integral number stored in the 32 last significant bits of a 32-bit big-endian integer.
            I64 = 1 << 29, // Value is an integral number stored in the 64 last significant bits of a 64-bit big-endian integer.
            F32 = 2 << 29, // Value is a real number stored as an IEEE 754 binary32 big-endian floating point.
            D64 = 3 << 29, // Value is a real number stored as an IEEE 754 binary64 big-endian double precision floating point.
            NUM = D64      // AND mask to select NUM bits
        };

        // Valid values for field MOD
        enum {
            DIR     = 0 << 27, // Unit is described by the product of SI base units raised to the powers recorded in the remaining fields.
            DIV     = 1 << 27, // Unit is U/U, where U is described by the product SI base units raised to the powers recorded in the remaining fields.
            LOG     = 2 << 27, // Unit is log_e(U), where U is described by the product of SI base units raised to the powers recorded in the remaining fields.
            LOG_DIV = 3 << 27, // Unit is log_e(U/U), where U is described by the product of SI base units raised to the powers recorded in the remaining fields.
            MOD = D64          // AND mask to select MOD bits
        };

        // Masks to select the SI units
        enum {
            SR      = 7 << 24,
            RAD     = 7 << 21,
            M       = 7 << 18,
            KG      = 7 << 15,
            S       = 7 << 12,
            A       = 7 <<  9,
            K       = 7 <<  6,
            MOL     = 7 <<  3,
            CD      = 7 <<  0
        };

        // Typical SI Quantities
        enum Quantity {
             //                        si      | mod     | sr            | rad           |  m            |  kg           |  s            |  A            |  K            |  mol          |  cd
             Length                  = 1 << 31 | 0 << 27 | (4 + 0) << 24 | (4 + 0) << 21 | (4 + 1) << 18 | (4 + 0) << 15 | (4 + 0) << 12 | (4 + 0) << 9  | (4 + 0) << 6  | (4 + 0) << 3  | (4 + 0),
             Mass                    = 1 << 31 | 0 << 27 | (4 + 0) << 24 | (4 + 0) << 21 | (4 + 0) << 18 | (4 + 1) << 15 | (4 + 0) << 12 | (4 + 0) << 9  | (4 + 0) << 6  | (4 + 0) << 3  | (4 + 0),
             Time                    = 1 << 31 | 0 << 27 | (4 + 0) << 24 | (4 + 0) << 21 | (4 + 0) << 18 | (4 + 0) << 15 | (4 + 1) << 12 | (4 + 0) << 9  | (4 + 0) << 6  | (4 + 0) << 3  | (4 + 0),
             Current                 = 1 << 31 | 0 << 27 | (4 + 0) << 24 | (4 + 0) << 21 | (4 + 0) << 18 | (4 + 0) << 15 | (4 + 0) << 12 | (4 + 1) << 9  | (4 + 0) << 6  | (4 + 0) << 3  | (4 + 0),
             Electric_Current        = Current,
             Temperature             = 1 << 31 | 0 << 27 | (4 + 0) << 24 | (4 + 0) << 21 | (4 + 0) << 18 | (4 + 0) << 15 | (4 + 0) << 12 | (4 + 0) << 9  | (4 + 1) << 6  | (4 + 0) << 3  | (4 + 0),
             Amount_of_Substance     = 1 << 31 | 0 << 27 | (4 + 0) << 24 | (4 + 0) << 21 | (4 + 0) << 18 | (4 + 0) << 15 | (4 + 0) << 12 | (4 + 0) << 9  | (4 + 0) << 6  | (4 + 1) << 3  | (4 + 0),
             Luminous_Intensity      = 1 << 31 | 0 << 27 | (4 + 0) << 24 | (4 + 0) << 21 | (4 + 0) << 18 | (4 + 0) << 15 | (4 + 0) << 12 | (4 + 0) << 9  | (4 + 0) << 6  | (4 + 0) << 3  | (4 + 1),
             Area                    = 1 << 31 | 0 << 27 | (4 + 0) << 24 | (4 + 0) << 21 | (4 + 2) << 18 | (4 + 0) << 15 | (4 + 0) << 12 | (4 + 0) << 9  | (4 + 0) << 6  | (4 + 0) << 3  | (4 + 0),
             Volume                  = 1 << 31 | 0 << 27 | (4 + 0) << 24 | (4 + 0) << 21 | (4 + 3) << 18 | (4 + 0) << 15 | (4 + 0) << 12 | (4 + 0) << 9  | (4 + 0) << 6  | (4 + 0) << 3  | (4 + 0),
             Speed                   = 1 << 31 | 0 << 27 | (4 + 0) << 24 | (4 + 0) << 21 | (4 + 1) << 18 | (4 + 0) << 15 | (4 - 1) << 12 | (4 + 0) << 9  | (4 + 0) << 6  | (4 + 0) << 3  | (4 + 0),
             Velocity                = Speed,
             Acceleration            = 1 << 31 | 0 << 27 | (4 + 0) << 24 | (4 + 0) << 21 | (4 + 1) << 18 | (4 + 0) << 15 | (4 - 2) << 12 | (4 + 0) << 9  | (4 + 0) << 6  | (4 + 0) << 3  | (4 + 0)
         };

        // SI Factors
        typedef char Factor;
        enum {
         // Name           Code         Symbol    Factor
            ATTO        = (8 - 8), //     a       0.000000000000000001
            FEMTO       = (8 - 7), //     f       0.000000000000001
            PICO        = (8 - 6), //     p       0.000000000001
            NANO        = (8 - 5), //     n       0.000000001
            MICRO       = (8 - 4), //     μ       0.000001
            MILI        = (8 - 3), //     m       0.001
            CENTI       = (8 - 2), //     c       0.01
            DECI        = (8 - 1), //     d       0.1
            NONE        = (8    ), //     -       1
            DECA        = (8 + 1), //     da      10
            HECTO       = (8 + 2), //     h       100
            KILO        = (8 + 3), //     k       1000
            MEGA        = (8 + 4), //     M       1000000
            GIGA        = (8 + 5), //     G       1000000000
            TERA        = (8 + 6), //     T       1000000000000
            PETA        = (8 + 7)  //     P       1000000000000000
        };


        template<int N>
        struct Get { typedef typename SWITCH<N, CASE<I32, long, CASE<I64, long long, CASE<DEFAULT, long>>>>::Result Type; };

        template<typename T>
        struct GET { enum { NUM = I32 }; };

    public:
        Unit(unsigned long u) { _unit = u; }

        operator unsigned long() const { return _unit; }

        int sr()  const { return ((_unit & SR)  >> 24) - 4 ; }
        int rad() const { return ((_unit & RAD) >> 21) - 4 ; }
        int m()   const { return ((_unit & M)   >> 18) - 4 ; }
        int kg()  const { return ((_unit & KG)  >> 15) - 4 ; }
        int s()   const { return ((_unit & S)   >> 12) - 4 ; }
        int a()   const { return ((_unit & A)   >>  9) - 4 ; }
        int k()   const { return ((_unit & K)   >>  6) - 4 ; }
        int mol() const { return ((_unit & MOL) >>  3) - 4 ; }
        int cd()  const { return ((_unit & CD)  >>  0) - 4 ; }

        friend Debug & operator<<(Debug & db, const Unit & u) {
            if(u & SI) {
                db << "{SI";
                switch(u & MOD) {
                case DIR: break;
                case DIV: db << "[U/U]"; break;
                case LOG: db << "[log(U)]"; break;
                case LOG_DIV: db << "[log(U/U)]";
                };
                switch(u & NUM) {
                case I32: db << ".I32"; break;
                case I64: db << ".I64"; break;
                case F32: db << ".F32"; break;
                case D64: db << ".D64";
                }
                db << ':';
                if(u.sr())
                    db << "sr^" << u.sr();
                if(u.rad())
                    db << "rad^" << u.rad();
                if(u.m())
                    db << "m^" << u.m();
                if(u.kg())
                    db << "kg^" << u.kg();
                if(u.s())
                    db << "s^" << u.s();
                if(u.a())
                    db << "A^" << u.a();
                if(u.k())
                    db << "K^" << u.k();
                if(u.mol())
                    db << "mol^" << u.mol();
                if(u.cd())
                    db << "cdr^" << u.cd();
            } else
                db << "{D:" << "l=" <<  (u >> 16);
            db << "}";
            return db;
        }

    private:
        unsigned long _unit;
    } __attribute__((packed));

    // SI values (either integer32, integer64, float32, double64)
    template<int NUM>
    class Value
    {
        typedef typename IF<NUM == TSTP_Common::Unit::I64, long long int,
                typename IF<NUM == TSTP_Common::Unit::F32, float,
                typename IF<NUM == TSTP_Common::Unit::D64, double, long int
                    >::Result>::Result>::Result Number;
    public:
        Value(Number v): _value(v) {}

        operator Number() { return _value; }

    private:
        Number _value;
    };

    // Precision or Error in SI values, expressed as 10^Error
    typedef char Precision;
    typedef char Error;
};

__END_SYS

#endif

#ifndef __tstp_h
#define __tstp_h

#include <utility/observer.h>
#include <utility/buffer.h>
#include <utility/hash.h>
#include <utility/string.h>
#include <utility/array.h>
#include <network.h>
#include <diffie_hellman.h>
#include <cipher.h>
#include <thread.h>
#include <alarm.h>
#include <poly1305.h>

__BEGIN_SYS

class TSTP: public TSTP_Common, private NIC::Observer
{
    template<typename> friend class Smart_Data;
    friend class Epoch;

public:
    typedef NIC::Buffer Buffer;
    typedef IF<Traits<Network>::NETWORKS::Count<TSTP>::Result, Traits<NIC>::NICS::Get<Traits<Network>::NETWORKS::Find<TSTP>::Result>::Result, Traits<NIC>::NICS::Get<0>::Result>::Result Radio;

    // Packet
    static const unsigned int MTU = NIC::MTU - sizeof(Header);
    template<Scale S>
    class _Packet: public Header
    {
    private:
        typedef unsigned char Data[MTU];

    public:
        _Packet() {}

        Header * header() { return this; }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        friend Debug & operator<<(Debug & db, const _Packet & p) {
            db << "{h=" << reinterpret_cast<const Header &>(p);
            switch(reinterpret_cast<const Header &>(p).type()) {
                case INTEREST:
                    db << reinterpret_cast<const Interest &>(p);
                    break;
                case RESPONSE:
                    db << reinterpret_cast<const Response &>(p);
                    break;
                case COMMAND:
                    db << reinterpret_cast<const Command &>(p);
                    break;
                case CONTROL: {
                    switch(reinterpret_cast<const Control &>(p).subtype()) {
                        case DH_RESPONSE:
                            db << reinterpret_cast<const DH_Response &>(p);
                            break;
                        case AUTH_REQUEST:
                            db << reinterpret_cast<const Auth_Request &>(p);
                            break;
                        case DH_REQUEST:
                            db << reinterpret_cast<const DH_Request &>(p);
                            break;
                        case AUTH_GRANTED:
                            db << reinterpret_cast<const Auth_Granted &>(p);
                            break;
                        case REPORT:
                            db << reinterpret_cast<const Report &>(p);
                            break;
                        case KEEP_ALIVE:
                            db << reinterpret_cast<const Keep_Alive &>(p);
                            break;
                        case EPOCH:
                            db << reinterpret_cast<const Epoch &>(p);
                            break;
                        default:
                            break;
                    }
                }
                default:
                    break;
            }
            db << "}";
            return db;
        }

    private:
        Data _data;
    } __attribute__((packed));
    typedef _Packet<SCALE> Packet;


    // TSTP observer/d conditioned to a message's address (ID)
    typedef Data_Observer<Buffer, int> Observer;
    typedef Data_Observed<Buffer, int> Observed;


    // Hash to store TSTP Observers by type
    class Interested;
    typedef Hash<Interested, 10, Unit> Interests;
    class Responsive;
    typedef Hash<Responsive, 10, Unit> Responsives;


    // TSTP Messages
    // Each TSTP message is encapsulated in a single package. TSTP does not need nor supports fragmentation.

    // Interest/Response Modes
    enum Mode {
        // Response
        SINGLE = 0, // Only one response is desired for each interest job (desired, but multiple responses are still possible)
        ALL    = 1, // All possible responses (e.g. from different sensors) are desired
        // Interest
        DELETE = 2  // Revoke an interest
    };

    // Interest Message
    class Interest: public Header
    {
    public:
        Interest(const Region & region, const Unit & unit, const Mode & mode, const Error & precision, const Microsecond & expiry, const Microsecond & period = 0)
        : Header(INTEREST, 0, 0, now(), here(), here()), _region(region), _unit(unit), _mode(mode), _precision(0), _expiry(expiry), _period(period) {}

        const Unit & unit() const { return _unit; }
        const Region & region() const { return _region; }
        Microsecond period() const { return _period; }
        Time_Offset expiry() const { return _expiry; }
        Mode mode() const { return static_cast<Mode>(_mode); }
        Error precision() const { return static_cast<Error>(_precision); }

        bool time_triggered() { return _period; }
        bool event_driven() { return !time_triggered(); }

        friend Debug & operator<<(Debug & db, const Interest & m) {
            db << reinterpret_cast<const Header &>(m) << ",u=" << m._unit << ",m=" << ((m._mode == ALL) ? 'A' : 'S') << ",e=" << int(m._precision) << ",x=" << m._expiry << ",re=" << m._region << ",p=" << m._period;
            return db;
        }

    protected:
        Region _region;
        Unit _unit;
        unsigned char _mode : 2;
        unsigned char _precision : 6;
        Time_Offset _expiry;
        Microsecond _period; // TODO: should be Time_Offset
        CRC _crc;
    } __attribute__((packed));

    // Response (Data) Message
    class Response: public Header
    {
    private:
        typedef unsigned char Data[MTU - sizeof(Unit) - sizeof(Error) - sizeof(Time_Offset) - sizeof(CRC)];

    public:
        Response(const Unit & unit, const Error & error = 0, const Time & expiry = 0)
        : Header(RESPONSE, 0, 0, now(), here(), here()), _unit(unit), _error(error), _expiry(expiry) {}

        const Unit & unit() const { return _unit; }
        Error error() const { return _error; }
        Time expiry() const { return _time + _expiry; }
        void expiry(const Time & e) { _expiry = e - _time; }

        template<typename T>
        void value(const T & v) { *reinterpret_cast<Value<Unit::GET<T>::NUM> *>(&_data) = v; }

        template<typename T>
        T value() { return *reinterpret_cast<Value<Unit::GET<T>::NUM> *>(&_data); }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        friend Debug & operator<<(Debug & db, const Response & m) {
            db << reinterpret_cast<const Header &>(m) << ",u=" << m._unit << ",e=" << int(m._error) << ",x=" << m._expiry << ",d=" << hex << *const_cast<Response &>(m).data<unsigned>() << dec;
            return db;
        }

    protected:
        Unit _unit;
        Error _error;
        Time_Offset _expiry;
        Data _data;
        CRC _crc;
    } __attribute__((packed));

    // Command Message
    class Command: public Header
    {
    private:
        typedef unsigned char Data[MTU - sizeof(Region) - sizeof(Unit) - sizeof(CRC)];

    public:
        Command(const Unit & unit, const Region & region)
        : Header(COMMAND, 0, 0, now(), here(), here()), _region(region), _unit(unit) {}

        const Region & region() const { return _region; }
        const Unit & unit() const { return _unit; }

        template<typename T>
        T * command() { return reinterpret_cast<T *>(&_data); }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        friend Debug & operator<<(Debug & db, const Command & m) {
            db << reinterpret_cast<const Header &>(m) << ",u=" << m._unit << ",reg=" << m._region;
            return db;
        }

    protected:
        Region _region;
        Unit _unit;
        Data _data;
        CRC _crc;
    } __attribute__((packed));

    // Security types
    typedef _UTIL::Array<unsigned char, 16> Node_ID;
    typedef _UTIL::Array<unsigned char, 16> Auth;
    typedef _UTIL::Array<unsigned char, 16> OTP;
    typedef Diffie_Hellman::Public_Key Public_Key;

    // Diffie-Hellman Request Security Bootstrap Control Message
    class DH_Request: public Control
    {
    public:
        DH_Request(const Region::Space & dst, const Public_Key & k)
        : Control(DH_REQUEST, 0, 0, now(), here(), here()), _destination(dst), _public_key(k) { }

        const Region::Space & destination() { return _destination; }
        void destination(const Region::Space & d) { _destination = d; }

        Public_Key key() { return _public_key; }
        void key(const Public_Key & k) { _public_key = k; }

        friend Debug & operator<<(Debug & db, const DH_Request & m) {
            db << reinterpret_cast<const Control &>(m) << ",d=" << m._destination << ",k=" << m._public_key;
            return db;
        }
        friend OStream & operator<<(OStream & db, const DH_Request & m) {
            db << reinterpret_cast<const Control &>(m) << ",d=" << m._destination << ",k=" << m._public_key;
            return db;
        }

    private:
        Region::Space _destination;
        Public_Key _public_key;
        CRC _crc;
    //} __attribute__((packed)); // TODO
    };

    // Diffie-Hellman Response Security Bootstrap Control Message
    class DH_Response: public Control
    {
    public:
        DH_Response(const Public_Key & k)
        : Control(DH_RESPONSE, 0, 0, now(), here(), here()), _public_key(k) { }

        Public_Key key() { return _public_key; }
        void key(const Public_Key & k) { _public_key = k; }

        friend Debug & operator<<(Debug & db, const DH_Response & m) {
            db << reinterpret_cast<const Control &>(m) << ",k=" << m._public_key;
            return db;
        }

    private:
        Public_Key _public_key;
        CRC _crc;
    //} __attribute__((packed)); // TODO
    };

    // Authentication Request Security Bootstrap Control Message
    class Auth_Request: public Control
    {
    public:
        Auth_Request(const Auth & a, const OTP & o)
        : Control(AUTH_REQUEST, 0, 0, now(), here(), here()), _auth(a), _otp(o) { }

        const Auth & auth() const { return _auth; }
        void auth(const Auth & a) { _auth = a; }

        const OTP & otp() const { return _otp; }
        void otp(const OTP & o) { _otp = o; }

        friend Debug & operator<<(Debug & db, const Auth_Request & m) {
            db << reinterpret_cast<const Control &>(m) << ",a=" << m._auth << ",o=" << m._otp;
            return db;
        }

    private:
        Auth _auth;
        OTP _otp;
        CRC _crc;
    //} __attribute__((packed)); // TODO
    };

    // Athentication Granted Security Bootstrap Control Message
    class Auth_Granted: public Control
    {
    public:
        Auth_Granted(const Region::Space & dst, const Auth & a)
        : Control(AUTH_GRANTED, 0, 0, now(), here(), here()), _destination(dst), _auth(a) { }

        const Region::Space & destination() { return _destination; }
        void destination(const Coordinates  & d) { _destination = d; }

        const Auth & auth() const { return _auth; }
        void auth(const Auth & a) { _auth = a; }

        friend Debug & operator<<(Debug & db, const Auth_Granted & m) {
            db << reinterpret_cast<const Control &>(m) << ",d=" << m._destination << ",a=" << m._auth;
            return db;
        }

    private:
        Region::Space _destination;
        Auth _auth; // TODO
        CRC _crc;
    // } __attribute__((packed)); // TODO
    };

    // Report Control Message
    class Report: public Control
    {
    public:
        Report(const Unit & unit, const Error & error = 0, bool epoch_request = false)
        : Control(REPORT, 0, 0, now(), here(), here()), _unit(unit), _error(error), _epoch_request(epoch_request) { }

        const Unit & unit() const { return _unit; }
        Error error() const { return _error; }
        bool epoch_request() const { return _epoch_request; }

        friend Debug & operator<<(Debug & db, const Report & r) {
            db << reinterpret_cast<const Control &>(r) << ",u=" << r._unit << ",e=" << r._error;
            return db;
        }

    private:
        Unit _unit;
        Error _error;
        bool _epoch_request;
        CRC _crc;
    } __attribute__((packed));

    // Keep Alive Control Message
    class Keep_Alive: public Control
    {
    public:
        Keep_Alive()
        : Control(KEEP_ALIVE, 0, 0, now(), here(), here()) { }

        friend Debug & operator<<(Debug & db, const Keep_Alive & k) {
            db << reinterpret_cast<const Control &>(k);
            return db;
        }

    private:
        CRC _crc;
    } __attribute__((packed));

    // Epoch Control Message
    class Epoch: public Control
    {
    public:
        Epoch(const Region & dst, const Time & ep = TSTP::_epoch, const Global_Coordinates & coordinates = TSTP::_global_coordinates)
        : Control(EPOCH, 0, 0, now(), here(), here()), _destination(dst), _epoch(ep), _coordinates(coordinates) { }

        const Region & destination() const { return _destination; }
        const Time epoch() const { return _epoch; }
        const Global_Coordinates & coordinates() const { return _coordinates; }

        friend Debug & operator<<(Debug & db, const Epoch & e) {
            db << reinterpret_cast<const Control &>(e) << ",e=" << e._epoch << ",c=" << e._coordinates;
            return db;
        }

    private:
        Region _destination;
        Time _epoch;
        Global_Coordinates _coordinates;
        CRC _crc;
    } __attribute__((packed));

    // TSTP Smart Data bindings
    // Interested (binder between Interest messages and Smart Data)
    class Interested: public Interest
    {
    public:
        template<typename T>
        Interested(T * data, const Region & region, const Unit & unit, const Mode & mode, const Precision & precision, const Microsecond & expiry, const Microsecond & period = 0)
        : Interest(region, unit, mode, precision, expiry, period), _link(this, T::UNIT) {
            db<TSTP>(TRC) << "TSTP::Interested(d=" << data << ",r=" << region << ",p=" << period << ") => " << reinterpret_cast<const Interest &>(*this) << endl;
            _interested.insert(&_link);
            advertise();
        }
        ~Interested() {
            db<TSTP>(TRC) << "TSTP::~Interested(this=" << this << ")" << endl;
            _interested.remove(&_link);
            revoke();
        }

        void advertise() { send(); }
        void revoke() { _mode = DELETE; send(); }

        template<typename Value>
        void command(const Value & v) {
            db<TSTP>(TRC) << "TSTP::Interested::command(v=" << v << ")" << endl;
            Buffer * buf = alloc(sizeof(Command));
            Command * command = new (buf->frame()->data<Command>()) Command(unit(), region());
            memcpy(command->command<Value>(), &v, sizeof(Value));
            marshal(buf);
            db<TSTP>(INF) << "TSTP::Interested::command:command=" << command << " => " << (*command) << endl;
            _nic->send(buf);
        }

    private:
        void send() {
            db<TSTP>(TRC) << "TSTP::Interested::send() => " << reinterpret_cast<const Interest &>(*this) << endl;
            Buffer * buf = alloc(sizeof(Interest));
            memcpy(buf->frame()->data<Interest>(), this, sizeof(Interest));
            marshal(buf);
            _nic->send(buf);
        }

    private:
        Interests::Element _link;
    };

    // Responsive (binder between Smart Data (Sensors) and Response messages)
    class Responsive: public Response
    {
    public:
        template<typename T>
        Responsive(T * data, const Unit & unit, const Error & error, const Time & expiry, bool epoch_request = false)
        : Response(unit, error, expiry), _size(sizeof(Response) - sizeof(Response::Data) + sizeof(typename T::Value)), _t0(0), _t1(0), _link(this, T::UNIT) {
            db<TSTP>(TRC) << "TSTP::Responsive(d=" << data << ",s=" << _size << ") => " << this << endl;
            db<TSTP>(INF) << "TSTP::Responsive() => " << reinterpret_cast<const Response &>(*this) << endl;
            _responsives.insert(&_link);
            advertise(epoch_request);
        }
        ~Responsive() {
            db<TSTP>(TRC) << "TSTP::~Responsive(this=" << this << ")" << endl;
            _responsives.remove(&_link);
        }

        using Header::time;
        using Header::origin;

        void t0(const Time & t) { _t0 = t; }
        void t1(const Time & t) { _t1 = t; }
        Time t0() const { return _t0; }
        Time t1() const { return _t1; }

        void respond(const Time & expiry) { send(expiry); }
        void advertise(bool epoch_request = false) {
            db<TSTP>(TRC) << "TSTP::Responsive::advertise()" << endl;
            Buffer * buf = alloc(sizeof(Report));
            Report * report = new (buf->frame()->data<Report>()) Report(unit(), error(), epoch_request);
            marshal(buf);
            db<TSTP>(INF) << "TSTP::Responsive::advertise:report=" << report << " => " << (*report) << endl;
            _nic->send(buf);
        }

    private:
        void send(const Time & expiry) {
            if((_time >= _t0) && (_time <= _t1)) {
                assert(expiry > now());
                db<TSTP>(TRC) << "TSTP::Responsive::send(x=" << expiry << ")" << endl;
                Buffer * buf = alloc(_size);
                Response * response = buf->frame()->data<Response>();
                memcpy(response, this, _size);
                response->expiry(expiry);
                marshal(buf);
                db<TSTP>(INF) << "TSTP::Responsive::send:response=" << response << " => " << (*response) << endl;
                _nic->send(buf);
            }
        }

    private:
        unsigned int _size;
        Time _t0;
        Time _t1;
        Responsives::Element _link;
    };


    class Router;

    // TSTP Locator
    class Locator: private NIC::Observer
    {
        friend class TSTP::Router;

        typedef char RSSI;

        struct Peer {
            Coordinates coordinates;
            Percent confidence;
            RSSI rssi;
        };

    public:
        Locator() {
            db<TSTP>(TRC) << "TSTP::Locator()" << endl;
            _n_peers = 0;
            _confidence = 0;
            do {
                _here = Coordinates(Random::random(), Random::random(), Random::random());
            } while(_here == TSTP::sink());
        }
        ~Locator();

        static Coordinates here() { return _here; }

        void bootstrap();

        static bool synchronized() { return _confidence >= 80; }

        static void marshal(Buffer * buf);

        void update(NIC::Observed * obs, NIC::Protocol prot, NIC::Buffer * buf);

    private:
        void add_peer(Coordinates coord, Percent conf, RSSI r) {
            db<TSTP>(INF) << "TSTP::Locator::add_peer(c=" << coord << ",conf=" << conf << ",r=" << static_cast<int>(r) << ")" << endl;

            unsigned int idx = -1u;

            for(unsigned int i = 0; i < _n_peers; i++) {
                if(_peers[i].coordinates == coord) {
                    if(_peers[i].confidence > conf)
                       return;
                    else {
                        idx = i;
                        break;
                    }
                }
            }

            if(idx == -1u) {
                if(_n_peers < 3)
                    idx = _n_peers++;
                else
                    for(unsigned int i = 0; i < _n_peers; i++)
                        if((_peers[i].confidence <= conf) && ((idx == -1u) || (conf >= _peers[i].confidence)))
                            idx = i;
            }

            if(idx != -1u) {
                _peers[idx].coordinates = coord;
                _peers[idx].confidence = conf;
                _peers[idx].rssi = r;

                if(_n_peers == 3) {
                    Coordinates new_here = _here.trilaterate(_peers[0].coordinates, _peers[0].rssi + 128, _peers[1].coordinates, _peers[1].rssi + 128, _peers[2].coordinates, _peers[2].rssi + 128);
                    if(new_here != TSTP::sink()) {
                        _here = new_here;
                        _confidence = (_peers[0].confidence + _peers[1].confidence + _peers[2].confidence) * 80 / 100 / 3;
                        db<TSTP>(INF) << "TSTP::Locator: Location updated: " << _here << ", confidence = " << _confidence << "%" << endl;
                    } else
                        db<TSTP>(INF) << "TSTP::Locator: Dropped trilateration that resulted in here == sink" << endl;
                }
            }
        }

    private:
        static Coordinates _here;
        static unsigned int _n_peers;
        static Percent _confidence;
        static Peer _peers[3];
    };


    // TSTP Timekeeper
    class Timekeeper: private NIC::Observer
    {
        typedef Radio::Timer::Time_Stamp Time_Stamp;
        typedef Radio::Timer::Offset Offset;

        static const unsigned int SYNC_PERIOD = 300000000; // TODO

    public:
        Timekeeper() {
            db<TSTP>(TRC) << "TSTP::Timekeeper()" << endl;
            _t0 = 0;
            _t1 = 0;
            _next_sync = 0;
        }
        ~Timekeeper();

        static Time now() { return Radio::Timer::count2us(Radio::Timer::read()); }

        static bool synchronized() {
            bool ret = now() <= _next_sync;
            if((!ret) && (now() > _next_sync + SYNC_PERIOD))
                _t1 = 0; // Find a new peer
            return ret;
        }

        void bootstrap();

        static void marshal(Buffer * buf);

        void update(NIC::Observed * obs, NIC::Protocol prot, NIC::Buffer * buf);

    private:
        static Time_Stamp _t0;
        static Time_Stamp _t1;
        static Time_Stamp _next_sync;
        static Coordinates _peer;
    };


    // TSTP Router
    class Router: private NIC::Observer
    {
    private:
        static const unsigned int CCA_TX_GAP = IEEE802_15_4::CCA_TX_GAP;
        static const unsigned int RADIO_RANGE = TSTP_Common::RADIO_RANGE;

    public:
        Router() {
            db<TSTP>(TRC) << "TSTP::Router()" << endl;
        }
        ~Router();

        void bootstrap();

        static bool synchronized() { return true; }

        static void marshal(Buffer * buf);

        void update(NIC::Observed * obs, NIC::Protocol prot, NIC::Buffer * buf);

    private:
        static void offset(Buffer * buf) {
            // TODO
            //long long dist = abs(buf->my_distance - (buf->sender_distance - RADIO_RANGE));
            //long long betha = (CCA_TX_GAP * RADIO_RADIUS * 1000000) / (dist * CCA_TX_GAP);
            buf->offset = abs(buf->my_distance - (buf->sender_distance - RADIO_RANGE));
        }
    };


    // TSTP Security
    class Security: private NIC::Observer
    {
        friend class TSTP;

        static const unsigned int KEY_MANAGER_PERIOD = 10 * 1000 * 1000;
        static const unsigned long long KEY_EXPIRY = 1 * 60 * 1000 * 1000;

    public:
        typedef Diffie_Hellman::Shared_Key Master_Secret;

    private:
        class Peer;
        typedef Simple_List<Peer> Peers;
        class Peer
        {
        public:
            Peer(const Node_ID & id, const Region & v)
               : _id(id), _valid(v), _el(this), _auth_time(0) {
               Security::_cipher.encrypt(_id, _id, _auth);
            }

            void valid(const Region & r) { _valid = r; }
            const Region & valid() const { return _valid; }

            bool valid_deploy(const Coordinates & where, const Time & when) {
                return _valid.contains(where, when);
            }

            bool valid_request(const Auth & auth, const Coordinates & where, const Time & when) {
                return !memcmp(auth, _auth, sizeof(Auth)) && _valid.contains(where, when);
            }

            const Time & authentication_time() { return _auth_time; }

            Peers::Element * link() { return &_el; }

            const Master_Secret & master_secret() const { return _master_secret; }
            void master_secret(const Master_Secret & ms) {
                _master_secret = ms;
                _auth_time = TSTP::now();
            }

            const Auth & auth() const { return _auth; }
            const Node_ID & id() const { return _id; }

            friend Debug & operator<<(Debug & db, const Peer & p) {
                db << "{id=" << p._id << ",au=" << p._auth << ",v=" << p._valid << ",ms=" << p._master_secret << ",el=" << &p._el << "}";
                return db;
            }

        private:
            Node_ID _id;
            Auth _auth;
            Region _valid;
            Master_Secret _master_secret;
            Peers::Element _el;
            Time _auth_time;
        };

        struct Pending_Key;
        typedef Simple_List<Pending_Key> Pending_Keys;
        class Pending_Key
        {
        public:
            Pending_Key(const Public_Key & pk) : _master_secret_calculated(false), _creation(TSTP::now()), _public_key(pk), _el(this) {}

            bool expired() { return TSTP::now() - _creation > KEY_EXPIRY; }

            const Master_Secret & master_secret() {
                if(_master_secret_calculated)
                    return _master_secret;
                _master_secret = Security::_dh.shared_key(_public_key);
                _master_secret_calculated = true;
                db<TSTP>(INF) << "TSTP::Security::Pending_Key: Master Secret set: " << _master_secret << endl;
                return _master_secret;
            }

            Pending_Keys::Element * link() { return &_el; };

            friend Debug & operator<<(Debug & db, const Pending_Key & p) {
                db << "{msc=" << p._master_secret_calculated << ",c=" << p._creation << ",pk=" << p._public_key << ",ms=" << p._master_secret << ",el=" << &p._el << "}";
                return db;
            }

        private:
            bool _master_secret_calculated;
            Time _creation;
            Public_Key _public_key;
            Master_Secret _master_secret;
            Pending_Keys::Element _el;
        };

    public:
        Security() {
            db<TSTP>(TRC) << "TSTP::Security()" << endl;

            // FIXME: hardcoded Machine ID size of 8
            // should be something like: Node_ID(Machine::id(), sizeof(Machine::ID));
            new (&_id) Node_ID(Machine::id(), 8);

            db<TSTP>(INF) << "Node ID: " << _id << endl;

            assert(Cipher::KEY_SIZE == sizeof(Node_ID));
            _cipher.encrypt(_id, _id, _auth);
        }
        ~Security();

        static void add_peer(const unsigned char * peer_id, unsigned int id_len, const Region & valid_region) {
            Node_ID id(peer_id, id_len);
            Peer * peer = new (SYSTEM) Peer(id, valid_region);
            //while(CPU::tsl(_peers_lock));
            _pending_peers.insert(peer->link());
            //_peers_lock = false;
            if(!_key_manager)
                _key_manager = new (SYSTEM) Thread(&key_manager);
        }

        void bootstrap();

        static bool synchronized() { return true; }

        static void marshal(Buffer * buf);

        void update(NIC::Observed * obs, NIC::Protocol prot, NIC::Buffer * buf);

    private:
        static void encrypt(const unsigned char * msg, const Peer * peer, unsigned char * out) {
            OTP key = otp(peer->master_secret(), peer->id());
            _cipher.encrypt(msg, key, out);
        }

        static void decrypt(const unsigned char * msg, const Peer * peer, unsigned char * out) {
            OTP key = otp(peer->master_secret(), peer->id());
            _cipher.decrypt(msg, key, out);
        }

        static OTP otp(const Master_Secret & master_secret, const Node_ID & id) {
            const unsigned char * ms = reinterpret_cast<const unsigned char *>(&master_secret);

            // mi = ms ^ _id
            static const unsigned int MI_SIZE = sizeof(Node_ID) > sizeof(Master_Secret) ? sizeof(Node_ID) : sizeof(Master_Secret);
            unsigned char mi[MI_SIZE];
            unsigned int i;
            for(i = 0; (i < sizeof(Node_ID)) && (i < sizeof(Master_Secret)); i++)
                mi[i] = id[i] ^ ms[i];
            for(; i < sizeof(Node_ID); i++)
                mi[i] = id[i];
            for(; i < sizeof(Master_Secret); i++)
                mi[i] = ms[i];

            Time t = TSTP::now() / (KEY_EXPIRY / 2);

            unsigned char nonce[16];
            memset(nonce, 0, 16);
            memcpy(nonce, &t, min(sizeof(Time), 16lu));

            OTP out;
            Poly1305(id, ms).stamp(out, nonce, mi, MI_SIZE);
            return out;
        }

        static bool verify(const Master_Secret & master_secret, const Node_ID & id, const OTP & otp) {
            const unsigned char * ms = reinterpret_cast<const unsigned char *>(&master_secret);

            // mi = ms ^ _id
            static const unsigned int MI_SIZE = sizeof(Node_ID) > sizeof(Master_Secret) ? sizeof(Node_ID) : sizeof(Master_Secret);
            unsigned char mi[MI_SIZE];
            unsigned int i;
            for(i = 0; (i < sizeof(Node_ID)) && (i < sizeof(Master_Secret)); i++)
                mi[i] = id[i] ^ ms[i];
            for(; i < sizeof(Node_ID); i++)
                mi[i] = id[i];
            for(; i < sizeof(Master_Secret); i++)
                mi[i] = ms[i];

            unsigned char nonce[16];
            Time t = TSTP::now() / (KEY_EXPIRY / 2);

            Poly1305 poly(id, ms);

            memset(nonce, 0, 16);
            memcpy(nonce, &t, min(sizeof(Time), 16lu));
            if(poly.verify(otp, nonce, mi, MI_SIZE))
                return true;

            t--;
            memset(nonce, 0, 16);
            memcpy(nonce, &t, min(sizeof(Time), 16lu));
            if(poly.verify(otp, nonce, mi, MI_SIZE))
                return true;

            t += 2;
            memset(nonce, 0, 16);
            memcpy(nonce, &t, min(sizeof(Time), 16lu));
            if(poly.verify(otp, nonce, mi, MI_SIZE))
                return true;

            return false;
        }

        static int key_manager() {

            Peers::Element * last_dh_request = 0;

            while(true) {
                Alarm::delay(KEY_MANAGER_PERIOD);

                db<TSTP>(TRC) << "TSTP::Security::key_manager()" << endl;
                CPU::int_disable();
                //while(CPU::tsl(_peers_lock));

                // Cleanup expired pending keys
                Pending_Keys::Element * next_key;
                for(Pending_Keys::Element * el = _pending_keys.head(); el; el = next_key) {
                    next_key = el->next();
                    Pending_Key * p = el->object();
                    if(p->expired()) {
                        _pending_keys.remove(el);
                        delete p;
                        db<TSTP>(INF) << "TSTP::Security::key_manager(): removed pending key" << endl;
                    }
                }

                // Cleanup expired peers
                Peers::Element * next;
                for(Peers::Element * el = _trusted_peers.head(); el; el = next) {
                    next = el->next();
                    Peer * p = el->object();
                    if(!p->valid_deploy(p->valid().center, TSTP::now())) {
                        _trusted_peers.remove(el);
                        delete p;
                        db<TSTP>(INF) << "TSTP::Security::key_manager(): permanently removed trusted peer" << endl;
                    }
                }
                for(Peers::Element * el = _pending_peers.head(); el; el = next) {
                    next = el->next();
                    Peer * p = el->object();
                    if(!p->valid_deploy(p->valid().center, TSTP::now())) {
                        _pending_peers.remove(el);
                        delete p;
                        db<TSTP>(INF) << "TSTP::Security::key_manager(): permanently removed pending peer" << endl;
                    }
                }

                // Cleanup expired established keys
                for(Peers::Element * el = _trusted_peers.head(); el; el = next) {
                    next = el->next();
                    Peer * p = el->object();
                    if(TSTP::now() - p->authentication_time() > KEY_EXPIRY) {
                        _trusted_peers.remove(el);
                        _pending_peers.insert(el);
                        db<TSTP>(INF) << "TSTP::Security::key_manager(): trusted peer's key expired" << endl;
                    }
                }

                // Send DH Request to at most one peer
                Peers::Element * el;
                if(last_dh_request && last_dh_request->next())
                    el = last_dh_request->next();
                else
                    el = _pending_peers.head();

                last_dh_request = 0;

                for(; el; el = el->next()) {
                    Peer * p = el->object();
                    if(p->valid_deploy(p->valid().center, TSTP::now())) {
                        last_dh_request = el;
                        Buffer * buf = alloc(sizeof(DH_Request));
                        new (buf->frame()->data<DH_Request>()) DH_Request(Region::Space(p->valid().center, p->valid().radius), _dh.public_key());
                        marshal(buf);
                        _dh_requests_open++;
                        TSTP::_nic->send(buf);
                        db<TSTP>(INF) << "TSTP::Security::key_manager(): Sent DH_Request: "  << *buf->frame()->data<DH_Request>() << endl;
                        break;
                    }
                }

                //_peers_lock = false;
                CPU::int_enable();
            }

            return 0;
        }

    private:
        static Cipher _cipher;
        static Node_ID _id;
        static Auth _auth;
        static Diffie_Hellman _dh;
        static Pending_Keys _pending_keys;
        static Peers _pending_peers;
        static Peers _trusted_peers;
        static volatile bool _peers_lock;
        static Thread * _key_manager;
        static unsigned int _dh_requests_open;
    };

    // TSTP Life Keeper explicitly asks for metadata whenever it's needed
    class Life_Keeper
    {
        friend class TSTP;

        static const unsigned int PERIOD = 10000000;

        Life_Keeper(): _thread(&life_keeper) {}

    private:
        static int life_keeper() {
            while(true) {
                if(!(TSTP::Locator::synchronized() && TSTP::Timekeeper::synchronized() && TSTP::Router::synchronized() && TSTP::Security::synchronized())) {
                    db<TSTP>(TRC) << "TSTP::Life_Keeper: sending keep alive message" << endl;
                    TSTP::keep_alive();
                }
                Alarm::delay(PERIOD);
            }
            return 0;
        }

        Thread _thread;
    };

protected:
    TSTP();

public:
    ~TSTP();

    // Local network Space-Time
    static Coordinates here() { return Locator::here(); }
    static Time now() { return Timekeeper::now(); }
    static Coordinates sink() { return Coordinates(0, 0, 0); }
    static Time local(const Time & global) { return global - _epoch; }
    static Coordinates local(Global_Coordinates global) {
        global -= _global_coordinates;
        return Coordinates(global.x, global.y, global.z);
    }

    // Global Space-Time
    static Global_Coordinates absolute(const Coordinates & coordinates) { return _global_coordinates + coordinates; }
    static Time absolute(const Time & t) {
        if((t == static_cast<Time>(-1)) || (t == 0))
           return t;
        return _epoch + t;
    }
    static void epoch(const Time & t) { _epoch = t - now(); }
    static void coordinates(const Global_Coordinates & c) { _global_coordinates = c; }

    static void attach(Observer * obs, void * subject) { _observed.attach(obs, int(subject)); }
    static void detach(Observer * obs, void * subject) { _observed.detach(obs, int(subject)); }
    static bool notify(void * subject, Buffer * buf) { return _observed.notify(int(subject), buf); }

    template<unsigned int UNIT>
    static void init(const NIC & nic);

private:
    static Region destination(Buffer * buf) {
        switch(buf->frame()->data<Frame>()->type()) {
            case INTEREST:
                return buf->frame()->data<Interest>()->region();
            case RESPONSE:
                return Region(sink(), 0, buf->frame()->data<Response>()->time(), buf->frame()->data<Response>()->expiry());
            case COMMAND:
                return buf->frame()->data<Command>()->region();
            case CONTROL:
                switch(buf->frame()->data<Control>()->subtype()) {
                    default:
                    case DH_RESPONSE:
                    case AUTH_REQUEST: {
                        Time origin = buf->frame()->data<Header>()->time();
                        Time deadline = origin + min(static_cast<unsigned long long>(Security::KEY_MANAGER_PERIOD), Security::KEY_EXPIRY) / 2;
                        return Region(sink(), 0, origin, deadline);
                    }
                    case DH_REQUEST: {
                        Time origin = buf->frame()->data<Header>()->time();
                        Time deadline = origin + min(static_cast<unsigned long long>(Security::KEY_MANAGER_PERIOD), Security::KEY_EXPIRY) / 2;
                        return Region(buf->frame()->data<DH_Request>()->destination().center, buf->frame()->data<DH_Request>()->destination().radius, origin, deadline);
                    }
                    case AUTH_GRANTED: {
                        Time origin = buf->frame()->data<Header>()->time();
                        Time deadline = origin + min(static_cast<unsigned long long>(Security::KEY_MANAGER_PERIOD), Security::KEY_EXPIRY) / 2;
                        return Region(buf->frame()->data<Auth_Granted>()->destination().center, buf->frame()->data<Auth_Granted>()->destination().radius, origin, deadline);
                    }
                    case REPORT: {
                        return Region(sink(), 0, buf->frame()->data<Report>()->time(), -1/*TODO*/);
                    }
                    case KEEP_ALIVE: {
                        while(true) {
                            Coordinates fake(here().x + (Random::random() % (RADIO_RANGE / 3)), here().y + (Random::random() % (RADIO_RANGE / 3)), (here().z + Random::random() % (RADIO_RANGE / 3)));
                            if(fake != here())
                                return Region(fake, 0, 0, -1); // Should never be destined_to_me
                        }
                    }
                    case EPOCH: {
                        return buf->frame()->data<Epoch>()->destination();
                    }
                }
            default:
                db<TSTP>(ERR) << "TSTP::destination(): ERROR: unrecognized frame type " << buf->frame()->data<Frame>()->type() << endl;
                return Region(TSTP::here(), 0, TSTP::now() - 2, TSTP::now() - 1);
        }
    }

    static void keep_alive() {
        db<TSTP>(TRC) << "TSTP::keep_alive()" << endl;
        Buffer * buf = alloc(sizeof(Keep_Alive));
        Keep_Alive * keep_alive = new (buf->frame()->data<Keep_Alive>()) Keep_Alive;
        marshal(buf);
        buf->deadline = now() + Life_Keeper::PERIOD;
        db<TSTP>(INF) << "TSTP::keep_alive():keep_alive = " << keep_alive << " => " << (*keep_alive) << endl;
        _nic->send(buf);
    }

    static void marshal(Buffer * buf) {
        Locator::marshal(buf);
        Timekeeper::marshal(buf);
        Router::marshal(buf);
        Security::marshal(buf);
    }

    static Buffer * alloc(unsigned int size) {
        return _nic->alloc(NIC::Address::BROADCAST, NIC::TSTP, 0, 0, size - sizeof(Header));
    }

    void update(NIC::Observed * obs, NIC::Protocol prot, NIC::Buffer * buf);

private:
    static NIC * _nic;
    static Interests _interested;
    static Responsives _responsives;
    static Observed _observed; // Channel protocols are singletons
    static Time _epoch;
    static Global_Coordinates _global_coordinates;
};

__END_SYS

#endif
