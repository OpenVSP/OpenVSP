Wireframe help figures
======================

These scripts draw the figures of ../../Wire.md into ../../images/Wire_*.png.  Nothing in this folder is installed.

    cd src/help/figure_src/wire
    <build>/venv/bin/python verify.py          # checks the patches, the order of the controls and import
    <build>/venv/bin/python wire_figs.py       # redraws the figures

A small wing grid is written as an XSec (.hrm) file, imported with ImportFile( IMPORT_XSEC_WIRE ), patched, and
read back through an XSec export of the wireframe, which writes the grid as the component has rearranged it.  The
drawing helpers are ../propeller/meshfig.py, which builds on ../propeller/figlib.py.
