"""Drawing helpers for the propeller help figures.

A 3D figure is a fixed orthographic projection drawn onto ordinary 2D matplotlib axes, so that every
figure has one scale, true aspect and full control over draw order.  Surfaces are painted far to near;
annotations are drawn over them.

Points passed in are in VSP propeller axes (X shaft/downstream, Y radial, Z tangential).  A View maps
them to the page; propeller_figs.py chooses the views.
"""

import numpy as np
import matplotlib as mpl
from matplotlib.collections import PolyCollection, LineCollection
from matplotlib.patches import FancyArrowPatch
from matplotlib.path import Path
from matplotlib.colors import to_rgb

# ---------------------------------------------------------------- one palette for every figure
# Okabe-Ito hues, chosen to stay distinct for colour-blind readers and in print.
COL = {
    "flap":     "#0072B2",   # blue
    "lag":      "#D55E00",   # vermillion
    "feather":  "#009E73",   # bluish green  (and twist: also a turn about the radial axis)
    "fold":     "#CC79A7",   # reddish purple
    "precone":  "#882255",   # wine: distinct from the lag's vermillion
    "shaft":    "#56B4E9",   # sky blue: Rotate, BladeAz, rotation sense, sweep (turns about X)
    "zrot":     "#999933",   # olive: ZRotate about the chord line
    "move":     "#555555",   # translations
    "vsp":      "#000000",   # propeller frame X, Y, Z
    "blade":    "#8C564B",   # blade frame B
    "thrust":   "#000000",
    "ghost":    "#9A9A9A",
    "surf":     "#C8CCD2",
    "surf2":    "#E3D3C3",   # second object (e.g. the Reverse rotor)
    "section":  "#333333",
}

FS = 9.0      # the one text size of every figure, points
LW = 1.0      # base line width, points

# Every figure is PAGE_W wide, so the page shows them all at one scale and FS is one size on the page.
PAGE_W = 7.0  # inches
HMAX = 4.6    # the tallest a one-row figure is let grow, inches
MARGIN = 0.04


def style():
    mpl.rcParams.update( {
        "font.family": "STIXGeneral",
        "mathtext.fontset": "stix",
        "font.size": FS,
        "axes.titlesize": FS,
        "savefig.bbox": None,
        "lines.solid_capstyle": "round",
    } )


def unit( v ):
    v = np.asarray( v, float )
    return v / np.linalg.norm( v )


def perp( a ):
    """A unit vector perpendicular to a."""
    a = unit( a )
    t = np.array( [ 1.0, 0, 0 ] ) if abs( a[0] ) < 0.8 else np.array( [ 0, 1.0, 0 ] )
    u = np.cross( a, t )
    return unit( u )


# ---------------------------------------------------------------- views

class View:
    """An orthographic view: the page's right and up directions, and out of the page, in propeller axes."""

    @staticmethod
    def plane( right, up ):
        """A 2D view with the given VSP directions to the right and up (out = right x up)."""
        v = View.__new__( View )
        v.right = unit( right )
        v.up = unit( up )
        v.out = np.cross( v.right, v.up )
        return v

    def p2( self, P ):
        P = np.asarray( P, float )
        return np.stack( [ P @ self.right, P @ self.up ], axis = -1 )

    def depth( self, P ):
        return np.asarray( P, float ) @ self.out


# ---------------------------------------------------------------- canvas

class Canvas:
    def __init__( self, ax, view ):
        self.ax = ax
        self.v = view
        self.prims = []      # ( depth, kind, data, style )
        self.pts = []        # projected points for limits
        self.bounds = None   # ( lo, hi ) to frame instead of the points
        ax.set_axis_off()
        ax.set_aspect( "equal" )
        self.light = unit( 0.35 * view.right + 0.6 * view.up + 0.75 * view.out )

    # ---- depth-sorted content

    def surface( self, G, color = None, alpha = 1.0, edge = True, ghost = False, outline_lw = 0.6, cap_rows = True ):
        """G: ( nu, nw, 3 ) grid in VSP axes.  w runs round the section."""
        if color is None:
            color = COL[ "surf" ]
        base = np.array( to_rgb( color ) )
        nu, nw, _ = G.shape
        for i in range( nu - 1 ):
            for j in range( nw - 1 ):
                q = np.array( [ G[i, j], G[i + 1, j], G[i + 1, j + 1], G[i, j + 1] ] )
                n = np.cross( q[2] - q[0], q[3] - q[1] )
                nn = np.linalg.norm( n )
                if nn < 1e-14:
                    continue
                n = n / nn
                if ghost:
                    c = np.array( [ 1.0, 1.0, 1.0 ] ) * 0.97
                    a = 0.55
                else:
                    lam = abs( n @ self.light )
                    shade = 0.62 + 0.38 * lam
                    c = np.clip( base * shade + 0.08 * lam ** 8, 0, 1 )
                    a = alpha
                self.prims.append( ( self.v.depth( q ).mean(), "poly", self.v.p2( q ), ( c, a ) ) )
        self.pts.append( self.v.p2( G.reshape( -1, 3 ) ) )
        if edge:
            ec = COL[ "ghost" ] if ghost else "#4a4a4a"
            ls = ( 0, ( 3, 2 ) ) if ghost else "-"
            lines = [ G[ :, 0 ], G[ :, nw // 2 ] ]
            if cap_rows:
                lines += [ G[ 0 ], G[ -1 ] ]
            for L in lines:
                self.segs( L, ec, outline_lw, ls, bias = 0.05 )

    def segs( self, L, color, lw, ls = "-", bias = 0.0 ):
        """A polyline whose pieces take part in the depth sort."""
        L = np.asarray( L, float )
        d = self.v.depth( L )
        P = self.v.p2( L )
        for k in range( len( L ) - 1 ):
            self.prims.append( ( 0.5 * ( d[k] + d[k + 1] ) + bias, "seg", P[ k:k + 2 ], ( color, lw, ls ) ) )
        self.pts.append( P )

    def flush( self, z0 = 1.0 ):
        """Paint the depth-sorted content, far to near, from zorder z0 up."""
        self.prims.sort( key = lambda p: p[0] )
        z = z0
        k = 0
        n = len( self.prims )
        while k < n:
            kind = self.prims[k][1]
            m = k
            while m < n and self.prims[m][1] == kind:
                m += 1
            run = self.prims[ k:m ]
            if kind == "poly":
                pc = PolyCollection( [ p[2] for p in run ], facecolors = [ ( *p[3][0], p[3][1] ) for p in run ],
                                     edgecolors = [ ( *p[3][0], p[3][1] ) for p in run ], linewidths = 0.25, zorder = z )
                self.ax.add_collection( pc )
            else:
                lc = LineCollection( [ p[2] for p in run ], colors = [ p[3][0] for p in run ],
                                     linewidths = [ p[3][1] for p in run ], linestyles = [ p[3][2] for p in run ],
                                     zorder = z, capstyle = "round" )
                self.ax.add_collection( lc )
            z += 0.001
            k = m
        self.prims = []

    # ---- overlay content (always on top)

    def line( self, P, color = "k", lw = LW, ls = "-", z = 50, **kw ):
        Q = self.v.p2( P )
        self.pts.append( Q )
        self.ax.plot( Q[:, 0], Q[:, 1], color = color, lw = lw, ls = ls, zorder = z, solid_capstyle = "round", **kw )

    def arrow_path( self, P, color = "k", lw = LW, head = 7.0, z = 60, ls = "-", both = False ):
        Q = self.v.p2( P )
        self.pts.append( Q )
        style_ = "<|-|>" if both else "-|>"
        a = FancyArrowPatch( path = Path( Q ), arrowstyle = style_, mutation_scale = head, color = color, lw = lw,
                             shrinkA = 0, shrinkB = 0, zorder = z, linestyle = ls, joinstyle = "miter",
                             capstyle = "butt" )
        self.ax.add_patch( a )

    def arrow( self, p0, p1, **kw ):
        self.arrow_path( np.array( [ p0, p1 ] ), **kw )

    def text( self, p, s, color = "k", size = FS, dx = 0.0, dy = 0.0, ha = "center", va = "center", z = 80, halo = True, **kw ):
        q = self.v.p2( p )
        self.pts.append( q[ None, : ] )
        t = self.ax.annotate( s, xy = q, xytext = ( dx, dy ), textcoords = "offset points", ha = ha, va = va,
                              color = color, fontsize = size, zorder = z, annotation_clip = False, **kw )
        if halo:
            import matplotlib.patheffects as pe
            t.set_path_effects( [ pe.withStroke( linewidth = 2.2, foreground = "white" ) ] )
        return t

    def point( self, p, color = "k", s = 6, z = 70, marker = "o" ):
        q = self.v.p2( p )
        self.ax.plot( [ q[0] ], [ q[1] ], marker = marker, ms = s ** 0.5 * 1.3, color = color, zorder = z, mec = "white", mew = 0.5 )

    def triad( self, o, vecs, labels, color, length = 1.0, lw = 1.1, z = 65, label_pad = 1.18, size = FS, dashed = False ):
        """Three labelled arrows from o along vecs (VSP axes)."""
        for v, lab in zip( vecs, labels ):
            v = unit( v )
            self.arrow( o, o + length * v, color = color, lw = lw, z = z, head = 7.0, ls = ( 0, ( 4, 1.5 ) ) if dashed else "-" )
            if lab:
                self.text( o + label_pad * length * v, lab, color = color, size = size, z = z + 5 )

    def arc_pts( self, c, axis, rho, t0, t1, n = 40, ref = None ):
        """Arc right-handed about axis, centre c, from angle t0 to t1 (deg), angle 0 along ref."""
        a = unit( axis )
        if ref is None:
            u = perp( a )
        else:
            u = unit( np.asarray( ref, float ) - ( np.asarray( ref, float ) @ a ) * a )
        w = np.cross( a, u )
        t = np.radians( np.linspace( t0, t1, n ) )
        return c + rho * ( np.outer( np.cos( t ), u ) + np.outer( np.sin( t ), w ) )

    def hinge( self, p, axis, color, half = 1.5, rho = 0.45, label = None, at = 0.75, sweep = 250.0, lw = 1.5,
               label_dx = 0, label_dy = 0, ref = None, z = 55, ls = "-", start = None, arrow_on_axis = True, size = FS ):
        """A hinge line through p along axis (right-handed for a positive turn), with a curved arrow
        showing the positive sense, centred at the fraction 'at' of the way to the + end."""
        a = unit( axis )
        p = np.asarray( p, float )
        p0 = p - half * a
        p1 = p + half * a
        if start is not None:
            p0 = p + start * a
        if arrow_on_axis:
            self.arrow( p0, p1 + 0.25 * a, color = color, lw = lw, z = z, head = 7.0, ls = ls )
        else:
            self.line( np.array( [ p0, p1 ] ), color = color, lw = lw, z = z, ls = ls )
        c = p0 + ( at ) * ( p1 - p0 )
        if ref is None:
            ref = self.v.up - ( self.v.up @ a ) * a
            if np.linalg.norm( ref ) < 0.2:
                ref = self.v.right
            # Start the arc where it is most visible: begin on the side facing the viewer.
            ref = unit( np.cross( a, self.v.out ) ) if np.linalg.norm( np.cross( a, self.v.out ) ) > 0.2 else ref
        A = self.arc_pts( c, a, rho, -0.5 * sweep, 0.5 * sweep, ref = ref )
        self.arrow_path( A, color = color, lw = lw * 0.85, z = z + 1, head = 6.0 )
        if label:
            self.text( p1 + 0.25 * a, label, color = color, dx = label_dx, dy = label_dy, size = size, z = z + 10 )

    def angle( self, c, v0, v1, rho, color, label = None, lw = 0.9, label_r = 1.35, dx = 0, dy = 0, head = True, size = FS, z = 58 ):
        """An angle arc from direction v0 to direction v1 about c (an arrowhead at v1)."""
        v0 = unit( v0 )
        v1 = unit( v1 )
        ang = np.degrees( np.arccos( np.clip( v0 @ v1, -1, 1 ) ) )
        if ang < 1e-6:
            return
        ax = unit( np.cross( v0, v1 ) )
        A = self.arc_pts( c, ax, rho, 0, ang, ref = v0 )
        if head:
            self.arrow_path( A, color = color, lw = lw, head = 5.0, z = z )
        else:
            self.line( A, color = color, lw = lw, z = z )
        if label:
            mid = self.arc_pts( c, ax, rho * label_r, 0.5 * ang, 0.5 * ang, n = 1, ref = v0 )[0]
            self.text( mid, label, color = color, dx = dx, dy = dy, size = size, z = z + 10 )

    def finish( self, pad = 0.04, xlim = None, ylim = None ):
        self.flush()
        if xlim is None:
            P = np.vstack( self.pts )
            lo = P.min( 0 )
            hi = P.max( 0 )
            span = ( hi - lo ).max()
            lo = lo - pad * span
            hi = hi + pad * span
            self.ax.set_xlim( lo[0], hi[0] )
            self.ax.set_ylim( lo[1], hi[1] )
        else:
            self.ax.set_xlim( *xlim )
            self.ax.set_ylim( *ylim )

    def limits( self ):
        if self.bounds is not None:
            return np.asarray( self.bounds[0], float ), np.asarray( self.bounds[1], float )
        P = np.vstack( self.pts )
        return P.min( 0 ), P.max( 0 )


def _overhang( fig, c, lo, hi ):
    """How far c's drawing reaches past its content box lo..hi: left, right, bottom, top, in inches."""
    r = fig.canvas.get_renderer()
    boxes = []
    for a in c.ax.get_children():
        if a is c.ax.patch or a in c.ax.spines.values() or a in ( c.ax.xaxis, c.ax.yaxis ) or not a.get_visible():
            continue
        a.set_clip_on( False )
        b = a.get_tightbbox( r )
        if b is not None and np.all( np.isfinite( b.extents ) ) and b.width > 0:
            boxes.append( b.extents )
    q = c.ax.transData.transform( np.array( [ lo, hi ] ) ) / fig.dpi
    if not boxes:
        return np.zeros( 4 )
    E = np.array( boxes ) / fig.dpi
    o = np.array( [ q[0, 0] - E[:, 0].min(), E[:, 2].max() - q[1, 0], q[0, 1] - E[:, 1].min(), E[:, 3].max() - q[1, 1] ] )
    return np.maximum( o, 0.0 )


def fit( fig, rows, scale = None, hmax = HMAX, margin = MARGIN ):
    """Make fig PAGE_W wide and lay out rows of canvases at one scale, in inches per model unit, with everything they
    draw inside the figure.  Each row splits the width evenly; each panel's content is centered in its cell.  The
    scale fills the width unless the figure would pass hmax, or unless scale is given.  Returns the scale."""
    W = PAGE_W
    cells = [ c for r in rows for c in r ]
    for c in cells:
        c.flush()
    box = { id( c ): c.limits() for c in cells }
    o = { id( c ): np.zeros( 4 ) for c in cells }
    s = scale
    for it in range( 5 ):
        def need( c ):
            lo, hi = box[ id( c ) ]
            return hi - lo, o[ id( c ) ]
        if scale is None:
            s = min( ( W / len( r ) - 2 * margin - need( c )[1][0] - need( c )[1][1] ) / max( need( c )[0][0], 1e-9 )
                     for r in rows for c in r )
            fixed = sum( max( need( c )[1][2] + need( c )[1][3] + 2 * margin for c in r ) for r in rows )
            tall = sum( max( need( c )[0][1] for c in r ) for r in rows )
            if fixed + s * tall > hmax:
                s = ( hmax - fixed ) / tall
            if s <= 0.0:
                raise ValueError( "the labels leave no room for the drawing: move them in" )
        heights = [ max( s * need( c )[0][1] + need( c )[1][2] + need( c )[1][3] + 2 * margin for c in r ) for r in rows ]
        H = sum( heights )
        fig.set_size_inches( W, H )
        y = H
        for r, h in zip( rows, heights ):
            y -= h
            cw = W / len( r )
            for k, c in enumerate( r ):
                ( d, oo ) = need( c )
                lo = box[ id( c ) ][0]
                left = 0.5 * ( cw - ( s * d[0] + oo[0] + oo[1] ) ) + oo[0]
                bot = 0.5 * ( h - ( s * d[1] + oo[2] + oo[3] ) ) + oo[2]
                c.ax.set_position( [ k * cw / W, y / H, cw / W, h / H ] )
                c.ax.set_xlim( lo[0] - left / s, lo[0] + ( cw - left ) / s )
                c.ax.set_ylim( lo[1] - bot / s, lo[1] + ( h - bot ) / s )
        if it == 4:
            break
        for c in cells:
            o[ id( c ) ] = _overhang( fig, c, *box[ id( c ) ] )
        # Panels in a row share their reach above and below, and panels in a column their reach to the sides, so
        # panels with one box line up.
        for r in rows:
            m = np.max( [ o[ id( c ) ][ 2: ] for c in r ], 0 )
            for c in r:
                o[ id( c ) ][ 2: ] = m
        cols = {}
        for r in rows:
            for k, c in enumerate( r ):
                cols.setdefault( ( len( r ), k ), [] ).append( c )
        for col in cols.values():
            m = np.max( [ o[ id( c ) ][ :2 ] for c in col ], 0 )
            for c in col:
                o[ id( c ) ][ :2 ] = m
    return s



def out_of_page( ax, xy, label = "", color = "k", inward = False, r = 3.2, size = FS, dx = 6, dy = 0 ):
    """The usual circle-dot (toward the reader) or circle-cross (away) axis symbol, a fixed size in points."""
    ax.plot( [ xy[0] ], [ xy[1] ], marker = "o", ms = 2 * r, mfc = "white", mec = color, mew = 0.9, zorder = 90 )
    if inward:
        ax.plot( [ xy[0] ], [ xy[1] ], marker = "x", ms = 1.25 * r, mec = color, mew = 0.9, zorder = 91 )
    else:
        ax.plot( [ xy[0] ], [ xy[1] ], marker = "o", ms = 0.55 * r, mfc = color, mec = color, zorder = 91 )
    ax.annotate( label, xy = xy, xytext = ( dx, dy ), textcoords = "offset points", ha = "left", va = "center",
                 fontsize = size, color = color, zorder = 92 )
