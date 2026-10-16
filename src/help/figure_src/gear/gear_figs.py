"""Figures for the OpenVSP Gear help page (src/help/Gear.md), written to ../../images/Gear_*.png.

    <build>/venv/bin/python gear_figs.py [names...]

Uses the drawing helpers of ../propeller (figlib, and the views and save() of propeller_figs).  Every shape is VSP's
own: tires are sampled from the Gear's surfaces with CompVecPnt01.  2D views keep OpenVSP's frame: the view from
ahead looks along +X (-Y right, Z up), the side view along +Y (X right, Z up), the top view along -Z (X right, Y up).
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )

import numpy as np
import matplotlib
matplotlib.use( "Agg" )
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon

import openvsp as vsp

import figlib as fl
from figlib import COL, FS
import propeller_figs as pf

import gearmodel as gm

X = np.array( [ 1.0, 0, 0 ] )
Y = np.array( [ 0, 1.0, 0 ] )
Z = np.array( [ 0, 0, 1.0 ] )
O = np.zeros( 3 )

AHEAD = pf.view( -Y, Z )      # from ahead (-X), looking aft
SIDE = pf.view( X, Z )        # looking along +Y
TOP = pf.view( X, Y )         # looking down -Z
pf.KEY[ "ahead" ] = ( r"$-Y$", r"$Z$", r"$-X$" )

TIRE = "#C8CCD2"
GROWN = "#0072B2"
CLEAR = "#D55E00"
DIM = "#333333"


def new_gear( ground = False ):
    vsp.VSPRenew()
    g = vsp.AddGeom( "GEAR" )
    b = vsp.CreateAndAddBogie( g )
    vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", 1 if ground else 0 )
    vsp.Update()
    return g, b


def bparm( b, name, grp ):
    return vsp.FindParm( b, name, grp )


def section( gid, isurf, n = 241 ):
    """The tire's section through its axle: the profile above the axle and below it, as one closed outline
    in the Y-Z plane through the axle."""
    top = gm.grid( gid, isurf, n, 1, w0 = 0.0, w1 = 0.0 )[ :, 0 ]
    bot = gm.grid( gid, isurf, n, 1, w0 = 0.5, w1 = 0.5 )[ :, 0 ]
    return top, bot


def draw_section( c, top, bot, fill = TIRE, edge = "#4a4a4a", lw = 1.0, ls = "-", z = 10 ):
    ring = np.vstack( [ top, bot[ ::-1 ] ] )
    if fill is not None:
        q = c.v.p2( ring )
        c.ax.add_patch( Polygon( q, closed = True, fc = fill, ec = "none", zorder = z ) )
    c.line( top, color = edge, lw = lw, ls = ls, z = z + 1 )
    c.line( bot, color = edge, lw = lw, ls = ls, z = z + 1 )


def hdim( c, y0, y1, z, label, dy = 0.0, color = DIM, above = True ):
    """A dimension across Y at height z, from y0 to y1."""
    p0 = np.array( [ 0, y0, z ] )
    p1 = np.array( [ 0, y1, z ] )
    c.arrow( p0, p1, color = color, lw = 0.8, head = 6, both = True, z = 70 )
    c.text( 0.5 * ( p0 + p1 ), label, color = color, dy = 7 if above else -7, va = "bottom" if above else "top" )


def vdim( c, y, z0, z1, label, color = DIM, side = "right", dx = 4 ):
    p0 = np.array( [ 0, y, z0 ] )
    p1 = np.array( [ 0, y, z1 ] )
    c.arrow( p0, p1, color = color, lw = 0.8, head = 6, both = True, z = 70 )
    if side == "right":
        c.text( 0.5 * ( p0 + p1 ), label, color = color, dx = dx, ha = "left" )
    else:
        c.text( 0.5 * ( p0 + p1 ), label, color = color, dx = -dx, ha = "right" )


def ext( c, p0, p1, color = "#9a9a9a" ):
    """An extension line."""
    c.line( np.array( [ p0, p1 ] ), color = color, lw = 0.5, ls = ( 0, ( 2, 2 ) ), z = 30 )


def tire_section():
    """The default tire, Full TRA, seen from ahead, cut through its axle, with its dimensions, the grown tire and the
    clearance envelope."""
    g, b = new_gear()
    top, bot = section( g, 0 )

    def val( n ):
        return vsp.GetParmVal( bparm( b, n, "Tire" ) )

    D = val( "DiameterModel" )
    W = val( "WidthModel" )
    Drim = val( "DrimModel" )
    Wrim = val( "WrimModel" )
    Ws = val( "WsModel" )
    Hs = val( "HsModel" )
    Dfl = val( "DFlangeModel" )
    slr = val( "StaticRadiusModel" )

    # The grown tire and the clearance envelope, through an Auxiliary Single Gear.
    a = vsp.AddGeom( "AUXILIARY", g )
    vsp.SetParmVal( a, "AuxiliaryGeomType", "Design", vsp.AUX_GEOM_SINGLE_GEAR )
    vsp.SetParmVal( a, "ContactPt1_ClearanceMode", "Design", vsp.TIRE_GROWTH_CLEARANCE )
    vsp.Update()
    gtop, gbot = section( a, 0 )
    vsp.SetParmVal( a, "ContactPt1_ClearanceMode", "Design", vsp.TIRE_CLEARANCE )
    vsp.Update()
    ctop, cbot = section( a, 0 )

    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = pf.panel( fig, AHEAD )
    axle = slr
    # The ground and the axle.
    gw = 0.85 * W
    c.line( np.array( [ [ 0, gw, 0 ], [ 0, -gw, 0 ] ] ), color = "#555555", lw = 1.2, z = 40 )
    c.text( [ 0, -gw, 0 ], "ground", dx = 4, dy = -7, ha = "right", color = "#555555" )
    c.line( np.array( [ [ 0, 0.62 * W, axle ], [ 0, -0.62 * W, axle ] ] ), color = "#888888", lw = 0.7, ls = ( 0, ( 8, 2, 2, 2 ) ), z = 40 )
    c.text( [ 0, 0.62 * W, axle ], "axle", dx = -4, ha = "right", color = "#666666" )

    draw_section( c, ctop, cbot, fill = None, edge = CLEAR, lw = 1.0, ls = ( 0, ( 1, 1.5 ) ), z = 12 )
    draw_section( c, gtop, gbot, fill = None, edge = GROWN, lw = 1.0, ls = ( 0, ( 4, 2 ) ), z = 13 )
    draw_section( c, top, bot, z = 14 )

    ztop = axle + 0.5 * D
    zbot = axle - 0.5 * D
    # Width and shoulder width across the top.
    hdim( c, 0.5 * W, -0.5 * W, ztop + 0.30 * D, "Width" )
    ext( c, [ 0, 0.5 * W, axle ], [ 0, 0.5 * W, ztop + 0.32 * D ] )
    ext( c, [ 0, -0.5 * W, axle ], [ 0, -0.5 * W, ztop + 0.32 * D ] )
    hdim( c, 0.5 * Ws, -0.5 * Ws, ztop + 0.14 * D, "Ws" )
    zs = axle + 0.5 * Drim + Hs
    ext( c, [ 0, 0.5 * Ws, zs ], [ 0, 0.5 * Ws, ztop + 0.16 * D ] )
    ext( c, [ 0, -0.5 * Ws, zs ], [ 0, -0.5 * Ws, ztop + 0.16 * D ] )
    # Rim width between the flanges, at the axle.
    hdim( c, 0.5 * Wrim, -0.5 * Wrim, axle + 0.04 * D, "Wrim" )

    # Diameters and heights on the right (-Y side).
    yr = -0.5 * W
    vdim( c, yr - 0.75 * W, zbot, ztop, "Diameter" )
    ext( c, [ 0, 0, ztop ], [ 0, yr - 0.8 * W, ztop ] )
    ext( c, [ 0, 0, zbot ], [ 0, yr - 0.8 * W, zbot ] )
    vdim( c, yr - 0.28 * W, axle - 0.5 * Drim, axle + 0.5 * Drim, "Drim" )
    ext( c, [ 0, -0.5 * Wrim, axle + 0.5 * Drim ], [ 0, yr - 0.32 * W, axle + 0.5 * Drim ] )
    ext( c, [ 0, -0.5 * Wrim, axle - 0.5 * Drim ], [ 0, yr - 0.32 * W, axle - 0.5 * Drim ] )
    # Shoulder height, above the rim.
    vdim( c, 0.5 * W + 0.18 * W, axle + 0.5 * Drim, zs, "Hs", side = "left" )
    ext( c, [ 0, 0.5 * Ws, zs ], [ 0, 0.5 * W + 0.22 * W, zs ] )
    ext( c, [ 0, 0.5 * Wrim, axle + 0.5 * Drim ], [ 0, 0.5 * W + 0.22 * W, axle + 0.5 * Drim ] )
    # Static loaded radius, axle to ground.
    vdim( c, 0.5 * W + 0.18 * W, 0.0, axle, "SLR", side = "left" )

    # Legend for the grown tire and the clearance envelope.
    lx = np.array( [ 0, -0.5 * W - 0.05 * W, zbot - 0.12 * D ] )
    c.line( np.array( [ lx, lx + [ 0, -0.25 * W, 0 ] ] ), color = GROWN, ls = ( 0, ( 4, 2 ) ), lw = 1.0 )
    c.text( lx + [ 0, -0.25 * W, 0 ], "grown tire", dx = 4, ha = "left", color = GROWN )
    lx2 = lx + [ 0, 0, -0.12 * D ]
    c.line( np.array( [ lx2, lx2 + [ 0, -0.25 * W, 0 ] ] ), color = CLEAR, ls = ( 0, ( 1, 1.5 ) ), lw = 1.0 )
    c.text( lx2 + [ 0, -0.25 * W, 0 ], "clearance envelope", dx = 4, ha = "left", color = CLEAR )

    pf.key( c, "ahead", at = ( 0.0, 0.0 ), dx = 6, dy = 6 )
    pf.save( fig, "Gear_tire_section", [ [ c ] ] )


TIRE_MODES = ( ( vsp.TIRE_TRA, "Full TRA" ), ( vsp.TIRE_FAIR_FLANGE, "TRA w/ Faired Flange" ),
               ( vsp.TIRE_FAIR_WHEEL, "TRA w/ Faired Wheel" ), ( vsp.TIRE_BALLOON, "Balloon Tire" ),
               ( vsp.TIRE_BALLOON_WHEEL, "Balloon Tire w/ Wheel" ), ( vsp.TIRE_BALLOON_FAIR_WHEEL, "Balloon Tire w/ Faired Wheel" ) )


def tire_modes():
    """The default tire in each Tire Mode, seen from ahead, cut through the axle."""
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    rows = [ [], [] ]
    for k, ( mode, name ) in enumerate( TIRE_MODES ):
        g, b = new_gear()
        vsp.SetParmVal( bparm( b, "TireMode", "Tire" ), mode )
        vsp.Update()
        top, bot = section( g, 0 )
        c = fl.Canvas( fig.add_axes( [ 0, 0, 1, 1 ] ), AHEAD )
        slr = vsp.GetParmVal( bparm( b, "StaticRadiusModel", "Tire" ) )
        W = vsp.GetParmVal( bparm( b, "WidthModel", "Tire" ) )
        c.line( np.array( [ [ 0, 0.75 * W, slr ], [ 0, -0.75 * W, slr ] ] ), color = "#888888", lw = 0.7,
                ls = ( 0, ( 8, 2, 2, 2 ) ), z = 40 )
        draw_section( c, top, bot )
        # One frame for every panel, so they share a scale and line up.
        D = vsp.GetParmVal( bparm( b, "DiameterModel", "Tire" ) )
        c.pts.append( c.v.p2( np.array( [ [ 0, 0.8 * W, slr - 0.5 * D ], [ 0, -0.8 * W, slr + 0.5 * D ] ] ) ) )
        pf.title( c, name, pad = 6 )
        rows[ k // 3 ].append( c )
    pf.key( rows[ 1 ][ 0 ], "ahead", at = ( 0.0, 0.0 ), dx = -30, dy = 0 )
    pf.save( fig, "Gear_tire_modes", rows )


BOGIE = { "NumAcross": 2, "NumTandem": 2, "XContactPt": 2.0, "YContactPt": 1.5, "Symmetrical": 1,
          "TravelCompressed": 0.4, "TravelExtended": 0.2 }


def bogie_gear():
    """An illustration gear: one symmetrical bogie, two across and two in tandem, with a short stroke."""
    g, b = new_gear()
    for k, v in BOGIE.items():
        vsp.SetParmVal( bparm( b, k, "Bogie" ), v )
    vsp.Update()
    return g, b


def leader( c, p, s, off, ha = "left", color = "k" ):
    """A label set off from its point by off (points), with a thin line back to it."""
    c.pts.append( c.v.p2( np.asarray( p, float ) )[ None, : ] )
    c.ax.annotate( s, xy = c.v.p2( p ), xytext = off, textcoords = "offset points", ha = ha, va = "center", color = color,
                   zorder = 90, annotation_clip = False,
                   arrowprops = dict( arrowstyle = "-", lw = 0.7, color = color, shrinkA = 2, shrinkB = 2 ) )


def origin_mark( c, label, dx = -6, dy = -8, ha = "right" ):
    q = c.v.p2( O )
    c.ax.plot( [ q[0] ], [ q[1] ], marker = "+", ms = 10, color = "k", mew = 1.2, zorder = 80 )
    c.text( O, label, dx = dx, dy = dy, ha = ha )


def bogie_top():
    """The bogie and its mirror from above, with the dimensions that place the tires."""
    g, b = bogie_gear()
    B = gm.Bogie( b )
    s = vsp.GetParmVal( bparm( b, "Spacing", "Bogie" ) )
    p = vsp.GetParmVal( bparm( b, "Pitch", "Bogie" ) )
    con = B.contact
    conm = con * np.array( [ 1, -1, 1 ] )

    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = pf.panel( fig, TOP )
    for i in range( vsp.GetNumMainSurfs( g ) ):
        c.surface( gm.grid( g, i, 33, 33 ), edge = False )
    c.flush()
    xr = con[0] + 0.5 * p + 1.5
    c.line( np.array( [ [ -0.9, 0, 0 ], [ xr, 0, 0 ] ] ), color = "#888888", lw = 0.7, ls = ( 0, ( 8, 2, 2, 2 ) ) )
    c.text( [ xr, 0, 0 ], "gear XZ plane", dx = 4, ha = "left", color = "#666666" )
    origin_mark( c, "gear origin", dx = -6, dy = 8 )
    c.point( con, color = "k", s = 10, z = 85 )
    c.point( conm, color = "k", s = 10, z = 85 )
    leader( c, con, "contact point", ( -70, 0 ), ha = "right" )
    leader( c, conm, "mirrored contact point", ( -70, 0 ), ha = "right" )
    # X Contact and Y Contact from the origin.
    yx = -0.22
    c.arrow( [ 0, yx, 0 ], [ con[0], yx, 0 ], color = DIM, lw = 0.8, head = 6, both = True )
    c.text( [ 0.5 * con[0], yx, 0 ], "X Contact", dy = -6, va = "top" )
    ext( c, con, [ con[0], yx - 0.08, 0 ] )
    xy = con[0] + 0.5 * p + 0.75
    c.arrow( [ xy, 0, 0 ], [ xy, con[1], 0 ], color = DIM, lw = 0.8, head = 6, both = True )
    c.text( [ xy, 0.5 * con[1], 0 ], "Y Contact", dx = -4, ha = "right" )
    ext( c, con, [ xy + 0.08, con[1], 0 ] )
    # Spacing across and Pitch in tandem, between tire centers.
    xs = xy + 0.55
    y0 = con[1] + 0.5 * s
    y1 = con[1] - 0.5 * s
    c.arrow( [ xs, y1, 0 ], [ xs, y0, 0 ], color = DIM, lw = 0.8, head = 6, both = True )
    c.text( [ xs, con[1], 0 ], "Spacing", dx = 4, ha = "left" )
    for yy in ( y0, y1 ):
        ext( c, [ con[0] + 0.5 * p, yy, 0 ], [ xs + 0.08, yy, 0 ] )
    yp = con[1] + 0.5 * s + 0.6
    c.arrow( [ con[0] - 0.5 * p, yp, 0 ], [ con[0] + 0.5 * p, yp, 0 ], color = DIM, lw = 0.8, head = 6, both = True )
    c.text( [ con[0], yp, 0 ], "Pitch", dy = 6, va = "bottom" )
    for xx in ( con[0] - 0.5 * p, con[0] + 0.5 * p ):
        ext( c, [ xx, y0, 0 ], [ xx, yp + 0.08, 0 ] )
    pf.key( c, "top", at = ( 0.0, 0.0 ), dx = -40, dy = 0 )
    pf.save( fig, "Gear_bogie_top", [ [ c ] ] )


def bogie_side():
    """The +Y bogie from the side: contact point, pivot and suspension travel."""
    g, b = bogie_gear()
    B = gm.Bogie( b )
    con = B.contact
    piv = B.pivot()
    p = vsp.GetParmVal( bparm( b, "Pitch", "Bogie" ) )

    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    d = pf.panel( fig, SIDE )
    for i in range( vsp.GetNumMainSurfs( g ) ):
        G = gm.grid( g, i, 33, 33 )
        if G[ :, :, 1 ].mean() > 0 and G[ :, :, 1 ].mean() > con[1]:
            d.surface( G, edge = False )
    d.flush()
    xr = con[0] + 0.5 * p + 1.4
    d.line( np.array( [ [ -0.9, 0, 0 ], [ xr, 0, 0 ] ] ), color = "#555555", lw = 1.2 )
    d.text( [ xr, 0, 0 ], "nominal ground plane", dx = 4, ha = "left", color = "#555555" )
    origin_mark( d, "gear origin", dx = 0, dy = 9, ha = "center" )
    ext_pt = piv - vsp.GetParmVal( bparm( b, "TravelExtended", "Bogie" ) ) * B.travel_dir
    com_pt = piv + vsp.GetParmVal( bparm( b, "TravelCompressed", "Bogie" ) ) * B.travel_dir
    d.line( np.array( [ ext_pt, com_pt ] ), color = "#00C000", lw = 2.0, z = 82 )
    for pt in ( ext_pt, piv, com_pt, con ):
        d.point( pt, color = "k", s = 12, z = 86 )
    leader( d, com_pt, "compressed", ( 60, 8 ) )
    leader( d, piv, "pivot", ( 60, 0 ) )
    leader( d, ext_pt, "extended", ( 60, -8 ) )
    leader( d, con, "contact point", ( 60, -22 ) )
    d.arrow( com_pt + 0.08 * B.travel_dir, com_pt + 0.55 * B.travel_dir, color = DIM, lw = 1.0, head = 7 )
    d.text( com_pt + 0.55 * B.travel_dir, "compression direction", dy = 7, va = "bottom" )
    xv = con[0] - 0.5 * p - 0.9
    d.arrow( [ xv, 0, 0 ], [ xv, 0, piv[2] ], color = DIM, lw = 0.8, head = 6, both = True )
    d.text( [ xv, 0, 0.5 * piv[2] ], "SLR", dx = -4, ha = "right" )
    ext( d, [ con[0], 0, piv[2] ], [ xv - 0.08, 0, piv[2] ] )
    pf.key( d, "front", at = ( 0.0, 0.0 ), dx = -40, dy = 0 )
    pf.save( fig, "Gear_bogie_side", [ [ d ] ] )


MECH = { "Mech_XLoc": 0.0, "Mech_YLoc": 0.0, "Mech_ZLoc": 2.6, "Mech_X_Axis": 0.0, "Mech_Y_Axis": 1.0, "Mech_Z_Axis": 0.0,
         "Mech_RetAngle": 90.0, "MechKneePos": 0.5, "MechKneeAngle": -100.0, "MechKRetract": 0.5 }


def mech_gear():
    """An illustration gear retracting by a mechanism: a trunnion above the axle, a knee halfway up the strut."""
    g, b = new_gear()
    vsp.SetParmVal( bparm( b, "RetMode", "Retract" ), vsp.GEAR_MECHANISM )
    vsp.Update()
    vsp.SetParmVal( bparm( b, "MechAbsRelFlag", "Retract" ), vsp.ABS )
    for k, v in MECH.items():
        vsp.SetParmVal( bparm( b, k, "Retract" ), v )
    vsp.SetParmVal( g, "GearConfigMode", "Gear", vsp.GEAR_CONFIGURATION_ALL )
    vsp.Update()
    return g, b


def outline( c, G, color, lw = 1.0, ls = "-", fill = None, z = 20 ):
    """A tire seen from the side: its outer edge (the crown circle) and its rim."""
    edge = G[ G.shape[0] // 2, : ]
    if fill is not None:
        c.ax.add_patch( Polygon( c.v.p2( edge ), closed = True, fc = fill, ec = "none", zorder = z ) )
    c.line( edge, color = color, lw = lw, ls = ls, z = z + 1 )


def retract():
    """A gear retracting by a mechanism, from the side: down, halfway and up."""
    g, b = mech_gear()
    B = gm.Bogie( b )
    k_half = MECH[ "MechKRetract" ]

    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = pf.panel( fig, SIDE )
    c.line( np.array( [ [ -2.6, 0, 0 ], [ 1.2, 0, 0 ] ] ), color = "#555555", lw = 1.2, z = 5 )
    c.text( [ 1.2, 0, 0 ], "ground", dx = 0, dy = -8, ha = "right", color = "#555555" )

    look = { 0.0: ( "#9A9A9A", "-", "#E6E6E6" ), k_half: ( "#555555", ( 0, ( 4, 2 ) ), None ), 1.0: ( "#0072B2", "-", "#CFE3F2" ) }
    for isurf, k in ( ( 0, 0.0 ), ( 1, 1.0 ), ( 2, k_half ) ):
        G = gm.grid( g, isurf, 41, 73 )
        col, ls, fill = look[ k ]
        # The model must place the tire where VSP does.
        cen, ax = gm.tire_center( g, isurf )
        M, knee = B.matrix( k )
        assert np.abs( cen - M[ :3, 3 ] ).max() < 1e-9, "gearmodel disagrees with VSP"
        outline( c, G, col, ls = ls, fill = fill )
        axle = M[ :3, 3 ]
        c.line( np.array( [ B.trunnion, knee, axle ] ), color = col, lw = 2.0, ls = ls, z = 40 )
        if k != k_half:
            c.point( knee, color = col, s = 8, z = 45 )
        c.point( axle, color = col, s = 8, z = 45 )

    # Labels.
    M0, knee0 = B.matrix( 0.0 )
    M1, knee1 = B.matrix( 1.0 )
    leader( c, knee0, "knee", ( 40, 0 ) )
    leader( c, M0[ :3, 3 ], "axle, down", ( 70, -6 ) )
    leader( c, M1[ :3, 3 ], "axle, up", ( -75, 14 ), ha = "right" )
    leader( c, knee1, "knee, up", ( -30, 20 ), ha = "right" )
    q = c.v.p2( B.trunnion )
    fl.out_of_page( c.ax, q, "", inward = True )
    c.pts.append( q[ None, : ] )
    c.text( B.trunnion, r"trunnion, axis along $+Y$", dx = 10, dy = 0, ha = "left" )
    # Retract Angle: the turn of the upper strut about the trunnion axis.
    c.angle( B.trunnion, knee0 - B.trunnion, knee1 - B.trunnion, 0.55, "k" )
    mid = B.trunnion + 0.55 * fl.unit( fl.unit( knee0 - B.trunnion ) + fl.unit( knee1 - B.trunnion ) )
    leader( c, mid, "Retract Angle", ( 70, -10 ) )
    Mh, kneeh = B.matrix( k_half )
    leader( c, Mh[ :3, 3 ], "halfway, Retract 0.5", ( -60, -50 ), ha = "right" )
    # Knee Rotation: the turn of the lower strut at the knee, measured from the upper strut's line.
    up_dir = knee1 - B.trunnion
    c.line( np.array( [ knee1, knee1 + 0.7 * fl.unit( up_dir ) ] ), color = "#0072B2", lw = 0.7, ls = ( 0, ( 2, 2 ) ), z = 41 )
    c.angle( knee1, up_dir, M1[ :3, 3 ] - knee1, 0.4, "#0072B2", label = "Rotation (negative)", label_r = 1.6, dx = -10, dy = -4 )
    pf.key( c, "front", at = ( 0.0, 0.0 ), dx = 0, dy = 22 )
    pf.save( fig, "Gear_retract", [ [ c ] ] )


NOSE = { "NumAcross": 2, "NumTandem": 1, "XContactPt": -6.0, "YContactPt": 0.0, "Symmetrical": 0 }
MAIN = { "NumAcross": 2, "NumTandem": 2, "XContactPt": 6.0, "YContactPt": 4.5, "Symmetrical": 1 }
NOSE_TIRE = { "DiameterIn": 22.0, "WidthIn": 6.6 }
MAIN_TIRE = { "DiameterIn": 34.0, "WidthIn": 10.0 }
CG = { "XCGNominalLocal": 3.5, "XCGMinLocal": 2.0, "XCGMaxLocal": 4.5, "YCGNominalLocal": 0.0, "YCGMinLocal": -0.8,
       "YCGMaxLocal": 0.8, "ZCGNominalLocal": 6.0, "ZCGMinLocal": 5.2, "ZCGMaxLocal": 6.6 }


def tricycle_gear():
    """An illustration gear: a nose bogie and a symmetrical main bogie, with a center of gravity range."""
    vsp.VSPRenew()
    g = vsp.AddGeom( "GEAR" )
    vsp.SetParmVal( g, "ShowNominalGroundPlane", "GroundPlane", 1 )
    vsp.SetParmVal( g, "AutoPlaneFlag", "GroundPlane", 0 )
    vsp.SetParmVal( g, "PlaneSize", "GroundPlane", 9.0 )
    vsp.SetParmVal( g, "CGLocalFlag", "GroundPlane", 1 )
    # Each bound limits the next, as of the last Update: maximum, then nominal, then minimum.
    for which in ( "Max", "Nominal", "Min" ):
        for k, v in CG.items():
            if which in k:
                vsp.SetParmVal( g, k, "GroundPlane", v )
        vsp.Update()
    for parms, tire in ( ( NOSE, NOSE_TIRE ), ( MAIN, MAIN_TIRE ) ):
        b = vsp.CreateAndAddBogie( g )
        for k, v in parms.items():
            vsp.SetParmVal( bparm( b, k, "Bogie" ), v )
        for k, v in tire.items():
            vsp.SetParmVal( bparm( b, k, "Tire" ), v )
    vsp.Update()
    for k, v in CG.items():
        assert abs( vsp.GetParmVal( g, k, "GroundPlane" ) - v ) < 1e-12, k
    return g


OVERVIEW = pf.view_from( [ -1.0, -2.2, 1.1 ] )


def overview( v = None, name = "Gear_overview" ):
    """A Gear from above and behind: its nominal ground plane, a nose bogie, a symmetrical main bogie and the center of
    gravity range."""
    g = tricycle_gear()
    cg = { k: vsp.GetParmVal( g, k, "GroundPlane" ) for k in CG }
    lo = np.array( [ cg[ "XCGMinLocal" ], cg[ "YCGMinLocal" ], cg[ "ZCGMinLocal" ] ] )
    hi = np.array( [ cg[ "XCGMaxLocal" ], cg[ "YCGMaxLocal" ], cg[ "ZCGMaxLocal" ] ] )
    nom = np.array( [ cg[ "XCGNominalLocal" ], cg[ "YCGNominalLocal" ], cg[ "ZCGNominalLocal" ] ] )

    fig = plt.figure( figsize = ( fl.PAGE_W, 4.5 ) )
    c = pf.panel( fig, v if v is not None else OVERVIEW )
    plane = None
    for i in range( vsp.GetNumMainSurfs( g ) ):
        G = gm.grid( g, i, 49, 49 )
        if np.ptp( G[ :, :, 2 ] ) < 1e-9 and np.ptp( G[ :, :, 0 ] ) > 5.0:
            plane = G
            c.surface( G, color = "#EEF0F2", edge = False )
        else:
            c.surface( G, edge = False )
    c.flush()
    assert plane is not None
    # The plane's outline and a few lines across it, so it reads as a plane.
    P = plane.reshape( -1, 3 )
    x0, x1 = P[ :, 0 ].min(), P[ :, 0 ].max()
    y0, y1 = P[ :, 1 ].min(), P[ :, 1 ].max()
    for t in np.linspace( 0.0, 1.0, 7 ):
        x = x0 + t * ( x1 - x0 )
        y = y0 + t * ( y1 - y0 )
        c.line( np.array( [ [ x, y0, 0 ], [ x, y1, 0 ] ] ), color = "#B8BEC6", lw = 0.5, z = 2 )
        c.line( np.array( [ [ x0, y, 0 ], [ x1, y, 0 ] ] ), color = "#B8BEC6", lw = 0.5, z = 2 )
    # The center of gravity range, a box, and the nominal point.
    corners = [ np.array( [ a, b, z ] ) for a in ( lo[0], hi[0] ) for b in ( lo[1], hi[1] ) for z in ( lo[2], hi[2] ) ]
    for i in range( 8 ):
        for j in range( i + 1, 8 ):
            if np.count_nonzero( corners[i] != corners[j] ) == 1:
                c.line( np.array( [ corners[i], corners[j] ] ), color = "#707070", lw = 0.9, z = 60 )
    c.line( np.array( [ nom, nom * np.array( [ 1, 1, 0 ] ) ] ), color = "#A0A0A0", lw = 0.6, ls = ( 0, ( 4, 2 ) ), z = 55 )
    c.point( nom, color = "#505050", s = 16, z = 70 )
    leader( c, hi, "center of gravity and its range", ( 30, 20 ) )
    origin_mark( c, "gear origin", dx = 0, dy = -10, ha = "center" )
    nose = np.array( [ NOSE[ "XContactPt" ], 0.0, 0.0 ] )
    main = np.array( [ MAIN[ "XContactPt" ], MAIN[ "YContactPt" ], 0.0 ] )
    leader( c, nose + np.array( [ 0, 0.0, 1.0 ] ), "nose bogie", ( -40, 30 ), ha = "right" )
    leader( c, main + np.array( [ -1.2, 0.0, 1.6 ] ), "main bogie", ( -40, 34 ), ha = "right" )
    leader( c, main * np.array( [ 1, -1, 1 ] ) + np.array( [ 0.8, 0.0, 1.6 ] ), "its mirror image (Symmetrical)", ( 10, -60 ),
            ha = "center" )
    c.text( np.array( [ 0.5 * ( x0 + x1 ), y0, 0 ] ), "nominal ground plane", dy = -8, va = "top", color = "#555555" )
    pf.save( fig, name, [ [ c ] ] )


KNEE = { "Mech_ZLoc": 4.0, "Mech_X_Axis": 0.0, "Mech_Y_Axis": 1.0, "Mech_Z_Axis": -0.4, "MechKneeAzimuthAngle": 30.0,
         "MechKneeElevationAngle": 20.0, "MechKneeDownAngle": 180.0 }
KNEE_COL = "#0072B2"
KNEE_C1 = "#1F4E79"
GREY = "#777777"


def knee_axis():
    """The knee axis built from the trunnion axis: its part square to the strut, turned by Azimuth Angle about the strut
    and tilted by Elevation Angle toward the trunnion, each angle the path of the tip of one vector."""
    g, b = mech_gear()
    for k, v in KNEE.items():
        vsp.SetParmVal( bparm( b, k, "Retract" ), v )
    vsp.Update()
    B = gm.Bogie( b )
    # The model must place the tires where VSP does, down, up and between.
    for isurf, k in ( ( 0, 0.0 ), ( 1, 1.0 ), ( 2, MECH[ "MechKRetract" ] ) ):
        cen, ax = gm.tire_center( g, isurf )
        M, knee = B.matrix( k )
        assert np.abs( cen - M[ :3, 3 ] ).max() < 1e-9, "gearmodel disagrees with VSP"
    pv = B.pivot()
    tr = B.trunnion
    ud = gm.unit( tr - pv )
    s = -ud                                            # the strut, from the trunnion down to the pivot
    n = gm.unit( np.cross( B.axis, ud ) )
    base = gm.unit( np.cross( ud, n ) )                # the part of the trunnion axis square to the strut
    az, el = B.knee_az, B.knee_el
    mid = gm.rot( s, az )[ :3, :3 ] @ base
    n2 = gm.unit( np.cross( mid, ud ) )                # tilting about n2 by a positive angle lifts mid toward the trunnion
    fin = gm.rot( n2, el )[ :3, :3 ] @ mid
    # Turned about the strut first, then tilted, it is the knee axis the model (and VSP) uses.
    assert abs( base @ ud ) < 1e-12 and np.linalg.norm( np.cross( base, B.axis - ( B.axis @ ud ) * ud ) ) < 1e-12
    assert np.abs( fin - B.knee_axis() ).max() < 1e-12
    assert fin @ ud > 0.0 and np.cross( base, mid ) @ s > 0.0
    kp = pv + B.knee_pos * ( tr - pv )

    LA = 1.2
    LG = 0.8
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.4 ) )
    c = pf.panel( fig, pf.ISO )
    G = gm.grid( g, 0, 33, 49 )
    c.surface( G, edge = False )
    c.flush()
    # The strut, the trunnion and its axis.
    c.line( np.array( [ tr, pv ] ), color = "#555555", lw = 2.4, z = 40 )
    c.point( tr, color = "k", s = 14, z = 85 )
    c.point( pv, color = "k", s = 10, z = 85 )
    ta = gm.unit( B.axis )
    c.arrow( tr - 0.9 * ta, tr + 1.1 * ta, color = "k", lw = 1.0, head = 7, ls = ( 0, ( 8, 2, 2, 2 ) ), z = 41 )
    c.text( tr + 1.1 * ta, "trunnion axis", dx = -8, ha = "right" )
    leader( c, tr, "trunnion point", ( 40, 12 ) )
    leader( c, pv, "pivot", ( 40, -10 ) )
    # The trunnion axis again at the knee, and its part square to the strut, the start of the construction.
    c.line( np.array( [ kp, kp + LA * ta ] ), color = "#9a9a9a", lw = 0.9, ls = ( 0, ( 8, 2, 2, 2 ) ), z = 42 )
    c.line( np.array( [ kp + LA * ta, kp + LA * ( ta @ base ) * base ] ), color = "#9a9a9a", lw = 0.6, ls = ( 0, ( 2, 2 ) ), z = 42 )
    # The axis turned about: the strut, toward the pivot, with its curl; and the tilt axis.
    c.hinge( kp, s, KNEE_C1, half = LG, start = 0.0, rho = 0.12, at = 0.6, lw = 1.3 )
    c.text( kp + ( LG + 0.25 ) * s, "strut, trunnion to pivot", color = KNEE_C1, dx = -8, ha = "right" )
    c.hinge( kp, n2, KNEE_COL, half = LG, start = 0.0, rho = 0.12, at = 0.6, lw = 1.3 )
    c.text( kp + ( LG + 0.25 ) * n2, "tilt axis", color = KNEE_COL, dx = 8, ha = "left" )
    # The start (dashed), after Azimuth (dotted) and the knee axis, each LA each way through the knee.
    c.arrow( kp - LA * base, kp + LA * base, color = GREY, lw = 1.3, head = 8, ls = ( 0, ( 4, 2 ) ), z = 50 )
    c.arrow( kp - LA * mid, kp + LA * mid, color = GREY, lw = 1.3, head = 8, ls = ( 0, ( 1, 1.5 ) ), z = 50 )
    c.arrow( kp - LA * fin, kp + LA * fin, color = KNEE_COL, lw = 2.4, head = 11, z = 51 )
    c.point( kp, color = KNEE_COL, s = 18, z = 86 )
    ts = np.linspace( 0.0, 1.0, 40 )
    P1 = np.array( [ kp + LA * ( gm.rot( s, az * k )[ :3, :3 ] @ base ) for k in ts ] )
    P2 = np.array( [ kp + LA * ( gm.rot( n2, el * k )[ :3, :3 ] @ mid ) for k in ts ] )
    for P, name, col, off in ( ( P1, "Azimuth Angle", KNEE_C1, ( -30, 24 ) ), ( P2, "Elevation Angle", KNEE_COL, ( 40, 6 ) ) ):
        c.arrow_path( P, color = col, lw = 2.0, head = 9 )
        leader( c, P[ 20 ], name, off, ha = "left" if off[ 0 ] > 0 else "right", color = col )
    leader( c, kp + 0.75 * LA * base, "trunnion axis, square to the strut", ( -30, -36 ), ha = "right" )
    leader( c, kp - 0.75 * LA * fin, "knee axis", ( 50, -6 ), color = KNEE_COL )
    leader( c, kp + 0.6 * LA * ta, "trunnion axis, at the knee", ( -40, -30 ), ha = "right" )
    leader( c, kp, "knee", ( 60, 26 ) )
    pf.save( fig, "Gear_knee_axis", [ [ c ] ] )


FIGS = {
    "overview": ( overview, ),
    "tire": ( tire_section, tire_modes ),
    "bogie": ( bogie_top, bogie_side ),
    "retract": ( retract, knee_axis ),
}

if __name__ == "__main__":
    names = sys.argv[ 1: ] or list( FIGS )
    pf.style()
    for n in names:
        for fn in FIGS[ n ]:
            fn()
