# A subsurface's Parms reaching the model.
#
# A subsurface's Parms do not arrive through the Geom, so the Geom must be told a subsurface
# changed or its UW curve stays stale.

import openvsp as vsp
import os
import tempfile


def fresh():
    vsp.VSPRenew()
    out = tempfile.mkdtemp()
    for t, n in ( ( vsp.COMP_GEOM_TXT_TYPE, "cg.txt" ), ( vsp.COMP_GEOM_CSV_TYPE, "cg.csv" ) ):
        vsp.SetComputationFileName( t, os.path.join( out, n ) )
    pop_errors()
    return out


def pop_errors():
    mgr = vsp.ErrorMgrSingleton.getInstance()
    return [ mgr.PopLastError().m_ErrorString for _ in range( mgr.GetNumTotalErrors() ) ]


def subsurf_wet_area():
    """The area CompGeom tags to the subsurface; deletes the mesh it makes."""
    mesh_id = vsp.ComputeCompGeom( vsp.SET_ALL, False, vsp.COMP_GEOM_CSV_TYPE )
    results = vsp.FindLatestResultsID( "Comp_Geom" )
    areas = vsp.GetDoubleResults( results, "SubSurf_Wet_Area" )
    if mesh_id:
        vsp.DeleteGeom( mesh_id )
        vsp.Update()
    assert areas, "CompGeom tagged no area to the subsurface"
    return areas[0]


def testChangingASubSurfacesExtentReachesTheModel():
    """Widening a rectangle subsurface widens the area tagged to it, which needs the Geom to
    rebuild the subsurface's UW curve."""
    fresh()
    wing = vsp.AddGeom( "WING" )
    vsp.Update()
    sub = vsp.AddSubSurf( wing, vsp.SS_RECTANGLE )
    vsp.Update()

    before = subsurf_wet_area()

    u_length = vsp.FindParm( sub, "U_Length", "SS_Rectangle" )
    assert u_length, "the rectangle has no U_Length, so nothing is measured"
    vsp.SetParmVal( u_length, 2.0 * vsp.GetParmVal( u_length ) )
    vsp.Update()

    after = subsurf_wet_area()
    assert after > 1.5 * before, \
           "widening the subsurface left the tagged area at %g (was %g)" % ( after, before )
    pop_errors()


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
