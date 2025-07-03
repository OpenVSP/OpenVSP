// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/BasisFunction.h>
#include <Mathematics/ParametricCurve.h>
#include <algorithm>
#include <cstdint>
#include <vector>

namespace gte
{
    template <int32_t N, typename Real>
    class BSplineCurve : public ParametricCurve<N, Real>
    {
    public:
        // Construction.  If the input controls is non-null, a copy is made of
        // the controls.  To defer setting the control points, pass a null
        // pointer and later access the control points via GetControls() or
        // SetControl() member functions.  The domain is t in [t[d],t[n]],
        // where t[d] and t[n] are knots with d the degree and n the number of
        // control points.
        BSplineCurve(BasisFunctionInput<Real> const& input, Vector<N, Real> const* controls)
            :
            ParametricCurve<N, Real>((Real)0, (Real)1),
            mBasisFunction(input)
        {
            // The mBasisFunction stores the domain but so does
            // ParametricCurve.
            this->mTime.front() = mBasisFunction.GetMinDomain();
            this->mTime.back() = mBasisFunction.GetMaxDomain();

            // The replication of control points for periodic splines is
            // avoided by wrapping the i-loop index in Evaluate.
            mControls.resize(input.numControls);
            if (controls)
            {
                std::copy(controls, controls + input.numControls, mControls.begin());
            }
            else
            {
                Vector<N, Real> zero{ (Real)0 };
                std::fill(mControls.begin(), mControls.end(), zero);
            }
            this->mConstructed = true;
        }

        // Member access.
        inline BasisFunction<Real> const& GetBasisFunction() const
        {
            return mBasisFunction;
        }

        inline int32_t GetNumControls() const
        {
            return static_cast<int32_t>(mControls.size());
        }

        inline Vector<N, Real> const* GetControls() const
        {
            return mControls.data();
        }

        inline Vector<N, Real>* GetControls()
        {
            return mControls.data();
        }

        void SetControl(int32_t i, Vector<N, Real> const& control)
        {
            if (0 <= i && i < GetNumControls())
            {
                mControls[i] = control;
            }
        }

        Vector<N, Real> const& GetControl(int32_t i) const
        {
            if (0 <= i && i < GetNumControls())
            {
                return mControls[i];
            }
            else
            {
                return mControls[0];
            }
        }

        // Evaluation of the curve.  The function supports derivative
        // calculation through order 3; that is, order <= 3 is required.  If
        // you want/ only the position, pass in order of 0.  If you want the
        // position and first derivative, pass in order of 1, and so on.  The
        // output array 'jet' must have enough storage to support the maximum
        // order.  The values are ordered as: position, first derivative,
        // second derivative, third derivative.
        virtual void Evaluate(Real t, uint32_t order, Vector<N, Real>* jet) const override
        {
            uint32_t const supOrder = ParametricCurve<N, Real>::SUP_ORDER;
            if (!this->mConstructed || order >= supOrder)
            {
                // Return a zero-valued jet for invalid state.
                for (uint32_t i = 0; i < supOrder; ++i)
                {
                    jet[i].MakeZero();
                }
                return;
            }

            int32_t imin, imax;
            mBasisFunction.Evaluate(t, order, imin, imax);

            // Compute position.
            jet[0] = Compute(0, imin, imax);
            if (order >= 1)
            {
                // Compute first derivative.
                jet[1] = Compute(1, imin, imax);
                if (order >= 2)
                {
                    // Compute second derivative.
                    jet[2] = Compute(2, imin, imax);
                    if (order == 3)
                    {
                        jet[3] = Compute(3, imin, imax);
                    }
                }
            }
        }

    private:
        // Support for Evaluate(...).
        Vector<N, Real> Compute(uint32_t order, int32_t imin, int32_t imax) const
        {
            // The j-index introduces a tiny amount of overhead in order to handle
            // both aperiodic and periodic splines.  For aperiodic splines, j = i
            // always.

            int32_t numControls = GetNumControls();
            Vector<N, Real> result;
            result.MakeZero();
            for (int32_t i = imin; i <= imax; ++i)
            {
                Real tmp = mBasisFunction.GetValue(order, i);
                int32_t j = (i >= numControls ? i - numControls : i);
                result += tmp * mControls[j];
            }
            return result;
        }

        BasisFunction<Real> mBasisFunction;
        std::vector<Vector<N, Real>> mControls;
    };
}
