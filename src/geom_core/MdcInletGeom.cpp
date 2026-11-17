//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "MdcInletGeom.h"
#include "MdcBodGeom.h"
#include "APIDefines.h"

#include <algorithm>

using namespace vsp;

const double MdcInletGeom::RAMP_END = 0.01;
const double MdcInletGeom::DIV_FIT = 0.95;

//==== Constructor ====//
MdcInletGeom::MdcInletGeom( Vehicle* vehicle_ptr ) : XSecSurfGeom( vehicle_ptr )
{
    m_Name = "MDC_INLET";
    m_Type.m_Name = "MDC_INLET";
    m_Type.m_Type = MDC_INLET_GEOM_TYPE;

    // The end caps are set from the shape, so they are not offered.
    HideEndCaps();

    // Its members are placed about the axis, not about a point along it, so there is nothing to rotate about.
    m_Origin.Deactivate();

    // Along a new MDC_BOD, clear of its nose.
    m_XRelLoc.Set( 4.0 );

    m_InType.Init( "INTYPE", "Design", this, TOP2D, TOP2D, AXI );
    m_InType.SetDescript( "Type of inlet: 2DTOP, 2DSIDE or AXI" );
    m_RBody.Init( "RBODY", "Design", this, 0.5, 1.0e-6, 1.0e12 );
    m_RBody.SetDescript( "Body radius under the inlet, from the axis to the mold line (not an INLET input)" );
    m_XDiv.Init( "XDIV", "Design", this, 0.5, 0.0, 1.0e12 );
    m_XDiv.SetDescript( "Longitudinal distance from inlet leading edge to diverter leading edge" );
    m_HDiv.Init( "HDIV", "Design", this, 0.1, -1.0e12, 1.0e12 );
    m_HDiv.SetDescript( "Diverter height at leading edge.  HDIV=0 defines conformal inlet.  HDIV<0 defines "
                        "semi-submerged inlet" );
    m_LDiv.Init( "LDIV", "Design", this, 2.0, 0.0, 1.0e12 );
    m_LDiv.SetDescript( "Length of diverter" );

    m_NIn.Init( "NIN", "Design", this, 1, 1, MAX_IN );
    m_NIn.SetDescript( "Number of inlets" );
    m_Inlet.Init( "INLET", "Design", this, 1, 1, MAX_IN );
    m_Inlet.SetDescript( "Inlet the roll input shows" );
    m_PhiFlag.Init( "PHIFlag", "Design", this, false, false, true );
    m_PhiFlag.SetDescript( "Flag to give PHI, else the inlets are evenly spaced from PHI = 0" );
    SizeParms();

    const char* where[NUM_STN] = { "leading edge or tip", "cowl lip leading edge", "midbody start", "boattail start",
                                   "boattail end" };
    double xs[NUM_STN] = { 0.0, 0.5, 0.6, 2.5, 3.0 };
    double hs[NUM_STN] = { 0.0, 0.4, 0.45, 0.45, 0.3 };
    double ws[NUM_STN] = { 0.5, 0.6, 0.6, 0.6, 0.5 };
    for ( int i = 1; i <= NUM_STN; i++ )
    {
        string at = string( " at the " ) + where[i - 1];
        if ( i > 1 )
        {
            m_X[i - 2].Init( "X" + to_string( i ), "Design", this, xs[i - 1], 0.0, 1.0e12 );
            m_X[i - 2].SetDescript( "Inlet longitudinal position relative to inlet leading edge" + at );
            m_H[i - 2].Init( "H" + to_string( i ), "Design", this, hs[i - 1], 1.0e-6, 1.0e12 );
            m_H[i - 2].SetDescript( "Inlet height" + at + ".  Not required if INTYPE=AXI" );
        }
        m_W[i - 1].Init( "W" + to_string( i ), "Design", this, ws[i - 1], 0.0, 1.0e12 );
        m_W[i - 1].SetDescript( "Inlet width if INTYPE=2DTOP or 2DSIDE, diameter if INTYPE=AXI" + at );
    }

    m_Cover.Init( "COVER", "Design", this, false, false, true );
    m_Cover.SetDescript( "Flag for covered inlet" );
    m_Ramp.Init( "RAMP", "Design", this, 15.0, 0.1, 89.0 );
    m_Ramp.SetDescript( "External compression inlet ramp angle" );

    m_Preset.Init( "Preset", "Design", this, vsp::MDC_INLET_PRESET_DEFAULT, vsp::MDC_INLET_PRESET_DEFAULT, vsp::NUM_MDC_INLET_PRESET - 1 );
    m_Preset.SetDescript( "Preset shape Set gives the inlet: as new, or a generic rocket's" );
    GetPresetValues( { "Design" }, { "Preset", "INLET" }, m_DefaultValues );

    m_TessU = 9;
    m_TessW = 17;
}

//==== Destructor ====//
MdcInletGeom::~MdcInletGeom()
{
    DeleteParms( m_Phi );
}

void MdcInletGeom::SizeParms()
{
    ResizeParms( m_Phi, m_NIn(), vsp::PARM_DOUBLE_TYPE, "PHI", 0.0, -360.0, 720.0,
                 "Inlet roll orientation measured clockwise from top vertical center looking forward" );
}

//==== Decode Data From XML Data Struct ====//
xmlNodePtr MdcInletGeom::DecodeXml( xmlNodePtr & node )
{
    m_NIn.Set( DesignValue( node, "NIN", m_NIn() ) );
    SizeParms();

    return XSecSurfGeom::DecodeXml( node );
}

// The generic rocket's inlet, under its body of radius 5.
static const XSecSurfGeom::PresetValue rocket_inlet[] = {
    { "Design", "COVER", 0.0 },
    { "Design", "H2", 2.5 },
    { "Design", "H3", 3.0 },
    { "Design", "H4", 3.0 },
    { "Design", "H5", 1.0 },
    { "Design", "HDIV", 0.0 },
    { "Design", "INTYPE", 1.0 },
    { "Design", "LDIV", 2.0 },
    { "Design", "NIN", 1.0 },
    { "Design", "PHIFlag", 1.0 },
    { "Design", "PHI_1", 180.0 },
    { "Design", "RAMP", 10.0 },
    { "Design", "RBODY", 5.0 },
    { "Design", "W1", 4.0 },
    { "Design", "W2", 5.5 },
    { "Design", "W3", 6.5 },
    { "Design", "W4", 6.5 },
    { "Design", "W5", 3.0 },
    { "Design", "X2", 6.0 },
    { "Design", "X3", 10.0 },
    { "Design", "X4", 55.0 },
    { "Design", "X5", 60.0 },
    { "Design", "XDIV", 0.5 }
};

void MdcInletGeom::SetPreset()
{
    vector< PresetValue > vals = m_DefaultValues;
    double scale = 1.0;
    if ( m_Preset() == vsp::MDC_INLET_PRESET_GENERIC_ROCKET )
    {
        vals = vector< PresetValue >( rocket_inlet, rocket_inlet + sizeof( rocket_inlet ) / sizeof( rocket_inlet[0] ) );
        double rmax = 0.0;
        if ( MdcBodGeom::MaxRadiusOfParent( this, rmax ) )
        {
            scale = rmax / 5.0;
        }
    }

    // The count first, then the rolls it sizes.
    SetPresetValues( vals );
    SizeParms();
    SetPresetValues( vals );
    if ( scale != 1.0 )
    {
        ApplyScale( scale );
    }
}

//==== Scale ====//
void MdcInletGeom::ApplyScale( double currentScale )
{
    m_RBody.Set( currentScale * m_RBody() );
    m_XDiv.Set( currentScale * m_XDiv() );
    m_HDiv.Set( currentScale * m_HDiv() );
    m_LDiv.Set( currentScale * m_LDiv() );
    for ( int i = 0; i < NUM_STN; i++ )
    {
        m_W[i].Set( currentScale * m_W[i]() );
    }
    for ( int i = 0; i < NUM_STN - 1; i++ )
    {
        m_X[i].Set( currentScale * m_X[i]() );
        m_H[i].Set( currentScale * m_H[i]() );
    }
}

MdcInletGeom::Sec MdcInletGeom::NewSec( double x, double ya, double yb, double za, double zb )
{
    Sec s;
    s.x = x;
    s.ya = ya;
    s.yb = yb;
    s.za = za;
    s.zb = zb;
    return s;
}

MdcInletGeom::Sec MdcInletGeom::Box( int intype, double x, double h, double w, double hmax, double zf )
{
    if ( intype == SIDE2D )
    {
        double t0 = -0.5 * hmax;
        return NewSec( x, t0, t0 + h, zf, zf + w );
    }
    return NewSec( x, -0.5 * w, 0.5 * w, zf, zf + h );
}

MdcInletGeom::Sec MdcInletGeom::Round( double x, double d, double ha )
{
    return NewSec( x, -0.5 * d, 0.5 * d, ha - 0.5 * d, ha + 0.5 * d );
}

void MdcInletGeom::BuildSecs( int intype, double r, double hd )
{
    m_Secs.clear();
    // Each station's position, height and width, station k at index k - 1.
    double sx[NUM_STN];
    double sh[NUM_STN];
    double sw[NUM_STN];
    sx[0] = 0.0;
    sh[0] = 0.0;
    sw[0] = m_W[0]();
    double hmax = 0.0;

    // Stations at least a thousandth of the length apart, so a change of section between two always has length.
    double gap = 1.0e-3 * std::max( std::max( m_X[3](), m_X[2]() ), 1.0e-3 );
    for ( int i = 1; i < NUM_STN; i++ )
    {
        sx[i] = std::max( m_X[i - 1](), sx[i - 1] + gap );
        sh[i] = m_H[i - 1]();
        sw[i] = m_W[i]();
        hmax = std::max( hmax, sh[i] );
    }

    double zf = r + hd;
    double ha = zf + 0.5 * sw[2];
    bool cover = m_Cover();
    double xr = sx[1] - sx[0];
    double tr = tan( m_Ramp() * M_PI / 180.0 );
    double tol = Tol( r );

    if ( intype == AXI )
    {
        m_Secs.push_back( Round( sx[0], sw[0], ha ) );
        double rs = 0.5 * sw[0] + xr * tr;
        if ( !cover && xr > 2.0 * gap && 2.0 * rs < sw[1] - tol )
        {
            m_Secs.push_back( Round( sx[1], 2.0 * rs, ha ) );
        }
        for ( int i = 1; i < NUM_STN; i++ )
        {
            m_Secs.push_back( Round( sx[i], sw[i], ha ) );
        }
        return;
    }

    // The ramp ends RAMP_END of its length short of the lip, and the face between it and the cowl leans back
    // that far, so no surface runs along an edge of the section.
    m_Secs.push_back( Box( intype, sx[0], 0.0, sw[0], hmax, zf ) );
    double xe = xr * ( 1.0 - RAMP_END );
    double hr = xe * tr;
    if ( !cover && xr > 2.0 * gap && hr < sh[1] - tol )
    {
        double wr = sw[0] + ( sw[1] - sw[0] ) * ( 1.0 - RAMP_END );
        m_Secs.push_back( Box( intype, sx[0] + xe, hr, wr, hmax, zf ) );
    }
    for ( int i = 1; i < NUM_STN; i++ )
    {
        m_Secs.push_back( Box( intype, sx[i], sh[i], sw[i], hmax, zf ) );
    }
}

MdcInletGeom::Sec MdcInletGeom::Shared( double x, double tol )
{
    double ya = -1.0e30;
    double yb = 1.0e30;
    double zm = 1.0e30;
    for ( int i = 0; i < ( int )m_Secs.size(); i++ )
    {
        const Sec & a = m_Secs[i];
        if ( fabs( a.x - x ) <= tol )
        {
            ya = std::max( ya, a.ya );
            yb = std::min( yb, a.yb );
            zm = std::min( zm, 0.5 * ( a.za + a.zb ) );
        }
        if ( i + 1 < ( int )m_Secs.size() )
        {
            const Sec & b = m_Secs[i + 1];
            if ( b.x - a.x > tol && x > a.x + tol && x < b.x - tol )
            {
                double f = ( x - a.x ) / ( b.x - a.x );
                ya = std::max( ya, a.ya + f * ( b.ya - a.ya ) );
                yb = std::min( yb, a.yb + f * ( b.yb - a.yb ) );
                zm = std::min( zm, 0.5 * ( ( a.za + a.zb ) + f * ( ( b.za + b.zb ) - ( a.za + a.zb ) ) ) );
            }
        }
    }
    return NewSec( x, ya, yb, zm, zm );
}

// A linear section at x through the box ya to yb and za to zb, its four corners spaced evenly in its parameter.
void MdcInletGeom::AddBoxSection( XSecSurf* surf, double x, double ya, double yb, double za, double zb )
{
    vector< double > ys = { ya, ya, yb, yb };
    vector< double > zs = { za, zb, zb, za };
    AddPolySection( surf, x, ys, zs );
}

void MdcInletGeom::InletSurf( int intype, double tol )
{
    XSecSurf* surf = NewXSecSurf();
    NeedTessW( 4 );
    for ( int i = 0; i < ( int )m_Secs.size(); i++ )
    {
        const Sec & s = m_Secs[i];
        double yc = 0.5 * ( s.ya + s.yb );
        double zc = 0.5 * ( s.za + s.zb );
        if ( intype == AXI )
        {
            double d = s.yb - s.ya;
            if ( d <= tol )
            {
                AppendPointXSec( surf, vec3d( s.x, yc, zc ) );
            }
            else
            {
                if ( i == 0 )
                {
                    AppendPointXSec( surf, vec3d( s.x, yc, zc ) );
                }
                AppendCircleXSec( surf, vec3d( s.x, yc, zc ), d );
            }
        }
        else
        {
            AddBoxSection( surf, s.x, s.ya, s.yb, s.za, s.zb );
        }
    }
    const Sec & e = m_Secs.back();
    AppendPointXSec( surf, vec3d( e.x, 0.5 * ( e.ya + e.yb ), 0.5 * ( e.za + e.zb ) ) );
}

bool MdcInletGeom::DiverterSurf( double r, double hd, double tol )
{
    double l = m_Secs.back().x - m_Secs[0].x;
    double xd = m_Secs[0].x + m_XDiv();
    double xe = xd + m_LDiv();
    double xend = m_Secs.back().x - 1.0e-3 * l;
    xe = std::min( xe, xend );
    // An apex at the leading edge has nothing of the inlet around it.
    xd = std::max( xd, m_Secs[0].x + 1.0e-2 * ( m_Secs[1].x - m_Secs[0].x ) );
    if ( xe - xd <= tol )
    {
        return false;
    }

    vector< double > xs = { xd };
    for ( int i = 0; i < ( int )m_Secs.size(); i++ )
    {
        double x = m_Secs[i].x;
        if ( x > xd + tol && x < xe - tol && x > xs.back() + tol )
        {
            xs.push_back( x );
        }
    }
    xs.push_back( xe );

    Sec ee = Shared( xe, tol );
    double yc = 0.5 * ( ee.ya + ee.yb );
    double wend = 2.0 * DIV_FIT * std::max( 0.0, std::min( yc - ee.ya, ee.yb - yc ) );

    int n = ( int )xs.size();
    vector< double > wd;
    vector< double > zt;
    double ymax = 0.0;
    for ( int i = 0; i < n; i++ )
    {
        Sec s = Shared( xs[i], tol );
        double lim = 2.0 * DIV_FIT * std::max( 0.0, std::min( yc - s.ya, s.yb - yc ) );
        double w = std::min( wend * ( xs[i] - xd ) / ( xe - xd ), lim );
        wd.push_back( w );
        zt.push_back( s.za );
        ymax = std::max( ymax, fabs( yc ) + 0.5 * w );
    }
    double zbot = Under( r, ymax ) - 0.1 * hd;

    XSecSurf* surf = NewXSecSurf();
    for ( int i = 0; i < n; i++ )
    {
        AddBoxSection( surf, xs[i], yc - 0.5 * wd[i], yc + 0.5 * wd[i], zbot, zt[i] );
    }
    AppendPointXSec( surf, vec3d( xe, yc, 0.5 * ( zbot + zt[n - 1] ) ) );
    return true;
}

void MdcInletGeom::UpdateSurf()
{
    SizeParms();

    int nin = m_NIn();
    m_Inlet.SetLowerUpperLimits( 1, nin );
    m_Inlet.Set( m_Inlet() );

    SpaceRolls( m_PhiFlag(), nin, m_Phi );

    int intype = m_InType();

    // The body's radius under each inlet's leading edge: a parent MDC_BOD's, as DATCOM takes it from the body, else
    // RBODY.
    vector< double > radii( nin, m_RBody() );
    bool attached = true;
    for ( int k = 0; k < nin && attached; k++ )
    {
        attached = MdcBodGeom::RadiusUnder( this, 0.0, m_Phi[k]->Get(), radii[k] );
    }
    if ( attached )
    {
        m_RBody.Set( radii[0] );
        m_RBody.Deactivate();
    }
    else
    {
        radii.assign( nin, m_RBody() );
        m_RBody.Activate();
    }

    // One inlet, rolled to each place, where they all sit on the same radius; else one at each place.
    bool same = true;
    for ( int k = 1; k < nin; k++ )
    {
        same = same && fabs( radii[k] - radii[0] ) <= Tol( radii[0] );
    }
    int nbuild = nin;
    if ( same )
    {
        nbuild = 1;
    }

    m_TessMult = 1;
    BeginXSecSurfs();
    vector< int > nsurf;
    for ( int k = 0; k < nbuild; k++ )
    {
        nsurf.push_back( AddSurfs( intype, radii[k] ) );
    }

    // Rows around every surface at every corner of its sections.
    m_TessW.SetMultShift( m_TessMult, 1 );

    // Every surface is closed by its own end sections.
    m_CapUMinOption.Set( NO_END_CAP );
    m_CapUMaxOption.Set( NO_END_CAP );

    EndXSecSurfs();
    SkinXSecSurf( false );

    if ( same )
    {
        Matrix4d pre;
        pre.loadIdentity();
        RollSurfs( nsurf[0], nin, m_Phi, pre );
        return;
    }

    int first = 0;
    for ( int k = 0; k < nin; k++ )
    {
        // Clockwise looking forward, from z towards y, is negative about x.
        Matrix4d mat;
        mat.loadIdentity();
        mat.rotateX( -m_Phi[k]->Get() );
        for ( int j = 0; j < nsurf[k]; j++ )
        {
            TransformSurf( first + j, mat );
        }
        first += nsurf[k];
    }
}

int MdcInletGeom::AddSurfs( int intype, double r )
{
    double hd = m_HDiv();
    double sink = hd;
    if ( hd == 0.0 )
    {
        sink = -0.01 * r;
    }
    double tol = Tol( r );

    BuildSecs( intype, r, sink );
    InletSurf( intype, tol );
    if ( hd > 0.0 && DiverterSurf( r, hd, tol ) )
    {
        return 2;
    }
    return 1;
}
