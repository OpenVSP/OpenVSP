# Running the intersection or the mesher on a set with nothing in it, as the Shown set is when
# everything is hidden.

import openvsp as vsp
import os

from testhelp import ( drop_errors )


def hidden_pod():
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetSetFlag( pod, vsp.SET_SHOWN, False )
    vsp.Update()


def run_on_shown( analysis, tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.SetAnalysisInputDefaults( analysis )
    vsp.SetIntAnalysisInput( analysis, "SelectedSetIndex", [ vsp.SET_SHOWN ] )
    vsp.ExecAnalysis( analysis )
    drop_errors()
    assert os.listdir( tmp_path ) == []


def testSurfaceIntersectionOnAnEmptySet( tmp_path, monkeypatch ):
    hidden_pod()
    run_on_shown( "SurfaceIntersection", tmp_path, monkeypatch )


def testCfdMeshOnAnEmptySet( tmp_path, monkeypatch ):
    hidden_pod()
    run_on_shown( "CfdMeshAnalysis", tmp_path, monkeypatch )
