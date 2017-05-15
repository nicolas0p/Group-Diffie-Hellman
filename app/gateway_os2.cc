#include <smart_data.h>
#include <utility/ostream.h>
#include <alarm.h>

using namespace EPOS;

const TSTP::Time DATA_PERIOD = 10 * 1000000;
const TSTP::Time DATA_EXPIRY = 2 * DATA_PERIOD;
const TSTP::Time INTEREST_EXPIRY = 5 * 60 * 1000000;

int main()
{
    // Interest center points
    TSTP::Coordinates center_lux(300, 320, 40);
    TSTP::Coordinates center_lamp(-140, 50, 250);

    // Regions of interest
    TSTP::Time start = TSTP::now();
    TSTP::Time end = start + INTEREST_EXPIRY;
    TSTP::Region region_lux(center_lux, 0, start, end);
    TSTP::Region region_lamp(center_lamp, 0, start, end);

    // Data of interest
    Luminous_Intensity data_lux(region_lux, DATA_EXPIRY, DATA_PERIOD);
    Current data_lamp(region_lamp, DATA_EXPIRY, DATA_PERIOD);

    OStream cout;

    while(TSTP::now() < end) {
        Alarm::delay(DATA_PERIOD);

	Luminous_Intensity::Value lux = data_lux;
	Current::Value set;

	//lux 0 = escuro, 1833 = claro
	set = (lux*98/1833) + 2; //2 = claro, 100 = escuro
	data_lamp = set;
    }

    cout << "Done!" << endl;

    return 0;
}
