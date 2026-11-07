DejaVu Sans, Sans Oblique, Sans Bold and Sans Bold Oblique, version 2.37, embedded as C++ byte arrays so that
OpenVSP draws text without reading a font file at run time.

The fonts come from the DejaVu release

    https://github.com/dejavu-fonts/dejavu-fonts/releases/download/version_2_37/dejavu-fonts-ttf-2.37.tar.bz2

To regenerate the .cpp files, unpack the release and run

    python make_font_source.py dejavu-fonts-ttf-2.37/ttf

The generated sources are committed, so building OpenVSP needs neither Python nor the font files.  LICENSE is
the release's license, unchanged.
