#include <secure_nic.h>
#include <alarm.h>
#include <diffie_hellman.h>
#include <utility/bignum.h>
#include <poly1305.h>
#include <chronometer.h>
#include <aes.h>
#include <ieee802_15_4.h>
#include <utility/cipher.h>

//------------------------------------------------------
// Model application for the Secure_NIC
// For more information, refer to doc/security/howto.pdf
//------------------------------------------------------

// #define SERVER_ADDR NIC::Address(40)
// #define MY_ADDRESS NIC::Address(20)

using namespace EPOS;

const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'A','1'};

OStream cout;

NIC * nic;
//PTP * ptp;

// 128-bit key parameters
// const unsigned char Diffie_Hellman::default_base_point_x[] = {134, 91, 44, 165, 124, 96, 40, 12, 45, 155, 137, 139, 82, 247, 31, 22};
// const unsigned char Diffie_Hellman::default_base_point_y[] = {131, 122, 237, 221, 146, 162, 45, 192, 19, 235, 175, 91, 57, 200, 90, 207};
// const unsigned int Bignum::default_mod[4] = {4294967295U, 4294967295U, 4294967295U, 4294967293U};
// const unsigned int Bignum::default_barrett_u[5] = {17,8,4,2,1};
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

/*
// 160-bit key parameters
const unsigned char Diffie_Hellman::default_base_point_x[] = {
    0x82, 0xFC, 0xCB, 0x13, 0xB9, 0x8B, 0xC3, 0x68, 
    0x89, 0x69, 0x64, 0x46, 0x28, 0x73, 0xF5, 0x8E, 
    0x68, 0xB5, 0x96, 0x4A, 
};
const unsigned char Diffie_Hellman::default_base_point_y[] = {
    0x32, 0xFB, 0xC5, 0x7A, 0x37, 0x51, 0x23, 0x04,
    0x12, 0xC9, 0xDC, 0x59, 0x7D, 0x94, 0x68, 0x31,
    0x55, 0x28, 0xA6, 0x23,
};
const unsigned int Bignum::default_mod[5] = {2147483647U, 4294967295U, 4294967295U, 4294967295U, 4294967295U};
const unsigned int Bignum::default_barrett_u[6] = {2147483649, 0, 0, 0, 0, 1};
*/
/*
// 192-bit key parameters
const unsigned char Diffie_Hellman::default_base_point_x[] = {
    0x12, 0x10, 0xFF, 0x82, 0xFD, 0x0A, 0xFF, 0xF4,
    0x00, 0x88, 0xA1, 0x43, 0xEB, 0x20, 0xBF, 0x7C,
    0xF6, 0x90, 0x30, 0xB0, 0x0E, 0xA8, 0x8D, 0x18,
};
const unsigned char Diffie_Hellman::default_base_point_y[] = {
    0x11, 0x48, 0x79, 0x1E, 0xA1, 0x77, 0xF9, 0x73,
    0xD5, 0xCD, 0x24, 0x6B, 0xED, 0x11, 0x10, 0x63,
    0x78, 0xDA, 0xC8, 0xFF, 0x95, 0x2B, 0x19, 0x07,
};
const unsigned int Bignum::default_mod[6] = {4294967295U, 4294967295U, 4294967294U, 4294967295U, 4294967295U, 4294967295U};
const unsigned int Bignum::default_barrett_u[7] = {1, 0, 1, 0, 0, 0, 1};
*/

class Secure_NIC_Listener : public Secure_NIC::Observer
{
    typedef IEEE802_15_4::Protocol Protocol;
    typedef IEEE802_15_4::Buffer Buffer;
    typedef IEEE802_15_4::Frame Frame;
    typedef IEEE802_15_4::Observed Observed;
    public:
    Secure_NIC_Listener(Secure_NIC * s)
    {
        _s = s;
        _s->attach(this, Traits<Secure_NIC>::PROTOCOL_ID);
    }

    // This method will be called when a secure message arrives.
    // The message is delivered to the application already decrypted.
    void update(Secure_NIC::Observed * o, Secure_NIC::Protocol p, Secure_NIC::Buffer * b)
    {
        Frame * f = b->frame();
        char * d = f->data<char>();
        cout << "================" << endl;
        cout << "Received " << b->size() << " bytes of payload from " << f->src() << " :" << endl;
        for(int i=0; i<b->size(); i++)
            cout << d[i];
        cout << endl << "================" << endl;
        for(int i=0; i<b->size(); i++)
            cout << (int)d[i] << " / " << d[i] << endl;
        cout << "================" << endl;
        _s->free(b);
    }

    private:
    Secure_NIC * _s;
    Secure_NIC::Address from;
    char msg[32];
};
int sensor()
{
    Chronometer c;
    Secure_NIC::Address from;
    cout << "Sensor running \n";
    cout << "Key size: " << Secure_NIC::SECRET_SIZE << endl;
    cout << "ID size: " << Secure_NIC::ID_SIZE << endl;

    // Initialize PTP
    // This is a slave node, which will respond to sync messages
//  ptp->_ptp_parameters._original_clock_stratum = PTP::PTP_DataSet::STRATUM_SLAVE;
//  ptp->_ptp_parameters._clock_stratum = PTP::PTP_DataSet::STRATUM_SLAVE;
//  ptp->_ptp_parameters._state = PTP::INITIALIZING;
//  ptp->execute();

//     AES aes;
//     char plain[16];
//     char key[16];
//     char encrypted[16];
//     char decrypted[16];
//     for(int i=0;i<16;i++)
//     {
//         plain[i] = 0;
//         key[i] = 0;
//         encrypted[i] = 0;
//         decrypted[i] = 0;
//     }
//     plain[0] = id[0];
//     plain[1] = id[1];
//     key[0] = id[0];
//     key[1] = id[1];
//     aes.encrypt(plain, key, encrypted);
//     aes.encrypt(plain, key, encrypted);
//     aes.decrypt(encrypted, key, decrypted);
//     for(int i=0;i<16;i++)
//         kout << (int)encrypted[i] << " ";
//     kout << endl;
//     for(int i=0;i<16;i++)
//         kout << (int)decrypted[i] << " ";
//     kout << endl;
//     for(int i=0;i<16;i++)
//         kout << (int)plain[i] << " ";
//     kout << endl;
//     while(1);

    c.start();

    // Initialize the secure NIC
    Secure_NIC * s = new Secure_NIC(false, new AES(), new Poly1305(new AES()), nic);
    s->set_id(Traits<Build>::ID);
    do
    {
        cout << "Delaying..." << endl;
        Alarm::delay(Random::random() % 10000000);
        cout << "Up!" << endl;
        if(!s->authenticated())
        {
            cout << "Trying to negotiate key\n";
            // Try to authenticate
            s->send_key_request(nic->broadcast());
        }
    }
    while(!s->authenticated());
    
    // At this point, this sensor is authenticated and shares 
    // a symmetric key with the server

    c.stop();

    cout << "Key set!\nTotal time: " << c.read() << endl;

    // Listen to secure messages
    Secure_NIC_Listener listener(s);

    while(true)
    {
        Alarm::delay(Random::random() % 10000000);
        cout << "Sending hi\n";
        // Send an encrypted message to the server
        s->send(s->gateway_address, "Hi, server!", 12);
    }

    cout << "Done!\n";
    return 0;
}


void led_rgb_r(bool on_off=true)
{
//  GPIO_Pin led(10);
//  led.put(on_off);
}
int main()
{
    /*
    const int seed = 42;
    kout << "seed = " << seed << endl;
    const Bignum::digit p42[] = 
        {575052427U, 312824777U, 634607964U, 4243028311U, 397578615U, 3607799056U, 2751938651U, 2743054843U};
//     {1521883887U, 4267258932U, 2364650486U, 1274222045U, 759300338U, 2972921386U, 306754569U, 3082351043U};
//     {1124465781U, 1741806427U, 3406545347U, 1127514467U, 166853985U, 2876027855U, 620822005U, 3158026640U};
    const Bignum::digit p43[] = 
        {2091795153U, 1122870634U, 3524044454U, 2240181806U, 4033669282U, 1726166037U, 1719631538U, 871524628U};

//     {643706617U, 4027320560U, 4225703986U, 2211864739U, 2042443688U, 2006316801U, 4271345659U, 2740977465U};
//     {1621951116U, 1922693332U, 1659047664U, 1454347128U, 1674914175U, 3501362586U, 4077215519U, 1567874908U};

    Random::seed(seed);
    Diffie_Hellman dh;
    unsigned char key[40];
    if(seed==43)
        dh.calculate_key(key, 40, (unsigned char *)p42, (sizeof p42) * (sizeof p42[0]));
    else if(seed==42)
        dh.calculate_key(key, 40, (unsigned char *)p43, (sizeof p43) * (sizeof p43[0]));
    while(1);
    */
    nic = new NIC();
//  ptp = new PTP();
//  ptp->setNIC(nic);

//  nic->address(MY_ADDRESS);
//  cout << "Address: " << nic->address() << '\n';
    sensor();
    return 0;
}
