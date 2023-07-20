import openvsp as vsp
import pytest

def checkGeom():
    # Check wing points
    p = vsp.CompPnt01( 'IRCCKDQEGC', 0, 0.1, 0 )
    assert p.x() == pytest.approx( 8.0 )
    p = vsp.CompPnt01( 'IRCCKDQEGC', 0, 0.2, 0 )
    assert p.x() == pytest.approx( 7.0 )
    p = vsp.CompPnt01( 'IRCCKDQEGC', 0, 0.3, 0 )
    assert p.x() == pytest.approx( 6.0 )
    p = vsp.CompPnt01( 'IRCCKDQEGC', 0, 0.4, 0 )
    assert p.x() == pytest.approx( 5.0 )
    p = vsp.CompPnt01( 'IRCCKDQEGC', 0, 0.5, 0 )
    assert p.x() == pytest.approx( 4.0 )
    p = vsp.CompPnt01( 'IRCCKDQEGC', 0, 0.6, 0 )
    assert p.x() == pytest.approx( 3.0 )
    p = vsp.CompPnt01( 'IRCCKDQEGC', 0, 0.7, 0 )
    assert p.x() == pytest.approx( 2.0 )
    p = vsp.CompPnt01( 'IRCCKDQEGC', 0, 0.8, 0 )
    assert p.x() == pytest.approx( 1.0 )
    p = vsp.CompPnt01( 'IRCCKDQEGC', 0, 0.9, 0 )
    assert p.x() == pytest.approx( 0.1 )

    # Check pod points
    p = vsp.CompPnt01( 'VVMJBNYQFD', 0, 0.0, 0 ) # Check position
    assert p.x() == pytest.approx( 2.0 )
    p = vsp.CompPnt01( 'VVMJBNYQFD', 0, 1.0, 0 ) # Check length
    assert p.x() == pytest.approx( 6.0 )
    p = vsp.CompPnt01( 'VVMJBNYQFD', 0, 0.5, 0 ) # Check fineness
    assert p.z() == pytest.approx( 0.8 )


def testChordAdjust():
    errorMgr = vsp.ErrorMgrSingleton.getInstance()

    vsp.ReadVSPFile( "tests/TestChordAdjust.vsp3" )

    # Apply parameters as determined from Design Var file.
    vsp.SetParmVal( 'ZHMTPQPPTTH', 5 ) # ZHMTPQPPTTH:PodGeom:Design:FineRatio: 5
    vsp.SetParmVal( 'PUEEUGUFHCI', 4 ) # PUEEUGUFHCI:PodGeom:Design:Length: 4
    vsp.SetParmVal( 'USZKLSRLJKJ', 2 ) # USZKLSRLJKJ:PodGeom:XForm:X_Rel_Location: 2
    vsp.SetParmVal( 'OUTGIMZHQWJ', 1 ) # OUTGIMZHQWJ:WingGeom:XSec_7:Tip_Chord: 1
    vsp.SetParmVal( 'AXMFHPCTXLU', 6 ) # AXMFHPCTXLU:WingGeom:XSec_2:Tip_Chord: 6
    vsp.SetParmVal( 'PGHISXKFHTB', 2 ) # PGHISXKFHTB:WingGeom:XSec_6:Tip_Chord: 2
    vsp.SetParmVal( 'ACYSLVVAWSW', 4 ) # ACYSLVVAWSW:WingGeom:XSec_4:Tip_Chord: 4
    vsp.SetParmVal( 'BKQWYGUTWCH', 8 ) # BKQWYGUTWCH:WingGeom:XSec_1:Root_Chord: 8
    vsp.SetParmVal( 'QQCTIDXEFPZ', 7 ) # QQCTIDXEFPZ:WingGeom:XSec_1:Tip_Chord: 7
    vsp.SetParmVal( 'MIRCQJBBGQM', 5 ) # MIRCQJBBGQM:WingGeom:XSec_3:Tip_Chord: 5
    vsp.SetParmVal( 'EFATHAAEGBX', 3 ) # EFATHAAEGBX:WingGeom:XSec_5:Tip_Chord: 3
    vsp.SetParmVal( 'WYDZRMNLFMU', 0.1 ) # WYDZRMNLFMU:WingGeom:XSec_8:Tip_Chord: 0

    vsp.Update()

    checkGeom()

    vsp.ClearVSPModel()

    vsp.ReadVSPFile( "tests/TestChordAdjust.vsp3" )
    vsp.ReadApplyDESFile( "tests/TestChordAdjust.des" )

    checkGeom()

    # Check for errors
    num_err = errorMgr.GetNumTotalErrors()
    assert num_err == 0

    for i in range(0, num_err):
        err = errorMgr.PopLastError()
        print("error = ", err.m_ErrorString)


if __name__ == "__main__":
    testChordAdjust()
