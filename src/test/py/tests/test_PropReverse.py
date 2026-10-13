"""A propeller with reverse rotation is the mirror image, through the plane of its rotation axis
and blade, of the same propeller turning the normal way.
"""

import math

import openvsp as vsp

from prophelp import group

UW = [ ( u, w ) for u in ( 0.05, 0.3, 0.6, 0.95 ) for w in ( 0.0, 0.2, 0.5, 0.7, 0.9 ) ]
TOL = 1e-9


def new_prop( parms, rev ):
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    vsp.SetParmVal( pid, "UseBeta34Flag", "Design", 0.0 )
    vsp.SetParmVal( pid, "ReverseFlag", "Design", float( rev ) )
    if "NumBlade" in parms:
        # The per-blade parms are made by an Update.
        vsp.SetParmVal( pid, "NumBlade", "Design", parms["NumBlade"] )
        vsp.Update()
    for name, val in parms.items():
        assert vsp.FindParm( pid, name, group( name ) ), name
        vsp.SetParmVal( pid, name, group( name ), val )
    vsp.Update()
    return pid


def blade( pid, surf = 0 ):
    pts = []
    for u, w in UW:
        p = vsp.CompPnt01( pid, surf, u, w )
        pts.append( ( p.x(), p.y(), p.z() ) )
    return pts


def mirror_error( a, b ):
    """Farthest point of a from the mirror image of b, in the plane z = 0."""
    mb = [ ( x, y, -z ) for x, y, z in b ]
    return max( min( math.dist( p, q ) for q in mb ) for p in a )


def reverse_mirror_error( parms, surf = 0 ):
    a = blade( new_prop( parms, False ), surf )
    b = blade( new_prop( parms, True ), surf )
    return mirror_error( a, b )


def test_feather_offset_mirrors():
    for parms in ( { "FeatherOffsetXoC": 0.2 },
                   { "FeatherOffsetXoC": -0.3, "FeatherAxisXoC": 0.3, "Feather": 15.0 } ):
        assert reverse_mirror_error( parms ) < TOL, parms
