#include <modbus_ascii.h>
#include <nic.h>
#include <adc.h>
#include <flash.h>
#include <machine/cortex_m/emote3_watchdog.h>
#include <machine/cortex_m/emote3_power_meter.h>
#include "home_thing.h"

using namespace EPOS;

typedef unsigned int sensor_data_type;

// These two should be the same
const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'B','c'};
const unsigned char MODBUS_ID = 0xBc;

OStream cout;
GPIO * coil0, * coil1;
bool coil0_state = false;
bool coil1_state = false;

Power_Meter * pm0, * pm1;

const unsigned int FLASH_ADDRESS = Flash::size() / 2; // Flash address to save coil state
const unsigned int COIL_OFF = 0;
const unsigned int COIL_ON = 0x12121212;

const char GATEWAY_ADDR[2] = {'E','E'};
NIC::Address gateway_address;

bool coil0_from_flash()
{
    unsigned int coil_code = Flash::read(FLASH_ADDRESS);
    return coil_code == COIL_ON;
}
void coil0_to_flash(bool coil_state) 
{
    unsigned int code = (coil_state ? COIL_ON : COIL_OFF);
    Flash::write(FLASH_ADDRESS, &code, sizeof(unsigned int));
}
bool coil1_from_flash()
{
    unsigned int coil_code = Flash::read(FLASH_ADDRESS+4);
    return coil_code == COIL_ON;
}
void coil1_to_flash(bool coil_state) 
{
    unsigned int code = (coil_state ? COIL_ON : COIL_OFF);
    Flash::write(FLASH_ADDRESS+4, &code, sizeof(unsigned int));
}

sensor_data_type sense_power0()
{
    auto ret = pm0->average();
    return ret;
}

sensor_data_type sense_power1()
{
    auto ret = pm1->average();
    return ret;
}

void Receiver::update(Observed * o, NIC::Protocol p, Buffer * b)
{
    //led.set(led_value = !led_value);
    Frame * f = b->frame();
    if(f->src() == gateway_address) {
        char * _msg = f->data<char>();
        Modbus_ASCII::Modbus_ASCII_Feeder::notify(_msg, b->size());
        //cout << "Received: " << b->size() << endl;
        //for(int i=0; i<b->size(); i++)
        //    cout << _msg[i];
        //cout << endl;
    }
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
        _nic->send(gateway_address, Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len);
        //_nic->send(_nic->broadcast(), Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len);
            //<< endl;
}

class Modbus: public Modbus_ASCII
{
public:
    Modbus(Modbus_ASCII_Sender * sender, unsigned char addr):
            Modbus_ASCII(sender, addr) { }

    void report_proactive(unsigned char cmd, unsigned short starting_address)
    {
        int idx = 0;
        auto data_size = sizeof(sensor_data_type);
        
        struct  {
            short offset;
            sensor_data_type data;
        }__attribute__((packed)) response;

        response.offset = htons(starting_address);

        switch(cmd) {
            case READ_HOLDING_REGISTER:
                memset(&(response.data), 0, sizeof(sensor_data_type));
                switch(starting_address) {
                    default:
                    case 0:
                        if (data_size == sizeof(short)) {
                            response.data = htons(sense_power0());
                        } else if(data_size == sizeof(int)) {
                            response.data = htonl(sense_power0());
                        } else {
                            response.data = sense_power0();
                        }
                        break;
                    case 1:
                        if (data_size == sizeof(short)) {
                            response.data = htons(sense_power1());
                        } else if(data_size == sizeof(int)) {
                            response.data = htonl(sense_power1());
                        } else {
                            response.data = sense_power1();
                        }
                        break;
                }
                send(myAddress(), cmd, reinterpret_cast<unsigned char *>(&response), sizeof(response));
                break;
            default:
                break;
        }
    }
    void handle_command(unsigned char cmd, unsigned char * data, int data_len)
    {
        unsigned short starting_address, quantity_of_registers;
        unsigned char coil_response;
        unsigned short register_response;
        unsigned short output_address;
        unsigned short output_value;
        short value;
        sensor_data_type response[2];
        int idx = 0;
        switch(cmd)
        {
            case READ_COILS:
                starting_address = (((unsigned short)data[0]) << 8) | data[1];
                //quantity_of_registers = (((unsigned short)data[2]) << 8) | data[3];
                coil_response = coil0_state  + (coil1_state << 1);
                coil_response >>= starting_address;
                send(myAddress(), READ_COILS, reinterpret_cast<unsigned char *>(&coil_response), sizeof (unsigned char));
                break;

            case WRITE_SINGLE_COIL:
                output_address = (((unsigned short)data[0]) << 8) | data[1];
                output_value = (((unsigned short)data[2]) << 8) | data[3];
                ////ack();
                if(output_address == 0) {
                    if((output_value & 1) != coil0_state) {
                        coil0_state = (output_value & 1);
                        coil0->set(coil0_state);
                        coil0_to_flash(coil0_state);
                    }
                }
                else if(output_address == 1) {
                    if((output_value & 1) != coil1_state) {
                        coil1_state = (output_value & 1);
                        coil1->set(coil1_state);
                        coil1_to_flash(coil1_state);
                    }
                }
                else if(output_address == 9)
                    Machine::reboot();

                break;

            case READ_HOLDING_REGISTER:
                memset(response, 0, 2*sizeof(sensor_data_type));
                starting_address = (((unsigned short)data[0]) << 8) | data[1];
                quantity_of_registers = (((unsigned short)data[2]) << 8) | data[3];
                if(quantity_of_registers > 2)
                    break;
                switch(starting_address)
                {
                    // There are intentionally no breaks
                    default:
                    case 0:
                        if(idx < quantity_of_registers)
                            response[idx++] = htons(sense_power0());
                        break;
                    case 1:
                        if(idx < quantity_of_registers)
                            response[idx++] = htons(sense_power1());
                }
                send(myAddress(), READ_HOLDING_REGISTER, reinterpret_cast<unsigned char *>(response), quantity_of_registers * sizeof(response[0]));
                break;

            default:
                break;
        }
    }
};

int main()
{
    cout << "LISHA Smart Room Power meter" << endl;
    cout << "ID: " << hex << MODBUS_ID << endl;
    cout << "Pins" << endl
         << "   Power meters: PA7, PA6 and PA5" << endl;

    gateway_address[0] = GATEWAY_ADDR[0];
    gateway_address[1] = GATEWAY_ADDR[1];

    coil0_state = coil0_from_flash();
    coil1_state = coil1_from_flash();

    coil0 = new GPIO('b',0, GPIO::OUTPUT);
    coil1 = new GPIO('b',1, GPIO::OUTPUT);

    pm0 = new Power_Meter(ADC::SINGLE_ENDED_ADC7, ADC::SINGLE_ENDED_ADC5, ADC::GND);
    pm1 = new Power_Meter(ADC::SINGLE_ENDED_ADC7, ADC::SINGLE_ENDED_ADC6, ADC::GND);

    coil0->set(coil0_state);
    coil1->set(coil1_state);

    NIC nic;
    NIC::Address addr;
    addr[0] = Traits<Build>::ID[0];
    addr[1] = Traits<Build>::ID[1];
    nic.address(addr);
    cout << "Address: " << nic.address() << endl;

    Thing<Modbus> sensor(MODBUS_ID, &nic);

    while(true) {
        Alarm::delay(1000000);
        sensor.modbus->report_proactive(Modbus::READ_HOLDING_REGISTER, 0);
        Alarm::delay(1000000);
        sensor.modbus->report_proactive(Modbus::READ_HOLDING_REGISTER, 1);
    }

    return 0;
}
