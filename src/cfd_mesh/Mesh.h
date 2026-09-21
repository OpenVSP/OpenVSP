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


// One STL facet as the seven lines the format asks for, appended to out.  buf is the caller's
// scratch buffer, so a loop over a million faces does not stand one up each time round.
void AppendSTLFacet( string &out, char* buf, int buflen, const vec3d &norm,
                     const vec3d &p0, const vec3d &p1, const vec3d &p2 );

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
    // repair means the caller is removing a face that is already unfit, rather than
    // collapsing for size.  Such a collapse is allowed to leave a poor triangle, because the
    // face it removes is worse than poor; it is still refused if it would overlap the mesh.
    bool CollapseEdge( Edge* edge, bool repair = false );

    // Is this face wound against the surface it lies on?
    bool FaceReversed( Face* f );

    // Would a triangle on these three points be wound against the surface?  Asked of points
    // that do not exist yet, so a step can decline to build a face it would have to undo.
    bool TriReversed( const vec3d &p0, const vec3d &p1, const vec3d &p2,
                      const vec2d &uw0, const vec2d &uw1, const vec2d &uw2 );
    int RemoveIllFormedFaces();

    // Bumped once per Collapse round; see Edge::m_CandStamp.
    int m_CandStamp = 0;

    // The edges worth looking at this round, and the ones to look at next.
    //
    // An edge only becomes interesting when its length or its target changes, so only the edges
    // something happened to need looking at again, rather than the whole edge list every round.
    //
    // m_ScanEdges is what this round examines; m_ActiveEdges is what the next round will.
    // A candidate that the budget did not reach stays on, because it is still a candidate.
    vector < Edge* > m_ScanEdges;
    vector < Edge* > m_ActiveEdges;
    int m_ActiveStamp = 0;
    int m_ShapeStamp = 0;

    // Put an edge on the list for the next round, once.
    void MakeActive( Edge* e )
    {
        if ( e && !e->m_DeleteMeFlag && e->m_ActiveStamp != m_ActiveStamp )
        {
            e->m_ActiveStamp = m_ActiveStamp;
            m_ActiveEdges.push_back( e );
        }
    }

    // Everything a node touches, for when the node itself has moved.
    void MakeActiveAround( Node* n )
    {
        if ( n )
        {
            for ( int i = 0 ; i < ( int )n->edgeVec.size() ; i++ )
            {
                MakeActive( n->edgeVec[i] );

                Face* ff[2] = { n->edgeVec[i]->f0, n->edgeVec[i]->f1 };

                for ( int k = 0 ; k < 2 ; k++ )
                {
                    if ( ff[k] )
                    {
                        MakeActive( ff[k]->e0 );
                        MakeActive( ff[k]->e1 );
                        MakeActive( ff[k]->e2 );
                    }
                }
            }
        }
    }

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

    // A face and the edges it names must agree: every edge joins two of the face's corners,
    // and every edge holds the face as one of the two it belongs to.  It costs a fixed handful
    // of comparisons, so an operation can afford to check the faces it just built.
    static void CheckFace( Face* f );
    void CheckValidAllEdges();

    Node* AddNode( const vec3d &p, const vec2d &uw_in );
    void  RemoveNode( Node* nptr );
    Node* FindNode( const vec3d& p );

    // Whether a node may be moved to a given point.  The faces the caller is about to remove
    // are named so they are left out: they are allowed to fold up, and would otherwise be the
    // very faces that refuse the move.
    static bool ValidNodeMove( Node* nptr, const vec3d & move_to, Face* ignoreFace = nullptr,
                               Face* ignoreFace2 = nullptr );

    // What a collapse of this edge to the given point would leave behind.  Returns the
    // smallest angle, in radians, over every face that survives the collapse, and reports
    // through flipped whether any of them would be turned inside out in the parametric
    // domain -- which is Wang 2006 5.2's negative area test for overlap.
    double CollapseConfigQuality( Edge* edge, const vec3d &pc, const vec2d &uwc, bool &flipped, double &qbefore );

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

    // Faces [ibeg,iend) as STL facets, appended as text.  Splitting the writing from the
    // file lets the formatting be done on several threads; see WriteChunked.
    void AppendSimpleSTL( int ibeg, int iend, string &out );

    int GetNumSimpFaces() const
    {
        return ( int )simpFaceVec.size();
    }

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

    // Append len / target_len for every interior edge of this patch.
    //
    // Border edges are left out on purpose.  ComputeTargetEdgeLength hands a border its own
    // length as its target, so every one of them scores exactly 1 and would flatter the
    // answer without saying anything about the mesher's work.
    void AccumLengthRatios( vector < double > &ratios ) const;

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
