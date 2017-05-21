#include <machine.h>
#include <gpio.h>
#include <alarm.h>
#include <utility/ostream.h>
#include <spi.h>
#include <cpu.h>
#include <machine/cortex_m/emote3_pwm.h>
#include <machine/cortex_m/emote3_gptm.h>
#include <nic.h>
#include <alarm.h>
#include "samsung_ac_message.h" //struct Message

using namespace EPOS;
const unsigned int PWM_FREQUENCY = 37900; // 37.9 KHz

OStream cout;
GPIO * led;

bool led_value;

const char GATEWAY_ADDR[2] = {'F','0'};
const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'F','1'};

class Samsung_AC
{
public:
    Samsung_AC () {
        //_pwm = new eMote3_PWM(1, PWM_FREQUENCY, 0, 'd', 2);
        _d2 = new GPIO('d', 2, GPIO::OUTPUT);
    }

    enum Mode {
        COOL    = 0x08,
        DRY     = 0x04,
        FAN     = 0x0C,
        HEAT    = 0x02,
        AUTO    = 0x00,
    };

    enum Fan {
        FAN1            = 0x02,
        FAN2            = 0x01,
        FAN3            = 0x06,
        FAN_AUTO1       = 0x00, //Dry, Hot, Cool
        FAN_AUTO2       = 0x03, //AUTO Mode
    };

    enum Turbo {
        TURBO_ON      = 0x03,
        TURBO_OFF     = 0x00,
    };

    enum Swing {
        SWING_OFF      = 0x7F,
        SWING_ON     = 0xF5,
    };

    enum Temperature {
        T30 = 0x07,
        T29 = 0x0B,
        T28 = 0x03,
        T27 = 0x0D,
        T26 = 0x05,
        T25 = 0x09,
        T24 = 0x01,
        T23 = 0x0E,
        T22 = 0x06,
        T21 = 0x0A,
        T20 = 0x02,
        T19 = 0x0C,
        T18 = 0x04,
        T17 = 0x08,
        T16 = 0x00,
    };

    void send_command(Mode mode, Fan fan, Temperature temperature, Turbo turbo, Swing swing) {
        unsigned int tmp = mode << 17 | fan  << 14 | temperature <<  10 | turbo << 8 | swing;

        unsigned int count = 0;
        for(unsigned int i = 0; i < 21; i++) {
            if((tmp & (1 << i)))
                count++;
        }

        count += 5; //0x80 + 0x8E

        unsigned char crc;
        switch(count) {
            case 13:
                crc = 0x40;
                break;
            case 14:
                crc = 0x47;
                break;
            case 15:
                crc = 0x4B;
                break;
            case 16:
                crc = 0x43;
                break;
            case 17:
                crc = 0x4D;
                break;
            case 18:
                crc = 0x45;
                break;
            default:
                crc = 0x40;
                break;
        }

        //cout << "count = " << count << endl;
        //cout << "Header = " << (void *) 0x80 << " crc = " << (void *) crc << " swing = " << (void *) swing <<
        //    " turbo = " << (void *) ((turbo << 5) | 0x8E) << " temp = " << (void *) temperature <<
        //    " fan = " << (void *) ((fan << 4) | mode | 0x80) << endl;

        //first block is always the same
        send_header();
        send_byte(0x40);
        send_byte(0x49);
        send_byte(0xF0);
        send_byte(0x00);
        send_byte(0x00);
        send_byte(0x00);
        send_byte(0x0F);
        send_bit('1');

        send_inter_block();
        send_byte(0x80); //block header
        send_byte(crc); //CRC
        send_byte(swing); //Swing
        send_byte((turbo << 5) | 0x8E); //Turbo
        send_byte(temperature); //Temperature
        send_byte((fan << 4) | mode | 0x80); //Fan + Mode
        send_byte(0x0F);
        send_bit('1');
    }

    void turn_on() {
        send_header();
        send_byte(0x40);
        send_byte(0x49);
        send_byte(0xF0);
        send_byte(0x00);
        send_byte(0x00);
        send_byte(0x00);
        send_byte(0x0F);
        send_bit('1');

        send_inter_block();

        send_byte(0x80);
        send_byte(0x4B);
        send_byte(0xF0);
        send_byte(0x00);
        send_byte(0x00);
        send_byte(0x00);
        send_byte(0x00);
        send_bit('1');

        send_inter_block();

        send_byte(0x80);
        send_byte(0x43);
        send_byte(0x7F);
        send_byte(0x8E);
        send_byte(0x01);
        send_byte(0xAC);
        send_byte(0x0F);
        send_bit('1');
    }

    void turn_off() {
        send_header();
        send_byte(0x40);
        send_byte(0x4D);
        send_byte(0xF0);
        send_byte(0x00);
        send_byte(0x00);
        send_byte(0x00);
        send_byte(0x03);
        send_bit('1');

        send_inter_block();

        send_byte(0x80);
        send_byte(0x4B);
        send_byte(0xF0);
        send_byte(0x00);
        send_byte(0x00);
        send_byte(0x00);
        send_byte(0x00);
        send_bit('1');

        send_inter_block();

        send_byte(0x80);
        send_byte(0x47);
        send_byte(0x7F);
        send_byte(0x8E);
        send_byte(0x01);
        send_byte(0xAC);
        send_byte(0x03);
        send_bit('1');
    }

private:

    void send_byte(unsigned char byte) {
        unsigned int i;
        i = 8;
        while(i > 0) {
            if(byte & (1 << (i - 1)))
                send_bit('1');
            else
                send_bit('0');
            i--;
        }
    }

    //send bit 0 or 1 according to the parameter
    void send_bit(unsigned char bit) {
        //_pwm->set(PWM_FREQUENCY, 33);
        _d2->set(0);
        eMote3_GPTM::delay(560);
        //_pwm->set(PWM_FREQUENCY, 0);
        _d2->set(1);
        if(bit == '1')
            eMote3_GPTM::delay(1690);
        else
            eMote3_GPTM::delay(560);
    }

    void send_header() {
        //_pwm->set(PWM_FREQUENCY, 33);
        _d2->set(0);
        eMote3_GPTM::delay(800);
        //_pwm->set(PWM_FREQUENCY, 0);
        _d2->set(1);
        eMote3_GPTM::delay(18000);
        //_pwm->set(PWM_FREQUENCY, 33);
        _d2->set(0);
        eMote3_GPTM::delay(3000);
        //_pwm->set(PWM_FREQUENCY, 0);
        _d2->set(1);
        eMote3_GPTM::delay(9000);
    }

    void send_inter_block() {
        //_pwm->set(PWM_FREQUENCY, 0);
        _d2->set(1);
        eMote3_GPTM::delay(1460);
        //_pwm->set(PWM_FREQUENCY, 33);
        _d2->set(0);
        eMote3_GPTM::delay(3040);
        //_pwm->set(PWM_FREQUENCY, 0);
        _d2->set(1);
        eMote3_GPTM::delay(8900);
    }

private:
    //eMote3_PWM * _pwm;
    GPIO * _d2;
};

class Receiver : public NIC::Observer
{
public:
    typedef NIC::Protocol Protocol;
    typedef NIC::Buffer Buffer;
    typedef NIC::Frame Frame;
    typedef NIC::Observed Observed;

    Receiver(NIC * nic) : _nic(nic), _gateway_address(), _ac()
    {
        _gateway_address[0] = GATEWAY_ADDR[0];
        _gateway_address[1] = GATEWAY_ADDR[1];
        _msg = new Message;
        _nic->attach(this, Traits<Secure_NIC>::PROTOCOL_ID);
        _pd0 = new GPIO('d', 0, GPIO::OUTPUT);
        _pd4 = new GPIO('d', 4, GPIO::OUTPUT);
        _pd0->set(1);
        _pd4->set(0);
    }

    void update(Observed * o, Protocol p, Buffer * b)
    {
        if(p == Traits<Secure_NIC>::PROTOCOL_ID)
        {
            led_value = !led_value;
            led->set(led_value);
            Frame * f = b->frame();
            Message * d = f->data<Message>();
            cout << endl << "=====================" << endl;
            cout << "Received " << b->size() << " bytes of payload from " << f->src() << " :" << endl;
            if(f->src() == _gateway_address) {
                cout << "Command = " << d->command << " Temp = " << d->temp << " mode = " << d->mode << " fan = " << d->fan << " turbo  = " << d->turbo << " swing = " << d->swing << endl;
                cout << endl << "=====================" << endl;
                _nic->free(b);
                memcpy(_msg, d, sizeof(Message));

                switch(_msg->command) {

                case '0':
                    cout << "Turning it on!\n";
                    _pd0->set(0);
                    _pd4->set(1);
                    eMote3_GPTM::delay(500);
                    _ac.turn_on();
                    eMote3_GPTM::delay(500);
                    _pd0->set(1);
                    _pd4->set(0);
                    break;
                case '1':
                    cout << "Turning it off!\n";
                    _pd0->set(0);
                    _pd4->set(1);
                    eMote3_GPTM::delay(500);
                    _ac.turn_off();
                    eMote3_GPTM::delay(500);
                    _pd0->set(1);
                    _pd4->set(0);
                    break;
                case '2':
                    cout << "Sending command..\n";
                    _pd0->set(0);
                    _pd4->set(1);
                    eMote3_GPTM::delay(500);
                    _ac.send_command((Samsung_AC::Mode) _msg->mode, (Samsung_AC::Fan) _msg->fan, (Samsung_AC::Temperature) _msg->temp,
                               (Samsung_AC::Turbo) _msg->turbo, (Samsung_AC::Swing) _msg->swing);
                    eMote3_GPTM::delay(500);
                    _pd0->set(1);
                    _pd4->set(0);
                    break;
                default:
                    cout << "Command unkown!\n";
                    break;
                }
            }
        }
    }

private:
    NIC * _nic;
    Message * _msg;
    NIC::Address _gateway_address;
    Samsung_AC _ac;
    GPIO * _pd0;
    GPIO * _pd4;
};

int main(void)
{
    cout << "Samsung Air Conditioning Controller" << endl;

    led = new GPIO('c',3, GPIO::OUTPUT);
    NIC * nic = new NIC();
    NIC::Address addr;
    addr[0] = Traits<Build>::ID[0];
    addr[1] = Traits<Build>::ID[1];
    nic->address(addr);
    Receiver * r = new Receiver(nic);

    while(true) {
        Alarm::delay(100000000);
    }

    return 0;
}
