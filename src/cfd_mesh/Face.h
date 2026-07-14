//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// Face.h
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(FACE_FACE__INCLUDED_)
#define FACE_FACE__INCLUDED_

#include <cmath>

#include "Vec2d.h"
#include "Vec3d.h"

#include <cassert>

#include <list>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
using namespace std;

class Face;
class Edge;
class Surf;

//////////////////////////////////////////////////////////////////////
class Node
{
public:
    Node()
    {
        fixed = m_DeleteMeFlag = false;
        edgeVec.reserve( 6 );
    }
    Node( const vec3d& p, const vec2d& uw_in )
    {
        pnt = p;
        uw = uw_in;
        fixed = m_DeleteMeFlag = false;
        edgeVec.reserve( 6 );
    }
    virtual ~Node();

    list< Node* >::iterator list_ptr;

    bool m_DeleteMeFlag;

    vec3d pnt;              // Position
    vec2d uw;               // Parametric
    bool fixed;             // Dont move or delete

    vector< Edge* >  edgeVec;       // All Edges Which Use This Node

    void GetConnectNodes( vector< Node* > & cnVec );
    void GetConnectFaces( vector< Face* > & cfVec );

    Edge *FindEdge( Node* n );

    void AddConnectEdge( Edge* e );
    void RemoveConnectEdge( Edge* e );

    // This node has moved, so every edge leaving it is a different length now.
    void MarkEdgesDirty();
    void LaplacianSmooth();
    void LaplacianSmoothUW();
//  void AngleSmooth();
    void OptSmooth();

    bool AllInteriorConnectedFaces();

    void LaplacianSmooth( Surf* surfPtr );
    void AreaWeightedLaplacianSmooth( Surf* surfPtr );

};


//////////////////////////////////////////////////////////////////////
class Edge
{
public:
    Edge()
    {
        n0 = n1 = nullptr;
        f0 = f1 = nullptr;
        ns = nullptr;
        ridge = border = debugFlag = m_DeleteMeFlag = false;
        target_len = 0;
        m_Length = 0;
    }
    Edge( Node* node0, Node* node1 )
    {
        n0 = node0;
        n1 = node1;
        ns = nullptr;
        f0 = f1 = nullptr;
        ridge = border = debugFlag = m_DeleteMeFlag = false;
        target_len = 0;
        m_Length = 0;
    }
    virtual ~Edge()                         {}

    list< Edge* >::iterator list_ptr;

    bool m_DeleteMeFlag;

    Node* n0;
    Node* n1;

    // Split node along border.  These points should lie on both surfaces along an intersection
    // curve.  Created in Mesh::InitMesh.  Used in Mesh::ConvertToQuads().  Manipulated in Mesh::CollapseEdge.
    Node* ns;

    Face* f0;
    Face* f1;

    bool ridge;             // Dont Remove but Can Split
    bool border;            // Dont remove or split

    bool debugFlag;         // Flag for testing

    double target_len;
    double m_Length;
    bool m_LengthDirty = true;

    // Which pass last offered this edge to the collapse.  Collapse gathers its candidates
    // from two places -- edges that are too short, then faces that are the wrong shape --
    // and an edge must not be offered twice, or it takes two places in a budget that is a
    // fraction of the list.  A stamp rather than a flag so nothing has to be cleared.
    int m_CandStamp = 0;

    // Which round last put this edge on the list of edges worth looking at.  See
    // Mesh::MakeActive.
    int m_ActiveStamp = -1;

    Face* OtherFace( Face* f );
    Node* OtherNode( Node* n );
    void ReplaceNode( Node* curr_node, Node* replace_node );

    bool SetFace( Face* f );
    void RemoveFace( Face* f );
    bool ContainsNodes( Node* in0, Node* in1 );
    bool ContainsNode( Node* in );

    double length()
    {
        return dist( n0->pnt, n1->pnt );
    }

    double ComputeLength()
    {
        m_Length = length();
        m_LengthDirty = false;
        return m_Length;
    }

    // The stored length, worked out again first if anything has moved an end of this edge.
    //
    // Several things move a node without saying so: LaplacianSmooth moves every node in the mesh
    // and recomputes nothing, and CollapseEdge reconnects an edge to a merged node in a new place,
    // also recomputing nothing.  Split and Collapse compare a length against a target, so the
    // length has to be current.
    double GetLength()
    {
        if ( m_LengthDirty )
        {
            m_Length = length();
            m_LengthDirty = false;
        }
        return m_Length;
    }

    void SetLengthDirty()
    {
        m_LengthDirty = true;
    }

    bool BothAdjoiningFacesInterior();
    void ReplaceFace( Face* f, Face* replace_f );

    void NodeForgetEdge();

};

//////////////////////////////////////////////////////////////////////
class Face
{
public:
    Face();
    Face( Node* nn0, Node* nn1, Node* nn2, Edge* ee0, Edge* ee1, Edge* ee2 );
    Face( Node* nn0, Node* nn1, Node* nn2, Node* nn3, Edge* ee0, Edge* ee1, Edge* ee2, Edge* ee3 );
    virtual ~Face();

    static void ComputeCosAngles( Node* nn0, Node* nn1, Node* nn2, double* ang0, double* ang1, double* ang2 );
    static void ComputeCosAngles( Node* nn0, Node* nn1, Node* nn2, Node* nn3, double* ang0, double* ang1, double* ang2, double* ang3 );

    void SetNodesEdges( Node* nn0, Node* nn1, Node* nn2, Edge* ee0, Edge* ee1, Edge* ee2 );
    void SetNodesEdges( Node* nn0, Node* nn1, Node* nn2, Node* nn3, Edge* ee0, Edge* ee1, Edge* ee2, Edge* ee3 );

    Edge* FindEdge( Node* nn0, Node* nn1 );
    Edge* FindEdgeWithout( Node* node_ptr );
    Edge* FindShortEdge();

    void ReplaceNode( Node* curr_node, Node* replace_node );
    void ReplaceEdge( Edge* curr_edge, Edge* replace_edge );

    // Has this face been squeezed until it has no inside left?  Smoothing will not make one,
    // and the mesher collapses any that it finds.
    bool Degenerate();

    double ComputeTriQual();
    static double ComputeTriQual( Node* n0, Node* n1, Node* n2 );

    double ComputeCosSmallAng();

    Node* OtherNodeTri( Node* a, Node* b );

    bool Contains( Node* a, Node* b );
    bool Contains( Edge* e );
    double Area();

    vec3d Normal();
    static vec3d Normal( Node* n0, Node* n1, Node* n2 );

    bool CorrectOrder( Node* n0, Node* n1 );

    void ComputeCenterPnt( Surf* surfPtr, vec3d &cen, vec2d &uwcen ) const;
    vec3d ComputeCenterPnt( Surf* surfPtr ) const;
    vec3d ComputeCenterNormal( Surf* surfPtr ) const;

    void LoadAdjFaces( int num_levels, set< Face* > & faceSet );

    void AddBorderNodes( vector< Node* > &nodeVec );
    void BuildRemovalSet( set < Face* > &remFaces, set < Edge* > &remEdges, set < Node* > &remNodes );
    void EdgeForgetFace();

    bool IsTri() { return !e3; }
    bool IsQuad(){ return  e3; }

    void GetNodePts( vector <vec3d> &pts );

    void WriteSTL( FILE* file_id );

    list< Face* >::iterator list_ptr;
    bool m_DeleteMeFlag;

    Node* n0;
    Node* n1;
    Node* n2;
    Node* n3;

    Edge* e0;
    Edge* e1;
    Edge* e2;
    Edge* e3;

    bool debugFlag;

    // Which round last measured this face's shape, so a face reached from two of its edges is
    // only measured once.  See Mesh::Collapse.
    int m_ShapeStamp = -1;

    // true if inside surface with a cid corresponding to an index.
    vector< bool > insideSurf;

    vector< int > insideCount;

    // Set to true if face should be removed
    bool deleteFlag;

    unsigned char rgb[3];

protected:

};


class SimpFace
{
public:
    SimpFace()
    {
        m_isQuad = false;
        ind0 = ind1 = ind2 = ind3 = -1;
        m_reason = -1;
        m_MinAngle = -1.0;
        m_LenRatio = -1.0;
        m_TargetLen = -1.0;
        m_WorstLenRatio = -1.0;
        m_OffBorder = false;
    }
    bool CheckDegen();


    int ind0;
    int ind1;
    int ind2;
    int ind3;
    bool m_isQuad;
    vector<int> m_Tags;
    int m_iSurf;
    int m_reason;

    // Filled in by CfdMeshMgrSingleton::Subtag, which already visits every face and already
    // asks the target length map about it.
    double m_MinAngle;      // Smallest angle of the face, in degrees.
    double m_LenRatio;      // Mean edge length over the target length at the face's centre.
    double m_TargetLen;     // The target length at the face's centre, so a per-corner ratio
                            // can be formed from it.

    // Worst of this face's own edges, each measured against the target length that edge was
    // built to, expressed so that 1.0 is on target and larger is worse whichever way the edge
    // missed.  Filled in by Mesh::LoadSimpFaces, which is the last moment the edges exist --
    // afterwards only the target at the face's centre is available, and that is a different
    // number wherever the target length varies quickly.
    double m_WorstLenRatio;

    // This face has at least one border edge, so it is the first row of triangles off an
    // intersection or patch boundary.  Worth counting on its own: the boundary itself is
    // tessellated to the target, and any miss shows up first in the row next to it.
    bool m_OffBorder;
};

#endif
