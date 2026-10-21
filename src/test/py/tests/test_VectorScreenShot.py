import importlib.util
import os
import re
import subprocess
import sys

import pytest


def svg_size(fname):
    with open(fname) as f:
        m = re.search(r'width="([0-9.e+-]+)pt" height="([0-9.e+-]+)pt"', f.read())
    return float(m.group(1)), float(m.group(2))


def vector_screen_shots(vsp, tmp_path):
    vsp.StartGUI()

    pod = vsp.AddGeom('POD', '')
    vsp.SetGeomDrawType(pod, vsp.GEOM_DRAW_SHADE)
    vsp.Update()
    vsp.FitAllViews()

    # Each file type, by its contents
    for t, ext, magic in ((vsp.VEC_SVG, 'svg', b'<?xml'), (vsp.VEC_PDF, 'pdf', b'%PDF')):
        fname = str(tmp_path / ('pod.' + ext))
        vsp.VectorScreenGrab(fname, t)
        with open(fname, 'rb') as f:
            assert f.read(16).startswith(magic), ext

    # A cropped page is smaller than the window
    full = str(tmp_path / 'full.svg')
    crop = str(tmp_path / 'crop.svg')
    vsp.VectorScreenGrab(full, vsp.VEC_SVG, False)
    vsp.VectorScreenGrab(crop, vsp.VEC_SVG, True)
    fw, fh = svg_size(full)
    cw, ch = svg_size(crop)
    assert cw < fw and ch < fh

    # Scaled to a 100 mm square page, the window fits it with its aspect kept
    page = str(tmp_path / 'page.svg')
    vsp.VectorScreenGrab(page, vsp.VEC_SVG, False, True, 100.0, 100.0, vsp.LEN_MM)
    pw, ph = svg_size(page)
    side = 100.0 / 25.4 * 72.0
    assert abs(max(pw, ph) - side) < 0.01
    assert abs(pw / ph - fw / fh) < 1e-3

    # A cropped page keeps the scale of the whole window
    pagecrop = str(tmp_path / 'pagecrop.svg')
    vsp.VectorScreenGrab(pagecrop, vsp.VEC_SVG, True, True, 100.0, 100.0, vsp.LEN_MM)
    pcw, pch = svg_size(pagecrop)
    assert abs(pcw / cw - pw / fw) < 0.02


def test_VectorScreenShot(tmp_path):
    if importlib.util.find_spec('openvsp._vsp_g') is None:
        pytest.skip('the graphics module, pyvsp_g, is not built')
    if sys.platform.startswith('linux') and not os.environ.get('DISPLAY'):
        pytest.skip('no display to start the GUI on')
    # The GUI takes openvsp loaded with graphics through the facade, so the shots are taken in a
    # process of their own; in this one the other tests have loaded it without.
    done = subprocess.run([sys.executable, __file__, str(tmp_path)], capture_output=True, text=True, timeout=300)
    assert done.returncode == 0, done.stdout + done.stderr


if __name__ == '__main__':
    import pathlib
    import tempfile

    import openvsp_config
    openvsp_config.LOAD_GRAPHICS = True
    openvsp_config.LOAD_FACADE = True
    import openvsp as vsp

    if len(sys.argv) > 1:
        out = pathlib.Path(sys.argv[1])
    else:
        out = pathlib.Path(tempfile.mkdtemp())
    vector_screen_shots(vsp, out)
