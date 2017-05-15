#include <gpio.h>
#include <utility/ostream.h>
#include <flash.h>

//#define HCSR04_RELAY
#include "ultrasonic_sensor/sensor/ultrasonic_sensor_controller.h"


using namespace EPOS;

const auto MINUTES = 5u;
const auto MINUTE_IN_US = 60000000u;
const unsigned int PROTOCOL_ID = 0x1234;

const unsigned int start_flash_address = Flash::size() / 2;

const NIC::Address nic_address("01:24");

OStream cout;

typedef User_Timer_1 Sleep_Timer;

void blink_led(unsigned int times, unsigned int period = 50000)
{
    auto led = GPIO{'c', 3, GPIO::OUTPUT};
    for (auto i = 0u; i < times; ++i) {
        led.set();
        Sleep_Timer::delay(period);
        led.clear();
        Sleep_Timer::delay(period);
    }
}

class Network_Handler : public NIC::Observer
{
public:
    enum Message_Type
    {
        READ = 'R',
        DATA = 'D',
        RESET = 'E',
    };

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

        Read_Message(unsigned int address = 0, unsigned int bytes_to_read = Flash::size() / 2) : type(Message_Type::READ), bytes(bytes_to_read), checksum(lrc()) { }

        static Read_Message * check(NIC::Frame * f, unsigned int size)
        {
            if(size == sizeof(Read_Message)) {
                auto d = f->data<Read_Message>();
                if(d->type == Message_Type::READ) {
                    if(d->lrc() == d->checksum) {
                        return d;
                    }
                }
            }
            return 0;
        }
    }__attribute__((packed));

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

        Data_Message(unsigned int flash_address)
            : type(Message_Type::DATA), address(flash_address) {
                seq = Flash::read(flash_address);
                level = Flash::read(flash_address + sizeof(unsigned int));
                turbidity = Flash::read(flash_address + 2*sizeof(unsigned int));
                pluviometer = Flash::read(flash_address + 3*sizeof(unsigned int));
                checksum = lrc();
            }

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

    Network_Handler(const Protocol & p) : _prot(p)
    {
        _nic.address(nic_address);
        _nic.attach(this, _prot);
    }

    void update(Observed * o, Protocol p, Buffer * b)
    {
        Frame * f = b->frame();
        auto src = f->src();
        auto size = b->size();
        if(auto d = Read_Message::check(f, size)) {
            auto read_address = d->read_address;
            auto bytes = d->bytes;
            _nic.free(b);

            for(auto i = 0u; i < bytes; i += 4*sizeof(unsigned int)) {
                auto addr = read_address + i;
                if(addr >= (Flash::size() - 2048)) {
                    addr = start_flash_address + 4;
                }
                Data_Message response(addr);
                _nic.send(src, p, reinterpret_cast<const char *>(&response), sizeof(Data_Message));
            }
        }
        else if(auto d = Reset_Message::check(f, size)) {
            auto addr = start_flash_address + 4u;
            Flash::write(start_flash_address, &addr, sizeof(unsigned int));
            Machine::reboot();
        }
        else {
            _nic.free(b);
        }
    }

private:
    Protocol _prot;
    NIC _nic;
};

int main()
{
    unsigned int data[4];
    const unsigned int size_of_data = 4 * sizeof(unsigned int);

    blink_led(10);

    // Read next write address from start_flash_address
    unsigned int current_flash_address = Flash::read(start_flash_address);
    if((current_flash_address <= start_flash_address) or (current_flash_address >= (Flash::size() - 2048))) {
        current_flash_address = start_flash_address + sizeof(unsigned int);
    }

    GPIO echo('d',0,GPIO::INPUT);
    GPIO trigger('d',3,GPIO::OUTPUT);
    //GPIO relay('b',3,GPIO::OUTPUT);

    Ultrasonic_Sensor_Controller ultrasonic_sensor(trigger,echo);
    Network_Handler network_handler(PROTOCOL_ID);

    Sleep_Timer timer;

    data[2] = 0;
    data[3] = 0;
    for(auto seq = 0u ; ; seq++) {
        // Count the time elapsed in the loop, so we can wake up in the 5-minute border
        blink_led(3);

        timer.set(MINUTE_IN_US);
        timer.enable();

        // Build data array to be written to flash
        data[0] = seq;
        data[1] = ultrasonic_sensor.sense();

        // Write seq, level, turbidity to flash
        Flash::write(current_flash_address, data, size_of_data);

        // Calculate flash address for next write
        current_flash_address += size_of_data;
        if(current_flash_address >= (Flash::size() - 2048))
            current_flash_address = start_flash_address + sizeof(unsigned int);

        // Write address of next write to start_flash_address
        Flash::write(start_flash_address, &current_flash_address, sizeof(unsigned int));

        for(auto i = 0; i < 4; i++)
            cout << data[i] << ", ";
        cout << endl;

        // Sleep until the end of the first minute
        while(timer.running());

        // Sleep for 4 more minutes
        for (auto i = 0u; i < MINUTES-1; ++i) {
            Sleep_Timer::delay(MINUTE_IN_US);
        }
    }
}
