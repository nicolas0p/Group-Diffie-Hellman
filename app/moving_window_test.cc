// EPOS Moving Window Component Test Program

#include <tstp.h>
#include <utility/ostream.h>

using namespace EPOS;

OStream cout;

void create_window(int messages_count) {

    int i;
    for (i = 0; i < messages_count; i++) {
      TSTP::Messages_Statistic::update_test();
    }

    cout << "now= " << TSTP::now() << endl;
    cout << "sample_start_time= " << TSTP::Messages_Statistic::_sample_start_time << endl;
    cout << "messages_count= " << TSTP::Messages_Statistic::_current_sample_messages_count << endl;

    Delay delay(5000000);
}

void print_windows() {
    int i;
    for (i = 0; i < TSTP::Messages_Statistic::WINDOWS_MAX_SIZE; i ++) {
      cout << "Elemento[" << i << "] = " << TSTP::Messages_Statistic::_windows[i] << endl;
    }
}

int main()
{
    cout << "Testing Messages_Statistic" << endl;

    create_window(100);
    create_window(200);

    print_windows();

    cout << "Average: " << TSTP::Messages_Statistic::messages_count_average()<< endl;

    cout << "Tests finished with " << endl;

    return 0;
}
