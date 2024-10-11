import time
import openvsp_config

openvsp_config.LOAD_GRAPHICS = True
openvsp_config.LOAD_FACADE = True
import openvsp as vsp


def test_FourViewScreenShot():
    vsp.StartGUI()

    time.sleep(1)  # Needed or following API calls fail.

    errorMgr = vsp.ErrorMgrSingleton.getInstance()

    pod = vsp.AddGeom('POD', '')
    vsp.SetGeomDrawType(pod, vsp.GEOM_DRAW_SHADE)
    vsp.SetGeomMaterialName( pod, 'Ruby')

    wing = vsp.AddGeom('WING', '')
    vsp.SetGeomDrawType(wing, vsp.GEOM_DRAW_SHADE)
    vsp.SetGeomMaterialName(wing, 'Jade')
    xwing = vsp.GetParm(wing, 'X_Rel_Location', 'XForm')
    vsp.SetParmVal(xwing, 2.25)

    vsp.Update()

    vsp.SetWindowLayout(2, 2)
    vsp.FitAllViews()
    vsp.UpdateGUI()

    time.sleep(1)  # Needed or UpdateGUI() does not have time to complete,

    png_path = 'favorite_airplane.png'
    vsp.ScreenGrab(png_path, 400, 400, True)

    # Check for errors
    num_err = errorMgr.GetNumTotalErrors()
    assert num_err == 0

    for i in range(0, num_err):
        err = errorMgr.PopLastError()
        print('error = ', err.m_ErrorString)


if __name__ == '__main__':
    test_FourViewScreenShot()
