"""Fit CST keeps a section's shape and place."""

import numpy as np
import openvsp as vsp


def tip_section( wid ):
    vsp.Update()
    return np.array( [ [ p.x(), p.y(), p.z() ] for p in ( vsp.CompPnt01( wid, 0, 1.0, k / 2000.0 ) for k in range( 2001 ) ) ] )


def gap( a, b ):
    """The largest distance from a point of a to the polyline b."""
    d = b[ 1: ] - b[ :-1 ]
    keep = np.einsum( "ij,ij->i", d, d ) > 1e-24
    b0 = b[ :-1 ][ keep ]
    d = d[ keep ]
    dd = np.einsum( "ij,ij->i", d, d )
    worst = 0.0
    for p in a[ ::10 ]:
        t = np.clip( np.einsum( "ij,ij->i", p - b0, d ) / dd, 0, 1 )
        worst = max( worst, float( np.min( np.linalg.norm( b0 + t[ :, None ] * d - p, axis = 1 ) ) ) )
    return worst


def test_fit_cst_keeps_the_modify_tabs_transform():
    for xform in ( { "Theta": 7.0, "Scale": 1.2, "DeltaX": 0.1, "DeltaY": 0.05, "ShiftLE": 0.3 },
                   { "Theta": -170.0, "Scale": 0.7, "DeltaX": -0.2 } ):
        vsp.VSPRenew()
        wid = vsp.AddGeom( "WING" )
        vsp.SetParmVal( wid, "CapUMinOption", "EndCap", vsp.NO_END_CAP )
        vsp.SetParmVal( wid, "CapUMaxOption", "EndCap", vsp.NO_END_CAP )
        surf = vsp.GetXSecSurf( wid, 0 )
        for k, v in xform.items():
            vsp.SetParmVal( vsp.GetXSecParm( vsp.GetXSec( surf, 1 ), k ), v )
        before = tip_section( wid )
        vsp.FitAfCST( surf, 1, 7 )
        after = tip_section( wid )
        assert max( gap( before, after ), gap( after, before ) ) < 2e-4, xform
