# Ogive end caps change smoothly with their inputs.
#
# Small changes to an ogive cap's length, offset, radius or angle make small changes to its surface and
# volume: sweeping any one of them in even steps, no step moves the cap or changes its volume many times
# more than the steps either side of it do.  A side jumping from one side of the sphere to the other shows
# up as such a step.  Nor does the cap ever reach further from the middle of the end than twice the end's
# radius, the length, the offset and the radius together.  A cap on a circular end is smooth round the end at every row: no point of a row turns
# or steps much more than its neighbours do.
#
# The cases are a tangent ogive on its axis and spheres raised far off the end, as in the models the
# ogive caps were checked against.

import math

import openvsp as vsp
import pytest

from testhelp import drop_errors
from test_ConeEndCap import cap_parm, point, sub, norm, cap_u, ENDS

OGIVES = ( vsp.OGIVE_END_CAP, vsp.OGIVE_SURF_END_CAP )

# name: ( diameter, length, offset, radius, angle )
CASES = {
    "tangent": ( 2.0, 3.0, 0.0, 0.3, 0.0 ),
    "high": ( 2.5, 1.7425, 4.0625, 1.036, 0.0 ),
    "long": ( 2.5, 8.93, 4.076, 0.2858, -25.0 ),
    "raised": ( 2.0, 2.0, 2.15, 1.25, 0.0 ),
    "lowered": ( 2.0, 2.0, -2.15, 1.25, 10.0 ),
}

NU = 16
NWG = 64


def body( diameter ):
    """A stack of circular sections diameter across."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( "STACK" )
    xss = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_CIRCLE )
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, i ), "Circle_Diameter" ), diameter )
    vsp.Update()
    return gid


def set_cap( gid, cap_type, length, offset, radius, angle ):
    """Caps both ends with dimensional length, offset and radius at angle."""
    for end, u in ENDS:
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "Option" ), cap_type )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RelFlag" ), False )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "LengthDim" ), length )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "OffsetDim" ), offset )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "RadiusDim" ), radius )
        vsp.SetParmVal( cap_parm( gid, "Cap" + end + "OgiveAngle" ), angle )
    vsp.Update()


def grid( gid ):
    """Points of the cap at the first end, rows from the tip to the end, NWG round each."""
    return [ [ point( gid, cap_u( gid, 0.0, k / NU ), j / NWG ) for j in range( NWG ) ] for k in range( NU + 1 ) ]


def volume( g ):
    """The volume the cap's grid g closes off with the end."""
    def tet( a, b, c ):
        return ( a[0] * ( b[1] * c[2] - b[2] * c[1] ) - a[1] * ( b[0] * c[2] - b[2] * c[0] ) +
                 a[2] * ( b[0] * c[1] - b[1] * c[0] ) ) / 6.0

    v = 0.0
    for k in range( NU ):
        for j in range( NWG ):
            jn = ( j + 1 ) % NWG
            v += tet( g[k][j], g[k + 1][j], g[k + 1][jn] ) + tet( g[k][j], g[k + 1][jn], g[k][jn] )
    mid = tuple( sum( c ) / NWG for c in zip( *g[NU] ) )
    for j in range( NWG ):
        v += tet( g[NU][j], mid, g[NU][( j + 1 ) % NWG] )
    return abs( v )


def spikes( d, tol ):
    """Steps of d more than three times the larger of their neighbours, beyond tol."""
    out = []
    for i in range( len( d ) ):
        near = []
        if i > 0:
            near.append( d[i - 1] )
        if i < len( d ) - 1:
            near.append( d[i + 1] )
        if d[i] > 3.0 * max( near ) + tol:
            out.append( i )
    return out


SWEEPS = {
    "angle": lambda c, s: c[:4] + ( c[4] - 30.0 + 60.0 * s, ),
    "offset": lambda c, s: c[:2] + ( c[2] - 1.0 + 2.0 * s, ) + c[3:],
    "length": lambda c, s: c[:1] + ( c[1] * ( 0.8 + 0.4 * s ), ) + c[2:],
    "radius": lambda c, s: c[:3] + ( c[3] * ( 0.5 + s ), ) + c[4:],
}


@pytest.mark.parametrize( "sweep", sorted( SWEEPS ) )
@pytest.mark.parametrize( "case", sorted( CASES ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
def testSweepingAnInputNeverJumps( cap_type, case, sweep ):
    """In 40 even steps of one input, no step moves the cap or changes its volume more than three times
    as much as the steps either side of it, and the cap stays within twice its inputs' reach of the middle
    of the end."""
    c = CASES[case]
    gid = body( c[0] )
    n = 40
    vols = []
    grids = []
    for i in range( n + 1 ):
        d, length, offset, radius, angle = SWEEPS[sweep]( c, i / n )
        set_cap( gid, cap_type, length, offset, radius, angle )
        g = grid( gid )
        mid = tuple( sum( p ) / NWG for p in zip( *g[NU] ) )
        reach = 2.0 * ( 0.5 * d + length + abs( offset ) + radius )
        assert max( norm( sub( p, mid ) ) for row in g for p in row ) < reach, i
        grids.append( g )
        vols.append( volume( g ) )
    dv = [ abs( vols[i + 1] - vols[i] ) for i in range( n ) ]
    dp = [ max( norm( sub( a, b ) ) for ra, rb in zip( grids[i], grids[i + 1] ) for a, b in zip( ra, rb ) )
           for i in range( n ) ]
    scale = c[0]
    assert spikes( dv, 1e-3 * scale ** 3 ) == [], dv
    assert spikes( dp, 1e-3 * scale ) == [], dp


@pytest.mark.parametrize( "angle", ( -25.0, 0.0, 10.0, 25.0 ) )
@pytest.mark.parametrize( "case", sorted( CASES ) )
@pytest.mark.parametrize( "cap_type", OGIVES )
def testACircularEndsCapIsSmoothRoundIt( cap_type, case, angle, request ):
    """Along every row of the cap on a circular end, no point steps or turns more than three times as much
    as the points either side of it do."""
    if cap_type == vsp.OGIVE_SURF_END_CAP and case == "tangent" and angle == 25.0:
        request.node.add_marker( pytest.mark.xfail( strict=True, reason="The sides cross each other at this "
                                                    "angle, as asked, and the row where they cross pinches to "
                                                    "a point" ) )
    c = CASES[case]
    gid = body( c[0] )
    set_cap( gid, cap_type, c[1], c[2], c[3], angle )
    nw = 256
    for k in range( 1, NU ):
        ps = [ point( gid, cap_u( gid, 0.0, k / NU ), j / nw ) for j in range( nw ) ]
        steps = [ sub( ps[( j + 1 ) % nw], ps[j] ) for j in range( nw ) ]
        lens = [ norm( s ) for s in steps ]
        if max( lens ) < 1e-6 * c[0]:
            continue
        turns = []
        for j in range( nw ):
            a = steps[j - 1]
            b = steps[j]
            la = norm( a )
            lb = norm( b )
            if la < 1e-12 or lb < 1e-12:
                turns.append( 0.0 )
            else:
                turns.append( math.acos( max( -1.0, min( 1.0, sum( x * y for x, y in zip( a, b ) ) / ( la * lb ) ) ) ) )
        bad = []
        for j in range( nw ):
            if lens[j] > 3.0 * max( lens[j - 2], lens[( j + 2 ) % nw] ) + 1e-3 * c[0]:
                bad.append( ( "step", j ) )
            if turns[j] > 3.0 * max( turns[j - 2], turns[( j + 2 ) % nw] ) + math.radians( 1.0 ):
                bad.append( ( "turn", j ) )
        assert bad == [], ( k, bad )
