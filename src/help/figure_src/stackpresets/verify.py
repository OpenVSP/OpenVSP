"""Checks the statements StackPresets.md makes against VSP.

    <build>/venv/bin/python verify.py

Every preset is built with InitStackPreset, then Update, as a user's Set button does.
"""

import os
import sys

sys.path.insert( 0, os.path.dirname( os.path.abspath( __file__ ) ) )

import numpy as np

import stackpresets_figs as sp
from stackpresets_figs import vsp

E = vsp

# key: XSec Order (0 FREE, 1 LOOP), caps, section shapes, raw x of each section's center and its height,
# Geom Type, Inlet Mode, Outlet Mode (None where greyed by the definition), surfaces built ( 0 positive, 1 negative ).
P, C, L = vsp.XS_POINT, vsp.XS_CIRCLE, vsp.XS_ELLIPSE
EXPECT = {
    "default":              ( 0, 0, [ P, L, L, L, P ], [ 0, 1, 3, 4, 4.5 ], [ 0, 2.5, 2.5, 2.5, 0 ], E.ENGINE_GEOM_NONE, None, None, [ 0 ] ),
    "cylinder_endpts":      ( 0, 0, [ P, C, C, P ], [ 0, 0, 3, 3 ], [ 0, 2.5, 2.5, 0 ], E.ENGINE_GEOM_NONE, None, None, [ 0 ] ),
    "cylinder_endcaps":     ( 0, 1, [ C, C ], [ 0, 3 ], [ 2.5, 2.5 ], E.ENGINE_GEOM_NONE, None, None, [ 0 ] ),
    "flowthru_outlip_orig": ( 1, 0, [ C ] * 7, [ 0, -2.5, -3.25, -4, -6, -3, 0 ], [ 3, 2, 2, 2, 3, 4, 3 ], E.ENGINE_GEOM_INLET_OUTLET, E.ENGINE_MODE_FLOWTHROUGH, None, [ 0 ] ),
    "flowthru_inlip_orig":  ( 1, 0, [ C ] * 7, [ 0, 3, 6, 3.5, 2.75, 2, 0 ], [ 3, 4, 3, 2, 2, 2, 3 ], E.ENGINE_GEOM_INLET_OUTLET, E.ENGINE_MODE_FLOWTHROUGH, None, [ 0 ] ),
    "flowthru_mid_orig":    ( 1, 0, [ C ] * 7, [ 0, -0.75, -2.75, 0.25, 3.25, 0.75, 0 ], [ 2, 2, 3, 4, 3, 2, 2 ], E.ENGINE_GEOM_INLET_OUTLET, E.ENGINE_MODE_FLOWTHROUGH, None, [ 0 ] ),
    "bothfaceface":         ( 0, 0, [ P, C, C, C, C, C, P ], [ 0, 0, -2, 1, 4, 1.5, 1.5 ], [ 0, 2, 3, 4, 3, 2, 0 ], E.ENGINE_GEOM_INLET_OUTLET, E.ENGINE_MODE_TO_FACE, E.ENGINE_MODE_TO_FACE, [ 0 ] ),
    "bothlipface":          ( 0, 0, [ P, C, C, C, C, P ], [ 0, 0, 3, 6, 3.5, 3.5 ], [ 0, 3, 4, 3, 2, 0 ], E.ENGINE_GEOM_INLET_OUTLET, E.ENGINE_MODE_TO_LIP, E.ENGINE_MODE_TO_FACE, [ 0 ] ),
    "bothfacelip":          ( 0, 0, [ P, C, C, C, C, P ], [ 0, 0, -2, 1, 4, 4 ], [ 0, 2, 3, 4, 3, 0 ], E.ENGINE_GEOM_INLET_OUTLET, E.ENGINE_MODE_TO_FACE, E.ENGINE_MODE_TO_LIP, [ 0 ] ),
    "bothliplip":           ( 0, 0, [ P, C, C, C, P ], [ 0, 0, 3, 6, 6 ], [ 0, 3, 4, 3, 0 ], E.ENGINE_GEOM_INLET_OUTLET, E.ENGINE_MODE_TO_LIP, E.ENGINE_MODE_TO_LIP, [ 0 ] ),
    "outface":              ( 0, 0, [ P, C, C, C, P ], [ 0, 3, 6, 3.5, 3.5 ], [ 0, 4, 3, 2, 0 ], E.ENGINE_GEOM_OUTLET, None, E.ENGINE_MODE_TO_FACE, [ 0 ] ),
    "outlip":               ( 0, 0, [ P, C, C, P ], [ 0, 3, 6, 6 ], [ 0, 4, 3, 0 ], E.ENGINE_GEOM_OUTLET, None, E.ENGINE_MODE_TO_LIP, [ 0 ] ),
    "inface":               ( 0, 0, [ P, C, C, C, P ], [ 0, 0, -2, 1, 4 ], [ 0, 2, 3, 4, 0 ], E.ENGINE_GEOM_INLET, E.ENGINE_MODE_TO_FACE, None, [ 0 ] ),
    "inlip":                ( 0, 0, [ P, C, C, P ], [ 0, 0, 3, 6 ], [ 0, 3, 4, 0 ], E.ENGINE_GEOM_INLET, E.ENGINE_MODE_TO_LIP, None, [ 0 ] ),
    "flowpath_both":        ( 0, 0, [ P, C, C, C, C, C, P ], [ 0, 0, 2, 2.75, 3.5, 6, 6 ], [ 0, 3, 2, 2, 2, 3, 0 ], E.ENGINE_GEOM_INLET_OUTLET, E.ENGINE_MODE_TO_LIP, None, [ 0 ] ),
    "flowpath_in":          ( 0, 0, [ P, C, C, P ], [ 0, 0, 2, 2 ], [ 0, 3, 2, 0 ], E.ENGINE_GEOM_INLET, E.ENGINE_MODE_TO_LIP, None, [ 0 ] ),
    "flowpath_out":         ( 0, 0, [ P, C, C, P ], [ 0, 0, 2.5, 2.5 ], [ 0, 2, 3, 0 ], E.ENGINE_GEOM_OUTLET, None, E.ENGINE_MODE_TO_LIP, [ 0 ] ),
}

# The stations each preset uses, by section index.
STATIONS = {
    "flowthru_outlip_orig": { "Inlet Lip": 4, "Inlet Face": 3, "Outlet Face": 1, "Outlet Lip": 0 },
    "flowthru_inlip_orig":  { "Inlet Lip": 0, "Inlet Face": 5, "Outlet Face": 3, "Outlet Lip": 2 },
    "flowthru_mid_orig":    { "Inlet Lip": 2, "Inlet Face": 1, "Outlet Face": 5, "Outlet Lip": 4 },
    "bothfaceface":         { "Inlet Lip": 2, "Inlet Face": 1, "Outlet Face": 5, "Outlet Lip": 4 },
    "bothlipface":          { "Inlet Lip": 1, "Outlet Face": 4, "Outlet Lip": 3 },
    "bothfacelip":          { "Inlet Lip": 2, "Inlet Face": 1, "Outlet Lip": 4 },
    "bothliplip":           { "Inlet Lip": 1, "Outlet Lip": 3 },
    "outface":              { "Outlet Face": 3, "Outlet Lip": 2 },
    "outlip":               { "Outlet Lip": 2 },
    "inface":               { "Inlet Lip": 2, "Inlet Face": 1 },
    "inlip":                { "Inlet Lip": 1 },
    "flowpath_both":        { "Inlet Lip": 1, "Inlet Face": 2, "Outlet Face": 4, "Outlet Lip": 5 },
    "flowpath_in":          { "Inlet Lip": 1, "Inlet Face": 2 },
    "flowpath_out":         { "Outlet Face": 1, "Outlet Lip": 2 },
}
PARM = dict( sp.STATIONS )


def close( a, b, tol, what ):
    a = np.asarray( a, float )
    b = np.asarray( b, float )
    if a.shape != b.shape or not np.allclose( a, b, atol = tol, rtol = 0 ):
        raise AssertionError( "%s: %s != %s" % ( what, a, b ) )


def raw( s ):
    """Turn the engine model and the caps off; return a function that turns them back on."""
    io = sp.ep( s, "GeomIOType" )
    caps = [ vsp.GetParmVal( s, cp, "EndCap" ) for cp in ( "CapUMinOption", "CapUMaxOption" ) ]
    vsp.SetParmVal( s, "GeomIOType", "EngineModel", vsp.ENGINE_GEOM_NONE )
    for cp in ( "CapUMinOption", "CapUMaxOption" ):
        vsp.SetParmVal( s, cp, "EndCap", vsp.NO_END_CAP )
    vsp.Update()

    def back():
        vsp.SetParmVal( s, "GeomIOType", "EngineModel", io )
        for cp, v in zip( ( "CapUMinOption", "CapUMaxOption" ), caps ):
            vsp.SetParmVal( s, cp, "EndCap", v )
        vsp.Update()
    return back


def presets():
    for key, ( order, cap, shapes, xs, hs, io, inm, outm, surfs ) in EXPECT.items():
        s = sp.build( key )
        xss = vsp.GetXSecSurf( s, 0 )
        n = vsp.GetNumXSec( xss )
        close( int( vsp.GetParmVal( s, "OrderPolicy", "Design" ) ), order, 0, key + " order" )
        close( [ vsp.GetParmVal( s, cp, "EndCap" ) for cp in ( "CapUMinOption", "CapUMaxOption" ) ],
               [ cap, cap ], 0, key + " caps" )
        close( [ vsp.GetXSecShape( vsp.GetXSec( xss, i ) ) for i in range( n ) ], shapes, 0, key + " shapes" )
        close( int( sp.ep( s, "GeomIOType" ) ), io, 0, key + " geom type" )
        if inm is not None:
            close( int( sp.ep( s, "InletModeType" ) ), inm, 0, key + " inlet mode" )
        if outm is not None:
            close( int( sp.ep( s, "OutletModeType" ) ), outm, 0, key + " outlet mode" )
        close( [ vsp.GetGeomVSPSurfCfdType( s, i ) for i in range( vsp.GetNumMainSurfs( s ) ) ],
               [ vsp.CFD_NEGATIVE if t else vsp.CFD_NORMAL for t in surfs ], 0, key + " surfaces" )
        # Station indices, and that each lands on its section.
        st = STATIONS.get( key, {} )
        assert sorted( st ) == sorted( sp.active_stations( s ) ), ( key, sp.active_stations( s ) )
        for name, i in st.items():
            close( sp.ep( s, PARM[ name ] ), i / ( n - 1.0 ), 1e-12, key + " " + name )
        back = raw( s )
        top = [ np.array( list( vsp.CompPnt01( s, 0, i / ( n - 1.0 ), 0.75 ) ) ) for i in range( n ) ]
        bot = [ np.array( list( vsp.CompPnt01( s, 0, i / ( n - 1.0 ), 0.25 ) ) ) for i in range( n ) ]
        close( [ p[0] for p in top ], xs, 1e-9, key + " section x" )
        close( [ a[2] - b[2] for a, b in zip( top, bot ) ], hs, 1e-9, key + " section height" )
        # The normal points out of the material: up on the outer wall, toward the axis on an inner wall.
        for i in range( n - 1 ):
            u = ( i + 0.5 ) / ( n - 1.0 )
            p = np.array( list( vsp.CompPnt01( s, 0, u, 0.75 ) ) )
            q = np.array( list( vsp.CompNorm01( s, 0, u, 0.75 ) ) )
            dx = top[ i + 1 ][0] - top[i][0]
            if abs( dx ) < 1e-9 or abs( q[2] ) < 1e-3:
                continue
            # Moving +X along the upper cut the material is below (outer wall); moving -X it is above (inner wall).
            outer = dx > 0
            assert ( q[2] > 0 ) == outer, ( key, i, q )
        back()
    print( "presets: sections, order, caps, definitions, stations, surfaces, normals ok" )


def lengths():
    """The sizes the page states, and that a new Stack is the Default preset."""
    vsp.ClearVSPModel()
    a = vsp.AddGeom( "STACK" )
    vsp.Update()
    G = [ list( vsp.CompPnt01( a, 0, u / 8.0, w / 8.0 ) ) for u in range( 9 ) for w in range( 9 ) ]
    vsp.InitStackPreset( a, vsp.STACK_PRESET_DEFAULT )
    vsp.Update()
    close( [ list( vsp.CompPnt01( a, 0, u / 8.0, w / 8.0 ) ) for u in range( 9 ) for w in range( 9 ) ], G, 1e-12,
           "a new Stack is the Default" )
    s = sp.build( "default" )
    back = raw( s )
    w = np.array( list( vsp.CompPnt01( s, 0, 0.5, 0.0 ) ) )[1] - np.array( list( vsp.CompPnt01( s, 0, 0.5, 0.5 ) ) )[1]
    close( abs( w ), 3.0, 1e-9, "default width" )
    back()
    print( "lengths ok" )


def attach_normal():
    """A component attached to the sections with UW translation and rotation has its Z along the outward normal: a
    Pod moved 1 along its own Z has its nose at the attachment point plus the normal.  Checked with the engine
    model off (Geom Type NONE); with a flow-through representation on, an attachment at U above about 0.68 aborts
    VSP (reported, not documented)."""
    s = sp.build( "flowthru_mid_orig" )
    vsp.SetParmVal( s, "GeomIOType", "EngineModel", vsp.ENGINE_GEOM_NONE )
    b = vsp.AddGeom( "POD", s )
    vsp.SetParmVal( b, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( b, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_UV )
    vsp.SetParmVal( b, "Z_Rel_Location", "XForm", 1.0 )
    for u, w in ( ( 0.05, 0.75 ), ( 0.4, 0.75 ), ( 0.9, 0.6 ) ):
        vsp.SetParmVal( b, "U_Attach_Location", "Attach", u )
        vsp.SetParmVal( b, "V_Attach_Location", "Attach", w )
        vsp.Update()
        nose = np.array( list( vsp.CompPnt01( b, 0, 0.0, 0.0 ) ) )
        p = np.array( list( vsp.CompPnt01( s, 0, u, w ) ) )
        nrm = np.array( list( vsp.CompNorm01( s, 0, u, w ) ) )
        close( nose, p + nrm, 1e-6, "attached Z along the normal" )
    # On the inner wall (U 0.05, between the mid-plane and the inlet face) the normal points to the axis.
    assert np.array( list( vsp.CompNorm01( s, 0, 0.05, 0.75 ) ) )[2] < -0.99
    print( "attach: UW rotation's Z is the outward normal ok" )


def flowpath_modes():
    """What each representation builds from the flowpath presets: positive 0, negative 1."""
    cases = [
        ( "flowpath_in", "InletModeType", { "TO_LIP": [ 0 ], "TO_FACE_NEG": [ 1 ], "TO_FACE_NEG_ONLY": [], "EXTEND": [ 0 ] } ),
        ( "flowpath_out", "OutletModeType", { "TO_LIP": [ 0 ], "TO_FACE_NEG": [ 1 ], "TO_FACE_NEG_ONLY": [], "EXTEND": [ 0 ] } ),
        ( "flowpath_both", "InletModeType", { "FLOWTHROUGH_NEG": [ 1 ], "FLOWTHROUGH_NEG_ONLY": [ 1 ], "TO_LIP": [ 0 ],
                                              "TO_FACE_NEG": [ 0, 1 ], "TO_FACE_NEG_ONLY": [ 1 ] } ),
    ]
    for key, parm, modes in cases:
        for m, want in modes.items():
            s = sp.build( key )
            vsp.SetParmVal( s, parm, "EngineModel", getattr( vsp, "ENGINE_MODE_" + m ) )
            vsp.Update()
            close( [ vsp.GetGeomVSPSurfCfdType( s, i ) for i in range( vsp.GetNumMainSurfs( s ) ) ], want, 0,
                   key + " " + m )
    print( "flowpath representations ok" )


def stations_by_u():
    """A preset sets the stations' indices, not whether they are given by index or by U."""
    s = sp.build( "flowthru_mid_orig" )
    vsp.SetParmVal( s, "InletLipMode", "EngineModel", vsp.ENGINE_LOC_U )
    vsp.SetParmVal( s, "InletLipU", "EngineModel", 0.5 )
    vsp.Update()
    vsp.InitStackPreset( s, vsp.STACK_PRESET_FLOWTHRU_INLIP_ORIG )
    vsp.Update()
    close( [ sp.ep( s, "InletLipMode" ), sp.ep( s, "InletLipU" ) ], [ vsp.ENGINE_LOC_U, 0.5 ], 0, "U mode kept" )
    print( "stations: a preset leaves a station given by U where it was ok" )


if __name__ == "__main__":
    presets()
    lengths()
    attach_normal()
    flowpath_modes()
    stations_by_u()
