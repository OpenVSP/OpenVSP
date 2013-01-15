//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Mesh Geometry Class
//  
// 
//   J.R. Gloudemans - 11/7/94
//   Sterling Software
//
//    
//******************************************************************************

#ifndef TRI_TEST_H
#define TRI_TEST_H


#include "vec2d.h"
#include "vec3d.h"
#include "bbox.h"
#include "stringc.h"
#include "DragFactors.h"

#include <vector>				//jrg windows??	
#include <algorithm>			//jrg windows??	
using namespace std;			//jrg windows??

extern "C"
{ void triangulate(const char *, struct triangulateio *, struct triangulateio *,
				   struct triangulateio *); }

class TEdge;
class TTri;
class TBndBox;
class NBndBox;
class TNodeGroup;

class TetraMassProp
{
public:
	TetraMassProp(int id, double den, vec3d& p0, vec3d& p1, vec3d& p2, vec3d& p3);
	TetraMassProp()			{}						
	~TetraMassProp()		{}

	void SetPointMass( double massIn, vec3d& posIn );			// For Point Mass

	bool pointMassFlag;

	vec3d v0;
	vec3d v1;
	vec3d v2;
	vec3d v3;

	int compId;

	vec3d cg;

	double den;
	double vol;
	double mass;

	double Ixx;
	double Iyy;
	double Izz;

	double Ixy;
	double Ixz;
	double Iyz;
};



class TriShellMassProp
{
public:
	TriShellMassProp(int id, double mass_area_in, vec3d& p0, vec3d& p1, vec3d& p2);
	~TriShellMassProp()		{}

	vec3d v0;
	vec3d v1;
	vec3d v2;

	vec3d cg;

	int compId;

	double mass_area;
	double tri_area;
	double mass;

	double Ixx;
	double Iyy;
	double Izz;

	double Ixy;
	double Ixz;
	double Iyz;
};

class TNode
{
public:
/*
	TNode()			{ static int cnt = 0;	cnt++;
						printf("TNode Construct Cnt = %d \n", cnt); }
	~TNode()		{ static int cnt = 0;	cnt++;
						printf("TNode Destruct Cnt = %d \n", cnt); }
*/
	TNode();
	virtual ~TNode();

	vec3d pnt;
	vec3d norm;
	int id;

	vector< TTri* > triVec;				// For WaterTight Check
	vector< TEdge* > edgeVec;			// For WaterTight Check

	vector< TNode* > mergeVec;

	vector< TNode* > splitNodeVec;

//	TNode* mapNode;

	int isectFlag;
};

class TNodeGroup
{
public:
	vector< TNode* > nVec;
};

class TEdge
{
public:
	TEdge()					{ n0 = n1 = 0; }
	virtual ~TEdge()		{}

	TNode* n0;
	TNode* n1;

	TTri* tri0;							// For WaterTight Check
	TTri* tri1;

};



class TTri
{
public:
	TTri();
	virtual ~TTri();

	TNode* n0;
	TNode* n1;
	TNode* n2;

	vec3d norm;

	vector< TEdge* > iSectEdgeVec;			// List of Intersection Edges
	vector< TTri* > splitVec;				// List of split tris
	vector< TNode* > nVec;					// Nodes for split tris
	vector< TEdge* > eVec;					// Edges for split tris

	virtual void splitTri(int meshFlag = 0);				// Split Tri to Fit ISect Edges 
	virtual void triangulateSplit( int flattenAxis );
	virtual void niceTriSplit( int flattenAxis );
	virtual double computeArea()			{	return area(n0->pnt, n1->pnt, n2->pnt); }
	virtual double computeAwArea()	
	{
		vec3d t1,t2,t3;
		t1.set_xyz(0.0,n0->pnt.y(),n0->pnt.z());
		t2.set_xyz(0.0,n1->pnt.y(),n1->pnt.z());
		t3.set_xyz(0.0,n2->pnt.y(),n2->pnt.z());
		return area(t1, t2, t3); 
	}
	virtual void draw();
	virtual void loadNodesFromEdges();
	virtual void computeCosAngles( double* ang0, double* ang1, double* ang2 );

	virtual void splitEdges( TNode* n01, TNode* n12, TNode* n20 );

	virtual vec3d compNorm();
	virtual bool  shareEdge( TTri* t );
	virtual bool matchEdge( TNode* n0, TNode* n1, TNode* nA, TNode* nB, double tol );

	int interiorFlag;
	int id;
	double mass;
	int invalidFlag;

	TEdge* e0;
	TEdge* e1;
	TEdge* e2;

	TNode* cn0;					// Nodes on Edge Centers for FEA Export
	TNode* cn1;
	TNode* cn2;


private:

	virtual int onEdge( vec3d & p, TEdge* e, double onEdgeTol );
	virtual int dupEdge( TEdge* e0, TEdge* e1, double tol);

};

class TBndBox
{
public:
	TBndBox();
	virtual ~TBndBox();

	bbox box;
	vector< TTri* > triVec;

	TBndBox* sBoxVec[8];		// Split Bnd Boxes

	void splitBox();
	void addTri( TTri* t );
	void draw();
	virtual void intersect( TBndBox* iBox );
	virtual void numCrossXRay( vec3d & orig, vector<double> & tParmVec );
	virtual void addLeafNodes( vector< TBndBox* > & leafVec );

	virtual void segIntersect( vec3d & p0, vec3d & p1, vector< vec3d > & ipntVec );

};

class NBndBox
{
public:
	NBndBox();
	virtual ~NBndBox();

	bbox box;
	vector< TNode* > nodeVec;

	NBndBox* sBoxVec[8];		// Split Bnd Boxes

	void splitBox(double maxSize);
	void addNode( TNode* n );
	void draw();
	virtual void addLeafNodes( vector< NBndBox* > & leafVec );

};
class Geom;

class TMesh
{
public:
	TMesh();
	virtual ~TMesh();

	vector< TTri* >  tVec;
	vector< TNode* > nVec;
	vector< TEdge* > eVec;

	TBndBox tBox;

	void copy( TMesh* m );

	//==== Stuff Copied From Geom That Created This Mesh ====//
	int ptr_id;
	bool reflected_flag;
	Stringc name_str;
	int materialID;
	vec3d color;
	int massPrior;
	double density;
	double shellMassArea;
	int shellFlag;
	DragFactors drag_factors;

	double theoArea;
	double wetArea;
	double theoVol;
	double guessVol;
	double wetVol;

	void loadGeomAttributes( Geom* geomPtr );
	int  removeDegenerate();
	void intersect( TMesh* tm );
	void split(int meshFlag = 0);
	void deterIntExt( vector< TMesh* >& meshVec );
	void deterIntExtTri( TTri* tri, vector< TMesh* >& meshVec );
	void massDeterIntExt( vector< TMesh* >& meshVec );
	void massDeterIntExtTri( TTri* tri, vector< TMesh* >& meshVec );

	int deterIntExtPnt( const vec3d& pnt, vector< TMesh* >& meshVec, TMesh* ignoreMesh = 0 );	// 1 Interior 0 Exterior

	void loadBndBox();

	virtual void draw();
	virtual void draw_wire();
//	virtual void draw_hidden(); 
    virtual void draw_shaded(); 

	virtual double computeTheoArea();
	virtual double computeWetArea();
	virtual double computeAwaveArea();
	virtual double computeTheoVol();
	virtual double computeTrimVol();

	virtual void addTri( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & norm );
	virtual void addTri( TNode* node0, TNode* node1, TNode* node2, const vec3d & norm );

	virtual void write_stl_tris( int sym_code_in, float mat[4][4], float refl_mat[4][4], FILE* file_id);

	virtual vec3d getVertex(int index);
	virtual int   numVerts();

	virtual void waterTightCheck(FILE* fid, vector< TMesh* > & tMeshVec );
	virtual TNode* lowNode( TNode* node );
	virtual void findEdge( TNode* node, TTri* tri0, TTri* tri1 );
	virtual void addEdge( TTri* tri0, TTri* tri1, TNode* node0, TNode* node1 );
	virtual void swapEdge( TEdge* edge );
	
	bool deleteMeFlag;
	vector< TTri* > nonClosedTriVec;
	virtual void mergeNonClosed( TMesh* tm );
	virtual void checkIfClosed();

	virtual void matchNodes();
	virtual void checkValid(FILE* fid);
	virtual void swapEdges( double size );
	virtual void relaxMesh(vector< TMesh* > & tMeshVec);
	virtual vec3d projectOnISectPairs( vec3d & offPnt, vector< vec3d > & pairVec );

	virtual void meshStats(double* minEdgeLen, double* minTriAng, double* maxTriAng );
	virtual void tagNeedles(double minTriAng, double minAspectRatio, int delFlag );
	virtual void moveNode( TNode* n0, TNode* n1 );
	virtual TTri* findTriNodes( TTri* ignoreTri, TNode* n0, TNode* n1 );
	virtual TTri* findTriPnts( TTri* ignoreTri, TNode* n0, TNode* n1 );

	virtual void mergeSplitNodes( vector< TTri* > & triVec, vector< TNode* > & nodeVec );
	static TNode* checkDupOrAdd( TNode* node, vector< TNode* > & nodeVec, double tol = 0.00000001 );
	static TNode* checkDupOrCreate( vec3d & p, vector< TNode* > & nodeVec, double tol = 0.00000001 );

	static void stressTest();
	static double rand01();

	vector< vec3d > vertVec;
	vector< vec3d > isectPairs;

	bool halfBoxFlag;

};




#endif
