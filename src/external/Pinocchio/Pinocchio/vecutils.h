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

#ifndef VECUTILS_H_D30BB6CC_4630_11E9_82EF_BBA014A6BD0E
#define VECUTILS_H_D30BB6CC_4630_11E9_82EF_BBA014A6BD0E

#include "vector.h"

namespace Pinocchio {

template<class Real>
void getBasis(const Vector<Real, 3> &n, Vector<Real, 3> &v1, Vector<Real, 3> &v2)
{
  if (n.lengthsq() < Real(1e-16)) {
    v1 = Vector<Real, 3>(1., 0., 0.);
    v2 = Vector<Real, 3>(0., 1., 0.);
    return;
  }

  if (fabs(n[0]) <= fabs(n[1]) && fabs(n[0]) <= fabs(n[2])) {
    v2 = Vector<Real, 3>(1., 0., 0.);
  } else if (fabs(n[1]) <= fabs(n[2])) {
    v2 = Vector<Real, 3>(0., 1., 0.);
  } else {
    v2 = Vector<Real, 3>(0., 0., 1.);
  }

  v1 = (n % v2).normalize(); // First basis vector
  v2 = (n % v1).normalize(); // Second basis vector
}


template<class Real, int Dim>
Real distsqToLine(const Vector<Real, Dim> &v, const Vector<Real, Dim> &l, const Vector<Real, Dim> &dir)
{
  return std::max(Real(), (v - l).lengthsq() - SQR((v - l) * dir) / dir.lengthsq());
}


template<class Real, int Dim>
Vector<Real, Dim> projToLine(const Vector<Real, Dim> &v, const Vector<Real, Dim> &l, const Vector<Real, Dim> &dir)
{
  return l + (((v - l) * dir) / dir.lengthsq()) * dir;
}


template<class Real, int Dim>
Real distsqToSeg(const Vector<Real, Dim> &v, const Vector<Real, Dim> &p1, const Vector<Real, Dim> &p2)
{
  typedef Vector<Real, Dim> Vec;

  Vec dir = p2 - p1;
  Vec difp2 = p2 - v;

  if (difp2 * dir < Real()) {
    return difp2.lengthsq();
  }

  Vec difp1 = v - p1;
  Real dot = difp1 * dir;

  if (dot <= Real()) {
    return difp1.lengthsq();
  }

  return std::max(Real(), difp1.lengthsq() - SQR(dot) / dir.lengthsq());
}


template<class Real, int Dim>
Vector<Real, Dim> projToSeg(const Vector<Real, Dim> &v, const Vector<Real, Dim> &p1, const Vector<Real, Dim> &p2)
{
  typedef Vector<Real, Dim> Vec;

  Vec dir = p2 - p1;

  if ((p2 - v) * dir < Real()) {
    return p2;
  }

  Real dot = (v - p1) * dir;

  if (dot <= Real()) {
    return p1;
  }

  return p1 + (dot / dir.lengthsq()) * dir;
}


//d is distance between centers, r1 and r2 are radii
template<class Real>
Real getCircleIntersectionArea(const Real &d, const Real &r1, const Real &r2)
{
  Real tol(1e-8);

  if (r1 + r2 <= d + tol) {
    return Real();
  }
  if (r1 + d <= r2 + tol) {
    return Real(M_PI) * SQR(r1);
  }
  if (r2 + d <= r1 + tol) {
    return Real(M_PI) * SQR(r2);
  }

  Real sqrdif = SQR(r1) - SQR(r2);
  //d^2 - r1^2 + r2^2
  Real dsqrdif = SQR(d) - sqrdif;
  Real a1 = SQR(r1) * acos((SQR(d) + sqrdif) / (Real(2.) * r1 * d));
  Real a2 = SQR(r2) * acos(dsqrdif / (Real(2.) * r2 * d));
  return a1 + a2 - Real(0.5) * sqrt(SQR(Real(2.) * d * r2) - SQR(dsqrdif));
}


template<class Real>
Vector<Real, 3> projToTri(const Vector<Real, 3> &from, const Vector<Real, 3> &p1, const Vector<Real, 3> &p2, const Vector<Real, 3> &p3)
{
  typedef Vector<Real, 3> Vec;
  Real tolsq(1e-16);

  Vec p2p1 = (p2 - p1);
  Vec p3p1 = (p3 - p1);
  Vec normal = p2p1 % p3p1;

  // Inside s1
  if ((p2p1 % (from - p1)) * normal >= Real()) {
    bool s2 = ((p3 - p2) % (from - p2)) * normal >= Real();
    bool s3 = (p3p1 % (from - p3)) * normal <= Real();

    if (s2 && s3) {
      if (normal.lengthsq() < tolsq) { // Incorrect, but whatever...
        return p1;
      }

      double dot = (from - p3) * normal;
      return from - (dot / normal.lengthsq()) * normal;
    }
    if (!s3 && (s2 || (from - p3) * p3p1 >= Real())) {
      return projToSeg(from, p3, p1);
    }
    return projToSeg(from, p2, p3);
  }

  // Outside s1
  if ((from - p1) * p2p1 < Real()) {
    return projToSeg(from, p3, p1);
  }
  if ((from - p2) * p2p1 > Real()) {
    return projToSeg(from, p2, p3);
  }
  return projToLine(from, p1, p2p1);
}

} // namespace Pinocchio

#endif // VECUTILS_H_D30BB6CC_4630_11E9_82EF_BBA014A6BD0E
