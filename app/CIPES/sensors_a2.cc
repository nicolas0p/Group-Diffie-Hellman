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

using namespace EPOS;

OStream cout;
UART uart;

GPIO * led, * coil0;
bool led_state = false;

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

bool sense_presence()
{
    return coil0->read();
}

// These two should be the same
const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'A','2'};
const unsigned char modbus_id = 0xA2;

void Receiver::update(Observed * o, Secure_NIC::Protocol p, Buffer * b)
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
    cout << "Sending: ";
    for(int i=0; i<len; i++)
        cout << c[i];
    cout << endl;
    cout << _nic->send(_nic->gateway_address, (const char *)_msg, len) << endl;
}

class Modbus : public Modbus_ASCII
{
public:
	Modbus(Modbus_ASCII_Sender * sender, unsigned char addr)
      : Modbus_ASCII(sender, addr) { }

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


int main()
{
    cout << "CIPES Presence / Luminosity sensor" << endl;
    cout << "ID: " << hex << modbus_id << endl;
    cout << "Pins:" << endl
         << "   led: PC3" << endl
         << "   presence(coil0): PB0" << endl
         << "   luminosity: PA6" << endl;

    led = new GPIO('c',3, GPIO::OUTPUT);
    coil0 = new GPIO('b',0, GPIO::INPUT);

    led->set(false);

    NIC * nic = new NIC();
    nic->address(NIC::Address::RANDOM);
    cout << "Address: " << nic->address() << endl;
    Secure_NIC * s = new Secure_NIC(false, new AES(), new Poly1305(new AES()), nic);

    Thing<Modbus> sensor(modbus_id, s);

    while(true)
    {
        led->set(sense_presence());
    }

    return 0;
}
