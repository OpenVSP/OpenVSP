//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(SURF_CORE__INCLUDED_)
#define SURF_CORE__INCLUDED_

#include "Vec3d.h"

#include "eli/code_eli.hpp"

#include "eli/geom/surface/bezier.hpp"
#include "eli/geom/surface/piecewise.hpp"
#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/surface/piecewise_general_skinning_surface_creator.hpp"
#include "eli/geom/intersect/minimum_distance_surface.hpp"

typedef eli::geom::surface::bezier<double, 3> surface_patch_type;
typedef eli::geom::surface::piecewise<eli::geom::surface::bezier, double, 3> piecewise_surface_type;
typedef piecewise_surface_type::point_type surface_point_type;

typedef piecewise_surface_type::tolerance_type surface_tolerance_type;

typedef eli::geom::curve::piecewise<eli::geom::curve::bezier, double, 3> piecewise_curve_type;

#include <vector>
using std::vector;

class Bezier_curve;
class Surf;

//////////////////////////////////////////////////////////////////////
class SurfCore
{
public:

    enum { UMIN, UMAX, WMIN, WMAX, NOBNDY = -1};

    SurfCore();
    virtual ~SurfCore();

    //===== Bezier Funcs ====//
    vec3d CompPnt( double u, double w ) const;
    vec3d CompTanU( double u, double w ) const;
    vec3d CompTanW( double u, double w ) const;

    vec3d CompNorm( double u, double w ) const;

    vec3d CompPnt01( double u, double w ) const;

    void CompCurvature( double u, double w, double& k1, double& k2, double& ka, double& kg ) const;

    int UWPointOnBorder( double u, double w, double tol ) const;

    int GetNumUPatches() const
    {
        return m_Surface.number_u_patches();
    }
    int GetNumWPatches() const
    {
        return m_Surface.number_v_patches();
    }
    double GetMinU() const
    {
        return m_Surface.get_u0();
    }
    double GetMinW() const
    {
        return m_Surface.get_v0();
    }
    double GetMaxU() const
    {
        return m_Surface.get_umax();
    }
    double GetMaxW() const
    {
        return m_Surface.get_vmax();
    }
    double GetDU() const
    {
        return m_Surface.get_umax() - m_Surface.get_u0();
    }
    double GetDW() const
    {
        return m_Surface.get_vmax() - m_Surface.get_v0();
    }
    double GetMidU() const
    {
        return ( m_Surface.get_u0() + m_Surface.get_umax() ) / 2.0;
    }
    double GetMidW() const
    {
        return ( m_Surface.get_v0() + m_Surface.get_vmax() ) / 2.0;
    }

    void SetSurf( const piecewise_surface_type &surf )
    {
        m_Surface = surf;
    }

    piecewise_surface_type * GetSurf()
    {
        return &m_Surface;
    }

    void GetBorderCurve( const vec3d &uw0, const vec3d &uw1, Bezier_curve &crv ) const;

    bool LessThanY( double val ) const;
    bool PlaneAtYZero() const;

    Bezier_curve GetBorderCurve( int iborder ) const;
    void LoadBorderCurves( vector < Bezier_curve > & borderCurves ) const;

    bool SurfMatch( SurfCore* otherSurf ) const;

    void WriteSurf( FILE* fp ) const;

    void MakeWakeSurf( const piecewise_curve_type & lecrv, double endx, double angle, double start_stretch_x = 0, double scale = 1 );
    void MakePlaneSurf( const threed_point_type &p0, const threed_point_type &p1, const threed_point_type &p2, const threed_point_type &p3 );

    void BuildPatches( Surf* srf ) const;

    double FindNearest( double &u, double &w, const vec3d &pt, double u0, double w0 ) const;
    double FindNearest( double &u, double &w, const vec3d &pt, double u0, double w0, double umin, double umax, double vmin, double vmax ) const;
    double FindNearest( double &u, double &w, const vec3d &pt ) const;

protected:
    piecewise_surface_type m_Surface;

    bool MatchThisOrientation( const piecewise_surface_type &osurf ) const;

    vec3d CompTanUU( double u, double w ) const;
    vec3d CompTanWW( double u, double w ) const;
    vec3d CompTanUW( double u, double w ) const;
};

#endif
