#include <alarm.h>
#include <adc.h>

#include <machine/cortex_m/emote3_power_meter.h>

using namespace EPOS;

int main()
{
    Power_Meter pm(ADC::SINGLE_ENDED_ADC0, ADC::SINGLE_ENDED_ADC1, ADC::GND);
    NIC * nic = new NIC();
    char data[2];
    int p;

    while(1) {
        p = pm.average();

        data[0] = p&0xff;
        data[1] = (p>>8)&0xff;

        nic->send(nic->broadcast(), 0x1010, data, sizeof data);

        Alarm::delay(1000000);
    }

    return 0;
}
