// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a solid triangle and a solid aligned box
// in 3D.
// 
// The triangle has vertices <V[0],V[1],V[2]>. A triangle point is
// X = sum_{i=0}^2 b[i] * V[i], where 0 <= b[i] <= 1 for all i and
// sum_{i=0}^2 b[i] = 1.
// 
// The aligned box has minimum corner A and maximum corner B. A box point is X
// where A <= X <= B; the comparisons are componentwise.
// 
// The closest point on the triangle closest is stored in closest[0] with
// barycentric coordinates (b[0],b[1],b[2). The closest point on the box is
// stored in closest[1]. When there are infinitely many choices for the pair
// of closest points, only one of them is returned.

#include <Mathematics/DistTriangle3CanonicalBox3.h>
#include <Mathematics/DistSegment3CanonicalBox3.h>
#include <Mathematics/AlignedBox.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Triangle3<T>, AlignedBox3<T>>
    {
    public:
        using TBQuery = DCPQuery<T, Triangle3<T>, CanonicalBox3<T>>;
        using Result = typename TBQuery::Result;

        Result operator()(Triangle3<T> const& triangle, AlignedBox3<T> const& box)
        {
            Result result{};

            // Translate the triangle and box so that the box has center at
            // the origin.
            Vector3<T> boxCenter{};
            CanonicalBox3<T> cbox{};
            box.GetCenteredForm(boxCenter, cbox.extent);
            Triangle3<T> xfrmTriangle(
                triangle.v[0] - boxCenter,
                triangle.v[1] - boxCenter,
                triangle.v[2] - boxCenter);

            // The query computes 'result' relative to the box with center
            // at the origin.
            TBQuery tbQuery{};
            result = tbQuery(xfrmTriangle, cbox);

            // Translate the closest points to the original coordinates.
            result.closest[0] += boxCenter;
            result.closest[1] += boxCenter;

            return result;
        }
    };
}
