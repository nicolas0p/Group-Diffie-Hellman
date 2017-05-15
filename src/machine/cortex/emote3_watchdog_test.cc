#include <gpio.h>
#include <machine.h>
#include <machine/cortex_m/emote3_watchdog.h>

using namespace EPOS;

OStream cout;

void busy_wait(int limit = 0x3fffff) { for(volatile int i=0; i<limit; i++); }

void blink_led(GPIO & led, unsigned int times = 10, unsigned int delay = 0xffff)
{
    for(unsigned int i=0;i<10;i++)
    {
        led.set(true);
        busy_wait(delay);
        led.set(false);
        busy_wait(delay);
    }
}

int main()
{
    GPIO led('c', 3, GPIO::OUTPUT);

    blink_led(led);

    cout << "Enabling the watchdog" << endl;

    eMote3_Watchdog::enable();

    cout << "Kicking the watchdog for 3s" << endl;
    eMote3_GPTM timer(3, 3000000);
    timer.enable();
    while(timer.running()) {
        eMote3_Watchdog::kick();
    }

    cout << "Done! Stopped kicking the watchdog. Should reboot in 1s" << endl;

    while(true);

    return 0;
}
