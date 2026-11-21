"""A body in the Degen Set is meshed as its two plates, vertical and horizontal, and each
symmetric copy as its own two plates."""

import numpy as np
import openvsp as vsp
import pytest


def read_tri( fn ):
    with open( fn ) as f:
        toks = f.read().split()
    nv, nt = int( toks[ 0 ] ), int( toks[ 1 ] )
    p = 2
    V = np.array( toks[ p:p + 3 * nv ], float ).reshape( nv, 3 )
    p += 3 * nv
    T = np.array( toks[ p:p + 3 * nt ], int ).reshape( nt, 3 ) - 1
    p += 3 * nt
    tag = np.array( toks[ p:p + nt ], int )
    return V, T, tag


@pytest.mark.parametrize( "typ", [ "FUSELAGE", "STACK", "ELLIPSOID", "POD" ] )
def test_each_copy_of_a_body_gives_its_two_plates( typ, tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    vsp.VSPRenew()
    g = vsp.AddGeom( typ )
    vsp.SetParmVal( g, "Y_Rel_Location", "XForm", 2.0 )
    vsp.SetParmVal( g, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
    vsp.Update()
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 0.5 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.025 )

    fn = str( tmp_path / "d.tri" )
    vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, fn )
    vsp.ComputeCFDMesh( vsp.SET_NONE, vsp.SET_ALL, vsp.CFD_TRI_TYPE )
    V, T, tag = read_tri( fn )

    planes = []
    for t in np.unique( tag ):
        P = V[ np.unique( T[ tag == t ] ) ]
        side = int( np.sign( P[ :, 1 ].mean() ) )
        if np.ptp( P[ :, 2 ] ) < 1e-6:
            planes.append( ( side, "XY" ) )
        elif np.ptp( P[ :, 1 ] ) < 1e-6:
            planes.append( ( side, "XZ" ) )
        else:
            planes.append( ( side, "other" ) )

    assert sorted( planes ) == [ ( -1, "XY" ), ( -1, "XZ" ), ( 1, "XY" ), ( 1, "XZ" ) ]
