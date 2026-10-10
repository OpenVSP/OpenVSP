# A Mode bundles a Normal set, a Degen set and Variable Preset settings.  Each analysis that can
# use a Mode keeps the Mode's ID, and has to find it, apply it and keep it the way it keeps the rest
# of its settings.

import os
import tempfile
import xml.etree.ElementTree

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


def testTheModeEachVehicleAnalysisUsesIsKeptInTheFile():
    vsp.VSPRenew()
    vsp.AddGeom( "POD" )
    mid = vsp.CreateAndAddMode( "Mode", vsp.SET_ALL, vsp.SET_NONE )
    fname = os.path.join( tempfile.mkdtemp(), "modes.vsp3" )
    vsp.WriteVSPFile( fname )

    # Only the GUI sets these, so they are set in the file
    tree = xml.etree.ElementTree.parse( fname )
    node = tree.getroot().find( "VehicleModes" )
    if node is None:
        node = xml.etree.ElementTree.SubElement( tree.getroot(), "VehicleModes" )
    for name in ( "CompGeomModeID", "MassPropModeID", "DegenGeomModeID", "PlanarSlicesModeID", "ExportModeID" ):
        child = node.find( name )
        if child is None:
            child = xml.etree.ElementTree.SubElement( node, name )
        child.text = mid
    tree.write( fname )

    for i in range( 2 ):
        vsp.ClearVSPModel()
        vsp.ReadVSPFile( fname )
        for analysis in ( "CompGeom", "MassProp", "DegenGeom", "PlanarSlice" ):
            vsp.SetAnalysisInputDefaults( analysis )
            assert vsp.GetStringAnalysisInput( analysis, "ModeID" )[0] == mid, analysis
        vsp.WriteVSPFile( fname )
