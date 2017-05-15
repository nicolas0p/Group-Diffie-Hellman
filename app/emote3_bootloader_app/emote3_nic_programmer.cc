#include <gpio.h>
#include <usb.h>
#include <nic.h>
#include <machine/cortex_m/bootloader.h>

using namespace EPOS;

const unsigned int MESSAGE_SIZE = sizeof(Bootloader::Message);

class NIC_Handler : public IEEE802_15_4::Observer
{
public:
    typedef IEEE802_15_4::Protocol Protocol;
    typedef IEEE802_15_4::Buffer Buffer;
    typedef IEEE802_15_4::Frame Frame;
    typedef IEEE802_15_4::Observed Observed;

    NIC_Handler(const Protocol & p = Traits<Bootloader>::NIC_PROTOCOL, unsigned int channel = Traits<Bootloader>::NIC_CHANNEL) : _prot(p), _channel(channel), _peer(_nic.broadcast()) {
        _nic.stop_listening();
        _nic.channel(_channel);
        _nic.attach(this, _prot);
        _nic.listen();
    }

    void update(Observed * o, Protocol p, Buffer * b) {
        if(b->size() == MESSAGE_SIZE) {
            auto f = b->frame();
            if(_peer == _nic.broadcast()) {
                _peer = f->src();
            }
            if(f->src() == _peer) {
                auto d = f->data<char>();
                for(int i=0; i<b->size(); i++)
                    USB::put(d[i]);
                USB::flush();
            } else {
                kerr << "Wrong src!" << endl;
            }
        }
        _nic.free(b);
    }

    int send(char * msg, unsigned int size) {
        return _nic.send(_peer, _prot, msg, size);
    }

private:
    Protocol _prot;
    unsigned int _channel;
    NIC _nic;
    NIC::Address _peer;
};

int main()
{
    GPIO led('c',3,GPIO::OUTPUT);
    led.set(false);
    while(not USB::ready());
    for(unsigned int i=0; i<10; i++) {
        led.set(true);
        for(volatile unsigned int j=0;j<0xffff;j++);
        led.set(false);
        for(volatile unsigned int j=0;j<0xffff;j++);
    }
    led.set(true);

    NIC_Handler nh;
    char buffer[MESSAGE_SIZE];
    while(true) {
        while(USB::get_data(buffer, MESSAGE_SIZE) != MESSAGE_SIZE);
        nh.send(buffer, MESSAGE_SIZE);
    }
    
    return 0;
}
