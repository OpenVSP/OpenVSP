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
#include "Matrix4d.h"
#include "VspCurve.h"
#include "Vsp1DCurve.h"
#include "BndBox.h"
#include "XferSurf.h"

#include "CADutil.h"

#include <api/dll_iges.h>
#include <api/dll_entity128.h>
#include <api/dll_entity406.h>

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

void SplitSurfsU( vector< piecewise_surface_type > &surfvec, const vector < double > &USplit );
void SplitSurfsW( vector< piecewise_surface_type > &surfvec, const vector < double > &WSplit );

class VspSurf
{
public:
    VspSurf();
    virtual ~VspSurf();

    void CopyNonSurfaceData( const VspSurf & s );

    // create surface as a body of revolution using the specified curve
    void CreateBodyRevolution( const VspCurve &input_crv, bool match_uparm = false, int iaxis = 0 );

    void CreateDisk( double dia, int ix = vsp::X_DIR, int iy = vsp::Y_DIR );

    void SkinRibs( const vector<rib_data_type> &ribs, const vector < int > &degree, const vector < double > & param, bool closed_flag );
    void SkinRibs( const vector<rib_data_type> &ribs, const vector < double > & param, bool closed_flag );
    void SkinRibs( const vector<rib_data_type> &ribs, const vector < int > &degree, bool closed_flag );
    void SkinRibs( const vector<rib_data_type> &ribs, bool closed_flag );

    void SkinCubicSpline( const vector<rib_data_type> &ribs, const vector<double> &param, const vector <double> &tdisc, const vector < int > &degree, bool closed_flag );
    void SkinCubicSpline( const vector<rib_data_type> &ribs, const vector<double> &param, const vector <double> &tdisc, bool closed_flag );
    void SkinCubicSpline( const vector<rib_data_type> &ribs, const vector<double> &param, bool closed_flag );

    void SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, const vector < int > &degree, const vector<double> &param, bool closed_flag );
    void SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, const vector < int > &degree, bool closed_flag );
    void SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, const vector< double > &param, bool closed_flag );
    void SkinCX( const vector< VspCurve > &input_crv_vec, const vector< int > &cx, bool closed_flag );
    void SkinCX( const vector< VspCurve > &input_crv_vec, int cx, const vector< double > &param, bool closed_flag );
    void SkinCX( const vector< VspCurve > &input_crv_vec, int cx, bool closed_flag );

    void SkinC0( const vector< VspCurve > &input_crv_vec, const vector<double> &param, bool closed_flag );
    void SkinC0( const vector< VspCurve > &input_crv_vec, bool closed_flag );
    void SkinC1( const vector< VspCurve > &input_crv_vec, bool closed_flag );
    void SkinC2( const vector< VspCurve > &input_crv_vec, bool closed_flag );

    double GetUMax() const;
    double GetUMapMax() const { return m_UMapMax; };
    double GetWMax() const;

    void ReverseUDirection();
    void ReverseWDirection();
    void SwapUWDirections();
    void Transform( Matrix4d & mat );
    void GetBoundingBox( BndBox &bb ) const;
    void GetLimitedBoundingBox( BndBox &bb, const double &U0, const double &Uf, const double &W0, const double &Wf );
    bool IsClosedU() const;
    bool IsClosedW() const;

    void RollU( const double &u );
    void RollW( const double &w );

    void SplitU( const double &u );
    void SplitW( const double &w );

    void JoinU( const VspSurf & sa, const VspSurf & sb );
    void JoinW( const VspSurf & sa, const VspSurf & sb );

    bool GetFlipNormal() const { return m_FlipNormal; }
    void FlipNormal() { m_FlipNormal = !m_FlipNormal; }
    void ResetFlipNormal( ) { m_FlipNormal = false; }

    bool IsMagicVParm() const { return m_MagicVParm; }
    void SetMagicVParm( bool t ) { m_MagicVParm = t; }

    bool IsHalfBOR() const { return m_HalfBOR; }
    void SetHalfBOR( bool t ) { m_HalfBOR = t; }

    int GetSurfType() const { return m_SurfType; }
    void SetSurfType( int type ) { m_SurfType = type; }

    int GetSurfCfdType() const { return m_SurfCfdType; }
    void SetSurfCfdType( int type ) { m_SurfCfdType = type; }

    bool GetThickSurf() { return m_ThickSurf; }
    void SetThickSurf( bool thicksurf ) { m_ThickSurf = thicksurf; }

    double FindNearest( double &u, double &w, const vec3d &pt ) const;
    double FindNearest( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const;

    double FindNearest01( double &u, double &w, const vec3d &pt ) const;
    double FindNearest01( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const;

    void FindDistanceAngle( double &u, double &w, const vec3d &pt, const vec3d &dir, const double &d, const double &theta, const double &u0, const double &w0 ) const;
    void GuessDistanceAngle( double &du, double &dw, const vec3d &udir, const vec3d & wdir, const double &d, const double &theta ) const;

    double FindRST( const vec3d & pt, const double &r0, const double &s0, const double &t0, double &r, double &s, double &t ) const;
    double FindRST( const vec3d & pt, double &r, double &s, double &t ) const;
    void FindRST( const vector < vec3d > & pt, vector < double > &r, vector < double > &s, vector < double > &t, vector < double > &d ) const;

    void ConvertRSTtoLMN( const double &r, const double &s, const double &t, double &l_out, double &m_out, double &n_out ) const;
    void ConvertRtoL( const double &r, double &l_out ) const;
    void ConvertLMNtoRST( const double &l, const double &m, const double &n, double &r_out, double &s_out, double &t_out ) const;
    void ConvertLtoR( const double &l, double &r_out ) const;

    double ProjectPt( const vec3d &inpt, const int &idir, double &u_out, double &w_out ) const;
    double ProjectPt( const vec3d &inpt, const int &idir, const double &u0, const double &w0, double &u_out, double &w_out ) const;

    double ProjectPt01( const vec3d &inpt, const int &idir, double &u_out, double &w_out ) const;
    double ProjectPt01( const vec3d &inpt, const int &idir, const double &u0, const double &w0, double &u_out, double &w_out ) const;

    bool IsInside( const vec3d &pt ) const;

    void GetUConstCurve( VspCurve &c, const double &u ) const;
    void GetWConstCurve( VspCurve &c, const double &w ) const;

    void GetU01ConstCurve( VspCurve &c, const double &u01 ) const;
    void GetW01ConstCurve( VspCurve &c, const double &w01 ) const;

    Matrix4d CompRotCoordSys( double u, double w ) const;
    Matrix4d CompTransCoordSys( const double &u, const double &w ) const;
    Matrix4d CompRotCoordSysRST( double r, double s, double t ) const;
    Matrix4d CompTransCoordSysRST(const double &r, const double &s, const double &t ) const;
    Matrix4d CompRotCoordSysLMN( const double &l, const double &m, const double &n ) const;
    Matrix4d CompTransCoordSysLMN( const double &l, const double &m, const double &n ) const;

    vec3d CompPntRST( double r, double s, double t ) const;
    vec3d CompTanR( double r, double s, double t ) const;
    vec3d CompTanS( double r, double s, double t ) const;
    vec3d CompTanT( double r, double s, double t ) const;

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
    vec3d CompAveNorm( double u, double v ) const;
    vec3d CompNorm01( double u, double v ) const;
    vec3d CompAveNorm01( double u, double v ) const;

    void CompCurvature( double u, double w, double& k1, double& k2, double& ka, double& kg ) const;
    void CompCurvature01( double u, double w, double& k1, double& k2, double& ka, double& kg ) const;

    int GetNumUFeature() const
    {
        return m_UFeature.size();
    }
    int GetNumWFeature() const
    {
        return m_WFeature.size();
    }
    void BuildSimpleFeatureLines();
    void BuildFeatureLines( bool force_xsec_flag = false );
    vector < double > GetUFeature() const
    {
        return m_UFeature;
    }
    vector < double > GetWFeature() const
    {
        return m_WFeature;
    }
    bool CapUMin(int capType, double len, double str, double offset, const vec3d &ptoff, bool swflag);
    bool CapUMax(int capType, double len, double str, double offset, const vec3d &ptoff, bool swflag);
    bool CapUHandler(int whichCap, int CapType, double len, double str, double offset, const vec3d &pt, bool swflag);

    void FetchXFerSurf( const std::string &geom_id, const std::string &name, int surf_ind, int comp_ind, int part_surf_num, vector< XferSurf > &xfersurfs, const vector < double > &usuppress = std::vector< double >(), const vector < double > &wsuppress = std::vector< double >() ) const;

    void ResetUSkip() const;
    void FlagDuplicate( const VspSurf &othersurf ) const;

    void SetClustering( const double &le, const double &te );
    void SetRootTipClustering( const vector < double > &root, const vector < double > &tip ) const;
    double GetRootCluster( const int &index ) const;
    double GetTipCluster( const int &index ) const;

    void MakeUTess( vector < double > &u, const vector < int > &num_u, const std::vector < int > &umerge, const int &n_cap, const int &n_default ) const;
    void MakeVTess( int num_v, std::vector<double> &vtess, const int &n_cap, bool degen ) const;

    //===== Tesselate ====//
    void TesselateTEforWake( std::vector< vector< vec3d > > & pnts ) const;
    void GetWakeTECurve( piecewise_curve_type& curve ) const;

    void Tesselate( int num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts, const int &n_cap, const int &n_default, bool degen ) const;
    void Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts, const int &n_cap, const int &n_default, bool degen, const std::vector<int> & umerge = std::vector<int>() ) const;

    void SplitTesselate( int num_u, int num_v, std::vector< vector< vector< vec3d > > > & pnts,  std::vector< vector< vector< vec3d > > > & norms, const int &n_cap, const int &n_default ) const;
    void SplitTesselate( const vector<int> &num_u, int num_v, std::vector< vector< vector< vec3d > > > & pnts,  std::vector< vector< vector< vec3d > > > & norms, const int &n_cap, const int &n_default, const std::vector<int> & umerge = std::vector<int>() ) const;

    void TessULine( double u, std::vector< vec3d > & pnts, double tol ) const;
    void TessUFeatureLine( int iu, std::vector< vec3d > & pnts, double tol ) const;
    void TessWFeatureLine( int iw, std::vector< vec3d > & pnts, double tol ) const;

    void TessAdaptLine( double umin, double umax, double wmin, double wmax, std::vector< vec3d > & pts, double tol, int Nlimit ) const;
    void TessAdaptLine( double umin, double umax, double wmin, double wmax, const vec3d & pmin, const vec3d & pmax, std::vector< vec3d > & pts, double tol, int Nlimit, int Nadapt = 0 ) const;

    void SplitSurfs( vector< piecewise_surface_type > &surfvec, const vector < double > &usuppress, const vector < double > &wsuppress ) const;
    void SplitSurfs( vector< piecewise_surface_type > &surfvec ) const;

    void TrimU( double u, bool before );
    void TrimV( double v, bool before );
    void TrimClosedV( double vstart, double vend );

    void ToSTEP_BSpline_Quilt( STEPutil *step, vector < SdaiB_spline_surface_with_knots * > &surfs,
                               const string &labelprefix, bool splitsurf, bool mergepts, bool tocubic, double tol,
                               bool trimte, bool mergeLETE, const vector < double > &USplit,
                               const vector < double > &WSplit, bool labelSplitNo, bool labelAirfoilPart,
                               const string &delim ) const;

    void ToIGES( IGESutil *iges, bool splitsurf, bool tocubic, double tol, bool trimTE, bool mergeLETE,
                 const vector < double > &USplit, const vector < double > &WSplit, const string &labelprefix,
                 bool labelSplitNo, bool labelAirfoilPart, const string &delim ) const;

    // Apply STEP or IGES settings to this VSPSurf in preparation for export
    vector < piecewise_surface_type >
    PrepCADSurfs( bool splitsurf, bool tocubic, double tol, bool trimTE, bool mergeLETE,
                  const vector < double > &USplit, const vector < double > &WSplit ) const;

    void SetUSkipFirst( int nskip, bool f );
    void SetUSkipLast( int nskip, bool f );

    piecewise_surface_type* GetBezierSurface()           { return &m_Surface; }

    enum { SKIN_NONE, SKIN_BODY_REV, SKIN_RIBS };

    int  GetSkinType()                                     const { return m_SkinType; }
    void GetBodyRevCurve( VspCurve & crv )                 const { crv = m_BodyRevCurve; }
    int  GetSkinClosedFlag()                               const { return m_SkinClosedFlag; }
    void GetSkinRibVec( vector< rib_data_type > & ribvec ) const { ribvec = m_SkinRibVec; }
    void GetSkinDegreeVec( vector< int > & degvec )        const { degvec = m_SkinDegreeVec; }
    void GetSkinParmVec( vector< double > & parmvec )      const { parmvec = m_SkinParmVec; }

    void SetClone( int index, const Matrix4d &mat )
    {
        m_CloneIndex = index;
        m_CloneMat = mat;
    }
    bool IsClone()
    {
        return m_CloneIndex != -1;
    }
    int GetCloneIndex()                                    const { return m_CloneIndex; }
    void GetCloneMat( Matrix4d &mat )                      const { mat = m_CloneMat; }

    int GetNumSectU() const;
    int GetNumSectW() const;

    void Offset( const vec3d &offvec );
    void OffsetX( double x );
    void OffsetY( double y );
    void OffsetZ( double Z );

    void Scale( double s );

    void ScaleX( double s );
    void ScaleY( double s );
    void ScaleZ( double s );

    void SetFoilSurf( VspSurf *s )                           { m_FoilSurf = s; }
    VspSurf *GetFoilSurf()                                   { return m_FoilSurf; }

    void MakePlaneSurf( const vec3d &ptA, const vec3d &ptB, const vec3d &ptC, const vec3d &ptD, double expand = 1.0 );

    void MakePolyPlaneSurf( const vector < vec3d > &up_pts, const vector < vec3d > &low_pts );

    void SetFeaOrientation( int otype, const vec3d &orient )
    {
        m_FeaOrientationType = otype;
        m_FeaOrientation = orient;
    }

    int GetFeaOrientationType() const
    {
        return m_FeaOrientationType;
    }

    vec3d GetFeaOrientation() const
    {
        return m_FeaOrientation;
    }

    void SetFeaSymmIndex( int s )
    {
        m_FeaSymmIndex = s;
    }

    int GetFeaSymmIndex()
    {
        return m_FeaSymmIndex;
    }

    void DegenCamberSurf( const VspSurf & parent );
    void DegenPlanarSurf( const VspSurf & parent, int vhflag );

    int GetPlateNum() { return m_PlateNum; }
    void SetPlateNum( int p ) { m_PlateNum = p; }

    void InitUMapping();
    void InitUMapping( double val );
    void PrintUMapping() const;

    void ParmReport();

    double InvertUMapping( double u ) const;
    double EvalUMapping( double u ) const;

    void BuildLCurve();
    void BuildMCurve( const double &r, Vsp1DCurve &c ) const;
    double GetLMax() const { return m_Lmax; };

    void SetPlanarUWAspect( double uwAspect ) { m_PlanarUWAspect = uwAspect; };
    double GetPlanarUWAspect() const { return m_PlanarUWAspect; };

protected:

    void Tesselate( const vector<double> &utess, const vector<double> &vtess, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts ) const;
    void SplitTesselate( const vector<double> &usplit, const vector<double> &vsplit, const vector<double> &u, const vector<double> &v, std::vector< vector< vector< vec3d > > > & pnts,  std::vector< vector< vector< vec3d > > > & norms ) const;

    static bool CheckValidPatch( const piecewise_surface_type &surf );

    bool m_FlipNormal;
    bool m_MagicVParm;
    bool m_HalfBOR;
    int m_SurfType;
    int m_SurfCfdType;

    // Possibly shouldn't be here, but seems as good as any place for now.
    int m_FeaOrientationType;
    vec3d m_FeaOrientation;
    int m_FeaSymmIndex;

    bool m_ThickSurf;
    int m_PlateNum;
    piecewise_surface_type m_Surface;

    vector < double > m_UFeature;
    vector < double > m_WFeature;

    Vsp1DCurve m_UMapping;
    double m_UMapMax;

    double m_Lmax;
    Vsp1DCurve m_LCurve;

    double m_LECluster;
    double m_TECluster;

    //==== Store Skinning Inputs =====//
    int m_SkinType;
    VspCurve m_BodyRevCurve;
    vector< rib_data_type > m_SkinRibVec;
    vector< int > m_SkinDegreeVec;
    vector< double > m_SkinParmVec;
    int m_SkinClosedFlag;

    int m_CloneIndex;
    Matrix4d m_CloneMat;

    VspSurf *m_FoilSurf;

    double m_PlanarUWAspect;

private:
    mutable vector < bool > m_USkip;

    mutable vector < double > m_RootCluster;
    mutable vector < double > m_TipCluster;

};
#endif
