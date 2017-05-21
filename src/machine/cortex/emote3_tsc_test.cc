// EPOS TimeStampCounter Test Program

#include <utility/ostream.h>
#include <tsc.h>

using namespace EPOS;

void handler(const unsigned int & id)
{
    kout << "Okay!" << endl;
}

int main()
{
    unsigned int t1, t2;
    OStream cout;

    auto int_time = TSC::time_stamp() + (2*TSC::frequency());
    cout << "Setting interrupt. If it works, the next message should be \"Okay!\": " << endl;
    TSC::wake_up_at(int_time, &handler);
    int_time+=100;
    while(TSC::time_stamp() < int_time);

    cout << "Setting time stamp to 0" << endl;
    TSC::offset(-TSC::time_stamp());
    
    cout << "Counting 3 seconds" << endl;
    do
    {
        t1 = TSC::time_stamp() / TSC::frequency();
        t2 = TSC::time_stamp() / TSC::frequency();
        while(t1 == t2)
            t2 = TSC::time_stamp() / TSC::frequency();
        cout << t2 << endl;
    } while(t2 < 3);

    cout << "Setting offset of +100 seconds" << endl;
    TSC::offset(100*TSC::frequency());
    cout << "Counting up to 110" << endl;
    do
    {
        t1 = TSC::time_stamp() / TSC::frequency();
        t2 = TSC::time_stamp() / TSC::frequency();
        while(t1 == t2)
            t2 = TSC::time_stamp() / TSC::frequency();
        cout << t2 << endl;
    } while(t2 < 110);

    cout << "Subtracting 20 seconds from offset" << endl;
    TSC::offset(-20*TSC::frequency());
    cout << "Counting up to 96" << endl;
    do
    {
        t1 = TSC::time_stamp() / TSC::frequency();
        t2 = TSC::time_stamp() / TSC::frequency();
        while(t1 == t2)
            t2 = TSC::time_stamp() / TSC::frequency();
        cout << t2 << endl;
    } while(t2 < 96);


    cout << "Done!" << endl;

    return 0;
}
