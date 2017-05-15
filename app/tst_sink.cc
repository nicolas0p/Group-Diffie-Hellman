// EPOS cout Test Program

#include <utility/ostream.h>
#include <utility/random.h>
#include <network.h>
#include <wiegand.h>

using namespace EPOS;

OStream cout;
TSTP * tstp;

Wiegand::Door_State_1 door_1;
Wiegand::Door_State_2 door_2;
Wiegand::Door_State_3 door_3;
Wiegand::Door_State_4 door_4;
Wiegand::RFID_1 rfid_1;
Wiegand::RFID_2 rfid_2;
Wiegand::RFID_3 rfid_3;
Wiegand::RFID_4 rfid_4;

TSTP::Meter * m;

void hello_door_state(TSTP::Interest * s)
{
    cout << "Received door state update!" << endl;
//    cout << "Interest = " << *s << endl;
    cout << "Value = " << *(s->data<bool>()) << endl;
    cout << "Time = " << s->last_update() << endl;
    cout << "now = " << s->tstp()->time() << endl;
}

void hello_rfid(TSTP::Interest * s)
{
    cout << "Received rfid update!" << endl;
//    cout << "Interest = " << *s << endl;
    auto v = s->data<Wiegand::ID_Code_Msg>();
    cout << "Serial = " << v->serial << endl;
    cout << "Facility = " << v->facility << endl;
    cout << "Time = " << s->last_update() << endl;
    cout << "now = " << s->tstp()->time() << endl;
}

void init()
{
    int n = 25000;
    while(n--) {
        cout << n << endl;
    }
    Network::init();
    tstp = TSTP::get_by_nic(0);
    if(!tstp) {
        while(1) {
            cout << "Waaaahhh" << endl;
        }
    }
    tstp->bootstrap();
}

void interest()
{
    while(true) {
        auto t0 = tstp->time();
        //auto i = new TSTP::Interest(tstp, m, TSTP::Remote_Address(10, 20, 30, 20), t0 + 2000000, t0 + 60000000, 10000000, 1, TSTP::RESPONSE_MODE::SINGLE_EVENT_DRIVEN, &hello_interest);
        TSTP::Interest ids1  (tstp, &door_1, TSTP::Remote_Address(10, 20, 30, 20), t0 + 1000000, t0 + 10000000, 3000000, 1, TSTP::RESPONSE_MODE::SINGLE_EVENT_DRIVEN, &hello_door_state);
        TSTP::Interest irfid1(tstp, &rfid_1, TSTP::Remote_Address(10, 20, 30, 20), t0 + 1000000, t0 + 10000000, 3000000, 1, TSTP::RESPONSE_MODE::SINGLE_EVENT_DRIVEN, &hello_rfid);

        cout << "Created interest: " << ids1 << endl;
        cout << "Created interest: " << irfid1 << endl;
        Alarm::delay(15000000);
    }
    while(true);
}

int main()
{
    init();
    interest();
    while(true);

    return 0;
}
