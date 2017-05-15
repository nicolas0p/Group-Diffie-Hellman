// EPOS Application Binding

#include <utility/spin.h>
#include <utility/ostream.h>
#include <application.h>
#include <cpu.h>
#include <framework/main.h>

// Framework class attributes
__BEGIN_SYS
Framework::Cache Framework::_cache;
__END_SYS


// Global objects
__BEGIN_SYS
OStream kerr;
__END_SYS


// Bindings
extern "C" {
    void _panic() { _API::Thread::exit(-1); }
    void _exit(int s) { _API::Thread::exit(s); }

    // Utility methods that differ from kernel and user space.
    // Heap
    static _UTIL::Simple_Spin _heap_spin;
    void _heap_lock() { _heap_spin.acquire(); }
    void _heap_unlock() { _heap_spin.release();}
}

__USING_SYS;
extern "C" {
    void _syscall(void * m) { CPU::syscall(m); }
    void _print(const char * s) {
        Message msg(Id(UTILITY_ID, 0), Message::PRINT, reinterpret_cast<unsigned int>(s));
        msg.act();
    }
    void _print_preamble() {}
    void _print_trailler(bool error) {}
}
