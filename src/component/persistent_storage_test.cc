#include <utility/ostream.h>
#include <alarm.h>
#include <persistent_storage.h>

using namespace EPOS;

int main()
{
    OStream cout;
    cout << "EPOS Persistent Storage test" << endl;

#ifdef __PERSISTENT_STORAGE_H
    const unsigned char bytes_to_write = Persistent_Storage::SIZE / 2121;
    const Persistent_Storage::Address start_address = 0;

    typedef Persistent_Storage::Address Address;

    unsigned char data[bytes_to_write];

    for(unsigned int i = 0; i < bytes_to_write; ++i)
        data[i] = i + 3;

    cout << "Delaying two seconds to avoid writing to storage too much..." << endl;
    Alarm::delay(2000000);

    cout << "Reading storage" << endl;
    cout << "If this is the first time this test is running, expect garbage below." << endl;
    cout << "Otherwise, it should print the sequence [3, 4, 5 ... " << (bytes_to_write + 2) << "]." << endl;

    unsigned char from_storage[bytes_to_write];

    Persistent_Storage::read(start_address, from_storage, bytes_to_write);

    bool test_passed = true;
    for(unsigned int i = 0; i < bytes_to_write; ++i) {
        if(from_storage[i] != data[i])
            test_passed = false;
        cout << from_storage[i] << ", ";
    }
    cout << endl;

    if(test_passed) {
        cout << "Test done! Storage content seems right!" << endl;
        cout << "I will put myself on an endless loop now, bye!" << endl;
        while(true);
    }

    cout << "Writing to persistent storage..." << endl;
    Alarm::delay(2000000);
    Persistent_Storage::write(start_address, data, bytes_to_write);

    cout << "Done!" << endl;

    cout << ">> If this line is printed more than once, the test failed!! <<" << endl;
    cout << "Rebooting and starting over." << endl;

    Machine::reboot();
#else

    cout << "This machine does not support persistent storage!" << endl;
#endif
}
