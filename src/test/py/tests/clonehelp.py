# Shared helpers for the Clone tests and the Geom bug tests.
#
# Not a test file -- pytest collects test_*.py, which import this.

import openvsp as vsp
import os
import tempfile

from testhelp import ( pop_errors, drop_errors, assert_no_errors, a_wire_file )


BBOX_PARMS = ( "X_Min", "X_Len", "Y_Min", "Y_Len", "Z_Min", "Z_Len" )


def box( gid ):
    """The Geom's placed bounding box."""
    return tuple( vsp.GetParmVal( vsp.FindParm( gid, n, "BBox" ) ) for n in BBOX_PARMS )


def switch( gid, name, on ):
    """Set one of a Clone's Behavior switches."""
    vsp.SetParmVal( vsp.FindParm( gid, name, "Behavior" ), 1.0 if on else 0.0 )
    vsp.Update()


def _by_name( res, field ):
    """One CompGeom column totalled per component name.

    Totalled because a symmetric Geom, or two Clones with the same name, give several rows
    under one name.  Column lengths are checked since zip would silently truncate.
    """
    names = list( vsp.GetStringResults( res, "Comp_Name" ) )
    vals = list( vsp.GetDoubleResults( res, field ) )
    assert len( names ) == len( vals ), \
        "%s has %d values for %d components" % ( field, len( vals ), len( names ) )

    totals = {}
    for name, val in zip( names, vals ):
        totals[name] = totals.get( name, 0.0 ) + val
    return totals


def comp_geom_areas():
    """Wetted area per component name, from CompGeom.

    Leaves a MeshGeom in the model, so call it once per test.
    """
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    return _by_name( vsp.FindLatestResultsID( "Comp_Geom" ), "Theo_Area" )


def comp_geom_volumes():
    """Theoretical volume per component name, from CompGeom."""
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    return _by_name( vsp.FindLatestResultsID( "Comp_Geom" ), "Theo_Vol" )


def comp_geom_of( gids ):
    """Wetted area and theoretical volume per component, measuring only the Geoms named.

    One run gives both, since each run leaves a MeshGeom that a second run would also measure.
    """
    user_set = 3
    for g in vsp.FindGeoms():
        vsp.SetSetFlag( g, user_set, g in gids )
    vsp.Update()
    vsp.ComputeCompGeom( user_set, False, 0 )
    res = vsp.FindLatestResultsID( "Comp_Geom" )
    areas = _by_name( res, "Theo_Area" )
    vols = _by_name( res, "Theo_Vol" )
    assert len( areas ) == len( gids ), "measured %s, asked for %s" % ( list( areas ), gids )
    return areas, vols


def comp_geom_areas_of( gids ):
    """Wetted area per component, measuring only the Geoms named."""
    return comp_geom_of( gids )[0]


def total_mass():
    """The whole model's mass, which shows a point mass counted twice."""
    vsp.ComputeMassProps( vsp.SET_ALL, 20, vsp.X_DIR )
    res = vsp.FindLatestResultsID( "Mass_Properties" )
    return list( vsp.GetDoubleResults( res, "Total_Mass" ) )[0]


def scratch_output():
    """Send the analyses' text output to a scratch directory, not into the source tree."""
    out = tempfile.mkdtemp()
    vsp.SetComputationFileName( vsp.COMP_GEOM_TXT_TYPE, os.path.join( out, "comp_geom.txt" ) )
    vsp.SetComputationFileName( vsp.COMP_GEOM_CSV_TYPE, os.path.join( out, "comp_geom.csv" ) )
    vsp.SetComputationFileName( vsp.MASS_PROP_TXT_TYPE, os.path.join( out, "mass_props.txt" ) )
    vsp.SetComputationFileName( vsp.DEGEN_GEOM_CSV_TYPE, os.path.join( out, "degen_geom.csv" ) )
    vsp.SetComputationFileName( vsp.DEGEN_GEOM_M_TYPE, os.path.join( out, "degen_geom.m" ) )
    return out


def assert_refused( word ):
    """Assert an error was reported that mentions the given word.

    Drain the error stack before the call under test, or an earlier error answers for it.
    """
    msgs = pop_errors()
    assert msgs, "nothing was refused, so the call went through"
    assert any( word.lower() in m.lower() for m in msgs ), \
           "refused, but never said %r: %s" % ( word, msgs )


def a_mesh():
    """A MeshGeom, by meshing a pod and throwing the pod away."""
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    vsp.DeleteGeom( pod )
    vsp.Update()
    return [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "Mesh" ][0]


def a_wireframe():
    """A WireFrame Geom, from the smallest plot3d file that makes a surface."""
    vsp.ImportFile( a_wire_file(), vsp.IMPORT_P3D_WIRE, "" )
    vsp.Update()
    return [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "WireFrame" ][0]


def a_point_cloud():
    """A PtCloud Geom, by taking the points of a mesh and throwing the mesh away."""
    mesh = a_mesh()
    cloud = vsp.CreatePtCloudGeom( mesh )
    vsp.DeleteGeom( mesh )
    vsp.Update()
    assert vsp.GetGeomTypeName( cloud ) == "PtCloud"
    return cloud


def a_polygon_mesh():
    """An NGonMesh Geom, a child of its source mesh; the mesh is kept since deleting it would
    delete the NGon too."""
    mesh = a_mesh()
    ngon = vsp.CreateNGonMeshGeom( mesh )
    vsp.Update()
    assert vsp.GetGeomTypeName( ngon ) == "NGonMesh"
    return ngon


def a_route():
    """A Routing Geom threaded through two points on a pod."""
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    route = vsp.AddGeom( "ROUTING" )
    for u in ( 0.1, 0.9 ):
        pt = vsp.AddRoutingPt( route, pod, 0 )
        vsp.SetParmVal( vsp.FindParm( pt, "U", "RoutePt" ), u )
    vsp.Update()
    return route


def degen_rows():
    """Name, type, symmetric copy index and the translation each degenerate entry records."""
    rows = []
    for i in range( vsp.GetNumResults( "Degen_DegenGeom" ) ):
        res = vsp.FindResultsID( "Degen_DegenGeom", i )
        name = list( vsp.GetStringResults( res, "name" ) )
        kind = list( vsp.GetStringResults( res, "type" ) )
        sym = list( vsp.GetIntResults( res, "sym_copy_index" ) )
        mat = list( vsp.GetDoubleResults( res, "transmat" ) )
        rows.append( ( name[0], kind[0], sym[0], mat[12], mat[13], mat[14] ) )
    return rows
