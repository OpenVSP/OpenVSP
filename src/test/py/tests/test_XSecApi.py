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
