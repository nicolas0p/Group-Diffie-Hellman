// EPOS Application Scaffold and Application Component Implementation

#include <utility/ostream.h>
#include <application.h>
#include <network.h>

__BEGIN_SYS

// Application class attributes
char Application::_preheap[];
Heap * Application::_heap;

__END_SYS

__BEGIN_API

// Global objects
__USING_UTIL
OStream cout;
OStream cerr;

__END_API

__USING_SYS;
extern "C" {
    void __pre_main() {
        if(Traits<Network>::enabled)
            Network::init();
    }
}
