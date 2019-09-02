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

#ifndef RECT_H_EEDC461A_462F_11E9_8109_4BDCBFDF28DE
#define RECT_H_EEDC461A_462F_11E9_8109_4BDCBFDF28DE

#include "vector.h"

namespace Pinocchio {

namespace _RectPrivate
{
  template <int Dim> class RectOp;
}

template<class Real, int Dim>
class Rect
{
  public:
    typedef Vector<Real, Dim> Vec;
    typedef Rect<Real, Dim> Self;
    typedef _RectPrivate::RectOp<Dim> RO;

    Rect() : empty(true) {}
    Rect(const Vec &vec) : empty(false), lo(vec), hi(vec) {}
    Rect(const Vec &inLo, const Vec &inHi) : lo(inLo), hi(inHi) { markEmpty(); }
    Rect(const Rect &inRect) : empty(inRect.empty), lo(inRect.lo), hi(inRect.hi) {}
    template<class R> Rect(const Rect<R, Dim> &inRect) : empty(inRect.empty), lo(inRect.lo), hi(inRect.hi) {}

    //constructs a Rect given an iterator over points--could be optimized with a min-max
    template<class Iter> Rect(Iter start, const Iter &finish) : empty(false)
    {
      if(start == finish)
      {
        empty = true;
        return;
      }
      lo = hi = *start;
      while(++start != finish)
      {
        lo = lo.apply(minimum<Real>(), *start);
        hi = hi.apply(maximum<Real>(), *start);
      }
    }

    bool isEmpty() const { return empty; }

    Self operator&(const Self &other) const
    {
      if(!empty && !other.empty)
        return Self(lo.apply(maximum<Real>(), other.lo), hi.apply(minimum<Real>(), other.hi));
      return Self();
    }

    Self operator|(const Self &other) const
    {
      if(empty)
        return other;
      if(other.empty)
        return (*this);
      return Self(false, lo.apply(minimum<Real>(), other.lo), hi.apply(maximum<Real>(), other.hi));
    }

    bool operator==(const Self &other) const
    {
      if(empty && other.empty)
        return true;
      return (empty == other.empty) && (lo == other.lo) && (hi == other.hi);
    }

  #define OPAS(op, typ) Self &operator op##=(const typ &x) { (*this) = (*this) op x; return *this; }
    OPAS(&, Self)
      OPAS(|, Self)
    #undef OPAS

      bool contains(const Self &other) const
    {
      return other.empty || !(empty || other.lo.accumulate(std::less<Real>(), std::logical_or<bool>(), lo) ||
        hi.accumulate(std::less<Real>(), std::logical_or<bool>(), other.hi));
    }

    bool contains(const Vec &other) const
    {
      return !(empty || other.accumulate(std::less<Real>(), std::logical_or<bool>(), lo) ||
        hi.accumulate(std::less<Real>(), std::logical_or<bool>(), other));
    }

    const Vec &getLo() const { return lo; }
    const Vec &getHi() const { return hi; }
    Vec getSize() const { return empty ? Vec() : hi - lo; }

    Real getContent() const { return empty ? Real() : (hi - lo).accumulate(ident<Real>(), std::multiplies<Real>()); }
    Real getDiagLength() const { return empty ? Real() : (hi - lo).length(); }
    Vec getCenter() const { return (lo + hi) / Real(2); }

    Real distSqTo(const Vec &other) const { return RO::distSq(*this, other); }
    Real distSqTo(const Self &other) const { return RO::distSq(*this, other); }

    Vec getCorner(int idx) const
    {
      Vec out;
      BitComparator<Dim>::assignCorner(idx, hi, lo, out);
      return out;
    }

  private:
    template<class R, int D> friend class Rect;

    Rect(bool inEmpty, const Vec &inLo, const Vec &inHi) : empty(inEmpty), lo(inLo), hi(inHi) { }
    void markEmpty() { empty = hi.accumulate(std::less<Real>(), std::logical_or<bool>(), lo); }

    bool empty;
    Vec lo, hi;
};

typedef Rect<double, 2> Rect2;
typedef Rect<double, 3> Rect3;

template <class charT, class traits, class Real, int Dim>
std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& os, const Rect<Real, Dim> &r)
{
  if(r.isEmpty())
    os << "Rect()";
  else
    os << "Rect(" << r.getLo() << ", " << r.getHi() << ")";
  return os;
}


namespace _RectPrivate
{
  #define VRD Vector<R, D>
  #define RRD Rect<R, D>
  template <int Dim>
    class RectOp
  {
    private:
      static const int last = Dim - 1;
      typedef RectOp<Dim - 1> Next;
      template<int D> friend class RectOp;
      template<class R, int D> friend class Rect;

    public:
      template<class R, int D>
        static R distSq(const RRD &r, const VRD &v)
      {
        R out = Next::distSq(r, v);
        if(r.getLo()[last] > v[last])
          return out + SQR(r.getLo()[last] - v[last]);
        if(r.getHi()[last] < v[last])
          return out + SQR(v[last] - r.getHi()[last]);
        return out;
      }

      template<class R, int D>
        static R distSq(const RRD &r, const RRD &r2)
      {
        R out = Next::distSq(r, r2);
        if(r.getLo()[last] > r2.getHi()[last])
          return out + SQR(r.getLo()[last] - r2.getHi()[last]);
        if(r.getHi()[last] < r2.getLo()[last])
          return out + SQR(r2.getLo()[last] - r.getHi()[last]);
        return out;
      }

  };

  template <>
    class RectOp<1>
  {
    private:
      template<int D> friend class RectOp;

    public:
      template<class R, int D>
        static R distSq(const RRD &r, const VRD &v)
      {
        if(r.getLo()[0] > v[0])
          return SQR(r.getLo()[0] - v[0]);
        if(r.getHi()[0] < v[0])
          return SQR(v[0] - r.getHi()[0]);
        return R();
      }

      template<class R, int D>
        static R distSq(const RRD &r, const RRD &r2)
      {
        if(r.getLo()[0] > r2.getHi()[0])
          return SQR(r.getLo()[0] - r2.getHi()[0]);
        if(r.getHi()[0] < r2.getLo()[0])
          return SQR(r2.getLo()[0] - r.getHi()[0]);
        return R();
      }

  };

} // namespace _RectPrivate

} // namespace Pinocchio

#endif // RECT_H_EEDC461A_462F_11E9_8109_4BDCBFDF28DE
