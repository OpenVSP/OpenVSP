Body help figures: Conformal
=============================

conformal_figs.py draws the figures of ../../Conformal.md into ../../images/: Conformal_offset, _ends, _trims, _wing
and _flap.  It uses the shared helpers in ../fuselagestack/bodyfig.py.  The checks of what the page states are in
../fuselagestack/verify.py and, for the trims around the surface and the hinged flap, in verify.py here.  Nothing in
this folder is installed; keep it free of .md files.

    cd src/help/figure_src/conformal
    <build>/venv/bin/python conformal_figs.py            # every figure, or name some (see FIGS at the end)
    <build>/venv/bin/python verify.py

See ../fuselagestack/README.txt for the conventions.
