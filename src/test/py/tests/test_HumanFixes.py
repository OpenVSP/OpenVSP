"""Human components."""

import os
import re

import openvsp as vsp


def test_stature_limits_follow_the_sex():
    vsp.VSPRenew()
    hid = vsp.AddGeom( "HUMAN" )
    vsp.Update()
    vsp.SetParmVal( hid, "LenUnit", "Anthropometric", vsp.LEN_M )
    vsp.Update()
    vsp.SetParmVal( hid, "GenderFlag", "Anthropometric", vsp.FEMALE )
    vsp.Update()
    st = vsp.FindParm( hid, "Stature", "Anthropometric" )
    assert abs( vsp.GetParmLowerLimit( st ) - 1.498 ) < 1e-9
    assert abs( vsp.GetParmUpperLimit( st ) - 1.735 ) < 1e-9
    assert abs( vsp.GetParmVal( st ) - 1.619 ) < 1e-3


def super_cone_bounds( size_first ):
    vsp.VSPRenew()
    hid = vsp.AddGeom( "HUMAN" )
    vsp.Update()
    if size_first:
        vsp.SetParmVal( hid, "Stature_pct", "Anthropometric", 0.95 )
        vsp.Update()
    cone = vsp.AddGeom( "AUXILIARY", hid )
    vsp.Update()
    vsp.SetParmVal( cone, "AuxiliaryGeomType", "Design", vsp.AUX_GEOM_SUPER_CONE )
    vsp.Update()
    if not size_first:
        vsp.SetParmVal( hid, "Stature_pct", "Anthropometric", 0.95 )
        vsp.Update()
    return vsp.GetGeomBBoxMin( cone ), vsp.GetGeomBBoxMax( cone )


def test_a_super_cone_follows_its_human():
    fresh = super_cone_bounds( True )
    followed = super_cone_bounds( False )
    assert ( fresh[0] - followed[0] ).mag() < 1e-9
    assert ( fresh[1] - followed[1] ).mag() < 1e-9


TRUNK_PARMS = [ "Back", "Waist", "TorsoTwist", "TorsoBend", "Nod", "RotateHead", "HeadTilt" ]
LIMB_PARMS = [ "Wrist", "WristDev", "Forearm", "Elbow", "ShoulderABAD", "ShoulderFE", "ShoulderIE", "HipABAD", "HipFE", "HipIE",
               "Knee", "Ankle" ]


def preset_table():
    """HumanGeom's preset poses, read from HumanGeom.cpp: name -> ( sym, trunk, right, left )."""
    path = os.path.join( os.path.dirname( __file__ ), "..", "..", "..", "geom_core", "HumanGeom.cpp" )
    text = open( path ).read()
    body = text.split( "human_preset_poses[vsp::HUMAN_NUM_PRESET_POSE] = {" )[1].split( "};" )[0]
    out = {}
    for m in re.finditer( r"// (\w+)\s*\{ (true|false), \{ ([^}]*) \},\s*\{ \{ ([^}]*) \},\s*\{ ([^}]*) \} \} \}", body ):
        vals = [ [ float( x ) for x in m.group( k ).split( "," ) ] for k in ( 3, 4, 5 ) ]
        out[ m.group( 1 ) ] = ( m.group( 2 ) == "true", *vals )
    return out


def test_the_presets_are_inside_the_parm_ranges():
    vsp.VSPRenew()
    hid = vsp.AddGeom( "HUMAN" )
    vsp.Update()

    def check( name, parm, v ):
        pid = vsp.FindParm( hid, parm, "Pose" )
        assert vsp.GetParmLowerLimit( pid ) <= v <= vsp.GetParmUpperLimit( pid ), ( name, parm, v )

    table = preset_table()
    assert len( table ) == 17
    for name, ( sym, trunk, right, left ) in table.items():
        for parm, v in zip( TRUNK_PARMS, trunk ):
            check( name, parm, v )
        for side, vals in ( ( "Rt", right ), ( "Lt", left ) ):
            for parm, v in zip( LIMB_PARMS, vals ):
                check( name, parm + side, v )
        if sym:
            assert right == left, name
    # Standing is the default pose.
    sym, trunk, right, left = table[ "STANDING" ]
    for parm, v in zip( TRUNK_PARMS, trunk ):
        assert vsp.GetParmVal( hid, parm, "Pose" ) == v, parm
    for parm, v in zip( LIMB_PARMS, right ):
        assert vsp.GetParmVal( hid, parm + "Rt", "Pose" ) == v, parm


def test_every_preset_poses_through_the_api():
    # SetHumanPresetPose sets every pose Parm to its value in the table; a value outside its
    # Parm's range would come back clamped.
    vsp.VSPRenew()
    hid = vsp.AddGeom( "HUMAN" )
    vsp.Update()
    table = preset_table()
    names = [ "STANDING", "SITTING", "SEATED_ERECT", "PASSENGER", "DRIVER", "PILOT", "RECUMBENT", "NBP_SKYLAB",
              "NBP_ISS", "KNEELING", "KNEELING_ONE_KNEE", "SQUATTING", "CRAWLING", "FORWARD_REACH",
              "OVERHEAD_REACH", "SPAN", "WALKING" ]
    assert len( names ) == vsp.HUMAN_NUM_PRESET_POSE
    for name in names:
        vsp.SetHumanPresetPose( hid, getattr( vsp, "HUMAN_PRESET_" + name ) )
        vsp.Update()
        sym, trunk, right, left = table[ name ]
        assert vsp.GetParmVal( hid, "RLSym", "Pose" ) == float( sym ), name
        for parm, v in zip( TRUNK_PARMS, trunk ):
            assert vsp.GetParmVal( hid, parm, "Pose" ) == v, ( name, parm )
        for side, vals in ( ( "Rt", right ), ( "Lt", left ) ):
            for parm, v in zip( LIMB_PARMS, vals ):
                assert vsp.GetParmVal( hid, parm + side, "Pose" ) == v, ( name, parm + side )
