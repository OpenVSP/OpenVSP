"""Figures for the OpenVSP help pages of the XForm and Sub tabs (src/help/XFormTab.md, src/help/SubTab.md).

    <build>/venv/bin/python commontabs_figs.py [names...]   # writes ../../images/XFormTab_*.png and SubTab_*.png

Every figure is drawn from geometry the openvsp module builds: surfaces sampled with CompPnt01, attachment frames and
points read back from attached Geoms, and sub-surface outlines read from a DegenGeom file.  The drawing primitives and
the page layout (one width, one text size) come from ../propeller/figlib.py.

Views keep OpenVSP's frame: 3D with Z up, Y into the page, X to the right, seen from about (-1, -1, 1); the top view
looks along -Z (X right, Y up); the front view along +Y (X right, Z up).
"""

import os
import sys
import tempfile

import numpy as np
import matplotlib
matplotlib.use( "Agg" )
import matplotlib.pyplot as plt
from matplotlib.transforms import offset_copy

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )

import figlib as fl                                           # noqa: E402
from figlib import COL, FS                                    # noqa: E402
import openvsp as vsp                                         # noqa: E402

OUT = os.path.join( HERE, "..", "..", "images" )
DPI = 200

X = np.array( [ 1.0, 0, 0 ] )
Y = np.array( [ 0, 1.0, 0 ] )
Z = np.array( [ 0, 0, 1.0 ] )
O = np.zeros( 3 )

ISO = fl.View.plane( [ 1, -1, 0 ], [ 1, 1, 2 ] )
TOP = fl.View.plane( X, Y )
FRONT = fl.View.plane( X, Z )
KEY = { "front": ( r"$X$", r"$Z$", r"$-Y$" ), "top": ( r"$X$", r"$Y$", r"$Z$" ) }

PARENT = COL[ "surf" ]
CHILD = "#E3C9A6"
FRAME = "#6A3D9A"          # attachment and symmetry frames
SSCOL = "#D55E00"          # sub-surfaces


# ---------------------------------------------------------------- VSP helpers

def setp( gid, name, group, val ):
    vsp.SetParmVal( gid, name, group, float( val ) )


def pnt( gid, isurf, u, w ):
    p = vsp.CompPnt01( gid, isurf, u, w )
    return np.array( [ p.x(), p.y(), p.z() ] )


def grid( gid, isurf = 0, nu = 61, nw = 49, u0 = 0.0, u1 = 1.0 ):
    """Surface isurf sampled on a U x W grid, W running round the section."""
    U = np.linspace( u0, u1, nu )
    W = np.linspace( 0.0, 1.0, nw )
    return np.array( [ [ pnt( gid, isurf, a, b ) for b in W ] for a in U ] )


def rot( gid ):
    """The absolute rotation of a Geom as a matrix: XRot, YRot, ZRot applied as Rx Ry Rz."""
    def R( axis, a ):
        a = np.radians( a )
        c, s = np.cos( a ), np.sin( a )
        if axis == 0:
            return np.array( [ [ 1, 0, 0 ], [ 0, c, -s ], [ 0, s, c ] ] )
        if axis == 1:
            return np.array( [ [ c, 0, s ], [ 0, 1, 0 ], [ -s, 0, c ] ] )
        return np.array( [ [ c, -s, 0 ], [ s, c, 0 ], [ 0, 0, 1 ] ] )
    a = [ vsp.GetParmVal( gid, n, "XForm" ) for n in ( "X_Rotation", "Y_Rotation", "Z_Rotation" ) ]
    return R( 0, a[0] ) @ R( 1, a[1] ) @ R( 2, a[2] )


def loc( gid ):
    return np.array( [ vsp.GetParmVal( gid, n, "XForm" ) for n in ( "X_Location", "Y_Location", "Z_Location" ) ] )


def degen_subsurfs( path ):
    """The SUBSURF_BNDY outlines of a DegenGeom CSV file: a list of ( name, uw, xyz ) per outline."""
    out = []
    lines = open( path ).read().splitlines()
    k = 0
    name = None
    while k < len( lines ):
        L = lines[k]
        if L.startswith( "SUBSURF," ):
            name = L.split( "," )[1]
        if L.startswith( "SUBSURF_BNDY" ):
            n = int( L.split( "," )[1] )
            rows = np.array( [ [ float( v ) for v in lines[ k + 2 + i ].split( "," ) ] for i in range( n ) ] )
            out.append( ( name, rows[ :, :2 ], rows[ :, 2: ] ) )
            k += 2 + n
            continue
        k += 1
    return out


def degen_outlines():
    """Write DegenGeom for the whole model and return its sub-surface outlines."""
    fd, path = tempfile.mkstemp( suffix = ".csv" )
    os.close( fd )
    vsp.SetComputationFileName( vsp.DEGEN_GEOM_CSV_TYPE, path )
    vsp.ComputeDegenGeom( vsp.SET_ALL, vsp.DEGEN_GEOM_CSV_TYPE )
    out = degen_subsurfs( path )
    os.remove( path )
    return out


# ---------------------------------------------------------------- figure helpers

def new( fig, v ):
    return fl.Canvas( fig.add_axes( [ 0, 0, 1, 1 ] ), v )


def save( fig, name, rows, scale = None, hmax = fl.HMAX ):
    fl.fit( fig, rows, scale = scale, hmax = hmax )
    os.makedirs( OUT, exist_ok = True )
    fig.savefig( os.path.join( OUT, name + ".png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made", name )


def key( c, which, at = ( 1.0, 0.0 ), dx = 0.0, dy = 0.0 ):
    """A small 2D axis key, dx, dy points from the point at fractions 'at' of the canvas's content box."""
    lo, hi = c.limits()
    q = lo + np.asarray( at, float ) * ( hi - lo )
    fig = c.ax.figure

    def T( x, y ):
        return offset_copy( c.ax.transData, fig, x = dx + x, y = dy + y, units = "points" )
    L = 22.0
    r, u, o = KEY[ which ]
    for ( x, y ), lab, ha, va, tx, ty in ( ( ( L, 0 ), r, "left", "center", L + 3, 0 ),
                                           ( ( 0, L ), u, "center", "bottom", 0, L + 2 ) ):
        c.ax.annotate( "", xy = q, xycoords = T( x, y ), xytext = q, textcoords = T( 0, 0 ), annotation_clip = False,
                       arrowprops = dict( arrowstyle = "-|>", lw = 0.8, color = "k", mutation_scale = 6, shrinkA = 0,
                                          shrinkB = 0 ) )
        c.ax.text( q[0], q[1], lab, transform = T( tx, ty ), ha = ha, va = va )
    c.ax.plot( [ q[0] ], [ q[1] ], transform = T( 0, 0 ), marker = "o", ms = 5.5, mfc = "white", mec = "k", mew = 0.7,
               zorder = 90 )
    c.ax.plot( [ q[0] ], [ q[1] ], transform = T( 0, 0 ), marker = "o", ms = 1.4, mfc = "k", mec = "k", zorder = 91 )
    c.ax.text( q[0], q[1], o, transform = T( -5, -5 ), ha = "right", va = "top" )


def title( c, s, pad = 10.0 ):
    lo, hi = c.limits()
    c.ax.annotate( s, xy = ( 0.5 * ( lo[0] + hi[0] ), hi[1] ), xytext = ( 0, pad ), textcoords = "offset points",
                   ha = "center", va = "bottom", annotation_clip = False )


def origin_mark( c, p, label, color = "k", dx = -6, dy = -8, ha = "right" ):
    q = c.v.p2( p )
    c.ax.plot( [ q[0] ], [ q[1] ], marker = "o", ms = 4.0, color = color, zorder = 70, mec = "white", mew = 0.6 )
    c.text( p, label, color = color, dx = dx, dy = dy, ha = ha )


# ---------------------------------------------------------------- Figure: attachment by Comp

ATT_FUSE = dict( X = 4.0, Y = -14.0, ZROT = 20.0 )
ATT_REL = ( 12.0, -5.0, 0.0 )


def attach_model( trans, rotate ):
    """A fuselage moved and turned, with a pod child at ATT_REL attached as given.  Returns ( fuse, pod )."""
    vsp.VSPRenew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    setp( f, "X_Rel_Location", "XForm", ATT_FUSE[ "X" ] )
    setp( f, "Y_Rel_Location", "XForm", ATT_FUSE[ "Y" ] )
    setp( f, "Z_Rel_Rotation", "XForm", ATT_FUSE[ "ZROT" ] )
    p = vsp.AddGeom( "POD", f )
    vsp.Update()
    setp( p, "Length", "Design", 6.0 )
    setp( p, "FineRatio", "Design", 6.0 )
    setp( p, "Trans_Attach_Flag", "Attach", trans )
    setp( p, "Rots_Attach_Flag", "Attach", rotate )
    for n, v in zip( ( "X_Rel_Location", "Y_Rel_Location", "Z_Rel_Location" ), ATT_REL ):
        setp( p, n, "XForm", v )
    vsp.Update()
    return f, p


def attach_comp():
    cases = [ ( vsp.ATTACH_TRANS_NONE, vsp.ATTACH_ROT_NONE, "None" ),
              ( vsp.ATTACH_TRANS_COMP, vsp.ATTACH_ROT_NONE, "Translate Comp" ),
              ( vsp.ATTACH_TRANS_COMP, vsp.ATTACH_ROT_COMP, "Translate and\nRotate Comp" ) ]
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = new( fig, TOP )
    f = None
    for k, ( t, r, lab ) in enumerate( cases ):
        f, p = attach_model( t, r )
        if k == 0:
            G = grid( f, nu = 81 )
            c.surface( G, color = PARENT )
            fo = loc( f )
            fR = rot( f )
        G = grid( p, nu = 41, nw = 33 )
        c.surface( G, color = CHILD )
        po = loc( p )
        # The relative location, drawn from the origin of the axes it is measured in.
        if t == vsp.ATTACH_TRANS_NONE:
            o, R = O, np.eye( 3 )
        elif r == vsp.ATTACH_ROT_NONE:
            o, R = fo, np.eye( 3 )
        else:
            o, R = fo, fR
        t_vec = R @ np.array( ATT_REL )
        assert np.abs( o + t_vec - po ).max() < 1e-9, ( lab, o + t_vec, po )
        c.arrow( o, po, color = FRAME, lw = 1.0, head = 7, ls = ( 0, ( 4, 2 ) ) )
        tip = pnt( p, 0, 1.0, 0.0 )
        c.text( tip, lab, dx = 8, dy = 0, ha = "left", va = "center" )
    c.flush()
    # Model axes at the model origin; the fuselage's axes at its origin.
    c.triad( O, [ X, Y ], [ r"$X$", r"$Y$" ], "k", length = 3.5, lw = 1.2 )
    origin_mark( c, O, "model origin", dx = -6, dy = -10 )
    c.triad( fo, [ fR @ X, fR @ Y ], [ r"$X_p$", r"$Y_p$" ], FRAME, length = 3.5, lw = 1.2 )
    origin_mark( c, fo, "fuselage origin", color = FRAME, dx = -6, dy = -10 )
    key( c, "top", at = ( 1.0, 0.0 ), dx = -40, dy = 4 )
    save( fig, "XFormTab_attach_comp", [ [ c ] ] )


# ---------------------------------------------------------------- Figure: UW attachment axes

UW_PTS = ( ( 0.3, 0.75, "top", 6, -12, "left" ), ( 0.55, 0.5, r"$-Y$ side", 14, 0, "left" ) )


def uw_frame( f, u, w ):
    """Origin and axes of the UW attachment at ( u, w ), read from a child attached there."""
    b = vsp.AddGeom( "BLANK", f )
    vsp.Update()
    setp( b, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    setp( b, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_UV )
    setp( b, "U_Attach_Location", "Attach", u )
    setp( b, "V_Attach_Location", "Attach", w )
    vsp.Update()
    o, R = loc( b ), rot( b )
    vsp.DeleteGeom( b )
    vsp.Update()
    return o, R


def attach_uw():
    vsp.VSPRenew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = new( fig, ISO )
    G = grid( f, nu = 81, nw = 49 )
    c.surface( G, color = PARENT )
    c.flush()
    for u, w, where, ldx, ldy, lha in UW_PTS:
        # The station U, as a line round the body.
        ring = np.array( [ pnt( f, 0, u, t ) for t in np.linspace( 0, 1, 97 ) ] )
        c.line( ring, color = "#7A7A7A", lw = 0.8, ls = ( 0, ( 3, 2 ) ), z = 52 )
        o, R = uw_frame( f, u, w )
        assert np.abs( o - pnt( f, 0, u, w ) ).max() < 1e-9
        n = vsp.CompNorm01( f, 0, u, w )
        n = fl.unit( [ n.x(), n.y(), n.z() ] )
        assert ( R @ Z ) @ n > 0.999, ( w, R @ Z, n )
        assert ( R @ X ) @ X > 0.9
        vecs = [ R @ X, R @ Y, R @ Z ]
        c.triad( o, vecs, [ r"$x$", r"$y$", r"$z$" ], FRAME, length = 3.0, lw = 1.3, label_pad = 1.25 )
        origin_mark( c, o, r"$U = %g$, $W = %g$ (%s)" % ( u, w, where ), color = FRAME, dx = ldx, dy = ldy, ha = lha )
    c.triad( np.array( [ -4.0, 4.0, -3.0 ] ), [ X, Y, Z ], [ r"$X$", r"$Y$", r"$Z$" ], "k", length = 2.5, lw = 1.1 )
    c.text( np.array( [ -4.0, 4.0, -3.0 ] ), "model axes", dy = -14 )
    save( fig, "XFormTab_attach_uw", [ [ c ] ] )


# ---------------------------------------------------------------- Figure: U and L along a fuselage

UL_SECT = ( 0.1, 0.25 )         # fractions of the length for sections 1 and 2; 0, 0.75 and 1 stay as built


def xform_ul_fuse():
    """The default fuselage with its sections moved to unequal stations, and a Blank child to read points with."""
    vsp.VSPRenew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    xs = vsp.GetXSecSurf( f, 0 )
    for i, x in zip( ( 1, 2 ), UL_SECT ):
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xs, i ), "XLocPercent" ), x )
        vsp.Update()
    b = vsp.AddGeom( "BLANK", f )
    vsp.Update()
    return f, b


def xform_u_and_l():
    f, b = xform_ul_fuse()
    length = vsp.GetParmVal( f, "Length", "Design" )
    vals = ( 0.0, 0.25, 0.5, 0.75, 1.0 )
    fig = plt.figure( figsize = ( fl.PAGE_W, 3.0 ) )
    c = new( fig, FRONT )
    c.surface( grid( f, nu = 121, nw = 49 ), color = PARENT )
    c.flush()
    top = max( pnt( f, 0, u, 0.75 )[2] for u in np.linspace( 0, 1, 61 ) )
    bot = -top
    for v in vals:
        # U: a whole number of sections; the station is where that section was placed.
        pr = at_rst( b, v, 0.5, 0.5 )
        ring = np.array( [ pnt( f, 0, v, t ) for t in np.linspace( 0, 1, 97 ) ] )
        assert np.abs( ring[ :, 0 ] - pr[0] ).max() < 1e-6
        c.line( np.array( [ [ pr[0], 0, top + 0.6 ], [ pr[0], 0, pr[2] ] ] ), color = "#7A7A7A", lw = 0.8,
                ls = ( 0, ( 3, 2 ) ), z = 52 )
        c.text( np.array( [ pr[0], 0, top + 0.6 ] ), r"$U = %g$" % v, dy = 8, va = "bottom" )
        # L: a fraction of the length along the center line.
        pl = at_lmn( b, v, 0.5, 0.5 )
        assert abs( pl[0] - v * length ) < 1e-3 * length, ( v, pl[0] )
        c.line( np.array( [ [ pl[0], 0, bot - 0.6 ], [ pl[0], 0, pl[2] ] ] ), color = FRAME, lw = 0.8,
                ls = ( 0, ( 3, 2 ) ), z = 52 )
        c.text( np.array( [ pl[0], 0, bot - 0.6 ] ), r"$L = %g$" % v, color = FRAME, dy = -8, va = "top" )
        for p, col in ( ( pr, "k" ), ( pl, FRAME ) ):
            q = c.v.p2( p )
            c.ax.plot( [ q[0] ], [ q[1] ], marker = "o", ms = 4, color = col, zorder = 70, mec = "white", mew = 0.5 )
    key( c, "front", at = ( 1.0, 0.5 ), dx = 30, dy = -10 )
    save( fig, "XFormTab_u_and_l", [ [ c ] ] )


# ---------------------------------------------------------------- Figure: RST and LMN on a wing section

RST_TC = 0.2


def rst_wing():
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    setp( w, "Sym_Planar_Flag", "Sym", 0 )
    xs = vsp.GetXSecSurf( w, 0 )
    for i in ( 0, 1 ):
        vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( xs, i ), "ThickChord" ), RST_TC )
    vsp.Update()
    b = vsp.AddGeom( "BLANK", w )
    vsp.Update()
    return w, b


def at_rst( b, r, s, t ):
    setp( b, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_RST )
    setp( b, "R_Attach_Location", "Attach", r )
    setp( b, "S_Attach_Location", "Attach", s )
    setp( b, "T_Attach_Location", "Attach", t )
    vsp.Update()
    return loc( b )


def at_lmn( b, l, m, n ):
    setp( b, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_LMN )
    setp( b, "L_Attach_Location", "Attach", l )
    setp( b, "M_Attach_Location", "Attach", m )
    setp( b, "N_Attach_Location", "Attach", n )
    vsp.Update()
    return loc( b )


def attach_rst():
    w, b = rst_wing()
    R0 = 1.0 / 3.0                              # the root section: the root cap spans U 0 to 1 of 3
    sec = np.array( [ pnt( w, 0, R0, t ) for t in np.linspace( 0, 1, 241 ) ] )
    setp( b, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_RST )
    setp( b, "R_Attach_Location", "Attach", R0 )
    vsp.Update()
    L0 = vsp.GetParmVal( b, "L_Attach_Location", "Attach" )
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    cs = new( fig, FRONT )
    cs.line( np.vstack( [ sec, sec[ :1 ] ] ), color = "#333333", lw = 1.2, z = 20 )
    vals = ( 0.0, 0.25, 0.5, 0.75, 1.0 )
    # S picks a point on each surface; T runs between them.
    for s in vals:
        lo = at_rst( b, R0, s, 0.0 )
        up = at_rst( b, R0, s, 1.0 )
        assert np.abs( lo - pnt( w, 0, R0, 0.5 * s ) ).max() < 1e-6
        assert np.abs( up - pnt( w, 0, R0, 1.0 - 0.5 * s ) ).max() < 1e-6
        cs.line( np.array( [ lo, up ] ), color = FRAME, lw = 0.9, ls = ( 0, ( 3, 2 ) ), z = 30 )
        for p in ( lo, up ):
            q = cs.v.p2( p )
            cs.ax.plot( [ q[0] ], [ q[1] ], marker = "o", ms = 4, color = FRAME, zorder = 40, mec = "white", mew = 0.5 )
        if s == 1.0:
            # The leading edge: put the label clear of the curve, above and to the left.
            cs.text( up, r"$S = %g$" % s, color = FRAME, dx = -4, dy = 11, ha = "right" )
        else:
            cs.text( up, r"$S = %g$" % s, color = FRAME, dx = 0, dy = 11 )
    for t, dy in ( ( 0.0, -9 ), ( 0.5, 0 ), ( 1.0, -9 ) ):
        p = at_rst( b, R0, 0.5, t )
        if t == 0.5:
            q = cs.v.p2( p )
            cs.ax.plot( [ q[0] ], [ q[1] ], marker = "o", ms = 4, color = "k", zorder = 41, mec = "white", mew = 0.5 )
        cs.text( p, r"$T = %g$" % t, dx = 7, dy = dy, ha = "left" )
    # M is the fraction of the chord from the trailing edge, on the line midway between the surfaces.
    te = pnt( w, 0, R0, 0.0 )
    le = pnt( w, 0, R0, 0.5 )
    chord = np.linalg.norm( le - te )
    for m in vals:
        p = at_lmn( b, L0, m, 0.5 )
        assert abs( np.linalg.norm( p - te ) / chord - m ) < 2e-3, ( m, np.linalg.norm( p - te ) / chord )
    cs.text( te, "trailing edge", dx = 10, dy = 0, ha = "left" )
    cs.text( le, "leading edge", dx = -10, dy = 0, ha = "right" )
    cs.text( pnt( w, 0, R0, 0.31 ), "lower surface", dx = 0, dy = -14 )
    cs.text( pnt( w, 0, R0, 0.69 ), "upper surface", dx = 0, dy = 26 )
    key( cs, "front", at = ( 0.0, 0.0 ), dx = -60, dy = -6 )
    save( fig, "XFormTab_attach_rst", [ [ cs ] ] )


# ---------------------------------------------------------------- Figure: symmetry frame

SYM_FUSE = dict( Y = 4.0, ZROT = 10.0 )


def sym_model( ancestor ):
    vsp.VSPRenew()
    f = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    setp( f, "Y_Rel_Location", "XForm", SYM_FUSE[ "Y" ] )
    setp( f, "Z_Rel_Rotation", "XForm", SYM_FUSE[ "ZROT" ] )
    w = vsp.AddGeom( "WING", f )
    vsp.Update()
    setp( w, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_COMP )
    setp( w, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_COMP )
    setp( w, "X_Rel_Location", "XForm", 10.0 )
    setp( w, "Sym_Ancestor", "Sym", ancestor )
    vsp.Update()
    return f, w


def sym_frame():
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    rows = []
    for anc, lab in ( ( 1, "About 1 (the wing), Attach: the default" ), ( 0, "About 0, the global origin" ) ):
        f, w = sym_model( anc )
        c = new( fig, TOP )
        c.surface( grid( f, nu = 81 ), color = PARENT )
        c.surface( grid( w, 0, nu = 41, nw = 33 ), color = CHILD )
        c.surface( grid( w, 1, nu = 41, nw = 33 ), color = CHILD )
        c.flush()
        fo, fR = loc( f ), rot( f )
        # The plane of symmetry, seen edge on: a line through the frame's origin along its X.
        if anc == 1:
            po, d = fo, fR @ X
            # The copy is the reflection of the wing in that plane.
            nrm = fR @ Y
        else:
            po, d = O, X
            nrm = Y
        a = pnt( w, 0, 1.0, 0.5 )
        b = pnt( w, 1, 1.0, 0.5 )
        refl = a - 2.0 * ( ( a - po ) @ nrm ) * nrm
        assert np.abs( refl - b ).max() < 1e-9, ( anc, refl, b )
        L = np.array( [ po - 6.0 * d, po + 36.0 * d ] )
        c.line( L, color = FRAME, lw = 1.2, ls = ( 0, ( 8, 3 ) ), z = 55 )
        c.text( po + 36.0 * d, "plane of symmetry", color = FRAME, dx = 6, dy = 0, ha = "left" )
        c.text( a, "wing", dx = 10, dy = 0, ha = "left" )
        c.text( b, "copy", dx = 10, dy = 0, ha = "left" )
        origin_mark( c, O, "model origin", dx = -6, dy = -10 )
        title( c, lab, pad = 12 )
        rows.append( [ c ] )
    key( rows[ -1 ][ 0 ], "top", at = ( 1.0, 0.0 ), dx = -40, dy = 4 )
    save( fig, "XFormTab_sym_frame", rows, hmax = 7.5 )


# ---------------------------------------------------------------- Figure: a flip, then a turn

FLIP_XROT = 30.0
NOSE = fl.View.plane( -Y, Z )                                 # looking along +X: -Y right, Z up


def flip_wing( flip, sym ):
    """The default Wing turned by XRot FLIP_XROT, with the flip and planar symmetry given.  Returns its surfaces."""
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    setp( w, "Sym_Planar_Flag", "Sym", sym )
    setp( w, "Flip_Flag", "Sym", flip )
    setp( w, "X_Rel_Rotation", "XForm", FLIP_XROT )
    vsp.Update()
    return [ grid( w, k, nu = 31, nw = 33 ) for k in range( vsp.GetNumMainSurfs( w ) * ( 2 if sym else 1 ) ) ]


def xform_flip():
    KEY[ "nose" ] = ( r"$-Y$", r"$Z$", r"$-X$" )
    cases = ( ( 0, 0, "Unflipped" ), ( vsp.SYM_XZ, 0, "Flip XZ" ), ( 0, vsp.SYM_XZ, "Planar XZ symmetry, unflipped" ) )
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    rows = []
    ext = 0.0
    for flip, sym, lab in cases:
        Gs = flip_wing( flip, sym )
        # Measure which way each tip goes: the tip leading edge of every surface.
        tips = [ G[ -1, G.shape[1] // 2 ] for G in Gs ]
        for t in tips:
            ext = max( ext, abs( t[1] ) )
        if flip:
            assert len( tips ) == 1 and tips[0][1] < 0 and tips[0][2] < 0, "a flipped wing turned by XRot 30 tips down"
        elif sym:
            assert len( tips ) == 2 and all( t[2] > 0 for t in tips ) and tips[0][1] * tips[1][1] < 0, "both up"
        else:
            assert len( tips ) == 1 and tips[0][1] > 0 and tips[0][2] > 0, "the wing tips up"
        c = new( fig, NOSE )
        for G in Gs:
            c.surface( G, color = CHILD )
        c.flush()
        for t in tips:
            side = "right" if t[1] < 0 else "left"
            c.text( t, "tip up" if t[2] > 0 else "tip down", dx = 8 if side == "right" else -8, ha = "left" if side == "right" else "right" )
        rows.append( ( c, lab, min( t[2] for t in tips ), max( t[2] for t in tips ) ) )
    out = []
    for c, lab, zlo, zhi in rows:
        # The model's Y axis through the origin, and the same extent in every row, so the rows line up.
        c.line( np.array( [ -1.08 * ext * Y, 1.08 * ext * Y ] ), color = "#888888", lw = 0.7, ls = ( 0, ( 5, 3 ) ), z = 40 )
        c.text( 1.08 * ext * Y, r"$+Y$", color = "#666666", dx = -10, ha = "right" )
        c.text( -1.08 * ext * Y, r"$-Y$", color = "#666666", dx = 10, ha = "left" )
        c.bounds = ( np.array( [ -1.08 * ext, min( zlo, 0.0 ) - 0.1 * ext ] ), np.array( [ 1.08 * ext, max( zhi, 0.0 ) + 0.05 * ext ] ) )
        title( c, lab, pad = 8 )
        out.append( [ c ] )
    key( out[ 0 ][ 0 ], "nose", at = ( 1.0, 1.0 ), dx = -40, dy = -40 )
    save( fig, "XFormTab_flip", out, hmax = 5.6 )


# ---------------------------------------------------------------- Figure: sub-surfaces in U and W

SS_POD = dict( Length = 10.0, FineRatio = 4.0 )
SS_LINE_U = 0.2


def ss_uw():
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    for k, v in SS_POD.items():
        setp( p, k, "Design", v )
    vsp.Update()
    ln = vsp.AddSubSurf( p, vsp.SS_LINE, -1 )
    rc = vsp.AddSubSurf( p, vsp.SS_RECTANGLE, -1 )
    el = vsp.AddSubSurf( p, vsp.SS_ELLIPSE, -1 )
    vsp.Update()

    def ssp( sid, name, val ):
        ids = { vsp.GetParmName( q ): q for q in vsp.GetSubSurfParmIDs( sid ) }
        vsp.SetParmVal( ids[ name ], float( val ) )
    ssp( ln, "Const_Line_Value", SS_LINE_U )
    ssp( rc, "Center_U", 0.68 )
    ssp( rc, "Center_W", 0.86 )
    ssp( rc, "Theta", 30.0 )
    ssp( el, "Center_U", 0.35 )
    ssp( el, "Center_W", 0.75 )
    ssp( el, "U_Length", 0.15 )
    ssp( el, "W_Length", 0.15 )
    ssp( el, "Tess_Num", 41 )
    vsp.Update()
    names = { vsp.GetSubSurfName( ln ): "line", vsp.GetSubSurfName( rc ): "rectangle", vsp.GetSubSurfName( el ): "ellipse" }
    outl = degen_outlines()
    umax = 2.0
    wmax = 4.0
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    UW = fl.View.plane( X, Y )                  # the U-W plane drawn with U right, W up
    c2 = fl.Canvas( fig.add_axes( [ 0, 0, 0.5, 1 ] ), UW )
    c3 = fl.Canvas( fig.add_axes( [ 0.5, 0, 0.5, 1 ] ), ISO )
    S = 4.0                                     # the U-W square drawn S units on a side
    sq = S * np.array( [ [ 0, 0, 0 ], [ 1, 0, 0 ], [ 1, 1, 0 ], [ 0, 1, 0 ], [ 0, 0, 0 ] ] )
    c2.line( sq, color = "#555555", lw = 0.9, z = 20 )
    for v in ( 0.0, 1.0 ):
        c2.text( S * np.array( [ v, 0, 0 ] ), "%g" % v, dy = -9 )
        c2.text( S * np.array( [ 0, v, 0 ] ), "%g" % v, dx = -8 )
    c2.text( S * np.array( [ 0.5, 0, 0 ] ), r"$U$", dy = -12 )
    c2.text( S * np.array( [ 0, 0.5, 0 ] ), r"$W$", dx = -12 )
    G = grid( p, nu = 61, nw = 61 )
    c3.surface( G, color = PARENT )
    seen = set()
    for name, uw, xyz in outl:
        if name not in names or name in seen:
            continue
        seen.add( name )
        lab = names[ name ]
        u = uw[ :, 0 ] / umax
        wv = uw[ :, 1 ] / wmax
        # The outline from DegenGeom is the surface at its own u, w.
        for k in range( 0, len( u ), max( 1, len( u ) // 8 ) ):
            assert np.abs( pnt( p, 0, u[k], wv[k] ) - xyz[k] ).max() < 1e-6
        if lab == "line":
            # DegenGeom gives a line by its two ends, which meet on a closed body; draw it along W.
            assert len( u ) == 2 and abs( u[0] - SS_LINE_U ) < 1e-12 and abs( u[1] - SS_LINE_U ) < 1e-12
            wv = np.linspace( wv[0], wv[1], 97 )
            u = np.full_like( wv, u[0] )
            xyz = np.array( [ pnt( p, 0, a, b ) for a, b in zip( u, wv ) ] )
        else:
            # Straight in U and W between the outline's points; fill in points so the outline follows the surface.
            k = np.arange( len( u ) )
            t = np.linspace( 0, len( u ) - 1, 20 * ( len( u ) - 1 ) + 1 )
            u = np.interp( t, k, u )
            wv = np.interp( t, k, wv )
            xyz = np.array( [ pnt( p, 0, a, b ) for a, b in zip( u, wv ) ] )
        c2.line( S * np.column_stack( [ u, wv, 0 * u ] ), color = SSCOL, lw = 1.4, z = 30 )
        c3.segs( xyz, SSCOL, 1.4, bias = 0.25 )
        if lab == "line":
            m = int( 0.12 * len( u ) )
            c2.text( S * np.array( [ u[m], wv[m], 0 ] ), lab, color = SSCOL, dx = 5, dy = 0, ha = "left" )
        else:
            # Below the outline, so the label does not cross it.
            m = np.argmin( wv )
            c2.text( S * np.array( [ u.mean(), wv[m], 0 ] ), lab, color = SSCOL, dx = 0, dy = -10 )
        if lab == "line":
            c3.text( xyz[ int( 0.62 * len( u ) ) ], lab, color = SSCOL, dx = -6, dy = 6, ha = "right" )
        elif lab == "rectangle":
            c3.text( xyz[ np.argmax( xyz[ :, 2 ] ) ], lab, color = SSCOL, dx = 0, dy = 12 )
        else:
            c3.text( xyz[ np.argmin( xyz[ :, 2 ] ) ], lab, color = SSCOL, dx = 0, dy = -12 )
    assert len( seen ) == 3, seen
    c3.flush()
    save( fig, "SubTab_ss_uw", [ [ c2, c3 ] ] )


# ---------------------------------------------------------------- Figure: control surface

def ss_control():
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    setp( w, "Sym_Planar_Flag", "Sym", 0 )
    cs = vsp.AddSubSurf( w, vsp.SS_CONTROL )
    vsp.Update()
    outl = [ o for o in degen_outlines() if o[0] == vsp.GetSubSurfName( cs ) ]
    assert outl
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = new( fig, TOP )
    G = grid( w, 0, nu = 61, nw = 49 )
    c.surface( G, color = PARENT )
    c.flush()
    name, uw, xyz = outl[0]
    c.line( xyz, color = SSCOL, lw = 1.6, z = 60 )
    # The panel runs U 1 to 2 of 3; the default Start U 0.4 and End U 0.6 fall at 20% and 80% of the span.
    span = pnt( w, 0, 2.0 / 3.0, 0.0 )[1]
    ys = ( pnt( w, 0, 0.4, 0.0 )[1], pnt( w, 0, 0.6, 0.0 )[1] )
    assert abs( ys[0] / span - 0.2 ) < 1e-9 and abs( ys[1] / span - 0.8 ) < 1e-9
    for u, lab in ( ( 0.4, "Start U 0.4" ), ( 0.6, "End U 0.6" ) ):
        c.text( pnt( w, 0, u, 1.0 ), lab, dx = 8, dy = -2, ha = "left", va = "top" )
    # The hinge points: seen from above, a quarter of the chord from the trailing edge, on the outline.
    te_s = pnt( w, 0, 0.4, 1.0 )
    te_e = pnt( w, 0, 0.6, 1.0 )
    hs = te_s + 0.25 * ( pnt( w, 0, 0.4, 0.5 ) - te_s )
    he = te_e + 0.25 * ( pnt( w, 0, 0.6, 0.5 ) - te_e )
    for h, te in ( ( hs, te_s ), ( he, te_e ) ):
        d = np.linalg.norm( ( xyz - h )[ :, :2 ], axis = 1 ).min()
        assert d < 0.02 * np.linalg.norm( h - te ), d
    c.line( np.array( [ hs - 0.15 * ( he - hs ), hs ] ), color = SSCOL, lw = 0.9, ls = ( 0, ( 4, 2 ) ), z = 61 )
    c.text( hs - 0.15 * ( he - hs ), "hinge line", color = SSCOL, dx = -4, dy = 0, ha = "right", va = "center" )
    c.text( 0.5 * ( he + te_e ), "25% chord", color = SSCOL, dx = 0, dy = 12 )
    c.text( pnt( w, 0, 0.5, 0.5 ), "leading edge", dx = -10, dy = 0, ha = "right" )
    c.text( pnt( w, 0, 0.55, 0.0 ), "trailing edge", dx = 10, dy = 0, ha = "left" )
    key( c, "top", at = ( 1.0, 0.0 ), dx = -40, dy = 4 )
    save( fig, "SubTab_ss_control", [ [ c ] ] )


FIGS = {
    "attach_comp": attach_comp,
    "attach_uw": attach_uw,
    "u_and_l": xform_u_and_l,
    "attach_rst": attach_rst,
    "sym_frame": sym_frame,
    "flip": xform_flip,
    "ss_uw": ss_uw,
    "ss_control": ss_control,
}

if __name__ == "__main__":
    names = sys.argv[ 1: ] or list( FIGS )
    bad = [ n for n in names if n not in FIGS ]
    if bad:
        sys.exit( "unknown figure %s; the figures are: %s" % ( ", ".join( bad ), " ".join( FIGS ) ) )
    fl.style()
    for n in names:
        FIGS[ n ]()
