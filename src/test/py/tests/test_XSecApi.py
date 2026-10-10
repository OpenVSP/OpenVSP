# Cross section calls through the API.

import openvsp as vsp

from testhelp import ( pop_errors, drop_errors, assert_no_errors )


def testClearingSkinningOnAPropIsRefused():
    """ClearSkinning refuses a Prop, whose sections are not skinned."""
    vsp.VSPRenew()
    drop_errors()
    pid = vsp.AddGeom( "PROP" )
    vsp.Update()
    drop_errors()
    vsp.ClearSkinning( pid )
    assert pop_errors(), "ClearSkinning accepted a Prop"


def testClearingSkinningPastTheLastSectionIsRefused():
    vsp.VSPRenew()
    drop_errors()
    sid = vsp.AddGeom( "STACK" )
    vsp.Update()
    drop_errors()
    vsp.ClearSkinning( sid, 1 )
    assert_no_errors()
    vsp.ClearSkinning( sid, 99 )
    assert pop_errors(), "ClearSkinning accepted a section past the last"


def testAnAliasForAMissingXSecIsAnError():
    """SetXSecAlias and SetXSecCurveAlias report a missing XSec and stop."""
    vsp.VSPRenew()
    drop_errors()
    vsp.SetXSecAlias( "no such xsec", "A" )
    assert pop_errors(), "SetXSecAlias accepted a missing XSec"
    vsp.SetXSecCurveAlias( "no such xsec", "A" )
    assert pop_errors(), "SetXSecCurveAlias accepted a missing XSec"


def testADriverGroupPastTheLastSectionIsAnError():
    """SetDriverGroup reports a fuselage section index past the last."""
    vsp.VSPRenew()
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    drop_errors()
    vsp.SetDriverGroup( fid, 99, vsp.WIDTH_XSEC_DRIVER, vsp.HEIGHT_XSEC_DRIVER )
    assert pop_errors(), "SetDriverGroup accepted a section past the last"


def testCutCopyPasteInsertAndSplitCheckTheIndex():
    vsp.VSPRenew()
    wid = vsp.AddGeom( "WING" )
    fid = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    wsurf = vsp.GetXSecSurf( wid, 0 )
    n = vsp.GetNumXSec( wsurf )
    drop_errors()
    for call in ( lambda: vsp.CutXSec( fid, 99 ), lambda: vsp.CopyXSec( fid, 99 ), lambda: vsp.PasteXSec( fid, 99 ),
                  lambda: vsp.InsertXSec( fid, 99, vsp.XS_ELLIPSE ), lambda: vsp.SplitWingXSec( wid, 99 ) ):
        call()
        assert pop_errors(), "an index past the last XSec raised no error"
    # A wing's root has no section to change
    vsp.SplitWingXSec( wid, 0 )
    assert pop_errors(), "SplitWingXSec split the root"
    vsp.CutXSec( wid, 0 )
    assert pop_errors(), "CutXSec cut the root"
    assert vsp.GetNumXSec( wsurf ) == n
    # Nor can a wing give up its only section
    vsp.CutXSec( wid, 1 )
    assert pop_errors(), "CutXSec cut a wing's only section"
    assert vsp.GetNumXSec( wsurf ) == n
