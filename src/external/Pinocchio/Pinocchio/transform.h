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

#ifndef TRANSFORM_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
#define TRANSFORM_H_BFCF2002_4190_11E9_AA8F_EFB66606E782

#include "vector.h"

namespace Pinocchio {

template<class Real = double> class Quaternion { // Normalized quaternion for representing rotations
  public:
    // Constructors
    Quaternion() : r(1.) { } // Initialize to identity
    Quaternion(const Quaternion &q) : r(q.r), v(q.v) {} // Copy constructor
    template<class R> Quaternion(const Quaternion<R> &q) : r(q.r), v(q.v) {} //Convert quaternions of other types

    template<class R> Quaternion(const R &w, const R &x, const R &y, const R &z)
    {
        r = w;
        v[0] = x;
        v[1] = y;
        v[2] = z;
    }

    // Axis angle constructor:
    template<class R> Quaternion(const Vector<R, 3> &axis,
      const R &angle) : r(cos(angle * Real(0.5))),
      v(sin(angle * Real(0.5)) * axis.normalize()) {}

    // Minimum rotation constructor:
    template<class R> Quaternion(const Vector<R, 3> &from,
      const Vector<R, 3> &to) : r(1.)
    {
      R fromLenSq = from.lengthsq(), toLenSq = to.lengthsq();
      if (fromLenSq < toLenSq) {
        if (fromLenSq < R(1e-16)) {
          return;
        }
        Vector<R, 3> mid = from * sqrt(toLenSq / fromLenSq) + to;
        R fac = 1. / sqrt(mid.lengthsq() * toLenSq);
        r = (mid * to) * fac;
        v = (mid % to) * fac;
      } else {
        if(toLenSq < R(1e-16)) {
          return;
        }
        Vector<R, 3> mid = from + to * sqrt(fromLenSq / toLenSq);
        R fac = 1. / sqrt(mid.lengthsq() * fromLenSq);
        r = (from * mid) * fac;
        v = (from % mid) * fac;
      }
    }

    //quaternion multiplication
    Quaternion operator*(const Quaternion &q) const { return Quaternion(r * q.r - v * q.v, r * q.v + q.r * v + v % q.v); }

    //transforming a std::vector
    Vector<Real, 3> operator*(const Vector<Real, 3> &p) const {
      Vector<Real, 3> v2 = v + v;
      Vector<Real, 3> vsq2 = v.apply(std::multiplies<Real>(), v2);
      Vector<Real, 3> rv2 = r * v2;
      Vector<Real, 3> vv2(v[1] * v2[2], v[0] * v2[2], v[0] * v2[1]);
      return Vector<Real, 3>(p[0] * (Real(1.) - vsq2[1] - vsq2[2]) + p[1] * (vv2[2] - rv2[2]) + p[2] * (vv2[1] + rv2[1]),
        p[1] * (Real(1.) - vsq2[2] - vsq2[0]) + p[2] * (vv2[0] - rv2[0]) + p[0] * (vv2[2] + rv2[2]),
        p[2] * (Real(1.) - vsq2[0] - vsq2[1]) + p[0] * (vv2[1] - rv2[1]) + p[1] * (vv2[0] + rv2[0]));
    }

    // Equality
    template<class R> bool operator==(const Quaternion<R> &oth) const {
      return (r == oth.r && v == oth.v) || (r == -oth.r && v == -oth.v);
    }

    Quaternion inverse() const { return Quaternion(-r, v); }

    Real getAngle() const { return Real(2.) * atan2(v.length(), r); }
    Vector<Real, 3> getAxis() const { return v.normalize(); }

    const Real &operator[](int i) const { return (i == 0) ? r : v[i - 1]; }
    void set(const Real &inR, const Vector<Real, 3> &inV) {
      Real ratio = Real(1.) / sqrt(inR * inR + inV.lengthsq());
      //normalize
      r = inR * ratio; v = inV * ratio;
    }

  private:
    Quaternion(const Real &inR, const Vector<Real, 3> &inV) : r(inR), v(inV) {}

    Real r;
    Vector<Real, 3> v;
};

template<class Real = double> class Transform { // T(v) = (rot * v * scale) + trans
  public:
    typedef Vector<Real, 3> Vec;

    Transform() : scale(1.) {}
    explicit Transform(const Real &inScale) : scale(inScale) {}
    explicit Transform(const Vec &inTrans) : scale(1.), trans(inTrans) {}
    Transform(const Quaternion<Real> &inRot, Real inScale = Real(1.), Vec inTrans = Vec()) : rot(inRot), scale(inScale), trans(inTrans) {}
    Transform(const Transform &t) : rot(t.rot), scale(t.scale), trans(t.trans) {}

    Transform operator*(const Transform &t) const { return Transform(rot * t.rot, scale * t.scale, trans + rot * (scale * t.trans)); }
    Vec operator*(const Vec &v) const { return rot * (v * scale) + trans; }
    Transform inverse() const { return Transform(rot.inverse(), 1. / scale, rot.inverse() * -trans * (1. / scale)); }

    Transform linearComponent() const { return Transform(rot, scale); }
    Vec mult3(const Vec &v) const { return rot * (v * scale); }

    Real getScale() const { return scale; }
    Vec getTrans() const { return trans; }
    Quaternion<Real> getRot() const { return rot; }

    void setScale( const Real &s ) { scale = s; }
    void setTrans( const Vec &t ) { trans = t; }
    void setRot( const Quaternion<Real> &r ) { rot = r; }

  private:
    Quaternion<Real> rot;
    Real scale;
    Vec trans;
};

template<class Real = double> class Matrix3 {
  public:
    typedef Vector<Real, 3> Vec;
    typedef Matrix3<Real> Self;

    Matrix3(const Real &diag = Real()) { m[0] = m[4] = m[8] = diag; m[1] = m[2] = m[3] = m[5] = m[6] = m[7] = Real(); }

    Matrix3(const Vec &c1, const Vec &c2, const Vec &c3) {
      m[0] = c1[0]; m[1] = c2[0]; m[2] = c3[0];
      m[3] = c1[1]; m[4] = c2[1]; m[5] = c3[1];
      m[6] = c1[2]; m[7] = c2[2]; m[8] = c3[2];
    }

    Matrix3(const Self &inM) { for(int i = 0; i < 9; ++i) m[i] = inM[i]; }

    Real &operator[](int idx) { return m[idx]; }
    const Real &operator[](int idx) const { return m[idx]; }
    Real &operator()(int row, int col) { return m[row * 3 + col]; }
    const Real &operator()(int row, int col) const { return m[row * 3 + col]; }

    Vec getRow(int row) const { row *= 3; return Vec(m[row], m[row + 1], m[row + 2]); }
    Vec getColumn(int col) const { return Vec(m[col], m[col + 3], m[col + 6]); }

    Self operator+(const Self &o) { Self out(S(0)); for(int i = 0; i < 9; ++i) out[i] = m[i] + o[i]; return out; }
    Self operator-(const Self &o) { Self out(S(0)); for(int i = 0; i < 9; ++i) out[i] = m[i] - o[i]; return out; }
    Self operator*(const Real &x) { Self out(S(0)); for(int i = 0; i < 9; ++i) out[i] = m[i] * x; return out; }
    Self operator/(const Real &x) { Self out(S(0)); for(int i = 0; i < 9; ++i) out[i] = m[i] / x; return out; }

    #define OPAS(op, typ, idx) Self &operator op(const typ &x) { for(int i = 0; i < 9; ++i) m[i] op x idx; return *this; }
      OPAS(+=, Self, [i])
      OPAS(-=, Self, [i])
      OPAS(*=, Real, )
      OPAS(/=, Real, )
    #undef OPAS

    Vec operator*(const Vec &v) const {
      return Vec(m[0] * v[0] + m[1] * v[1] + m[2] * v[2],
        m[3] * v[0] + m[4] * v[1] + m[5] * v[2],
        m[6] * v[0] + m[7] * v[1] + m[8] * v[2]);
    }

    Self operator*(const Self &o) const {
      return Self((*this) * Vec(o[0], o[3], o[6]), (*this) * Vec(o[1], o[4], o[7]), (*this) * Vec(o[2], o[5], o[8]));
    }

    Self operator~() const { // Transpose
      Self out(S(0)); // Uninitialized
      out[0] = m[0]; out[4] = m[4]; out[8] = m[8];
      out[1] = m[3]; out[3] = m[1]; out[2] = m[6]; out[6] = m[2]; out[5] = m[7]; out[7] = m[5];
      return out;
    }

    Self operator!() const { // Invert
      Self out(S(0));
      Real d = det();
      if(d == Real()) {
        return Self();
      }
      d = Real(1.) / d;
      out[0] = d * (m[4] * m[8] - m[5] * m[7]);
      out[1] = d * (m[2] * m[7] - m[1] * m[8]);
      out[2] = d * (m[1] * m[5] - m[2] * m[4]);
      out[3] = d * (m[5] * m[6] - m[3] * m[8]);
      out[4] = d * (m[0] * m[8] - m[2] * m[6]);
      out[5] = d * (m[2] * m[3] - m[0] * m[5]);
      out[6] = d * (m[3] * m[7] - m[4] * m[6]);
      out[7] = d * (m[1] * m[6] - m[0] * m[7]);
      out[8] = d * (m[0] * m[4] - m[1] * m[3]);
      return out;
    }

    Real det() const {
      return m[0] * (m[4] * m[8] - m[5] * m[7]) - m[1] * (m[3] * m[8] - m[5] * m[6]) + m[2] * (m[3] * m[7] - m[4] * m[6]);
    }

  private:
    struct S {
      S(int) {}
    };
    
    Matrix3(const S &) {} // No initialization

    Real m[9];
};

} // namespace Pinocchio

// overide <<  operator to allows us to print a Quaternion object to terminal
inline std::ostream& operator<<(std::ostream& os, const Pinocchio::Quaternion<double> &obj) {
  std::cout << "   Rotation Angle: " << obj.getAngle() << std::endl; // Print angle
  Vector3 axis = obj.getAxis(); // Print axis std::vector
  std::cout << " Rotation Axis (a std::vector): (" << axis[0] << " ," << axis[1] << " ," << axis[2] << ")" << std::endl;
  return os;
}


// override << operator to allows us to print a Transform object to terminal
inline std::ostream& operator<<(std::ostream& os, const Pinocchio::Transform<double> &obj) {
  std::cout << "Rotation Quaternion:\n " << obj.getRot() << std::endl; // Print rotation quaternion

  double scale = obj.getScale(); // Print scale std::vector
  std::cout << "Scale: " << scale << std::endl;

  Vector3 trans = obj.getTrans();  // Print translation std::vector
  std::cout << "Translation Vector: (" << trans[0] << " ," << trans[1] << " ," << trans[2] << ")" << std::endl;
  return os;
}

template <class charT, class traits, class Real>
std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& os, const Pinocchio::Matrix3<Real> &m)
{
  os << "[[" << m[0] << "," << m[1] << "," << m[2] << "]";
  os << "[" << m[3] << "," << m[4] << "," << m[5] << "]";
  os << "[" << m[6] << "," << m[7] << "," << m[8] << "]]";
  return os;
}
#endif
