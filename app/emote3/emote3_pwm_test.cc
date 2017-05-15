#include <timer.h>
#include <gpio.h>
#include <pwm.h>

using namespace EPOS;

const unsigned int PWM_PERIOD = 100000;

OStream cout;

int main()
{
    Percent duty_cycle = 50;

    cout << "eMote3 PWM test" << endl;

    GPIO led('C', 3, GPIO::OUT);
    User_Timer timer(0, PWM_PERIOD, 0);

    PWM pwm(&timer, &led, duty_cycle);

    while(true) {
        Machine::delay(1000000);
        pwm.duty_cycle(duty_cycle = (duty_cycle + 10) % 100);
    }

    return 0;
}
