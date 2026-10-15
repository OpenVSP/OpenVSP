"""Mesh and NGon mesh components made from other components."""

import openvsp as vsp


def moved_pod_mesh():
    vsp.VSPRenew()
    vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    vsp.Update()
    mesh = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Mesh" ][0]
    vsp.SetParmVal( mesh, "X_Rel_Location", "XForm", 20.0 )
    vsp.Update()
    return mesh


def test_an_ngon_mesh_is_made_where_the_mesh_is():
    mesh = moved_pod_mesh()
    ngon = vsp.CreateNGonMeshGeom( mesh )
    vsp.Update()
    assert abs( vsp.GetGeomBBoxMin( ngon ).x() - vsp.GetGeomBBoxMin( mesh ).x() ) < 1e-9
    assert abs( vsp.GetGeomBBoxMax( ngon ).x() - vsp.GetGeomBBoxMax( mesh ).x() ) < 1e-9
