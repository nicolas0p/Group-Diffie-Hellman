// EPOS Group Diffie-Hellman (GDH) Component Declarations

#ifndef __group_diffie_hellman_h
#define __group_diffie_hellman_h

#include <system/config.h>
#include <utility/ostream.h>
#include <utility/malloc.h>

__BEGIN_SYS

class Group_Diffie_Hellman
{
private:
	typedef unsigned long long Number;

public:
	typedef Number Round_Key;
	typedef Number Private_Key;
	typedef Number Shared_Key;
	typedef int Group_Id;

	class Parameters
	{
			Number _base, _q;
		public:
			Parameters(const Parameters& params) : _base(params.base()), _q(params.q()) {}
			Parameters(const Number& base, const Number& q) : _base(base), _q(q) {}

			Number base() const { return _base; }
			Number q() const { return _q; }

			friend OStream & operator<<(OStream & db, const Parameters & m) {
				db << "(Parameters base=" << m._base << ", q=" << m._q << ")";
				return db;
			}
	};

private:
	Private_Key generate_private_key(const Number& q) const;

	Parameters _parameters;
	Private_Key _private_key;

public:
	Group_Diffie_Hellman(const Parameters & params) :
		_parameters(params),
		_private_key(generate_private_key(params.q()))
	{}

	Group_Diffie_Hellman() :
		_parameters(7, 767410129),
		_private_key(generate_private_key(767410129))
	{}

	Parameters parameters() const { return _parameters; }
	Private_Key private_key() const { return _private_key; }

	Round_Key insert_key() const;
	Round_Key insert_key(Round_Key round_key) const;
	Round_Key remove_key(Round_Key round_key) const;

private:

	Round_Key inverted_private_key() const
	{
		return mod_inv(_private_key, _parameters.q()-1);
	}

	static Round_Key mod_exp(Round_Key base, Round_Key exponent, const Number& q){
		int i = 1;
		Number y = 1;
		while(exponent > 1){
			if((exponent % 2) == 1){
				exponent -= 1;
				y *= base;
				y %= q;
			}
			base *= base;
			base %= q;
			exponent /= 2;
			i++;
		}
		return (base * y) % q;
	}

	struct EGCD_Values {
		Number a;
		Number b;
		Number c;
	};

	static EGCD_Values egcd(Number a, Number b){
		EGCD_Values result;

		if(a == 0){
			result.a = b;
			result.b = 0;
			result.c = 1;
			return result;
		}

		EGCD_Values inner = egcd(b % a, a);
		result.a = inner.a;
		result.b = inner.c - (b / a) * inner.b;
		result.c = inner.b;

		return result;
	}

	static Number mod_inv(Number a, Number b){
		EGCD_Values result = egcd(a, b);

		a = result.a;
		if(result.b > b){
			b = (result.b + b) % b;
		} else {
			b = result.b % b;
		}

		if(a != 1){
			// raise exception modular inverse does not exist
			// shouldnt ever happen if the private key is done correctly
			return -1;
		}

		return b;
	}

	static Number gcd(Number a, Number b){
		while(b > 0){
			Number aux = b;
			b = a % b;
			a = aux;
		}
		return a;
	}
};

__END_SYS

#endif
