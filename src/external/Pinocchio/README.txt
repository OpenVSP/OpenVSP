This is the source release of the Pinocchio library.  The full
technical description is given in Ilya Baran and Jovan Popovic,
"Automatic Rigging and Animation of 3D Characters," SIGGRAPH 2007.

---------
LICENSING
---------

The library itself is in the Pinocchio subdirectory and is distributed
under the GNU LGPL.  A simple command-line demonstration tool is in
the DemoUI subdirectory and is distributed under the MIT license.  If
you use the library in research or a product, I would welcome an
acknowledgement of this work, although this is not necessary.

--------
BUILDING
--------

The DemoUI requires the FLTK library (I used 1.1.7, I imagine 1.1.x
should work).  You should have it installed with OpenGL support.

For Linux and other Unix, just run make.  If it doesn't work, edit the
makefiles--they are simple.

Under Windows, set the FLTKDIR environment variable appropriately so
that the program can find the FLTK headers.  Run Visual Studio 2005
and build the solution.

--------------
RUNNING DemoUI
--------------

Under Linux, make sure LD_LIBRARY_PATH is set so that DemoUI can see
libpinocchio.so.

First try running with the given objects:
DemoUI data/test.obj -motion data/walk.txt

After about 10 seconds, a window should open and a guy should walk
around in it.

In the window, use the left mouse button to pan, wheel to zoom, and
right mouse button to rotate the view.  Pressing S toggles the skeleton
display, F toggles flat shading, G toggles ground display, and T resets
the view.

Then try using your own meshes--make sure they are nice, closed, and
connected.  Igarashi's Teddy (and Fibermesh) can easily produce such
obj files.  Pinocchio can also read text files in .off, .ply and .stl
formats.

If the mesh is not oriented correctly (should be upright and facing
you), the "-rot x y z degrees" option lets you apply rotations.
Running with the -mo option only displays the mesh and doesn't
do any analysis.

When DemoUI runs, it also outputs the embedded skeleton to
skeleton.out and the attachment (bone weights) to attachment.out.
Each line in skeleton.out corresponds to a joint and is of the form:
idx x y z prev
where prev is the index of the previous joint.
Each line in attachment.out corresponds to a mesh vertex and
consists of bone weights for each bone in the order of skeleton
joints (see HumanSkeleton in Pinocchio/skeleton.cpp).

If you want to provide your own skeleton, use the "-skel file" where
file is in the format described above.  For annotations, you'll have
to modify the skeleton.cpp file to build the skeleton
programmatically.  If the skeleton has already been embedded, and
you just want Pinocchio to generate bone weights, use the -nofit
option.

---------------------------
USING THE PINOCCHIO LIBRARY
---------------------------

In DemoUI, the function process(...) in processor.cpp gives an example
of the usage.  The simplest mode is to include pinocchioApi.h,
construct a Skeleton (such as HumanSkeleton()) and a Mesh and call
autorig() on them.  This calls all of the individual steps in the
right order.  The individual functions are also exposed in
pinocchioApi.h and pinocchioApi.cpp has the implementation of autorig
that shows how to call them.  The implementations of the individual
steps are described in the paper.

-------------------------------
MODIFYING THE PINOCCHIO LIBRARY
-------------------------------

Most of the header files (and associated .cpp files) are just
utilities:

mathutils.h      basic definitions
vector.h         a vector class parametrized by dimension
vecutils.h       basic distance and projection operations
transform.h      quaternions and rotate-translate-scale transforms
deriv.h          automatic differentiation routines
rect.h           defines an axis-aligned bounding-box
matrix.h         defines a variable length vector and dense matrix
lsqSolver.h      defines a nice way of specifying sparse least squares systems
lsqSolver.cpp    solves them or has an interface to TAUCS
mesh.h           the mesh structure (based on directed edges--Campagna et al.)
intersector.h    routines for intersecting lines with a mesh
pointprojector.h defines kd-trees for projection
quaddisttree.h dtree.h multilinear.h indexer.h --- octree and distance fields

The files skeleton.h and skeleton.cpp contain the definition and
implementation of the base skeleton class and a few example skeletons.
The code is fairly self-explanatory.

The actual rigging functions are defined in pinocchioApi.h and
attachment.h and are implemented in attachment.cpp,
discretization.cpp, embedding.cpp, and refinement.cpp.  Before
attempting to modify these, reading the paper is a good idea.  The
discrete penalty functions and weights are defined in embedding.cpp.
The continuous ones in refinement.cpp.  The source to the large-margin
penalty weights learning described in the paper is not included
because it's a horrible hacky mess and anyone who wants to do training
will likely want their own interface.

The motion reading and retargetting in the DemoUI (defmesh.cpp, filter.cpp,
and motion.cpp) is also a mess (hard-coded for a specific motion format for
a specific skeleton--completely unreusable).  Again, anyone who wants to do
anything serious has their own ways of making transforms from motion data.
