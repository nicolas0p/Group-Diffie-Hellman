// EPOS OStream Utility Test Program

#include <utility/ostream.h>

using namespace EPOS;

int main()
{
    OStream cout;

    cout << "OStream test" << endl;
    cout << "This is a char:\t\t\t" << 'A' << endl;
    cout << "This is a negative char:\t" << '\377' << endl;
    cout << "This is an unsigned char:\t" << 'A' << endl;
    cout << "This is an int:\t\t\t" << (1 << sizeof(int) * 8 - 2) << endl
         << "\t\t\t\t" << hex << (1 << sizeof(int) * 8 - 2) << "(hex)" << endl
         << "\t\t\t\t" << dec << (1 << sizeof(int) * 8 - 2) << "(dec)" << endl
         << "\t\t\t\t" << oct << (1 << sizeof(int) * 8 - 2) << "(oct)" << endl
         << "\t\t\t\t" << bin << (1 << sizeof(int) * 8 - 2) << "(bin) "
         << endl;
    cout << "This is a negative int:\t\t" << (1 << sizeof(int) * 8 - 1) << endl
         << "\t\t\t\t" << hex << (1 << sizeof(int) * 8 - 1) << "(hex)" << endl
         << "\t\t\t\t" << dec << (1 << sizeof(int) * 8 - 1) << "(dec)" << endl
         << "\t\t\t\t" << oct << (1 << sizeof(int) * 8 - 1) << "(oct)" << endl
         << "\t\t\t\t" << bin << (1 << sizeof(int) * 8 - 1) << "(bin) " << endl;
    cout << "This is a string:\t\t" << "string" << endl;
    cout << "This is a pointer:\t\t" << &cout << endl;

    return 0;
}
