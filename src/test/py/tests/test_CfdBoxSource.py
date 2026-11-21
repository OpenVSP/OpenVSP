"""A box source covers the whole patch of the surface between its two corners, not only the
line from one corner to the other."""

import numpy as np
import openvsp as vsp


def read_tri( fn ):
    with open( fn ) as f:
        toks = f.read().split()
    nv, nt = int( toks[ 0 ] ), int( toks[ 1 ] )
    p = 2
    V = np.array( toks[ p:p + 3 * nv ], float ).reshape( nv, 3 )
    p += 3 * nv
    T = np.array( toks[ p:p + 3 * nt ], int ).reshape( nt, 3 ) - 1
    return V, T


def test_a_box_source_reaches_its_off_diagonal_corners( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    pid = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.DeleteAllCFDSources()
    # From U 0.05, W 0 near the nose on top to U 0.5, W 0.25 at the widest point on the side.
    vsp.AddCFDSource( vsp.BOX_SOURCE, pid, 0, 0.05, 0.01, 0.05, 0.0, 0.0, 0.0, 0.5, 0.25 )
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.5 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.01 )
    vsp.SetCFDMeshVal( vsp.CFD_MAX_GAP, 1.0 )
    vsp.SetCFDMeshVal( vsp.CFD_NUM_CIRCLE_SEGS, 4.0 )

    fn = str( tmp_path / "b.tri" )
    vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, fn )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )
    V, T = read_tri( fn )

    E = np.unique( np.sort( np.vstack( [ T[ :, [ 0, 1 ] ], T[ :, [ 1, 2 ] ], T[ :, [ 2, 0 ] ] ] ), axis = 1 ), axis = 0 )
    L = np.linalg.norm( V[ E[ :, 0 ] ] - V[ E[ :, 1 ] ], axis = 1 )
    M = 0.5 * ( V[ E[ :, 0 ] ] + V[ E[ :, 1 ] ] )

    # The corner at U 0.5, W 0: the top of the widest section, off the diagonal.
    c = vsp.CompPnt01( pid, 0, 0.5, 0.0 )
    near = np.linalg.norm( M - np.array( [ c.x(), c.y(), c.z() ] ), axis = 1 ) < 0.1
    assert near.sum() > 10
    assert np.median( L[ near ] ) < 0.07
