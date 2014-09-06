//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include <math.h>
#include "StlHelper.h"
using std::map;

//==== Find Interval for Single Valued Increasing or Decreasing Array ===//
int find_interval( const vector< double > & vals, double value )
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
double interpolate( const vector< double > & arr, double value, int interval )
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

double interpolate( const map< double, int > & val_map, double key, bool * in_range )
{
    map< double, int >::const_iterator low, high, it;
    vector<double> val_vector;
    double tol = 1e-10;

    if ( in_range )
    {
        *in_range = true;
    }


    high = val_map.upper_bound( key );

    if ( high == val_map.end() )
    {
        // value is greater than all values in vector
        double last_val =  val_map.rbegin()->first;
        if ( fabs( last_val - key ) > tol && in_range )
        {
            *in_range = false;
        }

        return ( double )val_map.rbegin()->second;
    }
    else if ( high == val_map.begin() )
    {
        double first_val =  val_map.begin()->first;
        if ( fabs( first_val - key ) > tol && in_range )
        {
            *in_range = false;
        }
        return ( double )val_map.begin()->second;
    }
    else
    {
        low = std::prev( high );
        double y0 = ( double )low->second;
        double y1 = ( double )high->second;

        double denom = ( high->first - low->first );
        if ( denom == 0.0 )
        {
            return 0.0;
        }
        else
        {
            return ( y1 - y0 ) / denom * ( key - low->first ) + y0;
        }
    }
}

int ClosestElement( const vector< double > & vec, double const & val )
{
    // Vector should be sorted before calling this method
    vector< double >::const_iterator low, vend(vec.end()), vlast;
    low = std::lower_bound( vec.begin(), vec.end(), val );
    vlast=vend; --vlast;

    if ( low == vec.end() )
    {
        // value is greater than all values in vector
        return vec.size();
    }
    else if ( low == vlast )
    {
        return ( low - vec.begin() );
    }
    else
    {
        // Check if the next value is closer
        double low_val = *low;
        double high_val = *( low + 1 );
        double dist_low = val - low_val;
        double dist_high = high_val - val;

        if ( dist_low < dist_high )
        {
            return ( low - vec.begin() );
        }
        else
        {
            return ( low + 1 - vec.begin() );
        }
    }
}
