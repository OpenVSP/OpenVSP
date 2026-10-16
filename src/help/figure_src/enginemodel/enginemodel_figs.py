"""Figures for EngineModel.md: inlet types, stations and the representations of a flow-through nacelle.

    <build>/venv/bin/python enginemodel_figs.py [stations types flowthrough ends]

The drawing helpers are shared with the other body pages, in ../fuselagestack/bodyfig.py.
"""

import os
import sys

sys.path.insert( 0, os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "fuselagestack" ) )

import numpy as np

import bodyfig as bf
from bodyfig import vsp, Y


def nacelle( inmode = None, outmode = None, dist = 1.0 ):
    """The Nacelle Flowthru Mid Origin preset, with the given representation."""
    bf.renew()
    s = vsp.AddGeom( "STACK" )
    vsp.Update()
    vsp.InitStackPreset( s, vsp.STACK_PRESET_FLOWTHRU_MID_ORIG )
    vsp.SetParmVal( s, "ExtensionDistance", "EngineModel", dist )
    if inmode is not None:
        vsp.SetParmVal( s, "InletModeType", "EngineModel", getattr( vsp, "ENGINE_MODE_" + inmode ) )
    if outmode is not None:
        vsp.SetParmVal( s, "OutletModeType", "EngineModel", getattr( vsp, "ENGINE_MODE_" + outmode ) )
    vsp.Update()
    return s


def draw_surfs( c, s ):
    """Every main surface of s cut by Y = 0: positive grey, negative red and hatched."""
    kinds = []
    for i in range( vsp.GetNumMainSurfs( s ) ):
        neg = vsp.GetGeomVSPSurfCfdType( s, i ) == vsp.CFD_NEGATIVE
        kinds.append( neg )
    # Positive first, so the negative surfaces show over them.
    for neg in ( False, True ):
        for i in range( vsp.GetNumMainSurfs( s ) ):
            if kinds[i] != neg:
                continue
            for P in bf.cut_outlines( s, i ):
                if neg:
                    bf.fill( c, P, face = ( 0.84, 0.37, 0.0, 0.15 ), edge = bf.NEG, lw = 1.0, hatch = "////", z = 12 )
                else:
                    bf.fill( c, P, z = 10 )
    return kinds


def stations():
    """EngineModel_stations: the four stations on the flow-through nacelle."""
    s = nacelle( "FLOWTHROUGH" )
    n = bf.nxsec( s )
    st = ( ( "Inlet Lip", "InletLipU" ), ( "Inlet Face", "InletFaceU" ), ( "Outlet Face", "OutletFaceU" ),
           ( "Outlet Lip", "OutletLipU" ) )
    us = { name: vsp.GetParmVal( s, parm, "EngineModel" ) for name, parm in st }
    # The stations are fractions of the sections' own surface, which a flow-through nacelle turns to start at the
    # inlet face; drawn on that surface as the sections lay it out.
    vsp.SetParmVal( s, "GeomIOType", "EngineModel", vsp.ENGINE_GEOM_NONE )
    vsp.Update()
    for name, i in ( ( "Inlet Lip", 2 ), ( "Inlet Face", 1 ), ( "Outlet Face", 5 ), ( "Outlet Lip", 4 ) ):
        assert abs( us[ name ] - i / ( n - 1.0 ) ) < 1e-12
    fig = bf.figure()
    c = bf.canvas( fig, bf.SIDE )
    draw_surfs( c, s )
    bf.axis_line( c, -3.4, 3.9 )
    place = { "Inlet Lip": ( -10, 0, "right" ), "Inlet Face": ( 0, 12, "center" ), "Outlet Face": ( 0, 12, "center" ),
              "Outlet Lip": ( 10, 0, "left" ) }
    for name, parm in st:
        u = us[ name ]
        top = np.array( list( vsp.CompPnt01( s, 0, u, 0.75 ) ) )
        bot = np.array( list( vsp.CompPnt01( s, 0, u, 0.25 ) ) )
        col = bf.STATION[ name ]
        c.line( np.array( [ bot, top ] ), color = col, lw = 2.6, ls = ( 0, ( 3, 1.5 ) ), z = 50 )
        dx, dy, ha = place[ name ]
        bf.label( c, bot, name, dx = dx, dy = -dy if dy else 0, color = col, ha = ha, va = "top" if dy else "center" )
    for i in range( n - 1 ):
        p = np.array( list( vsp.CompPnt01( s, 0, i / ( n - 1.0 ), 0.75 ) ) )
        c.point( p, "k", s = 10 )
        # Sections 1 and 5 carry a station line below them, so their numbers sit beside it.
        dxy = { 0: ( 0, -11, "center" ), 1: ( -6, -11, "right" ), 2: ( -9, 0, "center" ), 3: ( 0, 10, "center" ),
                4: ( 9, 0, "center" ), 5: ( 6, -11, "left" ) }[ i ]
        bf.label( c, p, "0, 6" if i == 0 else "%d" % i, dx = dxy[0], dy = dxy[1], ha = dxy[2] )
    bf.label( c, np.array( [ -2.6, 0, 2.15 ] ), "inlet", ha = "right" )
    bf.label( c, np.array( [ 3.4, 0, 2.0 ] ), "outlet", ha = "left" )
    bf.key( c, "side", ( 1.0, 0.0 ), 22, 0 )
    bf.save( fig, "EngineModel_stations", [ [ c ] ], hmax = 4.2 )


def family( name, cases, rows_of ):
    fig = bf.figure()
    canv = []
    for title, inm, outm in cases:
        s = nacelle( inm, outm )
        c = bf.canvas( fig, bf.SIDE )
        draw_surfs( c, s )
        bf.axis_line( c, -4.0, 4.0 )
        c.bounds = ( np.array( [ -4.0, -2.1 ] ), np.array( [ 4.0, 2.1 ] ) )
        bf.title( c, title, pad = 2 )
        canv.append( c )
    rows = []
    k = 0
    for m in rows_of:
        rows.append( canv[ k:k + m ] )
        k += m
    bf.key( canv[ -1 ], "side", ( 1.0, 0.0 ), 8, 0 )
    bf.save( fig, name, rows, hmax = 9.0 )


def flowthrough():
    """EngineModel_flowthrough: the three flow-through modes."""
    family( "EngineModel_flowthrough", [ ( "FLOWTHROUGH", "FLOWTHROUGH", None ),
                                         ( "FLOWTHROUGH NEGATIVE", "FLOWTHROUGH_NEG", None ),
                                         ( "FLOWTHROUGH NEGATIVE ONLY", "FLOWTHROUGH_NEG_ONLY", None ) ], [ 2, 1 ] )


def ends():
    """EngineModel_ends: the modes of one end, at the inlet."""
    family( "EngineModel_ends", [ ( "TO LIP", "TO_LIP", None ), ( "TO FACE", "TO_FACE", None ),
                                  ( "TO FACE NEGATIVE", "TO_FACE_NEG", None ),
                                  ( "TO FACE NEGATIVE ONLY", "TO_FACE_NEG_ONLY", None ),
                                  ( "EXTEND", "EXTEND", None ) ], [ 2, 2, 1 ] )


STATIONS = ( ( "Inlet Lip", "InletLip" ), ( "Inlet Face", "InletFace" ), ( "Outlet Face", "OutletFace" ),
             ( "Outlet Lip", "OutletLip" ) )


def types():
    """EngineModel_types: the curve the sections describe for each Inlet Type, with its lips and faces."""
    cases = ( ( "FLOWTHROUGH", "FLOWTHRU_MID_ORIG", vsp.ENGINE_GEOM_FLOWTHROUGH ),
              ( "FLOWPATH", "FLOWPATH_BOTH", vsp.ENGINE_GEOM_FLOWPATH ),
              ( "TO LIP", "BOTHLIPLIP", vsp.ENGINE_GEOM_TO_LIP ),
              ( "TO FACE", "BOTHFACEFACE", vsp.ENGINE_GEOM_TO_FACE ) )
    fig = bf.figure()
    canv = []
    for title, preset, intype in cases:
        bf.renew()
        s = vsp.AddGeom( "STACK" )
        vsp.Update()
        vsp.InitStackPreset( s, getattr( vsp, "STACK_PRESET_" + preset ) )
        vsp.Update()
        E = lambda k: vsp.GetParmVal( s, k, "EngineModel" )
        assert int( E( "GeomIOType" ) ) == vsp.ENGINE_GEOM_INLET_OUTLET and int( E( "GeomInType" ) ) == intype
        # The stations the definition uses: the lips always, the faces unless the type is TO LIP.
        used = [ ( name, int( E( p + "Index" ) ) ) for name, p in STATIONS if intype != vsp.ENGINE_GEOM_TO_LIP or "Lip" in name ]
        for name, p in STATIONS:
            assert int( E( p + "Mode" ) ) == vsp.ENGINE_LOC_INDEX
        # The sections as they lay them out, without the representation or the caps.
        vsp.SetParmVal( s, "GeomIOType", "EngineModel", vsp.ENGINE_GEOM_NONE )
        vsp.SetParmVal( s, "CapUMinOption", "EndCap", vsp.NO_END_CAP )
        vsp.SetParmVal( s, "CapUMaxOption", "EndCap", vsp.NO_END_CAP )
        vsp.Update()
        n = bf.nxsec( s )
        c = bf.canvas( fig, bf.SIDE )
        # Each span drawn on its own; a span to a point section, which closes an end, in grey.
        pts = [ np.array( list( vsp.CompPnt01( s, 0, i / ( n - 1.0 ), 0.75 ) ) ) for i in range( n ) ]
        for i in range( n - 1 ):
            M = np.array( [ list( vsp.CompPnt01( s, 0, ( i + t ) / ( n - 1.0 ), 0.75 ) ) for t in np.linspace( 0, 1, 81 ) ] )
            closing = min( pts[i][2], pts[i + 1][2] ) < 1e-9
            if closing:
                assert np.abs( M[:, 0] - pts[i][0] ).max() < 1e-9      # a flat end, across the axis
                c.line( M, color = bf.GHOST, lw = 0.8, z = 30 )
            else:
                c.line( M, color = "k", lw = 1.4, z = 30 )
        bf.axis_line( c, -3.2, 6.4 )
        for p in pts:
            if p[2] > 1e-9:
                c.point( p, "k", s = 7, z = 60 )
        for name, i in used:
            p = np.array( list( vsp.CompPnt01( s, 0, i / ( n - 1.0 ), 0.75 ) ) )
            c.line( np.array( [ [ p[0], 0, 0 ], p ] ), color = bf.STATION[ name ], lw = 2.4, ls = ( 0, ( 1, 1.2 ) ), z = 50 )
        c.bounds = ( np.array( [ -3.2, 0.0 ] ), np.array( [ 6.4, 2.0 ] ) )
        bf.title( c, title, pad = 8 )
        canv.append( c )
    # A key to the station colors.
    k = bf.canvas( fig, bf.SIDE )
    for j, ( name, p ) in enumerate( STATIONS ):
        x = -3.2 + 3.4 * j
        k.line( np.array( [ [ x, 0, 0 ], [ x + 0.5, 0, 0 ] ] ), color = bf.STATION[ name ], lw = 2.4, ls = ( 0, ( 1, 1.2 ) ) )
        bf.label( k, np.array( [ x + 0.5, 0, 0 ] ), name, dx = 5, ha = "left", color = bf.STATION[ name ] )
    bf.key( canv[ -1 ], "side", ( 1.0, 0.0 ), 8, 0 )
    bf.save( fig, "EngineModel_types", [ canv[ 0:2 ], canv[ 2:4 ], [ k ] ], hmax = 6.0 )


FIGS = { "stations": stations, "types": types, "flowthrough": flowthrough, "ends": ends }

if __name__ == "__main__":
    for name in ( sys.argv[ 1: ] or FIGS ):
        FIGS[ name ]()
