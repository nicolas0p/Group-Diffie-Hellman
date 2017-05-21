#include <machine.h>
#include <alarm.h>
#include <smart_data.h>

using namespace EPOS;

OStream cout;

const unsigned int INTEREST_PERIOD = 1000000;
const unsigned int INTEREST_EXPIRY = 2 * INTEREST_PERIOD;

int main()
{
    cout << "TSTP Gateway test" << endl;

    cout << "My machine ID is:";
    for(unsigned int i = 0; i < 8; i++)
        cout << " " << hex << Machine::id()[i];
    cout << endl;
    cout << "You can set this value at src/component/tstp_init.cc to set initial coordinates for this mote." << endl;

    cout << "My coordinates are " << TSTP::here() << endl;
    cout << "The time now is " << TSTP::now() << endl;
    cout << "I am" << (TSTP::here() == TSTP::sink() ? " " : " not ") << "the sink" << endl;

    Coordinates center_sensor(10,10,0);
    Region region_sensor(center_sensor, 0, TSTP::now(), -1);

    cout << "I will now ask for Acceleration data from any sensor located in " << region_sensor << endl;

    Acceleration acceleration(region_sensor, INTEREST_EXPIRY, INTEREST_PERIOD);

    while(true) {
        Alarm::delay(INTEREST_PERIOD);
        cout << "Acceleration in " << acceleration.location() << " at " << acceleration.time() << " was " << acceleration << endl;
    }

    return 0;
}
