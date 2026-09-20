# Deleting one Geom, and what becomes of whatever was parented under it.

import openvsp as vsp
import pytest

import os
import tempfile


def _scratch():
    """Send analysis reports to a scratch directory.  Every VSPRenew resets these."""
    out = tempfile.mkdtemp()
    for t in ( vsp.COMP_GEOM_TXT_TYPE, vsp.COMP_GEOM_CSV_TYPE, vsp.MASS_PROP_TXT_TYPE ):
        vsp.SetComputationFileName( t, os.path.join( out, "report.txt" ) )
    vsp.SetVSP3FileName( os.path.join( out, "model.vsp3" ) )
    return out


def _leftover_mesh():
    """The one MeshGeom an analysis left in the model, or None."""
    meshes = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) in ( "Mesh", "NGonMesh" ) ]
    return meshes[0] if len( meshes ) == 1 else None


#==== Deleting a Geom directly ====#

def testDeletingALoneGeomRemovesIt():
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()

    vsp.DeleteGeom( pod )
    vsp.Update()

    assert len( vsp.FindGeoms() ) == 0


def testDeletingAGeomKeepsWhatWasUnderIt():
    """Children come up to where the deleted Geom was."""
    vsp.VSPRenew()
    parent = vsp.AddGeom( "POD" )
    child = vsp.AddGeom( "POD", parent )
    grandchild = vsp.AddGeom( "POD", child )
    vsp.Update()

    assert vsp.GetGeomParent( child ) == parent
    assert vsp.GetGeomParent( grandchild ) == child

    vsp.DeleteGeom( parent )
    vsp.Update()

    left = sorted( vsp.FindGeoms() )
    assert left == sorted( [ child, grandchild ] ), "deleting one Geom left %r" % left
    assert vsp.GetGeomParent( child ) in ( "", "NONE" ), \
           "the child names %r, which is gone" % vsp.GetGeomParent( child )
    assert vsp.GetGeomParent( grandchild ) == child, "the rest of the tree moved"


def testDeletingAGeomLeavesNoDanglingParent():
    """Every surviving Geom names a parent that is in the model, or none."""
    vsp.VSPRenew()
    parent = vsp.AddGeom( "POD" )
    vsp.AddGeom( "POD", parent )
    vsp.Update()

    vsp.DeleteGeom( parent )
    vsp.Update()

    alive = list( vsp.FindGeoms() )
    assert len( alive ) == 1, "expected the child to survive, found %d Geoms" % len( alive )

    for g in alive:
        par = vsp.GetGeomParent( g )
        assert par in ( "", "NONE" ) or par in alive, \
               "%s names parent %r, which is not in the model" % ( vsp.GetGeomName( g ), par )


#==== Analyses that keep their mesh between runs ====#

def _run_twice_with_a_geom_under_the_mesh( analysis ):
    """Run the analysis, park a Pod under the mesh it leaves, run it again."""
    vsp.VSPRenew()
    _scratch()
    vsp.AddGeom( "WING" )
    vsp.Update()

    vsp.SetAnalysisInputDefaults( analysis )
    vsp.ExecAnalysis( analysis )
    vsp.Update()

    mesh = _leftover_mesh()
    if mesh is None:
        pytest.skip( "%s left no mesh to park a Geom under" % analysis )

    pod = vsp.AddGeom( "POD", mesh )
    vsp.Update()
    assert vsp.GetGeomParent( pod ) == mesh

    vsp.ExecAnalysis( analysis )
    vsp.Update()

    left = list( vsp.FindGeoms() )
    assert pod in left, "%s run twice lost the Geom parented under its mesh" % analysis
    assert vsp.GetGeomParent( pod ) in ( "", "NONE" ) or vsp.GetGeomParent( pod ) in left, \
           "%s left the Geom naming a parent that is gone" % analysis


def testWaveDragTwiceKeepsWhatWasParkedUnderItsMesh():
    _run_twice_with_a_geom_under_the_mesh( "WaveDrag" )


def testMassPropertiesTwiceKeepsWhatWasParkedUnderItsMesh():
    _run_twice_with_a_geom_under_the_mesh( "MassProp" )


def testRepeatedAnalysesLeaveTheModelAsTheyFoundIt():
    """An analysis that makes and drops a mesh leaves the Geom count where it started."""
    vsp.VSPRenew()
    _scratch()
    vsp.AddGeom( "WING" )
    vsp.Update()

    vsp.SetAnalysisInputDefaults( "MassProp" )

    counts = []
    for _ in range( 3 ):
        vsp.ExecAnalysis( "MassProp" )
        vsp.Update()
        counts.append( len( vsp.FindGeoms() ) )

    assert counts[0] == counts[1] == counts[2], \
           "the model grew as the analysis was repeated: %s" % counts
