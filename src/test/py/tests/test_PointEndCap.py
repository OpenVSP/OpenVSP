# Point end caps.
#
# A point cap draws an end straight in to one point: ahead of the middle of the end, along the axis the
# Geom is built along, by its length, and above it along Z by its offset, both in units of the end's
# size across the axis.  With no length and no offset it is halfway between the end's w 0.25 and w 0.75
# points.  The rest of the surface is left where it was.

import math

import openvsp as vsp
import pytest

from testhelp import drop_errors, assert_no_errors

ENDS = ( ( "UMin", 0.0 ), ( "UMax", 1.0 ) )
NW = 16
TYPES = ( "FUSELAGE", "STACK", "WING" )

# Length and offset of both caps.
LENOFFS = ( ( 0.0, 0.0 ), ( 1.0, 0.0 ), ( 0.0, 0.3 ), ( 0.6, 0.3 ), ( 1.5, -0.25 ) )


def body( typ ):
    """An uncapped Fuselage or Stack whose nose is a raised, sideways elliptical section and whose tail is
    a lower super ellipse, flatter below than above; or an uncapped swept wing with a thick cambered tip."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    n = vsp.GetNumXSec( xss )

    if typ == "WING":
        vsp.SetParmVal( gid, "Sweep", "XSec_1", 30.0 )
        vsp.SetParmVal( gid, "Tip_Chord", "XSec_1", 2.0 )
        tip = vsp.GetXSec( xss, 1 )
        vsp.SetParmVal( vsp.GetXSecParm( tip, "ThickChord" ), 0.2 )
        vsp.SetParmVal( vsp.GetXSecParm( tip, "Camber" ), 0.06 )
        for end, u in ENDS:
            vsp.SetParmVal( vsp.FindParm( gid, "Cap" + end + "Option", "EndCap" ), vsp.NO_END_CAP )
            vsp.SetParmVal( vsp.FindParm( gid, "Cap" + end + "SweepFlag", "EndCap" ), True )
        vsp.Update()
        return gid

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


def set_caps( gid, cap_type, length=0.0, offset=0.0 ):
    for end, u in ENDS:
        vsp.SetParmVal( vsp.FindParm( gid, "Cap" + end + "Option", "EndCap" ), cap_type )
        vsp.SetParmVal( vsp.FindParm( gid, "Cap" + end + "Length", "EndCap" ), length )
        vsp.SetParmVal( vsp.FindParm( gid, "Cap" + end + "Offset", "EndCap" ), offset )
    vsp.Update()


def point( gid, u, w ):
    p = vsp.CompPnt01( gid, 0, u, w )
    return ( p.x(), p.y(), p.z() )


def dist( a, b ):
    return max( abs( x - y ) for x, y in zip( a, b ) )


@pytest.mark.parametrize( "typ", ( "FUSELAGE", "STACK" ) )
def testPointCapIsAllowed( typ ):
    gid = body( typ )
    set_caps( gid, vsp.POINT_END_CAP )
    for end, u in ENDS:
        assert vsp.GetParmVal( vsp.FindParm( gid, "Cap" + end + "Option", "EndCap" ) ) == vsp.POINT_END_CAP
    assert_no_errors()


@pytest.mark.parametrize( "typ", TYPES )
def testAPointCapWithNoLengthClosesOnTheMiddleOfTheEnd( typ ):
    gid = body( typ )
    mids = []
    for end, u in ENDS:
        up = point( gid, u, 0.25 )
        down = point( gid, u, 0.75 )
        mids.append( tuple( 0.5 * ( a + b ) for a, b in zip( up, down ) ) )

    set_caps( gid, vsp.POINT_END_CAP )

    for ( end, u ), mid in zip( ENDS, mids ):
        for j in range( NW + 1 ):
            assert dist( point( gid, u, j / NW ), mid ) < 1e-12, ( end, j )


def frame_tip( gid, typ, u, length, offset, n=256 ):
    """Where the frame the Geom is built in puts the point of the uncapped end at u: from the middle of the end,
    between its points at W 0.25 and 0.75, along the axis away from the body by length, and along Z by offset,
    each times the end's root mean square distance from its middle across the axis.  The axis is Y for a
    wing and X otherwise."""
    ax = 0
    if typ == "WING":
        ax = 1
    ring = [ point( gid, u, ( k + 0.5 ) / n ) for k in range( n ) ]
    a = point( gid, u, 0.25 )
    b = point( gid, u, 0.75 )
    mid = tuple( 0.5 * ( x + y ) for x, y in zip( a, b ) )
    size = math.sqrt( sum( sum( ( p[i] - mid[i] ) ** 2 for i in range( 3 ) if i != ax ) for p in ring ) / n )
    out = -1.0
    if u > 0.5:
        out = 1.0
    tip = list( mid )
    tip[ax] += out * length * size
    tip[2] += offset * size
    return tuple( tip )


@pytest.mark.parametrize( "lenoff", LENOFFS )
@pytest.mark.parametrize( "typ", TYPES )
def testAPointCapClosesOnItsPointInTheBodysFrame( typ, lenoff ):
    """The point is ahead of the middle of the end along the axis the Geom is built along by the length, and
    above it along Z by the offset, both in units of the end's size across the axis, whatever the sections
    behind the end."""
    length, offset = lenoff
    gid = body( typ )
    set_caps( gid, vsp.NO_END_CAP )
    tips = [ frame_tip( gid, typ, u, length, offset ) for end, u in ENDS ]

    set_caps( gid, vsp.POINT_END_CAP, length, offset )

    for ( end, u ), tip in zip( ENDS, tips ):
        for j in range( NW + 1 ):
            assert dist( point( gid, u, j / NW ), tip ) < 1e-9, ( end, j )


@pytest.mark.parametrize( "typ", TYPES )
def testALongerPointCapReachesFurther( typ ):
    gid = body( typ )
    set_caps( gid, vsp.POINT_END_CAP )
    flat = [ point( gid, u, 0.0 ) for end, u in ENDS ]
    reach = []
    for length in ( 0.5, 1.0 ):
        set_caps( gid, vsp.POINT_END_CAP, length )
        reach.append( [ dist( point( gid, u, 0.0 ), p ) for ( end, u ), p in zip( ENDS, flat ) ] )
    for i, ( end, u ) in enumerate( ENDS ):
        assert reach[0][i] > 1e-3, end
        assert reach[1][i] == pytest.approx( 2.0 * reach[0][i], rel=1e-9 ), end


@pytest.mark.parametrize( "typ", TYPES )
def testPointCapLeavesTheBodyAlone( typ ):
    gid = body( typ )
    before = [ point( gid, i / 10.0, j / NW ) for i in range( 11 ) for j in range( NW + 1 ) ]

    set_caps( gid, vsp.POINT_END_CAP, 0.8, 0.2 )

    for p in before:
        d, u, w = vsp.ProjPnt01( gid, 0, vsp.vec3d( *p ) )
        assert d < 1e-9, p


@pytest.mark.parametrize( "typ", ( "FUSELAGE", "STACK" ) )
def testAPointCapWithNoLengthEnclosesWhatAFlatCapDoes( typ ):
    """Both caps lie in the plane of a planar end, so the closed bodies hold the same volume."""
    gid = body( typ )
    vols = []
    for cap_type in ( vsp.FLAT_END_CAP, vsp.POINT_END_CAP ):
        set_caps( gid, cap_type )
        mesh = vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
        rid = vsp.FindLatestResultsID( "Comp_Geom" )
        vols.append( vsp.GetDoubleResults( rid, "Theo_Vol" )[0] )
        vsp.DeleteGeom( mesh )
    assert vols[0] > 0.0
    assert vols[1] == pytest.approx( vols[0], rel=1e-12 )
    assert_no_errors()
