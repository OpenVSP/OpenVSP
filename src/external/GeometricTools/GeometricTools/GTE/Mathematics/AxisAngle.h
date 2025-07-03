// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Axis-angle representation for N = 3 or N = 4. When N = 4, the axis must be
// a vector of the form (x,y,z,0) [affine representation of the 3-tuple
// direction].

#include <Mathematics/Vector.h>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename Real>
    class AxisAngle
    {
    public:
        AxisAngle()
            :
            axis(Vector<N, Real>::Zero()),
            angle((Real)0)
        {
            static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
        }

        AxisAngle(Vector<N, Real> const& inAxis, Real inAngle)
            :
            axis(inAxis),
            angle(inAngle)
        {
            static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
        }

        Vector<N, Real> axis;
        Real angle;
    };
}
