// EPOS System Binding

#include <utility/spin.h>
#include <machine.h>
#include <display.h>
#include <thread.h>

extern "C" {
    __USING_SYS;

    // Libc legacy
    void _panic() { Machine::panic(); }
    void _exit(int s) { Thread::exit(s); }
    void __exit() { Thread::exit(CPU::fr()); }  // must be handled by the Page Fault handler for user-level tasks
    void __cxa_pure_virtual() { db<void>(ERR) << "Pure Virtual method called!" << endl; }

    // Utility-related methods that differ from kernel and user space.
    // OStream
    void _print(const char * s) { Display::puts(s); }
    static volatile int _print_lock = -1;
    void _print_preamble() {
        static char tag[] = "<0>: ";

        int me = Machine::cpu_id();
        int last = CPU::cas(_print_lock, -1, me);
        for(int i = 0, owner = last; (i < 10) && (owner != me); i++, owner = CPU::cas(_print_lock, -1, me));
        if(last != me) {
            tag[1] = '0' + Machine::cpu_id();
            _print(tag);
        }
    }
    void _print_trailler(bool error) {
        static char tag[] = " :<0>";

        if(_print_lock != -1) {
            tag[3] = '0' + Machine::cpu_id();
            _print(tag);

            _print_lock = -1;
        }
        if(error)
            _panic();
    }

    // Heap
    static Spin _heap_spin;
    void _heap_lock() {
        _heap_spin.acquire();
        CPU::int_disable();
    }
    void _heap_unlock() {
        _heap_spin.release();
        CPU::int_enable();
    }
}
