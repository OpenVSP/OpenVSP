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
        return true;
    return false;
}
