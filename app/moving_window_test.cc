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

    Delay delay(1000000);
}

int main()
{
    cout << "Testing Messages_Statistic" << endl;

    create_window(100);
    create_window(200);
    create_window(50);
    create_window(10);
    create_window(500);


    cout << "Average: " << TSTP::Messages_Statistic::messages_count_average() << 
	    ". Expected: " << (100 + 200 + 50 + 10 + 500)/5 << endl;
    
    create_window(30);
   
    cout << "Average: " << TSTP::Messages_Statistic::messages_count_average() << 
	    ". Expected: " << (30 + 200 + 50 + 10 + 500)/5 << endl;

    create_window(0);
    create_window(0);

    cout << "Average: " << TSTP::Messages_Statistic::messages_count_average() << 
	    ". Expected: " << (30 + 0 + 0 + 10 + 500)/5 << endl;

    
    create_window(150);

    cout << "Average: " << TSTP::Messages_Statistic::messages_count_average() << 
	    ". Expected: " << (30 + 0 + 0 + 150 + 500)/5 << endl;

    cout << "Tests finished with " << endl;

    return 0;
}
