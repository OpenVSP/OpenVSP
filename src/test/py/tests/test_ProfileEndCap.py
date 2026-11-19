# Profile end caps on a Fuselage or Stack: power law, Von Karman and Sears-Haack noses.
#
# A profile cap runs each end to a sharp point, at CapU*Length along the end's axis, along a profile: its
# radius, as a fraction of the end's, at s, the distance from the point as a fraction of the length, is
# s^CapU*PowerExp for POWER_LAW_END_CAP, the Haack series with C CapU*HaackC for VON_KARMAN_END_CAP, and the
# front half of the Sears-Haack body for SEARS_HAACK_END_CAP.  Each row of the cap is the end scaled about its
# middle, so any end's shape runs to the point.  A radius blunts the cap with a sphere whose front is the point
# (test_BluntProfileEndCap.py); these tests take none unless they say so.  An offset moves the point, the cap
# sheared along to it.
#
# The cap's half of the end in u is the profile's cubics from the end to the point.

import math

import openvsp as vsp
import pytest

from testhelp import assert_no_errors
from test_OgiveEndCap import ENDS, NW, TYPES, cap_parm, point, sub, add, scale, dot, norm, unit, cap_u, tube, oval, \
    set_caps, frame, fold, kink
from test_OgiveContinuity import body, set_cap, grid, volume, spikes

PROFILES = ( vsp.POWER_LAW_END_CAP, vsp.VON_KARMAN_END_CAP, vsp.SEARS_HAACK_END_CAP )


def haack( c ):
    """The Haack series' radius fraction at s for C c."""
    def fn( s ):
        th = math.acos( max( -1.0, min( 1.0, 1.0 - 2.0 * s ) ) )
        return math.sqrt( max( th - 0.5 * math.sin( 2.0 * th ) + c * math.sin( th ) ** 3, 0.0 ) / math.pi )
    return fn


def sears_haack( s ):
    return max( s * ( 2.0 - s ), 0.0 ) ** 0.75


def profile( cap_type, exponent, c=0.0 ):
    """The exact radius fraction at s for cap_type."""
    if cap_type == vsp.VON_KARMAN_END_CAP:
        return haack( c )
    if cap_type == vsp.SEARS_HAACK_END_CAP:
        return sears_haack
    return lambda s: max( s, 0.0 ) ** exponent


def set_exponent( gid, exponent ):
    for end, u in ENDS:
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "PowerExp" ), exponent )
    vsp.Update()


def set_haack( gid, c ):
    for end, u in ENDS:
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "HaackC" ), c )
    vsp.Update()


def gap( poly, q ):
    """The distance from q to the polyline poly."""
    best = math.hypot( poly[0][0] - q[0], poly[0][1] - q[1] )
    for a, b in zip( poly[:-1], poly[1:] ):
        dx = b[0] - a[0]
        dy = b[1] - a[1]
        den = dx * dx + dy * dy
        t = 0.0
        if den > 0.0:
            t = max( 0.0, min( 1.0, ( ( q[0] - a[0] ) * dx + ( q[1] - a[1] ) * dy ) / den ) )
        best = min( best, math.hypot( a[0] + t * dx - q[0], a[1] + t * dy - q[1] ) )
    return best


def cap_profile( gid, u, length, n=400 ):
    """The cap at u along W 0 as ( s, f ): the distance from the point as a fraction of length, and the
    radius as a fraction of the end's, about the end's axis."""
    mid, axis = frame( gid, u )
    rad = norm( sub( point( gid, cap_u( gid, u, 1.0 ), 0.0 ), mid ) )
    out = []
    for k in range( n + 1 ):
        d = sub( point( gid, cap_u( gid, u, k / n ), 0.0 ), mid )
        a = dot( d, axis )
        out.append( ( 1.0 - a / length, norm( sub( d, scale( axis, a ) ) ) / rad ) )
    return out


@pytest.mark.parametrize( "cap_type", PROFILES )
@pytest.mark.parametrize( "typ", TYPES )
def testABodyTakesAProfileCap( typ, cap_type ):
    gid = tube( typ )
    set_caps( gid, cap_type, 3.0 )
    for end, u in ENDS:
        assert vsp.GetParmVal( cap_parm( gid, "Cap" + end + "Option" ) ) == cap_type
    assert_no_errors()


@pytest.mark.parametrize( "cap_type", PROFILES )
@pytest.mark.parametrize( "typ", ( "WING", "PROP", "BODYOFREVOLUTION" ) )
def testOtherGeomsRefuseAProfileCap( typ, cap_type ):
    vsp.VSPRenew()
    gid = vsp.AddGeom( typ )
    for end, u in ENDS:
        parm = cap_parm( gid, "Cap" + end + "Option" )
        before = vsp.GetParmVal( parm )
        vsp.SetParmVal( parm, cap_type )
        assert vsp.GetParmVal( parm ) == before, end


FITS = [ ( vsp.POWER_LAW_END_CAP, e, 0.0 ) for e in ( 0.05, 0.1, 0.2, 0.3, 0.5, 0.75, 0.9, 0.99, 1.0, 1.01, 1.5,
                                                    2.0, 5.0, 20.0 ) ] + \
       [ ( vsp.VON_KARMAN_END_CAP, 0.5, c ) for c in ( 0.0, 1.0 / 3.0, 2.0 / 3.0 ) ] + \
       [ ( vsp.SEARS_HAACK_END_CAP, 0.5, 0.0 ) ]


@pytest.mark.parametrize( "cap_type, exponent, c", FITS )
@pytest.mark.parametrize( "typ", TYPES )
def testAProfileCapFollowsItsProfile( typ, cap_type, exponent, c ):
    """On a circular end, the cap comes within a thousandth of its profile, in the plane of the distance
    from the point as a fraction of the length and the radius as a fraction of the end's."""
    length = 3.0
    gid = tube( typ )
    set_exponent( gid, exponent )
    set_haack( gid, c )
    set_caps( gid, cap_type, length )
    fn = profile( cap_type, exponent, c )
    for end, u in ENDS:
        cap = cap_profile( gid, u, length, 1000 )
        for i in range( 1, 400 ):
            s = 0.5 * ( 1.0 - math.cos( math.pi * i / 400 ) )
            assert gap( cap, ( s, fn( s ) ) ) < 1e-3, ( end, s )
        for q in cap[::10]:
            assert gap( [ ( k / 4000.0, fn( k / 4000.0 ) ) for k in range( 4001 ) ], q ) < 1e-3, ( end, q )


@pytest.mark.parametrize( "cap_type, exponent, c", FITS )
@pytest.mark.parametrize( "typ", TYPES )
def testAProfileCapLeavesItsPointAsItsProfileDoes( typ, cap_type, exponent, c ):
    """The cap leaves its point square to the axis, as its profile does, but for a power law of exponent
    one, which leaves it along the cone, and above one, which leaves it along the axis."""
    length = 3.0
    gid = tube( typ )
    set_exponent( gid, exponent )
    set_haack( gid, c )
    set_caps( gid, cap_type, length )
    want = 90.0
    if cap_type == vsp.POWER_LAW_END_CAP and exponent == 1.0:
        want = math.degrees( math.atan( 1.0 / length ) )
    if cap_type == vsp.POWER_LAW_END_CAP and exponent > 1.0:
        want = 0.0
    for end, u in ENDS:
        mid, axis = frame( gid, u )
        for w in ( 0.0, 0.25 ):
            d = sub( point( gid, cap_u( gid, u, 1e-6 ), w ), point( gid, u, w ) )
            ang = math.degrees( math.acos( max( -1.0, min( 1.0, -dot( unit( d ), axis ) ) ) ) )
            assert ang == pytest.approx( want, abs=0.5 ), ( end, w )


@pytest.mark.parametrize( "cap_type", ( vsp.POWER_LAW_END_CAP, vsp.SEARS_HAACK_END_CAP ) )
def testOnlyAVonKarmanCapTakesAHaackC( cap_type ):
    gid = tube( "STACK" )
    set_caps( gid, cap_type, 3.0 )
    before = [ point( gid, i / 40.0, j / NW ) for i in range( 41 ) for j in range( NW + 1 ) ]
    set_haack( gid, 0.5 )
    after = [ point( gid, i / 40.0, j / NW ) for i in range( 41 ) for j in range( NW + 1 ) ]
    assert max( norm( sub( a, b ) ) for a, b in zip( before, after ) ) < 1e-12


@pytest.mark.parametrize( "cap_type", PROFILES )
@pytest.mark.parametrize( "typ", TYPES )
def testEachRowIsTheEndScaled( typ, cap_type ):
    """On an elliptical end, each row of the cap is the end scaled about its middle and moved along its
    axis to the point."""
    gid = oval( typ, 3.0, 2.0 )
    set_caps( gid, cap_type, 3.0 )
    for end, u in ENDS:
        mid, axis = frame( gid, u )
        ends = [ sub( point( gid, cap_u( gid, u, 1.0 ), j / NW ), mid ) for j in range( NW ) ]
        for k in range( 1, 20 ):
            uu = cap_u( gid, u, k / 20.0 )
            row = [ sub( point( gid, uu, j / NW ), mid ) for j in range( NW ) ]
            a = dot( row[0], axis )
            lam = dot( sub( row[0], scale( axis, a ) ), ends[0] ) / dot( ends[0], ends[0] )
            for j in range( NW ):
                assert norm( sub( row[j], add( scale( axis, a ), scale( ends[j], lam ) ) ) ) < 1e-9, ( end, k, j )


@pytest.mark.parametrize( "cap_type", PROFILES )
@pytest.mark.parametrize( "typ", TYPES )
def testRadiusKeepsThePointAndOffsetMovesIt( typ, cap_type ):
    """A tip radius blunts a profile cap but leaves its point where it is.  Its offset moves its point along
    Z, the cap sheared along to it."""
    length = 3.0
    gid = tube( typ )
    set_caps( gid, cap_type, length )
    sharp = [ point( gid, u, 0.0 ) for end, u in ENDS ]
    before = [ point( gid, i / 40.0, j / NW ) for i in range( 41 ) for j in range( NW + 1 ) ]
    set_caps( gid, cap_type, length, 0.0, 0.4 )
    after = [ point( gid, i / 40.0, j / NW ) for i in range( 41 ) for j in range( NW + 1 ) ]
    assert max( norm( sub( a, b ) ) for a, b in zip( before, after ) ) > 1e-2
    for tip, ( end, u ) in zip( sharp, ENDS ):
        assert norm( sub( point( gid, u, 0.0 ), tip ) ) < 1e-9, end
    frames = [ frame( gid, u ) for end, u in ENDS ]
    set_caps( gid, cap_type, length, 0.8, 0.4 )
    for ( end, u ), ( mid, axis ) in zip( ENDS, frames ):
        tip = add( add( mid, scale( axis, length ) ), ( 0.0, 0.0, 0.8 ) )
        assert norm( sub( point( gid, u, 0.0 ), tip ) ) < 1e-9, end
    assert_no_errors()


@pytest.mark.parametrize( "typ", TYPES )
def testAPowerLawOfExponentOneIsTheCone( typ ):
    """At exponent one a power law cap is the point cap's straight cone."""
    gid = tube( typ )
    set_caps( gid, vsp.POINT_END_CAP, 3.0 )
    cone = [ point( gid, cap_u( gid, u, k / 20.0 ), j / NW ) for end, u in ENDS for k in range( 21 )
             for j in range( NW ) ]
    set_exponent( gid, 1.0 )
    set_caps( gid, vsp.POWER_LAW_END_CAP, 3.0 )
    power = [ point( gid, cap_u( gid, u, k / 20.0 ), j / NW ) for end, u in ENDS for k in range( 21 )
              for j in range( NW ) ]
    # The same surface, though its points move along it with the patches' parameterization.
    for p in power:
        assert min( norm( sub( p, c ) ) for c in cone ) < 0.05


@pytest.mark.parametrize( "cap_type", ( vsp.VON_KARMAN_END_CAP, vsp.SEARS_HAACK_END_CAP ) )
@pytest.mark.parametrize( "typ", TYPES )
def testAHaackNoseLeavesACylinderSmoothly( typ, cap_type ):
    """Von Karman and Sears-Haack noses leave the end along the body's axis, so they carry a cylinder on
    smoothly."""
    gid = tube( typ )
    set_caps( gid, cap_type, 3.0 )
    for end, u in ENDS:
        assert kink( gid, u ) < 0.05, end


@pytest.mark.parametrize( "exponent", ( 0.05, 0.5, 0.75, 1.0, 2.0, 5.0 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testAPowerLawLeavesACylinderAtItsSlope( typ, exponent ):
    """A power law nose leaves the end inwards at the angle whose tangent is the exponent times the end's
    radius over the length."""
    length = 3.0
    gid = tube( typ )
    set_exponent( gid, exponent )
    set_caps( gid, vsp.POWER_LAW_END_CAP, length )
    for end, u in ENDS:
        assert kink( gid, u ) == pytest.approx( math.degrees( math.atan( exponent / length ) ), abs=0.05 ), end


@pytest.mark.parametrize( "taper", ( 1.0, 0.6 ) )
@pytest.mark.parametrize( "length", ( 0.3, 1.0, 3.0, 8.0 ) )
@pytest.mark.parametrize( "cap_type", PROFILES )
def testAProfileCapNeverFolds( cap_type, length, taper ):
    gid = tube( "STACK", taper )
    set_caps( gid, cap_type, length )
    for end, u in ENDS:
        assert fold( gid, u ) > 0.5, end


@pytest.mark.parametrize( "length", ( 0.3, 3.0, 8.0 ) )
@pytest.mark.parametrize( "exponent", ( 0.05, 0.2, 5.0, 20.0 ) )
def testAnExtremePowerLawNeverFolds( exponent, length ):
    """At the extremes of its exponent a power law cap turns sharply, near its end or near its point, but
    never folds back on itself."""
    gid = tube( "STACK" )
    set_exponent( gid, exponent )
    set_caps( gid, vsp.POWER_LAW_END_CAP, length )
    for end, u in ENDS:
        assert fold( gid, u ) > 0.0, end


@pytest.mark.parametrize( "cap_type", PROFILES )
@pytest.mark.parametrize( "typ", TYPES )
def testTheCapIsClosedAtACornerOfTheEnd( typ, cap_type ):
    """On a rectangle, the sides from either side of a corner of the end are the same, so the cap is closed
    along them."""
    gid = tube( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_ROUNDED_RECTANGLE )
        xs = vsp.GetXSec( xss, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundedRect_Width" ), 3.0 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundedRect_Height" ), 1.5 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "RoundRectXSec_Radius" ), 0.0 )
    vsp.Update()
    set_caps( gid, cap_type, 3.0 )
    for k in range( 1, 40 ):
        uu = cap_u( gid, 0.0, k / 40.0 )
        for j in range( 1, 64 ):
            w = j / 64.0
            assert norm( sub( point( gid, uu, w - 1e-10 ), point( gid, uu, w + 1e-10 ) ) ) < 1e-6, ( k, j )


@pytest.mark.parametrize( "cap_type", PROFILES )
@pytest.mark.parametrize( "typ", TYPES )
def testAProfileCapIsSavedAndRead( typ, cap_type, tmp_path ):
    gid = tube( typ, 0.8 )
    set_exponent( gid, 0.6 )
    set_haack( gid, 0.25 )
    set_caps( gid, cap_type, 2.5 )
    before = [ point( gid, i / 20.0, j / NW ) for i in range( 21 ) for j in range( NW + 1 ) ]
    fname = str( tmp_path / "profile.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    gid = vsp.FindGeoms()[0]
    for end, u in ENDS:
        assert vsp.GetParmVal( cap_parm( gid, "Cap" + end + "Option" ) ) == cap_type
        assert vsp.GetParmVal( cap_parm( gid, "Cap" + end + "PowerExp" ) ) == 0.6
        assert vsp.GetParmVal( cap_parm( gid, "Cap" + end + "HaackC" ) ) == 0.25
    after = [ point( gid, i / 20.0, j / NW ) for i in range( 21 ) for j in range( NW + 1 ) ]
    assert max( norm( sub( a, b ) ) for a, b in zip( before, after ) ) < 1e-12


@pytest.mark.parametrize( "cap_type", PROFILES )
@pytest.mark.parametrize( "typ", TYPES )
def testAProfileCapTessellates( typ, cap_type ):
    gid = tube( typ, 0.8 )
    set_caps( gid, cap_type, 2.5 )
    mesh = vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    assert mesh
    assert_no_errors()


@pytest.mark.parametrize( "cap_type", PROFILES )
@pytest.mark.parametrize( "typ", TYPES )
def testTheTessellationRunsFromPointToPoint( typ, cap_type ):
    """The tessellation's first and last rows are the caps' points, however short the patch at the point."""
    gid = oval( typ, 3.0, 2.0 )
    set_caps( gid, cap_type, 3.0 )
    u, w = vsp.GetUWTess01( gid, 0 )
    assert u[0] == 0.0
    assert u[-1] == 1.0


@pytest.mark.parametrize( "radius", ( 0.0, 0.3 ) )
@pytest.mark.parametrize( "cap_type", PROFILES )
@pytest.mark.parametrize( "typ", TYPES )
def testACapsRowsAreEvenStepsAlongIt( typ, cap_type, radius ):
    """However the cap's profile is split into patches, its rows are even steps of its parameter, so the rows of a
    coarser tessellation are among those of a finer one."""
    gid = tube( typ, 0.8 )
    set_caps( gid, cap_type, 2.5, radius=radius )
    rows = {}
    for n in ( 5, 9 ):
        vsp.SetParmVal( gid, "CapUMinTess", "EndCap", n )
        vsp.Update()
        u, w = vsp.GetUWTess01( gid, 0 )
        steps = [ u[i + 1] - u[i] for i in range( n - 1 ) ]
        assert max( steps ) - min( steps ) < 1e-12
        rows[n] = u[:n]
    assert all( min( abs( a - b ) for b in rows[9] ) < 1e-12 for a in rows[5] )
    assert_no_errors()


@pytest.mark.parametrize( "typ", TYPES )
def testACloneOfABodyTakesItsProfileCap( typ ):
    gid = tube( typ, 0.8 )
    set_exponent( gid, 0.6 )
    set_haack( gid, 0.25 )
    set_caps( gid, vsp.POWER_LAW_END_CAP, 2.5 )
    clone = vsp.AddGeom( "CLONE", gid )
    vsp.Update()
    for end, u in ENDS:
        for name in ( "Option", "PowerExp", "HaackC" ):
            assert vsp.GetParmVal( cap_parm( clone, "Cap" + end + name ) ) == \
                vsp.GetParmVal( cap_parm( gid, "Cap" + end + name ) ), ( end, name )
    assert_no_errors()


@pytest.mark.parametrize( "typ", TYPES )
def testAConformalOnABodyTakesItsProfileCap( typ ):
    gid = tube( typ, 0.8 )
    set_exponent( gid, 0.6 )
    set_haack( gid, 0.25 )
    set_caps( gid, vsp.POWER_LAW_END_CAP, 2.5 )
    conf = vsp.AddGeom( "CONFORMAL", gid )
    vsp.Update()
    for end, u in ENDS:
        for name in ( "Option", "PowerExp", "HaackC" ):
            assert vsp.GetParmVal( cap_parm( conf, "Cap" + end + name ) ) == \
                vsp.GetParmVal( cap_parm( gid, "Cap" + end + name ) ), ( end, name )
    assert_no_errors()


SWEEPS = {
    "length": lambda s: ( 1.0 + 7.0 * s, 0.5, 0.0 ),
    "exponent": lambda s: ( 3.0, 0.05 * 400.0 ** s, 0.0 ),
    "haack": lambda s: ( 3.0, 0.5, 2.0 / 3.0 * s ),
}


@pytest.mark.parametrize( "sweep", sorted( SWEEPS ) )
@pytest.mark.parametrize( "cap_type", PROFILES )
def testSweepingAnInputNeverJumps( cap_type, sweep ):
    """In 40 even steps of the length, exponent or Haack C, no step moves the cap or changes its volume more
    than three times as much as the steps either side of it."""
    if cap_type != vsp.POWER_LAW_END_CAP and sweep == "exponent":
        pytest.skip( "Only a power law takes an exponent" )
    if cap_type != vsp.VON_KARMAN_END_CAP and sweep == "haack":
        pytest.skip( "Only a Von Karman cap takes a Haack C" )
    gid = body( 2.0 )
    n = 40
    vols = []
    grids = []
    for i in range( n + 1 ):
        length, exponent, c = SWEEPS[sweep]( i / n )
        set_exponent( gid, exponent )
        set_haack( gid, c )
        set_cap( gid, cap_type, length, 0.0, 0.0, 0.0 )
        g = grid( gid )
        grids.append( g )
        vols.append( volume( g ) )
    dv = [ abs( vols[i + 1] - vols[i] ) for i in range( n ) ]
    dp = [ max( norm( sub( a, b ) ) for ra, rb in zip( grids[i], grids[i + 1] ) for a, b in zip( ra, rb ) )
           for i in range( n ) ]
    assert spikes( dv, 1e-3 * 8.0 ) == [], dv
    assert spikes( dp, 1e-3 * 2.0 ) == [], dp
