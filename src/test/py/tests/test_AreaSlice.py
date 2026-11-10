# Planar slice results: every slice reports a center, even one that cuts nothing.

import openvsp as vsp
import pytest

from testhelp import ( drop_errors, assert_no_errors )


@pytest.mark.parametrize( "norm, offset", [
    ( ( 1.0, 0.0, 0.0 ), ( 0.0, 0.0, 0.0 ) ),
    ( ( 1.0, 0.0, 0.0 ), ( 0.0, 3.0, -2.0 ) ),
    ( ( 0.0, 0.0, 1.0 ), ( 1.0, 3.0, -2.0 ) ),
] )
def testAnEmptySliceStillHasACenter( norm, offset, tmp_path, monkeypatch ):
    """The end slices of a pod touch only its tips, so they have no area.

    Their center must still lie on the slice plane, in the middle of the section, not at the origin.
    """
    # The slice writes a results file into the working directory.
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( pod, "X_Rel_Location", "XForm", offset[0] )
    vsp.SetParmVal( pod, "Y_Rel_Location", "XForm", offset[1] )
    vsp.SetParmVal( pod, "Z_Rel_Location", "XForm", offset[2] )
    vsp.Update()

    nslice = 6
    vsp.ComputePlaneSlice( vsp.SET_ALL, nslice, vsp.vec3d( norm[0], norm[1], norm[2] ), True )
    res = vsp.FindLatestResultsID( "Slice" )

    areas = vsp.GetDoubleResults( res, "Slice_Area" )
    locs = vsp.GetDoubleResults( res, "Slice_Loc" )
    centers = vsp.GetVec3dResults( res, "Slice_Area_Center" )
    assert len( areas ) == nslice
    assert len( centers ) == nslice

    # The pod's middle, from its bounding box.
    mid = []
    for axis in ( "X", "Y", "Z" ):
        lo = vsp.GetParmVal( vsp.FindParm( pod, axis + "_Min", "BBox" ) )
        span = vsp.GetParmVal( vsp.FindParm( pod, axis + "_Len", "BBox" ) )
        mid.append( lo + 0.5 * span )

    iaxis = norm.index( 1.0 )
    length = vsp.GetParmVal( pod, "Length", "Design" )
    tol = 1e-6 * length
    # The tessellated pod is only nearly round.
    facet_tol = 1e-3 * length
    empty = 0
    for s in range( nslice ):
        c = [ centers[s].x(), centers[s].y(), centers[s].z() ]
        if areas[s] == 0.0:
            empty += 1
            # The old answer for an empty slice was the origin.
            assert c != [ 0.0, 0.0, 0.0 ]
        # Every center lies on its slice plane ...
        assert c[iaxis] == pytest.approx( locs[s], abs=tol )
        # ... and, the pod being symmetric, on its axis.
        for k in range( 3 ):
            if k != iaxis and ( iaxis == 0 or k != 0 ):
                assert c[k] == pytest.approx( mid[k], abs=facet_tol )

    # Slicing along x, the first and last slices touch only the pod's tips.
    if iaxis == 0:
        assert areas[0] == 0.0
        assert areas[-1] == 0.0
        assert empty >= 2
    assert_no_errors()
