//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include <algorithm>
#include "XmlUtil.h"
#include "StringUtil.h"
#include <cfloat>

//==== Get Number of Same Names ====//
unsigned int XmlUtil::GetNumNames( xmlNodePtr node, const char * name )
{
    int num;
    xmlNodePtr iter_node;

    num = 0;
    iter_node = node->xmlChildrenNode;

    //==== Parse This Level ====//
    while( iter_node != NULL )
    {
        if ( !xmlStrcmp( iter_node->name, ( const xmlChar * )name ) )
        {
            num++;
        }
        iter_node = iter_node->next;
    }

    return num;
}

//==== Get Node w/ Name and ID (Seq Num 0 - n ) ====//
xmlNodePtr XmlUtil::GetNodeDbg( xmlNodePtr node, const char * name, int id, const char* file, int lineno )
{
    int num;
    xmlNodePtr iter_node;

    if ( node == NULL )
    {
        return NULL;
    }

    static bool once = false;
    if ( !once && id > 100 )
    {
        printf( "Possible O(n^2) behavior detected with large n in call to XmlUtil::GetNode from %s line %d\n.", file, lineno );
        once = true;
    }

    num = 0;
    iter_node = node->xmlChildrenNode;

    //==== Parse This Level ====//
    while( iter_node != NULL )
    {
        if ( !xmlStrcmp( iter_node->name, ( const xmlChar * )name ) )
        {
            if ( id == num )
            {
                return iter_node;
            }

            num++;
        }
        iter_node = iter_node->next;
    }

//  printf("xmlGetNode: Can't find %s in node %s\n", name, node->name );
    return NULL;
}

//==== Extract Double From Node  ====//
double XmlUtil::ExtractDouble( xmlNodePtr node )
{
    char* str = ( char* )xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
    double val = 0.0;
    if ( str )
    {
        val = atof( str );
        xmlFree( str );
    }
    return val;
}

//==== Extract Int From Node  ====//
int XmlUtil::ExtractInt( xmlNodePtr node )
{
    char* str = ( char* )xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
    int val = 0;
    if ( str )
    {
        val = atoi( str );
        xmlFree( str );
    }
    return val;
}

//==== Extract String From Node  ====//
string XmlUtil::ExtractString( xmlNodePtr node )
{
    char* str = ( char* )xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
    string val;
    if ( str )
    {
        val = string( str );
        xmlFree( str );
    }

    return val;
}

//==== Find Double With Name, If Not Return Default ====//
double XmlUtil::FindDouble( xmlNodePtr node, const char * name, double def )
{
    xmlNodePtr n;

    if ( node == NULL )
    {
        return def;
    }

    n = XmlUtil::GetNode( node, name, 0 );
    if ( n )
    {
        return XmlUtil::ExtractDouble( n );
    }

    return def;
}

//==== Find Int With Name, If Not Return Default ====//
int XmlUtil::FindInt( xmlNodePtr node, const char * name, int def )
{
    xmlNodePtr n;

    if ( node == NULL )
    {
        return def;
    }

    n = XmlUtil::GetNode( node, name, 0 );
    if ( n )
    {
        return XmlUtil::ExtractInt( n );
    }

    return def;
}

//==== Find Double With Name, If Not Return Default ====//
string XmlUtil::FindString( xmlNodePtr node, const char * name, const string & def )
{
    xmlNodePtr n;

    if ( node == NULL )
    {
        return def;
    }

    n = XmlUtil::GetNode( node, name, 0 );
    if ( n )
    {
        string ret = XmlUtil::ExtractString( n );
        if ( ret != string() )
        {
            return ret;
        }
    }
    return def;
}

double XmlUtil::FindDoubleProp( xmlNodePtr node, const char * name, double def )
{
    char* str;
    if ( node == NULL )
    {
        return def;
    }

    str = ( char * )xmlGetProp( node, ( const xmlChar * )name );
    if ( str )
    {
        def = atof( str );
        xmlFree( str );
    }

    return def;
}

string XmlUtil::FindStringProp( xmlNodePtr node, const char * name, const string & def )
{
    char* str;

    if ( node == NULL )
    {
        return def;
    }

    string ret = def;

    str = ( char * )xmlGetProp( node, ( const xmlChar * )name );
    if( str )
    {
        ret = string( str );
        xmlFree( str );
    }

    return ret;
}

int XmlUtil::FindIntProp( xmlNodePtr node, const char * name, int def )
{
    char* str;
    if ( node == NULL )
    {
        return def;
    }

    str = ( char * )xmlGetProp( node, ( const xmlChar * )name );
    if ( str )
    {
        def = atoi( str );
        xmlFree( str );
    }
    return def;
}

//==== Add Int Val With Name To Node ====//
xmlNodePtr XmlUtil::AddIntNode( xmlNodePtr root, const char * name, int val )
{
    char str[255];
    xmlNodePtr node = xmlNewChild( root, NULL, ( const xmlChar * )name, NULL );

    sprintf( str, "%d", val );
    xmlNodeSetContent( node, ( const xmlChar * )str );

    return node;
}

//==== Add Double Val With Name To Node ====//
xmlNodePtr XmlUtil::AddDoubleNode( xmlNodePtr root, const char * name, double val )
{
    char str[255];
    xmlNodePtr node = xmlNewChild( root, NULL, ( const xmlChar * )name, NULL );

    sprintf( str, "%.*e", DBL_DIG + 3, val );

    xmlNodeSetContent( node, ( const xmlChar * )str );

    return node;
}

//==== Add String Val With Name To Node ====//
xmlNodePtr XmlUtil::AddStringNode( xmlNodePtr root, const char * name, const string & val )
{
    xmlNodePtr node = xmlNewChild( root, NULL, ( const xmlChar * )name, NULL );

    xmlNodeSetContent( node, ( const xmlChar * )val.c_str() );

    return node;
}

void XmlUtil::SetIntProp( xmlNodePtr root, const char * name, int val )
{
    char str[255];
    sprintf( str, "%d", val );
    xmlSetProp( root, ( const xmlChar * )name, ( const xmlChar * )str );
}

void XmlUtil::SetDoubleProp( xmlNodePtr root, const char * name, double val )
{
    char str[255];
    sprintf( str, "%.*e", DBL_DIG + 3, val );
    xmlSetProp( root, ( const xmlChar * )name, ( const xmlChar * )str );
}

void XmlUtil::SetStringProp( xmlNodePtr root, const char * name, const string & val )
{
    xmlSetProp( root, ( const xmlChar * )name, ( const xmlChar * )val.c_str() );
}

//==== Get Number of Items in Array ====//
int XmlUtil::GetNumArray( xmlNodePtr node, const char &sep )
{
    char* str;
    int i, len, num, elemcnt;

    if ( node == NULL )
    {
        return 0;
    }

    num = 0;
    elemcnt = 0;
    str = ( char* )xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
    len = strlen( str );

    i = 0;
    while ( i < len  )
    {
        if ( sep == str[i] && elemcnt > 0 )
        {
            num++;
            elemcnt = 0;
        }
        else if ( sep != str[i] )
        {
            elemcnt++;
        }
        i++;
    }
    if ( elemcnt )
    {
        num++;
    }
    xmlFree( str );
    return num;
}

//==== Extract Double Array ====//
int XmlUtil::ExtractDoubleArray( xmlNodePtr node, const char &sep, double * array, int size )
{
    char elem[255];
    char* str;
    int i, len, num, elemcnt;

    if ( node == NULL )
    {
        return 0;
    }

    num = 0;
    elemcnt = 0;
    str = ( char* )xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
    len = strlen( str );

    i = 0;
    while ( i < len && num < size )
    {
        if ( sep == str[i] && elemcnt > 0 )
        {
            /*==== Extract Double ====*/
            elem[elemcnt] = '\0';
            array[num] = atof( elem );
            num++;
            elemcnt = 0;
        }
        else if ( sep != str[i] )
        {
            elem[elemcnt] = str[i];
            elemcnt++;
        }
        i++;
    }
    if ( elemcnt )
    {
        /*==== Extract Double ====*/
        elem[elemcnt] = '\0';
        array[num] = atof( elem );
        num++;
    }
    xmlFree( str );
    return num;
}

//==== Extract Int Array ====//
int XmlUtil::ExtractIntArray( xmlNodePtr node, const char &sep, int * array, int size )
{
    char elem[255];
    char* str;
    int i, len, num, elemcnt;

    if ( node == NULL )
    {
        return 0;
    }

    num = 0;
    elemcnt = 0;
    str = ( char* )xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
    len = strlen( str );

    i = 0;
    while ( i < len && num < size )
    {
        if ( sep == str[i] && elemcnt > 0 )
        {
            /*==== Extract Double ====*/
            elem[elemcnt] = '\0';
            array[num] = atoi( elem );
            num++;
            elemcnt = 0;
        }
        else if ( sep != str[i] )
        {
            elem[elemcnt] = str[i];
            elemcnt++;
        }
        i++;
    }
    if ( elemcnt )
    {
        /*==== Extract Int ====*/
        elem[elemcnt] = '\0';
        array[num] = ( int )atof( elem );
        num++;
    }
    xmlFree( str );
    return num;
}

//==== Create Node and Add Vector Of Bools ====//
xmlNodePtr XmlUtil::AddVectorBoolNode( xmlNodePtr root, const char * name, const vector< bool > & vec )
{
    string str;
    char buff[256];
    for ( int i = 0 ; i < ( int )vec.size() ; i++ )
    {
        sprintf( buff, "%d, ", vec[i] ? 1 : 0 );
        str.append( buff );
    }
    str.append( "\0" );

    return AddStringNode( root, name, str );
}

//==== Create Node and Add Vector Of Ints ====//
xmlNodePtr XmlUtil::AddVectorIntNode( xmlNodePtr root, const char * name, const vector< int > & vec )
{
    string str;
    char buff[256];
    for ( int i = 0 ; i < ( int )vec.size() ; i++ )
    {
        sprintf( buff, "%d, ", vec[i] );
        str.append( buff );
    }
    str.append( "\0" );

    return AddStringNode( root, name, str );
}

//==== Create Node and Add Vector Of Doubles ====//
xmlNodePtr XmlUtil::AddVectorDoubleNode( xmlNodePtr root, const char * name, const vector< double > & vec )
{
    string str;
    char buff[256];
    for ( int i = 0 ; i < ( int )vec.size() ; i++ )
    {
        sprintf( buff, "%.*e, ", DBL_DIG + 3, vec[i] );
        str.append( buff );
    }
    str.append( "\0" );

    return AddStringNode( root, name, str );
}

//==== Create Node and Add Vec2d (Double Values) ====//
xmlNodePtr XmlUtil::AddVec2dNode( xmlNodePtr root, const char * name, const vec2d & vec )
{
    string str;
    char buff[256];

    sprintf( buff, "%.*e, ", DBL_DIG + 3, vec[0] );
    str.append( buff );
    sprintf( buff, "%.*e, ", DBL_DIG + 3, vec[1] );
    str.append( buff );

    str.append( "\0" );

    return AddStringNode( root, name, str );
}

//==== Create Node and Add Vec3d (Double Values) ====//
xmlNodePtr XmlUtil::AddVec3dNode( xmlNodePtr root, const char * name, const vec3d & vec )
{
    string str;
    char buff[256];

    sprintf( buff, "%.*e, ", DBL_DIG + 3, vec[0] );
    str.append( buff );
    sprintf( buff, "%.*e, ", DBL_DIG + 3, vec[1] );
    str.append( buff );
    sprintf( buff, "%.*e, ", DBL_DIG + 3, vec[2] );
    str.append( buff );

    str.append( "\0" );

    return AddStringNode( root, name, str );
}

//==== Create Node and Add Vector Of Vec3d ====//
xmlNodePtr XmlUtil::AddVectorVec3dNode( xmlNodePtr root, const char * name, const vector< vec3d > & vec )
{
    vector< double > xyz_vec;
    for ( int i = 0 ; i < ( int )vec.size() ; i++ )
    {
        xyz_vec.push_back( vec[i].x() );
        xyz_vec.push_back( vec[i].y() );
        xyz_vec.push_back( vec[i].z() );
    }

    return AddVectorDoubleNode( root, name, xyz_vec );
}

//==== Extract Vector Of Bools ====//
vector< bool > XmlUtil::ExtractVectorBoolNode( xmlNodePtr root, const char * name )
{
    vector< bool > ret_vec;

    string str = FindString( root, name, string() );

    int last_comma = -1;
    for ( int i = 0 ; i < ( int )str.size() ; i++ )
    {
        if ( str[i] == ',' )
        {
            string item = str.substr( last_comma + 1, i - last_comma - 1  );
            ret_vec.push_back( !!atoi( item.c_str() ) );
            last_comma = i;
        }
    }
    return ret_vec;
}

//==== Extract Vector Of Ints ====//
vector< int > XmlUtil::ExtractVectorIntNode( xmlNodePtr root, const char * name )
{
    vector< int > ret_vec;

    string str = FindString( root, name, string() );

    int last_comma = -1;
    for ( int i = 0 ; i < ( int )str.size() ; i++ )
    {
        if ( str[i] == ',' )
        {
            string item = str.substr( last_comma + 1, i - last_comma - 1  );
            ret_vec.push_back( atoi( item.c_str() ) );
            last_comma = i;
        }
    }
    return ret_vec;
}



//==== Extract Vector Of Doubles ====//
vector< double > XmlUtil::ExtractVectorDoubleNode( xmlNodePtr root, const char * name )
{
    vector< double > ret_vec;

    string str = FindString( root, name, string() );

    int last_comma = -1;
    for ( int i = 0 ; i < ( int )str.size() ; i++ )
    {
        if ( str[i] == ',' )
        {
            string item = str.substr( last_comma + 1, i - last_comma - 1  );
            ret_vec.push_back( atof( item.c_str() ) );
            last_comma = i;
        }
    }
    return ret_vec;
}

//==== Extract Vec2d ====//
vec2d XmlUtil::ExtractVec2dNode( xmlNodePtr root, const char * name )
{
    vector< double > temp_vec = ExtractVectorDoubleNode( root, name );
    vec2d ret_vec2d;

    if ( temp_vec.size() == 2 )
    {
        ret_vec2d.set_xy( temp_vec[0], temp_vec[1] );
    }

    return ret_vec2d;
}

//==== Extract Vec3d ====//
vec3d XmlUtil::ExtractVec3dNode( xmlNodePtr root, const char * name )
{
    vector< double > temp_vec = ExtractVectorDoubleNode( root, name );
    vec3d ret_vec3d;

    if ( temp_vec.size() == 3 )
    {
        ret_vec3d.set_xyz( temp_vec[0], temp_vec[1], temp_vec[2] );
    }

    return ret_vec3d;
}

//==== Extract Vector Of Vec3d ====//
vector< vec3d > XmlUtil::ExtractVectorVec3dNode( xmlNodePtr root, const char * name )
{
    vector< vec3d > ret_vec;

    vector< double > xyz_vec = ExtractVectorDoubleNode( root, name );

    for ( int i = 0 ; i < ( int )xyz_vec.size() ; i += 3 )
    {
        ret_vec.push_back( vec3d( xyz_vec[i], xyz_vec[i + 1], xyz_vec[i + 2] ) );
    }

    return ret_vec;
}

//==== Get Vector of Doubles ====//
vector< double > XmlUtil::GetVectorDoubleNode( xmlNodePtr node )
{
    vector< double > ret_vec;

    string str = ExtractString( node );

    int last_comma = -1;
    for ( int i = 0 ; i < ( int )str.size() ; i++ )
    {
        if ( str[i] == ',' )
        {
            string item = str.substr( last_comma + 1, i - last_comma - 1  );
            ret_vec.push_back( atof( item.c_str() ) );
            last_comma = i;
        }
    }
    return ret_vec;
}

//==== Get Vec2d ====//
vec2d XmlUtil::GetVec2dNode( xmlNodePtr node )
{
    vector< double > temp_vec = GetVectorDoubleNode( node );
    vec2d ret_vec2d;

    ret_vec2d[0] = temp_vec[0];
    ret_vec2d[1] = temp_vec[1];

    return ret_vec2d;
}

//==== Get Vec3d ====//
vec3d XmlUtil::GetVec3dNode( xmlNodePtr node )
{
    vector< double > temp_vec = GetVectorDoubleNode( node );
    vec3d ret_vec3d;

    ret_vec3d[0] = temp_vec[0];
    ret_vec3d[1] = temp_vec[1];
    ret_vec3d[2] = temp_vec[2];

    return ret_vec3d;
}

//==== Get Vector of Vec3ds ====//
vector< vec3d > XmlUtil::GetVectorVec3dNode( xmlNodePtr node )
{
    vector< vec3d > ret_vec;

    vector< double > xyz_vec = GetVectorDoubleNode( node );

    for ( int i = 0 ; i < ( int )xyz_vec.size() ; i += 3 )
    {
        ret_vec.push_back( vec3d( xyz_vec[i], xyz_vec[i + 1], xyz_vec[i + 2] ) );
    }

    return ret_vec;
}

//==== Encode File Contents ====//
xmlNodePtr XmlUtil::EncodeFileContents( xmlNodePtr root, const char* file_name )
{
    string str;
    FILE* fp = fopen( file_name, "r" );
    if ( fp )
    {
        char buff[256];
        while ( fgets( buff, 256, fp ) )
        {
            str.append( buff );
        }
        str.append( "\0" );
        fclose( fp );
    }

    return AddStringNode( root, "FileContents", str );
}

//==== Encode File Contents ====//
xmlNodePtr XmlUtil::DecodeFileContents( xmlNodePtr root, const char* file_name )
{
    string str;

    xmlNodePtr file_node = XmlUtil::GetNode( root, "FileContents", 0 );

    if ( file_node )
        str = ExtractString( file_node );

    if ( str.size() )
    {
        FILE* fp = fopen( file_name, "w" );
        if ( fp )
        {
            fprintf( fp, "%s", str.c_str() );
            fclose( fp );
        }
    }

    return file_node;
}

//==== Convert Chars Such As & < and > to XML Versions ====//
string XmlUtil::ConvertToXMLSafeChars( const string & input )
{
    string output = input;
    StringUtil::replace_all( output, "&", "AmMmMmMmP" );
    StringUtil::replace_all( output, ">", "GrRrRrRrT" );
    StringUtil::replace_all( output, "<", "LeEeEeEeT" );
    return output;
}

//==== Convert Chars Such As & < and > to XML Versions ====//
string XmlUtil::ConvertFromXMLSafeChars( const string & input )
{
    string output = input;
    StringUtil::replace_all( output, "AmMmMmMmP", "&" );
    StringUtil::replace_all( output, "GrRrRrRrT", ">" );
    StringUtil::replace_all( output, "LeEeEeEeT", "<" );
    return output;
}
