// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Queries for intersection of objects with halfspaces.  These are useful for
// containment testing, object culling, and clipping.

#include <Mathematics/TIQuery.h>
#include <Mathematics/Capsule.h>
#include <Mathematics/Halfspace.h>

namespace gte
{
    template <typename T>
    class TIQuery<T, Halfspace3<T>, Capsule3<T>>
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

        Result operator()(Halfspace3<T> const& halfspace, Capsule3<T> const& capsule)
        {
            Result result{};

            // Project the capsule onto the normal line.  The plane of the
            // halfspace occurs at the origin (zero) of the normal line.
            T e0 = Dot(halfspace.normal, capsule.segment.p[0]) - halfspace.constant;
            T e1 = Dot(halfspace.normal, capsule.segment.p[1]) - halfspace.constant;

            // The capsule and halfspace intersect when the projection
            // interval maximum is nonnegative.
            result.intersect = (std::max(e0, e1) + capsule.radius >= (T)0);
            return result;
        }
    };
}
