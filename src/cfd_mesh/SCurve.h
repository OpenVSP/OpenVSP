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
#include "BezierCurve.h"

#include "Vec2d.h"
#include "Vec3d.h"

#include "MapSource.h"

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
    SCurve( Surf* s );
    virtual ~SCurve();

    void SetSurf( Surf* s )
    {
        m_Surf = s;
    }
    Surf* GetSurf()
    {
        return m_Surf;
    }

    double GetTargetLen( GridDensity* grid_den, SCurve* BCurve, vec3d p, vec3d uw, double u );

    void BorderTesselate( );
    void CheapTesselate( );
    void ProjectTessToSurf( SCurve* sca );
    void InterpDistTable( double idouble, double &t, double &u, double &s, double &dsdi );
    void BuildDistTable( GridDensity* grid_den, SCurve* BCurve, list< MapSource* > & splitSources );
    void CleanupDistTable();
    void LimitTarget( GridDensity* grid_den );
    void TessEndPts();
    void TessIntegrate();
    void TessRevIntegrate( vector< double > &utess );
    bool NewtonFind( double starget, double &s, double &ireal, double &t, double &dsdi, double &u, int direction );
    bool BisectFind( double starget, double &s, double &ireal, double &t, double &dsdi, double &u, int direction );
    void TessIntegrate( int direction, vector< double > &utess );
    void SmoothTess();
    void UWTess();
    void SpreadDensity( SCurve* BCurve );
    void CalcDensity( GridDensity* grid_den, SCurve* BCurve, list< MapSource* > & splitSources );
    void ApplyESSurface( double u, double t );
    void Tesselate();

    void SetBezierControlPnts( vector< vec3d > & pnts );
    void BuildBezierCurve( vector< vec3d > & pnts_to_interpolate );

    double Length( int num_segs );

    void LoadControlPnts3D( vector< vec3d > & control_pnts );

    void ExtractBorderControlPnts( vector< vec3d > & control_pnts );

    vector< double > GetUTessPnts()
    {
        return m_UTess;
    }
    void Tesselate( vector< double > & u_tess );

    vector< vec3d > GetUWTessPnts()
    {
        return m_UWTess;
    }

    void SetICurve( ICurve* iptr )
    {
        m_ICurve = iptr;
    }
    ICurve* GetICurve()
    {
        return m_ICurve;
    }

    Bezier_curve GetUWCrv()
    {
        return m_UWCrv;
    }

    void FlipDir();

    void Draw();

    vec3d CompPntUW( double u );

protected:

    Surf* m_Surf;               // Surface
    ICurve* m_ICurve;           // ICurve


    Bezier_curve m_UWCrv;       // UW Curve

    vector< double > m_UTess;   // Tess Curve Pnts in U Space
    vector< vec3d > m_UWTess;   // Tess Curve Pnts in UW Space

    int num_segs;
    vector< double > u_vec;
    vector< double > dist_vec;
    vector< double > target_vec;
    vector< vec3d >  pnt_vec;
};


#endif
