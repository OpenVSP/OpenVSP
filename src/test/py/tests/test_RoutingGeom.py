import openvsp as vsp
import pytest

def testRoutingGeom():
    errorMgr = vsp.ErrorMgrSingleton.getInstance()

    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)


    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)
    # u1 = vsp.GetParm( rpt1, 'U', 'RoutePt')       # U,W=0.5 is default.
    # vsp.SetParmVal(u1, 0.5)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    vsp.Update()
    assert vsp.GetNumRoutingPts(routing_geom) == 3
    assert vsp.GetParmVal(routing_geom, 'Length', 'Results') == pytest.approx(10.85254706406647)

    rptPre2 = vsp.InsertRoutingPt(routing_geom, 2, pod2, 0)
    uPre2 = vsp.GetParm( rptPre2, 'U', 'RoutePt')
    vsp.SetParmVal(uPre2, 0.)
    assert vsp.GetNumRoutingPts(routing_geom) == 4

    vsp.Update()
    assert vsp.GetParmVal(routing_geom, 'Length', 'Results') == pytest.approx(20.668561209359325)

    rpts = vsp.GetAllRoutingPtIds(routing_geom)
    assert rpts[0] == rpt0
    assert rpts[1] == rpt1
    assert rpts[2] == rptPre2
    assert rpts[3] == rpt2

    newindx = vsp.MoveRoutingPt( routing_geom, 1, vsp.REORDER_MOVE_DOWN )
    assert newindx == 2
    assert vsp.GetRoutingPtID(routing_geom, newindx) == rpt1

    rpts = vsp.GetAllRoutingPtIds(routing_geom)
    assert rpts[0] == rpt0
    assert rpts[1] == rptPre2
    assert rpts[2] == rpt1
    assert rpts[3] == rpt2

    vsp.Update()
    assert vsp.GetParmVal(routing_geom, 'Length', 'Results') == pytest.approx(12.470522182173754)

    vsp.DelRoutingPt( routing_geom, 1 )
    assert vsp.GetNumRoutingPts(routing_geom) == 3
    vsp.Update()
    assert vsp.GetParmVal(routing_geom, 'Length', 'Results') == pytest.approx(10.85254706406647)

    assert vsp.GetRoutingPtParentID(rpt1) == pod2

    vsp.SetRoutingPtParentID(rpt1, pod1)
    vsp.Update()

    assert vsp.GetParmVal(routing_geom, 'Length', 'Results') == pytest.approx(10.088497300281038)

    vsp.DelAllRoutingPt( routing_geom )
    assert vsp.GetNumRoutingPts(routing_geom) == 0
    vsp.Update()
    assert vsp.GetParmVal(routing_geom, 'Length', 'Results') == pytest.approx(0.0)

    # vsp.WriteVSPFile('routingtest.vsp3')
    # print( vsp.GetParmVal(routing_geom, 'Length', 'Results') )

    # Check for errors
    num_err = errorMgr.GetNumTotalErrors()
    assert num_err == 0

    for i in range(0, num_err):
        err = errorMgr.PopLastError()
        print("error = ", err.m_ErrorString)


if __name__ == "__main__":
    testRoutingGeom()
