// EPOS Cipher Mediator Test Program

#include <cipher.h>
#include <utility/ostream.h>
#include <utility/random.h>

using namespace EPOS;

OStream cout;

static const unsigned int ITERATIONS = 100;

int main()
{
    unsigned int seed = Random::random();

    cout << "EPOS Cipher Test" << endl;
    cout << "Configuration: " << endl;
    cout << "Traits<Software_AES<0>>::enabled = " << Traits<Software_AES<0>>::enabled << endl;
    cout << "Cipher::KEY_SIZE = " << Cipher::KEY_SIZE << endl;
    cout << "Random seed = " << seed << endl;
    cout << "Iterations = " << ITERATIONS << endl;

    unsigned int tests_failed = 0;

    Random::seed(seed);

    Cipher cipher;

    if(cipher.mode() == Cipher::Mode::ECB && Cipher::KEY_SIZE == 16) {
        cout << endl;
        cout << "Testing AES-128-ECB with known vectors...";

        // Test vectors from National Institute of Standards and Technology Special Publication 800-38A 2001 ED
        const unsigned char clear_text[4][16] = {{'\x6b','\xc1','\xbe','\xe2','\x2e','\x40','\x9f','\x96','\xe9','\x3d','\x7e','\x11','\x73','\x93','\x17','\x2a'},
                                        {'\xae','\x2d','\x8a','\x57','\x1e','\x03','\xac','\x9c','\x9e','\xb7','\x6f','\xac','\x45','\xaf','\x8e','\x51'},
                                        {'\x30','\xc8','\x1c','\x46','\xa3','\x5c','\xe4','\x11','\xe5','\xfb','\xc1','\x19','\x1a','\x0a','\x52','\xef'},
                                        {'\xf6','\x9f','\x24','\x45','\xdf','\x4f','\x9b','\x17','\xad','\x2b','\x41','\x7b','\xe6','\x6c','\x37','\x10'}};

        const unsigned char key[] = {'\x2b','\x7e','\x15','\x16','\x28','\xae','\xd2','\xa6','\xab','\xf7','\x15','\x88','\x09','\xcf','\x4f','\x3c'};

        const unsigned char expected[4][16] = {{'\x3a','\xd7','\x7b','\xb4','\x0d','\x7a','\x36','\x60','\xa8','\x9e','\xca','\xf3','\x24','\x66','\xef','\x97'},
                                      {'\xf5','\xd3','\xd5','\x85','\x03','\xb9','\x69','\x9d','\xe7','\x85','\x89','\x5a','\x96','\xfd','\xba','\xaf'},
                                      {'\x43','\xb1','\xcd','\x7f','\x59','\x8e','\xce','\x23','\x88','\x1b','\x00','\xe3','\xed','\x03','\x06','\x88'},
                                      {'\x7b','\x0c','\x78','\x5e','\x27','\xe8','\xad','\x3f','\x82','\x23','\x20','\x71','\x04','\x72','\x5d','\xd4'}};

        unsigned char result[4][16];
        for(unsigned int i = 0; i < 4; i ++)
            cipher.encrypt(clear_text[i], key, result[i]);

        bool ok = true;
        for(unsigned int i = 0; i < 4; i++)
            for(unsigned int j = 0; j < 16; j++)
                if(result[i][j] != expected[i][j]) {
                    ok = false;
                    break;
                }
        tests_failed += !ok;
        if(ok)
            cout << "OK!" << endl;
        else {
            cout << "ERROR!" << endl;
            cout << "Either the AES implementation is wrong or this is not an AES cipher!" << endl;
        }
    }

    for(unsigned int it = 0; it < ITERATIONS; it++) {
        unsigned char clear_text[Cipher::KEY_SIZE];
        unsigned char cipher_text[Cipher::KEY_SIZE];
        unsigned char decrypted_text[Cipher::KEY_SIZE];
        unsigned char key[Cipher::KEY_SIZE];
        for(unsigned int i = 0; i < Cipher::KEY_SIZE; i++) {
            clear_text[i] = Random::random();
            key[i] = Random::random();
        }

        cout << endl;
        cout << "Iteration " << it << endl;

        cout << "clear_text =";
        for(unsigned int i = 0; i < Cipher::KEY_SIZE; i++)
            cout << " " << hex << static_cast<unsigned int>(clear_text[i]);
        cout << endl;

        cout << "key =";
        for(unsigned int i = 0; i < Cipher::KEY_SIZE; i++)
            cout << " " << hex << static_cast<unsigned int>(key[i]);
        cout << endl;

        cout << "Testing encryption...";
        cipher.encrypt(clear_text, key, cipher_text);

        bool ok = false;
        for(unsigned int i = 0; i < Cipher::KEY_SIZE; i++)
            if(decrypted_text[i] != clear_text[i]) {
                ok = true;
                break;
            }
        if(ok)
            cout << "OK!" << endl;
        else
            cout << "ERROR!" << endl;
        tests_failed += !ok;
        cout << "cipher_text =";
        for(unsigned int i = 0; i < Cipher::KEY_SIZE; i++)
            cout << " " << hex << static_cast<unsigned int>(cipher_text[i]);
        cout << endl;



        cout << "Testing decryption...";

        cipher.decrypt(cipher_text, key, decrypted_text);
        ok = true;
        for(unsigned int i = 0; i < Cipher::KEY_SIZE; i++)
            if(decrypted_text[i] != clear_text[i]) {
                ok = false;
                break;
            }
        if(ok)
            cout << "OK!" << endl;
        else
            cout << "ERROR!" << endl;
        tests_failed += !ok;

        cout << "decrypted_text =";
        for(unsigned int i = 0; i < Cipher::KEY_SIZE; i++)
            cout << " " << hex << static_cast<unsigned int>(decrypted_text[i]);
        cout << endl;



        cout << "Testing decryption with wrong key...";

        key[Random::random() % Cipher::KEY_SIZE]++;

        cipher.decrypt(cipher_text, key, decrypted_text);

        ok = false;
        for(unsigned int i = 0; i < Cipher::KEY_SIZE; i++)
            if(decrypted_text[i] != clear_text[i]) {
                ok = true;
                break;
            }
        if(ok)
            cout << "OK!" << endl;
        else
            cout << "ERROR!" << endl;
        tests_failed += !ok;

        cout << "key =";
        for(unsigned int i = 0; i < Cipher::KEY_SIZE; i++)
            cout << " " << hex << static_cast<unsigned int>(key[i]);
        cout << endl;
        cout << "decrypted_text =";
        for(unsigned int i = 0; i < Cipher::KEY_SIZE; i++)
            cout << " " << hex << static_cast<unsigned int>(decrypted_text[i]);
        cout << endl;
    }

    cout << endl;
    cout << "Tests finished with " << tests_failed << " error" << (tests_failed > 1 ? "s" : "") << " detected." << endl;
    cout << endl;

    return 0;
}
