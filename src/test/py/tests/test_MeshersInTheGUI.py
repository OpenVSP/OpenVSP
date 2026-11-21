import importlib.util
import os
import signal
import subprocess
import sys

import pytest


# CFD Mesh, FEA Mesh and Surface Intersection run on worker threads that write their progress to a screen's
# console.  With the GUI up, the API call holds the GUI's lock while it waits on those threads, so each run here
# would hang if the console text still needed that lock.
def meshers_in_the_gui(vsp):
    vsp.StartGUI()

    pid = vsp.AddGeom("POD")
    wid = vsp.AddGeom("WING")
    vsp.Update()

    vsp.SetAnalysisInputDefaults("CfdMeshAnalysis")
    vsp.SetDoubleAnalysisInput("CfdMeshAnalysis", "BaseLen", [0.5])
    vsp.SetDoubleAnalysisInput("CfdMeshAnalysis", "MinLen", [0.05])
    rid = vsp.ExecAnalysis("CfdMeshAnalysis")
    assert vsp.GetIntResults(rid, "Num_Tris")[0] > 0

    vsp.AddFeaStruct(pid)
    vsp.SetFeaMeshStructIndex(0)
    vsp.SetAnalysisInputDefaults("FeaMeshAnalysis")
    vsp.SetDoubleAnalysisInput("FeaMeshAnalysis", "BaseLen", [0.5])
    vsp.SetDoubleAnalysisInput("FeaMeshAnalysis", "MinLen", [0.05])
    vsp.ExecAnalysis("FeaMeshAnalysis")

    vsp.SetAnalysisInputDefaults("SurfaceIntersection")
    vsp.ExecAnalysis("SurfaceIntersection")

    vsp.StopGUI()


def test_MeshersRunWithTheGUIUp():
    if importlib.util.find_spec('openvsp._vsp_g') is None:
        pytest.skip('the graphics module, pyvsp_g, is not built')
    if sys.platform.startswith('linux') and not os.environ.get('DISPLAY'):
        pytest.skip('no display to start the GUI on')
    # The GUI takes openvsp loaded with graphics through the facade, so this runs in a process of its own.  The
    # facade's server is a process of that one's, so a hang is ended by killing the whole group.
    proc = subprocess.Popen([sys.executable, __file__], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                            start_new_session=True)
    try:
        out, _ = proc.communicate(timeout=180)
    except subprocess.TimeoutExpired:
        os.killpg(proc.pid, signal.SIGKILL)
        proc.communicate()
        pytest.fail('a mesher hung with the GUI up')
    assert proc.returncode == 0, out


if __name__ == '__main__':
    import openvsp_config
    openvsp_config.LOAD_GRAPHICS = True
    openvsp_config.LOAD_FACADE = True
    import openvsp as vsp

    meshers_in_the_gui(vsp)
