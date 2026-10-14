"""Propeller blade hinges.  Each turns the blade about its own line, and a blade given its own
angle is placed exactly as blade 1 would be with that angle, turned to its azimuth.
"""

import math
import os
import tempfile

import openvsp as vsp

from prophelp import group

UW = [ ( u, w ) for u in ( 0.05, 0.3, 0.6, 0.95 ) for w in ( 0.0, 0.2, 0.5, 0.7, 0.9 ) ]
TOL = 1e-9


def new_prop( nblade, parms, rev = False ):
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( pid, "UseBeta34Flag", "Design", 0.0 )
    vsp.SetParmVal( pid, "ReverseFlag", "Design", float( rev ) )
    vsp.SetParmVal( pid, "NumBlade", "Design", nblade )
    vsp.Update()
    set_parms( pid, parms )
    return pid


def set_parms( pid, parms ):
    for name, val in parms.items():
        assert vsp.FindParm( pid, name, group( name ) ), name
        vsp.SetParmVal( pid, name, group( name ), val )
    vsp.Update()


def blade( pid, surf ):
    pts = []
    for u, w in UW:
        p = vsp.CompPnt01( pid, surf, u, w )
        pts.append( ( p.x(), p.y(), p.z() ) )
    return pts


def turn( pts, deg ):
    """Turn points about the propeller axis the way blade azimuth does."""
    c = math.cos( math.radians( deg ) )
    s = math.sin( math.radians( deg ) )
    return [ ( x, c * y + s * z, -s * y + c * z ) for x, y, z in pts ]


def furthest( a, b ):
    return max( math.dist( p, q ) for p, q in zip( a, b ) )


def blade_as_first( nblade, iblade, common, own, rev = False ):
    """Distance between blade iblade (from 1) given its own angles and blade 1 given them."""
    sgn = -1.0 if rev else 1.0
    pid = new_prop( nblade, dict( common, **own[0] ), rev )
    got = blade( pid, iblade - 1 )
    az = vsp.GetParmVal( pid, "BladeAz_%d" % iblade, "Design" )
    pid = new_prop( nblade, dict( common, **own[1] ), rev )
    want = turn( blade( pid, 0 ), sgn * az )
    return furthest( got, want )


def test_individual_feather():
    common = { "Feather": 5.0, "FeatherAxisXoC": 0.3, "FeatherOffsetXoC": 0.1, "Precone": 4.0 }
    for rev in ( False, True ):
        d = blade_as_first( 3, 2, common,
                            ( { "BladeMotionMode": 1.0, "Feather_1": 17.0 }, { "Feather": 17.0 } ),
                            rev )
        assert d < TOL, rev


def test_individual_feather_and_fold():
    common = { "Feather": 5.0, "FoldAngle": 10.0, "AzFoldDir": 12.0, "ElFoldDir": -8.0, "OffFoldAx": 0.05 }
    own = ( { "BladeMotionMode": 1.0, "Feather_2": -9.0,
              "IndividualBladeFoldFlag": 1.0, "FoldAngle_2": 40.0 },
            { "Feather": -9.0, "FoldAngle": 40.0 } )
    assert blade_as_first( 4, 3, common, own ) < TOL


def test_other_blades_follow_feather():
    pid = new_prop( 3, { "BladeMotionMode": 1.0, "Feather_1": 17.0 } )
    assert vsp.GetParmVal( pid, "Feather_2", "Articulation" ) == 0.0
    set_parms( pid, { "BladeMotionMode": 0.0, "Feather": 8.0 } )
    assert vsp.GetParmVal( pid, "Feather_1", "Articulation" ) == 8.0
    assert vsp.GetParmVal( pid, "Feather_2", "Articulation" ) == 8.0


def test_beta34_drives_all_blades():
    # While Beta34 drives, every blade takes the feather Beta34 gives; the blades' own angles
    # are kept for when the feather angle drives again.
    assert blade_as_first( 3, 2, { "UseBeta34Flag": 1.0 },
                           ( { "BladeMotionMode": 1.0, "Feather_1": 17.0 }, {} ) ) < TOL
    pid = new_prop( 3, { "BladeMotionMode": 1.0, "Feather_1": 17.0, "Feather": 4.0 } )
    own = blade( pid, 1 )
    set_parms( pid, { "UseBeta34Flag": 1.0 } )
    assert vsp.GetParmVal( pid, "BladeMotionMode", "Articulation" ) == 1.0
    assert vsp.GetParmVal( pid, "Feather_1", "Articulation" ) == 17.0
    set_parms( pid, { "UseBeta34Flag": 0.0 } )
    assert furthest( blade( pid, 1 ), own ) < TOL


def test_individual_feather_round_trip():
    pid = new_prop( 3, { "BladeMotionMode": 1.0, "Feather_1": 17.0, "Feather_2": -3.0 } )
    want = [ blade( pid, s ) for s in range( 3 ) ]
    path = os.path.join( tempfile.mkdtemp(), "feather.vsp3" )
    vsp.WriteVSPFile( path )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()
    pid = vsp.FindGeoms()[0]
    assert vsp.GetParmVal( pid, "Feather_2", "Articulation" ) == -3.0
    assert max( furthest( blade( pid, s ), want[s] ) for s in range( 3 ) ) < TOL


def hinge_turn_error( pid, unflapped, origin, axis ):
    """How far the blade is from a rigid turn of unflapped about the line through origin along
    axis: every point must keep its distance from the line and its place along it."""
    n = math.sqrt( sum( a * a for a in axis ) )
    axis = [ a / n for a in axis ]
    worst = 0.0
    for p, q in zip( blade( pid, 0 ), unflapped ):
        dp = [ a - o for a, o in zip( p, origin ) ]
        dq = [ a - o for a, o in zip( q, origin ) ]
        along_p = sum( a * b for a, b in zip( dp, axis ) )
        along_q = sum( a * b for a, b in zip( dq, axis ) )
        worst = max( worst, abs( along_p - along_q ), abs( math.hypot( *dp ) - math.hypot( *dq ) ) )
    return worst


def test_flap_turns_about_its_hinge():
    radius = 15.0
    hinge = { "RFlapAx": 0.1, "OffFlapAx": 0.02, "AxFlapAx": 0.03 }
    pid = new_prop( 3, dict( hinge, Feather = 10.0 ) )
    unflapped = blade( pid, 0 )
    set_parms( pid, { "FlapAngle": 25.0 } )
    origin = ( 0.03 * radius, 0.1 * radius, 0.02 * radius )
    assert hinge_turn_error( pid, unflapped, origin, ( 0.0, 0.0, 1.0 ) ) < TOL
    assert max( math.dist( p, q ) for p, q in zip( blade( pid, 0 ), unflapped ) ) > 1.0


def test_individual_flap():
    common = { "FlapAngle": 3.0, "RFlapAx": 0.08, "ElFlapDir": 15.0, "AzFlapDir": -5.0,
               "Feather": 7.0, "FoldAngle": 12.0 }
    own = ( { "BladeMotionMode": 1.0, "FlapAngle_2": -6.0, "Feather_2": 11.0 },
            { "FlapAngle": -6.0, "Feather": 11.0 } )
    for rev in ( False, True ):
        assert blade_as_first( 4, 3, common, own, rev ) < TOL, rev


def test_other_blades_follow_flap():
    pid = new_prop( 3, { "BladeMotionMode": 1.0, "FlapAngle_1": 9.0 } )
    assert vsp.GetParmVal( pid, "FlapAngle_2", "Articulation" ) == 0.0
    set_parms( pid, { "BladeMotionMode": 0.0, "FlapAngle": 4.0 } )
    assert vsp.GetParmVal( pid, "FlapAngle_1", "Articulation" ) == 4.0
    assert vsp.GetParmVal( pid, "FlapAngle_2", "Articulation" ) == 4.0


def pitch( pid, surf = 0, u = 0.6 ):
    """Pitch of a section, degrees: how far its chord, trailing edge to leading edge, points
    toward the thrust (-x)."""
    le = vsp.CompPnt01( pid, surf, u, 0.5 )
    te = vsp.CompPnt01( pid, surf, u, 0.0 )
    c = ( le.x() - te.x(), le.y() - te.y(), le.z() - te.z() )
    return math.degrees( math.asin( -c[0] / math.hypot( *c ) ) )


def tip( pid ):
    p = vsp.CompPnt01( pid, 0, 1.0, 0.25 )
    return ( p.x(), p.y(), p.z() )


def test_flap_up_is_toward_thrust():
    for rev in ( False, True ):
        pid = new_prop( 2, {}, rev )
        before = tip( pid )
        set_parms( pid, { "FlapAngle": 10.0 } )
        assert tip( pid )[0] < before[0] - 1.0, rev


def test_flap_azimuth_pitches_the_hinge_like_the_blade():
    # The hinge axis, turned 30 degrees nose up, carries a flapping tip aft as it rises.
    for rev in ( False, True ):
        aft = -1.0 if rev else 1.0
        pid = new_prop( 2, { "AzFlapDir": 30.0, "RFlapAx": 0.0 }, rev )
        before = tip( pid )
        set_parms( pid, { "FlapAngle": 1.0 } )
        after = tip( pid )
        rise = before[0] - after[0]
        back = aft * ( after[2] - before[2] )
        assert rise > 0.0 and back > 0.0, rev
        assert abs( back / rise - math.tan( math.radians( 30.0 ) ) ) < 0.02, rev


def test_flap_elevation_is_delta3():
    # tan( delta_3 ) is the pitch-flap coupling, positive for flap up, pitch down.
    for rev in ( False, True ):
        pid = new_prop( 2, { "FlapAngle": 10.0 }, rev )
        plain = pitch( pid )
        set_parms( pid, { "ElFlapDir": 30.0 } )
        coupled = pitch( pid )
        assert abs( ( coupled - plain ) + 10.0 * math.sin( math.radians( 30.0 ) ) ) < 0.5, rev


def test_lag_turns_about_its_hinge():
    radius = 15.0
    hinge = { "RLagAx": 0.12, "OffLagAx": 0.02, "AxLagAx": 0.03 }
    pid = new_prop( 3, dict( hinge, Feather = 10.0 ) )
    unlagged = blade( pid, 0 )
    set_parms( pid, { "LagAngle": 15.0 } )
    origin = ( 0.03 * radius, 0.12 * radius, 0.02 * radius )
    assert hinge_turn_error( pid, unlagged, origin, ( 1.0, 0.0, 0.0 ) ) < TOL
    assert max( math.dist( p, q ) for p, q in zip( blade( pid, 0 ), unlagged ) ) > 1.0


def test_individual_lag():
    common = { "LagAngle": 2.0, "RLagAx": 0.1, "CantLagDir": 12.0, "AzLagDir": 5.0,
               "FlapAngle": 3.0, "Feather": 7.0, "FoldAngle": 12.0 }
    # One mode gives every hinge its own angles, so blade 3's feather is given as well.
    own = ( { "BladeMotionMode": 1.0, "LagAngle_2": -7.0, "FlapAngle_2": 6.0, "Feather_2": 7.0 },
            { "LagAngle": -7.0, "FlapAngle": 6.0 } )
    for rev in ( False, True ):
        assert blade_as_first( 4, 3, common, own, rev ) < TOL, rev


def test_other_blades_follow_lag():
    pid = new_prop( 3, { "BladeMotionMode": 1.0, "LagAngle_1": 9.0 } )
    assert vsp.GetParmVal( pid, "LagAngle_2", "Articulation" ) == 0.0
    set_parms( pid, { "BladeMotionMode": 0.0, "LagAngle": 4.0 } )
    assert vsp.GetParmVal( pid, "LagAngle_1", "Articulation" ) == 4.0
    assert vsp.GetParmVal( pid, "LagAngle_2", "Articulation" ) == 4.0


def test_lag_is_aft():
    for rev in ( False, True ):
        aft = -1.0 if rev else 1.0
        pid = new_prop( 2, {}, rev )
        before = tip( pid )
        set_parms( pid, { "LagAngle": 10.0 } )
        assert aft * ( tip( pid )[2] - before[2] ) > 1.0, rev


def test_lag_azimuth_pitches_the_hinge_like_the_blade():
    # The hinge axis, turned 30 degrees nose up, carries a lagging tip downstream as it goes aft.
    for rev in ( False, True ):
        aft = -1.0 if rev else 1.0
        pid = new_prop( 2, { "AzLagDir": 30.0, "RLagAx": 0.0 }, rev )
        before = tip( pid )
        set_parms( pid, { "LagAngle": 1.0 } )
        after = tip( pid )
        back = aft * ( after[2] - before[2] )
        down = after[0] - before[0]
        assert back > 0.0 and down > 0.0, rev
        assert abs( down / back - math.tan( math.radians( 30.0 ) ) ) < 0.02, rev


def test_lag_cant_is_alpha1():
    # tan( alpha_1 ) is the pitch-lag coupling, positive for lag back, pitch down.
    for rev in ( False, True ):
        pid = new_prop( 2, { "LagAngle": 10.0 }, rev )
        plain = pitch( pid )
        set_parms( pid, { "CantLagDir": 30.0 } )
        coupled = pitch( pid )
        assert abs( ( coupled - plain ) + 10.0 * math.sin( math.radians( 30.0 ) ) ) < 0.5, rev


def turn_about( pts, origin, axis, deg ):
    """Right-handed turn of points about the line through origin along a unit axis."""
    c = math.cos( math.radians( deg ) )
    s = math.sin( math.radians( deg ) )
    out = []
    for p in pts:
        v = [ a - o for a, o in zip( p, origin ) ]
        k = axis
        kxv = ( k[1] * v[2] - k[2] * v[1], k[2] * v[0] - k[0] * v[2], k[0] * v[1] - k[1] * v[0] )
        kv = sum( a * b for a, b in zip( k, v ) )
        r = [ v[i] * c + kxv[i] * s + k[i] * kv * ( 1.0 - c ) for i in range( 3 ) ]
        out.append( tuple( a + o for a, o in zip( r, origin ) ) )
    return out


def test_inboard_hinge_carries_outboard_hinge():
    # Feather at 15% is inboard of fold at 20%, so the feather turns the folded blade, fold
    # hinge and all, about the radial axis.
    pid = new_prop( 2, { "FoldAngle": 30.0, "RFeatherAx": 0.15, "RFoldAx": 0.2 } )
    folded = blade( pid, 0 )
    set_parms( pid, { "Feather": 20.0 } )
    want = turn_about( folded, ( 0.0, 0.0, 0.0 ), ( 0.0, 1.0, 0.0 ), 20.0 )
    assert furthest( blade( pid, 0 ), want ) < TOL


def test_hinges_at_one_place_flap_first():
    # Flap and lag hinges at the same radius: the flap is inboard of the lag.
    radius = 15.0
    hinge = { "RFlapAx": 0.1, "RLagAx": 0.1, "OffFlapAx": 0.01, "OffLagAx": -0.02 }
    pid = new_prop( 2, dict( hinge, LagAngle = 12.0 ) )
    lagged = blade( pid, 0 )
    set_parms( pid, { "FlapAngle": 9.0 } )
    want = turn_about( lagged, ( 0.0, 0.1 * radius, 0.01 * radius ), ( 0.0, 0.0, 1.0 ), 9.0 )
    assert furthest( blade( pid, 0 ), want ) < TOL
    # Moving the lag inboard reverses the order.
    set_parms( pid, { "RLagAx": 0.05 } )
    assert furthest( blade( pid, 0 ), want ) > 1e-3


# Points on blades 1 and 2, ( surf, u, w, x, y, z ), written by OpenVSP 3.55.0 for propellers made
# by the API with the parms in golden_case.
GOLDEN_355 = {
    'folded' : [(0, 0.1, 0.0, 0.102068687788, 3.018107222716, -0.474788340638), (0, 0.5, 0.25, 1.656669496648, 8.144322324634, -3.241483313763), (0, 0.9, 0.5, 3.365604326194, 13.01710766814, -6.445782368072), (0, 1.0, 0.8, 3.592971548255, 13.43416522963, -5.386769546626), (1, 0.1, 0.0, 0.304381804099, -2.10257858117, -2.381041830175), (1, 0.5, 0.25, -2.72199022689, -5.863032844569, -5.993454055169), (1, 0.9, 0.5, -5.789719877441, -9.99227121414, -9.229261985713), (1, 1.0, 0.8, -5.205272801517, -9.504396210717, -10.105355939423)] ,
    'reversed' : [(0, 0.1, 0.0, -0.252769215708, 2.616872624342, -1.19062207651), (0, 0.5, 0.25, 1.358260914804, 8.310992331167, -2.344332916919), (0, 0.9, 0.5, 3.042213656354, 14.114835094232, -2.986002637898), (0, 1.0, 0.8, 3.442239445262, 13.718962495754, -4.001091438561), (1, 0.1, 0.0, -0.252769215708, -0.277327347607, 2.861589209403), (1, 0.5, 0.25, 1.358260914804, -2.125244304603, 8.369696947908), (1, 0.9, 0.5, 3.042213656354, -4.471463406929, 13.716807080782), (1, 1.0, 0.8, 3.442239445262, -3.394434419219, 13.88151575417)] ,
}


def golden_case( name ):
    if name == "folded":
        return False, { "Precone": 8.0, "Rotate": 20.0, "FoldAngle": 30.0, "AzFoldDir": 15.0,
                        "ElFoldDir": -25.0, "AxFoldAx": 0.05, "OffFoldAx": 0.1, "RFoldAx": 0.25,
                        "Feather": 12.0, "FeatherOffsetXoC": 0.1, "FeatherAxisXoC": 0.35,
                        "IndividualBladeFoldFlag": 1.0, "FoldAngle_1": -20.0 }
    return True, { "Precone": 5.0, "Rotate": -15.0, "FoldAngle": 25.0, "RFoldAx": 0.3,
                   "Feather": 9.0, "FeatherAxisXoC": 0.3 }


def test_matches_3_55_0():
    # With the feather bearing at the tip, outboard of the fold as in 3.55.0, a propeller is
    # placed exactly as 3.55.0 placed it.  The reversed case keeps to the parms 3.55.0 mirrored.
    for name, pts in GOLDEN_355.items():
        rev, parms = golden_case( name )
        pid = new_prop( 3, dict( parms, RFeatherAx = 1.0 ), rev )
        for s, u, w, x, y, z in pts:
            p = vsp.CompPnt01( pid, s, u, w )
            assert math.dist( ( p.x(), p.y(), p.z() ), ( x, y, z ) ) < 1e-9, ( name, s, u, w )


def turn_z( pts, deg ):
    """Turn points right-handed about the propeller's z axis, the way Precone does."""
    c = math.cos( math.radians( deg ) )
    s = math.sin( math.radians( deg ) )
    return [ ( c * x - s * y, s * x + c * y, z ) for x, y, z in pts ]


def test_hinges_ride_on_the_precone():
    # Precone turns the blade at the hub before any hinge, so a coned blade is the blade without
    # precone, hinges and all, turned about the propeller's z axis.
    hinges = { "FlapAngle": 6.0, "RFlapAx": 0.1, "OffFlapAx": 0.02, "AxFlapAx": 0.03, "ElFlapDir": 15.0,
               "LagAngle": -4.0, "RLagAx": 0.15, "CantLagDir": 10.0, "AzLagDir": 5.0, "Feather": 8.0,
               "FoldAngle": 12.0, "RFoldAx": 0.3 }
    for rev in ( False, True ):
        flat = blade( new_prop( 2, hinges, rev ), 0 )
        coned = blade( new_prop( 2, dict( hinges, Precone = 7.0 ), rev ), 0 )
        assert furthest( coned, turn_z( flat, 7.0 ) ) < TOL, rev


def test_feather_turns_about_its_line():
    # The feather axis is the radial line at the feather offset, FeatherOffsetXoC root chords aft.
    pid = new_prop( 2, { "FeatherOffsetXoC": 0.2, "FeatherAxisXoC": 0.3 } )
    unfeathered = blade( pid, 0 )
    le = vsp.CompPnt01( pid, 0, 0.0, 0.5 )
    te = vsp.CompPnt01( pid, 0, 0.0, 0.0 )
    croot = math.dist( ( le.x(), le.y(), le.z() ), ( te.x(), te.y(), te.z() ) )
    set_parms( pid, { "Feather": 20.0 } )
    want = turn_about( unfeathered, ( 0.0, 0.0, 0.2 * croot ), ( 0.0, 1.0, 0.0 ), 20.0 )
    assert furthest( blade( pid, 0 ), want ) < 1e-6


def skewed_axis( az, skew, base, about ):
    """A hinge vector: base turned by skew about axis about (0 x, 2 z), then by az about y, in
    propeller axes for normal rotation (blade x aft = +Z, z thrust = -X)."""
    def rot( v, axis, deg ):
        return turn_about( [ v ], ( 0.0, 0.0, 0.0 ), axis, deg )[0]
    v = rot( base, about, skew )
    return rot( v, ( 0.0, 1.0, 0.0 ), az )


def test_skewed_flap_hinge():
    # Flap: vector forward (-Z), skewed by -El about the shaft (blade z, -X), then Az about y;
    # the blade turns right-handed about minus that vector by FlapAngle.
    radius = 15.0
    hinge = { "RFlapAx": 0.1, "OffFlapAx": 0.02, "AxFlapAx": 0.03, "AzFlapDir": 30.0, "ElFlapDir": 20.0 }
    pid = new_prop( 2, hinge )
    unflapped = blade( pid, 0 )
    set_parms( pid, { "FlapAngle": 25.0 } )
    vec = skewed_axis( 30.0, -20.0, ( 0.0, 0.0, -1.0 ), ( -1.0, 0.0, 0.0 ) )
    origin = ( 0.03 * radius, 0.1 * radius, 0.02 * radius )
    want = turn_about( unflapped, origin, tuple( -c for c in vec ), 25.0 )
    assert furthest( blade( pid, 0 ), want ) < TOL


def test_skewed_lag_hinge():
    # Lag: vector toward the thrust (-X), canted by -Cant about the chord (blade x, +Z), then Az
    # about y; the blade turns right-handed about minus that vector by LagAngle.
    radius = 15.0
    hinge = { "RLagAx": 0.12, "OffLagAx": 0.02, "AxLagAx": 0.03, "AzLagDir": 30.0, "CantLagDir": 20.0 }
    pid = new_prop( 2, hinge )
    unlagged = blade( pid, 0 )
    set_parms( pid, { "LagAngle": 15.0 } )
    vec = skewed_axis( 30.0, -20.0, ( -1.0, 0.0, 0.0 ), ( 0.0, 0.0, 1.0 ) )
    origin = ( 0.03 * radius, 0.12 * radius, 0.02 * radius )
    want = turn_about( unlagged, origin, tuple( -c for c in vec ), 15.0 )
    assert furthest( blade( pid, 0 ), want ) < TOL


def test_blades_removed_and_added():
    # Blades removed take their own angles with them; blades added back start at the propeller's.
    masters = { "FlapAngle": 4.0, "LagAngle": -2.0, "FoldAngle": 7.0, "Feather": 5.0 }
    pid = new_prop( 4, masters )
    set_parms( pid, { "BladeMotionMode": 1.0, "IndividualBladeFoldFlag": 1.0 } )
    own = {}
    for name in ( "FlapAngle", "LagAngle", "FoldAngle", "Feather" ):
        for i in range( 1, 4 ):
            own[ "%s_%d" % ( name, i ) ] = 3.0 * i
    set_parms( pid, own )
    set_parms( pid, { "NumBlade": 2 } )
    set_parms( pid, { "NumBlade": 4 } )
    names = [ vsp.GetParmName( p ) for p in vsp.GetGeomParmIDs( pid )
              if vsp.GetParmGroupName( p ) in ( "Design", "Articulation" ) ]
    assert len( names ) == len( set( names ) )
    for name in ( "FlapAngle", "LagAngle", "FoldAngle", "Feather" ):
        assert vsp.GetParmVal( pid, name + "_1", group( name + "_1" ) ) == 3.0
        assert vsp.GetParmVal( pid, name + "_2", group( name + "_2" ) ) == masters[ name ]
    # Blade 3, added back, sits as blade 1 does, turned to its azimuth.
    az = vsp.GetParmVal( pid, "BladeAz_3", "Design" )
    assert furthest( blade( pid, 2 ), turn( blade( pid, 0 ), az ) ) < TOL


def test_hinges_at_one_place_lag_before_fold():
    # Lag and fold hinges at the same radius: the lag is inboard of the fold.
    radius = 15.0
    hinge = { "RLagAx": 0.2, "RFoldAx": 0.2 }
    pid = new_prop( 2, dict( hinge, FoldAngle = 30.0 ) )
    folded = blade( pid, 0 )
    set_parms( pid, { "LagAngle": 12.0 } )
    want = turn_about( folded, ( 0.0, 0.2 * radius, 0.0 ), ( 1.0, 0.0, 0.0 ), 12.0 )
    assert furthest( blade( pid, 0 ), want ) < TOL


def test_default_hinge_places():
    pid = new_prop( 2, {} )
    for name, val in ( ( "RFlapAx", 0.05 ), ( "RLagAx", 0.1 ), ( "RFeatherAx", 0.15 ), ( "RFoldAx", 0.2 ) ):
        assert vsp.GetParmVal( pid, name, group( name ) ) == val, name


def test_active_blade_kept_while_any_blade_motion_is_individual():
    pid = new_prop( 4, { "BladeMotionMode": 1.0 } )
    vsp.SetParmVal( pid, "ActiveBlade", "Index", 3 )
    set_parms( pid, { "FlapAngle": 4.0 } )
    assert vsp.GetParmVal( pid, "ActiveBlade", "Index" ) == 3
    set_parms( pid, { "BladeMotionMode": 0.0 } )
    assert vsp.GetParmVal( pid, "ActiveBlade", "Index" ) == 1


def test_senses_on_other_blades_and_turned_rotor():
    # A positive flap lifts every blade's tip toward the thrust, and a positive lag moves it aft,
    # with the rotor turned.
    for rev in ( False, True ):
        sgn = -1.0 if rev else 1.0
        for surf in ( 1, 2 ):
            pid = new_prop( 3, { "Rotate": 40.0 }, rev )
            p0 = vsp.CompPnt01( pid, surf, 1.0, 0.25 )
            set_parms( pid, { "FlapAngle": 10.0 } )
            p1 = vsp.CompPnt01( pid, surf, 1.0, 0.25 )
            assert p1.x() < p0.x() - 1.0, ( rev, surf )
            set_parms( pid, { "FlapAngle": 0.0, "LagAngle": 10.0 } )
            p2 = vsp.CompPnt01( pid, surf, 1.0, 0.25 )
            # Aft is against the rotation: the tip turns about +X by -sgn, the way BladeAz turns.
            r0 = math.atan2( p0.z(), p0.y() )
            r2 = math.atan2( p2.z(), p2.y() )
            assert sgn * math.remainder( r2 - r0, 2.0 * math.pi ) > 0.01, ( rev, surf )
