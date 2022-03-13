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
//      Surf::Intersect - subdivide in to patches, keep splitting till planer, intersect.
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
//      CfdMeshMgr::MergeBorderEndPoints: Merge IPnts into single points.
//
//      CfdMeshMgr::BuildMesh: For each surface, find chains and build triangle mesh.
//
//      CfdMeshMgr::RemoveInteriorTris: For each triangle, shoot ray and count number of crossings.
//              Remove intierior triangles.
//
//      CfdMeshMgr::Remesh: Remesh (split, collapse, swap, smooth) each surface mesh triangle.
//


#if !defined(SURFACE_INTERSECTION_MGR__INCLUDED_)
#define SURFACE_INTERSECTION_MGR__INCLUDED_

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
#include "MeshCommonSettings.h"
#include "SimpleSubSurface.h"
#include "SimpleMeshSettings.h"
#include "NURBS.h"
#include "CADutil.h"
#include "PntNodeMerge.h"
#include "AnalysisMgr.h"

#include "Vec2d.h"
#include "Vec3d.h"
#include "DrawObj.h"
#include "XferSurf.h"

#include <cassert>

#include <set>
#include <map>
#include <vector>
#include <list>
#include <string>
using namespace std;

class WakeMgrSingleton;

class Wake
{
public:

    Wake();
    virtual ~Wake();

    // void Draw();
    void MatchBorderCurve( ICurve* curve );
    void BuildSurfs();
    double DistToClosestLeadingEdgePnt( vec3d& p );

    piecewise_curve_type m_LeadingEdge;
    vector< ICurve* > m_LeadingCurves;
    vector< Surf* > m_SurfVec;

    int m_CompID;
    double m_Angle;
    double m_Scale;

};

class WakeMgrSingleton
{
public:

    WakeMgrSingleton();
    virtual ~WakeMgrSingleton();

    static WakeMgrSingleton& getInstance()
    {
        static WakeMgrSingleton instance;
        return instance;
    }

    void ClearWakes();

    void SetLeadingEdges( vector < piecewise_curve_type >& wake_leading_edges );
    void CreateWakesAppendBorderCurves( vector< ICurve* >& border_curves, SimpleGridDensity* grid_density_ptr );
    vector< Surf* > GetWakeSurfs();
    void StretchWakes();
    void AppendWakeSurfs( vector< Surf* >& surf_vec );

    //void Draw();
    void LoadDrawObjs( vector< DrawObj* >& draw_obj_vec );
    void Show( bool flag );

    void SetEndX( double x )
    {
        m_EndX = x;
    }
    double GetEndX()
    {
        return m_EndX;
    }
    void SetStartStretchX( double x )
    {
        m_StartStretchX = x;
    }
    double GetStartStretchX()
    {
        return m_StartStretchX;
    }
    void SetStretchMeshFlag( bool flag )
    {
        m_StretchMeshFlag = flag;
    }
    bool GetStretchMeshFlag()
    {
        return m_StretchMeshFlag;
    }

    vec3d ComputeTrailEdgePnt( vec3d le_pnt, double angle_deg );

    void SetWakeScaleVec( vector < double > wake_scale_vec )
    {
        m_WakeScaleVec = wake_scale_vec;
    }

    void SetWakeAngleVec( vector < double > wake_angle_vec )
    {
        m_WakeAngleVec = wake_angle_vec;
    }

protected:

    double m_EndX;
    double m_StartStretchX;

    DrawObj m_WakeDO;

    vector< Wake* > m_WakeVec;

    vector < piecewise_curve_type > m_LeadingEdgeVec;
    vector < double > m_WakeScaleVec;
    vector < double > m_WakeAngleVec;

    bool m_StretchMeshFlag; // Flag that stretches wake tris if true or stretches the wake surface if false

};

#define WakeMgr WakeMgrSingleton::getInstance()

class SurfaceIntersectionSingleton : public ParmContainer
{
protected:
    SurfaceIntersectionSingleton();
    SurfaceIntersectionSingleton( SurfaceIntersectionSingleton const& copy );          // Not Implemented
    SurfaceIntersectionSingleton& operator=( SurfaceIntersectionSingleton const& copy ); // Not Implemented

public:

    static SurfaceIntersectionSingleton& getInstance()
    {
        static SurfaceIntersectionSingleton instance;
        return instance;
    }


    ~SurfaceIntersectionSingleton() override;
    virtual void CleanUp();

    virtual void RegisterAnalysis();

    virtual void IntersectSurfaces();

    virtual void TransferMeshSettings();

    virtual void IdentifyCompIDNames();

    virtual void TransferSubSurfData();
    virtual vector < SimpleSubSurface > GetSimpSubSurfs( string geom_id, int surfnum, int comp_id );

    void addOutputText( const string &str, int output_type = VOCAL_OUTPUT );

//  virtual void Draw();
//  virtual void Draw_BBox( BndBox box );
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual void UpdateDisplaySettings();

    virtual void FetchSurfs( vector< XferSurf > &xfersurfs );
    virtual void LoadSurfs( vector< XferSurf > &xfersurfs, int start_surf_id = 0 );

    virtual void CleanMergeSurfs();

    virtual void WriteSurfsIntCurves( const string &filename  );
    virtual void WriteGridToolCurvFile( const string &filename, bool rawflag );
    virtual void WritePlot3DFile( const string &filename, bool rawflag );
    virtual void WriteIGESFile( const string &filename, int len_unit,
                                bool label_id = false, bool label_surf_num = false, bool label_split_num = false,
                                bool label_name = false, string label_delim = "" );
    virtual void WriteSTEPFile( const string& filename, int len_unit, double tol, bool merge_pnts,
                                bool label_id = false, bool label_surf_num = false, bool label_split_num = false,
                                bool label_name = false, string label_delim = "", int representation = 0 );

    virtual void ExportFiles();
    //virtual void CheckDupOrAdd( Node* node, vector< Node* > & nodeVec );

    virtual Surf* FindSurf( int surf_id ); // Find surface given surf ID

    virtual void DeleteDuplicateSurfs();
    virtual void BuildGrid();

    enum { QUIET_OUTPUT, VOCAL_OUTPUT, };

    virtual void Intersect();

//  virtual void AddISeg( Surf* sA, Surf* sB, vec2d & sAuw0, vec2d & sAuw1,  vec2d & sBuw0, vec2d & sBuw1 );
    virtual void AddIntersectionSeg( const SurfPatch& pA, const SurfPatch& pB, const vec3d & ip0, const vec3d & ip1 );
//  virtual ISeg* CreateSurfaceSeg( Surf* sPtr, vec3d & p0, vec3d & p1, vec2d & uw0, vec2d & uw1 );
    virtual ISeg* CreateSurfaceSeg( Surf* surfA, vec2d & uwA0, vec2d & uwA1, Surf* surfB, vec2d & uwB0, vec2d & uwB1  );

    virtual void WriteISegs();
    virtual void BuildChains();

    void RefineISegChainSeg( ISegChain* c, IPnt* ipnt );
    void RefineISegChain( ISegChain* c );

    virtual void ExpandChain( ISegChain* chain, PNTree* PN_tree );

    virtual void BuildCurves();
    virtual void IntersectSplitChains();

    virtual void BinaryAdaptIntCurves();

    virtual void MergeInteriorChainIPnts();

    virtual void LoadBorderCurves();
    virtual void SplitBorderCurves();

    virtual void DebugWriteChains( const char* name, bool tessFlag );

    // SubSurface Methods
    virtual void BuildSubSurfIntChains();

    virtual void HighlightNextChain();

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

    virtual void MergeFeaPartSSEdgeOverlap()    {}; // Only for FeaMesh; do nothing for CfdMesh
    virtual void CheckFixPointIntersects()    {}; // Only for FeaMesh; do nothing for CfdMesh
    virtual void SetFixPointBorderNodes()    {}; // Only for FeaMesh; do nothing for CfdMesh

    void TestStuff();
    vector< vec3d > debugPnts;
    vector< vec2d > debugUWs;
    vector< SurfPatch* > debugPatches;

    virtual void UpdateWakes();

    vector< ICurve* > GetICurveVec()
    {
        return m_ICurveVec;
    }
    virtual void SetICurveVec( ICurve* newcurve, int loc );

    virtual string GetWakeGeomID()
    {
        return m_WakeGeomID;
    }
    virtual void SetWakeGeomID( const string& gid )
    {
        m_WakeGeomID = gid;
    }

#ifdef DEBUG_CFD_MESH
    FILE* m_DebugFile;
    Stringc m_DebugDir;

    bool m_DebugDraw;
    vector< vector< vec3d > > m_DebugCurves;
    vector< vec3d > m_DebugColors;
#endif

    virtual SimpleIntersectSettings* GetIntersectSettingsPtr()
    {
        return &m_IntersectSettings;
    }
    virtual SimpleMeshCommonSettings* GetSettingsPtr()
    {
        return (SimpleMeshCommonSettings* )&m_IntersectSettings;
    }

    virtual SimpleGridDensity* GetGridDensityPtr()
    {
        return NULL;
    }

    bool GetMeshInProgress()
    {
        return m_MeshInProgress;
    }
    virtual void SetMeshInProgress( bool progress_flag )
    {
        m_MeshInProgress = progress_flag;
    }

protected:

    // Iterate over m_SurfVec and initialize a NURBS surface for each.
    void BuildNURBSSurfMap();

    // Convert each ISegChain into a NURBS curve. The curves are labeled as border 
    // curves or intersection curves. For border curves, a test is performed to 
    // determine if they are outside or inside another surface.
    void BuildNURBSCurvesVec();

    // Function to get all groups of component IDs. Components that are joined by intersection
    // curves make up a group. 
    vector < vector < int > > GetCompIDGroupVec();

    Vehicle* m_Vehicle;

    bool m_MeshInProgress;

    vector< Surf* > m_SurfVec;
    vector < SimpleSubSurface > m_SimpleSubSurfaceVec;

    vector< ICurve* > m_ICurveVec;

    list< ISegChain* > m_ISegChainList;

    vector < IPnt* > m_AllIPnts;

    unsigned int m_NumComps;
    int m_HighlightChainIndex;

    vector< Puw* > m_DelPuwVec;             // Store Created Puw and Ipnts
    vector< IPnt* > m_DelIPntVec;
    vector< IPntGroup* > m_DelIPntGroupVec;
    vector< ISegChain* > m_DelISegChainVec;

    vector < vector < vec3d > > m_IPatchADrawLines;
    vector < vector < vec3d > > m_IPatchBDrawLines;

    vector< vector< vec3d > > debugRayIsect;

    vector < vector < vec3d > > m_BinAdaptCurveAVec;
    vector < vector < vec3d > > m_BinAdaptCurveBVec;
    vector < vector < vec3d > > m_RawCurveAVec;
    vector < vector < vec3d > > m_RawCurveBVec;
    vector < bool > m_BorderCurveFlagVec;

    SimpleIntersectSettings m_IntersectSettings;

    //==== Vector of Surfs that may have a border that lies on Surf A ====//
    map< Surf*, vector< Surf* > > m_PossCoPlanarSurfMap;

    string m_MessageName; // Either "SurfIntersectMessage", "CFDMessage", or "FEAMessage"

    // m_SurfVec translated to a vector of NURBS surfaces
    vector < NURBS_Surface > m_NURBSSurfVec;

    // m_ISegChainList translated to a vector of NURBS curves
    vector < NURBS_Curve > m_NURBSCurveVec;

    map < int, string > m_CompIDNameMap;

    string m_WakeGeomID;

private:

    DrawObj m_IsectCurveDO;
    DrawObj m_IsectPtsDO;
    DrawObj m_BorderCurveDO;
    DrawObj m_BorderPtsDO;

    DrawObj m_RawIsectCurveDO;
    DrawObj m_RawIsectPtsDO;
    DrawObj m_RawBorderCurveDO;
    DrawObj m_RawBorderPtsDO;

    DrawObj m_ApproxPlanesDO;

    DrawObj m_DelPtsDO;

    vector < DrawObj > m_IPatchADO;
    vector < DrawObj > m_IPatchBDO;
};

#define SurfaceIntersectionMgr SurfaceIntersectionSingleton::getInstance()

#endif



