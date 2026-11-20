# A section between two of a propeller's is the two blended, every shape Parm interpolated.

import numpy as np
import openvsp as vsp
import pytest


def prop_section( kind, root, tip, n=3000 ):
    """Points around mid blade of a propeller of two sections whose Parms differ only as root and tip say."""
    vsp.VSPRenew()
    pid = vsp.AddGeom( "PROP" )
    surf = vsp.GetXSecSurf( pid, 0 )
    while vsp.GetNumXSec( surf ) > 2:
        vsp.CutXSec( pid, 1 )
    for i, parms in enumerate( ( root, tip ) ):
        vsp.ChangeXSecShape( surf, i, kind )
        xs = vsp.GetXSec( surf, i )
        for name, val in parms.items():
            vsp.SetParmVal( vsp.GetXSecParm( xs, name ), val )
    vsp.Update()
    pts = [ vsp.CompPnt01( pid, 0, 0.5, j / n ) for j in range( n + 1 ) ]
    return np.array( [ [ p.x(), p.y(), p.z() ] for p in pts ] )


def apart( a, b ):
    """How far apart two sections are: the farthest any point of one is from the other's points."""
    d = np.linalg.norm( a[ :, None, : ] - b[ None, :, : ], axis=2 )
    return max( d.min( axis=1 ).max(), d.min( axis=0 ).max() )


@pytest.mark.parametrize( "name,a,b", [ ( "ThickLoc", 0.3, 0.7 ), ( "ThickLocLow", 0.3, 0.7 ), ( "ZCamber", 0.0, 0.02 ),
                                        ( "FlatUp", 0.0, 0.3 ), ( "FlatLow", 0.0, 0.3 ), ( "UForeUp", 0.75, 0.9 ),
                                        ( "DuUp", 0.05, 0.2 ), ( "UForeLow", 0.1, 0.25 ), ( "DuLow", 0.05, 0.2 ) ] )
def testAWedgeBetweenTwoPropellerSectionsBlendsThem( name, a, b ):
    # Mid blade is made from the two sections blended; were a Parm not blended, it would keep the root's value there.
    kind = vsp.XS_WEDGE
    base = { "SymmThick": 0 }
    if name in ( "FlatLow", "DuLow" ):
        base[ "FlatLow" ] = 0.2
    if name in ( "FlatUp", "DuUp" ):
        base[ "FlatUp" ] = 0.2
    flat = prop_section( kind, dict( base, **{ name: a } ), dict( base, **{ name: a } ) )
    full = prop_section( kind, dict( base, **{ name: a } ), dict( base, **{ name: b } ) )
    flat_tip = prop_section( kind, dict( base, **{ name: b } ), dict( base, **{ name: b } ) )
    # Mid blade is between the root's shape and the tip's, clear of both.
    across = apart( flat, flat_tip )
    assert across > 1e-3
    assert apart( flat, full ) > 0.2 * across
    assert apart( full, flat_tip ) > 0.2 * across
