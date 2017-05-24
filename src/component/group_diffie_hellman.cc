// EPOS Group Diffie-Hellman Component Implementation

#include <group_diffie_hellman.h>
#include <utility/random.h>

__BEGIN_SYS

// Class methods
Group_Diffie_Hellman::Group_Diffie_Hellman()
{
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::insert_key(Group_Diffie_Hellman::Round_Key round_key) const
{
	db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << round_key << ",priv=" << _private << ")" << endl;

    round_key = mod_exp(round_key, _private);

    db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;
    return round_key;
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::insert_key() const
{
    db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << base << ",priv=" << _private << ")" << endl;

    Round_Key round_key = mod_exp(base, _private);

	db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;
	return round_key;
}

Group_Diffie_Hellman::Round_Key Group_Diffie_Hellman::remove_key(Group_Diffie_Hellman::Round_Key round_key) const
{
	db<Diffie_Hellman>(TRC) << "Diffie_Hellman::round_key(round=" << round_key << ",priv=" << _private << ")" << endl;

	round_key = mod_exp(round_key, invert(_private));

	db<Diffie_Hellman>(INF) << "Diffie_Hellman: round key = " << round_key << endl;

	return round_key;
}

__END_SYS
