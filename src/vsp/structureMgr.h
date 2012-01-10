//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Structure Mgr Class 
// 
//   J.R. Gloudemans - 10/22/06
//
//    
//******************************************************************************

#ifndef STRUCTURE_MGR_STRUCTURE_MGR_H
#define STRUCTURE_MGR_STRUCTURE_MGR_H

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "xmlvsp.h"
#include "tMesh.h"

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
#include <vector>	

using namespace std;			
class Aircraft;
class Part;


class StructureMgr
{
public:

	StructureMgr();
	virtual ~StructureMgr();

	void SetAircraftPtr( Aircraft* aptr )			{ aircraftPtr = aptr; }

	void setCurrGeomID( int id )					{ currGeomID = id; EditCurrPart(); TriggerUpdate(); }
	int  getCurrGeomID()							{ return currGeomID; }

	void SetActiveFlag( bool flag );
	bool GetActiveFlag()							{ return activeFlag; }

	virtual void SetExportFileName( const char* str )				{ exportFileName = str; }
	virtual Stringc GetExportFileName()								{ return exportFileName; }

	void ForceDraw();

	void OneSecondUpdate();

	void CreateAddPart( int part_type );
	Part* CreatePart( int part_type );

	void DeleteCurrPart();

	void EditCurrPart();
	Part* GetCurrPart();
	vector< Part* > GetAllParts();

	void TriggerUpdate();
	bool GetUpdateFlag()							{ return updateFlag; }

	vector<Stringc> GetPartNames();

	void GUI_Val( Stringc name, double val );
	void GUI_Val( Stringc name, int val );
	void GUI_Val( Stringc name, Stringc val );

	void Draw();

	void FinalMesh();

	void SetFinalTriArea( double a )				{ finalTriArea = a; }
	double GetFinalTriArea()						{ return finalTriArea; }

	void FindIntersection( Part* p0, Part* p1 );

	void CheckValidity( vector< TNode* > & nVec, vector< TTri > & tVec );

	vector < Stringc > & GetFinalResultStringVec()	{ return finalResultStringVec; }


private:

	Aircraft* aircraftPtr;

	int currGeomID;

	bool activeFlag;

	bool updateFlag;

	Stringc exportFileName;
	double finalTriArea;

	vector< vec3d > oneTriEdgeVec;
	vector< vec3d > twoTriEdgeVec;
	vector< vec3d > fourTriEdgeVec;
	vector< vec3d > otherTriEdgeVec;

	int finalNumTris;
	int finalNumEdges;

	enum { MIN_VAL, MAX_VAL, AVG_VAL };
	double finalArea[3];			// Min, Max, Avg
	double finalEdgeLen[3];
	double finalAngle[3];	

	vector < Stringc > finalResultStringVec;

	vector< vec3d > centerVec;
	vector< vec3d > normVec;

	vector< vec3d > smallTris;

};


class SM_Single
{
public:
	SM_Single();
	StructureMgr* structureMgr;
};


static SM_Single singleSM;

#define structureMgrPtr (singleSM.structureMgr)












   

#endif



