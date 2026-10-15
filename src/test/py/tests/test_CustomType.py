"""A custom component is named by its script, however it was made."""

import os

import openvsp as vsp


def test_a_custom_component_read_from_a_file_keeps_its_type_name( tmp_path ):
    vsp.VSPRenew()
    gid = vsp.AddGeom( "Cone" )
    vsp.Update()
    name = vsp.GetGeomTypeName( gid )
    fname = os.path.join( str( tmp_path ), "cone.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    assert [ vsp.GetGeomTypeName( g ) for g in vsp.FindGeoms() ] == [ name ]
