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
        vsp.ShowScreen(vsp.VSP_GEOMETRY_ANALYSIS_SCREEN)
        vsp.SetGeomDrawType(geom_id, vsp.GEOM_DRAW_SHADE)
        vsp.SetGeomDrawType(vsp.FindGeom("BallB", 0), vsp.GEOM_DRAW_SHADE)

    vsp.SetAnalysisInputDefaults('GeometryAnalysis')

    caseid = vsp.FindContainer('External', 0)
    # caseid = vsp.FindContainer('A_In_B', 0)

    vsp.SetStringAnalysisInput('GeometryAnalysis', 'CaseID', [caseid])

    # Make this the active case so it is the one visualized / captured by ScreenGrab.
    vsp.SetActiveGeometryAnalysis(caseid)

    xpos = np.linspace(0, 4, 21)
    ypos = [0, 0.5, 1, 1.5, 2, 2.5]

    con_all = []
    con2_all = []

    for j, y in enumerate(ypos):
        con = np.zeros(shape=len(xpos))
        con2 = np.zeros(shape=len(xpos))
        for i, x in enumerate(xpos):
            vsp.SetParmVal(geom_id, "X_Rel_Location", "XForm", x)
            vsp.SetParmVal(geom_id, "Y_Rel_Location", "XForm", y)
            vsp.Update()
            res = vsp.ExecAnalysis('GeometryAnalysis')
            con_val = vsp.GetDoubleResults(res, 'Con_Val')
            con_val2 = vsp.GetDoubleResults(res, 'Con_Val2')
            con[i] = con_val[0]
            con2[i] = con_val2[0]
            if openvsp_config.LOAD_GRAPHICS:
                png_path = 'test_Interference_' + str(i) + '_' + str(j) + '.png'
                vsp.ScreenGrab(png_path, 1000, 1000, True, True)

        con_all.append(con)
        con2_all.append(con2)

    def make_figure(show_con, show_con2, filename):
        fig = plt.figure()
        ax = fig.gca()
        ax.plot(xpos, np.zeros(shape=len(xpos)), path_effects=[patheffects.withTickedStroke(spacing=7)])
        for j, y in enumerate(ypos):
            color = None
            if show_con:
                line, = ax.plot(xpos, con_all[j], label='Y = ' + str(y))
                color = line.get_color()
            if show_con2:
                if show_con:
                    # Match con's color for this case; con2 dashed to distinguish.
                    ax.plot(xpos, con2_all[j], color=color, linestyle='--')
                else:
                    ax.plot(xpos, con2_all[j], linestyle='--', label='Y = ' + str(y))
        ax.set_title('Interference Sweep')
        ax.set_xlabel('X')
        ax.set_ylabel('Constraint Value')
        ax.set_ylim(-2, 3)
        ax.legend()
        fig.savefig(filename)
        plt.close(fig)

    make_figure(True, False, 'test_InterferenceAnalysis_con.svg')
    make_figure(False, True, 'test_InterferenceAnalysis_con2.svg')
    make_figure(True, True, 'test_InterferenceAnalysis_both.svg')

    # Check for errors
    num_err = errorMgr.GetNumTotalErrors()
    assert num_err == 0

    for i in range(0, num_err):
        err = errorMgr.PopLastError()
        print('error = ', err.m_ErrorString)


if __name__ == '__main__':
    test_InterferenceAnalysis()
