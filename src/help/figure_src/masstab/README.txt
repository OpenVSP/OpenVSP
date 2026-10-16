Mass tab help figures
=====================

masstab_figs.py draws the figures of ../../MassTab.md into ../../images/MassTab_*.png, and verify.py checks what the
page states beyond the mass checks in ../commontabs/verify.py.  Nothing in this folder is installed; keep it free of
.md files (the help build globs *.md).

    cd src/help/figure_src/masstab
    <build>/venv/bin/python masstab_figs.py
    <build>/venv/bin/python verify.py               # prints PASS or FAIL per statement; always exits 0
    <build>/venv/bin/python ../propeller/renumber.py ../../MassTab.md

The drawing primitives and the layout come from ../propeller/figlib.py: every image is 7 in wide at 200 dpi with 9 pt
text.  The front view looks along +Y (X right, Z up) with a small axis key.
