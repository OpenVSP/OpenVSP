# The Cp slicer reads every case of a VSPAERO run that has a control surface group.  Each case in
# the aerothermal database ends with the control surface deflection angles; vsploads read them
# once, after its loop over the cases, so from the second case on it read the mesh data from the
# wrong place and crashed, and the run came back with the first case's slices only.

import os
import tempfile

import openvsp as vsp

from testhelp import assert_no_errors

ALPHAS = ( 0.0, 2.0, 4.0 )


def wing_with_control_group( out ):
    """A flat rectangular wing with one trailing-edge control surface, in a VSPAERO control group."""
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.SetDriverGroup( w, 1, vsp.SPAN_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
    vsp.SetParmVal( w, "Span", "XSec_1", 4.0 )
    vsp.SetParmVal( w, "Root_Chord", "XSec_1", 1.0 )
    vsp.SetParmVal( w, "Tip_Chord", "XSec_1", 1.0 )
    vsp.SetParmVal( w, "Sweep", "XSec_1", 0.0 )
    vsp.SetParmVal( w, "SectTess_U", "XSec_1", 24 )
    vsp.SetParmVal( w, "Tess_W", "Shape", 17 )
    vsp.AddSubSurf( w, vsp.SS_CONTROL, 0 )
    vsp.Update()

    k = vsp.CreateVSPAEROControlSurfaceGroup()
    vsp.AddSelectedToCSGroup( [ 1, 2 ], k )
    vsp.Update()

    path = os.path.join( out, "wing.vsp3" )
    vsp.WriteVSPFile( path )
    vsp.SetVSP3FileName( path )
    vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_NONE ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ vsp.SET_ALL ] )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )


def test_cp_slices_cover_every_case_with_a_control_group():
    out = tempfile.mkdtemp()
    wing_with_control_group( out )

    a = "VSPAEROSweep"
    vsp.SetAnalysisInputDefaults( a )
    vsp.SetIntAnalysisInput( a, "GeomSet", [ vsp.SET_NONE ] )
    vsp.SetIntAnalysisInput( a, "ThinGeomSet", [ vsp.SET_ALL ] )
    for name, val in ( ( "Sref", 8.0 ), ( "bref", 8.0 ), ( "cref", 1.0 ), ( "Xcg", 0.25 ), ( "MachStart", 0.1 ),
                       ( "AlphaStart", ALPHAS[0] ), ( "AlphaEnd", ALPHAS[-1] ) ):
        vsp.SetDoubleAnalysisInput( a, name, [ val ] )
    vsp.SetIntAnalysisInput( a, "AlphaNpts", [ len( ALPHAS ) ] )
    vsp.SetIntAnalysisInput( a, "FixedWakeFlag", [ 1 ] )
    vsp.SetIntAnalysisInput( a, "NCPU", [ 2 ] )
    vsp.SetStringAnalysisInput( a, "RedirectFile", [ os.path.join( out, "vspaero.log" ) ] )

    # The default setup slices at Y = 0 after the solve.
    vsp.DeleteAllResults()
    vsp.ExecAnalysis( a )

    assert vsp.GetNumResults( "CpSlicer_Case" ) == len( ALPHAS )
    for i in range( len( ALPHAS ) ):
        rid = vsp.FindResultsID( "CpSlicer_Case", i )
        assert vsp.GetIntResults( rid, "Case" )[0] == i + 1
        assert abs( vsp.GetDoubleResults( rid, "Alpha" )[0] - ALPHAS[i] ) < 1e-6
        assert len( vsp.GetDoubleResults( rid, "Cp" ) ) > 0
    assert_no_errors()


if __name__ == "__main__":
    test_cp_slices_cover_every_case_with_a_control_group()
