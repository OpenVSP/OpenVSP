// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/Cone.h>
#include <cstdint>

namespace gte
{
    // Test for containment of a point by a cone.
    template <int32_t N, typename Real>
    bool InContainer(Vector<N, Real> const& point, Cone<N, Real> const& cone)
    {
        Vector<N, Real> diff = point - cone.ray.origin;
        Real h = Dot(cone.ray.direction, diff);
        return cone.HeightInRange(h) && h * h >= cone.cosAngleSqr * Dot(diff, diff);
    }
}
