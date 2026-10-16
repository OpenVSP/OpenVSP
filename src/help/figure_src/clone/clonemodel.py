"""Nacelles and their Clones made through the openvsp module, for the Clone help figures and checks."""

import numpy as np
import openvsp as vsp

Y0 = 5.0          # the original nacelle's Y; it is mirrored in Y = 0
DX = 10.0         # each Clone sits this much further aft than the one before


def nacelle():
    """A fresh model with one nacelle, a Pod at Y = Y0 mirrored in Y = 0."""
    vsp.VSPRenew()
    p = vsp.AddGeom( "POD" )
    vsp.SetParmVal( p, "Length", "Design", 6.0 )
    vsp.SetParmVal( p, "FineRatio", "Design", 4.0 )
    vsp.SetParmVal( p, "Y_Rel_Location", "XForm", Y0 )
    vsp.SetParmVal( p, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
    vsp.Update()
    return p


def clone( p, x, y ):
    """A Clone of p made with the Clone button, moved to X = x, Y = y."""
    c = vsp.CloneGeomVec( [ p ], "_Clone" )[ 0 ]
    vsp.Update()
    vsp.SetParmVal( c, "X_Rel_Location", "XForm", x )
    vsp.SetParmVal( c, "Y_Rel_Location", "XForm", y )
    vsp.Update()
    return c


def single( c ):
    """The remedy for a Clone on the mirror plane: stop copying the symmetry and turn the Clone's own off."""
    vsp.SetParmVal( c, "CloneSym", "Behavior", 0.0 )
    vsp.Update()
    vsp.SetParmVal( c, "Sym_Planar_Flag", "Sym", 0.0 )
    vsp.Update()


def grid( g, s, nu = 41, nw = 41 ):
    """Points ( nu, nw, 3 ) of surface s of Geom g."""
    return np.array( [ [ list( vsp.CompPnt01( g, s, u, w ) ) for w in np.linspace( 0, 1, nw ) ]
                       for u in np.linspace( 0, 1, nu ) ] )


def surfs( g, nu = 41, nw = 41 ):
    """Every surface of g, symmetric copies included."""
    return [ grid( g, s, nu, nw ) for s in range( vsp.GetTotalNumSurfs( g ) ) ]


def centers( g ):
    """The centroid of each surface's sample points."""
    return [ G.reshape( -1, 3 ).mean( 0 ) for G in surfs( g, 9, 9 ) ]
