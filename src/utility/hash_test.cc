
// EPOS Hash Utility Test Program

#include <utility/ostream.h>
#include <utility/malloc.h>
#include <utility/hash.h>

using namespace EPOS;

const int N = 10;

void test_few_synonyms_hash();
void test_many_synonyms_hash();

OStream cout;

int main()
{
    cout << "Hash Utility Test" << endl;

    test_few_synonyms_hash();
    test_many_synonyms_hash();

    cout << "\nDone!" << endl;

    return 0;
}

void test_few_synonyms_hash()
{
    cout << "\nThis is a hash table of integeres with few synonyms:" << endl;

    Simple_Hash<int, N> h;
    int o[N * 2];
    Simple_Hash<int, N>::Element * e[N * 2];

    cout << "Inserting the following integers into the hash table ";
    for(int i = 0; i < N * 2; i++) {
        o[i] = i;
        e[i] = new Simple_Hash<int, N>::Element(&o[i], i);
        h.insert(e[i]);
        cout << i;
        if(i != N * 2 - 1)
            cout << ", ";
    }
    cout << "" << endl;

    cout << "The hash table has now " << h.size() << " elements:" << endl;
    for(int i = 0; i < N * 2; i++) {
        cout << "[" << i << "]={o=" << *h.search_key(i)->object()
             << ",k=" << h.search_key(i)->key() << "}";
        if(i != N * 2 - 1)
            cout << ", ";
    }
    cout << "" << endl;

    cout << "Removing the element whose value is " << o[N/2] << " => "
         << *h.remove(&o[N/2])->object() << "" << endl;
    cout << "Removing the element whose key is " << 1 << " => "
         << *h.remove_key(1)->object() << "" << endl;
    cout << "Removing the element whose key is " << 11 << " => "
         << *h.remove_key(11)->object() << "" << endl;
    cout << "Removing the element whose value is " << o[N/4] << " => "
         << *h.remove(&o[N/4])->object() << "" << endl;
    cout << "Removing the element whose key is " << N-1 << " => "
         << *h.remove_key(N-1)->object() << "" << endl;
    cout << "Trying to remove an element that is not on the hash => "
         << h.remove(&o[N/2]) << "" << endl;

    cout << "The hash table has now " << h.size() << " elements:" << endl;
    for(int i = 0; i < N * 2; i++) {
        cout << "[" << i << "]={o=" << *h.search_key(i)->object()
             << ",k=" << h.search_key(i)->key() << "}";
        if(i != N * 2 - 1)
            cout << ", ";
    }
    cout << "" << endl;

    cout << "Removing all remaining elements => ";
    for(int i = 0; i < N * 2; i++) {
        cout << *h.remove_key(i)->object();
        if(i != N * 2 - 1)
            cout << ", ";
    }
    cout << "" << endl;

    for(int i = 0; i < N * 2; i++)
        delete e[i];
}

void test_many_synonyms_hash()
{
    cout << "\nThis is a hash table of integeres with many synonyms:" << endl;

    Hash<int, N> h;
    int o[N * N];
    Hash<int, N>::Element * e[N * N];

    cout << "Inserting the following integers into the hash table ";
    for(int i = 0; i < N * N; i++) {
        o[i] = i;
        e[i] = new Hash<int, N>::Element(&o[i], i);
        h.insert(e[i]);
        cout << i;
        if(i != N * N - 1)
            cout << ", ";
    }

    cout << "The hash table has now " << h.size() << " elements:" << endl;
    for(int i = 0; i < N * N; i++) {
        cout << "[" << i << "]={o=" << *h.search_key(i)->object()
             << ",k=" << h.search_key(i)->key() << "}";
        if(i != N * N - 1)
            cout << ", ";
    }
    cout << "" << endl;

    cout << "Removing the element whose value is " << o[N/2] << " => "
         << *h.remove(&o[N/2])->object() << "" << endl;
    cout << "Removing the element whose key is " << 1 << " => "
         << *h.remove_key(1)->object() << "" << endl;
    cout << "Removing the element whose key is " << 11 << " => "
         << *h.remove_key(11)->object() << "" << endl;
    cout << "Removing the element whose value is " << o[N/4] << " => "
         << *h.remove(&o[N/4])->object() << "" << endl;
    cout << "Removing the element whose key is " << N-1 << " => "
         << *h.remove_key(N-1)->object() << "" << endl;
    cout << "Trying to remove an element that is not on the hash => "
         << h.remove(&o[N/2]) << "" << endl;

    cout << "The hash table has now " << h.size() << " elements:" << endl;
    for(int i = 0; i < N * N; i++) {
        cout << "[" << i << "]={o=" << *h.search_key(i)->object()
             << ",k=" << h.search_key(i)->key() << "}";
        if(i != N * N - 1)
            cout << ", ";
    }
    cout << "" << endl;

    cout << "Removing all remaining elements => ";
    for(int i = 0; i < N * N; i++) {
        cout << *h.remove_key(i)->object();
        if(i != N * N - 1)
            cout << ", ";
    }
    cout << "" << endl;

    for(int i = 0; i < N * N; i++)
        delete e[i];
}
