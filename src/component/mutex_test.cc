// EPOS Mutex Component Test Program

#include <utility/ostream.h>
#include <thread.h>
#include <mutex.h>
#include <alarm.h>
#include <display.h>

using namespace EPOS;

const int iterations = 10;

Mutex mutex_display;

Thread * phil[5];
Mutex chopstick[5];

OStream cout;

int philosopher(int n, int l, int c)
{
    int first = (n < 4)? n : 0;
    int second = (n < 4)? n + 1 : 4;

    for(int i = iterations; i > 0; i--) {

        mutex_display.lock();
        Display::position(l, c);
        cout << "thinking";
        mutex_display.unlock();

        Delay thinking(2000000);

        chopstick[first].lock();   // get first chopstick
        chopstick[second].lock();   // get second chopstick

        mutex_display.lock();
        Display::position(l, c);
        cout << " eating ";
        mutex_display.unlock();

        Delay eating(1000000);

        chopstick[first].unlock();   // release first chopstick
        chopstick[second].unlock();   // release second chopstick
    }

    mutex_display.lock();
    Display::position(l, c);
    cout << "  done  ";
    mutex_display.unlock();

    return(iterations);
}

int main()
{
    mutex_display.lock();
    Display::clear();
    cout << "The Philosopher's Dinner:" << endl;

    phil[0] = new Thread(&philosopher, 0,  5, 32);
    phil[1] = new Thread(&philosopher, 1, 10, 44);
    phil[2] = new Thread(&philosopher, 2, 16, 39);
    phil[3] = new Thread(&philosopher, 3, 16, 24);
    phil[4] = new Thread(&philosopher, 4, 10, 20);

    cout << "Philosophers are alive and hungry!" << endl;

    Display::position(7, 44);
    cout << '/';
    Display::position(13, 44);
    cout << '\\';
    Display::position(16, 35);
    cout << '|';
    Display::position(13, 27);
    cout << '/';
    Display::position(7, 27);
    cout << '\\';
    Display::position(18, 0);

    cout << "The dinner is served ..." << endl;
    mutex_display.unlock();

    for(int i = 0; i < 5; i++) {
        int ret = phil[i]->join();
        mutex_display.lock();
        Display::position(20 + i, 0);
        cout << "Philosopher " << i << " ate " << ret << " times " << endl;
        mutex_display.unlock();
    }

    for(int i = 0; i < 5; i++)
        delete phil[i];

    cout << "The end!" << endl;

    return 0;
}
