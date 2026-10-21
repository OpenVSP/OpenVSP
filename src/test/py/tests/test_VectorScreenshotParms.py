# The Vector Screenshot settings are Vehicle Parms, saved with the model.

import openvsp as vsp


def test_VectorScreenshotParms(tmp_path):
    vsp.VSPRenew()
    errorMgr = vsp.ErrorMgrSingleton.getInstance()

    veh = vsp.FindContainer('Vehicle', 0)
    vals = {'FileType': vsp.VEC_PDF, 'AutoCropFlag': 1, 'ScaleToPageFlag': 1,
            'PageWidth': 150.0, 'PageHeight': 100.0, 'PageUnits': vsp.LEN_MM}
    for name, val in vals.items():
        vsp.SetParmVal(vsp.FindParm(veh, name, 'VectorScreenshot'), val)

    fname = str(tmp_path / 'vecshot.vsp3')
    vsp.WriteVSPFile(fname)
    vsp.VSPRenew()

    veh = vsp.FindContainer('Vehicle', 0)
    assert vsp.GetParmVal(vsp.FindParm(veh, 'PageWidth', 'VectorScreenshot')) == 4.0

    vsp.ReadVSPFile(fname)
    veh = vsp.FindContainer('Vehicle', 0)
    for name, val in vals.items():
        assert vsp.GetParmVal(vsp.FindParm(veh, name, 'VectorScreenshot')) == val, name

    assert errorMgr.GetNumTotalErrors() == 0


if __name__ == '__main__':
    import pathlib
    import tempfile
    test_VectorScreenshotParms(pathlib.Path(tempfile.mkdtemp()))
