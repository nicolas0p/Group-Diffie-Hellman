#include <smart_data.h>

using namespace EPOS;

ADC_Sensor::Observed ADC_Sensor::_observed; // TODO

int main()
{
    Temperature temperature(6, 1000000, Temperature::ADVERTISED);

    Thread::self()->suspend();

    return 0;
}
