// EPOS Elliptic Curve Point Component Test Program

#include <elliptic_curve_point.h>
#include <cipher.h>
#include <utility/ostream.h>
#include <utility/random.h>
#include <utility/bignum.h>

using namespace EPOS;

OStream cout;

const unsigned char _default_base_point_x[Cipher::KEY_SIZE] =
{
    '\x86', '\x5B', '\x2C', '\xA5',
    '\x7C', '\x60', '\x28', '\x0C',
    '\x2D', '\x9B', '\x89', '\x8B',
    '\x52', '\xF7', '\x1F', '\x16'
};

const unsigned char _default_base_point_y[Cipher::KEY_SIZE] =
{
    '\x83', '\x7A', '\xED', '\xDD',
    '\x92', '\xA2', '\x2D', '\xC0',
    '\x13', '\xEB', '\xAF', '\x5B',
    '\x39', '\xC8', '\x5A', '\xCF'
};

// 0xfffffffdfffffffe0000000200000000
const unsigned char _bsgs_min[Cipher::KEY_SIZE] =
{
    '\x00', '\x00', '\x00', '\x00',
    '\x02', '\x00', '\x00', '\x00',
    '\xfe', '\xff', '\xff', '\xff',
    '\xfd', '\xff', '\xff', '\xff'
};

// 0xfffffffe00000001fffffffe00000000
const unsigned char _bsgs_max[Cipher::KEY_SIZE] =
{
    '\x00', '\x00', '\x00', '\x00',
    '\xfe', '\xff', '\xff', '\xff',
    '\x01', '\x00', '\x00', '\x00',
    '\xfe', '\xff', '\xff', '\xff'
};

const auto SECRET_SIZE = Cipher::KEY_SIZE;

typedef _UTIL::Bignum<SECRET_SIZE> Bignumba;

int main()
{
    // using baby step giant step idea

    Elliptic_Curve_Point base, ecp;
	new (&base.x) Bignumba(_default_base_point_x, Cipher::KEY_SIZE);
	new (&base.y) Bignumba(_default_base_point_y, Cipher::KEY_SIZE);
	base.z = 1;

    Bignumba cur(_bsgs_min, Cipher::KEY_SIZE);
    Bignumba max(_bsgs_max, Cipher::KEY_SIZE);
    Bignumba zero(0);
    Bignumba one(1);

    Bignumba print = cur;
    print += 1000;
    while(cur <= max){
        ecp = base;

        ecp *= cur;

        if(ecp.x == zero || ecp.y == zero || ecp.z == zero ||
           ecp.x == one  || ecp.y == one){
            cout << endl;
            cout << "cur=" << cur << endl;
            cout << "ecp=" << ecp << endl;
        }

        if(cur == print){
            cout << "--- cur=" << cur << endl;
            print += 1000;
        }

        cur += 1;
    }

    return 0;
}
