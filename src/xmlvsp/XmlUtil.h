//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(VSPXMLUTIL__INCLUDED_)
#define VSPXMLUTIL__INCLUDED_

#include "Vec3d.h"

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/hash.h>

#include <vector>
#include <string>
#include <cstring>
using std::string;
using std::vector;

//==== String Functions =====//
namespace XmlUtil
{
int GetNumNames( xmlNodePtr node, const char * name );
xmlNodePtr GetNode( xmlNodePtr node, const char * name, int num );

double ExtractDouble( xmlNodePtr node );
int    ExtractInt( xmlNodePtr node );
string ExtractString( xmlNodePtr node );

double FindDouble( xmlNodePtr node, const char * name, double def );
int    FindInt( xmlNodePtr node, const char * name, int def );
string FindString( xmlNodePtr node, const char * name, const string & def );
double FindDoubleProp( xmlNodePtr node, const char * name, double def );
int FindIntProp( xmlNodePtr node, const char * name, int def );
string FindStringProp( xmlNodePtr node, const char * name, const string & def );

int GetNumArray( xmlNodePtr node, const char delim );

int ExtractDoubleArray( xmlNodePtr node, const char sep, double * arr, int size );
int ExtractIntArray( xmlNodePtr node, const char sep, int * arr, int size );

xmlNodePtr AddIntNode( xmlNodePtr root, const char * name, int val );
xmlNodePtr AddDoubleNode( xmlNodePtr root, const char * name, double val );
xmlNodePtr AddStringNode( xmlNodePtr root, const char * name, const string & val );
void SetIntProp( xmlNodePtr root, const char * name, int val );
void SetDoubleProp( xmlNodePtr root, const char * name, double val );
void SetStringProp( xmlNodePtr root, const char * name, string & val );

xmlNodePtr AddVectorBoolNode( xmlNodePtr root, const char * name, vector< bool > & vec );
xmlNodePtr AddVectorIntNode( xmlNodePtr root, const char * name, vector< int > & vec );
xmlNodePtr AddVectorDoubleNode( xmlNodePtr root, const char * name, vector< double > & vec );
xmlNodePtr AddVectorVec3dNode( xmlNodePtr root, const char * name, vector< vec3d > & vec );

vector< bool >   ExtractVectorBoolNode( xmlNodePtr root, const char * name );
vector< int >    ExtractVectorIntNode( xmlNodePtr root, const char * name );
vector< double > ExtractVectorDoubleNode( xmlNodePtr root, const char * name );
vector< vec3d > ExtractVectorVec3dNode( xmlNodePtr root, const char * name );
vector< double > GetVectorDoubleNode( xmlNodePtr node );
vector< vec3d > GetVectorVec3dNode( xmlNodePtr node );

xmlNodePtr EncodeFileContents( xmlNodePtr root, const char* file_name );
xmlNodePtr DecodeFileContents( xmlNodePtr root, const char* file_name );

string ConvertToXMLSafeChars( const string & input );
string ConvertFromXMLSafeChars( const string & input );

}


#endif

