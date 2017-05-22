// EPOS Group Diffie-Hellman Component Implementation

#include <group_diffie_hellman.h>

__BEGIN_SYS

// Class methods
Group_Diffie_Hellman::Group_Diffie_Hellman(const Parameters & parameters) : _private(2/*TODO random generation*/), _parameters(parameters)
{}

Group_Diffie_Hellman::Group_Diffie_Hellman() : _private(2/*TODO random generation*/), _parameters(7, 23)
{}

Group_Diffie_Hellman::Parameters Group_Diffie_Hellman::parameters() const
{
	return _parameters;
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::insert_key(const Group_Diffie_Hellman::Round_Key round_key) const
{
	db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << round_key << ",priv=" << _private << ")" << endl;
	Round_Key result(round_key);

  result.modular_exponentiation(_private, _parameters.q());
  db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;
  return result;
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::insert_key() const
{
  db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << _parameters.base() << ",priv=" << _private << ")" << endl;
	Round_Key round_key(_parameters.base());

	round_key.modular_exponentiation(_private, _parameters.q());

	db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;

	return round_key;
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::remove_key(const Group_Diffie_Hellman::Round_Key round_key) const
{
	db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << round_key << ",priv=" << _private << ")" << endl;

	Private_Key private_inverse(_private);
	//private_inverse._mod = Bignum(_parameters.q()); //how to set the mod? :(
	private_inverse.invert();

	Round_Key result(round_key);

	result.modular_exponentiation(private_inverse, _parameters.q());

	db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;

	return result;
}

__END_SYS
