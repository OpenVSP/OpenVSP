"""Figures for the OpenVSP CFD Mesh help page (src/help/CFDMesh.md).

    <build>/venv/bin/python cfdmesh_figs.py            # writes ../../images/CFDMesh_*.png
    <build>/venv/bin/python cfdmesh_figs.py wake       # or only some figures (see FIGS at the end)

Every mesh drawn is one CFD Mesh wrote: the model is meshed through the openvsp module, the Cart3D .tri file is read
back, and its triangles are drawn as they are, edges and all.  The two charts draw the target length rules of the
mesher, with the edge lengths of meshes VSP made marked on them.
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )
sys.path.insert( 0, HERE )

import numpy as np
import matplotlib
matplotlib.use( "Agg" )
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection, PolyCollection
from matplotlib.colors import to_rgb

import meshfig as mf
from meshfig import fl, vsp
import cfdmodel as cm

OUT = mf.OUT
DPI = mf.DPI

# The colors the Display tab's reason key gives each rule (DrawObj::reasonColorMap).
REASON = {
    "max": "#FF6347",       # Max Edge Len, tomato
    "gap": "#32CD32",       # Max Gap, lime green
    "ncirc": "#1E90FF",     # Num Circle Segments, dodger blue
    "source": "#FF00FF",    # Sources, magenta
    "min_ncirc": "#00008B",  # Min Edge Len over Num Circle Segments, dark blue
}

# One color for each kind of surface in the mesh pictures.
SURF = "#C8CCD2"
SURF2 = "#E3D3C3"
SYM = "#B9D3E8"
WAKE = "#F2E2A0"
EDGE = "#3a3a3a"


def draw_mesh( c, V, T, color = SURF, edge = EDGE, lw = 0.15, z0 = 1.0, colors = None ):
    """Triangles of a mesh, painted far to near and lit, each with its edges.  colors gives one color per triangle."""
    P3 = V[ T ]
    n = np.cross( P3[ :, 1 ] - P3[ :, 0 ], P3[ :, 2 ] - P3[ :, 0 ] )
    nn = np.linalg.norm( n, axis = 1 )
    ok = nn > 1e-300
    P3 = P3[ ok ]
    n = n[ ok ] / nn[ ok, None ]
    if colors is None:
        base = np.tile( np.array( to_rgb( color ) ), ( len( P3 ), 1 ) )
    else:
        base = np.array( [ to_rgb( k ) for k in np.asarray( colors )[ ok ] ] )
    lam = np.abs( n @ c.light )
    cols = np.clip( base * ( 0.62 + 0.38 * lam[ :, None ] ) + 0.08 * lam[ :, None ] ** 8, 0, 1 )
    d = c.v.depth( P3.reshape( -1, 3 ) ).reshape( -1, 3 ).mean( 1 )
    P = c.v.p2( P3.reshape( -1, 3 ) ).reshape( -1, 3, 2 )
    c.pts.append( P.reshape( -1, 2 ) )
    o = np.argsort( d )
    pc = PolyCollection( P[ o ], facecolors = cols[ o ], edgecolors = edge, linewidths = lw, zorder = z0 )
    c.ax.add_collection( pc )


def tags_of( tag, names ):
    """The tags of a .tri in order, paired with the surface names given for them."""
    return dict( zip( np.unique( tag ), names ) )


def new( view, h = 4.0 ):
    fig = plt.figure( figsize = ( fl.PAGE_W, h ) )
    return fig, mf.panel( fig, view )


# ---------------------------------------------------------------- target length from curvature


def curvature_len( r, gap, ncirc ):
    """The two curvature rules of Surf::TargetLen, as lengths, at radius of curvature r."""
    glen = np.where( r > gap, 2.0 * np.sqrt( np.maximum( 2.0 * r * gap - gap * gap, 0.0 ) ), 2.0 * gap )
    nlen = r * 2.0 * np.sin( np.pi / ncirc )
    return glen, nlen


def sphere_median( r ):
    """The median edge length of a sphere of radius r meshed at the default settings."""
    vsp.VSPRenew()
    g = vsp.AddGeom( "ELLIPSOID" )
    for n in [ "A_Radius", "B_Radius", "C_Radius" ]:
        vsp.SetParmVal( g, n, "Design", r )
    vsp.Update()
    V, T, tag = cm.mesh( "sphere" )
    E, L = cm.edges( V, T )
    return np.median( L )


SPHERES = [ float( '%.3g' % r ) for r in np.geomspace( 0.03, 40.0, 25 ) ]


def fig_curvature():
    gap, ncirc, lmin, lmax = 0.005, 16.0, 0.025, 0.5
    r = np.logspace( -2, 2, 400 )
    glen, nlen = curvature_len( r, gap, ncirc )
    t = np.clip( np.minimum( glen, nlen ), lmin, lmax )

    fig = plt.figure( figsize = ( fl.PAGE_W, 3.6 ) )
    ax = fig.add_axes( [ 0.09, 0.13, 0.88, 0.83 ] )
    ax.loglog( r, glen, color = REASON[ "gap" ], lw = 1.0, ls = "--" )
    ax.loglog( r, nlen, color = REASON[ "ncirc" ], lw = 1.0, ls = "--" )
    ax.axhline( lmin, color = "0.4", lw = 0.8, ls = ":" )
    ax.axhline( lmax, color = "0.4", lw = 0.8, ls = ":" )

    # The target, drawn in the color of the rule that sets it.
    seg = np.where( t <= lmin, 0, np.where( t >= lmax, 3, np.where( nlen < glen, 1, 2 ) ) )
    cols = [ REASON[ "min_ncirc" ], REASON[ "ncirc" ], REASON[ "gap" ], REASON[ "max" ] ]
    for k in range( 4 ):
        m = seg == k
        idx = np.where( m )[ 0 ]
        if len( idx ):
            lo = max( idx[ 0 ] - 1, 0 )
            hi = min( idx[ -1 ] + 2, len( r ) )
            ax.loglog( r[ lo:hi ], t[ lo:hi ], color = cols[ k ], lw = 3.0, solid_capstyle = "butt" )

    meas = [ sphere_median( s ) for s in SPHERES ]
    ax.loglog( SPHERES, meas, "o", ms = 4.5, mfc = "white", mec = "k", mew = 0.8, zorder = 10,
               label = "Median edge length of a sphere meshed by CFD Mesh" )

    ax.set_xlim( 0.01, 100 )
    ax.set_ylim( 0.004, 2.0 )
    ax.set_xlabel( "Radius of curvature" )
    ax.set_ylabel( "Target edge length" )
    ax.text( 0.012, lmax * 1.12, "Max Edge Len", color = "0.3", va = "bottom" )
    ax.text( 95.0, lmin * 1.12, "Min Edge Len", color = "0.3", ha = "right", va = "bottom" )
    ax.text( 30.0, 0.85, "Max Gap", color = REASON[ "gap" ], ha = "left", va = "bottom" )
    ax.text( 1.0, 0.8, "Num Circle Segments", color = REASON[ "ncirc" ], ha = "right", va = "center" )
    ax.legend( loc = "lower right", frameon = False, bbox_to_anchor = ( 1.0, 0.08 ) )
    for s in ax.spines.values():
        s.set_linewidth( 0.6 )
    ax.tick_params( width = 0.6 )
    fig.savefig( os.path.join( OUT, "CFDMesh_curvature.png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made CFDMesh_curvature" )


# ---------------------------------------------------------------- a point source


SRC_R = 10.0
SRC_LEN = 0.05
SRC_RAD = 4.0
SRC_BASE = 1.0


def source_sphere( growth = 1.3 ):
    """A sphere of radius SRC_R with one point source; Max Gap opened so that Max Edge Len sets the rest."""
    vsp.VSPRenew()
    g = vsp.AddGeom( "ELLIPSOID" )
    for n in [ "A_Radius", "B_Radius", "C_Radius" ]:
        vsp.SetParmVal( g, n, "Design", SRC_R )
    vsp.Update()
    cm.coarse( maxlen = SRC_BASE, minlen = 0.1, gap = 0.1, growth = growth )
    vsp.AddCFDSource( vsp.POINT_SOURCE, g, 0, SRC_LEN, SRC_RAD, 0.5, 0.25 )
    c = np.array( list( vsp.CompPnt01( g, 0, 0.5, 0.25 ) ) )
    return g, c


def source_profile( growth = 1.3, bins = np.arange( 0.0, 7.0, 0.25 ) ):
    """The median edge length in bands of distance from the source, measured on VSP's mesh."""
    g, c = source_sphere( growth )
    V, T, tag = cm.mesh( "source" )
    E, L = cm.edges( V, T )
    d = np.linalg.norm( 0.5 * ( V[ E[ :, 0 ] ] + V[ E[ :, 1 ] ] ) - c, axis = 1 )
    x = []
    y = []
    for lo in bins:
        m = ( d >= lo ) & ( d < lo + bins[ 1 ] - bins[ 0 ] )
        if m.sum() > 3:
            x.append( lo + 0.5 * ( bins[ 1 ] - bins[ 0 ] ) )
            y.append( np.median( L[ m ] ) )
    return np.array( x ), np.array( y )


def point_source_len( d, base = SRC_BASE ):
    """PointSimpleSource::GetTargetLen."""
    f = np.minimum( ( d / SRC_RAD ) ** 2, 1.0 )
    return np.where( d < SRC_RAD, SRC_LEN + f * ( base - SRC_LEN ), base )


# The growth limit from a source, in its Reason color (DrawObj::VIOLET, 238 130 238).  It is close to the source's
# magenta (255 0 255), so the two lines are told apart by style and weight as well, and labeled.
GROW_LIMIT = "#EE82EE"


def limited_len( d, t, growth = 1.3 ):
    """The target t( d ) after the growth limit, as Surf::WalkMap applies it: every point lowers every other to its own
    target plus ( growth - 1 ) times the distance between them, so the result is the lower envelope of those lines."""
    g = growth - 1.0
    return np.min( t[ None, : ] + g * np.abs( d[ :, None ] - d[ None, : ] ), axis = 1 )


def fig_point_source():
    d = np.linspace( 0, 7, 1401 )
    law = point_source_len( d )
    lim = limited_len( d, law )
    x, y = source_profile()

    fig = plt.figure( figsize = ( fl.PAGE_W, 3.2 ) )
    ax = fig.add_axes( [ 0.11, 0.15, 0.86, 0.81 ] )
    src = np.abs( lim - law ) < 1e-9
    held = ( ~src ) & ( lim < SRC_BASE - 1e-9 )
    top = lim >= SRC_BASE - 1e-9
    # The source's own law, thin, where the limit takes over from it.
    ax.plot( d[ ~src & ( d < SRC_RAD ) ], law[ ~src & ( d < SRC_RAD ) ], color = REASON[ "source" ], lw = 0.8,
             ls = ":" )
    for m, col, ls in ( ( src & ( d < SRC_RAD ), REASON[ "source" ], "-" ), ( held, GROW_LIMIT, ( 0, ( 4, 1.5 ) ) ),
                        ( top, REASON[ "max" ], "-" ) ):
        idx = np.where( m )[ 0 ]
        ax.plot( d[ idx ], lim[ idx ], color = col, lw = 3.0, ls = ls, solid_capstyle = "butt", dash_capstyle = "butt" )
    ax.plot( x, y, "o", ms = 4.5, mfc = "white", mec = "k", mew = 0.8, zorder = 10,
             label = "Median edge length of CFD Mesh's mesh" )
    ax.axvline( SRC_RAD, color = "0.4", lw = 0.8, ls = ":" )
    ax.axhline( SRC_BASE, color = "0.4", lw = 0.8, ls = ":" )
    ax.set_xlim( 0, 7 )
    ax.set_ylim( 0, 1.2 )
    ax.set_xlabel( "Distance from the source" )
    ax.set_ylabel( "Target edge length" )
    ax.text( SRC_RAD + 0.08, 1.12, "Rad", color = "0.3", ha = "left", va = "center" )
    ax.text( 6.9, SRC_BASE + 0.04, "Max Edge Len", color = "0.3", ha = "right", va = "bottom" )
    ax.text( 0.3, 0.015, "Len", color = "0.3", ha = "left", va = "center" )
    ax.text( 1.6, point_source_len( 1.6 ) + 0.06, "source", color = REASON[ "source" ], ha = "right", va = "bottom" )
    ax.text( 3.5, 0.86, "source alone", color = REASON[ "source" ], ha = "right", va = "bottom" )
    ax.text( 4.45, 0.62, "Growth Ratio 1.3\nlimit", color = GROW_LIMIT, ha = "left", va = "top" )
    ax.legend( loc = "lower right", frameon = False )
    for s in ax.spines.values():
        s.set_linewidth( 0.6 )
    ax.tick_params( width = 0.6 )
    fig.savefig( os.path.join( OUT, "CFDMesh_point_source.png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made CFDMesh_point_source" )


# ---------------------------------------------------------------- a wing's default sources


def wing_alone():
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0.0 )
    vsp.SetParmVal( w, "Span", "XSec_1", 5.0 )
    vsp.SetParmVal( w, "Root_Chord", "XSec_1", 2.0 )
    vsp.SetParmVal( w, "Tip_Chord", "XSec_1", 1.0 )
    vsp.SetParmVal( w, "Sweep", "XSec_1", 10.0 )
    vsp.Update()
    return w


# ---------------------------------------------------------------- the width limit


TE_THICK = 0.02


def blunt_wing():
    """A thin wing with a blunt trailing edge TE_THICK thick."""
    w = wing_alone()
    xss = vsp.GetXSecSurf( w, 0 )
    for i in range( vsp.GetNumXSec( xss ) ):
        x = vsp.GetXSec( xss, i )
        vsp.SetParmVal( vsp.GetXSecParm( x, "ThickChord" ), 0.06 )
        vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_Type" ), vsp.CLOSE_SKEWBOTH )
        vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_AbsRel" ), vsp.ABS )
        vsp.SetParmVal( vsp.GetXSecParm( x, "TE_Close_Thick" ), TE_THICK )
    vsp.Update()
    return w


TE_VIEW = fl.View.plane( [ 0, 1, 0 ], [ 0, 0, 1 ] )   # from behind, looking along -X
mf.KEY[ "behind" ] = ( r"$Y$", r"$Z$", r"$X$" )

# The window shown: a stretch of the trailing edge face, and the skins' edges beside it.
TE_Y = ( 2.2, 2.6 )
TE_Z = 0.035


def te_x( y ):
    """X of the trailing edge of wing_alone at span station y."""
    return 2.0 - ( 0.2 - np.tan( np.radians( 10.0 ) ) ) * y


def save_clipped( fig, c, name, hmax ):
    """Lay out one canvas framed by its bounds, and cut what it draws at them."""
    fl.fit( fig, [ [ c ] ], hmax = hmax )
    for a in c.ax.collections:
        a.set_clip_on( True )
        a.set_zorder( 0 )
    fig.savefig( os.path.join( OUT, name + ".png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made", name )


def save_clipped_rows( fig, rows, name, hmax ):
    """Lay out rows of canvases framed by their bounds, and cut what each draws at them."""
    fl.fit( fig, rows, hmax = hmax )
    for r in rows:
        for c in r:
            for a in c.ax.collections + c.ax.lines:
                a.set_clip_on( True )
    fig.savefig( os.path.join( OUT, name + ".png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made", name )


def label( c, s, at = ( 0.0, 1.0 ), dx = 4, dy = -4 ):
    """A panel's name in its top left corner."""
    lo, hi = c.limits()
    q = lo + np.asarray( at, float ) * ( hi - lo )
    c.ax.annotate( s, xy = q, xycoords = "data", xytext = ( dx, dy ), textcoords = "offset points", ha = "left",
                   va = "top", bbox = dict( boxstyle = "square,pad=0.2", fc = "white", ec = "none" ), zorder = 95 )


# ---------------------------------------------------------------- the precedence of the rules


# The Reason key's colors (DrawObj::reasonColorMap), by rule.
KEY = {
    "max": "#FF6347", "gap": "#32CD32", "ncirc": "#1E90FF", "width": "#FF8C00", "source": "#FF00FF",
    "g_gap": "#90EE90", "g_ncirc": "#87CEFA", "g_width": "#FFD700", "g_source": "#EE82EE",
    "m_gap": "#008000", "m_ncirc": "#00008B", "m_width": "#A52A2A",
}


def fig_precedence():
    """The sizing rules as a stack, laid out as the Display tab's reason key: Max Edge Len a cap over every rule, the
    rules side by side, Min Edge Len a floor under the curvature and width rules only, and the growth ratio over all."""
    from matplotlib.patches import FancyBboxPatch
    fig = plt.figure( figsize = ( fl.PAGE_W, 2.55 ) )
    ax = fig.add_axes( [ 0.0, 0.0, 1.0, 1.0 ] )
    ax.set_xlim( 0, 7.0 )
    ax.set_ylim( 0.4, 3.15 )
    ax.set_axis_off()
    x0, w, gap = 0.75, 1.05, 0.06

    def box( x, y, ww, h, color, text, dark = False ):
        ax.add_patch( FancyBboxPatch( ( x, y ), ww, h, boxstyle = "round,pad=0,rounding_size=0.05", fc = color,
                                      ec = "0.3", lw = 0.6 ) )
        col = "k"
        if dark:
            col = "white"
        ax.text( x + 0.5 * ww, y + 0.5 * h, text, ha = "center", va = "center", color = col )

    cols = [ ( "Max Gap", "gap", "g_gap", "m_gap" ), ( "Num Circle\nSegments", "ncirc", "g_ncirc", "m_ncirc" ),
             ( "Surface Width", "width", "g_width", "m_width" ), ( "Sources", "source", "g_source", None ) ]
    full = 4 * w + 3 * gap
    box( x0, 2.5, full, 0.42, KEY[ "max" ], "Max Edge Len" )
    for k, ( name, rule, grow, floor ) in enumerate( cols ):
        x = x0 + k * ( w + gap )
        box( x, 1.92, w, 0.42, KEY[ grow ], "Growth Ratio" )
        box( x, 1.2, w, 0.56, KEY[ rule ], name )
        if floor:
            box( x, 0.62, w, 0.42, KEY[ floor ], "Min Edge Len", dark = True )
    notes = [ ( 2.71, "a cap: nothing is longer" ), ( 2.13, "evens the targets out" ),
              ( 1.48, "the smallest is taken" ), ( 0.83, "a floor, not for sources" ) ]
    for y, t in notes:
        ax.text( x0 + full + 0.12, y, t, ha = "left", va = "center", color = "0.25" )
    ax.annotate( "", xy = ( x0 - 0.25, 0.68 ), xytext = ( x0 - 0.25, 2.92 ),
                 arrowprops = dict( arrowstyle = "<->", lw = 0.8, color = "0.3", shrinkA = 0, shrinkB = 0 ) )
    ax.text( x0 - 0.25, 2.98, "longer", ha = "center", va = "bottom", color = "0.25" )
    ax.text( x0 - 0.25, 0.58, "shorter", ha = "center", va = "top", color = "0.25" )
    fig.savefig( os.path.join( OUT, "CFDMesh_precedence.png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made CFDMesh_precedence" )


def fig_width():
    for name, on in [ ( "CFDMesh_width_off", 0.0 ), ( "CFDMesh_width_on", 1.0 ) ]:
        blunt_wing()
        cm.coarse()
        vsp.SetParmVal( cm.density( "WidthLimit" ), on )
        vsp.Update()
        V, T, tag = cm.mesh( "blunt" )
        m = V[ T ].mean( 1 )
        near = ( m[ :, 0 ] > te_x( m[ :, 1 ] ) - 0.6 ) & ( m[ :, 1 ] > TE_Y[ 0 ] - 0.6 ) & \
               ( m[ :, 1 ] < TE_Y[ 1 ] + 0.6 )
        Tn = T[ near ]
        P = V[ Tn ]
        n = np.cross( P[ :, 1 ] - P[ :, 0 ], P[ :, 2 ] - P[ :, 0 ] )
        face = np.abs( n[ :, 0 ] ) > 0.9 * np.linalg.norm( n, axis = 1 )
        fig, c = new( TE_VIEW, 2.0 )
        draw_mesh( c, V, Tn, lw = 0.4, colors = np.where( face, SURF2, SURF ) )
        c.bounds = ( np.array( [ TE_Y[ 0 ], -TE_Z ] ), np.array( [ TE_Y[ 1 ], TE_Z ] ) )
        mf.key( c, "behind", at = ( 1.0, 0.0 ), dx = -40, dy = 4 )
        save_clipped( fig, c, name, 2.2 )


# ---------------------------------------------------------------- the two curvature rules


def fig_rules():
    """Max Gap and Num Circle Segments drawn on a circle."""
    fig = plt.figure( figsize = ( fl.PAGE_W, 3.0 ) )
    th = np.linspace( 0, 2 * np.pi, 400 )
    # Max Gap: one chord of a circle and its gap.
    ax = fig.add_axes( [ 0.02, 0.04, 0.46, 0.92 ] )
    r = 1.0
    a = np.radians( 35.0 )
    ax.plot( r * np.cos( th ), r * np.sin( th ), color = "0.55", lw = 0.8 )
    arc = np.linspace( np.pi / 2 - a, np.pi / 2 + a, 60 )
    ax.plot( r * np.cos( arc ), r * np.sin( arc ), color = "k", lw = 1.6 )
    x0, y0 = r * np.sin( a ), r * np.cos( a )
    ax.plot( [ -x0, x0 ], [ y0, y0 ], color = REASON[ "gap" ], lw = 2.0 )
    ax.annotate( "", xy = ( 0, r ), xytext = ( 0, y0 ), arrowprops = dict( arrowstyle = "<->", lw = 0.8,
                                                                           shrinkA = 0, shrinkB = 0 ) )
    ax.text( 0.04, 0.5 * ( r + y0 ), "$g$", ha = "left", va = "center" )
    ax.text( 0.0, y0 - 0.06, "$t_g$", ha = "center", va = "top", color = REASON[ "gap" ] )
    ax.plot( [ 0, 0 ], [ 0, y0 - 0.2 ], color = "0.4", lw = 0.6, ls = ":" )
    ax.plot( [ 0, -x0 ], [ 0, y0 ], color = "0.4", lw = 0.6, ls = ":" )
    ax.plot( [ 0 ], [ 0 ], "o", ms = 3, color = "k" )
    ax.text( -0.5 * x0 - 0.06, 0.5 * y0, "$r$", ha = "right", va = "center" )
    ax.text( 0.0, -1.18, "Max Gap", ha = "center", va = "top", color = REASON[ "gap" ] )
    # Num Circle Segments: the circle as N equal chords.
    ax2 = fig.add_axes( [ 0.52, 0.04, 0.46, 0.92 ] )
    N = 8
    ax2.plot( r * np.cos( th ), r * np.sin( th ), color = "0.55", lw = 0.8 )
    k = np.arange( N + 1 ) * 2 * np.pi / N + np.pi / 2 - np.pi / N
    ax2.plot( r * np.cos( k ), r * np.sin( k ), color = REASON[ "ncirc" ], lw = 2.0, marker = "o", ms = 3 )
    ax2.plot( [ 0, r * np.cos( k[ 0 ] ) ], [ 0, r * np.sin( k[ 0 ] ) ], color = "0.4", lw = 0.6, ls = ":" )
    ax2.plot( [ 0, r * np.cos( k[ 1 ] ) ], [ 0, r * np.sin( k[ 1 ] ) ], color = "0.4", lw = 0.6, ls = ":" )
    ax2.plot( [ 0 ], [ 0 ], "o", ms = 3, color = "k" )
    ax2.text( 0.0, 1.0, "$t_N$", ha = "center", va = "bottom", color = REASON[ "ncirc" ] )
    ax2.text( 0.0, 0.42, r"$2\pi / N$", ha = "center", va = "center" )
    ax2.text( 0.0, -1.18, "Num Circle Segments, $N = 8$", ha = "center", va = "top", color = REASON[ "ncirc" ] )
    for a_ in ( ax, ax2 ):
        a_.set_aspect( "equal" )
        a_.set_xlim( -1.35, 1.35 )
        a_.set_ylim( -1.45, 1.25 )
        a_.set_axis_off()
    fig.savefig( os.path.join( OUT, "CFDMesh_rules.png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made CFDMesh_rules" )


GROWTHS = ( 1.1, 1.3, 2.0 )


# ---------------------------------------------------------------- the source types


def plate_wing():
    """A thin rectangular wing, wide enough for three sources side by side on its upper surface."""
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0.0 )
    xss = vsp.GetXSecSurf( w, 0 )
    vsp.InsertXSec( w, 1, vsp.XS_FOUR_SERIES )
    vsp.InsertXSec( w, 1, vsp.XS_FOUR_SERIES )
    vsp.Update()
    for k in ( 1, 2, 3 ):
        for n, v in ( ( "Span", PLATE_SECT ), ( "Root_Chord", 4.0 ), ( "Tip_Chord", 4.0 ), ( "Sweep", 0.0 ) ):
            vsp.SetParmVal( w, n, "XSec_%d" % k, v )
    for i in range( vsp.GetNumXSec( xss ) ):
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xss, i ), "ThickChord" ), 0.04 )
    vsp.Update()
    cm.coarse( maxlen = 0.5, minlen = 0.025 )
    vsp.DeleteAllCFDSources()
    return w


PLATE_SECT = 4.0


def plate_u( y ):
    """U of span station y on plate_wing: the root cap, the three sections and the tip cap take a fifth each."""
    return ( 1.0 + y / PLATE_SECT ) / 5.0


def source_types_model():
    """plate_wing with a Point, a Line and a Box source on its upper surface, each of Len 0.04."""
    w = plate_wing()
    u = plate_u
    vsp.AddCFDSource( vsp.POINT_SOURCE, w, 0, 0.04, 1.0, u( 2.0 ), 0.75 )
    vsp.AddCFDSource( vsp.LINE_SOURCE, w, 0, 0.04, 1.0, u( 5.0 ), 0.6, 0.04, 1.0, u( 7.0 ), 0.9 )
    vsp.AddCFDSource( vsp.BOX_SOURCE, w, 0, 0.04, 0.6, u( 9.0 ), 0.65, 0.0, 0.0, u( 11.0 ), 0.85 )
    vsp.Update()
    return w


# ---------------------------------------------------------------- the symmetry plane


SYM_SIZE = 50.0     # the far field box's sides, as multiples of the aircraft's bounding box diagonal
SYM_FAR_LEN = 50.0  # the far field's Max Edge Len
MODEL_LO3 = np.array( [ 0.0, -5.0, -1.5 ] )     # the aircraft's bounding box
MODEL_HI3 = np.array( [ 10.0, 5.0, 1.5 ] )


def half_far( split ):
    """The aircraft's half mesh in a far field box whose sides are all SYM_SIZE times its bounding box diagonal."""
    cm.aircraft()
    cm.coarse()
    diag = np.linalg.norm( MODEL_HI3 - MODEL_LO3 )
    vsp.SetCFDMeshVal( vsp.CFD_HALF_MESH_FLAG, 1.0 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_FIELD_FLAG, 1.0 )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_SIZE_ABS_FLAG, 1.0 )
    for k in ( vsp.CFD_FAR_LENGTH, vsp.CFD_FAR_WIDTH, vsp.CFD_FAR_HEIGHT ):
        vsp.SetCFDMeshVal( k, SYM_SIZE * diag )
    vsp.SetCFDMeshVal( vsp.CFD_FAR_MAX_EDGE_LEN, SYM_FAR_LEN )
    vsp.SetParmVal( cm.setting( "SymmetrySplitting", "FarField" ), split )
    vsp.Update()
    return cm.mesh( "half" )


SYM_VIEW = fl.View.plane( [ 1, 0, 0 ], [ 0, 0, 1 ] )
MODEL_LO = np.array( [ 0.0, -1.5 ] )     # the aircraft's bounding box in X and Z
MODEL_HI = np.array( [ 10.0, 1.5 ] )
NMAP = 100                               # intervals of the background map of a symmetry plane piece


def sym_pieces( split, lo, hi ):
    """The corners of each piece of the symmetry plane, in X and Z, as CfdMeshMgr::CreateDomainSurfs makes them:
    ( p0, p1, p2, p3 ) with p0-p1 one side and p2-p3 the side across from it."""
    if not split:
        return [ ( ( hi[ 0 ], lo[ 1 ] ), ( lo[ 0 ], lo[ 1 ] ), ( hi[ 0 ], hi[ 1 ] ), ( lo[ 0 ], hi[ 1 ] ) ) ]
    c = 0.5 * ( MODEL_LO + MODEL_HI )
    ilo = c - ( MODEL_HI - MODEL_LO )
    ihi = c + ( MODEL_HI - MODEL_LO )
    ibr, ibl, itr, itl = ( ihi[ 0 ], ilo[ 1 ] ), ( ilo[ 0 ], ilo[ 1 ] ), ( ihi[ 0 ], ihi[ 1 ] ), ( ilo[ 0 ], ihi[ 1 ] )
    obr, obl, otr, otl = ( hi[ 0 ], lo[ 1 ] ), ( lo[ 0 ], lo[ 1 ] ), ( hi[ 0 ], hi[ 1 ] ), ( lo[ 0 ], hi[ 1 ] )
    return [ ( ibr, ibl, itr, itl ), ( ibl, obl, itl, otl ), ( obr, obl, ibr, ibl ), ( obr, ibr, otr, itr ),
             ( itr, itl, otr, otl ) ]


def map_segs( split, lo, hi ):
    """The background map lines of each piece of the symmetry plane, as 3D segments ( n, 2, 3 ), and each piece's
    outline ( 5, 3 )."""
    segs = []
    outlines = []
    for p0, p1, p2, p3 in sym_pieces( split, lo, hi ):
        p0, p1, p2, p3 = ( np.array( [ q[ 0 ], 0.0, q[ 1 ] ] ) for q in ( p0, p1, p2, p3 ) )
        for k in range( NMAP + 1 ):
            t = k / NMAP
            segs.append( [ p0 + t * ( p1 - p0 ), p2 + t * ( p3 - p2 ) ] )
            segs.append( [ p0 + t * ( p2 - p0 ), p1 + t * ( p3 - p1 ) ] )
        outlines.append( np.array( [ p0, p1, p3, p2, p0 ] ) )
    return np.array( segs ), outlines


def sym_mesh( split ):
    V, T, tag = half_far( split )
    P = V[ T ]
    sym = np.all( np.abs( P[ :, :, 1 ] ) < 1e-9, axis = 1 )
    Q = V[ T[ sym ] ].reshape( -1, 3 )
    lo = np.array( [ Q[ :, 0 ].min(), Q[ :, 2 ].min() ] )
    hi = np.array( [ Q[ :, 0 ].max(), Q[ :, 2 ].max() ] )
    return V, T[ sym ], lo, hi


def fig_symmetry():
    # The pieces of a split symmetry plane, whole and near the model, the model's bounding box filled.
    V, T, lo, hi = sym_mesh( 1.0 )
    near = ( np.array( [ -20.0, -15.0 ] ), np.array( [ 30.0, 15.0 ] ) )
    for name, bounds, h in ( ( "CFDMesh_symmetry_pieces", None, 3.2 ), ( "CFDMesh_symmetry_near", near, 3.0 ) ):
        fig = plt.figure( figsize = ( fl.PAGE_W, h ) )
        c = mf.panel( fig, SYM_VIEW )
        segs, outlines = map_segs( 1.0, lo, hi )
        for o in outlines:
            c.segs( o, "k", 0.8 )
        c.ax.fill( [ MODEL_LO[ 0 ], MODEL_HI[ 0 ], MODEL_HI[ 0 ], MODEL_LO[ 0 ] ],
                   [ MODEL_LO[ 1 ], MODEL_LO[ 1 ], MODEL_HI[ 1 ], MODEL_HI[ 1 ] ], color = "0.3", zorder = 5 )
        if bounds is None:
            q = np.array( [ [ near[ 0 ][ 0 ], near[ 0 ][ 1 ] ], [ near[ 1 ][ 0 ], near[ 0 ][ 1 ] ],
                            [ near[ 1 ][ 0 ], near[ 1 ][ 1 ] ], [ near[ 0 ][ 0 ], near[ 1 ][ 1 ] ],
                            [ near[ 0 ][ 0 ], near[ 0 ][ 1 ] ] ] )
            c.ax.plot( q[ :, 0 ], q[ :, 1 ], color = "#C03030", lw = 0.8, zorder = 6 )
            mf.key( c, "front", at = ( 1.0, 0.0 ), dx = 6, dy = 4 )
        else:
            c.bounds = bounds
            mf.key( c, "front", at = ( 1.0, 0.0 ), dx = -40, dy = 4 )
        save_clipped_rows( fig, [ [ c ] ], name, h + 0.2 )

    # The mesh near the model, over the background map that sized it.
    fig = plt.figure( figsize = ( fl.PAGE_W, 5.0 ) )
    rows = []
    for split, name in ( ( 0.0, "off" ), ( 1.0, "on" ) ):
        V, T, lo, hi = sym_mesh( split )
        c = mf.panel( fig, SYM_VIEW )
        draw_mesh( c, V, T, color = SYM, lw = 0.2 )
        segs, outlines = map_segs( split, lo, hi )
        S = c.v.p2( segs.reshape( -1, 3 ) ).reshape( -1, 2, 2 )
        c.ax.add_collection( LineCollection( S, colors = "#C03030", linewidths = 0.25, alpha = 0.4, zorder = 60 ) )
        c.bounds = ( np.array( [ -12.0, -6.0 ] ), np.array( [ 22.0, 6.0 ] ) )
        label( c, name )
        rows.append( [ c ] )
    mf.key( rows[ -1 ][ 0 ], "front", at = ( 1.0, 0.0 ), dx = -40, dy = 6 )
    save_clipped_rows( fig, rows, "CFDMesh_symmetry_map", 5.2 )

FIGS = {
    "curvature": fig_curvature,
    "source": fig_point_source,
    "symmetry": fig_symmetry,
    "rules": fig_rules,
    "width": fig_width,
    "precedence": fig_precedence,
}

if __name__ == "__main__":
    mf.style()
    for k in ( sys.argv[ 1: ] or FIGS ):
        FIGS[ k ]()
