// materialMgr.h: interface for the materialMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VSPPREFERENCES_H)
#define AFX_VSPPREFERENCES_H

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "xmlvsp.h"


#include <stdio.h>



#ifdef WIN32
#include <windows.h>		
#endif

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include "stringc.h"
#include <vector>				//jrg windows??	
using namespace std;			//jrg windows??

class DefaultCompFile
{
public:
	Stringc m_Name;
	Stringc m_File;
};


class VspPreferences
{
public:
	static VspPreferences* Instance();
	static void DeleteInstance();

	int  getNumOutputNames()				{ return outputNameVec.size(); }
	Stringc getOutputName(int i);
	int windowMgrStyle;

	int getDefaultCompFileID()							{ return m_DefaultCompFileID; }
	void setDefaultCompFileID( int id );
	vector< DefaultCompFile > getDefaultCompFileVec()	{ return m_DefaultCompFileVec; }

	void saveFile();

protected:
	VspPreferences();
	virtual ~VspPreferences();
private:
	static VspPreferences* instance;
	vector< Stringc > outputNameVec;

	xmlDocPtr m_XmlDoc;

	int m_DefaultCompFileID;
	vector< DefaultCompFile > m_DefaultCompFileVec;
};

#endif



