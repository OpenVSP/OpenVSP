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
class ISegChain;

#include <cassert>

#include <vector>
#include <list>
#include <unordered_map>
using namespace std;


// One STL facet as the seven lines the format asks for, appended to out.
void AppendSTLFacet( string &out, const vec3d &norm, const vec3d &p0, const vec3d &p1, const vec3d &p2 );

class MeshSeg
{
public:
    int m_Index[2];
    vec2d m_UWmid;
    vec3d m_P[2];
    vec3d m_Pmid;
    ISegChain* m_Chain[2] = { nullptr, nullptr };   // The chains the segment lies on
};

// The chains a border edge of the initial mesh lies on: a second where two coincide
typedef std::unordered_map < Edge*, std::pair < ISegChain*, ISegChain* > > BorderChainMap;

//////////////////////////////////////////////////////////////////////
// Storage for one kind of mesh element.  Elements are made in blocks, so those made one after
// another lie side by side in memory, and one given back is the next one handed out.  The
// blocks are let go once every element is given back.
template < class T >
class MeshPool
{
public:

    MeshPool()
    {
        m_Live = 0;
        m_NextInBlock = BLOCK;
    }

    ~MeshPool()
    {
        Release();
    }

    void* Take()
    {
        m_Live++;

        if ( !m_Free.empty() )
        {
            void* p = m_Free.back();
            m_Free.pop_back();
            return p;
        }

        if ( m_NextInBlock == BLOCK )
        {
            m_Blocks.push_back( static_cast< char* >( ::operator new( BLOCK * sizeof( T ) ) ) );
            m_NextInBlock = 0;
        }

        void* p = m_Blocks.back() + m_NextInBlock * sizeof( T );
        m_NextInBlock++;
        return p;
    }

    void Give( T* p )
    {
        p->~T();
        m_Free.push_back( p );
        m_Live--;

        if ( m_Live == 0 )
        {
            Release();
        }
    }

private:

    MeshPool( const MeshPool & );
    MeshPool & operator=( const MeshPool & );

    void Release()
    {
        for ( size_t i = 0 ; i < m_Blocks.size() ; i++ )
        {
            ::operator delete( m_Blocks[i] );
        }
        vector< char* >().swap( m_Blocks );
        vector< void* >().swap( m_Free );
        m_NextInBlock = BLOCK;
    }

    static const size_t BLOCK = 1024;

    vector< char* > m_Blocks;
    vector< void* > m_Free;
    size_t m_NextInBlock;
    size_t m_Live;
};

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
    void FlipEdge( Edge* edge );

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
    bool SplitFaceStands( const vec3d &ps, const vec2d &uws, Node* p, Node* q, Face* f,
                          Node* other_apex, vector < Edge* > &flips );

    // The way every face is wound in UW: +1 or -1 when the starting triangulation has all of
    // its faces wound one way there, and 0 when it does not.  Where it is set, the mesh is a
    // flat triangulation of the UW domain, and a split may not build a face wound the other
    // way: such a face overlaps its neighbours.
    int m_UWSign = 0;
    void SetUWSign();

    // Would a triangle on these three UW points be wound the way m_UWSign asks?
    bool UWWoundRight( const vec2d &uw0, const vec2d &uw1, const vec2d &uw2 ) const;

    // Would every face a split of the edge n0 n1 at uws builds in fa and fb be wound right?
    bool SplitWoundRight( Face* fa, Face* fb, Node* n0, Node* n1, const vec2d &uws ) const;

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
    // cosine of the smallest angle over every face that survives the collapse -- the largest
    // of their cosines -- and reports through flipped whether any of them would be turned
    // inside out in the parametric domain -- which is Wang 2006 5.2's negative area test for
    // overlap.  cosbefore is the same cosine over those faces as they are now.
    double CollapseConfigQuality( Edge* edge, const vec3d &pc, const vec2d &uwc, bool &flipped, double &cosbefore );

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
    // surface past a robustness limit -- unlike the order the points are given in.  steinerout is
    // set when the triangulation used every point it was allowed and was refused for it.
    bool InitMesh_TRI( const vector< vec2d > & uw_prime, const vector< MeshSeg > & segs_indexes,
                       vector< vector< int > > & connlist, vector< vec2d > & points_out, int relax = 0, double areascale = 1.0,
                       bool * steinerout = nullptr );
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

    // Valid until the mesh is remeshed.
    const BorderChainMap & GetBorderChainMap() const
    {
        return m_BorderChainMap;
    }
    void ClearBorderChainMap()
    {
        BorderChainMap().swap( m_BorderChainMap );
    }

protected:

    Surf* m_Surf;
    SimpleGridDensity* m_GridDensity;

    list < Face* > faceList;
    list < Edge* > edgeList;
    list < Node* > nodeList;

    BorderChainMap m_BorderChainMap;

    MeshPool< Face > m_FacePool;
    MeshPool< Edge > m_EdgePool;
    MeshPool< Node > m_NodePool;

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
