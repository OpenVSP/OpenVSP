"""Drawing helpers shared by the figures of the Clone, Auxiliary, Human, Point Cloud and Wire help pages.

They build on figlib (the fixed orthographic projection, figlib.fit and the one width and text size every help image
uses) and add what those pages need: Geoms read back as triangles through an STL export, painted with depth sorting,
and the 2D axis key.
"""

import os
import tempfile

HERE = os.path.dirname( os.path.abspath( __file__ ) )

import numpy as np
import matplotlib
matplotlib.use( "Agg" )
import matplotlib.pyplot as plt
from matplotlib.colors import to_rgb
from matplotlib.transforms import offset_copy

import openvsp as vsp
import figlib as fl
from figlib import COL

OUT = os.path.join( HERE, "..", "..", "images" )
DPI = 200

X = np.array( [ 1.0, 0, 0 ] )
Y = np.array( [ 0, 1.0, 0 ] )
Z = np.array( [ 0, 0, 1.0 ] )

ISO = fl.View.plane( [ 1, -1, 0 ], [ 1, 1, 2 ] )     # from (-1, -1, 1), Z up
FRONT = fl.View.plane( X, Z )                         # looking along +Y
TOP = fl.View.plane( X, Y )                           # looking along -Z
UPSTREAM = fl.View.plane( -Y, Z )                     # from -X looking along +X

KEY = { "front": ( r"$X$", r"$Z$", r"$-Y$" ), "top": ( r"$X$", r"$Y$", r"$Z$" ), "upstream": ( r"$-Y$", r"$Z$", r"$-X$" ) }

_TMP = tempfile.mkdtemp()


def style():
    fl.style()


def read_stl( fn ):
    """The triangles of an ASCII STL file, ( n, 3, 3 )."""
    tris = []
    cur = []
    with open( fn ) as f:
        for line in f:
            s = line.split()
            if s and s[ 0 ] == "vertex":
                cur.append( [ float( v ) for v in s[ 1:4 ] ] )
                if len( cur ) == 3:
                    tris.append( cur )
                    cur = []
    return np.array( tris ).reshape( -1, 3, 3 )


def geom_tris( gid ):
    """The triangles of one Geom as VSP places them, through an STL export of a set holding only it."""
    s = vsp.SET_FIRST_USER
    for g in vsp.FindGeoms():
        vsp.SetSetFlag( g, s, g == gid )
    vsp.Update()
    fn = os.path.join( _TMP, "g.stl" )
    vsp.ExportFile( fn, s, vsp.EXPORT_STL )
    return read_stl( fn )


def new( view ):
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    return fig, panel( fig, view )


def panel( fig, view ):
    return fl.Canvas( fig.add_axes( [ 0, 0, 1, 1 ] ), view )


def save( fig, name, rows, scale = None, hmax = fl.HMAX ):
    fl.fit( fig, rows, scale = scale, hmax = hmax )
    os.makedirs( OUT, exist_ok = True )
    fig.savefig( os.path.join( OUT, name + ".png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made", name )


def tris( c, T, color, flat = False, alpha = 1.0, z0 = None, edge = None, edge_lw = 0.3, group_alpha = None,
          cull = False ):
    """Triangles T ( n, 3, 3 ) painted far to near, lit like figlib's surfaces.  flat paints one shade (a silhouette);
    z0 paints them at once at that zorder instead of joining the canvas's depth sort; group_alpha then makes the whole
    set one even translucent layer, however many triangles overlap.  cull drops the triangles facing away, for a
    closed surface whose triangles face out."""
    base = np.array( to_rgb( color ) )
    n = np.cross( T[ :, 1 ] - T[ :, 0 ], T[ :, 2 ] - T[ :, 0 ] )
    nn = np.linalg.norm( n, axis = 1 )
    ok = nn > 1e-14
    T = T[ ok ]
    n = n[ ok ] / nn[ ok, None ]
    if cull:
        front = n @ c.v.out > 0.0
        T = T[ front ]
        n = n[ front ]
    if flat:
        cols = np.tile( base, ( len( T ), 1 ) )
    else:
        lam = np.abs( n @ c.light )
        cols = np.clip( base[ None, : ] * ( 0.62 + 0.38 * lam[ :, None ] ) + 0.08 * lam[ :, None ] ** 8, 0, 1 )
    d = c.v.depth( T.reshape( -1, 3 ) ).reshape( -1, 3 ).mean( 1 )
    P = c.v.p2( T.reshape( -1, 3 ) ).reshape( -1, 3, 2 )
    c.pts.append( P.reshape( -1, 2 ) )
    if z0 is not None:
        from matplotlib.collections import PolyCollection
        o = np.argsort( d )
        fc = [ ( *cols[ i ], alpha ) for i in o ]
        ec = fc if edge is None else edge
        pc = PolyCollection( P[ o ], facecolors = fc, edgecolors = ec, linewidths = 0.25 if edge is None else edge_lw,
                             zorder = z0 )
        if group_alpha is not None:
            pc.set_agg_filter( _Fade( group_alpha ) )
        c.ax.add_collection( pc )
        return
    for i in range( len( T ) ):
        c.prims.append( ( d[ i ], "poly", P[ i ], ( cols[ i ], alpha ) ) )


class _Fade:
    """An agg filter that scales a rendered artist's alpha, so overlapping pieces fade as one layer."""

    def __init__( self, a ):
        self.a = a

    def __call__( self, im, dpi ):
        im = im.copy()
        im[ :, :, 3 ] *= self.a
        return im, 0, 0


def clean( T, rel = 1e-4 ):
    """T without needle triangles (area small beside the longest edge squared), whose normals are noise."""
    n = np.cross( T[ :, 1 ] - T[ :, 0 ], T[ :, 2 ] - T[ :, 0 ] )
    a = np.linalg.norm( n, axis = 1 )
    L = np.max( [ np.sum( ( T[ :, k ] - T[ :, ( k + 1 ) % 3 ] ) ** 2, axis = 1 ) for k in range( 3 ) ], axis = 0 )
    return T[ a > rel * np.maximum( L, 1e-300 ) ]


def creases( c, T, angle = 35.0, color = "#3a3a3a", lw = 0.6 ):
    """The edges where the surface turns by more than angle degrees, and the open edges, drawn into the depth sort."""
    n = np.cross( T[ :, 1 ] - T[ :, 0 ], T[ :, 2 ] - T[ :, 0 ] )
    n = n / np.maximum( np.linalg.norm( n, axis = 1 ), 1e-300 )[ :, None ]
    scale = np.abs( T ).max() + 1e-12
    key = lambda p: tuple( np.round( p / scale, 7 ) )
    edges = {}
    for t in range( len( T ) ):
        for k in range( 3 ):
            a = key( T[ t, k ] )
            b = key( T[ t, ( k + 1 ) % 3 ] )
            if a == b:
                continue
            e = ( a, b ) if a < b else ( b, a )
            edges.setdefault( e, [] ).append( ( t, k ) )
    cmin = np.cos( np.radians( angle ) )
    for e, uses in edges.items():
        t0, k0 = uses[ 0 ]
        if len( uses ) == 2 and abs( n[ t0 ] @ n[ uses[ 1 ][ 0 ] ] ) > cmin:
            continue
        seg = np.array( [ T[ t0, k0 ], T[ t0, ( k0 + 1 ) % 3 ] ] )
        c.segs( seg, color, lw, bias = 0.01 * scale )


def key( c, which, at = ( 1.0, 0.0 ), dx = 0.0, dy = 0.0 ):
    """A small 2D axis key, dx, dy points from the point at fractions 'at' of the canvas's content box.  The circled
    dot is the axis toward the reader."""
    lo, hi = c.limits()
    q = lo + np.asarray( at, float ) * ( hi - lo )
    fig = c.ax.figure

    def T( x, y ):
        return offset_copy( c.ax.transData, fig, x = dx + x, y = dy + y, units = "points" )
    L = 18.0
    r, u, o = KEY[ which ]
    for ( x, y ), lab, ha, va, tx, ty in ( ( ( L, 0 ), r, "left", "center", L + 3, 0 ),
                                           ( ( 0, L ), u, "center", "bottom", 0, L + 2 ) ):
        c.ax.annotate( "", xy = q, xycoords = T( x, y ), xytext = q, textcoords = T( 0, 0 ), annotation_clip = False,
                       arrowprops = dict( arrowstyle = "-|>", lw = 0.8, color = "k", mutation_scale = 6, shrinkA = 0,
                                          shrinkB = 0 ) )
        c.ax.text( q[ 0 ], q[ 1 ], lab, transform = T( tx, ty ), ha = ha, va = va )
    c.ax.plot( [ q[ 0 ] ], [ q[ 1 ] ], transform = T( 0, 0 ), marker = "o", ms = 5.5, mfc = "white", mec = "k", mew = 0.7,
               zorder = 90 )
    c.ax.plot( [ q[ 0 ] ], [ q[ 1 ] ], transform = T( 0, 0 ), marker = "o", ms = 1.4, mfc = "k", mec = "k", zorder = 91 )
    c.ax.text( q[ 0 ], q[ 1 ], o, transform = T( -5, -5 ), ha = "right", va = "top" )
