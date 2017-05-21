// EPOS TI CC2538 IEEE 802.15.4 NIC Mediator Test Program

#include <utility/ostream.h>
#include <nic.h>
#include <gpio.h>
#include <periodic_thread.h>

using namespace EPOS;

OStream cout;

const static unsigned int delay_time = 2000000;
const static bool use_receive = true;

bool led_value;
GPIO * led;
NIC::Address peer;

int sender(NIC * nic)
{
    char data[] = "0 Hello, World!";
    CPU::int_enable();
    cout << "Hello, I am the sender." << endl;
    cout << "I will send a message every " << delay_time << " microseconds." << endl;
    while(1)
    {
        cout << "Sending message: " << data << endl;
        cout << "to " << peer << endl;
        int send_result = nic->send(peer, NIC::PTP, data, 16);
        cout << "send result = " << send_result << endl;

        if(send_result <= 0)
            peer = nic->broadcast();

        data[0] = ((data[0] - '0' + 1) % 10) + '0';

        Periodic_Thread::wait_next();
    }
    return 0;
}

class Receiver : public IEEE802_15_4::Observer
{
public:
    typedef char data_type;

    typedef IEEE802_15_4::Protocol Protocol;
    typedef IEEE802_15_4::Buffer Buffer;
    typedef IEEE802_15_4::Frame Frame;
    typedef IEEE802_15_4::Observed Observed;

    Receiver(const Protocol & p, NIC * nic) : _prot(p), _nic(nic)
    {
        _nic->attach(this, _prot);
    }

    void update(Observed * o, Protocol p, Buffer * b)
    {
        cout << "Received buffer " << reinterpret_cast<void *>(b) << endl;
        led_value = !led_value;
        led->set(led_value);
        Frame * f = reinterpret_cast<Frame *>(b->frame());
        auto d = f->data<data_type>();
        auto from = f->src();
        peer = from;
        cout << endl << "=====================" << endl;
        cout << "Received " << b->size() << " bytes of payload from " << from << " :" << endl;
        for(int i=0; i<b->size()/sizeof(data_type); i++)
            cout << d[i] << " ";
        cout << endl << "=====================" << endl;
        _nic->free(b);
    }

private:
    Protocol _prot;
    NIC * _nic;
};

int receive(NIC * nic)
{
    CPU::int_enable();
    Receiver::data_type data[128];
    NIC::Protocol prot;
    NIC::Address from;

    while(true) {
        int size;
        size = 0;
        while(!size) {
            Periodic_Thread::wait_next();
            cout << "Receiver woke up!" << endl;
            size = nic->receive(&from, &prot, data, 128);
        }
        led_value = !led_value;
        led->set(led_value);
        cout << endl << "=====================" << endl;
        cout << "Received " << size << " bytes of payload from " << from << " :" << endl;
        for(int i=0; i<size/sizeof(Receiver::data_type); i++)
            cout << data[i] << " ";
        cout << endl << "=====================" << endl;
        peer = from;
    }
}

int main()
{
    cout << "CC2538 Radio test" << endl;

    led = new GPIO('C',3, GPIO::OUT);
    led_value = true;
    led->set(led_value);

    NIC * nic = new NIC();
    NIC::Address me = nic->address();
    me[0] = 0xaa;
    me[1] = 1;
    nic->address(me);

    cout << "My address is " << nic->address() << endl;
    peer = nic->broadcast();

    if(use_receive)
        new Periodic_Thread(delay_time/2, receive, nic);
    else
        Receiver * r = new Receiver(NIC::PTP, nic);

    Periodic_Thread * sender_thread = new Periodic_Thread(delay_time, sender, nic);

    sender_thread->join();

    return 0;
}
