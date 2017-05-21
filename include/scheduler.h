// EPOS Scheduler Component Declarations

#ifndef __scheduler_h
#define __scheduler_h

#include <utility/list.h>
#include <cpu.h>
#include <machine.h>

__BEGIN_SYS

// All scheduling criteria, or disciplines, must define operator int() with
// the semantics of returning the desired order of a given object within the
// scheduling list
namespace Scheduling_Criteria
{
    // Priority (static and dynamic)
    class Priority
    {
        friend class _SYS::RT_Thread;

    public:
        enum {
            MAIN   = 0,
            HIGH   = 1,
            NORMAL = (unsigned(1) << (sizeof(int) * 8 - 1)) - 3,
            LOW    = (unsigned(1) << (sizeof(int) * 8 - 1)) - 2,
            IDLE   = (unsigned(1) << (sizeof(int) * 8 - 1)) - 1
        };

        static const bool timed = false;
        static const bool dynamic = false;
        static const bool preemptive = true;

    public:
        Priority(int p = NORMAL): _priority(p) {}

        operator const volatile int() const volatile { return _priority; }

        void update() {}
        unsigned int queue() const { return 0; }

    protected:
        volatile int _priority;
    };

    // Round-Robin
    class RR: public Priority
    {
    public:
        enum {
            MAIN   = 0,
            NORMAL = 1,
            IDLE   = (unsigned(1) << (sizeof(int) * 8 - 1)) - 1
        };

        static const bool timed = true;
        static const bool dynamic = false;
        static const bool preemptive = true;

    public:
        RR(int p = NORMAL): Priority(p) {}
    };

    // First-Come, First-Served (FIFO)
    class FCFS: public Priority
    {
    public:
        enum {
            MAIN   = 0,
            NORMAL = 1,
            IDLE   = (unsigned(1) << (sizeof(int) * 8 - 1)) - 1
        };

        static const bool timed = false;
        static const bool dynamic = false;
        static const bool preemptive = false;

    public:
        FCFS(int p = NORMAL); // Defined at Alarm
    };


    // Multicore Algorithms
    class Variable_Queue
    {
    public:
        enum {ANY = -1};

    protected:
        Variable_Queue(unsigned int queue): _queue(queue) {};

    public:
        const volatile unsigned int & queue() const volatile { return _queue; }

    protected:
        volatile unsigned int _queue;
        static volatile unsigned int _next_queue;
    };

    // Global Round-Robin
    class GRR: public RR
    {
    public:
        static const unsigned int HEADS = Traits<Machine>::CPUS;

    public:
        GRR(int p = NORMAL): RR(p) {}

        static unsigned int current_head() { return Machine::cpu_id(); }
    };

    // CPU Affinity
    class CPU_Affinity: public Priority, public Variable_Queue
    {
    public:
        static const bool timed = false;
        static const bool dynamic = false;
        static const bool preemptive = true;

        static const unsigned int QUEUES = Traits<Machine>::CPUS;

    public:
        CPU_Affinity(int p = NORMAL, int cpu = ANY)
        : Priority(p), Variable_Queue(((_priority == IDLE) || (_priority == MAIN)) ? Machine::cpu_id() : (cpu != ANY) ? cpu : ++_next_queue %= Machine::n_cpus()) {}

        using Variable_Queue::queue;

        static unsigned int current_queue() { return Machine::cpu_id(); }
    };


    // Real-time Algorithms
    class RT_Common: public Priority
    {
    protected:
        typedef RTC::Microsecond Microsecond;

    public:
        enum {
            PERIODIC    = HIGH,
            APERIODIC   = NORMAL
        };

        // Constructor helpers
        enum {
            SAME        = 0,
            NOW         = 0,
            UNKNOWN     = 0,
            INFINITE    = RTC::INFINITE,
            ANY         = Variable_Queue::ANY
        };

    protected:
        RT_Common(int p): Priority(p), _deadline(0), _period(0), _capacity(0) {} // Aperiodic
        RT_Common(int i, const Microsecond & d, const Microsecond & p, const Microsecond & c)
        : Priority(i), _deadline(d), _period(p), _capacity(c) {}

    public:
        Microsecond _deadline;
        Microsecond _period;
        Microsecond _capacity;
    };

    // Rate Monotonic
    class RM:public RT_Common
    {
    public:
        static const bool timed = false;
        static const bool dynamic = false;
        static const bool preemptive = true;

    public:
        RM(int p = APERIODIC): RT_Common(p) {}
        RM(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, int cpu = ANY)
        : RT_Common(p ? p : d, d, p, c) {}
    };

    // Partitioned Rate Monotonic (multicore)
    class PRM: public RM, public Variable_Queue
    {
        enum { ANY = Variable_Queue::ANY };

    public:
        static const unsigned int QUEUES = Traits<Machine>::CPUS;

    public:
        PRM(int p = APERIODIC)
        : RM(p), Variable_Queue(((_priority == IDLE) || (_priority == MAIN)) ? Machine::cpu_id() : 0) {}

        PRM(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, int cpu = ANY)
        : RM(d, p, c, cpu), Variable_Queue((cpu != ANY) ? cpu : ++_next_queue %= Machine::n_cpus()) {}

        using Variable_Queue::queue;

        static unsigned int current_queue() { return Machine::cpu_id(); }
    };


     // Deadline Monotonic
     class DM: public RT_Common
     {
     public:
         static const bool timed = false;
         static const bool dynamic = false;
         static const bool preemptive = true;

     public:
         DM(int p = APERIODIC): RT_Common(p) {}
         DM(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, int cpu = ANY)
         : RT_Common(d, d, p, c) {}
     };

      // Earliest Deadline First
      class EDF: public RT_Common
      {
      public:
          static const bool timed = false;
          static const bool dynamic = true;
          static const bool preemptive = true;

      public:
          EDF(int p = APERIODIC): RT_Common(p) {}
          EDF(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, int cpu = ANY); // Defined at Alarm

          void update(); // Defined at Alarm
      };

      // Global Earliest Deadline First (multicore)
      class GEDF: public EDF
      {
      public:
          static const unsigned int HEADS = Traits<Machine>::CPUS;

      public:
          GEDF(int p = APERIODIC): EDF(p) {}
          GEDF(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, int cpu = ANY)
          : EDF(d, p, c, cpu) {}

          static unsigned int queue() { return current_head(); }
          static unsigned int current_head() { return Machine::cpu_id(); }
      };

      // Partitioned Earliest Deadline First (multicore)
      class PEDF: public EDF, public Variable_Queue
      {
          enum { ANY = Variable_Queue::ANY };

      public:
          static const unsigned int QUEUES = Traits<Machine>::CPUS;

      public:
          PEDF(int p = APERIODIC)
          : EDF(p), Variable_Queue(((_priority == IDLE) || (_priority == MAIN)) ? Machine::cpu_id() : 0) {}

          PEDF(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, int cpu = ANY)
          : EDF(d, p, c, cpu), Variable_Queue((cpu != ANY) ? cpu : ++_next_queue %= Machine::n_cpus()) {}

          using Variable_Queue::queue;

          static unsigned int current_queue() { return Machine::cpu_id(); }
      };

      // Clustered Earliest Deadline First (multicore)
      class CEDF: public EDF, public Variable_Queue
      {
          enum { ANY = Variable_Queue::ANY };

      public:
          // QUEUES x HEADS must be equal to Traits<Machine>::CPUS
          static const unsigned int HEADS = 2;
          static const unsigned int QUEUES = Traits<Machine>::CPUS / HEADS;

      public:
          CEDF(int p = APERIODIC)
          : EDF(p), Variable_Queue(((_priority == IDLE) || (_priority == MAIN)) ? current_queue() : 0) {} // Aperiodic

          CEDF(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, int cpu = ANY)
          : EDF(d, p, c, cpu), Variable_Queue((cpu != ANY) ? cpu / HEADS : ++_next_queue %= Machine::n_cpus() / HEADS) {}

          using Variable_Queue::queue;

          static unsigned int current_queue() { return Machine::cpu_id() / HEADS; }
          static unsigned int current_head() { return Machine::cpu_id() % HEADS; }
      };
}


// Scheduling_Queue
template<typename T, typename R = typename T::Criterion>
class Scheduling_Queue: public Scheduling_List<T> {};

template<typename T>
class Scheduling_Queue<T, Scheduling_Criteria::GRR>:
public Multihead_Scheduling_List<T> {};

template<typename T>
class Scheduling_Queue<T, Scheduling_Criteria::CPU_Affinity>:
public Scheduling_Multilist<T> {};

template<typename T>
class Scheduling_Queue<T, Scheduling_Criteria::PRM>:
public Scheduling_Multilist<T> {};

template<typename T>
class Scheduling_Queue<T, Scheduling_Criteria::GEDF>:
public Multihead_Scheduling_List<T> {};

template<typename T>
class Scheduling_Queue<T, Scheduling_Criteria::PEDF>:
public Scheduling_Multilist<T> {};

template<typename T>
class Scheduling_Queue<T, Scheduling_Criteria::CEDF>:
public Multihead_Scheduling_Multilist<T> {};


// Scheduler
// Objects subject to scheduling by Scheduler must declare a type "Criterion"
// that will be used as the scheduling queue sorting criterion (viz, through
// operators <, >, and ==) and must also define a method "link" to export the
// list element pointing to the object being handled.
template<typename T>
class Scheduler: public Scheduling_Queue<T>
{
private:
    typedef Scheduling_Queue<T> Base;

public:
    typedef typename T::Criterion Criterion;
    typedef Scheduling_List<T, Criterion> Queue;
    typedef typename Queue::Element Element;

public:
    Scheduler() {}

    unsigned int schedulables() { return Base::size(); }

    T * volatile chosen() {
    	// If called before insert(), chosen will dereference a null pointer!
    	// For threads, we this won't happen (see Thread::init()).
    	// But if you are unsure about your new use of the scheduler,
    	// please, pay the price of the extra "if" bellow.
//    	return const_cast<T * volatile>((Base::chosen()) ? Base::chosen()->object() : 0);
    	return const_cast<T * volatile>(Base::chosen()->object());
    }

    void insert(T * obj) {
        db<Scheduler>(TRC) << "Scheduler[chosen=" << chosen() << "]::insert(" << obj << ")" << endl;

        Base::insert(obj->link());
    }

    T * remove(T * obj) {
        db<Scheduler>(TRC) << "Scheduler[chosen=" << chosen() << "]::remove(" << obj << ")" << endl;

        return Base::remove(obj->link()) ? obj : 0;
    }

    void suspend(T * obj) {
        db<Scheduler>(TRC) << "Scheduler[chosen=" << chosen() << "]::suspend(" << obj << ")" << endl;

        Base::remove(obj->link());
    }

    void resume(T * obj) {
        db<Scheduler>(TRC) << "Scheduler[chosen=" << chosen() << "]::resume(" << obj << ")" << endl;

        Base::insert(obj->link());
    }

    T * choose() {
        db<Scheduler>(TRC) << "Scheduler[chosen=" << chosen() << "]::choose() => ";

        T * obj = Base::choose()->object();

        db<Scheduler>(TRC) << obj << endl;

        return obj;
    }

    T * choose_another() {
        db<Scheduler>(TRC) << "Scheduler[chosen=" << chosen() << "]::choose_another() => ";

        T * obj = Base::choose_another()->object();

        db<Scheduler>(TRC) << obj << endl;

        return obj;
    }

    T * choose(T * obj) {
        db<Scheduler>(TRC) << "Scheduler[chosen=" << chosen() << "]::choose(" << obj;

        if(!Base::choose(obj->link()))
            obj = 0;

        db<Scheduler>(TRC) << obj << endl;

        return obj;
    }
};

__END_SYS

#endif
