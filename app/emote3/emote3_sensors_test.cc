#include <alarm.h>
#include <utility/ostream.h>
#include <i2c.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "EPOSMote III I2C Sensor Test" << endl;

    I2C_Temperature_Sensor ts;
    I2C_Humidity_Sensor hs;

    while(true) {
        Alarm::delay(1000000);
        cout << "relative humidity = " << hs.get() << "%" << endl;
        cout << "temperature = " << ts.get() << "C" << endl;
    }

    return 0;
}
