#include <modbus_ascii.h>
#include <secure_nic.h>
#include <nic.h>
#include <gpio.h>
#include <aes.h>
#include <flash.h>
#include <machine/cortex_m/emote3_watchdog.h>
#include "home_thing.h"
#include <machine/cortex_m/emote3_power_meter.h>
#include <machine/cortex_m/emote3_pwm.h>

using namespace EPOS;

typedef unsigned short sensor_data_type;

//These two should be the same
const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'A','4'};
const unsigned char MODBUS_ID = 0xA4;

const unsigned int PWM_FREQUENCY = 10000; // 10 KHz

const unsigned int FLASH_ADDRESS = Flash::size() / 2; // Flash address to save coil state
const unsigned int COIL_OFF = 0;
const unsigned int COIL_ON = 0x12121212;

const char GATEWAY_ADDR[2] = {'E','E'};
NIC::Address gateway_address;

//GPIO led('c',3,GPIO::OUTPUT);
//bool led_value = false;

OStream cout;
GPIO * coil;
eMote3_PWM * pwm;
bool coil_state = false;

bool coil_from_flash() 
{
    unsigned int coil_code = Flash::read(FLASH_ADDRESS);
    return coil_code == COIL_ON;
}
void coil_to_flash(bool coil_state)
{
    unsigned int code = (coil_state ? COIL_ON : COIL_OFF);
    Flash::write(FLASH_ADDRESS, &code, sizeof(unsigned int));
}
unsigned int pwm_from_flash() 
{
    return Flash::read(FLASH_ADDRESS + 4);
}
void pwm_to_flash(unsigned int pwm_duty_cycle)
{
    unsigned int to_write = pwm_duty_cycle;
    Flash::write(FLASH_ADDRESS + 4, &to_write, sizeof(unsigned int));
}

Power_Meter * pm;

sensor_data_type sense_power()
{
    auto ret = pm->average();
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
        unsigned short response[2];
        response[0] = htons(starting_address);
        auto payload = &(response[1]);
        switch(cmd) {
            case READ_HOLDING_REGISTER:
                memset(payload, 0, 2*sizeof(unsigned short));
                switch(starting_address) {
                    default:
                    case 0:
                        payload[idx++] = htons(sense_power());
                        break;
                    case 1:
                        payload[idx++] = htons(pwm->duty_cycle());
                        break;
                }
                send(myAddress(), cmd, reinterpret_cast<unsigned char *>(response), 2 * sizeof(response[0]));
                break;
            default:
                break;
        }
    }

    void handle_command(unsigned char cmd, unsigned char * data, int data_len)
    {
        //cout << "received command: " << hex << (int)cmd;
        //for (int i = 0; i < data_len; ++i)
        //    cout << " " << (int)data[i];
        //cout << dec << endl;
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
                coil_response = coil_state;
                coil_response >>= starting_address;
                send(myAddress(), READ_COILS, reinterpret_cast<unsigned char *>(&coil_response), sizeof (unsigned char));
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
                    case 0:
                        if(idx < quantity_of_registers)
                            response[idx++] = htons(sense_power());
                    default:
                    case 1:
                        if(idx < quantity_of_registers)
                            response[idx++] = htons(pwm->duty_cycle());
                }
                send(myAddress(), READ_HOLDING_REGISTER, reinterpret_cast<unsigned char *>(response), quantity_of_registers * sizeof(response[0]));
                break;
            case WRITE_SINGLE_REGISTER:
                starting_address = (((unsigned short)data[0]) << 8) | data[1];
                if(starting_address != 1)
                    break;
                //ack();
                value = (((short)data[2]) << 8) | data[3];
                //pwm_to_flash(value);
                pwm->set(PWM_FREQUENCY, value);
                break;
            case WRITE_SINGLE_COIL:
                output_address = (((unsigned short)data[0]) << 8) | data[1];
                output_value = (((unsigned short)data[2]) << 8) | data[3];
                //ack();
                if(output_address == 0) {
                    if((output_value & 1) != coil_state) {
                        coil_state = (output_value & 1);
                        coil->set(coil_state);
                        //coil_to_flash(coil_state);
                    }
                }

                else if(output_address == 9)
                    Machine::reboot();

                break;
            default:
                break;
        }
    }
};


int main()
{
    cout << "LISHA Smart Room Lights actuator / Power meter" << endl;
    cout << "ID: " << hex << MODBUS_ID << endl;
    cout << "Pins" << endl
         << "    Coil: PD3" << endl
         << "    PWM: PD2" << endl
         << "    Power meter: PA7 and PA5" << endl;

    gateway_address[0] = GATEWAY_ADDR[0];
    gateway_address[1] = GATEWAY_ADDR[1];

    //coil_state = coil_from_flash();
    coil_state = true;
    unsigned int pwm_duty_cycle = pwm_from_flash();

    coil = new GPIO('d',3, GPIO::OUTPUT);
    coil->set(coil_state);

    pwm = new eMote3_PWM(1, PWM_FREQUENCY, pwm_duty_cycle, 'd', 2);

    pm = new Power_Meter(ADC::SINGLE_ENDED_ADC7, ADC::SINGLE_ENDED_ADC5, ADC::GND);

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
    }

    return 0;
}
