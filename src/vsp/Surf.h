//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// Surf.h
// J.R Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(SURF_SURF__INCLUDED_)
#define SURF_SURF__INCLUDED_

#include "vec2d.h"
#include "vec3d.h"

#include "Mesh.h"
#include "GridDensity.h"
#include "SurfPatch.h"
#include "edgeSource.h"

#include <assert.h>

#include <vector>
#include <list>
#include <set>
using namespace std;

class CfdMeshMgr;
class SCurve;
class ISegChain;

//////////////////////////////////////////////////////////////////////
class Surf
{
public:

	Surf();
	virtual ~Surf();

	void BuildClean();
	void SetCfdMeshMgr( CfdMeshMgr* mgr )					{ m_CfdMeshMgr = mgr; }
	CfdMeshMgr* GetCfdMeshMgr()								{ return m_CfdMeshMgr; }

	void ReadSurf( FILE* file_id );
	void LoadControlPnts( vector< vector< vec3d > > & pnts );

	//===== Bezier Funcs ====//
	vec3d CompPnt( double u, double w );
	vec3d CompTanU( double u, double w );
	vec3d CompTanW( double u, double w );
	vec3d CompTanUU( double u, double w );
	vec3d CompTanWW( double u, double w );
	vec3d CompTanUW( double u, double w );

	vec3d CompPnt01( double u, double w );
	vec3d CompTanU01( double u, double w );
	vec3d CompTanW01( double u, double w );
	vec3d CompTanUU01( double u, double w );
	vec3d CompTanWW01( double u, double w );
	vec3d CompTanUW01( double u, double w );

	void CompCurvature( double u, double w, double& k1, double& k2, double& ka, double& kg );
	double TargetLen( double u, double w, double gap, double radfrac );
	void BuildTargetMap( GridDensity* grid_den, ESCloud &ms_cloud );
	void LimitTargetMap( GridDensity* grid_den, ESCloud &ms_cloud, ESTree &ms_tree );
	double InterpTargetMap( double u, double w );

	static void BlendFuncs(double u, double& F1, double& F2, double& F3, double& F4);
	static void BlendDerivFuncs(double u, double& F1, double& F2, double& F3, double& F4);
	static void BlendDeriv2Funcs(double u, double& F1, double& F2, double& F3, double& F4);

	vec2d ClosestUW( vec3d & pnt, double guess_u, double guess_w, double guess_del_u, double guess_del_w, double tol );

	vec2d ClosestUW( vec3d & pnt_in, double guess_u, double guess_w );
	void CompDeltaUW( vec3d& pnt_in, vec3d& guess_pnt, double norm_uw[2], double delta_uw[2]);

	bool LessThanY( double val );
	bool OnYZeroPlane();
	bool PlaneAtYZero();

	void FindBorderCurves();

	void SetGridDensityPtr( GridDensity*  gp )				{ m_GridDensityPtr = gp; m_Mesh.SetGridDensityPtr( gp ); }

	void SetCompID( int id )								{ m_CompID = id; }
	int  GetCompID()										{ return m_CompID; }
	void SetSurfID( int id )								{ m_SurfID = id; }
	int  GetSurfID()										{ return m_SurfID; }
	Stringc GetCompName()									{ return m_CompName; }

	void Draw();

	void LoadSCurves( vector< SCurve* > & scurve_vec );
	void BuildGrid();

	void WriteSTL( const char* filename );

	Mesh* GetMesh()											{ return &m_Mesh; }

	void Intersect( Surf* surfPtr );
	void IntersectLineSeg( vec3d & p0, vec3d & p1, vector< double > & t_vals );
	void IntersectLineSegMesh( vec3d & p0, vec3d & p1, vector< double > & t_vals );

	bbox& GetBBox()											{ return m_BBox; }
	vector< SurfPatch* >& GetPatchVec()						{ return m_PatchVec; }

	void InitMesh( vector< ISegChain* > chains );

	double GetUWArea()										{ return m_MaxU*m_MaxW; }

	double GetMaxU()											{ return m_MaxU; }
	double GetMaxW()											{ return m_MaxW; }
	void BuildDistMap();
	double GetUScale( double w );
	double GetWScale( double u );

	bool ValidUW( vec2d & uw );

	void LoadBorderCurves( vector< vector <vec3d> > & borderCurves ); 
	bool BorderCurveMatch( vector< vec3d > & curveA, vector< vec3d > & curveB );
	bool BorderMatch( Surf* otherSurf );

	void SetWakeFlag( bool flag )					{ m_WakeFlag = flag; }
	bool GetWakeFlag()								{ return m_WakeFlag; }

	vector< vector< vec3d > > GetControlPnts()		{ return m_Pnts; }

protected:

	int m_CompID;
	Stringc m_CompName;
	int m_SurfID;

	int m_NumU;
	int m_NumW;
	double m_MaxU;
	double m_MaxW;

	bool m_WakeFlag;

	vector< vector< vec3d > > m_Pnts;			// Control Pnts

	bbox m_BBox;
	vector< SurfPatch* > m_PatchVec;

	vector< SCurve* > m_SCurveVec;

	GridDensity* m_GridDensityPtr;

	Mesh m_Mesh;

	CfdMeshMgr* m_CfdMeshMgr;

	int m_NumMap;
	vector< vector< double > > m_TargetMap;

	int m_NumWScalePnts;
	bool m_ScaleUFlag;
	vector< double > m_UScaleMap;
	vector< double > m_WScaleMap;


	vec3d CompBez( double u, double w,
		void (*uBlendFun)(double u, double& F1, double& F2, double& F3, double& F4),
		void (*wBlendFun)(double u, double& F1, double& F2, double& F3, double& F4) );


};


#endif 
