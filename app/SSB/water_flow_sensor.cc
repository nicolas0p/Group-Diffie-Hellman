#include <transducer.h>

using namespace EPOS;

int main()
{
    IF<Traits<Hydro_Board>::enabled && Traits<Hydro_Board>::P7_enabled, bool, void>::Result p7_check; // Traits<Hydro_Board>::P7_enabled && Traits<Hydro_Board>::enabled must be true

    Water_Flow water_flow(0, 1000000, Water_Flow::ADVERTISED);

    Thread::self()->suspend();

    return 0;
}
