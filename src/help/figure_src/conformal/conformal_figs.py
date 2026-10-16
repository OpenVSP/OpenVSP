"""Figures for Conformal.md: the offset, trims, a wing tank and a hinged flap.

    <build>/venv/bin/python conformal_figs.py [offset ends trims wing flap]

The drawing helpers are shared with the other body pages, in ../fuselagestack/bodyfig.py.
"""

import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "fuselagestack" ) )

import numpy as np

import bodyfig as bf
from bodyfig import vsp, X, Y, Z, FS


def fuse_conformal( **parms ):
    bf.renew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    c = vsp.AddGeom( "CONFORMAL", f )
    vsp.Update()
    for k, v in parms.items():
        vsp.SetParmVal( c, k, "Design", v )
    vsp.Update()
    return f, c


def ring_at( gid, x, w, s = 0 ):
    """The cross section of main surface s at station x."""
    us = np.linspace( 0.0, 1.0, 2001 )
    xs = np.array( [ vsp.CompPnt01( gid, s, u, 0.0 ).x() for u in us ] )
    u = np.interp( x, xs, us )
    return np.array( [ list( vsp.CompPnt01( gid, s, u, a ) ) for a in w ] )


def offset():
    """Conformal_offset: a Conformal of Offset 0.25 in the default Fuselage, across it at mid-length."""
    f, c = fuse_conformal( Offset = 0.25 )
    w = np.linspace( 0.0, 1.0, 241 )
    rf = ring_at( f, 15.0, w )
    rc = ring_at( c, 15.0, w )
    gap = np.max( rf[ :, 2 ] ) - np.max( rc[ :, 2 ] )
    assert abs( gap - 0.25 ) < 0.02, gap
    fig = bf.figure()
    cf = bf.canvas( fig, bf.FRONT )
    bf.fill( cf, rf )
    bf.fill( cf, rc, face = bf.CONF, edge = "#2E6C99", z = 12 )
    top_f = rf[ np.argmax( rf[ :, 2 ] ) ]
    top_c = rc[ np.argmax( rc[ :, 2 ] ) ]
    bf.dim( cf, top_c, top_f, "", off = ( 0.9, 0 ) )
    bf.label( cf, 0.5 * ( top_c + top_f ) + 0.9 * cf.v.right, "Offset", dx = 8, ha = "left" )
    bf.label( cf, np.array( [ 15.0, 0, 0 ] ), "Conformal", color = "#1F4E70" )
    bf.label( cf, np.array( [ 15.0, 1.12, -1.25 ] ), "Fuselage", ha = "right" )
    bf.key( cf, "front", ( 1.0, 0.0 ), 40, 0 )
    bf.save( fig, "Conformal_offset", [ [ cf ] ], hmax = 3.4 )


def ends():
    """Conformal_ends: the nose of the same Conformal, with Offset Ends and without."""
    f, c = fuse_conformal( Offset = 0.25 )
    with_ends = bf.body_cut( c )
    vsp.SetParmVal( c, "OffSetEnds", "Design", 0 )
    vsp.Update()
    without = bf.body_cut( c )
    assert with_ends[ :, 0 ].min() > 0.2 and abs( without[ :, 0 ].min() ) < 1e-9
    def nose( P, xmax = 6.0 ):
        """The part of a cut outline ahead of xmax, closed by the cut."""
        return P[ P[ :, 0 ] <= xmax ]
    fig = bf.figure()
    cs = bf.canvas( fig, bf.SIDE )
    bf.fill( cs, nose( bf.body_cut( f ) ) )
    bf.fill( cs, nose( with_ends ), face = bf.CONF, edge = "#2E6C99", z = 12 )
    P = nose( without )
    cs.line( P[ : len( P ) // 2 ], color = "#1F4E70", lw = 0.9, ls = ( 0, ( 4, 2 ) ), z = 14 )
    cs.line( P[ len( P ) // 2: ], color = "#1F4E70", lw = 0.9, ls = ( 0, ( 4, 2 ) ), z = 14 )
    bf.axis_line( cs, -0.6, 6.0 )
    bf.key( cs, "side", ( 1.0, 0.0 ), 14, 0 )
    bf.save( fig, "Conformal_ends", [ [ cs ] ], hmax = 3.4 )


def trims():
    """Conformal_trims: L 0.3 to 0.7, Side 4 0.3, in the default Fuselage."""
    f, c = fuse_conformal( Offset = 0.15, UTrimFlag = 1, UMinTrimTypeFalg = vsp.L_TRIM, UMaxTrimTypeFalg = vsp.L_TRIM,
                           Side4TrimFlag = 1, Side4Trim = 0.3 )
    vsp.SetParmVal( c, "LTrimMin", "Design", 0.3 )
    vsp.SetParmVal( c, "LTrimMax", "Design", 0.7 )
    vsp.Update()
    Gf = bf.grid( f, 0, nu = 81, nw = 49 )
    Gc = bf.grid( c, 0, nu = 81, nw = 97 )
    assert Gc[ ..., 0 ].min() > 8.5 and Gc[ ..., 0 ].max() < 21.5
    fig = bf.figure()
    cv = bf.canvas( fig, bf.ISO )
    cv.surface( Gc, color = bf.CONF, edge = False )
    cv.flush()
    # The edges of the flat face where the top band was cut off: W 0.6 and 0.9.
    for wc in ( 0.6, 0.9 ):
        E = bf.grid( c, 0, nu = 401, w = np.array( [ wc ] ) )[ :, 0 ]
        x0, x1 = Gc[ ..., 0 ].min(), Gc[ ..., 0 ].max()
        E = E[ ( E[ :, 0 ] > x0 + 1e-6 ) & ( E[ :, 0 ] < x1 - 1e-6 ) ]      # leave out the end caps
        cv.line( E, color = "#1F4E70", lw = 1.0, z = 35 )
    # The Fuselage as a wire outline over it.
    for j in range( 0, 49, 6 ):
        cv.line( Gf[ :, j ], color = bf.GHOST, lw = 0.5, z = 40 )
    for i in range( 0, 81, 10 ):
        cv.line( Gf[ i ], color = bf.GHOST, lw = 0.5, z = 40 )
    bf.triad( cv, np.array( [ -2.0, 4.0, -3.0 ] ), length = 2.5 )
    bf.save( fig, "Conformal_trims", [ [ cv ] ], hmax = 3.6 )


def wing():
    """Conformal_wing: a tank in the default Wing, from above."""
    bf.renew()
    wg = vsp.AddGeom( "WING" )
    vsp.Update()
    c = vsp.AddGeom( "CONFORMAL", wg )
    vsp.Update()
    for k, v in ( ( "Offset", 0.1 ), ( "UTrimFlag", 1 ), ( "UMinTrimTypeFalg", vsp.ETA_TRIM ), ( "UMaxTrimTypeFalg", vsp.ETA_TRIM ),
                  ( "EtaTrimMin", 0.2 ), ( "EtaTrimMax", 0.7 ), ( "ChordTrimFlag", 1 ), ( "ChordTrimMin", 0.2 ),
                  ( "ChordTrimMax", 0.6 ) ):
        vsp.SetParmVal( c, k, "Design", v )
    vsp.Update()
    Gw = bf.grid( wg, 0, nu = 81, nw = 121 )
    Gc = bf.grid( c, 0, nu = 81, nw = 121 )
    fig = bf.figure()
    ct = bf.canvas( fig, bf.TOP )
    # The wing's planform: its leading and trailing edges and its ends.
    le = Gw[ :, 60 ]
    te = Gw[ :, 0 ]
    bf.fill( ct, np.vstack( [ te, le[ ::-1 ] ] ) )
    # The tank seen from above: the outline of all its points.
    P = Gc.reshape( -1, 3 )
    lo_x = [ Gc[ i, :, 0 ].min() for i in range( Gc.shape[0] ) ]
    hi_x = [ Gc[ i, :, 0 ].max() for i in range( Gc.shape[0] ) ]
    ys = [ Gc[ i, :, 1 ].mean() for i in range( Gc.shape[0] ) ]
    out = np.array( [ [ a, b, 0 ] for a, b in zip( lo_x, ys ) ] + [ [ a, b, 0 ] for a, b in zip( hi_x[ ::-1 ], ys[ ::-1 ] ) ] )
    bf.fill( ct, out, face = bf.CONF, edge = "#2E6C99", z = 12 )
    bf.label( ct, np.array( [ 3.8, 4.0, 0 ] ), "tank", color = "#2E6C99" )
    bf.key( ct, "top", ( 1.0, 0.0 ), 20, 0 )
    bf.save( fig, "Conformal_wing", [ [ ct ] ], hmax = 4.0 )


FLAP_DEFL = 30.0


def flap():
    """Conformal_flap: a Wing split into a main element and a flap by two Conformals, the flap swung by a Hinge."""
    bf.renew()
    wg = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wg, "Sym_Planar_Flag", "Sym", 0 )
    vsp.Update()

    def piece( cmin, cmax ):
        c = vsp.AddGeom( "CONFORMAL", wg )
        vsp.Update()
        for k, v in ( ( "Offset", 0.0 ), ( "ChordTrimFlag", 1 ), ( "ChordTrimMin", cmin ), ( "ChordTrimMax", cmax ) ):
            vsp.SetParmVal( c, k, "Design", v )
        vsp.Update()
        return c
    main = piece( 0.25, 1.0 )
    fl_ = piece( 0.0, 0.25 )
    Gf0 = bf.grid( fl_, 0, nu = 61, nw = 81 )
    # The hinge line: the flap's front edge at its root and at its tip.
    r = Gf0[ 1 ][ np.argmin( Gf0[ 1 ][ :, 0 ] ) ]
    t = Gf0[ -2 ][ np.argmin( Gf0[ -2 ][ :, 0 ] ) ]
    Gm = bf.grid( main, 0, nu = 61, nw = 81 )
    assert abs( Gm[ ..., 0 ].max() - t[0] ) < 1e-6         # the two pieces meet at the cut
    h = vsp.AddGeom( "HINGE", wg )
    vsp.Update()
    d = t - r
    for k, v in ( ( "X_Rel_Location", r[0] ), ( "Y_Rel_Location", r[1] ), ( "Z_Rel_Location", r[2] ),
                  ( "Z_Rel_Rotation", np.degrees( np.arctan2( d[1], d[0] ) ) ) ):
        vsp.SetParmVal( h, k, "XForm", v )
    vsp.Update()
    vsp.SetGeomParent( fl_, h )
    vsp.SetParmVal( fl_, "DetachFlag", "Design", 1 )
    vsp.Update()
    # Detached under the Hinge with Position on Hinge on, the flap stays where it was while the Hinge is at rest.
    assert np.abs( bf.grid( fl_, 0, nu = 61, nw = 81 ) - Gf0 ).max() < 1e-9
    vsp.SetParmVal( h, "JointRotate", "Hinge", FLAP_DEFL )
    vsp.Update()
    Gf = bf.grid( fl_, 0, nu = 61, nw = 81 )
    assert Gf[ 30, 0, 2 ] < Gf0[ 30, 0, 2 ] - 0.1          # a positive Rotate turns the trailing edge down
    fig = bf.figure()
    cv = bf.canvas( fig, bf.ISO )
    cv.surface( Gm, color = bf.CONF )
    cv.surface( Gf, color = "#F0B48A" )
    # The flap's place at rest.
    for i in ( 1, 60 ):
        cv.segs( Gf0[ i ], bf.GHOST, 0.7, ls = ( 0, ( 3, 2 ) ), bias = 0.05 )
    cv.segs( Gf0[ :, 0 ], bf.GHOST, 0.7, ls = ( 0, ( 3, 2 ) ), bias = 0.05 )
    cv.flush()
    L = np.linalg.norm( d )
    cv.hinge( 0.5 * ( r + t ), d, "k", half = 0.62 * L, rho = 0.5, at = 0.9, lw = 1.1, z = 80 )
    bf.label( cv, t + 0.15 * d, "Hinge", dx = 0, dy = 10 )
    bf.label( cv, Gm[ 30, 40 ], "main element", color = "#1F4E70", dy = 4 )
    bf.label( cv, Gf[ 34, 0 ], "flap", color = "#A0410A", dx = 10, dy = -10, ha = "left" )
    bf.triad( cv, np.array( [ -1.5, 3.0, -1.0 ] ), length = 1.5 )
    bf.save( fig, "Conformal_flap", [ [ cv ] ], hmax = 4.0 )


FIGS = { "offset": offset, "ends": ends, "trims": trims, "wing": wing, "flap": flap }

if __name__ == "__main__":
    for name in ( sys.argv[ 1: ] or FIGS ):
        FIGS[ name ]()
