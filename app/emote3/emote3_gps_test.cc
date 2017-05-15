#include <machine.h>
#include <gpio.h>
#include <utility/ostream.h>
#include <spi.h>
#include <cpu.h>
#include <machine/cortex_m/emote3_gptm.h>

using namespace EPOS;

OStream cout;

class GPS {
public:
    GPS() : _spi(0, Traits<CPU>::CLOCK, SPI::FORMAT_MOTO_1, SPI::MASTER, 1000000, 8),
        _gps_on_off('d', 0, GPIO::OUTPUT),
        _gps_tm('d', 1, GPIO::INPUT),
        _gps_reset('d', 2, GPIO::OUTPUT),
        _gps_wakeup('d', 3, GPIO::OUTPUT)
    {
        _gps_reset.set(1);
        _gps_wakeup.set(1);
        _gps_on_off.clear();
        eMote3_GPTM::delay(200000); //200ms
        _gps_on_off.set(1);
        eMote3_GPTM::delay(200000); //200ms
        _gps_on_off.clear();
    }

    char read() {
        _spi.put_data_non_blocking(0);
        while(_spi.is_busy()) ;
        return (char) _spi.get_data_non_blocking();
    }

private:
    SPI _spi;
    GPIO _gps_on_off;
    GPIO _gps_tm;
    GPIO _gps_reset;
    GPIO _gps_wakeup;
};

int main()
{
    GPIO g('c', 3, GPIO::OUTPUT);
    GPS gps;

    while(1) {
       g.set(1);
       eMote3_GPTM::delay(1000000);
       g.clear();
       for(unsigned int i = 0; i < 80; i++) {
           cout << gps.read();
       }
       cout << "\n";
    }
    return 0;
}

