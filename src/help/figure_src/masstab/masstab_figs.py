"""Figures for the OpenVSP help page of the Mass tab (src/help/MassTab.md).

    <build>/venv/bin/python masstab_figs.py [names...]   # writes ../../images/MassTab_*.png

Every figure is drawn from geometry the openvsp module builds, and checks what it shows against the Mass Properties
analysis.  The drawing primitives and the page layout (one width, one text size) come from ../propeller/figlib.py.
"""

import os
import sys

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
Z = np.array( [ 0, 0, 1.0 ] )
FRONT = fl.View.plane( X, Z )

FUEL = "#56B4E9"
SKIN = "#333333"

# A body whose only mass is its skin, holding a fuel tank.
BODY = dict( Length = 10.0, FineRatio = 5.0, Mass_Area = 1.0 )
TANK = dict( Length = 4.0, FineRatio = 4.0, X = 3.0, Density = 100.0 )


def setp( gid, name, group, val ):
    vsp.SetParmVal( gid, name, group, float( val ) )


def total_mass():
    vsp.ComputeMassProps( vsp.SET_ALL, 60, vsp.X_DIR )
    r = vsp.FindLatestResultsID( "Mass_Properties" )
    return vsp.GetDoubleResults( r, "Total_Mass" )[0]


def body():
    b = vsp.AddGeom( "POD" )
    setp( b, "Length", "Design", BODY[ "Length" ] )
    setp( b, "FineRatio", "Design", BODY[ "FineRatio" ] )
    setp( b, "Density", "Mass_Props", 0.0 )
    setp( b, "Shell_Flag", "Mass_Props", 1 )
    setp( b, "Mass_Area", "Mass_Props", BODY[ "Mass_Area" ] )
    return b


def tank():
    t = vsp.AddGeom( "POD" )
    setp( t, "Length", "Design", TANK[ "Length" ] )
    setp( t, "FineRatio", "Design", TANK[ "FineRatio" ] )
    setp( t, "X_Rel_Location", "XForm", TANK[ "X" ] )
    setp( t, "Density", "Mass_Props", TANK[ "Density" ] )
    return t


def model( tank_prior ):
    """The body, then the tank, in that order; returns ( body, tank, total mass )."""
    vsp.VSPRenew()
    b = body()
    t = tank()
    setp( t, "Mass_Prior", "Mass_Props", tank_prior )
    vsp.Update()
    return b, t, total_mass()


def alone( make, den ):
    """The mass of one Geom by itself, with the given density and no shell."""
    vsp.VSPRenew()
    g = make()
    setp( g, "Shell_Flag", "Mass_Props", 0 )
    setp( g, "Density", "Mass_Props", den )
    vsp.Update()
    return total_mass()


def skin_alone():
    vsp.VSPRenew()
    body()
    vsp.Update()
    return total_mass()


def profile( gid, nu = 121, nw = 72 ):
    """The outline of an axisymmetric Geom in the plane Y = 0, as a closed loop in VSP axes."""
    top = []
    bot = []
    for u in 0.5 - 0.5 * np.cos( np.linspace( 0.0, np.pi, nu ) ):
        ring = np.array( [ [ c.x(), c.y(), c.z() ] for c in
                           ( vsp.CompPnt01( gid, 0, u, w ) for w in np.linspace( 0.0, 1.0, nw, endpoint = False ) ) ] )
        i = ring[ :, 2 ].argmax()
        k = ring[ :, 2 ].argmin()
        assert abs( ring[ i, 1 ] ) < 1e-6 and abs( ring[ k, 1 ] ) < 1e-6
        top.append( ring[i] )
        bot.append( ring[k] )
    return np.vstack( [ top, bot[ ::-1 ] ] )


def key( c, at = ( 1.0, 0.0 ), dx = 0.0, dy = 0.0 ):
    """A small 2D axis key for the front view."""
    lo, hi = c.limits()
    q = lo + np.asarray( at, float ) * ( hi - lo )
    fig = c.ax.figure

    def T( x, y ):
        return offset_copy( c.ax.transData, fig, x = dx + x, y = dy + y, units = "points" )
    L = 22.0
    for ( x, y ), lab, ha, va, tx, ty in ( ( ( L, 0 ), r"$X$", "left", "center", L + 3, 0 ),
                                           ( ( 0, L ), r"$Z$", "center", "bottom", 0, L + 2 ) ):
        c.ax.annotate( "", xy = q, xycoords = T( x, y ), xytext = q, textcoords = T( 0, 0 ), annotation_clip = False,
                       arrowprops = dict( arrowstyle = "-|>", lw = 0.8, color = "k", mutation_scale = 6, shrinkA = 0,
                                          shrinkB = 0 ) )
        c.ax.text( q[0], q[1], lab, transform = T( tx, ty ), ha = ha, va = va )
    c.ax.plot( [ q[0] ], [ q[1] ], transform = T( 0, 0 ), marker = "o", ms = 5.5, mfc = "white", mec = "k", mew = 0.7,
               zorder = 90 )
    c.ax.plot( [ q[0] ], [ q[1] ], transform = T( 0, 0 ), marker = "o", ms = 1.4, mfc = "k", mec = "k", zorder = 91 )
    c.ax.text( q[0], q[1], r"$-Y$", transform = T( -5, -5 ), ha = "right", va = "top" )


def title( c, s, pad = 8.0 ):
    lo, hi = c.limits()
    c.ax.annotate( s, xy = ( 0.5 * ( lo[0] + hi[0] ), hi[1] ), xytext = ( 0, pad ), textcoords = "offset points",
                   ha = "center", va = "bottom", annotation_clip = False )


# ---------------------------------------------------------------- Figure: priority decides who owns an overlap

def priority():
    m_skin = skin_alone()
    m_fuel = alone( tank, TANK[ "Density" ] )
    panels = []
    for prior in ( 1, 0 ):
        b, t, m = model( prior )
        if prior == 1:
            assert abs( m - ( m_skin + m_fuel ) ) < 0.02 * m_fuel, ( m, m_skin, m_fuel )
        else:
            assert abs( m - m_skin ) < 0.02 * m_fuel, ( m, m_skin )
        panels.append( ( prior, profile( b ), profile( t ), m ) )

    fig = plt.figure()
    rows = []
    for prior, PB, PT, m in panels:
        c = fl.Canvas( fig.add_axes( [ 0, 0, 1, 1 ] ), FRONT )
        qb = FRONT.p2( PB )
        qt = FRONT.p2( PT )
        c.pts.append( qb )
        c.ax.fill( qb[ :, 0 ], qb[ :, 1 ], color = "white", zorder = 2 )
        if prior == 1:
            c.ax.fill( qt[ :, 0 ], qt[ :, 1 ], color = FUEL, alpha = 0.85, zorder = 3 )
            ls = "-"
            what = "Tank Priority 1: the tank's volume counts at its own Density"
        else:
            ls = ( 0, ( 4, 2 ) )
            what = "Tank Priority 0, body first in the Geom Browser: the tank's volume counts at the body's Density"
        c.ax.plot( *np.vstack( [ qt, qt[ :1 ] ] ).T, color = "#0072B2", lw = 0.9, ls = ls, zorder = 4 )
        c.text( PT.mean( 0 ), "tank, Density %g" % TANK[ "Density" ], halo = False )
        c.ax.plot( *np.vstack( [ qb, qb[ :1 ] ] ).T, color = SKIN, lw = 2.2, zorder = 5 )
        under = PB[ PB[ :, 2 ].argmin() ] * np.array( [ 1.0, 1.0, 1.0 ] ) + np.array( [ 2.0, 0, 0 ] )
        k = np.argmin( np.abs( PB[ :, 0 ] - under[0] ) + 10.0 * ( PB[ :, 2 ] > 0 ) )
        s_pt = PB[k]
        c.ax.annotate( "body: Density 0, Include Thin Shell", xy = FRONT.p2( s_pt ), xytext = ( 0, -24 ),
                       textcoords = "offset points", ha = "center", va = "top", annotation_clip = False,
                       arrowprops = dict( arrowstyle = "-|>", lw = 0.7, color = "k", mutation_scale = 6,
                                          shrinkA = 1, shrinkB = 1 ) )
        title( c, "%s; total mass %.0f" % ( what, m ), pad = 6 )
        key( c, at = ( 0.0, 0.0 ), dx = -20, dy = -4 )
        rows.append( [ c ] )
    save( fig, "MassTab_priority", rows, hmax = 4.0 )


def save( fig, name, rows, scale = None, hmax = fl.HMAX ):
    fl.fit( fig, rows, scale = scale, hmax = hmax )
    os.makedirs( OUT, exist_ok = True )
    fig.savefig( os.path.join( OUT, name + ".png" ), dpi = DPI, metadata = { "Software": None } )
    plt.close( fig )
    print( "made", name )


FIGS = {
    "priority": priority,
}

if __name__ == "__main__":
    names = sys.argv[ 1: ] or list( FIGS )
    bad = [ n for n in names if n not in FIGS ]
    if bad:
        sys.exit( "unknown figure %s; the figures are: %s" % ( ", ".join( bad ), " ".join( FIGS ) ) )
    fl.style()
    for n in names:
        FIGS[ n ]()
