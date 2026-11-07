DejaVu Sans, Sans Oblique, Sans Bold and Sans Bold Oblique, version 2.37, embedded as C++ byte arrays so that
OpenVSP draws text without reading a font file at run time.

The fonts come from the DejaVu release

    https://github.com/dejavu-fonts/dejavu-fonts/releases/download/version_2_37/dejavu-fonts-ttf-2.37.tar.bz2

The four fonts, unchanged from the release, are in ttf.  To regenerate the .cpp files from them, run

    python make_font_source.py

The generated sources are committed, so building OpenVSP needs neither Python nor the font files.  LICENSE is
the release's license, unchanged; it covers both the fonts and the generated sources.
