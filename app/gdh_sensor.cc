#include <machine.h>
#include <thread.h>
#include <smart_data.h>

using namespace EPOS;

OStream cout;

int main()
{
    GPIO g('C',3, GPIO::OUT); //led
	bool b = false;

	for(int i = 0; i < 10; ++i) {
		b = !b;
		g.set(b); //blink the led
		for(volatile int t=0;t<0xfffff;t++);
	}

    cout << "TSTP Sensor test" << endl;

    cout << "My machine ID is:";
    for(unsigned int i = 0; i < 8; i++)
        cout << " " << hex << Machine::id()[i];
    cout << endl;
    cout << "You can set this value at src/component/tstp_init.cc to set initial coordinates for this mote." << endl;

    cout << "My coordinates are " << TSTP::here() << endl;
    cout << "The time now is " << TSTP::now() << endl;
    cout << "I am" << (TSTP::here() == TSTP::sink() ? " " : " not ") << "the sink" << endl;

    Acceleration a(0, 1000000, Acceleration::ADVERTISED);

	int g_id = 1;

	while(TSTP::GDH_Security::get_group_ids().size() == 0) {
		b = !b;
		g.set(b); //blink the led
		for(volatile int t=0;t<0xffffff;t++);
		cout << "Not yet!" << endl;
	}

	cout << "The final key for group id " << g_id << " is " << TSTP::GDH_Security::key(g_id) << endl;

    Thread::self()->suspend();

    return 0;
}
