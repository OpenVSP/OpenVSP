//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#include "SpreadSheetWidget.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
void SpreadSheet< int >::set_value( int R, int C, const char * str )
{
    if ( m_Data)
    {
        (*m_Data)[ R ] = strtol( str, nullptr, 10 );
    }
}

template <>
string SpreadSheet< int >::get_value( int R, int C )
{
    if ( m_Data && (*m_Data).size() )
    {
        char s[255];
        snprintf( s, sizeof( s ), "%d", (*m_Data)[ R ] );
        return string( s );
    }
    return string();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
void SpreadSheet< double >::set_value( int R, int C, const char * str )
{
    if ( m_Data )
    {
        (*m_Data)[ R ] = strtod( str, nullptr );
    }
}

template <>
string SpreadSheet< double >::get_value( int R, int C )
{
    if ( m_Data && (*m_Data).size() )
    {
        char s[255];
        snprintf( s, sizeof( s ), "%f", (*m_Data)[ R ] );
        return string( s );
    }
    return string();
}

template <>
string SpreadSheet< double >::get_exact_value( int R, int C )
{
    if ( m_Data && (*m_Data).size() )
    {
        char s[255];
        snprintf( s, sizeof(s), "%.*e", DBL_DIG + 3, (*m_Data)[ R ] );
        return string( s );
    }
    return string();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
void SpreadSheet< string >::set_value( int R, int C, const char * str )
{
    if ( m_Data )
    {
        (*m_Data)[ R ] = string( str );
    }
}

template <>
string SpreadSheet< string >::get_value( int R, int C )
{
    if ( m_Data && (*m_Data).size() )
    {
        return (*m_Data)[ R ];
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
void SpreadSheet< vec3d >::set_value( int R, int C, const char * str )
{
    if ( m_Data )
    {
        (*m_Data)[ R ][ C ] = strtod( str, nullptr );
    }
}

template <>
string SpreadSheet< vec3d >::get_value( int R, int C )
{
    if ( m_Data && (*m_Data).size() )
    {
        char s[255];
        snprintf( s, sizeof( s ), "%f", (*m_Data)[ R ][ C ] );
        return string( s );
    }
    return string();
}

template <>
string SpreadSheet< vec3d >::get_exact_value( int R, int C )
{
    if ( m_Data && (*m_Data).size() )
    {
        char s[255];
        snprintf( s, sizeof(s), "%.*e", DBL_DIG + 3, (*m_Data)[ R ][ C ] );
        return string( s );
    }
    return string();
}

// Unfortunately, the following two methods must be re-specialized for every
// vector type.  Fortunately, copy/paste should suffice as they are data type
// independent.
template <>
int SpreadSheet< vec3d >::get_data_ncol()
{
    if ( get_data_nrow() > 0 )
    {
        return 3;
    }
    return 0;
};

template <>
void SpreadSheet< vec3d >::update_size( int nrow, int ncol )
{
    bool change = false;

    if ( m_Data )
    {
        if ( get_data_nrow() != nrow )
        {
            (*m_Data).resize( nrow );
            change = true;
        }

        if ( change )
        {
            update_size();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
void SpreadSheet< vector < int > >::set_value( int R, int C, const char * str )
{
    if ( m_Data )
    {
        bool int_flag = false;
        try
        {
            std::stoi( str );
            int_flag = true;
        }
        catch ( ... ) {}

        if ( int_flag )
        {
            (*m_Data)[ R ][ C ] = stoi( str, nullptr );
        }
    }
}

template <>
string SpreadSheet< vector < int > >::get_value( int R, int C )
{
    if ( m_Data && (*m_Data).size() )
    {
        char s[255];
        snprintf( s, sizeof( s ), "%d", (*m_Data)[ R ][ C ] );
        return string( s );
    }
    return string();
}

template <>
string SpreadSheet< vector < int > >::get_exact_value( int R, int C )
{
    if ( m_Data && (*m_Data).size() )
    {
        char s[255];
        snprintf( s, sizeof(s), "%d", (*m_Data)[ R ][ C ] );
        return string( s );
    }
    return string();
}

// Unfortunately, the following two methods must be re-specialized for every
// vector type.  Fortunately, copy/paste should suffice as they are data type
// independent.
template <>
int SpreadSheet< vector < int > >::get_data_ncol()
{
    if ( m_Data )
    {
        if ( get_data_nrow() > 0 )
        {
            return (*m_Data)[ 0 ].size();
        }
        return 0;
    }
    return 0;
};

template <>
void SpreadSheet< vector < int > >::update_size( int nrow, int ncol )
{
    if ( m_Data )
    {
        bool change = false;

        if ( get_data_nrow() != nrow )
        {
            (*m_Data).resize( nrow );
            change = true;
        }

        if ( ( get_data_ncol() != ncol ) || change )
        {
            for ( int i = 0; i < get_data_nrow(); i++ )
            {
                ( *m_Data )[ i ].resize( ncol );
            }
            change = true;
        }

        if ( change )
        {
            update_size();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
void SpreadSheet< vector < double > >::set_value( int R, int C, const char * str )
{
    if ( m_Data )
    {
        bool dbl_flag = false;
        try
        {
            std::stod( str );
            dbl_flag = true;
        }
        catch ( ... ) {}
        if ( dbl_flag )
        {
            (*m_Data)[ R ][ C ] = strtod( str, nullptr );
        }
    }
}

template <>
string SpreadSheet< vector < double > >::get_value( int R, int C )
{
    if ( m_Data && (*m_Data).size() )
    {
        char s[255];
        snprintf( s, sizeof( s ), "%f", (*m_Data)[ R ][ C ] );
        return string( s );
    }
    return string();
}

template <>
string SpreadSheet< vector < double > >::get_exact_value( int R, int C )
{
    if ( m_Data && (*m_Data).size() )
    {
        char s[255];
        snprintf( s, sizeof(s), "%.*e", DBL_DIG + 3, (*m_Data)[ R ][ C ] );
        return string( s );
    }
    return string();
}

// Unfortunately, the following two methods must be re-specialized for every
// vector type.  Fortunately, copy/paste should suffice as they are data type
// independent.
template <>
int SpreadSheet< vector < double > >::get_data_ncol()
{
    if ( m_Data )
    {
        if ( get_data_nrow() > 0 )
        {
            return (*m_Data)[ 0 ].size();
        }
        return 0;
    }
    return 0;
};

template <>
void SpreadSheet< vector < double > >::update_size( int nrow, int ncol )
{
    if ( m_Data )
    {
        bool change = false;

        if ( get_data_nrow() != nrow )
        {
            (*m_Data).resize( nrow );
            change = true;
        }

        if ( ( get_data_ncol() != ncol ) || change )
        {
            for ( int i = 0; i < get_data_nrow(); i++ )
            {
                ( *m_Data )[ i ].resize( ncol );
            }
            change = true;
        }

        if ( change )
        {
            update_size();
        }
    }
}
