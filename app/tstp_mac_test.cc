#include <smart_data.h>
#include <utility/ostream.h>
#include <nic.h>
#include <gpio.h>
#include <periodic_thread.h>

using namespace EPOS;

OStream cout;

unsigned int seed;

class Serial_Sensor: public Keyboard
{
public:
    static const unsigned int UNIT = TSTP::Unit::Acceleration;
    static const unsigned int NUM = TSTP::Unit::I32;
    static const int ERROR = 0; // Unknown

    static const bool INTERRUPT = true;
    static const bool POLLING = false;

public:
    Serial_Sensor() {}

    static void sense(unsigned int dev, Smart_Data<Serial_Sensor> * data) {
        data->_value = seed++;
    }

    static void actuate(unsigned int dev, Smart_Data<Serial_Sensor> * data, void * command) {}
};

typedef Smart_Data<Serial_Sensor> Serial;

Coordinates c1, c2, c3;

int main()
{
    cout << "TSTP MAC test" << endl;
    cout << "Configuration: " << endl;

    c1 = Coordinates(0, 100, 0);
    c2 = Coordinates(100, 100, 0);
    c3 = Coordinates(100, 0, 0);

    cout << c1 << " " << c2 << " " << c3 << endl;
    /*
    cout << "INT_HANDLING_DELAY = " << TSTP_MAC<CC2538RF>::INT_HANDLING_DELAY << endl;
    cout << "TX_DELAY = " << TSTP_MAC<CC2538RF>::TX_DELAY << endl;
    cout << "G = " << TSTP_MAC<CC2538RF>::G << endl;
    cout << "Tu = " << TSTP_MAC<CC2538RF>::Tu << endl;
    cout << "Ti = " << TSTP_MAC<CC2538RF>::Ti << endl;
    cout << "TIME_BETWEEN_MICROFRAMES = " << TSTP_MAC<CC2538RF>::TIME_BETWEEN_MICROFRAMES << endl;
    cout << "Ts = " << TSTP_MAC<CC2538RF>::Ts << endl;
    cout << "MICROFRAME_TIME = " << TSTP_MAC<CC2538RF>::MICROFRAME_TIME << endl;
    cout << "Tr = " << TSTP_MAC<CC2538RF>::Tr << endl;
    cout << "RX_MF_TIMEOUT = " << TSTP_MAC<CC2538RF>::RX_MF_TIMEOUT << endl;
    cout << "NMF = " << TSTP_MAC<CC2538RF>::NMF << endl;
    cout << "N_MICROFRAMES = " << TSTP_MAC<CC2538RF>::N_MICROFRAMES << endl;
    cout << "CI = " << TSTP_MAC<CC2538RF>::CI << endl;
    cout << "PERIOD = " << TSTP_MAC<CC2538RF>::PERIOD << endl;
    cout << "SLEEP_PERIOD = " << TSTP_MAC<CC2538RF>::SLEEP_PERIOD << endl;
    cout << "DUTY_CYCLE = " << TSTP_MAC<CC2538RF>::DUTY_CYCLE << endl;
    cout << "DATA_LISTEN_MARGIN = " << TSTP_MAC<CC2538RF>::DATA_LISTEN_MARGIN << endl;
    cout << "DATA_SKIP_TIME = " << TSTP_MAC<CC2538RF>::DATA_SKIP_TIME << endl;
    cout << "RX_DATA_TIMEOUT = " << TSTP_MAC<CC2538RF>::RX_DATA_TIMEOUT << endl;
    cout << "CCA_TIME = " << TSTP_MAC<CC2538RF>::CCA_TIME << endl;
    */

    cout << "sizeof(Microsecond) = " << sizeof(RTC_Common::Microsecond) << endl;
    cout << "sizeof(CC2538RF::Timer::Time_Stamp) = " << sizeof(CC2538RF::Timer::Time_Stamp) << endl;

    cout << "Machine::id() =";
    for(unsigned int i = 0; i < 8; i++)
        cout << " " << hex << Machine::id()[i];
    cout << endl;

    cout << "TSTP::here() = " << TSTP::here() << endl;

    while(Traits<NIC>::promiscuous)
        Thread::self()->suspend();

    if(TSTP::here() == TSTP::sink()) {
        cout << "Sink" << endl;

        const unsigned char id1[] = "\x00\x4b\x12\x00\xec\x82\x0d\x06";
        //const unsigned char id2[] = "\x00\x4b\x12\x00\xae\x82\x0d\x06";
        const unsigned char id2[] = "\x00\x4b\x12\x00\xca\x0e\x16\x06";
        //const unsigned char id3[] = "\x00\x4b\x12\x00\x67\x83\x0d\x06";
        const unsigned char id3[] = "\x00\x4b\x12\x00\xee\x0e\x16\x06";
        TSTP::Security::add_peer(id1, sizeof(id1), Region(c1, 0, TSTP::now(), -1));
        TSTP::Security::add_peer(id2, sizeof(id2), Region(c2, 0, TSTP::now(), -1));
        TSTP::Security::add_peer(id3, sizeof(id3), Region(c3, 0, TSTP::now(), -1));

        Delay(4 * 10 * 1000 * 1000);

        while(true) {
            while(true)
            {
                const unsigned int PERIOD = 4 * TSTP_MAC<CC2538RF>::PERIOD +  3 * 2 * TSTP_MAC<CC2538RF>::PERIOD;

                cout << "Sending Interest 1" << endl;
                Region dst1(c1, 0, TSTP::now(), TSTP::now() + 10 * PERIOD);
                Serial a1(dst1, 3 * PERIOD, PERIOD);

                cout << "Sending Interest 2" << endl;
                Region dst2(c2, 0, TSTP::now(), TSTP::now() + 10 * PERIOD);
                Serial a2(dst2, 3 * PERIOD, PERIOD);

                cout << "Sending Interest 3" << endl;
                Region dst3(c3, 0, TSTP::now(), TSTP::now() + 10 * PERIOD);
                Serial a3(dst3, 3 * PERIOD, PERIOD);

                cout << "now = " << TSTP::now() << endl;

                Delay(3 * PERIOD);

                while(dst1.contains(dst1.center, TSTP::now())) {
                    Delay(1 * PERIOD);
                    cout << "a1 = " << a1 << endl;
                    cout << "a2 = " << a2 << endl;
                    cout << "a3 = " << a3 << endl;
                }

                {
                    Delay(3 * PERIOD);
                }
            }
        }
    } else {
        cout << "Sensor" << endl;

        if(TSTP::here() == c1)
            seed = 1000000;
        else if(TSTP::here() == c2)
            seed = 2000000;
        else if(TSTP::here() == c3)
            seed = 3000000;

        Serial a0(0, 250000, Serial::ADVERTISED);

        unsigned int n = 0;
        while(true) {
            Delay(10000000);
            cout << "Ping " << n++ << endl;
        }
    }

    return 0;
}
