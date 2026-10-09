# Reparenting a Geom through the API.
#
# SetGeomParent once let a Geom become the child of its own child, or of its own Clone,
# and the next Update recursed around the loop until the stack ran out.

import openvsp as vsp

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
