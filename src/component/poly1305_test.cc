// EPOS Poly1305-AES Message Authentication Code Component Test Program

#include <poly1305.h>
#include <utility/ostream.h>
#include <utility/random.h>

using namespace EPOS;

OStream cout;

static const unsigned int ITERATIONS = 100;
static const unsigned int MESSAGE_SIZE_MAX = 200;

unsigned int test_known_vectors();

int main()
{
    unsigned int seed = Random::random();

    cout << "EPOS Poly1305 Test" << endl;
    cout << "Configuration: " << endl;
    cout << "Traits<Software_AES<0>>::enabled = " << Traits<Software_AES<0>>::enabled << endl;
    cout << "Cipher::KEY_SIZE = " << Cipher::KEY_SIZE << endl;
    cout << "Random seed = " << seed << endl;
    cout << "Iterations = " << ITERATIONS << endl;
    cout << "Maximum message size = " << MESSAGE_SIZE_MAX << endl;

    unsigned int tests_failed = 0;

    Random::seed(seed);

    cout << endl;
    tests_failed += test_known_vectors();

    if(tests_failed > 0) {
        cout << "Poly1305 FAILED to produce " << tests_failed << " known vectors. Aborting." << endl;
        cout << endl;
        cout << "Tests finished with " << tests_failed << " error" << (tests_failed > 1 ? "s" : "") << " detected." << endl;
        cout << endl;
    }

    for(unsigned int it = 0; it < ITERATIONS; it++) {
        unsigned char nonce[16];
        unsigned char k[16];
        unsigned char r[16];
        unsigned char mac[16];
        unsigned char msg[MESSAGE_SIZE_MAX];
        unsigned int msg_len = Random::random() % MESSAGE_SIZE_MAX;
        for(unsigned int i = 0; i < 16; i++) {
            k[i] = Random::random();
            r[i] = Random::random();
            nonce[i] = Random::random();
        }
        for(unsigned int i = 0; i < msg_len; i++) {
            msg[i] = Random::random();
        }

        cout << endl;
        cout << "Iteration " << it << endl;

        cout << "nonce =";
        for(unsigned int i = 0; i < 16; i++)
            cout << " " << hex << static_cast<unsigned int>(nonce[i]);
        cout << endl;

        cout << "k =";
        for(unsigned int i = 0; i < 16; i++)
            cout << " " << hex << static_cast<unsigned int>(k[i]);
        cout << endl;

        cout << "r =";
        for(unsigned int i = 0; i < 16; i++)
            cout << " " << hex << static_cast<unsigned int>(r[i]);
        cout << endl;

        cout << "Message size = " << msg_len << endl;

        cout << "Message = ";
        for(unsigned int i = 0; i < msg_len; i++)
            cout << " " << hex << static_cast<unsigned int>(msg[i]);
        cout << endl;

        cout << "Creating Poly1305 object...";
        Poly1305 poly(k, r);
        cout << "done" << endl;

        cout << "Stamping message...";
        poly.stamp(mac, nonce, msg, msg_len);
        cout << "done" << endl;
        cout << "mac =";
        for(unsigned int i = 0; i < 16; i++)
            cout << " " << hex << static_cast<unsigned int>(mac[i]);
        cout << endl;


        cout << "Verifying MAC...";
        bool ok = poly.verify(mac, nonce, msg, msg_len);
        tests_failed += !ok;
        if(ok)
            cout << "OK!" << endl;
        else
            cout << "ERROR!" << endl;


        cout << "Verifying with wrong MAC...";
        unsigned char wrong_mac[16];
        for(unsigned int i = 0; i < 16; i++)
            wrong_mac[i] = mac[i];
        wrong_mac[Random::random() % 16u]++;
        ok = !poly.verify(wrong_mac, nonce, msg, msg_len);
        tests_failed += !ok;
        if(ok)
            cout << "OK!" << endl;
        else
            cout << "ERROR!" << endl;


        cout << "Verifying with wrong nonce...";
        unsigned char wrong_nonce[16];
        for(unsigned int i = 0; i < 16; i++)
            wrong_nonce[i] = nonce[i];
        wrong_nonce[Random::random() % 16u]++;
        ok = !poly.verify(mac, wrong_nonce, msg, msg_len);
        tests_failed += !ok;
        if(ok)
            cout << "OK!" << endl;
        else
            cout << "ERROR!" << endl;


        cout << "Verifying with wrong Message...";
        if(msg_len == 0) 
            cout << "SKIPPED! (Message length == 0)";
        else {
            msg[Random::random() % msg_len]++;
            ok = !poly.verify(mac, nonce, msg, msg_len);
            tests_failed += !ok;
            if(ok)
                cout << "OK!" << endl;
            else
                cout << "ERROR!" << endl;
        }
    }

    cout << endl;
    cout << "Tests finished with " << tests_failed << " error" << (tests_failed > 1 ? "s" : "") << " detected." << endl;
    cout << endl;

    return 0;
}

unsigned int test_known_vectors()
{
    unsigned int fails = 0;

	unsigned char out[16];
	unsigned char expected[16];
	unsigned char n[16];
	unsigned char s[16];
	unsigned char k[16];
	unsigned char r[16];
	unsigned char m[64];
	unsigned int sz;
	unsigned int i;


	// Test vector 1
    cout << "Testing known vector 1...";
	i=0;
	m[i++] = 0xf3; m[i++] = 0xf6; 
	sz=i;
	i=0;
	k[i++]=0xec; k[i++]=0x07; k[i++]=0x4c; k[i++]=0x83;
	k[i++]=0x55; k[i++]=0x80; k[i++]=0x74; k[i++]=0x17;
	k[i++]=0x01; k[i++]=0x42; k[i++]=0x5b; k[i++]=0x62;
	k[i++]=0x32; k[i++]=0x35; k[i++]=0xad; k[i++]=0xd6;
	i=0;
	r[i++]=0x85; r[i++]=0x1f; r[i++]=0xc4; r[i++]=0x0c;
	r[i++]=0x34; r[i++]=0x67; r[i++]=0xac; r[i++]=0x0b;
	r[i++]=0xe0; r[i++]=0x5c; r[i++]=0xc2; r[i++]=0x04;
	r[i++]=0x04; r[i++]=0xf3; r[i++]=0xf7; r[i++]=0x00;
	i=0;
	n[i++]=0xfb; n[i++]=0x44; n[i++]=0x73; n[i++]=0x50;
	n[i++]=0xc4; n[i++]=0xe8; n[i++]=0x68; n[i++]=0xc5;
	n[i++]=0x2a; n[i++]=0xc3; n[i++]=0x27; n[i++]=0x5c;
	n[i++]=0xf9; n[i++]=0xd4; n[i++]=0x32; n[i++]=0x7e;
	i=0;
	s[i++]=0x58; s[i++]=0x0b; s[i++]=0x3b; s[i++]=0x0f;
	s[i++]=0x94; s[i++]=0x47; s[i++]=0xbb; s[i++]=0x1e;
	s[i++]=0x69; s[i++]=0xd0; s[i++]=0x95; s[i++]=0xb5;
	s[i++]=0x92; s[i++]=0x8b; s[i++]=0x6d; s[i++]=0xbc;
	i=0;
	expected[i++]=0xf4; expected[i++]=0xc6; expected[i++]=0x33; expected[i++]=0xc3;
	expected[i++]=0x04; expected[i++]=0x4f; expected[i++]=0xc1; expected[i++]=0x45;
	expected[i++]=0xf8; expected[i++]=0x4f; expected[i++]=0x33; expected[i++]=0x5c;
	expected[i++]=0xb8; expected[i++]=0x19; expected[i++]=0x53; expected[i++]=0xde;

    Poly1305 poly(k, r);
    poly.stamp(out, n, m, sz);

    bool ok = true;
	for(i=0;i<16;i++)
		if(out[i] != expected[i]) {
            ok = false;
            break;
        }

    ok &= poly.verify(out, n, m, sz);
    fails += !ok;
    cout << (ok ? "OK!" : "ERROR!") << endl;


	// Test vector 2
    cout << "Testing known vector 2...";
	i=0;
	sz=i;
	i=0;
	k[i++]=0x75; k[i++]=0xde; k[i++]=0xaa; k[i++]=0x25;
	k[i++]=0xc0; k[i++]=0x9f; k[i++]=0x20; k[i++]=0x8e;
	k[i++]=0x1d; k[i++]=0xc4; k[i++]=0xce; k[i++]=0x6b;
	k[i++]=0x5c; k[i++]=0xad; k[i++]=0x3f; k[i++]=0xbf;
	i=0;
	r[i++]=0xa0; r[i++]=0xf3; r[i++]=0x08; r[i++]=0x00;
	r[i++]=0x00; r[i++]=0xf4; r[i++]=0x64; r[i++]=0x00;
	r[i++]=0xd0; r[i++]=0xc7; r[i++]=0xe9; r[i++]=0x07;
	r[i++]=0x6c; r[i++]=0x83; r[i++]=0x44; r[i++]=0x03;
	i=0;
	n[i++]=0x61; n[i++]=0xee; n[i++]=0x09; n[i++]=0x21;
	n[i++]=0x8d; n[i++]=0x29; n[i++]=0xb0; n[i++]=0xaa;
	n[i++]=0xed; n[i++]=0x7e; n[i++]=0x15; n[i++]=0x4a;
	n[i++]=0x2c; n[i++]=0x55; n[i++]=0x09; n[i++]=0xcc;
	i=0;
	s[i++]=0xdd; s[i++]=0x3f; s[i++]=0xab; s[i++]=0x22;
	s[i++]=0x51; s[i++]=0xf1; s[i++]=0x1a; s[i++]=0xc7;
	s[i++]=0x59; s[i++]=0xf0; s[i++]=0x88; s[i++]=0x71;
	s[i++]=0x29; s[i++]=0xcc; s[i++]=0x2e; s[i++]=0xe7;
	i=0;
	expected[i++]=0xdd; expected[i++]=0x3f; expected[i++]=0xab; expected[i++]=0x22;
	expected[i++]=0x51; expected[i++]=0xf1; expected[i++]=0x1a; expected[i++]=0xc7;
	expected[i++]=0x59; expected[i++]=0xf0; expected[i++]=0x88; expected[i++]=0x71;
	expected[i++]=0x29; expected[i++]=0xcc; expected[i++]=0x2e; expected[i++]=0xe7;

    poly.k(k);
    poly.r(r);
    poly.stamp(out, n, m, sz);

    ok = true;
	for(i=0;i<16;i++)
		if(out[i] != expected[i]) {
            ok = false;
            break;
        }

    ok &= poly.verify(out, n, m, sz);
    fails += !ok;
    cout << (ok ? "OK!" : "ERROR!") << endl;


	// Test vector 3
    cout << "Testing known vector 3...";
	i=0;
	m[i++]=0x66; m[i++]=0x3c; m[i++]=0xea; m[i++]=0x19;
	m[i++]=0x0f; m[i++]=0xfb; m[i++]=0x83; m[i++]=0xd8;
	m[i++]=0x95; m[i++]=0x93; m[i++]=0xf3; m[i++]=0xf4;
	m[i++]=0x76; m[i++]=0xb6; m[i++]=0xbc; m[i++]=0x24;
	m[i++]=0xd7; m[i++]=0xe6; m[i++]=0x79; m[i++]=0x10;
	m[i++]=0x7e; m[i++]=0xa2; m[i++]=0x6a; m[i++]=0xdb;
	m[i++]=0x8c; m[i++]=0xaf; m[i++]=0x66; m[i++]=0x52;
	m[i++]=0xd0; m[i++]=0x65; m[i++]=0x61; m[i++]=0x36;
	sz=i;
	i=0;
	k[i++]=0x6a; k[i++]=0xcb; k[i++]=0x5f; k[i++]=0x61;
	k[i++]=0xa7; k[i++]=0x17; k[i++]=0x6d; k[i++]=0xd3;
	k[i++]=0x20; k[i++]=0xc5; k[i++]=0xc1; k[i++]=0xeb;
	k[i++]=0x2e; k[i++]=0xdc; k[i++]=0xdc; k[i++]=0x74;
	i=0;
	r[i++]=0x48; r[i++]=0x44; r[i++]=0x3d; r[i++]=0x0b;
	r[i++]=0xb0; r[i++]=0xd2; r[i++]=0x11; r[i++]=0x09;
	r[i++]=0xc8; r[i++]=0x9a; r[i++]=0x10; r[i++]=0x0b;
	r[i++]=0x5c; r[i++]=0xe2; r[i++]=0xc2; r[i++]=0x08;
	i=0;
	n[i++]=0xae; n[i++]=0x21; n[i++]=0x2a; n[i++]=0x55;
	n[i++]=0x39; n[i++]=0x97; n[i++]=0x29; n[i++]=0x59;
	n[i++]=0x5d; n[i++]=0xea; n[i++]=0x45; n[i++]=0x8b;
	n[i++]=0xc6; n[i++]=0x21; n[i++]=0xff; n[i++]=0x0e;
	i=0;
	s[i++]=0x83; s[i++]=0x14; s[i++]=0x9c; s[i++]=0x69;
	s[i++]=0xb5; s[i++]=0x61; s[i++]=0xdd; s[i++]=0x88;
	s[i++]=0x29; s[i++]=0x8a; s[i++]=0x17; s[i++]=0x98;
	s[i++]=0xb1; s[i++]=0x07; s[i++]=0x16; s[i++]=0xef;
	i=0;
	expected[i++]=0x0e; expected[i++]=0xe1; expected[i++]=0xc1; expected[i++]=0x6b;
	expected[i++]=0xb7; expected[i++]=0x3f; expected[i++]=0x0f; expected[i++]=0x4f;
	expected[i++]=0xd1; expected[i++]=0x98; expected[i++]=0x81; expected[i++]=0x75;
	expected[i++]=0x3c; expected[i++]=0x01; expected[i++]=0xcd; expected[i++]=0xbe;

    poly.k(k);
    poly.r(r);
    poly.stamp(out, n, m, sz);

    ok = true;
	for(i=0;i<16;i++)
		if(out[i] != expected[i]) {
            ok = false;
            break;
        }

    ok &= poly.verify(out, n, m, sz);
    fails += !ok;
    cout << (ok ? "OK!" : "ERROR!") << endl;


	// Test vector 4
    cout << "Testing known vector 4...";
	i=0;
	m[i++]=0xab; m[i++]=0x08; m[i++]=0x12; m[i++]=0x72;
	m[i++]=0x4a; m[i++]=0x7f; m[i++]=0x1e; m[i++]=0x34;
	m[i++]=0x27; m[i++]=0x42; m[i++]=0xcb; m[i++]=0xed;
	m[i++]=0x37; m[i++]=0x4d; m[i++]=0x94; m[i++]=0xd1;
	m[i++]=0x36; m[i++]=0xc6; m[i++]=0xb8; m[i++]=0x79;
	m[i++]=0x5d; m[i++]=0x45; m[i++]=0xb3; m[i++]=0x81;
	m[i++]=0x98; m[i++]=0x30; m[i++]=0xf2; m[i++]=0xc0;
	m[i++]=0x44; m[i++]=0x91; m[i++]=0xfa; m[i++]=0xf0;
	m[i++]=0x99; m[i++]=0x0c; m[i++]=0x62; m[i++]=0xe4;
	m[i++]=0x8b; m[i++]=0x80; m[i++]=0x18; m[i++]=0xb2;
	m[i++]=0xc3; m[i++]=0xe4; m[i++]=0xa0; m[i++]=0xfa;
	m[i++]=0x31; m[i++]=0x34; m[i++]=0xcb; m[i++]=0x67;
	m[i++]=0xfa; m[i++]=0x83; m[i++]=0xe1; m[i++]=0x58;
	m[i++]=0xc9; m[i++]=0x94; m[i++]=0xd9; m[i++]=0x61;
	m[i++]=0xc4; m[i++]=0xcb; m[i++]=0x21; m[i++]=0x09;
	m[i++]=0x5c; m[i++]=0x1b; m[i++]=0xf9; 
	sz = i;
	i=0;
	k[i++]=0xe1; k[i++]=0xa5; k[i++]=0x66; k[i++]=0x8a;
	k[i++]=0x4d; k[i++]=0x5b; k[i++]=0x66; k[i++]=0xa5;
	k[i++]=0xf6; k[i++]=0x8c; k[i++]=0xc5; k[i++]=0x42;
	k[i++]=0x4e; k[i++]=0xd5; k[i++]=0x98; k[i++]=0x2d;
	i=0;
	r[i++]=0x12; r[i++]=0x97; r[i++]=0x6a; r[i++]=0x08;
	r[i++]=0xc4; r[i++]=0x42; r[i++]=0x6d; r[i++]=0x0c;
	r[i++]=0xe8; r[i++]=0xa8; r[i++]=0x24; r[i++]=0x07;
	r[i++]=0xc4; r[i++]=0xf4; r[i++]=0x82; r[i++]=0x07;
	i=0;
	n[i++]=0x9a; n[i++]=0xe8; n[i++]=0x31; n[i++]=0xe7;
	n[i++]=0x43; n[i++]=0x97; n[i++]=0x8d; n[i++]=0x3a;
	n[i++]=0x23; n[i++]=0x52; n[i++]=0x7c; n[i++]=0x71;
	n[i++]=0x28; n[i++]=0x14; n[i++]=0x9e; n[i++]=0x3a;
	i=0;
	s[i++]=0x80; s[i++]=0xf8; s[i++]=0xc2; s[i++]=0x0a;
	s[i++]=0xa7; s[i++]=0x12; s[i++]=0x02; s[i++]=0xd1;
	s[i++]=0xe2; s[i++]=0x91; s[i++]=0x79; s[i++]=0xcb;
	s[i++]=0xcb; s[i++]=0x55; s[i++]=0x5a; s[i++]=0x57;
	i=0;
	expected[i++]=0x51; expected[i++]=0x54; expected[i++]=0xad; expected[i++]=0x0d;
	expected[i++]=0x2c; expected[i++]=0xb2; expected[i++]=0x6e; expected[i++]=0x01;
	expected[i++]=0x27; expected[i++]=0x4f; expected[i++]=0xc5; expected[i++]=0x11;
	expected[i++]=0x48; expected[i++]=0x49; expected[i++]=0x1f; expected[i++]=0x1b;

    poly.k(k);
    poly.r(r);
    poly.stamp(out, n, m, sz);

    ok = true;
	for(i=0;i<16;i++)
		if(out[i] != expected[i]) {
            ok = false;
            break;
        }

    ok &= poly.verify(out, n, m, sz);
    fails += !ok;
    cout << (ok ? "OK!" : "ERROR!") << endl;

    return fails;
}
