// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Least-squares fit of a plane to (x,y,z) data by using distance measurements
// orthogonal to the proposed plane. The return value is 'true' if and only if
// the fit is unique (always successful, 'true' when a minimum eigenvalue is
// unique). The mParameters value is (P,N) = (origin,normal). The error for
// S = (x0,y0,z0) is |Dot(N,S-P)|.

#include <Mathematics/ApprQuery.h>
#include <Mathematics/SymmetricEigensolver3x3.h>
#include <Mathematics/Vector3.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace gte
{
    template <typename Real>
    class ApprOrthogonalPlane3 : public ApprQuery<Real, Vector3<Real>>
    {
    public:
        // Initialize the model parameters to zero.
        ApprOrthogonalPlane3()
        {
            mParameters.first = Vector3<Real>::Zero();
            mParameters.second = Vector3<Real>::Zero();
        }

        // Basic fitting algorithm. See ApprQuery.h for the various Fit(...)
        // functions that you can call.
        virtual bool FitIndexed(
            size_t numPoints, Vector3<Real> const* points,
            size_t numIndices, int32_t const* indices) override
        {
            if (this->ValidIndices(numPoints, points, numIndices, indices))
            {
                // Compute the mean of the points.
                Vector3<Real> mean = Vector3<Real>::Zero();
                int32_t const* currentIndex = indices;
                for (size_t i = 0; i < numIndices; ++i)
                {
                    mean += points[*currentIndex++];
                }
                Real invSize = (Real)1 / (Real)numIndices;
                mean *= invSize;

                if (std::isfinite(mean[0]) &&
                    std::isfinite(mean[1]) &&
                    std::isfinite(mean[2]))
                {
                    // Compute the covariance matrix of the points.
                    Real covar00 = (Real)0, covar01 = (Real)0, covar02 = (Real)0;
                    Real covar11 = (Real)0, covar12 = (Real)0, covar22 = (Real)0;
                    currentIndex = indices;
                    for (size_t i = 0; i < numIndices; ++i)
                    {
                        Vector3<Real> diff = points[*currentIndex++] - mean;
                        covar00 += diff[0] * diff[0];
                        covar01 += diff[0] * diff[1];
                        covar02 += diff[0] * diff[2];
                        covar11 += diff[1] * diff[1];
                        covar12 += diff[1] * diff[2];
                        covar22 += diff[2] * diff[2];
                    }
                    covar00 *= invSize;
                    covar01 *= invSize;
                    covar02 *= invSize;
                    covar11 *= invSize;
                    covar12 *= invSize;
                    covar22 *= invSize;

                    // Solve the eigensystem.
                    SymmetricEigensolver3x3<Real> es;
                    std::array<Real, 3> eval;
                    std::array<std::array<Real, 3>, 3> evec;
                    es(covar00, covar01, covar02, covar11, covar12, covar22,
                        false, +1, eval, evec);

                    // The plane normal is the eigenvector in the direction of
                    // smallest variance of the points.
                    mParameters.first = mean;
                    mParameters.second = evec[0];

                    // The fitted plane is unique when the minimum eigenvalue
                    // has multiplicity 1.
                    return eval[0] < eval[1];
                }
            }

            mParameters.first = Vector3<Real>::Zero();
            mParameters.second = Vector3<Real>::Zero();
            return false;
        }

        // Get the parameters for the best fit.
        std::pair<Vector3<Real>, Vector3<Real>> const& GetParameters() const
        {
            return mParameters;
        }

        virtual size_t GetMinimumRequired() const override
        {
            return 3;
        }

        virtual Real Error(Vector3<Real> const& point) const override
        {
            return std::fabs(Dot(mParameters.second, point - mParameters.first));
        }

        virtual void CopyParameters(ApprQuery<Real, Vector3<Real>> const* input) override
        {
            auto source = dynamic_cast<ApprOrthogonalPlane3<Real> const*>(input);
            if (source)
            {
                *this = *source;
            }
        }

    private:
        std::pair<Vector3<Real>, Vector3<Real>> mParameters;
    };
}
