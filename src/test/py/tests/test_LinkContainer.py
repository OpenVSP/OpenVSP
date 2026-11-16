# A Parm is filed under the Geom, or other linkable container, that holds it, whether or not anything has asked for
# the linkable data since the Parm was made.  Linking all of a component's Parms goes by that container.

import openvsp as vsp
import pytest


def two_fuselages():
    vsp.VSPRenew()
    fa = vsp.AddGeom( "FUSELAGE" )
    fb = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    xa = vsp.GetXSec( vsp.GetXSecSurf( fa, 0 ), 1 )
    xb = vsp.GetXSec( vsp.GetXSecSurf( fb, 0 ), 1 )
    return fa, fb, vsp.GetXSecParm( xa, "Ellipse_Width" ), vsp.GetXSecParm( xb, "Ellipse_Width" )


def link_all_comp( wa, wb ):
    assert vsp.LinkAllComp( wa, wb )
    return vsp.GetNumParmLinks()


@pytest.mark.parametrize( "change_shape", [ False, True ] )
def testLinkAllCompLinksTheWholeGeom( change_shape ):
    """Linking all of a component from a cross section's Parm links every Parm of the Geom, as it does once the
    linkable data has been asked for -- not only the Parms of the cross section's curve."""
    fa, fb, wa, wb = two_fuselages()
    if change_shape:
        vsp.ChangeXSecShape( vsp.GetXSecSurf( fa, 0 ), 1, vsp.XS_SUPER_ELLIPSE )
        vsp.ChangeXSecShape( vsp.GetXSecSurf( fb, 0 ), 1, vsp.XS_SUPER_ELLIPSE )
    direct = link_all_comp( wa, wb )

    fa, fb, wa, wb = two_fuselages()
    if change_shape:
        vsp.ChangeXSecShape( vsp.GetXSecSurf( fa, 0 ), 1, vsp.XS_SUPER_ELLIPSE )
        vsp.ChangeXSecShape( vsp.GetXSecSurf( fb, 0 ), 1, vsp.XS_SUPER_ELLIPSE )
    vsp.FindContainers()
    rebuilt = link_all_comp( wa, wb )

    assert direct == rebuilt
    assert direct > 500, "only the curve's Parms were linked"


@pytest.mark.parametrize( "ss_type, group", [ ( vsp.SS_ELLIPSE, "SS_Ellipse" ), ( vsp.SS_XSEC_CURVE, "SS_XsecCurve" ),
                                               ( vsp.SS_INTERSECT, "SS_XsecCurve" ) ] )
def testLinkAllCompFromASubsurfaceLinksTheWholeGeom( ss_type, group ):
    """Linking all of a component from a subsurface's Parm links every Parm of the Geom the subsurface is on."""
    fa, fb, wa, wb = two_fuselages()
    sa = vsp.AddSubSurf( fa, ss_type )
    sb = vsp.AddSubSurf( fb, ss_type )
    vsp.Update()
    whole = link_all_comp( wa, wb )

    fa, fb, wa, wb = two_fuselages()
    sa = vsp.AddSubSurf( fa, ss_type )
    sb = vsp.AddSubSurf( fb, ss_type )
    vsp.Update()
    from_sub = link_all_comp( vsp.GetParm( sa, "Center_U", group ), vsp.GetParm( sb, "Center_U", group ) )

    assert from_sub == whole
