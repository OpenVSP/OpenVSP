PREFIX ?= $(CONDA_PREFIX)
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib

# C and C++ compiler
CC = clang
CXX = clang++

# These are extra CXXFLAGS and LDFLAGS specific to building the code under `Solver/`.
SOLVER_CXXFLAGS = -fPIC -std=c++11 -O3 -funroll-loops -Wno-unused -Wno-format-security -Wno-error=non-pod-varargs
SOLVER_LDFLAGS =
ARFLAGS = rcs

# These are extra CXXFLAGS and LDFLAGS specific to building the viewer.
VIEWER_CXXFLAGS = -O2 -Wall -fno-exceptions 
VIEWER_LDFLAGS =

# These are extra CXXFLAGS and LDFLAGS specific to building adb2loads.
ADB2LOADS_CXXFLAGS = -O3 -funroll-loops -Wunused
ADB2LOADS_LDFLAGS =

# `-DVSPAERO_OPENMP` is used in `#ifdefs` in VSPAERO to include OpenMP-related code.
# Actually looks like OpenMP should define `_OPENMP`.
# Should just use that.
OPENMP_DIR ?= $(CONDA_PREFIX)
OPENMP_CXXFLAGS = -I$(OPENMP_DIR)/include -fopenmp -DVSPAERO_OPENMP
OPENMP_LDFLAGS = -L$(OPENMP_DIR)/lib/ -lomp -Wl,-rpath,$(OPENMP_DIR)/lib

# Only `vspaero_adjoint` needs Adept, but Adept doesn't appear to include a
# `*.pc` file for `pkg-config`, so we'll need to set this ourselves.
# Currently Adept needs to be built *without* OpenMP support, i.e. pass the `--disable-openmp` flag to its `configure`.
ADEPT_DIR ?= $(CONDA_PREFIX)
ADEPT_CXXFLAGS ?= -I$(ADEPT_DIR)/include
ADEPT_LDFLAGS ?= -L$(ADEPT_DIR)/lib -ladept -Wl,-rpath,$(ADEPT_DIR)/lib

# Only `viewer` needs fltk currently.
FLTK_DIR ?= $(CONDA_PREFIX)
FLTK_CXXFLAGS ?= -I$(FLTK_DIR)/include
FLTK_LDFLAGS ?= -L$(FLTK_DIR)/lib/ -lfltk -lfltk_gl -framework OpenGL -Wl,-rpath,$(FLTK_DIR)/lib

# Doxygen is needed to compile the docs
DOXYGEN = doxygen

# Needed to install python interface
PIP = pip
