# What a meshing operation measures, and what it leaves behind, has to be at the model's scale.
#
# MeshGeom does its intersection work at a fixed size: it scales the triangles up so the largest
# dimension is 1000 units, intersects them, and scales them back.  Because that working scale is
# 1000 over the model's own size, a failure to scale back divides the model out of every answer --
# the reported areas and volumes stop depending on how big the model is at all.  Nothing else in
# the suite would notice: mass properties come from MassSlice, which works from the triangles
# directly and never scales them, and the analysis tests print CompGeom's results rather than
# checking them.

import openvsp as vsp
import pytest

import os
import tempfile


def _scratch_output():
    """Send CompGeom's text and CSV output to a scratch directory, not into the source tree.

    The test runs with the source directory as its working directory, so an analysis that writes
    a report writes it next to the tests.
    """
    out = tempfile.mkdtemp()
    vsp.SetComputationFileName( vsp.COMP_GEOM_TXT_TYPE, os.path.join( out, "comp_geom.txt" ) )
    vsp.SetComputationFileName( vsp.COMP_GEOM_CSV_TYPE, os.path.join( out, "comp_geom.csv" ) )


def _comp_geom( scale = 1.0 ):
    """Mesh a single pod and give back the totals CompGeom reports for it."""
    vsp.VSPRenew()
    _scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( vsp.FindParm( pod, "Scale", "XForm" ), scale )
    vsp.Update()

    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )

    rid = vsp.FindLatestResultsID( "Comp_Geom" )
    return ( sum( vsp.GetDoubleResults( rid, "Theo_Area" ) ),
             sum( vsp.GetDoubleResults( rid, "Theo_Vol" ) ) )


def testCompGeomMeasuresTheModelAndNotTheScaleItWorksAt():
    """Twice the model is four times the area and eight times the volume.

    Left at the working scale the two come back equal instead, because the scale factor is
    itself 1000 over the model's size.
    """
    area1, vol1 = _comp_geom( 1.0 )
    area2, vol2 = _comp_geom( 2.0 )

    assert area1 > 0.0 and vol1 > 0.0
    assert area2 / area1 == pytest.approx( 4.0, rel = 0.01 ), \
           "wetted area did not follow the model: %g then %g" % ( area1, area2 )
    assert vol2 / vol1 == pytest.approx( 8.0, rel = 0.01 ), \
           "volume did not follow the model: %g then %g" % ( vol1, vol2 )


def testAMeshedComponentIsTheSizeOfWhatWasMeshed():
    """And the MeshGeom left in the model is the size of the Geom it was made from."""
    vsp.VSPRenew()
    _scratch_output()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()

    want_min = vsp.GetGeomBBoxMin( pod )
    want_max = vsp.GetGeomBBoxMax( pod )

    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    vsp.DeleteGeom( pod )
    vsp.Update()

    meshes = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Mesh" ]
    assert len( meshes ) == 1, "CompGeom left %d MeshGeoms" % len( meshes )
    mesh = meshes[0]

    got_min = vsp.GetGeomBBoxMin( mesh )
    got_max = vsp.GetGeomBBoxMax( mesh )

    # A mesh is a tessellation of the surface, so its box sits just inside the surface's own.
    span = want_max.x() - want_min.x()
    for axis in ( "x", "y", "z" ):
        for got, want, end in ( ( got_min, want_min, "min" ), ( got_max, want_max, "max" ) ):
            g = getattr( got, axis )()
            w = getattr( want, axis )()
            assert g == pytest.approx( w, abs = 0.02 * span ), \
                   "meshed pod %s %s %g, expected about %g" % ( axis, end, g, w )


def testAMeshsBBoxParmsAgreeWithTheMesh():
    """The BBox Parms are what the GUI, the API and an AdvLink read, so they have to agree."""
    vsp.VSPRenew()
    _scratch_output()
    vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    vsp.Update()

    mesh = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Mesh" ][0]
    mn = vsp.GetGeomBBoxMin( mesh )
    mx = vsp.GetGeomBBoxMax( mesh )

    for parm, got in ( ( "X_Len", mx.x() - mn.x() ),
                       ( "Y_Len", mx.y() - mn.y() ),
                       ( "Z_Len", mx.z() - mn.z() ) ):
        assert vsp.GetParmVal( vsp.FindParm( mesh, parm, "BBox" ) ) == pytest.approx( got, rel = 1e-6 ), \
               "%s disagrees with the box the mesh occupies" % parm


def testScalingAMeshScalesIt():
    """A mesh scales by what its Scale Parm says, from wherever the meshing left it."""
    vsp.VSPRenew()
    _scratch_output()
    vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    vsp.Update()

    mesh = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Mesh" ][0]
    xlen = vsp.FindParm( mesh, "X_Len", "BBox" )
    before = vsp.GetParmVal( xlen )
    assert before > 0.0

    vsp.SetParmVal( vsp.FindParm( mesh, "Scale", "XForm" ), 3.0 )
    vsp.Update()
    assert vsp.GetParmVal( xlen ) == pytest.approx( 3.0 * before )

    vsp.SetParmVal( vsp.FindParm( mesh, "Scale", "XForm" ), 1.0 )
    vsp.Update()
    assert vsp.GetParmVal( xlen ) == pytest.approx( before )
