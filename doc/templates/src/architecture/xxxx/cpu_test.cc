// EPOS XXXX Test Program

#include <utility/ostream.h>
#include <cpu.h>

using namespace EPOS;

int main()
{
    OStream cout;
    cout << "XXXX test" << endl;

    XXXX cpu;

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

    cout << "XXXX test finished" << endl;

    return 0;
}
