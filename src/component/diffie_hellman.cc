// EPOS Elliptic Curve Diffie-Hellman (ECDH) Component Implementation

#include <diffie_hellman.h>

__BEGIN_SYS

// Class attributes
//TODO: base point is dependent of SECRET_SIZE
const unsigned char Diffie_Hellman::_default_base_point_x[SECRET_SIZE] =
{
    '\x86', '\x5B', '\x2C', '\xA5',
    '\x7C', '\x60', '\x28', '\x0C',
    '\x2D', '\x9B', '\x89', '\x8B',
    '\x52', '\xF7', '\x1F', '\x16'
};

const unsigned char Diffie_Hellman::_default_base_point_y[SECRET_SIZE] =
{
    '\x83', '\x7A', '\xED', '\xDD',
    '\x92', '\xA2', '\x2D', '\xC0',
    '\x13', '\xEB', '\xAF', '\x5B',
    '\x39', '\xC8', '\x5A', '\xCF'
};


// Class methods
Diffie_Hellman::Diffie_Hellman(const Elliptic_Curve_Point & base_point) : _base_point(base_point)
{
    generate_keypair();
}

Diffie_Hellman::Diffie_Hellman()
{
    new (&_base_point.x) Bignum(_default_base_point_x, SECRET_SIZE);
    new (&_base_point.y) Bignum(_default_base_point_y, SECRET_SIZE);
    _base_point.z = 1;
    generate_keypair();
}

Diffie_Hellman::Shared_Key Diffie_Hellman::shared_key(Elliptic_Curve_Point public_key)
{
    db<Diffie_Hellman>(TRC) << "Diffie_Hellman::shared_key(pub=" << public_key << ",priv=" << _private << ")" << endl;

    public_key *= _private;
    public_key.x ^= public_key.y;

    db<Diffie_Hellman>(INF) << "Diffie_Hellman: shared key = " << public_key.x << endl;
    return public_key.x;
}

__END_SYS
