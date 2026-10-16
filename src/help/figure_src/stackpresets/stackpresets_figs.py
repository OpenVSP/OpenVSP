"""Figures for StackPresets.md: every Stack preset, cut through its axis.

    <build>/venv/bin/python stackpresets_figs.py [default cylinder_endpts ...]    # names are PRESETS' keys

Each preset is built with the API (InitStackPreset, then Update) and drawn with the body figures' helpers
(../fuselagestack/bodyfig.py, on top of ../propeller/figlib.py).  A figure shows:

- the surfaces the preset builds as it comes, cut by Y = 0: positive grey, negative red and hatched;
- its sections, as laid out before the engine representation acts, numbered on the upper half of the cut, with an
  arrowhead in each span pointing from a section to the next;
- one surface normal on the outside;
- the active engine stations, in the colors the 3D view draws them;
- the Stack's origin.

All are at one scale, so the presets can be compared.
"""

import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "fuselagestack" ) )

import numpy as np

import bodyfig as bf
from bodyfig import vsp, X, Z

SCALE = 0.62          # inches per model unit, for every figure
C_ORDER = "#0072B2"   # the arrowheads of the section order
C_NORMAL = "#009E73"

# Image suffix: ( GUI name, enum ).
PRESETS = {
    "default":              ( "Default", "STACK_PRESET_DEFAULT" ),
    "cylinder_endpts":      ( "Cylinder with EndPoints", "STACK_PRESET_CYLINDER_ENDPTS" ),
    "cylinder_endcaps":     ( "Cylinder with EndCaps", "STACK_PRESET_CYLINDER_ENDCAPS" ),
    "flowthru_outlip_orig": ( "Nacelle Flowthru Outlet Origin", "STACK_PRESET_FLOWTHRU_OUTLIP_ORIG" ),
    "flowthru_inlip_orig":  ( "Nacelle Flowthru Inlet Origin", "STACK_PRESET_FLOWTHRU_INLIP_ORIG" ),
    "flowthru_mid_orig":    ( "Nacelle Flowthru Mid Origin", "STACK_PRESET_FLOWTHRU_MID_ORIG" ),
    "bothfaceface":         ( "Nacelle Inlet + Outlet to Face", "STACK_PRESET_BOTHFACEFACE" ),
    "bothlipface":          ( "Nacelle Inlet Lip + Outlet Face", "STACK_PRESET_BOTHLIPFACE" ),
    "bothfacelip":          ( "Nacelle Inlet Face + Outlet Lip", "STACK_PRESET_BOTHFACELIP" ),
    "bothliplip":           ( "Nacelle Inlet + Outlet to Lip", "STACK_PRESET_BOTHLIPLIP" ),
    "outface":              ( "Nacelle Outlet to Face", "STACK_PRESET_OUTFACE" ),
    "outlip":               ( "Nacelle Outlet to Lip", "STACK_PRESET_OUTLIP" ),
    "inface":               ( "Nacelle Inlet to Face", "STACK_PRESET_INFACE" ),
    "inlip":                ( "Nacelle Inlet to Lip", "STACK_PRESET_INLIP" ),
    "flowpath_both":        ( "Nacelle Flowpath Inlet + Outlet", "STACK_PRESET_FLOWPATH_BOTH" ),
    "flowpath_in":          ( "Nacelle Flowpath Inlet", "STACK_PRESET_FLOWPATH_IN" ),
    "flowpath_out":         ( "Nacelle Flowpath Outlet", "STACK_PRESET_FLOWPATH_OUT" ),
}

STATIONS = ( ( "Inlet Lip", "InletLipU" ), ( "Inlet Face", "InletFaceU" ), ( "Outlet Face", "OutletFaceU" ),
             ( "Outlet Lip", "OutletLipU" ) )


def ep( s, name ):
    return vsp.GetParmVal( s, name, "EngineModel" )


def build( key ):
    """A new Stack set to the preset."""
    bf.renew()
    s = vsp.AddGeom( "STACK" )
    vsp.Update()
    vsp.InitStackPreset( s, getattr( vsp, PRESETS[ key ][1] ) )
    vsp.Update()
    return s


def active_stations( s ):
    """The stations the engine definition uses (GeomEngine::UpdateEngine's rules)."""
    io = int( ep( s, "GeomIOType" ) )
    it = int( ep( s, "GeomInType" ) )
    ot = int( ep( s, "GeomOutType" ) )
    act = []
    if io == vsp.ENGINE_GEOM_NONE:
        return act
    if io in ( vsp.ENGINE_GEOM_INLET, vsp.ENGINE_GEOM_INLET_OUTLET ):
        act.append( "Inlet Lip" )
        if it != vsp.ENGINE_GEOM_TO_LIP:
            act.append( "Inlet Face" )
    if io in ( vsp.ENGINE_GEOM_INLET_OUTLET, vsp.ENGINE_GEOM_OUTLET ):
        act.append( "Outlet Lip" )
        if ot != vsp.ENGINE_GEOM_TO_LIP or ( io == vsp.ENGINE_GEOM_INLET_OUTLET and it == vsp.ENGINE_GEOM_FLOWTHROUGH ):
            act.append( "Outlet Face" )
    return act


def surfaces( s ):
    """The main surfaces as built, cut by Y = 0: [ ( negative, [ outlines ] ) ]."""
    out = []
    for i in range( vsp.GetNumMainSurfs( s ) ):
        neg = vsp.GetGeomVSPSurfCfdType( s, i ) == vsp.CFD_NEGATIVE
        out.append( ( neg, bf.cut_outlines( s, i ) ) )
    return out


def sections( s ):
    """The sections as laid out, before the engine representation acts: the surface's upper and lower cuts, each
    section's point on the upper cut, the outward normal at the outermost span, and the stations' u."""
    io = ep( s, "GeomIOType" )
    stations = { name: ep( s, parm ) for name, parm in STATIONS }
    act = active_stations( s )
    # End caps lengthen the surface's u, so they come off too while the sections are found.
    caps = [ vsp.GetParmVal( s, cp, "EndCap" ) for cp in ( "CapUMinOption", "CapUMaxOption" ) ]
    vsp.SetParmVal( s, "GeomIOType", "EngineModel", vsp.ENGINE_GEOM_NONE )
    for cp in ( "CapUMinOption", "CapUMaxOption" ):
        vsp.SetParmVal( s, cp, "EndCap", vsp.NO_END_CAP )
    vsp.Update()
    n = bf.nxsec( s )
    top, bot = bf.cut_halves( s, n = 60 * ( n - 1 ) + 1 )
    pts = [ np.array( list( vsp.CompPnt01( s, 0, i / ( n - 1.0 ), 0.75 ) ) ) for i in range( n ) ]
    # The normal a third of the way along the span that reaches farthest from the axis (the order's arrowhead is at
    # the middle).
    k = int( np.argmax( [ 0.5 * ( pts[i][2] + pts[i + 1][2] ) for i in range( n - 1 ) ] ) )
    u = ( k + 0.3 ) / ( n - 1.0 )
    p = np.array( list( vsp.CompPnt01( s, 0, u, 0.75 ) ) )
    nrm = np.array( list( vsp.CompNorm01( s, 0, u, 0.75 ) ) )
    st = {}
    for name in act:
        uu = stations[ name ]
        st[ name ] = ( np.array( list( vsp.CompPnt01( s, 0, uu, 0.25 ) ) ),
                       np.array( list( vsp.CompPnt01( s, 0, uu, 0.75 ) ) ) )
    vsp.SetParmVal( s, "GeomIOType", "EngineModel", io )
    for cp, v in zip( ( "CapUMinOption", "CapUMaxOption" ), caps ):
        vsp.SetParmVal( s, cp, "EndCap", v )
    vsp.Update()
    return dict( n = n, top = top, bot = bot, pts = pts, normal = ( p, nrm ), stations = st )


def number_offsets( pts, top ):
    """Where to write each section's number: away from the middle of the wall it lies on, or along the axis for a
    section that is a point on the axis.  Sections at one place share a label."""
    groups = []
    for i, p in enumerate( pts ):
        for g in groups:
            if np.linalg.norm( pts[ g[0] ] - p ) < 1e-6:
                g.append( i )
                break
        else:
            groups.append( [ i ] )
    mid = 0.5 * ( top.min( 0 ) + top.max( 0 ) )
    out = []
    for g in groups:
        p = pts[ g[0] ]
        d = np.array( [ p[0] - mid[0], p[2] - mid[2] ] )
        if abs( p[2] ) < 1e-6:
            d = np.array( [ np.sign( d[0] ) or 1.0, 0.0 ] )
        d = d / np.linalg.norm( d )
        out.append( ( p, ", ".join( "%d" % i for i in g ), d ) )
    return out


def figure( key ):
    s = build( key )
    built = surfaces( s )
    sec = sections( s )
    fig = bf.figure()
    c = bf.canvas( fig, bf.SIDE )
    for neg in ( False, True ):
        for ng, outs in built:
            if ng != neg:
                continue
            for P in outs:
                if neg:
                    bf.fill( c, P, face = ( 0.84, 0.37, 0.0, 0.15 ), edge = bf.NEG, lw = 1.0, hatch = "////", z = 12 )
                else:
                    bf.fill( c, P, z = 10 )
    x0 = min( sec["top"][:, 0].min(), sec["bot"][:, 0].min() )
    x1 = max( sec["top"][:, 0].max(), sec["bot"][:, 0].max() )
    bf.axis_line( c, x0 - 0.6, x1 + 0.6 )
    # The order of the sections: an arrowhead halfway along each span that has length.
    top = sec["top"]
    n = sec["n"]
    m = len( top )
    for i in range( n - 1 ):
        if np.linalg.norm( sec["pts"][ i + 1 ] - sec["pts"][i] ) < 1e-6:
            continue
        k = int( round( ( i + 0.5 ) / ( n - 1 ) * ( m - 1 ) ) )
        c.arrow( top[ k - 2 ], top[ k + 2 ], color = C_ORDER, lw = 1.2, head = 8.0, z = 60 )
    for p, lab, d in number_offsets( sec["pts"], top ):
        c.point( p, "k", s = 10 )
        ha = "left" if d[0] > 0.5 else ( "right" if d[0] < -0.5 else "center" )
        bf.label( c, p, lab, dx = 9.0 * d[0], dy = 9.0 * d[1], ha = ha )
    # Stations, from the lower cut to the upper.
    for name, ( pb, pt ) in sec["stations"].items():
        col = bf.STATION[ name ]
        c.line( np.array( [ pb, pt ] ), color = col, lw = 2.2, ls = ( 0, ( 3, 1.5 ) ), z = 50 )
        bf.label( c, pb, name, dy = -10, color = col, va = "top" )
    # The outward normal.
    p, nrm = sec["normal"]
    c.arrow( p, p + 0.9 * nrm, color = C_NORMAL, lw = 1.3, head = 7.0, z = 62 )
    bf.label( c, p + 0.9 * nrm, "normal", dx = 6, color = C_NORMAL, ha = "left" )
    # The Stack's origin, a ring that leaves a section's point at the origin visible.
    o = np.zeros( 3 )
    c.ax.plot( [ 0 ], [ 0 ], marker = "o", ms = 8.0, mfc = "none", mec = "k", mew = 0.9, zorder = 75 )
    bf.label( c, o, "origin", dy = -9, va = "top" )
    c.pts.append( np.zeros( ( 1, 2 ) ) )
    bf.key( c, "side", ( 1.0, 0.0 ), 30, 0 )
    bf.save( fig, "StackPresets_" + key, [ [ c ] ], scale = SCALE )


FIGS = { k: ( lambda k = k: figure( k ) ) for k in PRESETS }

if __name__ == "__main__":
    for name in ( sys.argv[ 1: ] or FIGS ):
        FIGS[ name ]()
