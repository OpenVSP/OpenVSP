Custom component help figures
=============================

shots.py takes the screenshot of the Cone example's Design tab for ../../Custom.md into ../../images/.  It uses
../screenshots/snap.py (read its docstring for VSP_SNAP_EXE and SNAP_CACHE).  Nothing in this folder is installed;
keep it free of .md files (the help build globs *.md).

    cd src/help/figure_src/custom
    VSP_SNAP_EXE=<build>/src/vsp/vsp <build>/venv/bin/python shots.py
