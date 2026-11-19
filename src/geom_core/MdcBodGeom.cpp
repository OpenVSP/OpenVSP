//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "MdcBodGeom.h"
#include "APIDefines.h"
#include "Vehicle.h"

#include <algorithm>

using namespace vsp;

//==== Constructor ====//
MdcBodGeom::MdcBodGeom( Vehicle* vehicle_ptr ) : XSecSurfGeom( vehicle_ptr )
{
    m_Name = "MDC_BOD";
    m_Type.m_Name = "MDC_BOD";
    m_Type.m_Type = MDC_BOD_GEOM_TYPE;

    // The end caps are set from the shape, so they are not offered.
    HideEndCaps();

    m_Namelist.Init( "NAMELIST", "Design", this, AXIBOD, AXIBOD, ELLBOD );
    m_Namelist.SetDescript( "Namelist the body follows: AXIBOD or ELLBOD" );

    m_TNose.Init( "TNOSE", "Design", this, OGIVE, CONE, KARMAN );
    m_TNose.SetDescript( "Nose shape: cone, ogive, power law, L-V Haack or Von Karman" );
    m_Power.Init( "POWER", "Design", this, 1.0, 0.05, 20.0 );
    m_Power.SetDescript( "Exponent n of a power law nose, (r/R) = (x/L)^n" );
    m_LNose.Init( "LNOSE", "Design", this, 3.0, 1.0e-6, 1.0e12 );
    m_LNose.SetDescript( "Nose length" );
    m_DNose.Init( "DNOSE", "Design", this, 1.0, 1.0e-6, 1.0e12 );
    m_DNose.SetDescript( "Nose diameter at base" );
    m_WNose.Init( "WNOSE", "Design", this, 1.0, 1.0e-6, 1.0e12 );
    m_WNose.SetDescript( "Nose width at base" );
    m_ENose.Init( "ENOSE", "Design", this, 1.0, 1.0e-6, 1.0e6 );
    m_ENose.SetDescript( "Ellipticity at nose base (height/width)" );
    m_BNose.Init( "BNOSE", "Design", this, 0.0, 0.0, 1.0e12 );
    m_BNose.SetDescript( "Nose bluntness radius or radius of truncation" );
    m_Trunc.Init( "TRUNC", "Design", this, false, false, true );
    m_Trunc.SetDescript( "Flag for a truncated nose" );
    m_ZTip.Init( "ZTIP", "Design", this, 0.0, -1.0e12, 1.0e12 );
    m_ZTip.SetDescript( "Body camber at front tip of nose" );
    m_ZNose.Init( "ZNOSE", "Design", this, 0.0, -1.0e12, 1.0e12 );
    m_ZNose.SetDescript( "Body camber at base of nose" );
    m_SlpNos.Init( "SLPNOS", "Design", this, 0.0, -10.0, 10.0 );
    m_SlpNos.SetDescript( "Slope of secant ogive at base of nose" );

    m_LCentr.Init( "LCENTR", "Design", this, 7.0, 0.0, 1.0e12 );
    m_LCentr.SetDescript( "Centerbody length" );
    m_CentrDefault.Init( "CENTRDefault", "Design", this, true, false, true );
    m_CentrDefault.SetDescript( "Flag for the centerbody's base to take the nose's size, DCENTR = DNOSE and WCENTR = "
                                "WNOSE, as DATCOM's default" );
    m_DCentr.Init( "DCENTR", "Design", this, 1.0, 1.0e-6, 1.0e12 );
    m_DCentr.SetDescript( "Centerbody diameter at base" );
    m_WCentr.Init( "WCENTR", "Design", this, 1.0, 1.0e-6, 1.0e12 );
    m_WCentr.SetDescript( "Centerbody width at base" );
    m_ECentr.Init( "ECENTR", "Design", this, 1.0, 1.0e-6, 1.0e6 );
    m_ECentr.SetDescript( "Ellipticity at centerbody base (height/width)" );
    m_ZCentr.Init( "ZCENTR", "Design", this, 0.0, -1.0e12, 1.0e12 );
    m_ZCentr.SetDescript( "Body camber at aft end of centerbody" );

    m_TAft.Init( "TAFT", "Design", this, CONE, CONE, OGIVE );
    m_TAft.SetDescript( "Aft body shape: cone or ogive" );
    m_LAft.Init( "LAFT", "Design", this, 0.0, 0.0, 1.0e12 );
    m_LAft.SetDescript( "Aft body length" );
    m_DAft.Init( "DAFT", "Design", this, 0.8, 1.0e-6, 1.0e12 );
    m_DAft.SetDescript( "Aft body diameter at base" );
    m_WAft.Init( "WAFT", "Design", this, 0.8, 1.0e-6, 1.0e12 );
    m_WAft.SetDescript( "Aft body width at base" );
    m_EAft.Init( "EAFT", "Design", this, 1.0, 1.0e-6, 1.0e6 );
    m_EAft.SetDescript( "Ellipticity at aft body base (height/width)" );
    m_ZAft.Init( "ZAFT", "Design", this, 0.0, -1.0e12, 1.0e12 );
    m_ZAft.SetDescript( "Body camber at aft end of afterbody" );
    m_SlpTal.Init( "SLPTAL", "Design", this, 0.0, -10.0, 10.0 );
    m_SlpTal.SetDescript( "Slope of secant ogive at forward end of boattail or aft end of flared afterbody" );

    m_DExit.Init( "DEXIT", "Design", this, 0.0, 0.0, 1.0e12 );
    m_DExit.SetDescript( "Nozzle exit diameter, drawn as a cone into the base DEXIT / 4 deep" );

    m_NoseTessU.Init( "NoseTess_U", "Shape", this, 9, 2, 1000 );
    m_NoseTessU.SetDescript( "Number of rows along the nose, and across a flat base" );
    m_CentrTessU.Init( "CentrTess_U", "Shape", this, 9, 2, 1000 );
    m_CentrTessU.SetDescript( "Number of rows along the centerbody" );
    m_AftTessU.Init( "AftTess_U", "Shape", this, 9, 2, 1000 );
    m_AftTessU.SetDescript( "Number of rows along the aft body, and across a nozzle exit" );

    m_Preset.Init( "Preset", "Design", this, vsp::MDC_BOD_PRESET_DEFAULT, vsp::MDC_BOD_PRESET_DEFAULT, vsp::NUM_MDC_BOD_PRESET - 1 );
    m_Preset.SetDescript( "Preset shape Set gives the body: as new, or a generic rocket's" );
    GetPresetValues( { "Design", "EndCap" }, { "Preset" }, m_DefaultValues );

    m_TessU = 9;
    m_TessW = 17;
}

//==== Destructor ====//
MdcBodGeom::~MdcBodGeom()
{
}

// The generic rocket's body, 175 long and 10 across, nose and tail capped.
static const XSecSurfGeom::PresetValue rocket_body[] = {
    { "Design", "BNOSE", 0.0 },
    { "Design", "CENTRDefault", 1.0 },
    { "Design", "DAFT", 9.0 },
    { "Design", "DCENTR", 10.0 },
    { "Design", "DEXIT", 0.0 },
    { "Design", "DNOSE", 10.0 },
    { "Design", "EAFT", 1.0 },
    { "Design", "ECENTR", 1.0 },
    { "Design", "ENOSE", 1.0 },
    { "Design", "LAFT", 3.0 },
    { "Design", "LCENTR", 142.0 },
    { "Design", "LNOSE", 30.0 },
    { "Design", "NAMELIST", 0.0 },
    { "Design", "POWER", 1.0 },
    { "Design", "SLPNOS", 0.0 },
    { "Design", "SLPTAL", 0.0 },
    { "Design", "TAFT", 0.0 },
    { "Design", "TNOSE", 4.0 },
    { "Design", "TRUNC", 0.0 },
    { "Design", "WAFT", 9.0 },
    { "Design", "WCENTR", 10.0 },
    { "Design", "WNOSE", 10.0 },
    { "Design", "ZAFT", 0.0 },
    { "Design", "ZCENTR", 0.0 },
    { "Design", "ZNOSE", 0.0 },
    { "Design", "ZTIP", 0.0 },
    { "EndCap", "CapUMaxHaackC", 0.0 },
    { "EndCap", "CapUMaxLength", 0.0 },
    { "EndCap", "CapUMaxLengthDim", 0.0 },
    { "EndCap", "CapUMaxOffset", 0.0 },
    { "EndCap", "CapUMaxOffsetDim", 0.0 },
    { "EndCap", "CapUMaxOgiveAngle", 0.0 },
    { "EndCap", "CapUMaxOption", 5.0 },
    { "EndCap", "CapUMaxPowerExp", 0.5 },
    { "EndCap", "CapUMaxRadius", 0.05555352670287594 },
    { "EndCap", "CapUMaxRadiusDim", 0.24999999999999975 },
    { "EndCap", "CapUMaxRelFlag", 0.0 },
    { "EndCap", "CapUMaxScaledFlag", 0.0 },
    { "EndCap", "CapUMaxShearFlag", 0.0 },
    { "EndCap", "CapUMaxStrength", 0.5 },
    { "EndCap", "CapUMaxSweepFlag", 0.0 },
    { "EndCap", "CapUMaxTruncFlag", 0.0 },
    { "EndCap", "CapUMinHaackC", 0.0 },
    { "EndCap", "CapUMinLength", 5.99978088391061 },
    { "EndCap", "CapUMinLengthDim", 30.0 },
    { "EndCap", "CapUMinOffset", 0.0 },
    { "EndCap", "CapUMinOffsetDim", 0.0 },
    { "EndCap", "CapUMinOgiveAngle", 0.0 },
    { "EndCap", "CapUMinOption", 14.0 },
    { "EndCap", "CapUMinPowerExp", 1.0 },
    { "EndCap", "CapUMinRadius", 0.0 },
    { "EndCap", "CapUMinRadiusDim", 0.0 },
    { "EndCap", "CapUMinRelFlag", 0.0 },
    { "EndCap", "CapUMinScaledFlag", 1.0 },
    { "EndCap", "CapUMinShearFlag", 0.0 },
    { "EndCap", "CapUMinStrength", 0.5 },
    { "EndCap", "CapUMinSweepFlag", 0.0 },
    { "EndCap", "CapUMinTess", 3.0 },
    { "EndCap", "CapUMinTruncFlag", 0.0 }
};

void MdcBodGeom::SetPreset()
{
    vector< PresetValue > vals = m_DefaultValues;
    if ( m_Preset() == vsp::MDC_BOD_PRESET_GENERIC_ROCKET )
    {
        vals = vector< PresetValue >( rocket_body, rocket_body + sizeof( rocket_body ) / sizeof( rocket_body[0] ) );
    }
    SetPresetValues( vals );
}

//==== Scale ====//
void MdcBodGeom::ApplyScale( double currentScale )
{
    m_LNose.Set( currentScale * m_LNose() );
    m_DNose.Set( currentScale * m_DNose() );
    m_WNose.Set( currentScale * m_WNose() );
    m_BNose.Set( currentScale * m_BNose() );
    m_ZTip.Set( currentScale * m_ZTip() );
    m_ZNose.Set( currentScale * m_ZNose() );
    m_LCentr.Set( currentScale * m_LCentr() );
    m_DCentr.Set( currentScale * m_DCentr() );
    m_WCentr.Set( currentScale * m_WCentr() );
    m_ZCentr.Set( currentScale * m_ZCentr() );
    m_LAft.Set( currentScale * m_LAft() );
    m_DAft.Set( currentScale * m_DAft() );
    m_WAft.Set( currentScale * m_WAft() );
    m_ZAft.Set( currentScale * m_ZAft() );
    m_DExit.Set( currentScale * m_DExit() );
}

void MdcBodGeom::OgiveEnds( double x0, double r0, double x1, double r1, double slp, double d[4] )
{
    // The larger end is p, the smaller q, and s runs from p towards q along x.
    double px = x0;
    double pr = r0;
    double qx = x1;
    double qr = r1;
    double s = 1.0;
    if ( r1 > r0 )
    {
        px = x1;
        pr = r1;
        qx = x0;
        qr = r0;
        s = -1.0;
    }

    // The arc's direction at p, along x towards q and down towards the axis.
    double a = atan( slp );
    double dx = s * cos( a );
    double dr = -sin( a );

    // The arc bends towards q, about a center on the normal to its direction at p.
    double cx = qx - px;
    double cr = qr - pr;
    double nx = -dr;
    double nr = dx;
    double cn = cx * nx + cr * nr;
    if ( cn < 0.0 )
    {
        nx = -nx;
        nr = -nr;
        cn = -cn;
    }

    double chord = sqrt( cx * cx + cr * cr );
    double turn = 0.0;
    double str = chord;
    double qdx = dx;
    double qdr = dr;
    if ( cn > 1.0e-12 * chord )
    {
        double rho = chord * chord / ( 2.0 * cn );
        turn = 2.0 * asin( chord / ( 2.0 * rho ) );
        str = 4.0 * rho * tan( 0.25 * turn );

        // The direction at q is the direction at p turned through the arc towards the center.
        qdx = dx * cos( turn ) + nx * sin( turn );
        qdr = dr * cos( turn ) + nr * sin( turn );
    }

    // Derivatives along the section, from xsec a to xsec b, in x and r.
    d[0] = dx * str;
    d[1] = dr * str;
    d[2] = qdx * str;
    d[3] = qdr * str;
    if ( s < 0.0 )
    {
        d[0] = -qdx * str;
        d[1] = -qdr * str;
        d[2] = -dx * str;
        d[3] = -dr * str;
    }
}

void MdcBodGeom::OgiveSection( BareXSec* xa, BareXSec* xb, double x0, double h0, double w0, double x1, double h1, double w1,
                               double slp, bool first )
{
    double dh[4];
    double dw[4];
    OgiveEnds( x0, h0, x1, h1, slp, dh );
    OgiveEnds( x0, w0, x1, w1, slp, dw );

    // A surface's first section is skinned from its left side's settings, so where xsec a is first both its
    // sides are set.
    int side = XSEC_RIGHT_SIDE;
    if ( first )
    {
        side = XSEC_BOTH_SIDES;
    }
    xa->SetTanDerivatives( side, dh[0], dh[1], dw[0], dw[1] );
    xb->SetTanDerivatives( XSEC_LEFT_SIDE, dh[2], dh[3], dw[2], dw[3] );
}

void MdcBodGeom::UpdateSurf()
{
    bool ell = m_Namelist() == ELLBOD;
    int tnose = m_TNose();
    double lnose = m_LNose();
    double bnose = m_BNose();
    bool trunc = m_Trunc();
    double slpnos = m_SlpNos();
    double lcentr = m_LCentr();
    int taft = m_TAft();
    double laft = m_LAft();
    double slptal = m_SlpTal();

    if ( m_CentrDefault() )
    {
        m_DCentr.Set( m_DNose() );
        m_WCentr.Set( m_WNose() );
    }

    // The other namelist's sizes follow this one's, so a change of namelist changes the body least.
    if ( ell )
    {
        m_DNose.Set( m_WNose() );
        m_DCentr.Set( m_WCentr() );
        m_DAft.Set( m_WAft() );
    }
    else
    {
        m_WNose.Set( m_DNose() );
        m_WCentr.Set( m_DCentr() );
        m_WAft.Set( m_DAft() );
    }

    // Each station's size across, a diameter or a width, and its ellipticity.
    double snose = m_DNose();
    double scentr = m_DCentr();
    double saft = m_DAft();
    double enose = 1.0;
    double ecentr = 1.0;
    double eaft = 1.0;
    if ( ell )
    {
        snose = m_WNose();
        scentr = m_WCentr();
        saft = m_WAft();
        enose = m_ENose();
        ecentr = m_ECentr();
        eaft = m_EAft();
    }

    // The loft's stations, each with its half width and half height, and the camber's, which start at the
    // nose's tip.
    vector< double > xs;
    vector< double > ws;
    vector< double > hs;
    vector< int > shapes;    // The shape of the section ahead of each station after the first.
    vector< double > slps;
    m_SectPart.clear();

    vector< double > cx = { 0.0, lnose };
    vector< double > cz = { m_ZTip(), m_ZNose() };

    xs.push_back( lnose );
    ws.push_back( 0.5 * snose );
    hs.push_back( 0.5 * snose * enose );

    double x = lnose;
    if ( lcentr > 0.0 )
    {
        x += lcentr;
        xs.push_back( x );
        ws.push_back( 0.5 * scentr );
        hs.push_back( 0.5 * scentr * ecentr );
        shapes.push_back( CONE );
        slps.push_back( 0.0 );
        m_SectPart.push_back( CENTR_PART );
        cx.push_back( x );
        cz.push_back( m_ZCentr() );
    }

    if ( laft > 0.0 )
    {
        x += laft;
        xs.push_back( x );
        ws.push_back( 0.5 * saft );
        hs.push_back( 0.5 * saft * eaft );
        shapes.push_back( taft );
        slps.push_back( slptal );
        m_SectPart.push_back( AFT_PART );
        cx.push_back( x );
        cz.push_back( m_ZAft() );
    }

    // A nose alone is closed by its base, a flat section in to a point.
    bool nose_only = xs.size() == 1;
    if ( nose_only )
    {
        xs.push_back( xs[0] );
        ws.push_back( 0.0 );
        hs.push_back( 0.0 );
        shapes.push_back( CONE );
        slps.push_back( 0.0 );
        m_SectPart.push_back( NOSE_PART );
    }

    BeginXSecSurfs();
    XSecSurf* xsec_surf = NewXSecSurf();

    vector< BareXSec* > xsecs;
    for ( int i = 0; i < ( int )xs.size(); i++ )
    {
        BareXSec* xsec = nullptr;
        if ( ws[i] > 0.0 )
        {
            xsec = AppendEllipseXSec( xsec_surf, vec3d( xs[i], 0.0, 0.0 ), 2.0 * ws[i], 2.0 * hs[i] );
        }
        else
        {
            xsec = AppendPointXSec( xsec_surf, vec3d( xs[i], 0.0, 0.0 ) );
        }
        xsecs.push_back( xsec );
    }

    // The nozzle exit in the base: a flat ring out to the exit, where the exit is smaller than the base, and a cone
    // in to a point.
    bool nozzle = !nose_only && m_DExit() > 0.0;
    if ( nozzle )
    {
        int last = ( int )xs.size() - 1;
        double rbase = std::min( ws[last], hs[last] );
        double dexit = std::min( m_DExit(), 2.0 * rbase );
        if ( dexit < 2.0 * rbase - Tol( rbase ) )
        {
            AppendCircleXSec( xsec_surf, vec3d( xs[last], 0.0, 0.0 ), dexit );
            m_SectPart.push_back( AFT_PART );
        }
        AppendPointXSec( xsec_surf, vec3d( xs[last] - 0.25 * dexit, 0.0, 0.0 ) );
        m_SectPart.push_back( AFT_PART );
    }

    for ( int i = 0; i < ( int )shapes.size(); i++ )
    {
        if ( shapes[i] == OGIVE )
        {
            OgiveSection( xsecs[i], xsecs[i + 1], xs[i], hs[i], ws[i], xs[i + 1], hs[i + 1], ws[i + 1], slps[i],
                          i == 0 );
        }
    }

    // The nose, uncambered, along x, each row the base scaled.
    m_CapUMinRelFlag.Set( false );
    m_CapUMinScaledFlag.Set( true );
    if ( tnose == CONE )
    {
        m_CapUMinOption.Set( CONE_END_CAP );
    }
    else if ( tnose == OGIVE )
    {
        m_CapUMinOption.Set( OGIVE_END_CAP );
        m_CapUMinOgiveAngle.Set( atan( slpnos ) * 180.0 / M_PI );
    }
    else if ( tnose == POWER )
    {
        m_CapUMinOption.Set( POWER_LAW_END_CAP );
        m_CapUMinPowerExp.Set( m_Power() );
    }
    else if ( tnose == HAACK )
    {
        m_CapUMinOption.Set( VON_KARMAN_END_CAP );
        m_CapUMinHaackC.Set( 1.0 / 3.0 );
    }
    else
    {
        m_CapUMinOption.Set( VON_KARMAN_END_CAP );
        m_CapUMinHaackC.Set( 0.0 );
    }
    m_CapUMinLengthDim.Set( lnose );
    m_CapUMinOffsetDim.Set( 0.0 );
    m_CapUMinRadiusDim.Set( bnose );
    m_CapUMinTruncFlag.Set( trunc );

    // The base, flat in to a point at its middle, which a nose alone and a nozzle have already.
    if ( nose_only || nozzle )
    {
        m_CapUMaxOption.Set( NO_END_CAP );
    }
    else
    {
        m_CapUMaxOption.Set( POINT_END_CAP );
        m_CapUMaxRelFlag.Set( false );
        m_CapUMaxLengthDim.Set( 0.0 );
        m_CapUMaxOffsetDim.Set( 0.0 );
    }

    EndXSecSurfs();
    SkinXSecSurf( false );

    // Moved up by the camber, once capped.
    m_CamberX = cx;
    m_CamberZ = cz;
    m_MainSurfVec[0].SetCamber( xsec_surf->GetGlobalXForm().xform( vec3d() ), cx, cz );
}

int MdcBodGeom::SectTessU( int isect ) const
{
    int part = CENTR_PART;
    if ( isect >= 0 && isect < ( int )m_SectPart.size() )
    {
        part = m_SectPart[isect];
    }
    if ( part == NOSE_PART )
    {
        return m_NoseTessU();
    }
    if ( part == AFT_PART )
    {
        return m_AftTessU();
    }
    return m_CentrTessU();
}

// The nose cap, each section by its part, and a base cap, which a Stack tessellates as it does its nose cap.
bool MdcBodGeom::GetTessUVec( const VspSurf &surf, bool capUMinSuccess, bool capUMaxSuccess,
                              vector< int > &tessvec ) const
{
    tessvec.clear();
    if ( m_CapUMinOption() != NO_END_CAP && capUMinSuccess )
    {
        tessvec.push_back( m_NoseTessU() );
    }
    for ( int i = 0; i < ( int )m_SectPart.size(); i++ )
    {
        tessvec.push_back( SectTessU( i ) );
    }
    if ( m_CapUMaxOption() != NO_END_CAP && capUMaxSuccess )
    {
        tessvec.push_back( m_NoseTessU() );
    }
    return true;
}

//==== Decode Data From XML Data Struct ====//
xmlNodePtr MdcBodGeom::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr n = XSecSurfGeom::DecodeXml( node );

    // A body saved before it had rows by part takes Tess_U for each.
    if ( DesignValue( node, "NoseTess_U", -1.0, "Shape" ) < 0.0 )
    {
        m_NoseTessU.Set( m_TessU() );
        m_CentrTessU.Set( m_TessU() );
        m_AftTessU.Set( m_TessU() );
    }

    return n;
}

void MdcBodGeom::GetCamber( vector< double > & x, vector< double > & z ) const
{
    x = m_CamberX;
    z = m_CamberZ;
}

void MdcBodGeom::ComputeCenter()
{
    double length = m_LNose() + m_LCentr() + m_LAft();
    m_Center = vec3d( m_Origin() * length, 0.0, 0.0 );
}

void MdcBodGeom::UpdateEndCaps()
{
    XSecSurfGeom::UpdateEndCaps();
    UpdateProfile();
}

void MdcBodGeom::UpdateProfile()
{
    m_ProfileX.clear();
    m_ProfileHalfW.clear();
    m_ProfileHalfH.clear();
    m_ProfileZMid.clear();
    if ( m_MainSurfVec.empty() )
    {
        return;
    }

    // Each section's four quarter points are the ends of its ellipse's axes.  A flat end, where x stands still,
    // keeps its first, outermost point.
    const VspSurf & surf = m_MainSurfVec[0];
    int n = 400;
    for ( int i = 0; i <= n; i++ )
    {
        double u = double( i ) / double( n );
        vec3d p0 = surf.CompPnt01( u, 0.0 );
        vec3d p1 = surf.CompPnt01( u, 0.25 );
        vec3d p2 = surf.CompPnt01( u, 0.5 );
        vec3d p3 = surf.CompPnt01( u, 0.75 );

        double x = 0.25 * ( p0.x() + p1.x() + p2.x() + p3.x() );
        if ( !m_ProfileX.empty() && x <= m_ProfileX.back() )
        {
            continue;
        }

        double ylo = std::min( std::min( p0.y(), p1.y() ), std::min( p2.y(), p3.y() ) );
        double yhi = std::max( std::max( p0.y(), p1.y() ), std::max( p2.y(), p3.y() ) );
        double zlo = std::min( std::min( p0.z(), p1.z() ), std::min( p2.z(), p3.z() ) );
        double zhi = std::max( std::max( p0.z(), p1.z() ), std::max( p2.z(), p3.z() ) );

        m_ProfileX.push_back( x );
        m_ProfileHalfW.push_back( 0.5 * ( yhi - ylo ) );
        m_ProfileHalfH.push_back( 0.5 * ( zhi - zlo ) );
        m_ProfileZMid.push_back( 0.5 * ( zhi + zlo ) );
    }

    // The children that stand on the body follow its new shape.
    DirtyStandingChildren( this );
}

void MdcBodGeom::DirtyStandingChildren( Geom* body )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh || !body )
    {
        return;
    }

    vector< string > child_vec = body->GetChildIDVec();
    for ( int i = 0; i < ( int )child_vec.size(); i++ )
    {
        XSecSurfGeom* child = dynamic_cast< XSecSurfGeom* >( veh->FindGeom( child_vec[i] ) );
        if ( child && child->ShapeFollowsParent() )
        {
            child->SetDirtyFlag( GeomBase::SURF );
        }
    }
}

void MdcBodGeom::UpdateChildren( bool fullupdate )
{
    if ( m_UpdateXForm )
    {
        DirtyStandingChildren( this );
    }

    XSecSurfGeom::UpdateChildren( fullupdate );
}

bool MdcBodGeom::Radius( double x, double phi, double & r ) const
{
    int n = ( int )m_ProfileX.size();
    if ( n < 2 || x < m_ProfileX[0] || x > m_ProfileX[n - 1] )
    {
        return false;
    }

    int i = ( int )( std::upper_bound( m_ProfileX.begin(), m_ProfileX.end(), x ) - m_ProfileX.begin() ) - 1;
    i = std::min( std::max( i, 0 ), n - 2 );
    double f = ( x - m_ProfileX[i] ) / ( m_ProfileX[i + 1] - m_ProfileX[i] );
    double a = m_ProfileHalfW[i] + f * ( m_ProfileHalfW[i + 1] - m_ProfileHalfW[i] );
    double b = m_ProfileHalfH[i] + f * ( m_ProfileHalfH[i + 1] - m_ProfileHalfH[i] );
    double zc = m_ProfileZMid[i] + f * ( m_ProfileZMid[i + 1] - m_ProfileZMid[i] );

    if ( a < 1.0e-12 || b < 1.0e-12 )
    {
        r = 0.0;
        return true;
    }

    // Where the ray from the axis in the direction phi meets the ellipse a across and b high about zc.
    double dy = sin( phi * M_PI / 180.0 );
    double dz = cos( phi * M_PI / 180.0 );
    double qa = dy * dy / ( a * a ) + dz * dz / ( b * b );
    double qb = -2.0 * dz * zc / ( b * b );
    double qc = zc * zc / ( b * b ) - 1.0;
    double disc = std::max( qb * qb - 4.0 * qa * qc, 0.0 );
    r = std::max( ( -qb + sqrt( disc ) ) / ( 2.0 * qa ), 0.0 );
    return true;
}

double MdcBodGeom::MaxRadius() const
{
    double r = 0.0;
    for ( int i = 0; i < ( int )m_ProfileX.size(); i++ )
    {
        r = std::max( r, std::max( m_ProfileHalfW[i], m_ProfileHalfH[i] ) );
    }
    return r;
}

bool MdcBodGeom::MaxRadiusOfParent( Geom* geom_ptr, double & r )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return false;
    }
    const MdcBodRole* body = Geom::CastTo< MdcBodRole >( veh->FindGeom( geom_ptr->GetParentID() ) );
    if ( !body )
    {
        return false;
    }
    r = body->MaxRadius();
    return r > 0.0;
}

bool MdcBodGeom::RadiusUnder( Geom* geom_ptr, double x, double phi, double & r )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return false;
    }
    const MdcBodRole* body = Geom::CastTo< MdcBodRole >( veh->FindGeom( geom_ptr->GetParentID() ) );
    if ( !body )
    {
        return false;
    }

    // The point and the direction phi in the body's shape frame, which a Clone's flip reflects.
    Matrix4d to_body = body->GetRoleShapeMatrix();
    to_body.affineInverse();
    vec3d p = to_body.xform( geom_ptr->getModelMatrix().xform( vec3d( x, 0.0, 0.0 ) ) );
    vec3d d = body->GetRoleFlipMat().xformnorm( vec3d( 0.0, sin( phi * M_PI / 180.0 ), cos( phi * M_PI / 180.0 ) ) );
    return body->Radius( p.x(), atan2( d.y(), d.z() ) * 180.0 / M_PI, r );
}

double MdcBodGeom::FallUnder( Geom* geom_ptr, double x, double phi, double rr, double half )
{
    // The edge's roll off phi is where the body's radius r there reaches half across: r sin( off ) = half.
    double fall = -1.0e30;
    for ( int side = -1; side <= 1; side += 2 )
    {
        double r = rr;
        RadiusUnder( geom_ptr, x, phi, r );
        double off = asin( std::min( half / r, 1.0 ) );
        for ( int it = 0; it < 50; it++ )
        {
            RadiusUnder( geom_ptr, x, phi + side * off * 180.0 / M_PI, r );
            double next = asin( std::min( half / r, 1.0 ) );
            double step = next - off;
            off = next;
            if ( fabs( step ) < 1.0e-12 )
            {
                break;
            }
        }
        fall = std::max( fall, rr - r * cos( off ) );
    }
    return fall;
}
