"""Models whose surfaces meet exactly, or nearly so, must still mesh closed.

Each case here is one of a catalog of near-coincident models -- surfaces that coincide, touch,
or meet where they are tangent -- that the CFD mesher once left open or empty.  The mesh is
coarse: these are about whether the surfaces are cut and joined, not about the mesh on them.
"""

import openvsp as vsp
import pytest


def _mesh( size ):
    """Mesh everything at edge lengths a tenth and a hundredth of size; the CFDMesh results."""
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, size / 10.0 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, size / 100.0 )
    vsp.SetCFDMeshVal( vsp.CFD_MAX_GAP, size / 200.0 )
    vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 1 )

    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, 0 )

    rid = vsp.FindLatestResultsID( "CFDMesh" )
    assert rid, "the mesher wrote no CFDMesh results entry"

    out = {}
    for name in ( "Num_Tris", "Num_Border_Edges", "Num_Over_Connected_Edges" ):
        out[name] = vsp.GetIntResults( rid, name )[0]
    return out


def _add( kind ):
    gid = vsp.AddGeom( kind )
    if kind == "POD":
        vsp.SetParmVal( gid, "Length", "Design", 4.0 )
        vsp.SetParmVal( gid, "FineRatio", "Design", 6.0 )
    return gid


@pytest.mark.parametrize( "kind", [ "POD", "WING" ] )
def test_duplicate_geom_meshes_once( kind, tmp_path, monkeypatch ):
    """A Geom given twice, one exactly on the other, is meshed as the one body it makes."""
    monkeypatch.chdir( tmp_path )

    vsp.VSPRenew()
    _add( kind )
    vsp.Update()
    single = _mesh( 4.0 )

    vsp.VSPRenew()
    _add( kind )
    _add( kind )
    vsp.Update()
    double = _mesh( 4.0 )

    assert double["Num_Tris"] > 0, "both copies were thrown away"
    assert double["Num_Border_Edges"] == 0
    assert double["Num_Over_Connected_Edges"] == 0
    assert double["Num_Tris"] == single["Num_Tris"]
