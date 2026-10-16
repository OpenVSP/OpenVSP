"""A wing and a point cloud above it, made through the openvsp module, for the Point Cloud help figures and checks."""

import numpy as np
import openvsp as vsp

ZC = 2.5                                       # the height of the cloud above the wing
STATIONS = ( 1.0, 3.0, 5.0, 7.0, 8.5 )         # spanwise stations of the points over the wing, which has span 9
MISS = ( 4.5, 10.5 )                           # X, Y of the point beyond the tip


def wing():
    """A default wing, its right half only."""
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0.0 )
    vsp.Update()
    return w


def le_te( w, y ):
    """Leading and trailing edge X of the wing's upper surface at span station y (the planform is a trapezoid)."""
    span = vsp.GetParmVal( vsp.GetXSecParm( vsp.GetXSec( vsp.GetXSecSurf( w, 0 ), 1 ), "Span" ) )
    f = y / span
    le = [ np.array( list( vsp.CompPnt01( w, 0, u, 0.5 ) ) ) for u in ( 1.0 / 3.0, 2.0 / 3.0 ) ]
    te = [ np.array( list( vsp.CompPnt01( w, 0, u, 0.0 ) ) ) for u in ( 1.0 / 3.0, 2.0 / 3.0 ) ]
    return le[0][0] + f * ( le[1][0] - le[0][0] ), te[0][0] + f * ( te[1][0] - te[0][0] )


def cloud_points( w ):
    """Points above the wing at 40 percent of the chord, and one beyond the tip."""
    P = []
    for y in STATIONS:
        le, te = le_te( w, y )
        P.append( ( le + 0.4 * ( te - le ), y, ZC ) )
    P.append( ( MISS[0], MISS[1], ZC ) )
    return np.array( P )


def projected( w, P ):
    """The cloud made from P and projected onto the wing along Z: its ID and its points afterward."""
    c = vsp.CreatePtCloudGeomFromPts( [ vsp.vec3d( *p ) for p in P ], "Outline" )
    vsp.Update()
    vsp.ProjectPtCloudPts( c, w, 0, vsp.Z_DIR )
    vsp.Update()
    return c, np.array( [ [ q.x(), q.y(), q.z() ] for q in vsp.GetPtCloudPnts( c ) ] )


def grid( w, s = 0, nu = 31, nw = 61 ):
    return np.array( [ [ list( vsp.CompPnt01( w, s, u, t ) ) for t in np.linspace( 0, 1, nw ) ]
                       for u in np.linspace( 0, 1, nu ) ] )
