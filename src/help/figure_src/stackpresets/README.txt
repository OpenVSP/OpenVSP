Stack preset help figures
=========================

These scripts draw the figures of ../../StackPresets.md into ../../images/StackPresets_<preset>.png, one per Stack
preset, and check what the page states.  Nothing in this folder is installed (the help build globs *.md, so keep it
free of .md files).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/stackpresets
    <build>/venv/bin/python verify.py                         # checks StackPresets.md against VSP
    <build>/venv/bin/python stackpresets_figs.py              # redraws every figure
    <build>/venv/bin/python stackpresets_figs.py flowpath_in  # or only some (the keys of PRESETS)

After redrawing or editing the page, build the help target (or run pandoc as src/help/CMakeLists.txt does) so that
html/StackPresets.html is refreshed.  ../propeller/renumber.py ../../StackPresets.md renumbers the page's figures.

Files
-----

stackpresets_figs.py  One figure per preset.  Each preset is built with vsp.InitStackPreset( stack_id, preset ) and
                      Update, as the Set button does, and drawn cut through its axis with the body helpers of
                      ../fuselagestack/bodyfig.py (on ../propeller/figlib.py), both imported read-only.
verify.py             Checks each preset's sections, XSec Order, caps, engine definition, stations, the surfaces
                      it builds and their normals; that a new Stack is the Default; the UW attachment frame; what
                      each representation builds from the flowpath presets; and that a preset keeps a station
                      given by U.

Conventions
-----------

Every image is figlib.PAGE_W (7 in) wide at 200 dpi with figlib.FS (9 pt) text, laid out by figlib.fit at one scale
(SCALE, inches per model unit) for all presets.  The view looks along +Y (X right, Z up) and carries an axis key.
The surfaces built are cut by Y = 0 (W 0.75 above, W 0.25 below) and filled, positive grey, negative red and hatched.
Sections, the order arrowheads, the normal and the stations are taken from the sections' own surface, found with
the engine model and the end caps turned off for the moment.
