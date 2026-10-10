# A Mode bundles a Normal set, a Degen set and Variable Preset settings.  Each analysis that can
# use a Mode keeps the Mode's ID, and has to find it, apply it and keep it the way it keeps the rest
# of its settings.

import openvsp as vsp

from testhelp import drop_errors, assert_no_errors


def a_mode_with_a_setting( parm_id, val ):
    """A Mode whose one setting puts parm_id at val, and its group."""
    gid = vsp.AddVarPresetGroup( "Group" )
    vsp.AddVarPresetParm( gid, parm_id )
    sid = vsp.AddVarPresetSetting( gid, "Setting" )
    vsp.SetVarPresetParmVal( gid, sid, parm_id, val )
    mid = vsp.CreateAndAddMode( "Mode", vsp.SET_ALL, vsp.SET_NONE )
    vsp.ModeAddGroupSetting( mid, gid, sid )
    return mid, gid


def testAModeWhoseSettingGroupIsGoneStillApplies():
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    mid, gid = a_mode_with_a_setting( vsp.GetParm( pod, "X_Rel_Location", "XForm" ), 3.0 )
    vsp.DeleteVarPresetGroup( gid )
    drop_errors()

    vsp.ApplyModeSettings( mid )
    assert_no_errors()
