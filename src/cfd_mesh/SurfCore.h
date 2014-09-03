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

typedef eli::geom::surface::bezier<double, 3> surface_patch_type;
typedef eli::geom::surface::piecewise<eli::geom::surface::bezier, double, 3> piecewise_surface_type;
typedef piecewise_surface_type::point_type surface_point_type;

typedef piecewise_surface_type::tolerance_type surface_tolerance_type;
typedef eli::geom::curve::piecewise_cubic_spline_creator<double, 3, surface_tolerance_type> piecewise_cubic_spline_creator_type;
typedef eli::geom::surface::connection_data<double, 3, surface_tolerance_type> rib_data_type;

#include <vector>
using std::vector;

class Bezier_curve;

//////////////////////////////////////////////////////////////////////
class SurfCore
{
public:

    enum { UMIN, UMAX, WMIN, WMAX};

    SurfCore();
    virtual ~SurfCore();

    //===== Bezier Funcs ====//
    vec3d CompPnt( double u, double w ) const;
    vec3d CompTanU( double u, double w ) const;
    vec3d CompTanW( double u, double w ) const;

    vec3d CompPnt01( double u, double w ) const;
    vec3d CompTanU01( double u, double w ) const;
    vec3d CompTanW01( double u, double w ) const;

    void CompCurvature( double u, double w, double& k1, double& k2, double& ka, double& kg ) const;

    double GetUWArea() const
    {
        return GetMaxU() * GetMaxW();
    }

    int GetNumUPatches() const
    {
        return m_Surface.number_u_patches();
    }
    int GetNumWPatches() const
    {
        return m_Surface.number_v_patches();
    }
    double GetMaxU() const
    {
        return m_Surface.get_umax();
    }
    double GetMaxW() const
    {
        return m_Surface.get_vmax();
    }

    void SetControlPnts( vector< vector < vec3d > > pnts );
    vector< vector< vec3d > > GetControlPnts() const;
    void ExtractBorderControlPnts( const vec3d &uw0, const vec3d &uw1, Bezier_curve &crv );

    bool LessThanY( double val ) const;
    bool OnYZeroPlane() const;
    bool PlaneAtYZero() const;

    Bezier_curve GetBorderCurve( int iborder ) const;
    void LoadBorderCurves( vector < Bezier_curve > & borderCurves ) const;

    bool SurfMatch( SurfCore* otherSurf ) const;

    void WriteSurf( FILE* fp ) const;

    void MakeWakeSurf( const Bezier_curve &lecrv, double endx, double angle );

protected:
    piecewise_surface_type m_Surface;

    vec3d CompTanUU( double u, double w ) const;
    vec3d CompTanWW( double u, double w ) const;
    vec3d CompTanUW( double u, double w ) const;
};

#endif
