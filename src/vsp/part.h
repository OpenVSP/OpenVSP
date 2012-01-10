//******************************************************************************
//    
//   Part Class
//  
// 
//   J.R. Gloudemans - 9/24/06
//
//    
//******************************************************************************

#ifndef PART_PART_H
#define PART_PART_H


#include "vec2d.h"
#include "vec3d.h"
#include "bbox.h"
#include "stringc.h"
#include "xmlvsp.h"


#include "tMesh.h"


#include <vector>				
#include <algorithm>			
using namespace std;	

class Geom;
class PEdge;

class PNode
{
public:
	int id;
	vec3d pnt;
	vec2d uw;
	vector< PEdge* > edgeVec;
};

class PEdge
{
public:
	PNode* n0;
	PNode* n1;

	double length()			{ return dist( n0->pnt, n1->pnt ); }
};

class ISectNode
{
public:
	vec3d pnt;
	PEdge* edge;
};

class Perimeter
{
public:
	Perimeter()							{}
	virtual ~Perimeter();

	virtual void clear();
	virtual void load( 	vector< TNode* > & nodeVec, vector< TEdge* > edgeVec );
	virtual PNode* insertPnt( PEdge* edge, vec3d & pnt );
	virtual void addSplitIntersections(double targetLength);

	virtual void removeSmallEdges(double percent);

	virtual void writeToFile( const char* filename );

	vector< PNode* > nVec;
	vector< PEdge* > eVec;

	vector< PEdge* > isectEdgeVec;

};



class Part
{
public:
	Part();
	virtual ~Part();
	virtual void Copy( Part* fromPart );

	virtual void SetName( const char* str )					{ name = str; }
	virtual Stringc GetName()								{ return name; }

	virtual void Draw();
	virtual void UpdateOneSecond()							{}
	virtual void SetGeomPtr( Geom* g )						{ geomPtr = g; }

	virtual int  GetType()									{ return type; }

	virtual void SetEditFlag( bool f )						{ editFlag = f; }
	virtual void SetStaleFlag( bool f )						{ staleFlag = f; }

	virtual void DeleteFinalMesh();

	virtual void NiceTriangulate( double size, bool finalFlag ) = 0;
	virtual void FindPerimeter() = 0;
	virtual void MergeNodesRemoveEdges( vector< TNode* > & NodeVec, vector< TEdge* > & EdgeVec, double tol );

	virtual TMesh* GetSliceMesh()							{ return sliceMesh; }
	virtual TMesh* GetFinalMesh()							{ return finalMesh; }

	virtual vec3d GetOrig() = 0;
	virtual vec3d GetAxis0() = 0;
	virtual vec3d GetAxis1() = 0;

	virtual void WriteParms( xmlNodePtr node ) = 0;
	virtual void ReadParms( xmlNodePtr root )  = 0;

	enum{ THREE_PNT_SLICE, RIB, SPAR, NUM_SLICE_TYPES, };

	Perimeter perim;

protected:

	int type;
	Stringc name;
	Geom* geomPtr;

	TMesh* sliceMesh;
	TMesh* finalMesh;

	bool staleMesh;
	bool staleFlag;
	bool editFlag;

};


class ThreePntSlice : public Part
{
public:
	ThreePntSlice();		
	virtual ~ThreePntSlice();
	virtual void Copy( Part* fromPart );

	virtual void SetUPnt( double u, int ind );
	virtual void SetWPnt( double u, int ind );
	virtual vec2d GetUWPnt( int ind );

	virtual void ComputePlanePnts();
	TMesh* ComputeTMesh(); 

	virtual void Draw();
	virtual void UpdateOneSecond();

	virtual void FindPerimeter();
	virtual void NiceTriangulate(double size, bool finalFlag);

	virtual vec3d GetOrig()				{ return plnPnt[0]; }
	virtual vec3d GetAxis0()			{ vec3d axis; axis = plnPnt[1] - plnPnt[0]; axis.normalize(); return axis; }
	virtual vec3d GetAxis1()			{ vec3d axis; axis = plnPnt[3] - plnPnt[0]; axis.normalize(); return axis; }

	virtual void WriteParms( xmlNodePtr node );
	virtual void ReadParms( xmlNodePtr node );

protected:


	vec2d uwPnt[3];
	vec3d projPnt[3];
	vec3d plnPnt[4];

};


class Rib : public ThreePntSlice
{
public:
	Rib();		
	virtual ~Rib();
	virtual void Copy( Part* fromPart );

	virtual void SetPercentSpan( double p );
	virtual double GetPercentSpan()							{ return percentSpan; }
	virtual void WriteParms( xmlNodePtr node );
	virtual void ReadParms( xmlNodePtr node );

protected:

	double percentSpan;

};

class Spar : public ThreePntSlice
{
public:
	Spar();		
	virtual ~Spar();
	virtual void Copy( Part* fromPart );

	virtual void SetPercentChord( double p );
	virtual double GetPercentChord()						{ return percentChord; }
	virtual void WriteParms( xmlNodePtr node );
	virtual void ReadParms( xmlNodePtr node );

protected:

	double percentChord;

};




#endif
