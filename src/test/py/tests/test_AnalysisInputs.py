# Analysis inputs: every input an analysis looks up is one it defines.
#
# ExecAnalysis records each input name an analysis asks for and does not hold, and raises
# VSP_UNDEFINED_ANALYSIS_INPUT for it: such an input can never be set by a caller, and the
# analysis quietly goes without it.

import openvsp as vsp
import pytest


# Every analysis that runs in moments on a small model with its defaults.  The VSPAERO solver
# runs are left out; they need the solver and take minutes.
QUICK = [ "BladeElement", "CompGeom", "CpSlicer", "DegenGeom", "DegenGeomMesh", "EmintonLord",
          "GeometryAnalysis", "MassProp", "ParasiteDrag", "PlanarSlice", "Projection",
          "SurfacePatches", "WaveDrag", "VSPAEROComputeGeometry", "VSPAERODegenGeom",
          "VSPAEROReadPreviousAnalysis", "CfdMeshAnalysis", "SurfaceIntersection",
          "FeaMeshAnalysis", "FeaMeshExport" ]


def _undefined_inputs():
    """The VSP_UNDEFINED_ANALYSIS_INPUT messages on the error stack, which is emptied."""
    mgr = vsp.ErrorMgrSingleton.getInstance()
    found = []
    while mgr.GetNumTotalErrors() > 0:
        e = mgr.PopLastError()
        if e.GetErrorCode() == vsp.VSP_UNDEFINED_ANALYSIS_INPUT:
            found.append( e.GetErrorString().strip() )
    return found


@pytest.fixture( scope = "module" )
def model():
    vsp.VSPRenew()
    vsp.AddGeom( "POD" )
    wing = vsp.AddGeom( "WING" )
    prop = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( prop, "X_Rel_Location", "XForm", -2.0 )
    vsp.Update()
    vsp.AddFeaStruct( wing )
    vsp.AddFeaPart( wing, 0, vsp.FEA_RIB )
    vsp.Update()
    _undefined_inputs()


@pytest.mark.parametrize( "analysis", QUICK )
def test_every_input_looked_up_is_defined( model, analysis, tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    assert analysis in vsp.ListAnalysis()
    vsp.SetAnalysisInputDefaults( analysis )
    if analysis == "EmintonLord":
        # Its defaults are empty; a parabolic body of revolution gives it something to integrate.
        x = [ 0.1 * i for i in range( 11 ) ]
        vsp.SetDoubleAnalysisInput( analysis, "X_vec", x )
        vsp.SetDoubleAnalysisInput( analysis, "Area_vec", [ xi * ( 1.0 - xi ) for xi in x ] )
    vsp.ExecAnalysis( analysis )
    assert _undefined_inputs() == []


def test_too_few_stations_is_an_error():
    """EmintonLord with its empty default inputs reports an error rather than throwing."""
    vsp.SetAnalysisInputDefaults( "EmintonLord" )
    rid = vsp.ExecAnalysis( "EmintonLord" )
    assert rid == ""
    mgr = vsp.ErrorMgrSingleton.getInstance()
    codes = []
    while mgr.GetNumTotalErrors() > 0:
        codes.append( mgr.PopLastError().GetErrorCode() )
    assert vsp.VSP_INVALID_INPUT_VAL in codes


def test_areas_shorter_than_stations_is_an_error():
    """EmintonLord with fewer areas than stations reports an error rather than reading past them."""
    vsp.SetAnalysisInputDefaults( "EmintonLord" )
    vsp.SetDoubleAnalysisInput( "EmintonLord", "X_vec", [ 0.0, 1.0, 2.0, 3.0, 4.0 ] )
    vsp.SetDoubleAnalysisInput( "EmintonLord", "Area_vec", [ 0.0, 1.0, 0.0 ] )
    rid = vsp.ExecAnalysis( "EmintonLord" )
    assert rid == ""
    mgr = vsp.ErrorMgrSingleton.getInstance()
    codes = []
    while mgr.GetNumTotalErrors() > 0:
        codes.append( mgr.PopLastError().GetErrorCode() )
    assert vsp.VSP_INVALID_INPUT_VAL in codes
