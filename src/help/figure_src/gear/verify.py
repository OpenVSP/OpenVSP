"""Checks gearmodel.py, and the signs and placements Gear.md states, against the openvsp module.

    <build>/venv/bin/python verify.py
"""

import numpy as np
import openvsp as vsp

import gearmodel as gm

BASE = { "Mech_XLoc": 0.3, "Mech_YLoc": 0.2, "Mech_ZLoc": 2.5, "Mech_X_Axis": 0.2, "Mech_Y_Axis": 1.0, "Mech_Z_Axis": 0.1,
         "Mech_RetAngle": 95, "MechKneePos": 0.45, "MechKneeAngle": 0, "MechKneeAzimuthAngle": 0,
         "MechKneeElevationAngle": 0, "MechKneeDownAngle": 180, "Mech_TwistAngle": 0, "m_MechRollAngle": 0,
         "Mech_BogieAngle": 0, "Mech_StrutDL": 0.0 }

CASES = ( {}, { "MechKneeAngle": -70 }, { "MechKneeDownAngle": 160, "MechKneeAngle": -70 },
          { "MechKneeAzimuthAngle": 20, "MechKneeElevationAngle": 10, "MechKneeAngle": -70 },
          { "Mech_TwistAngle": 15, "m_MechRollAngle": 10, "Mech_BogieAngle": 25 }, { "Mech_StrutDL": 0.2 },
          { "MechKneeDownAngle": 160, "MechKneeAzimuthAngle": 20, "MechKneeElevationAngle": 10, "MechKneeAngle": -70,
            "Mech_TwistAngle": 15, "m_MechRollAngle": 10, "Mech_BogieAngle": 25, "Mech_StrutDL": 0.2 } )


def new_gear():
    vsp.VSPRenew()
    g = vsp.AddGeom( "GEAR" )
    b = vsp.CreateAndAddBogie( g )
    vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", 0 )
    vsp.Update()
    return g, b


def P( b, n, grp = "Retract" ):
    return vsp.FindParm( b, n, grp )


def check( ok, what ):
    print( ( "ok    " if ok else "FAIL  " ) + what )
    return ok


def mechanism():
    """The model places the tires where VSP does, down, up and between, for every mechanism parm."""
    worst = 0.0
    for case in CASES:
        g, b = new_gear()
        vsp.SetParmVal( P( b, "RetMode" ), vsp.GEAR_MECHANISM )
        vsp.Update()
        vsp.SetParmVal( P( b, "MechAbsRelFlag" ), vsp.ABS )
        d = dict( BASE )
        d.update( case )
        for k, v in d.items():
            vsp.SetParmVal( P( b, k ), v )
        vsp.SetParmVal( P( b, "MechKRetract" ), 0.4 )
        vsp.SetParmVal( g, "GearConfigMode", "Gear", vsp.GEAR_CONFIGURATION_ALL )
        vsp.Update()
        B = gm.Bogie( b )
        for isurf, k in ( ( 0, 0.0 ), ( 1, 1.0 ), ( 2, 0.4 ) ):
            c, ax = gm.tire_center( g, isurf )
            M, knee = B.matrix( k )
            worst = max( worst, np.abs( c - M[ :3, 3 ] ).max(), np.abs( ax - M[ :3, :3 ] @ [ 0, 1, 0 ] ).max() )
    return check( worst < 1e-9, "gearmodel matches VSP's mechanism (worst %.1e)" % worst )


def retract_sign():
    """Retract Angle 90 about +Y, trunnion above the axle, swings the axle forward (-X)."""
    g, b = new_gear()
    vsp.SetParmVal( P( b, "RetMode" ), vsp.GEAR_MECHANISM )
    vsp.Update()
    vsp.SetParmVal( P( b, "MechAbsRelFlag" ), vsp.ABS )
    for k, v in { "Mech_XLoc": 0, "Mech_YLoc": 0, "Mech_ZLoc": 2.0, "Mech_X_Axis": 0, "Mech_Y_Axis": 1, "Mech_Z_Axis": 0,
                  "Mech_RetAngle": 90 }.items():
        vsp.SetParmVal( P( b, k ), v )
    vsp.SetParmVal( g, "GearConfigMode", "Gear", vsp.GEAR_CONFIGURATION_UP )
    vsp.Update()
    c, ax = gm.tire_center( g, 0 )
    return check( c[0] < -1.0 and abs( c[2] - 2.0 ) < 1e-9, "a positive Retract Angle about +Y swings the axle forward" )


def tire_below_ground():
    """The pivot is SLR above the contact point; the unloaded tire reaches SLR - D/2 below the ground."""
    g, b = new_gear()
    D = vsp.GetParmVal( P( b, "DiameterModel", "Tire" ) )
    slr = vsp.GetParmVal( P( b, "StaticRadiusModel", "Tire" ) )
    G = gm.grid( g, 0, 41, 81 )
    c, ax = gm.tire_center( g, 0 )
    ok = check( abs( c[2] - slr ) < 1e-9, "the pivot is SLR above the contact point" )
    return check( abs( G[ :, :, 2 ].min() - ( slr - 0.5 * D ) ) < 1e-3, "the tire reaches below the ground by D/2 - SLR" ) and ok


def stow_frames():
    """Abs stow is in the Gear's axes; a symmetrical bogie's stowed position is mirrored; switching modes keeps it."""
    g, b = new_gear()
    vsp.SetParmVal( g, "X_Rel_Location", "XForm", 10.0 )
    vsp.SetParmVal( g, "GearConfigMode", "Gear", vsp.GEAR_CONFIGURATION_UP )
    vsp.SetParmVal( P( b, "StowAbsRelFlag" ), vsp.ABS )
    vsp.Update()
    vsp.SetParmVal( P( b, "StowZLoc" ), 3.0 )
    vsp.Update()
    c, ax = gm.tire_center( g, 0 )
    ok = check( np.abs( c - [ 10, 0, 3 ] ).max() < 1e-9, "Abs stowed location is in the Gear's axes" )
    vsp.SetParmVal( P( b, "Symmetrical", "Bogie" ), 1 )
    vsp.SetParmVal( P( b, "YContactPt", "Bogie" ), 3.0 )
    vsp.SetParmVal( P( b, "StowAbsRelFlag" ), vsp.REL )
    vsp.SetParmVal( P( b, "StowYRelLoc" ), -1.0 )
    vsp.SetParmVal( P( b, "StowXRelRot" ), 30.0 )
    vsp.Update()
    c0, a0 = gm.tire_center( g, 0 )
    c1, a1 = gm.tire_center( g, 1 )
    ok = check( np.abs( c0 * [ 1, -1, 1 ] - c1 ).max() < 1e-9, "a symmetrical bogie's stowed position is mirrored" ) and ok
    vsp.SetParmVal( P( b, "RetMode" ), vsp.GEAR_MECHANISM )
    vsp.Update()
    c2, a2 = gm.tire_center( g, 0 )
    return check( np.abs( c2 - c0 ).max() < 1e-9, "switching to Gear Mechanism keeps the stowed position" ) and ok


def shoulder_model_mode():
    """Hs and Ws given in model units keep their values."""
    g, b = new_gear()
    ok = True
    for mode, val in ( ( "WsMode", "WsModel" ), ( "HsMode", "HsModel" ) ):
        vsp.SetParmVal( P( b, mode, "Tire" ), vsp.TIRE_DIM_MODEL )
        vsp.SetParmVal( P( b, val, "Tire" ), 0.123 )
        vsp.Update()
        ok = check( abs( vsp.GetParmVal( P( b, val, "Tire" ) ) - 0.123 ) < 1e-12, "%s in Model mode keeps its value" % val ) and ok
    return ok


def stow_any_component():
    """The stowed position can be attached to any component, not only the Gear's parent, and follows it."""
    g, b = new_gear()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( g, "GearConfigMode", "Gear", vsp.GEAR_CONFIGURATION_UP )
    vsp.SetBogieStowParentID( b, pod )
    vsp.SetParmVal( P( b, "StowAbsRelFlag" ), vsp.REL )
    vsp.SetParmVal( P( b, "StowTransAttachFlag", "StowAttach" ), vsp.ATTACH_TRANS_COMP )
    vsp.Update()
    ok = check( vsp.GetBogieStowParentID( b ) == pod and vsp.GetGeomParent( g ) == "NONE",
                "a bogie's stowed position attaches to a component that is not the Gear's parent" )
    c0, a0 = gm.tire_center( g, 0 )
    vsp.SetParmVal( pod, "X_Rel_Location", "XForm", 5.0 )
    vsp.Update()
    c1, a1 = gm.tire_center( g, 0 )
    return check( np.abs( c1 - c0 - [ 5, 0, 0 ] ).max() < 1e-9, "and follows that component when it moves" ) and ok


def knee_axis_angles():
    """Azimuth Angle turns the knee axis right-handed about the strut, taken from the trunnion to the pivot, and a
    positive Elevation Angle tilts it toward the trunnion; turned first, then tilted, it is the axis VSP uses."""
    g, b = new_gear()
    vsp.SetParmVal( P( b, "RetMode" ), vsp.GEAR_MECHANISM )
    vsp.Update()
    vsp.SetParmVal( P( b, "MechAbsRelFlag" ), vsp.ABS )
    d = dict( BASE )
    d.update( { "Mech_Z_Axis": -0.4, "MechKneeAzimuthAngle": 30, "MechKneeElevationAngle": 20 } )
    for k, v in d.items():
        vsp.SetParmVal( P( b, k ), v )
    vsp.Update()
    B = gm.Bogie( b )
    ud = gm.unit( B.trunnion - B.pivot() )
    s = -ud
    n = gm.unit( np.cross( B.axis, ud ) )
    base = gm.unit( np.cross( ud, n ) )
    mid = gm.rot( s, B.knee_az )[ :3, :3 ] @ base
    fin = gm.rot( gm.unit( np.cross( mid, ud ) ), B.knee_el )[ :3, :3 ] @ mid
    return check( np.abs( fin - B.knee_axis() ).max() < 1e-12 and fin @ ud > 0 and np.cross( base, mid ) @ s > 0,
                  "the knee axis is the trunnion axis square to the strut, turned by Azimuth then tilted by Elevation" )


if __name__ == "__main__":
    results = [ mechanism(), retract_sign(), tire_below_ground(), stow_frames(), shoulder_model_mode(),
                stow_any_component(), knee_axis_angles() ]
    print( "all passed" if all( results ) else "SOME FAILED" )
