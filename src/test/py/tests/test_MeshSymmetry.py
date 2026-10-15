"""A Mesh makes no symmetric copies, so its symmetry controls are inactive."""

import os

import openvsp as vsp


def test_a_meshs_symmetry_controls_are_inactive( tmp_path ):
    vsp.VSPRenew()
    vsp.AddGeom( "POD" )
    vsp.Update()
    mesh_id = vsp.ExportFile( os.path.join( str( tmp_path ), "pod.stl" ), vsp.SET_ALL, vsp.EXPORT_STL )
    vsp.Update()
    for name in ( "Sym_Planar_Flag", "Sym_Axial_Flag", "Sym_Rot_N" ):
        assert not vsp.GetParmActiveFlag( vsp.GetParm( mesh_id, name, "Sym" ) ), name
