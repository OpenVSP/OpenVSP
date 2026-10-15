"""The inboard and outboard blending dihedral of a wing section."""

import openvsp as vsp


def sect( wid, i, name ):
    return vsp.GetParmVal( vsp.GetXSecParm( vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), i ), name ) )


def blended_wing( dihedral ):
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wid, "CapUMinOption", "EndCap", vsp.NO_END_CAP )
    vsp.SetParmVal( wid, "CapUMaxOption", "EndCap", vsp.NO_END_CAP )
    vsp.Update()
    vsp.SplitWingXSec( wid, 1 )
    vsp.Update()
    xs = vsp.GetXSec( vsp.GetXSecSurf( wid, 0 ), 1 )
    for side in ( "LE", "TE" ):
        vsp.SetParmVal( vsp.GetXSecParm( xs, "In%sMode" % side ), vsp.BLEND_ANGLES )
        vsp.SetParmVal( vsp.GetXSecParm( xs, "In%sDihedral" % side ), dihedral )
    vsp.Update()
    return wid


def edge_slopes( wid, w ):
    """The slope dz/dy of an edge just inboard and just outboard of the joint halfway along u."""
    h = 1e-4
    pm = vsp.CompPnt01( wid, 0, 0.5 - h, w )
    p0 = vsp.CompPnt01( wid, 0, 0.5, w )
    pp = vsp.CompPnt01( wid, 0, 0.5 + h, w )
    return ( p0.z() - pm.z() ) / ( p0.y() - pm.y() ), ( pp.z() - p0.z() ) / ( pp.y() - p0.y() )


def test_positive_inboard_blending_dihedral_rises_outboard():
    wid = blended_wing( 10.0 )
    # w 0.5 is the leading edge, w 0 the trailing edge.
    for w in ( 0.5, 0.0 ):
        assert edge_slopes( wid, w )[ 0 ] > 0.1, w
