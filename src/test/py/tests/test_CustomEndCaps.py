# End caps and the nose at the origin on a Custom Geom.
#
# A Custom Geom takes every end cap type, caps every main surface its script makes, sizes the caps in
# Rel or Abs units as other Geoms do, and has NoseOriginXFlag, NoseOriginYFlag and NoseOriginZFlag, as a
# Fuselage and Stack do.  The nose tip is the first main surface's, and every main surface moves with it.
#
# The Geom is CustomScripts/EndCapTube.vsppart, an open tube whose nose section is raised and moved
# sideways.  OpenVSP reads ./CustomScripts when it starts, so these tests run from src/test/py.

import math
import os

import openvsp as vsp
import pytest

from testhelp import drop_errors, assert_no_errors

CUSTOM = "EndCapTube"

FLAGS = ( "NoseOriginXFlag", "NoseOriginYFlag", "NoseOriginZFlag" )
ENDS = ( ( "UMin", 0.0 ), ( "UMax", 1.0 ) )

ALL_CAPS = tuple( range( vsp.NUM_END_CAP_OPTIONS ) )
CLOSING_CAPS = tuple( t for t in ALL_CAPS if t != vsp.NO_END_CAP )
LENGTH_CAPS = ( vsp.ROUND_END_CAP, vsp.EDGE_END_CAP, vsp.SHARP_END_CAP, vsp.POINT_END_CAP, vsp.CONE_END_CAP )
NOSE_CAPS = ( vsp.NO_END_CAP, vsp.ROUND_END_CAP, vsp.SHARP_END_CAP, vsp.POINT_END_CAP, vsp.CONE_END_CAP )


def tube( ncopy=0 ):
    vsp.VSPRenew()
    drop_errors()
    assert CUSTOM in vsp.GetGeomTypes(), "run from src/test/py, where CustomScripts/%s.vsppart is read" % CUSTOM
    gid = vsp.AddGeom( CUSTOM )
    vsp.SetParmVal( vsp.FindParm( gid, "NumCopy", "Design" ), ncopy )
    vsp.Update()
    return gid


def cap_parm( gid, name ):
    return vsp.FindParm( gid, name, "EndCap" )


def set_cap( gid, name, val ):
    vsp.SetParmVal( cap_parm( gid, name ), val )


def get_cap( gid, name ):
    return vsp.GetParmVal( cap_parm( gid, name ) )


def set_flags( gid, flags ):
    for name, val in zip( FLAGS, flags ):
        vsp.SetParmVal( vsp.FindParm( gid, name, "Design" ), val )
    vsp.Update()


def point( gid, isurf, u, w ):
    p = vsp.CompPnt01( gid, isurf, u, w )
    return ( p.x(), p.y(), p.z() )


def shape( gid ):
    pts = []
    for isurf in range( vsp.GetNumMainSurfs( gid ) ):
        for i in range( 11 ):
            for j in range( 11 ):
                pts.append( point( gid, isurf, i / 10.0, j / 10.0 ) )
    return pts


def max_move( a, b ):
    return max( math.dist( p, q ) for p, q in zip( a, b ) )


def tip( gid ):
    """The nose tip of the first main surface: the middle of the end with no cap, the point a point cap
    with the same length and offset closes on for a cone, and otherwise the middle of the u 0 edge."""
    cap_type = get_cap( gid, "CapUMinOption" )
    if cap_type == vsp.NO_END_CAP:
        a = point( gid, 0, 0.0, 0.25 )
        b = point( gid, 0, 0.0, 0.75 )
        return tuple( 0.5 * ( p + q ) for p, q in zip( a, b ) )
    if cap_type == vsp.CONE_END_CAP:
        set_cap( gid, "CapUMinOption", vsp.POINT_END_CAP )
        vsp.Update()
        p = point( gid, 0, 0.0, 0.25 )
        set_cap( gid, "CapUMinOption", vsp.CONE_END_CAP )
        vsp.Update()
        return p
    return point( gid, 0, 0.0, 0.25 )


def testTheTubeIsOpen():
    gid = tube()
    for end, u in ENDS:
        assert get_cap( gid, "Cap" + end + "Option" ) == vsp.NO_END_CAP
        assert math.dist( point( gid, 0, u, 0.0 ), point( gid, 0, u, 0.5 ) ) > 0.5, end


@pytest.mark.parametrize( "cap_type", ALL_CAPS )
def testACustomGeomTakesEveryCapType( cap_type ):
    gid = tube()
    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "Option", cap_type )
        assert get_cap( gid, "Cap" + end + "Option" ) == cap_type, end


def testACustomGeomRefusesAnUnknownCapType():
    gid = tube()
    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "Option", vsp.ROUND_END_CAP )
        set_cap( gid, "Cap" + end + "Option", vsp.NUM_END_CAP_OPTIONS )
        assert get_cap( gid, "Cap" + end + "Option" ) == vsp.ROUND_END_CAP, end


@pytest.mark.parametrize( "cap_type", CLOSING_CAPS )
def testEveryMainSurfaceIsCapped( cap_type ):
    gid = tube( 2 )
    assert vsp.GetNumMainSurfs( gid ) == 3
    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "Option", cap_type )
    vsp.Update()
    for isurf in range( 3 ):
        for end, u in ENDS:
            # A capped end closes on its tip curve, which its upper and lower halves share.
            for w in ( 0.1, 0.2, 0.3, 0.4 ):
                assert math.dist( point( gid, isurf, u, w ), point( gid, isurf, u, 1.0 - w ) ) < 1e-9, ( isurf, end, w )
    # The copies keep their places.
    for isurf in ( 1, 2 ):
        for p, q in zip( shape( gid )[ 121 * isurf : 121 * ( isurf + 1 ) ], shape( gid )[ 0 : 121 ] ):
            assert math.dist( p, ( q[0], q[1] + 4.0 * isurf, q[2] ) ) < 1e-9


def comp_geom_vol( gid ):
    """The volume CompGeom finds inside the Geom's tessellation, which must be closed."""
    mid = vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    rid = vsp.FindLatestResultsID( "Comp_Geom" )
    assert vsp.GetIntResults( rid, "Num_Open_Meshes_Removed" )[0] == 0
    assert vsp.GetIntResults( rid, "Num_Open_Meshes_Merged" )[0] == 0
    vol = vsp.GetDoubleResults( rid, "Wet_Vol" )[0]
    vsp.DeleteGeom( mid )
    vsp.Update()
    return vol


@pytest.mark.parametrize( "cap_type", ( vsp.ROUND_END_CAP, vsp.CONE_END_CAP, vsp.ROUND_EXT_END_CAP_BOTH ) )
def testACappedCustomGeomTessellatesClosed( cap_type ):
    gid = tube()
    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "Option", vsp.FLAT_END_CAP )
    vsp.Update()
    flat = comp_geom_vol( gid )

    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "Option", cap_type )
    vsp.Update()
    # Caps that reach past the ends hold more than the tube shut flat.
    assert comp_geom_vol( gid ) > flat > 0.0


@pytest.mark.parametrize( "cap_type", LENGTH_CAPS )
def testSwitchingToDimensionalKeepsTheShape( cap_type ):
    gid = tube()
    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "Option", cap_type )
        set_cap( gid, "Cap" + end + "Length", 0.8 )
        set_cap( gid, "Cap" + end + "Offset", 0.1 )
    vsp.Update()
    before = shape( gid )

    for end, u in ENDS:
        assert get_cap( gid, "Cap" + end + "LengthDim" ) > 0.0, end
        set_cap( gid, "Cap" + end + "RelFlag", False )
    vsp.Update()

    assert max_move( before, shape( gid ) ) < 1e-12
    for end, u in ENDS:
        assert get_cap( gid, "Cap" + end + "Length" ) == pytest.approx( 0.8, abs=1e-12 ), end
        assert get_cap( gid, "Cap" + end + "Offset" ) == pytest.approx( 0.1, abs=1e-12 ), end


@pytest.mark.parametrize( "cap_type", LENGTH_CAPS )
def testDimensionalSizesDriveTheCap( cap_type ):
    """An Abs cap's dimensional length, offset and radius give the shape the Rel cap they convert to
    gives."""
    gid = tube()
    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "Option", cap_type )
        set_cap( gid, "Cap" + end + "RelFlag", False )
        set_cap( gid, "Cap" + end + "LengthDim", 0.4 )
        set_cap( gid, "Cap" + end + "OffsetDim", 0.05 )
        set_cap( gid, "Cap" + end + "RadiusDim", 0.1 )
    vsp.Update()
    before = shape( gid )
    nondim = {}
    for end, u in ENDS:
        assert get_cap( gid, "Cap" + end + "LengthDim" ) == pytest.approx( 0.4, abs=1e-12 ), end
        nondim[end] = [ get_cap( gid, "Cap" + end + name ) for name in ( "Length", "Offset", "Radius" ) ]

    gid = tube()
    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "Option", cap_type )
        for name, val in zip( ( "Length", "Offset", "Radius" ), nondim[end] ):
            set_cap( gid, "Cap" + end + name, val )
    vsp.Update()

    assert max_move( before, shape( gid ) ) < 1e-9
    for end, u in ENDS:
        assert get_cap( gid, "Cap" + end + "LengthDim" ) == pytest.approx( 0.4, abs=1e-9 ), end
        if cap_type == vsp.CONE_END_CAP:
            assert get_cap( gid, "Cap" + end + "RadiusDim" ) == pytest.approx( 0.1, abs=1e-9 ), end


def testNoseOriginFlagsDefaultOff():
    gid = tube()
    for name in FLAGS:
        assert not vsp.GetBoolParmVal( vsp.FindParm( gid, name, "Design" ) ), name


@pytest.mark.parametrize( "flags", ( ( True, False, False ), ( False, True, False ), ( False, False, True ),
                                     ( True, True, True ) ) )
@pytest.mark.parametrize( "cap_type", NOSE_CAPS )
def testNoseTipMovesToTheOrigin( cap_type, flags ):
    gid = tube()
    set_cap( gid, "CapUMinOption", cap_type )
    set_cap( gid, "CapUMinOffset", 0.2 )
    vsp.Update()
    before = shape( gid )
    tip0 = tip( gid )
    assert abs( tip0[1] ) > 1e-3 and abs( tip0[2] ) > 1e-3
    assert abs( tip0[0] ) > 1e-3 or cap_type == vsp.NO_END_CAP

    set_flags( gid, flags )
    after = shape( gid )
    tip1 = tip( gid )

    for k in range( 3 ):
        if flags[k]:
            assert tip1[k] == pytest.approx( 0.0, abs=1e-12 ), k
        else:
            assert tip1[k] == pytest.approx( tip0[k], abs=1e-12 ), k

    move = [ a - b for a, b in zip( tip1, tip0 ) ]
    for p, q in zip( after, before ):
        assert math.dist( p, [ b + m for b, m in zip( q, move ) ] ) < 1e-12

    set_flags( gid, ( False, False, False ) )
    assert max_move( shape( gid ), before ) < 1e-12


@pytest.mark.parametrize( "cap_type", ( vsp.ROUND_END_CAP, vsp.CONE_END_CAP ) )
def testEveryMainSurfaceMovesWithTheNose( cap_type ):
    gid = tube( 2 )
    set_cap( gid, "CapUMinOption", cap_type )
    vsp.Update()
    before = shape( gid )
    tip0 = tip( gid )

    set_flags( gid, ( True, True, True ) )
    assert math.dist( tip( gid ), ( 0.0, 0.0, 0.0 ) ) < 1e-12

    for p, q in zip( shape( gid ), before ):
        assert math.dist( p, [ b - t for b, t in zip( q, tip0 ) ] ) < 1e-12


def testNoseOriginIsTheGeomsOrigin():
    gid = tube()
    set_cap( gid, "CapUMinOption", vsp.ROUND_END_CAP )
    vsp.SetParmVal( vsp.FindParm( gid, "X_Rel_Location", "XForm" ), 5.0 )
    vsp.SetParmVal( vsp.FindParm( gid, "Z_Rel_Location", "XForm" ), -2.0 )
    set_flags( gid, ( True, True, True ) )
    assert math.dist( tip( gid ), ( 5.0, 0.0, -2.0 ) ) < 1e-12


def testCustomEndCapsSurviveAFile( tmp_path ):
    gid = tube( 1 )
    set_cap( gid, "CapUMinOption", vsp.CONE_END_CAP )
    set_cap( gid, "CapUMinRelFlag", False )
    set_cap( gid, "CapUMinLengthDim", 0.5 )
    set_cap( gid, "CapUMinRadiusDim", 0.1 )
    set_cap( gid, "CapUMaxOption", vsp.SHARP_END_CAP )
    set_flags( gid, ( True, False, True ) )
    before = shape( gid )

    fname = os.path.join( str( tmp_path ), "customendcaps.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    gid = vsp.FindGeoms()[0]

    assert get_cap( gid, "CapUMinOption" ) == vsp.CONE_END_CAP
    assert not get_cap( gid, "CapUMinRelFlag" )
    assert vsp.GetBoolParmVal( vsp.FindParm( gid, "NoseOriginZFlag", "Design" ) )
    assert not vsp.GetBoolParmVal( vsp.FindParm( gid, "NoseOriginYFlag", "Design" ) )
    assert max_move( before, shape( gid ) ) < 1e-12


# Caps measured in the frame the surface is built in.
FRAME_CAPS = ( vsp.POINT_END_CAP, vsp.CONE_END_CAP, vsp.OGIVE_END_CAP, vsp.POWER_LAW_END_CAP )


def turned( p, deg ):
    c = math.cos( math.radians( deg ) )
    s = math.sin( math.radians( deg ) )
    return ( c * p[0] - s * p[1], s * p[0] + c * p[1], p[2] )


def frame_caps( gid, cap_type ):
    for end, u in ENDS:
        set_cap( gid, "Cap" + end + "Option", cap_type )
        set_cap( gid, "Cap" + end + "Length", 1.5 )
        set_cap( gid, "Cap" + end + "Offset", 0.2 )
        set_cap( gid, "Cap" + end + "Radius", 0.2 )
    vsp.Update()


def surface( gid, isurf ):
    return [ point( gid, isurf, i / 20.0, j / 16.0 ) for i in range( 21 ) for j in range( 17 ) ]


@pytest.mark.parametrize( "cap_type", FRAME_CAPS )
def testATiltedTubesCapsTurnWithIt( cap_type ):
    """Turning the cross section surface turns the frame its caps are measured in: the capped tube tilted is
    the capped tube turned."""
    gid = tube()
    frame_caps( gid, cap_type )
    flat = surface( gid, 0 )
    vsp.SetParmVal( vsp.FindParm( gid, "Tilt", "Design" ), 30.0 )
    vsp.Update()
    tilted = surface( gid, 0 )
    assert max( math.dist( turned( p, 30.0 ), q ) for p, q in zip( flat, tilted ) ) < 1e-9
    assert_no_errors()


@pytest.mark.parametrize( "cap_type", FRAME_CAPS )
def testATurnedCopysCapsTurnWithIt( cap_type ):
    """A copy cloned turned has its frame turned with it, so its caps are the original's turned and moved."""
    gid = tube( 1 )
    vsp.SetParmVal( vsp.FindParm( gid, "Turn", "Design" ), 40.0 )
    frame_caps( gid, cap_type )
    orig = surface( gid, 0 )
    copy = surface( gid, 1 )
    for p, q in zip( orig, copy ):
        r = turned( p, 40.0 )
        assert math.dist( ( r[0], r[1] + 4.0, r[2] ), q ) < 1e-9
    assert_no_errors()
