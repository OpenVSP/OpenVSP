"""Figures for the OpenVSP Wireframe help page (src/help/Wire.md).

    <build>/venv/bin/python wire_figs.py      # writes ../../images/Wire_*.png

A small wing grid is written as an XSec (.hrm) file, imported as a wireframe, patched, and read back through an XSec
export of the wireframe, which writes the grid as the Wireframe component has rearranged it.
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )

import numpy as np

import meshfig as mf
from meshfig import fl, vsp

# The colors the 3D view draws a wireframe's feature lines in (WireGeom::UpdateDrawObj): I red, J green.
I_COL = "#E00000"
J_COL = "#00A000"
# The points the J patches add.
C_START = "#C05000"
C_END = "#0060C0"


def wing_grid( nsec = 2, npt = 17, span = 0.5, tte = 0.04 ):
    """A left wing: sections from root (Y = 0) to tip (Y = -span), each from the lower trailing edge round the
    leading edge to the upper one, with a blunt trailing edge of thickness tte times the chord."""
    secs = []
    for i in range( nsec ):
        f = i / ( nsec - 1.0 )
        c = 1.0 - 0.2 * f
        x0 = 0.15 * f
        y = -span * f
        th = np.linspace( 0, np.pi, npt // 2 + 1 )
        xc = 0.5 * ( 1 + np.cos( th ) )               # 1 (TE) to 0 (LE)
        t = 0.18 * 5 * ( 0.2969 * np.sqrt( xc ) - 0.1260 * xc - 0.3516 * xc ** 2 + 0.2843 * xc ** 3 - 0.1015 * xc ** 4 )
        t = t + tte * xc
        lower = [ ( x0 + c * x, y, -c * z ) for x, z in zip( xc, t ) ]
        upper = [ ( x0 + c * x, y, c * z ) for x, z in zip( xc[ ::-1 ][ 1: ], t[ ::-1 ][ 1: ] ) ]
        secs.append( lower + upper )
    return np.array( secs )


def write_hrm( fn, G, name = "Wing", typ = 0 ):
    with open( fn, "w" ) as f:
        f.write( " HERMITE INPUT FILE\n\n NUMBER OF COMPONENTS = 1\n" )
        f.write( "%s\n GROUP NUMBER      = 0\n TYPE              = %d\n" % ( name, typ ) )
        f.write( " CROSS SECTIONS    = %d\n PTS/CROSS SECTION = %d\n" % ( G.shape[0], G.shape[1] ) )
        for row in G:
            for p in row:
                f.write( "%.12f %.12f %.12f\n" % tuple( p ) )


def read_hrm( fn ):
    """The first component's grid of an XSec file."""
    with open( fn ) as f:
        lines = [ l.strip() for l in f ]
    k = next( i for i, l in enumerate( lines ) if l.startswith( "CROSS SECTIONS" ) )
    nc = int( lines[ k ].split( "=" )[ 1 ] )
    npt = int( lines[ k + 1 ].split( "=" )[ 1 ] )
    vals = [ [ float( v ) for v in l.split() ] for l in lines[ k + 2:k + 2 + nc * npt ] ]
    return np.array( vals ).reshape( nc, npt, 3 )


def imported_wire( G ):
    vsp.VSPRenew()
    fn = os.path.join( mf._TMP, "w.hrm" )
    write_hrm( fn, G )
    vsp.ImportFile( fn, vsp.IMPORT_XSEC_WIRE, "" )
    w = [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == "WireFrame" ][ 0 ]
    return w


def wire_grid( w ):
    s = vsp.SET_FIRST_USER
    for g in vsp.FindGeoms():
        vsp.SetSetFlag( g, s, g == w )
    vsp.Update()
    fn = os.path.join( mf._TMP, "out.hrm" )
    vsp.ExportFile( fn, s, vsp.EXPORT_XSEC )
    return read_hrm( fn )


def draw_grid( c, G, lw = 0.45 ):
    """The grid's quads, shaded, and every grid line."""
    c.surface( G, edge = False )
    for i in range( G.shape[ 0 ] ):
        c.segs( G[ i ], "#3a3a3a", lw, bias = 0.02 )
    for j in range( G.shape[ 1 ] ):
        c.segs( G[ :, j ], "#3a3a3a", lw, bias = 0.02 )


def fig_patch():
    G0 = wing_grid()
    specs = [ ( "None", "PATCH_NONE" ), ( "Point", "PATCH_POINT" ), ( "Line", "PATCH_LINE" ) ]
    import matplotlib.pyplot as plt
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    cans = []
    for k, ( label, ptype ) in enumerate( specs ):
        w = imported_wire( G0 )
        vsp.SetParmVal( w, "IEndPatchType", "WireFrame", getattr( vsp, ptype ) )
        vsp.Update()
        G = wire_grid( w )
        c = mf.panel( fig, mf.ISO )
        draw_grid( c, G )
        if k == 0:
            # I and J from the first point, in the colors of the feature lines.
            p0 = G[ 0, 0 ]
            c.arrow( p0, G[ 1, 0 ], color = I_COL, lw = 1.6, z = 66 )
            c.text( G[ 1, 0 ], "I", color = I_COL, dx = 6, dy = 4, ha = "left" )
            c.arrow( p0, G[ 0, 3 ], color = J_COL, lw = 1.6, z = 66 )
            c.text( G[ 0, 3 ], "J", color = J_COL, dx = 4, dy = -8, ha = "left" )
        c.flush()
        P = c.v.p2( G.reshape( -1, 3 ) )
        lo = P.min( 0 )
        hi = P.max( 0 )
        c.ax.annotate( "I End " + label, xy = ( 0.5 * ( lo[ 0 ] + hi[ 0 ] ), lo[ 1 ] ), xytext = ( 0, -8 ),
                       textcoords = "offset points", ha = "center", va = "top", fontsize = fl.FS )
        cans.append( c )
    mf.save( fig, "Wire_patch", [ cans ], hmax = 3.2 )


def runs( S, keep ):
    """The pieces of polyline S whose points are all kept."""
    out = []
    cur = []
    for p, k in zip( S, keep ):
        if k:
            cur.append( p )
        elif cur:
            out.append( np.array( cur ) )
            cur = []
    if cur:
        out.append( np.array( cur ) )
    return out


def fig_blunt_te():
    """A blunt trailing edge closed by J patches: none, Halfway Opposite with Copy Opposite, and Halfway at both."""
    G0 = wing_grid()
    n0 = G0.shape[ 1 ]
    specs = [ ( "PATCH_NONE", "PATCH_NONE", "J Start None, J End None" ),
              ( "PATCH_HALFWAY", "PATCH_COPY", "J Start Halfway Opposite, J End Copy Opposite" ),
              ( "PATCH_HALFWAY", "PATCH_HALFWAY", "J Start Halfway Opposite, J End Halfway Opposite" ) ]
    import matplotlib.pyplot as plt
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    cans = []
    for js, je, label in specs:
        w = imported_wire( G0 )
        vsp.SetParmVal( w, "JStartPatchType", "WireFrame", getattr( vsp, js ) )
        vsp.SetParmVal( w, "JEndPatchType", "WireFrame", getattr( vsp, je ) )
        vsp.Update()
        G = wire_grid( w )
        S = G[ 0 ]
        lower, upper = G0[ 0, 0 ], G0[ 0, -1 ]
        mid = 0.5 * ( lower + upper )
        if js == "PATCH_NONE":
            assert G.shape[ 1 ] == n0 and np.allclose( S[ [ 0, -1 ] ], [ lower, upper ] )
        else:
            # Each patch adds a point to every section; J Start's is on the trailing edge's midline.
            assert G.shape[ 1 ] == n0 + 2 and np.allclose( S[ 0 ], mid ) and np.allclose( S[ 1 ], lower )
            if je == "PATCH_COPY":
                assert np.allclose( S[ -1 ], S[ 0 ] )
            else:
                # The second midpoint is taken to the first: three quarters of the way up the trailing edge.
                assert np.allclose( S[ -1 ], 0.5 * ( upper + mid ) )
        c = mf.panel( fig, mf.FRONT )
        keep = np.ones( len( S ), bool )
        for R in runs( S, keep ):
            c.line( R, color = "#3a3a3a", lw = 1.0, z = 50 )
        Q = S[ keep ]
        q = c.v.p2( Q )
        c.ax.plot( q[ :, 0 ], q[ :, 1 ], ls = "none", marker = "o", ms = 3.2, color = "#3a3a3a", zorder = 60 )
        c.pts.append( q )
        # J runs from the row's first point.
        L = min( 0.12, np.linalg.norm( S[ 1 ] - S[ 0 ] ) )
        e = S[ 0 ] + L * fl.unit( S[ 1 ] - S[ 0 ] )
        c.arrow( S[ 0 ], e, color = J_COL, lw = 1.4, z = 66, head = 6.0 )
        c.text( e, "J", color = J_COL, dx = -8, dy = -6 )
        if js == "PATCH_NONE":
            c.text( mid, "open", dx = 10, ha = "left" )
            c.text( S[ 0 ], "first point", dx = 10, dy = -4, ha = "left" )
            c.text( S[ -1 ], "last point", dx = 10, dy = 4, ha = "left" )
        else:
            c.ax.plot( *c.v.p2( S[ 0 ] ), marker = "o", ms = 7, mfc = "none", mec = C_START, mew = 1.3, zorder = 70 )
            c.text( S[ 0 ], "J Start point", color = C_START, dx = 12, dy = -6, ha = "left" )
            c.ax.plot( *c.v.p2( S[ -1 ] ), marker = "s", ms = 9, mfc = "none", mec = C_END, mew = 1.3, zorder = 69 )
            if je == "PATCH_COPY":
                c.text( S[ -1 ], "J End point", color = C_END, dx = 12, dy = 8, ha = "left" )
            else:
                c.text( S[ -1 ], "J End point", color = C_END, dx = 12, dy = 4, ha = "left" )
                c.text( 0.5 * ( S[ 0 ] + S[ -1 ] ), "open", dx = -10, ha = "right" )
        lo, hi = c.limits()
        c.ax.annotate( label, xy = ( 0.5 * ( lo[ 0 ] + hi[ 0 ] ), lo[ 1 ] ), xytext = ( 0, -10 ), textcoords = "offset points",
                       ha = "center", va = "top", fontsize = fl.FS )
        cans.append( c )
    box = ( np.min( [ c.limits()[ 0 ] for c in cans ], 0 ), np.max( [ c.limits()[ 1 ] for c in cans ], 0 ) )
    for c in cans:
        c.bounds = box
    mf.key( cans[ -1 ], "front", at = ( 1.0, 0.0 ), dx = 60, dy = 4 )
    mf.save( fig, "Wire_blunt_te", [ [ c ] for c in cans ], hmax = 7.0 )


FIGS = { "patch": fig_patch, "blunt_te": fig_blunt_te }

if __name__ == "__main__":
    mf.style()
    for n in ( sys.argv[ 1: ] or FIGS.keys() ):
        FIGS[ n ]()
