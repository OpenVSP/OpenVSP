"""Figures for MdcInlet.md."""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "mdc_bod" ) )

import numpy as np

import datcomfig as df
import bodyfig as bf
from bodyfig import vsp, X, Y, Z, FS

TYPES = [ ( "2DTOP", 1 ), ( "2DSIDE", 2 ), ( "AXI", 3 ) ]


def inlet( **kw ):
    """A new MDC_INLET on the default body's radius, with Design parms kw."""
    return df.add( "MDC_INLET", **kw )


def strip( body, x0, x1, z0 = 0.15 ):
    """The upper part of the body's side view, from x0 to x1 and down to z0, as an outline."""
    top = bf.meridian( body, 0, 0.75, 801 )
    top = top[ ( top[ :, 0 ] >= x0 ) & ( top[ :, 0 ] <= x1 ) ]
    return np.vstack( [ top, [ x1, 0.0, z0 ], [ x0, 0.0, z0 ] ] )


def types():
    """MdcInlet_types: the three types of inlet."""
    fig = bf.figure()
    cs = []
    for name, t, phi, w in ( ( "2DTOP", 1, 0.0, ( 0.58, 0.92 ) ), ( "2DSIDE", 2, 270.0, ( 0.33, 0.67 ) ), ( "AXI", 3, 0.0, ( 0.58, 0.92 ) ) ):
        bf.renew()
        body = df.cylinder( 0.5, 0.0, 9.0 )
        g = inlet( INTYPE = t, PHIFlag = 1, PHI_1 = phi, W1 = 0.0 if t == 3 else 0.5 )
        c = bf.canvas( fig, bf.ISO )
        df.draw( c, [ df.patch( body, 3.2, 8.0, w[ 0 ], w[ 1 ] ) ], df.BODY )
        c.flush( 1.0 )
        df.draw( c, df.surfaces( g ), df.PART )
        c.flush( 5.0 )
        bf.title( c, name, pad = 2 )
        cs.append( c )
    bf.save( fig, "MdcInlet_types", [ [ cs[ 0 ] ], [ cs[ 1 ] ], [ cs[ 2 ] ] ] )


STN = dict( X = 4.0, X2 = 0.6, X3 = 1.0, X4 = 2.6, X5 = 3.2, H2 = 0.35, H3 = 0.4, H4 = 0.4, H5 = 0.25, W1 = 0.4, W2 = 0.5, W3 = 0.5,
            W4 = 0.5, W5 = 0.3 )


def stations():
    """MdcInlet_stations: the five stations of a 2DTOP inlet, from the side and from above."""
    bf.renew()
    body = df.cylinder( 0.5, 0.0, 9.0 )
    kw = dict( STN )
    g = inlet( INTYPE = 1, HDIV = 0.1, XDIV = 0.5, LDIV = 2.0, **kw )
    xs = [ 4.0 + v for v in ( 0.0, kw[ "X2" ], kw[ "X3" ], kw[ "X4" ], kw[ "X5" ] ) ]
    zf = 0.6
    hs = [ 0.0, kw[ "H2" ], kw[ "H3" ], kw[ "H4" ], kw[ "H5" ] ]
    ws = [ kw[ "W%d" % i ] for i in range( 1, 6 ) ]
    G = df.surfaces( g )[ 0 ].reshape( -1, 3 )
    assert abs( G[ :, 0 ].max() - xs[ 4 ] ) < 1e-6 and abs( G[ :, 2 ].max() - ( zf + max( hs ) ) ) < 1e-6
    fig = bf.figure()
    cs = bf.canvas( fig, bf.SIDE )
    bf.fill( cs, strip( body, 3.3, 7.5, 0.1 ) )
    for G in df.surfaces( g ):
        cs.surface( G, color = df.PART, edge = True )
    cs.flush( 5.0 )
    ct = bf.canvas( fig, bf.TOP )
    for G in df.surfaces( g )[ 0:1 ]:
        ct.surface( G, color = df.PART, edge = True )
    ct.flush( 5.0 )
    for c, zt in ( ( cs, 1.35 ), ( ct, 0.5 ) ):
        for i, x in enumerate( xs ):
            lo = -0.05 if c is cs else -0.4
            c.line( np.array( [ [ x, lo, 0.0 ] if c is ct else [ x, 0, 0.0 ], [ x, 0.55 if c is ct else 0, zt ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ) )
            bf.label( c, np.array( [ x, 0.55 if c is ct else 0, zt ] ), str( i + 1 ), dy = 7 )
    for i in range( 1, 5 ):
        bf.dim( cs, np.array( [ xs[ 0 ], 0, 0.25 ] ), np.array( [ xs[ i ], 0, 0.25 ] ), "", ext = False ) if False else None
    for i in range( 1, 5 ):
        x = xs[ i ] - 0.04
        bf.dim( cs, np.array( [ x, 0, zf ] ), np.array( [ x, 0, zf + hs[ i ] ] ), "H%d" % ( i + 1 ), dx = -14 if i < 4 else 14, ext = False )
    bf.key( cs, "side", ( 1.0, 0.0 ), 30, 0 )
    for i in range( 5 ):
        x = xs[ i ] + ( 0.04 if i < 4 else -0.04 )
        bf.dim( ct, np.array( [ x, -0.5 * ws[ i ], 0 ] ), np.array( [ x, 0.5 * ws[ i ], 0 ] ), "W%d" % ( i + 1 ), dx = 14 if i < 4 else -14, ext = False )
    bf.dim( ct, np.array( [ xs[ 0 ], 0.0, 0 ] ), np.array( [ xs[ 0 ], 0.0, 0 ] ), "", ext = False ) if False else None
    bf.key( ct, "top", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "MdcInlet_stations", [ [ cs ], [ ct ] ], hmax = 6.0 )


def ramp():
    """MdcInlet_ramp: an open 2DTOP inlet with its ramp, and the same inlet covered."""
    fig = bf.figure()
    rows = []
    for cover in ( 0, 1 ):
        bf.renew()
        body = df.cylinder( 0.5, 0.0, 9.0 )
        kw = dict( STN )
        g = inlet( INTYPE = 1, HDIV = 0.1, XDIV = 0.5, LDIV = 2.0, COVER = cover, RAMP = 25.0, **kw )
        c = bf.canvas( fig, bf.SIDE )
        bf.fill( c, strip( body, 3.3, 7.5, 0.1 ) )
        for G in df.surfaces( g ):
            c.surface( G, color = df.PART, edge = True )
        c.flush( 5.0 )
        zf = 0.6
        if not cover:
            t = np.radians( np.linspace( 0.0, 25.0, 20 ) )
            r = 0.45
            c.line( np.array( [ [ 4.0 + r * np.cos( a_ ), 0, zf + r * np.sin( a_ ) ] for a_ in t ] ), color = "k", lw = 0.8, z = 60 )
            c.line( np.array( [ [ 3.8, 0, zf ], [ 4.6, 0, zf ] ] ), color = "#777777", lw = 0.5, ls = ( 0, ( 2, 2 ) ), z = 60 )
            bf.label( c, np.array( [ 4.0 + 0.5 * np.cos( np.radians( 11 ) ), 0, zf + 0.5 * np.sin( np.radians( 11 ) ) ] ), "RAMP", ha = "left", dx = 4, dy = -8 )
        bf.title( c, "COVER off" if not cover else "COVER on", pad = 4 )
        rows.append( [ c ] )
    bf.save( fig, "MdcInlet_ramp", rows, hmax = 6.0 )


def diverter():
    """MdcInlet_diverter: the diverter, from the side and from above."""
    bf.renew()
    body = df.cylinder( 0.5, 0.0, 9.0 )
    kw = dict( STN )
    g = inlet( INTYPE = 1, HDIV = 0.1, XDIV = 0.5, LDIV = 2.0, **kw )
    fig = bf.figure()
    cs = bf.canvas( fig, bf.SIDE )
    bf.fill( cs, strip( body, 3.3, 7.5, 0.1 ) )
    grids = df.surfaces( g )
    for G in grids:
        cs.surface( G, color = df.PART, edge = True )
    cs.flush( 5.0 )
    ct = bf.canvas( fig, bf.TOP )
    assert len( grids ) == 2
    ct.surface( grids[ 0 ], color = df.PART, edge = True )
    ct.flush( 5.0 )
    # The diverter lies under the inlet; its outline is drawn over it.
    D = grids[ 1 ]
    left = np.array( [ [ D[ i, :, 0 ].mean(), D[ i, :, 1 ].min(), 0.0 ] for i in range( D.shape[ 0 ] ) ] )
    right = np.array( [ [ D[ i, :, 0 ].mean(), D[ i, :, 1 ].max(), 0.0 ] for i in range( D.shape[ 0 ] ) ] )
    ct.line( np.vstack( [ left, right[ ::-1 ], left[ :1 ] ] ), color = "#7A3E00", lw = 1.2, ls = ( 0, ( 4, 2 ) ), z = 60 )
    bf.label( ct, np.array( [ 5.6, 0.0, 0.0 ] ), "diverter", color = "#7A3E00" )
    x0 = 4.0
    cs.line( np.array( [ [ 3.3, 0, 0.5 ], [ 7.5, 0, 0.5 ] ] ), color = "#777777", lw = 0.5, ls = ( 0, ( 2, 2 ) ), z = 60 )
    bf.dim( cs, np.array( [ 3.75, 0, 0.5 ] ), np.array( [ 3.75, 0, 0.6 ] ), "HDIV", dx = -24, ext = False )
    for c, y in ( ( cs, -0.1 ), ( ct, -0.55 ) ):
        c.line( np.array( [ [ x0, y, 0.0 ], [ x0, y + 0.25, 0.0 ] ] ), color = "#777777", lw = 0.6, z = 60 )
        c.line( np.array( [ [ x0 + 0.5, y, 0.0 ], [ x0 + 0.5, y + 0.25, 0.0 ] ] ), color = "#777777", lw = 0.6, z = 60 )
        c.line( np.array( [ [ x0 + 2.5, y, 0.0 ], [ x0 + 2.5, y + 0.25, 0.0 ] ] ), color = "#777777", lw = 0.6, z = 60 )
    bf.dim( ct, np.array( [ x0, -0.55, 0 ] ), np.array( [ x0 + 0.5, -0.55, 0 ] ), "XDIV", dy = -9, ext = False )
    bf.dim( ct, np.array( [ x0 + 0.5, -0.55, 0 ] ), np.array( [ x0 + 2.5, -0.55, 0 ] ), "LDIV", dy = -9, ext = False )
    bf.key( cs, "side", ( 1.0, 0.0 ), 30, 0 )
    bf.key( ct, "top", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "MdcInlet_diverter", [ [ cs ], [ ct ] ], hmax = 6.0 )


def hdiv():
    """MdcInlet_hdiv: the section of an inlet with a diverter, a conformal inlet and a semi-submerged inlet."""
    fig = bf.figure()
    cs = []
    for hd, nm in ( ( 0.1, "HDIV 0.1: diverter" ), ( 0.0, "HDIV 0: conformal" ), ( -0.15, "HDIV -0.15: semi-submerged" ) ):
        bf.renew()
        body = df.cylinder( 0.5, 0.0, 9.0 )
        g = inlet( INTYPE = 1, HDIV = hd, XDIV = 0.5, LDIV = 2.0, **STN )
        c = bf.canvas( fig, df.REAR )
        C = df.circle( body, 5.8, n = 721 )
        C = C[ ( np.abs( C[ :, 1 ] ) < 0.45 ) & ( C[ :, 2 ] > 0 ) ]
        C = C[ np.argsort( C[ :, 1 ] ) ]
        poly = np.vstack( [ C, [ 0.0, C[ -1, 1 ], 0.2 ], [ 0.0, C[ 0, 1 ], 0.2 ] ] )
        bf.fill( c, poly, face = df.BODY, edge = bf.POS_EDGE, z = 5 )
        for s_ in reversed( range( vsp.GetNumMainSurfs( g ) ) ):
            bf.fill( c, df.circle( g, 5.8, s = s_, n = 401 ), face = df.PART if s_ == 0 else "#B98A5C", edge = bf.POS_EDGE, z = 10 + s_ )
        if hd > 0:
            bf.label( c, np.array( [ 0.0, 0.0, 0.92 ] ), "inlet" )
            bf.label( c, np.array( [ 0.0, 0.0, 0.55 ] ), "diverter" )
        bf.title( c, nm, pad = 4 )
        cs.append( c )
    bf.save( fig, "MdcInlet_hdiv", [ cs ] )


def phi():
    """MdcInlet_phi: three inlets of a set, evenly spaced, from behind."""
    bf.renew()
    body = df.cylinder( 0.5, 0.0, 9.0 )
    g = inlet( INTYPE = 3, NIN = 3, W1 = 0.0, W2 = 0.2, W3 = 0.2, W4 = 0.2, W5 = 0.15, HDIV = 0.0 )
    fig = bf.figure()
    c = bf.canvas( fig, df.REAR )
    bf.fill( c, df.circle( body, 5.0 ), face = df.BODY, edge = bf.POS_EDGE )
    for G in df.surfaces( g ):
        c.surface( G, color = df.PART, edge = False )
    c.flush( 5.0 )
    for k in range( 3 ):
        a = 120.0 * k
        c.line( np.array( [ [ 0, 0, 0 ], [ 0, 0.9 * np.sin( np.radians( a ) ), 0.9 * np.cos( np.radians( a ) ) ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 20 )
    c.line( np.array( [ [ 0, 0, 0 ], [ 0, 0, 1.0 ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 20 )
    bf.label( c, np.array( [ 0, 0, 1.0 ] ), "top", dy = 7 )
    P = df.arc( c, 0.8, 0, 120 )
    bf.label( c, P[ -1 ], "PHI_2 = 120", ha = "left", dx = 6, dy = -2 )
    P = df.arc( c, 0.95, 0, 240 )
    bf.label( c, P[ -1 ], "PHI_3 = 240", ha = "right", dx = -4, dy = -8 )
    bf.label( c, np.array( [ 0, 0, 0.72 ] ), "PHI_1 = 0", dy = 8 )
    bf.key( c, "rear", ( 1.0, 0.0 ), 20, 0 )
    bf.save( fig, "MdcInlet_phi", [ [ c ] ], hmax = 4.8 )


FIGS = { "types": types, "stations": stations, "ramp": ramp, "diverter": diverter, "hdiv": hdiv, "phi": phi }

if __name__ == "__main__":
    for name in ( sys.argv[ 1: ] or FIGS ):
        FIGS[ name ]()
