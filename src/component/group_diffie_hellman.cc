// EPOS Group Diffie-Hellman Component Implementation

#include <group_diffie_hellman.h>
#include <utility/random.h>

__BEGIN_SYS

Group_Diffie_Hellman::Private_Key Group_Diffie_Hellman::generate_private_key(const Number& q) const
{
	Group_Diffie_Hellman::Private_Key generated;
	do{
		generated = Random::random() % q-1;
	} while(generated == 0 || generated == 1 || gcd(generated, q-1) > 1);

	return generated;
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::insert_key(Group_Diffie_Hellman::Round_Key round_key) const
{
	db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << round_key << ",priv=" << _private_key << ")" << endl;

	round_key = mod_exp(round_key, _private_key, _parameters.q());

	db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;

	return round_key;
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::insert_key() const
{
	db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << _parameters.base() << ",priv=" << _private_key << ")" << endl;

	Round_Key round_key(_parameters.base());
	round_key = mod_exp(round_key, _private_key, _parameters.q());	

	db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;

	return round_key;
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::remove_key(Group_Diffie_Hellman::Round_Key round_key) const
{
	db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << round_key << ",priv=" << _private_key << ")" << endl;

	round_key = mod_exp(round_key, inverted_private_key(), _parameters.q());

	db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;

	return round_key;
}

__END_SYS
