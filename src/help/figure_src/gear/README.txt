Gear help figures
=================

These scripts draw the figures of ../../Gear.md into ../../images/Gear_*.png.  Nothing in this folder is
installed (the help CMakeLists installs images/ and builds every *.md, so keep this folder free of .md files).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/gear
    <build>/venv/bin/python verify.py
    <build>/venv/bin/python gear_figs.py

They use the drawing helpers of ../propeller (figlib.py, and the views, keys and save() of propeller_figs.py), so
every image has the same width (7 in), resolution (200 dpi) and text size (9 pt) as the propeller figures.  Every
shape is VSP's own, sampled with CompVecPnt01 or read from the API.  After redrawing, renumber and render the page:

    <build>/venv/bin/python ../propeller/renumber.py ../../Gear.md

Files
-----

gear_figs.py        The figures: a Gear on its ground plane, tires, bogie layout, the retraction mechanism and its
                    knee axis.
gearmodel.py        A NumPy model of how a bogie retracts by a mechanism, mirroring Bogie::BuildRetractMatrix
                    in GearGeom.cpp, with helpers that sample the Gear's surfaces.
verify.py           Checks gearmodel.py, and the signs and placements Gear.md states, against VSP.
