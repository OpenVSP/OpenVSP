//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(UTIL_UTIL__INCLUDED_)
#define UTIL_UTIL__INCLUDED_

#include "Defines.h"
#include <vector>
#include "Vec3d.h"
using namespace std;

#ifndef MAX
#define MAX(x,y) (((x) < (y)) ? (y):(x))
#endif
#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x):(y))
#endif
#define DEG2RAD(x) ( (x)*0.017453293 )

#define INTERP_PARM( ptr_a, ptr_b, f, parm_name ) ((parm_name) = linterp( (ptr_a)->parm_name(), (ptr_b)->parm_name(), (f) ))

#include <cfloat>  /* for DBL_EPSILON */

template <class T> bool ContainsVal( vector< T > & vec, T const & val )
{
    for ( int i = 0 ; i < ( int )vec.size() ; i++ )
    {
        if ( val == vec[i] )
        {
            return true;
        }
    }
    return false;
}

extern string GenerateRandomID( int length );

extern bool ToBool( double val );

double mag( double x );
double magrounddn( double x );
double magroundup( double x );
double floor2scale( double x, double scale );
double ceil2scale( double x, double scale );
vector <double> linspace( double a, double b, double n );
vector <double> logspace( double a, double b, double n);

double linterp( double a, double b, double frac );

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

template < typename T >
bool aboutequal( T a, T b, T tol = 1e-12 )
{
    return std::abs( a - b ) < tol;
}

#endif

