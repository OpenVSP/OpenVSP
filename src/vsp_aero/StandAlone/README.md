# CMake Build Instructions for VSPAERO

For the most part, the VSPAERO CMake build process parallels the OpenVSP
CMake build process.  Most users will first build the Libraries project, then
the main project.  The build process should go something like this:


	git clone http://github.com/OpenMDAO/VSPAERO.git
	mkdir build
	cd build
	mkdir Libs
	cd Libs
	cmake -DCMAKE_BUILD_TYPE=Release ../../VSPAERO/StandAlone/Libraries
	make
	cd ..
	mkdir vspaero
	cd vspaero
	cmake -DCMAKE_BUILD_TYPE=Release -DVSPAERO_LIBRARY_PATH=/full/path/to/Libs ../../VSPAERO/StandAlone
	make


##### All project variables:

   - `VSP_NO_GRAPHICS` -- Set this variable to disable everything
     to do with graphics.  The build system will not search for
     or build any graphics libraries.  It will only build the
     headless batch-mode VSP, API, and bindings.  This is ideal
     for building VSP on a HPC machine with limited access.

   - `XXX_OMP_COMPILER` -- Set these variables to point at secondary
     compilers to use when the primary compiler does not support
     OpenMP.  This will allow the VSPAERO solver to be built as
     a multithreaded program.  Expected versions of this variable
     are:

      - `C_OMP_COMPILER`
      - `CXX_OMP_COMPILER`

##### OpenVSP project variables:

   - `VSPAERO_LIBRARY_PATH` -- Set this variable to point at the
     directory where the Libraries project was built.  The Libraries
     project writes a file `VSPAERO_Libraries_Config.cmake`
     containing numerous settings to this path.

   - `XXXX_INSTALL_DIR` -- Set this variable to point to a
     directory where a library has been installed.  This provides
     a suggestion to a corresponding `FIND_PACKAGE( XXXX )`.  Do
     not use these variables with `VSPAERO_LIBRARY_PATH`, it will
     overwrite them.  Acceptable versions of this variable are:

      - `FLTK_INSTALL_DIR`

##### Library project variables:

   - `VSP_USE_SYSTEM_XXXX` -- Set this variable to search for the
     particular library rather than use the bundled versions.
     Acceptable versions of this variable are:

      - `VSP_USE_SYSTEM_FLTK`
