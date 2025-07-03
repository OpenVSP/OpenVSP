// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#pragma once

#include <Mathematics/Math.h>
#include <Mathematics/Matrix.h>
#include <vector>
using namespace gte;

class FitATan
{
public:
    FitATan();

    // The degree D = 2*Order + 1.  The polynomial coefficients are for
    // the odd powers of x.
    template <int Order>
    void Generate(std::vector<double>& poly, double& error) const;

private:
    // f(x) = atan(x)
    // g(x) = atan(x) - p(x)
    double G(double x, int const order, double const* p) const;

    // g'(x) = 1/(1 + x^2) - p'(x)
    double GDer(double x, int const order, double const* p) const;

    int SignG(double x, int const order, double const* p) const;
    int SignGDer(double x, int const order, double const* p) const;
};


FitATan::FitATan()
{
}

double FitATan::G(double x, int const order, double const* p) const
{
    double xsqr = x * x;
    int i = order;
    double result = p[i];
    while (--i >= 0)
    {
        result = xsqr*result + p[i];
    }
    result = atan(x) - x * result;
    return result;
}

double FitATan::GDer(double x, int const order, double const* p) const
{
    double xsqr = x * x;
    int i = order;
    double result = (2 * i + 1)*p[i];
    while (--i >= 0)
    {
        result = xsqr*result + (2 * i + 1)*p[i];
    }
    result = 1.0 / (1.0 + xsqr) - result;
    return result;
}

int FitATan::SignG(double x, int const order, double const* p) const
{
    double g = G(x, order, p);
    return (g > 0.0 ? 1 : (g < 0.0 ? -1 : 0));
}

int FitATan::SignGDer(double x, int const order, double const* p) const
{
    double gder = GDer(x, order, p);
    return (gder > 0.0 ? 1 : (gder < 0.0 ? -1 : 0));
}

template <int Order>
void FitATan::Generate(std::vector<double>& poly, double& error) const
{
    Vector<Order + 1, double> g0root;
    g0root[0] = 0.0;
    for (int i = 1; i < Order; ++i)
    {
        g0root[i] = static_cast<double>(i) / static_cast<double>(Order);
    }
    g0root[Order] = 1.0;

    // Initial guess at the polynomial, gives you oscillation.
    Matrix<Order + 1, Order + 1, double> A;
    Vector<Order + 1, double> B;
    A(0, 0) = 1.0;
    for (int c = 1; c <= Order; ++c)
    {
        A(0, c) = 0.0;
    }
    B[0] = 1.0;
    for (int r = 1; r <= Order; ++r)
    {
        double x = g0root[r], xsqr = x * x;
        A(r, 0) = x;
        for (int c = 1; c <= Order; ++c)
        {
            A(r, c) = xsqr * A(r, c - 1);
        }
        B[r] = std::atan(x);
    }
    Vector<Order + 1, double> p = Inverse(A) * B;

    double absError[Order], e;

    // Perturb away from zero.
    g0root[0] = 1e-02;
    for (int iter = 0; iter < 16; ++iter)
    {
        // Find local extremum on [g0root[i],g0root[i+1]].
        Vector<Order, double> g1root;
        int k = 0;
        for (int i = 0; i < Order; ++i)
        {
            double x0 = g0root[i];
            double x1 = g0root[i + 1];
            int s0 = SignGDer(x0, Order, &p[0]);
            int s1 = SignGDer(x1, Order, &p[0]);
            LogAssert(s0*s1 < 0, "Unexpected condition.");
            int j;
            for (j = 0; j < 1024; ++j)
            {
                double xmid = 0.5*(x0 + x1);
                if (x0 != xmid && x1 != xmid)
                {
                    int smid = SignGDer(xmid, Order, &p[0]);
                    if (smid == s0)
                    {
                        x0 = xmid;
                    }
                    else if (smid == s1)
                    {
                        x1 = xmid;
                    }
                    else
                    {
                        g1root[k++] = xmid;
                        break;
                    }
                }
                else
                {
                    g1root[k++] = xmid;
                    break;
                }

            }
            LogAssert(j < 1024, "Max j-iterations not large enough.");
        }

        for (int i = 0; i < Order; ++i)
        {
            absError[i] = std::abs(G(g1root[i], Order, &p[0]));
        }

        // Solve p(x[i]) + (-1)^{i}*e = f(x[i]) for e and p[j].
        double sign = 1.0;
        for (int r = 0; r < Order; ++r, sign = -sign)
        {
            double x = g1root[r], xsqr = x * x;
            A(r, 0) = xsqr * x;
            for (int c = 1; c < Order; ++c)
            {
                A(r, c) = xsqr * A(r, c - 1);
            }
            A(r, Order) = sign;
            B[r] = std::atan(x) - x;
        }
        double x = 1.0, xsqr = x * x;
        A(Order, 0) = xsqr * x;
        for (int c = 1; c < Order; ++c)
        {
            A(Order, c) = xsqr * A(Order, c - 1);
        }
        A(Order, Order) = 0.0;
        B[Order] = std::atan(x) - x;

        Vector<Order + 1, double> X = Inverse(A)*B;
        for (int i = 0; i < Order; ++i)
        {
            p[i + 1] = X[i];
        }
        e = X[Order];

        k = 1;
        for (int i = 0; i < Order - 1; ++i)
        {
            double x0 = g1root[i];
            double x1 = g1root[i + 1];
            int s0 = SignG(x0, Order, &p[0]);
            int s1 = SignG(x1, Order, &p[0]);
            LogAssert(s0*s1 < 0, "Unexpected condition.");
            int j;
            for (j = 0; j < 1024; ++j)
            {
                double xmid = 0.5*(x0 + x1);
                if (x0 != xmid && x1 != xmid)
                {
                    int smid = SignG(xmid, Order, &p[0]);
                    if (smid == s0)
                    {
                        x0 = xmid;
                    }
                    else if (smid == s1)
                    {
                        x1 = xmid;
                    }
                    else
                    {
                        g0root[k++] = xmid;
                        break;
                    }
                }
                else
                {
                    g0root[k++] = (x0 == xmid ? x0 : x1);
                    break;
                }

            }
            LogAssert(j < 1024, "Max j-iterations not large enough.");
        }
    }

    poly.resize(Order + 1);
    for (int i = 0; i <= Order; ++i)
    {
        poly[i] = p[i];
    }
    error = e;
}

