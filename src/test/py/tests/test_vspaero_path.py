import openvsp as vsp

def test_vspaero_path():
    vsp_found = vsp.CheckForVSPAERO( vsp.GetVSPAEROPath() )
    if not vsp_found:
      print( "VSPAERO is not where OpenVSP thinks it is." )

    assert vsp_found

if __name__ == "__main__":
    test_vspaero_path()
