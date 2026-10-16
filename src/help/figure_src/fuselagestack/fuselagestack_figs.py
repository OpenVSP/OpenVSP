"""Figures for FuselageStack.md: section placement, section rotation, V Align, a Stack loop, Round end caps.

    <build>/venv/bin/python fuselagestack_figs.py [place rotate loop valign caps]
"""

import sys
import numpy as np

import bodyfig as bf
from bodyfig import vsp, X, Y, Z, FS

# One body built both ways: four steps of 1, ellipses 1 wide and 1.2 tall between point ends.
FRACS = [ 0.0, 0.25, 0.5, 0.75, 1.0 ]
LEN = 4.0


def make_pair( roty = 0.0 ):
    """A Fuselage of Length 4 and a Stack with the same sections; section 2 turned by roty."""
    bf.renew()
    f = vsp.AddGeom( "FUSELAGE" )
    s = vsp.AddGeom( "STACK" )
    vsp.Update()
    vsp.SetParmVal( f, "Length", "Design", LEN )
    for i in range( 5 ):
        if 0 < i < 4:
            for g in ( f, s ):
                bf.setx( g, i, "Ellipse_Width", 1.0 )
                bf.setx( g, i, "Ellipse_Height", 1.2 )
        if i > 0:
            bf.setx( s, i, "XDelta", 1.0 )
    vsp.Update()
    bf.setx( f, 2, "YRotate", roty )
    bf.setx( s, 2, "YRotate", roty )
    vsp.Update()
    # Both lay their sections at the same places.
    for i, fr in enumerate( FRACS ):
        assert abs( bf.getx( f, i, "XLocPercent" ) - fr ) < 1e-12
    return f, s


def section_trace( gid, i, n = 5 ):
    """Section i cut by Y = 0: from its bottom point to its top point."""
    u = i / ( n - 1.0 )
    return np.array( [ list( vsp.CompPnt01( gid, 0, u, 0.25 ) ), list( vsp.CompPnt01( gid, 0, u, 0.75 ) ) ] )


def draw_body( c, gid, ghost = None, hi = None ):
    if ghost is not None:
        bf.fill( c, ghost, face = "none", edge = bf.GHOST, lw = 0.8, z = 20, ls = ( 0, ( 4, 2 ) ) )
    bf.fill( c, bf.body_cut( gid ) )
    trs = [ section_trace( gid, i ) for i in range( 5 ) ]
    for i, tr in enumerate( trs ):
        col = "#1f1f1f" if hi != i else "#0072B2"
        c.line( tr, color = col, lw = 1.4 if hi == i else 0.9, z = 30 )
        # Number each section just beyond its top along its own line.  A point section is numbered above and
        # outward, clear of the axis, or along the body where the body turns down to it.
        d = tr[1] - tr[0]
        if np.linalg.norm( d ) > 1e-9:
            d = d / np.linalg.norm( d )
            dx, dy = 9.0 * d[0], 9.0 * d[2]
        else:
            j = 1 if i == 0 else i - 1
            d = tr[1] - trs[j].mean( 0 )
            d = d / np.linalg.norm( d )
            if d[2] > -0.3:
                dx, dy = 7.0 * np.sign( d[0] ), 9.0
            else:
                dx, dy = 9.0 * d[0], 9.0 * d[2]
        bf.label( c, tr[1], "%d" % i, dx = dx, dy = dy )


def place():
    """FuselageStack_place: one body, placed as fractions of a length and as steps."""
    f, s = make_pair()
    fig = bf.figure()
    cf = bf.canvas( fig, bf.SIDE )
    cs = bf.canvas( fig, bf.SIDE )
    draw_body( cf, f )
    draw_body( cs, s )
    bf.axis_line( cf, -0.3, LEN + 0.3 )
    bf.axis_line( cs, -0.3, LEN + 0.3 )
    zb = -0.6
    for i, ( fr, lab ) in enumerate( zip( FRACS[ 1: ], [ r"$0.25\,L$", r"$0.5\,L$", r"$0.75\,L$", r"$L$" ] ) ):
        bf.dim( cf, np.array( [ 0, 0, zb ] ), np.array( [ fr * LEN, 0, zb ] ), "", off = ( 0, -0.25 * ( i + 1 ) ) )
        bf.label( cf, np.array( [ fr * LEN, 0, zb - 0.25 * ( i + 1 ) ] ), r"$X = $" + lab, dx = 6, ha = "left" )
    for i in range( 1, 5 ):
        x0 = ( i - 1 ) * 1.0
        bf.dim( cs, np.array( [ x0, 0, zb ] ), np.array( [ x0 + 1.0, 0, zb ] ), r"$\Delta X = 1$", off = ( 0, -0.25 ), dy = -9 )
    bf.title( cf, "Fuselage, Length L = 4", pad = 16 )
    bf.title( cs, "Stack", pad = 16 )
    bf.key( cs, "side", ( 1.0, 0.0 ), 12, 0 )
    bf.save( fig, "FuselageStack_place", [ [ cf ], [ cs ] ], hmax = 6.5 )


def rotate():
    """FuselageStack_rotate: Rot Y 30 on section 2, in a Fuselage and in a Stack."""
    f, s = make_pair()
    ghost_f = bf.body_cut( f )
    ghost_s = bf.body_cut( s )
    f, s = make_pair( 30.0 )
    # The Stack's later sections follow the turn; the Fuselage's stay.
    assert abs( vsp.CompPnt01( f, 0, 0.75, 0.75 ).x() - 3.0 ) < 1e-9
    assert vsp.CompPnt01( s, 0, 1.0, 0.75 ).z() < -0.4    # the tail drops: 1 + 1 sin 30 below
    fig = bf.figure()
    cf = bf.canvas( fig, bf.SIDE )
    cs = bf.canvas( fig, bf.SIDE )
    for c, g, gh in ( ( cf, f, ghost_f ), ( cs, s, ghost_s ) ):
        draw_body( c, g, ghost = gh, hi = 2 )
        bf.axis_line( c, -0.3, LEN + 0.3 )
        ctr = np.array( [ 2.0, 0, 0 ] )
        c.line( np.array( [ ctr - 0.9 * Z, ctr + 0.9 * Z ] ), color = bf.GHOST, lw = 0.7, ls = ( 0, ( 2, 2 ) ), z = 25 )
        d1 = np.array( [ np.sin( np.radians( 30 ) ), 0, np.cos( np.radians( 30 ) ) ] )
        c.line( np.array( [ ctr - 0.9 * d1, ctr + 0.9 * d1 ] ), color = "#0072B2", lw = 0.7, ls = ( 0, ( 2, 2 ) ), z = 25 )
        c.angle( ctr, Z, d1, 0.85, "#0072B2", label = r"$30^\circ$", label_r = 1.12, dx = 4 )
    # The Stack's path: each section a step along the turned direction.
    bf.title( cf, "Fuselage", pad = 16 )
    bf.title( cs, "Stack", pad = 16 )
    bf.key( cs, "side", ( 1.0, 0.0 ), 12, 0 )
    bf.save( fig, "FuselageStack_rotate", [ [ cf ], [ cs ] ], hmax = 6.5 )


def loop():
    """FuselageStack_loop: the Nacelle Flowthru Mid Origin preset, its sections in a loop."""
    bf.renew()
    s = vsp.AddGeom( "STACK" )
    vsp.Update()
    vsp.InitStackPreset( s, vsp.STACK_PRESET_FLOWTHRU_MID_ORIG )
    vsp.SetParmVal( s, "GeomIOType", "EngineModel", vsp.ENGINE_GEOM_NONE )
    vsp.Update()
    n = bf.nxsec( s )
    assert n == 7 and int( vsp.GetParmVal( s, "OrderPolicy", "Design" ) ) == vsp.STACK_LOOP
    top, bot = bf.cut_halves( s )
    fig = bf.figure()
    c = bf.canvas( fig, bf.SIDE )
    bf.fill( c, top )
    bf.fill( c, bot )
    bf.axis_line( c, -3.3, 3.8 )
    pts = [ np.array( list( vsp.CompPnt01( s, 0, i / ( n - 1.0 ), 0.75 ) ) ) for i in range( n ) ]
    off = { 0: ( 0, -11 ), 1: ( 0, -11 ), 2: ( -10, 0 ), 3: ( 0, 10 ), 4: ( 10, 0 ), 5: ( 0, -11 ), 6: ( 0, -11 ) }
    for i, p in enumerate( pts ):
        c.point( p, "k", s = 10 )
    for i in range( n - 1 ):
        bf.label( c, pts[i], "0, 6" if i == 0 else "%d" % i, dx = off[i][0], dy = off[i][1] )
    # Arrows along the loop, at the middle of each span, in the order the sections run.
    m = len( top )
    for i in range( n - 1 ):
        k = int( ( i + 0.5 ) / ( n - 1 ) * ( m - 1 ) )
        c.arrow( top[ k - 3 ], top[ k + 3 ], color = "#0072B2", lw = 1.2, head = 8.0, z = 60 )
    bf.label( c, np.array( [ -2.6, 0, 2.1 ] ), "inlet", ha = "right" )
    bf.label( c, np.array( [ 3.4, 0, 2.0 ] ), "outlet", ha = "left" )
    bf.key( c, "side", ( 1.0, 0.0 ), 14, 0 )
    bf.save( fig, "FuselageStack_loop", [ [ c ] ], hmax = 4.0 )


def valign():
    """FuselageStack_valign: three ellipses of different heights at V Align 0 and 1."""
    heights = { 1: 0.8, 2: 1.4, 3: 0.6 }
    canv = []
    fig = bf.figure()
    for va in ( 0.0, 1.0 ):
        bf.renew()
        s = vsp.AddGeom( "STACK" )
        vsp.Update()
        for i in range( 5 ):
            if i in heights:
                bf.setx( s, i, "Ellipse_Width", 1.0 )
                bf.setx( s, i, "Ellipse_Height", heights[i] )
            if i > 0:
                bf.setx( s, i, "XDelta", 1.0 )
            bf.setx( s, i, "VAlign", va )
        vsp.Update()
        # At 1 every top lies on the Stack's line, and each section is placed from the one before as if unmoved.
        for i, h in heights.items():
            top = vsp.CompPnt01( s, 0, i / 4.0, 0.75 )
            assert abs( top.z() - 0.5 * h * ( 1.0 - va ) ) < 1e-9 and abs( top.x() - i ) < 1e-9
        c = bf.canvas( fig, bf.SIDE )
        draw_body( c, s )
        bf.axis_line( c, -0.3, LEN + 0.3 )
        if va == 1.0:
            ctr = np.array( [ 2.0, 0, -0.7 ] )
            c.point( ctr, "k", s = 10 )
            bf.label( c, ctr, "center", dx = 6, ha = "left" )
            bf.dim( c, np.array( [ 2.0, 0, 0 ] ), ctr, r"$H/2$", off = ( -0.35, 0 ), dx = -14, ext = False )
        bf.title( c, "V Align %g" % va, pad = 16 )
        canv.append( c )
    bf.key( canv[ -1 ], "side", ( 1.0, 0.0 ), 12, 0 )
    bf.save( fig, "FuselageStack_valign", [ [ canv[0] ], [ canv[1] ] ], hmax = 6.5 )


def hull( Q ):
    """The convex hull of 2D points Q ( n, 2 ), counterclockwise (Andrew's monotone chain)."""
    P = sorted( set( map( tuple, np.round( Q, 9 ) ) ) )

    def half( pts ):
        h = []
        for p in pts:
            while len( h ) >= 2 and ( ( h[-1][0] - h[-2][0] ) * ( p[1] - h[-2][1] ) -
                                      ( h[-1][1] - h[-2][1] ) * ( p[0] - h[-2][0] ) ) <= 0:
                h.pop()
            h.append( p )
        return h
    lo = half( P )
    hi = half( P[ ::-1 ] )
    return np.array( lo[ :-1 ] + hi[ :-1 ] )


def capped( W, H, step = 2.0 ):
    """A straight Stack of two sections W wide and H tall (a circle when they are equal), a step apart, flat at the
    nose and Round at the tail."""
    bf.renew()
    s = vsp.AddGeom( "STACK" )
    vsp.Update()
    vsp.InitStackPreset( s, vsp.STACK_PRESET_CYLINDER_ENDCAPS )
    vsp.Update()
    xs = vsp.GetXSecSurf( s, 0 )
    for i in ( 0, 1 ):
        if W == H:
            vsp.ChangeXSecShape( xs, i, vsp.XS_CIRCLE )
            vsp.Update()
            bf.setx( s, i, "Circle_Diameter", W )
        else:
            vsp.ChangeXSecShape( xs, i, vsp.XS_ELLIPSE )
            vsp.Update()
            bf.setx( s, i, "Ellipse_Width", W )
            bf.setx( s, i, "Ellipse_Height", H )
    bf.setx( s, 1, "XDelta", step )
    vsp.SetParmVal( s, "CapUMinOption", "EndCap", vsp.FLAT_END_CAP )
    vsp.SetParmVal( s, "CapUMaxOption", "EndCap", vsp.ROUND_END_CAP )
    vsp.Update()
    assert abs( vsp.GetParmVal( s, "CapUMaxLength", "EndCap" ) - 1.0 ) < 1e-12
    return s


def caps():
    """FuselageStack_caps: a Round tail cap on a circle and on an ellipse twice as wide as high, top and side."""
    step = 2.0
    rows = []
    for W, H, name, hw, hh in ( ( 2.0, 2.0, "Circle", r"$D/2$", r"$D/2$" ),
                                ( 2.0, 1.0, "Ellipse, width twice the height", r"$W/2$", r"$H/2$" ) ):
        s = capped( W, H, step )
        # The tail cap is the last third of U.
        G = bf.grid( s, 0, u = np.linspace( 0.0, 1.0, 241 ), w = np.linspace( 0.0, 1.0, 241 ) )
        cap = bf.grid( s, 0, u = np.linspace( 2.0 / 3.0, 1.0, 121 ), w = np.linspace( 0.0, 1.0, 241 ) )
        # The cap reaches out by the local half-height, whatever the width: H/2 at the center, in both views.
        x0 = step
        reach = cap[ ..., 0 ].max() - x0
        assert abs( reach - 0.5 * H ) < 1e-6, ( W, H, reach )
        loc = 0.5 * H * np.sqrt( np.clip( 1.0 - ( 2.0 * cap[ ..., 1 ] / W ) ** 2, 0.0, 1.0 ) )
        # Each cut at constant Y is a half circle of the local half-height.
        assert np.abs( np.hypot( cap[ ..., 0 ] - x0, cap[ ..., 2 ] ) - loc ).max() < 2e-3
        assert abs( np.abs( cap[ ..., 1 ] ).max() - 0.5 * W ) < 1e-9 and abs( np.abs( cap[ ..., 2 ] ).max() - 0.5 * H ) < 1e-9
        row = []
        for view, which, half, lab in ( ( bf.TOP, "top", 0.5 * W, hw ), ( bf.SIDE, "side", 0.5 * H, hh ) ):
            row.append( ( view, which, half, lab, G, name, H ) )
        rows.append( row )
    fig = bf.figure()
    canv = []
    for row in rows:
        crow = []
        for view, which, half, lab, G, name, H in row:
            c = bf.canvas( fig, view )
            Q = view.p2( G.reshape( -1, 3 ) )
            out = hull( Q )
            # Back to model axes on the view's plane.
            P = np.outer( out[:, 0], view.right ) + np.outer( out[:, 1], view.up )
            bf.fill( c, P )
            up = view.up
            # The section where the cap starts.
            c.line( np.array( [ x0 * X - half * up, x0 * X + half * up ] ), color = "#1f1f1f", lw = 0.9, z = 30 )
            bf.axis_line( c, -0.3, x0 + 1.3 )
            tip = ( x0 + 0.5 * H ) * X
            bf.dim( c, x0 * X - half * up, tip - half * up, lab if which == "side" else r"$H/2$" if H != 2.0 else r"$D/2$",
                    off = ( 0, -0.3 ), dy = -9 )
            bf.dim( c, x0 * X - 0.45 * X, x0 * X - 0.45 * X + half * up, lab, ext = False, dx = -16 )
            c.bounds = ( np.array( [ -0.3, -1.6 ] ), np.array( [ x0 + 1.3, 1.1 ] ) )
            bf.title( c, name, pad = 4 )
            bf.key( c, which, ( 1.0, 0.0 ), 4, 4 )
            crow.append( c )
        canv.append( crow )
    bf.save( fig, "FuselageStack_caps", canv, hmax = 6.0 )


FIGS = { "place": place, "rotate": rotate, "loop": loop, "valign": valign, "caps": caps }

if __name__ == "__main__":
    for name in ( sys.argv[ 1: ] or FIGS ):
        FIGS[ name ]()
