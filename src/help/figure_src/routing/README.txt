Routing help figures
====================

These scripts draw the figures of ../../Routing.md into ../../images/Routing_*.png.  Nothing in this folder is
installed (the help CMakeLists installs images/ and builds every *.md, so keep this folder free of .md files).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/routing
    <build>/venv/bin/python verify.py
    <build>/venv/bin/python routing_figs.py

They use the drawing helpers of ../propeller (figlib.py, and the views, keys and save() of propeller_figs.py), so
every image has the same width (7 in), resolution (200 dpi) and text size (9 pt) as the propeller figures.  Every
shape is VSP's own, sampled with CompVecPnt01 or read from the API.  After redrawing, renumber and render the page:

    <build>/venv/bin/python ../propeller/renumber.py ../../Routing.md

Files
-----

routing_figs.py     The figure of a route over a pod and a wing.
verify.py           Checks what Routing.md states about rounded corners, offsets and following components against VSP.
shots.py            The GUI screenshot of the Design tab.  Run with VSP_SNAP_EXE set; see ../screenshots/snap.py.
