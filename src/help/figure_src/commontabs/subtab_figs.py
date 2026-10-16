"""Further figures for the Sub tab help page (src/help/SubTab.md): Ellipse polygons and control surface deflection.

    <build>/venv/bin/python subtab_figs.py [names...]   # writes ../../images/SubTab_*.png

The helpers (views, DegenGeom readers, layout) are those of commontabs_figs.py.
"""

import os
import sys
import tempfile

import numpy as np
import matplotlib.pyplot as plt

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, HERE )

import commontabs_figs as cf                                  # noqa: E402
from commontabs_figs import fl, vsp, pnt, setp, X, Y, Z       # noqa: E402

SSCOL = cf.SSCOL


def ssp( sid, name, val ):
    ids = { vsp.GetParmName( q ): q for q in vsp.GetSubSurfParmIDs( sid ) }
    vsp.SetParmVal( ids[ name ], float( val ) )


def degen_file():
    fd, path = tempfile.mkstemp( suffix = ".csv" )
    os.close( fd )
    vsp.SetComputationFileName( vsp.DEGEN_GEOM_CSV_TYPE, path )
    vsp.ComputeDegenGeom( vsp.SET_ALL, vsp.DEGEN_GEOM_CSV_TYPE )
    return path


def degen_hinges( path ):
    """The HINGELINE records of a DegenGeom CSV file, in file order (one per surface copy): a list of
    ( name, start xyz, end xyz ), upper and lower hinge points averaged as VSPAERO averages them."""
    out = []
    lines = open( path ).read().splitlines()
    for k, L in enumerate( lines ):
        if L.startswith( "HINGELINE," ):
            n = int( L.split( "," )[2] )
            rows = np.array( [ [ float( v ) for v in lines[ k + 2 + i ].split( "," ) ] for i in range( n ) ] )
            out.append( ( L.split( "," )[1], rows[ :, 4:7 ].mean( 0 ), rows[ :, 7:10 ].mean( 0 ) ) )
    return out


def rotate( P, o, axis, deg ):
    """P turned right-handed about the line through o along axis, by deg degrees (Rodrigues)."""
    k = fl.unit( axis )
    t = np.radians( deg )
    V = np.asarray( P, float ) - o
    R = V * np.cos( t ) + np.cross( k, V ) * np.sin( t ) + np.outer( V @ k, k ) * ( 1 - np.cos( t ) )
    return R + o


# ---------------------------------------------------------------- Figure: Ellipse polygons

POLYS = ( ( 3, 0.0, "3" ), ( 3, 60.0, "3, Theta 60" ), ( 6, 0.0, "6" ), ( 6, 30.0, "6, Theta 30" ) )
PU = ( 0.29, 0.43, 0.57, 0.71 )
PW = 0.8
PL = 0.1


def ss_polygon():
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.Update()
    for k, v in cf.SS_POD.items():
        setp( p, k, "Design", v )
    vsp.Update()
    names = {}
    for ( n, th, lab ), u in zip( POLYS, PU ):
        e = vsp.AddSubSurf( p, vsp.SS_ELLIPSE, -1 )
        vsp.Update()
        ssp( e, "Tess_Num", n )
        ssp( e, "Theta", th )
        ssp( e, "Center_U", u )
        ssp( e, "Center_W", PW )
        ssp( e, "U_Length", PL )
        ssp( e, "W_Length", PL )
        names[ vsp.GetSubSurfName( e ) ] = ( n, th, lab, u )
    vsp.Update()
    outl = cf.degen_outlines()
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    UW = fl.View.plane( X, Y )                  # the U-W plane drawn with U right, W up
    c2 = fl.Canvas( fig.add_axes( [ 0, 0, 0.5, 1 ] ), UW )
    c3 = fl.Canvas( fig.add_axes( [ 0.5, 0, 0.5, 1 ] ), cf.ISO )
    S = 8.0                                     # U 0 to 1 drawn S units long
    W0, W1 = 0.6, 1.0                           # the band of W drawn
    box = np.array( [ [ 0, W0, 0 ], [ 1, W0, 0 ], [ 1, W1, 0 ], [ 0, W1, 0 ], [ 0, W0, 0 ] ] )
    c2.line( S * box, color = "#555555", lw = 0.9, z = 20 )
    for v in ( 0.0, 1.0 ):
        c2.text( S * np.array( [ v, W0, 0 ] ), "%g" % v, dy = -9 )
    for v in ( W0, W1 ):
        c2.text( S * np.array( [ 0, v, 0 ] ), "%g" % v, dx = -10 )
    c2.text( S * np.array( [ 0.5, W0, 0 ] ), r"$U$", dy = -12 )
    c2.text( S * np.array( [ 0, 0.5 * ( W0 + W1 ), 0 ] ), r"$W$", dx = -12 )
    c3.surface( cf.grid( p, nu = 61, nw = 61 ), color = cf.PARENT )
    seen = set()
    for name, uw, xyz in outl:
        if name not in names or name in seen:
            continue
        seen.add( name )
        n, th, lab, uc = names[ name ]
        u = uw[ :, 0 ] / 2.0
        wv = uw[ :, 1 ] / 4.0
        assert len( u ) == n + 1, ( n, len( u ) )
        # Every corner: Theta plus a whole number of 360/N from the U axis, half the length from the center.
        ang = np.radians( th + 360.0 * np.arange( n ) / n )
        want = np.column_stack( [ 0.5 * PL * np.cos( ang ), 0.5 * PL * np.sin( ang ) ] )
        got = np.column_stack( [ u[ :-1 ] - uc, wv[ :-1 ] - PW ] )
        assert all( np.abs( got - q ).max( 1 ).min() < 1e-9 for q in want ), lab
        for k in range( len( u ) ):
            assert np.abs( pnt( p, 0, u[k], wv[k] ) - xyz[k] ).max() < 1e-6
        k = np.arange( len( u ) )
        t = np.linspace( 0, len( u ) - 1, 20 * ( len( u ) - 1 ) + 1 )
        uu = np.interp( t, k, u )
        ww = np.interp( t, k, wv )
        P = np.array( [ pnt( p, 0, a, b ) for a, b in zip( uu, ww ) ] )
        Q = S * np.column_stack( [ uu, ww, 0 * uu ] )
        c2.line( Q, color = SSCOL, lw = 1.3, z = 30 )
        c3.segs( P, SSCOL, 1.3, bias = 0.25 )
        c2.text( S * np.array( [ uc, PW - 0.5 * PL, 0 ] ), lab, color = SSCOL, dy = -10 )
        c3.text( P[ np.argmax( P[ :, 2 ] ) ], lab, color = SSCOL, dy = 10 )
    assert len( seen ) == len( POLYS ), seen
    c3.flush()
    cf.save( fig, "SubTab_ss_polygon", [ [ c2, c3 ] ] )


# ---------------------------------------------------------------- Figure: positive deflection of a mirrored pair

DEFL = 25.0
REAR = fl.View.plane( Y, [ -0.45, 0, 1 ] )     # from behind and above: +Y to the right


def flap_grid( w, isurf, uw_upper, uw_lower, nu = 25, nw = 9 ):
    """The control surface region of surface isurf: W from the upper hinge round the trailing edge to the lower one."""
    def hinge_w( uw, upper ):
        u, wv = uw[ :, 0 ], uw[ :, 1 ]
        inner = ( u > u.min() + 1e-9 ) & ( u < u.max() - 1e-9 )
        ends = []
        for ue in ( u.min(), u.max() ):
            m = np.abs( u - ue ) < 1e-9
            ends.append( ( ue, wv[ m ].min() if upper else wv[ m ].max() ) )
        pts = sorted( [ ( a, b ) for a, b in zip( u[ inner ], wv[ inner ] ) ] + ends )
        return np.array( pts )
    hu = hinge_w( uw_upper, True )
    hl = hinge_w( uw_lower, False )
    U = np.linspace( hu[ 0, 0 ], hu[ -1, 0 ], nu )
    G = []
    for u in U:
        wu = np.interp( u, hu[ :, 0 ], hu[ :, 1 ] )
        wl = np.interp( u, hl[ :, 0 ], hl[ :, 1 ] )
        ws = np.concatenate( [ np.linspace( wu, 1.0, nw ), np.linspace( 0.0, wl, nw )[ 1: ] ] )
        G.append( [ pnt( w, isurf, u, x ) for x in ws ] )
    return np.array( G )


def ss_hinge():
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.Update()
    cs = vsp.AddSubSurf( w, vsp.SS_CONTROL )
    vsp.Update()
    name = vsp.GetSubSurfName( cs )
    path = degen_file()
    outl = [ o for o in cf.degen_subsurfs( path ) if o[0] == name ]
    hinges = [ h for h in degen_hinges( path ) if h[0] == name ]
    os.remove( path )
    # Two surfaces, each with an upper and a lower outline, and a hinge line per surface.
    assert len( outl ) == 4 and len( hinges ) == 2, ( len( outl ), len( hinges ) )
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = cf.new( fig, REAR )
    for s in ( 0, 1 ):
        c.surface( cf.grid( w, s, nu = 61, nw = 49 ), color = cf.PARENT )
    labels = []
    for s in ( 0, 1 ):
        _, xs, xe = hinges[ s ]
        # Start is at the root end of the hinge on both surfaces.
        assert abs( xs[1] ) < abs( xe[1] )
        up, lo = outl[ 2 * s ], outl[ 2 * s + 1 ]
        if up[1][ :, 1 ].mean() < lo[1][ :, 1 ].mean():
            up, lo = lo, up
        F = flap_grid( w, s, up[1] / np.array( [ 3.0, 4.0 ] ), lo[1] / np.array( [ 3.0, 4.0 ] ) )
        Fd = rotate( F.reshape( -1, 3 ), xs, xe - xs, DEFL ).reshape( F.shape )
        te0 = F[ F.shape[0] // 2, F.shape[1] // 2 ]
        te1 = Fd[ F.shape[0] // 2, F.shape[1] // 2 ]
        # Gain 1 on both: the surface whose hinge runs toward +Y turns its trailing edge down, its mirror image up.
        if xe[1] > xs[1]:
            assert te1[2] < te0[2] - 0.1
        else:
            assert te1[2] > te0[2] + 0.1
        c.surface( Fd, color = "#F0B48A", edge = False )
        c.segs( Fd[ :, 0 ], SSCOL, 0.8, bias = 0.05 )
        c.segs( Fd[ :, -1 ], SSCOL, 0.8, bias = 0.05 )
        c.segs( Fd[ :, F.shape[1] // 2 ], SSCOL, 0.8, bias = 0.05 )
        c.segs( Fd[ 0 ], SSCOL, 0.8, bias = 0.05 )
        c.segs( Fd[ -1 ], SSCOL, 0.8, bias = 0.05 )
        for o in ( up, lo ):
            c.segs( o[2], SSCOL, 0.7, ls = ( 0, ( 3, 2 ) ), bias = 0.02 )
        labels.append( ( s, xs, xe, te1, te0 ) )
    c.flush()
    for s, xs, xe, te1, te0 in labels:
        mid = 0.5 * ( xs + xe )
        L = np.linalg.norm( xe - xs )
        c.hinge( mid, xe - xs, "k", half = 0.62 * L, rho = 0.45, at = 0.85, lw = 1.1, z = 80 )
        c.text( mid - 0.62 * ( xe - xs ), "Start", dx = 0, dy = 9 )
        c.text( mid + 0.62 * ( xe - xs ), "End", dx = 0, dy = 9 )
        if te1[2] < te0[2]:
            c.text( te1, "trailing edge down", color = SSCOL, dx = 0, dy = -14 )
        else:
            c.text( te1, "trailing edge up", color = SSCOL, dx = 0, dy = 14 )
    cf.save( fig, "SubTab_ss_hinge", [ [ c ] ] )


FIGS = {
    "ss_polygon": ss_polygon,
    "ss_hinge": ss_hinge,
}

if __name__ == "__main__":
    names = sys.argv[ 1: ] or list( FIGS )
    bad = [ n for n in names if n not in FIGS ]
    if bad:
        sys.exit( "unknown figure %s; the figures are: %s" % ( ", ".join( bad ), " ".join( FIGS ) ) )
    fl.style()
    for n in names:
        FIGS[ n ]()
