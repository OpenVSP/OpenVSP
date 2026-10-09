# Reparenting a Geom through the API.
#
# SetGeomParent once let a Geom become the child of its own child, or of its own Clone,
# and the next Update recursed around the loop until the stack ran out.
#
# A Geom also depends on what its routing points, bogies and detached conformal
# surface lie on.

import openvsp as vsp
import os
import re
import tempfile

from testhelp import ( pop_errors, drop_errors, assert_no_errors )


def testAGeomCannotBecomeTheChildOfItsDescendant():
    vsp.VSPRenew()
    drop_errors()
    a = vsp.AddGeom( "POD" )
    b = vsp.AddGeom( "POD", a )
    c = vsp.AddGeom( "POD", b )
    vsp.Update()

    for parent in ( c, b, a ):
        vsp.SetGeomParent( a, parent )
        assert pop_errors(), "making a a child of %s raised no error" % parent
        assert vsp.GetGeomParent( a ) != parent

    vsp.SetParmVal( a, "X_Rel_Location", "XForm", 2.0 )
    vsp.Update()
    assert vsp.GetGeomParent( b ) == a
    assert vsp.GetGeomParent( c ) == b
    assert_no_errors()


def testAGeomCannotBecomeTheChildOfItsClone():
    vsp.VSPRenew()
    drop_errors()
    a = vsp.AddGeom( "AUXILIARY" )
    vsp.Update()
    c = vsp.CloneGeomVec( [ a ] )[0]
    cc = vsp.CloneGeomVec( [ c ] )[0]
    k = vsp.AddGeom( "POD", cc )
    vsp.Update()

    for parent in ( c, cc, k ):
        vsp.SetGeomParent( a, parent )
        assert pop_errors(), "making a a child of %s raised no error" % parent
        assert vsp.GetGeomParent( a ) != parent

    vsp.Update()
    assert_no_errors()


def testAGeomCanMoveUnderAnotherBranch():
    vsp.VSPRenew()
    drop_errors()
    a = vsp.AddGeom( "POD" )
    b = vsp.AddGeom( "POD", a )
    c = vsp.AddGeom( "POD" )
    vsp.Update()

    vsp.SetGeomParent( c, b )
    vsp.Update()
    assert_no_errors()
    assert vsp.GetGeomParent( c ) == b
    assert c in vsp.GetGeomChildren( b )


def testReparentingAMissingGeomIsAnError():
    vsp.VSPRenew()
    drop_errors()
    a = vsp.AddGeom( "POD" )
    vsp.SetGeomParent( "NOTAGEOMID", a )
    assert pop_errors(), "reparenting a missing Geom raised no error"


def testARouteCannotLieOnAGeomBuiltFromIt():
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    route = vsp.AddGeom( "ROUTING" )
    vsp.AddRoutingPt( route, pod, 0 )
    vsp.Update()

    vsp.SetGeomParent( pod, route )
    assert pop_errors(), "putting a route's surface under the route raised no error"
    assert vsp.GetGeomParent( pod ) != route

    child = vsp.AddGeom( "POD", route )
    pt = vsp.AddRoutingPt( route, pod, 0 )
    vsp.SetRoutingPtParentID( pt, child )
    assert pop_errors(), "putting a routing point on the route's own child raised no error"
    assert vsp.GetRoutingPtParentID( pt ) == pod


def testAGeomCannotMoveUnderItsDetachedConformal():
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    conformal = vsp.AddGeom( "CONFORMAL", pod )
    vsp.SetParmVal( conformal, "DetachFlag", "Design", 1 )
    vsp.Update()

    # Detached, it stays on the pod after it moves away from it
    other = vsp.AddGeom( "POD" )
    vsp.SetGeomParent( conformal, other )
    vsp.Update()
    assert vsp.GetGeomParent( conformal ) == other

    vsp.SetGeomParent( pod, conformal )
    assert pop_errors(), "putting a conformal's surface under the conformal raised no error"
    assert vsp.GetGeomParent( pod ) != conformal
    vsp.Update()
    assert_no_errors()


def testAGeomCannotMoveUnderTheGearAttachedToIt():
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    gear = vsp.AddGeom( "GEAR" )
    vsp.CreateAndAddBogie( gear )
    vsp.Update()

    # Only the GUI sets a bogie's attach Geom, so it is set in the file
    fname = os.path.join( tempfile.mkdtemp(), "gear.vsp3" )
    vsp.WriteVSPFile( fname, vsp.SET_ALL )
    with open( fname ) as f:
        text = f.read()
    text = re.sub( r"<StowParentID\s*/>|<StowParentID>[^<]*</StowParentID>", "<StowParentID>%s</StowParentID>" % pod, text )
    with open( fname, "w" ) as f:
        f.write( text )
    vsp.VSPRenew()
    vsp.ReadVSPFile( fname )
    vsp.Update()
    drop_errors()

    vsp.SetGeomParent( pod, gear )
    assert pop_errors(), "putting a bogie's attach Geom under the gear raised no error"
    assert vsp.GetGeomParent( pod ) != gear
