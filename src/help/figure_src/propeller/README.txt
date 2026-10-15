Propeller help figures
======================

These scripts draw the figures of ../../Propeller.md into ../../images/Propeller_*.png.  They are kept here so the
figures can be maintained; nothing in this folder is installed (the help CMakeLists installs images/ and builds every
*.md, so keep this folder free of .md files).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/propeller
    <build>/venv/bin/python verify.py               # checks propmodel.py, and the signs the page states, against VSP
    <build>/venv/bin/python propeller_figs.py       # redraws every figure
    <build>/venv/bin/python propeller_figs.py hinges chain     # or only some groups (see FIGS at the end)

The page builds from Propeller.md with pandoc as part of the help target.  After redrawing or editing, build that
target (or run pandoc as src/help/CMakeLists.txt does) so html/Propeller.html, the copy used when pandoc is missing,
is refreshed too.

The images carry no matplotlib version tag, so an unchanged picture is an unchanged file.  They were last drawn with
matplotlib 3.11.2 and numpy 2.5.4; another matplotlib may render text slightly differently.

Files
-----

propeller_figs.py   The figures, one function per image or group.  Each 3D figure uses VSP's own blades
                    (CompPnt01 on VSP's tessellation stations) and checks them against propmodel.py.
figparts.py         Illustration parms and small drawing helpers shared by the figures.
figlib.py           A fixed orthographic projection drawn on 2D matplotlib axes, and the drawing primitives.
meshfig.py          Helpers on figlib for the pages that draw other components: Geoms read back as triangles
                    through an STL export, painted with depth sorting, and the 2D axis key.
geom.py             Makes illustration propellers through the openvsp module and samples their surfaces.
propmodel.py        A NumPy model of how PropGeom shapes and places a blade, mirroring PropGeom.cpp.
verify.py           Checks propmodel.py against the openvsp module.
renumber.py         Renumbers the page's figures in page order, and every "Figure N" reference to them:
                        python renumber.py ../../Propeller.md
                    A new figure may be added without a number; renumber.py numbers it and lists any figure the
                    text does not yet refer to.

Conventions
-----------

3D views are OpenVSP's default: Z up, Y into the page, X to the right, seen from about (-1, -1, 1).  A rotor is
drawn in helicopter position, turned by Y Rotation 90.  Hinges use the colors the 3D view draws them in.  The hinge
motion figures share one canvas, scale and frame (hinge_frame), so they show at one scale on the page; the three
hinge axis figures are a closer view of the hinge point, at one scale among themselves.

Every image is figlib.PAGE_W (7 in) wide at 200 dpi, and all text is figlib.FS (9 pt).  The page fits every image to
its column, so one width means one text size on the page.  figlib.fit sizes a figure: it lays out rows of panels at
one scale and keeps every label inside the image, so place labels with offsets in points and let fit find room.  Do
not crop a figure (bbox_inches="tight") or give it another width.
