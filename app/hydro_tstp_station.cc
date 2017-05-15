#include <alarm.h>
#include <transducer.h>

using namespace EPOS;

int main()
{
    if(Traits<Hydro_Board>::P3_enabled)
        new Water_Level(0, 3000000, Water_Level::ADVERTISED);

    // P5 is used for turbidity infrared actuator

    if(Traits<Hydro_Board>::P6_enabled)
        new Water_Turbidity(3, 3700000, Water_Turbidity::ADVERTISED);

    if(Traits<Hydro_Board>::P7_enabled)
        new Rain(0, 1000000, Rain::ADVERTISED);

    Thread::self()->suspend();

    return 0;
}
