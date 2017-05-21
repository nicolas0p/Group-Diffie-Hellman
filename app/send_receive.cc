#include <utility/ostream.h>
#include <utility/random.h>
#include <nic.h>
#include <gpio.h>
#include <alarm.h>

using namespace EPOS;

OStream cout;

void busy_wait(int limit = 0x1fffff) { for(volatile int i=0; i<limit; i++); }

IEEE802_15_4::Address peer;

//const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'A','1'};

bool led_value;
GPIO * led;
class Receiver : public IEEE802_15_4::Observer
{
public:
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
        if(p == _prot)
        {
            led_value = !led_value;
            led->set(led_value);
            Frame * f = b->frame();
            char * d = f->data<char>();
            cout << endl << "=====================" << endl;
            cout << "Received " << b->size() << " bytes of payload from " << f->src() << " :" << endl;
            for(int i=0; i<b->size(); i++)
                cout << d[i];
            cout << endl << "=====================" << endl;
            peer = f->src();
            _nic->free(b);
//            _nic->stop_listening();
        }
    }

private:
    Protocol _prot;
    NIC * _nic;
};

int main()
{
    cout << "Hello main" << endl;

    led = new GPIO('C',3, GPIO::OUTPUT); // This works for eMote3

    led_value = true;
    led->set(led_value);

    NIC * nic = new NIC();
    Receiver * r = new Receiver(0x1010, nic);

    peer = nic->broadcast();
    char data[] = "0 Hello, World!";
    const unsigned int delay_time = 3000000;
    cout << "I will send a message periodically." << endl;
    while(1)
    {
        cout << "Sending message: " << data << endl;
        nic->send(peer, 0x1010, data, sizeof data);
        cout << "Sent" << endl;
        data[0] = ((data[0] - '0' + 1) % 10) + '0';
        busy_wait();
//        Alarm::delay(delay_time + (Random::random() % delay_time));
    }

    return 0;
}
