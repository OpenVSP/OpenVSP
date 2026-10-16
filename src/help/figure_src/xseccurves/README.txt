Cross section curve help figures
================================

These scripts draw the figures of ../../XSecCurves.md into ../../images/XSecCurves_*.png.  Nothing in this folder is
installed (the help CMakeLists installs images/ and builds every *.md, so keep this folder free of .md files).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/xseccurves
    <build>/venv/bin/python verify.py               # checks what the page says against VSP
    <build>/venv/bin/python xseccurves_figs.py      # redraws every figure
    <build>/venv/bin/python xseccurves_figs.py rrect genfuse    # or only some (see FIGS at the end)

After redrawing or editing, build the help target (or run pandoc as src/help/CMakeLists.txt does) so that
html/XSecCurves.html, the copy used when pandoc is missing, is refreshed too.

Files
-----

xsfig.py             Shared by the XSecCurves, Airfoils and XSecSkinning figures: samples body sections on a Stack and
                     airfoils on a Wing through the openvsp module, and draws them with ../propeller/figlib.py.
xseccurves_figs.py   The figures, one function per image.
verify.py            Checks the page's statements about the shapes against the openvsp module.

Conventions
-----------

Body sections are drawn from behind, looking forward (Y right, Z up, X toward the reader), with a small axis key.
Every image is figlib.PAGE_W (7 in) wide at 200 dpi with 9 pt text; figlib.fit lays each one out.  Renumber the
page's figures with ../propeller/renumber.py after adding one.

shots.py takes the GUI screenshots for the page into ../../images/ with ../screenshots/snap.py (read its docstring
for VSP_SNAP_EXE and SNAP_CACHE):

    VSP_SNAP_EXE=<vsp> <build>/venv/bin/python shots.py
