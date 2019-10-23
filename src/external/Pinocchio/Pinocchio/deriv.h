/*  This file is part of the Pinocchio automatic rigging library.
    Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef DERIV_H_BDB262D4_4631_11E9_84BF_67993DCD7514
#define DERIV_H_BDB262D4_4631_11E9_84BF_67993DCD7514

#include "vector.h"

template<class Real, int Vars> class Deriv {
  public:
    typedef Deriv<Real, Vars> Self;

    Deriv() : x(Real()) {}
    Deriv(const Real &inX) : x(inX) {}
    Deriv(const Real &inX, int varNum) : x(inX) { d[varNum] = Real(1.); }
    Deriv(const Self &inD) : x(inD.x), d(inD.d) {}
    Deriv(const Real &inX, const Vector<Real, Vars> &inD) : x(inX), d(inD) {}

    Real getReal() const { return x; }
    Real getDeriv(int num = 0) const { return d[num]; }

    Self operator*(const Self &other) const { return Self(x * other.x, x * other.d + other.x * d); }
    Self operator+(const Self &other) const { return Self(x + other.x, d + other.d); }
    Self operator-(const Self &other) const { return Self(x - other.x, d - other.d); }
    Self operator/(const Self &other) const { return Self(x / other.x, (other.x * d - x * other.d) / SQR(other.x)); }
    Self operator-() const { return Self(-x, -d); }
    Self &operator+=(const Self &other) { x += other.x; d += other.d; return *this; }
    Self &operator-=(const Self &other) { x -= other.x; d -= other.d; return *this; }
    Self &operator*=(const Self &other) { (*this) = (*this) * other; return *this; }
    Self &operator/=(const Self &other) { (*this) = (*this) / other; return *this; }

    bool operator<(const Self &other) const { return x < other.x; }
    bool operator<=(const Self &other) const { return x <= other.x; }
    bool operator>(const Self &other) const { return x > other.x; }
    bool operator>=(const Self &other) const { return x >= other.x; }
    bool operator==(const Self &other) const { return x == other.x; }
    bool operator!=(const Self &other) const { return x != other.x; }

    operator Real() const { return x; }

    //for internal use
    const Real &_x() const { return x; }
    const Vector<Real, Vars> &_d() const { return d; }

  private:

    Real x;
    Vector<Real, Vars> d;
};

#define DerivRV Deriv<Real, Vars>
#define ONEVAR(func, deriv) template<class Real, int Vars> \
  DerivRV func(const DerivRV &x) { return DerivRV(func(x._x()), x._d() * (deriv)); }

ONEVAR(sqrt, Real(0.5) / sqrt(x._x()))
ONEVAR(log, Real(1.) / x._x())
//the number is 1 / log(10)
ONEVAR(log10, Real(0.43429448190325182765) / x._x())
ONEVAR(exp, exp(x._x()))
ONEVAR(sin, cos(x._x()))
ONEVAR(cos, -sin(x._x()))
ONEVAR(tan, Real(1.) / SQR(cos(x._x())))
ONEVAR(acos, Real(-1.) / sqrt(Real(1. - SQR(x._x()))))
ONEVAR(asin, Real(1.) / sqrt(Real(1. - SQR(x._x()))))
ONEVAR(atan, Real(1.) / (Real(1.) + SQR(x._x())))
ONEVAR(fabs, x._x() < Real(0.) ? Real(-1.) : Real(1.))

#undef ONEVAR
#define TWOVAR(func, derivx, derivy) template<class Real, int Vars> \
  DerivRV func(const DerivRV &x, const DerivRV &y) { return DerivRV(func(x._x(), y._x()), x._d() * (derivx) + y._d() * (derivy)); }

TWOVAR(pow, y._x() * pow(x._x(), y._x() - Real(1.)), log(x._x()) * pow(x._x(), y._x()))
TWOVAR(atan2, y._x() / (SQR(x._x()) + SQR(y._x())), -x._x() / (SQR(x._x()) + SQR(y._x())))

#undef TWOVAR
#undef DerivRV

template <class charT, class traits, class Real, int Vars>
std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& os, const Deriv<Real, Vars> &d)
{
  os << "Deriv(" << d._x() << ", " << d._d() << ")";
  return os;
}

#endif // DERIV_H_BDB262D4_4631_11E9_84BF_67993DCD7514
