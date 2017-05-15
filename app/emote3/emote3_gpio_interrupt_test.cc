#include <machine.h>
#include <alarm.h>
#include <gpio.h>

using namespace EPOS;

OStream cout;

GPIO * led;

void handler(GPIO * pin)
{
    cout << "User handler" << endl;
    cout << "GPIO pin state: " << pin->get() << endl;
    led->set(pin->get());
}

void other(GPIO * pin)
{
    cout << "Other handler" << endl;
}

void test_output()
{
    cout << "Creating GPIO" << endl;
    GPIO a('b',4,GPIO::OUTPUT);
    cout << "Clearing GPIO" << endl;
    a.clear();

    cout << "Enabling interrupt" << endl;
    a.enable_interrupt(GPIO::BOTH_EDGES, &handler);
    while(1)
    {
        cout << "delaying 2 seconds" << endl;
        Alarm::delay(2000000);
        cout << "setting GPIO pin" << endl;
        a.set();
        cout << "delaying 2 seconds" << endl;
        Alarm::delay(2000000);
        cout << "clearing GPIO pin" << endl;
        a.clear();
    }
}

void test_input()
{
    cout << "Creating GPIO" << endl;
    GPIO a('b',4,GPIO::INPUT);
    cout << "Clearing GPIO" << endl;
    a.clear();

    cout << "Enabling interrupt" << endl;
    a.enable_interrupt(GPIO::BOTH_EDGES, &handler);

    cout << "Creating GPIO" << endl;
    GPIO b('b',5,GPIO::INPUT);
    cout << "Clearing GPIO" << endl;
    b.clear();

    cout << "Enabling interrupt" << endl;
    b.enable_interrupt(GPIO::BOTH_EDGES, &other);


    while(1);
}

int main()
{
    cout << "Creating LED" << endl;
    led = new GPIO('c', 3, GPIO::OUTPUT);
    cout << "Clearing LED" << endl;
    led->clear();

    //test_output();
    test_input();
    return 0;
}
