 #define HCSR04_RELAY

#include "sensor/ultrasonic_sensor_controller.h"
#include "gpio.h"
#include <utility/ostream.h>

using namespace EPOS;

OStream cout;

int main(){
    GPIO echo('c',7,GPIO::INPUT);
    GPIO trigger('c',6,GPIO::OUTPUT);
    GPIO relay('b',3,GPIO::OUTPUT);

    Ultrasonic_Sensor_Controller controller(relay,trigger,echo);

    Alarm::delay(10000000);
    cout << "starting the reads!!" << endl;
    while(1){
        int s = controller.sense();
        cout << "distance: " << s << endl;
        Alarm::delay(10000000);//5s
    }

    return 0;
}
