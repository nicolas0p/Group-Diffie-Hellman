// EPOS Elliptic Curve Diffie-Hellman (ECDH) Component Declarations

#ifndef __group_diffie_hellman_h
#define __Group_diffie_hellman_h

#include <system/config.h>
#include <utility/bignum.h>
#include <cipher.h>

__BEGIN_SYS

class Group_Diffie_Hellman
{
public:
	static const unsigned int SECRET_SIZE = Cipher::KEY_SIZE;
	static const unsigned int PUBLIC_KEY_SIZE = 2 * SECRET_SIZE;

private:
    typedef _UTIL::Bignum<SECRET_SIZE> Bignum;

public:
    class Parameters {
		public:
			Parameters(const Bignum& base, const Bignum& q) : _base(base), _q(q) {}
			Bignum base() {return _base;} //is this safe? Does it even matter?
			Bignum q() {return _q;}

		private:
			Bignum _base, _q; //q is used to calculate the modulus. base is the base of the exponentiation
	}

    typedef Bignum Round_Key;
    typedef Bignum Private_Key;

	Group_Diffie_Hellman();
	Group_Diffie_Hellman(const Parameters& parameters);

	Round_Key insert_key();
	Round_Key insert_key(const Round_Key& round_key);

	Round_Key remove_key(const Round_Key& round_key);

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
	Parameters _parameters;
};

__END_SYS

#endif
