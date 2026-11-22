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
    # Control surfaces work with a specific Set, chosen in VSPAERO's own settings as well as for
    # the run.
    settings = vsp.FindContainer( "VSPAEROSettings", 0 )
    vsp.SetParmVal( vsp.FindParm( settings, "GeomSet", "VSPAERO" ), vsp.SET_NONE )
    vsp.SetParmVal( vsp.FindParm( settings, "ThinGeomSet", "VSPAERO" ), thin_set )
    vsp.SetParmVal( vsp.FindParm( settings, "Symmetry", "VSPAERO" ), symmetry )
    vsp.Update()

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


def case_file_surfaces( out ):
    # The surface names of the control groups the last run wrote to its case file.
    lines = open( os.path.join( out, "wings.vspaero" ) ).read().splitlines()
    names = []
    for i, line in enumerate( lines ):
        if line.startswith( "NumberOfControlGroups" ):
            for g in range( int( line.split( "=" )[1] ) ):
                names += lines[ i + 2 + 4 * g ].split( "," )
    return names


def test_a_group_holds_only_surfaces_in_the_run_sets():
    out = tempfile.mkdtemp()
    w, other = setup( out, True )
    assert len( vsp.GetActiveCSNameVec( 0 ) ) == 4

    first_user_set = vsp.SET_FIRST_USER
    vsp.SetSetFlag( w, first_user_set, True )
    run( out, first_user_set, 0 )

    # The Tail's surfaces leave the group, are not written and are not offered for a new group.
    names = vsp.GetActiveCSNameVec( 0 )
    assert len( names ) == 2
    assert all( n.startswith( "WingGeom_" ) for n in names )
    assert sorted( case_file_surfaces( out ) ) == sorted( names )

    k = vsp.CreateVSPAEROControlSurfaceGroup()
    assert all( n.startswith( "WingGeom_" ) for n in vsp.GetAvailableCSNameVec( k ) )


def test_a_half_model_writes_only_the_side_it_meshes():
    out = tempfile.mkdtemp()
    setup( out, False )
    run( out, vsp.SET_ALL, 1 )

    written = case_file_surfaces( out )
    assert written == [ "WingGeom_Surf0_SS_CONT_0" ]

    # The -Y copy is not offered for a new group in a half model.
    k = vsp.CreateVSPAEROControlSurfaceGroup()
    assert all( "_Surf1_" not in n for n in vsp.GetAvailableCSNameVec( k ) )
    vsp.DeleteVSPAEROControlSurfaceGroup( k )

    # Both sides stay in the group, and a full model writes both again.
    assert len( vsp.GetActiveCSNameVec( 0 ) ) == 2
    run( out, vsp.SET_ALL, 0 )
    assert sorted( case_file_surfaces( out ) ) == [ "WingGeom_Surf0_SS_CONT_0", "WingGeom_Surf1_SS_CONT_0" ]


if __name__ == "__main__":
    test_a_half_model_runs_with_both_sides_of_a_control_surface_grouped()
    test_a_run_ignores_a_grouped_control_surface_outside_its_sets()
    test_a_group_holds_only_surfaces_in_the_run_sets()
    test_a_half_model_writes_only_the_side_it_meshes()
