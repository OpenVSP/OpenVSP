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
#include "Matrix4d.h"

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
    PGNode( const vec3d& p );
    virtual ~PGNode();

    list< PGNode* >::iterator m_List_it;
    vector< PGEdge* > m_EdgeVec;       // All PGEdges Which Use This PGNode

    bool m_DeleteMeFlag;

    vec3d m_Pnt;                   // Position
    map < int, vec2d > m_TagUWMap; // Parametric on a per-tag basis.
    int m_ID;

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
    void GetOtherNodes( vector< PGNode* > & nodVec, const vector< PGNode* > & skipNodVec ) const;

    void GetNodesAsTris( vector < PGNode* > & trinodVec );
    void Triangulate();
    void Triangulate_triangle();
    void Triangulate_DBA();
    void ClearTris();

    void AddEdge( PGEdge* e );
    void RemoveEdge( PGEdge* e );
    bool Contains( PGEdge* e ) const;
    bool Contains( PGNode* n ) const;
    void EdgeForgetFace();
    bool Check();

    void DumpMatlab();

    PGNode * FindDoubleBackNode( PGEdge* & edouble );

    void SplitEdge( PGEdge *e0, PGEdge *e1 );

    void GetHullEdges( vector < PGEdge* > & evec ) const;

    double ComputeTriQual();
    static double ComputeTriQual( PGNode* n0, PGNode* n1, PGNode* n2 );
    static void ComputeCosAngles( PGNode* n0, PGNode* n1, PGNode* n2, double* ang0, double* ang1, double* ang2 );
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


    PGNode* AddNode( vec3d p );
    void  RemoveNode( PGNode* nptr );

    PGEdge* AddEdge( PGNode* n0, PGNode* n1 );
    void  RemoveEdge( PGEdge* e );
    void  RemoveEdgeMergeFaces( PGEdge* e );
    void SwapEdge( PGEdge* e );
    void CheckQualitySwapEdges();
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

    vector < vector < PGEdge* > > m_WakeVec;

    std::map< int, std::string > m_TagNames;
    std::map< int, std::string > m_TagIDs;
    std::map< std::string, int > m_ThickMap;

    std::vector< std::vector<int> > m_TagKeys;
    std::map< std::vector<int>, int > m_SingleTagMap;

    void StartMatlab();

    void SplitEdge( PGEdge *e, PGNode *n );

    void SplitFaceFromDoubleBackNode( PGFace *f, PGEdge *e, PGNode *n );

    void SplitFace( PGFace *f0, PGEdge *e );

    void Triangulate();

    void Report();
    void ResetNodeNumbers();
    void WriteVSPGeom( FILE* file_id, const Matrix4d & XFormMat );
    void WriteVSPGeomPnts( FILE* file_id, const Matrix4d & XFormMat );
    void WriteVSPGeomFaces( FILE* file_id );
    void WriteVSPGeomParts( FILE* file_id );
    void WriteVSPGeomWakes( FILE* file_id );
    void WriteVSPGeomAlternateTris( FILE* file_id );
    void WriteVSPGeomAlternateParts( FILE* file_id );
    void WriteTagFiles( string file_name, vector < string > &all_fnames );
    void WriteTagFile( FILE* file_id, int part, int tag );
    void WriteVSPGEOMKeyFile(const string & file_name, vector < string > &all_fnames );

    void WriteSTL( string fname );
    void WriteTRI( string fname );

    // Get Comma Delimited list of names for a set of tags
    std::string GetTagNames( const std::vector<int> & tags );
    std::string GetTagNames( int indx );

    std::string GetTagIDs( const std::vector<int>& tags );
    std::string GetTagIDs( int indx );

    bool MatchPartAndTag( const vector < int > & tags, int part, int tag );
    bool MatchPartAndTag( int singletag, int part, int tag );
    bool ExistPartAndTag( int part, int tag );
    void MakePartList( std::vector < int > & partvec );

    int GetTag( const std::vector<int> & tags );
    int GetPart( const std::vector<int> & tags );
    int GetPart( int tag );
    void GetPartData( vector < string > &gidvec, vector < int > &partvec, vector < int > &surfvec );

    std::map< std::vector<int>, int > GetSingleTagMap() { return m_SingleTagMap; }
    unsigned int GetNumTags() { return m_SingleTagMap.size(); }

protected:
    vector< PGFace* > m_GarbageFaceVec;
    vector< PGEdge* > m_GarbageEdgeVec;
    vector< PGNode* > m_GarbageNodeVec;

};

PGNode* FindEndNode( const vector < PGEdge* > & eVec );
void GetNodes( const vector < PGEdge* > & eVec, vector< PGNode* > & nodVec );

#endif
