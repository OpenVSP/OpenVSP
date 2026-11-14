# Wave Drag runs one after another, and the parts its Result keeps.

import openvsp as vsp


def make_model():
    vsp.VSPRenew()
    vsp.DeleteAllResults()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( pod, "Length", "Design", 10.0 )
    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 3.0 )
    vsp.Update()
    vsp.SetAnalysisInputDefaults( "WaveDrag" )
    vsp.SetIntAnalysisInput( "WaveDrag", "NumSlices", [ 10 ] )
    vsp.SetIntAnalysisInput( "WaveDrag", "NumRotSects", [ 3 ] )
    return [ pod, wing ]


def run( set_index ):
    vsp.SetIntAnalysisInput( "WaveDrag", "Set", [ set_index ] )
    return vsp.ExecAnalysis( "WaveDrag" )


def meshes():
    return [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Mesh" ]


def shown():
    return [ g for g in vsp.FindGeoms() if vsp.GetSetFlag( g, vsp.SET_SHOWN ) ]


def testRunsAgainOnTheShownSet():
    """The Shown set keeps its components, and the next run deletes the old MeshGeom before slicing them again."""
    comps = make_model()

    first = run( vsp.SET_SHOWN )
    assert len( meshes() ) == 1
    assert sorted( shown() ) == sorted( comps + meshes() )

    second = run( vsp.SET_SHOWN )
    assert second != ""
    assert len( meshes() ) == 1
    assert sorted( shown() ) == sorted( comps + meshes() )
    assert vsp.GetDoubleResults( second, "CDWave" )[0] > 0.0
    assert vsp.GetDoubleResults( second, "CDWave" )[0] == vsp.GetDoubleResults( first, "CDWave" )[0]
    assert list( vsp.GetDoubleResults( second, "Slice_Area" ) ) == list( vsp.GetDoubleResults( first, "Slice_Area" ) )
