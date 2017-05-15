// eMote3's dedicated MAC Timer Test Program

#include <utility/ostream.h>
#include <nic.h>
#include <utility/random.h>

using namespace EPOS;

const static unsigned int SECONDS_TO_COUNT = 8;
const static unsigned int THRESHOLD = 1000;
const static unsigned int N_TESTS = 100;
unsigned int global_fails = 0;

OStream cout;

void pretty_print(unsigned int x)
{
    cout << x << (x < 10000000 ? "\t\t" : "\t");
}

int main()
{
    MAC_Timer m;
    m.config();
    unsigned int t1;

    cout << "====== Test 1 ======" << endl;
    cout << "Counting " << SECONDS_TO_COUNT << " seconds. You should check if it makes sense:" << endl;

    t1 = m.read() / m.frequency();
    m.start();

    for(unsigned int i = 0; i<=SECONDS_TO_COUNT; i++)
    {
        auto t2 = (m.read() / m.frequency()) - t1;
        while(t2 < i)
            t2 = (m.read() / m.frequency()) - t1;
        cout << t2 << " ";
    } 
    cout << "Done!" << endl;

    auto global_fails_before_test_2 = global_fails;
    cout << "====== Test 2 ======" << endl;
    cout << "Starting and stopping timer " << N_TESTS << " times" << endl;    
    for(unsigned int i = 0; i<N_TESTS; i++)    
    {
        m.stop();
        t1 = m.read();
        for(volatile unsigned int x = 0; x<0xff; x++);
        auto diff = m.read() - t1;
        bool passed = (diff == 0);
        if(!passed)
        {
            global_fails++;
            cout << "ERROR: timer failed to stop!" << endl;
        }
        m.start();
        t1 = m.read();
        for(volatile unsigned int x = 0; x<0xff; x++);
        diff = m.read() - t1;
        passed = (diff > 0);
        if(!passed)
        {
            global_fails++;
            cout << "ERROR: timer failed to start!" << endl;
        }
    }
    if(global_fails == global_fails_before_test_2)
        cout << "SUCCESS: All " << N_TESTS << " tests passed!" << endl;
    else
    {
        auto diff = global_fails - global_fails_before_test_2;
        cout << "ERROR: " << diff << " tests failed!" << endl;
    }

    int tests_passed = 0;
    cout << "====== Test 3 ======" << endl;
    cout << "Setting the clock to " << N_TESTS << " different random values and checking" << endl;
    cout << "Value set\tValue read\tDifference\tTest passed?" << endl;
    for(unsigned int i = 0; i<N_TESTS; i++)    
    {
        auto val = Random::random() >> 1;
        m.set(val);
        auto t = m.read();
        auto difference = t-val;
        bool passed = difference < THRESHOLD;
        tests_passed += passed;
        pretty_print(val);
        pretty_print(t);
        pretty_print(difference);
        cout << (passed ? "yes" : "NO!") << endl;;
    }

    if(tests_passed == N_TESTS)
        cout << "SUCCESS: All " << N_TESTS << " tests passed!" << endl;
    else
    {
        auto diff = N_TESTS - tests_passed;
        global_fails += diff;
        cout << "ERROR: " << diff << " tests failed!" << endl;
    }

    tests_passed = 0;
    cout << "====== Test 4 ======" << endl;
    cout << "Setting the clock to " << N_TESTS << " different random values from 0 to 256" << endl;
    for(unsigned int i = 0; i<N_TESTS; i++)    
    {
        auto val = Random::random() % 257;
        m.set(val);
        auto t = m.read();
        auto difference = t-val;
        bool passed = difference < THRESHOLD;
        tests_passed += passed;
        if(!passed)
        {
            if(tests_passed == i)
                cout << "Value set\tValue read\tDifference\tTest passed?" << endl;
            pretty_print(val);
            pretty_print(t);
            pretty_print(difference);
            cout << (passed ? "yes" : "NO!") << endl;;
        }
    }
    if(tests_passed == N_TESTS)
        cout << "SUCCESS: All " << N_TESTS << " tests passed!" << endl;
    else
    {
        auto diff = N_TESTS - tests_passed;
        global_fails += diff;
        cout << "ERROR: " << diff << " tests failed!" << endl;
    }

    tests_passed = 0;
    cout << "====== Test 5 ======" << endl;
    cout << "Setting the clock to " << N_TESTS << " different random values from 2147483648 to 4294967295" << endl;
    for(unsigned int i = 0; i<N_TESTS; i++)    
    {
        auto val = (Random::random() % 2147483648u) + 2147483648u;
        m.set(val);
        auto t = m.read();
        auto difference = t-val;
        bool passed = difference < THRESHOLD;
        tests_passed += passed;
        if(!passed)
        {
            if(tests_passed == i)
                cout << "Value set\tValue read\tDifference\tTest passed?" << endl;
            pretty_print(val);
            pretty_print(t);
            pretty_print(difference);
            cout << (passed ? "yes" : "NO!") << endl;;
        }
    }
    if(tests_passed == N_TESTS)
        cout << "SUCCESS: All " << N_TESTS << " tests passed!" << endl;
    else
    {
        auto diff = N_TESTS - tests_passed;
        global_fails += diff;
        cout << "ERROR: " << diff << " tests failed!" << endl;
    }

    {
        const unsigned int N_TESTS = 4;
        unsigned int vals[N_TESTS] = {1u,0u,4294967294u,4294967295u};
        int tests_passed = 0;
        cout << "====== Test 6 ======" << endl;
        cout << "Setting the clock to boundary values" << endl;
        cout << "Value set\tValue read\tDifference\tTest passed?" << endl;
        for(unsigned int i = 0; i<N_TESTS; i++)    
        {
            auto val = vals[i];
            m.set(val);
            auto t = m.read();
            auto difference = t-val;
            bool passed = difference < THRESHOLD;
            tests_passed += passed;
            pretty_print(val);
            pretty_print(t);
            pretty_print(difference);
            cout << (passed ? "yes" : "NO!") << endl;;
        }
        if(tests_passed == N_TESTS)
            cout << "SUCCESS: All " << N_TESTS << " tests passed!" << endl;
        else
        {
            auto diff = N_TESTS - tests_passed;
            global_fails += diff;
            cout << "ERROR: " << diff << " tests failed!" << endl;
        }
    }

    cout << endl << "All tests finished" << endl;
    if(global_fails > 0)
        cout << "ERROR! " << global_fails << " tests failed. See the messages above." << endl;
    else
        cout << "SUCCESS! No failures detected in any test." << endl;;

    return 0;
}
