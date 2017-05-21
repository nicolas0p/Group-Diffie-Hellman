#include <utility/ostream.h>
#include <utility/string.h>
#include <uart.h>
#include <gpio.h>
#include <machine.h>
#include <alarm.h>

__USING_SYS



int main() {
    OStream cout;

    /*GSM Pins*/

    auto led = GPIO{'c', 3, GPIO::OUTPUT};
    led.set();

    GPIO gsm_status('d', 5, GPIO::INPUT); //GPIO28 - Pino 36
    gsm_status.input();

    GPIO gsm_pwrkey('d', 3, GPIO::OUTPUT); //GPIO9 - Pino 25
    gsm_pwrkey.output();

    // to 0
    //gsm_pwrkey.clear();
    //
    // to 1
    //gsm_pwrkey.set();

    UART uart1{115200, 8, 0, 1, 0};
    UART uart2{9600, 8, 0, 1, 1};

    cout << "Starting..." << endl;

    while(1) {

        if (uart1.has_data()) {
            char data1 = uart1.get();
            //cout << "Data 1: " << data1 << endl;
            if (data1 == '#') {
                // ON
		cout << "# ON" << endl;
                gsm_pwrkey.set();
                Alarm::delay(2000000);
                gsm_pwrkey.clear();
            }

            if (data1 == '*') {
                // OFF
                gsm_pwrkey.set();
                Alarm::delay(800000);
                gsm_pwrkey.clear();
		cout << "* OFF" << endl;
            }

            uart2.put(data1);
        }
        if (uart2.has_data()) {
            char data2 = uart2.get();
            //cout << "Data 2: " << data2 << endl;
            uart1.put(data2);
        }
    }
}
