// EPOS Group Diffie-Hellman (GDH) Component Declarations

#ifndef __group_diffie_hellman_h
#define __group_diffie_hellman_h

#include <system/config.h>
#include <utility/bignum.h>
#include <elliptic_curve_point.h>
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
  typedef Elliptic_Curve_Point Round_Key;
  typedef Bignum Private_Key;

	Group_Diffie_Hellman();

	Round_Key insert_key() const;
	Round_Key insert_key(const Round_Key round_key) const;

	Round_Key remove_key(const Round_Key round_key) const;

private:
	Private_Key _private;
	Elliptic_Curve_Point _base_point;
	static const unsigned char _default_base_point_x[SECRET_SIZE];
	static const unsigned char _default_base_point_y[SECRET_SIZE];
};

__END_SYS

#endif
