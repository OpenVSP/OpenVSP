import openvsp_config
# openvsp_config.LOAD_GRAPHICS = True
# openvsp_config.LOAD_FACADE = True

import openvsp as vsp

import time
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import patheffects


def test_InterferenceAnalysis():
    if openvsp_config.LOAD_GRAPHICS:
        vsp.StartGUI()
        vsp.SetShowBorders(False)
        vsp.SetViewAxis(False)

    errorMgr = vsp.ErrorMgrSingleton.getInstance()

    vsp.ReadVSPFile("TestInterference.vsp3")

    geom_id = vsp.FindGeom("BallA", 0)

    if openvsp_config.LOAD_GRAPHICS:
        # vsp.ShowScreen(vsp.VSP_INTERFERENCE_SCREEN) # Does not work for some reason.  Hangs here.
        vsp.SetGeomDrawType(geom_id, vsp.GEOM_DRAW_SHADE)
        vsp.SetGeomDrawType(vsp.FindGeom("BallB", 0), vsp.GEOM_DRAW_SHADE)

    vsp.SetAnalysisInputDefaults('InterferenceAnalysis')

    caseid = vsp.FindContainer('External', 0)
    # caseid = vsp.FindContainer('A_In_B', 0)

    vsp.SetStringAnalysisInput('InterferenceAnalysis', 'CaseID', [caseid])

    xpos = np.linspace(0, 4, 21)
    ypos = [0, 0.5, 1, 1.5, 2, 2.5]

    con = np.zeros(shape=len(xpos))

    fig = plt.figure(1)
    ax = fig.gca()
    ax.plot(xpos, np.zeros(shape=len(xpos)), path_effects=[patheffects.withTickedStroke(spacing=7)])

    for j, y in enumerate(ypos):
        for i, x in enumerate(xpos):
            vsp.SetParmVal(geom_id, "X_Rel_Location", "XForm", x)
            vsp.SetParmVal(geom_id, "Y_Rel_Location", "XForm", y)
            vsp.Update()
            res = vsp.ExecAnalysis('InterferenceAnalysis')
            con_val = vsp.GetDoubleResults(res, 'Con_Val')
            con[i] = con_val[0]
            if openvsp_config.LOAD_GRAPHICS:
                png_path = 'test_Interference_' + str(i) + '_' + str(j) + '.png'
                vsp.ScreenGrab(png_path, 1000, 1000, True, True)

        ax.plot(xpos, con, label='Y = ' + str(y))

    plt.title('Interference Sweep')
    plt.xlabel('X')
    plt.ylabel('Constraint Value')
    plt.legend()

    plt.savefig('test_InterferenceAnalysis.png')
    plt.close()

    # Check for errors
    num_err = errorMgr.GetNumTotalErrors()
    assert num_err == 0

    for i in range(0, num_err):
        err = errorMgr.PopLastError()
        print('error = ', err.m_ErrorString)


if __name__ == '__main__':
    test_InterferenceAnalysis()
