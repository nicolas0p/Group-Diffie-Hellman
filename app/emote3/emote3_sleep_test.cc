#include <utility/ostream.h>
#include <alarm.h>
#include <machine.h>
#include <gpio.h>
#include <usb.h>

using namespace EPOS;

OStream cout;

void handler(GPIO * pin)
{
    cout << "Handler!" << endl;
}

int main()
{
    while(!eMote3_USB::initialized());
    Alarm::delay(1000000);

    cout << "Creating GPIO" << endl;
    GPIO a('b',4,GPIO::INPUT);
    cout << "Clearing GPIO" << endl;
    a.clear();

    cout << "Enabling interrupt" << endl;
    a.enable_interrupt(GPIO::BOTH_EDGES, &handler);

    cout << "Going to sleep..." << endl;
    int n = 0;
    eMote3::wake_up_on(eMote3::WAKE_UP_EVENT::GPIO_B);
    Alarm::delay(500000);
    eMote3::power_mode(eMote3::POWER_MODE_3);
    while(1)
        cout << "Woke up!" << endl;
    
    return 0;
}
