// EPOS Network Interface Mediator Common Package

#ifndef __nic_h
#define __nic_h

#include <utility/string.h>
#include <cpu.h>
#include <rtc.h>

__BEGIN_SYS

class NIC_Common
{
protected:
    NIC_Common() {}

public:
    // NIC physical address (e.g. MAC)
    template<unsigned int LENGTH>
    class Address
    {
    public:
        enum Null { NULL = 0 };
        enum Broadcast { BROADCAST = 255 };

    public:
        Address() {}

        Address(const Null &) {
            for(unsigned int i = 0; i < LENGTH; i++)
                _address[i] =  NULL;
        }

        Address(const Broadcast &) {
            for(unsigned int i = 0; i < LENGTH; i++)
                _address[i] = BROADCAST;
        }

        Address(const char * str) { // String formated as A.B.C.D or A:B:C:D:E:F
            static const char sep = (LENGTH == 4) ? '.' : ':';
            char * token = strchr(str, sep);
            for(unsigned int i = 0; i < LENGTH; i++, ++token, str = token, token = strchr(str, sep))
                _address[i] = atol(str);
        }

        Address(unsigned long a) {
            assert(LENGTH == sizeof(long));
            a = htonl(a);
            memcpy(this, &a, sizeof(long));
        }

        Address operator=(const Address & a) {
            for(unsigned int i = 0; i < LENGTH; ++i)
                _address[i] = a._address[i];
            return a;
        }
        Address operator=(const Address & a) volatile {
            for(unsigned int i = 0; i < LENGTH; ++i)
                _address[i] = a._address[i];
            return a;
        }

        operator bool() const {
            for(unsigned int i = 0; i < LENGTH; ++i) {
                if(_address[i])
                    return true;
            }
            return false;
        }
        operator bool() const volatile {
            for(unsigned int i = 0; i < LENGTH; ++i) {
                if(_address[i])
                    return true;
            }
            return false;
        }

        bool operator==(const Address & a) const {
            for(unsigned int i = 0; i < LENGTH; ++i) {
                if(_address[i] != a._address[i])
                    return false;
            }
            return true;
        }

        bool operator!=(const Address & a) const {
            for(unsigned int i = 0; i < LENGTH; ++i) {
                if(_address[i] != a._address[i])
                    return true;
            }
            return false;
        }

        Address operator&(const Address & a) const {
            Address ret;
            for(unsigned int i = 0; i < LENGTH; ++i)
                ret[i] = _address[i] & a._address[i];
            return ret;
        }

        Address operator|(const Address & a) const {
            Address ret;
            for(unsigned int i = 0; i < LENGTH; ++i)
                ret[i] = _address[i] | a._address[i];
            return ret;
        }

        Address operator~() const {
            Address ret;
            for(unsigned int i = 0; i < LENGTH; ++i)
                ret[i] = ~_address[i];
            return ret;
        }

        unsigned int operator%(unsigned int i) const {
            return _address[LENGTH - 1] % i;
        }

        unsigned char & operator[](const size_t i) { return _address[i]; }
        const unsigned char & operator[](const size_t i) const { return _address[i]; }

        friend OStream & operator<<(OStream & db, const Address & a) {
            db << hex;
            for(unsigned int i = 0; i < LENGTH; i++) {
                db << a._address[i];
                if(i < LENGTH - 1)
                    db << ((LENGTH == 4) ? "." : ":");
            }
            db << dec;
            return db;
        }

    private:
        unsigned char _address[LENGTH];
    } __attribute__((packed));

    // NIC protocol id
    typedef unsigned short Protocol;

    // NIC CRCs
    typedef unsigned short CRC16;
    typedef unsigned long CRC32;

    // NIC statistics
    struct Statistics
    {
        Statistics(): rx_packets(0), tx_packets(0), rx_bytes(0), tx_bytes(0) {}

        unsigned int rx_packets;
        unsigned int tx_packets;
        unsigned int rx_bytes;
        unsigned int tx_bytes;
    };

    // Buffer Metadata added to frames by higher-level protocols
    struct Metadata
    {
        union
        {
            // Note that the position of the structs matter. The TSTP field _r merges with the IEEE 802.15.4 field rssi.

            // IEEE 802.15.4 Metadata
            struct
            {
                int rssi;                          // Received Signal Strength Indicator
                unsigned long long sfd_time_stamp; // Start-of-frame reception time stamp
            };

            // TSTP Metadata
            struct
            {
                // TODO: remove unnecessary long longs
                int _rssi;                          // Received Signal Strength Indicator
                unsigned long long _sfd_time_stamp; // Start-of-frame reception time stamp
                unsigned int id;                    // Message identifier
                unsigned long long offset;          // MAC contention offset
                bool destined_to_me;                // Whether this node is the final destination for this message
                bool downlink;                      // Message direction (downlink == from sink to sensor)
                unsigned long long deadline;        // Time until when this message must arrive at the final destination
                unsigned int my_distance;           // This node's distance to the message's final destination
                unsigned int sender_distance;       // Last hop's distance to the message's final destination
                bool is_new;                        // Whether this message was just created by this node
                bool is_microframe;                 // Whether this message is a Microframe
                bool relevant;                      // Whether any component is interested in this message
                bool trusted;                       // If true, this message was successfully verified by the Security Manager
                bool freed;                         // If true, the MAC will not free this buffer
                unsigned int attempts;              // Number times the MAC tried to transmit this buffer
                unsigned int microframe_count;      // Number of Microframes left until data
            };
        };
    };

    // NIC Timer Interface (to be implemented by NIC aiming at supporting time-synchronous protocols)
    class Timer
    {
    private:
        typedef RTC::Microsecond Microsecond;

    public:
        typedef unsigned long long Time_Stamp;
        typedef long long Offset;

    public:
         Timer() {}

         static Time_Stamp frequency() { return 0; }
         static Time_Stamp read() { return 0; }
         static Time_Stamp sfd() { return 0; }
         static void adjust(const Offset & o) { }
         static Time_Stamp us2count(const Microsecond & us) { return 0; }
         static Microsecond count2us(const Time_Stamp & ts) { return 0; }
    };


    // Polymorphic NIC Base
    template<typename Family, bool polymorphic = true>
    class NIC_Base: public Family, public Family::Observed
    {
    public:
        NIC_Base(unsigned int unit = 0) {}
        virtual ~NIC_Base() {}

        virtual int send(const typename Family::Address & dst, const typename Family::Protocol & prot, const void * data, unsigned int size) = 0;
        virtual int receive(typename Family::Address * src, typename Family::Protocol * prot, void * data, unsigned int size) = 0;

        virtual typename Family::Buffer * alloc(NIC * nic, const typename Family::Address & dst, const typename Family::Protocol & prot, unsigned int once, unsigned int always, unsigned int payload) = 0;
        virtual int send(typename Family::Buffer * buf) = 0;
        virtual void free(typename Family::Buffer * buf) = 0;

        virtual const typename Family::Address & address() = 0;
        virtual void address(const typename Family::Address &) = 0;

        virtual const typename Family::Statistics & statistics() = 0;

        virtual void reset() = 0;
    };

    // Monomorphic NIC Base
    template<typename Family>
    class NIC_Base<Family, false>: public Family, public Family::Observed
    {
    public:
        NIC_Base(unsigned int unit = 0) {}
    };
};

__END_SYS

#ifdef __NIC_H
#include __NIC_H
#endif

#endif
