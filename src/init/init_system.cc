// EPOS System Initializer

#include <utility/random.h>
#include <machine.h>
#include <system.h>
#include <address_space.h>
#include <segment.h>

__BEGIN_SYS

class Init_System
{
private:
    static const unsigned int HEAP_SIZE = Traits<System>::HEAP_SIZE;

public:
    Init_System() {
        db<Init>(TRC) << "Init_System()" << endl;

        Machine::smp_barrier();

        // Only the boot CPU runs INIT_SYSTEM fully
        if(Machine::cpu_id() != 0) {
            // Wait until the boot CPU has initialized the machine
            Machine::smp_barrier();
            // For IA-32, timer is CPU-local. What about other SMPs?
            Timer::init();
            return;
        }

        // Initialize the processor
        db<Init>(INF) << "Initializing the CPU: " << endl;
        CPU::init();
        db<Init>(INF) << "done!" << endl;

        // Initialize System's heap
        db<Init>(INF) << "Initializing system's heap: " << endl;
        if(Traits<System>::multiheap) {
            System::_heap_segment = new (&System::_preheap[0]) Segment(HEAP_SIZE, WHITE, Segment::Flags::SYS);
            System::_heap = new (&System::_preheap[sizeof(Segment)]) Heap(Address_Space(MMU::current()).attach(System::_heap_segment, Memory_Map::SYS_HEAP), System::_heap_segment->size());
        } else
            System::_heap = new (&System::_preheap[0]) Heap(MMU::alloc(MMU::pages(HEAP_SIZE)), HEAP_SIZE);
        db<Init>(INF) << "done!" << endl;

        // Initialize the machine
        db<Init>(INF) << "Initializing the machine: " << endl;
        Machine::init();
        db<Init>(INF) << "done!" << endl;

        Machine::smp_barrier(); // signalizes "machine ready" to other CPUs

        // Initialize system abstractions
        db<Init>(INF) << "Initializing system abstractions: " << endl;
        System::init();
        db<Init>(INF) << "done!" << endl;

        // Randomize the Random Numbers Generator's seed
        if(Traits<Random>::enabled) {
            db<Init>(INF) << "Randomizing the Random Numbers Generator's seed: " << endl;
            if(Traits<TSC>::enabled)
                Random::seed(TSC::time_stamp());
#ifdef __NIC_H
            if(Traits<NIC>::enabled) {
                NIC nic;
                Random::seed(Random::random() ^ nic.address());
            }
#endif
#ifdef __ADC_H
            if(Traits<ADC>::enabled) {
                ADC adc;
                Random::seed(Random::random() ^ adc.read());
            }
#endif
            if(!Traits<TSC>::enabled && !Traits<NIC>::enabled)
                db<Init>(WRN) << "Due to lack of entropy, Random is a pseudo random numbers generator!" << endl;
            db<Init>(INF) << "done!" << endl;
        }

        // Initialization continues at init_first
    }
};

// Global object "init_system" must be constructed first.
Init_System init_system;

__END_SYS
