// EPOS Scheduler Test Program

#include <utility/ostream.h>
#include <periodic_thread.h>
#include <chronometer.h>

using namespace EPOS;

const unsigned int TASKS = 3;
const unsigned int TIMES = 5;

void task_a();
void task_b();
void task_c();
inline void exec(char c, unsigned int time = 0);
inline long max(unsigned int a, unsigned int b) { return (a >= b) ? a : b; }
inline long max(unsigned int a, unsigned int b, unsigned int c) { return ((a >= b) && (a >= c)) ? a : ((b >= a) && (b >= c) ? b : c); }

OStream cout;
Chronometer chrono;
Periodic_Thread * thread[TASKS];

// Task set (in ms)
struct Task_Set {
    void (* f)();
    int d;
    int p;
    int c;
    int a;
} set[TASKS] = {
    {&task_a, 100, 100, 50, 0},
    {&task_b,  80,  80, 20, 0},
    {&task_c,  60,  60, 10, 0}
};

void task_a() {
    exec('a', set[0].c);
}

void task_b()
{
    exec('b', set[1].c);
}

void task_c()
{
    exec('c', set[2].c);
}

int main()
{
    cout << "Periodic Thread Component Test" << endl;

    cout << "\nThis test consists in creating" << TASKS << "periodic threads as follows:" << endl;
    for(int i = 0; i < TASKS; i++) {
        cout << "  - After " << set[i].a << " ms, thread " << char ('A' + i) << " begins to print \"" << char ('a' + i)
             << "\" periodically each " << set[i].p << " ms." << endl;
        cout << "    Each " << char ('A' + i) << " job runs for " << set[i].c << " ms, printing additional \""
             << char ('a' + i) << "\"s as needed." << endl;
    }

    cout << "Threads will now be created and I'll wait for them to finish..." << endl;

    for(int i = 0; i < TASKS; i++)
        thread[i] = new RT_Thread(set[i].f, set[i].d * 1000, set[i].p * 1000, set[i].c * 1000, set[i].a * 1000, TIMES);

    chrono.start();
    exec('M');

    for(int i = 0; i < TASKS; i++)
        int ret = thread[i]->join();

    chrono.stop();
    exec('M');

    for(int i = 0; i < TASKS; i++)
        delete thread[i];

    cout << "\n... done!" << endl;

    int max_p = 0;
    for(int i = 0; i < TASKS; i++)
        max_p = max(max_p, set[i].p);

    cout << "\nThe estimated time to run the test was " << max_p * TIMES
         << " ms. The measured time was " << chrono.read() / 1000 <<" ms!" << endl;

    cout << "I'm also done, bye!" << endl;

    cout << "The end!" << endl;

    return 0;
}

inline void exec(char c, unsigned int time) // in miliseconds
{
    // Delay was not used here to prevent scheduling interference due to blocking
    // Alarm and Chronometer often use different timers, so a small variation is expected.
    Chronometer::Microsecond elapsed = chrono.read() / 1000;
    Chronometer::Microsecond end = elapsed + time;
    Chronometer::Microsecond last = -1;

    do {
        if(last != elapsed) {
            cout << begl << elapsed << "\t" << c;
            for(int i = 0; i < TASKS; i++)
                cout << "\tp(" << char('A' + i) << ")=" << thread[i]->priority();
            cout << endl;
            last = elapsed;
        }

        elapsed = chrono.read() / 1000;
    } while (end > elapsed);
}
