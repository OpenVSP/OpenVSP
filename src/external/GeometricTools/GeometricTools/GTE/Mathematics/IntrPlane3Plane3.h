// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Hyperplane.h>
#include <Mathematics/Line.h>
#include <Mathematics/Vector3.h>
#include <cmath>

namespace gte
{
    template <typename T>
    class TIQuery<T, Plane3<T>, Plane3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(Plane3<T> const& plane0, Plane3<T> const& plane1)
        {
            // If Cross(N0,N1) is zero, then either planes are parallel and
            // separated or the same plane.  In both cases, 'false' is
            // returned.  Otherwise, the planes intersect.  To avoid subtle
            // differences in reporting between Test() and Find(), the same
            // parallel test is used.  Mathematically,
            //   |Cross(N0,N1)|^2 = Dot(N0,N0)*Dot(N1,N1)-Dot(N0,N1)^2
            //                    = 1 - Dot(N0,N1)^2
            // The last equality is true since planes are required to have
            // unit-length normal vectors.  The test |Cross(N0,N1)| = 0 is the
            // same as |Dot(N0,N1)| = 1.

            Result result{};
            T dot = Dot(plane0.normal, plane1.normal);
            if (std::fabs(dot) < (T)1)
            {
                result.intersect = true;
                return result;
            }

            // The planes are parallel.  Check whether they are coplanar.
            T cDiff;
            if (dot >= (T)0)
            {
                // Normals are in same direction, need to look at c0-c1.
                cDiff = plane0.constant - plane1.constant;
            }
            else
            {
                // Normals are in opposite directions, need to look at c0+c1.
                cDiff = plane0.constant + plane1.constant;
            }

            result.intersect = (std::fabs(cDiff) == (T)0);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Plane3<T>, Plane3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                isLine(false),
                line(Vector3<T>::Zero(), Vector3<T>::Zero()),
                plane(Vector3<T>::Zero(), (T)0)
            {
            }

            bool intersect;

            // If 'intersect' is true, the intersection is either a line or
            // the planes are the same.  When a line, 'line' is valid.  When
            // the same plane, 'plane' is set to one of the planes.
            bool isLine;
            Line3<T> line;
            Plane3<T> plane;
        };

        Result operator()(Plane3<T> const& plane0, Plane3<T> const& plane1)
        {
            // If N0 and N1 are parallel, either the planes are parallel and
            // separated or the same plane.  In both cases, 'false' is
            // returned.  Otherwise, the intersection line is
            //   L(t) = t*Cross(N0,N1)/|Cross(N0,N1)| + c0*N0 + c1*N1
            // for some coefficients c0 and c1 and for t any real number (the
            // line parameter).  Taking dot products with the normals,
            //   d0 = Dot(N0,L) = c0*Dot(N0,N0) + c1*Dot(N0,N1) = c0 + c1*d
            //   d1 = Dot(N1,L) = c0*Dot(N0,N1) + c1*Dot(N1,N1) = c0*d + c1
            // where d = Dot(N0,N1).  These are two equations in two unknowns.
            // The solution is
            //   c0 = (d0 - d*d1)/det
            //   c1 = (d1 - d*d0)/det
            // where det = 1 - d^2.

            Result result{};

            T dot = Dot(plane0.normal, plane1.normal);
            if (std::fabs(dot) >= (T)1)
            {
                // The planes are parallel.  Check if they are coplanar.
                T cDiff;
                if (dot >= (T)0)
                {
                    // Normals are in same direction, need to look at c0-c1.
                    cDiff = plane0.constant - plane1.constant;
                }
                else
                {
                    // Normals are in opposite directions, need to look at
                    // c0+c1.
                    cDiff = plane0.constant + plane1.constant;
                }

                if (std::fabs(cDiff) == (T)0)
                {
                    // The planes are coplanar.
                    result.intersect = true;
                    result.isLine = false;
                    result.plane = plane0;
                    return result;
                }

                // The planes are parallel but distinct.
                result.intersect = false;
                return result;
            }

            T invDet = (T)1 / ((T)1 - dot * dot);
            T c0 = (plane0.constant - dot * plane1.constant) * invDet;
            T c1 = (plane1.constant - dot * plane0.constant) * invDet;
            result.intersect = true;
            result.isLine = true;
            result.line.origin = c0 * plane0.normal + c1 * plane1.normal;
            result.line.direction = UnitCross(plane0.normal, plane1.normal);
            return result;
        }
    };
}
