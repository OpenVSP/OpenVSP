# VSPAERO stops with "Could not find control surface" when the case file names a control
# surface that is not in the mesh.  A control surface group can hold surfaces that a run does
# not mesh: the reflected copy when X-Z Symmetry meshes half the model, or any surface of a Geom
# outside the run's sets.  The run should still solve.

import os
import tempfile

import openvsp as vsp


def setup( out, second_wing ):
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.AddSubSurf( w, vsp.SS_CONTROL, 0 )
    other = ""
    if second_wing:
        other = vsp.AddGeom( "WING" )
        vsp.SetGeomName( other, "Tail" )
        vsp.SetParmVal( other, "X_Rel_Location", "XForm", 10.0 )
        vsp.AddSubSurf( other, vsp.SS_CONTROL, 0 )
    vsp.Update()

    # One group holding both sides of every control surface.
    k = vsp.CreateVSPAEROControlSurfaceGroup()
    n = len( vsp.GetAvailableCSNameVec( k ) )
    vsp.AddSelectedToCSGroup( list( range( 1, n + 1 ) ), k )
    vsp.Update()

    path = os.path.join( out, "wings.vsp3" )
    vsp.WriteVSPFile( path )
    vsp.SetVSP3FileName( path )
    return w, other


def run( out, thin_set, symmetry ):
    for a in ( "VSPAEROComputeGeometry", "VSPAEROSweep" ):
        vsp.SetAnalysisInputDefaults( a )
        vsp.SetIntAnalysisInput( a, "GeomSet", [ vsp.SET_NONE ] )
        vsp.SetIntAnalysisInput( a, "ThinGeomSet", [ thin_set ] )
        vsp.SetIntAnalysisInput( a, "Symmetry", [ symmetry ] )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )

    a = "VSPAEROSweep"
    vsp.SetIntAnalysisInput( a, "AlphaNpts", [ 1 ] )
    vsp.SetIntAnalysisInput( a, "WakeNumIter", [ 1 ] )
    vsp.SetIntAnalysisInput( a, "NCPU", [ 2 ] )
    vsp.SetStringAnalysisInput( a, "RedirectFile", [ os.path.join( out, "vspaero.log" ) ] )
    vsp.DeleteAllResults()
    vsp.ExecAnalysis( a )
    return vsp.GetNumResults( "VSPAERO_History" )


def test_a_half_model_runs_with_both_sides_of_a_control_surface_grouped():
    out = tempfile.mkdtemp()
    setup( out, False )
    assert run( out, vsp.SET_ALL, 1 ) == 1


def test_a_run_ignores_a_grouped_control_surface_outside_its_sets():
    out = tempfile.mkdtemp()
    w, other = setup( out, True )
    first_user_set = vsp.SET_FIRST_USER
    vsp.SetSetFlag( w, first_user_set, True )
    assert run( out, first_user_set, 0 ) == 1


if __name__ == "__main__":
    test_a_half_model_runs_with_both_sides_of_a_control_surface_grouped()
    test_a_run_ignores_a_grouped_control_surface_outside_its_sets()
