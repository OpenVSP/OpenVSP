"""Checks, against the openvsp module, the statements Clone.md makes about a Clone's copied symmetry.

    <build>/venv/bin/python verify.py
"""

import numpy as np
import openvsp as vsp

import clonemodel as cm


def close( a, b, what ):
    assert np.allclose( a, b, atol = 1e-9 ), ( what, a, b )


def symmetry():
    p = cm.nacelle()
    close( [ q[ 1 ] for q in cm.centers( p ) ], [ cm.Y0, -cm.Y0 ], "original pair" )
    # A Clone starts where its original is, with its symmetry.
    c = vsp.CloneGeomVec( [ p ], "_Clone" )[ 0 ]
    vsp.Update()
    close( cm.centers( c ), cm.centers( p ), "Clone starts on its original" )
    # The Clone is mirrored in Y = 0 where it is placed: at Y = y its halves are at +y and -y.
    for y in ( cm.Y0, 3.0, -2.0, 0.0 ):
        vsp.SetParmVal( c, "X_Rel_Location", "XForm", cm.DX )
        vsp.SetParmVal( c, "Y_Rel_Location", "XForm", y )
        vsp.Update()
        Q = cm.centers( c )
        assert vsp.GetTotalNumSurfs( c ) == 2
        close( [ Q[ 0 ][ 1 ], Q[ 1 ][ 1 ] ], [ y, -y ], "Clone pair at y = %g" % y )
        close( Q[ 0 ][ 0 ] - cm.centers( p )[ 0 ][ 0 ], cm.DX, "moved aft" )
    # On Y = 0 the two halves coincide.
    S = cm.surfs( c, 9, 9 )
    A, B = ( np.array( sorted( map( tuple, np.round( G.reshape( -1, 3 ), 9 ) ) ) ) for G in S )
    close( A, B, "halves on top of each other" )
    # While Symmetry and Flip is checked the Clone's own symmetry setting does not hold.
    vsp.SetParmVal( c, "Sym_Planar_Flag", "Sym", 0.0 )
    vsp.Update()
    assert vsp.GetTotalNumSurfs( c ) == 2, "symmetry copied while checked"
    # Unchecked, the last copied symmetry stays until it is turned off.
    vsp.SetParmVal( c, "CloneSym", "Behavior", 0.0 )
    vsp.Update()
    assert vsp.GetParmVal( c, "Sym_Planar_Flag", "Sym" ) == vsp.SYM_XZ and vsp.GetTotalNumSurfs( c ) == 2
    cm.single( c )
    assert vsp.GetTotalNumSurfs( c ) == 1, "one Clone on the plane"
    close( cm.centers( c )[ 0 ][ 1 ], 0.0, "single Clone on Y = 0" )
    print( "symmetry: Clone mirrored in Y = 0 where placed, halves coincide on Y = 0, remedy ok" )


if __name__ == "__main__":
    symmetry()
