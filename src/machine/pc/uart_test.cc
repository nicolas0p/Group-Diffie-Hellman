// EPOS PC UART Mediator Test Program

#include <utility/ostream.h>
#include <uart.h>

using namespace EPOS;

int main()
{
    OStream cout;

    cout << "UART test\n" << endl;

    UART uart(0, 115200, 8, 0, 1);

    cout << "Loopback transmission test (conf = 115200 8N1):";
    uart.loopback(true);

    for(int i = 0; i < 256; i++) {
        uart.put(i);
        int c = uart.get();
        if(c != i)
            cout << " failed (" << c << ", should be " << i << ")!" << endl;
    }
    cout << " passed!" << endl;

    cout << "Link transmission test (conf = 9200 8N1):";
    uart.config(9600, 8, 0, 1);
    uart.loopback(false);

    for(int i = 0; i < 256; i++) {
        uart.put(i);
        for(int j = 0; j < 0xffffff; j++);
        int c = uart.get();
        if(c != i)
            cout << " failed (" << c << ", should be " << i << ")!" << endl;
    }
    cout << " passed!" << endl;

    return 0;
}
