//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include <stdarg.h>
#include "StringUtil.h"

//==== Change All "from" Characters -> "to" Characters ====//
void StringUtil::change_from_to( string & str, const char from, const char to )
{
    for ( int i = 0 ; i < ( int )str.size() ; i++ )
    {
        if ( str[i] == from )
        {
            str[i] = to;
        }
    }
}

//==== Change All "_" Characters -> " " Characters ====//
void StringUtil::chance_underscore_to_space( string & str )
{
    StringUtil::change_from_to( str, '_', ' ' );
}

void StringUtil::chance_space_to_underscore( string & str )
{
    StringUtil::change_from_to( str, ' ', '_' );
}

//==== Remove Leading Characters that match c =====//
void StringUtil::remove_leading( string & str, char c )
{
    int i;
    for ( i = 0 ; i < ( int )str.size() ; i++ )
    {
        if ( str[i] != c )
        {
            break;
        }
    }

    if ( i > 0 )
    {
        str.replace( 0, i, "" );
    }
}

//==== Remove Trailiner Characters that match c =====//
void StringUtil::remove_trailing( string & str, char c )
{
    int i;
    for ( i = ( int )str.size() - 1 ; i > 0 ; i-- )
    {
        if ( str[i] != c )
        {
            break;
        }
    }

    if ( i < ( int )str.size() - 1 )
    {
        str.replace( i + 1, str.size() - i, "" );
    }
}

//==== Convert Int To String =====//
string StringUtil::int_to_string( int i, const char* format )
{
    char str[256];
    sprintf( str, format, i );
    return string( str );
}

//==== Convert Double To String =====//
string StringUtil::double_to_string( double v, const char* format  )
{
    char str[256];
    sprintf( str, format, v );
    return string( str );
}

//==== Cound Number of Chars that match c =====//
int StringUtil::count_char_matches( string & str, char c )
{
    int cnt = 0;
    for ( int i = 0 ; i < ( int )str.size() ; i++ )
    {
        if ( str[i] == c )
        {
            cnt++;
        }
    }

    return cnt;
}
