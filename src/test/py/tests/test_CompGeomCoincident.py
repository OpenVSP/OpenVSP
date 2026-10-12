# Bodies that touch face to face, or lie one on another, give the same wetted area whichever way
# rounding falls.  A face of one normal surface lying on a face of another is inside it when the
# two face apart -- the bodies touch there -- and of two faces that point the same way only one
# is kept.

import os
import tempfile

import openvsp as vsp

from testhelp import drop_errors, assert_no_errors

TOL = 1e-9


def scratch_output():
    out = tempfile.mkdtemp()
    vsp.SetComputationFileName( vsp.COMP_GEOM_TXT_TYPE, os.path.join( out, "comp_geom.txt" ) )
    vsp.SetComputationFileName( vsp.COMP_GEOM_CSV_TYPE, os.path.join( out, "comp_geom.csv" ) )


def wet_areas():
    scratch_output()
    vsp.ComputeCompGeom( vsp.SET_ALL, False, 0 )
    rid = vsp.FindLatestResultsID( "Comp_Geom" )
    return list( vsp.GetDoubleResults( rid, "Wet_Area" ) ), list( vsp.GetDoubleResults( rid, "Theo_Area" ) )


def straight_wing( span, y ):
    w = vsp.AddGeom( "WING" )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", 0 )
    vsp.SetParmVal( w, "Span", "XSec_1", span )
    vsp.SetParmVal( w, "Sweep", "XSec_1", 0.0 )
    vsp.SetParmVal( w, "Root_Chord", "XSec_1", 1.0 )
    vsp.SetParmVal( w, "Tip_Chord", "XSec_1", 1.0 )
    vsp.SetParmVal( w, "Y_Rel_Location", "XForm", y )
    return w


def touching_wings( turn ):
    vsp.VSPRenew()
    drop_errors()
    straight_wing( 4.0, 0.0 )
    outer = straight_wing( 3.0, 4.0 )
    vsp.SetParmVal( outer, "Z_Rel_Rotation", "XForm", turn )
    vsp.Update()
    return wet_areas()


def testWingsTouchingAtACapLoseBothCaps():
    wet, theo = touching_wings( 0.0 )

    # The tip cap of one and the root cap of the other are the same section.
    cap = theo[0] - wet[0]
    assert cap > 0.01
    assert abs( ( theo[1] - wet[1] ) - cap ) < TOL

    # Rounding either way changes nothing; a turn far below any real gap changes only what the
    # turn itself moves.
    for turn, tol in [ ( 1e-13, TOL ), ( -1e-13, TOL ), ( 1e-9, 1e-6 ), ( -1e-9, 1e-6 ) ]:
        w, t = touching_wings( turn )
        assert max( abs( a - b ) for a, b in zip( w, wet ) ) < tol, ( turn, w, wet )

    assert_no_errors()


def testADuplicateBodyAddsNoWettedArea():
    def total( copies ):
        vsp.VSPRenew()
        drop_errors()
        for i in range( copies ):
            pod = vsp.AddGeom( "POD" )
            vsp.SetParmVal( pod, "Y_Rel_Rotation", "XForm", 17.0 )
        vsp.Update()
        wet, theo = wet_areas()
        return sum( wet ), theo[0]

    one, theo = total( 1 )
    two, theo2 = total( 2 )
    assert abs( one - theo ) < TOL
    assert abs( two - one ) < TOL * theo, ( one, two )

    assert_no_errors()


def testMirrorHalvesOverlappingAcrossTheirPlaneAgree():
    # A symmetric wing set off its plane so the halves overlap across it.  Each half keeps the
    # overlap on its own side, so the halves come out alike -- not exactly, as the intersection
    # cuts the two halves' coincident faces differently near the plane.  Kept by mesh order, one
    # half held the whole overlap and the other lost it: 8.18 against 4.10.
    vsp.VSPRenew()
    drop_errors()
    w = straight_wing( 4.0, -1.0 )
    vsp.SetParmVal( w, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ )
    vsp.Update()
    wet, theo = wet_areas()

    assert len( wet ) == 2
    assert abs( wet[0] - wet[1] ) < 0.02 * theo[0], wet
    assert wet[0] < theo[0] - 0.01, ( wet, theo )

    assert_no_errors()
