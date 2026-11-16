# Every Parm is listed under the container at the top of its container's parents: a component's cross
# sections and subsurfaces under the component.  A Custom component's cross sections are the exception: they
# are not listed with it.

import openvsp as vsp
import pytest


def section_parms( gid ):
    xs = vsp.GetXSec( vsp.GetXSecSurf( gid, 0 ), 0 )
    return vsp.GetXSecParmIDs( xs )


@pytest.mark.parametrize( "geom_type", [ "FUSELAGE", "STACK", "WING" ] )
def testASectionsParmsAreListedUnderItsComponent( geom_type ):
    vsp.VSPRenew()
    gid = vsp.AddGeom( geom_type )
    vsp.Update()
    listed = set( vsp.FindContainerParmIDs( gid ) )
    parms = section_parms( gid )
    assert parms
    assert set( parms ) <= listed


def testACustomComponentsSectionsAreNotListedWithIt():
    vsp.VSPRenew()
    gid = vsp.AddGeom( "Cone" )
    vsp.Update()
    listed = vsp.FindContainerParmIDs( gid )
    assert set( vsp.GetGeomParmIDs( gid ) ) == set( listed )
    for pid in listed:
        assert vsp.GetParmContainer( pid ) == gid or vsp.GetContainerName( vsp.GetParmContainer( pid ) ) == "Default"


def testLinkingAllOfACustomComponentLinksOnlyItsOwn():
    vsp.VSPRenew()
    a = vsp.AddGeom( "Cone" )
    b = vsp.AddGeom( "Cone" )
    vsp.Update()
    pa = vsp.FindContainerParmIDs( a )[0]
    pb = vsp.FindContainerParmIDs( b )[0]
    assert vsp.LinkAllComp( pa, pb )
    assert vsp.GetNumParmLinks() == len( vsp.FindContainerParmIDs( a ) )
