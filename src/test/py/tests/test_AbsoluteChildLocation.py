# A child Geom in absolute mode keeps an absolute location or rotation set through the API when
# it is updated by itself before the Vehicle, as the UpdateGeom documentation describes.

import pytest

import openvsp as vsp

from testhelp import drop_errors, assert_no_errors


def build_child():
    vsp.VSPRenew()
    drop_errors()

    wing = vsp.AddGeom( "WING" )
    vsp.SetParmVal( wing, "X_Rel_Location", "XForm", 10.0 )
    vsp.SetParmVal( wing, "Y_Rel_Rotation", "XForm", 5.0 )

    pod = vsp.AddGeom( "POD", wing )
    vsp.SetParmVal( pod, "Trans_Attach_Flag", "Attach", vsp.ATTACH_TRANS_UV )
    vsp.SetParmVal( pod, "Rots_Attach_Flag", "Attach", vsp.ATTACH_ROT_UV )
    vsp.SetParmVal( pod, "U_Attach_Location", "Attach", 1.0 )
    vsp.SetParmVal( pod, "X_Rel_Location", "XForm", 2.0 )
    vsp.Update()
    vsp.SetParmVal( pod, "Abs_Or_Relitive_flag", "XForm", vsp.ABS )
    vsp.Update()

    return pod


@pytest.mark.parametrize( "name, delta", [ ( "X_Location", 0.7 ), ( "Z_Location", -1.3 ), ( "Y_Rotation", 12.0 ) ] )
@pytest.mark.parametrize( "how", [ "UpdateGeom", "SetParmValUpdate" ] )
def testAbsoluteEditOfAChildIsKept( name, delta, how ):
    pod = build_child()
    before = vsp.GetParmVal( pod, name, "XForm" )

    if how == "UpdateGeom":
        vsp.SetParmVal( pod, name, "XForm", before + delta )
        vsp.UpdateGeom( pod )
    else:
        vsp.SetParmValUpdate( pod, name, "XForm", before + delta )
    vsp.Update()

    assert abs( vsp.GetParmVal( pod, name, "XForm" ) - ( before + delta ) ) < 1e-9

    # Another update leaves it there.
    vsp.SetParmVal( pod, "Length", "Design", vsp.GetParmVal( pod, "Length", "Design" ) + 1.0 )
    vsp.Update()
    assert abs( vsp.GetParmVal( pod, name, "XForm" ) - ( before + delta ) ) < 1e-9

    assert_no_errors()
