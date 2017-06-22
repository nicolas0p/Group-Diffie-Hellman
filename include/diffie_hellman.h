// EPOS Elliptic Curve Diffie-Hellman (ECDH) Component Declarations

#ifndef __diffie_hellman_h
#define __diffie_hellman_h

#include <system/config.h>
#include <utility/bignum.h>
#include <cipher.h>

__BEGIN_SYS

class Diffie_Hellman
{
public:
	static const unsigned int SECRET_SIZE = Cipher::KEY_SIZE;
	static const unsigned int PUBLIC_KEY_SIZE = 2 * SECRET_SIZE;

private:
    typedef _UTIL::Bignum<SECRET_SIZE> Bignum;

	class Elliptic_Curve_Point
	{
    public:
        typedef Diffie_Hellman::Bignum Coordinate;

		Elliptic_Curve_Point() __attribute__((noinline)) { }

		void operator*=(const Coordinate & b);

		friend Debug &operator<<(Debug &out, const Elliptic_Curve_Point &a) {
			out << "{x=" << a.x << ",y=" << a.y << ",z=" << a.z << "}";
			return out;
		}
		friend OStream &operator<<(OStream &out, const Elliptic_Curve_Point &a) {
			out << "{x=" << a.x << ",y=" << a.y << ",z=" << a.z << "}";
			return out;
		}

	private:
		void jacobian_double();
		void add_jacobian_affine(const Elliptic_Curve_Point &b);

    public:
        Coordinate x, y, z;
	};

public:
    typedef Elliptic_Curve_Point Public_Key;
    typedef Bignum Shared_Key;
    typedef Bignum Private_Key;

	Diffie_Hellman();
	Diffie_Hellman(const Elliptic_Curve_Point & base_point);

	Elliptic_Curve_Point public_key() { return _public; }

	Shared_Key shared_key(Elliptic_Curve_Point public_key);

private:
	void generate_keypair() {
		db<Diffie_Hellman>(TRC) << "Diffie_Hellman::generate_keypair()" << endl;

		_private.randomize();

		db<Diffie_Hellman>(INF) << "Diffie_Hellman Private: " << _private << endl;
		db<Diffie_Hellman>(INF) << "Diffie_Hellman Base Point: " << _base_point << endl;

		_public = _base_point;
		_public *= _private;

		db<Diffie_Hellman>(INF) << "Diffie_Hellman Public: " << _public << endl;
	}

private:
	Private_Key _private;
	Elliptic_Curve_Point _base_point;
	Elliptic_Curve_Point _public;
	static const unsigned char _default_base_point_x[SECRET_SIZE];
	static const unsigned char _default_base_point_y[SECRET_SIZE];
};

__END_SYS

#endif
