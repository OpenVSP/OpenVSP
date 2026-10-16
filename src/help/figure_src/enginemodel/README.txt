Body help figures: EngineModel
===============================

enginemodel_figs.py draws the figures of ../../EngineModel.md into ../../images/: EngineModel_types, _stations,
_flowthrough and _ends.  It uses the shared helpers in ../fuselagestack/bodyfig.py, and the checks of what the page
states are in ../fuselagestack/verify.py.  Nothing in this folder is installed; keep it free of .md files.

    cd src/help/figure_src/enginemodel
    <build>/venv/bin/python enginemodel_figs.py            # every figure, or name some (see FIGS at the end)

See ../fuselagestack/README.txt for the conventions.

shots.py takes the GUI screenshots for the page into ../../images/ with ../screenshots/snap.py (read its docstring
for VSP_SNAP_EXE and SNAP_CACHE):

    VSP_SNAP_EXE=<vsp> <build>/venv/bin/python shots.py
