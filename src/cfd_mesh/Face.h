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

#include "Defines.h"
#include "Vec2d.h"
#include "Vec3d.h"

#define _USE_MATH_DEFINES
#include <cmath>
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
    }
    Node( const vec3d& p, const vec2d& uw_in )
    {
        pnt = p;
        uw = uw_in;
        fixed = m_DeleteMeFlag = false;
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
        n0 = n1 = NULL;
        f0 = f1 = NULL;
        ridge = border = debugFlag = m_DeleteMeFlag = false;
        target_len = 0;
        m_Length = 0;
    }
    Edge( Node* node0, Node* node1 )
    {
        n0 = node0;
        n1 = node1;
        f0 = f1 = NULL;
        ridge = border = debugFlag = m_DeleteMeFlag = false;
        target_len = 0;
        m_Length = 0;
    }
    virtual ~Edge()                         {}

    list< Edge* >::iterator list_ptr;

    bool m_DeleteMeFlag;

    Node* n0;
    Node* n1;

    Face* f0;
    Face* f1;

    bool ridge;             // Dont Remove but Can Split
    bool border;            // Dont remove or split

    bool debugFlag;         // Flag for testing

    double target_len;
    double m_Length;

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
        return m_Length;
    }
    double GetLength()
    {
        return m_Length;
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
    Edge* FindLongEdge();

    void ReplaceNode( Node* curr_node, Node* replace_node );
    void ReplaceEdge( Edge* curr_edge, Edge* replace_edge );

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

    void AddBorderEdges( list< Edge* > &edgeList );
    void BuildRemovalSet( set < Face* > &remFaces, set < Edge* > &remEdges, set < Node* > &remNodes );
    void EdgeForgetFace();

    bool IsTri() { return !e3; }
    bool IsQuad(){ return  e3; }

    void GetNodePts( vector <vec3d> &pts );

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
    }
    int ind0;
    int ind1;
    int ind2;
    int ind3;
    bool m_isQuad;
    vector<int> m_Tags;
};

#endif
