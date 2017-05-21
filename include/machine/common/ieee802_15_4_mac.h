// EPOS IEEE 802.15.4 MAC Declarations

#ifndef __ieee802_15_4_mac_h
#define __ieee802_15_4_mac_h

#include <utility/random.h>
#include <utility/math.h>
#include <ieee802_15_4.h>

__BEGIN_SYS

template<typename Radio>
class IEEE802_15_4_MAC: public IEEE802_15_4, public Radio
{
private:
    static const unsigned int CSMA_CA_MIN_BACKOFF_EXPONENT = 3;
    static const unsigned int CSMA_CA_MAX_BACKOFF_EXPONENT = 5;
    static const unsigned int CSMA_CA_UNIT_BACKOFF_PERIOD = 320; // us
    static const unsigned int CSMA_CA_RETRIES = Traits<_API::ELP>::RETRIES > 4 ? 4 : Traits<_API::ELP>::RETRIES;

    static const unsigned int ACK_TIMEOUT = 352 * 2;

    static const bool acknowledged = Traits<_API::ELP>::acknowledged;

public:
    using IEEE802_15_4::Address;
    using IEEE802_15_4::Header;
    using IEEE802_15_4::Frame;

    static const unsigned int MTU = IEEE802_15_4::MTU - sizeof(Header);

    typedef _UTIL::Buffer<NIC, Frame, void, NIC_Common::Metadata> Buffer;

protected:
    IEEE802_15_4_MAC(unsigned int unit) {}

    // Called after the Radio's constructor
    void constructor_epilogue() {
        Radio::power(Power_Mode::FULL);
        Radio::listen();
    }

    // Filter and assemble RX Buffer Metainformation
    bool pre_notify(Buffer * buf) { return true; }

    bool post_notify(Buffer * buf) { return false; }

public:
    // Assemble TX Buffer Metainformation and MAC Header
    void marshal(Buffer * buf, const Address & src, const Address & dst, const Type & type) {
        Frame * frame = new (buf->frame()) Frame(type, src, dst);
        frame->ack_request(acknowledged && dst != broadcast());
    }

    unsigned int unmarshal(Buffer * buf, Address * src, Address * dst, Type * type, void * data, unsigned int size) {
        Frame * frame = buf->frame();
        unsigned int data_size = buf->size() - sizeof(Header);
        if(size > data_size)
            size = data_size;
        *src = frame->src();
        *dst = frame->dst();
        *type = frame->type();
        memcpy(data, frame->data<void>(), size);
        return size;
    }

    int send(Buffer * buf) {
        bool do_ack = acknowledged && buf->frame()->ack_request();

        Radio::power(Power_Mode::LIGHT);

        Radio::copy_to_nic(buf->frame(), buf->size());
        bool sent, ack_ok;
        ack_ok = sent = backoff_and_send();

        if(do_ack) {
            if(sent) {
                Radio::power(Power_Mode::FULL);
                ack_ok = Radio::wait_for_ack(ACK_TIMEOUT, buf->frame()->sequence_number());
            }

            for(unsigned int i = 0; !ack_ok && (i < CSMA_CA_RETRIES); i++) {
                Radio::power(Power_Mode::LIGHT);
                db<IEEE802_15_4_MAC>(TRC) << "IEEE802_15_4_MAC::retransmitting" << endl;
                ack_ok = sent = backoff_and_send();
                if(sent) {
                    Radio::power(Power_Mode::FULL);
                    ack_ok = Radio::wait_for_ack(ACK_TIMEOUT, buf->frame()->sequence_number());
                }
            }

            if(!sent)
                Radio::power(Power_Mode::FULL);

        } else {
            if(sent)
                while(!Radio::tx_done());
            Radio::power(Power_Mode::FULL);
        }

        unsigned int size = buf->size();

        delete buf;

        return ack_ok ? size : 0;
    }

private:
    bool backoff_and_send() {
        unsigned int exp = CSMA_CA_MIN_BACKOFF_EXPONENT;
        unsigned int backoff = pow(2, exp);

        unsigned int retry = 0;
        for(; (retry < CSMA_CA_RETRIES) ; retry++) {
            unsigned int time = (Random::random() % backoff) * CSMA_CA_UNIT_BACKOFF_PERIOD;
            if(time < CSMA_CA_UNIT_BACKOFF_PERIOD)
                time = CSMA_CA_UNIT_BACKOFF_PERIOD;

            Radio::backoff(time);
            if(Radio::cca(CSMA_CA_UNIT_BACKOFF_PERIOD) && Radio::transmit())
                break; // Success

            if(exp < CSMA_CA_MAX_BACKOFF_EXPONENT) {
                exp++;
                backoff *= 2;
            }
        }
        return (retry < CSMA_CA_RETRIES);
    }
};

__END_SYS

#endif
