# A Geom that depends on another one without hanging off it.
#
# A step child is how that is recorded: the Geom being depended on keeps a list of the Geoms
# that depend on it, and updates each of them when it changes.  A detached conformal, a routing
# point sitting on a surface and a landing gear's stow point are all step children.
#
# The list is saved with the model and re-added by whoever depends on the Geom, so adding is
# idempotent; and the registration is rebuilt when a hash of the Geoms depended on, together
# with the depending Geom's own ID, says it has gone stale.

import openvsp as vsp
import pytest
import os
import re
import tempfile


def fresh():
    vsp.VSPRenew()
    out = tempfile.mkdtemp()
    vsp.SetComputationFileName( vsp.COMP_GEOM_TXT_TYPE, os.path.join( out, "comp_geom.txt" ) )
    pop_errors()
    return out


def pop_errors():
    mgr = vsp.ErrorMgrSingleton.getInstance()
    return [ mgr.PopLastError().m_ErrorString for _ in range( mgr.GetNumTotalErrors() ) ]


def geoms_of_type( type_name ):
    return [ g for g in vsp.FindGeoms() if vsp.GetGeomTypeName( g ) == type_name ]


def step_child_lists( path ):
    """Every step child list in the file, as lists of IDs."""
    text = open( path ).read()
    out = []
    for listing in re.findall( r"<Step_Child_List>(.*?)</Step_Child_List>", text, re.S ):
        out.append( re.findall( r"<ID>(\w+)</ID>", listing ) )
    return [ ids for ids in out if ids ]


def a_pod_and_route():
    """A routing geom anchored to a pod, which makes the route a step child of the pod."""
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    route = vsp.AddGeom( "ROUTING" )
    for u in ( 0.25, 0.75 ):
        pt = vsp.AddRoutingPt( route, pod, 0 )
        vsp.SetParmVal( vsp.FindParm( pt, "U", "RoutePt" ), u )
    vsp.Update()
    return pod, route


def route_x( route ):
    return vsp.GetParmVal( vsp.FindParm( route, "X_Min", "BBox" ) )


def testAStepChildListDoesNotGrowOnEverySaveAndOpen():
    """The list is saved and re-added on every update, so a Geom is listed once."""
    out = fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.AddGeom( "CONFORMAL", pod )
    a_pod_and_route()
    vsp.Update()

    written = os.path.join( out, "step.vsp3" )
    sizes = []
    for cycle in range( 5 ):
        vsp.WriteVSPFile( written )
        vsp.VSPRenew()
        vsp.ReadVSPFile( written )
        vsp.Update()
        sizes.append( max( len( ids ) for ids in step_child_lists( written ) ) )

    assert sizes[0] > 0, "nothing in this model holds a step child, so nothing is measured"
    assert sizes == [ sizes[0] ] * len( sizes ), "the list grew: %s" % sizes
    pop_errors()


def testAStepChildListThatIsAlreadyTooLongComesBackPruned():
    """A list that already holds duplicates comes back with one entry per Geom."""
    out = fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    vsp.AddGeom( "CONFORMAL", pod )
    vsp.Update()

    written = os.path.join( out, "step.vsp3" )
    vsp.WriteVSPFile( written )
    text = open( written ).read()

    def triple( match ):
        return "<Step_Child_List>" + match.group( 1 ) * 3 + "</Step_Child_List>"

    bloated = re.sub( r"<Step_Child_List>(.*?)</Step_Child_List>", triple, text, flags = re.S )
    assert bloated != text, "this model wrote no step children, so nothing is measured"
    bad = os.path.join( out, "step_bloated.vsp3" )
    open( bad, "w" ).write( bloated )

    vsp.VSPRenew()
    vsp.ReadVSPFile( bad )
    vsp.Update()
    vsp.WriteVSPFile( written )

    for ids in step_child_lists( written ):
        assert len( ids ) == len( set( ids ) ), "duplicates survived the read: %s" % ids
    pop_errors()


def testOnePodListsARouteOnceHoweverManyPointsSitOnIt():
    """Several points can be anchored to the same Geom, and it depends on the route once."""
    out = fresh()
    pod = vsp.AddGeom( "POD" )
    vsp.Update()
    route = vsp.AddGeom( "ROUTING" )
    for u in ( 0.1, 0.3, 0.5, 0.7, 0.9 ):
        pt = vsp.AddRoutingPt( route, pod, 0 )
        vsp.SetParmVal( vsp.FindParm( pt, "U", "RoutePt" ), u )
    vsp.Update()

    written = os.path.join( out, "five.vsp3" )
    vsp.WriteVSPFile( written )

    listed = step_child_lists( written )
    assert listed, "the pod wrote no step child"
    for ids in listed:
        assert ids.count( route ) <= 1, "the route is listed %d times" % ids.count( route )
    pop_errors()


def testAStepChildStillFollowsItsParentAfterTheIdsHaveMoved():
    """Reading a file into a model that already holds it gives every copy a new ID.

    The registration is this Geom's ID sitting in its parents' lists, so this checks the
    remapped links land on the right pod.  Moving each pod is what says the route it carries is
    really registered against it.
    """
    out = fresh()
    pod, route = a_pod_and_route()
    written = os.path.join( out, "route.vsp3" )
    vsp.WriteVSPFile( written )

    vsp.VSPRenew()
    vsp.ReadVSPFile( written )
    vsp.Update()
    vsp.InsertVSPFile( written, "" )      # every identity collides, so every one is moved
    vsp.Update()

    pods = geoms_of_type( "Pod" )
    routes = geoms_of_type( "Routing" )
    assert len( pods ) == 2 and len( routes ) == 2

    # Which route sits on which pod, by what the points say.
    anchored = {}
    for r in routes:
        parents = set( vsp.GetRoutingPtParentID( pid ) for pid in vsp.GetAllRoutingPtIds( r ) )
        assert len( parents ) == 1, "a route's points are split across %s" % parents
        anchored[ r ] = parents.pop()
    assert sorted( anchored.values() ) == sorted( pods )

    # Move each pod in turn: the route anchored to it follows, and the other one does not.
    for i, ( r, p ) in enumerate( anchored.items() ):
        before = { x: route_x( x ) for x in routes }
        shift = 7.0 + i
        vsp.SetParmVal( vsp.FindParm( p, "X_Rel_Location", "XForm" ), shift )
        vsp.Update()

        assert route_x( r ) == pytest.approx( before[r] + shift ), \
               "the route did not follow the pod it sits on"
        for other in routes:
            if other is not r:
                assert route_x( other ) == pytest.approx( before[other] ), \
                       "a route followed a pod it does not sit on"
    pop_errors()


def testAGeomOnTheClipboardIsNotListedAsAStepChild():
    """A Geom on the clipboard is not written as a step child of anything in the model.

    Copying a route puts a copy on the clipboard, and its points name the same pod the
    original's do, so the copy is registered with that live pod.  The clipboard is not saved,
    and the copy joins the model only when it is pasted.
    """
    out = fresh()
    pod, route = a_pod_and_route()
    vsp.Update()

    written = os.path.join( out, "clip.vsp3" )
    vsp.WriteVSPFile( written )
    before = step_child_lists( written )
    assert before, "the pod wrote no step child, so nothing is measured"

    vsp.CopyGeomToClipboard( route )
    vsp.Update()
    vsp.WriteVSPFile( written )

    live = set( vsp.FindGeoms() )
    for ids in step_child_lists( written ):
        for i in ids:
            assert i in live, "a step child names %s, which is not a Geom in the model" % i
    assert step_child_lists( written ) == before, \
           "copying to the clipboard changed what the pod depends on: %s -> %s" % (
               before, step_child_lists( written ) )

    # Pasting makes it part of the model, and then the pod really does depend on it.
    vsp.PasteGeomClipboard()
    vsp.Update()
    vsp.WriteVSPFile( written )
    live = set( vsp.FindGeoms() )
    listed = step_child_lists( written )
    assert any( len( ids ) == 2 for ids in listed ), \
           "the pasted route was not registered with the pod: %s" % listed
    for ids in listed:
        for i in ids:
            assert i in live, "a step child names %s, which is not a Geom in the model" % i
    pop_errors()


if __name__ == "__main__":
    for name, fn in sorted( list( globals().items() ) ):
        if name.startswith( "test" ) and callable( fn ):
            print( name )
            fn()
