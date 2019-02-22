//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSec.cpp: implementation of the XSec class.
//
//////////////////////////////////////////////////////////////////////

#include "Airfoil.h"
#include "Geom.h"
#include "ParmMgr.h"
#include "StringUtil.h"
#include <float.h>
#include "VKTAirfoil.h"
#include "Vec2d.h"
#include "Cluster.h"
#include "StlHelper.h"

using std::string;
using namespace vsp;

//==== Default Constructor ====//
Airfoil::Airfoil( ) : XSecCurve( )
{
    m_Invert.Init( "Invert", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_Chord.Init( "Chord", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_ThickChord.Init( "ThickChord", m_GroupName, this, 0.1, 0.0, 1.0 );
    m_FitDegree.Init( "FitDegree", m_GroupName, this, 7, 1, MAX_CST_DEG );

    m_yscale = 1.0;
}

//==== Update ====//
void Airfoil::Update()
{
    m_OrigCurve = m_Curve;

    Matrix4d mat;
    mat.scale( m_Chord() );

    m_Curve.Transform( mat );

    // invert airfoil if needed
    if ( m_Invert() )
    {
        // reflect about xz-plane
        m_Curve.ReflectXZ();

        // reverse parameterization
        m_Curve.Reverse();
    }

    XSecCurve::Update();
}

//==== Get Width ====//
double Airfoil::GetWidth()
{
    return m_Chord();
}

//==== Get Height ====//
double Airfoil::GetHeight()
{
    return m_Chord() * m_ThickChord();
}

//==== Set Width and Height ====//
void Airfoil::SetWidthHeight( double w, double h )
{
    m_Chord = w;
    m_ThickChord = 0.0;

    if ( w > DBL_EPSILON )
    {
        m_ThickChord = h / w;
    }
}

//==== Get Curve ====//
VspCurve& Airfoil::GetOrigCurve()
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_OrigCurve;
}

void Airfoil::OffsetCurve( double offset_val )
{
    double c = m_Chord();
    double t = m_Chord() * m_ThickChord();

    double offset_c = c - 2.0*offset_val;
    m_Chord = offset_c;
    double offset_t = t - 2.0*offset_val;
    m_ThickChord = offset_t/m_Chord();
}

void Airfoil::ReadV2File( xmlNodePtr &root )
{
    m_Invert = XmlUtil::FindInt( root, "Inverted_Flag", m_Invert() );

    m_ThickChord = XmlUtil::FindDouble( root, "Thickness", m_ThickChord() );

//    slat_flag = XmlUtil::FindInt( node, "Slat_Flag", slat_flag );
//    slat_shear_flag = XmlUtil::FindInt( node, "Slat_Shear_Flag", slat_shear_flag );
//    slat_chord = XmlUtil::FindDouble( node, "Slat_Chord", slat_chord() );
//    slat_angle = XmlUtil::FindDouble( node, "Slat_Angle", slat_angle() );
//
//    flap_flag = XmlUtil::FindInt( node, "Flap_Flag", flap_flag );
//    flap_shear_flag = XmlUtil::FindInt( node, "Flap_Shear_Flag", flap_shear_flag );
//    flap_chord = XmlUtil::FindDouble( node, "Flap_Chord", flap_chord() );
//    flap_angle = XmlUtil::FindDouble( node, "Flap_Angle", flap_angle() );

}

double Airfoil::CalculateThick()
{
    double tloc;
    return m_Curve.CalculateThick( tloc );
}


//==========================================================================//
//==========================================================================//
//==========================================================================//


NACABase::NACABase() : Airfoil( )
{
}

void NACABase::BuildCurve( const naca_airfoil_type & af )
{
    int npts = 201; // Must be odd to hit LE point.

    double t0 = -1.0;
    double t = t0;
    double dt = 2.0 / ( npts - 1 );
    int ile = ( npts - 1 ) / 2;

    vector< vec3d > pnts( npts );
    vector< double > arclen( npts );

    vec2d p2d;
    p2d = af.f( t );
    pnts[0] = p2d;
    arclen[0] = 0.0;
    for ( int i = 1 ; i < npts ; i++ )
    {
        if ( i == ile )
        {
            t = 0.0; // Ensure LE point precision.
        }
        else if ( i == ( npts - 1 ) )
        {
            t = 1.0;  // Ensure end point precision.
        }
        else
        {
            t = t0 + dt * i; // All other points.
        }

        double tc = sgn( t ) * Cluster( std::abs( t ), 0.01, 0.1 );

        p2d = af.f( tc );
        pnts[i] = p2d;

        double ds = dist( pnts[i], pnts[i-1] );
        if ( ds < 1e-8 )
        {
            ds = 1.0/npts;
        }
        arclen[i] = arclen[i-1] + ds;
    }

    double lenlower = arclen[ile];
    double lenupper = arclen[npts-1] - lenlower;

    double lowerscale = 2.0/lenlower;
    int i;
    for ( i = 1; i < ile; i++ )
    {
        arclen[i] = arclen[i] * lowerscale;
    }
    arclen[ile] = 2.0;
    i++;

    double upperscale = 2.0/lenupper;
    for ( ; i < npts - 1; i++ )
    {
        arclen[i] = 2.0 + ( arclen[i] - lenlower) * upperscale;
    }
    arclen[npts-1] = 4.0;

    m_Curve.InterpolatePCHIP( pnts, arclen, false );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
FourSeries::FourSeries( ) : NACABase( )
{
    m_Type = XS_FOUR_SERIES;
    m_Camber.Init( "Camber", m_GroupName, this, 0.0, 0.0, 0.09 );
    m_CamberLoc.Init( "CamberLoc", m_GroupName, this, 0.2, 0.0, 1.0 );
    m_EqArcLen.Init( "EqArcLenFlag", m_GroupName, this, true, 0, 1 );
    m_SharpTE.Init( "SharpTEFlag", m_GroupName, this, true, 0, 1 );
}

//==== Update ====//
void FourSeries::Update()
{
    if ( !m_EqArcLen() ) // 'old' code with non-equal arc len parameterization.
    {
        piecewise_curve_type c, d;
        piecewise_four_digit_creator pwc;
        pwc.set_sharp_trailing_edge( m_SharpTE() );

        pwc.set_thickness( m_ThickChord() );
        pwc.set_camber( m_Camber(), m_CamberLoc() );

        pwc.create( c );

        d.set_t0( c.get_t0() );
        for ( int i = 0; i < c.number_segments(); i++ )
        {
            piecewise_curve_type::curve_type crv;
            piecewise_curve_type::data_type dt;
            c.get( crv, dt, i );
            d.push_back( crv, dt*2.0 );
        }

        m_Curve.SetCurve( d );
    }
    else // 'new' code that enforces equal arc len parameterization.
    {
        four_digit_airfoil_type af;
        af.set_sharp_trailing_edge( m_SharpTE() );

        af.set_thickness( m_ThickChord() );
        af.set_camber( m_Camber(), m_CamberLoc() );

        BuildCurve( af );
    }

    Airfoil::Update();
}

//===== Load Name And Number of 4 Series =====//
string FourSeries::GetAirfoilName()
{
    int icam     = int( m_Camber() * 100.0f + 0.5f );
    int icam_loc = int( m_CamberLoc() * 10.0f + 0.5f );
    int ithick   = int( m_ThickChord() * 100.0f + 0.5f );

    if ( icam == 0 )
    {
        icam_loc = 0;
    }

    char str[255];
    if ( ithick < 10 )
    {
        sprintf( str, "  NACA %d%d0%d", icam, icam_loc, ithick );
    }
    else
    {
        sprintf( str, "  NACA %d%d%d", icam, icam_loc, ithick );
    }

    return string( str );
}

void FourSeries::ReadV2File( xmlNodePtr &root )
{
    Airfoil::ReadV2File( root );

    m_Camber = XmlUtil::FindDouble( root, "Camber", m_Camber() );
    m_CamberLoc = XmlUtil::FindDouble( root, "Camber_Loc", m_CamberLoc() );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
FourDigMod::FourDigMod( ) : NACABase( )
{
    m_Type = XS_FOUR_DIGIT_MOD;
    m_Camber.Init( "Camber", m_GroupName, this, 0.0, 0.0, 0.09 );
    m_CamberLoc.Init( "CamberLoc", m_GroupName, this, 0.2, 0.1, 0.9 );
    m_ThickLoc.Init( "ThickLoc", m_GroupName, this, 0.3, 0.2, 0.6 );
    m_LERadIndx.Init( "LERadIndx", m_GroupName, this, 6.0, 0.0, 9.0 );
    m_SharpTE.Init( "SharpTEFlag", m_GroupName, this, true, 0, 1 );
}

//==== Update ====//
void FourDigMod::Update()
{
    four_digit_mod_airfoil_type af( m_Camber(), m_CamberLoc(), m_ThickChord(), m_LERadIndx(),  m_ThickLoc(), m_SharpTE() );

    BuildCurve( af );
    Airfoil::Update();
}

//===== Load Name And Number of 4 Series =====//
string FourDigMod::GetAirfoilName()
{
    int icam     = int( m_Camber() * 100.0f + 0.5f );
    int icam_loc = int( m_CamberLoc() * 10.0f + 0.5f );
    int ithick   = int( m_ThickChord() * 100.0f + 0.5f );
    int ilerad   = int( m_LERadIndx() + 0.5 );
    int ithick_loc = int( m_ThickLoc() * 10.0f + 0.5f );

    if ( icam == 0 )
    {
        icam_loc = 0;
    }

    char str[255];
    if ( ithick < 10 )
    {
        sprintf( str, "  NACA %d%d0%d-%d%d", icam, icam_loc, ithick, ilerad, ithick_loc );
    }
    else
    {
        sprintf( str, "  NACA %d%d%d-%d%d", icam, icam_loc, ithick, ilerad, ithick_loc );
    }

    return string( str );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
FiveDig::FiveDig( ) : NACABase( )
{
    m_Type = XS_FIVE_DIGIT;
    m_IdealCl.Init( "IdealCl", m_GroupName, this, 0.3, 0.0, 1.0 );
    m_CamberLoc.Init( "CamberLoc", m_GroupName, this, 0.15, 0.0, 0.423 );
    m_SharpTE.Init( "SharpTEFlag", m_GroupName, this, true, 0, 1 );
}

//==== Update ====//
void FiveDig::Update()
{
    five_digit_airfoil_type m_AF( m_ThickChord(), m_IdealCl(), m_CamberLoc(), m_SharpTE() );

    BuildCurve( m_AF );
    Airfoil::Update();
}

//===== Load Name And Number of 4 Series =====//
string FiveDig::GetAirfoilName()
{
    int icl     = int( m_IdealCl() * ( 2.0f / 3.0f ) * 10.0f + 0.5f );
    int icam_loc = int( m_CamberLoc() * 2.0f * 100.0f + 0.5f );
    int ithick   = int( m_ThickChord() * 100.0f + 0.5f );

    if ( icl == 0 )
    {
        icam_loc = 0;
    }

    char str[255];
    if ( ithick < 10 && icam_loc < 10 )
    {
        sprintf( str, "  NACA %d0%d0%d", icl, icam_loc, ithick );
    }
    if ( ithick < 10 )
    {
        sprintf( str, "  NACA %d%d0%d", icl, icam_loc, ithick );
    }
    if ( icam_loc < 10 )
    {
        sprintf( str, "  NACA %d0%d%d", icl, icam_loc, ithick );
    }
    else
    {
        sprintf( str, "  NACA %d%d%d", icl, icam_loc, ithick );
    }

    return string( str );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
FiveDigMod::FiveDigMod( ) : NACABase( )
{
    m_Type = XS_FIVE_DIGIT_MOD;
    m_IdealCl.Init( "IdealCl", m_GroupName, this, 0.3, 0.0, 1.0 );
    m_CamberLoc.Init( "CamberLoc", m_GroupName, this, 0.15, 0.0, 0.423 );
    m_ThickLoc.Init( "ThickLoc", m_GroupName, this, 0.3, 0.2, 0.6 );
    m_LERadIndx.Init( "LERadIndx", m_GroupName, this, 6.0, 0.0, 9.0 );
    m_SharpTE.Init( "SharpTEFlag", m_GroupName, this, true, 0, 1 );
}

//==== Update ====//
void FiveDigMod::Update()
{
    five_digit_mod_airfoil_type m_AF( m_ThickChord(), m_IdealCl(), m_CamberLoc(), m_LERadIndx(), m_ThickLoc(), m_SharpTE() );

    BuildCurve( m_AF );
    Airfoil::Update();
}

//===== Load Name And Number of 4 Series =====//
string FiveDigMod::GetAirfoilName()
{
    int icl     = int( m_IdealCl() * ( 2.0f / 3.0f ) * 10.0f + 0.5f );
    int icam_loc = int( m_CamberLoc() * 2.0f * 100.0f + 0.5f );
    int ithick   = int( m_ThickChord() * 100.0f + 0.5f );
    int ilerad   = int( m_LERadIndx() + 0.5 );
    int ithick_loc = int( m_ThickLoc() * 10.0f + 0.5f );

    if ( icl == 0 )
    {
        icam_loc = 0;
    }

    char str[255];
    if ( ithick < 10 && icam_loc < 10 )
    {
        sprintf( str, "  NACA %d0%d0%d-%d%d", icl, icam_loc, ithick, ilerad, ithick_loc );
    }
    if ( ithick < 10 )
    {
        sprintf( str, "  NACA %d%d0%d-%d%d", icl, icam_loc, ithick, ilerad, ithick_loc );
    }
    if ( icam_loc < 10 )
    {
        sprintf( str, "  NACA %d0%d%d-%d%d", icl, icam_loc, ithick, ilerad, ithick_loc );
    }
    else
    {
        sprintf( str, "  NACA %d%d%d-%d%d", icl, icam_loc, ithick, ilerad, ithick_loc );
    }

    return string( str );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
OneSixSeries::OneSixSeries( ) : NACABase( )
{
    m_Type = XS_ONE_SIX_SERIES;
    m_IdealCl.Init( "IdealCl", m_GroupName, this, 0.2, 0.0, 1.0 );
    m_SharpTE.Init( "SharpTEFlag", m_GroupName, this, true, 0, 1 );
}

//==== Update ====//
void OneSixSeries::Update()
{
    one_six_series_airfoil_type m_AF( m_ThickChord(), m_IdealCl(), m_SharpTE() );

    BuildCurve( m_AF );
    Airfoil::Update();
}

//===== Load Name And Number of 4 Series =====//
string OneSixSeries::GetAirfoilName()
{
    int icl     = int( m_IdealCl() * 10.0f + 0.5f );
    int ithick   = int( m_ThickChord() * 100.0f + 0.5f );

    char str[255];
    if ( ithick < 10 )
    {
        sprintf( str, "  NACA 16-%d0%d", icl, ithick );
    }
    else
    {
        sprintf( str, "  NACA 16-%d%d", icl, ithick );
    }

    return string( str );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
SixSeries::SixSeries( ) : Airfoil( )
{
    m_Type = XS_SIX_SERIES;

    m_Series.Init( "Series", m_GroupName, this, SERIES_63, SERIES_63, NUM_SERIES );
    m_IdealCl.Init( "IdealCl", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_A.Init( "A", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_ThickChord.SetUpperLimit( 0.5 );
}

//==== Update ====//
void SixSeries::Update()
{
    //==== Run Six Series Fortran Generation Code ====//
    float cli = ( float )m_IdealCl();
    float ta = ( float )m_A();
    float toc = ( float )m_ThickChord();
    int sixser = 63;
    if ( m_Series() <= SERIES_67 )
    {
        sixser = m_Series() + 63;
    }
    else if ( m_Series() < NUM_SERIES )
    {
        sixser = -( m_Series() + 58 );
    }

    //==== Generate Airfoil ====//
    sixseries_( &sixser, &toc, &cli, &ta );

    int num_pnts_upper = sixpnts_.nmu;
    int num_pnts_lower = sixpnts_.nml;

    // Force trailing edge point closed and x = 1.0.
    double yte = ( sixpnts_.yyl[ num_pnts_lower - 1 ] + sixpnts_.yyu[ num_pnts_upper - 1 ] ) * 0.5;
    sixpnts_.yyl[ num_pnts_lower - 1 ] = yte;
    sixpnts_.yyu[ num_pnts_upper - 1 ] = yte;

    sixpnts_.xxl[ num_pnts_lower - 1 ] = 1.0;
    sixpnts_.xxu[ num_pnts_upper - 1 ] = 1.0;

    //==== Load Points ====//
    vector< vec3d > pnts;
    pnts.resize( num_pnts_lower + num_pnts_upper - 1 );
    int k = 0;
    for ( int i = num_pnts_lower - 1 ; i >= 0 ; i-- )
    {
        pnts[k] = vec3d( sixpnts_.xxl[i], sixpnts_.yyl[i], 0.0 );
        k++;
    }
    for ( int i = 1 ; i < num_pnts_upper; i++ )
    {
        pnts[k] = vec3d( sixpnts_.xxu[i], sixpnts_.yyu[i], 0.0 );
        k++;
    }

    vector< double > arclen;
    int npts = pnts.size();
    arclen.resize( npts );
    arclen[0] = 0.0;

    for ( int i = 1 ; i < npts ; i++ )
    {
        double ds = dist( pnts[i], pnts[i-1] );
        if ( ds < 1e-8 )
        {
            ds = 1.0/npts;
        }
        arclen[i] = arclen[i-1] + ds;
    }

    int ile = num_pnts_lower - 1;
    double lenlower = arclen[ile];
    double lenupper = arclen[npts-1] - lenlower;

    double lowerscale = 2.0/lenlower;
    int i;
    for ( i = 0; i < ile; i++ )
    {
        arclen[i] = arclen[i] * lowerscale;
    }

    double upperscale = 2.0/lenupper;
    for ( ; i < npts; i++ )
    {
        arclen[i] = 2.0 + ( arclen[i] - lenlower) * upperscale;
    }

    m_Curve.InterpolatePCHIP( pnts, arclen, false );

    Airfoil::Update();
}

//===== Load Name And Number of 4 Series =====//
string SixSeries::GetAirfoilName()
{
    int ithick   = int( m_ThickChord() * 100.0 + 0.5f );
    int icl = int( m_IdealCl() * 10.0 + 0.5f );
    int ia = int( m_A() * 10.0 + 0.5f );

    int sixser = 63;
    if ( m_Series() < SERIES_67 )
    {
        sixser = m_Series() + 63;
    }
    else if ( m_Series() < NUM_SERIES )
    {
        sixser = -( m_Series() + 58 );
    }

    char str[255];
    if ( sixser > 0 )
    {
        if ( ithick < 10 )
        {
            sprintf( str, "  NACA %d%d0%d  a=0.%d", sixser, icl, ithick, ia );
        }
        else
        {
            sprintf( str, "  NACA %d%d%d  a=0.%d",  sixser, icl, ithick, ia );
        }
    }
    else
    {
        if ( ithick < 10 )
        {
            sprintf( str, "  NACA %dA%d0%d  a=0.%d", -sixser, icl, ithick, ia );
        }
        else
        {
            sprintf( str, "  NACA %dA%d%d  a=0.%d",  -sixser, icl, ithick, ia );
        }
    }
    return string( str );
}

void SixSeries::ReadV2File( xmlNodePtr &root )
{
    Airfoil::ReadV2File( root );

    int sixser = XmlUtil::FindInt( root, "Six_Series", 63 );

    if ( sixser > 0 )
    {
        m_Series = sixser - 63;
    }
    else
    {
        m_Series = 58 - sixser;
    }

    m_IdealCl = XmlUtil::FindDouble( root, "Ideal_Cl", m_IdealCl() );
    m_A = XmlUtil::FindDouble( root, "A", m_A() );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
Biconvex::Biconvex( ) : Airfoil( )
{
    m_Type = XS_BICONVEX;
}

//==== Update ====//
void Biconvex::Update()
{
    double x, xu, yu;

    int nbase = 21;

    //==== Initialize Array For Points ====//
    vector< vec3d > upnts( nbase );
    vector< vec3d > lpnts( nbase );

    //==== Generate Airfoil ====//
    for ( int i = 0 ; i < nbase ; i++ )
    {
        //==== More Points At Leading Edge
        x = ( double )i / ( double )( nbase - 1 );

        //==== Compute Upper Surface Points ====//
        xu = x;
        yu = 2.0 * m_ThickChord() * x * ( 1.0 - x );
        upnts[i] = vec3d( xu, yu, 0.0 );

        //==== Compute Lower Surface Points ====//
        lpnts[nbase - 1 - i] = vec3d( xu, -yu, 0.0 );
    }

    vector< double > uarclen( nbase );
    vector< double > larclen( nbase );
    uarclen[0] = 0.0;
    larclen[0] = 0.0;
    for ( int i = 1 ; i < nbase ; i++ )
    {
        uarclen[ i ] = uarclen[ i - 1 ] + dist( upnts[ i ], upnts[ i - 1 ] );
        larclen[ i ] = larclen[ i - 1 ] + dist( lpnts[ i ], lpnts[ i - 1 ] );
    }

    double lenscale = 2.0 / uarclen.back();

    for ( int i = 0 ; i < nbase ; i++ )
    {
        uarclen[ i ] = uarclen[ i ] * lenscale;
        larclen[ i ] = larclen[ i ] * lenscale;
    }

    VspCurve upcrv;
    upcrv.InterpolatePCHIP( upnts, uarclen, false );

    m_Curve.InterpolatePCHIP( lpnts, larclen, false );

    m_Curve.Append( upcrv );

    Airfoil::Update();
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
Wedge::Wedge( ) : Airfoil( )
{
    m_Type = XS_WEDGE;
    m_ThickLoc.Init( "ThickLoc", m_GroupName, this, 0.5, 0.0, 1.0 );

}

//==== Update ====//
void Wedge::Update()
{
    vector<vec3d> pt( 4 );
    vector<double> u( 5 );
    double x_apex( m_ThickLoc() ), y_apex( m_ThickChord() / 2 );

    pt[0].set_xyz( 1, 0, 0 );
    pt[1].set_xyz( x_apex, -y_apex, 0 );
    pt[2].set_xyz( 0, 0, 0 );
    pt[3].set_xyz( x_apex, y_apex, 0 );

    u[0] = 0;
    u[1] = 1;
    u[2] = 2;
    u[3] = 3;
    u[4] = 4;

    // build the wedge
    m_Curve.InterpolateLinear( pt, u, true );

    Airfoil::Update();
}

void Wedge::ReadV2File( xmlNodePtr &root )
{
    Airfoil::ReadV2File( root );

    m_ThickLoc = XmlUtil::FindDouble( root, "Thickness_Loc", m_ThickLoc() );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
FileAirfoil::FileAirfoil( ) : Airfoil( )
{
    m_Type = XS_FILE_AIRFOIL;

    // Initialize to closed circle.
    int n = 21;
    for ( int i = 0; i < n; i++ )
    {
        double theta = PI-PI*i/(n-1);
        m_UpperPnts.push_back( vec3d( 0.5 + 0.5*cos(theta), 0.5*sin(theta), 0.0 ) );
        theta = PI+PI*i/(n-1);
        m_LowerPnts.push_back( vec3d( 0.5 + 0.5*cos(theta), 0.5*sin(theta), 0.0 ) );
    }
}

//==== Update ====//
void FileAirfoil::Update()
{
    //==== Load Points ====//
    vector< vec3d > pnts;

    for ( int i = ( int )m_LowerPnts.size() - 1 ; i >= 0; i-- )
    {
        pnts.push_back( m_LowerPnts[i] );
    }
    for ( int i = 1 ; i < ( int )m_UpperPnts.size(); i++ )
    {
        pnts.push_back( m_UpperPnts[i] );
    }

    int num_pnts_lower = m_LowerPnts.size();

    vector< double > arclen;
    int npts = pnts.size();
    arclen.resize( npts );
    arclen[0] = 0.0;

    for ( int i = 1 ; i < npts ; i++ )
    {
        double ds = dist( pnts[i], pnts[i-1] );
        if ( ds < 1e-8 )
        {
            ds = 1.0/npts;
        }
        arclen[i] = arclen[i-1] + ds;
    }

    int ile = num_pnts_lower - 1;
    double lenlower = arclen[ile];
    double lenupper = arclen[npts - 1] - lenlower;

    double lowerscale = 2.0/lenlower;
    int i;
    for ( i = 0; i < ile; i++ )
    {
        arclen[i] = arclen[i] * lowerscale;
    }

    double upperscale = 2.0/lenupper;
    for ( ; i < npts; i++ )
    {
        arclen[i] = 2.0 + ( arclen[i] - lenlower) * upperscale;
    }

    m_Curve.InterpolatePCHIP( pnts, arclen, false );

    Airfoil::Update();
}

//==== Encode XML ====//
xmlNodePtr FileAirfoil::EncodeXml( xmlNodePtr & node )
{
    Airfoil::EncodeXml( node );

    xmlNodePtr child_node = xmlNewChild( node, NULL, BAD_CAST "FileAirfoil", NULL );
    if ( child_node )
    {
        XmlUtil::AddStringNode( child_node, "AirfoilName", m_AirfoilName );

        //==== Encode Pnts ====//
        XmlUtil::AddVectorVec3dNode( child_node, "UpperPnts", m_UpperPnts );
        XmlUtil::AddVectorVec3dNode( child_node, "LowerPnts", m_LowerPnts );
    }
    return child_node;
}

//==== Decode XML ====//
xmlNodePtr FileAirfoil::DecodeXml( xmlNodePtr & node )
{
    Airfoil::DecodeXml( node );

    xmlNodePtr child_node = XmlUtil::GetNode( node, "FileAirfoil", 0 );
    if ( child_node )
    {
        m_AirfoilName = XmlUtil::FindString( child_node, "AirfoilName", m_AirfoilName );
        m_UpperPnts = XmlUtil::ExtractVectorVec3dNode( child_node, "UpperPnts" );
        m_LowerPnts = XmlUtil::ExtractVectorVec3dNode( child_node, "LowerPnts" );
    }
    return child_node;
}

////==== Set GroupName ====//
//void FileAirfoil::SetGroupName( string group_name )
//{
//
//}

void FileAirfoil::OffsetCurve( double offset_val )
{
    double t = CalculateThick();
    double c = m_Chord();

    double offset_c = c - 2.0*offset_val;
    m_Chord = offset_c;

    double offset_t = t - 2.0 * offset_val;

    if ( offset_t < 0 )
    {
        offset_t = 0;
    }

    m_yscale = ( offset_t / offset_c ) / ( t / c );
}

//==== Read Airfoil File ====//
bool FileAirfoil::ReadFile( string file_name )
{
    //==== Open File ====//
    FILE* file_id =  fopen( file_name.c_str(), "r" );
    if ( file_id == ( FILE * )NULL )
    {
        return false;
    }

    char buff[256];
    string line;

    fgets( buff, 255, file_id );
    line.assign( buff );

    size_t found = line.find( "AIRFOIL FILE" );

    bool valid_file = false;
    if ( found == string::npos )
    {
        valid_file = ReadSeligAirfoil( file_id );           // Check If Selig Format

        if ( !valid_file )
        {
            valid_file = ReadLednicerAirfoil( file_id );    // Check If Lednicer Format
        }
    }
    else
    {
        fgets( buff, 255, file_id );                        // Name
        valid_file = ReadVspAirfoil( file_id );             // VSP Format
    }

    if ( !valid_file )
    {
        fclose( file_id );
        return false;
    }

    //==== Extract Name ====//
    m_AirfoilName.assign( buff );
    StringUtil::change_from_to( m_AirfoilName, '\n', ' ' );
    StringUtil::change_from_to( m_AirfoilName, '\r', ' ' );
    StringUtil::remove_trailing( m_AirfoilName, ' ' );

    fclose( file_id );

    return valid_file;

}


//==== Read Selig Airfoil File ====//
bool FileAirfoil::ReadSeligAirfoil( FILE* file_id )
{
    int i;
    char buff[256];
    float x, y;

    vector< float > xvec;
    vector< float > yvec;

    int more_data_flag = 1;
    while ( more_data_flag )
    {
        more_data_flag = 0;
        if ( fgets( buff, 255, file_id ) )
        {
            more_data_flag = 1;
        }

        if ( more_data_flag )
        {
            x = y = 100000.0;
            sscanf( buff, "%f %f", &x, &y );

            // This check is actually a test to determine the input file format.  Lednicer files will
            // read in the number of points on the top/bottom into these numbers.  Those values will
            // fail this test -- while reasonable airfoil points (even with some leading edge leakage,
            // or high lift devices) will pass.
            if ( x <= 3.0 && y <= 3.0 )
            {
                xvec.push_back( x );
                yvec.push_back( y );
            }
            else
            {
                more_data_flag = 0;
            }
        }
    }

    int totalPnts = xvec.size();

    if ( totalPnts < 5 )
    {
        return false;
    }

    //==== Find Leading Edge Index ====//
    int leInd = 0;
    float smallestX = 1.0e06;
    for ( i = 0 ; i < totalPnts ; i++ )
    {
        if ( xvec[i] < smallestX )
        {
            smallestX = xvec[i];
            leInd = i;
        }
    }

    //==== Not Enough Pnts ====//
    int num_pnts_lower = totalPnts - leInd;
    int num_pnts_upper = leInd + 1;

    if ( num_pnts_lower < 3 || num_pnts_upper < 3 )
    {
        return false;
    }

    m_UpperPnts.clear();
    m_LowerPnts.clear();

    //==== Load Em Up ====//
    for ( i = leInd ; i >= 0 ; i-- )
    {
        m_UpperPnts.push_back( vec3d( xvec[i], yvec[i], 0.0 ) );
    }

    for ( i = leInd ; i < totalPnts ; i++ )
    {
        m_LowerPnts.push_back( vec3d( xvec[i], yvec[i], 0.0 ) );
    }

    return true;
}

//==== Read Lednicer Airfoil File ====//
bool FileAirfoil::ReadLednicerAirfoil( FILE* file_id )
{
    char buff[256];
    float x, y;

    rewind( file_id );

    fgets( buff, 255, file_id );
    fgets( buff, 255, file_id );
    sscanf( buff, "%f %f", &x, &y );
    int num_pnts_upper = ( int )( x + 0.5 );
    int num_pnts_lower = ( int )( y + 0.5 );

    if ( num_pnts_upper < 3 || num_pnts_lower < 3 )
    {
        return false;
    }

    m_UpperPnts.clear();
    m_LowerPnts.clear();

    fgets( buff, 255, file_id );
    for ( int i = 0 ; i < num_pnts_upper ; i++ )
    {
        fgets( buff, 255, file_id );
        sscanf( buff, "%f %f", &x, &y );
        m_UpperPnts.push_back( vec3d( x, y, 0.0 ) );
    }
    fgets( buff, 255, file_id );
    for ( int i = 0 ; i < num_pnts_lower ; i++ )
    {
        fgets( buff, 255, file_id );
        sscanf( buff, "%f %f", &x, &y );
        m_LowerPnts.push_back( vec3d( x, y, 0.0 ) );
    }

    return true;
}

//==== Read VSP Airfoil File ====//
bool FileAirfoil::ReadVspAirfoil( FILE* file_id )
{
    int i;
    char buff[256];

    int sym_flag;
    int num_pnts_upper, num_pnts_lower;
    float x, y;

    fgets( buff, 255, file_id );
    sscanf( buff, "%d", &sym_flag );

    fgets( buff, 255, file_id );
    sscanf( buff, "%d", &num_pnts_upper );

    if ( sym_flag )
    {
        num_pnts_lower = num_pnts_upper;
    }
    else
    {
        fgets( buff, 255, file_id );
        sscanf( buff, "%d", &num_pnts_lower );
    }

    m_UpperPnts.clear();
    m_LowerPnts.clear();
    for ( i = 0 ; i < num_pnts_upper ; i++ )
    {
        fgets( buff, 255, file_id );
        sscanf( buff, "%f %f", &x, &y );

        m_UpperPnts.push_back( vec3d( x, y, 0.0 ) );

        if ( sym_flag )
        {
            m_LowerPnts.push_back( vec3d( x, -y, 0.0 ) );
        }
    }
    fgets( buff, 255, file_id );

    if ( !sym_flag )
    {
        for ( i = 0 ; i < num_pnts_lower ; i++ )
        {
            fgets( buff, 255, file_id );
            sscanf( buff, "%f %f", &x, &y );
            m_LowerPnts.push_back( vec3d( x, y, 0.0 ) );
        }
    }

    return true;
}

void FileAirfoil::ReadV2File( xmlNodePtr &root )
{
    Airfoil::ReadV2File( root );

    m_AirfoilName = XmlUtil::FindString( root, "Name", m_AirfoilName );

//    orig_af_thickness = (float)XmlUtil::FindDouble( root, "Original_AF_Thickness", orig_af_thickness );
//    radius_le_correction_factor = (float)XmlUtil::FindDouble( root, "Radius_LE_Correction_Factor", radius_le_correction_factor );
//    radius_te_correction_factor = (float)XmlUtil::FindDouble( root, "Radius_TE_Correction_Factor", radius_te_correction_factor );

    xmlNodePtr upp_node = XmlUtil::GetNode( root, "Upper_Pnts", 0 );
    if ( upp_node )
    {
        int num_arr = XmlUtil::GetNumArray( upp_node, ',' );
        double* arr = (double*)malloc( num_arr*sizeof(double) );
        XmlUtil::ExtractDoubleArray( upp_node, ',', arr, num_arr );

        int num_upper = num_arr/2;

        m_UpperPnts.clear();
        for ( int i = 0 ; i < num_arr ; i+=2)
        {
            m_UpperPnts.push_back( vec3d( arr[i], arr[i+1], 0.0 ) );
        }
        free(arr);
    }

    xmlNodePtr low_node = XmlUtil::GetNode( root, "Lower_Pnts", 0 );
    if ( low_node )
    {
        int num_arr = XmlUtil::GetNumArray( low_node, ',' );
        double* arr = (double*)malloc( num_arr*sizeof(double) );
        XmlUtil::ExtractDoubleArray( low_node, ',', arr, num_arr );

        int num_lower = num_arr/2;

        m_LowerPnts.clear();
        for ( int i = 0 ; i < num_arr ; i+=2)
        {
            m_LowerPnts.push_back( vec3d( arr[i], arr[i+1], 0.0 ) );
        }
        free(arr);
    }
}

//==========================================================================//
//==========================================================================//
//==========================================================================//


//==== Constructor ====//
CSTAirfoil::CSTAirfoil( ) : Airfoil( )
{
    m_Type = XS_CST_AIRFOIL;

    int initorder = 10;
    for ( int i = 0; i < initorder; i++ )
    {
        AddUpParm();
        AddLowParm();
    }

    double coeff[] = { 0.170987592880629, 0.157286894410384, 0.162311658384540,
        0.143623187913493, 0.149218456400780, 0.137218405082418, 0.140720628655908,
        0.141104769355436
    };

    m_UpDeg.Init( "UpDegree", m_GroupName, this, 7, 2, MAX_CST_DEG );
    m_LowDeg.Init( "LowDegree", m_GroupName, this, 7, 2, MAX_CST_DEG );

    m_ContLERad.Init( "ContLERad", m_GroupName, this, 1, 0, 1 );

    m_EqArcLen.Init( "EqArcLenFlag", m_GroupName, this, true, 0, 1 );

    for ( int i = 0; i <= m_UpDeg(); i++ )
    {
        Parm* p = m_UpCoeffParmVec[i];
        if ( p )
        {
            p->Set( coeff[i] );
        }

        p = m_LowCoeffParmVec[i];
        if ( p )
        {
            p->Set( -coeff[i] );
        }
    }
}

//==== Update ====//
void CSTAirfoil::Update()
{
    cst_airfoil_type cst;

    MakeCSTAirfoil( cst );

    piecewise_cst_creator pcst;

    // create curve
    pcst.set_conditions( cst );
    pcst.set_t0( 0 );
    pcst.set_segment_dt( 2, 0 );
    pcst.set_segment_dt( 2, 1 );

    piecewise_curve_type pc;
    pcst.create( pc );

    if ( !m_EqArcLen() )
    {
        m_Curve.SetCurve( pc );
    }
    else
    {
        int npts = 101; // Must be odd to hit LE point.

        double t = 0.0;
        double dt = 4.0 / ( npts - 1 );
        int ile = ( npts - 1 ) / 2;

        vector< vec3d > pnts( npts );
        vector< double > arclen( npts );

        pnts[0] = pc.f( t );
        arclen[0] = 0.0;
        for ( int i = 1 ; i < npts ; i++ )
        {
            if ( i == ile )
            {
                t = 2.0; // Ensure LE point precision.
            }
            else if ( i == ( npts - 1 ) )
            {
                t = 4.0;  // Ensure end point precision.
            }
            else
            {
                t = dt * i; // All other points.
            }

            pnts[i] = pc.f( t );

            double ds = dist( pnts[i], pnts[i-1] );
            if ( ds < 1e-8 )
            {
                ds = 1.0/npts;
            }
            arclen[i] = arclen[i-1] + ds;
        }

        double lenlower = arclen[ile];
        double lenupper = arclen[npts-1] - lenlower;

        double lowerscale = 2.0/lenlower;
        int i;
        for ( i = 1; i < ile; i++ )
        {
            arclen[i] = arclen[i] * lowerscale;
        }
        arclen[ile] = 2.0;
        i++;

        double upperscale = 2.0/lenupper;
        for ( ; i < npts - 1; i++ )
        {
            arclen[i] = 2.0 + ( arclen[i] - lenlower) * upperscale;
        }
        arclen[npts-1] = 4.0;

        m_Curve.InterpolatePCHIP( pnts, arclen, false );
    }

    Airfoil::Update();
}

void CSTAirfoil::PromoteUpper()
{
    if ( m_UpDeg() >= MAX_CST_DEG )
    {
        return;
    }

    cst_airfoil_type cst;

    MakeCSTAirfoil( cst );

    cst.upper_degree_promote();

    CSTtoParms( cst );
}

void CSTAirfoil::PromoteLower()
{
    if ( m_LowDeg() >= MAX_CST_DEG )
    {
        return;
    }

    cst_airfoil_type cst;

    MakeCSTAirfoil( cst );

    cst.lower_degree_promote();

    CSTtoParms( cst );
}

void CSTAirfoil::DemoteUpper()
{
    cst_airfoil_type cst;

    MakeCSTAirfoil( cst );

    cst.upper_degree_demote();

    CSTtoParms( cst );
}

void CSTAirfoil::DemoteLower()
{
    cst_airfoil_type cst;

    MakeCSTAirfoil( cst );

    cst.lower_degree_demote();

    CSTtoParms( cst );
}

void CSTAirfoil::FitCurve( VspCurve c, int deg )
{
    piecewise_curve_type pwc;
    pwc = c.GetCurve();

    int n = 101;

    vector < curve_point_type > lpt( n );
    vector < curve_point_type > upt( n );

    double t0 = pwc.get_t0();
    double tmax = pwc.get_tmax();
    double tmid = ( t0 + tmax ) / 2.0;

    for ( int i = 0; i < n; i++ )
    {
        double tlow = tmid - (tmid - t0) * double(i)/double(n-1);
        double tup = tmid + (tmax - tmid) * double(i)/double(n-1);
        lpt[i] = pwc.f( tlow );
        upt[i] = pwc.f( tup );
    }

    cst_fitter_type cst_fitter;

    cst_fitter.set_conditions(upt.begin(), upt.size(), deg,
                              lpt.begin(), lpt.size(), deg, true);

    cst_fitter.set_t0( t0 );
    cst_fitter.set_segment_dt( tmid - t0, 0 );
    cst_fitter.set_segment_dt( tmax - tmid, 1 );

    Eigen::Matrix<double, 3, 3> transform_out;
    curve_point_type translate_out;
    double actual_leading_edge_t;

    cst_airfoil_type cst;

    cst_fitter.create( cst, transform_out, translate_out, actual_leading_edge_t );

    m_Scale = 1.0 / transform_out(2,2);
    m_Theta = std::asin( -transform_out(1,0) * m_Scale() ) * 180.0 / PI;

    translate_out = (translate_out*m_Scale())*transform_out*m_Scale();
    m_DeltaX = -translate_out.x();
    m_DeltaY = -translate_out.y();

    if ( actual_leading_edge_t < 0 )
    {
        m_ShiftLE = -2.0 * sqrt( -actual_leading_edge_t );
    }
    else
    {
        m_ShiftLE = 2.0 * sqrt( actual_leading_edge_t );
    }

    double dte = cst.get_trailing_edge_thickness() * m_Scale();

    if ( std::abs( dte ) > 1e-6 )
    {
        m_TECloseType = CLOSE_SKEWBOTH;
        m_TECloseAbsRel = REL;
        m_TECloseThickChord = dte;
    }

    CSTtoParms( cst );
}

vector < double > CSTAirfoil::GetUpperCST()
{
    vector < double > retvec( m_UpDeg() + 1 );

    for ( int i = 0; i <= m_UpDeg(); ++i )
    {
        Parm* p = m_UpCoeffParmVec[i];
        if ( p )
        {
            retvec[i] = p->Get();
        }
    }

    return retvec;
}

vector < double > CSTAirfoil::GetLowerCST()
{
    vector < double > retvec( m_LowDeg() + 1 );

    for ( int i = 0; i <= m_LowDeg(); ++i )
    {
        Parm* p = m_LowCoeffParmVec[i];
        if ( p )
        {
            retvec[i] = p->Get();
        }
    }

    return retvec;
}

int CSTAirfoil::GetUpperDegree()
{
    return m_UpDeg();
}

int CSTAirfoil::GetLowerDegree()
{
    return m_LowDeg();
}

void CSTAirfoil::SetUpperCST( int deg, const vector < double > &coefs )
{
    ZeroUpParms();

    m_UpDeg = deg;

    ReserveUpDeg( m_UpDeg() );
    for ( int i = 0; i <= m_UpDeg(); ++i )
    {
        Parm* p = m_UpCoeffParmVec[i];
        if ( p )
        {
            p->Set( coefs[i] );
        }
    }
}

void CSTAirfoil::SetLowerCST( int deg, const vector < double > &coefs )
{
    ZeroLowParms();

    m_LowDeg = deg;

    ReserveLowDeg( m_LowDeg() );
    for ( int i = 0; i <= m_LowDeg(); ++i )
    {
        Parm* p = m_LowCoeffParmVec[i];
        if ( p )
        {
            p->Set( coefs[i] );
        }
    }

}

void CSTAirfoil::MakeCSTAirfoil( cst_airfoil_type &cst )
{
    CheckLERad();

    typedef cst_airfoil_type::control_point_type cst_airfoil_control_point_type;

    cst.resize_upper( m_UpDeg() );
    cst.resize_lower( m_LowDeg() );

    cst.set_trailing_edge_thickness( 0.0 );

    for ( int i = 0; i <= m_UpDeg(); ++i )
    {
        Parm* p = m_UpCoeffParmVec[i];
        if ( p )
        {
            cst_airfoil_control_point_type x;
            x << p->Get();
            cst.set_upper_control_point( x, i );
        }
        else
        {
            cst_airfoil_control_point_type x;
            x << 0.0;
            cst.set_upper_control_point( x, i );
        }
    }

    for ( int i = 0; i <= m_LowDeg(); ++i )
    {
        Parm* p = m_LowCoeffParmVec[i];
        if ( p )
        {
            cst_airfoil_control_point_type x;
            x << p->Get();
            cst.set_lower_control_point( x, i );
        }
        else
        {
            cst_airfoil_control_point_type x;
            x << 0.0;
            cst.set_lower_control_point( x, i );
        }
    }
}

void CSTAirfoil::CSTtoParms( cst_airfoil_type &cst )
{
    typedef cst_airfoil_type::control_point_type cst_airfoil_control_point_type;

    ZeroParms();

    m_UpDeg = cst.upper_degree();

    ReserveUpDeg( m_UpDeg() );
    for ( int i = 0; i <= m_UpDeg(); ++i )
    {
        Parm* p = m_UpCoeffParmVec[i];
        if ( p )
        {
            cst_airfoil_control_point_type x;
            x = cst.get_upper_control_point( i );

            p->Set( x.value() );
        }
    }

    m_LowDeg = cst.lower_degree();

    ReserveLowDeg( m_LowDeg() );
    for ( int i = 0; i <= m_LowDeg(); ++i )
    {
        Parm* p = m_LowCoeffParmVec[i];
        if ( p )
        {
            cst_airfoil_control_point_type x;
            x = cst.get_lower_control_point( i );
            p->Set( x.value() );
        }
    }

    CheckLERad();
}

void CSTAirfoil::ZeroParms()
{
    ZeroUpParms();
    ZeroLowParms();
}

void CSTAirfoil::ZeroUpParms()
{
    for ( int i = 0; i < m_UpCoeffParmVec.size(); ++i )
    {
        Parm* p = m_UpCoeffParmVec[i];
        if ( p )
        {
            p->Set( 0.0 );
        }
    }
}

void CSTAirfoil::ZeroLowParms()
{
    for ( int i = 0; i < m_LowCoeffParmVec.size(); ++i )
    {
        Parm* p = m_LowCoeffParmVec[i];
        if ( p )
        {
            p->Set( 0.0 );
        }
    }
}

void CSTAirfoil::ReserveUpDeg( int d )
{
    while ( m_UpCoeffParmVec.size() < d + 1 )
    {
        AddUpParm();
    }
}

void CSTAirfoil::ReserveLowDeg( int d )
{
    while ( m_LowCoeffParmVec.size() < d + 1 )
    {
        AddLowParm();
    }
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr CSTAirfoil::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr child_node = xmlNewChild( node, NULL, BAD_CAST "CSTAirfoil", NULL );
    if ( child_node )
    {
        XmlUtil::AddIntNode( child_node, "NumUpCoeff", m_UpCoeffParmVec.size() );
        XmlUtil::AddIntNode( child_node, "NumLowCoeff", m_LowCoeffParmVec.size() );
    }
    Airfoil::EncodeXml( node );

    return child_node;
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr CSTAirfoil::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr child_node = XmlUtil::GetNode( node, "CSTAirfoil", 0 );
    if ( child_node )
    {
        int nup = XmlUtil::FindInt( child_node, "NumUpCoeff", m_UpCoeffParmVec.size() );
        int nlow = XmlUtil::FindInt( child_node, "NumLowCoeff", m_LowCoeffParmVec.size() );

        ReserveUpDeg( nup - 1 );
        ReserveLowDeg( nlow - 1 );
    }
    Airfoil::DecodeXml( node );

    return child_node;
}

string CSTAirfoil::AddUpParm()
{
    Parm* p = ParmMgr.CreateParm( PARM_DOUBLE_TYPE );

    if ( p )
    {
        int i = m_UpCoeffParmVec.size();
        char str[255];
        sprintf( str, "Au_%d", i );
        p->Init( string( str ), "UpperCoeff", this, 0.0, -1.0e12, 1.0e12 );
        p->SetDescript( "Upper surface CST coefficient" );
        m_UpCoeffParmVec.push_back( p );
        return p->GetID();
    }
    return string();
}

string CSTAirfoil::AddLowParm()
{
    Parm* p = ParmMgr.CreateParm( PARM_DOUBLE_TYPE );

    if ( p )
    {
        int i = m_LowCoeffParmVec.size();
        char str[255];
        sprintf( str, "Al_%d", i );
        p->Init( string( str ), "LowerCoeff", this, 0.0, -1.0e12, 1.0e12 );
        p->SetDescript( "Lower surface CST coefficient" );
        m_LowCoeffParmVec.push_back( p );
        return p->GetID();
    }
    return string();
}

void CSTAirfoil::CheckLERad()
{
    if ( m_ContLERad() )
    {
        if ( m_UpCoeffParmVec.size() >= 1 && m_LowCoeffParmVec.size() >= 1 )
        {
            m_LowCoeffParmVec[0]->Set( -m_UpCoeffParmVec[0]->Get() );
        }
    }
}

void CSTAirfoil::OffsetCurve( double offset_val )
{
    double t = CalculateThick();
    double c = m_Chord();

    double offset_c = c - 2.0*offset_val;
    m_Chord = offset_c;

    double offset_t = t - 2.0 * offset_val;

    if ( offset_t < 0 )
    {
        offset_t = 0;
    }

    m_yscale = ( offset_t / offset_c ) / ( t / c );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
VKTAirfoil::VKTAirfoil( ) : Airfoil( )
{
    m_Type = XS_VKT_AIRFOIL;

    m_Epsilon.Init( "Epsilon", m_GroupName, this, 0.1, 0.0, 10.0 );
    m_Kappa.Init( "Kappa", m_GroupName, this, 0.1, -5.0, 5.0 );
    m_Tau.Init( "Tau", m_GroupName, this, 10.0, 0.0, 180.0 );
}

//==== Update ====//
void VKTAirfoil::Update()
{
    int npts = 101;

    vector< vec3d > pnts( npts );

    int ile = 0;
    double dmax = -1.0;
    // Evaluate points and track furthest from TE as surrogate for LE.
    // Would be better to identify LE as tightest curvature or similar.
    for ( int i = 0; i < npts - 1; i++ )
    {
        // Clockwise from TE
        double theta = 2.0 * PI * (1.0 - i * 1.0 / ( npts - 1 ) );
        pnts[i] = vkt_airfoil_point( theta, m_Epsilon(), m_Kappa(), m_Tau() * PI / 180.0 );

        double d = dist( pnts[i], pnts[0] );
        if ( d > dmax )
        {
            dmax = d;
            ile = i;
        }
    }
    pnts[npts-1] = pnts[0]; // Ensure closure

    // Shift and scale airfoil such that xle=0 and xte=1.
    double scale = pnts[ 0 ].x() - pnts[ ile ].x();
    double xshift = pnts[ ile ].x();

    for ( int i = 0; i < npts; i++ )
    {
        pnts[i].offset_x( -xshift );
        pnts[i] = pnts[i] / scale;
    }

    vector< double > arclen;

    arclen.resize( npts );
    arclen[0] = 0.0;

    for ( int i = 1 ; i < npts ; i++ )
    {
        double ds = dist( pnts[i], pnts[i-1] );
        if ( ds < 1e-8 )
        {
            ds = 1.0/npts;
        }
        arclen[i] = arclen[i-1] + ds;
    }

    double lenlower = arclen[ile];
    double lenupper = arclen[npts-1] - lenlower;

    double lowerscale = 2.0/lenlower;
    int i;
    for ( i = 0; i < ile; i++ )
    {
        arclen[i] = arclen[i] * lowerscale;
    }

    double upperscale = 2.0/lenupper;
    for ( ; i < npts; i++ )
    {
        arclen[i] = 2.0 + ( arclen[i] - lenlower) * upperscale;
    }

    m_Curve.InterpolatePCHIP( pnts, arclen, false );

    Airfoil::Update();
}

void VKTAirfoil::OffsetCurve( double offset_val )
{
    double t = CalculateThick();
    double c = m_Chord();

    double offset_c = c - 2.0*offset_val;
    m_Chord = offset_c;

    double offset_t = t - 2.0 * offset_val;

    if ( offset_t < 0 )
    {
        offset_t = 0;
    }

    m_yscale = ( offset_t / offset_c ) / ( t / c );
}
