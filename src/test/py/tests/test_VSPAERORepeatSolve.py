# VSPAERO solves the same case to the same answer however many cases came before it in the run.
# The finite difference stability derivatives take each perturbed case less the first, so a
# difference between the first solve and the later ones lands on every derivative: for this flat
# wing a sideslip or roll rate, which cannot change the lift to first order, moved it by about as
# much as the alpha step did.

import math
import os
import tempfile

import openvsp as vsp

from testhelp import assert_no_errors

ALPHA = 2.0


def flat_wing( out ):
    """A flat rectangular wing of aspect ratio 8, written out and meshed for VSPAERO."""
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.SetDriverGroup( w, 1, vsp.SPAN_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
    vsp.SetParmVal( w, "Span", "XSec_1", 4.0 )
    vsp.SetParmVal( w, "Root_Chord", "XSec_1", 1.0 )
    vsp.SetParmVal( w, "Tip_Chord", "XSec_1", 1.0 )
    vsp.SetParmVal( w, "Sweep", "XSec_1", 0.0 )
    vsp.SetParmVal( w, "SectTess_U", "XSec_1", 24 )
    vsp.SetParmVal( w, "Tess_W", "Shape", 17 )
    for i in ( 0, 1 ):
        vsp.SetParmVal( w, "ThickChord", "XSecCurve_%d" % i, 0.01 )
    vsp.Update()

    path = os.path.join( out, "wing.vsp3" )
    vsp.WriteVSPFile( path )
    vsp.SetVSP3FileName( path )
    vsp.SetAnalysisInputDefaults( "VSPAEROComputeGeometry" )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "GeomSet", [ vsp.SET_NONE ] )
    vsp.SetIntAnalysisInput( "VSPAEROComputeGeometry", "ThinGeomSet", [ vsp.SET_ALL ] )
    vsp.ExecAnalysis( "VSPAEROComputeGeometry" )


def sweep( out, npts, stability ):
    """Run VSPAERO with a fixed wake at ALPHA, npts times over, and return the results ID."""
    a = "VSPAEROSweep"
    vsp.SetAnalysisInputDefaults( a )
    vsp.SetIntAnalysisInput( a, "GeomSet", [ vsp.SET_NONE ] )
    vsp.SetIntAnalysisInput( a, "ThinGeomSet", [ vsp.SET_ALL ] )
    for name, val in ( ( "Sref", 8.0 ), ( "bref", 8.0 ), ( "cref", 1.0 ), ( "Xcg", 0.25 ),
                       ( "MachStart", 0.1 ), ( "FarAway", 1000.0 ),
                       ( "AlphaStart", ALPHA ), ( "AlphaEnd", ALPHA ) ):
        vsp.SetDoubleAnalysisInput( a, name, [ val ] )
    vsp.SetIntAnalysisInput( a, "AlphaNpts", [ npts ] )
    vsp.SetIntAnalysisInput( a, "FixedWakeFlag", [ 1 ] )
    vsp.SetIntAnalysisInput( a, "UnsteadyType", [ stability ] )
    vsp.SetIntAnalysisInput( a, "NCPU", [ 2 ] )
    vsp.SetStringAnalysisInput( a, "RedirectFile", [ os.path.join( out, "vspaero.log" ) ] )
    vsp.ExecAnalysis( a )
    return vsp.FindLatestResultsID( "VSPAERO_Stab" if stability != vsp.STABILITY_OFF else "VSPAERO_Polar" )


def test_repeated_solve_gives_the_same_lift():
    out = tempfile.mkdtemp()
    flat_wing( out )
    cl = vsp.GetDoubleResults( sweep( out, 2, vsp.STABILITY_OFF ), "CLtot" )
    assert len( cl ) == 2
    # Before the fix the second solve came out 2.2e-4 of the lift above the first.
    assert abs( cl[1] - cl[0] ) <= 1e-7 * abs( cl[0] )
    assert_no_errors()


def test_sideslip_and_roll_rate_leave_the_lift_of_a_symmetric_wing():
    out = tempfile.mkdtemp()
    flat_wing( out )
    rid = sweep( out, 1, vsp.STABILITY_DEFAULT )
    base = vsp.GetDoubleResults( rid, "Base_Aero_CL" )[0]
    beta = vsp.GetDoubleResults( rid, "Beta_CL" )[0]
    roll = vsp.GetDoubleResults( rid, "Roll__Rate_CL" )[0]
    alpha = vsp.GetDoubleResults( rid, "Alpha_CL" )[0]
    # The alpha step of 0.01 deg moves the lift by about 8e-4; the offset was 3.6e-5 on every case.
    step = alpha - base
    assert step > 0.
    assert abs( beta - base ) <= 1e-3 * step
    assert abs( roll - base ) <= 1e-3 * step

    # The derivative then agrees with two steady solves 1 deg either side.
    cla = vsp.GetDoubleResults( rid, "CL_Alpha" )[0]
    vsp.SetDoubleAnalysisInput( "VSPAEROSweep", "AlphaStart", [ ALPHA - 1.0 ] )
    vsp.SetDoubleAnalysisInput( "VSPAEROSweep", "AlphaEnd", [ ALPHA + 1.0 ] )
    vsp.SetIntAnalysisInput( "VSPAEROSweep", "AlphaNpts", [ 2 ] )
    vsp.SetIntAnalysisInput( "VSPAEROSweep", "UnsteadyType", [ vsp.STABILITY_OFF ] )
    vsp.ExecAnalysis( "VSPAEROSweep" )
    cl = vsp.GetDoubleResults( vsp.FindLatestResultsID( "VSPAERO_Polar" ), "CLtot" )
    secant = ( cl[1] - cl[0] ) / math.radians( 2.0 )
    # Before the fix CL_Alpha was 4.5 % above the steady slope.
    assert abs( cla - secant ) <= 0.005 * secant
    assert_no_errors()


if __name__ == "__main__":
    test_repeated_solve_gives_the_same_lift()
    test_sideslip_and_roll_rate_leave_the_lift_of_a_symmetric_wing()
