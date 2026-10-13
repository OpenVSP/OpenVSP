"""A propeller with reverse rotation is the mirror image, through the plane of its rotation axis
and blade, of the same propeller turning the normal way.
"""

import math
import os
import re
import tempfile

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


def test_fold_axis_mirrors():
    for parms in ( { "FoldAngle": 30.0, "ElFoldDir": 20.0 },
                   { "FoldAngle": 30.0, "AzFoldDir": 20.0 },
                   { "FoldAngle": -40.0, "AzFoldDir": 15.0, "ElFoldDir": -25.0, "OffFoldAx": 0.1,
                     "AxFoldAx": 0.05, "Feather": 10.0, "Precone": 5.0 } ):
        assert reverse_mirror_error( parms ) < TOL, parms


def test_individual_fold_mirrors():
    parms = { "NumBlade": 3, "IndividualBladeFoldFlag": 1.0, "FoldAngle_1": 50.0,
              "AzFoldDir": 15.0, "ElFoldDir": -25.0, "OffFoldAx": 0.1 }
    for surf in range( 3 ):
        assert reverse_mirror_error( parms, surf ) < TOL, surf
    pid = new_prop( parms, False )
    assert vsp.GetParmVal( pid, "FoldAngle_1", "Design" ) == 50.0


def test_flap_hinge_mirrors():
    for parms in ( { "FlapAngle": 12.0 },
                   { "FlapAngle": -8.0, "AzFlapDir": 10.0, "ElFlapDir": 20.0, "OffFlapAx": 0.02,
                     "AxFlapAx": -0.01, "RFlapAx": 0.1, "Feather": 10.0, "Precone": 3.0, "FoldAngle": 20.0 } ):
        assert reverse_mirror_error( parms ) < TOL, parms


def test_lag_hinge_mirrors():
    for parms in ( { "LagAngle": 6.0 },
                   { "LagAngle": -9.0, "AzLagDir": 10.0, "CantLagDir": 20.0, "OffLagAx": 0.02,
                     "AxLagAx": -0.01, "RLagAx": 0.12, "FlapAngle": 5.0, "Feather": 10.0,
                     "Precone": 3.0, "FoldAngle": 20.0 } ):
        assert reverse_mirror_error( parms ) < TOL, parms


def test_current_file_unchanged():
    parms = { "FeatherOffsetXoC": 0.2, "Feather": 10.0, "FoldAngle": 30.0, "AzFoldDir": 15.0,
              "ElFoldDir": -25.0, "OffFoldAx": 0.1 }
    want = blade( new_prop( parms, True ) )
    path = os.path.join( tempfile.mkdtemp(), "current.vsp3" )
    vsp.WriteVSPFile( path )
    vsp.VSPRenew()
    vsp.ReadVSPFile( path )
    vsp.Update()
    got = blade( vsp.FindGeoms()[0] )
    assert max( math.dist( p, q ) for p, q in zip( got, want ) ) < TOL


def test_paste_keeps_geometry():
    # A pasted propeller keeps its blades where they were.
    parms = { "NumBlade": 3, "FoldAngle": 30.0, "AzFoldDir": 15.0, "ElFoldDir": -25.0, "OffFoldAx": 0.1,
              "Feather": 10.0, "FeatherOffsetXoC": 0.2 }
    pid = new_prop( parms, True )
    want = [ blade( pid, s ) for s in range( 3 ) ]
    vsp.CopyGeomToClipboard( pid )
    new = vsp.PasteGeomClipboard()[0]
    vsp.Update()
    assert vsp.GetParmVal( new, "OffFoldAx", "Design" ) == 0.1
    assert max( math.dist( p, q ) for s in range( 3 ) for p, q in zip( blade( new, s ), want[s] ) ) < TOL
