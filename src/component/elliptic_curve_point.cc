#include <elliptic_curve_point.h>

__BEGIN_SYS

void Elliptic_Curve_Point::operator*=(const Coordinate & b)
{
    // Finding last '1' bit of b
    static const unsigned int bits_in_digit = sizeof(Coordinate::Digit) * 8;
    Coordinate::Digit now;
    unsigned int b_len = sizeof(Coordinate) / sizeof(Coordinate::Digit);
    for(; (b_len > 1) && (b[b_len - 1] == 0); b_len--);
    if((b_len == 0) || (b[b_len - 1] == 0)) {
        x = 0;
        y = 0;
        z = 0;
        return;
    }

    now = b[b_len - 1];

    bool bin[bits_in_digit]; // Binary representation of 'now'
    unsigned int current_bit = bits_in_digit;

    Elliptic_Curve_Point pp(*this);

    for(int i = bits_in_digit - 1; i >= 0; i--) {
        if(now % 2)
            current_bit = i + 1;
        bin[i] = now % 2;
        now /= 2;
    }

    for(int i = b_len - 1; i >= 0; i--) {
        for(; current_bit < bits_in_digit; current_bit++) {
            jacobian_double();
            if(bin[current_bit])
                add_jacobian_affine(pp);
        }
        if(i > 0) {
            now = b[i-1];
            for(int j = bits_in_digit-1; j >= 0; j--) {
                bin[j] = now % 2;
                now /= 2;
            }
            current_bit = 0;
        }
    }

    Coordinate Z;
    z.invert();
    Z = z;
    Z *= z;

    x *= Z;
    Z *= z;

    y *= Z;
    z = 1;
}

void Elliptic_Curve_Point::jacobian_double()
{
    Coordinate B, C(x), aux(z);

    aux *= z; C -= aux;
    aux += x; C *= aux;
    C *= 3;

    z *= y; z *= 2;

    y *= y; B = y;

    y *= x; y *= 4;

    B *= B; B *= 8;

    x = C; x *= x;
    aux = y; aux *= 2;
    x -= aux;

    y -= x; y *= C;
    y -= B;
}

void Elliptic_Curve_Point::add_jacobian_affine(const Elliptic_Curve_Point &b)
{
    Coordinate A(z), B, C, X, Y, aux, aux2;

    A *= z;

    B = A;

    A *= b.x;

    B *= z; B *= b.y;

    C = A; C -= x;

    B -= y;

    X = B; X *= B;
    aux = C; aux *= C;

    Y = aux;

    aux2 = aux; aux *= C;
    aux2 *= 2; aux2 *= x;
    aux += aux2; X -= aux;

    aux = Y; Y *= x;
    Y -= X; Y *= B;
    aux *= y; aux *= C;
    Y -= aux;

    z *= C;

    x = X; y = Y;
}

__END_SYS
