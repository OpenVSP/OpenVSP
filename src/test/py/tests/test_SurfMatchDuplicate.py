"""Coincident surfaces must be found and discarded.

A default wing carries two root cap patches that lie exactly on top of each other,
one wound the opposite way from the other.  Both have to go: what remains mates up
along the root and closes.  Finding them is SurfMatch, which compares two surfaces
every way round they might be turned.

Asking that of every pair of surfaces is expensive, so pairs whose bounding boxes
differ are turned away first.  Two surfaces that coincide have the same box, so the
fast path must let this pair through -- and this is the case that proves it does.

With the pair discarded the wing meshes to 6 faces over 10 edges and closes.
"""

import glob
import os

import openvsp as vsp


def read_topo_header( path ):
    with open( path ) as f:
        first = f.readline().split()

    return int( first[1] ), int( first[2] )      # nedge, nface


def count_open_edges( path ):
    n = 0

    with open( path ) as f:
        f.readline()

        for line in f:
            col = line.split()

            if len( col ) >= 5 and int( col[3] ) == 0:
                n += 1

    return n


def test_SurfMatchDuplicate( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )

    vsp.VSPRenew()
    vsp.AddGeom( "WING", "" )
    vsp.Update()

    # Coarse on purpose: this is about which surfaces survive, not about the mesh.
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 1.0 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.2 )

    # The faces and edges counted below are those of a wing cut into its feature patches
    vsp.SetCFDMeshVal( vsp.CFD_SPLIT_JOIN_SURFS_FLAG, 1 )

    # SetComputationFileName keys on COMPUTATION_FILE_TYPE, so the name only sticks when it is
    # given the *_TYPE value; a *_FILE_NAME value matches nothing and is dropped in silence.
    err_mgr = vsp.ErrorMgrSingleton.getInstance()
    nerr = err_mgr.GetNumTotalErrors()

    vsp.SetComputationFileName( vsp.CFD_POGS_TYPE, str( tmp_path / "dup.i.tri" ) )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_POGS_TYPE )

    # The error stack is never emptied, not even by VSPRenew, so what this run added is the
    # only thing that can be asserted about -- a count of zero would be asserting about every
    # test that ran before it in the same process.
    assert err_mgr.GetNumTotalErrors() == nerr, "meshing a default wing raised an error"

    topo = glob.glob( os.path.join( str( tmp_path ), "*.topo" ) )
    assert len( topo ) == 1, "expected one POGS topology file, got %s" % topo

    nedge, nface = read_topo_header( topo[0] )

    assert nface == 6, ( "expected 6 faces; %d says the coincident root caps were "
                         "not discarded" % nface )
    assert nedge == 10, "expected 10 edges, got %d" % nedge
    assert count_open_edges( topo[0] ) == 0, "the wing should close"
