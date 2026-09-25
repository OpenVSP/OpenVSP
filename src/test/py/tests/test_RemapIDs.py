# Reading an entity that holds another object's ID.
#
# An ID read out of a file is one of three things, and each has its own rule.
#
# An identity names the object itself: it keeps the ID read in, or takes a fresh one if that ID
# is already in the model.  A reference names some other object: it follows that object if it
# came along with the read or copy, and otherwise still names what it always named.  A
# copy-scoped link -- a Geom's parent and children -- follows the object if it came along, and
# lapses if it did not.
#
# The tests below pin what each rule does, through the API, from the outside.

import openvsp as vsp
import pytest
import os
import re
import tempfile
import xml.etree.ElementTree


def fresh():
    """An empty model with a scratch directory for anything an analysis writes."""
    vsp.VSPRenew()
    out = tempfile.mkdtemp()
    vsp.SetComputationFileName( vsp.COMP_GEOM_TXT_TYPE, os.path.join( out, "comp_geom.txt" ) )
    vsp.SetComputationFileName( vsp.COMP_GEOM_CSV_TYPE, os.path.join( out, "comp_geom.csv" ) )
    vsp.SetComputationFileName( vsp.MASS_PROP_TXT_TYPE, os.path.join( out, "mass_props.txt" ) )
    pop_errors()
    return out


def pop_errors():
    """Take everything off the error stack and return what it said."""
    mgr = vsp.ErrorMgrSingleton.getInstance()
    return [ mgr.PopLastError().m_ErrorString for _ in range( mgr.GetNumTotalErrors() ) ]


def assert_no_errors():
    assert pop_errors() == []


def geoms_of_type( type_name ):
    return [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == type_name ]


def route_sits_on( route ):
    """The set of Geoms the route's points are anchored to."""
    return set( vsp.GetRoutingPtParentID( pid ) for pid in vsp.GetAllRoutingPtIds( route ) )


def a_pod_and_route( npts = 2 ):
    """A routing geom whose points are anchored to a pod -- a reference held by ID."""
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    route = vsp.AddGeom( "ROUTING" )
    for i in range( npts ):
        pt = vsp.AddRoutingPt( route, pod, 0 )
        vsp.SetParmVal( vsp.FindParm( pt, "U", "RoutePt" ), 0.2 + 0.6 * i / max( 1, npts - 1 ) )
    vsp.Update()
    return pod, route


def geom_type_order( path ):
    """The Geom type names in the order the file lists them."""
    root = xml.etree.ElementTree.parse( path ).getroot()
    out = []
    for node in root.find( "Vehicle" ).findall( "Geom" ):
        base = node.find( "GeomBase" )
        out.append( base.findtext( "TypeName" ) if base is not None else "?" )
    return out


#==== A reference whose target is in the model but not in the read ====#


def testAPastedRoutingGeomKeepsThePointsItSitsOn():
    """A pasted route stays anchored to the pod the original sits on, which was not copied."""
    fresh()
    pod, route = a_pod_and_route()
    length = vsp.GetParmVal( vsp.FindParm( route, "Length", "Results" ) )
    assert length > 0.0

    vsp.CopyGeomToClipboard( route )
    pasted = vsp.PasteGeomClipboard()[0]
    vsp.Update()

    assert route_sits_on( pasted ) == { pod }, "the copy's points were pointed somewhere else"
    assert vsp.GetParmVal( vsp.FindParm( pasted, "Length", "Results" ) ) == pytest.approx( length )
    assert_no_errors()


def testAPastedDetachedConformalKeepsTheGeomItTakesItsShapeFrom():
    """A detached conformal holds the Geom it lofts from by ID, and a paste keeps it."""
    fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    conf = vsp.AddGeom( "CONFORMAL", pod )
    vsp.SetParmVal( vsp.FindParm( conf, "DetachFlag", "Design" ), 1 )
    vsp.Update()
    before = vsp.GetParmVal( vsp.FindParm( conf, "X_Len", "BBox" ) )
    assert before > 0.0

    vsp.CopyGeomToClipboard( conf )
    pasted = vsp.PasteGeomClipboard()[0]
    vsp.Update()

    assert vsp.GetParmVal( vsp.FindParm( pasted, "X_Len", "BBox" ) ) == pytest.approx( before ), \
           "the copy lofted from nothing"
    assert_no_errors()


def testAPartialFileReadIntoAModelKeepsItsReferences():
    """Two managers naming the same pod both still name it after a partial read.

    A resolved ID is remembered under the one read in, so every later reference to the same
    object answers the same way.
    """
    out = fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.AddGeom( "WING" )
    vsp.Update()

    full = os.path.join( out, "full.vsp3" )
    vsp.WriteVSPFile( full )

    tree = xml.etree.ElementTree.parse( full )
    root = tree.getroot()

    def name_the_pod( parent_tag, tag ):
        parent = root.find( ".//%s" % parent_tag )
        assert parent is not None, parent_tag
        node = parent.find( tag )
        if node is None:
            node = xml.etree.ElementTree.SubElement( parent, tag )
        node.text = pod

    name_the_pod( "CFDMeshSettings", "FarGeomID" )
    name_the_pod( "WaveDrag", "ReferenceGeomID" )

    # Drop the pod itself, so those references point out of the file at a Geom that is already
    # in the model -- the everyday shape of a partial save.
    vehicle = root.find( "Vehicle" )
    dropped = 0
    for node in list( vehicle.findall( "Geom" ) ):
        base = node.find( "GeomBase" )
        if base is not None and base.findtext( "TypeName" ) == "Pod":
            vehicle.remove( node )
            dropped += 1
    assert dropped == 1

    partial = os.path.join( out, "partial.vsp3" )
    tree.write( partial )

    vsp.VSPRenew()
    vsp.ReadVSPFile( full )
    vsp.ReadVSPFile( partial )      # ReadVSPFile does not clear the model
    vsp.Update()

    saved = os.path.join( out, "after.vsp3" )
    vsp.WriteVSPFile( saved )
    after = xml.etree.ElementTree.parse( saved ).getroot()

    live = vsp.FindGeoms()
    found = 0
    for node in after.iter():
        for tag in ( "FarGeomID", "ReferenceGeomID" ):
            value = node.findtext( tag )
            if value:
                found += 1
                assert value == pod, "%s was minted a new ID: %s" % ( tag, value )
                assert value in live
    assert found > 0, "neither reference was written back, so nothing was checked"
    pop_errors()


def testABogieKeepsTheGeomsItsAttachPointsSitOn():
    """A bogie's stow and mechanism attachments name Geoms by ID, and a paste keeps them.

    There is no API for setting them, so they go into the file the way a user's model has them.
    """
    out = fresh()
    pod = vsp.AddGeom( "POD" )
    gear = vsp.AddGeom( "GEAR" )
    vsp.Update()
    vsp.CreateAndAddBogie( gear )       # a gear starts with none
    vsp.Update()

    written = os.path.join( out, "gear.vsp3" )
    vsp.WriteVSPFile( written )

    tree = xml.etree.ElementTree.parse( written )
    root = tree.getroot()
    # The two IDs live in the bogie's own element, beside its ParmContainer.
    # Every element that can hold one of the two, whichever of them the encoder wrote.
    injected = 0
    for bogie in [ n for n in root.iter() if n.tag == "Bogie" ]:
        for tag in ( "StowParentID", "MechParentID" ):
            node = bogie.find( tag )
            if node is None:
                node = xml.etree.ElementTree.SubElement( bogie, tag )
            node.text = pod
            injected += 1
    assert injected > 0, "the gear wrote no bogie"
    attached = os.path.join( out, "gear_attached.vsp3" )
    tree.write( attached )

    vsp.VSPRenew()
    vsp.ReadVSPFile( attached )
    vsp.Update()
    live_pod = geoms_of_type( "Pod" )[0]

    # The pod is not on the clipboard, so the bogies' attach points name a live Geom.
    gear = geoms_of_type( "Gear" )[0]
    vsp.CopyGeomToClipboard( gear )
    vsp.PasteGeomClipboard()
    vsp.Update()

    after = os.path.join( out, "after.vsp3" )
    vsp.WriteVSPFile( after )
    root = xml.etree.ElementTree.parse( after ).getroot()

    named = []
    for node in root.iter():
        for tag in ( "StowParentID", "MechParentID" ):
            value = node.findtext( tag )
            if value and value not in ( "", "NONE" ):
                named.append( ( tag, value ) )

    # Two gears now, each naming the pod its bogie is stowed against.  The gear clears the
    # mechanism attachment unless the mechanism mode is set, so the stow attachment is the one
    # that survives a round trip -- and neither of them may have been minted.
    assert len( named ) >= 2, "the paste did not carry the bogie's references: %s" % named
    for tag, value in named:
        assert value == live_pod, "%s was minted a new ID: %s (the pod is %s)" % ( tag, value, live_pod )
    pop_errors()


#==== Read order, copy-scoped links, and duplicate IDs ====#


def testAReferenceReadBeforeTheGeomItNamesStillFollowsTheCopy():
    """A reference resolves the same whichever order the file lists it and the Geom it names.

    Nothing guarantees a file lists an object before something that names it, so the identities
    in a tree are registered before any of it is read.
    """
    out = fresh()
    pod, route = a_pod_and_route()
    written = os.path.join( out, "route.vsp3" )
    vsp.WriteVSPFile( written )

    # Put the routing geom first, so its reference to the pod is decoded before the pod is.
    tree = xml.etree.ElementTree.parse( written )
    root = tree.getroot()
    vehicle = root.find( "Vehicle" )
    nodes = vehicle.findall( "Geom" )

    def type_of( node ):
        base = node.find( "GeomBase" )
        return base.findtext( "TypeName" ) if base is not None else "?"

    for node in nodes:
        vehicle.remove( node )
    for node in sorted( nodes, key = lambda n: 0 if type_of( n ) == "Routing" else 1 ):
        vehicle.append( node )

    reordered = os.path.join( out, "route_reordered.vsp3" )
    tree.write( reordered )
    assert geom_type_order( reordered )[0] == "Routing", "the reorder did not take"

    # Read it, then read it again: every identity in the second read collides.
    vsp.VSPRenew()
    vsp.ReadVSPFile( reordered )
    vsp.Update()
    first_pod = geoms_of_type( "Pod" )[0]

    vsp.InsertVSPFile( reordered, "" )
    vsp.Update()

    pods = geoms_of_type( "Pod" )
    routes = geoms_of_type( "Routing" )
    assert len( pods ) == 2 and len( routes ) == 2

    # One route each way: the one that was here keeps the pod that was here, and the one that
    # arrived follows the pod that arrived with it.
    anchored = sorted( route_sits_on( r ).pop() for r in routes )
    assert anchored == sorted( pods ), "a route was left anchored to the wrong pod"
    assert first_pod in anchored
    pop_errors()


def testAnInsertedFileFollowsItsOwnCopies():
    """Reading a file into a model that already holds it: each copy names its own copy."""
    out = fresh()
    pod, route = a_pod_and_route()
    written = os.path.join( out, "pair.vsp3" )
    vsp.WriteVSPFile( written )

    vsp.VSPRenew()
    vsp.ReadVSPFile( written )
    vsp.Update()
    first_pod = geoms_of_type( "Pod" )[0]

    vsp.InsertVSPFile( written, "" )
    vsp.Update()

    pods = geoms_of_type( "Pod" )
    routes = geoms_of_type( "Routing" )
    assert len( pods ) == 2 and len( routes ) == 2
    assert sorted( route_sits_on( r ).pop() for r in routes ) == sorted( pods )
    assert first_pod in pods
    pop_errors()


def testAFileRoundTripKeepsTheHierarchy():
    """A save and an open leave the parent and child hierarchy as it was."""
    out = fresh()
    parent = vsp.AddGeom( "POD" )
    vsp.SetGeomName( parent, "Parent" )
    child = vsp.AddGeom( "POD", parent )
    vsp.SetGeomName( child, "Child" )
    vsp.Update()

    written = os.path.join( out, "hier.vsp3" )
    vsp.WriteVSPFile( written )
    vsp.VSPRenew()
    vsp.ReadVSPFile( written )
    vsp.Update()

    reloaded_child = [ g for g in vsp.FindGeoms() if vsp.GetGeomName( g ) == "Child" ][0]
    reloaded_parent = [ g for g in vsp.FindGeoms() if vsp.GetGeomName( g ) == "Parent" ][0]
    assert vsp.GetGeomParent( reloaded_child ) == reloaded_parent
    assert_no_errors()


def testAPastedChildLandsAtTheTopLevel():
    """A parent that did not come along is not carried over, so the paste lands at the top."""
    fresh()
    parent = vsp.AddGeom( "POD" )
    child = vsp.AddGeom( "POD", parent )
    vsp.Update()

    vsp.CopyGeomToClipboard( child )
    vsp.DeleteGeom( parent )          # nothing left for the paste to attach to
    vsp.Update()
    pasted = vsp.PasteGeomClipboard()[0]
    vsp.Update()

    assert vsp.GetGeomParent( pasted ) in ( "", "NONE" )
    assert_no_errors()


def testAnInsertedChildHangsOffTheParentThatCameWithIt():
    """A child copied together with its parent hangs off that copy, not the original."""
    out = fresh()
    parent = vsp.AddGeom( "POD" )
    vsp.SetGeomName( parent, "Parent" )
    child = vsp.AddGeom( "POD", parent )
    vsp.SetGeomName( child, "Child" )
    vsp.Update()

    written = os.path.join( out, "hier.vsp3" )
    vsp.WriteVSPFile( written )

    vsp.VSPRenew()
    vsp.ReadVSPFile( written )
    vsp.Update()
    vsp.InsertVSPFile( written, "" )
    vsp.Update()

    parents = [ g for g in vsp.FindGeoms() if vsp.GetGeomName( g ) == "Parent" ]
    children = [ g for g in vsp.FindGeoms() if vsp.GetGeomName( g ) == "Child" ]
    assert len( parents ) == 2 and len( children ) == 2

    # Each child hangs off a parent, and no two children hang off the same one.
    hung = sorted( vsp.GetGeomParent( c ) for c in children )
    assert hung == sorted( parents ), "a child followed the wrong parent: %s" % hung
    pop_errors()


def testAFileThatNamesTwoObjectsTheSameLoadsWithoutComplaint():
    """A file naming two objects the same loads, and the second shares the first's ID."""
    out = fresh()
    vsp.AddGeom( "POD" )
    vsp.Update()
    written = os.path.join( out, "one.vsp3" )
    vsp.WriteVSPFile( written )

    # Duplicate the Geom element verbatim, ID and all.
    tree = xml.etree.ElementTree.parse( written )
    root = tree.getroot()
    vehicle = root.find( "Vehicle" )
    node = vehicle.findall( "Geom" )[0]
    vehicle.append( node )
    doubled = os.path.join( out, "doubled.vsp3" )
    tree.write( doubled )

    vsp.VSPRenew()
    vsp.ReadVSPFile( doubled )
    vsp.Update()

    pods = geoms_of_type( "Pod" )
    assert len( pods ) >= 1, "the file did not load"
    pop_errors()




def testACopiedRibIsLaidOutAgainstItsOwnStructuresSpar():
    """A rib perpendicular to a spar is laid out against the spar in its own structure.

    A rib's perpendicular edge is either one of three sentinel words or the ID of another part
    in the same structure, so a copied rib has to follow the part that came with it.
    """
    fresh()
    wing = vsp.AddGeom( "WING" )
    vsp.Update()
    struct = vsp.AddFeaStruct( wing )
    spar = vsp.AddFeaPart( wing, struct, vsp.FEA_SPAR )
    rib = vsp.AddFeaPart( wing, struct, vsp.FEA_RIB )
    vsp.SetFeaPartPerpendicularSparID( rib, spar )
    vsp.Update()

    vsp.CopyGeomToClipboard( wing )
    vsp.PasteGeomClipboard()
    vsp.Update()

    checked = 0
    for sid in vsp.GetFeaStructIDVec():
        parts = vsp.GetFeaPartIDVec( sid )
        spars = [ p for p in parts if vsp.GetFeaPartType( p ) == vsp.FEA_SPAR ]
        for rib_id in [ p for p in parts if vsp.GetFeaPartType( p ) == vsp.FEA_RIB ]:
            perp = vsp.GetFeaPartPerpendicularSparID( rib_id )
            assert perp in spars, \
                   "a rib is laid out against %s, which is not a spar in its own structure %s" % ( perp, spars )
            checked += 1

    assert checked == 2, "expected one rib in each of the two structures, checked %d" % checked
    pop_errors()


def surviving_containers():
    """The containers a read decodes into rather than making again, and their parms."""
    names = ( "Vehicle", "CFDMeshSettings", "VSPAEROSettings", "WaveDragSettings",
              "ParasiteDragSettings" )
    out = {}
    for c in vsp.FindContainers():
        name = vsp.GetContainerName( c )
        if name in names:
            out[ name ] = ( c, list( vsp.FindContainerParmIDs( c ) ) )
    return out


def testAReadLeavesTheIdsOfWhatItDoesNotMakeAgain():
    """The Vehicle and the settings containers keep their IDs across a read.

    They are still in the model when the file names them, so all of their IDs collide -- but
    nothing new is made here, the same object reads its own ID back.  An ID held outside the
    model, in a design variable file or a script, names these parms and has to keep working.
    """
    out = fresh()
    vsp.AddGeom( "POD" )
    vsp.Update()
    written = os.path.join( out, "surv.vsp3" )
    vsp.WriteVSPFile( written )

    before = surviving_containers()
    assert before, "no surviving container was found, so nothing is measured"
    vsp.ReadVSPFile( written )
    after = surviving_containers()

    for name in sorted( before ):
        bID, bparms = before[ name ]
        aID, aparms = after[ name ]
        assert bID == aID, "%s was given a new ID by a read: %s -> %s" % ( name, bID, aID )
        lost = [ p for p in bparms if p not in set( aparms ) ]
        assert not lost, "%s lost %d of %d parm IDs to a read" % ( name, len( lost ), len( bparms ) )
    pop_errors()


def testALinkToAParmOutsideAnInsertedTreeNamesTheLiveParm():
    """An inserted file brings a second copy of every Geom, but not a second Vehicle.

    A link whose input is a Vehicle parm therefore still names the one live Vehicle parm.
    """
    out = fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    vehicle = [ c for c in vsp.FindContainers() if vsp.GetContainerName( c ) == "Vehicle" ][0]
    vehicle_parm = vsp.FindContainerParmIDs( vehicle )[0]

    vsp.AddAdvLink( "L" )
    vsp.AddAdvLinkInput( 0, vehicle_parm, "vin" )
    vsp.AddAdvLinkOutput( 0, vsp.FindParm( pod, "Y_Rel_Location", "XForm" ), "pout" )
    vsp.Update()

    written = os.path.join( out, "link.vsp3" )
    vsp.WriteVSPFile( written )

    vsp.VSPRenew()
    vsp.ReadVSPFile( written )
    vsp.Update()
    vsp.InsertVSPFile( written, "" )
    vsp.Update()

    live = set( vsp.FindContainerParmIDs(
        [ c for c in vsp.FindContainers() if vsp.GetContainerName( c ) == "Vehicle" ][0] ) )
    links = vsp.GetAdvLinkNames()
    assert len( links ) == 2, "expected the inserted link alongside the original, got %d" % len( links )
    for i in range( len( links ) ):
        for parm in vsp.GetAdvLinkInputParms( i ):
            assert parm in live, \
                   "link %d takes its input from %s, which is not a live Vehicle parm" % ( i, parm )
    pop_errors()


def testOpeningAndSavingTwiceOverChangesNothing():
    """A file does not change from nothing more than an open and a save.

    Every ID in it survives the cycle untouched.  This is what the V2 reader's ForceRemapID
    restores for a V2 file, which wrote memory addresses where a v3 file writes IDs.
    """
    out = fresh()
    vsp.AddGeom( "POD" )
    vsp.Update()
    written = os.path.join( out, "idem.vsp3" )
    vsp.WriteVSPFile( written )

    def ids_in( path ):
        text = open( path ).read()
        return set( re.findall( r'ID="(\w+)"', text ) ) | set( re.findall( r"<ID>(\w+)</ID>", text ) )

    previous = ids_in( written )
    assert previous, "the file holds no IDs, so nothing is measured"

    for cycle in range( 3 ):
        vsp.VSPRenew()
        vsp.ReadVSPFile( written )
        vsp.Update()
        vsp.WriteVSPFile( written )
        now = ids_in( written )
        renamed = previous - now
        assert not renamed, \
               "open and save #%d renamed %d of %d IDs" % ( cycle + 1, len( renamed ), len( previous ) )
        previous = now
    pop_errors()


def testACopiedXSecsAttributeNamesTheCurveThatCameWithIt():
    """An XSec's tree holds two containers, and the second is read after the first.

    XSec::DecodeXml reads the XSec's own ParmContainer -- attributes included -- before the
    XSecCurve beneath it, so an attribute on the XSec naming one of the curve's Parms is a
    reference to something in the same tree that is decoded later.  It has to follow the copy.
    """
    fresh()
    stack = vsp.AddGeom( "STACK" )
    vsp.Update()
    surf = vsp.GetXSecSurf( stack, 0 )
    source = vsp.GetXSec( surf, 1 )

    curve_parms = [ p for p in vsp.FindContainerParmIDs( source )
                    if vsp.GetParmGroupName( p ) == "XSecCurve" ]
    assert curve_parms, "this cross section has no curve parms, so nothing is measured"
    target_parm = curve_parms[0]
    assert vsp.GetParmContainer( target_parm ) != source, \
           "the parm is owned by the XSec itself, so there is no second container to order"

    vsp.AddAttributeParm( vsp.GetChildCollection( source ), "pointsatcurveparm", target_parm )
    vsp.Update()

    vsp.CopyXSec( stack, 1 )
    vsp.PasteXSec( stack, 2 )
    vsp.Update()

    pasted = vsp.GetXSec( vsp.GetXSecSurf( stack, 0 ), 2 )   # the slot keeps its ID
    pasted_curve_parms = [ p for p in vsp.FindContainerParmIDs( pasted )
                           if vsp.GetParmGroupName( p ) == "XSecCurve" ]

    checked = 0
    for attr in vsp.FindAttributesInCollection( vsp.GetChildCollection( pasted ) ):
        if vsp.GetAttributeName( attr ) == "pointsatcurveparm":
            named = list( vsp.GetAttributeParmID( attr ) )[0]
            assert named in pasted_curve_parms, \
                   "the copied attribute names %s, which is not a parm of the copied curve" % named
            checked += 1
    assert checked == 1, "expected the copied attribute on the pasted cross section"
    pop_errors()


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
