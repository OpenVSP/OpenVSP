"""Figures for the OpenVSP Auxiliary help page (src/help/Auxiliary.md), written to ../../images/Auxiliary_*.png.

    <build>/venv/bin/python auxiliary_figs.py [names...]

Uses the drawing helpers of ../propeller (figlib, and the views and save() of propeller_figs).  Every shape is VSP's
own, sampled with CompVecPnt01.  2D views keep OpenVSP's frame: the top view looks down -Z (X right, Y up), the
view from upstream looks along +X (-Y right, Z up).
"""

import os
import sys
import tempfile

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )
sys.path.insert( 0, os.path.join( HERE, "..", "gear" ) )

import numpy as np
import matplotlib
matplotlib.use( "Agg" )
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from matplotlib.text import OffsetFrom

import openvsp as vsp

import figlib as fl
from figlib import COL
import propeller_figs as pf
import gear_figs as gf
import gearmodel as gm
import meshfig as mf

X = np.array( [ 1.0, 0, 0 ] )
Y = np.array( [ 0, 1.0, 0 ] )
Z = np.array( [ 0, 0, 1.0 ] )
O = np.zeros( 3 )

ZONE = "#D55E00"
ZONE_FILL = "#F6D9C6"
DIM = gf.DIM


def grid( gid, isurf, nu, nw, u0 = 0.0, u1 = 1.0, w0 = 0.0, w1 = 1.0 ):
    us = np.linspace( u0, u1, nu )
    ws = np.linspace( w0, w1, nw )
    U, W = np.meshgrid( us, ws, indexing = "ij" )
    pts = vsp.CompVecPnt01( gid, isurf, list( U.ravel() ), list( W.ravel() ) )
    return np.array( [ [ p.x(), p.y(), p.z() ] for p in pts ] ).reshape( nu, nw, 3 )


def aux_of( parent, mode, parms ):
    a = vsp.AddGeom( "AUXILIARY", parent )
    vsp.SetParmVal( a, "AuxiliaryGeomType", "Design", mode )
    vsp.Update()
    for k, v in parms.items():
        vsp.SetParmVal( a, k, "Design", v )
    vsp.Update()
    return a


def profile( c, a, fill = ZONE_FILL ):
    """The section of a body of revolution about X in the plane Z = 0, Y > 0 (its w = 0 edge)."""
    P = grid( a, 0, 401, 1 )[ :, 0 ]
    c.ax.add_patch( Polygon( c.v.p2( P ), closed = True, fc = fill, ec = "none", zorder = 5 ) )
    c.line( P, color = ZONE, lw = 1.4, z = 30 )
    return P


def leader( c, p, s, off, ha = "left", color = "k" ):
    """A label set off from its point by off (points), with a thin line back to it."""
    c.pts.append( c.v.p2( np.asarray( p, float ) )[ None, : ] )
    c.ax.annotate( s, xy = c.v.p2( p ), xytext = off, textcoords = "offset points", ha = ha, va = "center", color = color,
                   zorder = 90, annotation_clip = False,
                   arrowprops = dict( arrowstyle = "-", lw = 0.7, color = color, shrinkA = 2, shrinkB = 2 ) )


def leader2( c, P, s, off, ha = "left" ):
    """One label with a thin line back to each of the points P; off (points) is from the first."""
    q0 = c.v.p2( np.asarray( P[ 0 ], float ) )
    for k, p in enumerate( P ):
        q = c.v.p2( np.asarray( p, float ) )
        c.pts.append( q[ None, : ] )
        c.ax.annotate( s if k == 0 else "", xy = q, xytext = off, textcoords = OffsetFrom( c.ax.transData, q0, "points" ),
                       ha = ha, va = "center",
                       zorder = 90, annotation_clip = False,
                       arrowprops = dict( arrowstyle = "-", lw = 0.7, color = "k", shrinkA = 2, shrinkB = 2 ) )


def shaft( c, x0, x1 ):
    c.line( np.array( [ x0 * X, x1 * X ] ), color = "#888888", lw = 0.7, ls = ( 0, ( 8, 2, 2, 2 ) ), z = 20 )
    c.text( x1 * X, "shaft", dx = 4, ha = "left", color = "#666666" )


def thrust( c, p0, length ):
    c.arrow( p0, p0 - length * X, color = "k", lw = 1.4, head = 9 )
    c.text( p0 - length * X, "thrust", dx = -4, ha = "right" )


TIP = { "ThetaThrust": 15.0, "ThetaAntiThrust": 15.0 }


def tip_path():
    """The rotor tip path of the default propeller, cut by the plane of its first blade, from above."""
    vsp.VSPRenew()
    p = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( p, "NumBlade", "Design", 1 )
    vsp.SetParmVal( p, "RFlapAx", "Articulation", 0.1 )
    vsp.Update()
    a = aux_of( p, vsp.AUX_GEOM_ROTOR_TIP_PATH, TIP )
    R = 0.5 * vsp.GetParmVal( a, "Diameter", "Design" )
    rf = vsp.GetParmVal( a, "FlapRadiusFract", "Design" ) * R
    L = R - rf
    hinge = rf * Y

    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = pf.panel( fig, pf.TOP )
    P = profile( c, a )
    shaft( c, -0.45 * R, 0.45 * R )
    thrust( c, -0.05 * R * Y, 0.3 * R )
    c.line( np.array( [ hinge, R * Y + 0.06 * R * Y ] ), color = "#9AA7B5", lw = 1.0, ls = ( 0, ( 6, 3 ) ), z = 21 )
    c.text( R * Y + 0.06 * R * Y, "plane of rotation", dy = 7, va = "bottom", color = "#6A7F95" )
    c.point( hinge, color = "k", s = 12, z = 80 )
    leader( c, hinge, "flap hinge, r_flap/R from the shaft", ( -40, 24 ), ha = "right" )
    tT = np.radians( TIP[ "ThetaThrust" ] )
    tA = np.radians( TIP[ "ThetaAntiThrust" ] )
    dT = np.array( [ -np.sin( tT ), np.cos( tT ), 0 ] )
    dA = np.array( [ np.sin( tA ), np.cos( tA ), 0 ] )
    for d in ( dT, dA ):
        c.line( np.array( [ hinge, hinge + L * d ] ), color = ZONE, lw = 0.7, ls = ( 0, ( 2, 2 ) ), z = 31 )
    c.angle( hinge, Y, dT, 0.6 * L, "k", label = "Theta Thrust", label_r = 1.1, dx = -55, dy = 0 )
    c.angle( hinge, Y, dA, 0.6 * L, "k", label = "Theta Anti Thrust", label_r = 1.1, dx = 62, dy = 0 )
    pf.leader( c, P[ len( P ) // 4 ], "tip path", ( -60, 0 ) )
    pf.key( c, "top", at = ( 1.0, 0.0 ), dx = -30, dy = 0 )
    pf.save( fig, "Auxiliary_tip_path", [ [ c ] ] )


BURST = { "AutoDiam": 0, "Diameter": 45.0, "RootLength": 3.0, "RootOffset": 0.5, "ThetaThrust": 15.0, "ThetaAntiThrust": 15.0 }


def burst():
    """A rotor burst zone about the default propeller, cut by the plane of its first blade, from above."""
    vsp.VSPRenew()
    p = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( p, "NumBlade", "Design", 1 )
    vsp.Update()
    a = aux_of( p, vsp.AUX_GEOM_ROTOR_BURST, BURST )
    R = 0.5 * BURST[ "Diameter" ]
    Lr = BURST[ "RootLength" ]
    x0 = -BURST[ "RootOffset" ] * Lr
    x1 = x0 + Lr

    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = pf.panel( fig, pf.TOP )
    P = profile( c, a )
    shaft( c, -0.4 * R, 0.4 * R )
    thrust( c, -0.6 * Lr * X - 0.04 * R * Y, 0.25 * R )
    # Root Length along the shaft, and where it sits.
    yd = -0.07 * R
    c.arrow( [ x0, yd, 0 ], [ x1, yd, 0 ], color = "#333333", lw = 0.8, head = 6, both = True )
    c.text( [ 0.5 * ( x0 + x1 ), yd, 0 ], "Root Length", dy = -6, va = "top" )
    for xx in ( x0, x1 ):
        c.line( np.array( [ [ xx, 0, 0 ], [ xx, yd - 0.01 * R, 0 ] ] ), color = "#9a9a9a", lw = 0.5, ls = ( 0, ( 2, 2 ) ) )
    c.line( np.array( [ O, R * Y + 0.05 * R * Y ] ), color = "#9AA7B5", lw = 1.0, ls = ( 0, ( 6, 3 ) ), z = 21 )
    c.text( R * Y + 0.05 * R * Y, "plane of rotation", dy = 7, va = "bottom", color = "#6A7F95" )
    tT = np.radians( BURST[ "ThetaThrust" ] )
    tA = np.radians( BURST[ "ThetaAntiThrust" ] )
    for xx, d, lab, dx in ( ( x0, np.array( [ -np.tan( tT ), 1, 0 ] ), "Theta Thrust", -56 ),
                           ( x1, np.array( [ np.tan( tA ), 1, 0 ] ), "Theta Anti Thrust", 68 ) ):
        base = np.array( [ xx, 0, 0 ] )
        c.line( np.array( [ base, base + R * Y ] ), color = "#9a9a9a", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 22 )
        c.angle( base, Y, d, 0.75 * R, "k", label = lab, label_r = 1.05, dx = dx, dy = 0 )
    pf.leader( c, [ 0, 0.5 * R, 0 ], "burst zone, out to Diameter / 2", ( 54, -30 ) )
    pf.key( c, "top", at = ( 1.0, 0.0 ), dx = -30, dy = 0 )
    pf.save( fig, "Auxiliary_burst", [ [ c ] ] )


XROTOR = 2.0

FRAG = { "RotorFragmentMode": vsp.GENERIC_FRAGMENT, "FragLength": 1.0, "CGRadius": 2.0, "ReleaseAngle": 30.0,
         "RotDir": 1, "ThetaThrust": 10.0, "ThetaAntiThrust": 5.0 }


def fragment():
    """A generic rotor fragment from an engine pod: its release, seen from upstream, and its spread, from above."""
    vsp.VSPRenew()
    pod = vsp.AddGeom( "POD" )
    vsp.SetParmVal( pod, "Length", "Design", 8.0 )
    vsp.SetParmVal( pod, "FineRatio", "Design", 3.0 )
    vsp.Update()
    a = aux_of( pod, vsp.AUX_GEOM_ROTOR_FRAGMENT, FRAG )
    # The rotor sits inside the pod, so the zone is set back from the pod's nose.
    vsp.SetParmVal( a, "X_Rel_Location", "XForm", XROTOR )
    vsp.Update()
    h = FRAG[ "CGRadius" ]
    rel = np.radians( FRAG[ "ReleaseAngle" ] )
    # Released at the CG radius, turned by the release angle in the direction of rotation (about -X).
    rdir = np.array( [ 0, np.sin( rel ), np.cos( rel ) ] )
    tdir = np.array( [ 0, np.cos( rel ), -np.sin( rel ) ] )
    # The zone's corners at the fragment (u = 0) and far away (u = 0.5): fore and aft at the low end, aft and
    # fore at the high end.
    W = ( 0.0, 0.25, 0.5, 0.75 )
    near = np.array( [ grid( a, 0, 1, 1, 0.0, 0.0, w, w )[ 0, 0 ] for w in W ] )
    far = np.array( [ grid( a, 0, 1, 1, 0.5, 0.5, w, w )[ 0, 0 ] for w in W ] )
    hub = XROTOR * X
    dirs = np.array( [ fl.unit( f - n ) for n, f in zip( near, far ) ] )
    assert np.abs( np.cross( near.mean( 0 ) - hub, rdir ) ).max() < 1e-6, "the fragment is not released where expected"
    assert ( dirs @ tdir ).min() > 0.9, "the fragment does not leave along the direction of rotation"
    Lb = 5.0
    ends = near + Lb * dirs

    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )

    # From upstream.
    u = pf.panel( fig, pf.UPSTREAM )
    G = grid( pod, 0, 81, 73 )
    imax = np.argmax( np.linalg.norm( G[ :, 0, 1: ], axis = 1 ) )
    u.line( G[ imax ], color = "#888888", lw = 1.0, z = 10 )
    u.text( G[ imax, 63 ], "pod", dx = 6, dy = 4, ha = "left", va = "bottom", color = "#666666" )
    ring = hub + np.array( [ [ 0, h * np.sin( t ), h * np.cos( t ) ] for t in np.linspace( 0, 2 * np.pi, 121 ) ] )
    u.line( ring, color = "#9AA7B5", lw = 0.8, ls = ( 0, ( 6, 3 ) ), z = 20 )
    band = np.array( [ near[0], near[3], ends[3], ends[0] ] )
    u.ax.add_patch( Polygon( u.v.p2( band ), closed = True, fc = ZONE_FILL, ec = "none", zorder = 6 ) )
    u.line( near[ [ 0, 3 ] ], color = ZONE, lw = 3.0, z = 40 )
    for k in ( 0, 3 ):
        u.line( np.array( [ near[k], ends[k] ] ), color = ZONE, lw = 1.0, z = 30 )
    u.line( np.array( [ hub, hub + h * rdir ] ), color = "k", lw = 0.8, z = 35 )
    u.line( np.array( [ hub, hub + 1.25 * h * Z ] ), color = "#888888", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 35 )
    u.angle( hub, Z, rdir, 0.5 * h, "k" )
    u.text( hub + 0.42 * h * Z, "Release Angle", dx = 6, dy = 0, ha = "left" )
    leader( u, hub + 0.6 * h * rdir, "CG Radius", ( -30, -75 ), ha = "right" )
    leader( u, near[ 3 ], "fragment, Fragment Length", ( 20, 16 ) )
    rot = u.arc_pts( hub, -X, 1.6 * h, 215, 275, ref = Z )
    u.arrow_path( rot, color = "k", lw = 1.0, head = 7 )
    u.text( rot[ 0 ], "rotation", dx = 4, dy = -4, ha = "left", va = "top" )
    pf.key( u, "upstream", at = ( 0.0, 0.0 ), dx = 10, dy = -10 )

    # From above.
    t = pf.panel( fig, pf.TOP )
    t.line( G[ :, 18 ], color = "#888888", lw = 1.0, z = 10 )
    t.line( G[ :, 54 ], color = "#888888", lw = 1.0, z = 10 )
    t.text( G[ 60, 18 ], "pod", dx = 0, dy = 8, va = "bottom", color = "#666666" )
    band = np.array( [ near[0], near[1], ends[1], ends[0] ] )
    t.ax.add_patch( Polygon( t.v.p2( band ), closed = True, fc = ZONE_FILL, ec = "none", zorder = 6, alpha = 0.85 ) )
    for k in ( 0, 1 ):
        t.line( np.array( [ near[k], ends[k] ] ), color = ZONE, lw = 1.0, z = 30 )
    o = 0.5 * ( near[0] + near[1] )
    mid = fl.unit( tdir - ( tdir @ Z ) * Z )
    t.line( np.array( [ o, o + 1.1 * Lb * mid ] ), color = "#888888", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 31 )
    tl = lambda v: fl.unit( v - ( v @ Z ) * Z )
    t.angle( o, mid, tl( dirs[0] ), 0.9 * Lb, "k", label = "Forward", label_r = 1.08, dx = -10, dy = 4 )
    t.angle( o, mid, tl( dirs[1] ), 0.9 * Lb, "k", label = "Aft", label_r = 1.08, dx = 10, dy = 4 )
    pf.key( t, "top", at = ( 1.0, 0.0 ), dx = -30, dy = 0 )
    pf.save( fig, "Auxiliary_fragment", [ [ u, t ] ] )


TWO_PT = "#0072B2"
THREE_PT = "#D55E00"
WHEEL_THETA = 12.0


def plane_line( a ):
    """The plane of a ground plane Auxiliary seen from the side: its corners, which project to a line."""
    P = grid( a, 0, 2, 2 ).reshape( -1, 3 )
    k = np.argsort( P[ :, 0 ] )
    return P[ k[ [ 0, -1 ] ] ]


def ground_planes():
    """Off-nominal ground planes of a tricycle gear, from the side: a 2pt plane through the main gear turned about
    the rear axles, and a 3pt plane with the nose tire flat and its strut compressed."""
    g = gf.tricycle_gear()
    vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", 0 )
    vsp.Update()
    nose_id, main_id = vsp.GetAllBogies( g )
    vsp.SetParmVal( vsp.FindParm( nose_id, "TravelCompressed", "Bogie" ), 0.4 )
    vsp.Update()

    two = aux_of( g, vsp.AUX_GEOM_TWO_PT_GROUND, {} )
    vsp.SetAuxiliaryGeomContactPtID( two, 0, main_id )
    vsp.SetAuxiliaryGeomContactPtID( two, 1, main_id )
    vsp.SetParmVal( two, "ContactPt2_Isymm", "Design", 1 )
    vsp.SetParmVal( two, "WheelTheta", "Design", WHEEL_THETA )
    three = aux_of( g, vsp.AUX_GEOM_THREE_PT_GROUND, {} )
    vsp.SetAuxiliaryGeomContactPtID( three, 0, nose_id )
    vsp.SetAuxiliaryGeomContactPtID( three, 1, main_id )
    vsp.SetAuxiliaryGeomContactPtID( three, 2, main_id )
    vsp.SetParmVal( three, "ContactPt3_Isymm", "Design", 1 )
    vsp.SetParmVal( three, "ContactPt1_SuspensionMode", "Design", vsp.GEAR_SUSPENSION_COMPRESSED )
    vsp.SetParmVal( three, "ContactPt1_TireMode", "Design", vsp.TIRE_FLAT_CONTACT )
    vsp.Update()

    M = gm.Bogie( main_id )
    pitch = vsp.GetParmVal( vsp.FindParm( main_id, "Pitch", "Bogie" ) )
    rear = M.pivot() + np.array( [ 0.5 * pitch, 0.0, 0.0 ] )
    L2 = plane_line( two )
    L3 = plane_line( three )
    # The 2pt plane turns about the rear axles, so it stays one static loaded radius from them.
    n2 = np.cross( L2[ 1 ] - L2[ 0 ], Y )
    n2 = n2 / np.linalg.norm( n2 )
    assert abs( abs( ( rear - L2[ 0 ] ) @ n2 ) - M.slr ) < 1e-9

    fig = plt.figure( figsize = ( fl.PAGE_W, 3.6 ) )
    c = pf.panel( fig, pf.FRONT )
    for i in range( vsp.GetNumMainSurfs( g ) ):
        G = gm.grid( g, i, 33, 33 )
        if G[ :, :, 1 ].mean() > -0.01:
            c.surface( G, edge = False )
    c.flush()
    xa = gf.NOSE[ "XContactPt" ] - 3.0
    xb = gf.MAIN[ "XContactPt" ] + 5.0

    def clip( L ):
        d = L[ 1 ] - L[ 0 ]
        return np.array( [ L[ 0 ] + ( xa - L[ 0, 0 ] ) / d[ 0 ] * d, L[ 0 ] + ( xb - L[ 0, 0 ] ) / d[ 0 ] * d ] )
    c.line( np.array( [ [ xa, 0, 0 ], [ xb, 0, 0 ] ] ), color = "#555555", lw = 1.2, z = 40 )
    c.text( [ xb, 0, 0 ], "nominal", dx = 4, ha = "left", color = "#555555" )
    A = clip( L2 )
    B = clip( L3 )
    c.line( A, color = TWO_PT, lw = 1.4, z = 41 )
    c.line( B, color = THREE_PT, lw = 1.4, z = 41 )
    c.text( A[ 1 ], "2pt, Wheel Theta %g" % WHEEL_THETA, dx = 4, ha = "left", color = TWO_PT )
    c.text( B[ 0 ], "3pt, nose tire Flat and strut Compressed", dx = -4, dy = 10, ha = "right", va = "bottom", color = THREE_PT )
    c.point( rear, color = "k", s = 12, z = 85 )
    leader( c, rear, "rear axle", ( 30, 40 ) )
    pf.key( c, "front", at = ( 0.0, 0.0 ), dx = -40, dy = 0 )
    pf.save( fig, "Auxiliary_ground_planes", [ [ c ] ] )


ROLL_THETA = 8.0


def ground_roll():
    """A 1pt ground plane under the left main bogie turned by Roll Theta, from ahead."""
    g = gf.tricycle_gear()
    vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", 0 )
    vsp.Update()
    nose_id, main_id = vsp.GetAllBogies( g )
    one = aux_of( g, vsp.AUX_GEOM_ONE_PT_GROUND, {} )
    vsp.SetAuxiliaryGeomContactPtID( one, 0, main_id )
    vsp.SetParmVal( one, "ContactPt1_Isymm", "Design", 1 )
    vsp.SetParmVal( one, "RollTheta", "Design", ROLL_THETA )
    vsp.Update()
    # The outer tire of the left (-Y) main bogie: its contact point, from the tires' own surfaces.
    cs = []
    for i in range( vsp.GetNumMainSurfs( g ) ):
        c_, ax = gm.tire_center( g, i )
        if c_[ 1 ] < 0.0 and abs( c_[ 0 ] - gf.MAIN[ "XContactPt" ] ) < 2.0:
            cs.append( c_ )
    cs = np.array( cs )
    M = gm.Bogie( main_id )
    cp = np.array( [ gf.MAIN[ "XContactPt" ], cs[ :, 1 ].min(), cs[ 0, 2 ] - M.slr ] )
    P = grid( one, 0, 2, 2 ).reshape( -1, 3 )
    n = np.cross( P[ 1 ] - P[ 0 ], P[ 2 ] - P[ 0 ] )
    n = n / np.linalg.norm( n ) * np.sign( n[ 2 ] )
    # The plane passes through the outer tire's contact point, turned about X: a positive Roll Theta lowers it on the
    # +Y side.
    assert abs( ( cp - P[ 0 ] ) @ n ) < 1e-9
    assert abs( n[ 0 ] ) < 1e-12 and abs( np.degrees( np.arctan2( n[ 1 ], n[ 2 ] ) ) - ROLL_THETA ) < 1e-9

    fig = plt.figure( figsize = ( fl.PAGE_W, 3.0 ) )
    c = pf.panel( fig, gf.AHEAD )
    for i in range( vsp.GetNumMainSurfs( g ) ):
        c.surface( gm.grid( g, i, 33, 33 ), edge = False )
    c.flush()
    ya, yb = -cp[ 1 ] + 2.5, cp[ 1 ] - 7.0
    c.line( np.array( [ [ 0, ya, 0 ], [ 0, yb, 0 ] ] ), color = "#555555", lw = 1.2, z = 40 )
    c.text( [ 0, ya, 0 ], "nominal", dx = -4, ha = "right", color = "#555555" )
    d = gm.unit( np.cross( X, n ) )
    d = d * np.sign( d[ 1 ] )
    L = np.array( [ cp + ( ya - cp[ 1 ] ) / d[ 1 ] * d, cp + ( yb - 1.0 - cp[ 1 ] ) / d[ 1 ] * d ] )
    c.line( L, color = TWO_PT, lw = 1.4, z = 41 )
    c.text( L[ 0 ], "1pt, Roll Theta %g" % ROLL_THETA, dx = -4, dy = 4, ha = "right", va = "bottom", color = TWO_PT )
    q = c.v.p2( cp )
    fl.out_of_page( c.ax, q, "", inward = True )
    c.pts.append( q[ None, : ] )
    c.angle( cp, -Y, -d, 5.5, "k", label = "Roll Theta", label_r = 1.0, dx = 34, dy = 0 )
    leader( c, cp, "roll axis along $X$, through the outer tire's contact point", ( -30, 72 ), ha = "right" )
    pf.key( c, "ahead", at = ( 1.0, 0.0 ), dx = -30, dy = 0 )
    pf.save( fig, "Auxiliary_ground_roll", [ [ c ] ] )


def gear_with( mode, parms, nose = False ):
    """The illustration tricycle gear of ../gear, with an Auxiliary component of mode on its main bogie, or on its
    nose bogie if nose."""
    g = gf.tricycle_gear()
    vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", 0 )
    vsp.Update()
    nose_id, main_id = vsp.GetAllBogies( g )
    a = aux_of( g, mode, {} )
    if nose:
        vsp.SetAuxiliaryGeomContactPtID( a, 0, nose_id )
    else:
        vsp.SetAuxiliaryGeomContactPtID( a, 0, main_id )
    for k, v in parms.items():
        vsp.SetParmVal( a, k, "Design", v )
    vsp.Update()
    return g, a, main_id


def outline( c, a, i, axes, fill = ZONE_FILL, edge = ZONE, ls = "-", alpha = 0.6 ):
    """The outline of surface i of a, seen along the view of c: the hull of its points projected on axes."""
    P = grid( a, i, 41, 41 ).reshape( -1, 3 )
    H = hull( P[ :, axes ] )
    H3 = np.zeros( ( len( H ) + 1, 3 ) )
    H3[ :-1, axes ] = H
    H3[ -1 ] = H3[ 0 ]
    if fill is not None:
        c.ax.add_patch( Polygon( c.v.p2( H3 ), closed = True, fc = fill, ec = "none", alpha = alpha, zorder = 3 ) )
    if edge is not None:
        c.line( H3, color = edge, lw = 1.0, ls = ls, z = 30 )
    return H3


def spray_model():
    """The nose bogie's ESDU 83042a spray at its default angles, read back from VSP: the start and far cross section of
    each plume, and the parms that set them.  Asserts that each angle and offset is where Auxiliary.md says."""
    g, a, main_id = gear_with( vsp.AUX_GEOM_TIRE_SPRAY, {}, nose = True )
    nose_id = vsp.GetAllBogies( g )[ 0 ]
    D = vsp.GetParmVal( vsp.FindParm( nose_id, "DiameterModel", "Tire" ) )

    def p( n ):
        return vsp.GetParmVal( a, n, "Design" )
    s = { "D": D, "h": p( "SprayTireContactHalfLength" ) * D, "bg": p( "SprayTireContactWidth" ) * D,
          "gs": p( "SpraySideElevationAngle" ), "ts": p( "SpraySidePlanAngle" ), "ls": p( "SpraySideIncrementalAngle" ),
          "ps": p( "SpraySideInclinationAngle" ), "gc": p( "SprayCenterElevationAngle" ), "dyc": p( "SprayCenterWidth" ) * D }
    assert vsp.GetNumMainSurfs( a ) == 3
    # Each plume runs from a point (u = 0) to its far cross section (u = 0.5): corners top outer, ground outer, ground
    # inner, top inner (w = 0, 0.25, 0.5, 0.75).
    W = ( 0.0, 0.25, 0.5, 0.75 )
    s[ "start4" ] = [ np.array( [ grid( a, i, 1, 1, 0.0, 0.0, w, w )[ 0, 0 ] for w in W ] ) for i in range( 3 ) ]
    s[ "start" ] = [ S[ 0 ] for S in s[ "start4" ] ]
    s[ "far" ] = [ np.array( [ grid( a, i, 1, 1, 0.5, 0.5, w, w )[ 0, 0 ] for w in W ] ) for i in range( 3 ) ]
    # The nose tires: their axle centers, from VSP's own surfaces.
    tires = []
    for i in range( vsp.GetNumMainSurfs( g ) ):
        c, ax = gm.tire_center( g, i )
        if abs( c[ 0 ] - gf.NOSE[ "XContactPt" ] ) < 1e-6:
            tires.append( ( i, c ) )
    assert len( tires ) == 2
    s[ "tires" ] = tires
    yt = max( c[ 1 ] for i, c in tires )
    xc = gf.NOSE[ "XContactPt" ]
    p0 = s[ "start" ][ 0 ]
    F = s[ "far" ][ 0 ]
    run = F[ 0, 0 ] - p0[ 0 ]
    deg = np.degrees
    # The side plume starts h ahead of the footprint and b_g / 4 outboard of it, on the ground.
    assert np.abs( s[ "start4" ][ 0 ] - [ xc - 2.0 * s[ "h" ], yt + 0.75 * s[ "bg" ], 0.0 ] ).max() < 1e-9
    # gamma_s from the ground to its top; theta_s from X to its top outer edge in plan.
    assert abs( deg( np.arctan2( F[ 0, 2 ], run ) ) - s[ "gs" ] ) < 1e-9
    assert abs( F[ 3, 2 ] - F[ 0, 2 ] ) < 1e-9 and abs( F[ 1, 2 ] ) < 1e-9 and abs( F[ 2, 2 ] ) < 1e-9
    assert abs( deg( np.arctan2( F[ 0, 1 ] - p0[ 1 ], run ) ) - s[ "ts" ] ) < 1e-9
    # lambda_s: the width, top and on the ground, is run tan( lambda_s ).
    for a_, b_ in ( ( 0, 3 ), ( 1, 2 ) ):
        assert abs( ( F[ a_, 1 ] - F[ b_, 1 ] ) - run * np.tan( np.radians( s[ "ls" ] ) ) ) < 1e-9
    # phi_s: the sides lean outward going up.
    assert abs( deg( np.arctan2( F[ 0, 1 ] - F[ 1, 1 ], F[ 0, 2 ] ) ) - s[ "ps" ] ) < 1e-9
    # The mirror plume on the other side.
    assert np.abs( s[ "far" ][ 1 ] * [ 1, -1, 1 ] - F ).max() < 1e-9
    # The center plume: from the same X, between the tires, Delta y_c wide, rising at gamma_c.
    C = s[ "far" ][ 2 ]
    C0 = s[ "start4" ][ 2 ]
    assert np.abs( C0[ :, 0 ] - p0[ 0 ] ).max() < 1e-9 and np.abs( C0[ :, 2 ] ).max() < 1e-9
    assert np.abs( C0[ :, 1 ] - C[ :, 1 ] ).max() < 1e-9
    assert abs( ( C[ 0, 1 ] - C[ 2, 1 ] ) - s[ "dyc" ] ) < 1e-9 and abs( C[ :, 1 ].mean() ) < 1e-9
    assert abs( deg( np.arctan2( C[ 0, 2 ], run ) ) - s[ "gc" ] ) < 1e-9
    s[ "g" ] = g
    s[ "a" ] = a
    s[ "xc" ] = xc
    s[ "yt" ] = yt
    s[ "run" ] = run
    return s


def spray_angles():
    """The spray of the nose bogie from the side, with gamma_s and gamma_c, and from above, with theta_s, lambda_s and
    Delta y_c."""
    s = spray_model()
    g = s[ "g" ]
    a = s[ "a" ]
    fig = plt.figure( figsize = ( fl.PAGE_W, 6.0 ) )
    side = pf.panel( fig, pf.FRONT )
    top = pf.panel( fig, pf.TOP )
    for i in range( vsp.GetNumMainSurfs( g ) ):
        G = gm.grid( g, i, 33, 33 )
        if G[ :, :, 1 ].mean() > 0.0:
            side.surface( G, edge = False )
        top.surface( G, edge = False )
    side.flush()
    top.flush()
    ns = vsp.GetNumMainSurfs( a )
    for i in range( ns ):
        outline( side, a, i, [ 0, 2 ] )
        outline( top, a, i, [ 0, 1 ], edge = None if i == 0 else ZONE )
    p0 = s[ "start" ][ 0 ]
    F = s[ "far" ][ 0 ]
    C = s[ "far" ][ 2 ]
    x1 = F[ 0, 0 ]
    run = s[ "run" ]
    side.line( np.array( [ [ s[ "xc" ] - 2.0, 0, 0 ], [ x1 + 1.0, 0, 0 ] ] ), color = "#555555", lw = 1.2, z = 40 )
    side.text( [ x1 + 1.0, 0, 0 ], "ground", dx = 4, ha = "left", color = "#555555" )

    # From the side: the two elevation angles, from the ground.
    for d, r, lab, dy in ( ( fl.unit( F[ 0 ] - p0 ), 0.55 * run, r"$\gamma_s$", -2 ), ( fl.unit( C[ 0 ] - s[ "start" ][ 2 ] ), 0.8 * run,
                                                                                      r"$\gamma_c$", 2 ) ):
        side.angle( p0, X, d, r, "k", label = lab, label_r = 1.0, dx = 10, dy = dy )
    side.text( [ p0[ 0 ] + 0.9 * run, 0, 0.3 * F[ 0, 2 ] ], "side plumes", halo = False )
    leader( side, [ 0.3 * p0[ 0 ] + 0.7 * x1, 0, 0.6 * ( 0.7 * C[ 0, 2 ] + 0.3 * F[ 0, 2 ] ) + 0.6 ], "center plume",
            ( -40, 26 ), ha = "right" )
    leader( side, s[ "tires" ][ 0 ][ 1 ] * [ 1, 0, 1 ], "nose tires", ( -10, 30 ), ha = "right" )

    # From above: the upper edges of the +Y side plume solid, its edges on the ground dashed.
    for k, ls in ( ( 0, "-" ), ( 3, "-" ), ( 1, ( 0, ( 4, 2 ) ) ), ( 2, ( 0, ( 4, 2 ) ) ) ):
        top.line( np.array( [ p0, F[ k ] ] ), color = ZONE, lw = 1.0, ls = ls, z = 31 )
    top.line( F[ [ 0, 2 ] ] * [ 1, 1, 0 ], color = ZONE, lw = 1.0, z = 31 )
    ref = np.array( [ p0, p0 + 1.03 * run * X ] )
    top.line( ref, color = "#888888", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 32 )
    top.angle( p0, X, F[ 0 ] - p0, 0.5 * run, "k", label = r"$\theta_s$", label_r = 1.0, dx = 10, dy = 0 )
    top.angle( p0, F[ 3 ] - p0, F[ 0 ] - p0, 0.82 * run, "k", label = r"$\lambda_s$", label_r = 1.0, dx = 10, dy = 0 )
    # Delta y_c across the center plume's far end, with arrows from outside.
    xd = C[ 0, 0 ] + 0.6
    yl, yr = C[ 2, 1 ], C[ 0, 1 ]
    for y0, y1 in ( ( yl - 1.0, yl ), ( yr + 1.0, yr ) ):
        top.arrow( [ xd, y0, 0 ], [ xd, y1, 0 ], color = DIM, lw = 0.8, head = 6 )
    for yy in ( yl, yr ):
        top.line( np.array( [ [ C[ 0, 0 ], yy, 0 ], [ xd + 0.2, yy, 0 ] ] ), color = "#9a9a9a", lw = 0.5, z = 30 )
    top.text( [ xd, yl - 1.0, 0 ], r"$\Delta y_c$", dy = -3, va = "top" )
    leader2( top, [ p0 + 0.86 * ( F[ k ] - p0 ) for k in ( 0, 3 ) ], "upper edges", ( 30, 22 ) )
    leader2( top, [ p0 + 0.93 * ( F[ k ] - p0 ) for k in ( 1, 2 ) ], "edges on the ground", ( 12, -40 ) )
    pf.key( side, "front", at = ( 0.0, 1.0 ), dx = -40, dy = -30 )
    pf.key( top, "top", at = ( 0.0, 1.0 ), dx = -40, dy = -30 )
    pf.save( fig, "Auxiliary_tire_spray", [ [ side ], [ top ] ] )


def spray_rear():
    """The spray's cross sections from behind: the side plumes lean outward by phi_s."""
    s = spray_model()
    g = s[ "g" ]
    fig = plt.figure( figsize = ( fl.PAGE_W, 3.0 ) )
    c = pf.panel( fig, pf.REAR )
    for i, ct in s[ "tires" ]:
        c.surface( gm.grid( g, i, 33, 33 ), edge = False )
    c.flush()
    for k in range( 3 ):
        Q = np.vstack( [ s[ "far" ][ k ], s[ "far" ][ k ][ :1 ] ] )
        c.ax.add_patch( Polygon( c.v.p2( Q ), closed = True, fc = ZONE_FILL, ec = "none", zorder = 3 ) )
        c.line( Q, color = ZONE, lw = 1.2, z = 30 )
    F = s[ "far" ][ 0 ]
    ymax = F[ :, 1 ].max() + 1.5
    c.line( np.array( [ [ 0, -ymax, 0 ], [ 0, ymax, 0 ] ] ), color = "#555555", lw = 1.2, z = 40 )
    c.text( [ 0, ymax, 0 ], "ground", dx = 4, ha = "left", color = "#555555" )
    # phi_s at the outer corner on the ground, from the vertical.
    b = F[ 1 ]
    c.line( np.array( [ b, b + 0.8 * F[ 0, 2 ] * Z ] ), color = "#888888", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 32 )
    c.angle( b, Z, F[ 0 ] - b, 0.75 * F[ 0, 2 ], "k", label = r"$\phi_s$", label_r = 1.0, dx = 2, dy = 12 )
    leader( c, 0.5 * ( F[ 2 ] + F[ 3 ] ) * [ 0, 1, 1 ] + [ 0, 0.4, 0 ], "side plume", ( 60, -20 ) )
    leader( c, s[ "far" ][ 2 ][ 0 ] * [ 0, 1, 0.7 ], "center plume", ( 40, 20 ) )
    leader( c, s[ "tires" ][ 0 ][ 1 ] * [ 0, -1, 1 ], "nose tires", ( -40, 10 ), ha = "right" )
    pf.key( c, "rear", at = ( 0.0, 1.0 ), dx = -40, dy = -30 )
    pf.save( fig, "Auxiliary_tire_spray_rear", [ [ c ] ], hmax = 3.4 )


def spray_footprint():
    """Where the plumes start, from above: a footprint b_g wide and 2h long under each nose tire, the plumes starting h
    ahead of it, the side plume b_g / 4 outboard."""
    s = spray_model()
    g = s[ "g" ]
    a = s[ "a" ]
    h = s[ "h" ]
    bg = s[ "bg" ]
    xc = s[ "xc" ]
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = pf.panel( fig, pf.TOP )
    for i, ct in s[ "tires" ]:
        c.surface( gm.grid( g, i, 33, 33 ), edge = False )
    c.flush()
    # The start of each plume, cut off a short way aft.
    L = 3.2 * h
    for k in range( 3 ):
        P0 = s[ "start4" ][ k ]
        F = s[ "far" ][ k ]
        t = L / ( F[ 0, 0 ] - P0[ 0, 0 ] )
        Q = P0 + t * ( F - P0 )
        H = hull( np.vstack( [ P0[ :, :2 ], Q[ :, :2 ] ] ) )
        H3 = np.zeros( ( len( H ) + 1, 3 ) )
        H3[ :-1, :2 ] = H
        H3[ -1 ] = H3[ 0 ]
        c.ax.add_patch( Polygon( c.v.p2( H3 ), closed = True, fc = ZONE_FILL, ec = "none", alpha = 0.6, zorder = 3 ) )
        for p, q in zip( P0, Q ):
            c.line( np.array( [ p, q ] ), color = ZONE, lw = 0.9, z = 30 )
        if k == 2:
            c.line( P0[ [ 0, 2 ] ], color = ZONE, lw = 2.0, z = 84 )
        else:
            c.point( P0[ 0 ], color = ZONE, s = 12, z = 85 )
    # The footprints.
    for i, ct in s[ "tires" ]:
        y = ct[ 1 ]
        R = np.array( [ [ xc - h, y - 0.5 * bg, 0 ], [ xc + h, y - 0.5 * bg, 0 ], [ xc + h, y + 0.5 * bg, 0 ],
                        [ xc - h, y + 0.5 * bg, 0 ], [ xc - h, y - 0.5 * bg, 0 ] ] )
        c.ax.add_patch( Polygon( c.v.p2( R ), closed = True, fc = "#FFFFFF", ec = "none", alpha = 0.55, zorder = 40 ) )
        c.line( R, color = "k", lw = 1.0, z = 41 )
        c.point( [ xc, y, 0 ], color = "k", s = 10, z = 85 )
    yt = s[ "yt" ]
    # b_g across the +Y footprint, behind it.
    xb = xc + h + 0.35 * h
    c.arrow( [ xb, yt - 0.5 * bg, 0 ], [ xb, yt + 0.5 * bg, 0 ], color = DIM, lw = 0.8, head = 6, both = True, z = 86 )
    for yy in ( yt - 0.5 * bg, yt + 0.5 * bg ):
        c.line( np.array( [ [ xc + h, yy, 0 ], [ xb + 0.05 * h, yy, 0 ] ] ), color = "#9a9a9a", lw = 0.5, z = 30 )
    c.text( [ xb, yt, 0 ], r"$b_g$", dx = 5, ha = "left" )
    # h twice along X, outboard of the side plume's start: contact point to the front of the footprint, then to the
    # plumes' start.
    p0 = s[ "start" ][ 0 ]
    yd = p0[ 1 ] + 0.55 * bg
    for x0, x1 in ( ( xc, xc - h ), ( xc - h, xc - 2.0 * h ) ):
        c.arrow( [ x0, yd, 0 ], [ x1, yd, 0 ], color = DIM, lw = 0.8, head = 6, both = True, z = 86 )
        c.text( [ 0.5 * ( x0 + x1 ), yd, 0 ], r"$h$", dy = 6, va = "bottom" )
    for xx, y0 in ( ( xc, yt ), ( xc - h, yt + 0.5 * bg ), ( xc - 2.0 * h, p0[ 1 ] ) ):
        c.line( np.array( [ [ xx, y0, 0 ], [ xx, yd + 0.08 * bg, 0 ] ] ), color = "#9a9a9a", lw = 0.5, ls = ( 0, ( 2, 2 ) ), z = 30 )
    # b_g / 4 from the side of the footprint to the side plume's start, with arrows from outside.
    xq = xc - 2.0 * h - 0.45 * h
    ya = yt + 0.5 * bg
    yb = p0[ 1 ]
    for y0, y1 in ( ( ya - 0.35 * bg, ya ), ( yb + 0.35 * bg, yb ) ):
        c.arrow( [ xq, y0, 0 ], [ xq, y1, 0 ], color = DIM, lw = 0.8, head = 6, z = 86 )
    for xx, yy in ( ( xc - h, ya ), ( p0[ 0 ], yb ) ):
        c.line( np.array( [ [ xx, yy, 0 ], [ xq - 0.08 * h, yy, 0 ] ] ), color = "#9a9a9a", lw = 0.5, ls = ( 0, ( 2, 2 ) ), z = 30 )
    c.text( [ xq, 0.5 * ( ya + yb ), 0 ], r"$b_g/4$", dx = -5, ha = "right" )
    leader( c, [ xc, -yt, 0 ], "contact point", ( 175, 0 ) )
    leader( c, [ xc + 0.6 * h, -yt - 0.5 * bg, 0 ], "footprint", ( 70, -20 ) )
    leader( c, p0, "side plume starts", ( -50, 18 ), ha = "right" )
    leader( c, s[ "start4" ][ 2 ].mean( 0 ), "center plume starts", ( -50, 0 ), ha = "right" )
    pf.key( c, "top", at = ( 1.0, 1.0 ), dx = -10, dy = -30 )
    pf.save( fig, "Auxiliary_tire_spray_footprint", [ [ c ] ] )


def hull( P ):
    """The convex hull of 2D points, in order."""
    P = sorted( set( map( tuple, np.round( P, 9 ) ) ) )

    def half( pts ):
        h = []
        for p in pts:
            while len( h ) >= 2 and ( ( h[-1][0] - h[-2][0] ) * ( p[1] - h[-2][1] ) -
                                      ( h[-1][1] - h[-2][1] ) * ( p[0] - h[-2][0] ) ) <= 0:
                h.pop()
            h.append( p )
        return h
    lo = half( P )
    hi = half( reversed( P ) )
    return np.array( lo[ :-1 ] + hi[ :-1 ] )


def wheel_failure():
    """AMC 25.734 Model 1 Large debris on the main bogie, from the side: the fan, less the strip ahead along the ground
    and the wedge behind."""
    g, a, main_id = gear_with( vsp.AUX_GEOM_WHEEL_TIRE_FAILURE, { "m_WheelTireFailureMode": vsp.WHEEL_TIRE_1LG } )
    fig = plt.figure( figsize = ( fl.PAGE_W, 3.8 ) )
    c = pf.panel( fig, pf.FRONT )
    for i in range( vsp.GetNumMainSurfs( g ) ):
        G = gm.grid( g, i, 33, 33 )
        if G[ :, :, 1 ].mean() > 0.0 and G[ :, :, 0 ].mean() > 0.0:
            c.surface( G, edge = False )
    c.flush()
    n = vsp.GetNumMainSurfs( a )
    assert n == 4
    # The two fans, one for each axle, and the two negative volumes cut from them.
    for i in range( n ):
        if i < 2:
            outline( c, a, i, [ 0, 2 ] )
        else:
            outline( c, a, i, [ 0, 2 ], fill = None, edge = "#555555", ls = ( 0, ( 4, 2 ) ) )
    lo = min( c.v.p2( grid( a, i, 9, 9 ).reshape( -1, 3 ) )[ :, 0 ].min() for i in range( n ) )
    hi = max( c.v.p2( grid( a, i, 9, 9 ).reshape( -1, 3 ) )[ :, 0 ].max() for i in range( n ) )
    c.line( np.array( [ [ lo, 0, 0 ], [ hi, 0, 0 ] ] ), color = "#555555", lw = 1.2, z = 40 )
    S = grid( a, 2, 9, 9 ).reshape( -1, 3 )
    W = grid( a, 3, 9, 9 ).reshape( -1, 3 )
    leader( c, [ S[ :, 0 ].min() + 2.0, 0, S[ :, 2 ].max() ], "strip along the ground (negative)", ( -10, 40 ), ha = "center" )
    leader( c, [ W[ :, 0 ].max() - 4.0, 0, 0.6 * W[ :, 2 ].max() ], "wedge behind (negative)", ( 30, 30 ) )
    F = grid( a, 0, 9, 9 ).reshape( -1, 3 )
    leader( c, [ 0.5 * ( F[ :, 0 ].min() + F[ :, 0 ].max() ), 0, 0.8 * F[ :, 2 ].max() ], "debris fan, one for each axle", ( -60, 10 ),
            ha = "right" )
    pf.key( c, "front", at = ( 0.0, 0.0 ), dx = -40, dy = 0 )
    pf.save( fig, "Auxiliary_wheel_failure", [ [ c ] ] )


# An example envelope profile across a catapult track, in inches: lateral position, height.
CCE_EXAMPLE = [ ( -1000.0, 18.0 ), ( -375.0, 18.0 ), ( -358.0, 0.0 ), ( 0.0, 0.0 ), ( 156.0, 0.0 ), ( 182.0, 15.0 ),
                ( 252.0, 15.0 ), ( 252.0, 28.0 ), ( 348.0, 28.0 ), ( 348.0, 58.0 ), ( 1000.0, 58.0 ) ]


def cce_gear( offset = 0.0 ):
    """The tricycle gear with a 3pt Composite Clearance Envelope of the example profile, read in inches."""
    g = gf.tricycle_gear()
    vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", 0 )
    vsp.Update()
    nose_id, main_id = vsp.GetAllBogies( g )
    a = aux_of( g, vsp.AUX_GEOM_THREE_PT_CCE, { "CCEUnits": vsp.LEN_IN, "CCEMainGearOffset": offset } )
    for i, ( b, isym ) in enumerate( ( ( nose_id, 0 ), ( main_id, 0 ), ( main_id, 1 ) ) ):
        vsp.SetAuxiliaryGeomContactPtID( a, i, b )
        vsp.SetParmVal( a, "ContactPt%d_Isymm" % ( i + 1 ), "Design", isym )
    path = os.path.join( tempfile.mkdtemp(), "example.cce" )
    open( path, "w" ).write( "".join( "%g %g\n" % p for p in CCE_EXAMPLE ) )
    vsp.ReadAuxiliaryGeomCCEFile( a, path )
    vsp.Update()
    return g, a


def track( a ):
    """The envelope's track: where the profile's lateral position is zero, along the envelope (w)."""
    P = grid( a, 0, 4001, 1 )[ :, 0 ]
    u = np.linspace( 0.0, 1.0, 4001 )[ np.argmin( np.hypot( P[ :, 1 ], P[ :, 2 ] ) ) ]
    return grid( a, 0, 1, 2, u0 = u, u1 = u )[ 0 ]


def clearance_envelope():
    """The example envelope, read from a file in inches, under the tricycle gear, seen from behind."""
    g, a = cce_gear()
    P = grid( a, 0, 2001, 1 )[ :, 0 ]
    # The profile is the file's, converted from inches to the Gear's feet.
    for y, z in CCE_EXAMPLE:
        assert np.min( np.hypot( P[ :, 1 ] - y / 12.0, P[ :, 2 ] - z / 12.0 ) ) < 1e-6, ( y, z )
    half = 35.0
    P = P[ np.abs( P[ :, 1 ] ) <= half ]
    fig = plt.figure( figsize = ( fl.PAGE_W, 2.6 ) )
    c = pf.panel( fig, pf.REAR )
    for i in range( vsp.GetNumMainSurfs( g ) ):
        c.surface( gm.grid( g, i, 33, 33 ), edge = False )
    c.flush()
    base = np.array( [ [ P[ -1, 0 ], P[ -1, 1 ], 0.0 ], [ P[ 0, 0 ], P[ 0, 1 ], 0.0 ] ] )
    c.ax.add_patch( Polygon( c.v.p2( np.vstack( [ P, base ] ) ), closed = True, fc = ZONE_FILL, ec = "none", zorder = 3 ) )
    c.line( P, color = ZONE, lw = 1.4, z = 30 )
    c.line( np.array( [ [ 0, -half, 0 ], [ 0, half, 0 ] ] ), color = "#555555", lw = 1.2, z = 40 )
    k = np.argmax( P[ :, 2 ] )
    leader( c, P[ k ], "envelope profile", ( -20, 24 ), ha = "right" )
    c.line( np.array( [ [ 0, 0, 0 ], [ 0, 0, 3.0 ] ] ), color = "#888888", lw = 0.7, ls = ( 0, ( 8, 2, 2, 2 ) ), z = 20 )
    c.text( [ 0, 0, 3.0 ], "track", dy = 6, va = "bottom", color = "#666666" )
    pf.key( c, "rear", at = ( 0.0, 1.0 ), dx = -40, dy = -30 )
    pf.save( fig, "Auxiliary_clearance_envelope", [ [ c ] ] )


CCE_OFFSET = 2.0


def clearance_offset():
    """Main Gear Offset, from above, in the frame of the track: the track straight, the aircraft turned about its nose
    gear's contact point, and the main gear's center the offset to one side of the track, measured square to it."""
    g, a = cce_gear( CCE_OFFSET )
    T = track( a )
    d = ( T[ 1 ] - T[ 0 ] ) / np.linalg.norm( T[ 1 ] - T[ 0 ] )
    d = d * np.sign( d[ 0 ] )
    nose = np.array( [ gf.NOSE[ "XContactPt" ], 0.0, 0.0 ] )
    m1 = np.array( [ gf.MAIN[ "XContactPt" ], gf.MAIN[ "YContactPt" ], 0.0 ] )
    m2 = m1 * np.array( [ 1, -1, 1 ] )
    mc = 0.5 * ( m1 + m2 )
    # Turn the aircraft about the nose gear's contact point so that the track runs along X.
    c0, s0 = d[ 0 ], d[ 1 ]
    R = np.array( [ [ c0, s0, 0.0 ], [ -s0, c0, 0.0 ], [ 0.0, 0.0, 1.0 ] ] )

    def deck( P ):
        return ( np.asarray( P ) - nose ) @ R.T + nose
    off = deck( mc )[ 1 ] - nose[ 1 ]
    assert abs( abs( off ) - CCE_OFFSET ) < 1e-6, off

    fig = plt.figure( figsize = ( fl.PAGE_W, 3.6 ) )
    c = pf.panel( fig, pf.TOP )
    for i in range( vsp.GetNumMainSurfs( g ) ):
        c.surface( deck( gm.grid( g, i, 33, 33 ) ), edge = False )
    c.flush()
    L = 1.6 * np.linalg.norm( mc - nose )
    c.line( np.array( [ nose - [ 0.15 * L, 0, 0 ], nose + [ L, 0, 0 ] ] ), color = ZONE, lw = 1.3, z = 30 )
    c.text( nose + [ L, 0, 0 ], "track", dx = 4, ha = "left", color = ZONE )
    cl = deck( np.array( [ nose - 0.15 * L * X, nose + L * X ] ) )
    c.line( cl, color = "#888888", lw = 1.0, ls = ( 0, ( 8, 2, 2, 2 ) ), z = 29 )
    c.text( cl[ 1 ], "aircraft centerline", dx = 4, ha = "left", color = "#666666" )
    c.line( deck( np.array( [ m1, m2 ] ) ), color = "#888888", lw = 0.7, ls = ( 0, ( 2, 2 ) ), z = 31 )
    mcd = deck( mc )
    c.point( mcd, color = "k", s = 12, z = 85 )
    foot = np.array( [ mcd[ 0 ], nose[ 1 ], 0.0 ] )
    c.arrow( foot, mcd, color = "#333333", lw = 0.8, head = 6, both = True, z = 86 )
    c.text( 0.5 * ( foot + mcd ), "Main Gear Offset", dx = 6, ha = "left" )
    leader( c, deck( m1 ), "Main Bogie 1", ( 30, 12 ) )
    leader( c, deck( m2 ), "Main Bogie 2", ( 30, -12 ) )
    leader( c, nose, "Nose Gear Bogie, on the track", ( -20, 24 ), ha = "right" )
    c.arrow( nose + [ 0.6 * L, -0.25 * L, 0 ], nose + [ 0.35 * L, -0.25 * L, 0 ], color = "k", lw = 1.0, head = 7 )
    c.text( nose + [ 0.6 * L, -0.25 * L, 0 ], "forward", dx = 4, ha = "left" )
    pf.save( fig, "Auxiliary_clearance_offset", [ [ c ] ] )

# The corners of the AC 25.773 pilot vision envelope, left cockpit seat (VspCurve::CreateAC25773): azimuth, elevation.
AC25_773 = [ ( 20, 0 ), ( 20, -10 ), ( 10, -17 ), ( -30, -17 ), ( -60, -24.5 ), ( -70, -27 ), ( -95, -27 ), ( -120, -15 ),
             ( -120, 0 ), ( -120, 15 ), ( -80, 35 ), ( -60, 35 ), ( -40, 35 ), ( 20, 15 ) ]
# The Human's SITTING preset.
SITTING = { "ElbowRt": 80.0, "ShoulderIERt": 45.0, "HipFERt": 80.0, "KneeRt": 80.0 }


def seated_cone():
    """A seated Human in meters with a default Super Cone: the Human's ID, the cone's, the apex, the ring of the
    cone's far edge, and the ring's azimuth and elevation from the apex, in degrees.  Asserts the ring is the
    AC 25.773 profile, azimuth toward +Y and elevation up, looking along -X."""
    vsp.VSPRenew()
    h = vsp.AddGeom( "HUMAN" )
    vsp.SetParmVal( h, "LenUnit", "Anthropometric", vsp.LEN_M )
    vsp.Update()
    for k, v in SITTING.items():
        vsp.SetParmVal( h, k, "Pose", v )
    vsp.Update()
    a = aux_of( h, vsp.AUX_GEOM_SUPER_CONE, {} )
    # The cone's surface runs from the center of its far cap (u = 0) through the profile (u = 0.5) to the apex (u = 1).
    apex = grid( a, 0, 1, 1, 1.0, 1.0 )[ 0, 0 ]
    ring = grid( a, 0, 1, 2801, 0.5, 0.5 )[ 0 ]
    d = ring - apex
    r = np.linalg.norm( d, axis = 1 )
    assert np.ptp( r ) < 1e-5 * r.max(), "the profile is not on a sphere about the apex"
    d = d / r[ :, None ]
    az = np.degrees( np.arctan2( d[ :, 1 ], -d[ :, 0 ] ) )
    el = np.degrees( np.arcsin( d[ :, 2 ] ) )
    for p in AC25_773:
        assert np.min( np.hypot( az - p[ 0 ], el - p[ 1 ] ) ) < 0.05, p
    return h, a, apex, ring, az, el


def supercone_profile():
    """The default Super Cone profile, the AC 25.773 pilot vision envelope, in azimuth and elevation."""
    h, a, apex, ring, az, el = seated_cone()
    fig = plt.figure( figsize = ( fl.PAGE_W, 3.6 ) )
    c = pf.panel( fig, pf.FRONT )

    def q( x, y ):
        return np.array( [ x, 0.0, y ] )
    P = np.array( [ q( x, y ) for x, y in zip( az, el ) ] )
    A0, A1, E0, E1 = -130, 30, -30, 40
    for x in range( -120, 21, 20 ):
        c.line( np.array( [ q( x, E0 ), q( x, E1 ) ] ), color = "#D8D8D8", lw = 0.5, z = 1 )
        c.text( q( x, E0 ), "%d" % x, dy = -6, va = "top", halo = False )
    for y in range( -30, 41, 10 ):
        c.line( np.array( [ q( A0, y ), q( A1, y ) ] ), color = "#D8D8D8", lw = 0.5, z = 1 )
        c.text( q( A0, y ), "%d" % y, dx = -5, ha = "right", halo = False )
    c.line( np.array( [ q( 0, E0 ), q( 0, E1 ) ] ), color = "#888888", lw = 0.8, z = 2 )
    c.line( np.array( [ q( A0, 0 ), q( A1, 0 ) ] ), color = "#888888", lw = 0.8, z = 2 )
    c.ax.add_patch( Polygon( c.v.p2( P ), closed = True, fc = ZONE_FILL, ec = "none", zorder = 5 ) )
    c.line( P, color = ZONE, lw = 1.4, z = 30 )
    c.point( q( 0, 0 ), color = "k", s = 12, z = 85 )
    leader( c, q( 0, 0 ), "straight ahead, along $-X$", ( 40, -40 ) )
    c.text( q( 0.5 * ( A0 + A1 ), E0 ), "azimuth, degrees, positive toward $+Y$ (right)", dy = -22, va = "top", halo = False )
    c.text( q( A0, 0.5 * ( E0 + E1 ) ), "elevation, degrees, positive up", dx = -26, rotation = 90, halo = False )
    c.text( q( -60, 8 ), "AC 25.773 pilot vision envelope, left seat", halo = False )
    pf.save( fig, "Auxiliary_supercone_profile", [ [ c ] ] )


def supercone():
    """The default Super Cone on a seated Human: the cone of directions from the design eye."""
    h, a, apex, ring, az, el = seated_cone()
    T = mf.geom_tris( h )
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.4 ) )
    c = pf.panel( fig, pf.ISO )
    mf.tris( c, T, "#C8CCD2" )
    c.flush()
    # The cone, one translucent layer over the Human, its far edge, and its edges to the apex at the profile's corners.
    G = grid( a, 0, 25, 281, 0.5, 1.0 )
    Q = np.array( [ [ G[ i, j ], G[ i + 1, j ], G[ i + 1, j + 1 ] ] for i in range( 24 ) for j in range( 280 ) ] +
                  [ [ G[ i, j ], G[ i + 1, j + 1 ], G[ i, j + 1 ] ] for i in range( 24 ) for j in range( 280 ) ] )
    mf.tris( c, Q, ZONE, flat = True, z0 = 20, group_alpha = 0.16 )
    c.line( ring, color = ZONE, lw = 1.3, z = 30 )
    r = np.linalg.norm( ring[ 0 ] - apex )
    for x, y in AC25_773:
        t, p = np.radians( x ), np.radians( y )
        e = apex + r * np.array( [ -np.cos( p ) * np.cos( t ), np.cos( p ) * np.sin( t ), np.sin( p ) ] )
        assert np.min( np.linalg.norm( ring - e, axis = 1 ) ) < 1e-3 * r
        c.line( np.array( [ apex, e ] ), color = ZONE, lw = 0.6, z = 29 )
    fwd = apex - r * X
    c.line( np.array( [ apex, fwd ] ), color = "k", lw = 0.9, ls = ( 0, ( 6, 3 ) ), z = 31 )
    c.text( fwd, "straight ahead, $-X$", dx = -4, dy = -10, ha = "right" )
    c.point( apex, color = "k", s = 12, z = 85 )
    leader( c, apex, "design eye, the apex", ( 50, 40 ) )
    k = np.argmax( c.v.p2( ring )[ :, 0 ] )
    leader( c, ring[ k ], "the cone's far edge, on a sphere about the eye", ( -40, -40 ), ha = "right" )
    pf.save( fig, "Auxiliary_supercone", [ [ c ] ] )



FIGS = {
    "cce": ( clearance_envelope, clearance_offset ),
    "cone": ( supercone_profile, supercone ),
    "spray": ( spray_angles, spray_rear, spray_footprint ),
    "failure": ( wheel_failure, ),
    "ground": ( ground_planes, ground_roll ),
    "tip": ( tip_path, ),
    "burst": ( burst, ),
    "fragment": ( fragment, ),
}

if __name__ == "__main__":
    names = sys.argv[ 1: ] or list( FIGS )
    pf.style()
    for n in names:
        for fn in FIGS[ n ]:
            fn()
