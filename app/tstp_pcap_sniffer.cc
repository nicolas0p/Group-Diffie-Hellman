#include <alarm.h>
#include <tstp.h>
#include <pcap.h>
#include <semaphore.h>

using namespace EPOS;

class TSTP_PCAP_Sniffer: private NIC::Observer
{
    typedef IF<Traits<USB>::enabled, USB, UART>::Result Out;
    typedef Traits<NIC>::NICS::Get<0>::Result::Timer Timer;

public:
    class Packet
    {
    public:
        typedef Simple_List<Packet> List;
        typedef List::Element Element;

        Packet(NIC::Buffer * buf):
            _header(TSTP::absolute(Timer::count2us(buf->sfd_time_stamp)), buf->size()),
            _link(this) {
                if(buf->is_microframe) {
                    TSTP::Microframe * mf = buf->frame()->data<TSTP::Microframe>();
                    mf->all_listen(buf->downlink);
                    mf->count(buf->microframe_count);
                }
                memcpy(_data, buf->frame()->data<const char>(), buf->size());
            }

        Element * lext() { return &_link; }
        unsigned int size() { return sizeof(PCAP::Packet_Header) + _header.size(); }

    private:
        PCAP::Packet_Header _header;
        unsigned char _data[IEEE802_15_4::MTU];
        Element _link;
    };

private:
    typedef Semaphore_Observer<Packet, int> Observer;
    typedef Semaphore_Observed<Packet, int> Observed;

public:
    TSTP_PCAP_Sniffer() {
        Out out;
        PCAP::Global_Header g(IEEE802_15_4::MTU, PCAP::IEEE802_15_4);
        for(unsigned int i = 0; i < sizeof(PCAP::Global_Header); i++)
            out.put(reinterpret_cast<const char*>(&g)[i]);
        attach(&_observer, PCAP::IEEE802_15_4);
        _nic.attach(this, NIC::TSTP);
    }

    Packet * updated() { return _observer.updated(); }

    void update(NIC::Observed * obs, NIC::Protocol prot, NIC::Buffer * buf) { notify(PCAP::IEEE802_15_4, new Packet(buf)); }

private:
    static bool notify(int t, Packet * p) { return _observed.notify(t, p); }
    static void attach(Observer * obs, int t) { _observed.attach(obs, t); }
    static void detach(Observer * obs, int t) { _observed.detach(obs, t); }

private:
    NIC _nic;
    static Observed _observed;
    Observer _observer;
};

TSTP_PCAP_Sniffer::Observed TSTP_PCAP_Sniffer::_observed;

int main()
{
    Machine::delay(5000000);

    TSTP_PCAP_Sniffer sniffer;
    USB out;

    while(true) {
        TSTP_PCAP_Sniffer::Packet * p = sniffer.updated();
        for(unsigned int i = 0; i < p->size(); i++)
            out.put(reinterpret_cast<const char*>(p)[i]);
        delete p;
    }

    return 0;
}
