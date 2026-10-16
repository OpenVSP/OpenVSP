"""Checks, against the openvsp module, what Wire.md states about patches, the order of the controls and import.

    <build>/venv/bin/python verify.py
"""

import numpy as np

import wire_figs as wf
from wire_figs import vsp

FAIL = []


def check( ok, what ):
    print( ( "ok    " if ok else "FAIL  " ) + what )
    if not ok:
        FAIL.append( what )


def grid( G0, **parms ):
    w = wf.imported_wire( G0 )
    for k, v in parms.items():
        grp = "Wireframe" if k in ( "FlipIJFlag", "RevIFlag", "RevJFlag", "InvertFlag" ) else "WireFrame"
        vsp.SetParmVal( w, k, grp, v )
    vsp.Update()
    return w, wf.wire_grid( w )


def main():
    G0 = wf.wing_grid( nsec = 3 )
    ni, nj, _ = G0.shape

    w, G = grid( G0 )
    check( np.allclose( G, G0, atol = 1e-9 ), "an XSec export writes the imported grid back unchanged" )

    _, G = grid( G0, IEndPatchType = vsp.PATCH_POINT )
    check( G.shape[ 0 ] == ni + 1 and np.allclose( G[ -1 ], G0[ -1 ].mean( 0 ), atol = 1e-9 ),
           "I End Point adds a row at the centroid of the end row" )
    _, G = grid( G0, IEndPatchType = vsp.PATCH_LINE )
    folded = 0.5 * ( G0[ -1 ] + G0[ -1 ][ ::-1 ] )
    check( np.allclose( G[ -1 ], folded, atol = 1e-9 ), "I End Line folds the end row on itself" )
    _, G = grid( G0, IEndPatchType = vsp.PATCH_COPY )
    check( np.allclose( G[ -1 ], G0[ 0 ], atol = 1e-9 ), "I End Copy Opposite adds a copy of the start row" )
    _, G = grid( G0, IEndPatchType = vsp.PATCH_HALFWAY )
    check( np.allclose( G[ -1 ], 0.5 * ( G0[ -1 ] + G0[ 0 ] ), atol = 1e-9 ), "I End Halfway Opposite: midpoints" )
    _, G = grid( G0, JStartPatchType = vsp.PATCH_HALFWAY, JEndPatchType = vsp.PATCH_COPY )
    te = 0.5 * ( G0[ :, 0 ] + G0[ :, -1 ] )
    check( G.shape[ 1 ] == nj + 2 and np.allclose( G[ :, 0 ], te, atol = 1e-9 ) and
           np.allclose( G[ :, -1 ], te, atol = 1e-9 ),
           "J Start Halfway with J End Copy closes a blunt trailing edge on its midline" )
    _, G = grid( G0, JStartPatchType = vsp.PATCH_HALFWAY, JEndPatchType = vsp.PATCH_HALFWAY )
    check( not np.allclose( G[ :, -1 ], G[ :, 0 ], atol = 1e-6 ), "Halfway at both J edges leaves the gap open" )

    _, G = grid( G0, ISkipStart = 1, IStride = 1 )
    check( G.shape[ 0 ] == ni - 1 and np.allclose( G[ 0 ], G0[ 1 ], atol = 1e-9 ), "I Start Skip drops the first row" )
    _, G = grid( G0, JStride = 4 )
    _, G3 = grid( G0, JStride = 3 )
    check( np.allclose( G[ 0, -1 ], G0[ 0, -1 ], atol = 1e-9 ) and G.shape[ 1 ] == len( range( 0, nj, 4 ) ) and
           np.allclose( G3[ 0, -1 ], G0[ 0, -1 ], atol = 1e-9 ) and G3.shape[ 1 ] == len( range( 0, nj, 3 ) ) + 1,
           "J Stride keeps every nth point and always the last" )
    _, G = grid( G0, FlipIJFlag = 1 )
    check( G.shape == ( nj, ni, 3 ) and np.allclose( G[ 2, 1 ], G0[ 1, 2 ], atol = 1e-9 ), "Swap I/J transposes the grid" )
    _, G = grid( G0, FlipIJFlag = 1, ISkipStart = 1 )
    check( G.shape[ 0 ] == nj - 1, "the skips act after the swap" )

    # Import turns the normals out: the same body with its rows reversed comes in inverted.
    w1, _ = grid( G0 )
    inv1 = vsp.GetParmVal( w1, "InvertFlag", "Wireframe" )
    w2, _ = grid( G0[ ::-1 ] )
    inv2 = vsp.GetParmVal( w2, "InvertFlag", "Wireframe" )
    check( inv1 != inv2, "import sets Invert from the orientation of the grid" )

    # A closed wireframe has a positive volume with its normals out, and a negative one with them in.
    w, _ = grid( G0, IStartPatchType = vsp.PATCH_LINE, IEndPatchType = vsp.PATCH_LINE,
                 JStartPatchType = vsp.PATCH_HALFWAY, JEndPatchType = vsp.PATCH_COPY )
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    v_out = vsp.GetDoubleResults( vsp.FindLatestResultsID( "Comp_Geom" ), "Theo_Vol" )[ 0 ]
    vsp.SetParmVal( w, "InvertFlag", "Wireframe", 1.0 - vsp.GetParmVal( w, "InvertFlag", "Wireframe" ) )
    vsp.Update()
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    v_in = vsp.GetDoubleResults( vsp.FindLatestResultsID( "Comp_Geom" ), "Theo_Vol" )[ 0 ]
    check( v_out > 0.01 and abs( v_in + v_out ) < 1e-9, "closed: volume %.4f with normals out, %.4f in" % ( v_out, v_in ) )

    print( "\n%d failed" % len( FAIL ) )


if __name__ == "__main__":
    main()
