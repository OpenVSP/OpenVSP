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

class TNode;
class TTri;
class TMesh;

class PGMulti;
class PGMesh;
class PGFace;
class PGEdge;
class PGNode;

//////////////////////////////////////////////////////////////////////

class PGPoint
{
public:
    PGPoint();
    explicit PGPoint( const vec3d& p );
    virtual ~PGPoint();

    list< PGPoint* >::iterator m_List_it;
    vector< PGNode* > m_NodeVec;       // All PGEdges Which Use This PGNode

    bool m_DeleteMeFlag;

    vec3d m_Pnt;                   // Position
    int m_ID;

    void AddConnectNode( PGNode* n );
    void RemoveConnectNode( const PGNode* n );
    void NodeForgetPoint( PGNode* n ) const;


    bool Check() const;

};

//////////////////////////////////////////////////////////////////////
class PGNode
{
public:
    explicit PGNode( PGPoint *pptr );
    virtual ~PGNode();

    list< PGNode* >::iterator m_List_it;
    vector< PGEdge* > m_EdgeVec;       // All PGEdges Which Use This PGNode

    PGPoint *m_Pt;

    bool m_DeleteMeFlag;

    map < int, vec2d > m_TagUWMap; // Parametric on a per-tag basis.


    void GetConnectNodes( vector< PGNode* > & cnVec ) const;
    void GetConnectFaces( vector< PGFace* > & cfVec ) const;

    void GetTags( vector < int > & tags ) const;

    bool GetUW( int tag, vec2d & uw ) const;

    PGEdge *FindEdge( const PGNode* n ) const;

    bool UsedBy( const PGEdge* e ) const;

    void AddConnectEdge( PGEdge* e );
    void RemoveConnectEdge( const PGEdge* e );
    void EdgeForgetNode( PGEdge* e ) const;

    bool ColinearNode() const;
    bool Check() const;

    bool DoubleBackNode( int &i, int &j ) const;
    void SealDoubleBackNode( PGMesh *pgm );

    void DumpMatlab() const;
    void Diagnostics() const;

    bool Validate() const;
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
    bool m_InLoopFlag;
    bool m_InCurrentLoopFlag;

    PGNode* m_N0;
    PGNode* m_N1;

    int m_ID;

    vector< PGFace* > m_FaceVec;

    bool ContainsNodes( const PGNode* in0, const PGNode* in1 ) const;
    bool ContainsNode( const PGNode* in ) const;
    bool UsedBy( const PGFace* f ) const;

    bool SetNode( PGNode* n );
    void AddConnectFace( PGFace* f );

    void RemoveFace( const PGFace* f );

    void ReplaceNode( const PGNode* curr_PGNode, PGNode* replace_PGNode );

    PGNode* OtherNode( const PGNode* n ) const;
    PGNode* SharedNode( const PGEdge* e ) const;

    PGFace* OtherManifoldFace( const PGFace* ) const;

    void NodesForgetEdge() const;

    void SortFaces();
    bool SameFaces( const PGEdge *e2 ) const;
    bool Check() const;

    void DumpMatlab() const;
    void Diagnostics() const;

    bool Validate() const;

    bool WakeEdge( const PGMesh *m, bool ContinueCoPlanarWakes ) const;
};

//////////////////////////////////////////////////////////////////////
class PGFace
{
public:
    PGFace();
    virtual ~PGFace();

    PGEdge* FindEdge( const PGNode* nn0, const PGNode* nn1 ) const;


    PGNode * FindPrevNode( int i ) const;

    void GetNodes( vector< PGNode* > & nodVec ) const;
    void GetOtherNodes( vector< PGNode* > & nodVec, const vector< PGNode* > & skipNodVec ) const;

    void GetNodesAsTris( vector < PGNode* > & trinodVec );
    void Triangulate();
    static void Triangulate( const vector < PGNode* > &nodVec, const vec3d & nvec, vector < PGNode* > &triNodeVec );
    static void Triangulate_triangle( const vector < PGNode* > &nodVec, const vec3d & nvec, vector < PGNode* > &triNodeVec );
    static void Triangulate_DBA( const vector < PGNode* > &nodVec, const vec3d & nvec, vector < PGNode* > &triNodeVec );
    void ClearTris();

    void AddEdge( PGEdge* e );
    void RemoveEdge( PGEdge* e );
    void ReplaceEdge( const PGEdge *eold, PGEdge *enew );
    bool Contains( const PGEdge* e ) const;
    bool Contains( const PGNode* n ) const;
    void EdgeForgetFace() const;
    bool Check() const;

    void DumpMatlab() const;
    void Diagnostics() const;

    bool Validate() const;

    double ComputeArea();
    static double ComputeArea( const vector < PGNode* > &nodVec );

    vec3d ComputeCenter();

    void WalkRegion() const;

    PGNode * FindDoubleBackNode( PGEdge* & edouble ) const;

    void SplitEdge( PGEdge *e0, PGEdge *e1 );

    void GetHullEdges( vector < PGEdge* > & evec ) const;

    double ComputeTriQual() const;
    static double ComputeTriQual( const PGNode* n0, const PGNode* n1, const PGNode* n2 );
    static void ComputeCosAngles( const PGNode* n0, const PGNode* n1, const PGNode* n2, double* ang0, double* ang1, double* ang2 );
    list< PGFace* >::iterator m_List_it;

    vec3d m_Nvec;
    int m_iQuad;
    int m_Tag;
    int m_jref;
    int m_kref;

    int m_ID;

    int m_Region;

    vector< PGEdge* > m_EdgeVec;
    vector < PGNode* > m_TriNodeVec;

    // Set to true if PGFace should be removed
    bool m_DeleteMeFlag;

};

//////////////////////////////////////////////////////////////////////

void QuadFaceMergeProps( PGFace *f0, PGFace *f1 );

void JrefMergeProps( PGFace *f0, PGFace *f1 );

void KrefMergeProps( PGFace *f0, PGFace *f1 );

bool QuadTagMatch( PGFace *f0, PGFace *f1 );

bool JrefTagMatch( PGFace *f0, PGFace *f1 );

bool KrefTagMatch( PGFace *f0, PGFace *f1 );

//////////////////////////////////////////////////////////////////////
class PGMesh
{
public:

    PGMesh( PGMulti* pgmulti );
    virtual ~PGMesh();

    void Clear();

    void CleanUnused();

    void DumpGarbage();




    PGNode* AddNode( PGPoint *pptr );
    void  RemoveNode( PGNode* nptr );

    PGEdge* AddEdge( PGNode* n0, PGNode* n1 );
    void  RemoveEdge( PGEdge* e );
    void  RemoveEdgeMergeFaces( PGEdge* e, void ( * facemergeproperties ) ( PGFace *f0, PGFace *f1 ) );
    static void SwapEdge( PGEdge* e );
    void CheckQualitySwapEdges();
    static PGEdge* FindEdge( const PGNode* n0, const PGNode* n1 ) ;

    PGFace* AddFace();
    PGFace* AddFace( PGNode* n0, PGNode* n1, PGNode* n2,
                             const vec2d &uw0, const vec2d &uw1, const vec2d &uw2,
                             const vec3d & norm, int iQuad, int tag, int jref, int kref );

    void  RemoveFace( PGFace* fptr );

    void RemoveNegativeiQuadFaces();

    void MergeNodes( PGNode* na, PGNode* nb );
    int MergeCoincidentNodes();

    void MergeEdges( PGEdge *ea, PGEdge *eb );
    int MergeDuplicateEdges();

    int RemoveDegenEdges();

    int RemoveDegenFaces();

    int GetNumFaces() const
    {
        return m_FaceList.size();
    }

    const list <PGFace*> & GetFaceList() const
    {
        return m_FaceList;
    }

    void RemoveNodeMergeEdges( PGNode* n );

    bool Check();

    PGMulti *m_PGMulti;
    list < PGFace* > m_FaceList;
    list < PGEdge* > m_EdgeList;
    list < PGNode* > m_NodeList;

    vector < PGNode* > m_DoubleBackNode;
    vector < vector < PGEdge * > > m_EdgeLoopVec;
    vector < vector < PGEdge* > > m_WakeVec;

    std::map< int, std::string > m_TagNames;
    std::map< int, std::string > m_TagIDs;
    std::map< std::string, int > m_ThickMap;
    std::vector< int > m_ThickVec;
    std::vector< int > m_TypeVec;
    std::vector< double > m_WminVec;

    std::vector< std::vector<int> > m_TagKeys;
    std::map< std::vector<int>, int > m_SingleTagMap;

    vector < PGFace* > m_Regions;

    bool m_DeleteMeFlag;

    void FindAllDoubleBackNodes();
    void SealDoubleBackNodes();

    void ResetEdgeLoopFlags();

    void ExtendWake( vector < PGEdge * > & wake, PGEdge *e, const PGNode *n, bool ContinueCoPlanarWakes );
    void IdentifyWakes( bool ContinueCoPlanarWakes );

    static void StartMatlab();

    PGEdge * SplitEdge( PGEdge *e, PGNode *n );
    PGEdge * SplitEdge( PGEdge *e, double t, PGNode *n0 );

    void SplitFaceFromDoubleBackNode( PGFace *f, const PGEdge *e, PGNode *n );

    void SplitFace( PGFace *f0, PGEdge *e );

    void MakeRegions();
    void CullOrphanThinRegions( double tol );

    void Triangulate();
    void ClearTris();

    void Coarsen1();
    void Coarsen2();

    void Report();
    void ResetEdgeNumbers();
    void ResetFaceNumbers();

    bool Validate();

    void MergeFaces( bool ( * facemergetest ) ( PGFace *f0, PGFace *f1 ), void ( * facemergeproperties ) ( PGFace *f0, PGFace *f1 ) = &QuadFaceMergeProps  );

    void PolygonizeMesh();
    void CleanColinearVerts();

    void WriteVSPGeom( FILE* file_id, const Matrix4d & XFormMat );
    void WriteVSPGeomPnts( FILE* file_id, const Matrix4d & XFormMat );
    void WriteVSPGeomFaces( FILE* file_id );
    void WriteVSPGeomParts( FILE* file_id );
    void WriteVSPGeomWakes( FILE* file_id ) const;
    void WriteVSPGeomAlternateTris( FILE* file_id );
    void WriteVSPGeomAlternateParts( FILE* file_id );
    void WriteTagFiles( const string& file_name, vector < string > &all_fnames );
    void WriteTagFile( FILE* file_id, int part, int tag );
    void WriteVSPGEOMKeyFile(const string & file_name, vector < string > &all_fnames );

    void WriteSTL( const string& fname );
    void WriteTRI( const string& fname );

    // Get Comma Delimited list of names for a set of tags
    std::string GetTagNames( const std::vector<int> & tags );
    std::string GetTagNames( int indx );

    std::string GetTagIDs( const std::vector<int>& tags );
    std::string GetTagIDs( int indx );

    string GetGID( const int& tag );

    static bool MatchPartAndTag( const vector < int > & tags, int part, int tag );
    bool MatchPartAndTag( int singletag, int part, int tag ) const;
    bool ExistPartAndTag( int part, int tag ) const;
    void MakePartList( std::vector < int > & partvec ) const;

    int GetTag( const std::vector<int> & tags );
    vector< int > GetTagVec( const int &t );
    int GetType( int part ) const;
    int GetThickThin( int part ) const;
    double GetWmin( int part ) const;
    static int GetPart( const std::vector<int> & tags );
    int GetPart( int tag ) const;
    void GetPartData( vector < string > &gidvec, vector < int > &partvec, vector < int > &surfvec );

    void BuildFromTMesh( const TMesh* tmi );
    void BuildFromTMeshVec( const vector< TMesh* > &tmv );

    std::map< std::vector<int>, int > GetSingleTagMap() const { return m_SingleTagMap; }
    unsigned int GetNumTags() const { return m_SingleTagMap.size(); }

protected:
    vector< PGFace* > m_GarbageFaceVec;
    vector< PGEdge* > m_GarbageEdgeVec;
    vector< PGNode* > m_GarbageNodeVec;

};

PGNode* FindEndNode( const vector < PGEdge* > & eVec );
void GetNodes( const vector < PGEdge* > & eVec, vector< PGNode* > & nodVec );

//////////////////////////////////////////////////////////////////////


class PGMulti
{
public:

    PGMulti();
    virtual ~PGMulti();

    void Clear();

    void CleanUnused();

    void DumpGarbage();

    PGMesh* AddMesh();
    void  RemoveMesh( PGMesh* m );

    PGPoint* AddPoint( const vec3d& p );
    void  RemovePoint( PGPoint* pptr );

    PGMesh* GetActiveMesh();

    bool Check();
    void ResetPointNumbers();

    vector < PGMesh* > m_MeshVec;
    list < PGPoint* > m_PointList;

    int m_ActiveMesh;


protected:

    vector< PGMesh* > m_GarbageMeshVec;
    vector< PGPoint* > m_GarbagePointVec;

};


#endif
