import openvsp as vsp

def test_AnalysisDoc():
    errorMgr = vsp.ErrorMgrSingleton.getInstance()

    vsp.AddGeom( 'POD', '' )
    vsp.Update()

    vsp.SetIntAnalysisInput( 'CompGeom', 'WriteCSVFlag', [0] )

    vsp.PrintAnalysisInputs( 'CompGeom' )
    vsp.PrintAnalysisDocs( 'CompGeom' )
    res = vsp.ExecAnalysis( 'CompGeom' )
    vsp.PrintResults(res)
    vsp.PrintResultsDocs(res)

    # Check for errors
    num_err = errorMgr.GetNumTotalErrors()
    assert num_err == 0

    for i in range(0, num_err):
        err = errorMgr.PopLastError()
        print("error = ", err.m_ErrorString)


if __name__ == "__main__":
    test_AnalysisDoc()
