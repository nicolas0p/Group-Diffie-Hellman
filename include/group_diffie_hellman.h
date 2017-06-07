// EPOS Group Diffie-Hellman (GDH) Component Declarations

#ifndef __group_diffie_hellman_h
#define __group_diffie_hellman_h

#include <system/config.h>
#include <utility/ostream.h>

__BEGIN_SYS

class Group_Diffie_Hellman
{
private:
	typedef unsigned long long Number;
public:
	static const Number base = 7;
	static const Number q = 767410129;

	typedef Number Round_Key;
	typedef Number Private_Key;
	typedef int Group_Id;

	Private_Key _private;
	Group_Diffie_Hellman();
	Group_Diffie_Hellman(const Parameters& parameters);

	Parameters parameters() const;

	Round_Key insert_key() const;
	Round_Key insert_key(const Round_Key round_key) const;

	static Round_Key mod_exp(Round_Key base, Round_Key exponent){
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

	static Round_Key invert(Private_Key private_key){
		return mod_inv(private_key, q-1);
	}

	Round_Key remove_key(Round_Key round_key) const
	{
		// db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << round_key << ",priv=" << _private << ")" << endl;

		round_key = mod_exp(round_key, invert(_private));

		// db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;

		return round_key;
	}

	static Number * egcd(Number a, Number b){
		Number * result = new Number[3];

		if(a == 0){
			result[0] = b;
			result[1] = 0;
			result[2] = 1;
			return result;
		}

		Number * inner = egcd(b % a, a);
		result[0] = inner[0];
		result[1] = inner[2] - (b / a) * inner[1];
		result[2] = inner[1];
		delete[] inner;

		return result;
	}

	static Number mod_inv(Number a, Number b){
		Number * result = egcd(a, b);

		a = result[0];
		if(result[1] > b){
			b = (result[1] + b) % b;
		} else {
			b = result[1] % b;
		}

		delete[] result;

		if(a != 1){
			// raise exception modular inverse does not exist
			// shouldnt ever happen
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
