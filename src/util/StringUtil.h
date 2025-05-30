//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(VSPSTRINGUTIL__INCLUDED_)
#define VSPSTRINGUTIL__INCLUDED_


#include <string>
#include "Vec3d.h"
using std::string;

//==== String Functions =====//
namespace StringUtil
{
void change_from_to( char* str, const char &from, const char &to );
void change_from_to( string & str, const char &from, const char &to );
void remove_all( string& str, const char& c );
void change_underscore_to_space( string & str );
void change_space_to_underscore( string & str );

bool replace_once( string & str, const string & from, const string & to );
void replace_all( string & str, const string & from, const string & to  );

void remove_leading( string & str, char c );
void remove_trailing( string & str, char c );
void remove_leading_trailing( string & str, char c );

bool string_is_number( const string &str );
string int_to_string( int i, const char* format );
string double_to_string( double v, const char* format );
string vec3d_to_string( const vec3d & vec, const char* format );
std::vector < string > csv_to_vecstr( const char * str );

int count_char_matches( const string & str, char c );

int compute_hash( const string & str );

string get_delim( int delim_type );

string truncateFileName( const string &fn, int len );

string NasFmt( double input );

void parse_table( const char * str, int len, std::vector < std::vector < string > > & table );

}


#endif

