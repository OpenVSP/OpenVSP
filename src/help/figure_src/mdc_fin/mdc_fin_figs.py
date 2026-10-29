"""Figures for MdcFin.md."""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "mdc_bod" ) )

import numpy as np

import datcomfig as df
import bodyfig as bf
from bodyfig import vsp, X, Y, Z, FS

FIN = "#E3B98F"


def fin( **kw ):
    """A new MDC_FIN with the Design parms in kw, set in order."""
    return df.add( "MDC_FIN", **kw )


def one_panel( **kw ):
    """An MDC_FIN of one panel on the right of the body, PHIF 90, so that its planform is in the XY plane."""
    return fin( NPANEL = 1, PHIFFlag = 1, PHIF_1 = 90.0, **kw )


def plan( g, s = 0 ):
    """The planform of panel s, a panel at PHIF 90, as a closed outline in the XY plane: its leading edge from the root
    out and its trailing edge back, from the surface's points."""
    G = bf.grid( g, s, nu = 241, nw = 201 )
    le = []
    te = []
    for i in range( G.shape[ 0 ] ):
        row = G[ i ]
        le.append( [ row[ :, 0 ].min(), row[ :, 1 ].mean(), 0.0 ] )
        te.append( [ row[ :, 0 ].max(), row[ :, 1 ].mean(), 0.0 ] )
    le = np.array( le )
    te = np.array( te )
    return le, te


def fill_plan( c, g, color = FIN ):
    le, te = plan( g )
    P = np.vstack( [ le, te[ ::-1 ] ] )
    bf.fill( c, P, face = color, edge = bf.POS_EDGE, z = 10 )
    return le, te


def body_strip( c, x0, x1, r = 0.5 ):
    bf.fill( c, np.array( [ [ x0, -r, 0 ], [ x1, -r, 0 ], [ x1, r, 0 ], [ x0, r, 0 ] ] ), face = df.BODY, edge = bf.POS_EDGE, z = 5 )
    bf.axis_line( c, x0 - 0.3, x1 + 0.3 )


def dash( c, p0, p1 ):
    c.line( np.array( [ p0, p1 ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 40 )


def planform():
    """MdcFin_planform: the stations of a panel and what sets them."""
    bf.renew()
    kw = dict( NSPAN = 3, RBODY = 0.5, SSPAN_1 = 0.0, SSPAN_2 = 1.0, SSPAN_3 = 1.7, CHORD_1 = 1.6, CHORD_2 = 1.1, CHORD_3 = 0.6, X = 2.0, SWEEP_1 = 45.0,
               STA_1 = 0.0, XLEFlag_2 = 1, XLE_3 = 1.8 )
    g = one_panel( **kw )
    xle = [ 2.0, 2.0 + 1.0 * np.tan( np.radians( 45.0 ) ), 3.8 ]
    ch = [ 1.6, 1.1, 0.6 ]
    y = [ 0.5, 1.5, 2.2 ]
    le, te = plan( g )
    assert abs( le[ :, 0 ].min() - 2.0 ) < 1e-6 and abs( te[ :, 0 ].max() - ( 3.8 + 0.6 ) ) < 1e-6
    for k in range( 3 ):
        j = np.argmin( abs( le[ :, 1 ] - y[ k ] ) )
        assert abs( le[ j, 0 ] - xle[ k ] ) < 2e-2 and abs( te[ j, 0 ] - ( xle[ k ] + ch[ k ] ) ) < 2e-2, ( k, le[ j ], te[ j ] )
    fig = bf.figure()
    c = bf.canvas( fig, bf.TOP )
    body_strip( c, 0.0, 6.0 )
    fill_plan( c, g )
    for k in range( 3 ):
        dash( c, np.array( [ 0.0, y[ k ], 0 ] ), np.array( [ 6.0, y[ k ], 0 ] ) )
        bf.label( c, np.array( [ 6.0, y[ k ], 0 ] ), "station %d" % ( k + 1 ), ha = "left", dx = 6 )
    dash( c, np.array( [ 0.0, 0.0, 0 ] ), np.array( [ 0.0, 2.5, 0 ] ) )
    bf.label( c, np.array( [ 0.0, 2.5, 0 ] ), "X = 0", dy = 7 )
    # The root's leading edge is the X location; a later station's XLE is from the root's leading edge.
    bf.dim( c, np.array( [ 0.0, y[ 0 ] + 0.12, 0 ] ), np.array( [ xle[ 0 ], y[ 0 ] + 0.12, 0 ] ), "X", dy = 7, ext = False )
    for k in range( 1, 3 ):
        bf.dim( c, np.array( [ xle[ 0 ], y[ k ] + 0.12, 0 ] ), np.array( [ xle[ k ], y[ k ] + 0.12, 0 ] ), "XLE_%d" % ( k + 1 ), dy = 7, ext = False )
        bf.dim( c, np.array( [ xle[ k ], y[ k ] - 0.12, 0 ] ), np.array( [ xle[ k ] + ch[ k ], y[ k ] - 0.12, 0 ] ), "CHORD_%d" % ( k + 1 ), dy = -8, ext = False ) if k > 0 else None
    bf.dim( c, np.array( [ xle[ 0 ], 0.5 - 0.12, 0 ] ), np.array( [ xle[ 0 ] + ch[ 0 ], 0.5 - 0.12, 0 ] ), "CHORD_1", dy = -8, ext = False )
    bf.dim( c, np.array( [ 4.9, 0.5, 0 ] ), np.array( [ 4.9, 1.5, 0 ] ), "SSPAN_2", dx = 24, ext = False )
    bf.dim( c, np.array( [ 5.3, 0.5, 0 ] ), np.array( [ 5.3, 2.2, 0 ] ), "SSPAN_3", dx = 24, ext = False )
    dash( c, np.array( [ 2.0, 0.5, 0 ] ), np.array( [ 2.0, 2.4, 0 ] ) )
    t = np.radians( np.linspace( 90.0, 45.0, 30 ) )
    c.line( np.array( [ [ 2.0 + 0.8 * np.cos( a_ ), 0.5 + 0.8 * np.sin( a_ ), 0 ] for a_ in t ] ), color = "k", lw = 0.8, z = 60 )
    q = np.array( [ 2.0 + 0.8 * np.cos( np.radians( 75 ) ), 0.5 + 0.8 * np.sin( np.radians( 75 ) ), 0 ] )
    c.line( np.array( [ q, [ 1.95, q[ 1 ], 0 ] ] ), color = "k", lw = 0.5, z = 60 )
    bf.label( c, np.array( [ 1.95, q[ 1 ], 0 ] ), "SWEEP_1", ha = "right", dx = -3 )
    bf.key( c, "top", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "MdcFin_planform", [ [ c ] ], hmax = 6.0 )


def sta():
    """MdcFin_sta: the same SWEEP measured at three values of STA."""
    fig = bf.figure()
    rows = []
    for sta_, nm in ( ( 0.0, "STA 0: the leading edge" ), ( 0.5, "STA 0.5: the midchord" ), ( 1.0, "STA 1: the trailing edge" ) ):
        bf.renew()
        g = one_panel( NSPAN = 2, SSPAN_1 = 0.0, SSPAN_2 = 1.0, CHORD_1 = 1.4, CHORD_2 = 0.6, X = 2.0, SWEEP_1 = 25.0, STA_1 = sta_ )
        le, te = plan( g )
        x0 = 2.0 + sta_ * 1.4
        x1 = x0 + np.tan( np.radians( 25.0 ) ) * 1.0
        assert abs( le[ -1, 0 ] - ( x1 - sta_ * 0.6 ) ) < 1e-6
        c = bf.canvas( fig, bf.TOP )
        fill_plan( c, g )
        c.line( np.array( [ [ x0, 0.5, 0 ], [ x1, 1.5, 0 ] ] ), color = "#0072B2", lw = 1.6, z = 50 )
        dash( c, np.array( [ x0, 0.5, 0 ] ), np.array( [ x0, 1.6, 0 ] ) )
        t = np.radians( np.linspace( 90.0, 90.0 - 25.0, 20 ) )
        c.line( np.array( [ [ x0 + 0.7 * np.cos( a_ ), 0.5 + 0.7 * np.sin( a_ ), 0 ] for a_ in t ] ), color = "k", lw = 0.8, z = 60 )
        bf.label( c, np.array( [ x0 - 0.05, 1.0, 0 ] ), "SWEEP 25", ha = "right", dx = -2 )
        bf.title( c, nm, pad = 4 )
        rows.append( [ c ] )
    bf.save( fig, "MdcFin_sta", rows, hmax = 7.0 )


def section( g, y, s = 0, n = 401 ):
    """The section of panel s at span position y, a panel at PHIF 90: a closed outline in the XZ plane."""
    us = np.linspace( 0.0, 1.0, 4001 )
    ys = np.array( [ vsp.CompPnt01( g, s, u, 0.25 ).y() for u in us ] )
    # y rises along the span between the root cap and the tip cap.
    i0 = int( np.searchsorted( ys, ys.min() + 1e-9 ) ) - 1
    i1 = int( np.searchsorted( ys, ys.max() - 1e-9 ) )
    u = np.interp( y, ys[ i0:i1 + 1 ], us[ i0:i1 + 1 ] )
    return np.array( [ list( vsp.CompPnt01( g, s, u, w ) ) for w in np.linspace( 0.0, 1.0, n ) ] )


def sections():
    """MdcFin_sections: HEX, ARC and a HEX with a rounded leading edge, each of chord 1."""
    fig = bf.figure()
    rows = []
    hexkw = dict( SECTYP = 0, ZUPPER_1 = 0.12, ZUPPER_2 = 0.12, ZLOWERRelFlag = 0, ZLOWER_1 = 0.06, ZLOWER_2 = 0.06, LMAXU_1 = 0.4, LMAXU_2 = 0.4,
                  LMAXLRelFlag = 0, LMAXL_1 = 0.5, LMAXL_2 = 0.5, LFLATU_1 = 0.2, LFLATU_2 = 0.2, LFLATLRelFlag = 0, LFLATL_1 = 0.1, LFLATL_2 = 0.1 )
    arckw = dict( SECTYP = 2, ZUPPER_1 = 0.12, ZUPPER_2 = 0.12, ZLOWERRelFlag = 0, ZLOWER_1 = 0.06, ZLOWER_2 = 0.06 )
    cases = [ ( "HEX", hexkw ), ( "ARC", arckw ), ( "HEX with LER 0.03, Keep Crest", dict( hexkw, LERType = 1, LER_1 = 0.03, LER_2 = 0.03 ) ) ]
    sharp = None
    for nm, kw in cases:
        bf.renew()
        g = one_panel( NSPAN = 2, SSPAN_1 = 0.0, SSPAN_2 = 1.0, CHORD_1 = 1.0, CHORD_2 = 1.0, X = 0.0, **kw )
        P = section( g, 1.0, n = 20001 )
        assert abs( P[ :, 0 ].max() - 1.0 ) < 1e-3 and abs( P[ :, 2 ].max() - 0.12 ) < 1e-3 and abs( P[ :, 2 ].min() + 0.06 ) < 1e-3
        if nm == "HEX":
            sharp = P
            assert abs( P[ :, 0 ].min() ) < 1e-3
        c = bf.canvas( fig, bf.SIDE )
        bf.fill( c, P )
        c.line( np.array( [ [ 0, 0, 0 ], [ 1, 0, 0 ] ] ), color = "#888888", lw = 0.6, ls = ( 0, ( 8, 2, 1, 2 ) ) )
        if nm == "HEX":
            bf.dim( c, np.array( [ 0.4, 0, 0 ] ), np.array( [ 0.4, 0, 0.12 ] ), "ZUPPER", dx = -30, ext = False )
            bf.dim( c, np.array( [ 0.5, 0, 0 ] ), np.array( [ 0.5, 0, -0.06 ] ), "ZLOWER", dx = 30, ext = False )
            bf.dim( c, np.array( [ 0.0, 0, 0.2 ] ), np.array( [ 0.4, 0, 0.2 ] ), "LMAXU", dy = 8, ext = False )
            bf.dim( c, np.array( [ 0.4, 0, 0.2 ] ), np.array( [ 0.6, 0, 0.2 ] ), "LFLATU", dy = 8, ext = False )
            bf.dim( c, np.array( [ 0.0, 0, -0.14 ] ), np.array( [ 0.5, 0, -0.14 ] ), "LMAXL", dy = -8, ext = False )
            bf.dim( c, np.array( [ 0.5, 0, -0.14 ] ), np.array( [ 0.6, 0, -0.14 ] ), "LFLATL", dy = -8, ext = False )
            for x, z0, z1 in ( ( 0.0, 0.0, 0.22 ), ( 0.4, 0.12, 0.22 ), ( 0.6, 0.12, 0.22 ), ( 0.5, -0.06, -0.16 ), ( 0.6, -0.06, -0.16 ), ( 0.0, 0.0, -0.16 ) ):
                c.line( np.array( [ [ x, 0, z0 ], [ x, 0, z1 ] ] ), color = "#777777", lw = 0.5, z = 40 )
        if nm == "ARC":
            bf.dim( c, np.array( [ 0.5, 0, 0 ] ), np.array( [ 0.5, 0, 0.12 ] ), "ZUPPER", dx = -30, ext = False )
            bf.dim( c, np.array( [ 0.5, 0, 0 ] ), np.array( [ 0.5, 0, -0.06 ] ), "ZLOWER", dx = 30, ext = False )
        if nm.startswith( "HEX with" ):
            c.line( sharp, color = "#777777", lw = 0.7, ls = ( 0, ( 3, 2 ) ), z = 40 )
        bf.title( c, nm, pad = 22 if nm == "HEX" else 4 )
        rows.append( [ c ] )
    bf.save( fig, "MdcFin_sections", rows, hmax = 7.0 )


def naca():
    """MdcFin_naca: sections of the NACA control card's series."""
    fig = bf.figure()
    cases = [ ( "4 digit, NACADES 2412", dict( NACASER = 0, NACADES = 2412 ), 0.12 ),
              ( "5 digit, NACADES 23012", dict( NACASER = 2, NACADES = 23012 ), 0.12 ),
              ( "6 series, NACADES 64212", dict( NACASER = 5, NACADES = 64212 ), 0.12 ),
              ( "Supersonic diamond, NACADES 6, NACAXMAX 50", dict( NACASER = 6, NACADES = 6, NACAXMAX = 50 ), 0.06 ),
              ( "Supersonic arc, NACADES 6, NACAXMAX 35", dict( NACASER = 7, NACADES = 6, NACAXMAX = 35 ), 0.06 ),
              ( "Supersonic hex, NACADES 6, NACAXMAX 30, NACAFLAT 30", dict( NACASER = 8, NACADES = 6, NACAXMAX = 30, NACAFLAT = 30 ), 0.06 ) ]
    cs = []
    for nm, kw, th in cases:
        bf.renew()
        g = one_panel( NSPAN = 2, SSPAN_1 = 0.0, SSPAN_2 = 1.0, CHORD_1 = 1.0, CHORD_2 = 1.0, X = 0.0, SECTYP = 1, **kw )
        P = section( g, 1.0, n = 20001 )
        assert abs( P[ :, 0 ].max() - 1.0 ) < 2e-3 and abs( P[ :, 0 ].min() ) < 2e-3, ( nm, P[ :, 0 ].min(), P[ :, 0 ].max() )
        print( nm, "thickness", P[ :, 2 ].max() - P[ :, 2 ].min() )
        c = bf.canvas( fig, bf.SIDE )
        bf.fill( c, P )
        c.line( np.array( [ [ 0, 0, 0 ], [ 1, 0, 0 ] ] ), color = "#888888", lw = 0.6, ls = ( 0, ( 8, 2, 1, 2 ) ) )
        bf.title( c, nm, pad = 4 )
        cs.append( c )
    bf.save( fig, "MdcFin_naca", [ cs[ 0:2 ], cs[ 2:4 ], cs[ 4:6 ] ] )


def rear_panels( fig, g, body, labels, rho = 1.55 ):
    """A REAR view of fin set g on body, with the panels as thick lines out from the root."""
    c = bf.canvas( fig, df.REAR )
    bf.fill( c, df.circle( body, 9.0 ), face = df.BODY, edge = bf.POS_EDGE, z = 5 )
    for G in df.surfaces( g, 31, 161 ):
        c.surface( G, color = FIN, edge = False )
    c.flush( 10.0 )
    return c


def panels():
    """MdcFin_panels: the roll angles PHIF of panels, and the dihedral GAM."""
    fig = bf.figure()
    cs = []
    bf.renew()
    body = df.cylinder( 0.5, 6.0, 12.0 )
    g = fin( NPANEL = 3, NSPAN = 2, SSPAN_2 = 1.0, ZUPPER_1 = 0.04, ZUPPER_2 = 0.04, ZLOWERRelFlag = 1 )
    c = rear_panels( fig, g, body, None )
    c.line( np.array( [ [ 0, 0, 0 ], [ 0, 0, 1.75 ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 20 )
    bf.label( c, np.array( [ 0, 0, 1.75 ] ), "top", ha = "left", dx = 4, dy = 4 )
    for ang in ( 120, 240 ):
        c.line( np.array( [ [ 0, 0, 0 ], [ 0, 1.6 * np.sin( np.radians( ang ) ), 1.6 * np.cos( np.radians( ang ) ) ] ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 20 )
    P = df.arc( c, 1.25, 0, 120 )
    bf.label( c, P[ -1 ], "PHIF_2 = 120", ha = "left", dx = 14, dy = -12 )
    P = df.arc( c, 1.4, 0, 240 )
    bf.label( c, P[ -1 ], "PHIF_3 = 240", ha = "right", dx = -6, dy = -8 )
    bf.label( c, np.array( [ 0, 0, 1.3 ] ), "PHIF_1 = 0", dx = -4, ha = "right" )
    bf.title( c, "NPANEL 3, evenly spaced", pad = 14 )
    bf.key( c, "rear", ( 1.0, 0.0 ), 60, 0 )
    cs.append( c )
    bf.renew()
    body = df.cylinder( 0.5, 6.0, 12.0 )
    g = fin( NPANEL = 2, NSPAN = 2, SSPAN_2 = 1.0, ZUPPER_1 = 0.04, ZUPPER_2 = 0.04, PHIFFlag = 1, PHIF_1 = 90.0, PHIF_2 = 270.0, GAM_1 = 25.0, GAM_2 = 25.0 )
    c = rear_panels( fig, g, body, None )
    for ang in ( 90, 270 ):
        r0 = np.array( [ 0, 0.5 * np.sin( np.radians( ang ) ), 0.5 * np.cos( np.radians( ang ) ) ] )
        e = np.array( [ 0, 1.75 * np.sin( np.radians( ang ) ), 1.75 * np.cos( np.radians( ang ) ) ] )
        c.line( np.array( [ r0, e ] ), color = "#777777", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 20 )
        # The dihedral turns the panel about its root, clockwise looking forward, by GAM.
        t = np.radians( np.linspace( ang, ang + 25.0, 20 ) )
        P = np.array( [ r0 + 1.3 * np.array( [ 0, np.sin( a_ ), np.cos( a_ ) ] ) for a_ in t ] )
        c.arrow_path( P, color = "k", lw = 0.8, head = 6.0, z = 60 )
        bf.label( c, P[ -1 ], "GAM = 25", ha = "left" if ang < 180 else "right", dx = 6 if ang < 180 else -6, dy = -6 if ang < 180 else 6 )
    bf.title( c, "NPANEL 2, PHIF 90 and 270", pad = 14 )
    bf.key( c, "rear", ( 1.0, 0.0 ), 60, 0 )
    cs.append( c )
    bf.save( fig, "MdcFin_panels", [ [ cs[ 0 ] ], [ cs[ 1 ] ] ], hmax = 6.5 )


def flaps():
    """MdcFin_flaps: the flap CFOC makes on one segment."""
    bf.renew()
    kw = dict( NSPAN = 4, SSPAN_1 = 0.0, SSPAN_2 = 0.6, SSPAN_3 = 1.2, SSPAN_4 = 1.8, CHORD_1 = 1.4, CHORD_2 = 1.2, CHORD_3 = 1.0, CHORD_4 = 0.7, X = 2.0,
               CFOC_1 = 0.0, CFOC_2 = 0.3, CFOC_3 = 0.25, CFOC_4 = 0.0 )
    g = one_panel( **kw )
    ids = vsp.GetSubSurfIDVec( g )
    assert len( ids ) == 1 and vsp.GetSubSurfName( ids[ 0 ] ) == "FLAP_2"
    pv = { vsp.GetParmName( q ): vsp.GetParmVal( q ) for q in vsp.GetSubSurfParmIDs( ids[ 0 ] ) }
    assert abs( pv[ "Length_C_Start" ] - 0.3 ) < 1e-9 and abs( pv[ "Length_C_End" ] - 0.25 ) < 1e-9
    le, te = plan( g )
    y = [ 0.5, 1.1, 1.7, 2.3 ]
    ch = [ 1.4, 1.2, 1.0, 0.7 ]
    xle = []
    for k in range( 4 ):
        j = np.argmin( abs( le[ :, 1 ] - y[ k ] ) )
        xle.append( le[ j, 0 ] )
    fig = bf.figure()
    c = bf.canvas( fig, bf.TOP )
    body_strip( c, 0.0, 5.0 )
    fill_plan( c, g )
    cf = [ 0.0, 0.3, 0.25, 0.0 ]
    hinge = [ xle[ k ] + ch[ k ] * ( 1.0 - cf[ k ] ) for k in ( 1, 2 ) ]
    flap = np.array( [ [ hinge[ 0 ], y[ 1 ], 0 ], [ hinge[ 1 ], y[ 2 ], 0 ], [ xle[ 2 ] + ch[ 2 ], y[ 2 ], 0 ], [ xle[ 1 ] + ch[ 1 ], y[ 1 ], 0 ] ] )
    bf.fill( c, flap, face = "#7FB3D5", edge = bf.POS_EDGE, z = 20 )
    bf.label( c, np.array( [ hinge[ 0 ], 0.5 * ( y[ 1 ] + y[ 2 ] ), 0 ] ), "FLAP_2", ha = "right", dx = -6, z = 60 )
    for k in range( 4 ):
        dash( c, np.array( [ 0.0, y[ k ], 0 ] ), np.array( [ 5.0, y[ k ], 0 ] ) )
        bf.label( c, np.array( [ 5.0, y[ k ], 0 ] ), "station %d, CFOC_%d = %s" % ( k + 1, k + 1, [ "0", "0.3", "0.25", "0" ][ k ] ), ha = "left", dx = 6 )
    bf.key( c, "top", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "MdcFin_flaps", [ [ c ] ], hmax = 6.0 )


def overview():
    bf.renew()
    body = df.cylinder( 0.5, 6.0, 12.0 )
    g = fin()
    fig = bf.figure()
    c = bf.canvas( fig, bf.ISO )
    df.draw( c, df.surfaces( body, 181, 129 ), df.BODY )
    df.draw( c, df.surfaces( g, 61, 161 ), FIN, edge = False )
    c.flush( 1.0 )
    bf.save( fig, "MdcFin_overview", [ [ c ] ] )


FIGS = { "overview": overview, "planform": planform, "sta": sta, "sections": sections, "naca": naca, "panels": panels, "flaps": flaps }

if __name__ == "__main__":
    for name in ( sys.argv[ 1: ] or FIGS ):
        FIGS[ name ]()
