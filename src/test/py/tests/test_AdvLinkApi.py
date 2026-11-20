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


def testALinkReadWithARepeatedParmKeepsTheFirstUse( tmp_path ):
    """A file whose link uses a Parm twice reads with the later uses dropped, their names
    renamed in the code to the kept variable, and an error raised."""
    vsp.VSPRenew()
    vsp.DelAllAdvLinks()
    pod = vsp.AddGeom( "POD" )
    length = vsp.FindParm( pod, "Length", "Design" )
    x_pos = vsp.GetParm( pod, "X_Rel_Location", "XForm" )
    vsp.AddAdvLink( "A" )
    vsp.AddAdvLinkInput( 0, length, "len" )
    vsp.AddAdvLinkOutput( 0, x_pos, "x" )
    vsp.SetAdvLinkCode( 0, "x = len2*len2 + len3 * 0.0 + 1.0;" )
    fname = str( tmp_path / "repeated_parm.vsp3" )
    vsp.WriteVSPFile( fname )
    drop_errors()

    # Add the Parm again, once as an input and once as an output.
    with open( fname ) as f:
        xml = f.read()
    tag = '<VarDef Name="len" ParmID="%s"/>' % length
    assert xml.count( tag ) == 1, "the saved file does not hold the input as expected"
    xml = xml.replace( tag, tag + '<VarDef Name="len2" ParmID="%s"/>' % length )
    xml = xml.replace( '<VarDef Name="x" ParmID="%s"/>' % x_pos,
                       '<VarDef Name="x" ParmID="%s"/><VarDef Name="len3" ParmID="%s"/>' % ( x_pos, length ) )
    with open( fname, "w" ) as f:
        f.write( xml )

    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    errs = pop_errors()
    assert len( [ e for e in errs if "more than once" in e ] ) == 2, errs
    assert list( vsp.GetAdvLinkInputNames( 0 ) ) == [ "len" ]
    assert list( vsp.GetAdvLinkOutputNames( 0 ) ) == [ "x" ]
    assert vsp.GetAdvLinkCode( 0 ) == "x = len*len + len * 0.0 + 1.0;", vsp.GetAdvLinkCode( 0 )

    # The link still works.
    vsp.SetParmVal( length, 7.0 )
    vsp.Update()
    assert abs( vsp.GetParmVal( x_pos ) - 50.0 ) < 1e-9, vsp.GetParmVal( x_pos )
