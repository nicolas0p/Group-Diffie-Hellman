#include <gpio.h>
#include <utility/ostream.h>
#include <adc.h>
#include <flash.h>
#include <machine/cortex_m/emote3_gptm.h>
#include <machine/cortex_m/emote3_watchdog.h>

// This application logs a sequence number and two ADC conversions on the flash memory every five
// minutes. On reset, it dumps the flash contents and the last position that
// was actually read on the current run.

using namespace EPOS;

const auto MINUTES = 5u;
const auto MINUTE_IN_US = 60000000u;
const unsigned int PROTOCOL_ID = 0x1234;

const auto INTERRUPT_DEBOUNCE_MARGIN = TSC::us_to_ts(100000);

const unsigned int start_flash_address = Flash::size() / 2;

const NIC::Address nic_address("01:24");

volatile unsigned int pluviometer_count = 0;

OStream cout;

void blink_led(unsigned int times, unsigned int period = 50000)
{
    auto led = GPIO{'c', 3, GPIO::OUTPUT};
    for (auto i = 0u; i < times; ++i) {
        led.set();
        eMote3_GPTM::delay(period);
        led.clear();
        eMote3_GPTM::delay(period);
    }
}

class Sensor_Base {
public:
    Sensor_Base(ADC &adc, GPIO &relay):
        adc(adc),
        relay(relay)
    {}

    int enable()
    {
        relay.set();
    }

    int disable()
    {
        relay.clear();
    }

protected:
    ADC &adc;
    GPIO &relay;
};

class Level_Sensor: public Sensor_Base {
public:
    Level_Sensor(ADC &adc, GPIO &relay):
        Sensor_Base{adc, relay}
    {}

    int sample()
    {
        return adc.read();
    }
};

class Turbidity_Sensor: public Sensor_Base {
public:
    Turbidity_Sensor(ADC &adc, GPIO &relay, GPIO &infrared):
        Sensor_Base{adc, relay},
        infrared(infrared)
    {}

    int sample()
    {
        // Filter daylight as directed by sensor manufacturer
        eMote3_GPTM::delay(250000); // Wait 250 ms before reading
        auto daylight = adc.read();
        eMote3_GPTM::delay(250000); // Wait more 250 ms because we've been told to
        infrared.set();
        eMote3_GPTM::delay(450000); // Wait 200+250 ms before reading again
        auto mixed = adc.read();
        infrared.clear();
        return mixed - daylight;
    }

private:
    GPIO &infrared;
};

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

class Pluviometer_Handler : public Handler {
public:
    Pluviometer_Handler(GPIO * gpio) : _gpio(gpio) {}

    void operator()() {
        static bool rising_edge = false;
        static TSC::Time_Stamp last_interrupt = 0;

        auto now = TSC::time_stamp();

        if((now - last_interrupt) >= INTERRUPT_DEBOUNCE_MARGIN) {
            last_interrupt = now;
            if(not rising_edge) {
                pluviometer_count++;
            }
            rising_edge = not rising_edge;
            _gpio->enable_interrupt(rising_edge ? GPIO::RISING_EDGE : GPIO::FALLING_EDGE, this);
        }
    }

private:
    GPIO * _gpio;
};

int main()
{
    // Use this to erase the flash
    //unsigned int dt[512];
    //unsigned int sz = 512 * 4;
    //for(auto i = 0u; i < 512; i++) {
    //    dt[i] = 0xffffffff;
    //}
    //for(auto i = start_flash_address; i < (Flash::size() - 2048); i+=sz) {
    //    Flash::write(i, dt, sz);
    //}
    //while(1) blink_led(10);

    auto level_toggle = GPIO{'b', 0, GPIO::OUTPUT};
    auto turbidity_infrared = GPIO{'b', 2, GPIO::OUTPUT};
    auto turbidity_toggle = GPIO{'b', 3, GPIO::OUTPUT};

    auto level_adc = ADC{ADC::SINGLE_ENDED_ADC2};
    auto turbidity_adc = ADC{ADC::SINGLE_ENDED_ADC5};

    auto level = Level_Sensor{level_adc, level_toggle};
    auto turbidity = Turbidity_Sensor{turbidity_adc,
                                      turbidity_toggle,
                                      turbidity_infrared};

    level.disable();
    turbidity.disable();

    auto pluviometer = GPIO{'b', 4, GPIO::INPUT};
    Pluviometer_Handler phandler(&pluviometer);

    pluviometer.enable_interrupt(GPIO::FALLING_EDGE, &phandler);

    unsigned int data[4];
    const unsigned int size_of_data = 4 * sizeof(unsigned int);

    // Dump flash contents to serial
    //cout << "# Next write address: ";
    //cout << hex << Flash::read(start_flash_address) << endl;
    //cout << "# flash address offset, sequence number, level, turbidity" << endl;
    //for(auto i = start_flash_address + sizeof(unsigned int); i < Flash::size(); i += size_of_data) {
    //    cout << hex << i << ", ";
    //    cout << hex << Flash::read(i) << ", ";
    //    cout << hex << Flash::read(i + sizeof(unsigned int)) << ", ";
    //    cout << hex << Flash::read(i + 2*sizeof(unsigned int)) << endl;
    //}

    blink_led(10);

    auto pwrkey = GPIO{'d', 3, GPIO::OUTPUT};
    auto status = GPIO{'d', 5, GPIO::INPUT};

    // Read next write address from start_flash_address
    unsigned int current_flash_address = Flash::read(start_flash_address);
    if((current_flash_address <= start_flash_address) or (current_flash_address >= (Flash::size() - 2048))) {
        current_flash_address = start_flash_address + sizeof(unsigned int);
    }

    Network_Handler network_handler(PROTOCOL_ID);

    eMote3_GPTM timer(0);

    for(auto seq = 0u ; ; seq++) {
        // Count the time elapsed in the loop, so we can wake up in the 5-minute border
        timer.set(MINUTE_IN_US);
        timer.enable();

        blink_led(3);

        level.enable();
        turbidity.enable();

        eMote3_GPTM::delay(3000000);

        // Build data array to be written to flash
        data[0] = seq;
        data[1] = level.sample();
        data[2] = turbidity.sample();
        data[3] = pluviometer_count;

        level.disable();
        turbidity.disable();

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
            eMote3_GPTM::delay(MINUTE_IN_US);
        }
    }
}
