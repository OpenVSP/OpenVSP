CFD Mesh help figures
=====================

These scripts draw the figures of ../../CFDMesh.md into ../../images/CFDMesh_*.png and .svg and check the page's numbers.
Nothing in this folder is installed; keep it free of .md files (the help build turns every *.md into a page).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/cfdmesh
    <build>/venv/bin/python verify.py               # checks the numbers and directions the page states
    <build>/venv/bin/python cfdmesh_figs.py         # redraws the charts, diagrams and the two Python close-ups
    <build>/venv/bin/python meshshots.py            # the pictures of meshes, taken in OpenVSP's own window
    <build>/venv/bin/python meshshots.py puck       # or only some (see FIGS and MAIN at the ends)
    VSP_SNAP_EXE=<build>/src/vsp/vsp SNAP_CACHE=<folder> <build>/venv/bin/python shots.py   # the screenshots

shots.py dumps every tab of the CFD Mesh screen once (see ../screenshots/snap.py) and crops each tab's controls.  The
Output tab shows file names made from the folder the dumped model is in, so give SNAP_CACHE a folder whose path reads
well; the images were taken with SNAP_CACHE ending in Documents/Projects, so the files read
.../Projects/MyPlane/model.*.

meshshots.py takes the pictures of meshes as OpenVSP draws them, through the openvsp module's GUI facade (the build
must have the GUI): each picture is made in a process of its own, which builds and meshes the model, then starts the
GUI, opens the CFD Mesh screen (it loads the mesh's draw objects), sets the display and grabs the main window, as SVG
(VectorScreenGrab) for whole views, or as a large PNG (ScreenGrab) that is cropped and scaled for close-ups and side
by side pairs.  The model is meshed before the GUI starts.

The trailing edge strip (drawn shaded, with the strip in its own color) and the symmetry plane over its background
maps are drawn in Python from the .tri file CFD Mesh wrote, since OpenVSP's window cannot show those.  Close-ups taken
in OpenVSP are zoomed through the Vehicle's view Parms (group AdjustView: CORX, CORY, CORZ, PanX, PanY, Zoom and the
rotations), the Parms of the Adjust View screen, after the view is fitted; a zoomed view fills the window, so it is
grabbed without autocrop and the pair keeps one frame.  The two charts draw the target length rules of the
mesher (Surf::TargetLen and PointSimpleSource::GetTargetLen), with the median edge lengths of meshes VSP made marked
on them.  The charts use the colors of the Display tab's reason key.

After redrawing or editing, build the help target (or run pandoc as src/help/CMakeLists.txt does) so that
html/CFDMesh.html is refreshed too.

Files
-----

cfdmodel.py       The illustration models (a small aircraft) and readers for the meshes CFD Mesh writes.
meshshots.py      The pictures of meshes taken in OpenVSP.
cfdmesh_figs.py   The charts and diagrams.  Uses ../propeller/figlib.py and meshfig.py for the fixed 3D views and layout.
shots.py          The screenshots of the CFD Mesh screen's tabs.
verify.py         Checks the page's claims against VSP: defaults, the curvature rules on spheres, the width limit,
                  sources, the far field box, half mesh, wakes, quads, sets and the CFDMesh Result.
