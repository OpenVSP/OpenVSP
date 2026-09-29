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
//  InitMesh: Tesselate chains, merge border end points, build initial mesh, remove interior tris.
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

#ifdef __JETBRAINS_IDE__
#define DEBUG_CFD_MESH
#endif

#include <atomic>
#include <functional>
#include "Surf.h"
#include "Mesh.h"
#include "SCurve.h"
#include "ICurve.h"
#include "ISegChain.h"
#include "GridDensity.h"
#include "BezierCurve.h"
#include "Vehicle.h"
#include "MessageMgr.h"
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
#include "BndBox.h"

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

    void MatchBorderCurve( ICurve* curve );
    void BuildSurfs();
    double DistToClosestLeadingEdgePnt( const vec3d& p );

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

    void SetLeadingEdges( const vector < piecewise_curve_type >& wake_leading_edges );
    void CreateWakesAppendBorderCurves( vector< ICurve* >& border_curves, SimpleGridDensity* grid_density_ptr );
    vector< Surf* > GetWakeSurfs();
    void StretchWakes();
    void AppendWakeSurfs( vector< Surf* >& surf_vec );

    void UpdateDrawObjs();
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

    void SetWakeScaleVec( const vector < double > &wake_scale_vec )
    {
        m_WakeScaleVec = wake_scale_vec;
    }

    void SetWakeAngleVec( const vector < double > &wake_angle_vec )
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

// What one surface pair's patch intersection produced.  Pairs are worked side by side, so each
// keeps its own output and they are folded into the manager's lists in pair order afterwards --
// which is the order the plain loop over pairs would have built them in.
class IsectOutput
{
public:
    vector< Puw* > m_Puws;
    vector< IPnt* > m_IPnts;
    vector< vector< vec3d > > m_PatchADraw;
    vector< vector< vec3d > > m_PatchBDraw;
};

class SurfaceIntersectionSingleton;

// Offers one analysis to AnalysisMgr when it asks for the meshing ones.
//
// AnalysisMgr lives in the geometry core, which cannot see this library and must not, so it
// cannot name these analyses itself.  It asks; these answer.  One sits beside each manager, so
// an analysis is offered exactly when the manager that runs it is built into the program.
//
// Made when the program loads, because a caller may only ever ask what analyses exist without
// meshing anything.  It must not touch its manager until asked: the managers are built on
// first use, and building one at load time would drag the vehicle up with it.
class AnalysisRegistrar : public MessageBase
{
public:
    AnalysisRegistrar( void ( *reg )() )
    {
        m_Register = reg;
        Register( "RegisterAnalyses" );
    }

    void MessageCallback( const MessageBase* from, const MessageData& data ) override
    {
        if ( m_Register && data.m_String == "Register" )
        {
            m_Register();
        }
    }

protected:
    void ( *m_Register )();
};


// Hears that the vehicle has been emptied.  A listener rather than a call, because the
// geometry core cannot see this library -- which is the case MessageMgr exists for.
class MeshRenewListener : public MessageBase
{
public:
    MeshRenewListener()
    {
        m_Mgr = nullptr;
    }

    void SetMgr( SurfaceIntersectionSingleton *mgr )
    {
        m_Mgr = mgr;
    }

    void MessageCallback( const MessageBase* from, const MessageData& data ) override;

protected:
    SurfaceIntersectionSingleton *m_Mgr;
};

class SurfaceIntersectionSingleton : public ParmContainer
{
protected:
    SurfaceIntersectionSingleton();

public:

    static SurfaceIntersectionSingleton& getInstance()
    {
        static SurfaceIntersectionSingleton instance;
        return instance;
    }


    ~SurfaceIntersectionSingleton() override;
    virtual void CleanUp();

    // Throw away everything built from the vehicle's geometry.  Sent when the vehicle is
    // emptied, by a new file or by one loaded over the top of this one.
    virtual void RenewMesh();

    virtual void RegisterAnalysis();

    virtual void IntersectSurfaces();

    // What the last run produced, for a caller that wants the numbers rather than the files.
    int GetNumSurfs() const
    {
        return ( int )m_SurfVec.size();
    }

    int GetNumChains() const
    {
        return ( int )m_ISegChainList.size();
    }

    // Raw intersection points over all the intersection curves.
    int GetNumCurvePnts() const;

    // The same numbers as a Results entry, so a scripted run can read what the intersection
    // found rather than parse the files it wrote.  Written by every run.
    virtual void RecordResults();

    const string& GetLastResultID() const
    {
        return m_LastResultID;
    }

    string m_LastResultID;

    virtual void LimitedIntersectSurfaces( const vector < string > & geomvec, vector < vector < vec3d > > & ptchains, vector < vector < vec3d > > & uwchains );

    virtual void TransferMeshSettings();

    virtual void IdentifyCompIDNames();

    virtual void TransferSubSurfData();
    virtual vector < SimpleSubSurface > GetSimpSubSurfs( const string &geom_id, int surfnum, int comp_id );
    virtual int GetSimpSubSurfIndex( const string &ss_id );

    void addOutputText( string str, int output_type = VOCAL_OUTPUT );

    virtual void UpdateDrawObjs();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    virtual bool GetVisBndBox( BndBox &bbox );

    virtual void UpdateDisplaySettings();

    virtual void FetchXFerSurfs( const vector < string > & geomvec, vector< XferSurf > &xfersurfs );
    virtual void FetchSurfs( vector< XferSurf > &xfersurfs, int n_ref = 0 );

    virtual void LoadSurfs( vector< XferSurf > &xfersurfs, double scale = 1.0, int start_surf_id = 0 );

    virtual void CleanMergeSurfs( bool skip_duplicate_removal );

    virtual void WriteIGESFile( const string &filename, int len_unit,
                                bool label_id = false, bool label_surf_num = false, bool label_split_num = false,
                                bool label_name = false, const string &label_delim = "" );
    virtual void WriteSTEPFile( const string& filename, int len_unit, double tol, bool merge_pnts,
                                bool label_id = false, bool label_surf_num = false, bool label_split_num = false,
                                bool label_name = false, const string &label_delim = "", int representation = 0 );

    virtual void ExportFiles();
    //virtual void CheckDupOrAdd( Node* node, vector< Node* > & nodeVec );

    virtual Surf* FindSurf( int surf_id ); // Find surface given surf ID
    virtual int FindSurfIndx( int surf_id ); // Find surface given surf ID

    virtual void DeleteDuplicateSurfs();
    virtual void SplitBordersToMatch();
    virtual void BuildGrid();

    enum { QUIET_OUTPUT, VOCAL_OUTPUT, };

    virtual void Intersect();

//  virtual void AddISeg( Surf* sA, Surf* sB, vec2d & sAuw0, vec2d & sAuw1,  vec2d & sBuw0, vec2d & sBuw1 );
    // A segment of the intersection of patches pA and pB, found between triangle triA of pA's
    // corners qa and triangle triB of pB's corners qb (1 is corners 0 2 3, 2 is corners 0 1 2)
    virtual void AddIntersectionSeg( const SurfPatch& pA, const SurfPatch& pB, const vec3d & ip0, const vec3d & ip1,
                                     const vec3d qa[4], const vec3d qb[4], int triA, int triB );
//  virtual ISeg* CreateSurfaceSeg( Surf* sPtr, vec3d & p0, vec3d & p1, vec2d & uw0, vec2d & uw1 );
    virtual ISeg* CreateSurfaceSeg( Surf* surfA, vec2d & uwA0, vec2d & uwA1, Surf* surfB, vec2d & uwB0, vec2d & uwB1  );

    virtual void WriteISegs();
    // Partner each intersection point with the one across the triangle edge it lies on
    virtual void LinkIPntPartners();

    virtual void BuildChains();
    virtual void CleanChains();
    virtual void CleanChain( ISegChain* c );
    virtual void RefineChains();

    void RefineISegChainSeg( ISegChain* c, IPnt* ipnt );
    void RefineISegChain( ISegChain* c );

    virtual void ExpandChain( ISegChain* chain, PNTree* PN_tree );

    virtual void BuildCurves();
    virtual void IntersectSplitChains();

    virtual void BuildIntChain( const string &id, vector < vector < vec3d > > & ptchains, vector < vector < vec3d > > & uwchains );

    // Keep each chain's raw points and what kind of curve it is
    virtual void RecordIntCurves();

    virtual void MergeInteriorChainIPnts();

    virtual void LoadBorderCurves();
    virtual void SplitBorderCurves();

    virtual void DebugWriteChainVec( const char* name, vector< ISegChain* > chainvec );
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
    virtual void WriteChainSplits( const char* name, vector< ISegChain* > chainvec );

    void AddPossCoPlanarSurf( Surf* surfA, Surf* surfB );
    vector< Surf* > GetPossCoPlanarSurfs( Surf* surfPtr );

    virtual void MergeFeaPartSSEdgeOverlap()    {}; // Only for FeaMesh; do nothing for CfdMesh
    virtual void CheckFixPointIntersects()    {}; // Only for FeaMesh; do nothing for CfdMesh
    virtual void SetFixPointBorderNodes()    {}; // Only for FeaMesh; do nothing for CfdMesh

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
    string m_DebugDir;

    // The per surface debug scripts are numbered within one pass over the surfaces, and the
    // master scripts that run them are opened on first use and closed when the pass ends.
    //
    // All of this used to be function static inside Mesh::InitMesh, which meant it belonged
    // to the process rather than to the run.  A second meshing run carried on numbering where
    // the first left off, so the master scripts were never reopened, and the condition that
    // closed them -- the surface count reaching the last surface -- could still come true and
    // write to a file that was already closed.
    int m_DebugSurfCnt;
    FILE* m_DebugSortedUWFile;
    FILE* m_DebugMeshUWFile;
    FILE* m_DebugTriMeshFile;

    void BeginDebugSurfFiles();
    void EndDebugSurfFiles();

    bool m_DebugDraw;
    vector< vector< vec3d > > m_DebugCurves;
    vector< vec3d > m_DebugColors;
#endif

    // A count of finished work, for a stage whose pieces come back in whatever order the
    // threads happen to finish them.  Which piece finished is not worth reporting when the
    // order is arbitrary; how much is left is, and it says plainly that the run is moving.
    void ReportProgress( const string &str, int output_type );
    void DrawProgress( int done, char term, int output_type );

    void BeginProgress( const string &label, int n, int output_type );
    void StepProgress( int output_type );
    void EndProgress( int output_type );

    string m_ProgressLabel;
    std::atomic< int > m_ProgressDone;
    int m_ProgressTotal;

    void IntersectPairs();

    // How many threads the mesher may use, and how a stage of independent pieces is run
    // across them.  These live here rather than with the CFD mesher because the intersection
    // stage needs them too.
    static int MeshThreadCount();
    int StageThreadCount( int nitem );
    void RunIndexed( int n, int nthread, const std::function< void( int ) > &body );

    // Write n items to fp, formatting them on several threads and writing them out in order.
    // body( ibeg, iend, out ) appends items [ibeg,iend) to out, and must depend on nothing
    // but those items.  The chunks go to the file in order, so the result is byte for byte
    // the file the plain loop wrote.
    void WriteChunked( FILE* fp, int n, const std::function< void( int, int, string & ) > &body );

    virtual SimpleMeshCommonSettings* GetSettingsPtr()
    {
        return (SimpleMeshCommonSettings* )&m_IntersectSettings;
    }

    virtual SimpleGridDensity* GetGridDensityPtr()
    {
        return nullptr;
    }

    bool GetMeshInProgress()
    {
        return m_MeshInProgress;
    }
    virtual void SetMeshInProgress( bool progress_flag )
    {
        m_MeshInProgress = progress_flag;
    }

    virtual int GetTotalNumSurfs()
    {
        return m_SurfVec.size();
    }

protected:

    // Iterate over m_SurfVec and initialize a NURBS surface for each.
    void BuildNURBSSurfMap();

    // Convert each ISegChain into a NURBS curve. The curves are labeled as border 
    // curves or intersection curves. For border curves, a test is performed to 
    // determine if they are outside or inside another surface.
    // cad builds each curve as it is written to trimmed CAD; otherwise each is only an
    // adapted polyline.
    void BuildNURBSCurvesVec( bool cad = true );

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

    // A chain whose two parents are the same surface is matched to itself: the surface is
    // laid against itself along it, and it bounds one patch rather than two.
    vector < bool > m_NonManifoldCurveFlagVec;

    vector < vector < vec3d > > m_RawCurveAVec;
    vector < vector < vec3d > > m_RawCurveBVec;
    vector < bool > m_BorderCurveFlagVec;

    SimpleIntersectSettings m_IntersectSettings;

    //==== Vector of Surfs that may have a border that lies on Surf A ====//
    unordered_map< Surf*, vector< Surf* > > m_PossCoPlanarSurfMap;

    string m_MessageName; // Either "SurfIntersectMessage", "CFDMessage", or "FEAMessage"

    // m_SurfVec translated to a vector of NURBS surfaces
    vector < NURBS_Surface > m_NURBSSurfVec;

    // m_ISegChainList translated to a vector of NURBS curves
    vector < NURBS_Curve > m_NURBSCurveVec;

    unordered_map < int, string > m_CompIDNameMap;

    string m_WakeGeomID;

private:

    MeshRenewListener m_RenewListener;

    DrawObj m_RawIsectCurveDO;
    DrawObj m_RawIsectPtsDO;
    DrawObj m_RawBorderCurveDO;
    DrawObj m_RawBorderPtsDO;

    DrawObj m_RawNonManifoldCurveDO;
    DrawObj m_RawNonManifoldPtsDO;

    DrawObj m_ApproxPlanesDO;

    DrawObj m_DelPtsDO;

    vector < DrawObj > m_IPatchADO;
    vector < DrawObj > m_IPatchBDO;
};

#define SurfaceIntersectionMgr SurfaceIntersectionSingleton::getInstance()

#endif



