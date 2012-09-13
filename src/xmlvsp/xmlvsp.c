//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "xmlvsp.h"
#include <string.h>

/* jrg - change this stuff to use hash tables */

/*==== Get Number of Same Names ====*/
int xmlGetNumNames( xmlNodePtr node, const char * name )
{
  int num;
  xmlNodePtr iter_node;

  num = 0;
  iter_node = node->xmlChildrenNode;

  //==== Parse This Level ====//
  while( iter_node != NULL )
  {
    if ( !xmlStrcmp( iter_node->name, name ) )
      num++;
    iter_node = iter_node->next;
  }

  return num;
}

/*==== Get Node w/ Name and ID (Seq Num 0 - n ) ====*/
xmlNodePtr xmlGetNode( xmlNodePtr node, const char * name, int id )
{
  int num;
  xmlNodePtr iter_node;

  if ( node == NULL )
    return NULL;

  num = 0;
  iter_node = node->xmlChildrenNode;

  //==== Parse This Level ====//
  while( iter_node != NULL )
  {
    //SR addition to read in external XML files
	  if (name==NULL) {
		  return iter_node;
	  }
	  else {

	  if ( !xmlStrcmp( iter_node->name, name ) )
    {
      if ( id == num )
        return iter_node;

      num++;
    }
	  }
    iter_node = iter_node->next;
  }
  printf("xmlGetNode: Cant find %s in node %s\n", name, node->name );
  return NULL;
}

double xmlExtractDouble( xmlNodePtr node )
{
  char* str = xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
  double val = 0.0;
  if ( str )
  {
	val = atof( str );
	xmlFree( str );
  }
  return val;
}

int xmlExtractInt( xmlNodePtr node )
{
  char* str = xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
  int val = 0;
  if ( str )
  {
	val = atoi( str );
	xmlFree( str );
  }
  return val;
}

char*  xmlExtractString( xmlNodePtr node )
{
	char* str = xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
	char* val;
	if ( str )
	{
		val = strdup (str);
		xmlFree( str );
	}
	else
	{
		val = strdup( " " );
	}

	return val;
}

double xmlFindDouble( xmlNodePtr node, const char * name, double def )
{
  xmlNodePtr n;

  if ( node == NULL )
    return def;

  n = xmlGetNode( node, name, 0 );
  if ( n )
    return xmlExtractDouble( n );

  return def;
}

int xmlFindInt( xmlNodePtr node, const char * name, int def )
{
  xmlNodePtr n;

  if ( node == NULL )
    return def;

  n = xmlGetNode( node, name, 0 );
  if ( n )
    return xmlExtractInt( n );

  return def;
}

char* xmlFindString( xmlNodePtr node, const char * name, char* def )
{
  xmlNodePtr n;

  if ( node == NULL )
    return def;

  n = xmlGetNode( node, name, 0 );
  if ( n )
  {
    char* ret = xmlExtractString( n );
    if ( ret )
      return ret;
  }

  return def;
}

double xmlFindPropDouble( xmlNodePtr node, const char * name, double def )
{
  char* str;
  if ( node == NULL )
    return def;

  str = xmlGetProp( node, name );
  if ( str )
  {
	def = atof( str );
	xmlFree( str );
  }

  return def;
}

char* xmlFindPropString( xmlNodePtr node, const char * name, char* def )
{
  char* ret;
  if ( node == NULL )
    return def;

  ret = xmlGetProp( node, name );
  if ( ret )
      return ret;

  return def;
}

xmlNodePtr xmlAddIntNode( xmlNodePtr root, const char * name, int val )
{
  char str[255];
  xmlNodePtr node = xmlNewChild( root, NULL, (const xmlChar *)name, NULL );

  sprintf( str, "%d\0", val );
	xmlNodeSetContent(node, (const xmlChar *)str);

  return node;
}

xmlNodePtr xmlAddDoubleNode( xmlNodePtr root, const char * name, double val )
{
  char str[255];
  xmlNodePtr node = xmlNewChild( root, NULL, (const xmlChar *)name, NULL );

  sprintf( str, "%lf\0", val );
	xmlNodeSetContent(node, (const xmlChar *)str);

  return node;
}

xmlNodePtr xmlAddStringNode( xmlNodePtr root, const char * name, const char * val )
{
  xmlNodePtr node = xmlNewChild( root, NULL, (const xmlChar *)name, NULL );
	xmlNodeSetContent(node, (const xmlChar *)val);

  return node;
}

void xmlSetDoubleProp( xmlNodePtr root, const char * name, double val )
{
  char str[255];

  sprintf( str, "%lf\0", val );
  xmlSetProp( root, (const xmlChar *)name, (const xmlChar *)str );
}

int xmlGetNumArray( xmlNodePtr node, const char sep )
{
  char* str;
  int i, len, num, elemcnt;

  if ( node == NULL )
    return 0;

  num = 0;
  elemcnt = 0;
  str = xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
  len = strlen( str );

  i = num = elemcnt = 0;
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

int xmlExtractDoubleArray( xmlNodePtr node, const char sep, double * array, int size )
{
  char elem[255];
  char* str;
  int i, len, num, elemcnt;

  if ( node == NULL )
    return 0;

  num = 0;
  elemcnt = 0;
  str = xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
  len = strlen( str );

  i = num = elemcnt = 0;
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

int xmlExtractIntArray( xmlNodePtr node, const char sep, int * array, int size )
{
  char elem[255];
  char* str;
  int i, len, num, elemcnt;

  if ( node == NULL )
    return 0;

  num = 0;
  elemcnt = 0;
  str = xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
  len = strlen( str );

  i = num = elemcnt = 0;
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
    array[num] = (int)atof( elem );
    num++;
  }
  xmlFree( str );
  return num;
}














