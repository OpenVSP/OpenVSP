//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(VSP_STL_HELPER__INCLUDED_)
#define VSP_STL_HELPER__INCLUDED_

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <algorithm>
#include <limits>
using std::vector;
using std::deque;
#include <APIDefines.h>


//==== Find Interval for Single Valued Increasing or Decreasing Array ===//
int find_interval( const vector< double > & vals, double val );

//==== Interpolate ===//
double interpolate( const vector< double > & vals, double val, int interval );

//==== Interpolate in Map ====//
double interpolate( const std::map< double, int > & val_map, double key, bool * in_range = NULL );

//==== Check If Vector Contains Val =====//
template <class T>
bool vector_contains_val( const vector< T > & vec, T const & val )
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

//==== Check If Vector Contains Val =====//
template <class T>
bool vector_contains_val_approx( const vector< T > & vec, T const & val, T const & tol )
{
    for ( int i = 0 ; i < ( int )vec.size() ; i++ )
    {
        if ( std::abs( val - vec[i] ) < tol )
        {
            return true;
        }
    }
    return false;
}

//==== Check If Deque Contains Val =====//
template <class T>
bool deque_contains_val( const deque< T > & deque, T const & val )
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

//==== Delete All Items From Vector Matching rem_vec ====//
template <class T>
void vector_remove_vector( vector< T > & vec, const vector< T > & rem_vec )
{
    for ( int i = 0 ; i < ( int )rem_vec.size() ; i++ )
    {
        vector_remove_val( vec, rem_vec[i] );
    }
}

template <class T>
int vector_find_minimum( const vector < T > & vec )
{
    int imin = 0;
    T vmin = vec[imin];
    for ( int i = 1; i < vec.size(); i++ )
    {
        if ( vec[i] < vmin )
        {
            imin = i;
            vmin = vec[imin];
        }
    }
    return imin;
}

template <class T>
void vector_insert_after( vector< T > & vec, int indx , T const & val )
{
    vector< T > new_vector;
    new_vector.reserve( vec.size() + 1 );
    int i;
    for ( i = 0 ; i <= indx ; i++ )
    {
        new_vector.push_back( vec[i] );
    }
    new_vector.push_back( val );
    for ( ; i < vec.size() ; i++ )
    {
        new_vector.push_back( vec[i] );
    }

    vec = new_vector;
}

//==== Find Index of Vector Where Val =====//
template <class T>
int vector_find_val( const vector< T > & vec, T const & val )
{
    for ( int i = 0 ; i < ( int )vec.size() ; i++ )
    {
        if ( val == vec[i] )
        {
            return i;
        }
    }
    return -1;
}

template <class T>
void vector_find_val_multiple( const vector< T > & vec, T const & val, vector< int > & indvec )
{
    indvec.clear();
    for ( int i = 0 ; i < ( int )vec.size() ; i++ )
    {
        if ( val == vec[i] )
        {
            indvec.push_back( i );
        }
    }
}

template <class T>
bool vector_remove_consecutive_duplicates( vector< T > & vec )
{
    bool condup = false;
    vector < int > dupindx;

    int n = vec.size();
    for ( int i = 0 ; i < n ; i++ )
    {
        int inext = i + 1;
        if ( inext >= n )
        {
            inext -= n;
        }

        if ( vec[i] == vec[inext] )
        {
            dupindx.push_back( i );
            dupindx.push_back( inext );
            condup = true;
        }
    }

    if ( condup )
    {
        vector < T > newvec;
        newvec.reserve( vec.size() );
        for ( int i = 0 ; i < n ; i++ )
        {
            if ( !vector_contains_val( dupindx, i ) )
            {
                newvec.push_back( vec[i] );
            }
        }
        vec = newvec;
    }
    return condup;
}

//==== Find Index of Vector Where Val =====//
template <class T>
int vector_find_val( const vector< T > & vec, T const & val, T const & tol )
{
    for ( int i = 0 ; i < ( int )vec.size() ; i++ )
    {
        if ( std::abs( val - vec[i] ) < tol )
        {
            return i;
        }
    }
    return -1;
}

//==== Find Index of Vector nearest to val =====//
template <class T>
int vector_find_nearest( const vector< T > & vec, T const & val )
{
    int ibest = -1;
    T dist = std::numeric_limits< T >::max();
    for ( int i = 0 ; i < ( int )vec.size() ; i++ )
    {
        T d = std::abs( val - vec[i] );
        if ( d < dist )
        {
            ibest = i;
            dist = d;
        }
    }
    return ibest;
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

template < typename T >
int ReorderVectorIndex( vector < T > &vec, int index, int action )
{
    int new_index = index;
    vector< T > new_vec;
    if ( action == vsp::REORDER_MOVE_TOP || action == vsp::REORDER_MOVE_BOTTOM )
    {
        if ( action == vsp::REORDER_MOVE_TOP )
        {
            new_vec.push_back( vec[index] );
            new_index = 0;
        }

        for ( int i = 0 ; i < ( int )vec.size() ; i++ )
        {
            if ( i != index )
            {
                new_vec.push_back( vec[ i ] );
            }
        }

        if ( action == vsp::REORDER_MOVE_BOTTOM )
        {
            new_index = new_vec.size();
            new_vec.push_back( vec[index] );
        }
    }
    else if ( action == vsp::REORDER_MOVE_UP || action == vsp::REORDER_MOVE_DOWN )
    {
        for ( int i = 0 ; i < ( int )vec.size() ; i++ )
        {
            if ( i < ( int )( vec.size() - 1 ) &&
                 ( ( action == vsp::REORDER_MOVE_DOWN && i == index ) ||
                   ( action == vsp::REORDER_MOVE_UP   && ( i + 1 ) == index ) ) )
            {
                new_vec.push_back( vec[i + 1] );
                new_vec.push_back( vec[i] );
                i++;

                if ( action == vsp::REORDER_MOVE_UP )
                {
                    new_index = index - 1;
                }
                else if (action == vsp::REORDER_MOVE_DOWN )
                {
                    new_index = index + 1;
                }
            }
            else
            {
                new_vec.push_back( vec[i] );
            }
        }
    }

    vec = new_vec;

    return new_index;
}

std::string string_vec_serialize( const vector < std::string > & str_vec );

#endif
