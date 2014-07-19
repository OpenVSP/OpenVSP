//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(VSP_STL_HELPER__INCLUDED_)
#define VSP_STL_HELPER__INCLUDED_

#include <vector>
#include <deque>
#include <map>
#include <algorithm>
using std::vector;
using std::deque;

//==== Find Interval for Single Valued Increasing or Decreasing Array ===//
int find_interval( vector< double > & vals, double val );

//==== Interpolate ===//
double interpolate( vector< double > & vals, double val, int interval );

//==== Interpolate in Map ====//
double interpolate( std::map< double, int > & val_map, double key, bool * in_range = NULL );

//==== Check If Vector Contains Val =====//
template <class T>
bool vector_contains_val( vector< T > & vec, T const & val )
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

//==== Check If Deque Contains Val =====//
template <class T>
bool deque_contains_val( deque< T > & deque, T const & val )
{
    for ( int i = 0 ; i < ( int )deque.size() ; i++ )
    {
        if ( val == deque[i] )
        {
            return true;
        }
    }
    return false;
}

//==== Delete All Items From Deque Matching Val ====//
template<class T>
void deque_remove_val( deque< T > & val_deque, T const & val )
{
    deque< int > del_indices;
    for ( int i = 0 ; i < ( int )val_deque.size() ; i++ )
    {
        if ( val == val_deque[i] )
        {
            del_indices.push_front( i );
        }
    }
    for ( int i = 0 ; i < ( int )del_indices.size() ; i++ )
    {
        val_deque.erase( val_deque.begin() + del_indices[i] );
    }
}

//==== Delete All Items From Vector Matching Val ====//
template <class T>
void vector_remove_val( vector< T > & vec, T const & val )
{
    vector< T > new_vector;
    for ( int i = 0 ; i < ( int )vec.size() ; i++ )
    {
        if ( val != vec[i] )
        {
            new_vector.push_back( vec[i] );
        }
    }
    vec = new_vector;
}

//==== Clamp Value Between Two Vals ====//
template <typename T>
T Clamp( const T& value, const T& low, const T& high )
{
    return value < low ? low : ( value > high ? high : value );
}

//=== Return Index to Closest Element in Vector
int ClosestElement( const vector< double > & vec, double const & val );

// Return int with sign of passed val.
template < typename T >
int sgn( T val )
{
    return ( T( 0 ) < val ) - ( val < T( 0 ) );
}

#endif

