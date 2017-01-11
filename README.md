# Build and Installation Instructions for OpenVSP


### INTRODUCTION

  OpenVSP is a parametric aircraft geometry tool.  OpenVSP allows the
  user to create a 3D model of an aircraft defined by common engineering
  parameters.  This model can be processed into formats suitable for
  engineering analysis.

  The predecessors to OpenVSP have been developed by JR Gloudemans and
  others for NASA since the early 1990's.  In January 2012, OpenVSP was
  released as an open source project under the NOSA 1.3 license.  The
  first open source version was 2.0.0.

### LICENSE

  OpenVSP is available under the terms of the NASA Open Source Agreement
  (NOSA) version 1.3.  The complete terms of the license are specified
  in the LICENSE file.

### DEPENDENCIES

  OpenVSP relies on a variety of libraries and code made available by
  other authors.  If you are a VSP user, you probably don't need to
  worry about this.  Take this section as informational, providing
  credit where due.

  If you wish to compile VSP or to use the VSP API, then you will need
  to know more.  These dependencies fall into four categories: those
  assumed to be provided by the operating system; those that must be
  downloaded and installed by the developer; those that are bundled
  with OpenVSP, but the developer may want to use a different version
  installed on the system; and those that are bundled with OpenVSP.

  Most users are familiar with the OpenVSP GUI.  OpenVSP can also be
  accessed by other programs via an API, or in a batch mode with no
  user interaction.  These 'headless' targets may be compiled without
  any of the graphics libraries installed.  Although most developers
  will want to build the graphical version, those dependencies only
  needed by the GUI program and graphics-enabled API are marked
  GRAPHICS_ONLY.

##### Assumed provided by the operating system.
  
   - [OpenGL](http://www.opengl.org) - 3D graphics library. OpenGL should be
   available on any platform capable of displaying 3D graphics. GRAPHICS_ONLY

##### Installed by the developer.

   - C++ Compiler -  OpenVSP requires a modern C++ compiler that supports
   some C++11 features.  We try to support popular free compilers on the
   main platforms.  For Windows, we require Visual Studio 2010 Express
   or newer.  For Mac OSX or Linux/BSD, we support LLVM and GCC.

   - [CMake 2.8](http://www.cmake.org) - Cross-platform build system.  CMake
   generates platform-native build files which control compilation of
   OpenVSP.  CMake is available as a standard package on most Linux
   systems and binary installers are available for many other platforms
   including Windows and Mac OS X.

   - [SWIG](http://www.swig.org) - Simplified Wrapper and Interface Generator.
   Optional dependency used to build interface to API for Python
   and other scripting languages.

   - [Python](http://www.python.org) - Scripting language.  Optional dependency
   required if building Python API module.

   - [Doxygen](http://www.doxygen.org) - Documentation generator.  Optional
   dependency used for generating documentation from the source.  Can
   also utilize graphviz dot to generate improved figures in the
   documentation. (http://www.graphviz.org)

##### Bundled with OpenVSP, but system libraries may substitute.

   - [Code-Eli](http://github.com/ddmarshall/Code-Eli) - Curve & surface library.
   Code-Eli was developed by David Marshall to meet OpenVSP's needs.
   It is a header-only library that must be available to build.  It is
   not likely to be packaged on any system.

   - [Eigen3](http://eigen.tuxfamily.org) - Template library for linear algebra.
   This header-only library is required by Code-Eli.  Eigen3 is likely
   to be available as a standard package on Linux.

   - [AngelScript](http://www.angelcode.com/angelscript) - Embedded scripting
   language for C++.  Used for user-defined components.

   - [CppTest](http://cpptest.sourceforge.net) - C++ Unit testing framework.

   - [Libxml2](http://www.xmlsoft.org) - XML parser and toolkit.  Libxml2 most
   likely comes pre-installed with most Linux distributions and 
   Mac OS X systems.  Windows users must download the source and
   build this library following the Libxml2 instructions.

   - [CMinpack](http://devernay.free.fr/hacks/cminpack/) - C/C++ port of
   standard Fortran Levenberg-Marquardt implementation.

   - [FLTK](http://www.fltk.org) - Cross-platform windowing library.  FLTK should
   be available as a standard package on most Linux systems and can
   be installed from source on Windows and Mac OS X according to the
   FLTK instructions.  GRAPHICS_ONLY

   - [libjpeg](http://www.ijg.org) - JPEG compression library.  We typically
   piggy-back on the libjpeg accompanying FLTK.  If you prefer to use
   your own, it is typically pre-installed on Linux.  You will need
   to build it yourself for MacOS or Windows.  GRAPHICS_ONLY

   - [GLM](http://glm.g-truc.net) - OpenGL math library.  GRAPHICS_ONLY

   - [GLEW](http://glew.sourceforge.net) - OpenGL Extension Wrangler Library.  This
   library is used to access any modern OpenGL features.  GRAPHICS_ONLY

   - [libIGES](http://github.com/cbernardo/libIGES) - Library for working with
   IGES files.  This is a very new library and is not likely to be packaged on
   any system.

##### Bundled with OpenVSP.

   - [STEPcode](http://stepcode.org) - Library for working with STEP standard
   files.  Based on an old library developed by NIST.  This is a
   relatively new library, in constant development.  Not likely
   to be packaged on any system.

   - [nanoflann](http://code.google.com/p/nanoflann) - Fast nearest neighbors
   library.

   - sixseries - NASA 6-Series airfoil generation Fortran code released
   to the public domain from NASA TM X 3069 September, 1974.
   Converted to C using F2C.
   
   - [Triangle by Jonathan Shewchuk](http://www.cs.cmu.edu/~quake/triangle.html)
   2D Delaunay triangulator.  This routine has been included in
   this distribution.
   
   - triInt by Tomas Moller [jgt.akpeters.com/papers/Moller97](http://web.archive.org/web/*/http://jgt.akpeters.com/papers/Moller97) - Fast triangle-triangle intersection test.
   This code comes from two sources previously available online.
   When JGT was taken over by Taylor and Francis, the source
   accompanying the articles became unavailable.  The routines are
   included in this distribution.
     - Old URL:
        -[jgt.akpeters.com/papers/Moller97/tritri.html](http://web.archive.org/web/*/http://jgt.akpeters.com/papers/Moller97/tritri.html)
        -[jgt.akpeters.com/papers/MollerTrumbore97/code.html](http://web.archive.org/web/*/http://jgt.akpeters.com/papers/MollerTrumbore97/code.html)
    
      - Tomas Mšller, Ben Trumbore, "Fast, Minimum Storage Ray-Triangle
      Intersection", Journal of Graphics Tools, Vol. 2, Iss. 1, 1997.

      - Tomas Mšller, "A Fast Triangle-Triangle Intersection Test", Journal
      of Graphics Tools, Vol. 2, Iss. 2, 1997.
  
   - glFont2 by Brad Fish [students.cs.byu.edu/~bfish/glfont2.php] (http://web.archive.org/web/*/https://students.cs.byu.edu/~bfish/glfont2.php) - 
   2D OpenGL font tool.  glFont2 has been included in this source
   distribution.  GRAPHICS_ONLY

### BUILD INSTRUCTIONS

   If you want to build OpenVSP on a Debian based Linux computer
   (Debian, Ubuntu, Mint, etc.), step-by-step instructions are
   included on the OpenVSP Wiki here: [Debian Based Build](http://www.openvsp.org/wiki/doku.php?id=ubuntu_instructions).

   All of the supporting libraries and code described above in
   DEPENDENCIES categories 1 and 2 must be properly installed.
   Those in categories 3 and 4 can be satisfied by OpenVSP itself.
   Any dependencies from category 3 that the developer prefers
   to use a system library must be properly installed.

   We only expect two common cases where developers would prefer
   to use the system libraries over the bundled ones.  First, on
   Linux where system libraries are readily available and
   distributions strongly prefer they are used.  Second, when a
   developer is simultaneously working on OpenVSP and the
   dependency in question.

   OpenVSP is packaged for compilation into three CMake projects:
   the main OpenVSP project with all the OpenVSP source and the
   category 4 dependencies; a Libraries project with all the
   category 3 dependencies; a SuperProject that can unify building
   the other two projects.

   Casual developers new to OpenVSP will want to use the
   SuperProject; building it should take care of everything.  More
   involved developers will want to use the OpenVSP and Libraries
   projects directly; the OpenVSP project integrates better than
   the SuperProject with most IDEs.  Developers who want to use
   only system libraries for the category 3 dependencies have no
   need for the Libraries project; they can work only with the
   main project.

   CMake is used to construct the build files for a project.  CMake
   supports out-of-tree builds which keep the source tree free
   of generated files.  Create a build directory to contain the
   generated files, say `openvsp/build`.  Then, change to the
   build directory and run CMake - passing a relative path to
   `openvsp/src`, say `cmake ../src`.

   Once CMake has generated the files for your favorite build
   system, load them in the IDE and compile or launch the build
   from the command line.

   The OpenVSP CMake system can be configured by defining a
   handful of variables.  These variables may be relevant to some
   or all of the projects.

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

   - `VSP_LIBRARY_PATH` -- Set this variable to point at the
     directory where the Libraries project was built.  The
     SuperProject sets this path automatically.  The Libraries
     project writes a file `VSP_Libraries_Config.cmake`
     containing numerous settings to this path.

   - `XXXX_INSTALL_DIR` -- Set this variable to point to a
     directory where a library has been installed.  This provides
     a suggestion to a corresponding `FIND_PACKAGE( XXXX )`.  Do
     not use these variables with `VSP_LIBRARY_PATH`, it will
     overwrite them.  Acceptable versions of this variable are:

      - `FLTK_INSTALL_DIR`
      - `CPPTEST_INSTALL_DIR`
      - `LIBXML2_INSTALL_DIR`
      - `GLM_INSTALL_DIR`
      - `GLEW_INSTALL_DIR`
      - `EIGEN_INSTALL_DIR`
      - `CODEELI_INSTALL_DIR`
      - `CMINPACK_INSTALL_DIR`
      - `LIBIGES_INSTALL_DIR`

##### Libraries & SuperProject project variables:

   - `VSP_USE_SYSTEM_XXXX` -- Set this variable to search for the
     particular library rather than use the bundled versions.
     Acceptable versions of this variable are:

      - `VSP_USE_SYSTEM_CPPTEST`
      - `VSP_USE_SYSTEM_LIBXML2`
      - `VSP_USE_SYSTEM_EIGEN`
      - `VSP_USE_SYSTEM_CODEELI`
      - `VSP_USE_SYSTEM_FLTK`
      - `VSP_USE_SYSTEM_GLM`
      - `VSP_USE_SYSTEM_GLEW`
      - `VSP_USE_SYSTEM_CMINPACK`
      - `VSP_USE_SYSTEM_LIBIGES`
