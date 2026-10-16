"""Check what Conformal.md states about trims around the surface and a hinged flap against the openvsp module.
(The offset and wing-trim checks are in ../fuselagestack/verify.py.)

    <build>/venv/bin/python verify.py

Prints ok or FAIL for each check; exits non-zero if any fails.
"""

import sys

import numpy as np
import openvsp as vsp

FAILS = []


def check( ok, what ):
    print( "%-4s %s" % ( "ok" if ok else "FAIL", what ) )
    if not ok:
        FAILS.append( what )


def grid( g, n = 21 ):
    return np.array( [ [ list( vsp.CompPnt01( g, 0, u, w ) ) for w in np.linspace( 0, 1, n ) ] for u in np.linspace( 0, 1, n ) ] )


def side_trim():
    # Side 4 of width 0.3 cuts the same band as V1 from 0.6 to 0.9: Side Trim is V Trim centered on a side.
    vsp.VSPRenew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    a = vsp.AddGeom( "CONFORMAL", f )
    b = vsp.AddGeom( "CONFORMAL", f )
    vsp.Update()
    for k, v in ( ( "Side4TrimFlag", 1 ), ( "Side4Trim", 0.3 ) ):
        vsp.SetParmVal( a, k, "Design", v )
    for k, v in ( ( "V1TrimFlag", 1 ), ( "V1TrimBegin", 0.6 ), ( "V1TrimEnd", 0.9 ) ):
        vsp.SetParmVal( b, k, "Design", v )
    vsp.Update()
    check( np.abs( grid( a ) - grid( b ) ).max() < 1e-9, "Side 4 0.3 is V1 0.6 to 0.9" )
    # The flat face of a Side 4 (top) trim stays level: its edges are at one height.
    G = grid( a, 41 )
    mid = G[ 20 ]
    zt = mid[ :, 2 ].max()
    top = mid[ np.abs( mid[ :, 2 ] - zt ) < 1e-9 ]
    check( len( top ) >= 2 and abs( top[ :, 1 ].mean() ) < 1e-6, "a Side 4 cut is level and centered on the top" )


def flap():
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0 )
    vsp.Update()
    f = vsp.AddGeom( "CONFORMAL", w )
    vsp.Update()
    for k, v in ( ( "Offset", 0.0 ), ( "ChordTrimFlag", 1 ), ( "ChordTrimMin", 0.0 ), ( "ChordTrimMax", 0.25 ) ):
        vsp.SetParmVal( f, k, "Design", v )
    vsp.Update()
    G0 = grid( f )
    h = vsp.AddGeom( "HINGE", w )
    vsp.SetParmVal( h, "X_Rel_Location", "XForm", 3.0 )
    vsp.SetParmVal( h, "Z_Rel_Rotation", "XForm", 80.0 )
    vsp.Update()
    vsp.CutGeomToClipboard( f )
    f2 = vsp.PasteGeomClipboard( h )[0]
    vsp.Update()
    vsp.SetParmVal( f2, "DetachFlag", "Design", 1 )
    vsp.Update()
    check( vsp.GetGeomParent( f2 ) == h and np.abs( grid( f2 ) - G0 ).max() < 1e-9,
           "Cut, Paste under a Hinge, Detach: the flap keeps its shape and place (the Wing stays its conformal parent)" )
    vsp.SetParmVal( h, "JointRotate", "Hinge", 30.0 )
    vsp.Update()
    G = grid( f2 )
    check( G[ 10, 0, 2 ] < G0[ 10, 0, 2 ] - 0.1, "a positive Rotate about an axis toward the tip turns the flap trailing edge down" )


if __name__ == "__main__":
    side_trim()
    flap()
    print( "%d failed" % len( FAILS ) )
    sys.exit( 1 if FAILS else 0 )
