// EPOS Periodic Thread Component Test Program

#include <utility/ostream.h>
#include <periodic_thread.h>
#include <chronometer.h>

using namespace EPOS;

const int iterations = 100;
const long period_a = 100; // ms
const long period_b = 300; // ms
const long period_c = 500; // ms

int func_a(void);
int func_b(void);
int func_c(void);
long max(long a, long b, long c) { return ((a >= b) && (a >= c)) ? a : ((b >= a) && (b >= c) ? b : c); }

OStream cout;

int main()
{
    cout << "Periodic Thread Component Test" << endl;

    cout << "\nThis test consists in creating three periodic threads as follows:" << endl;
    cout << "  Thread 1 prints \"a\" every " << period_a << " ms;" << endl;
    cout << "  Thread 2 prints \"b\" every " << period_b << " ms;" << endl;
    cout << "  Thread 3 prints \"c\" every " << period_c << " ms." << endl;

    Periodic_Thread thread_a(RTConf(period_a * 1000, iterations), &func_a);
    Periodic_Thread thread_b(RTConf(period_b * 1000, iterations), &func_b);
    Periodic_Thread thread_c(RTConf(period_c * 1000, iterations), &func_c);

    cout << "Threads have been created. I'll wait for them to finish...\n" << endl;

    Chronometer chrono;
    chrono.start();

    int status_a = thread_a.join();
    int status_b = thread_b.join();
    int status_c = thread_c.join();

    chrono.stop();

    cout << "\n\nThread A exited with status " << status_a
         << ", thread B exited with status " << status_b
         << " and thread C exited with status " << status_c << "." << endl;

    cout << "\nThe estimated time to run the test was "
         << max(period_a, period_b, period_c) * iterations
         << " ms. The measured time was " << chrono.read() / 1000 <<" ms!" << endl;

    cout << "I'm also done, bye!" << endl;

    return 0;
}

int func_a()
{
    cout << "A";
    for(int i = 0; i < iterations; i++) {
        Periodic_Thread::wait_next();
        cout << "a";
    }
    cout << "A";
    return 'A';
}

int func_b(void)
{
    cout << "B";
    for(int i = 0; i < iterations; i++) {
        Periodic_Thread::wait_next();
        cout << "b";
    }
    cout << "B";
    return 'B';
}

int func_c(void)
{
    cout << "C";
    for(int i = 0; i < iterations; i++) {
        Periodic_Thread::wait_next();
        cout << "c";
    }
    cout << "C";
    return 'C';
}
