# End caps built with scaled rows, as DATCOM builds a nose.
#
# With CapU*ScaledFlag set, each row of a cone or ogive cap is the end scaled about its middle along the cap's
# meridian: the cone or ogive, and its sphere, found for a round end the end's half width across.  On a
# circular end it is the cap without the flag.  On an elliptical end the sphere becomes an ellipsoid as high
# as the end is high to wide, and a truncated cap's face is the end scaled to the radius's share of the half
# width.  A profile cap's rows are scaled anyway; the flag sizes its face by the half width too.

import math

import openvsp as vsp
import pytest

from testhelp import assert_no_errors
from test_OgiveEndCap import ENDS, TYPES, cap_parm, point
from test_ProfileEndCap import gap

LENGTH = 3.0
RADIUS = 0.15

SCALED = ( vsp.CONE_END_CAP, vsp.OGIVE_END_CAP, vsp.OGIVE_SURF_END_CAP )

# name: ( cap type, radius, truncated, angle )
CASES = {
    "sharp cone": ( vsp.CONE_END_CAP, 0.0, False, 0.0 ),
    "blunt cone": ( vsp.CONE_END_CAP, RADIUS, False, 0.0 ),
    "truncated cone": ( vsp.CONE_END_CAP, RADIUS, True, 0.0 ),
    "sharp ogive": ( vsp.OGIVE_END_CAP, 0.0, False, 0.0 ),
    "blunt ogive": ( vsp.OGIVE_END_CAP, RADIUS, False, 0.0 ),
    "blunt secant ogive": ( vsp.OGIVE_END_CAP, RADIUS, False, 4.0 ),
    "truncated ogive": ( vsp.OGIVE_END_CAP, RADIUS, True, 0.0 ),
    "blunt surface ogive": ( vsp.OGIVE_SURF_END_CAP, RADIUS, False, -30.0 ),
}


def body( typ, width, height ):
    """An uncapped Fuselage or Stack of elliptical sections width across and height high on its axis."""
    vsp.VSPRenew()
    gid = vsp.AddGeom( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.ChangeXSecShape( xss, i, vsp.XS_ELLIPSE )
        xs = vsp.GetXSec( xss, i )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "Ellipse_Width" ), width )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "Ellipse_Height" ), height )
        if typ == "STACK":
            for n in ( "YDelta", "ZDelta" ):
                vsp.SetParmVal( vsp.GetXSecParm( xs, n ), 0.0 )
        else:
            for n in ( "YLocPercent", "ZLocPercent" ):
                vsp.SetParmVal( vsp.GetXSecParm( xs, n ), 0.0 )
    vsp.Update()
    return gid


def set_cap( gid, cap_type, radius, trunc, angle, scaled ):
    for n, v in ( ( "Option", cap_type ), ( "RelFlag", False ), ( "LengthDim", LENGTH ), ( "OffsetDim", 0.0 ),
                  ( "RadiusDim", radius ), ( "TruncFlag", trunc ), ( "OgiveAngle", angle ),
                  ( "ScaledFlag", scaled ) ):
        vsp.SetParmVal( cap_parm( gid, "CapUMin" + n ), v )
    vsp.Update()


def nose( gid, ws, n=400 ):
    """Points of the nose cap ahead of the end, x 0."""
    pts = []
    for k in range( n + 1 ):
        for w in ws:
            p = point( gid, 0.5 * k / n, w )
            if p[0] <= 1e-9:
                pts.append( p )
    return pts


@pytest.mark.parametrize( "case", sorted( CASES ) )
@pytest.mark.parametrize( "typ", TYPES )
def testOnACircularEndScaledRowsAreTheCap( typ, case ):
    """On a circular end the cap with scaled rows follows the cap without them to within their fits' accuracy,
    a cubic quarter circle's 2.7e-4 of the sphere's radius at most."""
    cap_type, radius, trunc, angle = CASES[case]
    gid = body( typ, 1.0, 1.0 )
    set_cap( gid, cap_type, radius, trunc, angle, False )
    ref = [ ( p[0], math.hypot( p[1], p[2] ) ) for p in nose( gid, ( 0.0, ), 2000 ) ]
    set_cap( gid, cap_type, radius, trunc, angle, True )
    for p in nose( gid, ( 0.0, 0.1, 0.25 ), 200 ):
        assert gap( ref, ( p[0], math.hypot( p[1], p[2] ) ) ) < 3e-4, p
    assert_no_errors()


@pytest.mark.parametrize( "case", sorted( CASES ) )
@pytest.mark.parametrize( "typ", TYPES )
def testOnAnEllipticalEndEachRowIsTheEndScaled( typ, case ):
    """On an elliptical end 1.2 wide and 0.72 high, every point's height over the end's height to width lies on
    the meridian the plan view shows: each row is the end scaled, so the sphere is an ellipsoid."""
    cap_type, radius, trunc, angle = CASES[case]
    e = 0.6
    gid = body( typ, 1.2, 1.2 * e )
    set_cap( gid, cap_type, radius, trunc, angle, True )
    side = [ ( p[0], abs( p[1] ) ) for p in nose( gid, ( 0.0, ), 2000 ) ]
    for p in nose( gid, ( 0.05, 0.125, 0.2, 0.25 ), 200 ):
        assert gap( side, ( p[0], math.hypot( p[1], p[2] / e ) ) ) < 2e-4, p


@pytest.mark.parametrize( "typ", TYPES )
def testABluntEllipticalNoseIsAnEllipsoid( typ ):
    """A blunt cone's sphere on a 0.6 ellipticity end is RADIUS wide and 0.6 RADIUS high at its front."""
    e = 0.6
    gid = body( typ, 1.2, 1.2 * e )
    set_cap( gid, vsp.CONE_END_CAP, RADIUS, False, 0.0, True )
    tip = point( gid, 0.0, 0.0 )
    assert tip[0] == pytest.approx( -LENGTH, abs=1e-9 )
    for p in nose( gid, ( 0.0, 0.1, 0.25 ), 400 ):
        if p[0] < -LENGTH + 0.2 * RADIUS:
            r = math.sqrt( ( p[0] + LENGTH - RADIUS ) ** 2 + p[1] ** 2 + ( p[2] / e ) ** 2 )
            assert r == pytest.approx( RADIUS, abs=2e-4 ), p


@pytest.mark.parametrize( "typ", TYPES )
def testATruncatedEllipticalNosesFaceIsTheEndScaledToTheHalfWidth( typ ):
    e = 0.6
    gid = body( typ, 1.2, 1.2 * e )
    set_cap( gid, vsp.OGIVE_END_CAP, RADIUS, True, 0.0, True )
    face = [ p for p in nose( gid, ( 0.0, 0.125, 0.25 ), 400 ) if abs( p[0] + LENGTH ) < 1e-9 ]
    assert max( abs( p[1] ) for p in face ) == pytest.approx( RADIUS, abs=2e-4 )
    assert max( abs( p[2] ) for p in face ) == pytest.approx( RADIUS * e, abs=2e-4 )


@pytest.mark.parametrize( "cap_type", ( vsp.FLAT_END_CAP, vsp.ROUND_END_CAP, vsp.POINT_END_CAP,
                                        vsp.POWER_LAW_END_CAP ) )
def testTheFlagLeavesOtherCapsAlone( cap_type ):
    gid = body( "STACK", 1.2, 0.72 )
    set_cap( gid, cap_type, RADIUS, False, 0.0, False )
    before = nose( gid, ( 0.0, 0.1, 0.25 ), 100 )
    set_cap( gid, cap_type, RADIUS, False, 0.0, True )
    assert max( math.dist( p, q ) for p, q in zip( before, nose( gid, ( 0.0, 0.1, 0.25 ), 100 ) ) ) < 1e-12


@pytest.mark.parametrize( "geom", ( "CLONE", "CONFORMAL" ) )
def testACloneAndAConformalTakeScaledRows( geom ):
    gid = body( "STACK", 1.2, 0.72 )
    set_cap( gid, vsp.CONE_END_CAP, RADIUS, False, 0.0, True )
    other = vsp.AddGeom( geom, gid )
    vsp.Update()
    assert vsp.GetParmVal( cap_parm( other, "CapUMinScaledFlag" ) ) == 1.0


def testScaledRowsAreSavedAndRead( tmp_path ):
    gid = body( "FUSELAGE", 1.2, 0.72 )
    set_cap( gid, vsp.OGIVE_END_CAP, RADIUS, False, 0.0, True )
    before = nose( gid, ( 0.0, 0.1, 0.25 ), 100 )
    fname = str( tmp_path / "scaled.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    gid = vsp.FindGeoms()[0]
    assert max( math.dist( p, q ) for p, q in zip( before, nose( gid, ( 0.0, 0.1, 0.25 ), 100 ) ) ) < 1e-12



@pytest.mark.parametrize( "cap_type", SCALED )
def testTheTessellationStartsAtTheCapsPoint( cap_type ):
    gid = body( "STACK", 1.2, 0.72 )
    set_cap( gid, cap_type, RADIUS, False, 0.0, True )
    u, w = vsp.GetUWTess01( gid, 0 )
    assert u[0] == 0.0
    assert point( gid, u[0], 0.0 )[0] == pytest.approx( -LENGTH, abs=1e-9 )


@pytest.mark.parametrize( "length", ( 0.8, 1.0, 1.2, 1.4, 1.6 ) )
@pytest.mark.parametrize( "typ", TYPES )
def testAShortOgiveClosesWithoutItsSphere( typ, length ):
    """An ogive short enough to reach the axis before its sphere would touch it closes there, with no sphere: the
    nose stays within the end, and near the length.  Longer, it touches the sphere and reaches the point."""
    gid = body( typ, 3.0, 1.5 )
    for n, v in ( ( "Option", vsp.OGIVE_END_CAP ), ( "RelFlag", False ), ( "LengthDim", length ),
                  ( "OffsetDim", 0.0 ), ( "RadiusDim", 0.3 ), ( "TruncFlag", False ), ( "OgiveAngle", 10.0 ),
                  ( "ScaledFlag", True ) ):
        vsp.SetParmVal( cap_parm( gid, "CapUMin" + n ), v )
    vsp.Update()
    pts = nose( gid, [ k / 32.0 for k in range( 32 ) ] )
    xmin = min( p[0] for p in pts )
    assert -1.2 * length < xmin < -0.9 * length
    if length >= 1.4:
        assert xmin == pytest.approx( -length, abs=1e-6 )
    assert max( abs( p[1] ) for p in pts ) < 1.5 + 1e-6
    assert max( abs( p[2] ) for p in pts ) < 0.75 + 1e-6
    assert_no_errors()
