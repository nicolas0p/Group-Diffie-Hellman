#include <gpio.h>
#include <ieee802_15_4.h>
#include <machine.h>
#include <utility/ostream.h>

using namespace EPOS;

OStream cout;

class ID_Request: public IEEE802_15_4::Header
{
public:
    ID_Request() : _code(0x42) {}

    bool valid() { return _code == 0x42; }

private:
    unsigned char _code;
    IEEE802_15_4::CRC _crc;
} __attribute__((packed));

class ID_Response: public IEEE802_15_4::Header
{
public:
    ID_Response() : _code(0x43) {
        memcpy(_id, Machine::id(), 8);
    }

    bool valid() { return _code == 0x43; }
    unsigned char * id() { return _id; }

private:
    unsigned char _code;
    unsigned char _id[8];
    IEEE802_15_4::CRC _crc;
} __attribute__((packed));

class Reset: public IEEE802_15_4::Header
{
public:
    Reset() : _code(0x45) {}

    bool valid() { return _code == 0x45; }

private:
    unsigned char _code;
    IEEE802_15_4::CRC _crc;
} __attribute__((packed));

class Receiver : public IEEE802_15_4::Observer
{
public:
    typedef IEEE802_15_4::Protocol Protocol;
    typedef IEEE802_15_4::Buffer Buffer;
    typedef IEEE802_15_4::Frame Frame;
    typedef IEEE802_15_4::Observed Observed;

    Receiver(const Protocol & p, NIC * nic) : _prot(p), _nic(nic) {
        _nic->attach(this, _prot);
    }

    void update(Observed * o, Protocol p, Buffer * b) {
        if(b->frame()->data<ID_Request>()->valid())
            _nic->send(b->frame()->src(), p, &_resp, sizeof(ID_Response));
        else if(b->frame()->data<Reset>()->valid())
            Machine::reboot();

        _nic->free(b);
    }

private:
    Protocol _prot;
    ID_Response _resp;
    NIC * _nic;
};

void blink_led(GPIO & led, unsigned int times = 10, RTC::Microsecond delay = 50000)
{
    for(unsigned int i=0;i<times;i++) {
        led.set(true);
        Machine::delay(delay);
        led.set(false);
        Machine::delay(delay);
    }
}

int main()
{
    CC2538 * cc2538 = CC2538::get(0);
    cc2538->channel(13);

    Machine::delay(5000000);
    cout << "ID Sender Application" << endl;

    GPIO led('C',3,GPIO::OUT);
    blink_led(led);

    cout << "Machine::id() = ";
    for(unsigned int i = 0; i < 8; i++)
        cout << " " << hex << Machine::id()[i];
    cout << endl;

    NIC * nic = new NIC();
    Receiver r(IEEE802_15_4::ELP, nic);

    while(true);
}
