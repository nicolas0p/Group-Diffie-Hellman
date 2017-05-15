#include <alarm.h>
#include <transducer.h>
#include <utility/ostream.h>

using namespace EPOS;

OStream cout;

int main()
{
	Switch_Sensor sensor(0, 'C', 3, GPIO::OUT);
	Switch data(0, 10000000, Switch::COMMANDED);
	
	while(true);
		//cout << "Hello";
	return 0;
}
