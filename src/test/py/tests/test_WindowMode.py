import importlib.util
import os
import subprocess
import sys

import pytest


def window_mode_in_the_gui(vsp):
    vsp.StartGUI()
    vsp.ShowScreen(vsp.VSP_MANAGE_GEOM_SCREEN)

    # Each mode, from either, and the one already set
    for mode in (vsp.WINDOW_MODE_CAPTIVE, vsp.WINDOW_MODE_FREE, vsp.WINDOW_MODE_FREE, vsp.WINDOW_MODE_CAPTIVE,
                 vsp.WINDOW_MODE_CAPTIVE, vsp.WINDOW_MODE_FREE):
        vsp.SetWindowMode(mode)
        assert vsp.GetWindowMode() == mode

    # A mode that is not one leaves the mode as it was
    vsp.SetWindowMode(vsp.WINDOW_MODE_CAPTIVE)
    vsp.SetWindowMode(5)
    assert vsp.GetWindowMode() == vsp.WINDOW_MODE_CAPTIVE
    vsp.SetWindowMode(vsp.WINDOW_MODE_FREE)


def test_WindowModeWithoutTheGUI():
    import openvsp as vsp

    # Nothing to show: the call does nothing, and the screens are free
    vsp.SetWindowMode(vsp.WINDOW_MODE_CAPTIVE)
    assert vsp.GetWindowMode() == vsp.WINDOW_MODE_FREE


def test_WindowModeInTheGUI():
    if importlib.util.find_spec('openvsp._vsp_g') is None:
        pytest.skip('the graphics module, pyvsp_g, is not built')
    if sys.platform.startswith('linux') and not os.environ.get('DISPLAY'):
        pytest.skip('no display to start the GUI on')
    # The GUI takes openvsp loaded with graphics through the facade, so this runs in a process of its own; in
    # this one the other tests have loaded it without.
    done = subprocess.run([sys.executable, __file__], capture_output=True, text=True, timeout=300)
    assert done.returncode == 0, done.stdout + done.stderr


if __name__ == '__main__':
    import openvsp_config
    openvsp_config.LOAD_GRAPHICS = True
    openvsp_config.LOAD_FACADE = True
    import openvsp as vsp

    window_mode_in_the_gui(vsp)
