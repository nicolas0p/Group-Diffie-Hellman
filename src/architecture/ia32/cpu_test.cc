// EPOS IA32 Test Program

#include <utility/ostream.h>
#include <cpu.h>

using namespace EPOS;

int main()
{
    OStream cout;
    cout << "IA32 test" << endl;

    CPU cpu;

    {
        volatile bool lock = false;
        if(cpu.tsl(lock))
            cout << "tsl(): doesn't function properly!" << endl;
        else
            if(cpu.tsl(lock))
        	cout << "tsl(): ok" << endl;
            else
        	cout << "tsl(): doesn't function properly!" << endl;
    }
    {
        volatile int number = 100;
        volatile int tmp;
        if((tmp = cpu.finc(number)) != 100)
            cout << "finc(): doesn't function properly (n=" << tmp << ", should be 100)!" << endl;
        else
            if((tmp = cpu.finc(number)) != 101)
        	cout << "finc(): doesn't function properly (n=" << tmp << ", should be 101)!" << endl;
            else
        	cout << "finc(): ok" << endl;
    }
    {
        volatile int number = 100;
        volatile int tmp;
        if((tmp = cpu.fdec(number)) != 100)
            cout << "fdec(): doesn't function properly (n=" << tmp << ", should be 100)!" << endl;
        else
            if((tmp = cpu.fdec(number)) != 99)
        	cout << "fdec(): doesn't function properly (n=" << tmp << ", should be 99)!" << endl;
            else
        	cout << "fdec(): ok" << endl;
    }
    {
        volatile int number = 100;
        volatile int compare = number;
        volatile int replacement = number - 1;
        volatile int tmp;
        if((tmp = cpu.cas(number, compare, replacement)) != compare)
            cout << "cas(): doesn't function properly [1] (n=" << tmp << ", should be " << compare << ")!" << endl;
        else
            if((tmp = cpu.cas(number, compare, replacement)) != replacement)
                cout << "cas(): doesn't function properly [2] (n=" << tmp << ", should be " << replacement << ")!" << endl;
            else
                cout << "cas(): ok" << endl;
    }
 
    cout << "IA32 test finished" << endl;

    return 0;
}
