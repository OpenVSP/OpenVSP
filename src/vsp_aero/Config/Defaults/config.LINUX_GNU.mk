PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib

# C and C++ compiler
CC = gcc
CXX = g++

# These are extra CXXFLAGS and LDFLAGS specific to building the code under `Solver/`.
SOLVER_CXXFLAGS = -fPIC -std=c++11 -O3 -funroll-loops -funroll-all-loops -Wno-unused -Wno-format-security -Wno-format-overflow
# Reasonable debug flags
# SOLVER_CXXFLAGS = -fPIC -O0 -g -fsanitize=address
SOLVER_LDFLAGS =
ARFLAGS = rcs

# These are extra CXXFLAGS and LDFLAGS specific to building the viewer.
VIEWER_CXXFLAGS = -O2 -Wall -fno-exceptions
VIEWER_LDFLAGS =

# These are extra CXXFLAGS and LDFLAGS specific to building adb2loads.
ADB2LOADS_CXXFLAGS = -O3 -funroll-loops -funroll-all-loops -Wunused
ADB2LOADS_LDFLAGS =

# `-DVSPAERO_OPENMP` is used in `#ifdefs` in VSPAERO to include OpenMP-related code.
# Actually looks like OpenMP should define `_OPENMP`.
# Should just use that.
OPENMP_CXXFLAGS = -fopenmp -DVSPAERO_OPENMP
OPENMP_LDFLAGS =

# Only `vspaero_adjoint` needs Adept, but Adept doesn't appear to include a
# `*.pc` file for `pkg-config`, so we'll need to set this ourselves.
# Currently Adept needs to be built *without* OpenMP support, i.e. pass the `--disable-openmp` flag to its `configure`.
ADEPT_DIR ?= /usr/local
ADEPT_CXXFLAGS ?= -I$(ADEPT_DIR)/include
# ADEPT_LDFLAGS ?= $(ADEPT_DIR)/lib/libadept.a
ADEPT_LDFLAGS ?= -L$(ADEPT_DIR)/lib -ladept -Wl,-rpath,$(ADEPT_DIR)/lib

# Only `viewer` needs fltk currently.
FLTK_CXXFLAGS ?= $(shell fltk-config --cxxflags)
FLTK_LDFLAGS ?= $(shell fltk-config --use-gl --ldflags)

# Doxygen is needed to compile the docs
DOXYGEN = doxygen

# Needed to install python interface
PIP = pip
