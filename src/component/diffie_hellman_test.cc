// EPOS Elliptic Curve Diffie-Hellman (ECDH) Component Test Program

#include <diffie_hellman.h>
#include <utility/ostream.h>
#include <utility/random.h>

using namespace EPOS;

OStream cout;

static const unsigned int ITERATIONS = 50;

int main()
{
    unsigned int seed = Random::random();

    cout << "EPOS Elliptic Curve Diffie-Hellman Test" << endl;
    cout << "Configuration: " << endl;
    cout << "Diffie_Hellman::SECRET_SIZE = " << Diffie_Hellman::SECRET_SIZE << endl;
    cout << "Diffie_Hellman::PUBLIC_KEY_SIZE = " << Diffie_Hellman::PUBLIC_KEY_SIZE << endl;
    cout << "sizeof(Diffie_Hellman) = " << sizeof(Diffie_Hellman) << endl;
    cout << "sizeof(Diffie_Hellman::Public_Key) = " << sizeof(Diffie_Hellman::Public_Key) << endl;
    cout << "sizeof(Diffie_Hellman::Private_Key) = " << sizeof(Diffie_Hellman::Private_Key) << endl;
    cout << "sizeof(Diffie_Hellman::Shared_Key) = " << sizeof(Diffie_Hellman::Shared_Key) << endl;
    cout << "Random seed = " << seed << endl;
    cout << "Iterations = " << ITERATIONS << endl;

    unsigned int tests_failed = 0;

    Random::seed(seed);

    for(unsigned int it = 0; it < ITERATIONS; it++) {
        cout << endl;
        cout << "Iteration " << it << endl;

        Diffie_Hellman alice;
        Diffie_Hellman bob;

        cout << "Alice's public key: " << alice.public_key() << endl;
        cout << "Bob's public key: " << bob.public_key() << endl;

        Diffie_Hellman::Shared_Key sk1 = alice.shared_key(bob.public_key());
        Diffie_Hellman::Shared_Key sk2 = bob.shared_key(alice.public_key());

        bool ok = sk1 == sk2;
        if(ok) {
            cout << "Shared key = " << sk1 << endl;
            cout << "OK! Alice and Bob share the same key" << endl;
        }
        else {
            cout << "Alice's shared key: " << sk1 << endl;
            cout << "Bob's shared key: " << sk2 << endl;
            cout << "ERROR! Shared keys do not match!" << endl;
        }

        tests_failed += !ok;
    }

    cout << endl;
    cout << "Tests finished with " << tests_failed << " error" << (tests_failed > 1 ? "s" : "") << " detected." << endl;
    cout << endl;

    return 0;
}
