// EPOS (Litte-endian) Big Numbers Utility Test Program
// The output of this script is meant to be verified by tools/epossectst/eposbignumtst.py

#include <utility/string.h>
#include <utility/bignum.h>
#include <utility/random.h>
#include <utility/aes.h>
#include <utility/diffie_hellman.h>

using namespace EPOS;

const unsigned int ITERATIONS = 125000;
const unsigned int SIZE = 16;

OStream cout;

int main()
{
    cout << "Bignum Utility Test" << endl;

    cout << "sizeof(Bignum<" << SIZE << ">) = " << sizeof(Bignum<SIZE>) << " bytes." << endl;
    cout << "sizeof(Bignum<" << SIZE << ">::Digit) = " << sizeof(Bignum<SIZE>::Digit) << " bytes." << endl; // This output is parsed by tools/epossectst/eposbignumtst.py

    Bignum<SIZE> a = 0, b = 1;
    cout << "a = " << a << ", b = " << b << endl;
    cout << "a + b = " << a + b << endl;
    cout << "a * b = " << a * b << endl;

    a = 100000000;
    b = 200000000;
    cout << "a = " << a << ", b = " << b << endl;
    cout << "a + b = " << a + b << endl;

    a = Bignum<SIZE>("10000000000000000000000000000000", 16);
    b = Bignum<SIZE>("20000000000000000000000000000000", 16);
    cout << "a = " << a << ", b = " << b << endl;
    a += b;
    cout << "a + b = " << a << endl;

    unsigned int seed = Random::random();
    Random::seed(seed);
    cout << "Random seed = " << seed << endl;

    a = 0;
    a -= 1;
    cout << "Modulo = " << a << " + 1" << endl; // This output is parsed by tools/epossectst/eposbignumtst.py

    for(unsigned int i = 0; i < ITERATIONS; i++) {
        a.randomize();
        b.randomize();
        cout << "a = " << a << endl; // This output is parsed by tools/epossectst/eposbignumtst.py
        cout << "b = " << b << endl; // This output is parsed by tools/epossectst/eposbignumtst.py
        a += b;
        cout << "a + b = " << a << endl; // This output is parsed by tools/epossectst/eposbignumtst.py

        a.randomize();
        b.randomize();
        cout << "a = " << a << endl; // This output is parsed by tools/epossectst/eposbignumtst.py
        cout << "b = " << b << endl; // This output is parsed by tools/epossectst/eposbignumtst.py
        a -= b;
        cout << "a - b = " << a << endl; // This output is parsed by tools/epossectst/eposbignumtst.py

        a.randomize();
        b.randomize();
        cout << "a = " << a << endl; // This output is parsed by tools/epossectst/eposbignumtst.py
        cout << "b = " << b << endl; // This output is parsed by tools/epossectst/eposbignumtst.py
        a *= b;
        cout << "a * b = " << a << endl; // This output is parsed by tools/epossectst/eposbignumtst.py

        a.randomize();
        b.randomize();
        cout << "a = " << a << endl; // This output is parsed by tools/epossectst/eposbignumtst.py
        cout << "b = " << b << endl; // This output is parsed by tools/epossectst/eposbignumtst.py
        b.invert();
        a *= b;
        cout << "a / b = " << a << endl; // This output is parsed by tools/epossectst/eposbignumtst.py
    }

    cout << "Done!" << endl; // This output is parsed by tools/epossectst/eposbignumtst.py

    return 0;
}
