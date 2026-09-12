# Clone Geom: the Geoms whose shape is not made of surfaces.
#
# A mesh, human, point cloud, wireframe, polygon mesh and route each hand their shape to a Clone
# in the Geom's own frame, and the Clone places it where the Clone is.

import openvsp as vsp
import pytest

from clonehelp import ( switch, comp_geom_areas,
                        comp_geom_areas_of, comp_geom_volumes,
                        scratch_output, drop_errors, assert_no_errors,
                        a_mesh, a_wireframe, a_point_cloud, a_polygon_mesh )
import os
import tempfile




def testACloneOfAMeshMeasuresTheSameAsADuplicate():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    mesh = a_mesh()
    clone = vsp.CloneGeomVec( [ mesh ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 20.0 )
    vsp.Update()

    areas = comp_geom_areas()
    assert areas[ vsp.GetGeomName( clone ) ] == pytest.approx( areas[ vsp.GetGeomName( mesh ) ] )
    assert areas[ vsp.GetGeomName( mesh ) ] > 0.0
    assert_no_errors()


def testACloneOfAMeshIsTheSameSizeAsTheMesh():
    """The borrowed shape is unscaled, so the Clone applies the original's scaling too."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    mesh = a_mesh()
    clone = vsp.CloneGeomVec( [ mesh ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 40.0 )
    vsp.Update()

    before = vsp.GetParmVal( vsp.FindParm( mesh, "X_Len", "BBox" ) )
    vsp.SetParmVal( vsp.FindParm( mesh, "Scale", "XForm" ), 3.0 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( mesh, "X_Len", "BBox" ) ) == pytest.approx( 3.0 * before ), \
           "the mesh did not scale, so the Clone proves nothing"

    for parm in ( "X_Len", "Y_Len", "Z_Len" ):
        assert vsp.GetParmVal( vsp.FindParm( clone, parm, "BBox" ) ) == \
               pytest.approx( vsp.GetParmVal( vsp.FindParm( mesh, parm, "BBox" ) ) )
    assert_no_errors()


def testACloneOfAHumanMeasuresTheSameAsADuplicate():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    human = vsp.AddGeom( "HUMAN" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ human ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 20.0 )
    vsp.Update()

    areas = comp_geom_areas()
    vols = comp_geom_volumes()
    assert areas[ vsp.GetGeomName( clone ) ] == pytest.approx( areas[ vsp.GetGeomName( human ) ] )
    assert vols[ vsp.GetGeomName( clone ) ] == pytest.approx( vols[ vsp.GetGeomName( human ) ] )
    assert vols[ vsp.GetGeomName( human ) ] > 0.0
    assert_no_errors()


def testACloneOfAHumanCanMirrorOnItsOwn():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    human = vsp.AddGeom( "HUMAN" )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ human ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Y_Rel_Location", "XForm" ), 10.0 )
    vsp.Update()
    one_wide = vsp.GetParmVal( vsp.FindParm( clone, "Y_Len", "BBox" ) )

    switch( clone, "CloneSym", False )
    vsp.SetParmVal( vsp.FindParm( clone, "Sym_Planar_Flag", "Sym" ), vsp.SYM_XZ )
    vsp.Update()

    assert vsp.GetParmVal( vsp.FindParm( clone, "Y_Len", "BBox" ) ) > one_wide + 15.0
    assert_no_errors()


def testACloneOfAPointCloudStandsWhereItIsPut():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    cloud = a_point_cloud()

    dx = 25.0
    clone = vsp.CloneGeomVec( [ cloud ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), dx )
    vsp.Update()

    assert vsp.GetParmVal( vsp.FindParm( clone, "X_Min", "BBox" ) ) == \
           pytest.approx( vsp.GetParmVal( vsp.FindParm( cloud, "X_Min", "BBox" ) ) + dx )
    for parm in ( "X_Len", "Y_Len", "Z_Len" ):
        assert vsp.GetParmVal( vsp.FindParm( clone, parm, "BBox" ) ) == \
               pytest.approx( vsp.GetParmVal( vsp.FindParm( cloud, parm, "BBox" ) ) )

    # Scaling the point cloud scales the Clone too.
    before = vsp.GetParmVal( vsp.FindParm( cloud, "X_Len", "BBox" ) )
    vsp.SetParmVal( vsp.FindParm( cloud, "Scale", "XForm" ), 2.0 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( cloud, "X_Len", "BBox" ) ) == pytest.approx( 2.0 * before )
    assert vsp.GetParmVal( vsp.FindParm( clone, "X_Len", "BBox" ) ) == pytest.approx( 2.0 * before )
    assert_no_errors()


def testACloneOfAWireFrameStandsWhereItIsPut():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    wire = a_wireframe()
    dx = 12.0
    clone = vsp.CloneGeomVec( [ wire ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), dx )
    vsp.Update()

    assert vsp.GetParmVal( vsp.FindParm( wire, "X_Len", "BBox" ) ) > 0.0
    assert vsp.GetParmVal( vsp.FindParm( clone, "X_Min", "BBox" ) ) == \
           pytest.approx( vsp.GetParmVal( vsp.FindParm( wire, "X_Min", "BBox" ) ) + dx )
    for parm in ( "X_Len", "Y_Len", "Z_Len" ):
        assert vsp.GetParmVal( vsp.FindParm( clone, parm, "BBox" ) ) == \
               pytest.approx( vsp.GetParmVal( vsp.FindParm( wire, parm, "BBox" ) ) )

    # The Clone shows the grid as the original rearranges it: skipping points shrinks both.
    before = vsp.GetParmVal( vsp.FindParm( wire, "X_Len", "BBox" ) )
    vsp.SetParmVal( vsp.FindParm( wire, "ISkipStart", "WireFrame" ), 2 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( wire, "X_Len", "BBox" ) ) < before, \
           "skipping points did not change the original, so the Clone proves nothing"
    assert vsp.GetParmVal( vsp.FindParm( clone, "X_Len", "BBox" ) ) == \
           pytest.approx( vsp.GetParmVal( vsp.FindParm( wire, "X_Len", "BBox" ) ) )

    # The grid is handed over unscaled, so the Clone applies the original's scaling.
    before = vsp.GetParmVal( vsp.FindParm( wire, "X_Len", "BBox" ) )
    vsp.SetParmVal( vsp.FindParm( wire, "Scale", "XForm" ), 2.0 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( wire, "X_Len", "BBox" ) ) == pytest.approx( 2.0 * before ), \
           "the original did not scale, so the Clone proves nothing"
    assert vsp.GetParmVal( vsp.FindParm( clone, "X_Len", "BBox" ) ) == pytest.approx( 2.0 * before )
    assert_no_errors()


def testACloneOfAPolygonMeshMeasuresTheSameAsTheMesh():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    ngon = a_polygon_mesh()

    clone = vsp.CloneGeomVec( [ ngon ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 20.0 )
    vsp.Update()

    areas = comp_geom_areas_of( [ ngon, clone ] )
    assert areas[ vsp.GetGeomName( clone ) ] == pytest.approx( areas[ vsp.GetGeomName( ngon ) ] )
    assert areas[ vsp.GetGeomName( ngon ) ] > 0.0

    # The polygon mesh is handed over unscaled, so the Clone applies the original's scaling.
    # An NGonMesh's own BBox Parms stay zero, so only the Clone's box is measured here.
    before = vsp.GetParmVal( vsp.FindParm( clone, "X_Len", "BBox" ) )
    assert before > 0.0
    vsp.SetParmVal( vsp.FindParm( ngon, "Scale", "XForm" ), 2.0 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( clone, "X_Len", "BBox" ) ) == pytest.approx( 2.0 * before )
    assert_no_errors()


def testACloneOfARouteStandsWhereItIsPut():
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    route = vsp.AddGeom( "ROUTING" )
    for u in ( 0.1, 0.9 ):
        pt = vsp.AddRoutingPt( route, pod, 0 )
        vsp.SetParmVal( vsp.FindParm( pt, "U", "RoutePt" ), u )
    vsp.Update()

    dz = 3.0
    clone = vsp.CloneGeomVec( [ route ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "Z_Rel_Location", "XForm" ), dz )
    vsp.Update()

    assert vsp.GetParmVal( vsp.FindParm( route, "X_Len", "BBox" ) ) > 0.0
    assert vsp.GetParmVal( vsp.FindParm( clone, "Z_Min", "BBox" ) ) == \
           pytest.approx( vsp.GetParmVal( vsp.FindParm( route, "Z_Min", "BBox" ) ) + dz )
    assert vsp.GetParmVal( vsp.FindParm( clone, "X_Len", "BBox" ) ) == \
           pytest.approx( vsp.GetParmVal( vsp.FindParm( route, "X_Len", "BBox" ) ) )

    # Moving the Geom the route's points sit on moves the route and the Clone.
    route_before = vsp.GetParmVal( vsp.FindParm( route, "X_Min", "BBox" ) )
    vsp.SetParmVal( vsp.FindParm( pod, "X_Rel_Location", "XForm" ), 4.0 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( route, "X_Min", "BBox" ) ) == \
           pytest.approx( route_before + 4.0 ), "the route did not follow the pod"
    assert vsp.GetParmVal( vsp.FindParm( clone, "X_Min", "BBox" ) ) == \
           pytest.approx( vsp.GetParmVal( vsp.FindParm( route, "X_Min", "BBox" ) ) )
    assert_no_errors()


def testACloneOfABlankIsACoordinateSystem():
    """A Clone of a Blank has no shape but still places its children."""
    vsp.VSPRenew()
    drop_errors()
    scratch_output()
    blank = vsp.AddGeom( "BLANK" )
    vsp.SetParmVal( vsp.FindParm( blank, "X_Rel_Location", "XForm" ), 5.0 )
    vsp.Update()
    clone = vsp.CloneGeomVec( [ blank ] )[0]
    vsp.SetParmVal( vsp.FindParm( clone, "X_Rel_Location", "XForm" ), 30.0 )
    vsp.Update()

    child = vsp.AddGeom( "POD", clone )
    vsp.SetParmVal( vsp.FindParm( child, "Trans_Attach_Flag", "Attach" ), vsp.ATTACH_TRANS_COMP )
    vsp.SetParmVal( vsp.FindParm( child, "Rots_Attach_Flag", "Attach" ), vsp.ATTACH_ROT_COMP )
    vsp.Update()

    # The child is placed in the Clone's frame.
    assert vsp.GetParmVal( vsp.FindParm( child, "X_Min", "BBox" ) ) == pytest.approx( 30.0 )
    assert_no_errors()


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
