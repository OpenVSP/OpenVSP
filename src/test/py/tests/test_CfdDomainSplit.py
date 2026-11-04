# A full mesh in a box outer domain with symmetry plane splitting left on.
#
# Splitting the outer domain at the symmetry plane only means something for a half mesh.  A
# full mesh with the splitting flag on must build the plain six-sided box, so its mesh is the
# mesh with the flag off, byte for byte.

import openvsp as vsp


def _pod_and_wing():
    vsp.VSPRenew()
    vsp.AddGeom( "POD", "" )
    wing = vsp.AddGeom( "WING", "" )
    vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 2.0 )
    vsp.SetParmVal( wing, "TotalSpan", "WingGeom", 8.0 )
    vsp.Update()
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.8 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.2 )
    vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, 0 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 1 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_MAX_EDGE_LEN, 4.0 )


def _mesh( path, split ):
    cfd = vsp.FindContainer( "CFDMeshSettings", 0 )
    vsp.SetParmVal( vsp.FindParm( cfd, "SymmetrySplitting", "FarField" ), split )
    vsp.SetComputationFileName( vsp.CFD_STL_TYPE, str( path ) )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_STL_TYPE )
    rid = vsp.FindLatestResultsID( "CFDMesh" )
    return open( str( path ), "rb" ).read(), vsp.GetIntResults( rid, "Water_Tight" )[0]


def testAFullMeshIgnoresSymmetrySplitting( tmp_path ):
    _pod_and_wing()
    off, tight_off = _mesh( tmp_path / "off.stl", 0 )
    on, tight_on = _mesh( tmp_path / "on.stl", 1 )

    assert tight_off == 1
    assert tight_on == 1, "the full mesh with splitting on did not close"
    assert on == off, "splitting changed a full mesh, which has no symmetry plane"


def testAFullMeshThroughTheAnalysisIgnoresSymmetrySplitting( tmp_path, monkeypatch ):
    """The same model setting reached through CfdMeshAnalysis, with the analysis's half mesh
    input off and the model's splitting on."""
    monkeypatch.chdir( tmp_path )
    _pod_and_wing()
    cfd = vsp.FindContainer( "CFDMeshSettings", 0 )
    vsp.SetParmVal( vsp.FindParm( cfd, "SymmetrySplitting", "FarField" ), 1 )
    vsp.SetAnalysisInputDefaults( "CfdMeshAnalysis" )
    vsp.SetIntAnalysisInput( "CfdMeshAnalysis", "GenerateHalfMesh", [ 0 ] )
    vsp.ExecAnalysis( "CfdMeshAnalysis" )
    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert rid
    assert vsp.GetIntResults( rid, "Water_Tight" )[0] == 1
