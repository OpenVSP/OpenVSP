"""Making a point cloud's convex hull writes no file."""

import os

import openvsp as vsp


def test_a_convex_hull_leaves_no_file_behind( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    vsp.AddGeom( "POD" )
    vsp.Update()
    mesh_id = vsp.ExportFile( "pod.msh", vsp.SET_ALL, vsp.EXPORT_GMSH )
    cloud_id = vsp.CreatePtCloudGeom( mesh_id )
    hull_id = vsp.CreateConvexHull( cloud_id )
    vsp.Update()
    assert hull_id
    assert not os.path.exists( "test.pts" )
