import openvsp as vsp

def test_AnalysisDoc():
    errorMgr = vsp.ErrorMgrSingleton.getInstance()

    vsp.AddGeom( 'POD', '' )
    vsp.Update()

    # This is about what the analysis and its results document about themselves, so it should
    # not leave files behind.  A previous test in the same session may have read a model, and
    # the output is named after whatever file the Vehicle last held -- so without this the
    # run drops a <that model>_CompGeom.txt next to it.
    vsp.SetIntAnalysisInput( 'CompGeom', 'WriteTXTFlag', [0] )
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
