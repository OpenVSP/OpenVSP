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
//      CfdMeshMgr::TessellateChains: Teseslate all chains based on grid density
//
//      CfdMeshMgr::MergeBorderEndPoints: Merge IPnts into single points.
//
//      CfdMeshMgr::JoinLooseChainEnds: Join each intersection curve end that meets no other curve
//              to the nearest point of one.
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

    virtual void GUI_Val( const string &name, double val );
    virtual void GUI_Val( const string &name, int val );
    virtual void GUI_Val( const string &name, const string &val );

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

    virtual void Update();
    virtual void UpdateSourcesAndWakes();
    virtual void UpdateDomain();

    // Everything the sources and wakes are built from: the sets and mode, and each Geom's
    // updates and sources
    string SourcesAndWakesState();

    virtual void UpdateDrawObjs() override;

    // Sort every face into a quality band and fill m_QualityDO.
    virtual void UpdateQualityDrawObjs();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec ) override;

    void UpdateDisplaySettings() override;

    virtual void WriteSTL( const string &filename );
    virtual void WriteTaggedSTL( const string &filename );
    virtual void WriteTetGen( const string &filename );
    virtual void WriteNASCART_Obj_Tri_Gmsh( const string &dat_fn, const string &key_fn, const string &obj_fn, const string &tri_fn, const string &gmsh_fn, const string & vspgeom_fn );
    virtual void WriteTagFiles( string file_name, const vector< SimpFace > &allFaceVec,
                                const vector< const vector < int >* > &allTagsVec, const vector< int > &allTagVec, bool allowquads );
    virtual void WriteTagFile( FILE* file_id, const vector < char > &match, const vector < int > &faceCombo,
                               const vector< SimpFace > &allFaceVec, bool allowquads );
    virtual void WriteFacet( const string &facet_fn );

    // The faces carrying each of tags, by position in faceTags and in that order.  A face is
    // put with the last of tags that equals its tag.
    void GroupFacesByTag( const vector< const vector < int >* > &faceTags, const vector< int > &tags,
                          vector< vector< int > > &groups );
    virtual void WritePOGS( const string &pogs_fn );
    virtual void ReportPOGSWrite( const string &fn, bool ok );
    virtual bool WritePOGSSurfFile( const string &uvin_fn, const string &uv_fn,
                                    const vector < int > &face_surf_vec );
    virtual bool WritePOGSCompFile( const string &fn, const vector < int > &face_surf_vec );
    virtual bool WritePOGSInputFile( const string &fn, const string &rootname, int isym );
    virtual bool PntTrimmedAway( const vec3d &pnt, Surf *srf, double x_dist,
                                 vector < vector < double > > &t_vec_vec );


    void ExportFiles() override;
    //virtual void CheckDupOrAdd( Node* node, vector< Node* > & nodeVec );

    virtual string CheckWaterTight();

    // What the last run produced.  The watertight check's verdict is a line of text meant
    // for a person; these are the same numbers for a caller that has to act on them.  An
    // edge with one triangle on it is a border edge and one with more than two is over
    // connected; a closed mesh has neither.
    int GetNumMeshTris() const
    {
        return m_NumMeshTris;
    }

    int GetNumBorderEdges() const
    {
        return m_NumBorderEdges;
    }

    int GetNumOverConnEdges() const
    {
        return m_NumOverConnEdges;
    }

    int m_NumMeshTris = 0;
    int m_NumBorderEdges = 0;
    int m_NumOverConnEdges = 0;
    int m_NumTopoSurfs = 0;             // Surfaces the chain topology classified, needing no rays

    // The same numbers as a Results entry, so a scripted run can read the verdict rather
    // than watch the console for it.  Written by every run.
    void RecordResults() override;

    const string& GetLastResultID() const
    {
        return m_LastResultID;
    }

    string m_LastResultID;

    // How close the mesh came to the edge lengths it was asked for.  Collected in PostMesh,
    // which is the last moment the edges exist.
    virtual string TargetLengthReport();
    vector < double > m_LengthRatios;

    // The same misses per face rather than per edge, a face counting as bad as its worst
    // edge.  This is what the coloured picture shows, and it is always a larger share than
    // the per edge number -- the two are not interchangeable.
    vector < double > m_FaceLengthRatios;

    // The subset sitting against a border, to test whether the row of triangles next to an
    // intersection curve misses its target worse than the field does.
    vector < double > m_BorderFaceLengthRatios;

    virtual void BuildDomain();
    void BuildGrid() override;

    enum { QUIET_OUTPUT, VOCAL_OUTPUT, };

    bool CoplanarSurfsMeshed() const override
    {
        return true;
    }

    bool RemovesInteriorTris() const override
    {
        return true;
    }

    // The triangles of the initial mesh classified by the topology of the intersection chains:
    // del[s][k] for face k of surface s, for each surface with handled[s] set.  The others are
    // left to the ray cast.
    void TopoInteriorTris( int ncross, vector< vector< char > > &del, vector< char > &handled );

    void CompSurfCounts( int ncross, vector< pair< int, int > > &compcounts );
    void RemoveInteriorTrisOneSurf( int s, double x_dist, int ncross, const vector< vector< pair< int, bool > > > &coplanar,
                                    vector< bool > &inside, vector< int > &count );

    virtual void Remesh( int output_type );
    virtual void RemeshOneSurf( int isurf, int nsurf, int output_type, int &num_tris );

    virtual void PostMesh();

    virtual void ConvertToQuads();

    virtual void InitMesh();

    virtual string GetQualString();

    virtual vector< Surf* > CreateDomainSurfs();

    virtual void MergeBorderEndPoints();
    virtual void MergeEndPointCloud( IPntCloud &cloud, double tol );
    virtual void MergeCoincidentTessPnts();
    virtual void JoinLooseChainEnds();
    virtual void TessellateChains();
    virtual void TieCoincidentBorderChains();
    virtual void SetWakeAttachChain( ISegChain* c );
    virtual void MatchWakes();
    virtual void AddWakeCoPlanarSurfaceChains();
    virtual void AddSurfaceChain( Surf* sPtr, ISegChain* chainIn );
    virtual void BuildMesh();
    virtual void RemoveCrossingTessSegs( const vector < vector < ISegChain* > > &surf_chains, int nthread );

    virtual void ForceSurfaceFixPoints( int surf_indx, vector < vec2d > &adduw ) {}; // used by FEAMesh Only.

    virtual void BuildTargetMap( int output_type );
    virtual void BuildChainDistTables();
    virtual void SpreadChainDensity( int nedgeprop, int nthread, SplitSourceSet &splitSources, int stride, int output_type );
    virtual void RemoveInteriorTris();
    virtual void RemoveTrimTris() {};  // Implemented for FEAMesh
    virtual void ConnectBorderNodes( bool wakeOnly );
    virtual void MatchBorderNodes( const vector< Node* > & nodeVec );

    // SubSurface Methods
    virtual void SubTagTris();
    virtual void SetSimpSubSurfTags( int tag_offset );
    // Tag one surface's faces.  The tag combinations found go into combo rather than
    // straight into the manager's set, so surfaces can be tagged side by side.
    virtual void Subtag( Surf* surf, std::set< std::vector< int > > &combo );

    // The smallest angle and the realized-over-target edge length of one face.

    virtual bool SetDeleteTriFlag( int aType, bool symPlane, const vector < bool > &aInB );

    virtual SimpleCfdMeshSettings* GetCfdSettingsPtr()
    {
        return &m_CfdSettings;
    }

    virtual SimpleGridDensity* GetGridDensityPtr() override
    {
        return &m_CfdGridDensity;
    }

    void FindDegenCorners();
    void AddDegenCornerChains();


protected:

    /*
    * Update Bounding Box DrawObjs.
    */
    virtual void UpdateBBoxDO( const BndBox &box );
    virtual void UpdateBBoxDOSymSplit( const BndBox &box );

    string m_CurrSourceGeomID;
    int m_CurrMainSurfIndx;
    string m_WakeGeomID;

    SimpleCfdMeshSettings m_CfdSettings;
    SimpleCfdGridDensity m_CfdGridDensity;

    BndBox m_Domain;

    vector<Edge*> m_BadEdges;
    vector<Face*> m_BadFaces;
    vector< Node* > m_nodeStore;

    // The mesh points merged once for every file written and for the watertight check.
    //
    // Each writer merges the points of its own choice of surfaces, numbered in the order it
    // visits them.  Where the points within merging distance of one another always fall into
    // groups whose members are all within that distance of each other, merging any choice of
    // surfaces gives those same groups restricted to the choice, so one merge of every point
    // serves them all.  Otherwise each writer merges its own points.
    void BuildExportMerge();
    void ClearExportMerge();

    // For the surfaces flagged in use, taken in order: each point's merged point, numbered as
    // a merge of those surfaces' points alone numbers them, and each merged point's place.
    // used_first, when given, receives the position among those points of each merged
    // point's first member.
    void MergeSurfPnts( const vector< bool > &use, vector< vector< int > > &used_key,
                        vector< vec3d > &used_pnt, vector< int > *used_first );

    bool m_ExportMergeBuilt = false;
    vector< int > m_ExportMergeStart;   // Position of each surface's first point in the merge
    vector< int > m_ExportMergeGroup;   // Each point's group, its lowest member; empty if unusable

    vector< IPnt* > m_DegenCorners;
    vector< ISegChain* > m_DegenCornerChains;

private:

    // What the sources and wakes were last built from
    string m_SourcesAndWakesState;


    DrawObj m_MeshBadEdgeDO;
    DrawObj m_MeshBadTriDO;
    DrawObj m_MeshBadQuadDO;
    DrawObj m_BBoxLineStripDO;
    DrawObj m_BBoxLinesDO;
    DrawObj m_BBoxLineStripSymSplit;
    DrawObj m_BBoxLineSymSplit;
    vector< DrawObj > m_TagDO;
    vector< DrawObj > m_ReasonDO;

    // One per quality band, tris then quads, laid out exactly as m_ReasonDO is, and which of
    // the two quality measures they were last filled for.
    vector< DrawObj > m_QualityDO;
    int m_QualityDOMetric = -1;

    DrawObj m_DegenCornerPointDO;
    DrawObj m_DegenCornerEdgeDO;

};

#define CfdMeshMgr CfdMeshMgrSingleton::getInstance()

#endif



