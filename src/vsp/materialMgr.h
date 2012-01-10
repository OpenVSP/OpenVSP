// materialMgr.h: interface for the materialMgr class.
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MATERIALMGR_H__635B9760_A025_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_MATERIALMGR_H__635B9760_A025_11D7_AC31_0003473A025A__INCLUDED_

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


class Material
{
public:
	Material();
	virtual ~Material()		{}

	Stringc name;
	float amb[4];
	float diff[4];
	float spec[4];
	float emiss[4];
	float shine;

	float alpha;
	int alphaFlag;

	void read( xmlNodePtr mat_node );
	void bind();

};


class MaterialMgr
{
public:

	MaterialMgr();
	virtual ~MaterialMgr();

	virtual void readFile( xmlNodePtr mat_node );

	int  getNumMaterial()				{ return matVec.size(); }
	Material* getMaterial(int i);

	Material getMaterialCopy(int i);

	Material getWhiteMaterial( float brightness, float shine );



private:

	vector< Material* > matVec;
};


class Single
{
public:
	Single();
	MaterialMgr* matMgr;
};


static Single single;

#define matMgrPtr (single.matMgr)












   

#endif



