// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The test-intersection queries use the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection queries use parametric clipping against the six
// faces of the box.  The find-intersection queries use Liang-Barsky
// clipping.  The queries consider the box to be a solid.  The algorithms
// are described in
// https://www.geometrictools.com/Documentation/IntersectionLineBox.pdf

#include <Mathematics/IntrSegment3AlignedBox3.h>
#include <Mathematics/OrientedBox.h>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Segment3<T>, OrientedBox3<T>>
        :
        public TIQuery<T, Segment3<T>, AlignedBox3<T>>
    {
    public:
        struct Result
            :
            public TIQuery<T, Segment3<T>, AlignedBox3<T>>::Result
        {
            Result()
                :
                TIQuery<T, Segment3<T>, AlignedBox3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Segment3<T> const& segment, OrientedBox3<T> const& box)
        {
            // Transform the segment to the oriented-box coordinate system.
            Vector3<T> tmpOrigin{}, tmpDirection{};
            T segExtent{};
            segment.GetCenteredForm(tmpOrigin, tmpDirection, segExtent);
            Vector3<T> diff = tmpOrigin - box.center;
            Vector3<T> segOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1]),
                Dot(diff, box.axis[2])
            };
            Vector3<T> segDirection
            {
                Dot(tmpDirection, box.axis[0]),
                Dot(tmpDirection, box.axis[1]),
                Dot(tmpDirection, box.axis[2])
            };

            Result result{};
            this->DoQuery(segOrigin, segDirection, segExtent, box.extent, result);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Segment3<T>, OrientedBox3<T>>
        :
        public FIQuery<T, Segment3<T>, AlignedBox3<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Segment3<T>, AlignedBox3<T>>::Result
        {
            Result()
                :
                FIQuery<T, Segment3<T>, AlignedBox3<T>>::Result{}
            {
            }

            // No additional relevant information to compute.
        };

        Result operator()(Segment3<T> const& segment, OrientedBox3<T> const& box)
        {
            // Transform the segment to the oriented-box coordinate system.
            Vector3<T> tmpOrigin{}, tmpDirection{};
            T segExtent{};
            segment.GetCenteredForm(tmpOrigin, tmpDirection, segExtent);
            Vector3<T> diff = tmpOrigin - box.center;
            Vector3<T> segOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1]),
                Dot(diff, box.axis[2])
            };
            Vector3<T> segDirection
            {
                Dot(tmpDirection, box.axis[0]),
                Dot(tmpDirection, box.axis[1]),
                Dot(tmpDirection, box.axis[2])
            };

            Result result{};
            this->DoQuery(segOrigin, segDirection, segExtent, box.extent, result);
            if (result.intersect)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    // Compute the intersection point in the oriented-box
                    // coordinate system.
                    Vector3<T> y = segOrigin + result.parameter[i] * segDirection;

                    // Transform the intersection point to the original coordinate
                    // system.
                    result.point[i] = box.center;
                    for (int32_t j = 0; j < 3; ++j)
                    {
                        result.point[i] += y[j] * box.axis[j];
                    }
                }
            }
            return result;
        }
    };
}
