"""Scale Wake stretches the meshed wake past the model: at 1 it ends where it is built, and
at a larger scale the length past the model's last point is multiplied by it."""

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
    p += 3 * nt
    tag = np.array( toks[ p:p + nt ], int )
    return V, T, tag


def wake_and_model_xmax( scale, tmp_path ):
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "Wake", "Shape", 1.0 )
    vsp.SetParmVal( w, "WakeScale", "WakeSettings", scale )
    vsp.Update()
    vsp.SetCFDMeshVal( vsp.CFD_MAX_EDGE_LEN, 1.0 )
    vsp.SetCFDMeshVal( vsp.CFD_MIN_EDGE_LEN, 0.05 )
    fn = str( tmp_path / "w.tri" )
    vsp.SetComputationFileName( vsp.CFD_TRI_TYPE, fn )
    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.SET_NONE, vsp.CFD_TRI_TYPE )
    V, T, tag = read_tri( fn )
    wake = V[ np.unique( T[ tag > 1000 ] ) ]
    model = V[ np.unique( T[ tag < 1000 ] ) ]
    return wake[ :, 0 ].max(), model[ :, 0 ].max()


def test_scale_wake_stretches_the_meshed_wake( tmp_path, monkeypatch ):
    monkeypatch.chdir( tmp_path )
    end1, xmax = wake_and_model_xmax( 1.0, tmp_path )
    end3, xmax3 = wake_and_model_xmax( 3.0, tmp_path )

    assert end1 > xmax
    assert abs( xmax3 - xmax ) < 1e-9
    # The length past the start of the stretch is tripled, and the stretch starts at or near the model's end.
    assert end3 > xmax + 2.9 * ( end1 - xmax )
