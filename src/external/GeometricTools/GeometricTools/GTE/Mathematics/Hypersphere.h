// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The hypersphere is represented as |X-C| = R where C is the center and R is
// the radius.  The hypersphere is a circle for dimension 2 or a sphere for
// dimension 3.

#include <Mathematics/Vector.h>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename Real>
    class Hypersphere
    {
    public:
        // Construction and destruction.  The default constructor sets the center
        // to (0,...,0) and the radius to 1.
        Hypersphere()
            :
            radius((Real)1)
        {
            center.MakeZero();
        }

        Hypersphere(Vector<N, Real> const& inCenter, Real inRadius)
            :
            center(inCenter),
            radius(inRadius)
        {
        }

        // Public member access.
        Vector<N, Real> center;
        Real radius;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Hypersphere const& hypersphere) const
        {
            return center == hypersphere.center && radius == hypersphere.radius;
        }

        bool operator!=(Hypersphere const& hypersphere) const
        {
            return !operator==(hypersphere);
        }

        bool operator< (Hypersphere const& hypersphere) const
        {
            if (center < hypersphere.center)
            {
                return true;
            }

            if (center > hypersphere.center)
            {
                return false;
            }

            return radius < hypersphere.radius;
        }

        bool operator<=(Hypersphere const& hypersphere) const
        {
            return !hypersphere.operator<(*this);
        }

        bool operator> (Hypersphere const& hypersphere) const
        {
            return hypersphere.operator<(*this);
        }

        bool operator>=(Hypersphere const& hypersphere) const
        {
            return !operator<(hypersphere);
        }
    };

    // Template aliases for convenience.
    template <typename Real>
    using Circle2 = Hypersphere<2, Real>;

    template <typename Real>
    using Sphere3 = Hypersphere<3, Real>;
}
