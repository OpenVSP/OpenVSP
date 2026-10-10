# Advanced links through the API.

import openvsp as vsp

from testhelp import ( pop_errors, drop_errors, assert_no_errors )


def testALinkCannotBeRenamedToAnotherLinksName():
    """SetAdvLinkName refuses a name another link has, and accepts the link's own."""
    vsp.VSPRenew()
    vsp.DelAllAdvLinks()
    drop_errors()
    vsp.AddAdvLink( "A" )
    vsp.AddAdvLink( "B" )
    vsp.SetAdvLinkName( 1, "A" )
    assert pop_errors(), "renaming a link to another link's name raised no error"
    assert list( vsp.GetAdvLinkNames() ) == [ "A", "B" ], vsp.GetAdvLinkNames()

    vsp.SetAdvLinkName( 1, "C" )
    assert_no_errors()
    vsp.SetAdvLinkName( 1, "C" )
    assert_no_errors()
    assert list( vsp.GetAdvLinkNames() ) == [ "A", "C" ], vsp.GetAdvLinkNames()


def testDeletingALinkPastTheEndIsAnError():
    vsp.VSPRenew()
    vsp.DelAllAdvLinks()
    drop_errors()
    vsp.AddAdvLink( "A" )
    vsp.DelAdvLink( 5 )
    assert pop_errors(), "deleting a link past the end raised no error"
    assert list( vsp.GetAdvLinkNames() ) == [ "A" ]
