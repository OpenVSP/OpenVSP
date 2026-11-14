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


def testPartsAddUpToTheTotal():
    """Each theta keeps every part's area and the buildup of the parts from each one on."""
    comps = make_model()
    res = run( vsp.SET_ALL )

    ncomp = vsp.GetIntResults( res, "Num_Comps" )[0]
    ntheta = vsp.GetIntResults( res, "Num_Cone_Sections" )[0]
    assert ncomp == len( comps )
    assert sorted( vsp.GetStringResults( res, "Comp_ID" ) ) == sorted( comps )
    assert vsp.GetNumData( res, "Comp_Slice_Area" ) == ntheta * ncomp
    assert vsp.GetNumData( res, "Buildup_Fit_Area" ) == ntheta * ncomp
    assert vsp.GetNumData( res, "Fit_Area" ) == ntheta

    for itheta in range( ntheta ):
        total = vsp.GetDoubleResults( res, "Slice_Area", itheta )
        parts = [ vsp.GetDoubleResults( res, "Comp_Slice_Area", itheta * ncomp + icomp ) for icomp in range( ncomp ) ]
        buildup = [ vsp.GetDoubleResults( res, "Buildup_Area", itheta * ncomp + icomp ) for icomp in range( ncomp ) ]
        assert max( parts[0] ) > 0.0 and max( parts[1] ) > 0.0
        for i in range( len( total ) ):
            assert abs( sum( p[i] for p in parts ) - total[i] ) < 1e-9 * max( total )
            for icomp in range( ncomp ):
                rest = sum( p[i] for p in parts[icomp:] )
                assert abs( buildup[icomp][i] - rest ) < 1e-9 * max( total )
        assert list( vsp.GetDoubleResults( res, "Buildup_Fit_Area", itheta * ncomp ) ) == list( vsp.GetDoubleResults( res, "Fit_Area", itheta ) )
