#define HCSR04_RELAY

#include "sensor/ultrasonic_sensor_controller.h"
#include <alarm.h>
#include <gpio.h>
#include <utility/ostream.h>

using namespace EPOS;

OStream cout;

int main(){
    GPIO echo('C',7,GPIO::IN);
    GPIO trigger('C',6,GPIO::OUT);
    GPIO relay('B',3,GPIO::OUT);

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
