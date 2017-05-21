#include <smart_data.h>

using namespace EPOS;

ADC_Sensor::Observed ADC_Sensor::_observed; // TODO

int main()
{
    Luminous_Intensity lux(6, 1000000, Luminous_Intensity::ADVERTISED);

    Thread::self()->suspend();

    return 0;
}
