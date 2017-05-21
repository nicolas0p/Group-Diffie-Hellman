#include <gpio.h>
#include <ieee802_15_4.h>
#include <machine.h>
#include <usb.h>
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
        if(b->frame()->data<ID_Response>()->valid()) {
            Machine::delay(10000);
            cout << b->frame()->src() << " =>";
            for(unsigned int i = 0; i < 8; i++)
                cout << " " << hex << b->frame()->data<ID_Response>()->id()[i];
            cout << endl;
        }

        _nic->free(b);
    }

private:
    Protocol _prot;
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

char to_hex(char hi, char lo) {
    if(hi >= 'a' && hi <= 'f')
        hi = 10 + hi - 'a';
    else if(hi >= 'A' && hi <= 'F')
        hi = 10 + hi - 'A';
    else if(hi >= '0' && hi <= '9')
        hi -= '0';

    if(lo >= 'a' && lo <= 'f')
        lo = 10 + lo - 'a';
    else if(lo >= 'A' && lo <= 'F')
        lo = 10 + lo - 'A';
    else if(lo >= '0' && lo <= '9')
        lo -= '0';

    return (hi << 4) + lo;
}

int main()
{
    cout << "ID Commander Application" << endl;
    int address_channel = 13;
    int id_channel = 12;

    GPIO led('C',3,GPIO::OUT);
    blink_led(led);

    NIC * nic = new NIC();
    CC2538 * cc = CC2538::get(0);

    Receiver r(IEEE802_15_4::ELP, nic);

    cout << "Finding motes..." << endl;
    cout << "[addr] => [ID]" << endl;

    ID_Request idr;
    nic->send(nic->broadcast(), IEEE802_15_4::ELP, &idr, sizeof(ID_Request));

    Machine::delay(1000000);
    cout << "Done" << endl;

    while(true) {
        cout << "Enter option (S = scan; A = reset by address; I = reset by ID; c = set ID channel; C = set Address channel): ";
        char opt = USB::get();
        cout << opt << endl;
        if(opt == 'S') {
            cc->channel(address_channel);

            cout << "Finding motes..." << endl;
            cout << "[addr] => [ID]" << endl;
            nic->send(nic->broadcast(), IEEE802_15_4::ELP, &idr, sizeof(ID_Request));
            Machine::delay(1000000);
            cout << "Done" << endl;
        } else if(opt == 'A') {
            cc->channel(address_channel);

            cout << "Enter address of mote to reset: ";
            char a = USB::get();
            char b = USB::get();
            char c = USB::get();
            char d = USB::get();

            NIC::Address addr;
            addr[0] = to_hex(a, b);
            addr[1] = to_hex(c, d);
            cout << addr << endl;

            Reset r;
            nic->send(addr, IEEE802_15_4::ELP, &r, sizeof(Reset));
        } else if(opt == 'I') {
            cc->channel(id_channel);

            cout << "Enter ID of mote to reset: ";
            char a = USB::get(); char b = USB::get(); char c = USB::get(); char d = USB::get();
            char e = USB::get(); char f = USB::get(); char g = USB::get(); char h = USB::get();
            char i = USB::get(); char j = USB::get(); char k = USB::get(); char l = USB::get();
            char m = USB::get(); char n = USB::get(); char o = USB::get(); char p = USB::get();

            unsigned char id[8];
            id[0] = to_hex(a, b); id[1] = to_hex(c, d); id[2] = to_hex(e, f); id[3] = to_hex(g, h);
            id[4] = to_hex(i, j); id[5] = to_hex(k, l); id[6] = to_hex(m, n); id[7] = to_hex(o, p);

            for(unsigned int i = 0; i < 8; i++)
                cout << " " << hex << id[i];
            cout << endl;

            CC2538RF::Reset_Backdoor_Message rbm(id);
            for(unsigned int i = 0; i < 1000; i++)
                nic->send(nic->broadcast(), IEEE802_15_4::ELP, &rbm, sizeof(CC2538RF::Reset_Backdoor_Message));
        } else if((opt == 'c') || (opt == 'C')) {
            cout << "Enter channel to set for " << (opt == 'c' ? "ID" : "Address") << " commands: ";
            char a = USB::get();
            char b = USB::get();

            int channel = (a - '0') * 10 + (b - '0');
            cout << channel << endl;
            if((channel > 10) && (channel < 27)) {
                if(opt == 'c')
                    id_channel = channel;
                else
                    address_channel = channel;
            }
            else
                cout << "Channel must be in [11,26]" << endl;
        } else {
            cout << "Unrecognized option" << endl;
        }
    }
}
