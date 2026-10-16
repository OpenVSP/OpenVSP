"""Wings made through the openvsp module, and samples of their surfaces, for the Wing help figures."""

import numpy as np
import openvsp as vsp


def new_wing( sym = False, root_cap = vsp.FLAT_END_CAP, tip_cap = vsp.FLAT_END_CAP ):
    """A fresh default wing.  sym False keeps only the right half."""
    vsp.VSPRenew()
    w = vsp.AddGeom( "WING" )
    if not sym:
        vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0.0 )
    vsp.SetParmVal( w, "CapUMinOption", "EndCap", root_cap )
    vsp.SetParmVal( w, "CapUMaxOption", "EndCap", tip_cap )
    vsp.Update()
    # Changing the symmetry leaves the new totals marked as changed through the API; a second Update consumes
    # them, so the next section edit is not rescaled to hold them.
    vsp.Update()
    return w


def xsec( w, i ):
    return vsp.GetXSec( vsp.GetXSecSurf( w, 0 ), i )


def setx( w, i, name, val ):
    """Set a parm of wing section (or airfoil) i."""
    vsp.SetParmVal( vsp.GetXSecParm( xsec( w, i ), name ), val )
    vsp.Update()


def getx( w, i, name ):
    return vsp.GetParmVal( vsp.GetXSecParm( xsec( w, i ), name ) )


def setw( w, name, val ):
    vsp.SetParmVal( w, name, "WingGeom", val )
    vsp.Update()


def getw( w, name ):
    return vsp.GetParmVal( w, name, "WingGeom" )


def nsect( w ):
    """Number of wing sections (one fewer than airfoils)."""
    return vsp.GetNumXSec( vsp.GetXSecSurf( w, 0 ) ) - 1


def ncaps( w ):
    n = 0
    for p in ( "CapUMinOption", "CapUMaxOption" ):
        if vsp.GetParmVal( w, p, "EndCap" ) != vsp.NO_END_CAP:
            n += 1
    return n


def u01( w, U ):
    """VSP's 0-1 u of the main surface at airfoil station U (0 = root airfoil, nsect = tip airfoil)."""
    off = 0
    if vsp.GetParmVal( w, "CapUMinOption", "EndCap" ) != vsp.NO_END_CAP:
        off = 1
    return ( U + off ) / ( nsect( w ) + ncaps( w ) )


def pnt( w, u, t, surf = 0 ):
    return np.array( list( vsp.CompPnt01( w, surf, u, t ) ) )


def grid( w, U0, U1, nu = 41, nw = 81, surf = 0 ):
    """Points ( nu, nw, 3 ) between airfoil stations U0 and U1; w runs TE lower, LE, TE upper."""
    us = [ u01( w, U ) for U in np.linspace( U0, U1, nu ) ]
    ts = np.linspace( 0.0, 1.0, nw )
    return np.array( [ [ pnt( w, u, t, surf ) for t in ts ] for u in us ] )


def grid01( w, u0, u1, nu = 41, nw = 81, surf = 0 ):
    """Points ( nu, nw, 3 ) between VSP 0-1 u values u0 and u1 (for the caps)."""
    ts = np.linspace( 0.0, 1.0, nw )
    return np.array( [ [ pnt( w, u, t, surf ) for t in ts ] for u in np.linspace( u0, u1, nu ) ] )


def foil( w, i, nw = 161, surf = 0 ):
    """Airfoil i as placed."""
    u = u01( w, i )
    return np.array( [ pnt( w, u, t, surf ) for t in np.linspace( 0.0, 1.0, nw ) ] )


def lete( w, i, surf = 0 ):
    """Leading and trailing edge points of airfoil i."""
    u = u01( w, i )
    return pnt( w, u, 0.5, surf ), pnt( w, u, 0.0, surf )


def winglet_example( blend = True ):
    """The page's example of a root fillet and a blended winglet: a short fillet section, the main wing, a short
    transition section that turns half the way to the winglet's dihedral, and the winglet.  blend False leaves every
    edge straight."""
    w = new_wing( root_cap = vsp.NO_END_CAP, tip_cap = vsp.NO_END_CAP )
    for k in range( 3 ):
        vsp.SplitWingXSec( w, 1 )
        vsp.Update()
    setx( w, 1, "Root_Chord", 6.0 )
    for i, b, sw, dh, ct in ( ( 1, 1.5, 60.0, 0.0, 4.0 ), ( 2, 8.0, 30.0, 4.0, 1.6 ), ( 3, 0.4, 30.0, 42.0, 1.5 ),
                              ( 4, 2.0, 40.0, 80.0, 0.8 ) ):
        setx( w, i, "Span", b )
        setx( w, i, "Sweep", sw )
        setx( w, i, "Dihedral", dh )
        setx( w, i, "Tip_Chord", ct )
    if blend:
        setx( w, 0, "OutLEMode", vsp.BLEND_ANGLES )
        setx( w, 0, "OutLESweep", 0.0 )
        setx( w, 0, "OutTEMode", vsp.BLEND_ANGLES )
        setx( w, 0, "OutTESweep", 0.0 )
        setx( w, 1, "InLEMode", vsp.BLEND_MATCH_OUT_LE_TRAP )
        setx( w, 1, "InTEMode", vsp.BLEND_MATCH_OUT_TE_TRAP )
        setx( w, 2, "OutLEMode", vsp.BLEND_MATCH_IN_LE_TRAP )
        setx( w, 2, "OutTEMode", vsp.BLEND_MATCH_IN_TE_TRAP )
        setx( w, 3, "InLEMode", vsp.BLEND_MATCH_OUT_LE_TRAP )
        setx( w, 3, "InTEMode", vsp.BLEND_MATCH_OUT_TE_TRAP )
        # A side taken out of FREE keeps the Strength FREE showed; start every blended side from 1.
        for i, side in ( ( 0, "Out" ), ( 1, "In" ), ( 2, "Out" ), ( 3, "In" ) ):
            for e in ( "LE", "TE" ):
                setx( w, i, side + e + "Strength", 1.0 )
    return w
