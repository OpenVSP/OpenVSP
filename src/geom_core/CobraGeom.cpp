//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES
#include <cmath>
#include "CobraGeom.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "VspCurve.h"
#include "VspSurf.h"
#include "Vehicle.h"

//==== Constructor ====//
CobraGeom::CobraGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "CobraGeom";
    m_Type.m_Name = "Cobra";
    m_Type.m_Type = COBRA_GEOM_TYPE;

    //==== Init Parms ====//
    m_TessU = 10;
    m_TessW = 8;

    m_XradN.Init( "XradN", "Design", this, 1.0, 0.0, 1.0e12 );
    m_YradN.Init( "YradN", "Design", this, 1.0, 0.0, 1.0e12 );
    m_PowX.Init( "PowX", "Design", this, 2.0, 1.0, 10.0 );

    m_ZradNL.Init( "ZradNL", "Design", this, 1.0, 0.0, 1.0e12 );
    m_PowNL.Init( "PowNL", "Design", this, 2.0, 1.0, 10.0 );
    m_NoseULSymm.Init( "NoseULSymm", "Design", this, true, false, true );
    m_ZradNU.Init( "ZradNU", "Design", this, 1.0, 0.0, 1.0e12 );
    m_PowNU.Init( "PowNU", "Design", this, 2.0, 1.0, 10.0 );

    m_XlenA.Init( "XlenA", "Design", this, 1.0, 0.0, 1.0e12 );
    m_YradA.Init( "YradA", "Design", this, 1.0, 0.0, 1.0e12 );
    m_ZradAL.Init( "ZradAL", "Design", this, 1.0, 0.0, 1.0e12 );
    m_PowAL.Init( "PowAL", "Design", this, 2.0, 1.0, 10.0 );
    m_AftULSymm.Init( "AftULSymm", "Design", this, true, false, true );
    m_ZradAU.Init( "ZradAU", "Design", this, 1.0, 0.0, 1.0e12 );
    m_PowAU.Init( "PowAU", "Design", this, 2.0, 1.0, 10.0 );

    m_Xoff = 0.0;
}

//==== Destructor ====//
CobraGeom::~CobraGeom()
{

}

struct funCOBRAxyz
{
    void operator()( const double &u, curve_point_type &f, curve_point_type &fp ) const
    {
        double xrib = m_xmin + m_xrng * pow( u, m_powX );
        double dxdu = m_powX * m_xrng * pow( u, m_powX - 1.0 );

        double yrib, zribU;
        double dyribdu, dzribdu;

        // zribU based on Aftbody Keel
        SE_SingleSuperEllipse_FixedPt_TangentLine_Calc(
            m_xrng, m_xradN,
            m_zradN, m_powX, m_xlenA, m_zradA, m_XAftKeelTanPt, m_ZAftKeelTanPt,
            m_alineKeel, m_xlineKeel, xrib, zribU, dzribdu );

        // yrib based on Aftbody Waterline
        SE_SingleSuperEllipse_FixedPt_TangentLine_Calc(
            m_xrng, m_xradN,
            m_yradN, m_powX, m_xlenA, m_yradA, m_XAftWaterLineTanPt, m_YAftWaterLineTanPt,
            m_alineWL, m_xlineWL, xrib, yrib, dyribdu );

        double dpdu = 0;
        double pow_ribU = m_powN;
        if ( xrib > m_XpowN )
        {

            double dpow = m_powA - m_powN;
            double dxR = M_PI * fabs( ( xrib - m_XpowN ) / ( m_xmax - m_XpowN ) );
            pow_ribU = m_powN + 0.5 * dpow * ( 1.0 - cos( dxR ) );
            dpdu = ( 0.5 * dpow * sin( dxR ) * M_PI / ( m_xmax - m_XpowN ) ) * dxdu;
        }

        double zrib = zribU;
        double pow_rib = pow_ribU;

        double y1 = cos( m_theta );
        double z1 = sin( m_theta );

        double yi, zi;
        double dydu, dzdu;
        SQ_SuperEllipse2D_YZCalc( yrib, zrib, pow_rib, dyribdu, dzribdu, dpdu, y1, z1, yi, zi, dydu, dzdu );

        f << xrib, yi, zi;
        fp << dxdu, dydu, dzdu;
    }

    double m_theta;
    double m_xmin, m_xrng, m_powX;
    double m_xmax;
    double m_xradN, m_yradN, m_zradN, m_powN, m_xlenA, m_yradA, m_zradA, m_powA;
    double m_XAftWaterLineTanPt, m_YAftWaterLineTanPt;
    double m_alineWL, m_xlineWL;

    double m_XAftKeelTanPt, m_ZAftKeelTanPt;

    double m_alineKeel, m_xlineKeel;
    double m_XpowN;

    void Set( double theta,
              double xmin, double xrng, double powX,
              double xmax,
              double xradN, double yradN, double zradN, double powN, double xlenA, double yradA, double zradA, double powA,
              double XAftWaterLineTanPt, double YAftWaterLineTanPt,
              double alineWL, double xlineWL,
              double XAftKeelTanPt, double ZAftKeelTanPt,
              double alineKeel, double xlineKeel,
              double XpowN )
    {
        m_theta = theta;
        m_xmin = xmin;
        m_xrng = xrng;
        m_powX = powX;
        m_xmax = xmax;
        m_xradN = xradN;
        m_yradN = yradN;
        m_zradN = zradN;
        m_powN = powN;
        m_xlenA = xlenA;
        m_yradA = yradA;
        m_zradA = zradA;
        m_powA = powA;
        m_XAftWaterLineTanPt = XAftWaterLineTanPt;
        m_YAftWaterLineTanPt = YAftWaterLineTanPt;
        m_alineWL = alineWL;
        m_xlineWL = xlineWL;
        m_XAftKeelTanPt = XAftKeelTanPt;
        m_ZAftKeelTanPt = ZAftKeelTanPt;
        m_alineKeel = alineKeel;
        m_xlineKeel = xlineKeel;
        m_XpowN = XpowN;
    }
};

struct funSuperEllipse
{
    void operator() ( const double &v, curve_point_type &p, curve_point_type &dp ) const
    {
        // double v = 4.0 * theta / ( 2.0 * M_PI );

        double theta = 2.0 * M_PI * v / 4.0;

        double ypt, zpt;
        double dydtheta, dzdtheta;

        SQ_SuperEllipse2D_YZCalc_dtheta ( m_yrib, m_zrib, m_pow_rib,
                                          theta,
                                          ypt, zpt,
                                          dydtheta, dzdtheta );

        double dydv = dydtheta * 2.0 * M_PI / 4.0;
        double dzdv = dzdtheta * 2.0 * M_PI / 4.0;

        p << 0, ypt, zpt;
        dp << 0, dydv, dzdv;
    }

    double m_yrib, m_zrib, m_pow_rib;

    void Set( double yrib, double zrib, double pow_rib )
    {
        m_yrib = yrib;
        m_zrib = zrib;
        m_pow_rib = pow_rib;
    }
};

void CobraGeom::UpdateSurf()
{
    double xo = m_Xoff;

    if ( m_NoseULSymm() )
    {
        m_ZradNL = m_ZradNU();
        m_PowNL = m_PowNU();
    }

    if ( m_AftULSymm() )
    {
        m_ZradAL = m_ZradAU();
        m_PowAL = m_PowAU();
    }

    double xradN = m_XradN();
    double yradN = m_YradN();
    double powX = m_PowX();
    double zradNL = m_ZradNL();
    double powNL = m_PowNL();
    double zradNU = m_ZradNU();
    double powNU = m_PowNU();
    double xlenA = m_XlenA();
    double yradA = m_YradA();
    double zradAL = m_ZradAL();
    double powAL = m_PowAL();
    double zradAU = m_ZradAU();
    double powAU = m_PowAU();

    double XAftWaterLineTanPt, YAftWaterLineTanPt;
    double alineWL, xlineWL;

    double XAftLKeelTanPt, ZAftLKeelTanPt;
    double XAftUKeelTanPt, ZAftUKeelTanPt;
    double alineLKeel, xlineLKeel;
    double alineUKeel, xlineUKeel;

    // Aftbody Waterline
    SE_SingleSuperEllipse_FixedPt_TangentLine_Define(
        xradN,
        yradN, powX, xlenA, yradA, XAftWaterLineTanPt, YAftWaterLineTanPt,
        alineWL, xlineWL );

    // funSuperEllipse fe{};
    // fe.Set( xradN, yradN, powX );
    //
    // VspCurve celip;
    // int de = celip.FunToBinaryCubic( fe, 0.0, 2.0 );
    //
    // static int figno = 0;
    // figno++;
    //
    // celip.GetCurve().octave_print( figno );
    //
    // printf( "xradN = %g;\n", xradN );
    // printf( "yradN = %g;\n", yradN );
    //
    //
    // printf( "xlenA = %g;\n", xlenA );
    // printf( "yradA = %g;\n", yradA );
    // printf( "XAftWaterLineTanPt = %g;\n", XAftWaterLineTanPt );
    // printf( "YAftWaterLineTanPt = %g;\n", YAftWaterLineTanPt );
    //
    // printf( "plot( [xlenA, XAftWaterLineTanPt], [yradA, YAftWaterLineTanPt], 'k-x' );\n" );

    // Aftbody Lower Keel
    SE_SingleSuperEllipse_FixedPt_TangentLine_Define(
        xradN,
        zradNL, powX, xlenA, zradAL, XAftLKeelTanPt, ZAftLKeelTanPt,
        alineLKeel, xlineLKeel );

    bool ulsymm = false;
    // Aftbody Upper Keel
    if ( ( zradNL == zradNU ) && ( powNL == powNU ) &&
         ( zradAL == zradAU ) && ( powAL == powAU ) )
    {
        XAftUKeelTanPt = XAftLKeelTanPt;
        ZAftUKeelTanPt = ZAftLKeelTanPt;
        alineUKeel = alineLKeel;
        xlineUKeel = xlineLKeel;
        ulsymm = true;
    }
    else
    {
        SE_SingleSuperEllipse_FixedPt_TangentLine_Define(
            xradN,
            zradNU, powX, xlenA, zradAU, XAftUKeelTanPt, ZAftUKeelTanPt,
            alineUKeel, xlineUKeel );
    }

    double XpowNL = 0.0;
    if ( XAftWaterLineTanPt > XpowNL ) XpowNL = XAftWaterLineTanPt;
    if ( XAftLKeelTanPt > XpowNL ) XpowNL = XAftLKeelTanPt;

    double XpowNU = 0.0;
    if ( XAftWaterLineTanPt > XpowNU ) XpowNU = XAftWaterLineTanPt;
    if ( XAftUKeelTanPt > XpowNU ) XpowNU = XAftUKeelTanPt;


    double xmin = -xradN;
    double xmax = xlenA;
    double xrng = xmax - xmin;

    double ymax = 2.0 * std::max( yradN, yradA );
    double zmaxN = std::max( zradNL, zradNU );
    double zmaxA = std::max( zradAL, zradAU );
    double zmax = 2.0 * std::max( zmaxN, zmaxA );

    double reflen = sqrt( xrng * xrng + ymax * ymax + zmax * zmax);
    double tol = reflen * 1e-5;

    // Find v parameter values through adaptation of nose base and aft curves.
    vector < double > v;
    {
        vector < double > yvec;
        vector < double > zvec;
        vector < double > powvec;

        yvec.push_back( yradN );
        zvec.push_back( zradNL );
        powvec.push_back( powNL );

        if ( ( zradNL != zradNU ) || ( powNL != powNU ) )
        {
            yvec.push_back( yradN );
            zvec.push_back( zradNU );
            powvec.push_back( powNU );
        }

        yvec.push_back( yradA );
        zvec.push_back( zradAL );
        powvec.push_back( powAL );

        if ( ( zradAL != zradAU ) || ( powAL != powAU ) )
        {
            yvec.push_back( yradA );
            zvec.push_back( zradAU );
            powvec.push_back( powAU );
        }


        // Adapt vm to each sample super ellipse using one quadrant.  Concatenate into vmap.
        vector < double > vmap;
        for ( int i = 0; i < yvec.size(); i++ )
        {
            funSuperEllipse fe{};

            fe.Set( yvec[i], zvec[i], powvec[i] );

            VspCurve celip;
            int de = celip.FunToBinaryCubic( fe, 0.0, 1.0, tol );

            vector < double > vm;
            celip.GetCurve().get_pmap( vm );

            vmap.insert( vmap.end(), vm.begin(), vm.end() );
        }
        // Reduce vmap into unique values.
        std::sort( vmap.begin(), vmap.end() );
        vmap.erase( std::unique( vmap.begin(), vmap.end() ), vmap.end() );

        // Convert vmap into dv.
        vector < double > dv( vmap.size() - 1 );
        for ( int i = 0; i < vmap.size() - 1; i++ )
        {
            dv[i] = vmap[i + 1] - vmap[i];
        }

        // Assemble dv into four quadrants.
        vector < double > dvcopy = dv;
        dv.insert( dv.end(), dvcopy.rbegin(), dvcopy.rend() );
        dvcopy = dv;
        dv.insert( dv.end(), dvcopy.begin(), dvcopy.end() );

        // Construct v from cumulative sum of dv.
        v.resize( dv.size() + 1, 0.0 );
        double vi = 0;
        for ( int i = 0; i < dv.size(); i++ )
        {
            vi += dv[ i ];
            v[ i + 1 ] = vi;
        }
    }
    int npt = v.size();


    // Find u parameter valuesthrough adaptation along curve at 'average' corner theta
    vector < double > urib;
    {
        // Sample upper side first.
        // Find Nose and Aft 'corner' thetas, average for sample theta.
        double thetaN = atan( zradNU / yradN );
        double thetaA = atan( zradAU / yradA );
        double theta = 0.5 * ( thetaN + thetaA );

        funCOBRAxyz f{};
        f.Set( theta,
               xmin, xrng, powX,
               xmax,
               xradN, yradN, zradNU, powNU, xlenA, yradA, zradAU, powAU,
               XAftWaterLineTanPt, YAftWaterLineTanPt,
               alineWL, xlineWL,
               XAftUKeelTanPt, ZAftUKeelTanPt,
               alineUKeel, xlineUKeel,
               XpowNU );

        // Always force point at end of nominal nose point.
        // Could possibly be improved by using tangent point(s).
        double usplit = pow( ( 0 - xmin ) / xrng, 1.0 / powX );

        VspCurve ctest;
        int depth = ctest.FunToBinaryCubic( f, 0.0, 1.0, usplit, tol );

        ctest.GetCurve().get_pmap( urib );

        // Repeat for lower side if not symmetric.
        if ( !ulsymm )
        {
            thetaN = atan( -zradNL / yradN );
            thetaA = atan( -zradAL / yradA );
            theta = 0.5 * ( thetaN + thetaA );

            f.Set( theta,
                   xmin, xrng, powX,
                   xmax,
                   xradN, yradN, zradNL, powNL, xlenA, yradA, zradAL, powAL,
                   XAftWaterLineTanPt, YAftWaterLineTanPt,
                   alineWL, xlineWL,
                   XAftLKeelTanPt, ZAftLKeelTanPt,
                   alineLKeel, xlineLKeel,
                   XpowNL );

            VspCurve ctest2;
            depth = std::max( depth, ctest2.FunToBinaryCubic( f, 0.0, 1.0, usplit, tol ) );

            vector < double > urib2;
            ctest2.GetCurve().get_pmap( urib2 );

            // Merge adapted parameter values and make unique
            urib.insert( urib.end(), urib2.begin(), urib2.end() );
            std::sort( urib.begin(), urib.end() );
            urib.erase( std::unique( urib.begin(), urib.end() ), urib.end() );
        }
    }


    int nrib = urib.size();

    vector< rib_data_type > rib_vec;
    rib_vec.resize( nrib );

    for ( int i = 0; i < nrib; i++ )
    {
        double u = urib[i];
        double xi = xmin + xrng * pow( u, powX );

        double dxdu = powX * xrng * pow( u, powX - 1.0 );

        double xrib = xi;

        // printf( "urib before %f u %f", urib[i], u );
        // urib[i] = u;

        double yrib, zribU, zribL, dyrib, dzribU, dzribL;
        // zribL based on Aftbody Keel
        SE_SingleSuperEllipse_FixedPt_TangentLine_Calc(
            xrng, xradN,
            zradNL, powX, xlenA, zradAL, XAftLKeelTanPt, ZAftLKeelTanPt,
            alineLKeel, xlineLKeel, xrib, zribL, dzribL );

        // zribU based on Aftbody Keel
        SE_SingleSuperEllipse_FixedPt_TangentLine_Calc(
            xrng, xradN,
            zradNU, powX, xlenA, zradAU, XAftUKeelTanPt, ZAftUKeelTanPt,
            alineUKeel, xlineUKeel, xrib, zribU, dzribU );

        // yrib based on Aftbody Waterline
        SE_SingleSuperEllipse_FixedPt_TangentLine_Calc(
            xrng, xradN,
            yradN, powX, xlenA, yradA, XAftWaterLineTanPt, YAftWaterLineTanPt,
            alineWL, xlineWL, xrib, yrib, dyrib );

        double dpduL = 0;
        double pow_ribL = powNL;
        if ( xrib > XpowNL )
        {
            double dpow = powAL - powNL;
            double dxR = M_PI * fabs( ( xrib - XpowNL ) / ( xmax - XpowNL ) );
            pow_ribL = powNL + 0.5 * dpow * ( 1.0 - cos( dxR ) );
            dpduL = ( 0.5 * dpow * sin( dxR ) * M_PI / ( xmax - XpowNL ) ) * dxdu;
        }

        double dpduU = 0;
        double pow_ribU = powNU;
        if ( xrib > XpowNU )
        {
            double dpow = powAU - powNU;
            double dxR = M_PI * fabs( ( xrib - XpowNU ) / ( xmax - XpowNU ) );
            pow_ribU = powNU + 0.5 * dpow * ( 1.0 - cos( dxR ) );
            dpduU = ( 0.5 * dpow * sin( dxR ) * M_PI / ( xmax - XpowNU ) ) * dxdu;
        }

        vector < vec3d > pt( npt );
        vector < vec3d > dptdtheta( npt );
        vector < vec3d > dpt( npt );
        vector < vec3d > d2ptdudtheta( npt );

        for ( int j = 0; j < npt; j++ )
        {
            double theta = 2.0 * M_PI * ( 1.0 - 0.25 * v[j] );
            double y1 = cos( theta );
            double z1 = sin( theta );

            double zrib = zribL;
            double pow_rib = pow_ribL;
            double dzribdu = dzribL;
            double dpdu = dpduL;
            if ( z1 >= 0 )
            {
                zrib = zribU;
                pow_rib = pow_ribU;
                dzribdu = dzribU;
                dpdu = dpduU;
            }

            double yi, zi;
            double dydu, dzdu;
            double dydtheta, dzdtheta;
            double d2ydudtheta, d2zdudtheta;
            SQ_SuperEllipse2D_YZCalc( yrib, zrib, pow_rib, dyrib, dzribdu, dpdu, y1, z1, yi, zi, dydu, dzdu, dydtheta, dzdtheta, d2ydudtheta, d2zdudtheta );

            pt[j] = vec3d( xrib + xradN + xo, yi, zi );
            dptdtheta[j] = vec3d( 0, -dydtheta * 2.0 * M_PI / 4.0, -dzdtheta * 2.0 * M_PI / 4.0 );
            dpt[j] = vec3d( dxdu, dydu, dzdu );
            d2ptdudtheta[j] = vec3d( 0, -d2ydudtheta * 2.0 * M_PI / 4.0, -d2zdudtheta * 2.0 * M_PI / 4.0 );
        }

        VspCurve c, dc;
        c.BuildCubic( pt, dptdtheta, v );
        dc.BuildCubic( dpt, d2ptdudtheta, v );

        rib_vec[i].set_f( c.GetCurve() );

        if ( i > 0 )
        {
            rib_vec[i].set_left_fp( dc.GetCurve() );
        }

        if ( i < nrib - 1 )
        {
            rib_vec[i].set_right_fp( dc.GetCurve() );
        }
    }

    m_MainSurfVec[0] = VspSurf();
    m_MainSurfVec[0].BuildCubic( rib_vec, urib );

    vec3d zero;
    m_MainSurfVec[0].CapUMax( vsp::POINT_END_CAP, 0, 0, 0, zero, false );
}

//==== Compute Rotation Center ====//
void CobraGeom::ComputeCenter()
{
    m_Center.set_x( ( m_XradN() + m_XlenA() ) * m_Origin() );
}

//==== Scale ====//
void CobraGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();

    m_XradN *= currentScale;
    m_YradN *= currentScale;
    m_ZradNL *= currentScale;
    m_ZradNU *= currentScale;
    m_XlenA *= currentScale;
    m_YradA *= currentScale;
    m_ZradAL *= currentScale;
    m_ZradAU *= currentScale;

    m_LastScale = m_Scale();
}

void CobraGeom::AddDefaultSources( double base_len )
{
}

void CobraGeom::OffsetXSecs( double off )
{
    m_XradN = std::max( 0.0, m_XradN() - off );
    m_YradN = std::max( 0.0, m_YradN() - off );
    m_ZradNL = std::max( 0.0, m_ZradNL() - off );
    m_ZradNU = std::max( 0.0, m_ZradNU() - off );

    m_XlenA = std::max( 0.0, m_XlenA() - off );
    m_YradA = std::max( 0.0, m_YradA() - off );
    m_ZradAL = std::max( 0.0, m_ZradAL() - off );
    m_ZradAU = std::max( 0.0, m_ZradAU() - off );

    m_Xoff = off;
}

// Helper function to find intersection point using bisection
// sign_flag: 1 for >=, -1 for <=
void FindIntersectionPoint (
    double ae, double be, double pe,
    double aline, double xp0, double yp0,
    double XM, double XP,
    int sign_flag,
    double &xp_result, double &yp_result, double &Del )
{
    // Find intercept of line, y = aline*(x-xline)
    // with ellipse  (x/ae)**pe + (y/be)**pe = 1
    //
    // Unchanged:
    // (xp0,yp0) -- One pt assumed given to be both on line and ellipse
    //
    // Returned:
    // (xp_result,yp_result) -- additional pt on both line and ellipse
    // Del  -- distance 2nd pt is off from being on both line and ellipse

    constexpr int nmaxiter = 100;

    double dx = 1;
    int icount = 0;
    while ( dx > 1.0e-16 && icount < nmaxiter )
    {
        double XTry = 0.5 * ( XM + XP );
        Del = ( yp0 + aline * ( XTry - xp0 ) ) - be * pow( 1.0 - pow( fabs( XTry / ae ), pe ), 1.0 / pe );

        if ( sign_flag * Del >= 0.0 )
        {
            XP = XTry;
        }
        else
        {
            XM = XTry;
        }

        dx = fabs( XM - XP );

        icount++;
    }

    xp_result = 0.5 * ( XM + XP );
    yp_result = yp0 + aline * ( xp_result - xp0 );
    Del = ( yp0 + aline * ( xp_result - xp0 ) ) - be * pow( 1.0 - pow( fabs( xp_result / ae ), pe ), 1.0 / pe );
}

void SE_SuperEllipse_Line_Pts (
    double ae, double be, double pe,
    double aline, double xline,
    double xp0, double yp0,
    double &xp1, double &yp1,
    double &xp2, double &yp2,
    double &Del, int &Ipts )
{
    // Find intercept of line, y = aline*(x-xline)
    // with ellipse  (x/ae)**pe + (y/be)**pe = 1
    //
    // Unchanged:
    // (xp0,yp0) -- One pt assumed given to be both on line and ellipse
    //
    // Returned:
    // (xp1,yp1) -- pt on both line and ellipse with minimum x
    // (xp2,yp2) -- pt on both line and ellipse with maximum x
    // Ipts = 1 if both pts are same, = 2 if two pts differ
    // Del  -- distance 2nd pt is off from being on both line and ellipse

    // Default values
    xp1 = xp0;
    yp1 = yp0;
    xp2 = xp0;
    yp2 = yp0;
    Del = 0.0;
    Ipts = 1;

    // Check for a 2nd pt to the right, change (xp2,yp2) only
    double XM = xp0;
    double XP = ae;

    FindIntersectionPoint( ae, be, pe, aline, xp0, yp0, XM, XP, 1, xp2, yp2, Del );

    double dx = fabs( xp2 - xp1 );
    if ( dx > 1.0e-16 )
    {
        Ipts = 2;
    }

    if ( Ipts == 2 )
    {
        return; // Already found both points
    }

    // Now check for a 2nd pt to the left, change (xp1,yp1) only
    XM = - ae;
    XP = xp0;

    FindIntersectionPoint( ae, be, pe, aline, xp0, yp0, XM, XP, -1, xp1, yp1, Del );

    dx = fabs( xp2 - xp1 );
    if ( dx > 1.0e-16 )
    {
        Ipts = 2;
    }
}

void SE_SingleSuperEllipse_FixedPt_TangentLine_Define (
    double ae, double be, double pe,
    double xfx, double yfx,
    double &xp0, double &yp0,
    double &aline, double &xline )
{
    // Given a single super-ellipse(ae,be,pe)
    // and a single fixed pt(xfx,yfx),
    // find tangent line (aline,xline) and tangent point on ellipse (xp0,yp0)
    //
    // Super-ellipse: (x/ae)**pe + (y/be)**pe = 1
    // Assumes ellipse is aligned on y=0 axis (so ye=0)
    // Ellipse assumed to the left of fixed pt (has smaller xe < xfx)

    if ( xfx <= 0 )
    {
        exit( 1 );
    }

    if ( be == yfx )
    {
        // Trivial case, slope = 0
        xp0 = 0;
        yp0 = be;
        aline = 0.0;
        xline = xp0;
    }
    else // for ( be > yfx ) or ( be < yfx )
    {

        if ( fabs( xfx ) < ae ) // Inside ellipse in X
        {
            // Find y point on ellipse at xfx
            double yelip = be * pow( 1.0 - pow( fabs( xfx / ae ), pe ), 1.0 / pe );
            if ( yfx < yelip )
            {
                // printf( "Aft point is inside nose ellipse.\n" );
                xp0 = 0;
                yp0 = be;

                // Establish starting line
                aline = ( yfx - yp0 ) / ( xfx - xp0 );
                xline = xp0;
                if ( aline != 0.0 )
                {
                    xline = xp0 - yp0 / aline;
                }
                return;
            }
        }

        // Establish starting "trial pt" for ellipse
        xp0 = 0;
        yp0 = be;

        // Establish starting line, y-yp0 = aline*(x-xp0)
        aline = ( yfx - yp0 ) / ( xfx - xp0 );
        xline = xp0;
        if ( aline != 0.0 )
        {
            xline = xp0 - yp0 / aline;
        }

        int ncnt = 100;

        int icnt = 0;
        double Del = 1;
        while ( fabs( Del ) > 1e-16 && icnt < ncnt )
        {
            double xp1 = xp0;
            double yp1 = yp0;
            double xp2 = xp0;
            double yp2 = yp0;

            int Ipts;
            // Find two pts line passes through superellipse, establish new trial pt
            SE_SuperEllipse_Line_Pts( ae, be, pe, aline, xline, xp0, yp0, xp1, yp1, xp2, yp2, Del, Ipts );

            if ( Ipts == 1 )
            {
                xp0 = xp1;
            }
            else if ( Ipts == 2 )
            {
                xp0 = 0.5 * ( xp1 + xp2 );
            }

            yp0 = be * pow( 1.0 - pow( fabs( xp0 / ae ), pe ), 1.0 / pe );

            // Establish starting line
            aline = ( yfx - yp0 ) / ( xfx - xp0 );
            xline = xp0;
            if ( aline != 0.0 )
            {
                xline = xp0 - yp0 / aline;
            }

            icnt++;
        }
    }
}

void SE_SingleSuperEllipse_FixedPt_TangentLine_Calc (
    double xrng, double ae, double be,
    double pe, double xfx,
    double yfx, double xp0,
    double yp0, double aline,
    double xline, double xpt, double &ypt, double &dydu )
{
    // Given:
    //   - super-ellipse parameters (ae, be, pe)
    //   - fixed point (xfx, yfx)
    //   - tangent point (xp0, yp0)
    //   - tangent line parameters (aline, xline)
    //   - x coordinate (xpt)
    // Calculate:
    //   - y coordinate (ypt) at xpt
    //
    // Super-ellipse: (x/ae)**pe + (y/be)**pe = 1

    ypt = 0.0;
    dydu = 0.0;

    if ( xpt < - ae )
    {
        // Point is to the left of the ellipse
        ypt = 0.0;
        dydu = 0.0;
    }
    else if ( std::abs( xpt - xp0 ) < 1e-12 )
    {
        // Point is almost exactly at tangent point
        ypt = yp0;
        xpt = xp0;

        const double u = pow( ( xpt + ae ) / xrng, 1.0 / pe );
        const double dxdu = pe * xrng * pow( u, pe - 1.0 );
        const double dydx = ( yp0 - yfx ) / ( xp0 - xfx );

        dydu = dydx * dxdu;
    }
    else if ( xpt < xp0 )
    {
        // Point is on the ellipse before tangent point
        ypt = be * pow( 1.0 - pow( fabs( xpt / ae ), pe ), 1.0 / pe );

        const double u = pow( ( xpt + ae ) / xrng, 1.0 / pe );

        if ( u < 1e-12 )
        {
            dydu = be * pow( pe * xrng / ae, 1.0 / pe );
        }
        else
        {
            double ax = fabs( xpt / ae );
            double f  = 1.0 - pow( ax, pe );

            double s = 1;
            if ( xpt < 0 )
            {
                s = -1;
            }

            const double dxdu = pe * xrng * pow( u, pe - 1.0 );
            double dydx = - ( be / ae ) *
                   pow( ax, pe - 1.0 ) *
                   s *
                   pow( f, ( 1.0 / pe ) - 1.0 );

            dydu = dydx * dxdu;
        }
    }
    else
    {
        const double u = pow( ( xpt + ae ) / xrng, 1.0 / pe );
        const double dxdu = pe * xrng * pow( u, pe - 1.0 );

        const double dydx = ( yp0 - yfx ) / ( xp0 - xfx );
        // Point is past tangent point, use tangent line
        ypt = yfx + ( xpt - xfx ) * dydx;

        dydu = dydx * dxdu;
    }

}

void SQ_SuperEllipse2D_YZCalc ( double yrib, double zrib, double pow_rib,
                                double dyribdu, double dzribdu, double dpowribdu,
                                double y1, double z1, double &ypt, double &zpt, double &dydu, double &dzdu )
{
    // Return ypt, zpt for a 2D superellipse
    // (y/yrib)**pow + (z/zrib)**pow = 1
    // such that ypt/zpt = y1/z1
    //
    // Maintains the direction (ratio) of (y1, z1) while placing the point
    // on the superellipse defined by yrib, zrib, and pow_rib

    double alpha, powi;
    double ysgn, zsgn, y1t, z1t;

    // Handle singular case at nose.  ypt,zpt=0 is straightforward.
    // I'm not sure how recursively calling this routien with dyribdu, dzribdu substituted for yrib, zrib
    // works to calculate dydu and dzdu, but it seems to give reasonable results.
    if ( fabs( yrib ) < 1.0e-12 && fabs( zrib ) < 1.0e-12 )
    {
        ypt = 0.0;
        zpt = 0.0;

        if ( fabs( dyribdu ) < 1.0e-12 && fabs( dzribdu ) < 1.0e-12 )
        {
            dydu = 0.0;
            dzdu = 0.0;
        }
        else
        {
            double dummy_dy, dummy_dz;
            SQ_SuperEllipse2D_YZCalc( fabs( dyribdu ), fabs( dzribdu ), pow_rib,
                                      0.0, 0.0, 0.0,
                                      y1, z1, dydu, dzdu, dummy_dy, dummy_dz );

            if ( dyribdu < 0.0 ) dydu = -dydu;
            if ( dzribdu < 0.0 ) dzdu = -dzdu;
        }
        return;
    }

    powi = 1.0 / pow_rib;
    ysgn = 1.0;
    zsgn = 1.0;

    // Handle signs - work in positive quadrant
    if ( y1 < 0.0 ) ysgn = -1.0;
    if ( z1 < 0.0 ) zsgn = -1.0;
    y1t = ysgn * y1;
    z1t = zsgn * z1;

    // Determine which axis dominates to avoid numerical issues
    if ( y1t * zrib >= z1t * yrib )
    {
        // Y dominates: solve for y first, then z
        alpha = z1t * yrib / ( y1t * zrib );
        ypt = yrib / pow( 1.0 + pow( alpha, pow_rib ), powi );
        zpt = zrib * pow( 1.0 - pow( ypt / yrib, pow_rib ), powi );

        if ( alpha < 1.0e-12 )
        {
            dydu = dyribdu;
        }
        else
        {
            const double term1 = ( dpowribdu / ( pow_rib * pow_rib ) ) * log( 1.0 + pow( alpha, pow_rib ) );
            const double alpha_p = pow( alpha, pow_rib );
            const double term2 = alpha_p / ( 1.0 + alpha_p );
            const double d_alpha_du_over_alpha = dyribdu / yrib - dzribdu / zrib;
            const double term3 = ( log( alpha ) * dpowribdu / pow_rib ) + d_alpha_du_over_alpha;

            dydu = ypt * ( dyribdu / yrib + term1 - term2 * term3 );
        }

        if ( y1t != 0.0 )
        {
            dzdu = dydu * ( z1t / y1t );
        }
        else
        {
            dzdu = 0.0;
        }
    }
    else
    {
        // Z dominates: solve for z first, then y
        alpha = y1t * zrib / ( z1t * yrib );
        zpt = zrib / pow( 1.0 + pow( alpha, pow_rib ), powi );
        ypt = yrib * pow( 1.0 - pow( zpt / zrib, pow_rib ), powi );

        if ( alpha < 1.0e-12 )
        {
            dzdu = dzribdu;
        }
        else
        {
            const double term1 = ( dpowribdu / ( pow_rib * pow_rib ) ) * log( 1.0 + pow( alpha, pow_rib ) );
            const double alpha_p = pow( alpha, pow_rib );
            const double term2 = alpha_p / ( 1.0 + alpha_p );
            const double d_alpha_du_over_alpha = dzribdu / zrib - dyribdu / yrib;
            const double term3 = ( log( alpha ) * dpowribdu / pow_rib ) + d_alpha_du_over_alpha;

            dzdu = zpt * ( dzribdu / zrib + term1 - term2 * term3 );
        }

        if ( z1t != 0.0 )
        {
            dydu = dzdu * ( y1t / z1t );
        }
        else
        {
            dydu = 0.0;
        }
    }

    // Restore original signs
    ypt = ysgn * ypt;
    zpt = zsgn * zpt;

    dydu = ysgn * dydu;
    dzdu = zsgn * dzdu;
}

void SQ_SuperEllipse2D_YZCalc ( double yrib, double zrib, double pow_rib,
                                double dyribdu, double dzribdu, double dpowribdu,
                                double y1, double z1,
                                double &ypt, double &zpt,
                                double &dydu, double &dzdu,
                                double &dydtheta, double &dzdtheta,
                                double &d2ydudtheta, double &d2zdudtheta )
{
    // This overload computes:
    //   ypt, zpt (point on superellipse along direction (y1,z1))
    //   dydu, dzdu (d/du keeping theta fixed)
    //   dydtheta, dzdtheta (d/dtheta keeping u fixed)
    //   d2ydudtheta, d2zdudtheta (mixed partials)
    //
    // Superellipse: (|y|/yrib)^p + (|z|/zrib)^p = 1, with (y,z) parallel to (y1,z1).

    constexpr double eps = 1.0e-12;

    // Defaults.
    ypt = 0.0; zpt = 0.0;
    dydu = 0.0; dzdu = 0.0;
    dydtheta = 0.0; dzdtheta = 0.0;
    d2ydudtheta = 0.0; d2zdudtheta = 0.0;

    // Handle singular radii.
    if ( fabs( yrib ) < eps && fabs( zrib ) < eps )
    {
        return;
    }

    const double a = fabs( yrib );
    const double b = fabs( zrib );

    if ( a < eps || b < eps )
    {
        return;
    }

    const double p = pow_rib;
    const double invp = 1.0 / p;

    // Direction and its theta-derivative (assuming y1=cos(theta), z1=sin(theta)).
    // If caller provides different parameterization, these are still the correct derivatives
    // for that trig parameterization.
    const double dy1dtheta = -z1;
    const double dz1dtheta =  y1;

    const double ay = fabs( y1 );
    const double az = fabs( z1 );

    // Scaled direction magnitudes.
    const double Ay0 = ay / a;
    const double Az0 = az / b;

    // S = (|y1|/a)^p + (|z1|/b)^p
    const double Sy = ( Ay0 > 0.0 ) ? pow( Ay0, p ) : 0.0;
    const double Sz = ( Az0 > 0.0 ) ? pow( Az0, p ) : 0.0;
    const double S = Sy + Sz;

    if ( S < eps )
    {
        return;
    }

    // Radial scale r(theta,u)
    const double r = pow( S, -invp );

    // Point on superellipse.
    ypt = r * y1;
    zpt = r * z1;

    // ---------- d/dtheta: compute dr/dtheta, then dy/dtheta, dz/dtheta ----------
    // dS/dtheta
    const double signy1 = ( y1 < 0.0 ) ? -1.0 : 1.0;
    const double signz1 = ( z1 < 0.0 ) ? -1.0 : 1.0;

    const double dAy0_dtheta = ( ay > 0.0 ) ? ( signy1 * dy1dtheta / a ) : 0.0; // d(|y1|/a)/dtheta
    const double dAz0_dtheta = ( az > 0.0 ) ? ( signz1 * dz1dtheta / b ) : 0.0; // d(|z1|/b)/dtheta

    const double dSy_dtheta = ( Ay0 > 0.0 ) ? ( p * pow( Ay0, p - 1.0 ) * dAy0_dtheta ) : 0.0;
    const double dSz_dtheta = ( Az0 > 0.0 ) ? ( p * pow( Az0, p - 1.0 ) * dAz0_dtheta ) : 0.0;

    const double dS_dtheta = dSy_dtheta + dSz_dtheta;

    // dr/dtheta = r * ( -(1/p) * (S'/S) )
    const double drdtheta = r * ( -invp * ( dS_dtheta / S ) );

    dydtheta = y1 * drdtheta + r * dy1dtheta;
    dzdtheta = z1 * drdtheta + r * dz1dtheta;

    // ---------- d/du: compute dr/du from a(u), b(u), p(u) ----------
    // Use log form:
    // ln r = -(1/p) ln S
    // d(ln r)/du = (p'/p^2) ln S - (1/p) * (S_u / S)
    const double a_u = ( fabs( yrib ) > eps ) ? ( dyribdu / yrib ) : 0.0; // (a'/a) using signed yrib; ok for magnitude ratios
    const double b_u = ( fabs( zrib ) > eps ) ? ( dzribdu / zrib ) : 0.0;

    // S_u = -p*Sy*(a'/a) - p*Sz*(b'/b) + p'*( Sy ln(Ay0) + Sz ln(Az0) )
    const double lnAy0 = ( Ay0 > eps ) ? log( Ay0 ) : 0.0;
    const double lnAz0 = ( Az0 > eps ) ? log( Az0 ) : 0.0;

    const double Su =
        ( -p * Sy * a_u ) +
        ( -p * Sz * b_u ) +
        ( dpowribdu * ( Sy * lnAy0 + Sz * lnAz0 ) );

    const double lnS = log( S );
    const double L = ( dpowribdu / ( p * p ) ) * lnS - invp * ( Su / S ); // L = d(ln r)/du

    const double drdu = r * L;

    dydu = y1 * drdu;
    dzdu = z1 * drdu;

    // ---------- mixed: d/ dtheta (dydu), d/ dtheta (dzdu) ----------
    // Need d(drdu)/dtheta = d(r*L)/dtheta = drdtheta*L + r*dL/dtheta
    //
    // dL/dtheta = (p'/p^2)*(S'/S) - (1/p)* d/dtheta(Su/S)
    // where d/dtheta(Su/S) = (Su' S - Su S') / S^2
    //
    // Su' depends on Sy,Sz and ln(Ay0), ln(Az0) terms.
    const double dlnAy0_dtheta = ( Ay0 > eps ) ? ( dAy0_dtheta / Ay0 ) : 0.0;
    const double dlnAz0_dtheta = ( Az0 > eps ) ? ( dAz0_dtheta / Az0 ) : 0.0;

    const double d_Sy_lnAy0_dtheta = dSy_dtheta * lnAy0 + Sy * dlnAy0_dtheta;
    const double d_Sz_lnAz0_dtheta = dSz_dtheta * lnAz0 + Sz * dlnAz0_dtheta;

    const double Su_theta =
        ( -p * a_u * dSy_dtheta ) +
        ( -p * b_u * dSz_dtheta ) +
        ( dpowribdu * ( d_Sy_lnAy0_dtheta + d_Sz_lnAz0_dtheta ) );

    const double d_over_S_dtheta = ( Su_theta * S - Su * dS_dtheta ) / ( S * S );

    const double dL_dtheta =
        ( dpowribdu / ( p * p ) ) * ( dS_dtheta / S ) -
        invp * d_over_S_dtheta;

    const double ddrdu_dtheta = drdtheta * L + r * dL_dtheta;

    d2ydudtheta = dy1dtheta * drdu + y1 * ddrdu_dtheta;
    d2zdudtheta = dz1dtheta * drdu + z1 * ddrdu_dtheta;
}

void SQ_SuperEllipse2D_YZCalc_dtheta ( double yrib, double zrib, double pow_rib,
                                       double theta,
                                       double &ypt, double &zpt,
                                       double &dydtheta, double &dzdtheta )
{
    double y1 = cos( theta );
    double z1 = sin( theta );

    // Return ypt, zpt for a 2D superellipse
    // (y/yrib)**pow + (z/zrib)**pow = 1
    // such that ypt/zpt = y1/z1
    //
    // Maintains the direction (ratio) of (y1, z1) while placing the point
    // on the superellipse defined by yrib, zrib, and pow_rib

    double alpha, powi;
    double ysgn, zsgn, y1t, z1t;

    // Handle singular case at nose.  ypt,zpt=0 is straightforward.
    // I'm not sure how recursively calling this routien with dyribdu, dzribdu substituted for yrib, zrib
    // works to calculate dydu and dzdu, but it seems to give reasonable results.
    if ( fabs( yrib ) < 1.0e-12 && fabs( zrib ) < 1.0e-12 )
    {
        ypt = 0.0;
        zpt = 0.0;
        dydtheta = 0.0;
        dzdtheta = 0.0;
        return;
    }

    powi = 1.0 / pow_rib;
    ysgn = 1.0;
    zsgn = 1.0;

    // Handle signs - work in positive quadrant
    if ( y1 < 0.0 ) ysgn = -1.0;
    if ( z1 < 0.0 ) zsgn = -1.0;
    y1t = ysgn * y1;
    z1t = zsgn * z1;

    // Determine which axis dominates to avoid numerical issues
    if ( y1t * zrib >= z1t * yrib )
    {
        // Y dominates: solve for y first, then z
        alpha = z1t * yrib / ( y1t * zrib );
        ypt = yrib / pow( 1.0 + pow( alpha, pow_rib ), powi );
        zpt = zrib * pow( 1.0 - pow( ypt / yrib, pow_rib ), powi );
    }
    else
    {
        // Z dominates: solve for z first, then y
        alpha = y1t * zrib / ( z1t * yrib );
        zpt = zrib / pow( 1.0 + pow( alpha, pow_rib ), powi );
        ypt = yrib * pow( 1.0 - pow( zpt / zrib, pow_rib ), powi );
    }

    // Restore original signs
    ypt = ysgn * ypt;
    zpt = zsgn * zpt;

    double a = fabs( yrib );
    double b = fabs( zrib );

    if ( a < 1.0e-12 || b < 1.0e-12 )
    {
        dydtheta = 0.0;
        dzdtheta = 0.0;
        return;
    }

    double r = sqrt( ypt * ypt + zpt * zpt );
    double n = pow_rib;
    double nm1 = n - 1.0;

    double Ay = ( 1.0 / a ) * pow( fabs( ypt / a ), nm1 );
    if ( ypt < 0.0 ) Ay = -Ay;

    double Bz = ( 1.0 / b ) * pow( fabs( zpt / b ), nm1 );
    if ( zpt < 0.0 ) Bz = -Bz;

    double denom = Ay * y1 + Bz * z1;

    if ( fabs( denom ) < 1.0e-12 )
    {
        dydtheta = 0.0;
        dzdtheta = 0.0;
    }
    else
    {
        dydtheta = -r * Bz / denom;
        dzdtheta = r * Ay / denom;
    }
}
