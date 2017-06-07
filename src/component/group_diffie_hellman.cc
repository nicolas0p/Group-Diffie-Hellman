// EPOS Group Diffie-Hellman Component Implementation

#include <group_diffie_hellman.h>

__BEGIN_SYS

// Class methods
Group_Diffie_Hellman::Group_Diffie_Hellman(const Parameters & parameters) : _private(2/*TODO random generation*/), _parameters(parameters)
{}

Group_Diffie_Hellman::Group_Diffie_Hellman() : _private(2/*TODO random generation*/), _parameters(7, 767410129)
{}

Group_Diffie_Hellman::Parameters Group_Diffie_Hellman::parameters() const
{
	do{
		_private = Random::random() % q-1;
	} while(_private == 0 || _private == 1 || gcd(_private, q-1) > 1);
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::insert_key(const Group_Diffie_Hellman::Round_Key round_key) const
{
	db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << round_key << ",priv=" << _private << ")" << endl;
	Round_Key result(round_key);

  //result = modular_exponentiation(result, _private, _parameters.q());
  db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;
  return result;
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::insert_key() const
{
  db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << _parameters.base() << ",priv=" << _private << ")" << endl;
	Round_Key round_key(_parameters.base());

	//round_key = modular_exponentiation(round_key, _private, _parameters.q());

	db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;

	return round_key;
}

__END_SYS
