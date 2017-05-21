// EPOS PC_Timer Test Program

#include <utility/ostream.h>
#include <machine.h>
#include <display.h>
#include <timer.h>

using namespace EPOS;

OStream cout;

void handler()
{
    static int elapsed;

    int lin, col;
    Display::position(&lin, &col);
    Display::position(0, 60 + Machine::cpu_id() * 2);
    Display::putc((elapsed++ % 10) + 48);
    Display::position(lin, col);
}

int main()
{
    cout << "PC_Timer test" << endl;

    User_Timer timer(10000, handler);
    
    for(int i = 0; i < 10000; i++);
    cout << "count = " << timer.read() << "" << endl;
    for(int i = 0; i < 10000; i++);
    cout << "count = " << timer.read() << "" << endl;
    for(int i = 0; i < 10000; i++);
    cout << "count = " << timer.read() << "" << endl;
    for(int i = 0; i < 10000; i++);
    cout << "count = " << timer.read() << "" << endl;
    for(int i = 0; i < 10000; i++);
    cout << "count = " << timer.read() << "" << endl;
    for(int i = 0; i < 10000; i++);
    cout << "count = " << timer.read() << "" << endl;
    
    cout << "The End!" << endl;

    return 0;
}
