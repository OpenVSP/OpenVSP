"""Moving propeller blades places the finished blade again without building it again.  A propeller
changed one step at a time must match one built at once with the same parms, whichever mix of
blade motion, shape and tessellation the steps change.
"""

import math
import os
import tempfile

import openvsp as vsp

from prophelp import group

UW = [ ( u, w ) for u in ( 0.0, 0.05, 0.3, 0.6, 0.95, 1.0 ) for w in ( 0.0, 0.2, 0.5, 0.7, 0.9 ) ]
TOL = 1e-9

# Each step changes blade motion only, the blade's shape, or its tessellation, or a mix.
STEPS = [
    { "FlapAngle": 6.0 },
    { "Diameter": 24.0 },
    { "LECluster": 0.1 },
    { "Rotate": 25.0, "Precone": 4.0 },
    { "Tess_W": 25 },
    { "NumBlade": 4 },
    { "LagAngle": -3.0 },
    { "FeatherOffsetXoC": 0.1, "ConstructXoC": 0.4 },
    # Each blade's own angle is given: one never synced to the master keeps its first value.
    { "BladeMotionMode": 1.0, "LagAngle_1": 5.0, "LagAngle_2": -3.0, "LagAngle_3": 2.0 },
    { "UseBeta34Flag": 1.0, "Beta34": 25.0 },
    { "FoldAngle": 20.0, "AzFoldDir": 10.0 },
    { "PropMode": vsp.PROP_BOTH },
    { "NumBlade": 2 },
    # The feather drives again; blade 2 has its own feather in this mode, so it is given.
    { "Feather": 3.0, "UseBeta34Flag": 0.0, "Feather_1": 3.0 },
    { "ReverseFlag": 1.0 },
    # A blade added back starts from the propeller's angle, here zero; then each is given its own.
    { "LagAngle": 0.0 },
    { "NumBlade": 5 },
    { "LagAngle_2": 1.0, "LagAngle_3": -2.0, "LagAngle_4": 4.0 },
    { "RFlapAx": 0.12, "ElFlapDir": 15.0 },
    { "NumBlade": 3, "Tess_W": 21 },
]


def parm_group( name ):
    if name == "Tess_W":
        return "Shape"
    return group( name )


def set_parms( pid, parms ):
    for name, val in parms.items():
        assert vsp.FindParm( pid, name, parm_group( name ) ), name
        vsp.SetParmVal( pid, name, parm_group( name ), val )
    vsp.Update()


def surfaces( pid ):
    pts = []
    for s in range( vsp.GetNumMainSurfs( pid ) ):
        for u, w in UW:
            p = vsp.CompPnt01( pid, s, u, w )
            pts.append( ( p.x(), p.y(), p.z() ) )
    return pts


def stl( pid ):
    """The STL vertices of pid alone, through a set of its own."""
    for gid in vsp.FindGeoms():
        vsp.SetSetFlag( gid, vsp.SET_FIRST_USER, gid == pid )
    path = os.path.join( tempfile.mkdtemp(), "prop.stl" )
    vsp.ExportFile( path, vsp.SET_FIRST_USER, vsp.EXPORT_STL )
    pts = []
    for line in open( path ):
        t = line.split()
        if t and t[0] == "vertex":
            pts.append( tuple( float( x ) for x in t[1:] ) )
    return pts


def box( pid ):
    names = ( "X_Min", "X_Len", "Y_Min", "Y_Len", "Z_Min", "Z_Len" )
    return [ tuple( vsp.GetParmVal( vsp.FindParm( pid, n, "BBox" ) ) for n in names ) ]


# Geom::Update adds the feature lines to the blade after it is built.
def feature_lines( pid ):
    return [ ( p.x(), p.y(), p.z() ) for p in vsp.GetFeatureLinePnts( pid ) ]


# The tessellation the display type shows, kept from update to update rather than built from the
# surfaces as the parts above are.
def display_tess( pid ):
    n = vsp.GetTotalNumSurfs( pid )
    # The degen preview leaves out the disk.
    if vsp.GetGeomDisplayType( pid ) != vsp.DISPLAY_BEZIER:
        mode = vsp.GetParmVal( pid, "PropMode", "Design" )
        if mode == vsp.PROP_DISK:
            n = 0
        elif mode == vsp.PROP_BOTH:
            n -= n // ( int( vsp.GetParmVal( pid, "NumBlade", "Design" ) ) + 1 )
    pts = []
    for s in range( n ):
        pts.extend( ( p.x(), p.y(), p.z() ) for p in vsp.GetGeomDisplayTessPnts( pid, s ) )
    return pts


def state( pid ):
    return { "surfaces": surfaces( pid ), "stl": stl( pid ), "box": box( pid ),
             "feature lines": feature_lines( pid ), "display tessellation": display_tess( pid ) }


def furthest( a, b ):
    assert len( a ) == len( b )
    return max( math.dist( p, q ) for p, q in zip( a, b ) )


# The parms that give blades their own angles.
MODES = ( "BladeMotionMode", "IndividualBladeFoldFlag" )


def own_angle( name ):
    return name in MODES or name.split( "_" )[-1].isdigit()


def fresh( parms, display = vsp.DISPLAY_BEZIER ):
    """A propeller given parms all at once, beside any already in the model: the shared ones, then
    the modes and each blade's own angles, as a propeller is set up by hand."""
    pid = vsp.AddGeom( "PROP" )
    # The per-blade parms are made by an Update.
    set_parms( pid, { "NumBlade": parms.get( "NumBlade", 3 ) } )
    set_parms( pid, { k: v for k, v in parms.items() if not own_angle( k ) } )
    set_parms( pid, { k: v for k, v in parms.items() if own_angle( k ) } )
    # A pasted propeller is new, so its first Update tessellates it with every parm set; the one
    # set up step by step could have kept a stale tessellation as the one under test may.
    vsp.CopyGeomToClipboard( pid )
    pasted = vsp.PasteGeomClipboard()[0]
    vsp.DeleteGeom( pid )
    vsp.SetGeomDisplayType( pasted, display )
    vsp.Update()
    assert vsp.GetGeomDisplayType( pasted ) == display
    return pasted


def steps_match_fresh_build( steps, display = vsp.DISPLAY_BEZIER ):
    vsp.VSPRenew()
    pid = fresh( {}, display )
    parms = {}
    for i, step in enumerate( steps ):
        set_parms( pid, step )
        # A removed blade's own parms go with it.
        if "NumBlade" in step:
            for name in list( parms ):
                if "_" in name and name.split( "_" )[-1].isdigit() and int( name.split( "_" )[-1] ) >= step["NumBlade"]:
                    del parms[name]
        parms.update( step )
        other = fresh( parms, display )
        got = state( pid )
        want = state( other )
        vsp.DeleteGeom( other )
        vsp.Update()
        for part in got:
            assert furthest( got[part], want[part] ) < TOL, ( i, step, part )


def test_steps_match_fresh_build():
    steps_match_fresh_build( STEPS )


def test_steps_match_fresh_build_degen_preview():
    steps_match_fresh_build( STEPS, vsp.DISPLAY_DEGEN_SURF )


def test_moved_blades_keep_tessellation_settings():
    # A tessellation change, then blade motion alone: the moved blades keep the new settings.
    steps_match_fresh_build( [ { "LECluster": 0.05, "Tess_W": 33 }, { "FoldAngle": 8.0 } ] )


def test_clone_follows_blade_motion():
    vsp.VSPRenew()
    pid = fresh( {} )
    clone = vsp.CloneGeomVec( [ pid ] )[0]
    vsp.Update()
    for step in ( { "FoldAngle": 9.0 }, { "Rotate": 30.0 }, { "IndividualBladeFoldFlag": 1.0, "FoldAngle_2": -4.0 },
                  { "NumBlade": 5 } ):
        set_parms( pid, step )
        assert furthest( surfaces( clone ), surfaces( pid ) ) < TOL, step
        assert furthest( display_tess( clone ), display_tess( pid ) ) < TOL, step


def test_blade_count_sets_solidity():
    vsp.VSPRenew()
    pid = fresh( { "NumBlade": 2 } )
    two = vsp.GetParmVal( pid, "Solidity", "Design" )
    set_parms( pid, { "NumBlade": 4 } )
    assert abs( vsp.GetParmVal( pid, "Solidity", "Design" ) - 2.0 * two ) < TOL


def test_removed_blade_loses_its_subsurface():
    vsp.VSPRenew()
    pid = fresh( { "NumBlade": 4 } )
    ss = vsp.AddSubSurf( pid, vsp.SS_LINE )
    vsp.SetParmVal( vsp.FindParm( ss, "MainSurfIndx", "SubSurface" ), 3 )
    vsp.Update()
    assert vsp.GetNumSubSurf( pid ) == 1
    set_parms( pid, { "NumBlade": 3 } )
    assert vsp.GetNumSubSurf( pid ) == 0
