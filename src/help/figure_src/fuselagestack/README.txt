Body help figures: Fuselage and Stack
=====================================

These scripts draw the figures of ../../FuselageStack.md into ../../images/FuselageStack_*.png.  This folder also holds
the helpers and the checks shared by the other body pages (EngineModel, Chevron, Pod, BOR, Cobra, Conformal), whose
figure scripts live in their own folders beside this one and import bodyfig.py from here.  Nothing in these folders is
installed (the help build globs *.md, so keep them free of .md files).

Run them with the Python of an OpenVSP build, so that `import openvsp` finds that build:

    cd src/help/figure_src/fuselagestack
    <build>/venv/bin/python verify.py                  # checks what all eight body pages state, against VSP
    <build>/venv/bin/python fuselagestack_figs.py      # redraws this page's figures
    <build>/venv/bin/python fuselagestack_figs.py loop # or only some (see FIGS at the end)

After redrawing or editing a page, build the help target (or run pandoc as src/help/CMakeLists.txt does) so its html
copy is refreshed.  ../propeller/renumber.py <page> renumbers a page's figures.

Files
-----

bodyfig.py              Views, layout and drawing helpers on top of ../propeller/figlib.py (imported read-only), and
                        sampling of VSP surfaces with CompPnt01.
fuselagestack_figs.py   FuselageStack_place, _rotate, _valign, _loop, _caps.
verify.py               Checks the signs, defaults and measured numbers the body pages state, and the Gen tab
                        page's advice on Negative Volume cutters.

Conventions
-----------

Every image is figlib.PAGE_W (7 in) wide at 200 dpi with figlib.FS (9 pt) text, laid out by figlib.fit.  2D views keep
OpenVSP's frame and carry an axis key: the side view looks along +Y (X right, Z up), the top view down -Z (X right, Y
up), the front view aft along +X from ahead (-Y right, Z up).  3D views look from about (-1, -1, 1) with Z up.  A body
"cut through its axis" is the surface's meridians at W 0.75 (top) and W 0.25 (bottom), filled.

shots.py takes the GUI screenshots for the page into ../../images/ with ../screenshots/snap.py (read its docstring
for VSP_SNAP_EXE and SNAP_CACHE):

    VSP_SNAP_EXE=<vsp> <build>/venv/bin/python shots.py
