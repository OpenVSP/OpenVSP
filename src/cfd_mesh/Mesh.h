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

#include "Vec2d.h"
#include "Vec3d.h"
#include "Face.h"

class Surf;
class SimpleGridDensity;
class SurfaceIntersectionSingleton;

#include <cassert>

#include <vector>
#include <list>
#include <unordered_map>
using namespace std;


class MeshSeg
{
public:
    int m_Index[2];
    vec2d m_UWmid;
    vec3d m_P[2];
    vec3d m_Pmid;
};

//////////////////////////////////////////////////////////////////////
class Mesh
{
public:

    Mesh();
    virtual ~Mesh();

    void Clear();

    // void Draw();

    void Remesh();
    void LoadSimpFaces();
    void CondenseSimpFaces();


    int Split( int num_iter );
    void SplitEdge( Edge* edge );

    static bool ThreeEdgesThreeFaces( Edge* edge );
    void SwapEdge( Edge* edge );

    int Collapse( int num_iter );
    static bool ValidCollapse( Edge* edge );
    void CollapseEdge( Edge* edge );

    // Is this face wound against the surface it lies on?
    bool FaceReversed( Face* f );

    // Would a triangle on these three points be wound against the surface?  Asked of points
    // that do not exist yet, so a step can decline to build a face it would have to undo.
    bool TriReversed( const vec3d &p0, const vec3d &p1, const vec3d &p2,
                      const vec2d &uw0, const vec2d &uw1, const vec2d &uw2 );
    int RemoveRevFaces();

    void LimitTargetEdgeLength();
    void LimitTargetEdgeLength( Edge* e );
    void LimitTargetEdgeLength( Node* n );
    void LimitTargetEdgeLength( Edge* e, Node* notn );

    void ComputeTargetEdgeLength( Edge* edge );
    void ComputeTargetEdgeLength( Node* n );

    void SetNodeFlags();

    void LaplacianSmooth( int num_iter );
    void OptSmooth( int num_iter );

    bool SetFixPoint( const vec3d &fix_pnt, vec2d fix_uw );

    void DumpGarbage();

    void AdjustEdgeLengths();

    static void CheckValidEdge( Edge* e );
    void CheckValidAllEdges();

    Node* AddNode( const vec3d &p, const vec2d &uw_in );
    void  RemoveNode( Node* nptr );
    Node* FindNode( const vec3d& p );

    static bool ValidNodeMove( Node* nptr, const vec3d & move_to, Face* ignoreFace = nullptr );

    Edge* AddEdge( Node* n0, Node* n1 );
    void  RemoveEdge( Edge* eptr );
    Edge* FindEdge( Node* n0, Node* n1 );

    Face* AddFace( Node* nn0, Node* nn1, Node* nn2, Edge* ee0, Edge* ee1, Edge* ee2 );
    Face* AddFace( Node* nn0, Node* nn1, Node* nn2, Node* nn3, Edge* ee0, Edge* ee1, Edge* ee2, Edge* ee3 );
    void  RemoveFace( Face* fptr );

    void InitMesh( vector< vec2d > & uw_points, vector< MeshSeg > & segs_indexes, SurfaceIntersectionSingleton *MeshMgr );

    static vector< int > RandomizePointOrder( vector< vec2d > & uw, vector< MeshSeg > & segs, unsigned int seed );
    static void RandomizeSegOrder( vector< MeshSeg > & segs, unsigned int seed );

    // relax 0 asks for the quality mesh, 1 drops the minimum angle, 2 drops the size limit as
    // well.  areascale nudges the size limit, which moves every added point and is what gets a
    // surface past a robustness limit -- unlike the order the points are given in.
    bool InitMesh_TRI( const vector< vec2d > & uw_prime, const vector< MeshSeg > & segs_indexes,
                       vector< vector< int > > & connlist, vector< vec2d > & points_out, int relax = 0, double areascale = 1.0 );
    // spacing seeds the inside of the patch, so a surface the main triangulator refused still
    // comes back near the size it was asked for rather than spanned by whatever reaches across.
    bool InitMesh_DBA( const vector< vec2d > & uw_prime, const vector< MeshSeg > & segs_indexes,
                       vector< vector< int > > & connlist, vector< vec2d > & points_out,
                       double spacing = 0.0 );

    void ReadSTL( const char* file_name );
    void WriteSimpleSTL( const char* file_name );
    void WriteSimpleSTL( FILE* file_id );

    void WriteSTL( const char* file_name );
    void WriteSTL( FILE* file_id );

    void ConvertToQuads();

    void SetSurfPtr( Surf* sptr )
    {
        m_Surf = sptr;
    }
    void SetGridDensityPtr ( SimpleGridDensity* gptr )
    {
        m_GridDensity = gptr;
    }

    void HighlightNextNode()
    {
        m_HighlightNodeIndex = ( m_HighlightNodeIndex + 1 ) % ( int )nodeList.size();
    }
    void HighlightNextEdge()
    {
        m_HighlightEdgeIndex = ( m_HighlightEdgeIndex + 1 ) % ( int )edgeList.size();
    }

    void CollapseHighlightEdge();

    void ColorTris();

    int GetNumFaces()
    {
        return faceList.size();
    }

    const list <Face*> & GetFaceList()
    {
        return faceList;
    }

    vector < vec3d >& GetSimpPntVec()
    {
        return simpPntVec;
    }
    vector < vec2d >& GetSimpUWPntVec()
    {
        return simpUWPntVec;
    }
    vector < SimpFace >& GetSimpFaceVec()
    {
        return simpFaceVec;
    }

    void StretchSimpPnts( double start_x, double end_x, double factor, double angle );

    void RemoveInteriorFacesEdgesNodes();

protected:

    Surf* m_Surf;
    SimpleGridDensity* m_GridDensity;

    list < Face* > faceList;
    list < Edge* > edgeList;
    list < Node* > nodeList;

    vector< Face* > garbageFaceVec;
    vector< Edge* > garbageEdgeVec;
    vector< Node* > garbageNodeVec;

    int m_HighlightNodeIndex;
    int m_HighlightEdgeIndex;

    vector< vec3d > simpPntVec;
    vector< vec2d > simpUWPntVec;
    vector< SimpFace > simpFaceVec;
};


#endif
