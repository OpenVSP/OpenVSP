Point Cloud help figures
========================

These scripts draw the figures of ../../PtCloud.md into ../../images/PtCloud_*.png.  Nothing in this folder is
installed (the help CMakeLists installs images/ and builds every *.md, so keep this folder free of .md files).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/ptcloud
    <build>/venv/bin/python verify.py            # checks the statements PtCloud.md makes about projection
    <build>/venv/bin/python ptcloud_figs.py      # redraws every figure

After redrawing or editing, build the help target (or run pandoc as src/help/CMakeLists.txt does) so
html/PtCloud.html is refreshed too.

Files
-----

ptcloud_figs.py   The figures.  The wing is VSP's own, sampled with CompPnt01; the projected points are read back
                  from the cloud.
cloudmodel.py     The wing and the cloud above it, made through the openvsp module, shared by the figures and checks.
verify.py         Checks, against VSP, that a projection moves points along the axis only, to the nearest crossing,
                  leaves a point that misses where it was, and uses one surface at a time.

The drawing helpers are ../propeller/meshfig.py, which builds on ../propeller/figlib.py: every image is figlib.PAGE_W
(7 in) wide at 200 dpi with 9 pt text, and 3D views have Z up, Y into the page and X to the right, seen from about
(-1, -1, 1).
