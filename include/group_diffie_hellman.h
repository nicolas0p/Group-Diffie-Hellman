// EPOS Group Diffie-Hellman (GDH) Component Declarations

#ifndef __group_diffie_hellman_h
#define __group_diffie_hellman_h

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
    typedef long long Number;

public:
    class Parameters {
		public:
			Parameters(const Number& base, const Number& q) : _base(base), _q(q) {}
			Parameters(const Parameters& params) : _base(params.base()), _q(params.q()) {}
			Number base() const {return _base;}
			Number q() const {return _q;}

			friend OStream &operator<<(OStream & out, const Parameters & b){
				out << "base = " << b.base() << ". q = " << b.q();
				return out;
			}

		private:
			Number _base, _q; //q is used to calculate the modulus. base is the base of the exponentiation
	};

    typedef Number Round_Key;
    typedef Number Private_Key;
    typedef Number Shared_Key;
	typedef int Group_Id;

	Group_Diffie_Hellman();
	Group_Diffie_Hellman(const Parameters& parameters);

	Parameters parameters() const;

	Round_Key insert_key() const;
	Round_Key insert_key(const Round_Key round_key) const;

	Round_Key remove_key(const Round_Key round_key) const;


private:
	Private_Key _private;
	Parameters _parameters;
};

__END_SYS

#endif
