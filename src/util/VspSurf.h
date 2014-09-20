//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspSurf.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_SURF_H
#define VSP_SURF_H

#include "Vec3d.h"
#include "Matrix.h"
#include "VspCurve.h"
#include "BndBox.h"

#include "STEPutil.h"

#include "eli/code_eli.hpp"

#include "eli/geom/surface/bezier.hpp"
#include "eli/geom/surface/piecewise.hpp"
#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/surface/piecewise_general_skinning_surface_creator.hpp"

typedef eli::geom::surface::bezier<double, 3> surface_patch_type;
typedef eli::geom::surface::piecewise<eli::geom::surface::bezier, double, 3> piecewise_surface_type;

typedef piecewise_surface_type::tolerance_type surface_tolerance_type;
typedef eli::geom::curve::piecewise_cubic_spline_creator<double, 3, surface_tolerance_type> piecewise_cubic_spline_creator_type;
typedef eli::geom::surface::connection_data<double, 3, surface_tolerance_type> rib_data_type;

#include <vector>
#include <string>
using std::vector;

class VspSurf
{
public:

    enum SURF_TYPE {    NORMAL_SURF,
                        WING_SURF,
                        NUM_SURF_TYPES,
                   };

    enum { NO_END_CAP,
           FLAT_END_CAP,
           NUM_END_CAP_OPTIONS
         };

    VspSurf();
    virtual ~VspSurf();

    // create surface as a body of revolution using the specified curve
    void CreateBodyRevolution( const VspCurve &input_crv );

    void SkinRibs( const vector<rib_data_type> &ribs, const vector < int > &degree, bool closed_flag );
    void SkinRibs( const vector<rib_data_type> &ribs, bool closed_flag );

    void SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, const vector < int > &degree, bool closed_flag );
    void SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, bool closed_flag );
    void SkinCX( const vector< VspCurve > &input_crv_vec, int cx, bool closed_flag );

    void SkinC0( const vector< VspCurve > &input_crv_vec, bool closed_flag );
    void SkinC1( const vector< VspCurve > &input_crv_vec, bool closed_flag );
    void SkinC2( const vector< VspCurve > &input_crv_vec, bool closed_flag );

    int GetNumSectU() const;
    int GetNumSectW() const;

    double GetUMax() const;
    double GetWMax() const;

    void ReverseUDirection();
    void ReverseWDirection();
    void SwapUWDirections();
    void Transform( Matrix4d & mat );
    void GetBoundingBox( BndBox &bb ) const;
    bool IsClosedU() const;
    bool IsClosedW() const;

    bool GetFlipNormal() { return m_FlipNormal; }
    void FlipNormal() { m_FlipNormal = !m_FlipNormal; }
    void ResetFlipNormal( ) { m_FlipNormal = false; }

    int GetSurfType() { return m_SurfType; }
    void SetSurfType( int type ) { m_SurfType = type; }

    double FindNearest( double &u, double &w, const vec3d &pt ) const;
    double FindNearest( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const;

    double FindNearest01( double &u, double &w, const vec3d &pt ) const;
    double FindNearest01( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const;

    void GetUConstCurve( VspCurve &c, const double &u ) const;
    void GetWConstCurve( VspCurve &c, const double &w ) const;

    Matrix4d CompRotCoordSys( const double &u, const double &w );
    Matrix4d CompTransCoordSys( const double &u, const double &w );

    //===== Bezier Funcs ====//
    vec3d CompPnt( double u, double v ) const;
    vec3d CompTanU( double u, double v ) const;
    vec3d CompTanW( double u, double v ) const;
    vec3d CompTanUU( double u, double v ) const;
    vec3d CompTanWW( double u, double v ) const;
    vec3d CompTanUW( double u, double v ) const;

    vec3d CompPnt01( double u, double v ) const;
    vec3d CompTanU01( double u, double v ) const;
    vec3d CompTanW01( double u, double v ) const;
    vec3d CompTanUU01( double u, double v ) const;
    vec3d CompTanWW01( double u, double v ) const;
    vec3d CompTanUW01( double u, double v ) const;

    vec3d CompNorm( double u, double v ) const;
    vec3d CompNorm01( double u, double v ) const;

    int GetNumUFeature()
    {
        return m_UFeature.size();
    }
    int GetNumWFeature()
    {
        return m_WFeature.size();
    }
    void BuildFeatureLines();
    bool CapUMin(int capType);
    bool CapUMax(int capType);
    bool CapWMin(int capType);
    bool CapWMax(int capType);
    void WriteBezFile( FILE* id, const std::string &geom_id, int surf_ind );

    void ResetUWSkip();
    void FlagDuplicate( VspSurf *othersurf );

    //===== Tesselate ====//
    void Tesselate( int num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms ) const;
    void Tesselate( int num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts ) const;
    void Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms ) const;
    void Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts ) const;

    void TessUFeatureLine( int iu, int num_v, std::vector< vec3d > & pnts );
    void TessWFeatureLine( int iw, int num_u, std::vector< vec3d > & pnts );
    void TessLine( double umin, double umax, double wmin, double wmax, int numpts, std::vector< vec3d > & pnts );

    void SplitSurfs( vector< piecewise_surface_type > &surfvec );
    void SplitSurfs( vector< piecewise_surface_type > &surfvec, piecewise_surface_type initsurf );

    void ToSTEP_Bez_Patches( STEPutil * step, vector<SdaiBezier_surface *> &surfs );
    void ToSTEP_BSpline_Quilt( STEPutil * step, vector<SdaiB_spline_surface_with_knots *> &surfs );

protected:
    int ClosestPatchEnd( const vector<double> & patch_endings, double end_val ) const;

protected:

    bool m_FlipNormal;
    int m_SurfType;
    piecewise_surface_type m_Surface;

    vector < double > m_UFeature;
    vector < double > m_WFeature;

    vector < bool > m_USkip;
    vector < bool > m_WSkip;
};
#endif
