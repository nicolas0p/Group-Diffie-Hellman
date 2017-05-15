#include <modbus_ascii.h>
#include <secure_nic.h>
#include <utility/string.h>
#include <utility/key_database.h>
#include <nic.h>
#include <aes.h>
#include <uart.h>

using namespace EPOS;

OStream cout;
UART uart(Traits<UART>::DEF_BAUD_RATE, Traits<UART>::DEF_DATA_BITS, Traits<UART>::DEF_PARITY, Traits<UART>::DEF_STOP_BITS, 1);

bool debugging_mode = false;
const char debug_mode_start[] = "DEBUG";
const char debug_mode_end[] = "/DEBUG";
//bool led_value = false;
//GPIO led('c',3,GPIO::OUTPUT);

const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'E','E'};

class Receiver : public NIC::Observer
{
public:
    typedef NIC::Buffer Buffer;
    typedef NIC::Frame Frame;
    typedef NIC::Observed Observed;

    Receiver(NIC * nic) : _nic(nic)
    {        
        _nic->attach(this, Traits<Secure_NIC>::PROTOCOL_ID);
    }   

    void update(Observed * o, NIC::Protocol p, Buffer * b)
    {
        int i=0;
        //led_value = !led_value;
        //led.set(led_value);
        Frame * f = b->frame();
        char * _msg = f->data<char>();

        if(!Modbus_ASCII::check_format(_msg, b->size()))
        {
            _nic->free(b);
            return;
        }

        while((_msg[i] != '\r') && (_msg[i+1] != '\n')) 
        {
            uart.put(_msg[i++]);
        }
        uart.put('\r');
        uart.put('\n');

//        cout << endl << "=====================" << endl;
//        cout << "Received " << b->size() << " bytes of payload from " << f->src() << " :" << endl;
        for(int i=0; i<b->size(); i++)
            cout << _msg[i];
//        cout << endl << "=====================" << endl;
        _nic->free(b);
        //            _nic->stop_listening();
    }

private:
    NIC * _nic;
};

class Sender
{
public:
    Sender(NIC * nic) : _nic(nic) {}
    virtual ~Sender() {}

    virtual int run()
    {
        while(true)
        {
            int len;
            bool ok_message = false;
            while(!ok_message) {
                ok_message = true;
                len = 0;
                _msg[len++] = uart.get();
                _msg[len++] = uart.get();
                while(!((_msg[len-2] == '\r') and (_msg[len-1] == '\n'))) {
                    _msg[len++] = uart.get();
                    if(len >= Modbus_ASCII::MSG_LEN) {
                        ok_message = false;
                        break;
                    }
                }
            }

            // Account for \0
            if(((len-1) == sizeof(debug_mode_start)) && (!strncmp(debug_mode_start, _msg, sizeof(debug_mode_start)-1)))
            {
                debugging_mode = true;
                cout << "Entered debugging mode" << endl;
            }
            else if(((len-1) == sizeof(debug_mode_end)) && (!strncmp(debug_mode_end, _msg, sizeof(debug_mode_end)-1)))
            {
                debugging_mode = false;
                cout << "Exited debugging mode" << endl;
            }
            else
            {
                memset(_msg+len, 0x00, Modbus_ASCII::MSG_LEN-len);
                char id[3];
                id[0] = _msg[1];
                id[1] = _msg[2];
                id[2] = 0;

                if(debugging_mode)
                {
                    unsigned char lrc = 0;
                    for(int i=1; i<len-2; i+=2)
                        lrc += Modbus_ASCII::decode(_msg[i], _msg[i+1]);

                    lrc = ((lrc ^ 0xff) + 1) & 0xff;
                    Modbus_ASCII::encode(lrc, &_msg[len-2], &_msg[len-1]);

                    _msg[len] = '\r';
                    _msg[len+1] = '\n';
                    len+=2;

                    cout << "Sending: ";
                    for(int i=0;!(_msg[i-2] == '\r' && _msg[i-1] == '\n');i++)
                        cout << (char)_msg[i];
                    cout << endl;
                }

                if(Modbus_ASCII::check_format(_msg, len)) {
                    NIC::Address to;
                    to[0] = _msg[1];
                    to[1] = _msg[2];
                    //GPIO led('c',3,GPIO::OUTPUT);
                    //led.set(true);
                    //for(unsigned int i = 0; (i < 100) and (_nic->send(to, Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len) < 0); i++);
                    //for(unsigned int i = 0; (i < 10) and (_nic->send(to, Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len) < 0); i++);
                    for(unsigned int i = 0; (i < 20); i++) {
                        _nic->send(to, Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len);
                    }
                    //_nic->send(to, Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len);
                    //while(_nic->send(to, Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len) < 0);
                    //_nic->send(_nic->broadcast(), Traits<Secure_NIC>::PROTOCOL_ID, (const char *)_msg, len);
                    //led.set(false);
                }
                else {
                    cout << "Wrong syntax! Message not sent!" << endl;
                }
            }
        }

        return 0;
    }

private:
    char _msg[Modbus_ASCII::MSG_LEN];
    NIC * _nic;
};

int main()
{
    cout << "Home Gateway" << endl;
    NIC * nic = new NIC();
    NIC::Address addr;
    addr[0] = Traits<Build>::ID[0];
    addr[1] = Traits<Build>::ID[1];
    nic->address(addr);
    cout << "Address: " << nic->address() << endl;

    Receiver receiver(nic);
    Sender sender(nic);

    sender.run();

    return 0;
}
