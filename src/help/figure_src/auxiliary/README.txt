Auxiliary help figures
======================

These scripts draw the figures of ../../Auxiliary.md into ../../images/Auxiliary_*.png.  Nothing in this folder is
installed (the help CMakeLists installs images/ and builds every *.md, so keep this folder free of .md files).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/auxiliary
    <build>/venv/bin/python verify.py
    <build>/venv/bin/python auxiliary_figs.py

They use the drawing helpers of ../propeller (figlib.py, and the views, keys and save() of propeller_figs.py), so
every image has the same width (7 in), resolution (200 dpi) and text size (9 pt) as the propeller figures.  Every
shape is VSP's own, sampled with CompVecPnt01 or read from the API.  After redrawing, renumber and render the page:

    <build>/venv/bin/python ../propeller/renumber.py ../../Auxiliary.md

Files
-----

auxiliary_figs.py   The figures: rotor tip path, rotor burst, rotor fragment, off-nominal ground planes and Roll Theta,
                    tire spray (its angles, from behind, and where it starts), wheel and tire failure, an example
                    clearance envelope and its Main Gear Offset, and the Super Cone's profile and its cone on a seated
                    Human; the gear figures hang from the illustration gear of ../gear/gear_figs.py, the Human is
                    read through ../propeller/meshfig.py.
verify.py           Checks the directions Auxiliary.md states, where each bogie state puts the contact point, which
                    bogies throw a center plume and where the plumes start, the sense of Roll Theta, and a Super
                    Cone's Intersect sub-surface, against VSP.
