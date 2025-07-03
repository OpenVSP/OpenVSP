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

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine3AlignedBox3.h>
#include <Mathematics/Segment.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Segment3<T>, AlignedBox3<T>>
        :
        public TIQuery<T, Line3<T>, AlignedBox3<T>>
    {
    public:
        struct Result
            :
            public TIQuery<T, Line3<T>, AlignedBox3<T>>::Result
        {
            Result()
                :
                TIQuery<T, Line3<T>, AlignedBox3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Segment3<T> const& segment, AlignedBox3<T> const& box)
        {
            // Get the centered form of the aligned box. The axes are
            // implicitly axis[d] = Vector3<T>::Unit(d).
            Vector3<T> boxCenter{}, boxExtent{};
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the segment to a centered form in the aligned-box
            // coordinate system.
            Vector3<T> transformedP0 = segment.p[0] - boxCenter;
            Vector3<T> transformedP1 = segment.p[1] - boxCenter;
            Segment3<T> transformedSegment(transformedP0, transformedP1);
            Vector3<T> segOrigin{}, segDirection{};
            T segExtent{};
            transformedSegment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result{};
            DoQuery(segOrigin, segDirection, segExtent, boxExtent, result);
            return result;
        }

    protected:
        void DoQuery(Vector3<T> const& segOrigin, Vector3<T> const& segDirection,
            T segExtent, Vector3<T> const& boxExtent, Result& result)
        {
            for (int32_t i = 0; i < 3; ++i)
            {
                if (std::fabs(segOrigin[i]) > boxExtent[i] + segExtent * std::fabs(segDirection[i]))
                {
                    result.intersect = false;
                    return;
                }
            }

            TIQuery<T, Line3<T>, AlignedBox3<T>>::DoQuery(
                segOrigin, segDirection, boxExtent, result);
        }
    };

    template <typename T>
    class FIQuery<T, Segment3<T>, AlignedBox3<T>>
        :
        public FIQuery<T, Line3<T>, AlignedBox3<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Line3<T>, AlignedBox3<T>>::Result
        {
            Result()
                :
                FIQuery<T, Line3<T>, AlignedBox3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Segment3<T> const& segment, AlignedBox3<T> const& box)
        {
            // Get the centered form of the aligned box. The axes are
            // implicitly axis[d] = Vector3<T>::Unit(d).
            Vector3<T> boxCenter{}, boxExtent{};
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the segment to a centered form in the aligned-box
            // coordinate system.
            Vector3<T> transformedP0 = segment.p[0] - boxCenter;
            Vector3<T> transformedP1 = segment.p[1] - boxCenter;
            Segment3<T> transformedSegment(transformedP0, transformedP1);
            Vector3<T> segOrigin{}, segDirection{};
            T segExtent{};
            transformedSegment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result{};
            DoQuery(segOrigin, segDirection, segExtent, boxExtent, result);
            if (result.intersect)
            {
                // The segment origin is in aligned-box coordinates. Transform
                // it back to the original space.
                segOrigin += boxCenter;
                for (size_t i = 0; i < 2; ++i)
                {
                    result.point[i] = segOrigin + result.parameter[i] * segDirection;
                }
            }
            return result;
        }

    protected:
        // The caller must ensure that on entry, 'output' is default
        // constructed as if there is no intersection. If an intersection is
        // found, the 'output' values will be modified accordingly.
        void DoQuery(Vector3<T> const& segOrigin, Vector3<T> const& segDirection,
            T segExtent, Vector3<T> const& boxExtent, Result& result)
        {
            FIQuery<T, Line3<T>, AlignedBox3<T>>::DoQuery(
                segOrigin, segDirection, boxExtent, result);

            if (result.intersect)
            {
                // The line containing the segment intersects the box; the
                // t-interval is [t0,t1]. The segment intersects the box as
                // long as [t0,t1] overlaps the segment t-interval
                // [-segExtent,+segExtent].
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery{};
                std::array<T, 2> segInterval{ -segExtent, segExtent };
                auto iiResult = iiQuery(result.parameter, segInterval);
                if (iiResult.intersect)
                {
                    result.numIntersections = iiResult.numIntersections;
                    result.parameter = iiResult.overlap;
                }
                else
                {
                    // The line containing the segment does not intersect
                    // the box.
                    result = Result{};
                }
            }
        }
    };
}
