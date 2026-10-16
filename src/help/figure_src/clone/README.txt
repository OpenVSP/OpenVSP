Clone help figures
==================

These scripts draw the figures of ../../Clone.md into ../../images/Clone_*.png.  Nothing in this folder is installed
(the help CMakeLists installs images/ and builds every *.md, so keep this folder free of .md files).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/clone
    <build>/venv/bin/python verify.py
    <build>/venv/bin/python clone_figs.py
    VSP_SNAP_EXE=<vsp> <build>/venv/bin/python shots.py   # the GUI screenshot (see ../screenshots/snap.py)

Files
-----

clone_figs.py       The figures.
clonemodel.py       Nacelles and their Clones made through the openvsp module, for the figures and checks.
verify.py           Checks the statements Clone.md makes about a Clone's copied symmetry against VSP.
shots.py            The Geom Browser screenshot, Clone_browser.png.  Its model also gives Hinge.md its Geom
                    Browser crop (../hinge/shots.py).
