#include <modbus_ascii.h>
#include <alarm.h>
#include <chronometer.h>
#include <secure_nic.h>
#include <utility/string.h>
#include <utility/key_database.h>
#include <nic.h>
#include <gpio.h>
#include <aes.h>
#include <uart.h>
#include "home_thing.h"
#include <adc.h>
#include <machine/cortex_m/emote3_gptm.h>

using namespace EPOS;

OStream cout;
UART uart;

NIC * nic;
GPIO * coil0;

const char GATEWAY_ADDR[2] = {'E','E'};
NIC::Address gateway_address;

bool sense_presence()
{
    return coil0->read();
}

// These two should be the same
const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'D','0'};
const unsigned char modbus_id = 0xD0;

void Receiver::update(Observed * o, NIC::Protocol p, Buffer * b)
{
    int i=0;
    Frame * f = b->frame();
    char * _msg = f->data<char>();
    Modbus_ASCII::Modbus_ASCII_Feeder::notify(_msg, b->size());
    _nic->free(b);
}

void Sender::send(const char * c, int len)
{
    memcpy(_msg, c, len);
    //cout << "Sending: " << len << endl;
    //for(int i=0; i<len; i++)
    //    cout << c[i];
    //cout << endl;
//    cout << 
    for(unsigned int i = 0; (i < 20) and (_nic->send(gateway_address, Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len) < 0); i++);
        //_nic->send(_nic->broadcast(), Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len);
            //<< endl;
}

class Modbus : public Modbus_ASCII
{
public:
    Modbus(Modbus_ASCII_Sender * sender, unsigned char addr)
      : Modbus_ASCII(sender, addr) { }

    void report_proactive(unsigned char cmd, unsigned short starting_address)
    {
        int idx = 0;
        unsigned short response[2];
        response[0] = starting_address;
        auto payload = &(response[1]);
        switch(cmd) {
            case READ_COILS:
                memset(payload, 0, sizeof(unsigned short));
                payload[0] = sense_presence();
                payload[0] = htons(payload[0]) >> starting_address;
                send(myAddress(), cmd, reinterpret_cast<unsigned char *>(response), 2 * sizeof(response[0]));
                break;
            default:
                break;
        }
    }

    void handle_command(unsigned char cmd, unsigned char * data, int data_len)
    {
        cout << "received command: " << hex << (int)cmd;
        for (int i = 0; i < data_len; ++i)
            cout << " " << (int)data[i];
        cout << dec << endl;
        unsigned short starting_address, quantity_of_registers;
        unsigned char coil_response;
        unsigned short register_response;
        unsigned short sensor_data;
        unsigned short output_address;
        unsigned short output_value;
        switch(cmd)
        {
            case READ_COILS:
                starting_address = (((unsigned short)data[0]) << 8) | data[1];
                //quantity_of_registers = (((unsigned short)data[2]) << 8) | data[3];
                coil_response = sense_presence();
                coil_response >>= starting_address;
                send(myAddress(), READ_COILS, &coil_response, 1);
                break;

            case WRITE_SINGLE_COIL:
                output_address = (((unsigned short)data[0]) << 8) | data[1];
                output_value = (((unsigned short)data[2]) << 8) | data[3];
                ack();
                if(output_address == 9)
                    Machine::reboot();
                break;
            default:
                break;
        }
    }
private:
};

Thing<Modbus> * sensor;
 
void send(GPIO * pin)
{
    nic->listen();
    sensor->modbus->report_proactive(Modbus::READ_COILS, 0);
    nic->stop_listening();
    return;
}

int main()
{
    cout << "CIPES Presence sensor" << endl;
    cout << "ID: " << hex << modbus_id << endl;
    cout << "Pins:" << endl
         << "   presence(coil0): PB0" << endl;
    
    gateway_address[0] = GATEWAY_ADDR[0];
    gateway_address[1] = GATEWAY_ADDR[1];

    coil0 = new GPIO('b',0, GPIO::INPUT);

    NIC::Address addr;
    addr[0] = Traits<Build>::ID[0];
    addr[1] = Traits<Build>::ID[1];
    NIC * nic = new NIC();
    nic->address(addr);
    cout << "Address: " << nic->address() << endl;

    sensor = new Thing<Modbus>(modbus_id, nic);

    eMote3::wake_up_on(eMote3::WAKE_UP_EVENT::GPIO_B);
    auto edge = GPIO::RISING_EDGE;
    GPIO led('c',3,GPIO::OUTPUT);
    led.set(false);

    while(true) {
        coil0->enable_interrupt(edge, &send, true, edge);
        edge = (edge == GPIO::RISING_EDGE ? GPIO::FALLING_EDGE : GPIO::RISING_EDGE);
        eMote3::power_mode(eMote3::POWER_MODE_3);
    }

    return 0;
}
