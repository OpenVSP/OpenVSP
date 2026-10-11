# A Geom placed in absolute coordinates and attached to a wing tip stays there when the file
# is read back.
#
# A Conformal deletes the working copy of its parent each time it updates.  When that delete
# also updated the whole Vehicle, a Routing Geom later in the file updated the Geom its point
# sits on before the wing under it was built.  That Geom, in absolute mode, took its relative
# location from an identity attach matrix, so it was put back at the tip plus its absolute
# location.

import math
import os
import tempfile

import openvsp as vsp

from testhelp import drop_errors, assert_no_errors


def xform( geom_id, names ):
    return [ vsp.GetParmVal( geom_id, n, "XForm" ) for n in names ]


def build_model():
    vsp.VSPRenew()
    drop_errors()

    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fuse, "Length", "Design", 30.0 )

    # The Conformal comes before the Wing among the fuselage's children, so it updates first.
    vsp.AddGeom( "CONFORMAL", fuse )

    wing = vsp.AddGeom( "WING", fuse )
    vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 10.0 )
    vsp.SetParmVal( wing, "Z_Rel_Location", "XForm", 1.0 )

    pod = vsp.AddGeom( "POD", wing )
    vsp.SetParmVal( pod, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( pod, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_COMP )
    vsp.SetParmVal( pod, "U_Attach_Location", "Attach", 1.0 )
    vsp.SetParmVal( pod, "V_Attach_Location", "Attach", 0.5 )
    vsp.SetParmVal( pod, "X_Rel_Location", "XForm", 2.0 )
    vsp.SetParmVal( pod, "Y_Rel_Location", "XForm", 1.5 )
    vsp.Update()
    vsp.SetParmVal( pod, "Abs_Or_Relitive_flag", "XForm", vsp.ABS )
    vsp.Update()

    # A top level Routing Geom after the fuselage, with a point on the attached Geom.
    routing = vsp.AddGeom( "ROUTING" )
    vsp.AddRoutingPt( routing, pod, 0 )
    vsp.AddRoutingPt( routing, wing, 0 )
    vsp.Update()

    return wing, pod


def tip_offset( wing, pod ):
    tip = vsp.CompPnt01( wing, 0, 1.0, 0.5 )
    absloc = xform( pod, [ "X_Location", "Y_Location", "Z_Location" ] )
    return [ absloc[0] - tip.x(), absloc[1] - tip.y(), absloc[2] - tip.z() ]


def testAbsoluteGeomOnWingTipSurvivesReadBack():
    wing, pod = build_model()

    relnames = [ "X_Rel_Location", "Y_Rel_Location", "Z_Rel_Location" ]
    absnames = [ "X_Location", "Y_Location", "Z_Location" ]
    rel_before = xform( pod, relnames )
    abs_before = xform( pod, absnames )
    assert math.dist( tip_offset( wing, pod ), [ 2.0, 1.5, 0.0 ] ) < 1e-9

    fname = os.path.join( tempfile.mkdtemp(), "tiptest.vsp3" )
    vsp.WriteVSPFile( fname, vsp.SET_ALL )
    vsp.ClearVSPModel()
    vsp.ReadVSPFile( fname )
    vsp.Update()

    wing = vsp.FindGeomsWithName( "WingGeom" )[0]
    pod = vsp.FindGeomsWithName( "PodGeom" )[0]

    rel_after = xform( pod, relnames )
    abs_after = xform( pod, absnames )

    assert math.dist( rel_after, rel_before ) < 1e-9, ( rel_before, rel_after )
    assert math.dist( abs_after, abs_before ) < 1e-9, ( abs_before, abs_after )
    assert math.dist( tip_offset( wing, pod ), [ 2.0, 1.5, 0.0 ] ) < 1e-9

    assert_no_errors()



def testRoutingBeforeTheWingLeavesTheAttachedGeomAlone():
    # A Routing Geom first in the tree updates before the wing.  Its points read the Geoms
    # they sit on, but do not update them.
    vsp.VSPRenew()
    drop_errors()

    routing = vsp.AddGeom( "ROUTING" )

    fuse = vsp.AddGeom( "FUSELAGE" )
    vsp.SetParmVal( fuse, "Length", "Design", 30.0 )

    wing = vsp.AddGeom( "WING", fuse )
    vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 10.0 )
    vsp.SetParmVal( wing, "Z_Rel_Location", "XForm", 1.0 )

    pod = vsp.AddGeom( "POD", wing )
    vsp.SetParmVal( pod, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( pod, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_COMP )
    vsp.SetParmVal( pod, "U_Attach_Location", "Attach", 1.0 )
    vsp.SetParmVal( pod, "V_Attach_Location", "Attach", 0.5 )
    vsp.SetParmVal( pod, "X_Rel_Location", "XForm", 2.0 )
    vsp.SetParmVal( pod, "Y_Rel_Location", "XForm", 1.5 )
    vsp.Update()
    vsp.SetParmVal( pod, "Abs_Or_Relitive_flag", "XForm", vsp.ABS )
    vsp.Update()

    vsp.AddRoutingPt( routing, pod, 0 )
    vsp.AddRoutingPt( routing, wing, 0 )
    vsp.Update()

    pts_before = vsp.GetAllRoutingPtCoords( routing, 0 )
    assert math.dist( tip_offset( wing, pod ), [ 2.0, 1.5, 0.0 ] ) < 1e-9

    fname = os.path.join( tempfile.mkdtemp(), "routefirst.vsp3" )
    vsp.WriteVSPFile( fname, vsp.SET_ALL )
    vsp.ClearVSPModel()
    vsp.ReadVSPFile( fname )
    vsp.Update()

    routing = vsp.FindGeomsWithName( "RoutingGeom" )[0]
    wing = vsp.FindGeomsWithName( "WingGeom" )[0]
    pod = vsp.FindGeomsWithName( "PodGeom" )[0]

    assert math.dist( tip_offset( wing, pod ), [ 2.0, 1.5, 0.0 ] ) < 1e-9

    pts_after = vsp.GetAllRoutingPtCoords( routing, 0 )
    assert len( pts_after ) == len( pts_before )
    for p, q in zip( pts_before, pts_after ):
        assert ( p - q ).mag() < 1e-9

    # The route follows the wing it sits on.
    vsp.SetParmVal( wing, "Z_Rel_Location", "XForm", 6.0 )
    vsp.Update()
    for p, q in zip( pts_before, vsp.GetAllRoutingPtCoords( routing, 0 ) ):
        assert abs( ( q - p ).z() - 5.0 ) < 1e-9

    assert_no_errors()


def build_conformal_model( with_routing ):
    vsp.VSPRenew()
    drop_errors()

    fuse = vsp.AddGeom( "FUSELAGE" )
    # A joint updates the wing on every Vehicle update, and the wing its Conformal.
    pivot = vsp.AddGeom( "HINGE", fuse )
    vsp.SetParmVal( pivot, "X_Rel_Location", "XForm", 10.0 )
    wing = vsp.AddGeom( "WING", pivot )

    # The Conformal comes before the Hinge among the wing's children, so it updates first.
    conf = vsp.AddGeom( "CONFORMAL", wing )

    hinge = vsp.AddGeom( "HINGE", wing )
    vsp.SetParmVal( hinge, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( hinge, "U_Attach_Location", "Attach", 1.0 )
    vsp.SetParmVal( hinge, "X_Rel_Location", "XForm", 2.0 )
    vsp.Update()
    vsp.SetParmVal( hinge, "Abs_Or_Relitive_flag", "XForm", vsp.ABS )
    vsp.Update()

    if with_routing:
        routing = vsp.AddGeom( "ROUTING" )
        vsp.AddRoutingPt( routing, hinge, 0 )
        vsp.AddRoutingPt( routing, conf, 0 )
        vsp.Update()

    return hinge


def testRoutingLeavesTheGeomsItSitsOnAlone():
    # The same edit gives the same model with or without a route over it.
    xform_names = [ "X_Location", "Y_Location", "Z_Location", "X_Rel_Location", "Y_Rel_Location", "Z_Rel_Location" ]

    result = []
    for with_routing in [ False, True ]:
        hinge = build_conformal_model( with_routing )
        x = vsp.GetParmVal( hinge, "X_Location", "XForm" )
        vsp.SetParmVal( hinge, "X_Location", "XForm", x + 0.7 )
        vsp.Update()
        result.append( xform( hinge, xform_names ) )

    assert math.dist( result[0], result[1] ) < 1e-12, result

    assert_no_errors()
