#include <utility/ostream.h>
#include <rfid_reader.h>
#include <alarm.h>
#include <gpio.h>

using namespace EPOS;

OStream cout;

class Reader: private RFID_Reader::Observer
{
public:
    Reader() : _gpio0('D', 2, GPIO::IN), _gpio1('D', 4, GPIO::IN), _engine(0, &_gpio0, &_gpio1) {
        _engine.attach(this);
    }

    void update(RFID_Reader::Observed * obs) {
        cout << "Card read: ";
        RFID_Reader::UID u = _engine.get();
        cout << u << endl;
        _engine.halt(u);
    }

private:
    GPIO _gpio0;
    GPIO _gpio1;
    RFID_Reader _engine;
};

int main()
{
    Alarm::delay(2000000);

    Reader reader;

    cout << "You can place cards on the reader now" << endl;

    while(true);
}
