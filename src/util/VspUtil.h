//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(UTIL_UTIL__INCLUDED_)
#define UTIL_UTIL__INCLUDED_

#include <cmath>

#include <vector>
#include <string>
using std::vector;
using std::string;

#include "Vec3d.h"
#include "Vec2d.h"

#ifndef MAX
#define MAX(x,y) (((x) < (y)) ? (y):(x))
#endif
#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x):(y))
#endif
#define DEG2RAD(x) ( (x)*0.017453293 )

// Function forms of the MAX, MIN and DEG2RAD macros above.  The macros evaluate their arguments
// twice, and a macro cannot be exposed through the API at all; these can be, and are -- the
// APIUtilities group in VSP_Geom_API.h wraps them, which is what makes them reachable from a
// script.  Kept next to the macros so the two do not drift apart.
inline double Min( double x, double y )
{
    if ( x < y )
    {
        return x;
    }
    return y;
}

inline double Max( double x, double y )
{
    if ( x > y )
    {
        return x;
    }
    return y;
}

inline double Rad2Deg( double r )
{
    return r * ( 180.0 / M_PI );
}

inline double Deg2Rad( double d )
{
    return d * ( M_PI / 180.0 );
}

// Print to stdout.  Four overloads so a script can print the types it deals in without converting
// them first, which is the whole point of having them.
extern void Print( const string & data, bool new_line = true );
extern void Print( const vec3d & data, bool new_line = true );
extern void Print( double data, bool new_line = true );
extern void Print( int data, bool new_line = true );

#ifndef SQ
#define SQ(x) ((x)*(x))
#endif

#define INTERP_PARM( ptr_a, ptr_b, f, parm_name ) ((parm_name) = linterp( (ptr_a)->parm_name(), (ptr_b)->parm_name(), (f) ))

#include <cfloat>  /* for DBL_EPSILON */

extern string GenerateRandomID( int length );

extern bool ToBool( double val );

double mag( double x );
double magrounddn( double x );
double magroundup( double x );
double magp1roundup( double x );
double floor2scale( double x, double scale );
double ceil2scale( double x, double scale );
vector <double> linspace( double a, double b, double n );
vector <double> logspace( double a, double b, double n);

double linterp( double a, double b, double frac );

int toint( double x );

bool angle_less( double a, double b );

/*
template <typename T> T clamp( T val, T min, T max )
{
    if ( val < min )
    {
        val = min;
    }

    if ( val > max )
    {
        val = max;
    }

    return val;
}
*/

// Clamp cyclically, where val == max gets clamped to min, max/min will always have unique value.
template <typename T> T clampCyclic( T val, T min, T max )
{
    T dv = max - min;

    while ( val < min )
    {
        val += dv;
    }

    while ( val >= max )
    {
        val -= dv;
    }

    return val;
}

// Clamp cyclically, where val == max does not get changed, max/min can have multiple values.
template <typename T> T clampCyclicUBInclusive( T val, T min, T max )
{
    T dv = max - min;

    while ( val < min )
    {
        val += dv;
    }

    while ( val > max )
    {
        val -= dv;
    }

    return val;
}

template <typename T> T clampCyclic( T val, T max )
{
    if ( max != 0 )
    {
        while ( val < 0 )
        {
            val += max;
        }

        while ( val >= max )
        {
            val -= max;
        }
    }
    return val;
}

// Clamp val from (-pi, pi]
template < typename T > T clampAngle( T val )
{
    val = fmod( val, 2.0 * M_PI );
    if ( val > M_PI )
    {
        val -= 2.0 * M_PI;
    }
    return val;
}

template < typename T >
bool aboutequal( T a, T b, T tol = 1e-12 )
{
    return std::abs( a - b ) < tol;
}

#endif

