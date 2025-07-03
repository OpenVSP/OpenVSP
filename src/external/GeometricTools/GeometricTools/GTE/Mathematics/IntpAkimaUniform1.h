// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The Akima interpolation is described in
// https://en.wikipedia.org/wiki/Akima_spline

#include <Mathematics/IntpAkima1.h>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    template <typename Real>
    class IntpAkimaUniform1 : public IntpAkima1<Real>
    {
    public:
        // Construction and destruction.  The interpolator is for uniformly
        // spaced x-values.
        IntpAkimaUniform1(int32_t quantity, Real xMin, Real xSpacing, Real const* F)
            :
            IntpAkima1<Real>(quantity, F),
            mXMin(xMin),
            mXSpacing(xSpacing)
        {
            LogAssert(mXSpacing > (Real)0, "Spacing must be positive.");

            mXMax = mXMin + mXSpacing * static_cast<Real>(static_cast<size_t>(quantity) - 1);

            // Compute slopes.
            Real invDX = (Real)1 / mXSpacing;
            std::vector<Real> slope(static_cast<size_t>(quantity) + 3);
            int32_t i, ip1, ip2;
            for (i = 0, ip1 = 1, ip2 = 2; i < quantity - 1; ++i, ++ip1, ++ip2)
            {
                slope[ip2] = (this->mF[ip1] - this->mF[i]) * invDX;
            }

            slope[1] = (Real)2 * slope[2] - slope[3];
            slope[0] = (Real)2 * slope[1] - slope[2];
            slope[static_cast<size_t>(quantity) + 1] = (Real)2 * slope[quantity] - slope[static_cast<size_t>(quantity) - 1];
            slope[static_cast<size_t>(quantity) + 2] = (Real)2 * slope[static_cast<size_t>(quantity) + 1] - slope[quantity];

            // Construct derivatives.
            std::vector<Real> FDer(quantity);
            for (i = 0; i < quantity; ++i)
            {
                FDer[i] = this->ComputeDerivative(&slope[i]);
            }

            // Construct polynomials.
            Real invDX2 = (Real)1 / (mXSpacing * mXSpacing);
            Real invDX3 = invDX2 / mXSpacing;
            for (i = 0, ip1 = 1; i < quantity - 1; ++i, ++ip1)
            {
                auto& poly = this->mPoly[i];

                Real F0 = F[i];
                Real F1 = F[ip1];
                Real df = F1 - F0;
                Real FDer0 = FDer[i];
                Real FDer1 = FDer[ip1];

                poly[0] = F0;
                poly[1] = FDer0;
                poly[2] = ((Real)3 * df - mXSpacing * (FDer1 + (Real)2 * FDer0)) * invDX2;
                poly[3] = (mXSpacing * (FDer0 + FDer1) - (Real)2 * df) * invDX3;
            }
        }

        virtual ~IntpAkimaUniform1() = default;

        // Member access.
        inline virtual Real GetXMin() const override
        {
            return mXMin;
        }

        inline virtual Real GetXMax() const override
        {
            return mXMax;
        }

        inline Real GetXSpacing() const
        {
            return mXSpacing;
        }

    protected:
        virtual void Lookup(Real x, int32_t& index, Real& dx) const override
        {
            // The caller has ensured that mXMin <= x <= mXMax.
            int32_t indexP1;
            for (index = 0, indexP1 = 1; indexP1 < this->mQuantity; ++index, ++indexP1)
            {
                if (x < mXMin + mXSpacing * (indexP1))
                {
                    dx = x - (mXMin + mXSpacing * index);
                    return;
                }
            }

            --index;
            dx = x - (mXMin + mXSpacing * index);
        }

        Real mXMin, mXMax, mXSpacing;
    };
}
