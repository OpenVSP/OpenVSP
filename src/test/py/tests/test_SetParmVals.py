# SetParmVals sets many Parms in one call, or none of them.

import openvsp as vsp
import pytest

from testhelp import ( drop_errors, assert_no_errors, pop_errors )


def a_pod_and_parms():
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    ids = [ vsp.GetParm( pod, "X_Rel_Location", "XForm" ),
            vsp.GetParm( pod, "Length", "Design" ),
            vsp.GetParm( pod, "FineRatio", "Design" ) ]
    return pod, ids


def testSetParmValsSetsEachParm():
    pod, ids = a_pod_and_parms()
    vals = [ 2.5, 13.0, 4.0 ]
    got = vsp.SetParmVals( ids, vals )
    vsp.Update()
    assert list( got ) == pytest.approx( vals )
    for pid, v in zip( ids, vals ):
        assert vsp.GetParmVal( pid ) == pytest.approx( v )
    assert_no_errors()


def testSetParmValsReturnsTheClampedValues():
    pod, ids = a_pod_and_parms()
    lower = vsp.GetParmLowerLimit( ids[1] )
    got = vsp.SetParmVals( ids[1:2], [ lower - 100.0 ] )
    assert list( got ) == pytest.approx( [ lower ] )
    assert vsp.GetParmVal( ids[1] ) == pytest.approx( lower )
    assert_no_errors()


def testSetParmValsWithAnEmptyListDoesNothing():
    pod, ids = a_pod_and_parms()
    assert len( vsp.SetParmVals( [], [] ) ) == 0
    assert_no_errors()


@pytest.mark.parametrize( "case", [ "sizes", "badid" ] )
def testSetParmValsSetsNothingOnBadInput( case ):
    pod, ids = a_pod_and_parms()
    before = [ vsp.GetParmVal( pid ) for pid in ids ]
    if case == "sizes":
        got = vsp.SetParmVals( ids, [ 1.0, 2.0 ] )
        what = "Got 3 Parm IDs and 2 values"
    else:
        got = vsp.SetParmVals( ids[:2] + [ "BADID" ], [ 3.0, 14.0, 5.0 ] )
        what = "Can't Find Parm BADID"
    assert len( got ) == 0
    assert [ vsp.GetParmVal( pid ) for pid in ids ] == before
    assert pop_errors() == [ "SetParmVals::" + what ]
