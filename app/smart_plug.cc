#include <transducer.h>

using namespace EPOS;

int main()
{
    Current c0(0, 1000000, Current::COMMANDED);

    Thread::self()->suspend();

    return 0;
}
