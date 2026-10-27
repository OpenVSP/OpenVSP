# End caps offset by shearing, as DATCOM cambers a body.
#
# With CapU*ShearFlag set, a cone or ogive cap is built with no offset and then sheared: each point moves
# along Z, the frame's up direction, by the offset times its distance from the plane of the end over the
# point's, so the end stays where it is and the point reaches the offset.  A point or profile cap's offset is that shear
# already, with or without the flag.

import math

import openvsp as vsp
import pytest

from testhelp import assert_no_errors
from test_OgiveEndCap import ENDS, NW, TYPES, cap_parm, point, tube, oval

LENGTH = 3.0
OFFSET = 0.6

SHEARED = ( vsp.CONE_END_CAP, vsp.OGIVE_END_CAP, vsp.OGIVE_SURF_END_CAP )
ALREADY = ( vsp.POINT_END_CAP, vsp.POWER_LAW_END_CAP, vsp.VON_KARMAN_END_CAP, vsp.SEARS_HAACK_END_CAP )


def set_cap( gid, cap_type, offset, shear, radius=0.3, trunc=False ):
    for end, u in ENDS:
        for name, val in ( ( "Option", cap_type ), ( "RelFlag", False ), ( "LengthDim", LENGTH ),
                           ( "OffsetDim", offset ), ( "RadiusDim", radius ), ( "ShearFlag", shear ),
                           ( "TruncFlag", trunc ) ):
            vsp.SetParmVal( cap_parm( gid, "Cap" + end + name ), val )
    vsp.Update()


def cap_points( gid ):
    """Points of each end's cap and the end's X, the cap's half of u in from each end."""
    out = []
    for end, u in ENDS:
        xend = point( gid, u, 0.25 )[0]
        pts = []
        for k in range( 41 ):
            uu = 0.25 * k / 40
            if u > 0.5:
                uu = 1.0 - uu
            pts.extend( point( gid, uu, j / NW ) for j in range( NW ) )
        out.append( ( end, pts ) )
    return out


def ends( gid ):
    """Each end's X, from the uncapped body."""
    set_cap( gid, vsp.NO_END_CAP, 0.0, False )
    return [ point( gid, u, 0.25 )[0] for end, u in ENDS ]


def sheared( pts, xend, sign ):
    return [ ( p[0], p[1], p[2] + OFFSET * max( 0.0, sign * ( p[0] - xend ) ) / LENGTH ) for p in pts ]


@pytest.mark.parametrize( "trunc", ( False, True ) )
@pytest.mark.parametrize( "cap_type", SHEARED + ALREADY )
@pytest.mark.parametrize( "typ", TYPES )
def testASheredCapIsTheCapWithNoOffsetSheared( typ, cap_type, trunc ):
    gid = tube( typ )
    xends = ends( gid )
    set_cap( gid, cap_type, 0.0, True, trunc=trunc )
    flat = cap_points( gid )
    set_cap( gid, cap_type, OFFSET, True, trunc=trunc )
    for ( end, f ), ( end2, s ), xend, sign in zip( flat, cap_points( gid ), xends, ( -1.0, 1.0 ) ):
        for p, q in zip( sheared( f, xend, sign ), s ):
            assert math.dist( p, q ) < 1e-9, ( end, p, q )
    assert_no_errors()


@pytest.mark.parametrize( "cap_type", ALREADY )
@pytest.mark.parametrize( "typ", TYPES )
def testAPointOrProfileCapIsShearedWithOrWithoutTheFlag( typ, cap_type ):
    gid = tube( typ )
    set_cap( gid, cap_type, OFFSET, True )
    with_flag = [ p for end, pts in cap_points( gid ) for p in pts ]
    set_cap( gid, cap_type, OFFSET, False )
    without = [ p for end, pts in cap_points( gid ) for p in pts ]
    assert max( math.dist( p, q ) for p, q in zip( with_flag, without ) ) < 1e-12


@pytest.mark.parametrize( "cap_type", SHEARED )
@pytest.mark.parametrize( "typ", TYPES )
def testAShearedCapsPointReachesTheOffset( typ, cap_type ):
    """The shear carries the point of a sharp cap, or a blunt one's front, to the offset, as without it."""
    gid = tube( typ )
    set_cap( gid, cap_type, OFFSET, False, radius=0.0 )
    plain = [ point( gid, u, 0.0 ) for end, u in ENDS ]
    set_cap( gid, cap_type, OFFSET, True, radius=0.0 )
    for ( end, u ), p in zip( ENDS, plain ):
        assert math.dist( point( gid, u, 0.0 ), p ) < 1e-9, end


@pytest.mark.parametrize( "cap_type", SHEARED )
def testTheShearIsCarriedOnAnEllipticalEnd( cap_type ):
    gid = oval( "STACK", 3.0, 2.0 )
    xends = ends( gid )
    set_cap( gid, cap_type, 0.0, True )
    flat = cap_points( gid )
    set_cap( gid, cap_type, OFFSET, True )
    for ( end, f ), ( end2, s ), xend, sign in zip( flat, cap_points( gid ), xends, ( -1.0, 1.0 ) ):
        for p, q in zip( sheared( f, xend, sign ), s ):
            assert math.dist( p, q ) < 1e-9, ( end, p, q )


@pytest.mark.parametrize( "geom", ( "CLONE", "CONFORMAL" ) )
def testACloneAndAConformalTakeTheShear( geom ):
    gid = tube( "STACK" )
    set_cap( gid, vsp.CONE_END_CAP, OFFSET, True )
    other = vsp.AddGeom( geom, gid )
    vsp.Update()
    for end, u in ENDS:
        assert vsp.GetParmVal( cap_parm( other, "Cap" + end + "ShearFlag" ) ) == 1.0, end


def testAShearedCapIsSavedAndRead( tmp_path ):
    gid = tube( "FUSELAGE" )
    set_cap( gid, vsp.OGIVE_END_CAP, OFFSET, True )
    before = [ point( gid, i / 20.0, j / NW ) for i in range( 21 ) for j in range( NW + 1 ) ]
    fname = str( tmp_path / "shear.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    gid = vsp.FindGeoms()[0]
    after = [ point( gid, i / 20.0, j / NW ) for i in range( 21 ) for j in range( NW + 1 ) ]
    assert max( math.dist( p, q ) for p, q in zip( before, after ) ) < 1e-12


@pytest.mark.parametrize( "cap_type", SHEARED + ALREADY )
@pytest.mark.parametrize( "typ", TYPES )
def testAShearedCapStaysOnACantedEnd( typ, cap_type ):
    """With the ends turned off square to the axis, the sheared caps meet the body: no step at either seam."""
    gid = tube( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    nxsec = vsp.GetNumXSec( xss )
    for i in ( 0, nxsec - 1 ):
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, i ), "YRotate" ), 25.0 )
    set_cap( gid, cap_type, OFFSET, True )

    # Each cap is one span of u01 beside the body's nxsec - 1.
    du = 1.0 / ( nxsec + 1 )
    for useam in ( du, 1.0 - du ):
        for j in range( NW ):
            p = point( gid, useam - 1e-9, j / NW )
            q = point( gid, useam + 1e-9, j / NW )
            assert math.dist( p, q ) < 1e-5, ( useam, j, p, q )
    assert_no_errors()
