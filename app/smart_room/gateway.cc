#include <modbus_ascii.h>
#include <secure_nic.h>
#include <utility/string.h>
#include <utility/key_database.h>
#include <nic.h>
#include <aes.h>
#include <uart.h>
#include <gpio.h>

using namespace EPOS;

OStream cout;
GPIO * led;
UART uart(Traits<UART>::DEF_BAUD_RATE, Traits<UART>::DEF_DATA_BITS, Traits<UART>::DEF_PARITY, Traits<UART>::DEF_STOP_BITS, 1);

bool debugging_mode = false;
const char debug_mode_start[] = "DEBUG";
const char debug_mode_end[] = "/DEBUG";

const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'F','F'};

const unsigned char Diffie_Hellman::default_base_point_x[] = 
{       
    0x86, 0x5B, 0x2C, 0xA5,
    0x7C, 0x60, 0x28, 0x0C,
    0x2D, 0x9B, 0x89, 0x8B,
    0x52, 0xF7, 0x1F, 0x16
};
const unsigned char Diffie_Hellman::default_base_point_y[] =
{
    0x83, 0x7A, 0xED, 0xDD,
    0x92, 0xA2, 0x2D, 0xC0,
    0x13, 0xEB, 0xAF, 0x5B,
    0x39, 0xC8, 0x5A, 0xCF
};
const unsigned char Bignum::default_mod[] = 
{
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFD, 0xFF, 0xFF, 0xFF
};
const unsigned char Bignum::default_barrett_u[] = 
{
    17, 0, 0, 0, 
    8, 0, 0, 0, 
    4, 0, 0, 0, 
    2, 0, 0, 0, 
    1, 0, 0, 0
};

class Receiver : public Secure_NIC::Observer
{
public:
    typedef Secure_NIC::Buffer Buffer;
    typedef Secure_NIC::Frame Frame;
    typedef Secure_NIC::Observed Observed;

    Receiver(Secure_NIC * nic) : _nic(nic)
    {        
        _nic->attach(this, Traits<Secure_NIC>::PROTOCOL_ID);
    }   

    void update(Observed * o, Secure_NIC::Protocol p, Buffer * b)
    {
        int i=0;
        //             led_value = !led_value;
        //             led->set(led_value);
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
    Secure_NIC * _nic;
};

class Sender
{
public:
	Sender(Secure_NIC * nic) : _nic(nic) {}
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
					_nic->send(id, (const char *)_msg, len);
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
    Secure_NIC * _nic;
};

int main()
{
    cout << "Home Gateway" << endl;
    NIC * nic = new NIC();
    nic->address("BE:EF");

	Secure_NIC * s = new Secure_NIC(true, new AES(), new Poly1305(new AES()), nic);

	// These IDs are trusted
	char id[Traits<Build>::ID_SIZE];
    unsigned int n_ids = 0;
    for(char first = 'A'; first <= 'F'; first++) {
        const unsigned int limit = Traits<Secure_NIC>::MAX_PEERS;

        if(n_ids >= limit) {
            break;
        }

        id[0] = first;
        for(char second = '0'; second <= '9'; second++) {
            id[1] = second;
            s->insert_trusted_id(id);
            n_ids++;
            if(n_ids >= limit) {
                break;
            }
        }

        if(n_ids >= limit) {
            break;
        }
        for(char second = 'A'; second <= 'F'; second++) {
            id[1] = second;
            s->insert_trusted_id(id);
            n_ids++;
            if(n_ids >= limit) {
                break;
            }
        }
    }

	// Start accepting authentication requests
	s->accepting_connections = true;

    led = new GPIO{'c', 3, GPIO::OUTPUT};
    led->set();
    Receiver receiver(s);
    Sender sender(s);

    sender.run();

    return 0;
}
