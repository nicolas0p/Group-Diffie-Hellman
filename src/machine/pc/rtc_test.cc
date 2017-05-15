// EPOS RTC Test Program

#include <utility/ostream.h>
#include <display.h>
#include <rtc.h>

const unsigned int TEST_DURATION = 20; // s

using namespace EPOS;

int main()
{
    OStream cout;

    cout << "RTC test" << endl;

    RTC rtc;
    RTC::Second t0 = rtc.seconds_since_epoch();

    cout << "It's now " << t0 << " seconds since epoch." << endl;

    RTC::Date last_date = rtc.date();
    while(rtc.seconds_since_epoch() < t0 + TEST_DURATION) {
        if(last_date.second() != rtc.date().second()) {
            last_date = rtc.date();
            Display::position(20, 30);
            cout << last_date.day() << '/'
        	 << last_date.month() << '/'
        	 << last_date.year() << ' ';
            Display::position(20, 42);
            cout << last_date.hour() << ':'
        	 << last_date.minute() << ':'
        	 << last_date.second() << "    ";
        }
    }

    cout << "\n\nSetting the time to its current value: ";
    rtc.date(rtc.date());
    cout << "done!" << endl;

    cout << "The End!" << endl;

    return 0;
}
