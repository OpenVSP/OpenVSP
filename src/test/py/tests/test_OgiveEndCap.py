# Ogive end caps on a Fuselage or Stack.
#
# An ogive cap runs each end in towards the point a point cap with the same length and offset closes on,
# along a circular arc, and blunts it with a sphere of CapU*Radius times the cap's length scale, or
# CapU*RadiusDim, whose front is that point, as a cone cap's is.  Each side leaves the end at
# CapU*OgiveAngle degrees, inwards for positive angles, from the end's normal for OGIVE_END_CAP and from
# the surface's own direction across the end for OGIVE_SURF_END_CAP, and meets the sphere smoothly.  At
# angle zero OGIVE_END_CAP is a tangent ogive, and OGIVE_SURF_END_CAP carries the surface on across the
# end smoothly whatever its taper.  The sides gather at the sphere's pole, where the straight extension of
# the mean line leaves it: the circular arc from the middle of the end along its normal through the
# sphere's center.
#
# Each side is the circular arc that leaves its point at its angle and touches the sphere, of the family
# holding the straight side a cone cap would have there, and keeps its angle even where steep angles make
# the sides cross.  The cone cap's tests with its sphere raised off the end hold for the ogives too.
#
# The top and bottom sides are the arcs, and the others are blended from them: in side view the sides at
# W 0 and 0.5 run midway between them into the middle of the sphere, and a circular end's rows stay
# circles.
#
# The cap's half of the end in u is the sphere next to the tip, then the ogive arc.

import math

import openvsp as vsp
import pytest

from testhelp import drop_errors, assert_no_errors
from test_ConeEndCap import raised, outward

ENDS = ( ( "UMin", 0.0 ), ( "UMax", 1.0 ) )
NW = 32
TYPES = ( "FUSELAGE", "STACK" )
OGIVES = ( vsp.OGIVE_END_CAP, vsp.OGIVE_SURF_END_CAP )


def cap_parm( gid, name ):
    return vsp.FindParm( gid, name, "EndCap" )


def point( gid, u, w ):
    p = vsp.CompPnt01( gid, 0, u, w )
    return ( p.x(), p.y(), p.z() )


def sub( a, b ):
    return tuple( x - y for x, y in zip( a, b ) )


def add( a, b ):
    return tuple( x + y for x, y in zip( a, b ) )


def scale( a, s ):
    return tuple( x * s for x in a )


def dot( a, b ):
    return sum( x * y for x, y in zip( a, b ) )


def cross( a, b ):
    return ( a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0] )


def norm( a ):
    return math.sqrt( dot( a, a ) )


def unit( a ):
    return scale( a, 1.0 / norm( a ) )


def nsect( gid ):
    """Sections of the surface capped at both ends, in u."""
    return vsp.GetNumXSec( vsp.GetXSecSurf( gid, 0 ) ) - 1 + 2


def cap_u( gid, u, f ):
    """u of the fraction f of the way from the tip of the end at u across its cap."""
    du = f / nsect( gid )
    if u > 0.5:
        return 1.0 - du
    return du


def release( xs ):
    """Let the skinning follow the sections, with no angles or strengths set at them."""
    for pid in vsp.GetXSecParmIDs( xs ):
        n = vsp.GetParmName( pid )
        if ( n.endswith( "Eq" ) or n.endswith( "Set" ) ) and ( "Angle" in n or "Strength" in n or "Slew" in n ):
            vsp.SetParmVal( pid, 0 )


def tube( typ, taper=1.0 ):
    """A body of circular sections 2 across on its axis, the nose section taper times as wide."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_CIRCLE )
        xs = vsp.GetXSec( xss, i )
        d = 2.0
        if i == 0:
            d = 2.0 * taper
        vsp.SetParmVal( vsp.GetXSecParm( xs, "Circle_Diameter" ), d )
        if typ == "STACK":
            for n in ( "YDelta", "ZDelta" ):
                vsp.SetParmVal( vsp.GetXSecParm( xs, n ), 0.0 )
        else:
            for n in ( "YLocPercent", "ZLocPercent" ):
                vsp.SetParmVal( vsp.GetXSecParm( xs, n ), 0.0 )
        release( xs )
    vsp.Update()
    return gid


def oval( typ, width, height ):
    """A body of elliptical sections width across and height high on its axis."""
    gid = tube( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_ELLIPSE )
        xs = vsp.GetXSec( xss, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "Ellipse_Width" ), width )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "Ellipse_Height" ), height )
    vsp.Update()
    return gid


def set_caps( gid, cap_type, length, offset=0.0, radius=0.0, angle=0.0 ):
    """Caps both ends with dimensional length, offset and radius."""
    for end, u in ENDS:
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Option" ), cap_type )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RelFlag" ), False )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "LengthDim" ), length )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "OffsetDim" ), offset )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RadiusDim" ), radius )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "OgiveAngle" ), angle )
    vsp.Update()


def frame( gid, u ):
    """The middle of the end at u and the unit axis from it towards that end's tip."""
    up = point( gid, cap_u( gid, u, 1.0 ), 0.25 )
    down = point( gid, cap_u( gid, u, 1.0 ), 0.75 )
    mid = scale( add( up, down ), 0.5 )
    return mid, unit( sub( point( gid, u, 0.0 ), mid ) )


def fold( gid, u, nw=96, nu=24 ):
    """The least agreement between the normals of neighbouring points over the cap at u; 1 is flat, a
    fold is negative."""
    def normal( uu, w ):
        du = sub( point( gid, uu + 1e-7, w ), point( gid, uu - 1e-7, w ) )
        dw = sub( point( gid, uu, w + 1e-7 ), point( gid, uu, w - 1e-7 ) )
        n = cross( du, dw )
        if norm( n ) < 1e-14:
            return None
        return unit( n )

    ns = [ [ normal( cap_u( gid, u, k / nu ), j / nw ) for j in range( nw + 1 ) ] for k in range( 1, nu + 1 ) ]
    least = 1.0
    for k in range( nu ):
        for j in range( nw + 1 ):
            for kk, jj in ( ( k + 1, j ), ( k, j + 1 ) ):
                if kk < nu and jj <= nw and ns[k][j] is not None and ns[kk][jj] is not None:
                    least = min( least, dot( ns[k][j], ns[kk][jj] ) )
    return least


def kink( gid, u, nw=NW ):
    """The greatest angle, in degrees, between the surface's normals on either side of the end at u."""
    uu = cap_u( gid, u, 1.0 )
    h = 1e-6

    def normal( uv, w ):
        du = sub( point( gid, uv + 1e-8, w ), point( gid, uv - 1e-8, w ) )
        dw = sub( point( gid, uv, w + 1e-8 ), point( gid, uv, w - 1e-8 ) )
        return unit( cross( du, dw ) )

    worst = 0.0
    for j in range( nw ):
        w = j / nw
        c = max( -1.0, min( 1.0, dot( normal( uu - h, w ), normal( uu + h, w ) ) ) )
        worst = max( worst, math.degrees( math.acos( c ) ) )
    return worst


@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testABodyTakesAnOgiveCap( typ, cap_type ):
    gid = tube( typ )
    set_caps( gid, cap_type, 3.0 )
    for end, u in ENDS:
        assert vsp.GetParmVal( cap_parm( gid, "Cap" + end + "Option" ) ) == cap_type
    assert_no_errors()


@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", ( "WING", "PROP", "BODYOFREVOLUTION" ) )
def testOtherGeomsRefuseAnOgiveCap( typ, cap_type ):
    vsp.VSPRenew()
    gid = vsp.AddGeom( typ )
    for end, u in ENDS:
        parm = cap_parm( gid, "Cap" + end + "Option" )
        before = vsp.GetParmVal( parm )
        vsp.SetParmVal( parm, cap_type )
        assert vsp.GetParmVal( parm ) == before, end


@pytest.mark.parametrize( "angle", ( 0.0, 5.0, -8.0 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testAPointedOgiveIsTheArcToThePoint( typ, cap_type, angle ):
    """On a cylinder of radius 1, a side leaving the end at angle t inwards and reaching the point at
    length L is a circle of radius ( L^2 + 1 ) / ( 2 ( cos t - L sin t ) ), centered square to the side's
    start on the axis's side.  At angle zero it is the tangent ogive, of radius ( L^2 + 1 ) / 2."""
    length = 3.0
    gid = tube( typ )
    set_caps( gid, cap_type, length, angle=angle )
    t = math.radians( angle )
    rho = ( length * length + 1.0 ) / ( 2.0 * ( math.cos( t ) - length * math.sin( t ) ) )

    for end, u in ENDS:
        mid, axis = frame( gid, u )
        for k in range( 21 ):
            for j in range( NW ):
                p = sub( point( gid, cap_u( gid, u, k / 20.0 ), j / NW ), mid )
                a = dot( p, axis )
                r = norm( sub( p, scale( axis, a ) ) )
                d = math.hypot( a + rho * math.sin( t ), r - ( 1.0 - rho * math.cos( t ) ) )
                assert abs( d - rho ) < 1e-3 * rho, ( end, k, j )


@pytest.mark.parametrize( "angle", ( 0.0, 8.0, -10.0 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testABluntOgiveMeetsItsSphereOnTheArc( typ, cap_type, angle ):
    """With a sphere of radius b a radius back from the point, the side is the circle leaving the end at
    angle t that touches the sphere: in the side's plane, with q its start, c the sphere's center and n
    square to the side towards the axis, of radius ( b^2 - |q - c|^2 ) / ( 2 ( b + n . ( q - c ) ) )."""
    length, b = 3.0, 0.3
    gid = tube( typ )
    set_caps( gid, cap_type, length, radius=b, angle=angle )
    t = math.radians( angle )
    q = ( 0.0, 1.0 )
    c = ( length - b, 0.0 )
    n = ( -math.sin( t ), -math.cos( t ) )
    qc = sub( q, c )
    rho = ( b * b - dot( qc, qc ) ) / ( 2.0 * ( b + dot( n, qc ) ) )
    o = add( q, scale( n, rho ) )

    for end, u in ENDS:
        mid, axis = frame( gid, u )
        for k in range( 21 ):
            for j in range( NW ):
                p = sub( point( gid, cap_u( gid, u, k / 20.0 ), j / NW ), mid )
                a = dot( p, axis )
                pr = ( a, norm( sub( p, scale( axis, a ) ) ) )
                if k <= 10:
                    assert abs( norm( sub( pr, c ) ) - b ) < 2e-4 * b, ( end, k, j )
                else:
                    assert abs( norm( sub( pr, o ) ) - abs( rho ) ) < 1e-3 * abs( rho ), ( end, k, j )


@pytest.mark.parametrize( "radius", ( 0.0, 0.3 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testAtTheConesHalfAngleAnOgiveIsTheCone( typ, radius ):
    """Leaving the end along the cone cap's side, the arc never turns, and the cap is the cone cap."""
    length = 3.0
    gid = tube( typ )
    set_caps( gid, vsp.CONE_END_CAP, length, radius=radius )
    q = point( gid, cap_u( gid, 0.0, 1.0 ), 0.0 )
    tq = point( gid, cap_u( gid, 0.0, 0.5 ), 0.0 )
    mid, axis = frame( gid, 0.0 )
    side = unit( sub( tq, q ) )
    half = math.degrees( math.atan2( norm( sub( side, scale( axis, dot( side, axis ) ) ) ), dot( side, axis ) ) )
    vsp.CopyGeomToClipboard( gid )
    cid = vsp.PasteGeomClipboard()[0]

    set_caps( gid, vsp.OGIVE_END_CAP, length, radius=radius, angle=half )
    for i in range( 21 ):
        for j in range( NW + 1 ):
            d, u, w = vsp.ProjPnt01( cid, 0, vsp.vec3d( *point( gid, cap_u( gid, 0.0, i / 20.0 ), j / NW ) ) )
            assert d < 1e-4, ( i, j )


@pytest.mark.parametrize( "offset", ( 0.0, 0.3, -0.5 ) )
@pytest.mark.parametrize( "radius", ( 0.0, 0.3 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testATangentOgiveLeavesACylinderSmoothly( typ, cap_type, radius, offset ):
    gid = tube( typ )
    set_caps( gid, cap_type, 3.0, offset, radius )
    for end, u in ENDS:
        assert kink( gid, u ) < 0.05, end


@pytest.mark.parametrize( "offset", ( 0.0, 0.2 ) )
@pytest.mark.parametrize( "radius", ( 0.0, 0.3 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testASurfaceOgiveLeavesATaperSmoothly( typ, radius, offset ):
    """The nose of a body that narrows towards it: the surface ogive carries the taper on into the cap,
    where an ogive square to the end turns by the taper's angle."""
    gid = tube( typ, 0.7 )
    set_caps( gid, vsp.OGIVE_SURF_END_CAP, 3.0, offset, radius )
    assert kink( gid, 0.0 ) < 0.05

    # The surface's greatest angle to the end's normal.
    uu = cap_u( gid, 0.0, 1.0 )
    normal = unit( cross( sub( point( gid, uu, 0.25 ), point( gid, uu, 0.75 ) ),
                          sub( point( gid, uu, 0.0 ), point( gid, uu, 0.5 ) ) ) )
    taper = 0.0
    for j in range( NW ):
        across = unit( sub( point( gid, uu, j / NW ), point( gid, uu + 1e-6, j / NW ) ) )
        taper = max( taper, math.degrees( math.acos( abs( dot( across, normal ) ) ) ) )
    assert taper > 1.0
    set_caps( gid, vsp.OGIVE_END_CAP, 3.0, offset, radius )
    assert kink( gid, 0.0 ) == pytest.approx( taper, abs=0.1 )


@pytest.mark.parametrize( "offset", ( 0.0, 0.3, -0.5, 2.0 ) )
@pytest.mark.parametrize( "radius", ( 0.0, 0.3 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testTheTipIsWhereTheMeanLineLeavesTheSphere( typ, radius, offset ):
    """The sides gather at the pole, where the mean line leaves the sphere: the circular arc from the
    middle of the end along its normal through the sphere's center, then straight on.  With no offset or
    no radius that is the point a point cap closes on."""
    gid = tube( typ )
    set_caps( gid, vsp.POINT_END_CAP, 3.0, offset )
    points = [ point( gid, u, 0.0 ) for end, u in ENDS ]
    set_caps( gid, vsp.OGIVE_END_CAP, 3.0, offset, radius, 6.0 )
    for ( end, u ), pnt in zip( ENDS, points ):
        tip = point( gid, u, 0.0 )
        for j in range( NW + 1 ):
            assert norm( sub( point( gid, u, j / NW ), tip ) ) < 1e-9, ( end, j )
        mid = scale( add( point( gid, cap_u( gid, u, 1.0 ), 0.25 ), point( gid, cap_u( gid, u, 1.0 ), 0.75 ) ), 0.5 )
        if offset == 0.0 or radius == 0.0:
            assert norm( sub( tip, pnt ) ) < 1e-9, end
            continue
        # The sphere's center is a radius back from the point along the end's normal.
        nrm = unit( sub( point( gid, cap_u( gid, u, 1.0 ), 0.0 ), point( gid, cap_u( gid, u, 1.0 + 1e-6 ), 0.0 ) ) )
        cen = sub( pnt, scale( nrm, radius ) )
        # The arc leaves mid along nrm and reaches cen along nrm reflected in its chord.
        c = unit( sub( cen, mid ) )
        pdir = sub( scale( c, 2.0 * dot( c, nrm ) ), nrm )
        assert norm( sub( tip, add( cen, scale( pdir, radius ) ) ) ) < 1e-9, end


@pytest.mark.parametrize( "angle", ( 0.0, 6.0, -8.0 ) )
@pytest.mark.parametrize( "offset", ( 0.0, 0.6, 1.0, -0.8 ) )
@pytest.mark.parametrize( "radius", ( 0.0, 0.3, 0.8 ) )
@pytest.mark.parametrize( "taper", ( 1.0, 0.7 ) )
def testAnOgiveCapNeverFolds( taper, radius, offset, angle ):
    gid = tube( "STACK", taper )
    set_caps( gid, vsp.OGIVE_END_CAP, 3.0, offset, radius, angle )
    assert fold( gid, 0.0 ) > 0.5


@pytest.mark.parametrize( "angle", ( 0.0, 6.0, -8.0 ) )
@pytest.mark.parametrize( "offset", ( 0.0, 0.1 ) )
@pytest.mark.parametrize( "radius", ( 0.0, 0.3, 0.8 ) )
@pytest.mark.parametrize( "taper", ( 1.0, 0.7 ) )
def testASurfaceOgiveCapNeverFolds( taper, radius, offset, angle ):
    gid = tube( "STACK", taper )
    set_caps( gid, vsp.OGIVE_SURF_END_CAP, 3.0, offset, radius, angle )
    assert fold( gid, 0.0 ) > 0.5


@pytest.mark.parametrize( "angle", ( 0.0, 6.0, -8.0 ) )
@pytest.mark.parametrize( "offset", ( 0.6, 1.0, -0.8 ) )
@pytest.mark.parametrize( "radius", ( 0.3, 0.8 ) )
@pytest.mark.parametrize( "taper", ( 1.0, 0.7 ) )
def testABluntSurfaceOgiveCapNeverFolds( taper, radius, offset, angle ):
    """A sphere lets the sides of a surface ogive reach the pole from far off the end's middle."""
    gid = tube( "STACK", taper )
    set_caps( gid, vsp.OGIVE_SURF_END_CAP, 3.0, offset, radius, angle )
    assert fold( gid, 0.0 ) > 0.5


def ogive_fractions( radius, n=20 ):
    """Fractions of the way from the tip across the cap that are on the ogive arc, not the sphere."""
    lo = 0.0
    if radius > 0.0:
        lo = 0.5
    return [ lo + ( 1.0 - lo ) * k / n for k in range( n + 1 ) ]


@pytest.mark.parametrize( "size", ( ( 2.0, 2.0, 0.0 ), ( 2.0, 2.0, 0.3 ), ( 3.0, 2.0, 0.0 ), ( 2.0, 3.0, 0.0 ) ) )
@pytest.mark.parametrize( "offset", ( 0.6, 1.0, -0.8 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testTheSidesRunMidwayBetweenTopAndBottom( typ, cap_type, offset, size ):
    """Seen from the side, the sides at W 0 and 0.5 run along the middle of the top and bottom: of a
    circular end, and of any end with no sphere."""
    width, height, radius = size
    gid = oval( typ, width, height )
    set_caps( gid, cap_type, 3.0, offset, radius, 6.0 )
    for end, u in ENDS:
        mid, axis = frame( gid, u )
        updown = sub( point( gid, cap_u( gid, u, 1.0 ), 0.75 ), point( gid, cap_u( gid, u, 1.0 ), 0.25 ) )
        lateral = unit( cross( axis, updown ) )
        for f in ogive_fractions( radius ):
            uu = cap_u( gid, u, f )
            m = scale( add( point( gid, uu, 0.25 ), point( gid, uu, 0.75 ) ), 0.5 )
            for w in ( 0.0, 0.5 ):
                d = sub( point( gid, uu, w ), m )
                assert norm( sub( d, scale( lateral, dot( d, lateral ) ) ) ) < 1e-9, ( end, f, w )


def side_view_gap( p, curve ):
    """Distance in side view, x and z, from point p to the polyline through the points curve."""
    best = math.inf
    for a, b in zip( curve[ :-1 ], curve[ 1: ] ):
        ab = ( b[0] - a[0], b[2] - a[2] )
        ap = ( p[0] - a[0], p[2] - a[2] )
        t = 0.0
        ll = ab[0] * ab[0] + ab[1] * ab[1]
        if ll > 0.0:
            t = max( 0.0, min( 1.0, ( ap[0] * ab[0] + ap[1] * ab[1] ) / ll ) )
        best = min( best, math.hypot( ap[0] - t * ab[0], ap[1] - t * ab[1] ) )
    return best


@pytest.mark.parametrize( "size", ( ( 3.0, 2.0 ), ( 2.0, 3.0 ) ) )
@pytest.mark.parametrize( "offset", ( 0.0, 0.6, 1.0, -0.8 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testAnEllipticalEndsSidesRunCloseToTheMiddle( typ, cap_type, offset, size ):
    """With a sphere an elliptical end's sides meet it nearer to or further from the pole than the top and
    bottom, so at the same u they are not midway between them, but seen from the side they stay close to
    the middle's curve."""
    gid = oval( typ, *size )
    set_caps( gid, cap_type, 3.0, offset, 0.3, 6.0 )
    for end, u in ENDS:
        middle = []
        for k in range( 401 ):
            uu = cap_u( gid, u, k / 400.0 )
            middle.append( scale( add( point( gid, uu, 0.25 ), point( gid, uu, 0.75 ) ), 0.5 ) )
        for k in range( 1, 40 ):
            for w in ( 0.0, 0.5 ):
                p = point( gid, cap_u( gid, u, k / 40.0 ), w )
                assert side_view_gap( p, middle ) < 0.025 * size[1], ( end, k, w )


@pytest.mark.parametrize( "angle", ( 0.0, 8.0 ) )
@pytest.mark.parametrize( "offset", ( 0.6, 1.0, -0.8 ) )
@pytest.mark.parametrize( "radius", ( 0.0, 0.3 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testACircularEndsRowsStayCircles( typ, cap_type, radius, offset, angle ):
    """Along the ogive arc each row of a circular end is a circle about the middle of its top and bottom."""
    gid = tube( typ )
    set_caps( gid, cap_type, 3.0, offset, radius, angle )
    for end, u in ENDS:
        for f in ogive_fractions( radius )[ 1:-1 ]:
            uu = cap_u( gid, u, f )
            top = point( gid, uu, 0.75 )
            m = scale( add( point( gid, uu, 0.25 ), top ), 0.5 )
            r = norm( sub( top, m ) )
            for j in range( NW ):
                assert abs( norm( sub( point( gid, uu, j / NW ), m ) ) - r ) < 1e-2 * r, ( end, f, j )


@pytest.mark.parametrize( "size", ( ( 3.0, 2.0 ), ( 2.0, 3.0 ) ) )
@pytest.mark.parametrize( "offset", ( 0.0, 0.6, 1.0, -0.8 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testAnEllipticalEndMeetsTheSphere( typ, cap_type, offset, size ):
    """The sphere's center is its radius back along the axis from the point a point cap closes on."""
    b = 0.3
    gid = oval( typ, *size )
    set_caps( gid, vsp.POINT_END_CAP, 3.0 )
    axes = [ frame( gid, u )[1] for end, u in ENDS ]
    set_caps( gid, vsp.POINT_END_CAP, 3.0, offset )
    points = [ point( gid, u, 0.0 ) for end, u in ENDS ]
    set_caps( gid, cap_type, 3.0, offset, b, 6.0 )
    for ( end, u ), axis, pnt in zip( ENDS, axes, points ):
        c = sub( pnt, scale( axis, b ) )
        for k in range( 11 ):
            for j in range( NW ):
                p = point( gid, cap_u( gid, u, k / 20.0 ), j / NW )
                assert abs( norm( sub( p, c ) ) - b ) < 2e-3 * b, ( end, k, j )


@pytest.mark.parametrize( "size", ( ( 2.0, 2.0 ), ( 3.0, 2.0 ), ( 2.0, 3.0 ) ) )
@pytest.mark.parametrize( "offset", ( 0.0, 0.6, 1.0, -0.8 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testEachSideRunsSmoothlyOntoTheSphere( typ, cap_type, offset, size ):
    """Not only the surface: each line of constant W keeps its direction where it meets the sphere."""
    gid = oval( typ, *size )
    set_caps( gid, cap_type, 3.0, offset, 0.3, 6.0 )
    for end, u in ENDS:
        uu = cap_u( gid, u, 0.5 )
        h = 1e-4
        for j in range( NW ):
            w = j / NW
            a = unit( sub( point( gid, uu, w ), point( gid, uu - h, w ) ) )
            c = unit( sub( point( gid, uu + h, w ), point( gid, uu, w ) ) )
            assert math.degrees( math.acos( min( 1.0, dot( a, c ) ) ) ) < 0.5, ( end, j )


@pytest.mark.parametrize( "size", ( ( 3.0, 2.0 ), ( 2.0, 3.0 ) ) )
@pytest.mark.parametrize( "offset", ( 0.0, 0.6, -0.8 ) )
@pytest.mark.parametrize( "radius", ( 0.0, 0.3 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testATangentOgiveLeavesAnEllipticalCylinderSmoothly( typ, cap_type, radius, offset, size ):
    gid = oval( typ, *size )
    set_caps( gid, cap_type, 3.0, offset, radius )
    for end, u in ENDS:
        assert kink( gid, u ) < 0.05, end


@pytest.mark.parametrize( "angle", ( 0.0, 8.0, -8.0 ) )
@pytest.mark.parametrize( "offset", ( 0.0, 0.6, 1.0, -0.8 ) )
@pytest.mark.parametrize( "radius", ( 0.0, 0.3 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "size", ( ( 3.0, 2.0 ), ( 2.0, 3.0 ) ) )
def testAnEllipticalOgiveCapNeverFolds( size, cap_type, radius, offset, angle ):
    gid = oval( "STACK", *size )
    set_caps( gid, cap_type, 3.0, offset, radius, angle )
    assert fold( gid, 0.0 ) > 0.5


@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testTheRadiusHasTheLengthScale( typ, cap_type ):
    gid = tube( typ )
    set_caps( gid, cap_type, 3.0 )
    for end, u in ENDS:
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RelFlag" ), True )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Radius" ), 0.1 )
    vsp.Update()
    for end, u in ENDS:
        lscale = vsp.GetParmVal( cap_parm( gid, "Cap" + end + "LengthDim" ) ) / \
            vsp.GetParmVal( cap_parm( gid, "Cap" + end + "Length" ) )
        assert vsp.GetParmVal( cap_parm( gid, "Cap" + end + "RadiusDim" ) ) == pytest.approx( 0.1 * lscale, rel=1e-12 )


@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testAnOgiveCapIsSavedAndRead( typ, cap_type, tmp_path ):
    gid = tube( typ, 0.8 )
    set_caps( gid, cap_type, 2.5, 0.1, 0.3, 4.0 )
    before = [ point( gid, i / 20.0, j / NW ) for i in range( 21 ) for j in range( NW + 1 ) ]
    fname = str( tmp_path / "ogive.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    gid = vsp.FindGeoms()[0]
    for end, u in ENDS:
        assert vsp.GetParmVal( cap_parm( gid, "Cap" + end + "Option" ) ) == cap_type
        assert vsp.GetParmVal( cap_parm( gid, "Cap" + end + "OgiveAngle" ) ) == 4.0
    after = [ point( gid, i / 20.0, j / NW ) for i in range( 21 ) for j in range( NW + 1 ) ]
    assert max( norm( sub( a, b ) ) for a, b in zip( before, after ) ) < 1e-12


@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testAnOgiveCapTessellates( typ, cap_type ):
    gid = tube( typ, 0.8 )
    set_caps( gid, cap_type, 2.5, 0.1, 0.3, 4.0 )
    mesh = vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    assert mesh
    assert_no_errors()


@pytest.mark.parametrize( "typ", TYPES )
def testACloneOfABodyTakesItsOgiveCap( typ ):
    gid = tube( typ, 0.8 )
    set_caps( gid, vsp.OGIVE_SURF_END_CAP, 2.5, 0.1, 0.3, 4.0 )
    clone = vsp.AddGeom( "CLONE", gid )
    vsp.Update()
    for end, u in ENDS:
        for name in ( "Option", "Radius", "OgiveAngle" ):
            assert vsp.GetParmVal( cap_parm( clone, "Cap" + end + name ) ) == \
                vsp.GetParmVal( cap_parm( gid, "Cap" + end + name ) ), ( end, name )
    assert_no_errors()


@pytest.mark.parametrize( "typ", TYPES )
def testAConformalOnABodyTakesItsOgiveCap( typ ):
    gid = tube( typ, 0.8 )
    set_caps( gid, vsp.OGIVE_SURF_END_CAP, 2.5, 0.1, 0.3, 4.0 )
    conf = vsp.AddGeom( "CONFORMAL", gid )
    vsp.Update()
    for end, u in ENDS:
        for name in ( "Option", "Radius", "OgiveAngle" ):
            assert vsp.GetParmVal( cap_parm( conf, "Cap" + end + name ) ) == \
                vsp.GetParmVal( cap_parm( gid, "Cap" + end + name ) ), ( end, name )
    assert_no_errors()


@pytest.mark.parametrize( "lenoffrad", ( ( 1.7425, 4.0625, 1.036 ), ( 1.7425, 3.5, 1.036 ), ( 8.93, 4.076, 0.2858 ),
                                         ( 2.0, 2.15, 1.25 ), ( 2.0, -2.15, 1.25 ), ( 2.0, 2.0, 1.25 ) ) )
@pytest.mark.parametrize( "angle", ( -25.0, 0.0, 10.0 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testWithTheSphereFarOffTheSidesMeetItsMiddle( typ, cap_type, angle, lenoffrad ):
    """With the sphere far off the end, even touching it, the sides at W 0 and 0.5 of a circular end meet
    the sphere out at its sides, at least half its radius to the side of its center, not where the top
    or bottom side does."""
    length, offset, radius = lenoffrad
    gid, cen = raised( typ, length, offset, radius, cap_type, angle, 2.5 )
    u = ENDS[0][1]
    lateral = unit( sub( point( gid, cap_u( gid, u, 1.0 ), 0.0 ), point( gid, cap_u( gid, u, 1.0 ), 0.5 ) ) )
    for w, sign in ( ( 0.0, 1.0 ), ( 0.5, -1.0 ) ):
        ptan = point( gid, cap_u( gid, u, 0.5 ), w )
        assert sign * dot( sub( ptan, cen ), lateral ) > 0.5 * radius, w


@pytest.mark.parametrize( "lenoff", ( ( 1.7425, 4.0625 ), ( 1.7425, 3.5 ), ( 1.25, 4.0625 ), ( 2.25, 4.375 ),
                                      ( 1.7425, 3.0 ) ) )
@pytest.mark.parametrize( "angle", ( 0.0, 10.0 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testASideCloseToTheLineThroughThePoleLeavesAtItsAngle( typ, angle, lenoff ):
    """An offset far above the end runs the line through the pole close past the top of the end.  The top
    side's arc crosses that line on its way to the sphere, and every side still leaves the end at its
    angle from the end's normal."""
    length, offset = lenoff
    gid, cen = raised( typ, length, offset, 1.036, vsp.OGIVE_END_CAP, angle, 2.5 )
    u = ENDS[0][1]
    qs = [ point( gid, cap_u( gid, u, 1.0 ), j / NW ) for j in range( NW ) ]
    mid = scale( add( qs[0], qs[NW // 2] ), 0.5 )
    nrm = unit( cross( sub( qs[NW // 4], qs[3 * NW // 4] ), sub( qs[0], qs[NW // 2] ) ) )
    if dot( sub( point( gid, cap_u( gid, u, 1.0 + 1e-3 ), 0.0 ), qs[0] ), nrm ) > 0.0:
        nrm = scale( nrm, -1.0 )
    for j in range( NW ):
        q = qs[j]
        t = unit( sub( point( gid, cap_u( gid, u, 1.0 - 1e-5 ), j / NW ), q ) )
        inward = unit( sub( mid, q ) )
        a = math.degrees( math.atan2( dot( t, inward ), dot( t, nrm ) ) )
        assert abs( a - angle ) < 0.05, j


@pytest.mark.parametrize( "offset", ( 1.02, 1.4, -1.1 ) )
@pytest.mark.parametrize( "angle", ( -10.0, 0.0, 10.0 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testASideBulgingOutTouchesTheSphereOnItsOutwardSide( typ, cap_type, angle, offset ):
    """As a cone cap's side does, a side that leaves the end no further inwards than the cone touches the
    sphere on its outward side, away from the middle of the end, even with the sphere raised past part of
    the end."""
    nw = 128
    u = ENDS[0][1]

    def outwards( gid, j, f ):
        """How far outwards, away from the middle of the end, the cap's side at j stands at f of the way
        from its tangent point to the end, across the line from its point of the end to the tip."""
        q = point( gid, cap_u( gid, u, 1.0 ), j / nw )
        qt = unit( sub( point( gid, cap_u( gid, u, f ), j / nw ), q ) )
        qtip = sub( point( gid, u, 0.0 ), q )
        across = sub( qt, scale( qtip, dot( qt, qtip ) / dot( qtip, qtip ) ) )
        qs = [ point( gid, cap_u( gid, u, 1.0 ), k / nw ) for k in range( nw ) ]
        mid = tuple( sum( c ) / nw for c in zip( *qs ) )
        return dot( across, unit( sub( q, mid ) ) )

    gid, cen = raised( typ, 2.0, offset, 0.25, vsp.CONE_END_CAP )
    cone = [ outwards( gid, j, 0.5 ) for j in range( nw ) ]
    gid, cen = raised( typ, 2.0, offset, 0.25, cap_type, angle )
    for j in range( nw ):
        if outwards( gid, j, 1.0 - 1e-5 ) >= cone[j]:
            assert outwards( gid, j, 0.5 ) > 0.0, j


@pytest.mark.parametrize( "lenoff", ( ( 2.0, 2.15 ), ( 2.6, 2.0 ), ( 2.0, -2.15 ), ( 2.3, 2.0 ) ) )
@pytest.mark.parametrize( "angle", ( -10.0, 0.0, 10.0, 25.0 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testASphereRaisedFarOffTheEndNeverFoldsOverItself( typ, cap_type, angle, lenoff ):
    """As a cone cap's does, raised off the end the sphere is exposed round behind the point, and the sides
    gather at its pole; the cap faces outward everywhere."""
    length, offset = lenoff
    gid, cen = raised( typ, length, offset, 1.25, cap_type, angle )
    nw = 128
    for k in range( 1, 21 ):
        for j in range( nw ):
            assert outward( gid, ENDS[0][1], cen, k, j, nw ) > 0.9, ( k, j )


@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testAnEndTouchingTheSphereNeverFolds( typ, cap_type ):
    """The bottom of the end lies on the sphere, so the bottom side has no room for an arc, and its
    neighbours little; the cap still faces outward everywhere."""
    gid, cen = raised( typ, 2.0, 2.0, 1.25, cap_type, 0.0 )
    nw = 128
    for k in range( 1, 21 ):
        for j in range( nw ):
            assert outward( gid, ENDS[0][1], cen, k, j, nw ) > 0.9, ( k, j )


@pytest.mark.parametrize( "angle", ( -10.0, 0.0, 10.0, 25.0 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testAnEndInsideTheSphereReachesOutToIt( typ, cap_type, angle ):
    """As a cone cap's do, where the sphere takes in part of the end the sides from there run out to it,
    and the cap faces outward everywhere."""
    gid, cen = raised( typ, 2.0, 1.3, 1.25, cap_type, angle )
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


@pytest.mark.parametrize( "radius", ( 0.25, 0.05 ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testATangentOgiveAsLongAsTheEndIsWideKeepsItsSphere( typ, cap_type, radius ):
    """With the default length, a tangent ogive is as long as the end is wide, the hemisphere, and meets its
    sphere at the pole.  The split from the sphere's patch moves back along the arc, so that patch keeps a
    size, and the cap never folds."""
    gid = tube( typ )
    for end, u in ENDS:
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Option" ), cap_type )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Length" ), 1.0 )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Radius" ), radius )
    vsp.Update()
    b = vsp.GetParmVal( cap_parm( gid, "CapUMinRadiusDim" ) )
    for end, u in ENDS:
        assert fold( gid, u ) > 0.5, end
        pole = point( gid, u, 0.0 )
        for j in range( NW ):
            assert norm( sub( point( gid, cap_u( gid, u, 0.5 ), j / NW ), pole ) ) > b * math.pi / 64.0, ( end, j )


def reach( gid, u ):
    """The furthest the cap at u reaches from the middle of its end."""
    mid = scale( add( point( gid, cap_u( gid, u, 1.0 ), 0.25 ), point( gid, cap_u( gid, u, 1.0 ), 0.75 ) ), 0.5 )
    return max( norm( sub( point( gid, cap_u( gid, u, k / 40.0 ), j / 64.0 ), mid ) )
                for k in range( 41 ) for j in range( 64 ) )


@pytest.mark.parametrize( "angle", ( 20.0, 31.0, 45.0, 80.0 ) )
@pytest.mark.parametrize( "radius", ( 0.0, 0.1, 0.3 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testASteepInwardAngleIsKept( typ, radius, angle ):
    """Leaving a thin end steeply inwards, the arcs that meet the angle cross the axis, and the top and
    bottom sides cross each other; each side still leaves at its angle, and the cap stays near the end."""
    gid = tube( typ, 0.4 )
    set_caps( gid, vsp.OGIVE_END_CAP, 3.0, 0.0, radius, angle )
    u = ENDS[0][1]
    qs = [ point( gid, cap_u( gid, u, 1.0 ), j / NW ) for j in range( NW ) ]
    mid = scale( add( qs[0], qs[NW // 2] ), 0.5 )
    nrm = unit( cross( sub( qs[NW // 4], qs[3 * NW // 4] ), sub( qs[0], qs[NW // 2] ) ) )
    if dot( sub( point( gid, cap_u( gid, u, 1.0 + 1e-3 ), 0.0 ), qs[0] ), nrm ) > 0.0:
        nrm = scale( nrm, -1.0 )
    for j in range( NW ):
        t = unit( sub( point( gid, cap_u( gid, u, 1.0 - 1e-5 ), j / NW ), qs[j] ) )
        inward = unit( sub( mid, qs[j] ) )
        assert math.degrees( math.atan2( dot( t, inward ), dot( t, nrm ) ) ) == pytest.approx( angle, abs=0.05 ), j
    assert reach( gid, u ) < 2.0 * ( 0.4 + 3.0 + radius )


@pytest.mark.parametrize( "offset", ( 0.0, 0.3, 0.6 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testASurfaceOgiveOnAStronglyNarrowingEndLeavesAlongIt( typ, offset ):
    """A surface ogive leaving an end that narrows at 60 degrees carries the surface on into the cap,
    though its sides then cross each other, and the cap stays near the end."""
    gid = tube( typ, 0.4 )
    set_caps( gid, vsp.OGIVE_SURF_END_CAP, 3.0, offset, 0.3, 0.0 )
    assert kink( gid, 0.0 ) < 0.05
    assert reach( gid, 0.0 ) < 2.0 * ( 0.4 + 3.0 + offset + 0.3 )


@pytest.mark.parametrize( "cap_type", OGIVES )
@pytest.mark.parametrize( "typ", TYPES )
def testTheCapIsClosedAtACornerOfTheEnd( typ, cap_type ):
    """Where segments of the end meet at a corner, as on a rectangle, the sides from the corner on either
    side of it are the same, at any angle, so the cap is closed along them."""
    gid = tube( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_ROUNDED_RECTANGLE )
        xs = vsp.GetXSec( xss, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundedRect_Width" ), 3.0 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundedRect_Height" ), 1.5 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundRectXSec_Radius" ), 0.0 )
    vsp.Update()
    set_caps( gid, cap_type, 3.0, 0.5, 0.3, 10.0 )
    for k in range( 1, 40 ):
        uu = cap_u( gid, 0.0, k / 40.0 )
        for j in range( 1, 64 ):
            w = j / 64.0
            assert norm( sub( point( gid, uu, w - 1e-10 ), point( gid, uu, w + 1e-10 ) ) ) < 1e-6, ( k, j )
