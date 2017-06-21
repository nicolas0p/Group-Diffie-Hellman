// EPOS Moving Window Component Test Program

#include <tstp.h>
#include <utility/ostream.h>

using namespace EPOS;

OStream cout;

int main()
{
    Messages_Statistic statistics;

    cout << "Testing Messages_Statistic" << endl;

    cout << "Messages count: " << statistics._messages_count << endl;
    cout << "Window start: " << statistics._window_start << endl;

    cout << "Tests finished with " << endl;

    return 0;
}
