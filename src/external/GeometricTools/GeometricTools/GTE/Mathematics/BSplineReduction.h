// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The BSplineReduction class is an implementation of the algorithm in
// https://www.geometrictools.com/Documentation/BSplineReduction.pdf
// for least-squares fitting of points in the continuous sense by
// an L2 integral norm.  The least-squares fitting implemented in the
// file BSplineCurveFit.h is in the discrete sense by an L2 summation.
// The intended use for this class is to take an open B-spline curve,
// defined by its control points and degree, and reducing the number of
// control points dramatically to obtain another curve that is close to
// the original one.

#include <Mathematics/BandedMatrix.h>
#include <Mathematics/GMatrix.h>
#include <Mathematics/Integration.h>
#include <Mathematics/IntrIntervals.h>
#include <Mathematics/Vector.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace gte
{
    // The input numCtrlPoints must be 2 or larger.  The input degree must
    // satisfy the condition 1 <= degree <= inControls.size()-1.  The degree
    // of the output curve is the same as that of the input curve.  The input
    // fraction must be in [0,1].  If the fraction is 1, the output curve
    // is identical to the input curve.  If the fraction is too small to
    // produce a valid number of control points, outControls.size() is chosen
    // to be degree+1.
    template <int32_t N, typename Real>
    class BSplineReduction
    {
    public:
        BSplineReduction()
            :
            mDegree(0),
            mQuantity{ 0, 0 },
            mNumKnots{ 0, 0 },
            mKnot{},
            mBasis{ 0, 0 },
            mIndex{ 0, 0 }
        {
        }

        void operator()(std::vector<Vector<N, Real>> const& inControls,
            int32_t degree, Real fraction, std::vector<Vector<N, Real>>& outControls)
        {
            int32_t numInControls = static_cast<int32_t>(inControls.size());
            LogAssert(numInControls >= 2 && 1 <= degree && degree < numInControls, "Invalid input.");

            // Clamp the number of control points to [degree+1,quantity-1].
            int32_t numOutControls = static_cast<int32_t>(fraction * numInControls);
            if (numOutControls >= numInControls)
            {
                outControls = inControls;
                return;
            }
            if (numOutControls < degree + 1)
            {
                numOutControls = degree + 1;
            }

            // Allocate output control points.
            outControls.resize(numOutControls);

            // Set up basis function parameters.  Function 0 corresponds to
            // the output curve.  Function 1 corresponds to the input curve.
            mDegree = degree;
            mQuantity[0] = numOutControls;
            mQuantity[1] = numInControls;

            for (int32_t j = 0; j <= 1; ++j)
            {
                mNumKnots[j] = mQuantity[j] + mDegree + 1;
                mKnot[j].resize(mNumKnots[j]);

                int32_t i;
                for (i = 0; i <= mDegree; ++i)
                {
                    mKnot[j][i] = (Real)0;
                }

                Real denom = static_cast<Real>(mQuantity[j]) - static_cast<Real>(mDegree);
                Real factor = (Real)1 / denom;
                for (/**/; i < mQuantity[j]; ++i)
                {
                    mKnot[j][i] = (static_cast<size_t>(i) - mDegree) * factor;
                }

                for (/**/; i < mNumKnots[j]; ++i)
                {
                    mKnot[j][i] = (Real)1;
                }
            }

            // Construct matrix A (depends only on the output basis function).
            Real value, tmin, tmax;
            int32_t i0, i1;

            mBasis[0] = 0;
            mBasis[1] = 0;

            std::function<Real(Real)> integrand = [this](Real t)
            {
                Real value0 = F(mBasis[0], mIndex[0], mDegree, t);
                Real value1 = F(mBasis[1], mIndex[1], mDegree, t);
                Real result = value0 * value1;
                return result;
            };

            BandedMatrix<Real> A(mQuantity[0], mDegree, mDegree);
            for (i0 = 0; i0 < mQuantity[0]; ++i0)
            {
                mIndex[0] = i0;
                tmax = MaxSupport(0, i0);

                for (i1 = i0; i1 <= i0 + mDegree && i1 < mQuantity[0]; ++i1)
                {
                    mIndex[1] = i1;
                    tmin = MinSupport(0, i1);

                    value = Integration<Real>::Romberg(8, tmin, tmax, integrand);
                    A(i0, i1) = value;
                    A(i1, i0) = value;
                }
            }

            // Construct A^{-1}.  TODO: This is inefficient.  Use an iterative
            // scheme to invert A?
            GMatrix<Real> invA(mQuantity[0], mQuantity[0]);
            bool invertible = A.template ComputeInverse<true>(&invA[0]);
            LogAssert(invertible, "Failed to invert matrix.");

            // Construct B (depends on both input and output basis functions).
            mBasis[1] = 1;
            GMatrix<Real> B(mQuantity[0], mQuantity[1]);
            FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>> query;
            for (i0 = 0; i0 < mQuantity[0]; ++i0)
            {
                mIndex[0] = i0;
                Real tmin0 = MinSupport(0, i0);
                Real tmax0 = MaxSupport(0, i0);

                for (i1 = 0; i1 < mQuantity[1]; ++i1)
                {
                    mIndex[1] = i1;
                    Real tmin1 = MinSupport(1, i1);
                    Real tmax1 = MaxSupport(1, i1);

                    std::array<Real, 2> interval0 = { tmin0, tmax0 };
                    std::array<Real, 2> interval1 = { tmin1, tmax1 };
                    auto result = query(interval0, interval1);
                    if (result.numIntersections == 2)
                    {
                        value = Integration<Real>::Romberg(8, result.overlap[0],
                            result.overlap[1], integrand);

                        B(i0, i1) = value;
                    }
                    else
                    {
                        B(i0, i1) = (Real)0;
                    }
                }
            }

            // Construct A^{-1}*B.
            GMatrix<Real> prod = invA * B;

            // Construct the control points for the least-squares curve.
            std::fill(outControls.begin(), outControls.end(), Vector<N, Real>::Zero());
            for (i0 = 0; i0 < mQuantity[0]; ++i0)
            {
                for (i1 = 0; i1 < mQuantity[1]; ++i1)
                {
                    outControls[i0] += inControls[i1] * prod(i0, i1);
                }
            }
        }

    private:
        inline Real MinSupport(int32_t basis, int32_t i) const
        {
            return mKnot[basis][i];
        }

        inline Real MaxSupport(int32_t basis, int32_t i) const
        {
            return mKnot[basis][static_cast<size_t>(i) + 1 + static_cast<size_t>(mDegree)];
        }

        Real F(int32_t basis, int32_t i, int32_t j, Real t)
        {
            if (j > 0)
            {
                Real result = (Real)0;

                Real denom = mKnot[basis][static_cast<size_t>(i) + static_cast<size_t>(j)] - mKnot[basis][i];
                if (denom > (Real)0)
                {
                    result += (t - mKnot[basis][i]) *
                        F(basis, i, j - 1, t) / denom;
                }

                denom = mKnot[basis][static_cast<size_t>(i) + static_cast<size_t>(j) + 1] - mKnot[basis][static_cast<size_t>(i) + 1];
                if (denom > (Real)0)
                {
                    result += (mKnot[basis][static_cast<size_t>(i) + static_cast<size_t>(j) + 1] - t) *
                        F(basis, i + 1, j - 1, t) / denom;
                }

                return result;
            }

            if (mKnot[basis][i] <= t && t < mKnot[basis][static_cast<size_t>(i) + 1])
            {
                return (Real)1;
            }
            else
            {
                return (Real)0;
            }
        }

        int32_t mDegree;
        std::array<int32_t, 2> mQuantity;
        std::array<int32_t, 2> mNumKnots;  // N+D+2
        std::array<std::vector<Real>, 2> mKnot;

        // For the integration-based least-squares fitting.
        std::array<int32_t, 2> mBasis, mIndex;
    };
}
