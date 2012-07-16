//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

/***************************************/
/*          Misc XML Utilities         */
/*          J.R. Gloudemans            */
/***************************************/


#ifndef __XMLVSP_H__
#define __XMLVSP_H__

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/hash.h>

#ifdef __cplusplus
extern "C" {
#endif

int xmlGetNumNames( xmlNodePtr node, const char * name );
xmlNodePtr xmlGetNode( xmlNodePtr node, const char * name, int num );

double xmlExtractDouble( xmlNodePtr node );
int    xmlExtractInt( xmlNodePtr node );
char*  xmlExtractString( xmlNodePtr node );

double xmlFindDouble( xmlNodePtr node, const char * name, double def );
int    xmlFindInt( xmlNodePtr node, const char * name, int def );
char*  xmlFindString( xmlNodePtr node, const char * name, char* def );

double xmlFindPropDouble( xmlNodePtr node, const char * name, double def );
char* xmlFindPropString( xmlNodePtr node, const char * name, char* def );

int xmlGetNumArray( xmlNodePtr node, const char delim );

int xmlExtractDoubleArray( xmlNodePtr node, const char sep, double * arr, int size );
int xmlExtractIntArray( xmlNodePtr node, const char sep, int * arr, int size );

xmlNodePtr xmlAddIntNode( xmlNodePtr root, const char * name, int val );
xmlNodePtr xmlAddDoubleNode( xmlNodePtr root, const char * name, double val );
xmlNodePtr xmlAddStringNode( xmlNodePtr root, const char * name, const char * val );

void xmlSetDoubleProp( xmlNodePtr root, const char * name, double val );

#ifdef __cplusplus
}
#endif

#endif /* __XMLVSP_H__ */
