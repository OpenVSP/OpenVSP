Human help figures
==================

These scripts draw the figures of ../../Human.md into ../../images/Human_*.png.  Nothing in this folder is
installed (the help CMakeLists installs images/ and builds every *.md, so keep this folder free of .md files).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/human
    <build>/venv/bin/python verify.py            # checks the directions and behavior Human.md states
    <build>/venv/bin/python human_figs.py        # redraws every figure
    <build>/venv/bin/python human_figs.py arm    # or only some (see FIGS at the end)

After redrawing or editing, build the help target (or run pandoc as src/help/CMakeLists.txt does) so
html/Human.html is refreshed too.

Files
-----

human_figs.py   The figures.  Each human is VSP's own, read back through an STL export, in meters.
verify.py       Checks the page's statements against the openvsp module: which way the human faces, the sign of
                every joint angle, units, scale, the sex switch, and the design eye.

The drawing helpers are ../propeller/meshfig.py: triangles read back from VSP, painted with depth sorting, crease
lines, and the 2D axis key, built on figlib.py there (the fixed orthographic projection, figlib.fit, and the one width
and text size of every help image).

Conventions
-----------

As for the propeller figures: 3D views from about (-1, -1, 1) with Z up; 2D views keep the model frame (the front
view looks along +Y, the top view along -Z, the view from upstream along +X) and carry an axis key.  Every image is
figlib.PAGE_W (7 in) wide at 200 dpi with 9 pt text.

The pose figures show the posed human shaded, and the triangles that moved, as they were before, as one translucent
blue layer over it.  Front views move the left side, which faces the reader there; the left angles mean the same
motions as the right.
