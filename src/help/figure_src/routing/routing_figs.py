"""Figure for the OpenVSP Routing help page (src/help/Routing.md), written to ../../images/Routing_*.png.

    <build>/venv/bin/python routing_figs.py

Uses the drawing helpers of ../propeller (figlib, and the views and save() of propeller_figs).  The pod, the wing
and the route are VSP's own: surfaces sampled with CompVecPnt01, the route from GetRoutingCurve.  The 3D view is
OpenVSP's frame: Z up, Y into the page, X to the right, seen from about (-1, -1, 1).
"""

import os
import sys

HERE = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert( 0, os.path.join( HERE, "..", "propeller" ) )

import numpy as np
import matplotlib
matplotlib.use( "Agg" )
import matplotlib.pyplot as plt

import openvsp as vsp

import figlib as fl
import propeller_figs as pf

ROUTE = "#D55E00"

# Each point: the Geom it lies on, U, W, an offset in model axes, and the corner radius.
POINTS = ( ( "pod", 0.12, 0.0, ( 0.0, 0.0, 0.35 ), 0.0 ),
           ( "pod", 0.42, 0.1, ( 0.0, 0.0, 0.0 ), 0.0 ),
           ( "wing", 0.36, 0.62, ( 0.0, 0.0, 0.0 ), 1.0 ),
           ( "wing", 0.62, 0.62, ( 0.0, 0.0, 0.0 ), 0.0 ) )


def grid( gid, isurf, nu, nw, u0 = 0.0, u1 = 1.0 ):
    us = np.linspace( u0, u1, nu )
    ws = np.linspace( 0.0, 1.0, nw )
    U, W = np.meshgrid( us, ws, indexing = "ij" )
    pts = vsp.CompVecPnt01( gid, isurf, list( U.ravel() ), list( W.ravel() ) )
    return np.array( [ [ p.x(), p.y(), p.z() ] for p in pts ] ).reshape( nu, nw, 3 )


def v3( p ):
    return np.array( [ p.x(), p.y(), p.z() ] )


def model():
    vsp.VSPRenew()
    geoms = { "pod": vsp.AddGeom( "POD" ), "wing": vsp.AddGeom( "WING" ) }
    vsp.SetParmVal( geoms[ "wing" ], "X_Rel_Location", "XForm", 4.0 )
    vsp.Update()
    r = vsp.AddGeom( "ROUTING" )
    ids = []
    for name, u, w, off, rad in POINTS:
        pt = vsp.AddRoutingPt( r, geoms[ name ], 0 )
        vsp.SetParmVal( vsp.FindParm( pt, "U", "RoutePt" ), u )
        vsp.SetParmVal( vsp.FindParm( pt, "W", "RoutePt" ), w )
        vsp.SetParmVal( vsp.FindParm( pt, "DeltaType", "RoutePt" ), vsp.ABS )
        for k, n in enumerate( ( "DeltaX", "DeltaY", "DeltaZ" ) ):
            vsp.SetParmVal( vsp.FindParm( pt, n, "RoutePt" ), off[ k ] )
        vsp.SetParmVal( vsp.FindParm( pt, "Radius", "RoutePt" ), rad )
        ids.append( pt )
    vsp.Update()
    return geoms, r, ids


def route():
    """A route over a pod and onto a wing: four points, one of them rounded, one set off from its surface."""
    geoms, r, ids = model()
    fig = plt.figure( figsize = ( fl.PAGE_W, 4.0 ) )
    c = pf.panel( fig, pf.ISO )
    c.surface( grid( geoms[ "pod" ], 0, 41, 33 ), edge = False )
    c.surface( grid( geoms[ "wing" ], 0, 33, 41, 0.0, 0.75 ), edge = False )
    c.flush()

    crv = np.array( [ v3( p ) for p in vsp.GetRoutingCurve( r, 0 ) ] )
    corners = np.array( [ v3( vsp.GetMainRoutingPtCoord( p ) ) for p in ids ] )
    c.line( corners, color = ROUTE, lw = 0.7, ls = ( 0, ( 3, 2 ) ), z = 60 )
    c.line( crv, color = ROUTE, lw = 2.2, z = 61 )
    offs = ( ( -12, 10 ), ( -4, 12 ), ( 12, -4 ), ( 10, 10 ) )
    for k, p in enumerate( corners ):
        c.point( p, color = ROUTE, s = 14, z = 70 )
        c.text( p, str( k + 1 ), color = ROUTE, dx = offs[ k ][0], dy = offs[ k ][1] )
    # Point 1 is set off from where it lies on the pod.
    name, u, w, off, rad = POINTS[ 0 ]
    anchor = v3( vsp.CompPnt01( geoms[ name ], 0, u, w ) )
    assert np.linalg.norm( anchor + np.array( off ) - corners[0] ) < 1e-9, "the offset is not added in model axes"
    c.line( np.array( [ anchor, corners[0] ] ), color = "k", lw = 0.8, ls = ( 0, ( 1, 1.5 ) ), z = 65 )
    c.point( anchor, color = "k", s = 8, z = 66 )
    pf.leader( c, anchor, "point on the surface", ( -20, -26 ) )
    pf.leader( c, 0.5 * ( anchor + corners[0] ), "offset", ( -50, 12 ) )
    # The rounded corner at point 3.
    pf.leader( c, crv[ np.argmin( np.linalg.norm( crv - corners[2], axis = 1 ) ) ], "rounded by Radius", ( 40, -30 ) )
    pf.save( fig, "Routing_route", [ [ c ] ] )


FIGS = { "route": ( route, ) }

if __name__ == "__main__":
    names = sys.argv[ 1: ] or list( FIGS )
    pf.style()
    for n in names:
        for fn in FIGS[ n ]:
            fn()
