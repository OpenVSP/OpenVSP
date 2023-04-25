//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Polygon Mesh Geometry Class
//
//
//   Rob McDonald
//
//
//******************************************************************************

#ifndef PGMesh_H
#define PGMesh_H

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

class PGFace;
class PGEdge;

//////////////////////////////////////////////////////////////////////
class PGNode
{
public:
    PGNode();
    PGNode( const vec3d& p, const vec2d& uw_in );
    virtual ~PGNode();

    list< PGNode* >::iterator m_List_it;
    vector< PGEdge* > m_EdgeVec;       // All PGEdges Which Use This PGNode

    bool m_DeleteMeFlag;

    vec3d m_Pnt;              // Position
    vec2d m_UW;               // Parametric

    void GetConnectNodes( vector< PGNode* > & cnVec ) const;
    void GetConnectFaces( vector< PGFace* > & cfVec ) const;

    PGEdge *FindEdge( const PGNode* n ) const;

    void AddConnectEdge( PGEdge* e );
    void RemoveConnectEdge( PGEdge* e );
    void EdgeForgetNode( PGEdge* e );

    bool ColinearNode( double tol );
    bool Check();

    void DumpMatlab();

};


//////////////////////////////////////////////////////////////////////
class PGEdge
{
public:
    PGEdge();
    PGEdge( PGNode* PGNode0, PGNode* PGNode1 );
    virtual ~PGEdge();

    list< PGEdge* >::iterator m_List_it;

    bool m_DeleteMeFlag;

    PGNode* m_N0;
    PGNode* m_N1;

    vector< PGFace* > m_FaceVec;

    bool ContainsNodes( const PGNode* in0, const PGNode* in1 ) const;
    bool ContainsNode( const PGNode* in ) const;
    bool UsedBy( PGFace* f ) const;

    bool SetNode( PGNode* n );
    void AddConnectFace( PGFace* f );

    void RemoveFace( PGFace* f );

    void ReplaceNode( PGNode* curr_PGNode, PGNode* replace_PGNode );

    PGNode* OtherNode( const PGNode* n ) const;
    PGNode* SharedNode( const PGEdge* e ) const;

    void NodesForgetEdge();

    void SortFaces();
    bool SameFaces( PGEdge *e2 );
    bool Check();

    void DumpMatlab();

};

//////////////////////////////////////////////////////////////////////
class PGFace
{
public:
    PGFace();
    virtual ~PGFace();

    PGEdge* FindEdge( PGNode* nn0, PGNode* nn1 ) const;


    PGNode * FindPrevNode( int i ) const;

    void GetNodes( vector< PGNode* > & nodVec ) const;

    void GetNodesAsTris( vector < PGNode* > & trinodVec );
    void Triangulate();
    void Triangulate_triangle();
    void ClearTris();

    void AddEdge( PGEdge* e );
    void RemoveEdge( PGEdge* e );
    bool Contains( PGEdge* e ) const;
    void EdgeForgetFace();
    bool Check();

    void DumpMatlab();

    PGNode * FindDoubleBackNode();

    list< PGFace* >::iterator m_List_it;

    vec3d m_Nvec;
    int m_iQuad;
    int m_Tag;

    vector< PGEdge* > m_EdgeVec;
    vector < PGNode* > m_TriNodeVec;

    // Set to true if PGFace should be removed
    bool deleteFlag;

};


//////////////////////////////////////////////////////////////////////
class PGMesh
{
public:

    PGMesh();
    virtual ~PGMesh();

    void Clear();


    void DumpGarbage();


    PGNode* AddNode( vec3d p, vec2d uw_in );
    void  RemoveNode( PGNode* nptr );

    PGEdge* AddEdge( PGNode* n0, PGNode* n1 );
    void  RemoveEdge( PGEdge* e );
    void  RemoveEdgeMergeFaces( PGEdge* e );
    PGEdge* FindEdge( const PGNode* n0, const PGNode* n1 ) const;

    PGFace* AddFace();

    void  RemoveFace( PGFace* fptr );

    int GetNumFaces()
    {
        return m_FaceList.size();
    }

    const list <PGFace*> & GetFaceList()
    {
        return m_FaceList;
    }

    void RemoveNodeMergeEdges( PGNode* n );

    bool Check();

    list < PGFace* > m_FaceList;
    list < PGEdge* > m_EdgeList;
    list < PGNode* > m_NodeList;

    void StartMatlab();

protected:
    vector< PGFace* > m_GarbageFaceVec;
    vector< PGEdge* > m_GarbageEdgeVec;
    vector< PGNode* > m_GarbageNodeVec;

};

#endif
