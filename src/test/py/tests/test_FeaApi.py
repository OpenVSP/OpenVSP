# The FEA structure API.

import openvsp as vsp

from testhelp import ( pop_errors, drop_errors, assert_no_errors )


def testDeletingAFeaSubSurfFromAnotherStructureIsRefused():
    """DeleteFeaSubSurf refuses a subsurface of another structure and leaves both structures alone."""
    vsp.VSPRenew()
    drop_errors()
    pod = vsp.AddGeom( "POD" )
    first = vsp.AddFeaStruct( pod )
    second = vsp.AddFeaStruct( pod )
    kept = vsp.AddFeaSubSurf( pod, first, vsp.SS_RECTANGLE )
    other = vsp.AddFeaSubSurf( pod, second, vsp.SS_RECTANGLE )
    vsp.Update()
    first_id = vsp.GetFeaStructID( pod, first )
    second_id = vsp.GetFeaStructID( pod, second )

    vsp.DeleteFeaSubSurf( pod, first, other )
    assert pop_errors(), "deleting another structure's subsurface raised no error"
    assert vsp.GetFeaSubSurfIDVec( first_id ) == ( kept, ), vsp.GetFeaSubSurfIDVec( first_id )
    assert vsp.GetFeaSubSurfIDVec( second_id ) == ( other, ), vsp.GetFeaSubSurfIDVec( second_id )

    vsp.DeleteFeaSubSurf( pod, second, other )
    assert_no_errors()
    assert vsp.GetFeaSubSurfIDVec( second_id ) == (), vsp.GetFeaSubSurfIDVec( second_id )
