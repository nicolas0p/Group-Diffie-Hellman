// EPOS Group Diffie-Hellman (GDH) Component Declarations

#ifndef __group_diffie_hellman_h
#define __group_diffie_hellman_h

#include <system/config.h>

__BEGIN_SYS

class Group_Diffie_Hellman
{
// public:
// 	static const unsigned int SECRET_SIZE = Cipher::KEY_SIZE;
// 	static const unsigned int PUBLIC_KEY_SIZE = 2 * SECRET_SIZE;
private:
	typedef long long Number;

	static const long long base = 7;
	static const long long q = 23;

public:

	typedef Number Round_Key;
	typedef Number Private_Key;
	typedef int Group_Id;

	Group_Diffie_Hellman();

	Round_Key insert_key() const;
	Round_Key insert_key(const Round_Key round_key) const;

	Round_Key remove_key(const Round_Key round_key) const;

	static Round_Key mod_exp(Round_Key base, Round_Key exponent){
		return base;
	}

	static Round_Key invert(Private_Key private_key){
		return private_key;
	}

private:
	Private_Key _private;

// private:
// 	Private_Key _private;
// 	Elliptic_Curve_Point _base_point;
// 	static const unsigned char _default_base_point_x[SECRET_SIZE];
// 	static const unsigned char _default_base_point_y[SECRET_SIZE];
};

__END_SYS

#endif
