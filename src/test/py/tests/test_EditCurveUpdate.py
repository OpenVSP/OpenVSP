# An edit curve changed through the API is rebuilt at once, and its Geom's surface follows at the next Update().

import openvsp as vsp
import pytest


def pnt( gid ):
    p = vsp.CompPnt01( gid, 0, 0.3, 0.27 )
    return ( p.x(), p.y(), p.z() )


def edit_fuselage():
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    vsp.ConvertXSecToEdit( fid, 2 )
    xs = vsp.GetXSec( vsp.GetXSecSurf( fid, 0 ), 2 )
    vsp.EditXSecConvertTo( xs, vsp.LINEAR )
    vsp.Update()
    return fid, xs


def testNewPointsReachTheSurfaceAtUpdate():
    """Setting an edit curve's points marks its Geom's surface to be rebuilt."""
    fid, xs = edit_fuselage()
    before = pnt( fid )

    u = [ 0.0, 0.25, 0.5, 0.75, 1.0 ]
    pts = [ vsp.vec3d( 0.5, 0, 0 ), vsp.vec3d( 0, -0.7, 0 ), vsp.vec3d( -0.5, 0, 0 ), vsp.vec3d( 0, 0.4, 0 ),
            vsp.vec3d( 0.5, 0, 0 ) ]
    vsp.SetEditXSecPnts( xs, u, pts, [ 0.0 ] * 5 )
    vsp.Update()

    assert pnt( fid ) != pytest.approx( before ), "the surface did not take the new points"


def testAConvertedCurveCanBeReparameterized():
    """A cross section just converted to an edit curve has its base curve built, so reparameterizing it works."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    vsp.ConvertXSecToEdit( fid, 1 )
    xs = vsp.GetXSec( vsp.GetXSecSurf( fid, 0 ), 1 )
    vsp.ReparameterizeEditXSec( xs )
    vsp.Update()

    assert vsp.GetNumXSec( vsp.GetXSecSurf( fid, 0 ) ) == 5
    assert len( vsp.GetEditXSecUVec( xs ) ) > 0
