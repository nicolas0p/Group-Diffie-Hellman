// EPOS Trustful SpaceTime Protocol MAC Declarations

#ifndef __tstp_mac_h
#define __tstp_mac_h

// Include only TSTP_Common from tstp.h
#ifdef __tstp_h
#include <tstp.h>
#else
#define __tstp_h
#include <tstp.h>
#undef __tstp_h
#endif

#include <ic.h>
#include <utility/random.h>
#include <utility/math.h>
#include <watchdog.h>

__BEGIN_SYS

template<typename Radio>
class TSTP_MAC: public TSTP_Common, public Radio
{
    typedef IEEE802_15_4 Phy_Layer;

    enum State {
        UPDATE_TX_SCHEDULE = 0,
        SLEEP_S            = 1,
        RX_MF              = 2,
        SLEEP_DATA         = 3,
        RX_DATA            = 4,
        BACKOFF            = 5,
        CCA                = 6,
        TX_MF              = 7,
        TX_DATA            = 8,
    };

public:
    using TSTP_Common::Address;
    using TSTP_Common::Header;
    using TSTP_Common::Frame;
    typedef typename Radio::Timer Timer;
    typedef typename Radio::Timer::Time_Stamp Time_Stamp;

    static const unsigned int MTU = Frame::MTU;

private:
public: //TODO: for debugging

    static const bool sniffer = Traits<NIC>::promiscuous;
    static const bool state_machine_debugged = false;

    static const unsigned int INT_HANDLING_DELAY = 9; // Time delay between scheduled tx_mf interrupt and actual Radio TX
    static const unsigned int TX_DELAY = INT_HANDLING_DELAY + Radio::RX_TO_TX_DELAY;

    static const unsigned int Tu = IEEE802_15_4::TURNAROUND_TIME;
    static const unsigned int Ti = Tu + Radio::RX_TO_TX_DELAY + INT_HANDLING_DELAY + 100; // 100us margin for delay between Microframes // FIXME
    static const unsigned int TIME_BETWEEN_MICROFRAMES = Ti;
    static const unsigned int Ts = (sizeof(Microframe) + Phy_Layer::PHY_HEADER_SIZE) * 1000000ull
                                    / Phy_Layer::BYTE_RATE
                                    + Radio::TX_TO_RX_DELAY; // Time to send a single Microframe (including PHY headers)
    static const unsigned int MICROFRAME_TIME = Ts;
    static const unsigned int Tr = 2*Ts + Ti + Tu;
    static const unsigned int RX_MF_TIMEOUT = Tr;

    static const unsigned int NMF = 1 + (((1000000ull * Tr) / Traits<System>::DUTY_CYCLE) + (Ti + Ts) - 1) / (Ti + Ts);
    static const unsigned int N_MICROFRAMES = NMF;

    static const unsigned int CI = Ts + (NMF - 1) * (Ts + Ti);
    static const unsigned int PERIOD = CI;
    static const unsigned int SLEEP_PERIOD = CI - RX_MF_TIMEOUT;

    static const typename IF<(Tr * 1000000ull / CI <= Traits<System>::DUTY_CYCLE), unsigned int, void>::Result
        DUTY_CYCLE = Tr * 1000000ull / CI; // in ppm. This line failing means that TSTP_MAC is unable to provide a duty cycle smaller than or equal to Traits<System>::DUTY_CYCLE

    // TODO
    static const unsigned int DATA_LISTEN_MARGIN = (TIME_BETWEEN_MICROFRAMES + MICROFRAME_TIME) * 5; // Subtract this amount when calculating time until data transmission
    static const unsigned int DATA_SKIP_TIME = DATA_LISTEN_MARGIN + 4500;

    static const unsigned int RX_DATA_TIMEOUT = DATA_SKIP_TIME + DATA_LISTEN_MARGIN;

    static const unsigned int G = IEEE802_15_4::CCA_TX_GAP;
    static const unsigned int CCA_TIME = Ti + G;

protected:
    TSTP_MAC(unsigned int unit) : _unit(unit) {
        db<TSTP_MAC<Radio>, Init>(TRC) << "TSTP_MAC(u=" << unit << ")" << endl;
    }

    // Called after the Radio's constructor
    void constructor_epilogue() {
        db<TSTP_MAC<Radio>, Init>(TRC) << "TSTP_MAC::constructor_epilogue()" << endl;
        if(sniffer)
            sniff(0);
        else {
            Watchdog::enable();
            update_tx_schedule(0);
        }
    }

    static void sniff(const IC::Interrupt_Id & id) {
        Radio::power(Power_Mode::FULL);
        Radio::listen();
    }

    // Filter and assemble RX Buffer Metainformation
    bool pre_notify(Buffer * buf) {
        CPU::int_disable();
        if(Traits<TSTP_MAC>::hysterically_debugged)
            db<TSTP_MAC<Radio>>(TRC) << "pre_notify(buf=" << buf << ")" << endl;

        if(sniffer) {
            static unsigned int last_id = 0;
            static unsigned int last_hint = 0;
            if(buf->size() == sizeof(Microframe)) {
                Microframe * mf = buf->frame()->data<Microframe>();
                if((mf->id() != last_id) || (mf->hint() != last_hint)) {
                    last_id = mf->id();
                    last_hint = mf->hint();

                    buf->id = last_id;
                    buf->downlink = mf->all_listen();
                    buf->is_new = false;
                    buf->is_microframe = true;
                    buf->microframe_count = mf->count();

                    // Forge a TSTP identifier to make the radio notify listeners
                    mf->all_listen(false);
                    mf->count(TSTP_Common::V0 >> 1);

                    CPU::int_enable();
                    return true;
                }
                CPU::int_enable();
                return false;
            } else {
                last_id = 0;
                last_hint = 0;
                buf->is_microframe = false;
                CPU::int_enable();
                return true;
            }
            CPU::int_enable();
            return false;
        }

        if(_in_rx_mf) { // State: RX MF (part 2/3)
                                                    // TODO: I don't know why, but some MFs with a huge count are getting here
            if((buf->size() == sizeof(Microframe)) && (buf->frame()->data<Microframe>()->count() < N_MICROFRAMES)) {

                Timer::int_disable();

                Radio::power(Power_Mode::SLEEP);

                _in_rx_mf = false;

                Microframe * mf = buf->frame()->data<Microframe>();
                Frame_ID id = mf->id();

                // Clear scheduled messages with same ID
                for(Buffer::Element * el = _tx_schedule.head(); el; el = el->next()) {
                    Buffer * b = el->object();
                    if(b->id == id) {

                        // Frame ID seen before. Skip it.

                        if(!b->destined_to_me) {
                            db<TSTP_MAC<Radio>>(INF) << "TSTP_MAC::pre_notify: ACK received: " << *mf << " at " << Radio::Timer::count2us(buf->sfd_time_stamp) << endl;
                            _tx_schedule.remove(el);
                            delete b;
                        }

                        Time_Stamp data_time = buf->sfd_time_stamp + Timer::us2count(TIME_BETWEEN_MICROFRAMES) + mf->count() * Timer::us2count(TIME_BETWEEN_MICROFRAMES + MICROFRAME_TIME) - Timer::us2count(DATA_LISTEN_MARGIN);

                        Watchdog::kick();
                        if(state_machine_debugged)
                            kout << SLEEP_DATA ;
                        // State: Sleep until Data
                        Timer::interrupt(data_time + Timer::us2count(DATA_SKIP_TIME), update_tx_schedule);

                        CPU::int_enable();
                        return false;
                    }
                }

                // Initialize Buffer Metainformation
                buf->id = id;
                buf->downlink = mf->all_listen();
                buf->is_new = false;
                buf->is_microframe = true;
                buf->relevant = sniffer || mf->all_listen();
                buf->trusted = false;
                buf->sender_distance = mf->hint();
                buf->microframe_count = mf->count();

                // Forge a TSTP identifier to make the radio notify listeners
                mf->all_listen(false);
                mf->count(TSTP_Common::V0 >> 1);

                CPU::int_enable();
                return true;
            }
            CPU::int_enable();
            return false;
        } else if(_in_rx_data) { // State: RX Data (part 2/3)

            if((buf->size() == sizeof(Microframe)) || (id(buf) != _receiving_data_id)) {
                CPU::int_enable();
                return false;
            }

            Radio::power(Power_Mode::SLEEP);

            // Initialize Buffer Metainformation
            buf->id = _receiving_data_id;
            buf->sender_distance = _receiving_data_hint;
            buf->is_new = false;
            buf->is_microframe = false;
            buf->trusted = false;
            buf->freed = false;
            buf->attempts = 0;
            buf->microframe_count = 0;

            db<TSTP_MAC<Radio>>(TRC) << "TSTP_MAC::pre_notify: Frame received: " << buf->frame() << " at " << Radio::Timer::count2us(buf->sfd_time_stamp) << endl;

            CPU::int_enable();
            return true;
        } else {
            CPU::int_enable();
            return false;
        }
    }

    bool post_notify(Buffer * buf) {
        if(Traits<TSTP_MAC>::hysterically_debugged)
            db<TSTP_MAC<Radio>>(TRC) << "post_notify(buf=" << buf << ")" << endl;

        if(buf->is_microframe && !sniffer) { // State: RX MF (part 3/3)
            Time_Stamp data_time = buf->sfd_time_stamp + Timer::us2count(TIME_BETWEEN_MICROFRAMES) + buf->microframe_count * Timer::us2count(TIME_BETWEEN_MICROFRAMES + MICROFRAME_TIME) - Timer::us2count(DATA_LISTEN_MARGIN);

            Watchdog::kick();
            if(state_machine_debugged)
                kout << SLEEP_DATA;

            if(buf->relevant) { // Transition: [Relevant MF]
                _receiving_data_id = buf->id;
                _receiving_data_hint = buf->sender_distance;
                // State: Sleep until Data
                Timer::interrupt(data_time, rx_data);
            } else // Transition: [Irrelevant MF]
                Timer::interrupt(data_time + Timer::us2count(DATA_SKIP_TIME), update_tx_schedule);

            if(Traits<TSTP_MAC>::debugged) {
                Microframe * mf = buf->frame()->data<Microframe>();
                // Rebuild Microframe for printing
                mf->all_listen(buf->downlink);
                mf->count(buf->microframe_count);
                db<TSTP_MAC<Radio>>(INF) << "[RX][TSTP Microframe] " << *mf << endl;
            }
        }

        free(buf);
        return true;
    }

public:
    // TODO
    Frame_ID id(Buffer * buf) {
        Frame_ID ret = 0;
        for(unsigned int i = 0; i < 5; i++)
            ret ^= buf->frame()->data<Frame_ID>()[i+1];
        return ret & 0x0fff;
    }

    // Assemble TX Buffer Metainformation
    void marshal(Buffer * buf, const Address & src, const Address & dst, const Type & type) {
        buf->is_microframe = false;
        buf->trusted = false;
        buf->is_new = true;
        buf->attempts = 0;
        buf->microframe_count = 0;
    }

    unsigned int unmarshal(Buffer * buf, Address * src, Address * dst, Type * type, void * data, unsigned int size) {
        *src = Address::BROADCAST;
        *dst = Address::BROADCAST;
        *type = buf->frame()->data<Header>()->version();
        memcpy(data, buf->frame()->data<Frame>(), min(buf->size(), size));
        return buf->size();
    }

    int send(Buffer * buf) {
        buf->id = id(buf);

        if(sniffer) {
            delete buf;
            return 0;
        } else {
            // Components calculate the offset in microseconds according to their own metrics.
            // We finish the calculation here to keep SLEEP_PERIOD, G, and Timestamps
            // encapsulated by the MAC, and MAC::marshal() happens before the other components' marshal methods
            buf->offset = Timer::us2count(((buf->offset * SLEEP_PERIOD) / (G*RADIO_RANGE)) * G);

            if(buf->offset < Timer::us2count(3*CCA_TIME))
                buf->offset = Timer::us2count(3*CCA_TIME);
            else if(buf->offset > Timer::us2count(SLEEP_PERIOD - CCA_TIME))
                buf->offset = Timer::us2count(SLEEP_PERIOD - CCA_TIME);

            // Check if we have a message with the same ID
            CPU::int_disable();
            Buffer::Element * next;
            for(Buffer::Element * el = _tx_schedule.head(); el; el = next) {
                next = el->next();
                Buffer * b = el->object();
                if(b->id == buf->id) {
                    unsigned int ret = buf->size();
                    delete buf;
                    CPU::int_enable();
                    return ret;
                }
            }
            _tx_schedule.insert(buf->link());
            CPU::int_enable();

            return buf->size();
        }
    }

private:
    // State Machine

    static void update_tx_schedule(const IC::Interrupt_Id & id) {
        Timer::int_disable();
        if(state_machine_debugged)
            kout << UPDATE_TX_SCHEDULE;
        Watchdog::kick();
        if(Traits<TSTP_MAC>::hysterically_debugged)
            db<TSTP_MAC<Radio>>(TRC) << "TSTP_MAC::update_tx_schedule(id=" << id << ")" << endl;

        // State: Update TX Schedule
        Radio::power(Power_Mode::SLEEP);
        _in_rx_data = false;
        _in_rx_mf = false;

        _tx_pending = 0;

        Time_Stamp now_ts = Timer::read();
        Microsecond now_us = Timer::count2us(now_ts);

        CPU::int_disable();
        // Fetch next message and remove expired ones
        // TODO: Turn _tx_schedule into an ordered list
        for(Buffer::Element * el = _tx_schedule.head(); el; ) {
            Buffer::Element * next = el->next();
            Buffer * b = el->object();
            // Message was created in the future. This might happen when Timekeeper adjusts the timer.
            if(b->frame()->data<Header>()->time() > now_us) {
                _tx_schedule.remove(el);
                delete b;
            } else if(drop_expired && (b->deadline <= now_us)) {
                _tx_schedule.remove(el);
                delete b;
            } else if((!_tx_pending) || ((!_tx_pending->destined_to_me) && ((b->destined_to_me) || (b->attempts < _tx_pending->attempts) || (b->deadline < _tx_pending->deadline))))
                _tx_pending = b;
            el = next;
        }
        CPU::int_enable();

        if(_tx_pending) { // Transition: [TX pending]
            // State: Backoff CCA (Backoff part)
            new (&_mf) Microframe((!_tx_pending->destined_to_me) && _tx_pending->downlink, _tx_pending->id, N_MICROFRAMES - 1, _tx_pending->my_distance);
            Radio::power(Power_Mode::LIGHT);
            Radio::copy_to_nic(&_mf, sizeof(Microframe));

            Watchdog::kick();
            if(state_machine_debugged)
                kout << BACKOFF ;

            // Increase ACK priority and decrease non-ACK priority by a random component,
            // based on number of transmission attempts.
            // This prevents permanent inteference by a given pair of nodes, and
            // makes unresponded messages have the lowest priorities
            _tx_pending->attempts++;
            unsigned long long offset;
            offset = _tx_pending->offset;
            unsigned int lim = pow(2u, _tx_pending->attempts);
            if(_tx_pending->destined_to_me) {
                offset -= (Random::random() % lim) * Timer::us2count(CCA_TIME);
                if((offset < Timer::us2count(CCA_TIME)) || (offset > Timer::us2count(SLEEP_PERIOD - CCA_TIME))) {
                    offset = Timer::us2count(CCA_TIME);
                    _tx_pending->attempts--;
                }
            } else {
                offset += (Random::random() % lim) * Timer::us2count(CCA_TIME);
                if((offset < Timer::us2count(3*CCA_TIME)) || (offset > Timer::us2count(SLEEP_PERIOD - CCA_TIME))) {
                    offset = Timer::us2count(SLEEP_PERIOD - CCA_TIME);
                    _tx_pending->attempts--;
                }
            }

            Timer::interrupt(now_ts + offset, cca);
        } else { // Transition: [No TX pending]
            // State: Sleep S
            if(state_machine_debugged)
                kout << SLEEP_S ;
            Watchdog::kick();
            Timer::interrupt(now_ts + Timer::us2count(SLEEP_PERIOD), rx_mf);
        }
    }

    // State: Backoff CCA (CCA part)
    static void cca(const IC::Interrupt_Id & id) {
        if(state_machine_debugged)
            kout << CCA ;
        Watchdog::kick();
        if(Traits<TSTP_MAC>::hysterically_debugged)
            db<TSTP_MAC<Radio>>(TRC) << "TSTP_MAC::cca(id=" << id << ")" << endl;

        assert(N_MICROFRAMES > 1);

        Radio::listen();

        // Try to send the first Microframe
        if(Radio::cca(CCA_TIME)) {
            _mf_time = Timer::read();
            if(Radio::transmit()) { // Transition: [Channel free]
                _mf_time += Timer::us2count(TIME_BETWEEN_MICROFRAMES + MICROFRAME_TIME);
                _mf.dec_count();
                if(state_machine_debugged)
                    kout << TX_MF ;
                Watchdog::kick();
                while(!Radio::tx_done());
                Radio::copy_to_nic(&_mf, sizeof(Microframe));
                //tx_mf(0);
                Timer::interrupt(_mf_time, tx_mf);
            } else { // Transition: [Channel busy]
                rx_mf(0);
            }
        } else { // Transition: [Channel busy]
            rx_mf(0);
        }
    }

    // State: RX MF (part 1/3)
    static void rx_mf(const IC::Interrupt_Id & id) {
        if(state_machine_debugged)
            kout << RX_MF ;
        Watchdog::kick();
        if(Traits<TSTP_MAC>::hysterically_debugged)
            db<TSTP_MAC<Radio>>(TRC) << "TSTP_MAC::rx_mf(id=" << id << ")" << endl;

        _in_rx_mf = true;

        // If timeout is reached, Transition: [No MF]
        Timer::interrupt(Timer::read() + Timer::us2count(RX_MF_TIMEOUT), update_tx_schedule);

        Radio::power(Power_Mode::FULL);
        Radio::listen();
    }

    // State: RX Data (part 1/3)
    static void rx_data(const IC::Interrupt_Id & id) {
        if(state_machine_debugged)
            kout << RX_DATA ;
        Watchdog::kick();
        if(Traits<TSTP_MAC>::hysterically_debugged)
            db<TSTP_MAC<Radio>>(TRC) << "TSTP_MAC::rx_data(id=" << id << ")" << endl;

        _in_rx_data = true;
        _in_rx_mf = false;

        // Set timeout
        Timer::interrupt(Timer::read() + Timer::us2count(RX_DATA_TIMEOUT), update_tx_schedule);

        Radio::power(Power_Mode::FULL);
        Radio::listen();
    }

    // State: TX MFs
    static void tx_mf(const IC::Interrupt_Id & id) {
        if(Traits<TSTP_MAC>::hysterically_debugged)
            db<TSTP_MAC<Radio>>(TRC) << "TSTP_MAC::tx_mf()" << endl;

        //while(true) {

            //while(Timer::read() < _mf_time);

            // The first Microframe is sent at cca()
            Radio::transmit_no_cca();

            _mf_time += Timer::us2count(TIME_BETWEEN_MICROFRAMES + MICROFRAME_TIME);

            Watchdog::kick();
            //if(state_machine_debugged)
                //kout << TX_MF;

            if(_mf.dec_count() > 0) {
                while(!Radio::tx_done());
                Radio::copy_to_nic(&_mf, sizeof(Microframe));
                Timer::interrupt(_mf_time, tx_mf);
            } else {
                // The precise time when this frame is actually sent by the physical layer
                _tx_pending->frame()->data<Header>()->last_hop_time(_mf_time + Timer::us2count(TX_DELAY + Tu));
                while(!Radio::tx_done());
                Radio::copy_to_nic(_tx_pending->frame(), _tx_pending->size());
                Timer::interrupt(_mf_time, tx_data);
                //while(Timer::read() < _mf_time);
                //tx_data(0);
                //return;
            }
        //}
    }

    static void tx_data(const IC::Interrupt_Id & id) {
        if(state_machine_debugged)
            kout << TX_DATA;
        if(Traits<TSTP_MAC>::hysterically_debugged)
            db<TSTP_MAC<Radio>>(TRC) << "TSTP_MAC::tx_data()" << endl;

        if(!_tx_pending->destined_to_me) { // Transition: [Is not dest.]
            // State: TX Data
            Radio::transmit_no_cca();
            Watchdog::kick();
            while(!Radio::tx_done());

            _mf_time = Timer::read();

            // Keep Alive messages are never ACK'ed or forwarded
            if((_tx_pending->frame()->data<Header>()->type() == CONTROL) && (_tx_pending->frame()->data<Control>()->subtype() == KEEP_ALIVE)) {
                CPU::int_disable();
                _tx_schedule.remove(_tx_pending);
                delete _tx_pending;
                CPU::int_enable();
            }
        } else { // Transition: [Is dest.]
            CPU::int_disable();
            Watchdog::kick();
            _tx_schedule.remove(_tx_pending);
            delete _tx_pending;
            CPU::int_enable();
        }

        // State: Sleep S
        Radio::power(Power_Mode::SLEEP);
        Watchdog::kick();
        if(state_machine_debugged)
            kout << SLEEP_S;
        Timer::interrupt(_mf_time + Timer::us2count(SLEEP_PERIOD), rx_mf);
    }

    void free(Buffer * b);

    static Microframe _mf;
    static Time_Stamp _mf_time;
    static Frame_ID _receiving_data_id;
    static Hint _receiving_data_hint;
    static Buffer::List _tx_schedule;
    static Buffer * _tx_pending;
    static bool _in_rx_mf;
    static bool _in_rx_data;

    unsigned int _unit;
};

// The compiler makes sure that template static variables are only defined once

// Class attributes
template<typename Radio>
TSTP_Common::Microframe TSTP_MAC<Radio>::_mf;

template<typename Radio>
typename TSTP_MAC<Radio>::Time_Stamp TSTP_MAC<Radio>::_mf_time;

template<typename Radio>
TSTP_Common::Frame_ID TSTP_MAC<Radio>::_receiving_data_id;

template<typename Radio>
typename TSTP_MAC<Radio>::Hint TSTP_MAC<Radio>::_receiving_data_hint;

template<typename Radio>
TSTP_MAC<Radio>::Buffer::List TSTP_MAC<Radio>::_tx_schedule;

template<typename Radio>
typename TSTP_MAC<Radio>::Buffer * TSTP_MAC<Radio>::_tx_pending;

template<typename Radio>
bool TSTP_MAC<Radio>::_in_rx_mf;

template<typename Radio>
bool TSTP_MAC<Radio>::_in_rx_data;

__END_SYS

#endif
