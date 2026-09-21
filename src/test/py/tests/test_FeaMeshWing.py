# A wing with a structure in it, built here rather than loaded, so the mesher's structures path
# has a case that travels with the source.
#
# The skin is the one surface in a structure the feature-based splitting applies to -- a wing
# skin is carved into a trailing edge, a leading edge, two end caps and the upper and lower
# surface -- while the spars and ribs are plain surfaces cut from planes.  What is pinned here
# is that the two go together: that every part reaches the mesh, and that the answer does not
# depend on how many threads ran or on whether the skin was carved.
#
# The NASTRAN deck states its own counts in its header, which is the only place any FEA output
# says how much mesh it holds.

import os
import re

import openvsp as vsp
import pytest


def _errors():
    """How many errors are on the API stack.

    The stack is never emptied, so a count is only meaningful against an earlier count from the
    same process.
    """
    return vsp.ErrorMgrSingleton.getInstance().GetNumTotalErrors()


def _wing_with_a_structure():
    """A wing carrying a spar and two ribs, with both end caps rounded."""
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING", "" )

    vsp.SetParmVal( vsp.FindParm( wid, "CapUMinOption", "EndCap" ), vsp.ROUND_END_CAP )
    vsp.SetParmVal( vsp.FindParm( wid, "CapUMaxOption", "EndCap" ), vsp.ROUND_END_CAP )
    vsp.Update()

    vsp.AddFeaStruct( wid )

    spar = vsp.AddFeaPart( wid, 0, vsp.FEA_SPAR )
    vsp.SetParmVal( vsp.FindParm( spar, "RelCenterLocation", "FeaPart" ), 0.4 )

    for loc in ( 0.35, 0.7 ):
        rib = vsp.AddFeaPart( wid, 0, vsp.FEA_RIB )
        vsp.SetParmVal( vsp.FindParm( rib, "RelCenterLocation", "FeaPart" ), loc )

    vsp.Update()
    return wid


def _counts( wid, nastran ):
    """Mesh the structure and read the counts out of the NASTRAN deck's header."""
    vsp.SetFeaMeshVal( wid, 0, vsp.CFD_MAX_EDGE_LEN, 0.5 )
    vsp.SetFeaMeshVal( wid, 0, vsp.CFD_MIN_EDGE_LEN, 0.1 )

    vsp.SetFeaMeshFileName( wid, 0, vsp.FEA_NASTRAN_FILE_NAME, str( nastran ) )
    vsp.ComputeFeaMesh( vsp.GetFeaStructID( wid, 0 ), vsp.FEA_NASTRAN_FILE_NAME )

    assert os.path.exists( str( nastran ) ), "the FEA mesher wrote no NASTRAN deck"

    head = open( str( nastran ) ).read()
    out = {}
    for name in ( "Num_Nodes", "Num_Els", "Num_Tris", "Num_Quads", "Num_Beams" ):
        m = re.search( r"^\$ %s:\s+(\d+)" % name, head, re.M )
        assert m, "the NASTRAN deck states no %s" % name
        out[name] = int( m.group( 1 ) )

    return out


def test_AWingStructureMeshes( tmp_path ):
    """A wing with a spar and two ribs meshes, and the deck it writes holds a mesh.

    Asserting on the counts rather than on the file size: a deck with every count at zero is
    still a valid file with a header in it.
    """
    n0 = _errors()
    wid = _wing_with_a_structure()

    counts = _counts( wid, tmp_path / "wing.nas" )

    assert _errors() == n0, "meshing a wing structure raised an error"
    assert counts["Num_Nodes"] > 100, "expected a mesh, got %d nodes" % counts["Num_Nodes"]
    assert counts["Num_Els"] > 100, "expected elements, got %d" % counts["Num_Els"]
    assert counts["Num_Els"] == counts["Num_Tris"] + counts["Num_Quads"] + counts["Num_Beams"], \
           "the element count does not add up: %s" % counts


def test_TheStructureMeshDoesNotDependOnTheThreadCount( tmp_path ):
    """Meshing a structure on several threads gives the same mesh as meshing it on one."""
    n0 = _errors()
    wid = _wing_with_a_structure()

    vsp.SetFeaMeshVal( wid, 0, vsp.CFD_PARALLEL_MESH_FLAG, 0.0 )
    one = _counts( wid, tmp_path / "one.nas" )

    vsp.SetFeaMeshVal( wid, 0, vsp.CFD_PARALLEL_MESH_FLAG, 1.0 )
    many = _counts( wid, tmp_path / "many.nas" )

    assert _errors() == n0, "meshing a wing structure raised an error"
    assert one == many, "one thread gave %s, several gave %s" % ( one, many )


def test_TheSkinCanBeMeshedWithoutBeingCarved( tmp_path ):
    """The structure meshes whether or not the skin is carved into its features.

    With the splitting on, the skin is cut into the features it carries; with it off it goes to
    the mesher whole.  On a wing structure the two come out the same, because the end caps -- the
    part of the carving that changes a wing's patches most -- are never asked for on this path:
    FeaPart::FetchFeaXFerSurf leaves the cap flags at their defaults.  What is pinned here is
    that the setting is carried into a structure and that neither value loses the mesh.
    """
    n0 = _errors()
    wid = _wing_with_a_structure()

    vsp.SetFeaMeshVal( wid, 0, vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 1.0 )
    assert vsp.GetFeaMeshVal( wid, 0, vsp.CFD_SPLIT_JOIN_SURFS_FLAG ) == 1.0, \
           "the structure did not take the splitting flag"
    carved = _counts( wid, tmp_path / "carved.nas" )

    vsp.SetFeaMeshVal( wid, 0, vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 0.0 )
    assert vsp.GetFeaMeshVal( wid, 0, vsp.CFD_SPLIT_JOIN_SURFS_FLAG ) == 0.0, \
           "the structure did not take the splitting flag"
    whole = _counts( wid, tmp_path / "whole.nas" )

    assert _errors() == n0, "meshing a wing structure raised an error"

    for tag, c in ( ( "carved", carved ), ( "whole", whole ) ):
        assert c["Num_Nodes"] > 100, "%s: expected a mesh, got %d nodes" % ( tag, c["Num_Nodes"] )
        assert c["Num_Els"] > 100, "%s: expected elements, got %d" % ( tag, c["Num_Els"] )
