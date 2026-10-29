//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "MdcProtubGeom.h"
#include "MdcBodGeom.h"
#include "APIDefines.h"

#include <algorithm>

using namespace vsp;

//==== Constructor ====//
MdcProtubGeom::MdcProtubGeom( Vehicle* vehicle_ptr ) : XSecSurfGeom( vehicle_ptr )
{
    m_Name = "MDC_PROTUB";
    m_Type.m_Name = "MDC_PROTUB";
    m_Type.m_Type = MDC_PROTUB_GEOM_TYPE;

    // The end caps are set from the shape, so they are not offered.
    HideEndCaps();

    // Its members are placed about the axis, not about a point along it, so there is nothing to rotate about.
    m_Origin.Deactivate();

    // Along a new MDC_BOD, clear of its nose.
    m_XRelLoc.Set( 5.0 );

    m_PType.Init( "PTYPE", "Design", this, VCYL, VCYL, FAIRING );
    m_PType.SetDescript( "Protuberance set type: VCYL, HCYL, LUG, SHOE, BLOCK or FAIRING" );
    m_RBody.Init( "RBODY", "Design", this, 0.5, 1.0e-6, 1.0e12 );
    m_RBody.SetDescript( "Body radius at XPROT, from the axis to the mold line (not a PROTUB input)" );
    m_NLoc.Init( "NLOC", "Design", this, 1, 1, MAX_LOC );
    m_NLoc.SetDescript( "Number of protuberances in set" );
    m_LocIndex.Init( "LocIndex", "Design", this, 1, 1, MAX_LOC );
    m_LocIndex.SetDescript( "Protuberance shown for editing its PHIPRO" );
    m_PhiProFlag.Init( "PHIPROFlag", "Design", this, false, false, true );
    m_PhiProFlag.SetDescript( "Flag to give PHIPRO, else the protuberances are evenly spaced from PHIPRO = 0" );

    SizeParms();

    m_TessU = 9;
    m_TessW = 17;
}

//==== Destructor ====//
MdcProtubGeom::~MdcProtubGeom()
{
    DeleteParms( m_PhiPro );
    DeleteParms( m_LProt );
    DeleteParms( m_WProt );
    DeleteParms( m_HProt );
    DeleteParms( m_OProt );
}

void MdcProtubGeom::SizeParms()
{
    ResizeParms( m_PhiPro, m_NLoc(), vsp::PARM_DOUBLE_TYPE, "PHIPRO", 0.0, -360.0, 360.0,
                 "Protuberance angular orientation measured clockwise from top vertical center looking forward" );

    int nmem = NumMembers( m_PType() );
    ResizeParms( m_LProt, nmem, vsp::PARM_DOUBLE_TYPE, "LPROT", 0.5, 1.0e-6, 1.0e12, "Length of protuberance" );
    ResizeParms( m_WProt, nmem, vsp::PARM_DOUBLE_TYPE, "WPROT", 0.25, 1.0e-6, 1.0e12, "Width of protuberance" );
    ResizeParms( m_HProt, nmem, vsp::PARM_DOUBLE_TYPE, "HPROT", 0.1, 1.0e-6, 1.0e12, "Height of protuberance" );
    ResizeParms( m_OProt, nmem, vsp::PARM_DOUBLE_TYPE, "OPROT", 0.0, 0.0, 1.0e12, "Vertical offset of protuberance" );
}

//==== Decode Data From XML Data Struct ====//
xmlNodePtr MdcProtubGeom::DecodeXml( xmlNodePtr & node )
{
    m_NLoc.Set( DesignValue( node, "NLOC", m_NLoc() ) );
    m_PType.Set( DesignValue( node, "PTYPE", m_PType() ) );
    SizeParms();

    return XSecSurfGeom::DecodeXml( node );
}

//==== Scale ====//
void MdcProtubGeom::ApplyScale( double currentScale )
{
    m_RBody.Set( currentScale * m_RBody() );
    for ( int m = 0; m < ( int )m_LProt.size(); m++ )
    {
        m_LProt[m]->Set( currentScale * m_LProt[m]->Get() );
        m_WProt[m]->Set( currentScale * m_WProt[m]->Get() );
        m_HProt[m]->Set( currentScale * m_HProt[m]->Get() );
        m_OProt[m]->Set( currentScale * m_OProt[m]->Get() );
    }
}

int MdcProtubGeom::NumMembers( int ptype )
{
    if ( ptype == LUG )
    {
        return 4;
    }
    if ( ptype == SHOE )
    {
        return 3;
    }
    return 1;
}

void MdcProtubGeom::SetDefaults()
{
    // LPROT, WPROT, HPROT and OPROT of each member, for a body 6 in radius.  The sample body has no HCYL or LUG.
    vector< vector< double > > mems;
    int ptype = m_PType();
    if ( ptype == VCYL )
    {
        mems = { { 1.0, 1.0, 0.5, 0.0 } };
    }
    else if ( ptype == HCYL )
    {
        mems = { { 3.0, 1.0, 1.0, 0.0 } };
    }
    else if ( ptype == LUG )
    {
        mems = { { 3.0, 2.0, 0.1, 0.0 }, { 1.5, 0.25, 0.5, 0.1 }, { 1.5, 0.25, 0.5, 0.1 }, { 1.5, 1.0, 0.15, 0.6 } };
    }
    else if ( ptype == SHOE )
    {
        mems = { { 10.0, 4.0, 0.1, 0.0 }, { 10.0, 0.25, 0.75, 0.1 }, { 10.0, 1.0, 0.25, 0.85 } };
    }
    else if ( ptype == BLOCK )
    {
        mems = { { 0.5, 2.0, 1.5, 0.0 } };
    }
    else
    {
        mems = { { 5.0, 2.0, 1.0, 0.0 } };
    }

    SizeParms();

    double k = m_RBody() / 6.0;
    for ( int m = 0; m < ( int )mems.size(); m++ )
    {
        m_LProt[m]->Set( k * mems[m][0] );
        m_WProt[m]->Set( k * mems[m][1] );
        m_HProt[m]->Set( k * mems[m][2] );
        m_OProt[m]->Set( k * mems[m][3] );
    }
}

double MdcProtubGeom::Bottom( double r, double w, double h, double o )
{
    if ( o > 0.0 )
    {
        return r + o;
    }
    return Under( r, 0.5 * w ) - 0.1 * h;
}

double MdcProtubGeom::Overlap( double a0, double a1, double b0, double b1 )
{
    return std::max( 0.0, std::min( a1, b1 ) - std::max( a0, b0 ) );
}

void MdcProtubGeom::BuryIfTouching( Member & a, Member & b, double tol )
{
    if ( fabs( b.z0 - a.z1 ) <= tol )
    {
        Bury( a, b );
    }
    else if ( fabs( a.z0 - b.z1 ) <= tol )
    {
        Bury( b, a );
    }
}

// Buries b, which stands on a, or a, where that puts less outside the other.
void MdcProtubGeom::Bury( Member & a, Member & b )
{
    double shared = 2.0 * std::min( a.hl, b.hl ) * Overlap( a.y0, a.y1, b.y0, b.y1 );
    if ( shared <= 0.0 )
    {
        return;
    }
    double ha = a.z1 - a.z0;
    double hb = b.z1 - b.z0;
    double out_b = ( 2.0 * b.hl * ( b.y1 - b.y0 ) - shared ) * 0.5 * ha;
    double out_a = ( 2.0 * a.hl * ( a.y1 - a.y0 ) - shared ) * 0.5 * hb;
    if ( out_b <= out_a )
    {
        b.z0 = a.z1 - 0.5 * ha;
    }
    else
    {
        a.z1 = b.z0 + 0.5 * hb;
    }
}

MdcProtubGeom::Member MdcProtubGeom::MemberBox( int m, double r, double y0, double y1 )
{
    double w = m_WProt[m - 1]->Get();
    double h = m_HProt[m - 1]->Get();
    double o = m_OProt[m - 1]->Get();

    Member mem;
    mem.hl = 0.5 * m_LProt[m - 1]->Get();
    mem.y0 = y0;
    mem.y1 = y1;
    mem.z0 = Bottom( r, w, h, o );
    mem.z1 = r + o + h;
    return mem;
}

BareXSec* MdcProtubGeom::AddSection( XSecSurf* surf, int shape, double x, double y, double z, double w, double h )
{
    BareXSec* xs = nullptr;
    if ( shape == BOX )
    {
        xs = AppendRectXSec( surf, vec3d( x, y, z ), w, h );
    }
    else if ( shape == ELLIPSE )
    {
        xs = AppendEllipseXSec( surf, vec3d( x, y, z ), w, h );
    }
    else
    {
        xs = AppendPointXSec( surf, vec3d( x, y, z ) );
    }
    return xs;
}

// A prism of the section shape w wide and h high from x0 to x1, centered at y and z, flat at its ends.
void MdcProtubGeom::Prism( XSecSurf* surf, int shape, double x0, double x1, double y, double z, double w, double h )
{
    if ( shape == BOX )
    {
        NeedTessW( 8 );
    }
    else
    {
        NeedTessW( 4 );
    }
    AddSection( surf, POINT, x0, y, z, 0.0, 0.0 );
    AddSection( surf, shape, x0, y, z, w, h );
    AddSection( surf, shape, x1, y, z, w, h );
    AddSection( surf, POINT, x1, y, z, 0.0, 0.0 );
}

// The top half of an ellipsoid l long, w wide and h high, built along x from its flat base at the origin for
// UpdateSurf to stand up on z and turn to lie along x: the base, an ellipse, is the surface's one section, which the
// end caps close, flat below and round h long above.  The round cap's poles are at the ends of its length.
void MdcProtubGeom::HalfEllipsoid( XSecSurf* surf, double l, double w )
{
    NeedTessW( 4 );
    AddSection( surf, ELLIPSE, 0.0, 0.0, 0.0, l, w );
}

// The box as a surface of its own.
void MdcProtubGeom::BoxSurf( double x, const Member & mem )
{
    Prism( NewXSecSurf(), BOX, x - mem.hl, x + mem.hl, 0.5 * ( mem.y0 + mem.y1 ), 0.5 * ( mem.z0 + mem.z1 ),
           mem.y1 - mem.y0, mem.z1 - mem.z0 );
}

// The polygon ( ys, zs ) extruded from x0 to x1, each end closed by lines from its points to the matching points
// ( sys, szs ) of a spine inside it, a section with no area.
void MdcProtubGeom::Extrude( double x0, double x1, const vector< double > & ys, const vector< double > & zs,
                          const vector< double > & sys, const vector< double > & szs )
{
    NeedTessW( ( int )ys.size() );
    XSecSurf* surf = NewXSecSurf();
    AddPolySection( surf, x0, sys, szs );
    AddPolySection( surf, x0, ys, zs );
    AddPolySection( surf, x1, ys, zs );
    AddPolySection( surf, x1, sys, szs );
}

// Members centered across the body and of one length as surfaces, one for each run of them that meet or
// overlap, and answers how many.
int MdcProtubGeom::Staircase( double x, const vector< Member > & group, double tol )
{
    // The spans between the heights where a member starts or ends, each as wide as the widest member over it.
    vector< double > zs;
    for ( int i = 0; i < ( int )group.size(); i++ )
    {
        zs.push_back( group[i].z0 );
        zs.push_back( group[i].z1 );
    }
    std::sort( zs.begin(), zs.end() );

    vector< double > lo;
    vector< double > hi;
    vector< double > hw;
    for ( int k = 0; k + 1 < ( int )zs.size(); k++ )
    {
        double a = zs[k];
        double b = zs[k + 1];
        if ( b - a > tol )
        {
            double w = 0.0;
            for ( int i = 0; i < ( int )group.size(); i++ )
            {
                if ( group[i].z0 <= a + tol && group[i].z1 >= b - tol )
                {
                    w = std::max( w, group[i].y1 );
                }
            }
            int n = ( int )hw.size();
            if ( n > 0 && fabs( hw[n - 1] - w ) <= tol )
            {
                hi[n - 1] = b;
            }
            else
            {
                lo.push_back( a );
                hi.push_back( b );
                hw.push_back( w );
            }
        }
    }

    int nsurf = 0;
    int k = 0;
    while ( k < ( int )hw.size() )
    {
        vector< double > rlo;
        vector< double > rhi;
        vector< double > rhw;
        while ( k < ( int )hw.size() && hw[k] > 0.0 )
        {
            rlo.push_back( lo[k] );
            rhi.push_back( hi[k] );
            rhw.push_back( hw[k] );
            k++;
        }
        if ( rhw.size() > 0 )
        {
            StairSurf( x, group[0].hl, rlo, rhi, rhw );
            nsurf++;
        }
        else
        {
            k++;
        }
    }
    return nsurf;
}

// A stack of blocks centered across the body, block k from lo[k] to hi[k] up and hw[k] either side, as one
// surface hl either side of x along it.  Its section runs from the middle of its bottom round to the middle of
// its top and back, fourteen points for up to three blocks, the sides of the tallest split to make them up.
// Its ends close on a spine up its middle: each point on a side goes level to it, each corner of a bottom or a
// top and the outer corner of a step a third of the way into the block it faces, and the inner corner of a step
// a sixth of the way into the wider block, so no line leaves the section or runs along its edge.
void MdcProtubGeom::StairSurf( double x, double hl, const vector< double > & lo, const vector< double > & hi,
                            const vector< double > & hw )
{
    int m = ( int )hw.size();

    // The right half, from the middle of the bottom up, and each point's height on the spine.
    vector< double > ys = { 0.0, hw[0] };
    vector< double > zs = { lo[0], lo[0] };
    vector< double > ts = { lo[0], lo[0] + ( hi[0] - lo[0] ) / 3.0 };

    // Points to split the sides of the tallest block with.
    int npad = 6 - 2 * m;
    int tall = 0;
    for ( int k = 1; k < m; k++ )
    {
        if ( hi[k] - lo[k] > hi[tall] - lo[tall] )
        {
            tall = k;
        }
    }

    for ( int k = 0; k < m; k++ )
    {
        double h = hi[k] - lo[k];
        if ( k == tall )
        {
            for ( int j = 1; j <= npad; j++ )
            {
                double z = lo[k] + h / 3.0 + h / 3.0 * double( j ) / double( npad + 1 );
                ys.push_back( hw[k] );
                zs.push_back( z );
                ts.push_back( z );
            }
        }

        double z = hi[k];
        if ( k + 1 < m )
        {
            if ( hw[k + 1] < hw[k] )
            {
                ys.push_back( hw[k] );
                zs.push_back( z );
                ts.push_back( z - h / 3.0 );
                ys.push_back( hw[k + 1] );
                zs.push_back( z );
                ts.push_back( z - h / 6.0 );
            }
            else
            {
                double hn = hi[k + 1] - z;
                ys.push_back( hw[k] );
                zs.push_back( z );
                ts.push_back( z + hn / 6.0 );
                ys.push_back( hw[k + 1] );
                zs.push_back( z );
                ts.push_back( z + hn / 3.0 );
            }
        }
        else
        {
            ys.push_back( hw[k] );
            zs.push_back( z );
            ts.push_back( z - h / 3.0 );
            ys.push_back( 0.0 );
            zs.push_back( z );
            ts.push_back( z );
        }
    }

    // Round the left side first, so the section runs clockwise looking forward, then back down the right.
    int n = ( int )ys.size();
    vector< double > sy;
    vector< double > sz;
    vector< double > ss;
    for ( int i = 0; i < n; i++ )
    {
        sy.push_back( -ys[i] );
        sz.push_back( zs[i] );
        ss.push_back( ts[i] );
    }
    for ( int i = n - 2; i >= 1; i-- )
    {
        sy.push_back( ys[i] );
        sz.push_back( zs[i] );
        ss.push_back( ts[i] );
    }
    vector< double > sys( sy.size(), 0.0 );
    Extrude( x - hl, x + hl, sy, sz, sys, ss );
}

// A SHOE: its base plate, web and top plate, centered across the body.  Members of one length make one surface
// where they meet or overlap; members of different lengths that meet are buried in each other.
int MdcProtubGeom::Shoe( double x, double r )
{
    double tol = Tol( r );
    vector< Member > mems;
    for ( int m = 1; m <= 3; m++ )
    {
        double w = m_WProt[m - 1]->Get();
        mems.push_back( MemberBox( m, r, -0.5 * w, 0.5 * w ) );
    }

    for ( int i = 0; i < 3; i++ )
    {
        for ( int j = i + 1; j < 3; j++ )
        {
            if ( fabs( mems[i].hl - mems[j].hl ) > tol )
            {
                BuryIfTouching( mems[i], mems[j], tol );
            }
        }
    }

    int nsurf = 0;
    vector< bool > done( 3, false );
    for ( int i = 0; i < 3; i++ )
    {
        if ( !done[i] )
        {
            vector< Member > group;
            for ( int j = i; j < 3; j++ )
            {
                if ( !done[j] && fabs( mems[j].hl - mems[i].hl ) <= tol )
                {
                    group.push_back( mems[j] );
                    done[j] = true;
                }
            }
            nsurf += Staircase( x, group, tol );
        }
    }
    return nsurf;
}

// Whether the post reaches the top plate from below, its top within the plate.
bool MdcProtubGeom::Reaches( const Member & post, const Member & top, double tol )
{
    return post.z0 < top.z0 - tol && post.z1 >= top.z0 - tol && post.z1 <= top.z1 + tol;
}

// A LUG: its base plate, its posts and its top plate.  Posts as long as the top plate that reach it make one
// surface with it, up the first post, across the top plate and down the second, its section running from the
// middle of the first post's end round its outside, to the middle of the second's, and back round its inside,
// its ends closed on a spine up the middle of the posts and across the middle of the top plate.  Otherwise each
// member is a surface of its own, the posts set in from the ends of the top plate by half their widths so their
// sides do not meet its.  Members that meet are buried in each other.
int MdcProtubGeom::Lug( double x, double r )
{
    double tol = Tol( r );
    double w1 = m_WProt[0]->Get();
    double w2 = m_WProt[1]->Get();
    double w3 = m_WProt[2]->Get();
    double yo = 0.5 * m_WProt[3]->Get();

    Member base = MemberBox( 1, r, -0.5 * w1, 0.5 * w1 );
    Member p2 = MemberBox( 2, r, -yo, w2 - yo );
    Member p3 = MemberBox( 3, r, yo - w3, yo );
    Member top = MemberBox( 4, r, -yo, yo );

    bool wrap = fabs( p2.hl - top.hl ) <= tol && fabs( p3.hl - top.hl ) <= tol && Reaches( p2, top, tol ) &&
                Reaches( p3, top, tol ) && p2.y1 < p3.y0 - tol;
    if ( !wrap )
    {
        p2.y0 += 0.5 * w2;
        p2.y1 += 0.5 * w2;
        p3.y0 -= 0.5 * w3;
        p3.y1 -= 0.5 * w3;
    }

    BuryIfTouching( base, p2, tol );
    BuryIfTouching( base, p3, tol );
    BuryIfTouching( base, top, tol );
    BoxSurf( x, base );

    if ( wrap )
    {
        double cl = 0.5 * ( p2.y0 + p2.y1 );
        double cr = 0.5 * ( p3.y0 + p3.y1 );
        double z2 = top.z0;
        double z3 = top.z1;
        double zc = 0.5 * ( z2 + z3 );
        double za2 = p2.z0 + 0.25 * ( z2 - p2.z0 );
        double za3 = p3.z0 + 0.25 * ( z2 - p3.z0 );
        vector< double > ys = { cl, -yo, -yo, yo, yo, cr, p3.y0, p3.y0, p2.y1, p2.y1 };
        vector< double > zs = { p2.z0, p2.z0, z3, z3, p3.z0, p3.z0, p3.z0, z2, z2, p2.z0 };
        vector< double > sys = { cl, cl, cl, cr, cr, cr, cr, cr, cl, cl };
        vector< double > szs = { p2.z0, za2, zc, zc, za3, p3.z0, za3, zc, zc, za2 };
        Extrude( x - top.hl, x + top.hl, ys, zs, sys, szs );
        return 2;
    }

    BuryIfTouching( p2, top, tol );
    BuryIfTouching( p3, top, tol );
    BoxSurf( x, p2 );
    BoxSurf( x, p3 );
    BoxSurf( x, top );
    return 4;
}

int MdcProtubGeom::AddSurfs( int ptype, double r )
{
    // The members are centered on the origin along x.
    double x = 0.0;

    if ( ptype == SHOE )
    {
        return Shoe( x, r );
    }
    if ( ptype == LUG )
    {
        return Lug( x, r );
    }

    double l = m_LProt[0]->Get();
    double w = m_WProt[0]->Get();
    double h = m_HProt[0]->Get();
    double o = m_OProt[0]->Get();

    XSecSurf* surf = NewXSecSurf();
    if ( ptype == VCYL )
    {
        // Built along x from the origin, the height from its bottom, for UpdateSurf to stand up on z.
        Prism( surf, ELLIPSE, 0.0, r + o + h - Bottom( r, w, h, o ), 0.0, 0.0, w, l );
    }
    else if ( ptype == HCYL )
    {
        Prism( surf, ELLIPSE, x - 0.5 * l, x + 0.5 * l, 0.0, r + o + 0.5 * h, w, h );
    }
    else if ( ptype == FAIRING )
    {
        HalfEllipsoid( surf, l, w );
    }
    else
    {
        double zb = Bottom( r, w, h, o );
        double zt = r + o + h;
        Prism( surf, BOX, x - 0.5 * l, x + 0.5 * l, 0.0, 0.5 * ( zb + zt ), w, zt - zb );
    }
    return 1;
}

void MdcProtubGeom::UpdateSurf()
{
    SizeParms();

    int nloc = m_NLoc();
    m_LocIndex.SetLowerUpperLimits( 1, nloc );
    m_LocIndex.Set( m_LocIndex() );

    SpaceRolls( m_PhiProFlag(), nloc, m_PhiPro );

    int ptype = m_PType();

    // The body's radius under each protuberance: a parent MDC_BOD's, as DATCOM takes it from the body, else RBODY.
    vector< double > radii( nloc, m_RBody() );
    bool attached = true;
    for ( int k = 0; k < nloc && attached; k++ )
    {
        attached = MdcBodGeom::RadiusUnder( this, 0.0, m_PhiPro[k]->Get(), radii[k] );
    }
    if ( attached )
    {
        m_RBody.Set( radii[0] );
        m_RBody.Deactivate();
    }
    else
    {
        radii.assign( nloc, m_RBody() );
        m_RBody.Activate();
    }

    // One protuberance, rolled to each place, where they all stand on the same radius; else one at each place.
    bool same = true;
    for ( int k = 1; k < nloc; k++ )
    {
        same = same && fabs( radii[k] - radii[0] ) <= Tol( radii[0] );
    }
    int nbuild = nloc;
    if ( same )
    {
        nbuild = 1;
    }

    m_TessMult = 1;
    BeginXSecSurfs();
    vector< int > nsurf;
    for ( int k = 0; k < nbuild; k++ )
    {
        nsurf.push_back( AddSurfs( ptype, radii[k] ) );
    }

    // Rows around every surface at every corner of its sections.
    m_TessW.SetMultShift( m_TessMult, 1 );

    // Every surface is closed by its own end sections, but a FAIRING's, closed by its caps.
    if ( ptype == FAIRING )
    {
        m_CapUMinOption.Set( FLAT_END_CAP );
        m_CapUMaxOption.Set( ROUND_END_CAP );
        m_CapUMaxRelFlag.Set( false );
        m_CapUMaxLengthDim.Set( m_HProt[0]->Get() );
        m_CapUMinTess.Set( m_TessU() );
    }
    else
    {
        m_CapUMinOption.Set( NO_END_CAP );
        m_CapUMaxOption.Set( NO_END_CAP );
    }

    EndXSecSurfs();
    SkinXSecSurf( false );

    if ( same )
    {
        RollSurfs( nsurf[0], nloc, m_PhiPro, Stand( ptype, radii[0] ) );
        return;
    }

    int first = 0;
    for ( int k = 0; k < nloc; k++ )
    {
        // Clockwise looking forward, from z towards y, is negative about x.
        Matrix4d mat;
        mat.loadIdentity();
        mat.rotateX( -m_PhiPro[k]->Get() );
        Matrix4d stand = Stand( ptype, radii[k] );
        mat.matMult( stand.const_data() );
        for ( int j = 0; j < nsurf[k]; j++ )
        {
            TransformSurf( first + j, mat );
        }
        first += nsurf[k];
    }
}

Matrix4d MdcProtubGeom::Stand( int ptype, double r )
{
    // A VCYL stands on its bottom at the origin, a FAIRING on its base on the body's top line, turned so its length
    // lies along x.
    Matrix4d stand;
    stand.loadIdentity();
    if ( ptype == VCYL )
    {
        double zb = Bottom( r, m_WProt[0]->Get(), m_HProt[0]->Get(), m_OProt[0]->Get() );
        stand.translatef( 0.0, 0.0, zb );
        stand.rotateY( -90.0 );
    }
    else if ( ptype == FAIRING )
    {
        stand.translatef( 0.0, 0.0, r );
        stand.rotateZ( 90.0 );
        stand.rotateY( -90.0 );
    }
    return stand;
}
