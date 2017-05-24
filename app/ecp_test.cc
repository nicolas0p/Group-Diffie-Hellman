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

const auto SECRET_SIZE = Cipher::KEY_SIZE;

typedef _UTIL::Bignum<SECRET_SIZE> Bignumba;

int main()
{
    Elliptic_Curve_Point base, ecp, ecp_i, ecp_is;
	new (&base.x) Bignumba(_default_base_point_x, Cipher::KEY_SIZE);
	new (&base.y) Bignumba(_default_base_point_y, Cipher::KEY_SIZE);
	base.z = 1;

    ecp = base;

    Bignumba a, a_i, a_is;
    a.randomize();

    cout << "base=" << ecp << endl;
    cout << " a=" << a << endl ;

    ecp *= a;

    a_i = a;
    a_i.invert();

    a_is = a;
    a_is.invert_special();

    cout << endl;
    cout << "base*a=" << ecp << endl;
    cout <<" a_i=" << a_i << endl;
    cout << "a_is=" << a_is << endl;

    ecp_i = ecp;
    ecp_i *= a_i;

    ecp_is = ecp;
    ecp_is *= a_is;

    cout << endl;
    cout << "(base*a)*a_i=" << ecp_i << endl;
    cout << "(base*a)*a_is=" << ecp_is << endl;

    a_i *= a;
    a_is *= a;

    cout << endl;
    cout << "a_i*a=" << a_i << endl;
    cout << "a_is*a=" << a_is << endl;

    return 0;
}
