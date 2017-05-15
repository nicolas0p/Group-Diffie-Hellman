#ifndef ultrasonic_sensor_hcsr04_hdr
#define ultrasonic_sensor_hcsr04_hdr

#include <timer.h>
#include <gpio.h>

using namespace EPOS;

class Ultrasonic_Sensor_Controller;

class Ultrasonic_Sensor_HC_SR04{
    typedef User_Timer_0 Timer;
    typedef unsigned int Sense;

    friend class Ultrasonic_Sensor_Controller;

public:
    static const unsigned int MAX_DISTANCE_CM = 400; //HC-SR04 maximum measure
    static const unsigned int MAX_MEASURE_TIME_US = 23200; // formula is cm = uS/58. 400*58 = 23,200
    static const unsigned int TIME_FRAME = 24000; // 800us extra for loose window.

protected:
    GPIO _trigger;
    GPIO _echo;

    #ifdef HCSR04_RELAY // if using relays
    GPIO _relay;

    Ultrasonic_Sensor_HC_SR04(GPIO relay,GPIO trigger,GPIO echo): _trigger(trigger), _echo(echo), _relay(relay){
        _trigger.output();
        _echo.input();
        _relay.output();
    }

    int enable() {_relay.set(); }
    int disable(){_relay.clear(); }

    #else
    Ultrasonic_Sensor_HC_SR04(GPIO trigger,GPIO echo): _trigger(trigger), _echo(echo){
        _trigger.output();
        _echo.input();
    }
    #endif

    Sense sense()
    {
        int elapsed_time;
        Timer timer(TIME_FRAME);

        //Make sure that the trigger was clear before.
        _trigger.clear();
        Timer::delay(10);

        //Protocol for triggering.
        _trigger.set();
        Timer::delay(10);
        _trigger.clear();

        //Wait until the echo is set to 1. To avoid infinite loops in case of sensor failure, a timer is used.
        timer.enable();
        while (!_echo.get() && timer.running());
        timer.disable();
        if(!_echo.get())
            return -1;

        timer.set(TIME_FRAME);
        //Start counting the time while the echo value is being calculated back or until a timeout on timer.
        timer.enable();
        while(_echo.get() && timer.running());
        timer.disable();

        elapsed_time = TIME_FRAME - timer.read();
        // the echo may get dissipated, in that case the timer will count all the way and the distance wouldn't be correcly calculated.
        return (elapsed_time > MAX_MEASURE_TIME_US)? -1 : elapsed_time/58;
    }
};

#endif
