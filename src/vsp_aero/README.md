[![Build and unit tests](https://github.com/OpenMDAO/VSPAERO/actions/workflows/unit_tests.yml/badge.svg)](https://github.com/OpenMDAO/VSPAERO/actions/workflows/unit_tests.yml)
# VSPAERO

## Docs 

* Code documentation can be locally compiled by using the following command:

    ```
    $ make docs
    ```

* Online documentation is located at [https://OpenMDAO.github.io/VSPAero/](https://OpenMDAO.github.io/VSPAero/)

## Prerequisites

  * A C++ compiler (duh).
    On linux, `g++` version `11.3.0` seems to work fine.
    On MacOS,  `clang++` version `12.0.0` seems to work.
    You can change that in `config.mk` or pass a different value on the command line:

      ```
      $ make CXX=clang++
      ```

  * The [Adept library](http://www.met.reading.ac.uk/clouds/adept/), built *without* OpenMP support (i.e., `./configure --disable-openmp`).
    Only needed for `vspaero_adjoint` and `vspaero_opt`.
    The latest version of Adept at the time of writing is `2.1.1`, which seems to work fine.

  * [FLTK](https://www.fltk.org/) (only needed for the Viewer): I'm using `fltk 1.3.5`.

## Build and Install

* To start, find a configuration file close to your current setup in:

    ```
    $ Config/Defaults
    ```
  
    and copy it to ‘’config/config.mk’’. For example:

    ```
    $ cp config/defaults/config.LINUX_GPLUSPLUS.mk config/config.mk
    ```

* Edit the ``config.mk`` file to match your system.
  In particular, adjust the value of the variables `ADEPT_DIR`, or `ADEPT_CXXFLAGS` and `ADEPT_LDFLAGS` to allow the C++ compiler to find your installation of Adept.
  Or you can pass the values you'd like on the command line.
  For example:

    ```
    $ make ADEPT_DIR=/foo/bar/adept/2.1.1 all
    ```

  assuming that you've put Adept in `/foo/bar/adept/2.1.1`.

* To build everything, just call make:

    ```
    $ make
    ```

* To install the binaries, use the `install` target in the makefile:

    ```
    $ make install PREFIX=$HOME/foo/bar/vspaero/whatever
    ```

* To run the test cases, use the `test` target in the makefile:

    ```
    $ make test
    ```

  This currently just runs the two test cases and checks the error codes—it doesn't actually check the VSPAERO solution.

## Explanation of the variables in `config.mk`
* `SOLVER_CXXFLAGS`, `SOLVER_LDFLAGS`: Extra flags used during compilation and linking, respectively, when building stuff in the `Solver/` directory (currently `vspaero`, `vspaero_adjoint`, `vspaero_complex`, `vspaero_opt`, and `solverlib.a` and `adjointlib.a`).
  These variables may be empty, but might include things like `-O3`, `-Wall`, etc..
* `VIEWER_CXXFLAGS`, `VIEWER_LDFLAGS`: Extra flags used during compilation and linking, respectively, when building stuff in the `Viewer/` directory (currently just `viewer`).
  These variables may be empty, but might include things like `-O3`, `-Wall`, etc..
* `ADB2LOADS_CXXFLAGS`, `ADB2LOADS_LDFLAGS`: Extra flags used during compilation and linking, respectively, when building stuff in the `Adb2Load` directory (currently just `ad2loads`).
  These variables may be empty, but might include things like `-O3`, `-Wall`, etc..
* `OPENMP_CXXFLAGS`, `OPENMP_LDFLAGS`: Flags needed for compiling and linking code that uses OpenMP.
  Leave them empty if you don't want OpenMP support.
  Currently `vspaero`, `vspaero_complex` and `vspaero_opt` can use OpenMP.
* `ADEPT_CXXFLAGS`, `ADEPT_LDFLAGS`: Flags needed for compiling and linking code with the Adept AD library.
  Currently `vspaero_adjoint` and `vspaero_opt` use Adept.
  (`ADEPT_DIR` isn't used in the `Makefiles`—it's just for contently setting `ADEPT_CXXFLAGS` AND `ADEPT_LDFLAGS`.)
* `FLTK_CXXFLAGS`, `FLTK_LDFLAGS`: Flags needed for compiling and linking code that uses the FLTK library.
  Currently only needed for `viewer`.
  These are set using `pkg-config` by default.
