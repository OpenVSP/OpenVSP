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

#ifndef VECTOR_H_77813B1A_4284_11E9_A288_10FEED04CD1C
#define VECTOR_H_77813B1A_4284_11E9_A288_10FEED04CD1C

#include <iostream>
#include <functional>
#include <vector>
#include <numeric>
#include "hashutils.h"
#include "mathutils.h"

namespace _VectorPrivate {
  template <int Dim> class VecOp;
}


template <class Real, int Dim>
class Vector {
  public:
    typedef Vector<Real, Dim> Self;
    typedef _VectorPrivate::VecOp<Dim> VO;

    Vector() { VO::assign(Real(), *this); }
    Vector(const Self &other) { VO::assign(other, *this); }
    explicit Vector(const Real &m) { VO::assign(m, *this); }
    Vector(const Real &m1, const Real &m2) { m[0] = m1; m[1] = m2; checkDim<2>(VO()); }
    Vector(const Real &m1, const Real &m2, const Real &m3) { m[0] = m1; m[1] = m2; m[2] = m3; checkDim<3>(VO()); }
    template<class R> Vector(const Vector<R, Dim> &other) { VO::assign(other, *this); }

    Real &operator[](int n) { return m[n]; }
    const Real &operator[](int n) const { return m[n]; }

    // Basic recursive functions
    template<class F> Vector<typename F::result_type, Dim> apply(const F &func) const
      { return VO::apply(func, *this); }

    template<class F> Vector<typename F::result_type, Dim> apply(const F &func, const Self &other) const
      { return VO::apply(func, *this, other); }

    template<class Op, class Accum>
      typename Accum::result_type accumulate(const Op &op, const Accum &accum) const
      { return VO::accumulate(op, accum, *this); }

    template<class Op, class Accum>
      typename Accum::result_type accumulate(const Op &op, const Accum &accum, const Self &other) const
      { return VO::accumulate(op, accum, *this, other); }

    // Operators
    Real operator*(const Self &other) const { return accumulate(std::multiplies<Real>(), std::plus<Real>(), other); }
    Self operator+(const Self &other) const { return apply(std::plus<Real>(), other); }
    Self operator-(const Self &other) const { return apply(std::minus<Real>(), other); }
    Self operator*(const Real &scalar) const { return apply(bind2nd(std::multiplies<Real>(), scalar)); }
    Self operator/(const Real &scalar) const { return apply(bind2nd(std::divides<Real>(), scalar)); }
    Self operator-() const { return apply(std::negate<Real>()); }
    bool operator==(const Self &other) const { return accumulate(std::equal_to<Real>(), std::logical_and<Real>(), other); }

    #define OPAS(op, typ) Self &operator op##=(const typ &x) { (*this) = (*this) op x; return *this; }
      OPAS(+, Self)
      OPAS(-, Self)
      OPAS(*, Real)
      OPAS(/, Real)
    #undef OPAS

    Real lengthsq() const { return (*this) * (*this); }
    Real length() const { return sqrt(lengthsq()); }

    Self normalize() const { return (*this) / length(); }

    int size() const { return Dim; }

  private:
    template<class R, int D> friend class Vector;
    template<int WantedDim> void checkDim(const _VectorPrivate::VecOp<WantedDim> &) const {}

    Real m[Dim];
};

template <class Real>
class Vector<Real, -1> {
  public:
    typedef Vector<Real, -1> Self;

    Vector() { }
    Vector(const Self &other) : m(other.m) { }
    Vector(const std::vector<Real> &inM) : m(inM) { }
    explicit Vector(const Real &inM) { m.push_back(inM); }

    Real &operator[](int n) { if((int)m.size() <= n) m.resize(n + 1); return m[n]; }
    const Real &operator[](int n) const { if((int)m.size() <= n) const_cast<Vector<Real, -1> *>(this)->m.resize(n + 1); return m[n]; }

    //basic recursive functions
    template<class F> Vector<typename F::result_type, -1> apply(const F &func) const {
      std::vector<typename F::result_type> out(m.size());
      transform(m.begin(), m.end(), out.begin(), func);
      return Vector<typename F::result_type, -1>(out);
    }

    template<class F> Vector<typename F::result_type, -1> apply(const F &func, const Self &other) const {
      std::vector<typename F::result_type> out(std::max(m.size(), other.m.size()));
      if(m.size() == other.m.size()) {
        transform(m.begin(), m.end(), other.m.begin(), out.begin(), func);
      } else if(m.size() < other.m.size()) {
        transform(m.begin(), m.end(), other.m.begin(), out.begin(), func);
        for (int i = m.size(); i < (int)other.m.size(); ++i) out[i] = func(Real(), other.m[i]);
      } else {
        transform(m.begin(), m.begin() + (other.m.end() - other.m.begin()), other.m.begin(), out.begin(), func);
        for (int i = other.m.size(); i < (int)m.size(); ++i) out[i] = func(m[i], Real());
      }
      return Vector<typename F::result_type, -1>(out);
    }

    template<class Op, class Accum>
      typename Accum::result_type accumulate(const Op &op, const Accum &accum) const
    {
      if(m.empty())
        return typename Accum::result_type();
      typename Accum::result_type out = op(m[0]);
      for (int i = 1; i < (int)m.size(); ++i) { out = accum(out, op(m[i])); }
      return out;
    }

    template<class Op, class Accum>
      typename Accum::result_type accumulate(const Op &op, const Accum &accum, const Self &other) const
    {
      typename Accum::result_type out;
      if(m.empty() || other.m.empty()) {
        if(m.empty() && other.m.empty()) return typename Accum::result_type();
        if(m.empty()) out = op(Real(), other.m[0]);
        else out = op(m[0], Real());
      } else {
        out = op(m[0], other.m[0]);
      }
      if(m.size() == other.m.size()) {
        for (int i = 1; i < (int)m.size(); ++i) { out = accum(out, op(m[i], other.m[i])); }
      } else if (m.size() < other.m.size()) {
        for (int i = 1; i < (int)m.size(); ++i) { out = accum(out, op(m[i], other.m[i])); }
        for (int i = m.size(); i < (int)other.m.size(); ++i) { out = accum(out, op(Real(), other.m[i])); }
      } else {
        for (int i = 1; i < (int)other.m.size(); ++i) { out = accum(out, op(m[i], other.m[i])); }
        for (int i = other.m.size(); i < (int)m.size(); ++i) { out = accum(out, op(m[i], Real())); }
      }
      return out;
    }

    //operators
    Real operator*(const Self &other) const { return accumulate(std::multiplies<Real>(), std::plus<Real>(), other); }
    Self operator+(const Self &other) const { return apply(std::plus<Real>(), other); }
    Self operator-(const Self &other) const { return apply(std::minus<Real>(), other); }
    Self operator*(const Real &scalar) const { return apply(bind2nd(std::multiplies<Real>(), scalar)); }
    Self operator/(const Real &scalar) const { return apply(bind2nd(std::divides<Real>(), scalar)); }
    Self operator-() const { return apply(std::negate<Real>()); }

    #define OPAS(op, typ) Self &operator op##=(const typ &x) { (*this) = (*this) op x; return *this; }
      OPAS(+, Self)
      OPAS(-, Self)
      OPAS(*, Real)
      OPAS(/, Real)
    #undef OPAS

    Real lengthsq() const { return (*this) * (*this); }
    Real length() const { return sqrt(lengthsq()); }

    Self normalize() const { return (*this) / length(); }

    int size() const { return m.size(); }

  private:
    std::vector<Real> m;
};

template <class Real, int Dim>
Vector<Real, Dim> operator*(const Real &scalar, const Vector<Real, Dim> &vec)
{
  //multiplication commutes around here
  return vec * scalar;
}


//cross product
template <class Real>
Vector<Real, 3> operator%(const Vector<Real, 3> &v1, const Vector<Real, 3> &v2)
{
  return Vector<Real, 3>(v1[1] * v2[2] - v1[2] * v2[1],
    v1[2] * v2[0] - v1[0] * v2[2],
    v1[0] * v2[1] - v1[1] * v2[0]);
}


typedef Vector<double, 3> Vector3;
typedef Vector<double, 2> Vector2;

template <class charT, class traits, class Real, int Dim>
std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& os, const Vector<Real, Dim> &v)
{
  os << "[";
  int ms = Dim;
  if(ms == -1)
    ms = v.size();
  for (int i = 0; i < ms; ++i) {
    os << v[i];
    if(i < ms - 1) {
      os << ", ";
    }
  }
  os << "]";
  return os;
}


namespace _VectorPrivate
{
  #define VRD Vector<R, D>
  #define VRD1 Vector<R1, D>
  template <int Dim>
    class VecOp
  {
    public:
      static const int last = Dim - 1;
      typedef VecOp<Dim - 1> Next;
      template<int D> friend class VecOp;
      template<class R, int D> friend class Vector;

      template<class R, class R1, int D>
        static void assign(const VRD1 &from, VRD &to) { to[last] = from[last]; Next::assign(from, to); }

      template<class R, class R1, int D>
        static void assign(const R1 &from, VRD &to) { to[last] = from; Next::assign(from, to); }

      template<class R, int D, class F>
        static Vector<typename F::result_type, D> apply(const F &func, const VRD &v)
        { Vector<typename F::result_type, D> out; _apply(func, v, out); return out; }

      template<class R, int D, class F>
        static Vector<typename F::result_type, D> apply(const F &func, const VRD &v, const VRD &other)
        { Vector<typename F::result_type, D> out; _apply(func, v, other, out); return out; }

      template<class R, int D, class Op, class Accum>
        static typename Accum::result_type accumulate(const Op &op, const Accum &accum, const VRD &v)
        { return accum(op(v[last]), Next::accumulate(op, accum, v)); }

      template<class R, int D, class Op, class Accum>
        static typename Accum::result_type accumulate(const Op &op, const Accum &accum, const VRD &v, const VRD &other)
        { return accum(op(v[last], other[last]), Next::accumulate(op, accum, v, other)); }

      template<class R, int D, class F>
        static void _apply(const F &func, const VRD &v, Vector<typename F::result_type, D> &out)
        { out[last] = func(v[last]); Next::_apply(func, v, out); }

      template<class R, int D, class F>
        static void _apply(const F &func, const VRD &v, const VRD &other, Vector<typename F::result_type, D> &out)
        { out[last] = func(v[last], other[last]); Next::_apply(func, v, other, out); }
  };

  template <>
    class VecOp<1>
  {
    public:
      template<int D> friend class VecOp;

      template<class R, class R1, int D> static void assign(const VRD1 &from, VRD &to) { to[0] = from[0]; }

      template<class R, class R1, int D> static void assign(const R1 &from, VRD &to) { to[0] = from; }

      template<class R, int D, class F>
        static Vector<typename F::result_type, D> apply(const F &func, const VRD &v)
        { Vector<typename F::result_type, D> out; _apply(func, v, out); return out; }

      template<class R, int D, class F>
        static Vector<typename F::result_type, D> apply(const F &func, const VRD &v, const VRD &other)
        { Vector<typename F::result_type, D> out; _apply(func, v, other, out); return out; }

      template<class R, int D, class Op, class Accum>
        static typename Accum::result_type accumulate(const Op &op, const Accum &, const VRD &v)
        { return op(v[0]); }

      template<class R, int D, class Op, class Accum>
        static typename Accum::result_type accumulate(const Op &op, const Accum &, const VRD &v, const VRD &other)
        { return op(v[0], other[0]); }

      template<class R, int D, class F>
        static void _apply(const F &func, const VRD &v, Vector<typename F::result_type, D> &out)
        { out[0] = func(v[0]); }

      template<class R, int D, class F>
        static void _apply(const F &func, const VRD &v, const VRD &other, Vector<typename F::result_type, D> &out)
        { out[0] = func(v[0], other[0]); }
  };
}


//BitComparator is a utility class that helps with rectangle and dtree indices
template<int Dim> class BitComparator
{
  public:
    static const int last = Dim - 1;
    typedef BitComparator<Dim - 1> Next;

    template<class R, int D> static unsigned int less(const VRD &v1, const VRD &v2)
      { return ((v1[last] < v2[last]) ? (1 << last) : 0) + Next::std::less(v1, v2); }

    template<class R, int D> static void assignCorner(int idx, const VRD &v1, const VRD &v2, VRD &out)
      { out[last] = (idx & (1 << last)) ? v1[last] : v2[last]; Next::assignCorner(idx, v1, v2, out); }
};

template<> class BitComparator<1>
{
  public:
    template<class R, int D> static unsigned int less(const VRD &v1, const VRD &v2)
      { return (v1[0] < v2[0]) ? 1 : 0; }

    template<class R, int D> static void assignCorner(int idx, const VRD &v1, const VRD &v2, VRD &out)
      { out[0] = (idx & 1) ? v1[0] : v2[0];}
};
#undef VRD
#undef VRD1

#endif // VECTOR_H_77813B1A_4284_11E9_A288_10FEED04CD1C
