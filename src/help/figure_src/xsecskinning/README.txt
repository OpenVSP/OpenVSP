Cross section and skinning help figures
=======================================

These scripts draw the figures of ../../XSecSkinning.md into ../../images/XSecSkinning_*.png.  Nothing in this folder
is installed; keep it free of .md files.

Run them with the Python of an OpenVSP build:

    cd src/help/figure_src/xsecskinning
    <build>/venv/bin/python verify.py                # checks what the page says against VSP
    <build>/venv/bin/python xsecskinning_figs.py     # redraws every figure
    <build>/venv/bin/python xsecskinning_figs.py angle spine     # or only some (see FIGS at the end)

Then refresh html/XSecSkinning.html with pandoc, as src/help/CMakeLists.txt does.

Files
-----

xsecskinning_figs.py   The figures.  Each is a Stack of circles two across and two apart, every section cleared of
                       skinning before the one being shown is set, sampled with CompPnt01.  Profiles are the Top line
                       seen from the side (X right, Z up); cuts are seen from behind (Y right, Z up).  Helpers come
                       from ../xseccurves/xsfig.py.
verify.py              Checks the page's statements about sections and skinning against the openvsp module.

The stations are drawn in the colors the 3D view uses (GeomXSec::SkinDrawColor), darkened to read on white.

shots.py takes the GUI screenshots for the page into ../../images/ with ../screenshots/snap.py (read its docstring
for VSP_SNAP_EXE and SNAP_CACHE):

    VSP_SNAP_EXE=<vsp> <build>/venv/bin/python shots.py
