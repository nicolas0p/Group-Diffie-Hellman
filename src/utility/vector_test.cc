// EPOS Vector Utility Test Program

#include <utility/ostream.h>
#include <utility/malloc.h>
#include <utility/vector.h>

using namespace EPOS;

const int N = 10;

OStream cout;

int main()
{

    cout << "Vector Utility Test" << endl;

    cout << "\nThis is a vector of integers:" << endl;
    Vector<int, N> v;
    int o[N];
    Vector<int, N>::Element * e[N];
    cout << "Inserting the following integers into the vector ";
    for(int i = 0; i < N; i++) {
        o[i] = i;
        e[i] = new Vector<int, N>::Element(&o[i]);
        v.insert(e[i], i);
        cout << "[" << i << "]=" << i;
        if(i != N - 1)
            cout << ", ";
    }
    cout << "" << endl;

    cout << "The vector has now " << v.size() << " elements:" << endl;
    for(int i = 0; i < N; i++) {
        cout << "[" << i << "]=" << *v[i]->object();
        if(i != N - 1)
            cout << ", ";
    }
    cout << "" << endl;

    for(int i = 0; i < N; i++)
        (*v[i]->object())++;
    cout << "The vector's elements were incremented and are now:" << endl;
    for(int i = 0; i < N; i++) {
        cout << "[" << i << "]=" << *v[i]->object();
        if(i != N - 1)
            cout << ", ";
    }
    cout << "" << endl;

    cout << "Removing the element whose value is " << o[N/2] << " => "
         << *v.remove(&o[N/2])->object() << "" << endl;
    cout << "Removing the second element => " << *v.remove(1)->object()
         << "" << endl;
    cout << "Removing the element whose value is " << o[N/4] << " => "
         << *v.remove(&o[N/4])->object() << "" << endl;
    cout << "Removing the last element => " << *v.remove(N - 1)->object()
         << "" << endl;
    cout << "Trying to remove an element that is not on the vector => "
         << v.remove(&o[N/2]) << "" << endl;
    cout << "Removing all remaining elements => ";
    for(int i = 0; i < N; i++) {
        cout << *v.remove(i)->object();
        if(i != N - 1)
            cout << ", ";
    }
    cout << "" << endl;
    cout << "The vector has now " << v.size() << " elements" << endl;
    for(int i = 0; i < N; i++)
        delete e[i];

    cout << "\nDone!" << endl;

    return 0;
}
