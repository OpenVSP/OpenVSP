//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "StringUtil.h"

void StringUtil::change_from_to( char *str, const char from, const char to )
{
    int i = 0;
    while( str[i] != 0 )
    {
        if ( str[i] == from )
        {
            str[i] = to;
        }
        i++;
    }
}

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


//==== Replace First Instance of From String with To String ====//
bool StringUtil::replace_once( string & str, const string & from, const string & to )
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

//==== Replace All Instances of From String with To String ====//
void StringUtil::replace_all( string & str, const string & from, const string & to  )
{
    if(from.empty())
        return;

    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
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

//==== Convert Vec3d to a string with values separated by spaces ====//
string StringUtil::vec3d_to_string(const vec3d & vec, const char* format)
{
    char buff[256];
    sprintf(buff,format,vec[0],vec[1],vec[2]);
    return string(buff);
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


//==== Compute A Hash From String (Not Unique but Close I Think) =====//
int StringUtil::compute_hash( const string & str )
{
    int hash = 0;

    for ( int i = 0 ; i < (int)str.size() ; i++ )
    {
        hash += str[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    // final avalanche
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}
