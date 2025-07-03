// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// An algorithm for the test-intersection query between a triangle and a
// finite cylinder is described in
// https://www.geometrictools.com/Documentation/IntersectionTriangleCylinder.pdf
// The code here is an implementation of that algorithm. The comments include
// references to Figure 1 of the PDF.

#include <Mathematics/TIQuery.h>
#include <Mathematics/Triangle.h>
#include <Mathematics/Cylinder3.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/Vector3.h>
#include <array>
#include <cstddef>

namespace gte
{
    template <typename T>
    class TIQuery<T, Triangle3<T>, Cylinder3<T>>
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

        Result operator()(Triangle3<T> const& triangle, Cylinder3<T> const& cylinder)
        {
            Result result{};

            // Get a right-handed orthonormal basis from the cylinder axis
            // direction.
            std::array<Vector3<T>, 3> basis{};  // {U2,U0,U1}
            basis[0] = cylinder.axis.direction;
            ComputeOrthogonalComplement(1, basis.data());

            // Compute coordinates of the triangle vertices in the coordinate
            // system {C;U0,U1,U2}, where C is the cylinder center and U2 is
            // the cylinder direction. The basis {U0,U1,U2} is orthonormal and
            // right-handed.
            std::array<Vector3<T>, 3> P{};
            for (size_t i = 0; i < 3; ++i)
            {
                Vector3<T> delta = triangle.v[i] - cylinder.axis.origin;
                P[i][0] = Dot(basis[1], delta);  // x[i]
                P[i][1] = Dot(basis[2], delta);  // y[i]
                P[i][2] = Dot(basis[0], delta);  // z[i]
            }

            // Sort the triangle vertices so that z[0] <= z[1] <= z[2].
            size_t j0{}, j1{}, j2{};
            if (P[0][2] < P[1][2])
            {
                if (P[2][2] < P[0][2])
                {
                    j0 = 2;
                    j1 = 0;
                    j2 = 1;
                }
                else if (P[2][2] < P[1][2])
                {
                    j0 = 0;
                    j1 = 2;
                    j2 = 1;
                }
                else
                {
                    j0 = 0;
                    j1 = 1;
                    j2 = 2;
                }
            }
            else
            {
                if (P[2][2] < P[1][2])
                {
                    j0 = 2;
                    j1 = 1;
                    j2 = 0;
                }
                else if (P[2][2] < P[0][2])
                {
                    j0 = 1;
                    j1 = 2;
                    j2 = 0;
                }
                else
                {
                    j0 = 1;
                    j1 = 0;
                    j2 = 2;
                }
            }

            std::array<T, 3> z = { P[j0][2], P[j1][2], P[j2][2] };

            // Maintain the xy-components and z-components separately. The
            // z-components are used for clipping against bottom and top
            // planes of the cylinder. The xy-components are used for
            // disk-containment tests x * x + y * y <= r * r.

            // Attempt an early exit by testing whether the triangle is
            // strictly outside the cylinder slab -h/2 < z < h/2.
            T const hhalf = static_cast<T>(0.5) * cylinder.height;
            if (z[2] < -hhalf)
            {
                // The triangle is strictly below the bottom-disk plane of
                // the cylinder. See case 0a of Figure 1 in the PDF.
                result.intersect = false;
                return result;
            }

            if (z[0] > hhalf)
            {
                // The triangle is strictly above the top-disk plane of the
                // cylinder. See case 0b of Figure 1 in the PDF.
                result.intersect = false;
                return result;
            }

            // Project the triangle vertices onto the xy-plane.
            std::array<Vector2<T>, 3> Q
            {
                Vector2<T>{ P[j0][0], P[j0][1] },
                Vector2<T>{ P[j1][0], P[j1][1] },
                Vector2<T>{ P[j2][0], P[j2][1] }
            };

            // Attempt an early exit when the triangle does not have to be
            // clipped.
            T const& radius = cylinder.radius;
            if (-hhalf <= z[0] && z[2] <= hhalf)
            {
                // The triangle is between the planes of the top-disk and
                // the bottom disk of the cylinder. Determine whether the
                // projection of the triangle onto a plane perpendicular
                // to the cylinder axis overlaps the disk of projection
                // of the cylinder onto the same plane. See case 3a of
                // Figure 1 of the PDF.
                result.intersect = DiskOverlapsPolygon(3, Q.data(), radius);
                return result;
            }

            // Clip against |z| <= h/2. At this point we know that z2 >= -h/2
            // and z0 <= h/2 with either z0 < -h/2 or z2 > h/2 or both. The
            // test-intersection query involves testing for overlap between
            // the xy-projection of the clipped triangle and the xy-projection
            // of the cylinder (a disk in the projection plane). The code
            // below computes the vertices of the projection of the clipped
            // triangle. The t-values of the triangle-edge parameterizations
            // satisfy 0 <= t <= 1.
            if (z[0] < -hhalf)
            {
                if (z[2] > hhalf)
                {
                    if (z[1] >= hhalf)
                    {
                        // Cases 4a and 4b of Figure 1 in the PDF.
                        //
                        // The edge <V0,V1> is parameterized by V0+t*(V1-V0).
                        // On the bottom of the slab,
                        //   -h/2 = z0 + t * (z1 - z0)
                        //   t = (-h/2 - z0) / (z1 - z0) = numerNeg0 / denom10
                        // and on the tob of the slab,
                        //   +h/2 = z0 + t * (z1 - z0)
                        //   t = (+h/2 - z0) / (z1 - z0) = numerPos0 / denom10
                        //
                        // The edge <V0,V2> is parameterized by V0+t*(V2-V0).
                        // On the bottom of the slab,
                        //   -h/2 = z0 + t * (z2 - z0)
                        //   t = (-h/2 - z0) / (z2 - z0) = numerNeg0 / denom20
                        // and on the tob of the slab,
                        //   +h/2 = z0 + t * (z2 - z0)
                        //   t = (+h/2 - z0) / (z2 - z0) = numerPos0 / denom20
                        T numerNeg0 = -hhalf - z[0];
                        T numerPos0 = +hhalf - z[0];
                        T denom10 = z[1] - z[0];
                        T denom20 = z[2] - z[0];
                        Vector2<T> dir20 = (Q[2] - Q[0]) / denom20;
                        Vector2<T> dir10 = (Q[1] - Q[0]) / denom10;
                        std::array<Vector2<T>, 4> polygon
                        {
                            Q[0] + numerNeg0 * dir20,
                            Q[0] + numerNeg0 * dir10,
                            Q[0] + numerPos0 * dir10,
                            Q[0] + numerPos0 * dir20
                        };
                        result.intersect = DiskOverlapsPolygon(4, polygon.data(), radius);
                    }
                    else if (z[1] <= -hhalf)
                    {
                        // Cases 4c and 4d of Figure 1 of the PDF.
                        //
                        // The edge <V2,V0> is parameterized by V0+t*(V2-V0).
                        // On the bottom of the slab,
                        //   -h/2 = z2 + t * (z0 - z2)
                        //   t = (-h/2 - z2) / (z0 - z2) = numerNeg2 / denom02
                        // and on the tob of the slab,
                        //   +h/2 = z2 + t * (z0 - z2)
                        //   t = (+h/2 - z2) / (z0 - z2) = numerPos2 / denom02
                        //
                        // The edge <V2,V1> is parameterized by V2+t*(V1-V2).
                        // On the bottom of the slab,
                        //   -h/2 = z2 + t * (z1 - z2)
                        //   t = (-h/2 - z2) / (z1 - z2) = numerNeg2 / denom12
                        // and on the top of the slab,
                        //   +h/2 = z2 + t * (z1 - z2)
                        //   t = (+h/2 - z2) / (z1 - z2) = numerPos2 / denom12
                        T numerNeg2 = -hhalf - z[2];
                        T numerPos2 = +hhalf - z[2];
                        T denom02 = z[0] - z[2];
                        T denom12 = z[1] - z[2];
                        Vector2<T> dir02 = (Q[0] - Q[2]) / denom02;
                        Vector2<T> dir12 = (Q[1] - Q[2]) / denom12;
                        std::array<Vector2<T>, 4> polygon
                        {
                            Q[2] + numerNeg2 * dir02,
                            Q[2] + numerNeg2 * dir12,
                            Q[2] + numerPos2 * dir12,
                            Q[2] + numerPos2 * dir02
                        };
                        result.intersect = DiskOverlapsPolygon(4, polygon.data(), radius);
                    }
                    else  // -hhalf < z[1] < hhalf
                    {
                        // Case 5 of Figure 1 of the PDF.
                        //
                        // The edge <V0,V2> is parameterized by V0+t*(V2-V0).
                        // On the bottom of the slab,
                        //   -h/2 = z0 + t * (z2 - z0)
                        //   t = (-h/2 - z0) / (z2 - z0) = numerNeg0 / denom20
                        // and on the tob of the slab,
                        //   +h/2 = z0 + t * (z2 - z0)
                        //   t = (+h/2 - z0) / (z2 - z0) = numerPos0 / denom20
                        //
                        // The edge <V1,V0> is parameterized by V1+t*(V0-V1).
                        // On the bottom of the slab,
                        //   -h/2 = z1 + t * (z0 - z1)
                        //   t = (-h/2 - z1) / (z0 - z1) = numerNeg1 / denom01
                        //
                        // The edge <V1,V2> is parameterized by V1+t*(V2-V1).
                        // On the top of the slab,
                        //   +h/2 = z1 + t * (z2 - z1)
                        //   t = (+h/2 - z1) / (z2 - z1) = numerPos1 / denom21
                        T numerNeg0 = -hhalf - z[0];
                        T numerPos0 = +hhalf - z[0];
                        T numerNeg1 = -hhalf - z[1];
                        T numerPos1 = +hhalf - z[1];
                        T denom20 = z[2] - z[0];
                        T denom01 = z[0] - z[1];
                        T denom21 = z[2] - z[1];
                        Vector2<T> dir20 = (Q[2] - Q[0]) / denom20;
                        Vector2<T> dir01 = (Q[0] - Q[1]) / denom01;
                        Vector2<T> dir21 = (Q[2] - Q[1]) / denom21;
                        std::array<Vector2<T>, 5> polygon
                        {
                            Q[0] + numerNeg0 * dir20,
                            Q[1] + numerNeg1 * dir01,
                            Q[1],
                            Q[1] + numerPos1 * dir21,
                            Q[0] + numerPos0 * dir20
                        };
                        result.intersect = DiskOverlapsPolygon(5, polygon.data(), radius);
                    }
                }
                else if (z[2] > -hhalf)
                {
                    if (z[1] <= -hhalf)
                    {
                        // Cases 3b and 3c of Figure 1 of the PDF.
                        //
                        // The edge <V2,V0> is parameterized by V2+t*(V0-V2).
                        // On the bottom of the slab,
                        //   -h/2 = z2 + t * (z0 - z2)
                        //   t = (-h/2 - z2) / (z0 - z2) = numerNeg2 / denom02
                        //
                        // The edge <V2,V1> is parameterized by V2+t*(V1-V2).
                        // On the bottom of the slab,
                        //   -h/2 = z2 + t * (z1 - z2)
                        //   t = (-h/2 - z2) / (z1 - z2) = numerNeg2 / denom12
                        T numerNeg2 = -hhalf - z[2];
                        T denom02 = z[0] - z[2];
                        T denom12 = z[1] - z[2];
                        Vector2<T> dir02 = (Q[0] - Q[2]) / denom02;
                        Vector2<T> dir12 = (Q[1] - Q[2]) / denom12;
                        std::array<Vector2<T>, 3> polygon
                        {
                            Q[2],
                            Q[2] + numerNeg2 * dir02,
                            Q[2] + numerNeg2 * dir12
                        };
                        result.intersect = DiskOverlapsPolygon(3, polygon.data(), radius);
                    }
                    else // z[1] > -hhalf
                    {
                        // Case 4e of Figure 1 of the PDF.
                        //
                        // The edge <V0,V1> is parameterized by V0+t*(V1-V0).
                        // On the bottom of the slab,
                        //   -h/2 = z0 + t * (z1 - z0)
                        //   t = (-h/2 - z0) / (z1 - z0) = numerNeg0 / denom10
                        //
                        // The edge <V0,V2> is parameterized by V0+t*(V2-V0).
                        // On the bottom of the slab,
                        //   -h/2 = z0 + t * (z2 - z0)
                        //   t = (-h/2 - z0) / (z2 - z0) = numerNeg0 / denom20
                        T numerNeg0 = -hhalf - z[0];
                        T denom10 = z[1] - z[0];
                        T denom20 = z[2] - z[0];
                        Vector2<T> dir20 = (Q[2] - Q[0]) / denom20;
                        Vector2<T> dir10 = (Q[1] - Q[0]) / denom10;
                        std::array<Vector2<T>, 4> polygon
                        {
                            Q[0] + numerNeg0 * dir20,
                            Q[0] + numerNeg0 * dir10,
                            Q[1],
                            Q[2]
                        };
                        result.intersect = DiskOverlapsPolygon(4, polygon.data(), radius);
                    }
                }
                else  // z[2] == -hhalf
                {
                    if (z[1] < -hhalf)
                    {
                        // Case 1a of Figure 1 of the PDF.
                        result.intersect = DiskOverlapsPoint(Q[2], radius);
                    }
                    else
                    {
                        // Case 2a of Figure 1 of the PDF.
                        result.intersect = DiskOverlapsSegment(Q[1], Q[2], radius);
                    }
                }
            }
            else if (z[0] < hhalf)
            {
                if (z[1] >= hhalf)
                {
                    // Cases 3d and 3e of Figure 1 of the PDF.
                    //
                    // The edge <V0,V1> is parameterized by V0+t*(V1-V0).
                    // On the top of the slab,
                    //   +h/2 = z0 + t * (z1 - z0)
                    //   t = (+h/2 - z0) / (z1 - z0) = numerPos0 / denom10
                    //
                    // The edge <V0,V2> is parameterized by V0+t*(V2-V0).
                    // On the top of the slab,
                    //   +h/2 = z0 + t * (z2 - z0)
                    //   t = (+h/2 - z0) / (z2 - z0) = numerPos0 / denom20
                    T numerPos0 = +hhalf - z[0];
                    T denom10 = z[1] - z[0];
                    T denom20 = z[2] - z[0];
                    Vector2<T> dir10 = (Q[1] - Q[0]) / denom10;
                    Vector2<T> dir20 = (Q[2] - Q[0]) / denom20;
                    std::array<Vector2<T>, 3> polygon
                    {
                        Q[0],
                        Q[0] + numerPos0 * dir10,
                        Q[0] + numerPos0 * dir20
                    };
                    result.intersect = DiskOverlapsPolygon(3, polygon.data(), radius);
                }
                else // z[1] < hhalf
                {
                    // Case 4f of Figure 1 of the PDF.
                    //
                    // The edge <V2,V0> is parameterized by V2+t*(V0-V2).
                    // On the top of the slab,
                    //   +h/2 = z2 + t * (z0 - z2)
                    //   t = (+h/2 - z2) / (z0 - z2) = numerPos2 / denom02
                    //
                    // The edge <V2,V1> is parameterized by V2+t*(V1-V2).
                    // On the top of the slab,
                    //   +h/2 = z2 + t * (z1 - z2)
                    //   t = (+h/2 - z2) / (z1 - z2) = numerPos2 / denom12
                    T numerPos2 = +hhalf - z[2];
                    T denom02 = z[0] - z[2];
                    T denom12 = z[1] - z[2];
                    Vector2<T> dir02 = (Q[0] - Q[2]) / denom02;
                    Vector2<T> dir12 = (Q[1] - Q[2]) / denom12;
                    std::array<Vector2<T>, 4> polygon
                    {
                        Q[0],
                        Q[1],
                        Q[2] + numerPos2 * dir12,
                        Q[2] + numerPos2 * dir02
                    };
                    result.intersect = DiskOverlapsPolygon(4, polygon.data(), radius);
                }
            }
            else // z[0] == hhalf
            {
                if (z[1] > hhalf)
                {
                    // Case 1b of Figure 1 of the PDF.
                    result.intersect = DiskOverlapsPoint(Q[0], radius);
                }
                else
                {
                    // Case 2b of Figure 1 of the PDF.
                    result.intersect = DiskOverlapsSegment(Q[0], Q[1], radius);
                }
            }

            return result;
        }

    private:
        // Support for the static test query.
        bool DiskOverlapsPoint(Vector2<T> const& Q, T const& radius) const
        {
            return Dot(Q, Q) <= radius * radius;
        }

        bool DiskOverlapsSegment(Vector2<T> const& Q0, Vector2<T> const& Q1,
            T const& radius) const
        {
            T sqrRadius = radius * radius;
            Vector2<T> direction = Q0 - Q1;
            T dot = Dot(Q0, direction);
            if (dot <= static_cast<T>(0))
            {
                return Dot(Q0, Q0) <= sqrRadius;
            }

            T sqrLength = Dot(direction, direction);
            if (dot >= sqrLength)
            {
                return Dot(Q1, Q1) <= sqrRadius;
            }

            dot = DotPerp(direction, Q0);
            return dot * dot <= sqrLength * sqrRadius;
        }

        bool DiskOverlapsPolygon(size_t numVertices, Vector2<T> const* Q,
            T const& radius) const
        {
            // Test whether the polygon contains (0,0).
            T const zero = static_cast<T>(0);
            size_t positive = 0, negative = 0;
            size_t i0, i1;
            for (i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
            {
                T dot = DotPerp(Q[i0], Q[i0] - Q[i1]);
                if (dot > zero)
                {
                    ++positive;
                }
                else if (dot < zero)
                {
                    ++negative;
                }
            }
            if (positive == 0 || negative == 0)
            {
                // The polygon contains (0,0), so the disk and polygon
                // overlap.
                return true;
            }

            // Test whether any edge is overlapped by the polygon.
            for (i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
            {
                if (DiskOverlapsSegment(Q[i0], Q[i1], radius))
                {
                    return true;
                }
            }

            return false;
        }
    };
}
