Hinge help figures
==================

These scripts draw the figures of ../../Hinge.md into ../../images/Hinge_*.png.  Nothing in this folder is
installed (the help CMakeLists installs images/ and builds every *.md, so keep this folder free of .md files).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/hinge
    <build>/venv/bin/python verify.py
    <build>/venv/bin/python hinge_figs.py

They use the drawing helpers of ../propeller (figlib.py, and the views, keys and save() of propeller_figs.py), so
every image has the same width (7 in), resolution (200 dpi) and text size (9 pt) as the propeller figures.  Every
shape is VSP's own, sampled with CompVecPnt01 or read from the API.  After redrawing, renumber and render the page:

    <build>/venv/bin/python ../propeller/renumber.py ../../Hinge.md

Files
-----

hinge_figs.py       The figures: a wing panel on a Hinge (motion), a Hinge oriented by vectors (orient), and an
                    aileron Hinge on a folding wing tip (chain).  hinge_figs.py <name> redraws only that one.
verify.py           Checks the signs and frames Hinge.md states against VSP.
shots.py            The GUI screenshots: the Motion and Orient tabs, and the Geom Browser (the model of
                    ../clone/shots.py).  Run with VSP_SNAP_EXE set; see ../screenshots/snap.py.
