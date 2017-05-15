// EPOS IEEE 802.15.4 Mediator Common Package

#include <nic.h>

#ifndef __ieee802_15_4_h
#define __ieee802_15_4_h

#include <cpu.h>
#include <utility/list.h>
#include <utility/observer.h>
#include <utility/buffer.h>
#include <utility/random.h>

__BEGIN_SYS

class IEEE802_15_4: public NIC_Common
{
private:
    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg16 Reg16;

public:
    typedef NIC_Common::Address<2> Short_Address;
    typedef NIC_Common::Address<8> Extended_Address;
    typedef Short_Address Address;
    typedef NIC_Common::CRC16 CRC;

    // IEEE 802.15.4 Physical Layer
    static const unsigned int MTU = 127;
    static const unsigned int CCA_TX_GAP = 320;
    static const unsigned int TURNAROUND_TIME = 192;
    static const unsigned int BYTE_RATE = 31250; // bytes per second
    static const unsigned int SHR_SIZE = 5; // bytes
    static const unsigned int PHR_SIZE = 1; // bytes
    static const unsigned int PHY_HEADER_SIZE = SHR_SIZE + PHR_SIZE; // bytes

    typedef unsigned char Data[MTU];

    // Frame
    class Phy_Frame
    {
    public:
        Phy_Frame() {}

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        friend Debug & operator<<(Debug & db, const Phy_Frame & f) {
            db << "{d=" << f._data << "}";
            return db;
        }

    protected:
        Data _data;
    } __attribute__((packed));


    // IEEE 802.15.4 MAC Layer
    // Frame types
    typedef unsigned char Type;
    enum
    {
        BEACON  = 0,
        DATA    = 1,
        ACK     = 2,
        MAC_CMD = 3,
    };

    typedef Type Protocol;
    enum
    {
        IP     = 1,
        ARP    = 1,
        RARP   = 1,
        TSTP   = 4,
        ELP    = 1,
        PTP    = 1
    };

    enum Frame_Version
    {
        IEEE802_15_4_2003 = 0,
        IEEE802_15_4_2006 = 1,
    };

    // Addresses
    enum Addressing_Mode
    {
        NO_ADDRRESS      = 0,
        SHORT_ADDRESS    = 2,
        EXTENTED_ADDRESS = 4,
    };

    enum
    {
        PAN_ID_BROADCAST = 0xffff
    };

    // Header
    // 802.15.4 headers can have variable format, for now this only
    // supports a simple, fixed format
    class Header
    {
    public:
        // Frame Control
        class Frame_Control
        {
        public:
            Frame_Control(unsigned short type = DATA, bool se = 0, bool fp = 0, bool ar = 0, bool pic = 1, unsigned short dam = SHORT_ADDRESS, unsigned short fv = Frame_Version::IEEE802_15_4_2006, unsigned short sam = SHORT_ADDRESS)
            : _fc(type | se << 3 | fp << 4 | ar << 5 | pic << 6 | dam << 10 | fv << 12 | sam << 14) {}

            unsigned short type() const { return _fc & 0x0007; }
            void type(unsigned short t) { _fc = (_fc & (~0x0007)) | (t & 0x0007); }

            bool se() const { return (_fc >> 3) & 0x0001; }
            void se(bool b) { _fc = (_fc & (~(1 << 3))) | (b << 3); }

            bool fp() const { return (_fc >> 4) & 0x0001; }
            void fp(bool b) { _fc = (_fc & (~(1 << 4))) | (b << 4); }

            bool ar() const { return (_fc >> 5) & 0x0001; }
            void ar(bool b) { _fc = (_fc & (~(1 << 5))) | (b << 5); }

            bool pic() const { return (_fc >> 6) & 0x0001; }
            void pic(bool b) { _fc = (_fc & (~(1 << 6))) | (b << 6); }

            unsigned short dam() const { return (_fc >> 10) & 0x0003; }
            void dam(unsigned short m) { _fc = (_fc & (~(3 << 10))) | ((m & 0x0003) << 10); }

            unsigned short fv() const { return (_fc >> 12) & 0x0003; }
            void fv(unsigned short v) { _fc = (_fc & (~(3 << 12))) | ((v & 0x0003) << 12); }

            unsigned short sam() const { return (_fc & 0x0003); }
            void sam(unsigned short m) { _fc = (_fc & (~(3 << 14))) | ((m & 0x0003) << 14); }

            friend Debug & operator<<(Debug & db, const Frame_Control & fc) {
                db << "{type=" << fc.type() << ",se=" << fc.se() << ",fp=" << fc.fp() << ",ar=" << fc.ar() << ",pic=" << fc.pic() << ",dam=" << fc.dam() << ",fv=" << fc.fv() << ",sam=" << fc.sam() << "}";
                return db;
            }

        private:
            unsigned short _fc;
        } __attribute__((packed));

    public:
        Header() {}

        Header(const Type & type)
        : _frame_control(type), _sequence_number(Random::random()), _dst_pan_id(PAN_ID_BROADCAST) {};

        Header(const Type & type, const Address & src, const Address & dst)
        : _frame_control(type), _sequence_number(Random::random()), _dst_pan_id(PAN_ID_BROADCAST), _dst(dst), _src(src) { ack_request(dst != broadcast()); }

        const Address & src() const { return _src; }
        const Address & dst() const { return _dst; }

        void sequence_number(Reg8 seq) { _sequence_number = seq; }
        const Reg8 & sequence_number() { return _sequence_number; }

        bool pending() const { return _frame_control.fp(); }
        unsigned int type() const { return _frame_control.type(); }

        void ack_request(bool val) { _frame_control.ar(val); }
        bool ack_request() { return _frame_control.ar(); }

        friend Debug & operator<<(Debug & db, const Header & h) {
            db << "{" << "fc=" << h._frame_control << ",sn=" << h._sequence_number << ",pid=" << h._dst_pan_id << ",dst=" << h._dst << ",src=" << h._src << "," << "}";
            return db;
        }

    protected:
        Frame_Control _frame_control;
        Reg8 _sequence_number;
        Reg16 _dst_pan_id;
        Address _dst;
        Address _src;
    } __attribute__((packed));

    class Frame: public Header
    {
    public:
        static const unsigned int MTU = IEEE802_15_4::MTU - sizeof(Header) - sizeof(CRC);
        typedef unsigned char Data[MTU];

    public:
        Frame() {}
        Frame(const Type & type, const Address & src, const Address & dst): Header(type, src, dst) {}
        Frame(const Type & type, const Address & src, const Address & dst, const void * data, unsigned int size)
        : Header(type, src, dst) {
            assert(size <= MTU);
            memcpy(_data, data, size);
        }

        Header * header() { return this; }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        friend Debug & operator<<(Debug & db, const Frame & f) {
            db << "{h=" << reinterpret_cast<const Header &>(f) << ",d=" << f._data << "}";
            return db;
        }

    protected:
        Data _data;
        CRC _crc;
    } __attribute__((packed));

    typedef Frame PDU;

    // Buffers used to hold frames across a zero-copy network stack
    typedef _UTIL::Buffer<NIC, IF<Traits<_SYS::TSTP>::enabled, Phy_Frame, Frame>::Result, void, NIC_Common::Metadata> Buffer;

    // Observers of a protocol get a also a pointer to the received buffer
    typedef Data_Observer<Buffer, Type> Observer;
    typedef Data_Observed<Buffer, Type> Observed;

    // Meaningful statistics for IEEE 802.15.4
    struct Statistics: public NIC_Common::Statistics
    {
        Statistics(): rx_overruns(0), tx_overruns(0), frame_errors(0), carrier_errors(0), collisions(0) {}

        friend Debug & operator<<(Debug & db, const Statistics & s) {
            db << "{rxp=" << s.rx_packets
               << ",rxb=" <<  s.rx_bytes
               << ",rxorun=" <<  s.rx_overruns
               << ",txp=" <<  s.tx_packets
               << ",txb=" <<  s.tx_bytes
               << ",txorun=" <<  s.tx_overruns
               << ",frm=" <<  s.frame_errors
               << ",car=" <<  s.carrier_errors
               << ",col=" <<  s.collisions
               << "}";
            return db;
        }

        unsigned int rx_overruns;
        unsigned int tx_overruns;
        unsigned int frame_errors;
        unsigned int carrier_errors;
        unsigned int collisions;
    };

protected:
    IEEE802_15_4() {}

public:
    static const unsigned int mtu() { return Frame::MTU; }
    static const Address broadcast() { return Address::BROADCAST; }
};

__END_SYS

#endif
