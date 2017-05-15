// EPOS Memory Allocation Utility Test Program

#include <utility/ostream.h>
#include <utility/string.h>
#include <utility/malloc.h>

using namespace EPOS;

int main()
{
    OStream cout;

    cout << "Memory allocation test" << endl;
    char * cp = new char('A');
    cout << "new char('A')\t\t=> {p=" << (void *)cp << ",v=" << *cp << "}" << endl;
    int * ip = new int(1);
    cout << "new int(1)\t\t=> {p=" << (void *)ip << ",v=" << *ip << "}" << endl;
    long int * lp = new long int(1);
    cout << "new long int(1)\t\t=> {p=" << (void *)lp << ",v=" << *lp << "}" << endl;
    char * sp = new char[1024];
    strcpy(sp, "string");
    cout << "new char[1024]\t\t=> {p=" << (void *)sp << ",v=" << sp << "}" << endl;

    cout << "deleting everything!" << endl;
    delete cp;
    delete ip;
    delete lp;
    delete sp;

    cout << "and doing it all again!" << endl;
    cp = new char('A');
    cout << "new char('A')\t\t=> {p=" << (void *)cp << ",v=" << *cp << "}" << endl;
    ip = new int(1);
    cout << "new int(1)\t\t=> {p=" << (void *)ip << ",v=" << *ip << "}" << endl;
    lp = new long int(1);
    cout << "new long int(1)\t\t=> {p=" << (void *)lp << ",v=" << *lp << "}" << endl;
    sp = new char[1024];
    strcpy(sp, "string");
    cout << "new char[1024]\t\t=> {p=" << (void *)sp << ",v=" << sp << "}" << endl;


    return 0;
}
