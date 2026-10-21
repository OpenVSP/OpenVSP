# VectorScreenGrab checks its arguments before it needs the GUI, so the errors are seen without it.

import os
import openvsp as vsp


def test_VectorScreenGrabErrors(tmp_path):
    vsp.VSPRenew()
    errorMgr = vsp.ErrorMgrSingleton.getInstance()
    while errorMgr.GetNumTotalErrors() > 0:
        errorMgr.PopLastError()

    bad = str(tmp_path / 'bad.svg')
    for args, code in (((bad, vsp.VEC_NUM_TYPES), vsp.VSP_INVALID_TYPE),
                       ((bad, -1), vsp.VSP_INVALID_TYPE),
                       ((bad, vsp.VEC_SVG, False, True, 0.0, 3.0, vsp.LEN_IN), vsp.VSP_INVALID_INPUT_VAL),
                       ((bad, vsp.VEC_SVG, False, True, 4.0, -3.0, vsp.LEN_IN), vsp.VSP_INVALID_INPUT_VAL),
                       ((bad, vsp.VEC_SVG, False, True, 4.0, 3.0, vsp.LEN_FT), vsp.VSP_INVALID_TYPE)):
        vsp.VectorScreenGrab(*args)
        assert errorMgr.GetNumTotalErrors() == 1, args
        assert errorMgr.PopLastError().m_ErrorCode == code, args
        assert not os.path.exists(bad)

    # Without scaletopage the page arguments are not used, so they are not checked
    vsp.VectorScreenGrab(bad, vsp.VEC_PDF, True, False, 0.0, 0.0, vsp.LEN_FT)
    assert errorMgr.GetNumTotalErrors() == 0


if __name__ == '__main__':
    import pathlib
    import tempfile
    test_VectorScreenGrabErrors(pathlib.Path(tempfile.mkdtemp()))
