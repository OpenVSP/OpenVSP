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
#include "SimpleMeshSettings.h"
#include "BezierCurve.h"

#include "Vec2d.h"
#include "Vec3d.h"

#include "MapSource.h"

#include <cassert>

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

    double GetTargetLen( SimpleGridDensity* grid_den, SCurve* BCurve, vec3d p, vec3d uw, double u );

    void BorderTesselate( );
    void CheapTesselate( );
    void ProjectTessToSurf( SCurve* sca );
    void InterpDistTable( double idouble, double &t, double &u, double &s, double &dsdi );
    void BuildDistTable( SimpleGridDensity* grid_den, SCurve* BCurve, list< MapSource* > & splitSources );
    void CleanupDistTable();
    void LimitTarget( SimpleGridDensity* grid_den );
    void TessEndPts();
    bool NewtonFind( double starget, double &s, double &ireal, double &t, double &dsdi, double &u );
    bool BisectFind( double starget, double &s, double &ireal, double &t, double &dsdi, double &u, int direction );
    void TessIntegrate( int direction, vector< double > &stess );
    void STessToUTess();
    void STessToUTess( const vector< double > &stess, vector< double > &utess );
    void TessIntegrateAndSmooth();
    void DoubleTess();
    void UWTess();
    void SpreadDensity( SCurve* BCurve );
    void CalcDensity( SimpleGridDensity* grid_den, SCurve* BCurve, list< MapSource* > & splitSources );
    void ApplyESSurface( double u, double t );
    void Tesselate();

    void InterpolateLinear(vector<vec3d> &pnts_to_interpolate);
    void PromoteTo( int deg );

    double Length( int num_segs );

    void GetBorderCurve( Bezier_curve & crv ) const;

    vector< double > GetUTessPnts()
    {
        return m_UTess;
    }
    void Tesselate( const vector< double > & u_tess );

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

    void SetUWCrv( const Bezier_curve &crv )
    {
        m_UWCrv = crv;
    }

    void FlipDir();

    // void Draw();

    vec3d CompPntUW( double u );

    void TessAdapt( const Surf& srf, double tol, int Nlimit );

protected:

    Surf* m_Surf;               // Surface
    ICurve* m_ICurve;           // ICurve


    Bezier_curve m_UWCrv;       // UW Curve

    vector< double > m_UTess;   // Tess Curve Pnts in U Space
    vector< vec3d > m_UWTess;   // Tess Curve Pnts in UW Space
    vector< double > m_STess;    // Tess Curve Pnts in arc-length

    int num_segs;
    vector< double > u_vec;
    vector< double > dist_vec;
    vector< double > target_vec;
};


#endif
