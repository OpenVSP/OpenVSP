import openvsp as vsp

def test_NumAnalyses():
    lbefore = vsp.ListAnalysis()
    vsp.VSPRenew()
    lafter = vsp.ListAnalysis()
    assert len(lafter) == len(lbefore)

if __name__ == "__main__":
    test_NumAnalyses()
