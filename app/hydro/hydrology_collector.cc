#include <flash.h>
#include <alarm.h>
#include <nic.h>
#include <gpio.h>
#include <machine/cortex_m/emote3_gptm.h>

using namespace EPOS;

OStream cout;

const unsigned int PROTOCOL_ID = 0x1234;

const unsigned int start_flash_address = Flash::size() / 2;

class Network_Handler : public NIC::Observer
{
public:
    enum Message_Type 
    {
        READ = 'R',
        DATA = 'D',
        RESET = 'E',
    };

    struct Reset_Message
    {
        char type;
        unsigned char checksum;

        unsigned char lrc() const 
        {
            unsigned char lrc = 0;
            for(unsigned int i=0; i<sizeof(Reset_Message) - sizeof(char); i++)
                lrc += reinterpret_cast<const char *>(this)[i];
            return ((lrc ^ 0xff) + 1) & 0xff;
        }

        Reset_Message() 
            : type(Message_Type::RESET), checksum(lrc()) { } 

        static Reset_Message * check(NIC::Frame * f, unsigned int size)
        {
            if(size == sizeof(Reset_Message)) {
                auto d = f->data<Reset_Message>();
                if(d->type == Message_Type::RESET) {
                    if(d->lrc() == d->checksum)
                        return d;
                }
            }
            return 0;
        }
    }__attribute__((packed));

    struct Read_Message
    {
        char type;
        unsigned int read_address;
        unsigned int bytes;
        unsigned char checksum;

        unsigned char lrc() const 
        {
            unsigned char lrc = 0;
            for(unsigned int i=0; i<sizeof(Read_Message) - sizeof(char); i++)
                lrc += reinterpret_cast<const char *>(this)[i];
            return ((lrc ^ 0xff) + 1) & 0xff;
        }

        Read_Message(unsigned int address = 0, unsigned int bytes_to_read = Flash::size() / 2) 
            : type(Message_Type::READ), read_address(address), bytes(bytes_to_read), checksum(lrc()) { }

        static Read_Message * check(NIC::Frame * f, unsigned int size)
        {
            if(size == sizeof(Read_Message)) {
                auto d = f->data<Read_Message>();
                if(d->type == Message_Type::DATA) {
                    if(d->lrc() == d->checksum)
                        return d;
                }
            }
            return 0;
        }
    }__attribute__((packed));

    struct Data_Message
    {
        char type;
        unsigned int address;
        unsigned int seq;
        unsigned int level;
        unsigned int turbidity;
        unsigned int pluviometer;
        unsigned char checksum;

        unsigned char lrc() const 
        {
            unsigned char lrc = 0;
            for(unsigned int i=0; i<sizeof(Data_Message) - sizeof(char); i++)
                lrc += reinterpret_cast<const char *>(this)[i];
            return ((lrc ^ 0xff) + 1) & 0xff;
        }

        Data_Message(unsigned int flash_address) : type(Message_Type::DATA), address(flash_address), seq(Flash::read(flash_address)), level(Flash::read(flash_address + sizeof(unsigned int))), turbidity(Flash::read(2*sizeof(unsigned int))), checksum(lrc()) { }

        static Data_Message * check(NIC::Frame * f, unsigned int size)
        {
            if(size == sizeof(Data_Message)) {
                auto d = f->data<Data_Message>();
                if(d->type == Message_Type::DATA) {
                    if(d->lrc() == d->checksum)
                        return d;
                }
            }
            return 0;
        }
    }__attribute__((packed));


    typedef NIC::Protocol Protocol;
    typedef NIC::Buffer Buffer;
    typedef NIC::Frame Frame;
    typedef NIC::Observed Observed;

    Network_Handler(const Protocol & p) : _prot(p), _updated(true), _dst(NIC::Address::BROADCAST)
    {
        _nic.attach(this, _prot);
    }
    
    void read(unsigned int address = 0, unsigned int bytes_to_read = 4 * sizeof(unsigned int)) 
    { 
        Read_Message msg(address, bytes_to_read);
        eMote3_GPTM timeout(3, 100000);
        _updated = false;
        while(not _updated) {
            _nic.send(_dst, _prot, reinterpret_cast<const char *>(&msg), sizeof(Read_Message));
            timeout.enable();
            while((not _updated) and timeout.running());
        }
    }
    void reset() 
    { 
        Reset_Message msg;
        _nic.send(_dst, _prot, reinterpret_cast<const char *>(&msg), sizeof(Reset_Message));
    }

    void update(Observed * o, Protocol p, Buffer * b)
    {
        Frame * f = b->frame();
        auto src = f->src();
        auto size = b->size();
        if(auto d = Data_Message::check(f, size)) {
            if(_dst == _nic.broadcast()) {
                destination(src);
            }
            if(src == _dst) {
                _updated = true;
                if(d->address == start_flash_address) {
                    //cout << hex << d->address << ", " << d->seq << endl;
                    last_address = d->seq;
                    Flash::write(d->address, &(d->seq), 4);
                }
                else {
                    //cout << hex << d->address << ", ";
                    //cout << dec << d->seq << ", ";
                    //cout << d->level << ", ";
                    //cout << d->turbidity << ", ";
                    //cout << d->pluviometer << endl;
                    Flash::write(d->address, &(d->seq), 4*4);
                }
            }
        }
        _nic.free(b);
    }

    void destination(NIC::Address dst) { _dst = dst; }
    unsigned int last_address;

private:
    Protocol _prot;
    NIC _nic;
    NIC::Address _dst;
    volatile bool _updated;
};

int main()
{
    const unsigned int size_of_data = 4 * sizeof(unsigned int);

    GPIO led('c',3,GPIO::OUTPUT);
    bool led_val = true;
    led.set(led_val = !led_val);

    eMote3_GPTM::delay(4000000);

    cout << "Hydrology Collector" << endl;

    cout << "===== Dump of last collection =====" << endl;
    unsigned int current_flash_address = Flash::read(start_flash_address);
    if((current_flash_address <= start_flash_address) or (current_flash_address >= (Flash::size() - 2048))) {
        current_flash_address = start_flash_address + sizeof(unsigned int);
    }
    cout << hex << start_flash_address << ", " << Flash::read(start_flash_address) << endl;
    //for(unsigned int i = start_flash_address + 4u; i < current_flash_address; i+=size_of_data) {
    for(unsigned int i = current_flash_address - size_of_data; (i >= start_flash_address + 4u) and (i < current_flash_address); i-=size_of_data) {
        cout << hex << i << ", ";
        cout << dec << Flash::read(i) << ", ";
        cout << Flash::read(i+4) << ", ";
        cout << Flash::read(i+8) << ", ";
        cout << Flash::read(i+12) << endl;
    }
    cout << "===== /Dump of last collection =====" << endl;

    Network_Handler network(PROTOCOL_ID);

    network.read(start_flash_address, size_of_data);

    for(unsigned int i = 4u; (i < Flash::size() / 2 - 2048) and (start_flash_address + i < network.last_address); i+=size_of_data) {
        network.read(start_flash_address + i, size_of_data);
        led.set(led_val = !led_val);
    }

    while(true) {
        eMote3_GPTM::delay(3000000);
        network.reset();
        led.set(led_val = !led_val);
    }

    return 0;
}
