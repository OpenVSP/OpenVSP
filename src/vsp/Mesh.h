//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// Mesh.h
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(MESH_MESH__INCLUDED_)
#define MESH_MESH__INCLUDED_

#include "vec2d.h"
#include "vec3d.h"
#include "Tri.h"

class Surf;
class GridDensity;

#ifndef WIN32
#define NDEBUG
#endif

#include <assert.h>

#include <vector>
#include <list>
using namespace std;

extern "C"
{ void triangulate(char *, struct triangulateio *, struct triangulateio *,
				   struct triangulateio *); }

class MeshSeg
{
public:
	int m_Index[2];
};

//////////////////////////////////////////////////////////////////////
class Mesh
{
public:

	Mesh();
	virtual ~Mesh();

	void Clear();

	void Draw();

	void Remesh();
	void LoadSimpTris();
	void CondenseSimpTris();
	int CheckDupOrAdd( int ind, map< int, vector< int > > & indMap, vector< vec3d > & pntVec );


	int Split( int num_iter );
	void SplitEdge( Edge* edge );

	bool ThreeEdgesThreeTris( Edge* edge );
	void SwapEdge( Edge* edge );

	int Collapse(int num_iter);
	bool ValidCollapse( Edge* edge );
	void CollapseEdge( Edge* edge );

	void SmoothTargetEdgeLength();
	void SmoothTargetEdgeLength( Edge* e );
	void SmoothTargetEdgeLength( Node* n );
	void SmoothTargetEdgeLength( Edge* e, Node* notn );

	void ComputeTargetEdgeLength( Edge* edge );

	void SetNodeFlags();

	void LaplacianSmooth(int num_iter);
	void OptSmooth(int num_iter);
	void FindNodeTris();

	void DumpGarbage();

	void AdjustEdgeLengths();

	void CheckValidEdge(Edge* e);
	void CheckValidAllEdges();

	Node* AddNode( vec3d p, vec2d uw_in );
	void  RemoveNode( Node* nptr );
	Node* FindNode( const vec3d& p );

	bool ValidNodeMove( Node* nptr, vec3d & move_to, Tri* ignoreTri = NULL );

	Edge* AddEdge( Node* n0, Node* n1 );
	void  RemoveEdge( Edge* eptr );
	Edge* FindEdge( Node* n0, Node* n1 );

	Tri* AddTri( Node* nn0, Node* nn1, Node* nn2, Edge* ee0, Edge* ee1, Edge* ee2 );
	void  RemoveTri( Tri* tptr );

	void TriangulateBorder( vector< vec3d > uw_border );	

	void InitMesh( vector< vec2d > & uw_points, vector< MeshSeg > & segs_indexes );

	void CheckValidTriInput( vector< vec2d > & uw_points, vector< MeshSeg > & segs_indexes );


	void ReadSTL( const char* file_name );
	void WriteSTL( const char* file_name );
	void WriteSTL( FILE* fp );

	void SetSurfPtr( Surf* sptr )							{ m_Surf = sptr; }
	void SetGridDensityPtr ( GridDensity* gptr )			{ m_GridDensity = gptr; }

	void HighlightNextNode()		{ m_HighlightNodeIndex = (m_HighlightNodeIndex+1)%(int)nodeList.size(); }
	void HighlightNextEdge()		{ m_HighlightEdgeIndex = (m_HighlightEdgeIndex+1)%(int)edgeList.size(); }

	void CollapseHighlightEdge();

	void ColorTris();

	list <Tri*> GetTriList()					{ return triList; }

	vector < vec3d >& GetSimpPntVec()		{ return simpPntVec; }
	vector < SimpTri >& GetSimpTriVec()		{ return simpTriVec; }

	void RemoveInteriorTrisEdgesNodes();

	int m_Iteration;

protected:

	Surf* m_Surf;
	GridDensity* m_GridDensity;

	list < Tri* > triList;
	list < Edge* > edgeList;
	list < Node* > nodeList;

	vector< Tri* > garbageTriVec;
	vector< Edge* > garbageEdgeVec;
	vector< Node* > garbageNodeVec;

	int m_TotalIterations;
	int m_HighlightNodeIndex;
	int m_HighlightEdgeIndex;

	vector< vec3d > simpPntVec;
	vector< SimpTri > simpTriVec;

};


#endif 
