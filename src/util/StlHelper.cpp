//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "StlHelper.h"


//==== Find Interval for Single Valued Increasing or Decreasing Array ===//
int find_interval( vector< double > & vals, double value )
{
    int dim = vals.size();
    if ( dim == 0 )
    {
        return 0;
    }

    //==== Check If Array Increasing Or Decreasing ====//
    if ( vals[0] < vals.back() )
    {
        for ( int i = 1 ; i < dim ; i++ )
        {
            if ( value < vals[i] )
            {
                return( i - 1 );
            }
        }
    }
    else
    {
        for ( int i = 1 ; i < dim ; i++ )
        {
            if ( value > vals[i] )
            {
                return( i - 1 );
            }
        }
    }
    return( dim - 2 );
}

//==== Interpolate ===//
double interpolate( vector< double > & arr, double value, int interval )
{
    int dim = arr.size();
    if ( dim <= 0 )
    {
        return( 0.0 );
    }

    if ( interval < 0 )
    {
        return( 0.0 );
    }
    if ( interval > dim - 2 )
    {
        return( 1.0 );
    }

    double denom = arr[interval + 1] - arr[interval];

    if ( denom == 0.0 )
    {
        return( 0.0 );
    }

    return( ( double )( value - arr[interval] ) / denom );
}

