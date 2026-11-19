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


def testAParmIsUsedOnceInALink():
    """A Parm may be one variable of a link, input or output, but other links may use it too."""
    vsp.VSPRenew()
    vsp.DelAllAdvLinks()
    pod = vsp.AddGeom( "POD", "" )
    length = vsp.FindParm( pod, "Length", "Design" )
    x_pos = vsp.GetParm( pod, "X_Rel_Location", "XForm" )
    drop_errors()

    vsp.AddAdvLink( "A" )
    vsp.AddAdvLinkInput( 0, length, "len" )
    vsp.AddAdvLinkOutput( 0, x_pos, "x" )
    assert_no_errors()

    vsp.AddAdvLinkInput( 0, length, "len2" )
    assert pop_errors(), "a Parm added twice as an input raised no error"
    vsp.AddAdvLinkOutput( 0, length, "len3" )
    assert pop_errors(), "an input Parm added as an output raised no error"
    vsp.AddAdvLinkInput( 0, x_pos, "x2" )
    assert pop_errors(), "an output Parm added as an input raised no error"
    vsp.AddAdvLinkOutput( 0, x_pos, "x3" )
    assert pop_errors(), "a Parm added twice as an output raised no error"
    assert list( vsp.GetAdvLinkInputNames( 0 ) ) == [ "len" ], vsp.GetAdvLinkInputNames( 0 )
    assert list( vsp.GetAdvLinkOutputNames( 0 ) ) == [ "x" ], vsp.GetAdvLinkOutputNames( 0 )

    vsp.AddAdvLink( "B" )
    vsp.AddAdvLinkInput( 1, length, "len" )
    vsp.AddAdvLinkOutput( 1, x_pos, "x" )
    assert_no_errors()
