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

#include "Surf.h"
#include "Mesh.h"
#include "SCurve.h"
#include "ICurve.h"
#include "ISegChain.h"
#include "GridDensity.h"
#include "BezierCurve.h"
#include "Vehicle.h"
#include "SurfaceIntersectionMgr.h"
#include "MeshCommonSettings.h"
#include "SimpleSubSurface.h"
#include "SimpleMeshSettings.h"
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

//////////////////////////////////////////////////////////////////////
class CfdMeshMgrSingleton : public SurfaceIntersectionSingleton
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


    ~CfdMeshMgrSingleton() override;
    void CleanUp() override;

    virtual void RegisterAnalysis() override;

    SimpleMeshCommonSettings* GetSettingsPtr() override
    {
        return (SimpleMeshCommonSettings* ) &m_CfdSettings;
    }

    virtual void GenerateMesh();

    void TransferMeshSettings() override;

    virtual void GUI_Val( string name, double val );
    virtual void GUI_Val( string name, int val );
    virtual void GUI_Val( string name, string val );

    virtual string GetCurrSourceGeomID()
    {
        return m_CurrSourceGeomID;
    }
    virtual void SetCurrSourceGeomID( const string &gid )
    {
        m_CurrSourceGeomID = gid;
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
    virtual void AddDefaultSourcesCurrGeom();
    virtual void UpdateSourcesAndWakes();
    virtual void UpdateDomain();

    virtual void UpdateDrawObjs() override;
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec ) override;

    void UpdateDisplaySettings() override;

    virtual void WriteSTL( const string &filename );
    virtual void WriteTaggedSTL( const string &filename );
    virtual void WriteTetGen( const string &filename );
    virtual void WriteNASCART_Obj_Tri_Gmsh( const string &dat_fn, const string &key_fn, const string &obj_fn, const string &tri_fn, const string &gmsh_fn, const string & vspgeom_fn );
    virtual void WriteFacet( const string &facet_fn );

    void ExportFiles() override;
    //virtual void CheckDupOrAdd( Node* node, vector< Node* > & nodeVec );
    virtual int BuildIndMap( vector< vec3d* > & allPntVec, map< int, vector< int > >& indMap, vector< int > & pntShift );
    virtual int  FindPntIndex( vec3d& pnt, vector< vec3d* > & allPntVec,
                               map< int, vector< int > >& indMap );

    virtual string CheckWaterTight();
    virtual Edge* FindAddEdge( map< int, vector<Edge*> > & edgeMap, vector< Node* > & nodeVec, int ind1, int ind2 );

    virtual void BuildDomain();
    void BuildGrid() override;

    enum { QUIET_OUTPUT, VOCAL_OUTPUT, };
    virtual void Remesh( int output_type );

    virtual void PostMesh();

    virtual void ConvertToQuads();

    virtual void InitMesh();

    virtual string GetQualString();

    virtual vector< Surf* > CreateDomainSurfs();

    virtual void MergeBorderEndPoints();
    virtual void MergeEndPointCloud( IPntCloud &cloud, double tol );
    virtual void TessellateChains();
    virtual void SetWakeAttachChain( ISegChain* c );
    virtual void MatchWakes();
    virtual void AddWakeCoPlanarSurfaceChains();
    virtual void AddSurfaceChain( Surf* sPtr, ISegChain* chainIn );
    virtual void BuildMesh();
    virtual void BuildTargetMap( int output_type );
    virtual void RemoveInteriorTris();
    virtual void RemoveTrimTris() {};  // Implemented for FEAMesh
    virtual void ConnectBorderEdges( bool wakeOnly );
    virtual void MatchBorderEdges( list< Edge* > edgeList );

    // SubSurface Methods
    virtual void SubTagTris();
    virtual void SetSimpSubSurfTags( int tag_offset );
    virtual void Subtag( Surf* surf );

    virtual bool SetDeleteTriFlag( int aType, bool symPlane, vector < bool > aInB );

    virtual SimpleCfdMeshSettings* GetCfdSettingsPtr()
    {
        return &m_CfdSettings;
    }

    virtual SimpleGridDensity* GetGridDensityPtr() override
    {
        return &m_CfdGridDensity;
    }

protected:

    /*
    * Update Bounding Box DrawObjs.
    */
    virtual void UpdateBBoxDO( BndBox box );
    virtual void UpdateBBoxDOSymSplit( BndBox box );

    string m_CurrSourceGeomID;
    int m_CurrMainSurfIndx;
    string m_WakeGeomID;

    SimpleCfdMeshSettings m_CfdSettings;
    SimpleCfdGridDensity m_CfdGridDensity;

    BndBox m_Domain;

    vector<Edge*> m_BadEdges;
    vector<Face*> m_BadFaces;
    vector< Node* > m_nodeStore;

private:
    DrawObj m_MeshBadEdgeDO;
    DrawObj m_MeshBadTriDO;
    DrawObj m_MeshBadQuadDO;
    DrawObj m_BBoxLineStripDO;
    DrawObj m_BBoxLinesDO;
    DrawObj m_BBoxLineStripSymSplit;
    DrawObj m_BBoxLineSymSplit;
    vector< DrawObj > m_TagDO;

};

#define CfdMeshMgr CfdMeshMgrSingleton::getInstance()

#endif



