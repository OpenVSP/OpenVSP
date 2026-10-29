MdcFin help figures
===================

mdc_fin_figs.py draws the figures of ../../MdcFin.md into ../../images/: MdcFin_overview, _panels, _planform, _sta,
_flaps, _sections, _naca.  It uses the helpers of ../mdc_bod/datcomfig.py, and the checks of what the page states are in
../mdc_bod/verify.py.  Nothing in this folder is installed; keep it free of .md files.

    cd src/help/figure_src/mdc_fin
    <build>/venv/bin/python mdc_fin_figs.py            # every figure, or name some (see FIGS at the end)

A one panel fin set rolled to PHIF 90 lies in the XY plane, so its planform is the top view and its section is the side
view; the planform and section figures use one.

See ../mdc_bod/README.txt.
