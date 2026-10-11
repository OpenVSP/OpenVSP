# The default edge angle of a control surface comes from the span tangent and the chord.

import math

import openvsp as vsp

from testhelp import drop_errors, assert_no_errors


def control_angles( ustart, cap ):
    vsp.VSPRenew()
    drop_errors()

    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "Sweep", "XSec_1", 30.0 )
    vsp.SetParmVal( w, "CapUMinOption", "EndCap", cap )
    vsp.Update()

    ss = vsp.AddSubSurf( w, vsp.SS_CONTROL, 0 )
    ids = { vsp.GetParmName( p ): p for p in vsp.GetSubSurfParmIDs( ss ) }
    vsp.SetParmVal( ids[ "UStart" ], ustart )
    vsp.SetParmVal( ids[ "UEnd" ], ustart + 0.05 )
    vsp.Update()

    return vsp.GetParmVal( ids[ "StartAngle" ] ), vsp.GetParmVal( ids[ "EndAngle" ] )


def testControlSurfaceAngleOnTheSpan():
    # The same span station gives the same angle with or without a root cap, and a wing
    # without a cap gives it at its root too.
    nocap, nocap_end = control_angles( 0.4, vsp.NO_END_CAP )
    cap, cap_end = control_angles( 0.4, vsp.FLAT_END_CAP )
    root, root_end = control_angles( 0.0, vsp.NO_END_CAP )

    assert 90.0 < nocap < 180.0
    for a in [ nocap_end, cap, cap_end, root, root_end ]:
        assert abs( a - nocap ) < 1e-9
    assert_no_errors()


def testControlSurfaceStartingOnTheRootCap():
    # Across a flat root cap the trailing edge does not move, so there is no span tangent.
    start, end = control_angles( 0.0, vsp.FLAT_END_CAP )
    assert start == 0.0
    assert math.isfinite( end )
    assert_no_errors()
