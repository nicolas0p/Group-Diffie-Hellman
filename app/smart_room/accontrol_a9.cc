// FIXME: Check if all these headers are really necessary
// FIXME: Ident this code properly, please!
#include <modbus_ascii.h>
#include <alarm.h>
#include <chronometer.h>
#include <secure_nic.h>
#include <utility/string.h>
#include <utility/key_database.h>
#include <nic.h>
#include <aes.h>
#include "home_thing.h"
#include <adc.h>
#include <machine/cortex_m/emote3_watchdog.h>
#include "CIPES/ac_control.h"

using namespace EPOS;

// FIXME: Is 'cout' needed for our final application?
OStream cout;
// FIXME: UART is not necessary
//UART uart;

// FIXME: These guys shouldn't be global
GPIO *ir_sig,*led;
bool coil0_state = false;
bool led_state = false;

// FIXME: These two should be the same
const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = { 'A', '9' };
const unsigned char modbus_id = 0xA9;

// FIXME: Are all of these methods really necessary?
void Receiver::update(Observed * o, Secure_NIC::Protocol p, Buffer * b)
{
    Frame * f = b->frame();
    char * _msg = f->data<char>();
    kout << "Received: " << b->size() << endl;
    for(int i = 0; i < b->size(); i++)
        kout << _msg[i];
    kout << endl;
    Modbus_ASCII::Modbus_ASCII_Feeder::notify(_msg, b->size());
    _nic->free(b);
}

void Sender::send(const char * c, int len)
{
    memcpy(_msg, c, len);
//    kout << "Sending: " << len << endl;
//    for (int i = 0; i < len; i++)
//        kout << c[i];
//    kout << endl;
    _nic->send(_nic->gateway_address, (const char *) _msg, len);
}

class Modbus: public Modbus_ASCII
{
public:
    Modbus(Modbus_ASCII_Sender * sender, unsigned char addr)
    : Modbus_ASCII(sender, addr), _ac(ir_sig) { }

    virtual ~Modbus() { }

    virtual void handle_command(unsigned char cmd, unsigned char * data, int data_len)
    {
        kout << "received command: " << hex << (int) cmd;
        for(int i = 0; i < data_len; ++i)
            kout << (int) data[i];
        kout << dec;
        unsigned short coil, value;
        unsigned short starting_address, quantity_of_registers;
        unsigned char coil_response;
        unsigned short response;
        int idx = 0;
        unsigned short output_address;
        unsigned short output_value;
        switch(cmd)
        {
            case READ_COILS:
                starting_address = (((unsigned short)data[0]) << 8) | data[1];
                coil_response = coil0_state;
                coil_response >>= starting_address;
                send(myAddress(), READ_COILS, reinterpret_cast<unsigned char *>(&coil_response), sizeof(unsigned char));
                break;
            case READ_HOLDING_REGISTER:
                starting_address = (((unsigned short)data[0]) << 8) | data[1];
                quantity_of_registers = (((unsigned short) data[2]) << 8) | data[3];
                if(quantity_of_registers > 1)
                    break;
                response = _ac.temperature();
                response = htons(response);
                send(myAddress(), READ_HOLDING_REGISTER, reinterpret_cast<unsigned char *>(&response), sizeof(unsigned short));
                break;
            case WRITE_SINGLE_REGISTER:
                starting_address = (((unsigned short)data[0]) << 8) | data[1];
                if(starting_address != 1)
                    break;
                ack();
                value = (((short)data[2]) << 8) | data[3];
                _ac.set_temperature((char) value);
                break;
            case WRITE_SINGLE_COIL:
                output_address = (((unsigned short)data[0]) << 8) | data[1];
                output_value = (((unsigned short)data[2]) << 8) | data[3];
                ack();
                if(output_address == 0)
                    coil0_state = output_value;
                else if(output_address == 9)
                    Machine::reboot();
                led_state = output_value;

                if(coil0_state)
                    _ac.turn_ac_on();
                else
                    _ac.turn_ac_off();
                led->set(led_state);
                break;
            default:
                break;
        }
    }
private:
    //FIXME: Classes should be declared in headers
    AC_Control _ac;
};

int main()
{
    cout << "CIPES AC actuator" << endl;
    cout << "ID: " << hex << modbus_id << endl;
    cout << "Pins:" << endl << "   ir_sig: PC3" << endl;

    ir_sig = new GPIO('b', 0, GPIO::OUTPUT);
    led = new GPIO('c',3, GPIO::OUTPUT);

    // FIXME: Useless shit below
    //AC_Control ac(ir_sig);

    NIC * nic = new NIC();
    nic->address(NIC::Address::RANDOM);
    cout << "Address: " << nic->address() << endl;
    Secure_NIC * s = new Secure_NIC(false, new AES(), new Poly1305(new AES()), nic);

    Thing<Modbus> sensor(modbus_id, s);

    eMote3_Watchdog::enable();
    while(true) {
        eMote3_Watchdog::kick();
    }

    return 0;
}
