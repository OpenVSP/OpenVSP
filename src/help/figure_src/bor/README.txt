Body help figures: BOR
=======================

bor_figs.py draws the figures of ../../BOR.md into ../../images/: BOR_flowthrough, _areas, _upperlower.  It uses the
shared helpers in ../fuselagestack/bodyfig.py, and the checks of what the page states are in
../fuselagestack/verify.py.  Nothing in this folder is installed; keep it free of .md files.

    cd src/help/figure_src/bor
    <build>/venv/bin/python bor_figs.py            # every figure, or name some (see FIGS at the end)

See ../fuselagestack/README.txt for the conventions.
