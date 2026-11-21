OpenVSP for POGS help figures
=============================

These scripts make the figures of ../../POGS.md into ../../images/POGS_*.png and check the page's statements.  Nothing
in this folder is installed; keep it free of .md files (the help build turns every *.md into a page).  They use the
models and the screenshot code of ../cfdmesh (see its README).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/pogs
    <build>/venv/bin/python verify.py               # checks the files, counts and parameters the page states
    <build>/venv/bin/python meshshots.py            # Figures 3 and 4, the faces with Split and Join Surfaces off and on
    <build>/venv/bin/python pogs_figs.py            # Figures 5 and 6, from the .uvin file: Num Ref. and blanking
    VSP_SNAP_EXE=<build>/src/vsp/vsp SNAP_CACHE=<folder> <build>/venv/bin/python shots.py   # the tab screenshots

shots.py dumps the CFD Mesh screen once with the model set up as Getting Started says (Max Edge Len, Min Edge Len and
Max Gap relative at 0.01, Num Circle Segments 90, the Set All, Split and Join Surfaces on, the POGS files the only
output) and crops the Global and Output tabs; give SNAP_CACHE a folder ending in Documents/Projects so the file
names on the Output tab read .../Projects/MyPlane/model.*.

meshshots.py meshes the aircraft of pogsmodel.py (the small aircraft with a blunt trailing edge on its wing) each way
before starting the GUI, and draws only the intersection and border curves, with the mesh, the face colors and Show
Patch Join Curves off.  Each figure is a pair of grabs at one view and zoom, set through the view Parms (FRAMES).

verify.py also runs the scripting example on the page, and measures the Shown-with-Rel trap the page warns of.

After redrawing or editing, build the help target (or run pandoc as src/help/CMakeLists.txt does) so that
html/POGS.html is refreshed too.
