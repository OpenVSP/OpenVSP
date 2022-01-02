//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "VspUtil.h"
#include <random>
#include <pcg_random.hpp>

//==== Generate A Unique Random String of Length =====//
string GenerateRandomID( int length )
{
    static bool seed = false;
    static pcg64_fast rng;

    if ( !seed )
    {
        seed = true;
        pcg_extras::seed_seq_from< std::random_device > seed_source;
        rng.seed( seed_source );
    }

    static char str[256];
    for ( int i = 0 ; i < length ; i++ )
    {
        str[i] = ( char )( ( rng() % 26 ) + 65 );
    }
    return string( str, length );
}

//==== Convert A Double To Bool ====//
bool ToBool( double val )
{
    if ( val > 1.0e-12 )
    { return true; }
    return false;
}

double mag( double x )
{
    if ( x == 0 )
    {
        return 0;
    }
    else
    {
        double lx = log10( std::abs( x ) );
        if ( lx < 0 )
        {
            return ceil( lx );
        }
        else
        {
            return floor( lx );
        }
    }
}

double magrounddn( double x )
{
    return MIN( 0.0, floor2scale( x, pow( 10.0, mag( x ) ) ) );
}

double magroundup( double x )
{
    return MAX( 0.0, ceil2scale( x, pow( 10.0, mag( x ) ) ) );
}

/* floor2scale(x,scale) rounds a number down to a specific precision
    Examples:
    320         = floor2scale( 321.123456, 10   )
    321.        = floor2scale( 321.123456, 1    )
    321.1       = floor2scale( 321.123456, 0.1  )
    321.12      = floor2scale( 321.123456, 0.01 )
    318.        = floor2scale( 321.123456, 6    )   // floor to nearest multiple of 6
    324.        = floor2scale( 321.123456, -6   )   // ceil to nearest multiple of 6 (note the "-" sign or scale)
    321.123456  = floor2scale( 321.123456, 0    )   // infinite precision --> return original value
*/
double floor2scale( double x, double scale )
{
    if( scale == 0 )
    {
        return x;
    }
    return scale * floor( x / scale );
}

/* ceil2scale(x,scale) rounds a number up to a specific precision
    Examples:
    330         = ceil2scale( 321.123456, 10   )
    322.        = ceil2scale( 321.123456, 1    )
    321.2       = ceil2scale( 321.123456, 0.1  )
    321.13      = ceil2scale( 321.123456, 0.01 )
    324.        = ceil2scale( 321.123456, 6    )   // ceil to nearest multiple of 6
    318.        = ceil2scale( 321.123456, -6   )   // floor to nearest multiple of 6 (note the "-" sign or scale)
    321.123456  = ceil2scale( 321.123456, 0    )   // infinite precision --> return original value
*/
double ceil2scale( double x, double scale )
{
    if( scale == 0 )
    {
        return x;
    }
    return scale * ceil( x / scale );
}

vector<double> logspace(double a, double b, double n)
{
    std::vector<double> linarray = linspace(a,b, n);
    std::vector<double> logarray;
    for(size_t i = 0; i < linarray.size(); ++i)
    {
        double v = pow(10.0, linarray[i]);
        logarray.push_back(v);
    }
    return logarray;
}

vector<double> linspace(double a, double b, double n)
{
    vector<double> linarray;
    double step = (b-a) / (n-1.0);

    while(a <= b)
    {
        linarray.push_back(a);
        a += step;
    }
    return linarray;
}

double linterp( double a, double b, double frac )
{
    return a + frac * ( b - a );
}
