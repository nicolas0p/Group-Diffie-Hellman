#include <alarm.h>

#include "../include/machine/cortex_m/uart.h"
#include "../include/machine/cortex_m/cm1101.h"

using namespace EPOS;

int main()
{
    UART u(9600, 8, 0, 1, 0);
    CM1101 cm1101(&u);
    NIC * nic = new NIC();
    char data[6];
    int co2, temp, humid;

    while(1) {
        co2 = cm1101.co2();
        temp = cm1101.temp();
        humid = cm1101.humid();

        data[0] = co2&0xff;
        data[1] = (co2>>8)&0xff;
        data[2] = temp&0xff;
        data[3] = (temp>>8)&0xff;
        data[4] = humid&0xff;
        data[5] = (humid>>8)&0xff;

        nic->send(nic->broadcast(), 0x1010, data, sizeof data);

        Alarm::delay(1000000);
    }

    return 0;
}
