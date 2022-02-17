//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#define _USE_MATH_DEFINES
#include <cmath>
#include "Defines.h"

#include "Cluster.h"

double Cluster( const double &t, const double &ds0, const double &ds1 )
{
    return HypTan_Stretch( t, ds0, ds1 );
}

double Cubic_Stretch( const double &t, const double &ds0, const double &ds1 )
{
    double mt = 1.0 - t;
    return mt * mt * t * ds0 + mt * t * t * ( 3.0 - ds1 ) + t * t * t;
}

// Hyperbolic tangent stretching function
//
// J.F. Thompson, Z.U.A. Warsi, and C.W. Mastin, NUMERICAL GRID
// GENERATION, North-Holland, New York, pp. 307-308 (1985).
//
// M. Vinokur, On One-Dimensional Stretching Functions for
// Finite Difference Calculations, J. of Comp. Phys., Vol. 50,
// pp. 215-234 (1983).
//
double HypTan_Stretch( const double &t, const double &ds0, const double &ds1 )
{
    static double d0 = -1;
    static double d1 = -1;
    static double a = -1;
    static double b = -1;
    static double hdelta = -1;
    static double tnh2 = -1;

    if ( d0 != ds0 || d1 != ds1 )
    {
        d0 = ds0;
        d1 = ds1;
        a = sqrt( ds1 / ds0 );
        b = 1.0 / sqrt( ds0 * ds1 );

        // Avoid singularity at b == 1.0
        if ( b <= 0.999 )  // b < 1.0
        {
            hdelta = 0.5 * asinc( b );
            tnh2 = tan( hdelta );
        }
        else if ( b >= 1.001 )
        {
            hdelta = 0.5 * asinhc( b );
            tnh2 = tanh( hdelta );
        }
    }

    double x;

    // Avoid singularity at b == 1.0
    if ( b <= 0.999 )  // b < 1.0
    {
        x = 0.5 * ( 1.0 + tan( hdelta * ( 2.0 * t - 1.0 ) ) / tnh2 );
    }
    else if ( b >= 1.001 )
    {
        x = 0.5 * ( 1.0 + tanh( hdelta * ( 2.0 * t - 1.0 ) ) / tnh2 );
    }
    else // Handle b near 1.0
    {
        x = t * ( 1.0 + ( 2.0 * ( 1.0 - b ) ) * ( t - 0.5 ) * ( t - 1.0) );
    }

    //  Rescale coordinates
    x = x / ( a + ( 1.0 - a ) * x );

    return x;
}

// Series expansion of inverse cardinal hyperbolic sine.
// Where the cardinal hyperbolic sine is y( x ) = sinh( x ) / x.
//
// Consider algorithm from
// http://mathforum.org/kb/message.jspa?messageiD=449151
//
double asinhc_approx( const double &y )
{
    double x;
    if ( y <= 2.7829681178603 )
    {
        double a1 = -0.15;
        double a2 = 0.0573214285714;
        double a3 = -0.024907294878;
        double a4 = 0.0077424460899;
        double a5 = -0.0010794122691;

        double bb = y - 1.0;
        x = sqrt( 6.0 * bb ) * ( ( ( ( ( a5 * bb + a4 ) * bb + a3 ) * bb + a2 ) * bb + a1 ) * bb + 1.0);
    }
    else
    {
        double c0 = -0.0204176930892;
        double c1 = 0.2490272170591;
        double c2 = 1.9496443322775;
        double c3 = -2.629454725241;
        double c4 = 8.5679591096315;

        double v = log( y );
        double w = 1.0 / y - 1.0 / 35.0539798452776;
        x = v + log( 2.0 * v ) * ( 1.0 + 1.0 / v ) + ( ( ( c4 * w + c3 ) * w + c2) * w + c1 ) * w + c0;
    }
    return x;
}

// Inverse cardinal hyperbolic sine.
// Where the cardinal hyperbolic sine is y( x ) = sinh( x ) / x.
//
double asinhc( const double &y )
{
    static double lasty = -1.0; // Negative argument impossible
    static double lastx = 0;

    if ( y == lasty )
    {
        return lastx;
    }
    lasty = y;

    // Initialize with series expansion initial guess.
    double x = asinhc_approx( y );

    int maxiter = 4;
    double xmin = 5e-5;
    double tol = 1e-6;

    // Newton iterations
    for( int i = 0; i < maxiter; i++ )
    {
        double dx = 0.0;
        double f = 1.0;
        if ( std::abs( x ) >= xmin )
        {
            double sx = sinh( x );
            double cx = cosh( x );
            f = sx / x - y;
            double fp = ( x * cx - sx ) / ( x * x );
            dx = -f / fp;
        }
        else
        {
            // x fell below xmin
            x = 0.0;
            lastx = x;
            return x;
        }


        if ( std::abs( f ) < tol )
        {
            // Converged
            lastx = x;
            return x;
        }

        x = x + dx;
    }

    // Exceeded max iterations.
    lastx = x;
    return x;
}

// Series expansion of inverse cardinal sine.
// Where the cardinal sine is y( x ) = sin( x ) / x.
//
double asinc_approx( const double &y )
{
    double x;
    if ( y <= 0.2693897165164 )
    {
        double c3 = -2.6449340668482;
        double c4 = 6.7947319658321;
        double c5 = -13.2055008110734;
        double c6 = 11.7260952338351;

        x = PI * ( ( ( ( ( ( c6 * y + c5 ) * y + c4) * y + c3 ) * y + 1.0 ) * y - 1.0 ) * y + 1.0 );
    }
    else
    {
        double a1 = 0.15;
        double a2 = 0.0573214285714;
        double a3 = 0.0489742834696;
        double a4 = -0.053337753213;
        double a5 = 0.0758451335824;

        double bb = 1.0 - y;
        x = sqrt( 6.0 * bb ) * ( ( ( ( ( a5 * bb + a4 ) * bb + a3 ) * bb + a2 ) * bb + a1 ) * bb + 1.0 );
    }
    return x;
}

// Inverse cardinal sine.
// Where the cardinal sine is y( x ) = sin( x ) / x.
//
double asinc( const double &y )
{
    static double lasty = -1.0; // Negative argument impossible
    static double lastx = 0;

    if ( y == lasty )
    {
        return lastx;
    }
    lasty = y;

    // Initialize with series expansion initial guess.
    double x = asinc_approx( y );

    int maxiter = 4;
    double xmin = 5e-5;
    double tol = 1e-6;

    //    Newton iterations
    for( int i = 0; i < maxiter; i++ )
    {
        double f;
        double dx;
        if ( std::abs( x ) >= xmin )
        {
            double sx = sin( x );
            double cx = cos( x );
            f  = sx / x - y;
            double fp = ( x * cx - sx ) / ( x * x );
            dx = -f / fp;
        }
        else
        {
            // x fell below xmin
            x = 0.0;
            lastx = x;
            return x;
        }

        if ( std::abs( f ) < tol )
        {
            // Converged
            lastx = x;
            return x;
        }

        x = x + dx;
    }

    // Exceeded max iterations
    lastx = x;
    return x;
}
