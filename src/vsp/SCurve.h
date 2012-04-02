//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// SCurve.h
// J.R Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(SCURVE_SCURVE__INCLUDED_)
#define SCURVE_SCURVE__INCLUDED_

#include "Surf.h"
#include "GridDensity.h"
#include "bezier_curve.h"

#include "vec2d.h"
#include "vec3d.h"

#include <assert.h>

#include <vector>
#include <list>
using namespace std;

class ICurve;

//////////////////////////////////////////////////////////////////////
class SCurve
{
public:

	SCurve();
	SCurve(Surf* s);
	virtual ~SCurve();

	void ReadFile( const char* filename );

	void SetSurf( Surf* s )									{ m_Surf = s; }
	Surf* GetSurf()											{ return m_Surf; }

	double GetTargetLen( GridDensity* grid_den, SCurve* BCurve, vec3d p, vec3d uw, double u );

	void BorderTesselate( );
	void BuildDistTable( GridDensity* grid_den, SCurve* BCurve );
	void Tesselate( GridDensity* grid_den, SCurve* BCurve );
	void Tesselate( vector< vec3d > & target_pnts );

	void SetBezierControlPnts( vector< vec3d > & pnts );
	void BuildBezierCurve( vector< vec3d > & pnts_to_interpolate );

	double Length( int num_segs );

	void LoadControlPnts3D( vector< vec3d > & control_pnts );

	vector< double > GetUTessPnts()						{  return m_UTess; }
	void Tesselate( vector< double > & u_tess );

	vector< vec3d > GetUWTessPnts()						{ return m_UWTess; }

	void SetICurve( ICurve* iptr )						{ m_ICurve = iptr; }
	ICurve* GetICurve()									{ return m_ICurve; }

	void FlipDir();

	void Draw();

	vec3d CompPntUW( double u );

protected:

	Surf* m_Surf;				// Surface
	ICurve* m_ICurve;			// ICurve


	Bezier_curve m_UWCrv;		// UW Curve

	vector< double > m_UTess;	// Tess Curve Pnts in U Space
	vector< vec3d > m_UWTess;	// Tess Curve Pnts in UW Space

	int num_segs;
	vector< double > u_vec;
	vector< double > dist_vec;
	vector< double > target_vec;
};


#endif 
