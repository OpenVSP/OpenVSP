import importlib.util
import os
import subprocess
import sys
import time

import pytest


def windows_user32():
    """user32, with the window handles it takes and returns declared as handles, not ints."""
    import ctypes
    import ctypes.wintypes as wt

    user32 = ctypes.windll.user32
    user32.GetWindow.restype = wt.HWND
    user32.GetWindow.argtypes = [wt.HWND, wt.UINT]
    user32.GetWindowTextLengthW.argtypes = [wt.HWND]
    user32.GetWindowTextW.argtypes = [wt.HWND, wt.LPWSTR, ctypes.c_int]
    user32.IsWindowVisible.argtypes = [wt.HWND]
    return user32


def child_windows(hwnd):
    """The visible child windows of a Windows window, the one drawn on top first."""
    import ctypes

    user32 = windows_user32()
    GW_CHILD, GW_HWNDNEXT = 5, 2

    def title(h):
        n = user32.GetWindowTextLengthW(h)
        buf = ctypes.create_unicode_buffer(n + 1)
        user32.GetWindowTextW(h, buf, n + 1)
        return buf.value

    out = []
    child = user32.GetWindow(hwnd, GW_CHILD)
    while child:
        if user32.IsWindowVisible(child):
            out.append((child, title(child)))
        child = user32.GetWindow(child, GW_HWNDNEXT)
    return out


def main_window():
    """The GUI's main window, found by its title: the GUI runs in a process of its own."""
    import ctypes
    import ctypes.wintypes as wt

    user32 = windows_user32()
    found = []

    def cb(hwnd, _):
        n = user32.GetWindowTextLengthW(hwnd)
        buf = ctypes.create_unicode_buffer(n + 1)
        user32.GetWindowTextW(hwnd, buf, n + 1)
        if user32.IsWindowVisible(hwnd) and buf.value.startswith('OpenVSP 3.'):
            found.append(hwnd)
        return True

    proc = ctypes.WINFUNCTYPE(wt.BOOL, wt.HWND, wt.LPARAM)(cb)
    user32.EnumWindows(proc, 0)
    assert found, 'the main window was not found'
    return found[0]


def captive_screen_over_the_view(vsp):
    """A screen shown inside the main window is drawn over the 3D view, not behind it.

    Windows draws a child window just created beneath its siblings, where FLTK's model draws it on
    top, so the windows in the main window are put in order whenever one of them is shown.
    """
    if sys.platform != 'win32':
        return

    vsp.SetWindowMode(vsp.WINDOW_MODE_CAPTIVE)
    try:
        time.sleep(1.0)

        main = main_window()
        before = set(h for h, t in child_windows(main))

        vsp.ShowScreen(vsp.VSP_CLIPPING_SCREEN)
        time.sleep(1.0)

        windows = child_windows(main)
        order = [h for h, t in windows]
        view = [h for h, t in windows if t == 'VSP GL Window']
        new = [h for h in order if h not in before]
        assert view, 'the 3D view was not found'
        assert new, 'the screen was not shown inside the main window'
        # Every window shown since, the screen's frame among them, is above the view
        for h in new:
            assert order.index(h) < order.index(view[0]), 'a window inside the main window is drawn behind the 3D view'
    finally:
        vsp.HideScreen(vsp.VSP_CLIPPING_SCREEN)
        vsp.SetWindowMode(vsp.WINDOW_MODE_FREE)


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

    captive_screen_over_the_view(vsp)


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
