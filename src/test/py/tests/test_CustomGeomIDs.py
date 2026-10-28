# The IDs of a Custom Geom's Parms through a save, a read, an insert and a paste.
#
# A Custom Geom's script makes its Parms afresh each time the Geom is read, and each takes the ID
# it was saved with, or a new one if that ID is already in the model.  CustomScripts/IDPart.vsppart
# is the component; its Parms are in two groups of the script's own.

import os
import re
import tempfile

import openvsp as vsp
import pytest

from testhelp import drop_errors, assert_no_errors

VALUES = { ( "Width", "Design" ): 1.7, ( "Height", "Design" ): 0.6, ( "Length", "Design" ): 3.0,
           ( "Taper", "Shaping" ): 0.7, ( "Rise", "Shaping" ): 0.4 }


def part():
    """A model holding one IDPart with each of VALUES set, and a scratch directory."""
    vsp.VSPRenew()
    drop_errors()
    gid = vsp.AddGeom( "IDPart" )
    assert gid, "IDPart.vsppart did not load"
    for ( name, group ), val in VALUES.items():
        vsp.SetParmVal( vsp.FindParm( gid, name, group ), val )
    vsp.Update()
    return gid, tempfile.mkdtemp()


def parts():
    return [ g for g in vsp.FindGeoms() if vsp.GetGeomName( g ) == "IDPart" ]


def ids_in( path ):
    with open( path ) as f:
        text = f.read()
    return re.findall( r' ID="(\w+)"', text )


def surface( gid ):
    """Coordinates of points over the surface, one after another."""
    out = []
    for i in range( 7 ):
        for j in range( 13 ):
            p = vsp.CompPnt01( gid, 0, i / 6.0, j / 12.0 )
            out += [ p.x(), p.y(), p.z() ]
    return out


def assert_each_its_own( gids ):
    """Each part answers its Parms by name with Parms of its own, holding VALUES, and no two parts
    share a Parm ID."""
    seen = set()
    for gid in gids:
        own = set( vsp.GetGeomParmIDs( gid ) )
        assert not ( own & seen )
        seen |= own
        for ( name, group ), val in VALUES.items():
            pid = vsp.FindParm( gid, name, group )
            assert pid in own
            assert vsp.GetParmContainer( pid ) == gid
            assert vsp.GetParmVal( pid ) == pytest.approx( val )


def assert_independent( gids ):
    """Setting a Parm of one part changes no other."""
    for gid in gids:
        vsp.SetParmVal( vsp.FindParm( gid, "Width", "Design" ), 1.0 + 0.25 * gids.index( gid ) )
    vsp.Update()
    for gid in gids:
        assert vsp.GetParmVal( vsp.FindParm( gid, "Width", "Design" ) ) == \
               pytest.approx( 1.0 + 0.25 * gids.index( gid ) )


def testASavedFileNamesEachParmOnce():
    gid, out = part()
    fname = os.path.join( out, "part.vsp3" )
    vsp.WriteVSPFile( fname )
    ids = ids_in( fname )
    assert ids
    for name, group in VALUES:
        assert ids.count( vsp.FindParm( gid, name, group ) ) == 1
    assert len( ids ) == len( set( ids ) )
    assert_no_errors()


def testAReadKeepsEachParmsIdAndValue():
    gid, out = part()
    before = { pid: vsp.GetParmVal( pid ) for pid in vsp.GetGeomParmIDs( gid ) }
    shape = surface( gid )
    fname = os.path.join( out, "part.vsp3" )
    vsp.WriteVSPFile( fname )

    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    assert parts() == [ gid ]
    after = { pid: vsp.GetParmVal( pid ) for pid in vsp.GetGeomParmIDs( gid ) }
    assert after == pytest.approx( before )
    assert_each_its_own( [ gid ] )
    assert surface( gid ) == pytest.approx( shape, abs=1e-12 )
    assert_no_errors()


def testAReadPasteOrInsertKeepsTheScriptsTypeName():
    gid, out = part()
    fname = os.path.join( out, "part.vsp3" )
    vsp.WriteVSPFile( fname )
    vsp.CopyGeomToClipboard( gid )
    vsp.PasteGeomClipboard()
    vsp.InsertVSPFile( fname, "" )
    vsp.Update()
    assert [ vsp.GetGeomTypeName( g ) for g in parts() ] == [ "IDPart" ] * 3

    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    assert [ vsp.GetGeomTypeName( g ) for g in parts() ] == [ "IDPart" ]
    assert_no_errors()


def testAPartSavedUnderARenamedModuleReadsAsItsScriptsType():
    # A session holding two different scripts named IDPart loads the second as IDPart0, and a model saved
    # there says so.  Read where this IDPart is the one loaded, it is an IDPart, as AddGeom makes it.
    gid, out = part()
    fname = os.path.join( out, "part.vsp3" )
    vsp.WriteVSPFile( fname )
    with open( fname ) as f:
        text = f.read()
    assert text.count( "<ScriptFileModule>IDPart</ScriptFileModule>" ) == 1
    with open( fname, "w" ) as f:
        f.write( text.replace( "<ScriptFileModule>IDPart</ScriptFileModule>",
                               "<ScriptFileModule>IDPart0</ScriptFileModule>" ) )

    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    assert [ vsp.GetGeomTypeName( g ) for g in parts() ] == [ "IDPart" ]
    assert_no_errors()


def testOpeningAndSavingOverChangesNothing():
    gid, out = part()
    fname = os.path.join( out, "part.vsp3" )
    vsp.WriteVSPFile( fname )
    with open( fname ) as f:
        first = f.read()
    for cycle in range( 2 ):
        vsp.VSPRenew()
        vsp.ReadVSPFile( fname )
        vsp.Update()
        vsp.WriteVSPFile( fname )
    with open( fname ) as f:
        assert f.read() == first
    assert_no_errors()


def testAnAdvancedLinkToACustomParmFollowsItThroughARead():
    gid, out = part()
    vsp.AddAdvLink( "Wide" )
    vsp.AddAdvLinkInput( 0, vsp.FindParm( gid, "Width", "Design" ), "w" )
    vsp.AddAdvLinkOutput( 0, vsp.FindParm( gid, "Height", "Design" ), "h" )
    vsp.SetAdvLinkCode( 0, "h = 0.5 * w;" )
    vsp.BuildAdvLinkScript( 0 )
    vsp.Update()
    fname = os.path.join( out, "part.vsp3" )
    vsp.WriteVSPFile( fname )

    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.SetParmVal( vsp.FindParm( gid, "Width", "Design" ), 3.0 )
    vsp.Update()
    assert vsp.GetParmVal( vsp.FindParm( gid, "Height", "Design" ) ) == pytest.approx( 1.5 )
    assert_no_errors()


@pytest.mark.parametrize( "times", [ 1, 3 ] )
def testEachInsertedPartHasParmsOfItsOwn( times ):
    gid, out = part()
    shape = surface( gid )
    fname = os.path.join( out, "part.vsp3" )
    vsp.WriteVSPFile( fname )
    for i in range( times ):
        vsp.InsertVSPFile( fname, "" )
    vsp.Update()

    gids = parts()
    assert len( gids ) == times + 1
    assert gid in gids
    assert_each_its_own( gids )
    for g in gids:
        assert surface( g ) == pytest.approx( shape, abs=1e-12 )
    assert_independent( gids )
    assert_no_errors()


@pytest.mark.parametrize( "times", [ 1, 3 ] )
def testEachPastedPartHasParmsOfItsOwn( times ):
    gid, out = part()
    shape = surface( gid )
    vsp.CopyGeomToClipboard( gid )
    for i in range( times ):
        vsp.PasteGeomClipboard()
    vsp.Update()

    gids = parts()
    assert len( gids ) == times + 1
    assert_each_its_own( gids )
    for g in gids:
        assert surface( g ) == pytest.approx( shape, abs=1e-12 )
    assert_independent( gids )
    assert_no_errors()


def testAPastedPartSavesAndReadsBack():
    """Pasted twice, then saved and read back: three parts, each still its own."""
    gid, out = part()
    vsp.CopyGeomToClipboard( gid )
    vsp.PasteGeomClipboard()
    vsp.PasteGeomClipboard()
    vsp.Update()
    gids = sorted( parts() )
    before = { pid: vsp.GetParmVal( pid ) for g in gids for pid in vsp.GetGeomParmIDs( g ) }
    fname = os.path.join( out, "parts.vsp3" )
    vsp.WriteVSPFile( fname )

    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    assert sorted( parts() ) == gids
    after = { pid: vsp.GetParmVal( pid ) for g in gids for pid in vsp.GetGeomParmIDs( g ) }
    assert after == pytest.approx( before )
    assert_each_its_own( gids )
    assert_no_errors()
