# Placing a Fuselage's or Stack's nose tip at the Geom's origin.
#
# NoseOriginXFlag, NoseOriginYFlag and NoseOriginZFlag translate the surface so the chosen coordinates of
# its nose tip are zero.  The tip is where the nose cap's length and offset place it, and with no cap the
# middle of the end.  The rest of the shape moves with it.
#
# For most caps the tip is the middle of the capped surface's u 0 edge, w 0.25, about which the cap's tip
# curve folds.
#
# A cone's or ogive's sides gather instead at its sphere's pole, and its tip is the point a point cap with
# the same length and offset closes on.

import math
import os

import openvsp as vsp
import pytest

from testhelp import drop_errors

FLAGS = ( "NoseOriginXFlag", "NoseOriginYFlag", "NoseOriginZFlag" )

NOSE_CAPS = (
    vsp.NO_END_CAP,
    vsp.ROUND_END_CAP,
    vsp.SHARP_END_CAP,
    vsp.POINT_END_CAP,
    vsp.CONE_END_CAP,
    vsp.OGIVE_END_CAP,
    vsp.OGIVE_SURF_END_CAP,
)


def body( typ, cap_type ):
    """A body with an elliptical nose section raised and moved sideways, so the tip is off every axis
    once capped."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( typ )
    xss = vsp.GetXSecSurf( gid, 0 )
    vsp.ChangeXSecShape( xss, 0, vsp.XS_ELLIPSE )
    xs = vsp.GetXSec( xss, 0 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "Ellipse_Width" ), 1.0 )
    vsp.SetParmVal( vsp.GetXSecParm( xs, "Ellipse_Height" ), 0.6 )
    if typ == "FUSELAGE":
        vsp.SetParmVal( vsp.GetXSecParm( xs, "YLocPercent" ), 0.02 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "ZLocPercent" ), 0.05 )
    else:
        vsp.SetParmVal( vsp.GetXSecParm( xs, "XDelta" ), 0.7 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "YDelta" ), 0.3 )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "ZDelta" ), 0.5 )
    vsp.SetParmVal( vsp.FindParm( gid, "CapUMinOption", "EndCap" ), cap_type )
    vsp.SetParmVal( vsp.FindParm( gid, "CapUMinOffset", "EndCap" ), 0.2 )
    vsp.Update()
    return gid


def set_flags( gid, flags ):
    for name, val in zip( FLAGS, flags ):
        vsp.SetParmVal( vsp.FindParm( gid, name, "Design" ), val )
    vsp.Update()


def shape( gid ):
    pts = []
    for i in range( 11 ):
        for j in range( 11 ):
            p = vsp.CompPnt01( gid, 0, i / 10.0, j / 10.0 )
            pts.append( ( p.x(), p.y(), p.z() ) )
    return pts


def tip( gid ):
    opt = vsp.FindParm( gid, "CapUMinOption", "EndCap" )
    cap_type = vsp.GetParmVal( opt )
    if cap_type == vsp.NO_END_CAP:
        a = vsp.CompPnt01( gid, 0, 0.0, 0.25 )
        b = vsp.CompPnt01( gid, 0, 0.0, 0.75 )
        return ( 0.5 * ( a.x() + b.x() ), 0.5 * ( a.y() + b.y() ), 0.5 * ( a.z() + b.z() ) )
    if cap_type in ( vsp.CONE_END_CAP, vsp.OGIVE_END_CAP, vsp.OGIVE_SURF_END_CAP ):
        vsp.SetParmVal( opt, vsp.POINT_END_CAP )
        vsp.Update()
        p = vsp.CompPnt01( gid, 0, 0.0, 0.25 )
        vsp.SetParmVal( opt, cap_type )
        vsp.Update()
        return ( p.x(), p.y(), p.z() )
    p = vsp.CompPnt01( gid, 0, 0.0, 0.25 )
    return ( p.x(), p.y(), p.z() )


@pytest.mark.parametrize( "typ", ( "FUSELAGE", "STACK" ) )
def testNoseOriginFlagsDefaultOff( typ ):
    vsp.VSPRenew()
    gid = vsp.AddGeom( typ )
    for name in FLAGS:
        assert not vsp.GetBoolParmVal( vsp.FindParm( gid, name, "Design" ) ), name


@pytest.mark.parametrize( "flags", ( ( True, False, False ), ( False, True, False ), ( False, False, True ),
                                     ( True, True, True ) ) )
@pytest.mark.parametrize( "cap_type", NOSE_CAPS )
@pytest.mark.parametrize( "typ", ( "FUSELAGE", "STACK" ) )
def testNoseTipMovesToTheOrigin( typ, cap_type, flags ):
    gid = body( typ, cap_type )
    before = shape( gid )
    tip0 = tip( gid )
    # An uncapped Fuselage's nose section is at x 0 already.
    assert abs( tip0[0] ) > 1e-3 or ( typ == "FUSELAGE" and cap_type == vsp.NO_END_CAP )
    assert abs( tip0[1] ) > 1e-3 and abs( tip0[2] ) > 1e-3

    set_flags( gid, flags )
    after = shape( gid )
    tip1 = tip( gid )

    for k in range( 3 ):
        if flags[k]:
            assert tip1[k] == pytest.approx( 0.0, abs=1e-12 ), k
        else:
            assert tip1[k] == pytest.approx( tip0[k], abs=1e-12 ), k

    # The whole shape moves with the tip.
    move = [ a - b for a, b in zip( tip1, tip0 ) ]
    for p, q in zip( after, before ):
        assert math.dist( p, [ b + m for b, m in zip( q, move ) ] ) < 1e-12

    set_flags( gid, ( False, False, False ) )
    assert max( math.dist( p, q ) for p, q in zip( shape( gid ), before ) ) < 1e-12


@pytest.mark.parametrize( "typ", ( "FUSELAGE", "STACK" ) )
def testNoseOriginIsTheGeomsOrigin( typ ):
    gid = body( typ, vsp.ROUND_END_CAP )
    vsp.SetParmVal( vsp.FindParm( gid, "X_Rel_Location", "XForm" ), 5.0 )
    vsp.SetParmVal( vsp.FindParm( gid, "Z_Rel_Location", "XForm" ), -2.0 )
    set_flags( gid, ( True, True, True ) )
    assert math.dist( tip( gid ), ( 5.0, 0.0, -2.0 ) ) < 1e-12


def testNoseOriginSurvivesConversionToStack():
    gid = body( "FUSELAGE", vsp.ROUND_END_CAP )
    set_flags( gid, ( True, False, True ) )
    before = shape( gid )

    sid = vsp.ConvertFuselageToStack( gid )
    vsp.Update()

    assert vsp.GetBoolParmVal( vsp.FindParm( sid, "NoseOriginXFlag", "Design" ) )
    assert max( math.dist( p, q ) for p, q in zip( shape( sid ), before ) ) < 1e-9


@pytest.mark.parametrize( "typ", ( "FUSELAGE", "STACK" ) )
def testNoseOriginSurvivesAFile( typ, tmp_path ):
    gid = body( typ, vsp.SHARP_END_CAP )
    set_flags( gid, ( True, False, True ) )
    before = shape( gid )

    fname = os.path.join( str( tmp_path ), "noseorigin.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    gid = vsp.FindGeoms()[0]

    assert vsp.GetBoolParmVal( vsp.FindParm( gid, "NoseOriginZFlag", "Design" ) )
    assert max( math.dist( p, q ) for p, q in zip( shape( gid ), before ) ) < 1e-12


@pytest.mark.parametrize( "cap_type", NOSE_CAPS )
@pytest.mark.parametrize( "typ", ( "FUSELAGE", "STACK" ) )
def testAConformalMovesWithItsParentsNose( typ, cap_type ):
    """A Conformal on a body follows it as its nose moves to the origin: its surface moves by the same
    amount as the body's."""
    gid = body( typ, cap_type )
    cid = vsp.AddGeom( "CONFORMAL", gid )
    vsp.SetParmVal( vsp.FindParm( cid, "Offset", "Design" ), 0.05 )
    vsp.Update()
    before = shape( cid )
    parent_before = shape( gid )
    set_flags( gid, ( True, True, True ) )
    parent_after = shape( gid )
    move = tuple( a - b for a, b in zip( parent_after[0], parent_before[0] ) )
    assert math.sqrt( sum( m * m for m in move ) ) > 0.1
    for p in before:
        moved = vsp.vec3d( p[0] + move[0], p[1] + move[1], p[2] + move[2] )
        d, u, w = vsp.ProjPnt01( cid, 0, moved )
        assert d < 1e-9, p
