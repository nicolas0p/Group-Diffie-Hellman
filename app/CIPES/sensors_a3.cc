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
#include <machine/cortex_m/cm1101.h>
#include <machine/cortex_m/sht11.h>

using namespace EPOS;

OStream cout;

CM1101 * co2_temperature_sensor;
SHT11_Humidity * humidity_sensor;

typedef short sensor_data_type;

sensor_data_type sense_co2()
{
    return co2_temperature_sensor->co2();
}
sensor_data_type sense_temperature()
{
    return co2_temperature_sensor->temp();
}
sensor_data_type sense_humidity()
{
    return humidity_sensor->sample();
}


GPIO * led;
bool led_state = false;

// These two should be the same
const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'A','3'};
const unsigned char modbus_id = 0xA3;

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
    _nic->send(_nic->gateway_address, (const char *)_msg, len);
}

class Modbus : public Modbus_ASCII
{
public:
	Modbus(Modbus_ASCII_Sender * sender, unsigned char addr)
      : Modbus_ASCII(sender, addr) { }

	void handle_command(unsigned char cmd, unsigned char * data, int data_len)
	{
        unsigned short starting_address, quantity_of_registers;
        unsigned short output_address, output_value;
        sensor_data_type response[3];
        int idx = 0;
        auto rc = reinterpret_cast<unsigned char *>(response);
		switch(cmd)
		{
            case READ_HOLDING_REGISTER:
                memset(response, 0, sizeof(response));
                starting_address = (((unsigned short)data[0]) << 8) | data[1];
                quantity_of_registers = (((unsigned short)data[2]) << 8) | data[3];
                if(quantity_of_registers > 3)
                    break;
                switch(starting_address)
                {
                    // There are intentionally no breaks
                    case 0:
                        if(idx < quantity_of_registers)
                            response[idx++] = htons(sense_co2());
                    case 1:
                        if(idx < quantity_of_registers)
                            response[idx++] = htons(sense_temperature());
                    default:
                    case 2:
                        if(idx < quantity_of_registers)
                            response[idx++] = htons(sense_humidity());
                }
                send(myAddress(), READ_HOLDING_REGISTER, reinterpret_cast<unsigned char *>(response), quantity_of_registers * sizeof(response[0]));
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
    cout << "CIPES CO2 / Temperature (Fahrenheit) / Humidity sensor" << endl;
    cout << "ID: " << hex << modbus_id << endl;
    cout << "Pins:" << endl
         << "   led: PC3" << endl
         << "   CO2: UART RX" << endl
         << "   Temperature: UART RX" << endl
         << "   Humidity: PD4/PD5" << endl;

    NIC * nic = new NIC();
    nic->address(NIC::Address::RANDOM);
    cout << "Address: " << nic->address() << endl;
    Secure_NIC * s = new Secure_NIC(false, new AES(), new Poly1305(new AES()), nic);

    Thing<Modbus> sensor(modbus_id, s);

    // The UART is used by the sensor, so don't print anything from here on
    auto u = new UART(9600, 8, 0, 1, 0);
    co2_temperature_sensor = new CM1101(u);
    humidity_sensor = new SHT11_Humidity('d', 4, 'd', 5);

    while(true);

    return 0;
}
