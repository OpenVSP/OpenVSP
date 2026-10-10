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
