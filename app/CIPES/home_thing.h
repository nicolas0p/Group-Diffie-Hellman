#include <alarm.h>
#include <nic.h>
#include <gpio.h>

__USING_SYS
//const unsigned char Diffie_Hellman::default_base_point_x[] = 
//{       
//    0x86, 0x5B, 0x2C, 0xA5,
//    0x7C, 0x60, 0x28, 0x0C,
//    0x2D, 0x9B, 0x89, 0x8B,
//    0x52, 0xF7, 0x1F, 0x16
//};
//const unsigned char Diffie_Hellman::default_base_point_y[] =
//{
//    0x83, 0x7A, 0xED, 0xDD,
//    0x92, 0xA2, 0x2D, 0xC0,
//    0x13, 0xEB, 0xAF, 0x5B,
//    0x39, 0xC8, 0x5A, 0xCF
//};
//const unsigned char Bignum::default_mod[] = 
//{
//    0xFF, 0xFF, 0xFF, 0xFF,
//    0xFF, 0xFF, 0xFF, 0xFF,
//    0xFF, 0xFF, 0xFF, 0xFF,
//    0xFD, 0xFF, 0xFF, 0xFF
//};
//const unsigned char Bignum::default_barrett_u[] = 
//{
//    17, 0, 0, 0, 
//    8, 0, 0, 0, 
//    4, 0, 0, 0, 
//    2, 0, 0, 0, 
//    1, 0, 0, 0
//};

class Receiver : public NIC::Observer, public Modbus_ASCII::Modbus_ASCII_Feeder
{
public:
    typedef NIC::Buffer Buffer;
    typedef NIC::Frame Frame;
    typedef NIC::Observed Observed;

    Receiver(NIC * nic) : _nic(nic)
    { _nic->attach(this, Traits<Secure_NIC>::PROTOCOL_ID); }   

    void update(Observed * o, NIC::Protocol p, Buffer * b);
private:
    NIC * _nic;
};

class Sender : public Modbus_ASCII::Modbus_ASCII_Sender
{
public:
    Sender(NIC * nic) : _nic(nic) {}
    virtual ~Sender() {}

    void send(const char * c, int len);

private:
    unsigned char _msg[Modbus_ASCII::MSG_LEN];
    NIC * _nic;
};

template<typename Modbus>
class Thing
{
    static const unsigned int MAX_AUTH_TRIALS = 5;

    void busy_wait(int limit = 0x3fffff) { for(volatile int i=0; i<limit; i++); }

    void blink_led(GPIO & led, unsigned int times = 10, unsigned int delay = 0xffff)
    {
        for(unsigned int i=0;i<10;i++)
        {
            led.set(true);
            busy_wait(delay);
            led.set(false);
            busy_wait(delay);
        }
    }

    public:
    Thing(int modbus_id, NIC * s)
    {
        OStream cout;
        _nic = s;
        GPIO led('c',3, GPIO::OUTPUT);
        blink_led(led);

        Receiver * receiver = new Receiver(s);
        Sender * sender = new Sender(s);
        modbus = new Modbus(sender, modbus_id);
        receiver->registerModbus(modbus);
    }

    Modbus * modbus;

private:
    NIC * _nic;
};
