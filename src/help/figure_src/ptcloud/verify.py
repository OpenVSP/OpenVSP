"""Checks, against the openvsp module, the statements PtCloud.md makes about projecting points onto a surface.

    <build>/venv/bin/python verify.py
"""

import numpy as np
import openvsp as vsp

import cloudmodel as cm


def on_surface( w, p, s = 0 ):
    d, u, t = vsp.ProjPnt01( w, s, vsp.vec3d( *p ) )
    return d


def projection():
    w = cm.wing()
    P = cm.cloud_points( w )
    c, Q = cm.projected( w, P )
    n = len( cm.STATIONS )
    # Each point moves along Z only.
    assert np.allclose( Q[ :, :2 ], P[ :, :2 ] ), "X and Y kept"
    # A point above the wing lands on the nearest crossing of its line: the upper surface.
    for q in Q[ :n ]:
        assert on_surface( w, q ) < 1e-6 and q[2] > 0.0, ( "on the upper surface", q )
    # A point whose line misses the surface stays where it was.
    assert np.allclose( Q[ n ], P[ n ] ), "miss stays"
    # From below, the nearest crossing is the lower surface; and a point inside the wing goes to the nearer side.
    for z, sign in ( ( -2.0, -1.0 ), ( 0.05, 1.0 ) ):
        p = P[ 0 ].copy()
        p[2] = z
        c2, q = cm.projected( w, p[ None, : ] )
        assert on_surface( w, q[ 0 ] ) < 1e-6 and np.sign( q[ 0 ][2] ) == sign, ( "nearest crossing", z, q )
    # Surf_0 is one surface: with symmetry on, a point over the mirror half misses it.
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
    vsp.Update()
    p = P[ 0 ].copy()
    p[1] = -p[1]
    c3, q = cm.projected( w, p[ None, : ] )
    assert np.allclose( q[ 0 ], p ), "mirror half is another surface"
    print( "projection: along Z only, nearest crossing, misses stay, one surface at a time ok" )


if __name__ == "__main__":
    projection()
