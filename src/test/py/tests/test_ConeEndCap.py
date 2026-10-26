# Cone end caps on a Fuselage or Stack.
#
# A cone cap runs each end straight in towards the point a point cap with the same length and offset
# closes on, and blunts it with a sphere that touches that point.  The sphere's center is one radius back
# from the point along the direction the point moves with length, and each side of the cone meets it
# where the side is tangent, then runs over it to its pole, where the sides gather: the sphere point on
# the line from the middle of the end through the center, ahead along the axis.  With no offset the pole
# is the point.  The radius is CapU*Radius times the cap's length scale, or CapU*RadiusDim.  With no
# radius the cone is the point cap.
#
# The cap's half of the end in u is the sphere next to the tip, then the cone side.

import math
import os

import openvsp as vsp
import pytest

from testhelp import drop_errors, assert_no_errors

ENDS = ( ( "UMin", 0.0 ), ( "UMax", 1.0 ) )
NW = 16
TYPES = ( "FUSELAGE", "STACK" )


def body( typ ):
    """An uncapped Fuselage or Stack whose nose is a raised, sideways elliptical section and whose tail is
    a lower super ellipse, flatter below than above."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    n = vsp.GetNumXSec( xss )

    vsp.ChangeXSecShape( xss, 0, vsp.XS_ELLIPSE )
    nose = vsp.GetXSec( xss, 0 )
    vsp.SetParmVal( vsp.GetXSecParm( nose, "Ellipse_Width" ), 1.0 )
    vsp.SetParmVal( vsp.GetXSecParm( nose, "Ellipse_Height" ), 0.6 )

    vsp.ChangeXSecShape( xss, n - 1, vsp.XS_SUPER_ELLIPSE )
    tail = vsp.GetXSec( xss, n - 1 )
    vsp.SetParmVal( vsp.GetXSecParm( tail, "Super_Width" ), 0.8 )
    vsp.SetParmVal( vsp.GetXSecParm( tail, "Super_Height" ), 0.5 )
    vsp.SetParmVal( vsp.GetXSecParm( tail, "Super_TopBotSym" ), 0 )
    vsp.SetParmVal( vsp.GetXSecParm( tail, "Super_M_bot" ), 4.0 )

    if typ == "FUSELAGE":
        vsp.SetParmVal( vsp.GetXSecParm( nose, "YLocPercent" ), 0.02 )
        vsp.SetParmVal( vsp.GetXSecParm( nose, "ZLocPercent" ), 0.05 )
        vsp.SetParmVal( vsp.GetXSecParm( tail, "ZLocPercent" ), -0.04 )
    else:
        vsp.SetParmVal( vsp.GetXSecParm( nose, "YDelta" ), 0.3 )
        vsp.SetParmVal( vsp.GetXSecParm( nose, "ZDelta" ), 0.5 )
        vsp.SetParmVal( vsp.GetXSecParm( tail, "ZDelta" ), -0.4 )
    vsp.Update()
    return gid


def cap_parm( gid, name ):
    return vsp.FindParm( gid, name, "EndCap" )


def set_caps( gid, cap_type, length=1.0, offset=0.0, radius=0.0 ):
    for end, u in ENDS:
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Option" ), cap_type )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Length" ), length )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Offset" ), offset )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Radius" ), radius )
    vsp.Update()


def point( gid, u, w ):
    p = vsp.CompPnt01( gid, 0, u, w )
    return ( p.x(), p.y(), p.z() )


def sub( a, b ):
    return tuple( x - y for x, y in zip( a, b ) )


def dot( a, b ):
    return sum( x * y for x, y in zip( a, b ) )


def norm( a ):
    return math.sqrt( dot( a, a ) )


def nsect( gid ):
    """Sections of the surface capped at both ends, in u."""
    return vsp.GetNumXSec( vsp.GetXSecSurf( gid, 0 ) ) - 1 + 2


def cap_u( gid, u, f ):
    """u of the fraction f of the way from the tip of the end at u across its cap."""
    du = f / nsect( gid )
    if u > 0.5:
        return 1.0 - du
    return du


def sphere( gid, length, offset, radius ):
    """Each end's tip, its sphere's center and its radius, from a point cap with the same length and
    offset and the direction a point cap's point moves with length, with no offset."""
    set_caps( gid, vsp.POINT_END_CAP, 0.0, 0.0 )
    p0 = [ point( gid, u, 0.0 ) for end, u in ENDS ]
    set_caps( gid, vsp.POINT_END_CAP, 1.0, 0.0 )
    p1 = [ point( gid, u, 0.0 ) for end, u in ENDS ]
    set_caps( gid, vsp.POINT_END_CAP, length, offset )
    tips = [ point( gid, u, 0.0 ) for end, u in ENDS ]

    set_caps( gid, vsp.CONE_END_CAP, length, offset, radius )
    out = []
    for i, ( end, u ) in enumerate( ENDS ):
        d = sub( p1[i], p0[i] )
        d = tuple( c / norm( d ) for c in d )
        r = vsp.GetParmVal( cap_parm( gid, "Cap" + end + "RadiusDim" ) )
        cen = tuple( p - r * c for p, c in zip( tips[i], d ) )
        out.append( ( tips[i], cen, r ) )
    return out


@pytest.mark.parametrize( "typ", TYPES )
def testABodyTakesAConeCap( typ ):
    gid = body( typ )
    set_caps( gid, vsp.CONE_END_CAP )
    for end, u in ENDS:
        assert vsp.GetParmVal( cap_parm( gid, "Cap" + end + "Option" ) ) == vsp.CONE_END_CAP
    assert_no_errors()


@pytest.mark.parametrize( "typ", ( "WING", "PROP", "BODYOFREVOLUTION" ) )
def testOtherGeomsRefuseAConeCap( typ ):
    vsp.VSPRenew()
    gid = vsp.AddGeom( typ )
    for end, u in ENDS:
        parm = cap_parm( gid, "Cap" + end + "Option" )
        before = vsp.GetParmVal( parm )
        vsp.SetParmVal( parm, vsp.CONE_END_CAP )
        assert vsp.GetParmVal( parm ) == before, end


@pytest.mark.parametrize( "lenoff", ( ( 1.0, 0.0 ), ( 0.6, 0.3 ), ( 1.5, -0.25 ) ) )
@pytest.mark.parametrize( "typ", TYPES )
def testAConeWithNoRadiusIsAPointCap( typ, lenoff ):
    length, offset = lenoff
    gid = body( typ )
    set_caps( gid, vsp.POINT_END_CAP, length, offset )
    vsp.CopyGeomToClipboard( gid )
    pid = vsp.PasteGeomClipboard()[0]
    set_caps( gid, vsp.CONE_END_CAP, length, offset )

    for i in range( 41 ):
        for j in range( NW + 1 ):
            d, u, w = vsp.ProjPnt01( pid, 0, vsp.vec3d( *point( gid, i / 40.0, j / NW ) ) )
            assert d < 1e-9, ( i, j )


@pytest.mark.parametrize( "radius", ( 0.0, 0.3, 1.0 ) )
@pytest.mark.parametrize( "lenoff", ( ( 1.0, 0.0 ), ( 0.6, 0.3 ), ( 1.5, -0.25 ) ) )
@pytest.mark.parametrize( "typ", TYPES )
def testAConeTipIsThePoleOfItsSphere( typ, lenoff, radius ):
    length, offset = lenoff
    gid = body( typ )
    mids = []
    for end, u in ENDS:
        up = point( gid, u, 0.25 )
        down = point( gid, u, 0.75 )
        mids.append( tuple( 0.5 * ( a + b ) for a, b in zip( up, down ) ) )
    spheres = sphere( gid, length, offset, radius )

    for ( end, u ), mid, ( tip, cen, r ) in zip( ENDS, mids, spheres ):
        pole = tip
        if r > 0.0 and norm( sub( cen, mid ) ) > 1e-9:
            d = sub( cen, mid )
            d = tuple( c / norm( d ) for c in d )
            if dot( d, sub( tip, cen ) ) < 0.0:
                d = tuple( -c for c in d )
            pole = tuple( c + r * e for c, e in zip( cen, d ) )
        if offset == 0.0:
            assert norm( sub( pole, tip ) ) < 1e-9, end
        for j in range( NW + 1 ):
            assert norm( sub( point( gid, u, j / NW ), pole ) ) < 1e-9, ( end, j )


@pytest.mark.parametrize( "radius", ( 0.2, 0.5 ) )
@pytest.mark.parametrize( "lenoff", ( ( 1.0, 0.0 ), ( 1.5, 0.2 ) ) )
@pytest.mark.parametrize( "typ", TYPES )
def testTheBluntTipIsTheSphereAndTheSideTouchesIt( typ, lenoff, radius ):
    length, offset = lenoff
    gid = body( typ )
    spheres = sphere( gid, length, offset, radius )

    for ( end, u ), ( tip, cen, r ) in zip( ENDS, spheres ):
        assert r > 1e-3, end
        for k in range( 11 ):
            for j in range( NW + 1 ):
                p = point( gid, cap_u( gid, u, 0.5 * k / 10 ), j / NW )
                # A cubic quarter circle strays by up to 2.7e-4 of its radius.
                assert abs( norm( sub( p, cen ) ) - r ) < 3e-4 * r, ( end, k, j )

        # Each side is straight, from the tangent point back to the end, and touches the sphere.
        for j in range( NW + 1 ):
            a = point( gid, cap_u( gid, u, 0.5 ), j / NW )
            b = point( gid, cap_u( gid, u, 1.0 ), j / NW )
            m = point( gid, cap_u( gid, u, 0.75 ), j / NW )
            assert norm( sub( m, tuple( 0.5 * ( x + y ) for x, y in zip( a, b ) ) ) ) < 1e-12, ( end, j )

            t = sub( b, a )
            t = tuple( c / norm( t ) for c in t )
            assert abs( dot( sub( a, cen ), t ) ) < 1e-3 * r, ( end, j )


@pytest.mark.parametrize( "typ", TYPES )
def testTheRadiusHasTheLengthScale( typ ):
    gid = body( typ )
    set_caps( gid, vsp.CONE_END_CAP, 1.0, 0.0, 0.4 )
    for end, u in ENDS:
        lscale = vsp.GetParmVal( cap_parm( gid, "Cap" + end + "LengthDim" ) )
        assert vsp.GetParmVal( cap_parm( gid, "Cap" + end + "RadiusDim" ) ) == pytest.approx( 0.4 * lscale, rel=1e-12 )

    for end, u in ENDS:
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RelFlag" ), False )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RadiusDim" ), 0.05 )
    vsp.Update()
    for end, u in ENDS:
        lscale = vsp.GetParmVal( cap_parm( gid, "Cap" + end + "LengthDim" ) )
        assert vsp.GetParmVal( cap_parm( gid, "Cap" + end + "Radius" ) ) == pytest.approx( 0.05 / lscale, rel=1e-12 )


@pytest.mark.parametrize( "typ", TYPES )
def testAConeCapEnclosesMoreAsItsRadiusGrows( typ ):
    """The sphere's front stays on the point, so a bigger sphere makes a fuller nose."""
    gid = body( typ )
    vols = []
    for cap_type, radius in ( ( vsp.POINT_END_CAP, 0.0 ), ( vsp.CONE_END_CAP, 0.0 ), ( vsp.CONE_END_CAP, 0.3 ),
                              ( vsp.CONE_END_CAP, 0.8 ) ):
        set_caps( gid, cap_type, 1.0, 0.0, radius )
        mesh = vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
        rid = vsp.FindLatestResultsID( "Comp_Geom" )
        vols.append( vsp.GetDoubleResults( rid, "Theo_Vol" )[0] )
        vsp.DeleteGeom( mesh )
    assert vols[1] == pytest.approx( vols[0], rel=1e-9 )
    assert 0.0 < vols[0] < vols[2] < vols[3]
    assert_no_errors()


@pytest.mark.parametrize( "typ", TYPES )
def testARadiusTooBigForTheEndIsHonored( typ ):
    """A sphere big enough to reach back past the end keeps its radius and the cap still ends on the point.
    Its sides can only meet it smoothly by running most of the way round it, which one cubic cannot
    follow, so its shape is not checked."""
    gid = body( typ )
    spheres = sphere( gid, 0.5, 0.0, 20.0 )
    for ( end, u ), ( tip, cen, r ) in zip( ENDS, spheres ):
        lscale = vsp.GetParmVal( cap_parm( gid, "Cap" + end + "LengthDim" ) ) / 0.5
        assert r == pytest.approx( 20.0 * lscale, rel=1e-12 ), end
        for i in range( 11 ):
            for j in range( NW + 1 ):
                p = point( gid, cap_u( gid, u, i / 10.0 ), j / NW )
                assert all( math.isfinite( c ) for c in p ), ( end, i, j )
        assert norm( sub( point( gid, u, 0.3 ), tip ) ) < 1e-12, end


@pytest.mark.parametrize( "typ", TYPES )
def testAConeCapIsSavedAndRead( typ, tmp_path ):
    gid = body( typ )
    set_caps( gid, vsp.CONE_END_CAP, 1.1, 0.1, 0.3 )
    before = [ point( gid, i / 20.0, j / NW ) for i in range( 21 ) for j in range( NW + 1 ) ]
    fname = str( tmp_path / "cone.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    gid = vsp.FindGeoms()[0]
    assert vsp.GetParmVal( cap_parm( gid, "CapUMinOption" ) ) == vsp.CONE_END_CAP
    after = [ point( gid, i / 20.0, j / NW ) for i in range( 21 ) for j in range( NW + 1 ) ]
    assert max( norm( sub( a, b ) ) for a, b in zip( before, after ) ) < 1e-12


def testAConeCapSurvivesConversionToAStack():
    gid = body( "FUSELAGE" )
    set_caps( gid, vsp.CONE_END_CAP, 1.1, 0.1, 0.3 )
    before = [ point( gid, i / 20.0, j / NW ) for i in range( 21 ) for j in range( NW + 1 ) ]
    sid = vsp.ConvertFuselageToStack( gid )
    vsp.Update()
    assert vsp.GetParmVal( cap_parm( sid, "CapUMinOption" ) ) == vsp.CONE_END_CAP
    after = [ point( sid, i / 20.0, j / NW ) for i in range( 21 ) for j in range( NW + 1 ) ]
    assert max( norm( sub( a, b ) ) for a, b in zip( before, after ) ) < 1e-9


@pytest.mark.parametrize( "typ", TYPES )
def testAConeCapTessellates( typ ):
    gid = body( typ )
    set_caps( gid, vsp.CONE_END_CAP, 1.0, 0.0, 0.3 )
    mesh = vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    assert mesh
    assert_no_errors()


@pytest.mark.parametrize( "typ", TYPES )
def testACloneOfABodyTakesItsConeCap( typ ):
    gid = body( typ )
    set_caps( gid, vsp.CONE_END_CAP, 1.1, 0.1, 0.3 )
    clone = vsp.AddGeom( "CLONE", gid )
    vsp.Update()
    for end, u in ENDS:
        for name in ( "Option", "Radius", "RadiusDim" ):
            assert vsp.GetParmVal( cap_parm( clone, "Cap" + end + name ) ) == \
                vsp.GetParmVal( cap_parm( gid, "Cap" + end + name ) ), ( end, name )
    assert_no_errors()


@pytest.mark.parametrize( "typ", TYPES )
def testAConformalOnABodyTakesItsConeCap( typ ):
    gid = body( typ )
    set_caps( gid, vsp.CONE_END_CAP, 1.1, 0.1, 0.3 )
    conf = vsp.AddGeom( "CONFORMAL", gid )
    vsp.Update()
    for end, u in ENDS:
        for name in ( "Option", "Radius" ):
            assert vsp.GetParmVal( cap_parm( conf, "Cap" + end + name ) ) == \
                vsp.GetParmVal( cap_parm( gid, "Cap" + end + name ) ), ( end, name )
    assert_no_errors()


@pytest.mark.parametrize( "radius", ( 0.3, 1.0 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testACircularEndGivesARoundNose( typ, radius ):
    """Every section of the cap of a circular end is as round as the end itself, so the tip is a sphere
    to the accuracy of the end's circle."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_CIRCLE )
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, i ), "Circle_Diameter" ), 2.0 )
    set_caps( gid, vsp.CONE_END_CAP, 2.0, 0.0, radius )

    def spread( u ):
        tip = point( gid, 0.0, 0.0 )
        rs = []
        for j in range( 4 * NW + 1 ):
            p = point( gid, u, j / ( 4 * NW ) )
            rs.append( math.hypot( p[1] - tip[1], p[2] - tip[2] ) )
        return ( max( rs ) - min( rs ) ) / max( rs )

    end = spread( cap_u( gid, 0.0, 1.0 ) )
    for k in range( 1, 11 ):
        assert spread( cap_u( gid, 0.0, k / 10.0 ) ) < 1.01 * end, k


@pytest.mark.parametrize( "offset", ( 0.0, 0.3, -0.25 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testAWideOffsetNoseStaysOnTheSphereBetweenControlPoints( typ, offset ):
    """A wide elliptical end, offset, with the sphere and its tangency checked densely across the end,
    not only where the end's control points put the cap's."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_ELLIPSE )
        xs = vsp.GetXSec( xss, i )
        grow = 1.0 + 0.3 * min( i, 1 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "Ellipse_Width" ), 4.0 * grow )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "Ellipse_Height" ), 1.0 * grow )
    vsp.Update()

    # The sphere is wider than the end is high, and the rows fitted across the end's segments reach it to
    # within a few thousandths of its radius.
    nw = 256
    ( end, u ), ( tip, cen, r ) = ENDS[0], sphere( gid, 1.5, offset, 0.5 )[0]
    for k in range( 11 ):
        for j in range( nw + 1 ):
            p = point( gid, cap_u( gid, u, 0.5 * k / 10 ), j / nw )
            assert abs( norm( sub( p, cen ) ) - r ) < 4e-3 * r, ( k, j )
    for j in range( nw + 1 ):
        a = point( gid, cap_u( gid, u, 0.5 ), j / nw )
        b = point( gid, cap_u( gid, u, 1.0 ), j / nw )
        t = sub( b, a )
        t = tuple( c / norm( t ) for c in t )
        assert abs( dot( sub( a, cen ), t ) ) < 3e-3 * r, j


@pytest.mark.parametrize( "offset", ( 1.02, 1.4, -1.1 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testASideTouchesTheSphereOnItsOutwardSide( typ, offset ):
    """Each side from the end has two tangent points on the sphere.  The cap takes the one on the side's
    outward side, away from the middle of the end, even when the offset raises the sphere past part of
    the end."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_CIRCLE )
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, i ), "Circle_Diameter" ), 2.0 )
    for end, u in ENDS:
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Option" ), vsp.CONE_END_CAP )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RelFlag" ), False )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "LengthDim" ), 2.0 )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "OffsetDim" ), offset )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RadiusDim" ), 0.25 )
    vsp.Update()

    nw = 128
    u = ENDS[0][1]
    tip = point( gid, u, 0.0 )
    qs = [ point( gid, cap_u( gid, u, 1.0 ), j / nw ) for j in range( nw ) ]
    mid = tuple( sum( c ) / nw for c in zip( *qs ) )
    for j in range( nw ):
        q = qs[j]
        t = point( gid, cap_u( gid, u, 0.5 ), j / nw )
        qt = sub( t, q )
        qtip = sub( tip, q )
        across = sub( qt, tuple( c * dot( qt, qtip ) / dot( qtip, qtip ) for c in qtip ) )
        assert dot( across, sub( q, mid ) ) > 0.0, j



@pytest.mark.parametrize( "typ", TYPES )
def testATipRaisedOffTheEndStaysConvexOverTheSphere( typ ):
    """Raised well off the end, the sphere stands behind the top of the end, and the side from the top
    touches it behind.  The cap then runs on over the top of the sphere to the tip, not back under it."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_CIRCLE )
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, i ), "Circle_Diameter" ), 2.0 )

    def cap( typ_, length, offset ):
        for end, u in ENDS:
            vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Option" ), typ_ )
            vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RelFlag" ), False )
            vsp.SetParmVal( cap_parm( gid, "Cap" + end + "LengthDim" ), length )
            vsp.SetParmVal( cap_parm( gid, "Cap" + end + "OffsetDim" ), offset )
            vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RadiusDim" ), 1.25 )
        vsp.Update()
        return point( gid, ENDS[0][1], 0.0 )

    # The axis is the way the point moves with length.
    axis = sub( cap( vsp.POINT_END_CAP, 3.3, 0.0 ), cap( vsp.POINT_END_CAP, 2.3, 0.0 ) )
    axis = tuple( c / norm( axis ) for c in axis )
    tip = cap( vsp.POINT_END_CAP, 2.3, 2.0 )
    cen = tuple( p - 1.25 * a for p, a in zip( tip, axis ) )
    cap( vsp.CONE_END_CAP, 2.3, 2.0 )

    nw = 256
    u = ENDS[0][1]
    qs = [ point( gid, cap_u( gid, u, 1.0 ), j / nw ) for j in range( nw ) ]
    mid = tuple( sum( c ) / nw for c in zip( *qs ) )

    # Up: the way the offset raised the tip, and the row from the top of the end, nearest the tip.
    up = sub( tip, mid )
    up = sub( up, tuple( a * dot( up, axis ) for a in axis ) )
    up = tuple( c / norm( up ) for c in up )
    jtop = min( range( nw ), key=lambda j: norm( sub( qs[j], tip ) ) )

    top = max( dot( sub( point( gid, cap_u( gid, u, 0.5 * k / 20 ), jtop / nw ), cen ), up ) for k in range( 21 ) )
    assert top > 0.9 * 1.25


def raised( typ, length, offset, radius ):
    """A body of circular sections, 2 across, with a cone nose of the given dimensional length, offset and
    radius, and the sphere's center: a radius back from a point cap's point along the way that point
    moves with length."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_CIRCLE )
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, i ), "Circle_Diameter" ), 2.0 )

    def cap( cap_type, l, o ):
        for end, u in ENDS:
            vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Option" ), cap_type )
            vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RelFlag" ), False )
            vsp.SetParmVal( cap_parm( gid, "Cap" + end + "LengthDim" ), l )
            vsp.SetParmVal( cap_parm( gid, "Cap" + end + "OffsetDim" ), o )
            vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RadiusDim" ), radius )
        vsp.Update()
        return point( gid, ENDS[0][1], 0.0 )

    axis = sub( cap( vsp.POINT_END_CAP, length + 1.0, 0.0 ), cap( vsp.POINT_END_CAP, length, 0.0 ) )
    axis = tuple( c / norm( axis ) for c in axis )
    tip = cap( vsp.POINT_END_CAP, length, offset )
    cen = tuple( p - radius * a for p, a in zip( tip, axis ) )
    cap( vsp.CONE_END_CAP, length, offset )
    return gid, cen


def outward( gid, u, cen, k, j, nw ):
    """How squarely the cap's surface faces away from cen at fraction k / 20 of its sphere, row j of nw."""
    uu = cap_u( gid, u, 0.5 * k / 20 )
    w = j / nw
    p = point( gid, uu, w )
    du = sub( point( gid, uu + 1e-6, w ), point( gid, uu - 1e-6, w ) )
    dw = sub( point( gid, uu, w + 1e-6 ), point( gid, uu, w - 1e-6 ) )
    n = ( du[1] * dw[2] - du[2] * dw[1], du[2] * dw[0] - du[0] * dw[2], du[0] * dw[1] - du[1] * dw[0] )
    return dot( n, sub( p, cen ) ) / ( norm( n ) * norm( sub( p, cen ) ) )


@pytest.mark.parametrize( "lenoff", ( ( 2.0, 2.15 ), ( 2.6, 2.0 ), ( 2.0, 2.0 ), ( 2.0, -2.15 ) ) )
@pytest.mark.parametrize( "typ", TYPES )
def testASphereRaisedFarOffTheEndNeverFoldsOverItself( typ, lenoff ):
    """Raised off the end, the sphere is exposed round behind the point, and the sides gather at its pole
    instead; its surface faces outward everywhere."""
    length, offset = lenoff
    gid, cen = raised( typ, length, offset, 1.25 )
    nw = 128
    for k in range( 1, 21 ):
        for j in range( nw ):
            assert outward( gid, ENDS[0][1], cen, k, j, nw ) > 0.9, ( k, j )


@pytest.mark.parametrize( "typ", TYPES )
def testAnEndInsideTheSphereReachesOutToIt( typ ):
    """Where the sphere takes in part of the end, the sides from there run out to the sphere, not on to
    the tip, and the sphere still faces outward."""
    gid, cen = raised( typ, 2.0, 1.3, 1.25 )
    nw = 128
    u = ENDS[0][1]
    inside = 0
    for j in range( nw ):
        q = point( gid, cap_u( gid, u, 1.0 ), j / nw )
        t = point( gid, cap_u( gid, u, 0.5 ), j / nw )
        if norm( sub( q, cen ) ) < 1.25:
            inside += 1
        assert abs( norm( sub( t, cen ) ) - 1.25 ) < 2e-2 * 1.25, j
    assert inside > 0
    for k in range( 1, 21 ):
        for j in range( nw ):
            assert outward( gid, u, cen, k, j, nw ) > 0.9, ( k, j )


@pytest.mark.parametrize( "radius", ( 0.3, 0.499, 0.501, 0.8 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testASphereCenteredBehindTheEndStillLeadsToItsPole( typ, radius ):
    """With no offset and the radius past the length, the sphere's center lies behind the end, yet each side
    still runs forward from the end to the sphere and over its front to the pole."""
    gid, cen = raised( typ, 0.5, 0.0, radius )
    u = ENDS[0][1]
    axis = sub( point( gid, u, 0.0 ), cen )
    axis = tuple( c / norm( axis ) for c in axis )
    nw = 64
    for j in range( nw ):
        q = point( gid, cap_u( gid, u, 1.0 ), j / nw )
        t = point( gid, cap_u( gid, u, 0.5 ), j / nw )
        assert dot( sub( t, q ), axis ) > 0.3, j
    for k in range( 1, 21 ):
        for j in range( nw ):
            assert outward( gid, u, cen, k, j, nw ) > 0.5, ( k, j )


@pytest.mark.parametrize( "offset", ( 0.09, 0.3 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testASideNeverTouchesTheSpherePastItsPole( typ, offset ):
    """With the sphere's center just ahead of the end and a small offset, the line from some points of the
    end touches the sphere only beyond the pole; those sides run to the pole instead, and the sphere still
    faces outward."""
    gid, cen = raised( typ, 0.6, offset, 0.5 )
    u = ENDS[0][1]
    nw = 64
    for k in range( 1, 21 ):
        for j in range( nw ):
            assert outward( gid, u, cen, k, j, nw ) > 0.5, ( k, j )
