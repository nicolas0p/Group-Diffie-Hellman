// EPOS Task Test Program

#include <utility/ostream.h>
#include <alarm.h>
#include <thread.h>
#include <task.h>

using namespace EPOS;

const int iterations = 10;

int func_a(void);
int func_b(void);

Thread * a;
Thread * b;
Thread * m;

OStream cout;

int main()
{
    cout << "Task test" << endl;

    m = Thread::self();

    Task * task0 = Task::self();
    Address_Space * as0 = task0->address_space();
    cout << "My address space's page directory is located at " << as0->pd() << endl;

    Segment * cs0 = task0->code_segment();
    CPU::Log_Addr code0 = task0->code();
    cout << "My code segment is located at "
         << static_cast<void *>(code0)
         << " and it is " << cs0->size() << " bytes long" << endl;

    Segment * ds0 = task0->data_segment();
    CPU::Log_Addr data0 = task0->data();
    cout << "My data segment is located at "
         << static_cast<void *>(data0)
         << " and it is " << ds0->size() << " bytes long" << endl;

    cout << "Creating a thread:";
    a = new Thread(&func_a);
    cout << " done!" << endl;

    cout << "Creating another thread:";
    b = new Thread(&func_b);
    cout << " done!" << endl;

    cout << "I'll now suspend my self to see the other threads running:" << endl;
    m->suspend();

    cout << "Both threads are now done and have suspended themselves. I'll now wait for 1 second and then wake them up so they can exit ..." << endl;

    // Alarm::delay(1000000);

    a->resume();

    Thread::yield();
    b->resume();

    int status_a = a->join();
    int status_b = b->join();

    cout << "Thread A exited with status " << status_a << " and thread B exited with status " << status_b << "." << endl;

    delete a;
    delete b;

    cout << "I'm also done, bye!" << endl;

    return 0;
}


int func_a(void)
{
    for(int i = iterations; i > 0; i--) {
        for(int i = 0; i < 79; i++)
            cout << "a";
        cout << endl;
        Thread::yield();
    }

    Thread::self()->suspend();

    return 'A';
}

int func_b(void)
{
    for(int i = iterations; i > 0; i--) {
        for(int i = 0; i < 79; i++)
            cout << "b";
        cout << endl;
        Thread::yield();
    }

    m->resume();

    Thread::self()->suspend();

    return 'B';
}
