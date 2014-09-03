//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// CfdMeshMgr.h
// J.R Gloudemans
//////////////////////////////////////////////////////////////////////

//===== CfdMesh Overview ====//
//  WriteSurfs: Each component writes out cubic Bezier surfaces (split depending on topology)
//
//  CleanUp: Clear all allocated resources
//
//  ReadSurfs: Read Bezier surf from file.  Combine components that have matching border curves.
//
//  UpdateSourcesAndWakes: Get all sources locations from geom components.  Set up wakes.
//
//  BuildGrid: Build surf dist maps, find  surf border curves, load surface curves (SCurve)
//              Match SCurves to create ICurves.  Create wakes surfs.
//
//  Intersect: Intersect all surfaces.  Intersect Y Slice Plane.
//      Surf::Intersect - subdivide in to patchs, keep splitting till planer, intersect.
//          CfdMeshMgr::AddIntersectionSeg - Create intersection points and segments.
//
//      CfdMeshMgr::LoadBorderCurves: Tesselate border curves, build border chains.
//
//      CfdMeshMgr::BuildChains: Build chains from intersection segments.
//
//      CfdMeshMgr:MergeInteriorChainIPnts: For all chains merge intersection points (IPnt).
//
//      CfdMeshMgr:SplitBorderCurves: Split border chains at the beginning and end of intersection curves.
//
//      CfdMeshMgr::IntersectSplitChains: Intersect non-border chains and split.
//
//  InitMesh: Tesselate chaings, merge border end points, build initial mesh, remove interior tris.
//
//      CfdMeshMgr::TessellateChains: Teseslate all chains based on grid density
//
//      CfdMeshMgr::MergeBorderEndPoints: Merge IPnts into single points.
//
//      CfdMeshMgr::BuildMesh: For each surface, find chains and build triangle mesh.
//
//      CfdMeshMgr::RemoveInteriorTris: For each triangle, shoot ray and count number of crossings.
//              Remove intierior triangles.
//
//      CfdMeshMgr::Remesh: Remesh (split, collapse, swap, smooth) each surface mesh triangle.
//


#if !defined(CfdMeshMgr_CfdMeshMgr__INCLUDED_)
#define CfdMeshMgr_CfdMeshMgr__INCLUDED_

//#ifndef DEBUG_CFD_MESH
//#define DEBUG_CFD_MESH
//#endif

#include "Surf.h"
#include "Mesh.h"
#include "SCurve.h"
#include "ICurve.h"
#include "ISegChain.h"
#include "GridDensity.h"
#include "BezierCurve.h"
#include "Vehicle.h"
#include "CfdMeshSettings.h"

#include "Vec2d.h"
#include "Vec3d.h"
#include "DrawObj.h"

#include <assert.h>

#include <set>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

class WakeMgr;

class Wake
{
public:

    Wake( WakeMgr* mgr );
    virtual ~Wake();

    void Draw();
    void MatchBorderCurve( ICurve* curve );
    void BuildSurfs();
    double DistToClosestLeadingEdgePnt( vec3d & p );

    WakeMgr* m_WakeMgrPtr;
    vector< vec3d > m_LeadingEdge;
    vector< ICurve* > m_LeadingCurves;
    vector< Surf* > m_SurfVec;

    int m_CompID;

};

class WakeMgr
{
public:

    WakeMgr();
    virtual ~WakeMgr();

    void ClearWakes();

    void SetLeadingEdges( vector < vector < vec3d > > & wake_leading_edges );
    void CreateWakesAppendBorderCurves( vector< ICurve* > & border_curves );
    vector< Surf* > GetWakeSurfs();
    void AppendWakeSurfs( vector< Surf* > & surf_vec );
    void StretchWakes();

    //void Draw();
    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    void Show( bool flag );

    void SetEndX( double x )
    {
        m_EndX = x;
    }
    void SetAngle( double a )
    {
        m_Angle = a;
    }
    double GetEndX()
    {
        return m_EndX;
    }
    double GetAngle()
    {
        return m_Angle;
    }
    void SetStartStretchX( double x )
    {
        m_StartStretchX = x;
    }
    vec3d ComputeTrailEdgePnt( vec3d le_pnt );

protected:

    double m_EndX;
    double m_Angle;
    double m_StartStretchX;

    DrawObj m_WakeDO;

    vector< Wake* > m_WakeVec;

    vector< vector< vec3d > > m_LeadingEdgeVec;

};

//////////////////////////////////////////////////////////////////////
class CfdMeshMgrSingleton : public ParmContainer
{
protected:
    CfdMeshMgrSingleton();
    CfdMeshMgrSingleton( CfdMeshMgrSingleton const& copy );          // Not Implemented
    CfdMeshMgrSingleton& operator=( CfdMeshMgrSingleton const& copy ); // Not Implemented

public:

    static CfdMeshMgrSingleton& getInstance()
    {
        static CfdMeshMgrSingleton instance;
        return instance;
    }


    virtual ~CfdMeshMgrSingleton();
    virtual void CleanUp();

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual void GenerateMesh();

    virtual void addOutputText( const string &str, int output_type = CFD_OUTPUT );

    virtual void GUI_Val( string name, double val );
    virtual void GUI_Val( string name, int val );
    virtual void GUI_Val( string name, string val );

    virtual GridDensity* GetGridDensityPtr()
    {
        return m_Vehicle->GetCfdGridDensityPtr();
    }

    virtual string GetCurrGeomID()
    {
        return m_CurrGeomID;
    }
    virtual void SetCurrGeomID( const string &gid )
    {
        m_CurrGeomID = gid;
    }
    virtual string GetFarGeomID()
    {
        return m_FarGeomID;
    }
    virtual void SetFarGeomID( string gid )
    {
        m_FarGeomID = gid;
    }
    virtual int GetCurrMainSurfIndx()
    {
        return m_CurrMainSurfIndx;
    }
    virtual void SetCurrMainSurfIndx( int indx )
    {
        m_CurrMainSurfIndx = indx;
    }
    virtual BaseSource* GetCurrSource();
    virtual BaseSource* AddSource( int type );
    virtual void DeleteCurrSource();
    virtual void DeleteAllSources();

    virtual void AdjustAllSourceLen( double mult );
    virtual void AdjustAllSourceRad( double mult );

    virtual void AddDefaultSources();
    virtual void UpdateSourcesAndWakes();
    virtual void UpdateDomain();
    virtual void ScaleTriSize( double scale );

//  virtual void Draw();
//  virtual void Draw_BBox( BndBox box );
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual void WriteSurfs( const string &filename );
    virtual void ReadSurfs( const string &filename );

    virtual void CleanMergeSurfs();

    virtual void WriteSTL( const string &filename );
    virtual void WriteTetGen( const string &filename );
    virtual void WriteNASCART_Obj_Tri_Gmsh( const string &dat_fn, const string &key_fn, const string &obj_fn, const string &tri_fn, const string &gmsh_fn );
    virtual void WriteSurfsIntCurves( const string &filename  );

    virtual void ExportFiles();
    //virtual void CheckDupOrAdd( Node* node, vector< Node* > & nodeVec );
    virtual int BuildIndMap( vector< vec3d* > & allPntVec, map< int, vector< int > >& indMap, vector< int > & pntShift );
    virtual int  FindPntIndex( vec3d& pnt, vector< vec3d* > & allPntVec,
                               map< int, vector< int > >& indMap );

    virtual string CheckWaterTight();
    virtual Edge* FindAddEdge( map< int, vector<Edge*> > & edgeMap, vector< Node* > & nodeVec, int ind1, int ind2 );

    virtual void BuildDomain();
    virtual void DeleteDuplicateSurfs();
    virtual void BuildGrid();

    enum { NO_OUTPUT, CFD_OUTPUT, FEA_OUTPUT, };
    virtual void Remesh( int output_type );
    virtual void RemeshSingleComp( int comp_id, int output_type );

    virtual void Intersect();
    virtual void InitMesh();

    virtual void PrintQual();
    virtual string GetQualString();

    virtual Surf* GetSurf( int ind )
    {
        return m_SurfVec[ind];
    }

//  virtual void AddISeg( Surf* sA, Surf* sB, vec2d & sAuw0, vec2d & sAuw1,  vec2d & sBuw0, vec2d & sBuw1 );
    virtual void AddIntersectionSeg( SurfPatch& pA, SurfPatch& pB, vec3d & ip0, vec3d & ip1 );
//  virtual ISeg* CreateSurfaceSeg( Surf* sPtr, vec3d & p0, vec3d & p1, vec2d & uw0, vec2d & uw1 );
    virtual ISeg* CreateSurfaceSeg( Surf* surfA, vec2d & uwA0, vec2d & uwA1, Surf* surfB, vec2d & uwB0, vec2d & uwB1  );

    virtual void BuildChains();
    virtual void ExpandChain( ISegChain* chain );

    virtual void BuildCurves();
    virtual void IntersectSplitChains();

    virtual vector< Surf* > CreateDomainSurfs();

    virtual void IntersectWakes();

    virtual void MergeInteriorChainIPnts();

    virtual void LoadBorderCurves();
    virtual void SplitBorderCurves();
    virtual void MergeBorderEndPoints();
    virtual void MergeIPntGroups( list< IPntGroup* > & iPntGroupList, double tol );
    virtual void TessellateChains();
    virtual void SetWakeAttachChain( ISegChain* c );
    virtual void MatchWakes();
    virtual void AddWakeCoPlanarSurfaceChains();
    virtual void AddSurfaceChain( Surf* sPtr, ISegChain* chainIn );
    virtual void BuildMesh();
    virtual void BuildTargetMap( int output_type );
    virtual void RemoveInteriorTris();
    virtual void ConnectBorderEdges( bool wakeOnly );
    virtual void MatchBorderEdges( list< Edge* > edgeList );

    virtual void DebugWriteChains( const char* name, bool tessFlag );

    // SubSurface Methods
    virtual void BuildSubSurfIntChains();
    virtual void BuildTestIntChains();
    virtual void SubTagTris();

    virtual void HighlightNextChain();

    virtual void SetBatchFlag( bool f )
    {
        m_BatchFlag = f;
    }
    virtual bool GetBatchFlag()
    {
        return m_BatchFlag;
    }

    virtual void AddDelPuw( Puw* puw )
    {
        m_DelPuwVec.push_back( puw );
    }
    virtual void AddDelIPnt( IPnt* ip )
    {
        m_DelIPntVec.push_back( ip );
    }

    virtual void WriteChains();

    void AddPossCoPlanarSurf( Surf* surfA, Surf* surfB );
    vector< Surf* > GetPossCoPlanarSurfs( Surf* surfPtr );

    void TestStuff();
    vector< vec3d > debugPnts;
    vector< vec2d > debugUWs;
    vector< SurfPatch* > debugPatches;

    vector< ICurve* > GetICurveVec()
    {
        return m_ICurveVec;
    }
    virtual void SetICurveVec( ICurve* newcurve, int loc );

#ifdef DEBUG_CFD_MESH
    FILE* m_DebugFile;
    Stringc m_DebugDir;

    bool m_DebugDraw;
    vector< vector< vec3d > > m_DebugCurves;
    vector< vec3d > m_DebugColors;
#endif

    ostringstream m_OutStream;


    CfdMeshSettings* GetCfdSettingsPtr()
    {
        return m_Vehicle->GetCfdSettingsPtr();
    }


protected:

    /*
    * Update Bounding Box DrawObjs.
    */
    virtual void UpdateBBoxDO( BndBox box );

    Vehicle* m_Vehicle;

    string m_CurrGeomID;
    int m_CurrMainSurfIndx;
    string m_FarGeomID;
    bool m_BatchFlag;

    vector< Surf* > m_SurfVec;

    //==== Wakes ====//
    WakeMgr m_WakeMgr;

    vector< ICurve* > m_ICurveVec;

    list< ISegChain* > m_ISegChainList;

    vector< IPnt* > m_IPntVec;
    vector< ISeg* > m_IsegVec;
    map< int, IPntBin > m_BinMap;

    //vector< ISegSplit* > m_ISegSplitVec;

    int m_NumComps;
    int m_HighlightChainIndex;

    BndBox m_Domain;

    vector< Puw* > m_DelPuwVec;             // Store Created Puw and Ipnts
    vector< IPnt* > m_DelIPntVec;
    vector< IPntGroup* > m_DelIPntGroupVec;
    vector< ISegChain* > m_DelISegChainVec;

    vector< vector< vec3d > > debugRayIsect;


    //==== Vector of Surfs that may have a border that lies on Surf A ====//
    map< Surf*, vector< Surf* > > m_PossCoPlanarSurfMap;

    vector<Edge*> m_BadEdges;
    vector<Tri*> m_BadTris;
    vector< Node* > m_nodeStore;

    vector< string > m_GeomIDs;

private:
    vector < DrawObj > m_MeshTriDO;
    vector < DrawObj > m_MeshWakeTriDO;
    DrawObj m_MeshBadEdgeDO;
    DrawObj m_MeshBadTriDO;
    DrawObj m_BBoxLineStripDO;
    DrawObj m_BBoxLinesDO;
    vector< DrawObj > m_TagDO;
};

#define CfdMeshMgr CfdMeshMgrSingleton::getInstance()

#endif



