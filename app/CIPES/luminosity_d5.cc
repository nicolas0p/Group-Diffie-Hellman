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
#include <machine/cortex_m/emote3_watchdog.h>

using namespace EPOS;

OStream cout;
UART uart;

static const unsigned int DUTY_CYCLE = 1; // %
static const bool half_duty_cycle = true;

const char GATEWAY_ADDR[2] = {'E','E'};
NIC::Address gateway_address;

const auto ADC_PIN = ADC::SINGLE_ENDED_ADC6;
typedef int sensor_data_type;
sensor_data_type sensor_data;
sensor_data_type convert_from_adc(int adc_measurement)
{
    return adc_measurement;
}
sensor_data_type sense_luminosity()
{
    auto adc_measurement = ADC{ADC_PIN}.read();
    return convert_from_adc(adc_measurement);
}

// These two should be the same
const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'D','5'};
const unsigned char modbus_id = 0xD5;

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
    //for(unsigned int i = 0; (i < 100) and (_nic->send(gateway_address, Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len) < 0); i++);
    _nic->send(gateway_address, Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len);
    //memcpy(_msg, c, len);
    //cout << "Sending: ";
    //for(int i=0; i<len; i++)
    //    cout << c[i];
    //cout << endl;
    //int ret;
    //while((ret = _nic->send(_nic->broadcast(), Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len)) < 0)
    //    cout << ret << endl;
    //cout << ret << endl;
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
        unsigned short register_response;
        unsigned short sensor_data;
        switch(cmd) {
            case READ_HOLDING_REGISTER:
                memset(payload, 0, sizeof(unsigned short));
                sensor_data = sense_luminosity();
                register_response = (sensor_data << 8) | (sensor_data >> 8);
                payload[0] = register_response;
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
            case READ_HOLDING_REGISTER:
                sensor_data = sense_luminosity();
                register_response = (sensor_data << 8) | (sensor_data >> 8);
                send(myAddress(), READ_HOLDING_REGISTER, reinterpret_cast<unsigned char *>(&register_response), sizeof(unsigned short));
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
NIC * nic;
TSC::Time_Stamp t0;

void send(const unsigned int & interrupt_id)
{
    t0 = TSC::time_stamp();    
    nic->listen();
    sensor->modbus->report_proactive(Modbus::READ_HOLDING_REGISTER, 0);
    nic->stop_listening();
    return;
}

int main()
{
    cout << "CIPES Presence / Luminosity sensor" << endl;
    cout << "ID: " << hex << modbus_id << endl;
    cout << "Pins:" << endl
         << "   luminosity: PA6" << endl;

    gateway_address[0] = GATEWAY_ADDR[0];
    gateway_address[1] = GATEWAY_ADDR[1];

    NIC::Address addr;
    addr[0] = Traits<Build>::ID[0];
    addr[1] = Traits<Build>::ID[1];
    NIC * nic = new NIC();
    nic->address(addr);
    cout << "Address: " << nic->address() << endl;

    sensor = new Thing<Modbus>(modbus_id, nic);

    nic->stop_listening();

    eMote3::wake_up_on(eMote3::WAKE_UP_EVENT::SLEEP_MODE_TIMER);

    // Make sure duty cycle is respected in case of reboot by watchdog
    TSC::wake_up_at(TSC::time_stamp() + ((TSC::us_to_ts(1500000) * ((100 - DUTY_CYCLE) / DUTY_CYCLE)) << half_duty_cycle), &send);
    eMote3::power_mode(eMote3::POWER_MODE_2);
    eMote3::power_mode(eMote3::POWER_MODE::ACTIVE);

    eMote3_Watchdog::enable();

    TSC::Time_Stamp t1;

    send(0);
    while(true) {
        eMote3_Watchdog::kick();

        t1 = TSC::time_stamp();
        TSC::wake_up_at(TSC::time_stamp() + (((t1-t0) * ((100 - DUTY_CYCLE) / DUTY_CYCLE)) << half_duty_cycle), &send);

        eMote3::power_mode(eMote3::POWER_MODE_2);

        eMote3::power_mode(eMote3::POWER_MODE::ACTIVE);
    }

    return 0;
}
