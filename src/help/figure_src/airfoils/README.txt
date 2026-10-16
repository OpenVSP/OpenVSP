Airfoil help figures
====================

These scripts draw the figures of ../../Airfoils.md into ../../images/Airfoils_*.png.  Nothing in this folder is
installed; keep it free of .md files.

Run them with the Python of an OpenVSP build:

    cd src/help/figure_src/airfoils
    <build>/venv/bin/python verify.py            # checks what the page says against VSP
    <build>/venv/bin/python airfoils_figs.py     # redraws every figure
    <build>/venv/bin/python airfoils_figs.py caps flap      # or only some (see FIGS at the end)

Then refresh html/Airfoils.html with pandoc, as src/help/CMakeLists.txt does.

Files
-----

airfoils_figs.py   The figures.  Every airfoil is the root section of a Wing, sampled after all of the Modify tab's
                   operations, in chords, seen from the side (X right, Z up).  Helpers come from
                   ../xseccurves/xsfig.py.
verify.py          Checks the page against the openvsp module.  Its last two checks record behaviour that looks
                   unintended (Fit CST replacing Theta; a propeller section's flap having no effect); they pass while
                   that behaviour lasts.

shots.py takes the GUI screenshots for the page into ../../images/ with ../screenshots/snap.py (read its docstring
for VSP_SNAP_EXE and SNAP_CACHE):

    VSP_SNAP_EXE=<vsp> <build>/venv/bin/python shots.py
