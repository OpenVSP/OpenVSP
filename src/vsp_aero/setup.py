from setuptools import setup, find_packages
import os
import sys

from distutils.core import Extension as Ext
from Cython.Build import cythonize
import numpy as np

# Note: solverlib.a and adjointlib.a need to be built with the usual `-fPIC` flag.
inc_dirs, lib_dirs, libs, def_macros = [], [], [], []

ADEPT_CXXFLAGS = os.environ.get('ADEPT_CXXFLAGS')
flags = ADEPT_CXXFLAGS.split()
for flag in flags:
    if flag[:2] == '-I':
        inc_dirs.append(flag[2:])
    if flag[:2] == '-D':
        def_macros.append((flag[2:], None))

ADEPT_LDFLAGS = os.environ.get('ADEPT_LDFLAGS')
flags = ADEPT_LDFLAGS.split()
for flag in flags:
    if flag[:2] == '-L':
        lib_dirs.append(flag[2:])
    elif flag[:2] == '-l':
        libs.append(flag[2:])

inc_dirs.append(np.get_include())

VSPAERO_DIR = os.path.abspath(os.path.dirname(__file__))

inc_dirs.append(f"{VSPAERO_DIR}")
lib_dirs.append(f"{VSPAERO_DIR}/Solver")

if sys.platform == 'darwin':
    from distutils import sysconfig
    vars = sysconfig.get_config_vars()
    vars['LDSHARED'] = vars['LDSHARED'].replace('-bundle', '-dynamiclib')

cython_modules = ["optimizer", "functions"]
vspaero_ext_list = []
for module in cython_modules:
    ext = Ext(f"vspaero.{module}", [f"vspaero/{module}.pyx"],
              language="c++",
              include_dirs=inc_dirs, library_dirs=lib_dirs, libraries=libs,
              runtime_library_dirs=lib_dirs,
              define_macros=def_macros,
              extra_objects=[f"{VSPAERO_DIR}/Solver/solverlib.a", f"{VSPAERO_DIR}/Solver/adjointlib.a"],
              extra_compile_args=[],
              extra_link_args=[])
    vspaero_ext_list.append(ext)

optional_dependencies = {
    "testing": ["testflo>=1.4.7"],
    "openvsp": ["openvsp>=3.31.1"],
}

# Add an optional dependency that concatenates all others
optional_dependencies["all"] = sorted(
    [
        dependency
        for dependencies in optional_dependencies.values()
        for dependency in dependencies
    ]
)

setup(name='vspaero',
      version='6.4.6',
      install_requires=['numpy'],
      extras_require=optional_dependencies,
      packages=find_packages(include=['vspaero*']),
      ext_modules=cythonize(vspaero_ext_list))
