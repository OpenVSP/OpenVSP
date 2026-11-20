# A Biconvex or Wedge airfoil rounded at its leading edge: a circle of radius LERadius ( over the chord ) about
# ( r, 0 ), whose front is the leading edge at the origin, so the section still runs from x/c 0 to 1.  LERadiusType
# keeps the crests, the forward surfaces refit to touch the circle, or the forward surfaces' shape, which slides
# forward with the crests.

import math

import openvsp as vsp
import pytest

from test_SectionBlend import apart, prop_section

KINDS = [ ( vsp.XS_WEDGE, False ), ( vsp.XS_BICONVEX, False ), ( vsp.XS_BICONVEX, True ) ]
KIND_IDS = [ "wedge", "parabolas", "arcs" ]
TYPES = [ vsp.LE_RADIUS_KEEP_CREST, vsp.LE_RADIUS_KEEP_ANGLE ]
TYPE_IDS = [ "crest", "angle" ]


def make( kind, circle=False, r=0.0, ltype=vsp.LE_RADIUS_KEEP_CREST, t=0.05, **parms ):
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    surf = vsp.GetXSecSurf( wid, 0 )
    vsp.ChangeXSecShape( surf, 1, kind )
    xs = vsp.GetXSec( surf, 1 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "ThickChord" ), t )
    if kind == vsp.XS_BICONVEX:
        vsp.SetParmVal( vsp.GetXSecParm( xs, "UseCircleArc" ), circle )
    for name, val in parms.items():
        vsp.SetParmVal( vsp.GetXSecParm( xs, name ), val )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "LERadiusType" ), ltype )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "LERadius" ), r )
    vsp.Update()
    return xs


def points( xs, n=4000 ):
    """The section over its chord, from the trailing edge under the lower side and back over the upper, with the
    leading edge, u = 0.5, at the origin."""
    le = vsp.ComputeXSecPnt( xs, 0.5 )
    chord = vsp.ComputeXSecPnt( xs, 0.0 ).x() - le.x()
    pts = []
    for j in range( n + 1 ):
        p = vsp.ComputeXSecPnt( xs, j / n )
        pts.append( ( ( p.x() - le.x() ) / chord, ( p.z() - le.z() ) / chord, j / n ) )
    return pts


def wedge_crest( t, loc, zc, upper ):
    h = 0.5 * t
    if upper:
        return loc, h + zc
    return loc, -h + zc


def tangent_point( r, cx, cz, upper ):
    """Where the line from the crest ( cx, cz ) touches the nose circle about ( r, 0 )."""
    dx = cx - r
    l = math.hypot( dx, cz )
    th = math.atan2( cz, dx )
    turn = math.acos( r / l )
    if upper:
        th += turn
    else:
        th -= turn
    return r + r * math.cos( th ), r * math.sin( th )


def side( pts, upper ):
    if upper:
        return [ p for p in pts if p[2] >= 0.5 ]
    return [ p for p in pts if p[2] <= 0.5 ]


def z_at( pts, x ):
    """The height of a side, its points in order along it, at x, by linear interpolation."""
    for a, b in zip( pts, pts[1:] ):
        if min( a[0], b[0] ) <= x <= max( a[0], b[0] ) and a[0] != b[0]:
            return a[1] + ( b[1] - a[1] ) * ( x - a[0] ) / ( b[0] - a[0] )
    raise ValueError( "x outside the side" )


def parabola_slide( h, run, r ):
    """Where the crest of a parabola of run `run`, h high, slid forward to touch the nose circle about ( r, 0 ), ends
    up: the point r inside it crosses the chord line at x = r."""
    ah = abs( h )
    lo, hi = 0.0, run
    for i in range( 100 ):
        v = 0.5 * ( lo + hi )
        w = 2.0 * ah * v / ( run * run )
        if ah * ( 1.0 - v * v / ( run * run ) ) - r / math.sqrt( 1.0 + w * w ) > 0.0:
            lo = v
        else:
            hi = v
    w = 2.0 * ah * v / ( run * run )
    return r + v - r * w / math.sqrt( 1.0 + w * w )


@pytest.mark.parametrize( "ltype", TYPES, ids=TYPE_IDS )
@pytest.mark.parametrize( "kind,circle", KINDS, ids=KIND_IDS )
def testARoundedSectionRunsFromZeroToOne( kind, circle, ltype ):
    pts = points( make( kind, circle, 0.005, ltype ) )
    assert min( p[0] for p in pts ) == pytest.approx( 0.0, abs=1e-6 )
    assert max( p[0] for p in pts ) == pytest.approx( 1.0, abs=1e-9 )
    # The front is the leading edge, on the chord line.
    front = min( pts, key=lambda p: p[0] )
    assert front[1] == pytest.approx( 0.0, abs=2e-4 )


@pytest.mark.parametrize( "ltype", TYPES, ids=TYPE_IDS )
@pytest.mark.parametrize( "kind,circle", KINDS, ids=KIND_IDS )
def testTheNoseIsACircleOfTheRadius( kind, circle, ltype ):
    r = 0.005
    pts = points( make( kind, circle, r, ltype ) )
    near = [ p for p in pts if p[0] < 0.5 * r ]
    assert len( near ) > 3
    for x, z, u in near:
        assert math.hypot( x - r, z ) == pytest.approx( r, abs=5e-4 * r )


@pytest.mark.parametrize( "kind,circle", KINDS, ids=KIND_IDS )
def testKeepCrestKeepsTheThickestPoints( kind, circle ):
    xs = make( kind, circle, 0.005, vsp.LE_RADIUS_KEEP_CREST, t=0.08, ThickLoc=0.4 )
    pts = points( xs )
    hi = max( pts, key=lambda p: p[1] )
    lo = min( pts, key=lambda p: p[1] )
    assert hi[1] == pytest.approx( 0.04, abs=1e-6 )
    assert lo[1] == pytest.approx( -0.04, abs=1e-6 )
    assert hi[0] == pytest.approx( 0.4, abs=2e-3 )
    # A Wedge's thicknesses are symmetric unless SymmThick is off.
    if kind == vsp.XS_WEDGE:
        assert lo[0] == pytest.approx( 0.4, abs=2e-3 )
    else:
        assert lo[0] == pytest.approx( 0.5, abs=2e-3 )


@pytest.mark.parametrize( "kind,circle", KINDS, ids=KIND_IDS )
def testKeepCrestLeavesTheAftSurfacesAlone( kind, circle ):
    sharp = points( make( kind, circle, 0.0, t=0.08, ThickLoc=0.4 ) )
    round_ = points( make( kind, circle, 0.005, vsp.LE_RADIUS_KEEP_CREST, t=0.08, ThickLoc=0.4 ) )
    for upper in ( True, False ):
        for x in ( 0.5, 0.7, 0.9 ):
            assert z_at( side( round_, upper ), x ) == pytest.approx( z_at( side( sharp, upper ), x ), abs=1e-6 )


def testAWedgeKeepingItsCrestRunsStraightFromTheCrestToTheCircle():
    r = 0.005
    pts = points( make( vsp.XS_WEDGE, r=r, ltype=vsp.LE_RADIUS_KEEP_CREST, t=0.06, SymmThick=0, ThickLoc=0.4,
                        ThickLocLow=0.6, ZCamber=0.01 ) )
    for upper, loc in ( ( True, 0.4 ), ( False, 0.6 ) ):
        cx, cz = wedge_crest( 0.06, loc, 0.01, upper )
        tx, tz = tangent_point( r, cx, cz, upper )
        side = [ p for p in pts if ( p[2] > 0.5 ) == upper and tx + 1e-3 < p[0] < cx - 1e-3 ]
        assert len( side ) > 10
        for x, z, u in side:
            assert z == pytest.approx( tz + ( cz - tz ) * ( x - tx ) / ( cx - tx ), abs=1e-6 )


def testAWedgeKeepingItsAngleKeepsTheSlopeAndMovesTheCrest():
    r = 0.005
    t = 0.05
    pts = points( make( vsp.XS_WEDGE, r=r, ltype=vsp.LE_RADIUS_KEEP_ANGLE, t=t ) )
    h = 0.5 * t
    d = math.atan( h / 0.5 )
    # The side touches the circle at r ( 1 - sin d ), r cos d, at the sharp angle, so the crest moves forward.
    tx = r - r * math.sin( d )
    tz = r * math.cos( d )
    cx = tx + ( h - tz ) / math.tan( d )
    upper = [ p for p in pts if p[2] > 0.5 ]
    hi = max( upper, key=lambda p: p[1] )
    assert hi[1] == pytest.approx( h, abs=1e-7 )
    assert hi[0] == pytest.approx( cx, abs=2e-3 )
    side = [ p for p in upper if tx + 1e-3 < p[0] < cx - 1e-3 ]
    assert len( side ) > 10
    for x, z, u in side:
        assert z == pytest.approx( tz + math.tan( d ) * ( x - tx ), abs=1e-6 )
    assert cx < 0.45


def testABiconvexOfArcsKeepingItsCrestRefitsTheForwardArc():
    r = 0.005
    t = 0.05
    h = 0.5 * t
    pts = points( make( vsp.XS_BICONVEX, True, r, vsp.LE_RADIUS_KEEP_CREST, t=t ) )
    rk = ( ( 0.5 - r ) ** 2 + h * h - r * r ) / ( 2.0 * ( h - r ) )
    for x, z, u in pts:
        if u > 0.5 and 0.05 < x < 0.49:
            assert math.hypot( x - 0.5, z - ( h - rk ) ) == pytest.approx( rk, abs=1e-5 )


def testABiconvexOfArcsKeepingItsAngleSlidesTheForwardArc():
    r = 0.005
    t = 0.05
    h = 0.5 * t
    rf = ( 0.25 + h * h ) / ( 2.0 * h )
    xp = r + math.sqrt( ( rf - r ) ** 2 - ( h - rf ) ** 2 )
    pts = points( make( vsp.XS_BICONVEX, True, r, vsp.LE_RADIUS_KEEP_ANGLE, t=t ) )
    for x, z, u in pts:
        if u > 0.5 and 0.05 < x < xp - 0.01:
            assert math.hypot( x - xp, z - ( h - rf ) ) == pytest.approx( rf, abs=1e-5 )
    hi = max( pts, key=lambda p: p[1] )
    assert hi[0] == pytest.approx( xp, abs=2e-3 )


def testABiconvexOfParabolasKeepingItsAngleSlidesTheForwardParabola():
    r = 0.005
    t = 0.05
    h = 0.5 * t
    pts = points( make( vsp.XS_BICONVEX, False, r, vsp.LE_RADIUS_KEEP_ANGLE, t=t ) )
    upper = [ p for p in pts if p[2] > 0.5 ]
    xp = parabola_slide( h, 0.5, r )
    assert xp < 0.46
    assert max( upper, key=lambda p: p[1] )[0] == pytest.approx( xp, abs=5e-3 )
    # The forward parabola keeps its run 0.5, so it is z = h ( 1 - ( ( xp - x ) / 0.5 )^2 ).
    for x, z, u in upper:
        if 0.05 < x < xp - 0.02:
            assert z == pytest.approx( h * ( 1.0 - ( ( xp - x ) / 0.5 ) ** 2 ), abs=1e-6 )


@pytest.mark.parametrize( "ltype", TYPES, ids=TYPE_IDS )
@pytest.mark.parametrize( "kind,circle", KINDS, ids=KIND_IDS )
def testTheNoseJoinsTheSidesSmoothly( kind, circle, ltype ):
    # No corner anywhere ahead of the crests: successive chords turn by little.
    pts = points( make( kind, circle, 0.005, ltype ), n=20000 )
    # The curve rests at the leading edge over the tiny pieces the curve modifiers add there; those are skipped.
    front = []
    for p in pts:
        if p[0] < 0.2 and ( not front or math.hypot( p[0] - front[-1][0], p[1] - front[-1][1] ) > 1e-9 ):
            front.append( p )
    worst = 0.0
    for a, b, c in zip( front, front[1:], front[2:] ):
        a1 = math.atan2( b[1] - a[1], b[0] - a[0] )
        a2 = math.atan2( c[1] - b[1], c[0] - b[0] )
        d = abs( ( a2 - a1 + math.pi ) % ( 2 * math.pi ) - math.pi )
        worst = max( worst, d )
    assert worst < 0.05


@pytest.mark.parametrize( "ltype", TYPES, ids=TYPE_IDS )
@pytest.mark.parametrize( "kind,circle", KINDS, ids=KIND_IDS )
def testAnInvertedRoundedSectionStillStartsAtZero( kind, circle, ltype ):
    xs = make( kind, circle, 0.005, ltype, Invert=1 )
    pts = points( xs )
    assert min( p[0] for p in pts ) == pytest.approx( 0.0, abs=1e-6 )


@pytest.mark.parametrize( "kind,circle", KINDS, ids=KIND_IDS )
def testNoRadiusIsTheSharpSection( kind, circle ):
    a = points( make( kind, circle, 0.0 ) )
    xs = make( kind, circle, 0.005 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "LERadius" ), 0.0 )
    vsp.Update()
    b = points( xs )
    for p, q in zip( a, b ):
        assert p[0] == pytest.approx( q[0], abs=1e-12 )
        assert p[1] == pytest.approx( q[1], abs=1e-12 )


@pytest.mark.parametrize( "ltype", TYPES, ids=TYPE_IDS )
def testTheRadiusIsLimitedWhereASideHasNoHeight( ltype ):
    xs = make( vsp.XS_BICONVEX, False, 0.005, ltype, ThickFracUp=1.0 )
    assert vsp.GetParmVal( vsp.GetXSecParm( xs, "LERadius" ) ) == 0.0


@pytest.mark.parametrize( "ltype", TYPES, ids=TYPE_IDS )
@pytest.mark.parametrize( "kind,circle", KINDS, ids=KIND_IDS )
def testTheRadiusIsLimitedToWhatTheSectionCanHold( kind, circle, ltype ):
    xs = make( kind, circle, 0.5, ltype )
    # The nose can be no taller than the crest; keeping the angle, its top reaches the crest's height a little sooner.
    rmax = vsp.GetParmVal( vsp.GetXSecParm( xs, "LERadius" ) )
    assert 0.02 < rmax < 0.0251
    pts = points( xs )
    assert min( p[0] for p in pts ) == pytest.approx( 0.0, abs=1e-6 )


@pytest.mark.parametrize( "kind", [ vsp.XS_WEDGE, vsp.XS_BICONVEX ], ids=[ "wedge", "biconvex" ] )
def testTheRadiusBlendsBetweenPropellerSections( kind ):
    root = prop_section( kind, { "LERadius": 0.0 }, { "LERadius": 0.0 } )
    mid = prop_section( kind, { "LERadius": 0.0 }, { "LERadius": 0.004 } )
    tip = prop_section( kind, { "LERadius": 0.004 }, { "LERadius": 0.004 } )
    across = apart( root, tip )
    assert across > 1e-3
    assert apart( root, mid ) > 0.2 * across
    assert apart( mid, tip ) > 0.2 * across
