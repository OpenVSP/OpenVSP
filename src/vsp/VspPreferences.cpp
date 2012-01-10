//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// materialMgr.cpp: implementation of the materialMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "VspPreferences.h"
#include "materialMgr.h"

VspPreferences* VspPreferences::instance = 0;

VspPreferences* VspPreferences::Instance()
{
	if (instance == 0)
	{
		instance = new VspPreferences;
	}
	return instance;
}

void VspPreferences::DeleteInstance()
{
	if ( instance )
		delete instance;

	instance = 0;
}

VspPreferences::VspPreferences()
{
	outputNameVec.push_back( Stringc("No_Default") );

	m_DefaultCompFileVec.clear();
	DefaultCompFile dcf;
	dcf.m_Name = Stringc("Default");
	dcf.m_File = Stringc("");
	m_DefaultCompFileVec.push_back( dcf );

//	xmlDocPtr doc;

//	LIBXML_TEST_VERSION
	xmlKeepBlanksDefault(0);

	//==== Build an XML tree from a the file ====//
	m_XmlDoc = xmlParseFile("VspPreferences.xml");
	if (m_XmlDoc == NULL) return;

	xmlNodePtr root = xmlDocGetRootElement(m_XmlDoc);
	if (root == NULL)
	{
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(m_XmlDoc);
		return;
	}

	if ( xmlStrcmp( root->name, (const xmlChar *)"Vsp_Preferences" ) )
	{
		fprintf(stderr, "document of the wrong type, VSP Preferences Name list not found\n");
		xmlFreeDoc(m_XmlDoc);
		return;
	}

	m_DefaultCompFileID = xmlFindInt(root, "DefaultComponentFileID", 0);

	xmlNodePtr defaultCompListNode = xmlGetNode(root, "DefaultComponentFileList", 0);
	if (defaultCompListNode)
	{
		//==== Read Default Comp Files ====//
		int num_def_files = xmlGetNumNames( defaultCompListNode, "DefaultComponentFile" );
		for ( int i = 0 ; i < num_def_files ; i++ )
		{
			xmlNodePtr def_file_node = xmlGetNode( defaultCompListNode, "DefaultComponentFile", i );
			if ( def_file_node )
			{
				dcf.m_Name = Stringc( xmlFindString( def_file_node, "Name", "Default_Name" ) );
				dcf.m_File = Stringc( xmlFindString( def_file_node, "File", "Default_File" ) );
				m_DefaultCompFileVec.push_back( dcf );
			}
		}
	}

	xmlNodePtr outputNames = xmlGetNode(root, "Vsp_OutputNames", 0);
	if (outputNames)
	{
		//==== Read Names ====//
		int num_names = xmlGetNumNames( outputNames, "OutputName" );
		for ( int i = 0 ; i < num_names ; i++ )
		{
			xmlNodePtr name_node = xmlGetNode( outputNames, "OutputName", i );
			char* str = xmlFindString( name_node, "Value", "Default_Name" );
			outputNameVec.push_back( Stringc(str) );
		}
	}

	xmlNodePtr materialDefines = xmlGetNode(root, "Vsp_Material_Defines", 0);
	if (materialDefines)
	{
		matMgrPtr->readFile(materialDefines);
	}

	windowMgrStyle = xmlFindInt(root, "WindowManagerStyle", 0);

}

VspPreferences::~VspPreferences()
{
	if ( m_XmlDoc )
		xmlFreeDoc(m_XmlDoc);
}

void VspPreferences::saveFile()
{
	xmlNodePtr root = xmlDocGetRootElement(m_XmlDoc);
	if (root == NULL)
		return;
		
	xmlNodePtr node = xmlGetNode( root, "DefaultComponentFileID", 0 );
//	xmlAddIntNode( root, "DefaultComponentFileID", m_DefaultCompFileID );

	char str[16];
	sprintf(str, "%d", m_DefaultCompFileID );
	xmlNodeSetContent( node, (const xmlChar *)str );

	xmlSaveFormatFile("VspPreferences.xml", m_XmlDoc, 1);
}

Stringc VspPreferences::getOutputName( int i )
{
	if ( i < 0 || i >= (int)outputNameVec.size() )
		return outputNameVec[0];

	return outputNameVec[i];
}

void VspPreferences::setDefaultCompFileID( int id )
{
	m_DefaultCompFileID = id;
	saveFile();
}

