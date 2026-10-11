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

