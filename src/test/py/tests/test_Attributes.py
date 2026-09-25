# Attributes carry an ID of their own, and an attach naming whatever holds them.
#
# Both have to survive a file.  An attribute with no ID is invisible to every ID rule, and an
# attach is only meaningful while it names a collection that exists.

import openvsp as vsp
import os
import re
import tempfile
import xml.etree.ElementTree


def fresh():
    vsp.VSPRenew()
    out = tempfile.mkdtemp()
    pop_errors()
    return out


def pop_errors():
    mgr = vsp.ErrorMgrSingleton.getInstance()
    return [ mgr.PopLastError().m_ErrorString for _ in range( mgr.GetNumTotalErrors() ) ]


def testAGroupAndAParmAttributeSurviveAFile():
    """AddAttributeGroup and AddAttributeParm give the attribute an ID of its own.

    An attribute with no ID is invisible to every ID rule, so two of them on one object would
    share an identity and only one would survive a save and an open.
    """
    out = fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    coll = vsp.GetChildCollection( pod )

    group_id = vsp.AddAttributeGroup( coll, "grp" )
    parm_id = vsp.AddAttributeParm( coll, "pointsatparm",
                                    vsp.FindParm( pod, "Y_Rel_Location", "XForm" ) )
    assert group_id, "AddAttributeGroup handed back no ID"
    assert parm_id, "AddAttributeParm handed back no ID"
    vsp.Update()

    written = os.path.join( out, "attr.vsp3" )
    vsp.WriteVSPFile( written )
    assert '<Attribute ID=""' not in open( written ).read(), "an attribute was written with no ID"

    before = sorted( vsp.FindAttributeNamesInCollection( coll ) )
    assert before == [ "grp", "pointsatparm" ]

    vsp.VSPRenew()
    vsp.ReadVSPFile( written )
    vsp.Update()

    coll_after = vsp.GetChildCollection( vsp.FindGeoms()[0] )
    after = sorted( vsp.FindAttributeNamesInCollection( coll_after ) )
    assert after == before, "the round trip lost an attribute: %s -> %s" % ( before, after )
    for name in before:
        assert vsp.FindAttributesByName( name ), "%s cannot be found by name any more" % name
    pop_errors()


def attach_ids( path ):
    """Every attribute's attach, and every collection ID defined, in a written file."""
    root = xml.etree.ElementTree.parse( path ).getroot()
    defined = set()
    attaches = []
    for coll in root.iter( "AttributeCollection" ):
        defined.add( coll.get( "ID" ) )
    for attr in root.iter( "Attribute" ):
        defined.add( attr.get( "ID" ) )
        attaches.append( ( attr.get( "Name" ), attr.get( "AttachID" ) ) )
    return defined, attaches


def testAnAttributeStillNamesItsCollectionAfterTheIdsHaveMoved():
    """No attribute may name a collection the file does not define.

    An attribute's attach names the collection holding it, and a collection taking a new ID
    tells the attributes inside, so the two ends stay together however the IDs move.
    """
    out = fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.AddAttributeString( vsp.GetChildCollection( pod ), "note", "hello" )
    vsp.Update()

    written = os.path.join( out, "attach.vsp3" )
    vsp.WriteVSPFile( written )

    vsp.VSPRenew()
    vsp.ReadVSPFile( written )
    vsp.Update()
    vsp.InsertVSPFile( written, "" )      # every identity collides, so every one moves
    vsp.Update()

    again = os.path.join( out, "attach2.vsp3" )
    vsp.WriteVSPFile( again )

    defined, attaches = attach_ids( again )
    assert attaches, "the file carries no attributes, so nothing is measured"
    dangling = [ ( n, a ) for n, a in attaches if a and a not in defined and a != "NONE" ]
    assert not dangling, "an attribute names something the file does not define: %s" % dangling
    pop_errors()


def testTwoAttributesWithNoIdInTheFileBothSurvive():
    """Two attributes whose ID property is missing from the file both survive.

    No version of OpenVSP writes an <Attribute> without an ID, so this is a file from somewhere
    else.  An identity falls back to the ID the attribute was constructed with, which is unique
    to it, so the two do not collide.
    """
    out = fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    coll = vsp.GetChildCollection( pod )
    vsp.AddAttributeString( coll, "alpha", "one" )
    vsp.AddAttributeString( coll, "beta", "two" )
    vsp.Update()

    written = os.path.join( out, "ids.vsp3" )
    vsp.WriteVSPFile( written )
    text = open( written ).read()

    stripped = re.sub( r'(<Attribute )ID="[A-Za-z0-9_]*" ( ?[^>]*Name="(?:alpha|beta)")',
                       r'\1\2', text )
    assert stripped != text, "neither attribute was written with an ID, so nothing is measured"
    no_id = os.path.join( out, "no_id.vsp3" )
    open( no_id, "w" ).write( stripped )

    vsp.VSPRenew()
    vsp.ReadVSPFile( no_id )
    vsp.Update()

    coll_after = vsp.GetChildCollection( vsp.FindGeoms()[0] )
    names = sorted( vsp.FindAttributeNamesInCollection( coll_after ) )
    assert names == [ "alpha", "beta" ], "an attribute with no ID was lost: %s" % names
    pop_errors()


def testEveryCollectionNamesWhatHoldsItFromTheStart():
    """A collection names the object holding it from the moment the object is made.

    A cross section surface, a skinning spine, a structure, a mesh source and a texture set
    their collection's attach only when their ID changed or a file was read, so until then an
    attribute on one named nothing and was summarized as unattached.
    """
    fresh()
    fuse = vsp.AddGeom( "FUSELAGE" )
    k = vsp.AddSkinSpine( fuse, 0.125 )
    fea = vsp.AddFeaStruct( fuse )
    vsp.AddCFDSource( vsp.POINT_SOURCE, fuse, 0, 0.5, 1.0, 0.5, 0.5 )
    tex = vsp.AttachGeomTexture( fuse, "hull.png" )
    vsp.Update()

    xss = vsp.GetXSecSurf( fuse, 0 )
    held = {
        "XSecSurf": xss,
        "Spine": vsp.GetSkinSpineID( vsp.GetXSec( xss, 1 ), k ),
        "FeaStructure": vsp.GetFeaStructID( fuse, fea ),
        "Source": vsp.GetCFDSourceID( fuse, 0 ),
        "Texture": tex,
    }
    for name, obj in held.items():
        assert obj, name
        assert vsp.GetObjectParent( vsp.GetChildCollection( obj ) ) == obj, name
    pop_errors()


def testAnAttributeOnAParmFollowsThatParmsID():
    """Two Parms that swap identities swap what is attached to them, because an attribute
    names its Parm by ID.

    A cross section keeps its Parm IDs when its shape changes, so that links, design variables
    and advanced links go on naming the same thing.  An attribute is one more thing naming it,
    and so is its collection's ID.  The Parm object that carried them is destroyed with the old
    cross section, so unless they move to whichever Parm holds the ID afterwards they are lost
    with nothing said.
    """
    fresh()
    fuse = vsp.AddGeom( "FUSELAGE" )
    xsurf = vsp.GetXSecSurf( fuse, 0 )
    vsp.ChangeXSecShape( xsurf, 1, vsp.XS_SUPER_ELLIPSE )
    vsp.Update()

    width = vsp.GetXSecParm( vsp.GetXSec( xsurf, 1 ), "Super_Width" )
    assert width, "the reshaped cross section has no width Parm"
    coll = vsp.GetChildCollection( width )
    attr = vsp.AddAttributeString( coll, "WidthNote", "mine" )
    vsp.Update()
    assert list( vsp.FindAttributesInCollection( coll ) ) == [ attr ]

    vsp.ChangeXSecShape( xsurf, 1, vsp.XS_ELLIPSE )
    vsp.Update()

    after = vsp.GetXSecParm( vsp.GetXSec( xsurf, 1 ), "Ellipse_Width" )
    assert after == width, "the width Parm did not keep its ID, so this measures nothing"
    assert vsp.GetChildCollection( after ) == coll, "the Parm's collection took a new ID"
    assert list( vsp.FindAttributesInCollection( coll ) ) == [ attr ], \
           "the attribute stayed with the Parm object rather than following its ID"
    assert list( vsp.GetAttributeStringVal( attr ) ) == [ "mine" ]

    # A collection held across the change still takes attributes.
    assert vsp.AddAttributeString( coll, "Later", "too" )
    assert not pop_errors()


def testACopyOntoALoopsLastSectionReplacesItsAttributes():
    """A loop's last cross section is kept a copy of its first on every update.  The copy
    added the first curve's attributes to whatever the last one held, so each update left
    another copy of every one of them behind, and each save wrote them all."""
    for gtype in ( "FUSELAGE", "STACK" ):
        fresh()
        gid = vsp.AddGeom( gtype )
        # The loop policy is 1 on both types.
        vsp.SetParmVal( gid, "OrderPolicy", "Design", 1 )
        xss = vsp.GetXSecSurf( gid, 0 )
        vsp.ChangeXSecShape( xss, 0, vsp.XS_ELLIPSE )
        vsp.Update()
        width = vsp.GetXSecParm( vsp.GetXSec( xss, 0 ), "Ellipse_Width" )
        assert width
        vsp.AddAttributeString( vsp.GetChildCollection( vsp.GetParmContainer( width ) ), "CurveNote", "c" )
        vsp.AddAttributeString( vsp.GetChildCollection( width ), "WidthNote", "w" )
        vsp.Update()
        counts = [ len( vsp.FindAllAttributes() ) ]

        for i in range( 3 ):
            vsp.SetParmVal( width, 1.0 + 0.1 * i )
            vsp.Update()
            counts.append( len( vsp.FindAllAttributes() ) )

        # The last section is a copy of the first, attributes and all -- once.
        assert counts[1:] == [ counts[1] ] * 3, "%s: %s" % ( gtype, counts )
    pop_errors()


def testASectionCopiedAgainAndAgainPastesOneCopyOfItsAttributes():
    """Copying a cross section reuses the saved one when the type matches, and the copy added
    to what the saved one held -- so a paste after three copies carried three of each."""
    fresh()
    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.Update()
    xss = vsp.GetXSecSurf( fuse, 0 )
    vsp.AddAttributeString( vsp.GetChildCollection( vsp.GetXSec( xss, 1 ) ), "SectionNote", "s" )
    vsp.Update()

    for i in range( 3 ):
        vsp.CopyXSec( fuse, 1 )
    vsp.PasteXSec( fuse, 2 )
    vsp.Update()

    pasted = vsp.GetChildCollection( vsp.GetXSec( xss, 2 ) )
    assert len( vsp.FindAttributesInCollection( pasted ) ) == 1
    pop_errors()


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
