import openvsp as vsp

def test_AnalysisDoc():
    vsp.AddGeom( 'POD', '' )
    vsp.Update()

    vsp.SetIntAnalysisInput( 'CompGeom', 'WriteCSVFlag', [0] )

    vsp.PrintAnalysisInputs( 'CompGeom' )
    vsp.PrintAnalysisDocs( 'CompGeom' )
    res = vsp.ExecAnalysis( 'CompGeom' )
    vsp.PrintResults(res)
    vsp.PrintResultsDocs(res)


if __name__ == "__main__":
    test_AnalysisDoc()
