// EPOS Group Diffie-Hellman Component Implementation

#include <group_diffie_hellman.h>

__BEGIN_SYS

// Class attributes
//TODO: base point is dependent of SECRET_SIZE
const unsigned char Group_Diffie_Hellman::_default_base_point_x[SECRET_SIZE] =
{
    '\x86', '\x5B', '\x2C', '\xA5',
    '\x7C', '\x60', '\x28', '\x0C',
    '\x2D', '\x9B', '\x89', '\x8B',
    '\x52', '\xF7', '\x1F', '\x16'
};

const unsigned char Group_Diffie_Hellman::_default_base_point_y[SECRET_SIZE] =
{
    '\x83', '\x7A', '\xED', '\xDD',
    '\x92', '\xA2', '\x2D', '\xC0',
    '\x13', '\xEB', '\xAF', '\x5B',
    '\x39', '\xC8', '\x5A', '\xCF'
};


// Class methods
Group_Diffie_Hellman::Group_Diffie_Hellman()
{
	// initialize base with above defaults
	// q("mod") will be the same as BigNum._mod
	new (&_base_point.x) Bignum(_default_base_point_x, SECRET_SIZE);
	new (&_base_point.y) Bignum(_default_base_point_y, SECRET_SIZE);
	_base_point.z = 1;

	_private.randomize();
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::insert_key(Group_Diffie_Hellman::Round_Key round_key) const
{
	db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << round_key << ",priv=" << _private << ")" << endl;

	round_key *= _private;
  db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;
  return round_key;
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::insert_key() const
{
  db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << _base_point << ",priv=" << _private << ")" << endl;
	Group_Diffie_Hellman::Round_Key round_key;
	new (&round_key.x) Bignum(_default_base_point_x, SECRET_SIZE);
	new (&round_key.y) Bignum(_default_base_point_y, SECRET_SIZE);
	round_key.z = 1;

	round_key *= _private;

	db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;

	return round_key;
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::remove_key(Group_Diffie_Hellman::Round_Key round_key) const
{
	db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << round_key << ",priv=" << _private << ")" << endl;

	Private_Key private_inverse(_private);
	private_inverse.invert();

	round_key *= private_inverse;

	db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;

	return round_key;
}

__END_SYS
