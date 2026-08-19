# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
sys.path.insert(0, os.path.abspath('..'))


# -- Project information -----------------------------------------------------

project = 'OpenVSP Python API'
copyright = 'NASA Open Source Agreement (NOSA) version 1.3'
author = 'OpenVSP Development Team'

# The full version, including alpha/beta/rc tags

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.autodoc',
    # The group pages lead with a summary table of the functions they hold, which is what makes a
    # group of eighty functions scannable.
    'sphinx.ext.autosummary',
    'sphinx.ext.viewcode',
]

# The tables are written out by gen_api_docs.py, so Sphinx does not need to generate stub pages.
autosummary_generate = False

# Keep the functions in the order the API header declares them rather than alphabetically, so
# related calls stay together.
autodoc_member_order = 'bysource'


# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store','conda-recipe','.in','.exe','__init__.py','.rst', 'gen_unit_test.py']


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
# 'classic' has no persistent sidebar, which makes forty group pages hard to move between.  Fall
# back to it if the better theme is not installed, so the documentation still builds.
try:
    import sphinx_rtd_theme  # noqa: F401
    html_theme = 'sphinx_rtd_theme'
except ImportError:
    html_theme = 'classic'


# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = []