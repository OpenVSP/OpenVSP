"""Checks the statements of MdcBod.md, MdcProtub.md, MdcInlet.md and MdcFin.md against the openvsp module.

    <build>/venv/bin/python verify.py

Each check prints what it measured; a failed check raises.  Rerun after any change to the Geoms these pages describe.
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
for sub in ( "fuselagestack", "mdc_bod", "mdc_protub", "mdc_inlet", "mdc_fin" ):
    sys.path.insert( 0, os.path.join( HERE, "..", sub ) )

import numpy as np

import bodyfig as bf
from bodyfig import vsp
import datcomfig as df
import mdc_bod_figs as mf
import mdc_protub_figs as pf
import mdc_inlet_figs as inf
import mdc_fin_figs as ff


def near( a, b, tol, what ):
    assert abs( a - b ) <= tol, "%s: %g, expected %g" % ( what, a, b )
    print( "ok  %-64s %.6g" % ( what, a ) )


def check( cond, what ):
    assert cond, what
    print( "ok  %s" % what )


def extent( g, s = 0, nu = 61, nw = 161 ):
    P = bf.grid( g, s, nu = nu, nw = nw ).reshape( -1, 3 )
    return P.min( 0 ), P.max( 0 )


def mdc_bod():
    g = mf.body()
    lo, hi = extent( g )
    near( hi[ 0 ], 10.0, 1e-9, "New MDC_BOD is 10 long" )
    near( hi[ 2 ], 0.5, 1e-6, "New MDC_BOD is 1 in diameter" )
    # A centerbody's size takes the nose's by default, and is its own without the default.
    g = mf.body( DNOSE = 2.0 )
    near( vsp.GetParmVal( g, "DCENTR", "Design" ), 2.0, 1e-12, "DCENTR takes DNOSE by default" )
    vsp.SetParmVal( g, "CENTRDefault", "Design", 0.0 )
    vsp.SetParmVal( g, "DCENTR", "Design", 3.0 )
    vsp.Update()
    near( vsp.GetParmVal( g, "DCENTR", "Design" ), 3.0, 1e-12, "DCENTR is its own without the default" )
    # Nose shapes.
    g = mf.body( TNOSE = 2, POWER = 0.5, LNOSE = 4.0, DNOSE = 1.0 )
    P = mf.top( g )
    r = np.interp( 2.0, P[ :, 0 ], P[ :, 2 ] )
    near( r / 0.5, np.sqrt( 0.5 ), 2e-3, "Power law nose r/R = (x/L)^n at x/L = 0.5, n = 0.5" )
    # Ogive slope at the base, and the secant limit.
    for slp, tangent in ( ( 0.0, True ), ( 0.5 / 3.0, False ) ):
        g = mf.body( TNOSE = 1, LNOSE = 3.0, DNOSE = 1.0, SLPNOS = slp )
        P = mf.top( g, 4001 )
        i = np.argmin( abs( P[ :, 0 ] - 2.9 ) )
        j = np.argmin( abs( P[ :, 0 ] - 2.95 ) )
        sl = ( P[ j, 2 ] - P[ i, 2 ] ) / ( P[ j, 0 ] - P[ i, 0 ] )
        print( "    SLPNOS %.3f: slope near the base %.3f" % ( slp, sl ) )
        check( ( sl < 0.1 ) == tangent, "SLPNOS %.3f is %s at the base" % ( slp, "tangent" if tangent else "the cone's slope" ) )
    g = mf.body( TNOSE = 1, LNOSE = 3.0, DNOSE = 1.0, SLPNOS = 0.5 )
    check( mf.top( g )[ :, 2 ].min() < -1e-3, "SLPNOS well past the cone's slope crosses the axis" )
    # Blunt and truncated.
    g = mf.body( TNOSE = 1, BNOSE = 0.2, TRUNC = 1 )
    P = mf.top( g, 2001 )
    near( P[ P[ :, 0 ] < 1e-9, 2 ].max(), 0.2, 0.002, "A truncated nose's flat face has the radius BNOSE" )
    near( P[ 0, 0 ], 0.0, 1e-9, "The face is at the nose tip, XO" )
    g = mf.body( TNOSE = 1, BNOSE = 0.2 )
    P = mf.top( g, 2001 )
    near( P[ :, 0 ].min(), 0.0, 1e-9, "A blunted nose keeps its length" )
    # A power law, Haack or Von Karman nose blunted: its tip on the sphere, its length and base kept.
    for t, name in ( ( 2, "power law" ), ( 3, "Haack" ), ( 4, "Von Karman" ) ):
        g = mf.body( TNOSE = t, POWER = 0.75, BNOSE = 0.1 )
        P = mf.top( g, 4001 )
        P = P[ np.argsort( P[ :, 0 ] ) ]
        near( P[ :, 0 ].min(), 0.0, 1e-9, "A blunted %s nose keeps its length" % name )
        near( P[ P[ :, 0 ] <= 3.0 + 1e-9, 2 ].max(), 0.5, 5e-4, "A blunted %s nose keeps its base" % name )
        near( np.interp( 0.02, P[ :, 0 ], P[ :, 2 ] ), np.sqrt( 0.1 ** 2 - 0.08 ** 2 ), 1e-3,
              "A blunted %s nose's tip is on the sphere of radius BNOSE" % name )
    # A power law nose of exponent one-half is rounder than a sphere smaller than R^2 / ( 2 L ), and stays sharp.
    g = mf.body( TNOSE = 2, POWER = 0.5, BNOSE = 0.02 )
    P = mf.top( g, 4001 )
    P = P[ np.argsort( P[ :, 0 ] ) ]
    near( np.interp( 0.02, P[ :, 0 ], P[ :, 2 ] ), 0.5 * np.sqrt( 0.02 / 3.0 ), 1e-3,
          "A power law nose of exponent one-half under a small sphere stays sharp" )
    # A sphere as large as the base gives the blunted cone's shape.
    tops = []
    for t in ( 0, 2 ):
        g = mf.body( TNOSE = t, POWER = 0.75, BNOSE = 0.5 )
        P = mf.top( g, 2001 )
        P = P[ np.argsort( P[ :, 0 ] ) ]
        tops.append( np.interp( np.linspace( 0.0, 3.0, 301 ), P[ :, 0 ], P[ :, 2 ] ) )
    near( np.abs( tops[ 0 ] - tops[ 1 ] ).max(), 0.0, 2e-3, "A sphere as large as the base gives the blunted cone" )
    # Camber.
    g = mf.body( TNOSE = 1, LNOSE = 3.0, LCENTR = 4.0, LAFT = 2.0, DAFT = 0.6, ZTIP = 0.0, ZNOSE = 0.4, ZCENTR = 1.0, ZAFT = 0.6 )
    us = np.linspace( 0.0, 1.0, 4001 )
    X = np.array( [ vsp.CompPnt01( g, 0, u, 0.0 ).x() for u in us ] )
    for x, z in ( ( 3.0, 0.4 ), ( 7.0, 1.0 ), ( 9.0 - 1e-6, 0.6 ) ):
        u = np.interp( x, X, us )
        a = vsp.CompPnt01( g, 0, u, 0.0 )
        b = vsp.CompPnt01( g, 0, u, 0.5 )
        near( 0.5 * ( a.z() + b.z() ), z, 1e-6, "Section center at x = %g" % x )
    # Boattail and flare slopes.
    g = mf.body( CENTRDefault = 0, LNOSE = 2.0, LCENTR = 2.0, LAFT = 3.0, DAFT = 1.4, TAFT = 1, SLPTAL = 0.3 )
    check( mf.top( g )[ :, 2 ].min() < 0.5, "A secant flare first narrows below the centerbody" )
    # ELLBOD.
    g = mf.body( NAMELIST = 1, WNOSE = 1.0, ENOSE = 0.6, ECENTR = 0.6 )
    lo, hi = extent( g )
    near( hi[ 1 ], 0.5, 1e-3, "ELLBOD half width" )
    near( hi[ 2 ], 0.3, 1e-3, "ELLBOD half height is ENOSE times the width over two" )


def protub():
    bf.renew()
    for t in range( 1, 7 ):
        p = pf.protub( t )
        print( "    PTYPE %d: %d surfaces" % ( t, vsp.GetNumMainSurfs( p ) ) )
    bf.renew()
    p = pf.protub( 5, defaults = False )
    vsp.SetParmVal( p, "NLOC", "Design", 3.0 )
    vsp.Update()
    for k, v in dict( PHIPROFlag = 1, PHIPRO_1 = 0, PHIPRO_2 = 90, PHIPRO_3 = 225 ).items():
        vsp.SetParmVal( p, k, "Design", float( v ) )
    vsp.Update()
    cen = [ bf.grid( p, s, nu = 11, nw = 41 ).reshape( -1, 3 ).mean( 0 ) for s in range( 3 ) ]
    ang = sorted( ( np.degrees( np.arctan2( c[ 1 ], c[ 2 ] ) ) % 360.0 ) for c in cen )
    for got, want in zip( ang, ( 0.0, 90.0, 225.0 ) ):
        near( got, want, 1.0, "A protuberance at PHIPRO %g is at that roll angle, clockwise from the top looking forward" % want )
    # Evenly spaced angles are written from NLOC.
    bf.renew()
    p = pf.protub( 5 )
    vsp.SetParmVal( p, "NLOC", "Design", 3.0 )
    vsp.Update()
    near( vsp.GetParmVal( p, "PHIPRO_2", "Design" ), 120.0, 1e-9, "PHIPRO is evenly spaced from NLOC" )
    # A block with no offset reaches below the mold line.
    bf.renew()
    p = pf.protub( 5, defaults = True )
    lo, hi = extent( p, nu = 41, nw = 161 )
    near( lo[ 2 ], np.sqrt( 36.0 - 1.0 ) - 0.1 * 1.5, 1e-6, "A block with OPROT zero goes under the mold line by a tenth of its height" )
    near( hi[ 2 ], 7.5, 1e-6, "A block is HPROT above the mold line" )
    # The members of a LUG and a SHOE are stacked at their offsets.
    for t in ( 3, 4 ):
        m = pf.preset_members( t )
        top = 0.0
        for k in range( len( m ) ):
            print( "    PTYPE %d member %d: LPROT %g WPROT %g HPROT %g OPROT %g" % ( ( t, k + 1 ) + tuple( m[ k ] ) ) )
    near( pf.preset_members( 3 )[ 3 ][ 3 ], 0.1 + 0.5, 1e-12, "LUG top plate offset is the base plate plus a post" )
    near( pf.preset_members( 4 )[ 2 ][ 3 ], 0.1 + 0.75, 1e-12, "SHOE top plate offset is the base plate plus the web" )


def inlet():
    bf.renew()
    g = inf.inlet( HDIV = 0.1 )
    lo, hi = extent( g )
    near( lo[ 2 ], 0.6, 1e-9, "The inner face is HDIV above the mold line" )
    near( hi[ 2 ], 0.6 + 0.45, 1e-9, "H is measured from the inner face" )
    near( hi[ 0 ] - lo[ 0 ], 3.0, 1e-9, "The inlet is X(5) long" )
    bf.renew()
    g = inf.inlet( HDIV = 0.0 )
    lo, hi = extent( g )
    near( lo[ 2 ], 0.5 - 0.005, 1e-9, "A conformal inlet is sunk a hundredth of RBODY" )
    # A ramp steeper than the plug is the plug.
    for t in ( 1, 3 ):
        bf.renew()
        a = bf.grid( inf.inlet( INTYPE = t, COVER = 1 ), 0, nu = 41, nw = 81 )
        bf.renew()
        b = bf.grid( inf.inlet( INTYPE = t, COVER = 0, RAMP = 80.0 ), 0, nu = 41, nw = 81 )
        near( abs( a - b ).max(), 0.0, 1e-9, "INTYPE %d: a very steep ramp is the plug" % t )
    bf.renew()
    g = inf.inlet( INTYPE = 3, W1 = 0.0 )
    P = bf.grid( g, 0, nu = 41, nw = 81 ).reshape( -1, 3 )
    near( P[ :, 2 ].min(), 0.6, 1e-9, "An AXI inlet's lowest line is the inner face" )
    near( P[ :, 2 ].max(), 1.2, 1e-9, "An AXI inlet's axis is W(3) over two above it" )
    bf.renew()
    g = inf.inlet( INTYPE = 2, PHIFlag = 1, PHI_1 = 270.0 )
    lo, hi = extent( g )
    check( hi[ 1 ] < -0.5, "PHI 270 is on the left, -Y, looking forward" )


def finset():
    bf.renew()
    g = ff.fin()
    near( vsp.GetNumMainSurfs( g ), 4, 0, "A new MDC_FIN has four panels" )
    near( vsp.GetParmVal( g, "XLE_2", "Design" ), 0.75, 1e-12, "XLE_2, from the root's leading edge, follows the sweep" )
    # SSPAN_1 and RBODY.
    bf.renew()
    g = ff.one_panel( RBODY = 2.0 )
    lo, hi = extent( g )
    near( lo[ 1 ], 2.0, 1e-9, "The root is RBODY from the axis" )
    bf.renew()
    g = ff.one_panel( SSPAN_1 = 1.0, SSPAN_2 = 2.0, RBODY = 0.5 )
    lo, hi = extent( g )
    near( lo[ 1 ], 1.0, 1e-9, "The root is SSPAN_1 from the axis where it is not zero" )
    # A panel at the side is rolled to +Y at 90.
    bf.renew()
    g = ff.fin( NPANEL = 4, NSPAN = 2, SSPAN_2 = 1.0, ZUPPER_1 = 0.1, ZUPPER_2 = 0.1, ZLOWERRelFlag = 0, ZLOWER_1 = 0.0, ZLOWER_2 = 0.0,
                PHIFFlag = 1, PHIF_1 = 0.0, PHIF_2 = 90.0, PHIF_3 = 180.0, PHIF_4 = 270.0 )
    ext = [ extent( g, s, 31, 161 ) for s in range( 4 ) ]
    check( ext[ 1 ][ 0 ][ 1 ] > 0.4 and ext[ 3 ][ 1 ][ 1 ] < -0.4, "PHIF 90 is on +Y and 270 on -Y" )
    check( ext[ 1 ][ 0 ][ 2 ] > -1e-9 and ext[ 3 ][ 0 ][ 2 ] > -1e-9, "Camber is toward +Z on panels at both sides" )
    check( ext[ 0 ][ 0 ][ 1 ] > -1e-9 and ext[ 2 ][ 0 ][ 1 ] > -1e-9, "Camber is toward +Y on the top and bottom panels" )
    # GAM is positive in the direction of increasing PHIF: the tip of a right panel goes down.
    bf.renew()
    g = ff.fin( NPANEL = 1, NSPAN = 2, SSPAN_2 = 1.0, PHIFFlag = 1, PHIF_1 = 90.0, GAM_1 = 25.0 )
    lo, hi = extent( g )
    check( lo[ 2 ] < -0.4, "Positive GAM turns the tip of a right panel down" )
    # LER is drawn sharp unless LER Type rounds it, and rounded it keeps the chord.
    bf.renew()
    g = ff.one_panel( NSPAN = 2, CHORD_1 = 1.0, CHORD_2 = 1.0, X = 0.0, LER_1 = 0.005, LER_2 = 0.005 )
    P = ff.section( g, 1.0, n = 801 )
    check( abs( P[ :, 0 ].min() ) < 1e-9 and abs( P[ :, 0 ].max() - 1.0 ) < 1e-6, "LER with LER Type Sharp leaves the diamond sharp" )
    for lertype, what in ( ( 1, "Keep Crest" ), ( 2, "Keep Angle" ) ):
        bf.renew()
        g = ff.one_panel( NSPAN = 2, CHORD_1 = 1.0, CHORD_2 = 1.0, X = 0.0, LER_1 = 0.005, LER_2 = 0.005, LERType = lertype )
        P = ff.section( g, 1.0, n = 2001 )
        crest = P[ int( np.argmax( P[ :, 2 ] ) ), 0 ]
        blunt = np.max( np.abs( P[ P[ :, 0 ] < 0.002, 2 ] ) )
        check( abs( P[ :, 0 ].min() ) < 1e-6 and abs( P[ :, 0 ].max() - 1.0 ) < 1e-6 and blunt > 0.003,
               "LER Type %s rounds the diamond and keeps its chord" % what )
        if lertype == 1:
            near( crest, 0.5, 0.002, "Keep Crest keeps the crest at LMAXU" )
        else:
            check( crest < 0.45, "Keep Angle moves the crest forward (%.3f)" % crest )
    # Flaps.
    bf.renew()
    g = ff.fin( NPANEL = 4, CFOC_1 = 0.3, CFOC_2 = 0.3 )
    check( [ vsp.GetSubSurfName( s ) for s in vsp.GetSubSurfIDVec( g ) ] == [ "FLAP_1" ], "CFOC of 0.3 at stations 1 and 2 makes one flap, FLAP_1" )
    bf.renew()
    g = ff.fin( NPANEL = 4, CFOC_1 = 1.0, CFOC_2 = 0.3 )
    near( vsp.GetNumSubSurf( g ), 0, 0, "CFOC_1 of 1 makes no flap" )
    # The USER section is a 5 percent biconvex airfoil until read.
    bf.renew()
    g = ff.one_panel( SECTYP = 3, NSPAN = 2, CHORD_1 = 1.0, CHORD_2 = 1.0, X = 0.0 )
    P = ff.section( g, 1.0 )
    near( P[ :, 2 ].max() - P[ :, 2 ].min(), 0.05, 1e-3, "The USER section is 5 percent thick until a file is read" )


if __name__ == "__main__":
    mdc_bod()
    protub()
    inlet()
    finset()
    print( "all checks passed" )
