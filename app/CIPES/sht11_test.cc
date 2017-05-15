#include <alarm.h>

#include "../include/machine/cortex_m/sht11.h"

using namespace EPOS;

int main()
{
    SHT11_Humidity sht11('d', 4, 'd', 5);
    NIC * nic = new NIC();
    char data[2];
    int humid;

    while(1) {
        humid = sht11.sample();

        data[0] = humid&0xff;
        data[1] = (humid>>8)&0xff;

        nic->send(nic->broadcast(), 0x1010, data, sizeof data);

        Alarm::delay(1000000);
    }

    return 0;
}
