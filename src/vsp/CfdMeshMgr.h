//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// CfdMeshMgr.h
// J.R Gloudemans
//////////////////////////////////////////////////////////////////////

//===== CfdMesh Overview ====//
//	WriteSurfs: Each component writes out cubic Bezier surfaces (split depending on topology)
//
//	CleanUp: Clear all allocated resources
//
//	ReadSurfs: Read Bezier surf from file.  Combine components that have matching border curves.
//
//	UpdateSourcesAndWakes: Get all sources locations from geom components.  Set up wakes.
//
//	BuildGrid: Build surf dist maps, find  surf border curves, load surface curves (SCurve)
//				Match SCurves to create ICurves.  Create wakes surfs.
//
//  Intersect: Intersect all surfaces.  Intersect Y Slice Plane.
//		Surf::Intersect - subdivide in to patchs, keep splitting till planer, intersect.
//			CfdMeshMgr::AddIntersectionSeg - Create intersection points and segments.
//
//		CfdMeshMgr::LoadBorderCurves: Tesselate border curves, build border chains.
//
//		CfdMeshMgr::BuildChains: Build chains from intersection segments.
//
//		CfdMeshMgr:MergeInteriorChainIPnts: For all chains merge intersection points (IPnt).
//
//		CfdMeshMgr:SplitBorderCurves: Split border chains at the beginning and end of intersection curves.
//
//		CfdMeshMgr::IntersectSplitChains: Intersect non-border chains and split.
//
//  InitMesh: Tesselate chaings, merge border end points, build initial mesh, remove interior tris.
//
//		CfdMeshMgr::TessellateChains: Teseslate all chains based on grid density
//
//		CfdMeshMgr::MergeBorderEndPoints: Merge IPnts into single points.
//
//		CfdMeshMgr::BuildMesh: For each surface, find chains and build triangle mesh.
//
//		CfdMeshMgr::RemoveInteriorTris: For each triangle, shoot ray and count number of crossings.
//				Remove intierior triangles.
//
//		CfdMeshMgr::Remesh: Remesh (split, collapse, swap, smooth) each surface mesh triangle.
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
#include "bezier_curve.h"

#include "vec2d.h"
#include "vec3d.h"
#include "stringc.h"

#include <assert.h>

#include <set>
#include <vector>
#include <list>
using namespace std;

class Aircraft;
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

	void Draw();

 
	void SetEndX( double x )						{ m_EndX = x; }
	void SetStartStretchX( double x )				{ m_StartStretchX = x; }
	vec3d ComputeTrailEdgePnt( vec3d le_pnt );

	void SetWakeAngle( double a )					{ m_Angle = a; }
	double GetWakeAngle()							{ return m_Angle; }


protected:

	double m_EndX;
	double m_StartStretchX;
	double m_Angle;
	vector< Wake* > m_WakeVec;

	vector< vector< vec3d > > m_LeadingEdgeVec;

};


//////////////////////////////////////////////////////////////////////
class CfdMeshMgr
{
public:

	CfdMeshMgr();
	virtual ~CfdMeshMgr();
	virtual void CleanUp();
        
	virtual void addOutputText( const char* str, int output_type );

	virtual void SetAircraftPtr( Aircraft* aptr )			{ aircraftPtr = aptr; }

	virtual void GUI_Val( Stringc name, double val );
	virtual void GUI_Val( Stringc name, int val );
	virtual void GUI_Val( Stringc name, Stringc val );

	virtual GridDensity* GetGridDensityPtr()		{ return &m_GridDensity; }

	virtual int  GetCurrGeomID()					{ return m_CurrGeomID; }
	virtual void SetCurrGeomID(int gid)				{ m_CurrGeomID = gid; }
	virtual int GetFarGeomID()						{ return m_FarGeomID; }
	virtual void SetFarGeomID( int gid )			{ m_FarGeomID = gid; }
	virtual BaseSource* GetCurrSource();
	virtual void AddSource( int type );
	virtual BaseSource* CreateSource( int type );
	virtual void DeleteCurrSource();

	virtual void AdjustAllSourceLen( double mult );
	virtual void AdjustAllSourceRad( double mult );

	virtual void AddDefaultSources();
	virtual void UpdateSourcesAndWakes();
	virtual void ScaleTriSize( double scale );

	virtual void Draw();
	virtual void Draw_BBox( bbox box );

	virtual void WriteSurfs( const char* filename );
	virtual void ReadSurfs( const char* filename );

	virtual void WriteSTL( const char* filename );
	virtual void WriteTetGen( const char* filename );
	virtual void WriteNASCART_Obj_Tri_Gmsh( const char* dat_fn, const char* key_fn, const char* obj_fn, const char* tri_fn, const char* gmsh_fn );
	virtual void WriteSurfsIntCurves( const char* filename  );

	virtual void ExportFiles();
	//virtual void CheckDupOrAdd( Node* node, vector< Node* > & nodeVec );
	virtual int BuildIndMap( vector< vec3d* > & allPntVec, map< int, vector< int > >& indMap, vector< int > & pntShift );
	virtual int  FindPntIndex( vec3d& pnt, vector< vec3d* > & allPntVec, 
					map< int, vector< int > >& indMap );

	virtual Stringc CheckWaterTight();
	virtual Edge* FindAddEdge( map< int, vector<Edge*> > & edgeMap, vector< Node* > & nodeVec, int ind1, int ind2 );

	virtual void BuildGrid();

	enum { NO_OUTPUT, CFD_OUTPUT, FEA_OUTPUT, };
	virtual void Remesh(int output_type);
	virtual void RemeshSingleComp( int comp_id, int output_type );

	virtual void Intersect();
	virtual void InitMesh();

	virtual void PrintQual();
	virtual Stringc GetQualString();

	virtual Surf* GetSurf( int ind )							{ return m_SurfVec[ind]; }

//	virtual void AddISeg( Surf* sA, Surf* sB, vec2d & sAuw0, vec2d & sAuw1,  vec2d & sBuw0, vec2d & sBuw1 );
	virtual void AddIntersectionSeg( SurfPatch& pA, SurfPatch& pB, vec3d & ip0, vec3d & ip1 );
//	virtual ISeg* CreateSurfaceSeg( Surf* sPtr, vec3d & p0, vec3d & p1, vec2d & uw0, vec2d & uw1 );
	virtual ISeg* CreateSurfaceSeg( Surf* surfA, vec2d & uwA0, vec2d & uwA1, Surf* surfB, vec2d & uwB0, vec2d & uwB1  );

	virtual void BuildChains();
	virtual void ExpandChain( ISegChain* chain );

	virtual void BuildCurves();
	virtual void IntersectSplitChains();
	virtual void IntersectYSlicePlane();
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

	virtual bool GetDrawMeshFlag()					{ return m_DrawMeshFlag; }
	virtual void SetDrawMeshFlag( bool f )			{ m_DrawMeshFlag= f; }
	virtual bool GetDrawSourceFlag()				{ return m_DrawSourceFlag; }
	virtual void SetDrawSourceFlag( bool f )		{ m_DrawSourceFlag= f; }
	virtual bool GetDrawFarFlag()					{ return m_DrawFarFlag; }
	virtual void SetDrawFarFlag( bool f )			{ m_DrawFarFlag = f; }
	virtual bool GetDrawFarPreFlag()				{ return m_DrawFarPreFlag; }
	virtual void SetDrawFarPreFlag( bool f )		{ m_DrawFarPreFlag = f; }
	virtual bool GetDrawBadFlag()					{ return m_DrawBadFlag; }
	virtual void SetDrawBadFlag( bool f )			{ m_DrawBadFlag = f; }
	virtual bool GetDrawSymmFlag()					{ return m_DrawSymmFlag; }
	virtual void SetDrawSymmFlag( bool f )			{ m_DrawSymmFlag = f; }
	virtual bool GetDrawWakeFlag()					{ return m_DrawWakeFlag; }
	virtual void SetDrawWakeFlag( bool f )			{ m_DrawWakeFlag = f; }

	virtual bool GetFarMeshFlag()					{ return m_FarMeshFlag; }
	virtual void SetFarMeshFlag( bool f )			{ m_FarMeshFlag = f; }
	virtual bool GetFarCompFlag()					{ return m_FarCompFlag; }
	virtual void SetFarCompFlag(bool f )			{ m_FarCompFlag = f; }
	virtual bool GetFarManLocFlag()					{ return m_FarManLocFlag; }
	virtual void SetFarManLocFlag( bool f )			{ m_FarManLocFlag = f; }
	virtual bool GetFarAbsSizeFlag()				{ return m_FarAbsSizeFlag; }
	virtual void SetFarAbsSizeFlag( bool f )		{ m_FarAbsSizeFlag = f; }
	virtual bool GetHalfMeshFlag()					{ return m_HalfMeshFlag; }
	virtual void SetHalfMeshFlag( bool f )			{ m_HalfMeshFlag = f; }

	virtual void HighlightNextChain();	

	virtual void SetBatchFlag( bool f )				{ m_BatchFlag = f; }
	virtual bool GetBatchFlag()						{ return m_BatchFlag; }

	virtual void AddDelPuw( Puw* puw )				{ m_DelPuwVec.push_back( puw ); }
	virtual void AddDelIPnt( IPnt* ip )				{ m_DelIPntVec.push_back( ip ); }

	virtual void SetFarXScale( double s )			{ m_FarXScale = s; }
	virtual void SetFarYScale( double s )			{ m_FarYScale = s; }
	virtual void SetFarZScale( double s )			{ m_FarZScale = s; }
	virtual double GetFarXScale()					{ return m_FarXScale; }
	virtual double GetFarYScale()					{ return m_FarYScale; }
	virtual double GetFarZScale()					{ return m_FarZScale; }

	virtual void SetFarXLocation( double x )		{ m_FarXLocation = x; }
	virtual void SetFarYLocation( double y )		{ m_FarYLocation = y; }
	virtual void SetFarZLocation( double z )		{ m_FarZLocation = z; }
	virtual double GetFarXLocation()				{ return m_FarXLocation; }
	virtual double GetFarYLocation()				{ return m_FarYLocation; }
	virtual double GetFarZLocation()				{ return m_FarZLocation; }

	virtual void SetFarLength( double l )		{ m_FarLength = l; }
	virtual void SetFarWidth( double w )		{ m_FarWidth = w; }
	virtual void SetFarHeight( double h )		{ m_FarHeight = h; }
	virtual double GetFarLength()				{ return m_FarLength; }
	virtual double GetFarWidth()				{ return m_FarWidth; }
	virtual double GetFarHeight()				{ return m_FarHeight; }


	virtual void SetWakeScale( double s )			{ m_WakeScale = s; }
	virtual double GetWakeScale()					{ return m_WakeScale; }
	virtual void SetWakeAngle( double a )			{ m_WakeMgr.SetWakeAngle( a ); }
	virtual double GetWakeAngle()					{ return m_WakeMgr.GetWakeAngle(); }

	virtual void WriteChains();

	enum{ STL_FILE_NAME, POLY_FILE_NAME, TRI_FILE_NAME, 
		OBJ_FILE_NAME, DAT_FILE_NAME, KEY_FILE_NAME, GMSH_FILE_NAME, SRF_FILE_NAME, NUM_FILE_NAMES };

	Stringc GetExportFileName( int type );
	void SetExportFileName( const char* fn, int type );
	bool GetExportFileFlag( int type );
	void SetExportFileFlag( bool flag, int type );
	void ResetExportFileNames();

	void AddPossCoPlanarSurf( Surf* surfA, Surf* surfB );
	vector< Surf* > GetPossCoPlanarSurfs( Surf* surfPtr );

	void TestStuff();
	vector< vec3d > debugPnts;
	vector< vec2d > debugUWs;
	vector< SurfPatch* > debugPatches;

#ifdef DEBUG_CFD_MESH
	FILE* m_DebugFile;
	Stringc m_DebugDir;

	bool m_DebugDraw;
	vector< vector< vec3d > > m_DebugCurves;
	vector< vec3d > m_DebugColors;
#endif


protected:

	Aircraft* aircraftPtr;

	int m_CurrGeomID;
	int m_FarGeomID;
	bool m_BatchFlag;
	
	GridDensity m_GridDensity;
	vector< Surf* > m_SurfVec;

	Surf* m_YSlicePlane;

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

	bool m_DrawMeshFlag;
	bool m_DrawSourceFlag;
	bool m_DrawFarFlag;
	bool m_DrawFarPreFlag;
	bool m_DrawBadFlag;
	bool m_DrawSymmFlag;
	bool m_DrawWakeFlag;

	bool m_FarMeshFlag;
	bool m_FarCompFlag;
	bool m_FarManLocFlag;
	bool m_FarAbsSizeFlag;
	bool m_HalfMeshFlag;

	double m_FarXScale;
	double m_FarYScale;
	double m_FarZScale;

	double m_FarLength;
	double m_FarWidth;
	double m_FarHeight;

	double m_FarXLocation;
	double m_FarYLocation;
	double m_FarZLocation;

	double m_WakeScale;

	vector< Puw* > m_DelPuwVec;				// Store Created Puw and Ipnts
	vector< IPnt* > m_DelIPntVec;
	vector< IPntGroup* > m_DelIPntGroupVec;
	vector< ISegChain* > m_DelISegChainVec;

	vector< vector< vec3d > > debugRayIsect;

	bool m_ExportFileFlags[NUM_FILE_NAMES];
	Stringc m_ExportFileNames[NUM_FILE_NAMES];

	//==== Vector of Surfs that may have a border that lies on Surf A ====//
	map< Surf*, vector< Surf* > > m_PossCoPlanarSurfMap;

	vector<Edge*> m_BadEdges;
	vector<Tri*> m_BadTris;
	vector< Node* > m_nodeStore;

};

class CFDM_Single
{
public:
	CFDM_Single();
	CfdMeshMgr* cfdMeshMgr;
};


static CFDM_Single singleCFDM;

#define cfdMeshMgrPtr (singleCFDM.cfdMeshMgr)
#endif 



