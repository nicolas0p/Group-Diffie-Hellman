#ifndef __elliptic_curve_point_h
#define __elliptic_curve_point_h

#include <utility/bignum.h>
#include <cipher.h>

__BEGIN_SYS

class Elliptic_Curve_Point
{
private:
  typedef _UTIL::Bignum<Cipher::KEY_SIZE> Bignum;

public:
  typedef Bignum Coordinate;

  Elliptic_Curve_Point() __attribute__((noinline)) { }

  void operator*=(const Coordinate & b);

  friend Debug &operator<<(Debug &out, const Elliptic_Curve_Point &a) {
    out << "{x=" << a.x << ",y=" << a.y << ",z=" << a.z << "}";
    return out;
  }
  friend OStream &operator<<(OStream &out, const Elliptic_Curve_Point &a) {
    out << "{x=" << a.x << ",y=" << a.y << ",z=" << a.z << "}";
    return out;
  }

private:
  void jacobian_double();
  void add_jacobian_affine(const Elliptic_Curve_Point &b);

  public:
      Coordinate x, y, z;
};

__END_SYS

#endif
