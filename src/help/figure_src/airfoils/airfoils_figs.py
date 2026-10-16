"""Figures for src/help/Airfoils.md.

    <build>/venv/bin/python airfoils_figs.py [names...]      # writes ../../images/Airfoils_*.png

Every airfoil is VSP's own: the root section of a Wing, sampled with ComputeXSecPnt after every Modify operation,
and drawn in chords (X/c, Z/c) from the side, looking along +Y.  Shared helpers are in ../xseccurves/xsfig.py.
"""

import os
import sys

import numpy as np
import openvsp as vsp

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "xseccurves" ) )

import xsfig as xf                                   # noqa: E402
from xsfig import TRIO, INK, GHOST, MARK             # noqa: E402

FOUR = { "Camber": 0.02, "CamberLoc": 0.4, "ThickChord": 0.12 }        # NACA 2412

# Cut off where the trailing edge is 0.04 or 0.03 chords thick: a blunt trailing edge to put a cap on.
BLUNT = { "TE_Trim_Type": ( "Trim", vsp.TRIM_THICK ), "TE_Trim_AbsRel": ( "Trim", vsp.REL ),
          "TE_Trim_Thick_Chord": ( "Trim", 0.04 ) }
BLUNT3 = dict( BLUNT )
BLUNT3[ "TE_Trim_Thick_Chord" ] = ( "Trim", 0.03 )


def window( P, lo, hi ):
    """The pieces of polyline P inside the box lo..hi, so a close-up stays inside its panel."""
    inside = np.all( ( P >= lo ) & ( P <= hi ), axis = 1 )
    out = []
    cur = []
    for p, ok in zip( P, inside ):
        if ok:
            cur.append( p )
        elif cur:
            out.append( np.array( cur ) )
            cur = []
    if cur:
        out.append( np.array( cur ) )
    return [ q for q in out if len( q ) > 1 ]


def draw( c, P, lo = None, hi = None, **kw ):
    if lo is None:
        xf.curve( c, P, **kw )
        return
    for q in window( P, np.asarray( lo, float ), np.asarray( hi, float ) ):
        xf.curve( c, q, **kw )


def ghost( c, P, lo = None, hi = None ):
    draw( c, P, lo, hi, color = GHOST, lw = 1.0, ls = ( 0, ( 4, 2 ) ), z = 40 )


def frame( c, lo, hi ):
    c.bounds = ( np.asarray( lo, float ), np.asarray( hi, float ) )


def param():
    """Where an airfoil's curve starts and which way it runs."""
    P, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR )
    fig = xf.new()
    c = xf.canvas( fig )
    xf.curve( c, P, lw = 1.6 )
    c.line( xf.to3( [ [ 0, 0 ], [ 1, 0 ] ] ), color = GHOST, lw = 0.8, ls = ( 0, ( 6, 2, 1, 2 ) ) )
    for t in ( 0.2, 0.8 ):
        i = int( np.argmin( np.abs( f - t ) ) )
        xf.arrow_along( c, P, i, n = 6 )
    xf.dot( c, xf.at( P, f, 0.0 ), color = MARK )
    xf.dot( c, xf.at( P, f, 0.5 ), color = MARK )
    xf.label( c, xf.at( P, f, 0.0 ), "0 and 1: trailing edge", dx = 10, ha = "left" )
    xf.label( c, xf.at( P, f, 0.5 ), "0.5: leading edge", dx = -10, ha = "right" )
    xf.label( c, xf.at( P, f, 0.25 ), "lower surface", dy = -12 )
    xf.label( c, xf.at( P, f, 0.75 ), "upper surface", dy = 12 )
    xf.label( c, [ 0.3, 0.0 ], "chord", dy = 7, color = "#555555" )
    xf.key_side( c, at = ( 0.0, 0.0 ), dx = -40, dy = -30 )
    frame( c, [ 0, -0.08 ], [ 1, 0.1 ] )
    xf.save( fig, "Airfoils_param", [ [ c ] ], hmax = 2.0 )


FAMILIES = (
    ( "Four Series, NACA 2412", vsp.XS_FOUR_SERIES, FOUR ),
    ( "Four Digit Mod, NACA 2412-63", vsp.XS_FOUR_DIGIT_MOD, FOUR ),
    ( "Five Digit, NACA 23012", vsp.XS_FIVE_DIGIT, { "ThickChord": 0.12 } ),
    ( "Five Digit Mod, NACA 23012-63", vsp.XS_FIVE_DIGIT_MOD, { "ThickChord": 0.12 } ),
    ( "16 Series, NACA 16-212", vsp.XS_ONE_SIX_SERIES, { "ThickChord": 0.12 } ),
    ( "Six Series, NACA 63-212", vsp.XS_SIX_SERIES, { "ThickChord": 0.12, "IdealCl": 0.2 } ),
    ( "Biconvex, 10%", vsp.XS_BICONVEX, {} ),
    ( "Wedge, default", vsp.XS_WEDGE, {} ),
    ( "Karman-Trefftz, default", vsp.XS_VKT_AIRFOIL, {} ),
    ( "CST, default", vsp.XS_CST_AIRFOIL, {} ),
)


def families():
    """One of each airfoil family the equations define."""
    fig = xf.new()
    rows = []
    row = []
    for name, shape, parms in FAMILIES:
        P, f, xs = xf.wing_section( shape, parms )
        c = xf.canvas( fig )
        xf.curve( c, P )
        frame( c, [ 0, -0.1 ], [ 1, 0.12 ] )
        xf.title( c, name, pad = 4 )
        row.append( c )
        if len( row ) == 2:
            rows.append( row )
            row = []
    xf.key_side( rows[ -1 ][ 0 ], at = ( 0.0, 0.0 ), dx = -14, dy = -26 )
    xf.save( fig, "Airfoils_families", rows, hmax = 7.0 )


def four_mod():
    """Four digit modified: leading edge radius index and position of maximum thickness."""
    fig = xf.new()
    rows = []
    for parm, vals, name in ( ( "LERadIndx", ( 2.0, 6.0, 9.0 ), "LERadIndx" ),
                              ( "ThickLoc", ( 0.2, 0.4, 0.6 ), "T/CLoc" ) ):
        c = xf.canvas( fig )
        for k, v in enumerate( vals ):
            p = { "ThickChord": 0.12 }
            p[ parm ] = v
            P, f, xs = xf.wing_section( vsp.XS_FOUR_DIGIT_MOD, p )
            xf.curve( c, P, color = TRIO[k], lw = 1.2 )
            xf.label( c, [ 1.04, 0.03 - 0.025 * k ], "%s %g" % ( name, v ), color = TRIO[k], ha = "left" )
        frame( c, [ 0, -0.08 ], [ 1.3, 0.08 ] )
        rows.append( [ c ] )
    xf.key_side( rows[ -1 ][ 0 ], at = ( 0.0, 0.0 ), dx = -14, dy = -24 )
    xf.save( fig, "Airfoils_fourmod", rows, hmax = 4.0 )


def wedge():
    """The wedge and what each of its parameters places."""
    p = { "ThickChord": 0.12, "SymmThick": 0.0, "ThickLoc": 0.3, "FlatUp": 0.3, "ThickLocLow": 0.4, "FlatLow": 0.2,
          "ZCamber": 0.02 }
    P, f, xs = xf.wing_section( vsp.XS_WEDGE, p )
    fig = xf.new()
    c = xf.canvas( fig )
    xf.curve( c, P )
    c.line( xf.to3( [ [ 0, 0 ], [ 1, 0 ] ] ), color = GHOST, lw = 0.8, ls = ( 0, ( 6, 2, 1, 2 ) ) )
    up = 0.06 + 0.02
    lo = -0.06 + 0.02
    for q in ( [ 0.3, up ], [ 0.6, up ], [ 0.4, lo ], [ 0.6, lo ] ):
        xf.dot( c, q, color = MARK, ms = 4.0 )
    xf.dim( c, [ 0.0, 0 ], [ 0.3, 0 ], "Thick X", off = ( 0, 0.12 ), dy = 7 )
    xf.dim( c, [ 0.3, 0 ], [ 0.6, 0 ], "Flat Up", off = ( 0, 0.12 ), dy = 7 )
    xf.dim( c, [ 0.0, 0 ], [ 0.4, 0 ], "Thick X Low", off = ( 0, -0.09 ), dy = -7 )
    xf.dim( c, [ 0.4, 0 ], [ 0.6, 0 ], "Flat Low", off = ( 0, -0.09 ), dy = -7 )
    xf.dim( c, [ 0, lo ], [ 0, up ], "T/C", off = ( 0.66, 0 ), dx = 14 )
    xf.dim( c, [ 0, 0 ], [ 0, 0.02 ], "", off = ( 0.8, 0 ) )
    xf.label( c, [ 0.8, 0.01 ], "Camber", dx = 8, ha = "left" )
    xf.key_side( c, at = ( 0.0, 0.0 ), dx = -40, dy = -10 )
    frame( c, [ 0, -0.1 ], [ 1, 0.14 ] )
    xf.save( fig, "Airfoils_wedge", [ [ c ] ], hmax = 2.4 )


def vkt():
    """Karman-Trefftz: thickness, camber and trailing edge angle."""
    fig = xf.new()
    rows = []
    for parm, vals in ( ( "Epsilon", ( 0.05, 0.1, 0.2 ) ), ( "Kappa", ( 0.0, 0.1, 0.2 ) ), ( "Tau", ( 0.0, 10.0, 30.0 ) ) ):
        c = xf.canvas( fig )
        for k, v in enumerate( vals ):
            P, f, xs = xf.wing_section( vsp.XS_VKT_AIRFOIL, { parm: v } )
            xf.curve( c, P, color = TRIO[k], lw = 1.2 )
            xf.label( c, [ 1.03, 0.07 - 0.035 * k ], "%s %g" % ( parm, v ), color = TRIO[k], ha = "left" )
        frame( c, [ 0, -0.1 ], [ 1.25, 0.14 ] )
        rows.append( [ c ] )
    xf.key_side( rows[ -1 ][ 0 ], at = ( 0.0, 0.0 ), dx = -14, dy = -24 )
    xf.save( fig, "Airfoils_vkt", rows, hmax = 5.5 )


def srs():
    """Shift, rotate and scale."""
    base, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR )
    fig = xf.new()
    rows = []
    for name, p in ( ( "Theta 10", { "Theta": 10.0 } ), ( "Delta X/C 0.1, Delta Y/C 0.05", { "DeltaX": 0.1, "DeltaY": 0.05 } ),
                     ( "Scale 0.7", { "Scale": 0.7 } ) ):
        q = dict( FOUR )
        q.update( p )
        P, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, q )
        c = xf.canvas( fig )
        ghost( c, base )
        xf.curve( c, P )
        xf.dot( c, [ 0, 0 ], color = MARK, ms = 4.0 )
        frame( c, [ -0.02, -0.08 ], [ 1.12, 0.26 ] )
        xf.title( c, name, pad = 2 )
        rows.append( [ c ] )
    xf.key_side( rows[ -1 ][ 0 ], at = ( 0.0, 0.0 ), dx = -14, dy = -20 )
    xf.save( fig, "Airfoils_srs", rows, hmax = 5.5 )


def shiftle():
    """Shift LE moves the curve's leading edge point, not the shape."""
    q = dict( FOUR )
    fig = xf.new()
    c = xf.canvas( fig )
    lo = [ -0.03, -0.07 ]
    hi = [ 0.3, 0.09 ]
    for k, s in enumerate( ( -0.15, 0.0, 0.15 ) ):
        q[ "ShiftLE" ] = s
        P, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, q )
        if k == 1:
            draw( c, P, lo, hi )
        p = xf.at( P, f, 0.5 )
        xf.dot( c, p, color = TRIO[k], ms = 6.0 )
        xf.label( c, p, "Shift LE %g" % s, dx = ( -8, 10, -8 )[k], dy = ( -10, 0, 10 )[k], ha = ( "right", "left", "right" )[k],
                  color = TRIO[k] )
    xf.key_side( c, at = ( 1.0, 0.0 ), dx = -30, dy = 30 )
    frame( c, lo, hi )
    xf.save( fig, "Airfoils_shiftle", [ [ c ] ], hmax = 3.2 )


def blunt_points():
    """Upper and lower points, leading edge to trailing edge, of NACA 2412 cut off where it is 0.03 thick: an airfoil
    with a blunt trailing edge, for a File airfoil."""
    P, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR, groups = BLUNT3 )
    # Leave out the trailing edge face, the first and last thousandth of the curve.
    ok = ( f > 0.0011 ) & ( f < 0.9989 )
    P = P[ ok ]
    f = f[ ok ]
    P = P / P[ :, 0 ].max()
    i = int( np.argmin( np.abs( f - 0.5 ) ) )
    lower = distinct( P[ :i + 1 ][ ::-1 ] )
    upper = distinct( P[ i: ] )
    up = [ vsp.vec3d( p[0], p[1], 0.0 ) for p in upper ]
    lo = [ vsp.vec3d( p[0], p[1], 0.0 ) for p in lower ]
    return up, lo


def distinct( Q ):
    """Q without repeated points."""
    keep = np.r_[ 0, np.nonzero( np.linalg.norm( np.diff( Q, axis = 0 ), axis = 1 ) > 1e-9 )[0] + 1 ]
    return Q[ keep ]


def teclose():
    """Trailing edge closure, close up on the aft part of the airfoil."""
    lo = [ 0.55, -0.05 ]
    hi = [ 1.2, 0.08 ]
    base, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR )
    up, low = blunt_points()

    def as_file( xs ):
        vsp.SetAirfoilPnts( xs, up, low )
    blunt, f, xs = xf.wing_section( vsp.XS_FILE_AIRFOIL, prep = as_file )
    fig = xf.new()
    rows = []
    row = []
    for name, t, thick in ( ( "SKEW LOWER, T 0.03", vsp.CLOSE_SKEWLOW, 0.03 ), ( "SKEW UPPER, T 0.03", vsp.CLOSE_SKEWUP, 0.03 ),
                            ( "SKEW BOTH, T 0.03", vsp.CLOSE_SKEWBOTH, 0.03 ), ( "EXTRAPOLATE, T 0", vsp.CLOSE_EXTRAP, 0.0 ) ):
        g = { "TE_Close_Type": ( "Close", t ), "TE_Close_AbsRel": ( "Close", vsp.REL ),
              "TE_Close_Thick_Chord": ( "Close", thick ) }
        c = xf.canvas( fig )
        if t == vsp.CLOSE_EXTRAP:
            P, f, xs = xf.wing_section( vsp.XS_FILE_AIRFOIL, groups = g, prep = as_file )
            ghost( c, blunt, lo, hi )
        else:
            P, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR, groups = g )
            ghost( c, base, lo, hi )
        draw( c, P, lo, hi )
        frame( c, lo, hi )
        xf.title( c, name, pad = 2 )
        row.append( c )
        if len( row ) == 2:
            rows.append( row )
            row = []
    xf.key_side( rows[ -1 ][ 0 ], at = ( 0.0, 0.0 ), dx = -14, dy = -22 )
    xf.save( fig, "Airfoils_teclose", rows, hmax = 4.6 )


def leclose():
    """Leading edge closure, close up on the forward part."""
    lo = [ -0.03, -0.09 ]
    hi = [ 0.4, 0.1 ]
    base, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR )
    fig = xf.new()
    row = []
    for name, t in ( ( "SKEW LOWER", vsp.CLOSE_SKEWLOW ), ( "SKEW UPPER", vsp.CLOSE_SKEWUP ),
                     ( "SKEW BOTH", vsp.CLOSE_SKEWBOTH ) ):
        P, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR, groups = { "LE_Close_Type": ( "Close", t ),
                                    "LE_Close_AbsRel": ( "Close", vsp.REL ), "LE_Close_Thick_Chord": ( "Close", 0.03 ) } )
        c = xf.canvas( fig )
        ghost( c, base, lo, hi )
        draw( c, P, lo, hi )
        frame( c, lo, hi )
        xf.title( c, name + ", T 0.03", pad = 2 )
        row.append( c )
    xf.key_side( row[0], at = ( 0.0, 0.0 ), dx = -6, dy = -22 )
    xf.save( fig, "Airfoils_leclose", [ row ] )


def trim():
    """Trimming the trailing and leading edges."""
    base, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR )
    cases = ( ( "TE Trim X/C 0.1", { "TE_Trim_Type": vsp.TRIM_X, "TE_Trim_AbsRel": vsp.REL, "TE_Trim_X_Chord": 0.1 },
                [ 0.55, -0.05 ], [ 1.05, 0.08 ] ),
              ( "TE Trim T/C 0.03", { "TE_Trim_Type": vsp.TRIM_THICK, "TE_Trim_AbsRel": vsp.REL,
                                      "TE_Trim_Thick_Chord": 0.03 }, [ 0.55, -0.05 ], [ 1.05, 0.08 ] ),
              ( "LE Trim X/C 0.03", { "LE_Trim_Type": vsp.TRIM_X, "LE_Trim_AbsRel": vsp.REL, "LE_Trim_X_Chord": 0.03 },
                [ -0.03, -0.06 ], [ 0.47, 0.09 ] ),
              ( "LE Trim T/C 0.06", { "LE_Trim_Type": vsp.TRIM_THICK, "LE_Trim_AbsRel": vsp.REL,
                                      "LE_Trim_Thick_Chord": 0.06 }, [ -0.03, -0.06 ], [ 0.47, 0.09 ] ) )
    fig = xf.new()
    rows = []
    row = []
    for name, p, lo, hi in cases:
        P, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR, groups = { k: ( "Trim", v ) for k, v in p.items() } )
        c = xf.canvas( fig )
        ghost( c, base, lo, hi )
        draw( c, P, lo, hi )
        frame( c, lo, hi )
        xf.title( c, name, pad = 2 )
        row.append( c )
        if len( row ) == 2:
            rows.append( row )
            row = []
    xf.key_side( rows[ -1 ][ 0 ], at = ( 0.0, 0.0 ), dx = -14, dy = -22 )
    xf.save( fig, "Airfoils_trim", rows, hmax = 4.6 )




def caps():
    """The trailing edge caps on a trimmed, blunt trailing edge."""
    lo = [ 0.7, -0.05 ]
    hi = [ 1.0, 0.06 ]
    base, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR, groups = BLUNT )
    fig = xf.new()
    rows = []
    row = []
    for name, t in ( ( "FLAT", vsp.FLAT_END_CAP ), ( "ROUND", vsp.ROUND_END_CAP ), ( "EDGE", vsp.EDGE_END_CAP ),
                     ( "SHARP", vsp.SHARP_END_CAP ) ):
        g = dict( BLUNT )
        g[ "TE_Cap_Type" ] = ( "Cap", t )
        P, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR, groups = g )
        c = xf.canvas( fig )
        if t != vsp.FLAT_END_CAP:
            ghost( c, base, lo, hi )
        draw( c, P, lo, hi )
        frame( c, lo, hi )
        xf.title( c, name, pad = 2 )
        row.append( c )
        if len( row ) == 2:
            rows.append( row )
            row = []
    xf.key_side( rows[ -1 ][ 0 ], at = ( 0.0, 0.0 ), dx = -14, dy = -22 )
    xf.save( fig, "Airfoils_caps", rows, hmax = 4.6 )


def capparms():
    """Length, Offset and Strength of a cap."""
    lo = [ 0.74, -0.035 ]
    hi = [ 0.95, 0.04 ]
    cases = ( ( "ROUND, Length", vsp.ROUND_END_CAP, "TE_Cap_Length", ( 0.5, 1.0, 2.0 ) ),
              ( "ROUND, Offset", vsp.ROUND_END_CAP, "TE_Cap_Offset", ( -0.5, 0.0, 0.5 ) ),
              ( "SHARP, Length 2, Strength", vsp.SHARP_END_CAP, "TE_Cap_Strength", ( 0.0, 0.5, 1.0 ) ) )
    fig = xf.new()
    rows = []
    for name, t, parm, vals in cases:
        c = xf.canvas( fig )
        for k, v in enumerate( vals ):
            g = dict( BLUNT )
            g[ "TE_Cap_Type" ] = ( "Cap", t )
            g[ parm ] = ( "Cap", v )
            if t == vsp.SHARP_END_CAP:
                g[ "TE_Cap_Length" ] = ( "Cap", 2.0 )
            P, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR, groups = g )
            draw( c, P, lo, hi, color = TRIO[k], lw = 1.3 )
            xf.label( c, [ 0.915, 0.012 - 0.01 * k ], "%g" % v, color = TRIO[k], ha = "left" )
        frame( c, lo, hi )
        xf.title( c, name, pad = 2 )
        rows.append( [ c ] )
    xf.key_side( rows[ -1 ][ 0 ], at = ( 0.0, 0.0 ), dx = -40, dy = 0 )
    xf.save( fig, "Airfoils_capparms", rows, hmax = 6.0 )


def flap_hinge( base, f, xflap, yfrac ):
    """Where the hinge of a plain flap falls: on each surface, the point xflap from the trailing edge; then yfrac of
    the way from the lower point to the upper (XSecCurve::DeflectTE)."""
    te = xf.at( base, f, 0.0 )
    low = base[ f < 0.5 ]
    upp = base[ f > 0.5 ]
    pl = low[ int( np.argmin( np.abs( np.linalg.norm( low - te, axis = 1 ) - xflap ) ) ) ]
    pu = upp[ int( np.argmin( np.abs( np.linalg.norm( upp - te, axis = 1 ) - xflap ) ) ) ]
    return pl + yfrac * ( pu - pl )


def flap():
    """A plain trailing edge flap."""
    base, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR )
    h = flap_hinge( base, f, 0.25, 0.5 )
    fig = xf.new()
    rows = []
    for d in ( 20.0, -20.0 ):
        g = { "TE_Flap_Type": ( "Flap", vsp.FLAP_PLAIN ), "TE_Flap_AbsRel": ( "Flap", vsp.REL ),
              "TE_Flap_X_Chord": ( "Flap", 0.25 ), "TE_Flap_Y_Frc": ( "Flap", 0.5 ), "TE_Flap_Deflection": ( "Flap", d ) }
        P, f, xs = xf.wing_section( vsp.XS_FOUR_SERIES, FOUR, groups = g )
        c = xf.canvas( fig )
        ghost( c, base )
        xf.curve( c, P )
        xf.dot( c, h, color = MARK, ms = 5.0 )
        xf.label( c, h, "hinge", dx = -8, ha = "right", color = MARK )
        frame( c, [ 0, -0.14 ], [ 1.05, 0.12 ] )
        xf.title( c, "Deflection %+g" % d, pad = 2 )
        rows.append( [ c ] )
    xf.key_side( rows[ -1 ][ 0 ], at = ( 0.0, 0.0 ), dx = -14, dy = -20 )
    xf.save( fig, "Airfoils_flap", rows, hmax = 4.2 )


FIGS = { "param": param, "families": families, "fourmod": four_mod, "wedge": wedge, "vkt": vkt, "srs": srs,
         "shiftle": shiftle, "teclose": teclose, "leclose": leclose, "trim": trim, "caps": caps,
         "capparms": capparms, "flap": flap }

if __name__ == "__main__":
    xf.style()
    names = sys.argv[1:] or list( FIGS )
    for n in names:
        FIGS[ n ]()
