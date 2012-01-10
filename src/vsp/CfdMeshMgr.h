//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// CfdMeshMgr.h
// J.R Gloudemans
//////////////////////////////////////////////////////////////////////

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
	virtual BaseSource* GetCurrSource();
	virtual void AddSource( int type );
	virtual BaseSource* CreateSource( int type );
	virtual void DeleteCurrSource();

	virtual void AdjustAllSourceLen( double mult );
	virtual void AdjustAllSourceRad( double mult );

	virtual void AddDefaultSources();
	virtual void UpdateSources();
	virtual void ScaleTriSize( double scale );

	virtual void Draw();
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

	virtual void BuildChains();
	virtual void ExpandChain( ISegChain* chain );

	virtual void IntersectSplitChains();
	virtual void IntersectYSlicePlane();

	virtual void MergeInteriorChainIPnts();

	virtual void LoadBorderCurves();
	virtual void SplitBorderCurves();
	virtual void MergeBorderEndPoints();
	virtual void MergeIPntGroups( list< IPntGroup* > & iPntGroupList, double tol );
	virtual void TessellateChains( GridDensity* grid_density );
	virtual void BuildMesh();
	virtual void RemoveInteriorTris();
	virtual void ConnectBorderEdges();
	virtual void MatchBorderEdges( list< Edge* > edgeList );

	virtual void DebugWriteChains( const char* name, bool tessFlag );

	virtual bool GetDrawMeshFlag()					{ return m_DrawMeshFlag; }
	virtual void SetDrawMeshFlag( bool f )			{ m_DrawMeshFlag= f; }
	virtual bool GetDrawSourceFlag()				{ return m_DrawSourceFlag; }
	virtual void SetDrawSourceFlag( bool f )		{ m_DrawSourceFlag= f; }
	virtual bool GetHalfMeshFlag()					{ return m_HalfMeshFlag; }
	virtual void SetHalfMeshFlag( bool f )			{ m_HalfMeshFlag= f; }

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

	virtual void WriteChains();

	enum{ STL_FILE_NAME, POLY_FILE_NAME, TRI_FILE_NAME, 
		OBJ_FILE_NAME, DAT_FILE_NAME, KEY_FILE_NAME, GMSH_FILE_NAME, SRF_FILE_NAME, NUM_FILE_NAMES };

	Stringc GetExportFileName( int type );
	void SetExportFileName( const char* fn, int type );
	bool GetExportFileFlag( int type );
	void SetExportFileFlag( bool flag, int type );
	void ResetExportFileNames();

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
	bool m_BatchFlag;
	
	GridDensity m_GridDensity;
	vector< Surf* > m_SurfVec;

	Surf* m_YSlicePlane;

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
	bool m_HalfMeshFlag;

	double m_FarXScale;
	double m_FarYScale;
	double m_FarZScale;

	vector< Puw* > m_DelPuwVec;				// Store Created Puw and Ipnts
	vector< IPnt* > m_DelIPntVec;
	vector< IPntGroup* > m_DelIPntGroupVec;
	vector< ISegChain* > m_DelISegChainVec;

	vector< vector< vec3d > > debugRayIsect;

	bool m_ExportFileFlags[NUM_FILE_NAMES];
	Stringc m_ExportFileNames[NUM_FILE_NAMES];

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



