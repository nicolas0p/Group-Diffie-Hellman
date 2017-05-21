#include <transducer.h>

using namespace EPOS;

int main()
{
    IF<Traits<Hydro_Board>::enabled && Traits<Hydro_Board>::P7_enabled, bool, void>::Result p7_check; // Traits<Hydro_Board>::P7_enabled && Traits<Hydro_Board>::enabled must be true

    Rain rain(0, 1000000, Rain::ADVERTISED);

    Thread::self()->suspend();

    return 0;
}
