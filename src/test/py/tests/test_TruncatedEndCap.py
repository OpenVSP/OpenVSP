# Truncated end caps on a Fuselage or Stack.
#
# With CapU*TruncFlag set, a cone, ogive or profile cap ends at a flat face at its point in place of the point
# or the sphere: the end scaled about its middle and moved to the point, CapU*Radius in radius on a circular
# end.  A cone's sides run straight from the end to the face.  An ogive's are the circular arcs that leave
# the end at its angle and run through the face, which cut the ogive through the face's edge as it would run
# on to a point.  A profile's are its profile cut where its radius is the face's, the profile running on to a
# point beyond the face.  On any other cap, or with no radius, the flag changes nothing.

import math

import openvsp as vsp
import pytest

from testhelp import assert_no_errors
from test_OgiveEndCap import ENDS, NW, TYPES, cap_parm, point, sub, add, scale, dot, cross, norm, unit, cap_u, \
    tube, oval, set_caps, frame
from test_OgiveContinuity import grid, volume, spikes
from test_ProfileEndCap import profile, set_exponent, set_haack, gap

LENGTH = 3.0
FACE = 0.3

TRUNCATED = ( vsp.CONE_END_CAP, vsp.OGIVE_END_CAP, vsp.OGIVE_SURF_END_CAP, vsp.POWER_LAW_END_CAP,
              vsp.VON_KARMAN_END_CAP, vsp.SEARS_HAACK_END_CAP )


def set_trunc( gid, flag ):
    for end, u in ENDS:
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "TruncFlag" ), flag )
    vsp.Update()


def cut( fn, face, length ):
    """The radius at distance d ahead of a unit end of the profile fn, a fraction of the radius at s, cut
    where it is face, the cut length ahead of the end."""
    lo = 0.0
    hi = 1.0
    for i in range( 80 ):
        mid = 0.5 * ( lo + hi )
        if fn( mid ) < face:
            lo = mid
        else:
            hi = mid
    full = length / ( 1.0 - 0.5 * ( lo + hi ) )
    return lambda d: fn( ( full - d ) / full )


def arc( angle, face, length ):
    """The radius at distance d ahead of a unit end of the circle that leaves it at angle degrees towards the
    axis and runs through the face's edge, length ahead."""
    a = math.radians( angle )
    dx = math.cos( a )
    dr = -math.sin( a )
    cx = length
    cr = face - 1.0
    nx = -dr
    nr = dx
    cn = cx * nx + cr * nr
    if cn < 0.0:
        nx = -nx
        nr = -nr
        cn = -cn
    rho = ( cx * cx + cr * cr ) / ( 2.0 * cn )
    ox = rho * nx
    orr = 1.0 + rho * nr
    return lambda d: orr + math.sqrt( max( rho * rho - ( d - ox ) ** 2, 0.0 ) )


def exact( cap_type, angle, exponent, c ):
    if cap_type == vsp.CONE_END_CAP:
        return lambda d: 1.0 + ( FACE - 1.0 ) * d / LENGTH
    if cap_type in ( vsp.OGIVE_END_CAP, vsp.OGIVE_SURF_END_CAP ):
        return arc( angle, FACE, LENGTH )
    return cut( profile( cap_type, exponent, c ), FACE, LENGTH )


def polyline( fn, length, n=2000 ):
    """The radius fn from the end to the face as a polyline of ( d, r ), d the distance ahead of the end,
    taken at even steps of both, so steep and flat stretches alike are close."""
    ds = [ length * k / n for k in range( n + 1 ) ]
    r0 = fn( 0.0 )
    r1 = fn( length )
    for k in range( 1, n ):
        r = r0 + ( r1 - r0 ) * k / n
        lo = 0.0
        hi = length
        for i in range( 60 ):
            mid = 0.5 * ( lo + hi )
            if fn( mid ) > r:
                lo = mid
            else:
                hi = mid
        ds.append( 0.5 * ( lo + hi ) )
    ds.sort()
    return [ ( d, fn( d ) ) for d in ds ]


def all_points( gid ):
    return [ point( gid, i / 40.0, j / NW ) for i in range( 41 ) for j in range( NW + 1 ) ]


CASES = [ ( vsp.CONE_END_CAP, 0.0, 0.5, 0.0 ),
          ( vsp.OGIVE_END_CAP, 0.0, 0.5, 0.0 ),
          ( vsp.OGIVE_END_CAP, 5.0, 0.5, 0.0 ),
          ( vsp.OGIVE_SURF_END_CAP, -5.0, 0.5, 0.0 ) ] + \
        [ ( vsp.POWER_LAW_END_CAP, 0.0, e, 0.0 ) for e in ( 0.05, 0.3, 0.5, 0.75, 1.0, 2.0, 5.0, 20.0 ) ] + \
        [ ( vsp.VON_KARMAN_END_CAP, 0.0, 0.5, c ) for c in ( 0.0, 1.0 / 3.0 ) ] + \
        [ ( vsp.SEARS_HAACK_END_CAP, 0.0, 0.5, 0.0 ) ]


@pytest.mark.parametrize( "cap_type, angle, exponent, c", CASES )
@pytest.mark.parametrize( "typ", TYPES )
def testATruncatedCapFollowsItsCutProfile( typ, cap_type, angle, exponent, c ):
    """On a circular end the cap comes within a thousandth of the end's radius of its cut profile, all the
    way from the end to the face."""
    gid = tube( typ )
    set_exponent( gid, exponent )
    set_haack( gid, c )
    set_trunc( gid, True )
    set_caps( gid, cap_type, LENGTH, 0.0, FACE, angle )
    line = polyline( exact( cap_type, angle, exponent, c ), LENGTH )
    for end, u in ENDS:
        mid, axis = frame( gid, u )
        for k in range( 401 ):
            for w in ( 0.0, 0.125, 0.25 ):
                d = sub( point( gid, cap_u( gid, u, k / 400.0 ), w ), mid )
                a = dot( d, axis )
                if a > LENGTH - 1e-9:
                    continue
                assert gap( line, ( a, norm( sub( d, scale( axis, a ) ) ) ) ) < 1e-3, ( end, k, w )
    assert_no_errors()


@pytest.mark.parametrize( "cap_type", TRUNCATED )
@pytest.mark.parametrize( "typ", TYPES )
def testTheFaceIsFlatAtTheLengthAndItsRadiusAcross( typ, cap_type ):
    """The cap's point is the middle of a flat face square to the axis at its length, whose edge is
    Radius from the axis."""
    gid = tube( typ )
    set_trunc( gid, True )
    set_caps( gid, cap_type, LENGTH, 0.0, FACE )
    for end, u in ENDS:
        mid, axis = frame( gid, u )
        assert dot( sub( point( gid, u, 0.0 ), mid ), axis ) == pytest.approx( LENGTH, abs=1e-9 ), end
        rmax = 0.0
        for k in range( 201 ):
            for j in range( NW ):
                d = sub( point( gid, cap_u( gid, u, k / 200.0 ), j / NW ), mid )
                a = dot( d, axis )
                r = norm( sub( d, scale( axis, a ) ) )
                if r < FACE - 1e-3:
                    assert a == pytest.approx( LENGTH, abs=1e-9 ), ( end, k, j )
                if abs( a - LENGTH ) < 1e-9:
                    rmax = max( rmax, r )
        assert rmax == pytest.approx( FACE, abs=1e-3 ), end


@pytest.mark.parametrize( "cap_type", TRUNCATED )
@pytest.mark.parametrize( "typ", TYPES )
def testEachRowIsTheEndScaled( typ, cap_type ):
    """On an elliptical end the face, and each row of a cone or profile cap, is the end scaled about its
    middle and moved along its axis."""
    gid = oval( typ, 3.0, 2.0 )
    set_trunc( gid, True )
    set_caps( gid, cap_type, LENGTH, 0.0, FACE )
    rows = range( 1, 20 )
    if cap_type in ( vsp.OGIVE_END_CAP, vsp.OGIVE_SURF_END_CAP ):
        rows = ( 1, 2, 3 )
    for end, u in ENDS:
        mid, axis = frame( gid, u )
        ends = [ sub( point( gid, cap_u( gid, u, 1.0 ), j / NW ), mid ) for j in range( NW ) ]
        for k in rows:
            uu = cap_u( gid, u, k / 20.0 )
            row = [ sub( point( gid, uu, j / NW ), mid ) for j in range( NW ) ]
            a = dot( row[0], axis )
            if abs( a - LENGTH ) > 1e-9 and cap_type in ( vsp.OGIVE_END_CAP, vsp.OGIVE_SURF_END_CAP ):
                continue
            lam = dot( sub( row[0], scale( axis, a ) ), ends[0] ) / dot( ends[0], ends[0] )
            for j in range( NW ):
                assert norm( sub( row[j], add( scale( axis, a ), scale( ends[j], lam ) ) ) ) < 1e-6, ( end, k, j )


@pytest.mark.parametrize( "cap_type", ( vsp.FLAT_END_CAP, vsp.ROUND_END_CAP, vsp.POINT_END_CAP ) )
def testOnlyACapThatClosesOnAShapeIsTruncated( cap_type ):
    gid = tube( "STACK" )
    set_caps( gid, cap_type, LENGTH, 0.0, FACE )
    before = all_points( gid )
    set_trunc( gid, True )
    after = all_points( gid )
    assert max( norm( sub( a, b ) ) for a, b in zip( before, after ) ) < 1e-12


@pytest.mark.parametrize( "cap_type", TRUNCATED )
def testWithNoRadiusATruncatedCapIsSharp( cap_type ):
    gid = tube( "STACK" )
    set_caps( gid, cap_type, LENGTH, 0.0, 0.0 )
    before = all_points( gid )
    set_trunc( gid, True )
    after = all_points( gid )
    assert max( norm( sub( a, b ) ) for a, b in zip( before, after ) ) < 1e-12


@pytest.mark.parametrize( "cap_type", ( vsp.CONE_END_CAP, vsp.OGIVE_END_CAP, vsp.OGIVE_SURF_END_CAP ) )
@pytest.mark.parametrize( "typ", TYPES )
def testAnOffsetMovesTheFace( typ, cap_type ):
    """An offset moves the face's middle with the point, keeping it square to the end's axis."""
    gid = tube( typ )
    set_trunc( gid, True )
    set_caps( gid, cap_type, LENGTH, 0.0, FACE )
    flat = [ ( point( gid, u, 0.0 ), frame( gid, u ) ) for end, u in ENDS ]
    set_caps( gid, cap_type, LENGTH, 0.6, FACE )
    for ( end, u ), ( p0, ( mid, axis ) ) in zip( ENDS, flat ):
        tip = point( gid, u, 0.0 )
        assert dot( sub( tip, mid ), axis ) == pytest.approx( LENGTH, abs=1e-9 ), end
        assert norm( sub( tip, p0 ) ) == pytest.approx( 0.6, abs=1e-6 ), end
        for j in range( NW ):
            q = point( gid, cap_u( gid, u, 0.02 ), j / NW )
            assert dot( sub( q, mid ), axis ) == pytest.approx( LENGTH, abs=1e-9 ), ( end, j )


def normal( gid, uu, w ):
    du = sub( point( gid, uu + 1e-7, w ), point( gid, uu - 1e-7, w ) )
    dw = sub( point( gid, uu, w + 1e-7 ), point( gid, uu, w - 1e-7 ) )
    return unit( cross( du, dw ) )


def side_fold( gid, u, nw=96, nu=24 ):
    """The least agreement between the normals of neighbouring points over the sides of the truncated cap at
    u, the half of its u from the face's edge to the end; 1 is flat, a fold is negative."""
    ns = [ [ normal( gid, cap_u( gid, u, 0.5 + 0.5 * k / nu ), j / nw ) for j in range( nw + 1 ) ]
           for k in range( 1, nu + 1 ) ]
    least = 1.0
    for k in range( nu ):
        for j in range( nw + 1 ):
            for kk, jj in ( ( k + 1, j ), ( k, j + 1 ) ):
                if kk < nu and jj <= nw:
                    least = min( least, dot( ns[k][j], ns[kk][jj] ) )
    return least


@pytest.mark.parametrize( "taper", ( 1.0, 0.6 ) )
@pytest.mark.parametrize( "cap_type", TRUNCATED )
def testATruncatedCapNeverFolds( cap_type, taper ):
    """The sides never fold, and the face is flat, square to the axis."""
    gid = tube( "STACK", taper )
    set_trunc( gid, True )
    for face in ( 0.05, 0.3, 0.9 ):
        set_caps( gid, cap_type, LENGTH, 0.0, face * taper )
        for end, u in ENDS:
            assert side_fold( gid, u ) > 0.5, ( end, face )
            mid, axis = frame( gid, u )
            for k in range( 1, 10 ):
                for j in range( 0, NW, 4 ):
                    n = normal( gid, cap_u( gid, u, 0.5 * k / 10 ), j / NW )
                    assert abs( dot( n, axis ) ) == pytest.approx( 1.0, abs=1e-9 ), ( end, face, k, j )


SWEEPS = {
    "radius": lambda s: ( LENGTH, 0.0, 0.02 + 0.9 * s ),
    "length": lambda s: ( 0.5 + 7.5 * s, 0.0, FACE ),
    "offset": lambda s: ( LENGTH, -0.8 + 1.6 * s, FACE ),
}


@pytest.mark.parametrize( "sweep", sorted( SWEEPS ) )
@pytest.mark.parametrize( "cap_type", TRUNCATED )
def testSweepingAnInputNeverJumps( cap_type, sweep ):
    """In 40 even steps of the face's radius, the length or the offset, no step moves the cap or changes its
    volume more than three times as much as the steps either side of it do."""
    gid = tube( "STACK" )
    set_trunc( gid, True )
    n = 40
    vols = []
    grids = []
    for i in range( n + 1 ):
        length, offset, face = SWEEPS[sweep]( i / n )
        set_caps( gid, cap_type, length, offset, face )
        g = grid( gid )
        grids.append( g )
        vols.append( volume( g ) )
    dv = [ abs( vols[i + 1] - vols[i] ) for i in range( n ) ]
    dp = [ max( norm( sub( a, b ) ) for ra, rb in zip( grids[i], grids[i + 1] ) for a, b in zip( ra, rb ) )
           for i in range( n ) ]
    assert spikes( dv, 1e-3 * 8.0 ) == [], dv
    assert spikes( dp, 1e-3 * 2.0 ) == [], dp


@pytest.mark.parametrize( "cap_type", TRUNCATED )
@pytest.mark.parametrize( "typ", TYPES )
def testATruncatedCapTessellates( typ, cap_type ):
    gid = tube( typ, 0.8 )
    set_trunc( gid, True )
    set_caps( gid, cap_type, 2.5, 0.0, FACE )
    mesh = vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    assert mesh
    assert_no_errors()


@pytest.mark.parametrize( "typ", TYPES )
def testATruncatedCapIsSavedAndRead( typ, tmp_path ):
    gid = tube( typ, 0.8 )
    set_trunc( gid, True )
    set_caps( gid, vsp.OGIVE_END_CAP, 2.5, 0.0, FACE )
    before = all_points( gid )
    fname = str( tmp_path / "trunc.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    gid = vsp.FindGeoms()[0]
    for end, u in ENDS:
        assert vsp.GetParmVal( cap_parm( gid, "Cap" + end + "TruncFlag" ) ) == 1.0
    after = all_points( gid )
    assert max( norm( sub( a, b ) ) for a, b in zip( before, after ) ) < 1e-12


@pytest.mark.parametrize( "geom", ( "CLONE", "CONFORMAL" ) )
@pytest.mark.parametrize( "typ", TYPES )
def testACloneAndAConformalTakeTheTruncation( typ, geom ):
    gid = tube( typ, 0.8 )
    set_trunc( gid, True )
    set_caps( gid, vsp.POWER_LAW_END_CAP, 2.5, 0.0, FACE )
    other = vsp.AddGeom( geom, gid )
    vsp.Update()
    for end, u in ENDS:
        assert vsp.GetParmVal( cap_parm( other, "Cap" + end + "TruncFlag" ) ) == 1.0, end
    assert_no_errors()
