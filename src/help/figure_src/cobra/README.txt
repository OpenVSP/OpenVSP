Body help figures: Cobra
=========================

cobra_figs.py draws the figures of ../../Cobra.md into ../../images/: Cobra_views, _sections.  It uses the shared helpers in
../fuselagestack/bodyfig.py, and the checks of what the page states are in ../fuselagestack/verify.py.  Nothing in this
folder is installed; keep it free of .md files.

    cd src/help/figure_src/cobra
    <build>/venv/bin/python cobra_figs.py            # every figure, or name some (see FIGS at the end)

See ../fuselagestack/README.txt for the conventions.
