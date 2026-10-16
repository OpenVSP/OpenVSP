Common component tabs help figures
==================================

These scripts draw the figures of ../../XFormTab.md and ../../SubTab.md into ../../images/XFormTab_*.png and
../../images/SubTab_*.png, and check the statements of the four common tab pages (GenTab.md, XFormTab.md, MassTab.md
and SubTab.md) against OpenVSP.  Nothing in this folder is installed; keep it free of .md files (the help build globs
*.md).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/commontabs
    <build>/venv/bin/python verify.py                    # checks every sign, direction and rule the pages state
    <build>/venv/bin/python commontabs_figs.py           # redraws every figure
    <build>/venv/bin/python commontabs_figs.py ss_uw     # or only some (see FIGS at the end)
    <build>/venv/bin/python subtab_figs.py               # the further Sub tab figures

Then renumber each page's figures and rebuild its html:

    <build>/venv/bin/python ../propeller/renumber.py ../../XFormTab.md
    <build>/venv/bin/python ../propeller/renumber.py ../../SubTab.md

Files
-----

commontabs_figs.py  The figures, one function per image.  Surfaces are sampled with CompPnt01; attachment frames and
                    points are read back from Blank children attached to the parent; sub-surface outlines come from
                    a DegenGeom CSV file.  Each figure asserts that what it draws agrees with VSP.
subtab_figs.py      Further SubTab.md figures: Ellipse polygons (SubTab_ss_polygon) and the positive deflection of a
                    mirrored control surface pair (SubTab_ss_hinge), read from DegenGeom outlines and hinge lines.
verify.py           Checks the pages against the openvsp module: rotation order, the rotation origin and Abs/Rel,
                    attachment frames and coordinates, flip and symmetry frames, scale, mass, tessellation rounding
                    and sub-surface tagging.  Its analyses write to a temporary folder.

Conventions
-----------

The drawing primitives and the layout come from ../propeller/figlib.py (put on sys.path): every image is 7 in wide
at 200 dpi with 9 pt text.  3D views are Z up, Y into the page, X to the right, seen from about (-1, -1, 1); the top
view looks along -Z (X right, Y up) and the front view along +Y (X right, Z up), each with a small axis key.
Attachment and symmetry frames are drawn in purple, sub-surfaces in vermillion.
