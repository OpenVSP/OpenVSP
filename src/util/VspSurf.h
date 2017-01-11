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
#include "XferSurf.h"

#include "STEPutil.h"

#include <api/dll_iges.h>
#include <api/dll_entity128.h>

#include "eli/code_eli.hpp"

#include "eli/geom/surface/bezier.hpp"
#include "eli/geom/surface/piecewise.hpp"
#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/surface/piecewise_general_skinning_surface_creator.hpp"
#include "eli/geom/surface/piecewise_cubic_spline_skinning_surface_creator.hpp"

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
    VspSurf();
    virtual ~VspSurf();

    // create surface as a body of revolution using the specified curve
    void CreateBodyRevolution( const VspCurve &input_crv );

    void SkinRibs( const vector<rib_data_type> &ribs, const vector < int > &degree, bool closed_flag );
    void SkinRibs( const vector<rib_data_type> &ribs, bool closed_flag );

    void SkinCubicSpline( const vector<rib_data_type> &ribs, const vector<double> &param, const vector <double> &tdisc, const vector < int > &degree, bool closed_flag );
    void SkinCubicSpline( const vector<rib_data_type> &ribs, const vector<double> &param, const vector <double> &tdisc, bool closed_flag );
    void SkinCubicSpline( const vector<rib_data_type> &ribs, const vector<double> &param, bool closed_flag );

    void SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, const vector < int > &degree, bool closed_flag );
    void SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, bool closed_flag );
    void SkinCX( const vector< VspCurve > &input_crv_vec, int cx, bool closed_flag );

    void SkinC0( const vector< VspCurve > &input_crv_vec, bool closed_flag );
    void SkinC1( const vector< VspCurve > &input_crv_vec, bool closed_flag );
    void SkinC2( const vector< VspCurve > &input_crv_vec, bool closed_flag );

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

    bool IsMagicVParm() const { return m_MagicVParm; }
    void SetMagicVParm( bool t ) { m_MagicVParm = t; }

    int GetSurfType() { return m_SurfType; }
    void SetSurfType( int type ) { m_SurfType = type; }

    int GetSurfCfdType() { return m_SurfCfdType; }
    void SetSurfCfdType( int type ) { m_SurfCfdType = type; }

    double FindNearest( double &u, double &w, const vec3d &pt ) const;
    double FindNearest( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const;

    double FindNearest01( double &u, double &w, const vec3d &pt ) const;
    double FindNearest01( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const;

    void GetUConstCurve( VspCurve &c, const double &u ) const;
    void GetWConstCurve( VspCurve &c, const double &w ) const;

    void GetU01ConstCurve( VspCurve &c, const double &u01 ) const;
    void GetW01ConstCurve( VspCurve &c, const double &w01 ) const;

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
    bool CapUMin(int capType, double len, double str, double offset, bool swflag);
    bool CapUMax(int capType, double len, double str, double offset, bool swflag);
    bool CapWMin(int capType);
    bool CapWMax(int capType);
    void FetchXFerSurf( const std::string &geom_id, int surf_ind, int comp_ind, vector< XferSurf > &xfersurfs );

    void ResetUWSkip();
    void FlagDuplicate( VspSurf *othersurf );

    void SetClustering( const double &le, const double &te );
    void SetRootTipClustering( const vector < double > &root, const vector < double > &tip );

    void MakeUTess( const vector<int> &num_u, std::vector<double> &utess, const std::vector<int> & umerge ) const;
    void MakeVTess( int num_v, std::vector<double> &vtess, const int &n_cap, bool degen ) const;

    //===== Tesselate ====//
    void TesselateTEforWake( std::vector< vector< vec3d > > & pnts ) const;

    void Tesselate( int num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts, const int &n_cap, bool degen ) const;
    void Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts, const int &n_cap, bool degen, const std::vector<int> & umerge = std::vector<int>() ) const;

    void SplitTesselate( int num_u, int num_v, std::vector< vector< vector< vec3d > > > & pnts,  std::vector< vector< vector< vec3d > > > & norms, const int &n_cap ) const;
    void SplitTesselate( const vector<int> &num_u, int num_v, std::vector< vector< vector< vec3d > > > & pnts,  std::vector< vector< vector< vec3d > > > & norms, const int &n_cap, const std::vector<int> & umerge = std::vector<int>() ) const;

    void TessUFeatureLine( int iu, std::vector< vec3d > & pnts, double tol );
    void TessWFeatureLine( int iw, std::vector< vec3d > & pnts, double tol );

    void TessAdaptLine( double umin, double umax, double wmin, double wmax, std::vector< vec3d > & pts, double tol, int Nlimit );
    void TessAdaptLine( double umin, double umax, double wmin, double wmax, const vec3d & pmin, const vec3d & pmax, std::vector< vec3d > & pts, double tol, int Nlimit, int Nadapt = 0 );

    void SplitSurfs( const piecewise_surface_type &basesurf, vector< piecewise_surface_type > &surfvec );

    void ExtractCPts( piecewise_surface_type &s, vector< vector< int > > &ptindxs, vector< vec3d > &allPntVec,
                      piecewise_surface_type::index_type &maxu, piecewise_surface_type::index_type &maxv,
                      piecewise_surface_type::index_type &nupatch, piecewise_surface_type::index_type &nvpatch,
                      piecewise_surface_type::index_type &nupts, piecewise_surface_type::index_type &nvpts );

    void ToSTEP_Bez_Patches( STEPutil * step, vector<SdaiBezier_surface *> &surfs );
    void ToSTEP_BSpline_Quilt( STEPutil * step, vector<SdaiB_spline_surface_with_knots *> &surfs, bool splitsurf, bool mergepts, bool tocubic, double tol, bool trimte );

    void ToIGES( DLL_IGES &model, bool splitsurf, bool tocubic, double tol, bool trimTE );

    void SetUSkipFirst( bool f );
    void SetUSkipLast( bool f );
    void SetWSkipFirst( bool f );
    void SetWSkipLast( bool f );

    int GetNumSectU() const;
    int GetNumSectW() const;

protected:

    void Tesselate( const vector<double> &utess, const vector<double> &vtess, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts ) const;
    void SplitTesselate( const vector<double> &usplit, const vector<double> &vsplit, const vector<double> &u, const vector<double> &v, std::vector< vector< vector< vec3d > > > & pnts,  std::vector< vector< vector< vec3d > > > & norms ) const;

    static void IGESKnots( int deg, int npatch, vector< double > &knot );

    bool CheckValidPatch( const piecewise_surface_type &surf );

    bool m_FlipNormal;
    bool m_MagicVParm;
    int m_SurfType;
    int m_SurfCfdType;
    piecewise_surface_type m_Surface;

    vector < double > m_UFeature;
    vector < double > m_WFeature;

    vector < bool > m_USkip;
    vector < bool > m_WSkip;

    double m_LECluster;
    double m_TECluster;

    vector < double > m_RootCluster;
    vector < double > m_TipCluster;

};
#endif
