//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//


#include "Util.h"
#include <math.h>
#include <time.h>

//==== Generate A Unique Random String of Length =====//
string GenerateRandomID( int length )
{
    static bool seed = false;
    if ( !seed )
    {
        seed = true;
        srand( ( unsigned int )time( NULL ) );
    }

    static char str[256];
    for ( int i = 0 ; i < length ; i++ )
    {
        str[i] = ( char )( ( rand() % 26 ) + 65 );
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
        double lx = log10( abs( x ) );
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
    return MIN( 0.0, floor2scale( x, scale( x ) ) );
}

double magroundup( double x )
{
    return MAX( 0.0, ceil2scale( x, scale( x ) ) );
}

double scale( double x )
{
    return pow( 10.0, mag( x ) );
}

double floor2scale( double x, double scale )
{
    if( scale == 0 )
    {
        scale = FLT_MIN * 10;
    }
    return scale * floor( x / scale );
}

double ceil2scale( double x, double scale )
{
    if( scale == 0 )
    {
        scale = FLT_MIN * 10;
    }
    return scale * ceil( x / scale );
}
