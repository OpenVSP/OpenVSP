// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

// The curve is defined by F(x,y) = 0. In all member functions it is the
// application's responsibility to ensure that (x,y) is a solution to F = 0.
// The class is abstract, so you must derive from it and implement the
// function and derivative evaluations.

#include <GTL/Mathematics/Algebra/Matrix.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T>
    class ImplicitCurve2
    {
    public:
        // Abstract base class.
        virtual ~ImplicitCurve2() = default;

        // Evaluate the implicit function.
        virtual T F(Vector2<T> const& position) const = 0;

        // Evaluate the first-order partial derivatives.
        virtual T FX(Vector2<T> const& position) const = 0;
        virtual T FY(Vector2<T> const& position) const = 0;

        // Evaluate the second-order partial derivatives.
        virtual T FXX(Vector2<T> const& position) const = 0;
        virtual T FXY(Vector2<T> const& position) const = 0;
        virtual T FYY(Vector2<T> const& position) const = 0;

        // Verify the point is on the curve within the tolerance specified
        // by epsilon.
        bool IsOnCurve(Vector2<T> const& position, T const& epsilon) const
        {
            return std::fabs(F(position)) <= epsilon;
        }

        // Compute all first-order derivatives.
        Vector2<T> GetGradient(Vector2<T> const& position) const
        {
            T fx = FX(position);
            T fy = FY(position);
            return Vector2<T>{ fx, fy };
        }

        // Compute all second-order derivatives.
        Matrix2x2<T> GetHessian(Vector2<T> const& position) const
        {
            T fxx = FXX(position);
            T fxy = FXY(position);
            T fyy = FYY(position);
            return Matrix2x2<T>{ {fxx, fxy}, { fxy, fyy } };
        }

        // Compute a coordinate frame. The set {T, N} is a right-handed
        // orthonormal basis.
        void GetFrame(Vector2<T> const& position, Vector2<T>& tangent, Vector2<T>& normal) const
        {
            normal = GetGradient(position);
            ComputeOrthonormalBasis(normal, tangent);
        }

        // Compute the curvature at a point on the curve.
        bool GetCurvature(Vector2<T> const& position, T& curvature) const
        {
            // The curvature is
            // (-Fy^2*Fxx + 2*Fx*Fy*Fxy - Fx^2*Fyy) / (Fx^2+Fy^2)^{3/2}

            // Evaluate the first derivatives.
            T fx = FX(position);
            T fy = FY(position);

            // Evaluate the denominator.
            T fxSqr = fx * fx;
            T fySqr = fy * fy;
            T denom = std::pow(fxSqr + fySqr, C_<T>(3, 2));
            if (denom == C_<T>(0))
            {
                curvature = C_<T>(0);
                return false;
            }

            // Evaluate the second derivatives.
            T fxx = FXX(position);
            T fxy = FXY(position);
            T fyy = FYY(position);

            // Evaluate the numerator.
            T numer = -fySqr * fxx + C_<T>(2) * fx * fy * fxy - fxSqr * fyy;

            curvature = numer / denom;
            return true;
        }

    protected:
        ImplicitCurve2() = default;
    };
}
