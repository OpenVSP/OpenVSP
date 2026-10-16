Wing help figures
=================

These scripts draw the figures of ../../Wing.md into ../../images/Wing_*.png.  Nothing in this folder is installed
(the help CMakeLists installs images/ and builds every *.md, so keep this folder free of .md files).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/wing
    <build>/venv/bin/python verify.py               # checks the statements Wing.md makes against VSP
    <build>/venv/bin/python wing_figs.py            # redraws every figure
    <build>/venv/bin/python wing_figs.py caps twist # or only some (see FIGS at the end of wing_figs.py)
    VSP_SNAP_EXE=<vsp> <build>/venv/bin/python shots.py   # retakes the GUI screenshots (see ../screenshots/snap.py)

The drawing style comes from ../propeller/figlib.py, imported read-only: every image is figlib.PAGE_W (7 in) wide at
200 dpi with all text at figlib.FS (9 pt), laid out by figlib.fit.  Do not crop a figure or give it another width.

After redrawing or editing, rebuild the help target (or run pandoc as src/help/CMakeLists.txt does) so that
html/Wing.html, the copy used when pandoc is missing, is refreshed too.

Files
-----

wing_figs.py    The figures, one function per image.  Every surface is VSP's own, sampled with CompPnt01.
wingmodel.py    Makes illustration wings through the openvsp module and samples their surfaces and airfoils.
verify.py       Checks, against VSP: section relations, drivers, sweep and its location, dihedral and projected
                span, airfoil rotation and thickness correction, twist sign and reference, totals and MAC,
                blending angle conventions, split, insert, ruled lofting, flaps, and extreme settings; the
                neighbor of a shared-chord change, the 39 accepted driver sets, the Plan and Sect tab airfoil
                rotation controls, Incidence against Y Rotation, eta (and a propeller's r/R), and the fillet and
                winglet blending example.  The 17 "Invalid wing drivers" errors it prints are the refused driver
                sets it tries on purpose.

Conventions
-----------

3D views have Z up, Y into the page and X to the right, seen from about (-1, -1, 1).  2D views keep that frame: the
top view looks down -Z (X right, Y up), the side view along +Y (X right, Z up), the view from upstream along +X
(-Y right, Z up); each carries a small axis key.  Airfoils are drawn red; blend vectors in the fuchsia the 3D view
uses for them, the inboard ones dashed as there.
