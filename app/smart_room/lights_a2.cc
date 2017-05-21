#include <modbus_ascii.h>
#include <secure_nic.h>
#include <nic.h>
#include <gpio.h>
#include <aes.h>
#include <machine/cortex_m/emote3_watchdog.h>
#include "home_thing.h"
#include <machine/cortex_m/emote3_power_meter.h>
#include <machine/cortex_m/emote3_pwm.h>

using namespace EPOS;

typedef unsigned short sensor_data_type;

//These two should be the same
const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'A','2'};
const unsigned char MODBUS_ID = 0xA2;
const unsigned int PWM_FREQUENCY = 10000; // 10 KHz

OStream cout;
GPIO * led, * coil;
eMote3_PWM * pwm;
bool led_state = false;
bool coil_state = false;
// FIXME: Remove coil1_state
bool coil1_state = false;

Power_Meter * pm;

sensor_data_type sense_power()
{
    auto ret = pm->average();
    return ret;
}

void Receiver::update(Observed * o, Secure_NIC::Protocol p, Buffer * b)
{
    Frame * f = b->frame();
    char * _msg = f->data<char>();
    kout << "Received: " << b->size() << endl;
    for(int i=0; i<b->size(); i++)
        kout << _msg[i];
    kout << endl;
    Modbus_ASCII::Modbus_ASCII_Feeder::notify(_msg, b->size());
    _nic->free(b);
}

void Sender::send(const char * c, int len)
{
    memcpy(_msg, c, len);
    kout << "Sending: " << len << endl;
    for(int i=0; i<len; i++)
        kout << c[i];
    kout << endl;
    kout << _nic->send(_nic->gateway_address, (const char *)_msg, len) << endl;
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
        kout << "received command: " << hex << (int)cmd;
        for (int i = 0; i < data_len; ++i)
            kout << " " << (int)data[i];
        kout << dec << endl;
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
                coil_response = coil_state  + (coil1_state << 1);
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
                ack();
                value = (((short)data[2]) << 8) | data[3];
                pwm->set(PWM_FREQUENCY, value);
                break;
            case WRITE_SINGLE_COIL:
                output_address = (((unsigned short)data[0]) << 8) | data[1];
                output_value = (((unsigned short)data[2]) << 8) | data[3];
                ack();
                if(output_address == 0)
                    coil_state = output_value;
                else if(output_address == 1)
                    coil1_state = output_value;
                else if(output_address == 9)
                    Machine::reboot();
                led_state = output_value;

                coil->set(coil_state);
                led->set(led_state);
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
         << "    LED: PC3" << endl
         << "    Coil: PD3" << endl
         << "    PWM: PD2" << endl
         << "    Power meter: PA7 and PA5" << endl;

    pm = new Power_Meter(ADC::SINGLE_ENDED_ADC7, ADC::SINGLE_ENDED_ADC5, ADC::GND);
    led = new GPIO('c',3, GPIO::OUTPUT);
    coil = new GPIO('d',3, GPIO::OUTPUT);

    led->set(led_state);
    coil->set(coil_state);

    NIC * nic = new NIC();
    nic->address(NIC::Address::RANDOM);
    cout << "Address: " << nic->address() << endl;
    Secure_NIC * s = new Secure_NIC(false, new AES(), new Poly1305(new AES()), nic);

    Thing<Modbus> sensor(MODBUS_ID, s);

    pwm = new eMote3_PWM(1, PWM_FREQUENCY, 50, 'd', 2);

    while(true) {
        Alarm::delay(1000000);
        sensor.modbus->report_proactive(Modbus::READ_HOLDING_REGISTER, 0);
        sensor.modbus->report_proactive(Modbus::READ_HOLDING_REGISTER, 1);
    }

    return 0;
}
