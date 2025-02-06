# [OpenVSP 3.42.3](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.42.3)

2025-02-06

Another small bugfix release that addresses some issues that were biting
some users.

Fixes:
 - Fix casting of string literals to bool for NameValData.
 - Fix RenameAttr in ResultsMgr enforces persistent AttrID.
 - Attributes API revisions.
 - Cleanup AttributeExplorer construction.
 - Link pthread library to cartesian example on linux.
 - Correctly delegate constructor for NVD bool case.


---


# [OpenVSP 3.42.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.42.2)

2025-02-03

This is a bugfix release that addresses a bunch of recently introduced issues.  Some
of these are pretty critical (crashes in common use cases).  Everyone should update.

Features:
 - Display alert message if accessing disabled screen is attempted.
 - Make Mode 'Apply and Show Only' button more visually distinctive.

Fixes:
 - Fix advanced link sceen failing to be disabled.
 - Various API documentation fixes.
 - Make Set_0 show up in Geom Browser by default.
 - Do not make Vehicle appear selected when it is not.
 - Update NerfGeomManager to match GeomManager Attribute changes.
 - Don't restore viewport size and view from file.
 - Revert bool NameValData to int type.
 - Ensure Vehicle BBox is up-to-date from API.
 - Fix gimbal lock with rotation matrix.
 - Only reset vehicle screen Parms from main screen.
 - Fix crash when ParmChanged called with NULL Parm*.  Thanks Mike V.
 - Improve facade search for host python executable.
 - Automatically call InitGUI from graphics API.


---


# [OpenVSP 3.42.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.42.1)

2025-01-20

Fixes:
 - Fix vanishing window on Windows with high resolution display.


---


# [OpenVSP 3.42.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.42.0)

2025-01-19

Although this version comes less than two months after the last, the improvements
it contains have been in the works for nearly a year.  This version contains a bunch
of infrastructure changes that will be appreciated by serious OpenVSP users.  There
are also a handful of smaller features and fixes that everyone will enjoy.

The biggest feature in this version is the addition of Attributes.  Attributes were
developed by Tim Cuatt -- thanks much Tim.  Attributes are a mechanism to add
metadata to just bout anything in OpenVSP.  You can use this metadata for whatever
you want.  Perhaps you want to make a note of where the value of a certain parameter
came from, or perhaps you want to add auxiliary information that will be used by
some downstream analysis tool.

Attributes are Name / Value pairs that can be associated with Parms, Geoms, Vehicle,
Measures, Advanced Links, etc.  Their value can be a string, bool, int, double, vec3d,
a vector, or a matrix.  The user can interact with Attributes through the GUI or the
API.

A few special case Attributes have been built-in to models.  First is a Watermark
capability.  You can set the text, color, and size of a text box that will be
superimposed on top fo the 3D window.  You might use this to add proprietary
markings to a file.  Second is a general Notes capability.  You can think of this
as a journal that you keep with a file.

It is always exciting to see what users do with new features -- Attributes will
certainly be another example of this.  More than most things we add, Attributes
are an unstructured feature meant to be used however the user deems fit.  Go forth
and attribute!

Sets and Variable Presets are two features that new users often find confusing, but
that experienced users find essential.  Modes is a new feature that combines Sets
and Variable Presets.  This will be even more powerful for experienced users -- but
hopefully will be within reach of novices.  If you haven't ever used Sets or
Variable Presets, now is the time to level-up, learn about them, and add Modes
to your skillset.

By popular demand, OpenVSP now calculates the MAC of a wing.  It also calculates a
new reference area called Scurve.  Scurve will only differ from Stot when blending
is used.  Scurve takes into account the curved LE and TE of the wing, while Stot
is meerely the sum of the areas of the trapezoidal base segments for a wing.  The
MAC and Scurve are now available for use as VSPAERO reference quantities.

The Python API facade has been extended to support the 'Multi-Facade'.  The
multi-facade will allow one Python process to interact with multiple OpenVSP
models simultaneously via the API.  Each OpenVSP model resides in a separate instance,
each loaded in a dedicated Python process.

There are a scattering of more features and fixes all around.  The descriptions
below are pretty self explanatory, so I won't belabor them here.  Update
to the latest and enjoy.

Features:
 - Attributes added as means to attach metadata to many things in OpenVSP.
 - Attribute interaction added throughout OpenVSP GUI.
 - Modes added as concept combining Sets and Variable Presets.
 - Modes interaction added to all analysis and export capabilities.
 - Re-write Variable Presets to be ID based instead of index based.
 - New Variable Presets GUI.
 - Added Watermark that can be displayed on OpenVSP screen.
 - Added Notes editor for tracking general model information.
 - Added Multi-facade mode to Python API
 - Added warning when GUI windows exceed 800 pixels tall.
 - New Measure GUI to fit in 800 pixel limit.
 - Improve resizability of numerous GUIs.
 - Add new CHANGELOG.md file with release notes for all past versions.
 - Add creation of vspscript test code from API documentation examples.
 - Add SplitWingXSec to API.
 - Calculate MAC for wings.
 - Calculate Scurve, a Sref that includes curved LE/TE.
 - Make MAC & Scurve available as reference quantities for VSPAERO.

Library Updates:
 - Update Code-Eli to support calculation of MAC.

Build system:
 - Build on MacOS with LLVM/CLang, not XCode.
 - Now build VSPAERO on MacOS entirely with CLang & libomp, not GCC.

Fixes:
 - Fix view inconsistencies with ManageViewScreen open.
 - Fix view updating from API.
 - Fix problem with OpenGL detection on startup.
 - Clean up verbose search for Help files.
 - Silence echo of reference parameters from VSPAERO API Analysis.
 - Fix conformal components on custom components.
 - Fix many issues with API example code - both Python and C++


---


# [OpenVSP 3.41.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.41.2)

2024-12-03

While there are some microfeatures in this version, it is really about
fixing some specific bugs in some relatively recently developed
capabilities.  As it gets put to use, some issues crop up, so here are
the fixes.

This version fixes a number of issues when using the GUI from the Facade
and the API.  If you are a plain GUI user or a non-GUI API user, you
shouldn't see any difference with this version (with some very minor
exceptions).

Fixes:
 - Improve displayed precision in User Parm sliders both int and float.
 - Add VSPCrash() to API to help test Wrappers.
 - Update on-screen file label when file name changes from GUI.
 - Make relative paths passed to SetVSP3ileName absolute.
 - Make vsp online help work from Python API, include files in package.
 - Improve finding python from facade.
 - Setup vspaero path from facade.
 - Make InitGUI() stand-alone and automatically call it.
 - Allow changes to GUI from API before it is open for first time.
 - Fix problems opening and closing GUI from API.


---


# [OpenVSP 3.41.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.41.1)

2024-11-10

Some API facade fixes were accidentally left out of 3.41.0.


---


# [OpenVSP 3.41.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.41.0)

2024-11-10

Four months in the making, this release is one that almost didn't happen...
At least not now and not like it is.

You might think that four months without a release would mean things have
been quiet on the development front -- you'd be wrong.  Work was
steadily progressing on a few big long-term projects when a few
high-priority bugs came to light.  Initially, I planned on quickly pushing
out 3.40.2.  However, when I stepped back, there were a lot of other things
that were also ready to go out the door.

So this amounts to a pretty big release -- with probably two big releases
coming in the near future.

Even though this release is pretty large, most of it can be considered
fixes and cleanups.  So the 'features' list is going to be sparse this
time around.

ParmIDs are now transferred when you change an XSecCurve type.  So, if
you've identified Height and Width on an Ellipse (in Advanced Linking,
Linking, FitModel, Design Variables, Saved Parameter Settings, etc) and
you change to a Rounded Rectangle, everything should 'just work'.

The structures capability received quite a bit of attention by addressing
issues identified by James Joseph and Bryan Sandoz.  The structures
mesher should be noticibly more robust.  When things do fail, they should
less frequently crash the whole thing.  Parts (including trims) no longer
extend across the symmetry plane to the other side.  Mass of assemblies
are now calculated.  Assemblies can now be written to STL and GMsh files.
Several lagging graphics update issues were fixed.

Note, a wing used in Structures should always be self-watertight.  I.e.
tip caps should be used in the model.  However, if you don't want to
include the tip caps in the structural skin, there is now an option to
turn them off.

ScreenGrab from the API should be fixed. Several issues with the new API
facade have been resolved.

The Set Editor now has some online Help -- thanks Jason W.

Many typo fixes across the code base -- thanks Luzpaz.  Lots of general
code cleanups.

Fix CompGeomp when a subsurface line lies exactly on top of a
tessellation line.

There are a number of build system improvements, it should be easier
to build on Arch and Debian now.

Added VSP_NO_API_WRAPPERS and VSP_NO_HELP build options.  So if you
only want a bare-bones build, or are working around build problems,
you have more options.

Several libraries were updated (Clipper2, Pinocchio, Triangle and
Delabella).  Consequently, the Libraries sub-project will need to be
re-built.  Most of this was to address cross platform build issues, but
the Triangle updates also include fixes to avoid some crashes and
an infinite loop.

So there you have it -- a heap of stuff from all over the place.
Everyone should update at least to make sure you still know how to
download a new version for when the next big feature updates are
ready.


Features:
 - Re-map ParmID's when changing XSecCurve type
 - Add link loop breaking message to advanced links
 - Write structure assemblies to STL and GMsh files
 - Calculate structure assembly mass

Library Updates:
 - Clipper2, better match packages available in Linux distributions
 - Pinocchio, build fix for some platforms
 - Delabella, build fix for some platforms
 - Triangle, avoid crashes and infinite loops, avoid namespace issues

Fixes:
 - Fix base airfoil scale when setting airfoil points via API.
 - Make API ScreenGrab() and UpdateGUI() blocking.
 - Fix ScreenGrab from API.
 - Fix StopGUI behavior from GUI within facade.
 - Fix GUI and Server shutdown from facade on MacOS.
 - Fix need for sleep() on facade GUI API startup.
 - Fix Conformal Component memory error identified by Jason Le.
 - Fix Conformal Component update lag when parent symmetry changed.
 - Fix bug of copying Parm instead of Parm value
 - Fix Structures handling of symmetry for parts including trims
 - Fix Structures meshing issues, should reduce crashes
 - Fix Structures trimming at the center line


---


# [OpenVSP 3.40.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.40.1)

2024-07-24

Quick release to fix bug when reading in props with less than three blades.


---


# [OpenVSP 3.40.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.40.0)

2024-07-19

This version may not be huge, but it is certainly worthy.  It includes
a few small features, some quality of life improvements, API additions,
and a bunch of bug fixes.

Whether you'll be at AirVenture, Aviation, the OpenVSP Workshop, or none
of the above, this version should be exactly what you need for your
mid-summer modeling needs.

There are two cool additions to the propeller component.  Individual
blade folding control allows you to model stowed blades the way
rotorcraft typically do it rather than the more symmetrical approach
usually taken by folding propellers.  Also, there is now support for
non-uniorm blade spacing.  Non-uniorm spacing is a technique to reduce
the noise level of a prop / rotor.  In addition to allowing the user
to dial in the spacing, OpenVSP will automatically calculate the
positions of the final two blades to maintain rotor balance.

If you've been thinking that OpenVSP needs extensive online help -- and
you've been wishing there was a way you could contribute to OpenVSP as
a project -- have I got a deal for you.  OpenVSP now has the
infrastructure to make it incredibly easy to add a help screen to any
screen in the program.  There will be a presentation about this at the
Workshop, but if you want to give it a go before then, let me know and
I'll get you started.  Check out the new help on the Advanced Linking
screen as an example.

When switching to a multi-view mode, the default view selections were
not great.  Now the default 4-view mode matches the traditional choices
for an engineering drawing.  Some additional hot keys have been added
to make working with multiple views easier.  Try shift-F, shift-C, and
shift-R.

There are a bunch of API additions all around.  Some to improve automation
of graphics, materials, and screen shots.  Some to improve working with
FEA structures and generating meshes automatically.  Some to work with
the image files associated with 3D backgrounds.  It is great to see
what people are accomplishing with the API.

I've updated the build system to build packages for Ubuntu 24.04 -- and
I'm consequently dropping support for 20.04.  It should still work fine
if you need it, but you'll need to build for yourself.

There are a variety of bug fixes all around, but the one that the most
users will probably notice is that grid and tick lines have been
restored in the VSPAERO and Wave Drag plots.


Features:
 - Individual blade folding
 - Non-uniform blade spacing
 - Blade balance calculation and control
 - Online help infrastructure
 - Help cheat sheet for advanced linking
 - Improved 4-view view selection
 - Multi view control hot keys
 - FEA Mesh API additions
 - View and material setting API additions
 - 3D background file API additions

Build system:
 - Add support for Ubuntu 24.04 / drop 20.04
 - Update FLTK to latest 1.4.pre version

Bug fixes:
 - Fix grid lines not drawing on Windows
 - Adjust tolerance in MergeBorderEndPoints
 - Fix build system for facade generation
 - Use ParmContainer::EncodeXml/DecodeXml when writing simple links
 - Fix missing inputs for parasite drag analysis


---


# [OpenVSP 3.39.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.39.1)

2024-05-14

Quick bug fix release that helps with 3D background relative paths on
Windows.  The fix looks correct, but if this works, it doesn't make sense
how the other platforms worked without this.  Oh well.

Bug fix:
 - Fix 3D background relative path bug.


---


# [OpenVSP 3.39.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.39.0)

2024-05-12

Coming less than a month after the prior release, I thought about holding
off on this release until there was more to go with it.  After all, this
release really consists of just one feature and a handful of bug fixes.

However, when I've shown the new feature to users, they want it released
yesterday -- or sooner if possible.  Who am I to argue?

OpenVSP (and its predecessors) have long had the ability to place a 2D
image as a background to be used as a guide when setting up a model.
This version takes that idea to the next level.  Instead of the image
appearing as a static background, it is placed in the model such that it
can pan and zoom appropriately.  Likewise, there is no limit to the
number of reference images that can be used.

Describing this feature doesn't succumb to the written word -- so look
for a video from me or Brandon in the near future.

Aside from that, there are a couple of bug fixes mixed in.  We're still
working through the aftermath of the DrawObj m_GeomChanged enforcement
from a few versions back.  A CFDMesh tolerance has been tightened
so cusped airfoils won't get smooshed.  And some continued build system
improvements to help with portability to more diverse platforms.

We've also updated (again) to the latest FLTK 1.4.0-pre release.  The
FLTK dev team is on a big push to 1.4 and I'm trying to make sure there
aren't any surprises when it goes final.

Features:
 - 3D Background images

Library Updates:
 - Update to latest FLTK dev as they approach 1.4.0
 - Build LibXml2 with -PIC
 - Update GLEW build integration to match modern GLEW

Bug Fixes:
 - Move Documentation build to MacOS from Ubuntu to get latest Swig
 - Drop MacOS-11 build, move to MacOS-12
 - Use gcc-11 on MacOS for ADEPT and OMP builds
 - Fix MeshGeom DrawObj issue
 - Tighten tolerance in MatchBorderNodes - Thanks Andy.


---


# [OpenVSP 3.38.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.38.0)

2024-04-17

Coming just a month since the last release, you won't be surprised that
this version is somewhat small overall.  It includes some more fixes
that I thought about releasing as 3.37.3.  However there is also a
broad variety of quality of life improvements that this release
deserves a .0 update.

In the Geom Browser, the Show Only, Show, and NoShow buttons now work
like the other things (like Hidden, Shade, Feature lines) with respect
to applying changes recursively.  I.e. now they only recurse a collapsed
tree.  When you Show or NoShow a parent entity on an expanded tree,
the visibility change only affects the selected entities.

The number of Sets is now user-adjustable.  Go to the Set Editor,
click the arrows at the top of the window.

Measure (probe, ruler, protractor) now works for non-surface Geoms.  This
is mostly for blank and hinge geoms, but works in a rudimentary way
for Mesh and HumanGeom (it always measures from their component origin).
So users who use Blanks extensively as intermediate coordinate systems
can now use them for all their Measure needs.

Several improvements around advanced links.  Unique variable names are
now enforced when renaming variables.  The variable renaming buttons
were re-worded for clarity.  Multi-line error messages are now displayed
when links break.

The full-path filename at the bottom of the screen is now displayed
in a larger font for better visibility on high resolution screens.
The filename (without a path) is now displayed in the title bar
at the top of the screen.

The integrated flowpath engine extension has a pair of improvements.
First, streamtubes are now extended to a constant X value -- even
when the start of the streamtube is not at a constant X.  There is also
a new option to auto-determine the extension distance based on the
overall dimensions of a selected Set.

Height/width/area/aspect sliders are now displayed for CEdit type
XSec curves -- even when the detailed editing window is not displayed.
This should allow for quick adjustments without having to get deep
every time.

Better patch names are assigned to wing patches when exporting STEP
and IGES files (Thanks Jeff V.).  This should make it easier to identify
wing trailing edge, upper, and lower surfaces in meshing tools.

There is also now an option to merge the lower and upper trailing edge
patches together (and leading edge too).  Some meshing tools did not
appreciate that we output split TE patches -- so now we'll merge them.

As usual, there are a few bugfixes too -- mostly related to graphics
glitches that were introduced in 3.37.0.

I don't think there are any high risk changes in this -- and hopefully
the graphics glitches introduced in 3.37.0 are calming down.  Everyone
should update.


Features:
 - Show Only, Show, NoShow buttons now recurse like everything else.
 - User adjustable number of Sets
 - Measure works with non-surface Geoms (blank and hinge)
 - Unique variable names enforced when renaming advanced link vars
 - Re-worded variable renaming buttons
 - Output multi-line error messages when links are broken
 - Bigger filename at screen bottom
 - Filename in main title bar
 - Integrated flowpath modeling extension now extends to constant X
 - Integrated flowpath modeling extension distance auto determined
 - Height/width/area/aspect sliders for CEdit XSecCurve now displayed
 - Better patch names for unintersected STEP/IGES surface export
 - TE surface merging for STEP/IGES export

Library Updates:
 - Update to latest FLTK dev as they approach 1.4.0

Bug Fixes:
 - Fix invisible subsurfaces
 - Fix XSecSurf highlights that won't go away
 - Expand ranges for AR and Span - should fix some odd wings seen in Group
 - Fix some odd scattered indexing bugs
 - Fix some problems with CFDMesh sourcing


---


# [OpenVSP 3.37.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.37.2)

2024-03-22

Version 3.37.0 had a handful of blemishes.  One lead to the release of
3.37.1 a few days later -- and today I'm releasing 3.37.2 with a few
more fixes.

No new features or other big improvements here.  Just bug fixes.  So,
everyone should update as soon as they can.

This version includes another FLTK update.  This update should make it
a tiny bit easier to build OpenVSP on some unusual computers that some
folks may have.

Features
 - None

Library updates
 - FLTK Updated to latest pre-1.4.0
   . Eliminates -no-pie flag from build, helps with Python API on RHEL9

Bug fixes
 - Fix a handful of crashes in/around structures
 - Fix a handful of 3D graphics glitches


---


# [OpenVSP 3.37.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.37.1)

2024-03-10

This patch changes a compiler flag that makes a new dependency build
properly on Windows.  This in turn fixes CFDMesh and FEAMesh on Windows.

Without this change, 3.37.0 will quickly crash if you try to use either
mesher.


---


# [OpenVSP 3.37.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.37.0)

2024-03-04

This version is dedicated to the X-57 Maxwell.

I remember the joy and excitement the team had as they watched
Administrator Bolden announce the program and the first piloted
NASA X-plane designation in more than a decade.  I was in the
audience at Aviation 2016 poised to upload the model to the Hangar
as soon as the announcement was made.

OpenVSP played at least a small part in the early stages of the
program.  Fit Model was used to build the fuselage model for the
Tecnam.  Nick used OpenVSP for the configuration studies.  Jeff used
it for the wing design.  Brandon designed the signature folding props
and nacelles.  He went on to maintain the definitive geometry that
would be the basis for all the CFD studies you've seen.  There are
other uses and users that I am regrettably leaving out.  It was an
honor to support their work and to have OpenVSP make a contribution.

This week, I'll join those friends and many others at the X-57
Closeout to glean what I can from the vast learnings of this team.
I know this didn't turn out the way you'd all hoped.  I also know
that you've made a big contribution to aeronautics.  You've changed
the design space.  You've moved the needle forward.  Congratulations
on your collective success.  Thank you for all your work.


Once again, this release has been a long time in the making.  There are
only a handful of main themes here -- and while you might not think they
apply to you, I strongly encourage everyone to upgrade.  In addition
to some really cool advances (that you might not need), there are a ton
of smaller improvements and fixes that will benefit everyone.

So go ahead and start your download before you finish reading this
message.  I'll wait here until you come back.

Thomas Nascenzi & the team at M4 Engineering and I teamed up on an SBIR
for the Navy starting a bit over two years ago.  While lots of smaller
features from this effort have already been released, this release
marks a milestone in this project -- the general release of what was
originally the 'big ask'.  Don't worry, there will be a lot more
progress from this project to share in the future, but this is an
exciting development for everyone involved.

The Navy uses a couple conceptual design tools (ADAPT and AIDEN)
that can use OpenVSP as a geometry engine.  They talk to OpenVSP
through the Python API.  These tools have a GUI front-end that the user
spends most of their time with.  The Navy wanted to be able to have
the OpenVSP GUI open and interactive at the same time as their other
aircraft design tool's GUI was open and actively manipulating the OpenVSP
model.  Change something in ADAPT, see the change in OpenVSP and
vice versa.

This may seem straightforward, but there is a lot of magic that has to
happen for this to work.  Making it work took a lot of Python magic as
well as a lot of C++.  Thomas and I worked together on this -- with
Thomas handling all the Python (and the good ideas) and me supporting
with whatever was needed on the C++ side.

So, you can now have an OpenVSP GUI open and interactive while your
custom Python application pounds away.  At a minimum, you'll be able
to visualize and interact with a 3D model of your concept as your
optimization study progresses.  I have no doubt that users will come
up with many more applications for this capability.

Of course, having a fully-featured OpenVSP GUI open while another
program is manipulating the model may seem like running with flaming
scissors.  It might be.  To address this, we've added some
'parental control' API routines.  These calls allow you to disable
certain features and capabilities in the OpenVSP GUI.  For example,
you can Nerf the OpenVSP GUI to a view-only interface for a
model.  We'll see how this works out in practice and if more is needed.

We've moved the API documentation from being specified where things
are registered in AngelScript to their native C++ home.  This also
allows us to generate a parallel set of Python documentation for
the API.  Going forward, the OpenVSP website will host both Python
and C++ / AngelScript documentation for the API.  The Python docs
are also now included in the openvsp package and should be available
in your IDE when you're using OpenVSP.

Among all of this work, there are some small changes to the Python
API.  It should be pretty harmless and generally more Pythonic, but it
might cause API users a few headaches.

One of the C++ changes to support using the GUI from Python affects
how 3D objects are updated to the graphics card.  We now attempt to
update these objects less frequently.  This should also provide a nice
performance boost, but at the risk of losing things once in a while.
If you find that something vanishes where it was previously visible,
mention it on the Google Group.



Next, some bad news.  In addition to lots of good stuff, 3.33.0 also
included some lousy regressions.  These regressions made FEAMesh and
CFDMesh both slow and crashy.

If you aren't into sausage making, skip a few paragraphs...

   The performance regression was super subtle.  The regression was
   actually over in Code-Eli (50487cc2bbfe).  Notice how I nonchalantly
   say "This does mean that a copy of the surfaces must be made because
   they are no longer const.".  Yep, that was it right there.  I even
   knew I was doing it at the time.

   Something I did to improve the accuracy and reliability of a
   surface-surface intersection algorithm inadvertently caused us
   to copy and destroy a giant complex data structure a bazillion times.
   All the time was spent allocating and releasing memory.

   The fix (c53f910aa2e4) was to allow a shift to be incorporated into
   the surface evaluation on-the-fly and switching back to a const
   reference.


   The crash-causing regression was something else entirely.  When
   our mesher (CFD or FEA) generates an initial mesh (InitMesh), it
   transforms every surface patch into flat 2D space and generates
   the initial mesh there.

   You might be wondering why you can't just use the surface's U,V
   parametric coordinates for generating the initial mesh.
   Unfortunately, a square in U,V is not guaranteed to look anything
   like a square on the surface.  The closer we can get to flattening
   the surface without distortion, the better.

   For years, we've used an algorithm based on Mysticism and Voodoo
   to map surfaces from U,V to 2D space.  It generally worked, but it
   was a dark place that will make you see things that aren't there.

   In our next regression (47e5744550eb), I tried to replace the
   old approach with something I thought would be an improvement.
   Spoiler alert, it broke things.  In particular, it would distort
   surfaces near degeneracies in a way that fine meshes would turn
   out hosed.

   As it turns out, this flattening problem is something that computer
   graphics researchers have been working on for years, so it made
   sense to drop the custom code and use an off-the-shelf solution
   instead (OpenABF).  It does a great job and should provide all
   the benefits I was hoping to achieve when I 'upgraded' the
   algorithm earlier.


The good news is, not only are these regressions fixed in this version,
I spent a bunch of time profiling FEAMesh under some extreme use
cases (thanks James Joseph) which revealed some unexpected slowdowns.
I was able to re-write several algorithms providing dramatic speedups.

If you previously grew tired of wondering how long ConnectBorderEdges,
BuildFeaMesh, and BuildChains could possibly take, you'll like what
you see here.  In addition to these three, a bunch of other speedups
were made across the meshing workflow.

While I was in there, I was able to fix a bunch of bugs in and around
FEAMesh.  Most of these were reported by users - thanks James & Bryan.


The other big item you'll notice in this release is we finally have
full Apple Silicon M1/M2 processor support.  The holdup had been our
scripting language - AngelScript.  AngelScript does some low level
stuff such that it includes some assembly code that needs to know
exactly how compilers on a given operating system and processor
platform send data to a subroutine.  This includes things like
do you pass arguments LtoR or RtoL, how many registers there are,
how do you use the registers, what do you do when you run out of
registers, etc.  The main AngelScript author doesn't have an Apple
Silicon machine and has had to rely on others to do this work
(which is outside my range).  Someone finally came along to make
it work, so here we are.  I helped track down one bug that turned
up along the way.

So, if you've purchased a Mac in the last three and a half years or so,
you'll want to check out the new download option.  It should be faster
for you, but everything else should be the same.


You may have noticed that there are a bunch of library updates along
the way.  In addition, there are a bunch of updates to the CMake build
system.  If you build OpenVSP yourself, you'll probably want to blow
away your build directories and start over.  Sorry for the hassle.  If
you typically use -DVSP_USE_SYSTEM_FLTK=TRUE (likely for Linux users),
I suggest you don't do that now.  The FLTK project is about to release
version 1.4.0 and I've bundled a cutting edge version here.  It includes
many improvements that won't be available in a system-installed package
until the final release happens and then the Linux distributions update
to it and then your machine is updated too.  Just use the one I bundle,
you'll get the latest.


Beyond that, there are a good number of small fixes and features tossed
in.  As mentioned earlier, everyone should update.  It is the right thing
to do.

Features:
 - OpenVSP GUI available from Python API
 - Facade allows OpenVSP API to run transparently on separate process
 - GUI parental controls
 - Python API documentation
 - Surface Intersecion, CFDMesh, and FEAmesh much faster
 - Add surface names to Intersect, CFDMesh, and FEAMesh progress output
 - Native Apple M1/M2 support

Library Updates:
 - New AngelScript - full M1 support
 - Removed local modifications to AngelScript for documentation
 - Moved AngelScript build to Libraries, better CMake integration
 - Updated FLTK to 1.4.0-preview version
 - Updated FLTK integration to Modern CMake
 - Added OpenABF - Angle Based Flattening
 - Updates to Code-Eli, performance improvements for meshing
 - Update how Eigen is included in project

Bug Fixes:
 - Fix meshing performance regression from 3.33.0
 - Fix meshing crash regression from 3.33.0
 - Fix on-the-fly updating of meshing lengths with model unit scale
 - Fix not scaling fixed points with model scale
 - Fix not scaling trim planes with model scale (thanks Bryan)
 - Fix FEA crash with subsurfaces and trims (thanks Bryan)
 - Fix FEA crash with empty trim part (thanks James)
 - Fix corruption of Quad Meshes
 - Fix FEA Individualize Rib and Plane routines (thanks James)
 - Fix scroll wheel zoom on Windows
 - Fix Wing insert section update from API.
 - Fix error in K to F unit conversion (thanks sladesladeslade)
 - Fix VSP icon in Ubuntu (thanks Cibin)


---


# [OpenVSP 3.36.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.36.0)

2023-10-27

It has been almost three months since the last release.  Tons of
work has been going on, but mostly on a big change that still isn't
quite ready yet.  While I have been focused on the big stuff, a ton of
other features and fixes have accumulated.  In fact, some of them are
pretty big in their own right.  So clearly we're past due for a release.

If you compile OpenVSP yourself (particularly on antiquated platforms),
updating to this version is going to be painful for you.  One of the
libraries I need for the next round of changes requires we move
to C++17 (we've been on C++11).  In turn, that requires that we move
to CMake 3.24 (we previously only required 3.1).  These changes cause
a nightmarish ripple through the toolchain continuum.  I'm sorry.

I've stopped builing and packaging for Python 3.6, moving support to
Python 3.9 and 3.11.

You can now choose to use your operating system's native file open/save
dialog window instead of OpenVSP's custom one.  Go to File... Preferences...
and make the change.  You will lose access to your list of favorite
folders, but it will act like most every other program on your computer.

There are many improvements to the user experience around advanced links.
You can now sort advanced link variables.  Variable names can be generated
automatically.  Unique names are suggested when you try to use conflicting
names.  You can rename a variable and the code will be updated too.
Advanced link actions are now available from the API.

When a circular link loop update is stopped, a warning message is issued.

A spanwise fraction 'eta' coordinate has been introduced for wings.  This
is available for specifying control surfaces, attachment to wings, and
conformal components based on wings.  This should be more natural than
U, R, and L for wings.

A protractor angle measure has been added to the Ruler and Probe measure.
It can be used to measure and mark angles and projected angles.

A classical lamination theory (CLT) calculator for composite materials
has been added.  Now you can build up a ply schedule in OpenVSP for your
structures.

Structures can also now include off-body fixed points.  These can be used
to build connections or to apply boundary conditions and loads.  Unlike
on-body fixed points, these float in space outside of the FEM mesh.

Several structures tweaks were provided by Bryan S.

There are lots of bug fixes all around.  Lots of help from users finding
and tracking them down.

Two bugs were fixed that caused long-running API cases to have heavy
memory growth.  If you still encounter memory-growth please help out with
a test case so I can get it fixed.

A subtle problem with IGES and STEP files was fixed.  Many downstream
programs didn't seem to care, but perhaps this will improve interoperability
for some of you.

OpenVSP should work on Apple Silicon again (still via Rosetta2).

Features:

 - Native file dialog
 - Advanced Link UI improvements
   .Sorting variables in link
   .Suggest when conflicting name used
   .Automatic naming
   .Variable renaming w/ code search
   .Advanced Link API
 - Issue warning message when a link loop is stopped
 - Protractor measure
 - Wing eta coordinate for control surface, attach, and conformal
 - Classical laminate theory for materials specification
 - Off-body fixed points for FEA Mesh
 - Write template NASTRAN executive and case control sections
 - Change NASTRAN files to use RBE2 instead of RBAR1
 - Improved error message when OpenGL will not load
 - Various cleanups to build system
 - Build Pyhton 3.9/3.11 instead of 3.6/3.9

Libraries:
 - Pinocchio moved to Libraries
 - Update Pinocchio to C++17 version
 - Update Clipper1 to Clipper2, move to Libraries
 - Update Delabella

Bug fixes:
 - Fix knot values in STEP and IGES files
 - Improve greying-out of 4-digit CLi parameters for prop
 - Improve adherence to 6-series airfoil nomenclature
 - Fix memory leak and growth plaguing long-running API users
 - Fix order of Calculix dependent/independent connected nodes
 - Fix planar slicing behavior with just one slice


---


# [OpenVSP 3.35.3](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.35.3)

2023-08-01

Disable feature that is causing problems with CFD and FEA Mesh before
the Workshop.


---


# [OpenVSP 3.35.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.35.2)

2023-07-28

Quick fix to prevent inf and nan values from propagating to delabella
where they can cause a crash.

These inf values likely come from the tri-tri intersection code.  Fixing
them there would be preferable, but this remains a reasonable safeguard.


---


# [OpenVSP 3.35.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.35.1)

2023-07-26

Quick fix to help DegenGeom and VSPAERO thin surface work with
changes to Mass Prop.


---


# [OpenVSP 3.35.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.35.0)

2023-07-23

This release is a little modest, but there is still enough here to
warrant a release before the Workshop.  I'm looking forward to seeing
some of you in two weeks -- unfortunately, if you aren't registered,
the registration is full.

The most user-visible change here is an update to the Mass Properties
capabilities.  Before, only a Blank component could be assigned a
point-mass value (and only a mass, no inertias).  With this update,
any component can be assigned mass properties -- including mass, local cg,
and inertias for the component.  (What do you call a point-mass with
non-zero inertia?)

These component mass properties will be included with the overall mass
properties calculation along with the volume-based and wetted area-based
calculations already performed.

Existing files with point-mass assigned to Blanks will be converted to the
new system -- this is not a backwards compatible change.

Blank components can now have symmetry (like all other components).  That
way, you can easily add a symmetrical copy of a Blank's mass properties.

The under-the-hood mass properties calculations have had some small changes
that should slightly increase accuracy (in ways that nobody will really
care about, but that make me happy.)

Most of the rest of the changes are very much behind-the-scenes.

Some API calls have been added to enable creating/deleting User Parms
from the API.

OpenVSP's PMARC 12 file writing was slightly improved and some Python
scripts for working with PMARC input/output files are now included.

The ability to read an existing VSPAERO run from disk was added to the
Analysis Manager -- exposing it to the API.

The biggest under the hood change should be invisible to users (except
for hopefully fewer crashes).  I've begun replacing a fundamental
library with an alternative.  The old library has a great reputation
(and we've used it forever), but when things go bad, it does not fail
gracefully -- it crashes the entire program.  This new library seems
robust and reliable so far -- but I'm hoping that if there are problems,
it will fail much more gracefully.

You will mainly see this difference in CompGeom and friends.  If this
goes well, we will later also switch to this new library in CFDMesh and
FEAMesh.  It isn't quite ready for that yet.

There were a few improvements contributed from the community.

Thanks Cibin Joseph for a bug fix when writing Degen Geom files.

Thanks to Jeff Allen for making it possible to delete multiple parameter
links at one time -- making the Add-All buttons much more useful.

All that and a few bug fixes here and there.

If you can't make Oshkosh this week, download a new OpenVSP and make
yourself an airplane.

Features:
 - Specified mass properties extended to all components
 - Point masses can now have inertia specified
 - Blanks can now be assigned symmetry properties
 - User Parm management added to API
 - pyPMARC added to python package

Libraries:
 - Use Delabella CDT library for CompGeom & friends

Bug fixes:
 - Fix missing fields in DegenGeom file.
 - Fix halfmesh bug for VLM with vspgeom files to vspaero.  Thanks Tim.


---


# [OpenVSP 3.34.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.34.0)

2023-06-04

Although I've been furiously working on lots of things, I hadn't planned
on making a release because I didn't think I had enough stuff ready. I
stepped back and took another look and realized that most of the in-work
stuff was actually ready and only one big thing (that won't be ready for
a while) was not.  So here is a pile of stuff for everyone to enjoy.

It was previously very challenging to get satisfactory results from
V-trimming of conformal components.  This was partially due to inherently
complex behavior -- but was exacerbated by terrible default settings.
Default values for V-trimming have been improved, hopefully making the
existing capability more usable.  In addition, a simplified Side 1,2,3,4
approach to V-trimming has been introduced.

In addition to simplifying V-trimming, the V-parameter speed of a
trimmed surface used to be very non-uniform.  This erratic behavior made
features like UV attachment to trimmed conformal components impossible.
It also made subsequent analysis of conformal components more difficult.

This behavior has been fixed.  The V-parameter speed of a trimmed
conformal component is uniform and the V-origin does not shift.  The
V-parameter of un-trimmed portions of the component remain unchanged.
This improvement has a perhaps unexpected effect -- end capping is now
supported for conformal components with U trimming.  Most significantly,
this means a trimmed conformal component can now have a rounded end.

Jason Welstead had contributed some improvements and cleanups to the
CHARM Automation Python tools.  While these are relatively minor changes,
this serves as proof of concept for navigating the NASA release process.
Look for more updates to come from Jason.

The primary meshing controls for CFDMesh and FEAMesh have been augmented
with non-dimensional alternatives.  The maximum edge length can now be
specified as a fraction of model size (as determined by the bounding box
diagonal).  The minimum edge length and maximum gap can now be specified
as a fraction of the maximum edge length.  These changes should make it
easier to get to an initial reasonable mesh and to better build intuition
in the meshing parameters.

CFDMesh and FEAMesh have been augmented with visualization of the local
mesh edge length criteria.  Mesh parameters are determined by the
interaction of multiple criteria -- it is not always obvious which
criteria determines the edge length at any particular location.  This
new visualization mode illustrates the active criteria and should make
adjusting meshing parameters more intuitive.

The under-development adjoint version of VSPAERO has been ported to
Windows (it was previously only available on Mac and Linux).  Now that
it is available on all platforms, the developers will be able to start
making its capabilities generally available.

U-direction clustering control has been added to fuselage and stack
components.  This can be used to tailor the wireframe mesh representation
or to compensate for a distorted mesh caused by skinning.

Limits on end-cap tessellation requirements have been relaxed.  Before,
only odd values greater than or equal to three were possible.  Now, both
even and odd values greater than or equal to two are possible.  This
will allow finer control of mesh resolution and also allow coarser
representations of end caps and trailing edges.

While technically a bug fix, one fix is significant enough to almost be
considered a feature.  OpenVSP has long behaved erratically when more
than one chord value of a given wing were updated at the same time.
Although this is impossible through normal interactive use, this occurs
commonly when automating some sort of design process via Design Variable
support or the API.  Various incantations have been tried in the past,
most involving frequent calls to Update() -- severely slowing any
design process that involves changes to the wing chord.  This issue has
been fixed -- or at least an official workaround is now supported.

If you need to change wing section chords from a design process, use the
following recipe:

1) Set all wing section drivers to root-chord, tip-chord, span.
2) Use the tip-chord to set every chord value possible.
3) Use the root-chord of the first wing segment to control the root chord
     of the entire wing.

This recipe should work and will only require a single call to Update()
after all Parms have been set.  Let me know if you have any trouble
with this.

There are a few small bug fixes in this release, but this one was
dominated by new features.  Download and enjoy.

Features
 - Improved V-trimming defaults for conformal components
 - Side 1,2,3,4 V-trimming for conformal components.
 - Improved skinning of V-trimmed conformal components.
 - End caps for U-trimmed conformal components.
 - Improvements to CHARM automation scripts (thanks Jason Welstead)
 - Non-dimensional CFD/FEA Mesh controls
 - Visualization of CFD/FEA Mesh criteria
 - Windows support for VSPAERO adjoint version
 - U-direction clustering control for Fuse and Stack
 - Relaxed limits on end-cap tessellation

Fixes
 - Setting multiple chord values in design context.


---


# [OpenVSP 3.33.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.33.1)

2023-04-30

Fix problem attempting to calculate RST/LMN attachment parameters to
parents that don't have surfaces - and therefore don't have these
coordinates defined.


---


# [OpenVSP 3.33.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.33.0)

2023-04-28

Earlier this year, the world lost a brilliant light far before its time.
Alex Stoll was a kind and gentle soul, a visionary aircraft designer, and
a good friend.  Alex committed his career to eVTOL far before it was a
thing -- he helped create the revolution.  Thank you for everything you
brought to the world.  This version is for you Alex.


A lot of recent development time has gone into long term work that isn't
ready for release yet.  However, this release includes some cool features
and a stack of good bug fixes.  Everyone should update

The often imitated wing tip-cap feature introduced in OpenVSP 3.6.0
provided a simple way to create rounded (and other) wing tip treatments.
Instead of wrestling with CAD, you got what you wanted with just a few
clicks.  Since then, I've spent a lot of time looking at airplane wingtips.
In that time, I've noticed that the rounded tips of 3.6.0 don't quite
match how many aircraft do it.  This version introduces a new
extended tip cap type that should better match some of these cases.

This change had some unexpected ripple effects.  The new tip caps are
constructed with a challenging surface topology.  OpenVSP's mesher had
to be improved to handle them robustly -- and those changes should
improve meshing for all shapes.

OpenVSP's parent-child attachment is used to simplify building complex
models by allowing the designer to express intent instead of values
in positioning.  This version adds two new coordinate systems that
can be used to define attachment.

The RST coordinate system is a volumetric coordinate system defined within
an object.  This is perfect for packaging studies -- where you're working
to place all the stuff you need inside the aircraft.

The LMN coordinate system is like RST -- but instead of varying with
parameter speed, LMN is linear in arc-length along the RST directions.
Use RST if you want to position something on the third bulkhead.  Use LMN
if you want to position it one third of the way along the fuselage.

RST coordinates had previously been exposed via the API for some users
doing cool structural MDO work.  Take note, the S coordinate has been
redefined to be double what it was before.  Before, it existed in the
range [0, 0.5] -- now it exists in [0, 1.0].  If you stored old S values
and you need to use them with a new version, just multiply by two.

Finally, 'dimensional' forms of the U, R, and L coordinate can now be used
to specify attachment.  This should make it easier to achieve certain
results (attach to the third bulkhead) without doing any pesky algebra.

In addition to those features, there are some VSPAERO updates (mostly
fixes) and a stack of general fixes.  This includes patching a few
memory holes that users had found -- if you're still seeing memory leak,
reach out and we can work on building a better test case.

If you're in doubt, you need this version.  Download it now.


Features
 - New extended tip-cap geometry
 - Convert CFDMesh 2D mapping to arclength based mapping.
 - Handle potential no-volume corners in CFDMesh.
 - RST and LMN Attachment
 - RST S re-defined.
 - Dimensional U,R,L attachment.
 - Visualize wake edges on vspgeom export.

Fixes
 - Clean up behavior of pull-down menus with special characters
 - Fix remaining issues from snprintf conversion
 - Fix syntax errors in master aero V&V script
 - Fix some quad-mesh bugs in FEAMesh.  Hopefully fixes needle meshes.
 - Make beam elements obey trim zones in FEA Structure generation.
 - Expand height of ExportScreen
 - Fix memory leaks from CreateParm() being called.
 - Fix memory leak in MeshGeom::Flatten...
 - Fix wake identification.
 - Handful of VSPAERO fixes.
 - Silence VSPAERO viewer checks for VSP_NO_GRAPHICS build
 - Silence cgx/ccx not found errors on Windows
 - Fix MeshGeom not scaling on load problem.


---


# [OpenVSP 3.32.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.32.2)

2023-03-22

While 3.32.1 fixed the most egregious issues with 3.32.0, it was still
a bit of a hot mess in its own right.  This release helps tame the chaos,
hopefully it gets people back on track.

Of course, I couldn't help myself and stick to _just_ bug fixes.  So,
if you use CFDMesh & Co, you will find that the 'f' and 'r' keys work
like they do in the rest of OpenVSP.  These are UX candy -- go ahead
and indulge.

(For those of you that are wondering what pressing 'f' or 'r' does --
try it!  If you can't try it right now, pressing 'f' will 'f'it all the
visible stuff onto the screen by auto zoom, pan, and setting the center
of rotation.  Pressing 'r' presents a bunch of green points.  Click one
and it will be set to the center of rotation and moved to the center of
the screen.

Other than that, it should mostly be a bunch of fixes.  The migration
from sprintf to snprintf broke a few things.  Those have been fixed and
an audit has been conducted, so hopefully we're through with that.

Fix lingering problems with CompGeom half meshes and tag files.  Previous
fixes broke things like Wave Drag and Parasite Drag.

Fix error validating subsurface structures properties on structures
data transfer.  Was preventing analysis of structures with subsurfaces.
Improve error messages while we're at it.

Attempt to set property length unit when converting an old file.
Unfortunately, I can't see a way to convert

Some of the VSPAERO output files were updated, our parsing of those files
has now been updated to match.

Fix some small VSPAERO issues that were causing hangs and crashes when
slicing planes were active.

Overall, not a ton of stuff -- but everyone should update because the
problems these fixes address were hitting a bunch of users.  Hopefully
we'll keep things under control for a while going forward.

Features
 - Make 'f' and 'r' keys work with CFDMesh & Co.
 - Allow parasite drag to use a reference wing not in the active Set.
 - Print missing command line when calling CpSlicer.
 - Clean up mesh from repeated mass properties analysis.
 - Attempt to set property length on import of older structures files.

Fixes
 - Fix validating structures subsurfaces
 - Fix visual artifacts in orthotropic material GUI in structures.
 - Fix double fee in Cartesian plotting library that shows up with bleeding
   edge versions of FLTK.
 - Fix parsing of VSPAERO output files.
 - Fix wave drag, parasite drag, half mesh, tag files issue.
 - Fix snprintf conversion issues
 - Silence snprintf warnings
 - Fix VSPAERO with slicing planes active


---


# [OpenVSP 3.32.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.32.1)

2023-03-01

Fix VSPAERO DOA problem.


---


# [OpenVSP 3.32.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.32.0)

2023-03-01

Perspectives could vary on this release -- some may say too soon, others
may say long overdue.  I suppose it depends on what features you're most
interested in.

Note:  If you use the OpenVSP structures capability, make backup copies of
your files before updating to this version.  There are a bunch of file
format changes that will not be backwards compatible.  There is a chance
of bugs in the new version.  So, make a backup to be safe and then help
me shake out any bugs in this version.

The last release was three and a half months ago and while there could
have been a release sooner, it did not seem like there was enough progress
to warrant an update.  Now that it is all pulled together, this release
looks huge.

FillMass is a feature that was developed in 2016 and never got released.
It computes running totals of mass properties (mass, volume, cg, and
inertias) as the slicing integral progresses.  Along with an option to
allow choice of direction for mass slicing, this feature can be used to
compute the volume, mass, cg, and inertias of the fuel in a tank as it
progresses from empty to full.  You probably don't need this feature, but
if you're worried about fuel driven cg movement and want to generate a
cg envelope, then this is for you.

Mousewheel zoom behavior has been changed to be a zoon/scroll rather
than a pure zoom.  Give it a try.

VSPAERO has been updated, but mostly updates and fixes to the adjoint
capability that is under development.  Not much for normal users this
time.

There is a new mode for Stack and Fuselage components to better allow
modeling of engine flowpath.  Before, one could build a model with a
single representation of the engine flowpath (one of several).  This
representation is usually tied to the needs of some analysis tool.
Changing to a different representation was very difficult.  The new
integrated flowpath modeling feature makes it easy to build a model once
and then change the flowpath representation at a later time.

In related work, the area slice tool was given the MeasureDuct option that
will calculate the cross sectional area of a negative volume inside a
positive volume.  While this may seem like a strange request, it will
allow calculation of the flow area through an engine modeled with
negative components.  Hopefully this works for Andy.

The Analysis and Results managers worked on a basis of name-value pairs.
These have been extended to name-value-documentation triples.  An API has
been created to provide access to the documentation and all analysis
inputs and results outputs have been documented.  This should make it
easier for API users to access OpenVSP analysis capabilities in a general
manner.

I've received some great feedback from users of the improving structures
capability (Thanks Bryan, Jeff/Sandra/Marc, and James).  Also a big
shout-out to PyNastranGUI for providing a means for me to check a lot
of these changes.  It is developed by OpenVSP user Steve Doyle.

As this release came together, I decided I would spend a week fixing as
many of the small issues they have found as I could.  After the end of
that week, one of these users asked for 'one more fix' for something I
have been avoiding.  Instead of a quick fix, this one required a
significant design change.  Eventually I decided to spend a day to see
how bad it was -- a week and a half later, I finally have it sorted.

That said, there is a chance I missed something or messed something up.
These changes required incompatible file format changes (structures only).
So, make backup changes and then give this version a good shakedown before
committing fully.

Most of the small structures stuff was on the NASTRAN side and CalculiX
users might not even notice a difference.  Some data fields were not
written correctly, elements and nodes were not indexed from one, material
property orientation was calculated wrong, BCs written using a different
form, split output to *.dat and *.bdf files, etc.

Structures subsurfaces could not be used to keep a limited area of skin
while deleting the rest (say for an aileron).  This has been fixed,
but the previous (shell, beam, shell and beam) option has been separated
into two options (keep or delete shell) and (generate beam T/F).  When
combined with the subsurface (inside, outside, none) option to control
how it is applied, this should enable all desired modes.

Structures fixed points should now appear first in the output files and
should appear first in the node index order.  Furthermore, they should
appear in a consistent order as changes are made.  This should make it
easier to work with the files for things like applying point loads.

The FEA output files (to NASTRAN or CalculiX) can now be prepared in
a different scale than the primary aircraft model.  This will allow
the aircraft designer to work in one set of units (say ft or m) and the
structures designer to work in a different set (say in or mm).

Material properties and shell/beam properties can now be input in
user-defined units on a per-quantity basis (not just selecting a
consistent set).  This will allow density to be input in familiar units
like lbm/in^3 instead of slinch/in^3 (lbf sec/in^4).

The built-in properties for metals have been updated to correspond to a
more appropriate source of aerospace data.  Material's now have a
description field where the user can add an extended set of notes about
the applicability and validity of the data.

Material and Property association (part X uses beam Y made of material Z)
is now done with permanent ID's instead of array index position.  Users
had been experiencing bugs where these associations would (seemingly)
randomly change.  This new approach should resolve any such problems.

OpenVSP will attempt to map your material/property index values to the
new ID based approach.  However, this might not be perfect.  When you open
an old file in this version, be sure to check your part/property/material
associations.

There are too many other changes to detail, check out the list below and
give it all a try.

Everyone should update.  Structures users will see a lot of benefit -- but
they also have some risk with this version.  If you use structures, make
backups of your files before moving to this version.  Once you make the
change, give things a thorough once-over before trusting everything.  Let
me know if you run into issues.


Features
 - Add new engine representation modes to Stack and Fuselage
 - Add documentation to AnalysisMgr and ResultsMgr
 - Add MeasureDuct option to measure negative inside positive area
 - Add subsurface tagging option to Export non-intersected files
 - Add thick/thin flag to vspgeom files -- disabled for now
 - Add degen surface support to CFDMesh
 - Add FillMass to mass properties calculations
 - Change MassSliceX to MassSlice - direction independent
 - Merge degenGeomMassSliceX into MassSliceX, eliminating duplicate code
 - Improved mousewheel zoom/scroll functionality, thanks iforce2d

Structures Improvements
 - Write fixed points before other nodes
 - Start node numbering with fixed points
 - Clean up NASTRAN file headers
 - Be less aggressive calculating node/element offsets
 - Improve shell/beam options to clearly enable all combinations
 - Write NASTRAN constraints as SPC1 instead of as part of GRID points
 - Separate NASTRAN output to *.dat and *.bdf
 - Allow FEM units to differ from OpenVSP Model units
 - Allow materials and properties to be input in independent units
 - Track material and property associations by ID instead of index
 - Add description field to material properties
 - Update metal properties to more authoritative aerospace reference
 - Consolidate FeaPart to use simplified subsurface shell flags

VSPAERO Updates
 - Fixed VSPGeom viscous drag calculations for wings in VLM mode
 - Unsteady ADJOINT bug fixes, including derivatives wrt rotor RPM
 - Added rotor efficiency as an explicit adjoint objective function
 - Exposed Forward and Adjoint solver matrix – vector, RHS to API
 - Exposed Adjoint partial wrt mesh and wrt solution components to API
 - Replaced if statement cut-offs in Biot-Savart law for near-zero
    radius distances with a 'smoother', core 'like' model
 - Kill off dead code in Viewer
 - Move sprintf to snprintf to silence clang warnings

Library Updates
 - Updated stb image library

Fixes
 - Expand max cl design for 6-series airfoils
 - Add vspaero binaries to MANIFEST, should help Python install
 - Fix example Python scripts for Swig change affecting ErrorMgr getInstance()
 - Fix conformal component with area parameterization of XSec
 - Fix CFDMesh file export extensions
 - Cleanups and fixes to MeshGeom
 - Empty meshes not written to key files.
 - Change sprintf to snprintf to silence clang warnings
 - Typo fixes from Luz Paz

Structures Fixes
 - Fix crash when clicking on Structure header
 - Fix NASTRAN material theta calculation
 - Fix NASTRAN MAT8 density written to wrong field
 - Fix NASTRAN PSHELL MID2 value
 - Always start FEA element counting at 1
 - Eliminated use of BuildIndMap, switching to nanoFlann instead
 - Fix bug visualizing QUAD8 elements
 - Don't write un-used nodes to FEM files
 - Do not allow spaces in material names for CalculiX *.inp files
 - Better handle -0.0 in NASTRAN formatted output
 - Ensure NASTRAN grid points are never -0.0
 - Fix various GUI crashes


---


# [OpenVSP 3.31.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.31.1)

2022-11-10

A handful of bugs quickly cropped up in 3.31.0 that justify this .1
release.  While there are always more bugs to be fixed, hopefully these
fixes help some people out.

There are a few new features in this release, but they're really small.
In a normal (larger) .0 release, these features probably wouldn't even
get mentioned here.

The real story of this release is the bug fixes - and it is a story mostly
about structures.

As it turns out, CalculiX B32R's are better than B32's and per-element
normal vectors don't work (they are still the default).  An option to
disable beam element per-element normals has been added -- it should be
used until CalculiX fixes their bug.

Features:
 - Add ResetMeshDisplay button to StructScreen
 - Structural assemblies can now be inserted with rest of vsp3 file.
 - Measure probes and rulers can now be inserted with rest of vsp3 file.
 - Drop Ubuntu 18.04 from automated build system.

VSPAERO Updates and Fixes:
 - Adding sensitivities for reference quantities.
 - Improve setting CpSteadyMin/Max in steady flow.
 - Bump and synchronize versions to 6.4.5.

Bug Fixes:
 - Improve Calculix handling of beam elements.
 - Fix crash when writing *.vspgeom file from imported mesh.
 - Fix Vinf activation with stability choice.  Thanks Daniel R.
 - Fix tags for CompGeom half-mesh.  Fixes *.vspgeom half-mesh panel mode.
 - Fix consistency of StructScreen current structure choice.
 - Fix StructAssemblyScreen height for various recent changes.
 - Fix writing of ghost nodes for half-geom *.vspgeom files.
 - Fix ID handling for copy/paste and insert of structures. Thanks Bryan.
 - Fix subsurface tagging bug with quad meshes.


---


# [OpenVSP 3.31.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.31.0)

2022-10-31

OpenVSP Halloween Edition!

For those of you keeping track, this ominous release was foreshadowed
back in the 3.28.0 release notes.  Like that release, this one is all
about structures.  There are a few other things, but mostly structures.

That isn't to say if you don't use OpenVSP's structures capability you
shouldn't update -- you most certainly should.

This release includes one very subtle feature that should impact every
single user.  It is worth updating for this feature alone!

The catch?  I'm not going to tell you what it is.  We'll see who is
the first to figure it out.  Post your guesses on LinkedIn or the
Google Group.

First, some of the non-structures updates...

NACA 6A Series airfoils will now be restricted to the modified a=0.8
camber line.  Although in many cases we go beyond the strict NACA airfoil
definitions and allow things their creators did not intend, in this case
it seems appropriate to restrict ourselves.  If you consider this a bug
fix, it has been with us since the first 6-series support added in RAM
v2.1 in the early 2000's.

The Analysis menu has received some re-organization and renaming to
facilitate all the structures work.  There are now "Structure" and
"Aero" sub-menus.  Hopefully everyone's muscle memory will adjust
without too much trouble.

The CMake build system has been extended to build RPM files for RedHat
type distributions.  We are not yet providing pre-built RPM's as our
CI build servers only run Ubuntu.  This should be useful for anyone
supporting multiple RPM based machines.

There are a bunch of VSPAERO fixes and cleanups.  Nothing really major
here, but a whole bunch of little things from the team.

And the structures updates...

The FEA Mesh workflow has been broken apart.  You can now generate
a mesh without writing it to file -- and write a previously generated
mesh to file at a later time.  This added flexibility was required for
some of the later improvements, but come as a nice addition on their own.

Structural boundary conditions can now be specified for each structure.
BC's are applied to mesh nodes.  The user selects which DOF to restrict
and also how to identify which nodes to apply each BC to.

Structural assemblies have been added.  Each assembly is made of one or
more structures.  These structures can be connected with rigid links
between Fixed Points in each structure.  Assemblies are written out to
a combined FEM file with node and element offsets automatically generated.

A VSPAERO / CalculiX based Aero-Structure workflow GUI has been added.
This GUI helps guide the user through a process of generating loads,
applying them to a structure, and then analyzing the result.

Note:  This is the final release where an Ubuntu 18.04 package will be
built for download.  Please update to 20.04 or 22.04.

Features:
 - Trick or treat?  Halloween mystery feature...
 - Add structural assemblies made from multiple structures.
 - Add rigid connections between structures in assembly.
 - Add boundary condition specification to structures.
 - Add aero-structure coupled problem GUI using CalculiX and VSPAERO.
 - Split FEA Mesh generation from writing to file.
 - Also split FEA CAD file generation and export from Meshing.
 - Keep FEA Mesh in-memory, allowing more flexible workflow.
 - Re-arranged menus to group structures and aero analyses.
 - Build RPM packages for RedHat-based distributions.  Thanks Jeff A.
 - Add aerodynamic center stability mode to GUI.
 - Force NACA 6A series to use a=0.8 camber line.  Thanks Daniel M.
 - Remove VSPAERO single analysis from API - reduce code duplication.

Bug Fixes:
 - Allow non-unique names in pull-down menus.  Thanks Brandon.
 - Make environment.yml match Python version.  Thanks Jason W.
 - Fix component IDs passed to *.vspgeom.  Fixes propellers.
 - Write NORMAL at all B32 nodes, not just the first.
 - Fix exporting symmetrical propeller at origin to STL.  Thanks Brandon.
 - Insert FixPoins before InitMesh -- guarantee Fix Points
 - Lots of VSPAERO fixes
 - Lots of structures fixes


---


# [OpenVSP 3.30.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.30.0)

2022-08-27

Coming just a month since the last release, this one is small, but you'll
want to update.  A few handy features and some nice fixes all around.
Quite a few of the fixes were identified by users at the Workshop.  It was
a great meeting.  I hope to see you there next year.

If you look closely, a bunch of the changes may seem like they don't
accomplish much beyond deck chair rearrangement.  Many of these changes
lay the groundwork for some bigger features that aren't quite ready yet.
However, it is helpful to get the smaller bits out so development
can continue with them included.

In preparation for the Workshop, I made a bunch of presentations.  I got
pretty tired of having to crop all of my OpenVSP screenshots, so this
version includes an automatic crop feature added to the built-in
screenshot.  Next time, it'll be much easier.

Superellipses have long been ugly for large exponent values.  They aren't
anymore.  The maximum exponent has been raised from 5 to 20 and the
issues that used to start appearing around 5 have been eliminated.  If
you need a superellipse with exponent greater than 20, you probably want
to use a rounded rectangle.  If you really need it, let me know.

I've decided to drop the Win32 build from the normal set of builds.  I was
mostly keeping it around in case any problems came up with the Win x64
build.  There have been none, so away it goes.  Let me know if you
desperately need the 32-bit build for some reason.  Otherwise, step up
to the 64 bit version.

The Ubuntu 18.04 build is next on the chopping block.  The CI server has
started warning of its deprecation.  Speak up now if you need it to
stick around.

Features
 - Added automatic crop for built-in screenshots.
 - Increased precision for subsurface parameters.  Thanks Andy
 - Enabled subsurface line parameterization by [0,N] basis.  Thanks Andy
 - Drop Win32 build

VSPAERO Updates
 - Improve CMake build to copy binaries to Python packages and vsp build
 - Fixed Unsteady Cp calculation for panel solves.
 - Fixed test for wake TE points that lie on a wing/body intersection.
 - Inclusion of some developer test cases.
 - Updates to the API for time accurate gradients.
 - Bug fixes in the time accurate gradients.
 - Added in adjoint gradients wrt to inputs (M, AoA, Beta,P, Q, R, & RPM)
 - Inclusion of wing optimization test case.
 - Overhaul of Makefile system.
 - Started adding VSPAERO API Doxygen documentation.
 - Started building CI workflow based on Makefiles
 - Fixed nans for the fem load file
 - Fixed built-in screen grab on high-res displays.  Thanks Rich.
 - Fix no -label case for adb2load
 - Make adb2load understand CalculiX files with ORIENTATION information
 - Fix change to *.history file that broke OpenVSP's parsing.  Thanks Cale
 - Add Y=0 slice automatically to VSPAERO runs
 - Change default wake iterations to three at Dave's recommendation

Python Integration
 - Python package includes VSPAERO binaries
 - Python package should auto-detect VSPAERO binaries
 - Virtual environment set up for Python development and testing
 - Analyses should be available when package is loaded.  Thanks Brandon.

Bug fixes
 - Fix advanced link crash when clicking on browsers.  Thanks AJ.
 - Make some API calls work with any Container, not just Geom. Thx Brandon.
 - Fix area parameterized XSecCurve Scale.  Thanks Cale
 - Remove remnant triangles from CompGeom half model.
 - Make CalculiX files print either tris or quads, not both
 - Fix GUI for finite beam structure parts
 - Probe GUI would go crazy with singular points.  Thanks Juan.
 -


---


# [OpenVSP 3.29.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.29.1)

2022-07-27

Revert some changes to AngelScript.

These were intended to help with the Apple M1 architecture, but they
have broken something else on x86-64.  Revert the changes until we can
work out a proper fix that moves all systems forward.


---


# [OpenVSP 3.29.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.29.0)

2022-07-26

In many ways, you could say that the modern history of OpenVSP started
with a conversation I had with Mark Moore at Oshkosh 2005.  I'm not at
the show this year, but why not release OpenVSP 3.29.0 to mark
the occasion.  If you're there this week -- have a great time.

This release is mostly about a small number of things that took a large
amount of work rather than a large number of small things.

The biggest OpenVSP change is the ability to generate quadrilateral
meshes using CFDMesh and FEAMesh.  These are not high-quality quads.  They
are trivial conversions of tris to quads.  If you need quads, they in fact
have four sides.  If you are picky about your quads, they may not meet
your needs -- except that they have one more side than triangles.

Rather than convert tris to quads at the last possible moment (say as
the files are written to disk), the conversion process was performed
somewhat earlier.  This allowed the inserted points to be added _on_ the
surface (or intersection curve).  It also means that all the internal
data structures were modified to work with quads as well as tris.  This
should mean that most of the infrastructure is in place if someone were
to come along and want to work on quad mesh improvement algorithms
to generate higher quality meshes.

The other big change is a long-awaited update to VSPAERO.  Many changes
across the board -- although many are not user-visible.

The OpenVSP side of the VSPAERO GUI has been updated to integrate these
changes, but many of the new VSPAERO capabilities are not yet supported
from the GUI (some may never be).

Force summation calculations in VSPAERO have been improved.  Accuracy and
consistency should be improved - particularly for unsteady cases.

There are now adjoint and complex step derivative versions of the VSPAERO
solver.  API hooks have been added to allow tight integration with
OpenMDAO and structural analysis tools.  These capabilities are only
available on Linux and MacOS.  These capabilities are not available from
the GUI.  There is no new documentation for these capabilities.  If you
want to use them, you will need to be comfortable with C++ and be able
to figure it out yourself.  There will be papers, examples, and
documentation for this stuff in due time.  This is still being actively
developed.  Do not ask for hand holding at this time.

The VSPAERO GUI has made *.vspaero files (previously marked experimental)
the default for panel models.  The DegenGeom files are still default for
thick surface models.  The option now says 'Alternate file format' instead
of 'Experimental'.

Work has begun on porting OpenVSP and VSPAERO to the Apple M1 chip. I have
moved my primary development machine to a M1 Mac, so hopefully progress
will be quick.  It is now to the point where it will compile, it will run,
and it will crash.  If you want to help progress this port, you will need
to compile OpenVSP on your own M1 machine and start debugging problems.
The port is not yet ready for non-developers to try out.  If you are a
'regular' user, you can continue using the x86-64 version through the
Rosetta 2 translation software.

Unfortunately, some of the differences in the chip architectures are going
to lead to subtle problems that will take a long time to hunt down and fix.
For example, when converting "int i = (int)((float)INFINITY);" will result
in i = 2147483647 on the M1, but i = -2147483648 on x86-64.  We should not
generally be relying on float to int overflow behavior, so this sort of
thing represents a bug that should be fixed no matter what. One such case
has already been found.

Quite a few build system changes (mostly VSPAERO) including addition
of an automatic differentiation tool for the VSPAERO adjoint version.
These changes warrant a version bump.

Features
 - Optional quad meshing for CFDMesh and FEAMesh
 - Option to turn off high-order elements in FEAMesh

VSPAERO Changes
 - Improved force (especially drag) calculations, and bug fixes there in.
 - Adjoint solver, stand alone now works for steady and usteady
 - Adjoint solver + solver = Optimizer with an API. (Steady and unsteady)
 - Initial API hooks for integration with OpenMDAO
 - Initial API hooks for tightly coupled structural solves
 - Complex step version should anyone care
 - Fix for thin/cusped trailing edges on panel meshes.
 - The degen geom and vspgeom files are both supported equally in terms
     of comp ids/surface now... it was a mess before
 - Sticking with DengGeom for default VLM solves
 - VSPGEOM for Panel solves as default
 - Some of the output files have changed as folks were asking for more
     info in things like the span loading data, etc.
 - Cut planes added for solution visualization
 - Span loading for thick geometries
 - Preliminary supersonic panel support
 - Bugs.  Some fixed, some added.

Bug Fixes
 - Fix control surface addition for Wing Geoms
 - Silence echo of BEM file on read
 - Fix airfoil thickness scaling problem with blended wings
 - Fix VSPAERO Viewer movie creation on Windows (hopefully)
 - Fix int to float conversion overflow in InterpDistTable in CFDMesh


---


# [OpenVSP Version 3.28.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.28.0)

2022-05-30

Lots of work on structures modeling and FEA mesh generation - with a few
other small things too.  If you aren't into structures, you could be
forgiven for skipping this version.  On the other hand, there is no
downside, so why not update and stay current.

If you're into structures and your wish isn't fulfilled by this release,
I expect a lot more work in this area in the next few months.  Expect
some larger changes in this area by late fall.

There are other changes in the works that aren't ready for this release,
so it isn't all structures either.

Enough vagary, lets get specific...  I'm going to try something a little
different here (let me know if it is a positive) -- I'm going to paste
in the SHA of the approximate end to each feature.  So, if you're
following along at home, you can see what is up.  I'll do this summary
in order of the commits, so the start of each range is the commit after
the previous range.  Occasionally other things slip in, so remember
this is approximate.

Also, a lot of the progress in this release was actually done in
Code-Eli (our surface library), so check that out if you want to see
how the sausage is made.

6a37ad The Update process when structures are involved has been improved.  Think
the faster update changes a few versions back -- but for ribs and spars.
This should mean that interacting with a structures model is _much_
faster.

32b0ee Finite line subsurfaces have been added -- and added to structures.  You
can now add a subsurface line that starts at (Us,Ws) and ends at (Ue,We).
These lines do not enclose an area and can not be used to tag tris, but
they do enforce a line in a mesh and can be used to form beam elements
in the FEM.  This allows stiffeners to span a limited distance of a model.

b57244 Initial support for orthotropic materials -- adding them in the GUi,
writing them to NASTRAN and CalculiX files, etc.

21682e Add API calls to evaluate RST volumetric coordinates and also
invert XYZ points back to RST.  This is intended to support structural
optimization - including cases where the structure and mesh come from
outside of OpenVSP.

f6ab38 Add support for writing material orientation information to
CalculiX files.  Previously, NASTRAN files included information that
oriented each element with the parent surface's U direction.  CalculiX
gets orientation data on a per-part basis (not per-element).  Orientation
can now be specified in more than a dozen ways and both NASTRAN and
CalculiX files will do their best to represent the user's intent.

d0e54a User supplied offsets can now be applied to node and element
numbering for NASTRAN and CalculiX files.  This is useful when combining
two files into a larger assembly.

597f9a The ability to add fixed points on both skins and other structure
parts was fixed.  An un-needed complex iterative algorithm was removed
from this process.  Some crashes should be avoided.

3e47ad Add the ability to trim FEA structures by groups of FEA parts.
This makes it easy to trim the ribs and skin in front of the front spar
or to remove the skin and rib tails from where an aileron or flap goes.
This only applies to the mesh (not the STEP/IGES files) at this time.

ed0051 A variety of bug fixes and cleanups.

486e23 Make mesh generation process more verbose.  The added messages
should be useful in cases where things go wrong.

5253d6 Update to the latest version of the nanoflann nearest neighbor
library.  No particular need, but since an additional use of nanoflann
was going to be added, it seemed appropriate to check for the latest
version.

50751a Refactor MergeBorderEndPoints algorithm from mesh generator.
The MBEP algorithm was usually not a problem for CFDMesh, but could
become very expensive for complex FEA meshes.  It was fundamentally
re-written to be faster.  My test case went from 7.5 minutes to
20 ms.  Nothing like a 20,000x speedup.

86af9e Modify Triangle library to avoid infinite loop.

53d4d5 Add locking to meshing console output.  This prevents crashes
due to race conditions of output to the console.  This became prevalent
after the earlier added verbosity changes.

b1ccea Add a bunch of debugging output for intersection and meshing
processes.  Will be helpful tracking down issues in the future.

4724f9 Clean up structures GUI code a bit.

32a518 Implement save/restore of trim parts.  Trim parts were added
earlier, but save/restore was skipped.

0fc9fc Misc. changes to improve structures

367269 Add API call with ray-shooting based inside/outside test.  The
algorithm is based on the one from CFDMesh, but it has been re-implemented
in Code-Eli so it can easily be called from OpenVSP.

699a1e Add API for conversion from LMN to RST coordinates and reverse.
LMN is similar to RST, but it is linear in distance -- where RST's
behavior is determined by the UW parameter speed.

0849a3 Beautify laminate material notation displays.  Also only write the
utilized materials to the CalculiX and NASTRAN files.

6b9132 Changes to speed up FindRST API call.

0b4265 Update AngelScript to latest version

dcce24 Fix problem with airfoil points API call from Python

390bc8 Fix CompGeom behavior with half-mesh and negative volumes.
Discovered by Andy Hahn.

Features
 - API RST volumetric coordinate evaluation XYZ=f(RST)
 - API RST coordinate inversion RST=f(XYZ)
 - API LMN/RST coordinate conversion
 - API for component inside/outside check
 - Structures GUI much faster
 - Finite line subsurfaces - also in structures
 - Orthotropic material support in GUI
 - Write orthotropic materials to NASTRAN and CalculiX
 - Specify material orientation method on per-part basis
 - Write material orientation information for CalculiX
 - Node and element offsets for FEA files
 - Add FEA part and mesh trimming by other structural parts
 - Increased verbosity of output during intersection and meshing

Library Updates
 - AngelScript updated to 2.35.1
 - nanoflann updated to 1.4.2

Bug Fixes
 - Fix specification of fixed points in FEA mesh
 - Rewrite MergeBorderEndPoints to be fast
 - Prevent infinite loop in Triangle that would cause crashes
 - Fix crashes from intersection / meshing console
 - Add more debugging output for intersection / meshing process
 - Fix default Calculix file extension
 - Only write utilized materials to CalculiX files
 - Fix setting airfoil points from Python API
 - Fix problem with CompGeom and negative components with half mesh
 - Many small bug fixes and cleanups


---


# [OpenVSP 3.27.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.27.1)

2022-03-18

A new winner for the smallest OpenVSP release ever.  A single line changed.

As it turns out, the skinning GUI does not work in 3.27.0.  Some may find
this to be an improvement, but unfortunately it does count as a regression
and it was unintentional.

With much embarrassment, we will push out this .1 and hopefully everyone
can pretend like 3.27.0 never happened.  I hear people don't like .0
versions anyway.

Features
 - None

Fixes
 - Make skinning GUI work again


---


# [OpenVSP 3.27.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.27.0)

2022-03-17

A few modeling features, bunches of usability tweaks, some tiny performance
improvements, and an equal dose of bug fixes.  Something for everyone here,
enjoy an update.

I generally get (very limited) positive feedback from these release notes.
Would you prefer something more concise -- or more detailed?  Perhaps you
love mystery and would rather read the source to find what has changed.
Let me know on social media or come to the Google Group to discuss your
preference.

The most user-visible changes in 3.27.0 are some new modeling capabilities.

You're most likely to use Edit Curve corner rounding.  Cubic Bezier and
Linear Edit Curves can now apply corner rounding to any sharp corners in
the curve.  Simply specify the radius and the corner will be replaced with
a circular rounded corner.  This reduces the number of parameters needed
to represent many shapes -- while improving shape precision and making
control much easier.  This seems like a small change, but will have
big impact for those who make use of it.

OpenVSP bodies (Stack, Fuselage) have traditionally been made from 2D
curves placed in 3D space and then skinned or lofted to form a surface.
Have you ever wondered what you could do if the curves were 3D?  There
are now two ways to do so.

First, 3D Edit Curves.  In the curve editor, you now have freedom to
control the out-of-plane dimension of curves too.  You can also change
the view perspective to work with the curves from top and side views.

3D Edit Curves are extremely powerful -- but my experience shows they
should be used sparingly.  If you don't _need_ them, don't use them.

They are most effective when a body doubles back on itself -- like the
lip of an inlet.

I have a feeling that there are going to be tons of new user questions
on this feature where the answer is "don't use it".

The second way to achieve 3D curves is with the new Chevron capability.
Chevrons allow a wave-like perturbation to be applied to any curve type.
A short description does not do this feature justice.  It allows easy
parametric generation of extremely complex shapes.

Most of the rest of the changes are less obvious.

The chevron work and edit curve corner rounding led to another round of
improvements to the corner rounding work -- you may notice that rounded
rectangle corner rounding limits have been relaxed.

The Measure tool can now be constrained to construct axis-aligned rulers.
Mike R. will enjoy this.

The Bezier surface intersection algorithms have seen several improvements.
These should improve quality and robustness of CFDMesh, FEAMesh, and
intersected STEP/IGES exports.  Thanks to William C. for the help finding
some of these problems and to AJ for providing a test case that found
some problems with the fixes.

A small step to improve our testing framework has been taken.  Tests
written in AngelScript and Python can now be added to a centralized
testing framework.  This seems simple and obvious -- but was quite complex
to achieve.  Hopefully next time there will be a host of tests added
because of this.

View clipping Parms are now linkable and discoverable.  Precision has been
added to the parameter linking GUI and to any files written by ResultsMgr.
Thanks to Andy H. for pushing for those.

I was convinced by Jeff V. that our transformations for wing dihedral and
twist were applied in the wrong order.  This has been fixed. Unfortunately,
this will result in a slightly different shape for identical inputs.  I
don't like to make changes like this, but it is the right way to go.

Rotate foils to make dihedral is now a per-XSec variable along with a
global setting.  There is also now an option to correct airfoil thickness
for airfoil rotation due to dihedral.

Advanced link editing and broken link behavior has been generally improved
thanks to input from AJ.

The Edit Curve control point browser now resizes thanks to Anwarmou.  They
also contributed a handful of code cleanups and bug fixes.  Thanks much.

Irian O. found some problems with v2 import while doing an archaeological
dig through ancient supersonic VSP files.

Another large round of source code typo fixes by Luz Paz.

The vspaero viewer save movie path problem should be fixed -- hopefully I
didn't make it worse.  Testing appreciated (particularly on Windows).

Plenty of other bugs were fixed too.

No changes to the VSPAERO solver in this drop.  I expect the next release
will be heavy on VSPAERO updates.

Features
 - Edit Curve corner rounding
 - 3D Edit Curves
 - Chevrons
 - Improvements to corner rounding
 - Ruler measure single-component mode
 - Numerous improvements to surface intersection calculations
 - Set up unit test framework based on AngelScript scripts
 - Set up unit test framework based on Python API
 - Make view clipping Parms visible for linking and discovery
 - Increase output precision of ResultsMgr CSV files
 - Increase displayed precision in simple parameter linking
 - Rotate foils to match dihedral is now a per-xsec option
 - Add option to scale airfoils to correct for dihedral thinning
 - Add pop-up warning when a link becomes invalid
 - Mark advanced link as red when invalid
 - Protect for duplicate Var names in AdvLink when drag-n-drop creating
 - Populate var browser when variable selected in Advanced Link
 - Pass script return value to command line in batch mode
 - Make control point list resizable - Anwarmou

Fixes
 - Scale of FileFuse XSecs when importing v2 file
 - Fix sign of wing twist on v2 import
 - Feature line DO pre-allocation performance bug
 - Make VSPAERO GUI automatic time stepping calc match VSPAERO
 - Correct order of dihedral and twist transformations
 - Fix treatment of propellers by parasite drag tool
 - Make XSec Cut behave as expected with Linking
 - Do better job showing group edit screen
 - Cache hyperbolic tangent stretching calcs -- insignificant speedup
 - Fix spelling error in VSPAERO GUI - Brandon L.
 - Add GetVSPVersion() to C++ API - was in AngelScript - Jason W.
 - Fix lots of typos - Luz Paz
 - Fix botched assert - Acxz
 - Handful of bug fixes & source cleanups - Anwarmou
 - Drop comma from Selig airfoil export - Anwarmou
 - Fix propeller cylinder projection problem
 - Fix VSPAERO viewer save movie path name issue


---


# [OpenVSP 3.26.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.26.1)

2022-01-05

If 3.26.0 wasn't new enough for you, here is one better.  No new features
here, but a couple of bug fixes that are pretty essential.

If you weren't on 3.26.0, you should because it has lots of good stuff,
If you're on 3.26.0, you should move to 3.26.1 because it has some
important fixes.

Most importantly, a list-moment change to 3.26.0 disabled a few buttons
and other things in the GUI -- Wing Airfoil Copy/Paste, changing which
Prop Curve you were editing, and two new Prop curve features
Approximate All and Reset Thickness.  These fixes alone are worth
the price of admission.

In addition, I've finally (knock on wood) fixed the issues that were
making actuator disk not work from the GUI with thick-surface files,
thin-surface via the experimental format, and general odd behavior
when using the Shown set.

Plus a fix for a bug that might be impossible to hit.

Also, some more spelling corrections from Luz Paz -- for people who care
about spelling errors in the comments of source code.  Is my mother
watching?  Thanks for the contibution.

Features:
 - None

Bug Fixes:
 - Fix broken GUI functions (Wing Airfoil copy/paste & Prop curve stuff)
 - Fix actuator disk broken cases with VSPAERO
 - Other small fixes

Cleanups:
 - Another round of source comment spelling fixes - thanks Luz Paz


---


# [OpenVSP 3.26.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.26.0)

2021-12-31

Happy New OpenVSP!  A new version for the New Year.

This version is long overdue.  First expected to contain a small number
of minor features -- the fixes are what really stand out in this
release.  2022 looks like a good year for OpenVSP, expect more frequent
releases on a variety of fronts.

There will be an OpenVSP overview paper presented (virtually) at SciTech,
unfortunately I will not be there in person.  Have a safe conference
and look for the new paper.

There are no significant VSPAERO updates here -- but I know Dave has some
in the pipeline that we will try to get out soon.

There are a few significant geometry changes.

First, XSec Curves (think super-ellipse, rounded rectangle, etc) can now
be parameterized by any combination of Width, Height, Area, and
Height/Width ratio (pick two).  It takes some magic under the hood,
but it should 'just work' for the user.  Next time you're designing an
internal flow passage, set your area and adjust other parameters as
needed.

Second, Propeller section curves can now be lofted onto the cylindrical
surface they are on instead of a flat plane.  This will be useful for
anyone designing ducted propulsors with finite tip-gaps, or if you think
a curved foil will better match your blade-element theory design.

Third, Rounded Rectangle corner rounding has been fixed for non-orthogonal
corners.  It astounds me that nobody has noticed and complained about
this before -- it has been broken for years.  Sometimes I wonder if
anybody uses this stuff.  Tap tap tap...  Is this mic on?  The radii are
correct now and the corners should be much more circular in extreme cases.

There are a few analysis improvements - open meshes are allowed for planar
slice, and single slicing is now allowed (useful for FitModel workflows).

Two changes should greatly improve the performance of the GUI.

First, the Faster Update changes from a few versions back were extended
to prevent a full Update when changing the active XSec or Airfoil
(clicking arrows at the top of the XSec, Skin, Modify, Airfoil, or
Blend tabs).  This should be nearly instant now.

Second, the GUI update was doing something foolish that really slowed
things down in some cases (mostly complex propellers).  General interaction
should be much faster now.

One particular bug fought dirty and made it personal.  I feel like it is
one of the most challenging bugs I've ever fixed.  I'm fairly sure
that it only affected me on my development machine -- but it may have
been the source of random seeming problems for other users too.  The
symptoms looked like memory corruption (it wasn't).  You may know that
OpenVSP uses unique identifier strings extensively.  The bug caused us
to sometimes generate non-unique identifiers -- this is bad.  One of
the libraries we use was resetting the random number generator seed without
our knowledge.  Sometimes it would get reset to the start of the identical
sequence of random numbers we were using.  Other times it would start
on a fresh sequence.  When reset to the same sequence, collisions would
occur.  Changing to a new RNG with a private seed cleared this right up.

Chasing this bug provided motivation to perform lots of cleanups across
the code base.  While none were critical or likely substantial, they
represent good progress to prevent future problems and improve the signal
to noise ratio when using various diagnostic tools.

Justin Gravett helped out with this release in a number of ways.
Announcement of his departure in the last release was slightly premature.
Jason Welstead make a handful of improvements to the CHARM automation
framework.  Luz Paz corrected typos throughout the source code.
Julius Quitter and anwarmou both contributed tweaks to how VSPAERO
presents results or how OpenVSP reads them in.  Thanks also to
everyone who reported bugs and made other suggestions.

There are lots more updates and fixes in here, but the performance
improvements and bug fixes should provide enough motivation for
everyone to update.

Features:
 - Area parameterization of 2D body-type curves
 - Cylindrical projection of propeller sections
 - Allow slicing of open meshes
 - Allow planar slicing to do just one slice
 - Increase sig-figs in Prop Edit Curve GUI
 - Add Approximate All button for Prop Curves
 - Add Reset thickness button for Prop Curves
 - Improve behavior for deleted variables with Advanced Links
 - Optimize ScreenBase update -- much faster now
 - Improvements to CHARM Automation from Jason Welstead
 - Add missing Body of Revolution API Calls
 - Add GetVehicleID() to API
 - Draw arrow icons for symmetrical props and disk-mode props
 - Allow Wedge airfoils to be inverted (GUI addition)
 - Scroll wheel to zoom
 - Calculate T/C for CST and VKT airfoils
 - Remove remnants of v2 drag TSV file output from CompGeom

Bug Fixes:
 - Fix RNG problem that could result in ID collisions
 - Don't trigger full update when changing active XSec or Airfoil
 - Fix rounded rectangle for non-orthogonal corners
 - Improve accuracy of circle approximations
 - Fix crash in FEA GUI when attempting to delete OML skin
 - Make 2D View screen work with Body of Revolution
 - Call ToBinaryCubic before interpolating airfoils, prevents crashes
 - Limit ToBinaryCubic recursion level
 - Improved support for reversed props and disks

Cleanups:
 - Make curve editing GUI code common across users
 - Various typos in source thanks to Luz Paz
 - Move Util.cpp/h to VspUtil.cpp/h to avoid name collisions
 - Myriad general source cleanups suggested by static analysis and Valgrind
 - Make sure all Parms have a non-null ParmContainer
 - Move non-Vehicle Parms registered to Vehicle to be Vehicle Parms
 - Make LightMgr proper singleton, eliminate VehicleGuiDraw


---


# [OpenVSP 3.25.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.25.0)

2021-08-09

About five years ago, Justin Gravett made his first contributions to
OpenVSP.  He has had his hands in pretty much every version since 3.9.0
and has been the source of most of the progress for about the last
three and a half years.

Justin started by adding the 2D SVG and DXF export followed by XPatch
export support.  He expanded our API coverage to be far more complete,
also writing API documentation with examples for every routine.

Justin has kept the VSPAERO GUI up with Dave Kinney's continuous
progress, also developing the VSPAERO V&V script.  The OpenVSP v3
Incarnation of FEAMesh structures capability was written by Justin.
It represented a huge capability leap from what was in v2.

Justin developed our capability to export intersected STEP and IGES files
including BREP solids.  He also developed the Generic XSec lending great
freedom to geometry modeling.

I'm sure I'm missing other contributions.  All along the way,
he has found and fixed countless bugs across the codebase.

OpenVSP isn't losing Justin - but we are going to see less of him.  He
is moving on to the next adventure in his life where OpenVSP will be
more of a side hustle than his main gig.  We wish him luck and extend
the greatest of thanks for all he has done for OpenVSP.


On to the 3.25.0 release...


This version is mostly bug fixes, but also has a significant update to
VSPAERO.  Everyone should upgrade.

We have dropped the pre-built binaries for Win32.  Our build system was
having trouble finding Python for Win32, so we're going to drop it and
see if anyone notices.  If this is a problem for you, ask on the Google
Group and we'll figure something out.

We hopefully have finally resolved the Python version issues we've had.

Win64 should be at Python 3.6.8
MacOS should be at Python 3.6.14
Ubuntu 20.04 should use 'apt-get install python3' which is 3.8.10
Ubuntu 18.04 should use 'apt-get install python3' which is 3.6.9

If the consensus is that we should move our Windows/MacOS Python support
forward, we should be able to do so easily -- help us form a consensus
on the Google Group.

Is anyone out there using OpenVSP Python API with Python 2?

VSPAERO v6.2.0 comes with lots of bug fixes and improvements including
span load distribution plots for thick-surface solutions.  The
VSPAERO viewer now does screenshots with PNG files and adds some
animation generation support across platforms.  Generating
a movie requires ffmpeg somewhere in your path.

The force/moment calculations are all re-written to use the
Kutta Jukowski theorem for lift and drag, but explicit downwash
calculations at edge centers (instead of loop averages).  Some differences
are seen in drag, so Cd is calculated from the KJ theorem, but it
also writes out Cdi as calculated from a TE / Trefftz-type method.
They should generally match for steady cases.

We now include OpenVSP and VSPAERO support for the experimental
*.vspgeom file.  There are features to add and bugs to work out, but
this will eventually be the only way of communicating geometry from
OpenVSP to VSPAERO.  Think of it like a free development preview.

Features:
 - Updated VSPAERO Solver, Viewer, and CPSlicer to v6.2.0
 - Support for ReCref sweeps in VSPAERO
 - Adds reading of VSPAERO *.polar results
 - Improves behavior of vertical scroll bars
 - Add Mach and set name to Wave Drag output and file name (thanks anwarmou)
 - Improves drag and drop for skinning parameters (thanks anwarmou)
 - Improves detection of feature line and planar part intersections
 - Improves Debian installation instructions (thanks Cibin Joseph)

Bug Fixes:
 - Fixes for Total Wing Parameter Calculations
 - Fixes crash with exactly aligned Prop XSecs
 - Fixes upper/lower design mode for BORGeoms
 - Fixes multiple issues with parameter links
 - Fixes memory leaks in FEA Mesh and resizable columns
 - Updates shell property specification in NASTRAN export
 - Fix clearing of VSPAERO CpSlicer results
 - Fixes Python version specification in GitHub Actions build
 - Fixes update issue for Variable Presets
 - Fix problem with VSPAERO viewer not launching

Other:
 - Increases minimum CMake version from 2.8 to 3.1


---


# [OpenVSP 3.24.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.24.0)

2021-06-02

While initially planned as a patch of primarily bug fixes, this release
has ended up with enough new material to justify a minor version increase.
Primarily, this is due to a large number of library updates that have been
included. With these new libraries comes an increased risk in new issues,
especially if you’re compiling yourself, so please report any problems to
the Google Group or GitHub.

This release includes many substantial bug fixes. In particular, a variety
of issues that were still lingering from 3.22.0’s Faster Update.
Clustering was not being applied to exported geometry files and therefore
was not being applied in VSPAERO either. Imported meshes would not display
until another update was triggered, leading many users to believe they
were doing something wrong. Similarly, Fit Model and Snap To results would
not display after the tools finished executing. Parm Containers were not
being completely linked, causing API functions to not be able to find
certain Parms. These and a handful of other update issues have been
addressed in this release. Additional changes to the Faster Update code
have been made to help ensure there are no additional bugs remaining, but
as always please report anything that seems odd.

Many improvements have been made to the API with the help of a *.vspscript
that is exported alongside the API documentation. This script contains a
standalone function for each snippet of example code. Executing this
script has allowed for a large number of errors in the documented examples
to be fixed. General improvements were also completed for the API tests
and various API functions. Jason W contributed to the Python API
integration with CHARM and Cibin J further improved the VSPAERO API
example. API scripts that iteratively execute VSPAERO no longer need to
compute the geometry input files (DegenGeom or Cart3D) each iteration if
changes are not being made to the geometry. This may allow you to speed
up your APi script.

Propellers can now be specified for Blade Element Analysis through the
API, as has already been supported through the GUI. These types of
discrepancies between GUI and API inputs, outputs, and capabilities often
occur when a feature is first added. We strive to identify and address
these differences, so please report if something seems missing in the API.

Almost half of OpenVSP’s libraries have been updated in this release. Some
may differ from their previous version by only a few commits but others
have much more substantial changes. The FLTK update is one of them, as
we’ve switched to an entirely different development path. The new FLTK
version is a unofficial development branch that contains a significant
number of new features and fixes compared to the official stable branch.
One of the most noticeable will be a higher resolution display on Windows.
Since FLTK 1.4-Pre has not yet been officially released there may be
undiscovered issues, so please report any found. Similarly, AngelScript
and Eigen are included as unofficial releases that include a large number
of changes since the last time we updated them.

EDIT_CURVE now has an option to flip the background image in order to
align a cross-section with either a front or rear view. Rounded Rectangle
XSecs now support skew in two directions and control of each corner radii.

Many of the other changes listed below have come from issues or
suggestions reported by users. For example, ExportFile now returns an ID
if a mesh is generated in the export, allowing for the MeshGeom to be easily
deleted or manipulated from the API. Parasite Drag calculations for grouped
components is now done correctly. Many thanks to the users that bring up
ways to improve OpenVSP or contribute directly, like with Peter M’s
support for *.poly surface tagging.

This release includes too many other small fixes and features to be
discussed. While there are no big new capabilities in this release, it
fixes numerous bugs that have been hitting users.  This version is in
the must-update category.

Features:
 - Option to flip background images in EDIT_CURVE editor
 - Color specification for EDIT_CURVE points
 - Improvements to API tests
 - Maintains VSPAERO geometry data in memory after VSPAERO execution
 - PropGeom ID input added for Blade Element API analysis
 - Each API documentation example extracted as standalone *.vspscript
 - All example code has now been fixed to run successfully
 - Improvements for Python API and CHARM integration (thanks Jason W)
 - Windows build instructions added to README (thanks Jason W)
 - Vertical skew and individual radii specification for Rounded Rectangle
 - Negative TE caps (Experimental)
 - Return MeshGeom ID from applicable export functions (Issue #183)
 - Surface tags in CFD/FEA Mesh *.poly output (thanks Peter M.)
 - General code cleanup
 - Various additional API improvements

Library & Build Updates:
 - Update GLM to 9.9.8
 - Triangle modified for CMake 2.8 compatibility
 - Eigen updated to 3.4-rc1
 - Code-Eli updated to support new Eigen
 - STB image updated to 2.26 and image_write to 1.15
 - LibXML2 updated to 2.9.10
 - GLEW updated to 2.1.0
 - Angelscript updated to 2.35.1 WIP – 2021/04/15
 - CMinPack updated to 1.3.8
 - FLTK updated to 1.4-Pre - supports higher resolution Windows display
 - Planar Entity Type 108 added to libIGES

Bug Fixes:
 - Geom dropdowns in Parasite Drag GUI
 - Parasite Drag calculations for grouped components
 - Crash in Wave Drag GUI when selecting flowthrough SubSurfaces
 - Bookkeeping for negative volumes in CompGeom
 - Issue of points moving while zooming in EDIT_CURVE editor
 - Reading & writing VSPAERO control surface stability derivatives
 - Update DegenGeom preview (issue #174)
 - Update Snap To results
 - Update clustering for all Prop blades
 - Update clustering for export (issue #180)
 - Update MeshGeom after import
 - Update Fit Model results
 - Update Parm Links
 - EDIT_CURVE update issues
 - Coincident Stack XSec crash
 - GCC 4.8 compile issues (issue #179)
 - Deleted control surfaces left in control surface groups (issue #175)
 - VSPAERO Propeller uniform RPM issues
 - Unsupported parent for ConformalGeom crash
 - Fixes for VSPAERO example scripts (thanks Cibin J)
 - Various issues with resizeable columns
 - ControlSurface grouping API function
 - Other misc. bugs


---


# [OpenVSP 3.23.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.23.0)

2020-12-12

This version fixes some critical bugs that have been hounding users.
If you're using 3.22.0, you certainly want to grab this ASAP.

3.22.0's Faster Update changes came with significant risk of breaking
things.  Sure enough, that risk was realized.  Users have been hitting
a handful of update bugs that have made 3.22.0 difficult for some to use.
This release fixes all the known Update related bugs.  There may be more
hiding in the corners, but they'll be the less frequently encountered
type.

There are also some updates and features included here.  Users of the
(relatively new) general XSec capability will enjoy a nice overhaul that
factors in lots of user feedback.  It barely missed the last release, but
even more improvements there have come along in the meantime.

A point-projection algorithm was added and exposed to the API at the
request of the M4 Structures Studio developers.  Hopefully it meets their
needs to improve their toolchain.

These changes alone probably wouldn't warrant a version bump, but there
are some library and build process changes that raise the level here a bit.
FLTK has been updated to hopefully fix some problems that show
up on the brand new MacOS Big Sur.  The Triangle library has been
replaced with a version modified for better error handling and a more
C++ API.

The last change is process rather than code.  Our automated build system
has moved from Travis-CI and Appveyor to GitHub Actions.  This change
was forced by an account policy change by Travis-CI.  So, the builds
may be done with slightly different versions of the OS, compiler,
system libraries, Python, etc.  So, things that worked before might break.
Hopefully it isn't too much and we can sort out any issues.  This is
the way forward.

Overall, you'll want to get this just to fix all the annoying update
problems.

Features:
 - Add point projection along an axis algorithms to API.
 - Improved background image for Edit XSec type
 - Improved re-parameterization of Edit XSec types
 - Improvements to Edit XSec GUI

Library & Build Updates:
 - Update Code-Eli to support axis projection and other cleanups
 - Update FLTK to fix problems seen on MacOS Big Sur
 - Update Triangle to version with proper error handling
 - Build system moved to GitHub Actions

Bug Fixes:
 - Update w/ clustering
 - Update of Point Cloud Geom
 - Update with non-intersected MeshGeom
 - Update wing after paste airfoil
 - Update of axis markers for HingeGeom
 - Update AxisLength
 - Thickness issue with file-type airfoils imported from v2 files
 - Radius scaling of corner radius of rounded rectangles on wings
 - Errant documentation of vspgeom file format
 - Differentiate active airfoil name and active XSec name
 - Various issues with Edit XSec types
 - Other misc. bugs


---


# [OpenVSP Version 3.22.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.22.0)

2020-11-08

After more than three months of cooking, we have a little big release...

OpenVSP generally follows a version of the major.minor.patch versioning
system.  Major numbers are reserved for really big  changes -- like the
first open source release, or fundamental re-writes that cause
input file incompatibility.  Minor numbers typically accompany a bunch
of features and also updates to the packaged libraries.  Patch updates
mostly contain bug fixes, but some features always creep in.

It may not look like it to everyone, but this release has a lot to it.
Even so, I nearly released it as a patch release 3.21.3 - mostly because
a lot of what is here is building to more things down the road.  My
excitement for what is to come makes me see these changes as a bit less.
However, this release does include a small update to Code-Eli, so
anyone building it themselves needs to watch for that.

The big change that most users will notice is that OpenVSP should now
update significantly faster for complex models.  The model Update()
routine has been substantially restructured to be much smarter about
what needs to be updated depending on which Parm was changed.  It also
caches more intermediate results to allow a variety of shortcuts when
warranted.

This change required major surgery.  This does not come without risk.
Several volunteers have been testing this for a while, but there is still
a chance that something is not right.  The most likely problem to watch
out for is something doesn't update when you think it should.  Keep an
eye out and if you have any problems, please post a bug report to the
Google Group.

BTW, if you aren't aware (or subscribed) to the Google Group, it is the
best place to ask questions or discuss OpenVSP.

Another big change is a big update to MeshGeom & Friends. (CompGeom,
Mass Properties, Area Slice, Wave Drag, etc.  These now support 'negative'
components.  The user can designate one or more Geoms as 'negative' and
instead of contributing to the OML as a union (addition), they contribute
as subtraction.  CFDMesh has had this capability for some time and CAD
export recently joined in the fun.  Now MeshGeom & Friends can poke holes
in your model too.

Negative component support for MeshGeom was actually a step towards another
feature in this version.  The ability to meaningfully handle
non-watertight surfaces.  Some may view non-watertight surfaces as a bad
thing -- but they don't have much imagination.  OpenVSP has long used
non-watertight surfaces to represent things - actuator disks and wakes
in CFDMesh, structures in FEAMesh, etc.  CompGeom can now work with
thin (non-watertight) surfaces mixed with thick watertight bodies.  The
thin surface representation is derived from the plate or camber Degen
representation.  So, CompGeom can now process a geometry with (for example)
the wing, tail, and propeller surfaces modeled as thin camber surfaces
and the fuselage and other things modeled as thick bodies.  If this
doesn't excite you, that is OK.  Wait for the next version, it will be
cool.

A new file format has been defined and set up for export from MeshGeoms
and CFDMesh.  The *.vspgeom file is similar to a *.tri file, but it
adds some information.  The surface u/v parameter values are provided at
polygon vertices.  This means they can be multi-valued where two
objects intersect.  Although files currently will only contain triangles,
the format will represent arbitrary polygons.  Furthermore, wake edges
are explicitly defined in this file, so downstream analysis programs
don't have to use heuristics to detect them.

For those of you still reading, one cool update is that various tables
throughout the GUI now have resizable columns.  So no more frustration
because the columns are all the wrong size.  Just resize them.

Features:
 - Faster Update complex models should update more quickly than before.
 - Negative components to MeshGeom & Friends.
 - Thin (non-watertight) surface support for CompGeom.
 - vspgeom file export.
 - Enhanced wedge airfoil type (requested by Mike W. and Liam M.).
 - Print out search path for VSPAERO (thanks Jason W.).
 - Expand API to set VSPAERO search path.
 - Updates to Python tools from Uber team.
 - General code cleanups.
 - API test framework for CFDMesh.
 - Build support for Matlab API.  Adds VSP_ENABLE_MATLAB_API CMake flag.
 - Improved FEA Structure spar parameterization (suggested by Mischa P.).
 - Option to rotate FEA Structure ribs with dihedral.
 - Lots of API additions and cleanups.
 - Tables now have resizable columns.
 - Trimmed CAD intersection curve tolerance control.
 - Automatic actuator disk hub diameter.

Bug Fixes:
 - Improvements to automated build process.
 - Fix cross platform browser launching.
 - Fix bug in unit conversion in Python tool.
 - Tag CFDMesh surfaces even when SubSurfaces are not intersected.
 - Fix problems with SuperEllipse XSec Type.
 - Bookkeeping for point masses in Mass Properties.
 - Fix API update issues for VSPAERO, Wave Drag, and Parasite Drag.
 - Many other fixes.


---


# [OpenVSP 3.21.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.21.2)

2020-07-27

By definition, polishing is a process of making ever smaller changes.
While we'll never be finished, this release serves to further polish
OpenVSP.  There are tons of fixes and small features.  There might
not be one huge feature - but then again, maybe we made that one tweak
that will make your life better.  Either way, you should update
right away

Welcome Gary Gray to the development team.  Gary is interning with ESAero
and his first contributions are all over this release.

Look no further than a total Refresh of the Set Editor -- should be much
nicer to work with now.  This also entailed converting the Set Editor
from a *.fl file to a code-generated GUI.  This won't mean much to
users, but this is great from a developer point of view.

Gary also converted all other *.fl based GUI's in OpenVSP to code-generated
GUI's.  OpenVSP proper no longer depends on FLTK's fluid executable
(the VSPAERO viewer still does).  Gary converted Set Editor,
Mass Properties, Planar Slice, Type Editor (now an export option),
Texture Manager, and the File Dialog!  Justin killed off
the pickSetFlScreen.fl to finish the job.

I am often more excited about lines / files removed than those added.
This is no exception.

You can now re-load VSPAERO results into the GUI for later visualization.
VSPAERO is further updated with a number of improvements inspired by
user questions on the Google Group.

Trimmed CAD export got a lot of improvements (keep the testing and
feedback coming).  It now supports negative and transparent components.
So, if you want to poke a hole in something, you can.  Also, you can
include an actuator disk or wing wake in your CAD export.

Nobody was using the error checking while using the API, so errors are now
printed out by default.  If you really don't like this, you can call
SilenceErrors() or PrintOnErrors() to toggle this feature off and on.

Too many other small features and fixes to list.  Download and give
it a try.


Features:
 - Update of Python files developed by Uber.
 - Negative component support for CAD export.
 - Transparent component support for CAD export (disk and wake).
 - Improved labeling for CAD export.
 - User contributed VSPAERO API tutorial.  Thanks Cibin.
 - Make double-click association of .vsp3 files possible on Linux.  Thanks Cibin.
 - Read prior VSPAERO results into GUI Results Manager
 - Updated tri-tri intersection routine, more robust.
 - Print errors during API by default, can be silenced.
 - Updated VSPAERO isolated rotor example to use GUI where possible.
 - Many varied code cleanups identified by automated analysis.
 - Extension of FEA Structures API functions.
 - Extensions to API for Copy/Paste of Sets.
 - Renamed surface intersection to Trimmed Surfaces.
 - Echo vspviewer command to console at launch.
 - Include list of merged and removed meshes from CompGeom and friends.
 - Allow negative offset conformal geoms.
 - Make HingeGeom obey Scale().
 - Added info to Line SubSurf display.
 - Loosen limits on General XSec type.
 - Readjust column widths when resizing windows.

VSPAERO to 6.0.4:
 - Improved agglomeration routine at creating coarse meshes.
 - Updated the viscous airfoil model.
 - Fixed the spikes near the centerline in the loading.
 - Fixed a bug in the ground effects implementation.
 - Various bug fixes.
 - Fix crash with multiple disconnected bodies in panel code.
 - Fix problem with zero wake iters on unsteady cases.
 - Fix problem with Cp hot-spot near small chord tips.
 - Some fixes for V&V script.

Bug Fixes:
 - Fix surface identification for FEA Fixed Points.
 - Fix Surf ID and Surf Index mixup for Trimmed CAD Labels.
 - Fix crash on FEA Mesh if no structure selected.
 - Fix wake attachment line for curved trailing edges.
 - Fix GUI Update for Stack Loop policy.
 - Fix memory leak in cppexecv (Mac / Linux only).
 - Fix smart input for both inputs of FractionParm.
 - Deactivate width/height parm for CEdit XSec for Wing and Props.
 - Various fixes to CEdit GUI.
 - Fix problems with initial flow condition on VSPAERO sweep analysis.
 - Fix crash for intersection curve refinement for CAD export.
 - Allow XSec insertion after last XSec for Stacks.
 - Fix name identification for 67 series airfoils.
 - Improve API error messages.
 - General fixes for Shown / NoShow Sets
 - Fix bugs with generic XSecs
 - Many other fixes.


---


# [OpenVSP 3.21.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.21.1)

2020-05-10

Coming just a month after the previous release, this release is heavy
on bug fixes with just a couple features thrown in.  The fixes are
widespread -- everyone should update to take advantage.

Not a lot else to say -- just go download it!

Features:
 - Rearrange Prop gui to accommodate recent additions
 - Background image support for XSec editor
 - API support for number of actuator disk and unsteady groups
 - Edit cross section in dimensional or scaled form.
 - Plot rotor distributed loads in Results Manager

VSPAERO to 6.0.3, mostly bug fixes
 - Line contour drawing option
 - Fixed the centerline issue for cases with symmetry OFF
 - Fixed the side slip at the symmetry plane jump

Bug Fixes:
 - Restore writing degen stick on Matlab
 - Write all fields to BEM file
 - Correct writing pre_cone to BEM file
 - Fix variable preset screen update
 - Remove GUI and API support for removed VSPAERO options
 - Clean up last VSPAERO result before each run
 - Fix smart input on Windows
 - Don't allow spaces in unsteady group name to VSPAERO
 - Fixes with rotors to VSPAERO
 - Fix pressure unit conversions in parasite drag GUI
 - STEP file fix to help SolidWorks
 - Fix bug with CFDMesh half meshes very slow.  Fixes #116
 - Fix auto time step with nothing moving
 - Fix actuator disk rotation direction and reversal
 - Fix edit XSec type use with propeller
 - And more


---


# [OpenVSP Version 3.21.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.21.0)

2020-04-10

Have you checked out the OpenVSP Ground School yet?  You should.
https://vspu.larc.nasa.gov/

When Justin suggested we should pull together the pending changes and roll
another release, I don't think either one of us expected there to be this
much.  While the world is social distancing, enjoy this release - it has
more than expected.

The significance of the first change can not be over stated.  OpenVSP can
now output trimmed surfaces to STEP and IGES files.  This can form a
watertight BREP solid!  This capability extends to the structures models
though those aren't technically BREP solid models.

This capability is very tricky to get right and different programs import
these files with varying degrees of success.  So, please try it out and
report any success or failure to the Google Group.  With your help, we can
make this feature the best it can be.

The second big change is the extension of the VSPAERO GUI to cover all the
features of the recently upgraded VSPAERO v6.  This includes built-in
support for unsteady rotors and props.  Analyzing your VTOL multi-rotor
has never been easier.

A bunch of propeller improvements -- new shape functions for axial and
tangential (alternatives to rake and skew) as well as precone.  Props
designed in a BEM code and imported often have blade curves with many
control points.  There is now a way to approximate those curves.  Also,
improvements to prop skinning when TE thickening and certain airfoil
types are used.

Lots more improvements across the board -- too many to detail here.

There are some things to keep an eye out for.  The default Set is now the
Shown set instead of All.  This change should be more intuitive for
general use.

The recently released smart edit feature relies on the std::regexp library
which has only been supported by gcc since version 4.9 (ca 2014).  So, if
your machine can only have a compiler older than that (you know who you
are), this feature is now conditionally compiled to support those
machines.

Many bug fixes  all around while we're at it.

This version has exciting features for just about everyone.  Pull it down,
try it out, and stay safe.

Features
 - Add export of trimmed CAD files for structure and OML
 - Update VSPAERO GUI to support recent improvements
 - Update VSPAERO to 6.0.2 see cc0786863868
 - Default Set is Shown instead of All
 - Mass properties includes HumanGeom
 - Calculate volume and density of HumanGeom
 - HumanGeom and MeshGeom support for DegenGeom via MESH_TYPE
 - Axial and Tangential blade shape control for propellers
 - Precone shape control for propeller
 - Approximate Cubic Bezier operation for propeller blade curves
 - Add simple Plot3D import
 - Change order of modifications in WireGeom - patch after skip and stride
 - Misc. additions to the API
 - Full CST support for Stack, BOR, and Fuselage
 - Expand Parm info in link screen
 - Improve link editing and highlighting
 - Resize parm link browser columns
 - Add current Geom name to Geom screen title bar
 - Smart edit conditional compilation to help on gcc <4.9

Library
 - Update LibIGES to fix NURBS curve problem
 - Update Nanoflann to 1.3.2

Fixes
 - Many improvements to surface intersection quality and robustness
 - Improvements to STEP/IGES output
 - Fix problem with prop surface waves due to thickness and airfoil mods
 - Improve scale independence of curve approximation
 - Fix problem with Python tools on Windows
 - Do not load variable preset on file load.  Only on Apply.
 - Account for vertical scroll bar width in CurveEditor
 - Fix airfoil extraction for BEM files.
 - Clean up fineness ratio use in drag buildup
 - Fix errors in Schmenesky form factor in drag buildup
 - Fix labels and table headings in parasite drag file
 - Fix CEDIT aspect ratio load from file
 - Silence compiler warnings
 - Fix colorchooser behavior when entering numbers
 - Fix for VSPAERO viewer with new MacOS and retina display


---


# [OpenVSP 3.20.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.20.1)

2020-01-20

OpenVSP 3.20.1 includes a few bug fixes.  Most importantly, one that fixes
a problem introduced in 3.20.0 that causes some models (saved in 3.20) to
open up wrong.  The other bug fixes are a bit more longstanding and subtle.
But there is nothing to be scared of -- everyone should update.

Bug Fixes:
 - Wing projected span as Parm did not have unique name.
 - Number of sets assumed in some GUI's not updated for recent expansion
 - Error in Bezier evaluation (Code-Eli)
 - Error in surface point projection (Code-Eli) will help intersection code


---


# [OpenVSP 3.20.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.20.0)

2019-12-28

OpenVSP 3.20 just in time for 2020!  Start off the new year with a new
version with one really cool feature (Smart Input) and a bunch of fixes.

Overall, this update is very small.  It wouldn't cause a .20 bump except
that the new feature also adds a new library dependency and that warrants
a little more attention.

You may be asking, "What is Smart Input?".  Input boxes (the white box
with parameter values) are now smarter.  They can now can do simple math
(+-*/^).

Maybe you want to nudge a parameter by 0.375 -- just click in the input
box and put '+.375' after the current value.  Maybe you want to double
something '*2', increase by 10% '*1.1', convert units on the fly '/2.54'.
This is a really handy feature that will find many uses.

In addition to Smart Input, there are a handful of build fixes, code
cleanups and improvements to the automation of the build process.

Features:
 - Smart Input for simple math in input boxes
 - API examples in documentation
 - Draw cg from mass prop screen
 - CEDIT improvements
 - Make wing section projected area an output Parm
 - Add get parent/child to API
 - Fix VSPAERO crash for Uber CRM example case
 - Lots of cleanups identified through static analysis
 - Travis deployment
 - Update Travis MacOS build to match release builds

Library Updates:
 - Include expparse simple expression parsing library

Bug Fixes:
 - Don't crash structures STEP/IGES output when no structure defined
 - Parasite drag for identically named geoms
 - DegenGeom crash when subsurfaces extended beyond [0,1]
 - CEDIT fixes


---


# [OpenVSP 3.19.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.19.1)

2019-11-17

Right on the heels of .0, 3.19.1 is mostly about tidying up a few loose
ends.  There are a few bug fixes and some documentation improvements.

Most significantly, if you're interested in using VSPAERO to analyze a
case with rotating blades, this version fixes an issue with DegenGeom
and also includes some example cases for you to try.

Not much for everyone else, but no reason not to have the latest either.

Features:
 - Add rotating blade examples for VSPAERO
 - Let vspviewer accept *.adb in file name passed, helps with double-click
 - Use less precision in M,A,B list from GUI to VSPAERO -- long command lines
 - Clean up some warnings in VSPAERO code
 - Update VSPAERO command line documentation

Bug Fixes:
 - Fix field ordering in DegenGeom file -- this broke VSPAERO
 - Fix problem with image backgrounds
 - Remove XS_BEZIER reference from Python test
 - Improve Ubuntu package to remove prior version
 - Fix PathToExe for FreeBSD


---


# [OpenVSP 3.19.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.19.0)

2019-11-09

This release comes with three highly anticipated new features:
VSPAERO 6.0.0, the Generic XSec Editor, and Doxygen-generated API
documentation. Dozens of smaller improvements and fixes are included as
well, so upgrading is highly recommended. A significant portion of this
effort can be attributed to ESAero, funded by the Air Force Research
Laboratory.

The latest VSPAERO jumps an entire major version, offering countless
improvements and fixes over 4.4.1. Significant effort was taken to
ensure cross-platform support, accuracy, and robustness. We encourage
anyone currently using VSPAERO to rerun models in 6.0.0 and report if
any issues are found. While the changes to the VSPAERO GUI are minimal,
users are free to use the command line interface to access all of the
new advanced capabilities. The most significant is the ability to handle
mulitple unsteady rotating bodies. In time, GUI support will catch up to
the latest capabilites.

The Generic XSec Editor allows for modeling of any arbitrary 2D cross-
section shape. For more details, see the wiki page available here:
http://openvsp.org/wiki/doku.php?id=genxsec.

The API documentation, automatically generated through Doxygen, has been
posted to the OpenVSP home page here:
http://openvsp.org/api_docs/3.19.0/. Users that compile
OpenVSP themselves can generate the documentation offline by building
the "doc" solution (requires the Doxygen executable).

Last, this release comes with a Debian package for Ubuntu (thanks Cibin
Joseph), along with Windows 64-bit binaries for users with models
approaching maximum memory limits. As always please report any problems
with the new features in this release.

Features:
-VSPAERO 6.0.0
    -Fully unsteady, time accurate analysis
    -PSU-WOPWOP coupling for noise predictions
    -Significant speed improvements
    -Support of rotors in hover through Vref
    -Improved vortex core model
    -Local Prandtl Glauert correction
    -Improvements to Karman-Tsien model
    -Remove broken vortex lift & LE suction
    -Better force and moment calculations
    -More viewing options for wakes
    -Lots of bug fixes
-Generic XSec Editor
-Automated API documentation
-Propeller blade curve editor GUI improvements
-API functions for VSPAERO control surface grouping
-Set transformation API functions
-MeshGeom color specification
-Lots of code analysis recommended cleanup and improvements

Bug Fixes:
-VSPAERO V&V script fixes
-Fix 0 chord length airfoil modifier crash
-Pinoccio build fixes
-Update MeshGeoms from MeshGeom GUI
-Fix crash caused by empty control surface groups
-Adds missing VSPAERO analysis inputs (i.e. Rho)

Other:
-Debian package for Ubuntu
-64-bit Windows binaries
-Migration of STEPCode to Libraries
-Update cpptest to 2.0.0


---


# [OpenVSP 3.18.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.18.0)

2019-09-02

One major feature, lots of tweaks, quite a few fixes.  This release comes
a bit before the Workshop (see you then).  Expect another release in the
reasonably near future with a bunch more features in the works from
ESAero.

In this release, OpenVSP finally gets a built-in human figure to act as
pilot, passenger, or whatever you need.  Although it isn't perfect, it
should be better than past attempts (sorry PodMan and Male90Pct) in just
about every possibble way.  Note that this component is implemented as
a mesh (not a Bezier surface), so you can't CFDMesh, FEAMesh, or export
it as STEP, IGES, or a few others.

Lots of user-requested tweaks -- more Sets (20 for now), different color
highlights for XSec editing, (near) eradication of non-adjustable sliders.

All good stuff -- but the reason to update is to improve the pilot/pax in
your models.  You know you want to.

Features:
 - New anthropomorphic pose-able human model
 - More sets by default (20)
 - Blue and green highlights for wing/fuse/stack editing
 - Convert non-adjustable sliders to adjustable
 - Improved Cpslice visualiation
 - Other small cleanups

Fixes:
 - Fix some API Error messages
 - Don't draw Prop XSecs when in disk mode
 - Fix memory leak


---


# [OpenVSP 3.17.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.17.1)

2019-07-07

Perhaps sooner than expected and perhaps less feature rich than expected.
This release is almost all bug fixes -- you'll certainly want to make
the jump.

Not a lot of details this time around -- I'll let the short descriptions
speak for themselves.

Features:
 - Calculate average and weighted chord and solidity for props
 - Expand API coverage
 - Improve CMake to help FLTK build on Linux
 - Add names to exported STEP files.

Library Update:
 - Update GLM to 0.9.9.5

Fixes:
 - Fix skin friction equations
 - Multiple FEA GUI Fixes
 - Don't assume FEA skin is first component
 - IGES Structure Parm Name Collision (Thanks Jim Fenbert)
 - STEP Structure missing options
 - Overlapping Prop XSecs
 - Register missing attachment enums w/ API
 - Protect 4-dig airfoils from out of bounds parms
 - Improve ResetRemapID behavior
 - Fix problems with prop and file airfoils
 - Add missing GUI handling of read-file XSecs for body of revolution
 - Silence a bunch of warnings
 - Use equal arc len parameterization for super ellipses


---


# [OpenVSP 3.17.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.17.0)

2019-03-28

Once again, it has been about half a year since the last release -- that
seems to be about the cadence these days.  This version includes some cool
features and fixes some longstanding bugs.  Hopefully it will be a good
step up for everyone.

conformalOpenVSP to CHARM export scripts.  This release includes a set of Python
scripts that will generate the geometry input files to run CHARM - the
comprehensive rotorcraft analysis tool.  These scripts debuted at the
OpenVSP/CHARM/PSU-WOPWOP eVTOL acoustics short course at the VFS
conference earlier this year.

Transparent screenshots - we now export screenshots with the background
made transparent (no more off-white box screaming VSP).  Even translucent
components maintain their translucency.

Cutting planes that define a structure can now be exported as STEP and IGES
files.  Note, these are accessed through the main File-Export menu, not
the FEAMesh menu.

Improvements to STL export for unintersected geoms with better tags.  Also
allow export of propellers relocated at the origin -- these options were
preferred for some CFD workflows.

Support design lift coefficient for four-digit airfoils.  The design lift
coefficient is now calculated -- and the option exists to set the camber
to achieve a desired design lift coefficient.  This brings a more useful
and modern parameterization to the old classic airfoil set.

Lots of propeller improvements -- Instead of discrete airfoils at stations,
propellers can now be defined in terms of distributions of
thickness-to-chord and design lift coefficient.  This will result in
smoother propeller lofts better able to match design intent (and to
recreate the geometry of many published props).  Also calculate integrated
design lift coefficient for props (already calculates activity factor).
Add control of feather axis location and feather offset - should help with
modeling rotors and some novel prop designs.  Also add the option of a disk
model for every propeller.

Faster surface evaluation -- a few improvements to the way surface points
(and normals) are evaluated to speed refreshes.  OpenVSP should be
noticeably faster feeling.

Two really significant bug fixes (along with a bunch of smaller ones).
The 'Unexpected ResetRemapID' warning with models including conformal
geometry is fixed.  This should also help fix the destruction of links
when opening models.

The file corruption bug where the decimal separator was switched to a
comma has been fixed.  This only happened for users with a non-default
locale, who saved the VSP file after exporting an IGES file.  It was a bug
in the IGES library.  Sorry for the pain this one has caused.

Overall, this should be a great release -- and perhaps the only one for
six months or so.

Features:
 - CHARM Python export scripts
 - Transparent screenshots
 - Border toggle in menu
 - Improved show axis toggle
 - Export structure surfaces to IGES and STEP
 - Design lift coefficient support for four-digit airfoils
 - Propeller definition by thickness and CLi distribution
 - Calculate integrated design lift coefficient for props
 - BEM files in terms of thickness and CLi distribution
 - Feather axis and offset control for propeller
 - Implicit disk model for propeller
 - STL solid names include tag number
 - Tag unintersected triangles for STL export
 - Increase tessellation limits
 - Option to export props at origin (unintersected STL and STEP/IGES)
 - Batch evaluation of Bezier surface points and normals (faster refresh)

Library updates:
 - FLTK to 1.3.5
 - Eigen to 3.3.7
 - LibXml2 to 2.9.9
 - LibIGES to version that respects locale
 - Code-Eli to version that supports latest Eigen and other improvements.

Fixes:
 - Restore visualization of CFDMesh sources and wake
 - Don't crash when BOR XSec changed to Bezier
 - Don't crash when reading Lednicer airfoils
 - Import v2 multi-section wings properly
 - Make key-corner default for rounded rectangle
 - Reverse props with shifted construction line built correctly
 - Don't allow insertion of XSec at negative index from API
 - Don't allow comma as decimal point marker; fixes file corruption.
 - Fix writing excressence drag label to XML file
 - Fix unexpected ResetRemapID in files with conformal geometry


---


# [OpenVSP Version 3.16.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.16.2)

2018-10-28

This version has been long in coming -- it doesn't really contain anything
monumental -- but enough small things built up to make this release
overdue.

The most significant geometry changes are some new airfoil types --
NACA 4-digit modified, 5-digit, 5-digit modified, and 16-Series.  Also
easy GUI access to add nose/tail end caps to Fuse and Stack.

Lots of improvements to the API -- picking up things that were left out
or were inconsistent between AngelScript and C++/Python.  It isn't perfect,
but it is better.

A crude animation capability has been added to the API.  This includes
some example scripts to get users started.  Animation scripts must be run
from inside the GUI.

Beyond that, a ton of small fixes across the board.

Features:
 - DegenGeom additions
 - API Animation support
 - Four digit modified airfoils
 - Five digit & modified airfoils
 - 16 series airfoils
 - Fuselage and stack end caps
 - BOR support in API
 - General API cleanups

Bug Fixes:
 - Many small fixes
 - Fuse file xsec symmetry problem
 - Fix point clustering on airfoil export


---


# [OpenVSP Version 3.16.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.16.1)

2018-03-26

Bump version to .1 for quick propeller parm fix.


---


# [OpenVSP Version 3.16.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.16.0)

2018-03-25

This version is long overdue, but it has lots of good stuff -- should be
well worth the wait.  Not much of a theme here, but bits and pieces all
over the place.

We're in the process of migrating the www.openvsp.org web server.  If the
site goes down in the next few days, don't dismay, it'll come back soon.
Hopefully this doesn't cause problems with obtaining this version.

VSPAERO gets a big update to version 4.4.1.  The binary file format has
changed, so you must use the new viewer and slicer.  There is also a
VSPAERO test script that has received a lot of work.

New export types/formats -- airfoil stack, simple PMARC 12 files, OBJ from
MeshGeom.  DegenGeom now available from API.

To round it out, some propeller modeling improvements, and significant
speedups to the improved intersection curves recently released.

Features:
 - Airfoil stack export
 - Simple PMARC 12 export
 - MeshGeom OBJ export
 - DegenGeom support to API
 - Structures small improvements and bug fixes.
 - Propeller modeling improvements (sweep and construction line)
 - Update VSPAERO to version 4.4.1
   . ADB file format change.
   . Supersonic solutions should work again.
   . New surface pressure calculation method
   . Fixed a 2 floating around in the unsteady aero forces
   . Fixed a bug in the time accurate wake code.
   . Steady state rotor calculations.
   . UnSteady rotor calculations.

Bug Fixes:
 - Faster Intersection Curves
 - Many fixes to VSPAERO test script


---


# [OpenVSP Version 3.15.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.15.0)

2017-12-31

Happy New Year!  And Happy New OpenVSP!  One significant new feature and
a bunch of fixes and cleanups make this release worthy of a .0 milestone.
I hope to see you at SciTech or AHS this year.

The new feature is support for stand-alone calculation of intersection
curves (without running CFDMesh).  The curves can be visualized and
exported to GridTool and Plot3D formats.  These changes also afforded an
audit of the surface intersection code -- resulting in many improvements.

These improvements should significantly improve the quality of the curves,
which will hopefully improve the robustness and quality of the CFDMesh and
FEAMesh processes.

Unfortunately, the improved algorithms are potentially noticeably slower.
If you notice CFDMesh pausing for a longer period during 'Intersect', this
is why.

There are a bunch of changes to the organization of the Structures data.
This should make all of the Parms available through the API or as design
variables.  However, this will also likely mean that structures currently
saved to a *.vsp3 file will not read into versions going forward.

We try to avoid this sort of thing, but this one slipped by -- very sorry
for the inconvenience.  However, I am certain that the best time to fix
this issue is now.

The VSPAERO GUI has been extended to support unsteady stability runs.

There are a handful of bug fixes.  Most significantly, a bunch of fixes
related to saved views, the openness of the adjust view GUI, and the
preset views from the View menu.

Overall, lots of good stuff.  Download right away.  Enjoy in moderation.

Features:
 - Stand-alone curve intersection generation, visualization, and export
 - VSPAERO GUI integration of PQR unsteady stability stuff

Bug Fixes:
 - Structures data organization to make Parms available to API
 - View adjust bugs


---


# [OpenVSP 3.14.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.14.2)

2017-12-16

Mostly a bug fix release, but a few treats thrown in for good measure.

This release fixes a bunch of issues that cropped up in 3.14.  So far, no
problems have been reported that relate to the structures support in .1,
so only one release will be made here.

In addition to the fixes, two 'features' are included.

First, we're including pre-built Python bindings!  Nothing here that
you couldn't do yourself before, but now that we're doing the heavy
lifting, a bunch more people should be able to Python their OpenVSP
to their hearts content.  These bindings will only work with the
Python they were built against.  These were built against Anaconda3.6.
(The 32-bit version on Windows.)

Second, there is an update to libIGES that allows naming of the exported
surfaces.  We use Entity 406 Type 15 names -- this allows names to be
of arbitrary length.  You might want simple names derived from the
component names -- or you might want something guaranteed unique that
identifies all split surfaces with their parent - and symmetrical copies
with their master.  The IGES export GUI now has a bunch of optinos to allow
control of the name fields and delimeter.

The MacOS release has been built with a new compiler on a newer machine,
please report any improvements or problems.

The fixes are important too, a couple of nasty ones in there.  You'll want
to update.

Features:
 - Python bindings now included - Anaconda3.6 based.
 - IGES surface names

Bug Fixes:
 - Parasite drag SWet showing as zero (thanks Corrado)
 - Fix crash when pasting components to top level (thanks Jasper)
 - Fix VSPAERO GUI layout problems (thanks Marco)
 - Fix problem reading line subsurfaces from file (thanks Nat)
 - Scattered other fixes


---


# [OpenVSP 3.14.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.14.1)

2017-12-07

Two servings of OpenVSP Pi on the same day!

This release is very exciting and it contains just one thing - the
long-awaited structures modeling and meshing capability.  OpenVSP v2 had
a structures layout capability that only worked for wings.  That wasn't
brought forward to v3 because we'd learned enough from that and wanted
to do it better.

Shout out to Justin Gravett who did most of the work to make this happen.

All the lessons from v2 wing structures have been applied and this new
capability should be an exciting step forward.  It supports all component
types (including custom geoms).  It supports multiple structures per
model (and even multiple per geom).  It supports many more structure parts
- full-depth ribs/spars/bulkheads/frames/floors, zero-depth beam elements,
key points, and pressure domes.

If you like structures, dive in.

The structures update is a huge set of changes that touches many parts of
OpenVSP (including CFDMesh).  Just in case it breaks something that usually
works, we've separated the 3.14.0 and 3.14.1 releases.  We encourage
everyone to use 3.14.1 -- but if something is broken that you need, you can
try 3.14.0 to see if it works there.

This structures capability is a huge set of changes.  It has been in the
works for a long time and was previewed at the OpenVSP workshop.  There are
likely issues that we haven't found.  Help us make this the best we can.

Features:
 - OpenVSP Structures Capability


---


# [OpenVSP 3.14.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.14.0)

2017-12-07

'Tis the season for OpenVSP Holiday Pi.  Partake heartily.

It has been far too long since the last release.  This one started out
small, but then had to wait for some things to finish.  While waiting,
it grew.  Then it had to wait on something else -- and it grew more.  Now
that it is finally out the door, it is no longer a small update.

Lots of great features and bug fixes for both VSPAERO and OpenVSP.  Too
many things to highlight just one or two.  Download the latest, check out
the release notes, and enjoy.

Features:
 - VSPAERO Update to 4.3
  . Matrix based preconditioner is now default
  . Vortex lift on by default
  . Leading edge suction a separate option
  . Option to disable 2nd order Mach correction
  . Multiple Re support for CDo (not supported yet by OpenVSP GUI)
  . Some small changes to calculating the surface pressures on the body.
 - OpenVSP updates to support VSPAERO 4.3
 - CPSlicer GUI
 - VSPAERO Automated Test Script
 - SubSurface API Extensions
 - Improvements to DegenGeom representation of control surfaces
 - Wireframe import of *.hrm files for CompGeom/DegenGeom based analysis
 - Measure, point probe in GUI
 - Surface query API routines
 - U,W Line Sources for CFDMesh
 - Some outputs now handled as Parms and can be used in linking.
 - Defer Update() in CreateGeom

Bug Fixes:
 - First (instead of last) wake iteration plotted in VSPAEROPlotScreen
 - Fix VBO buffer size and increment to prevent crashing on complex models
 - Scale rounded rectangle corner radius.
 - Scale root section of multi-section wings.
 - Remove dangling panel method mesh every time VSPAERO is initiated.
 - Fix counter-intuitive behavior pasting components onto hinges.
 - Fix hinge GUI name update.
 - Misc. other bugs
 - VSPAERO 4.3 Bug fixes
  . Fixed a bug with forces/moments for propulsive cases - vlm and panel
  . Various other bug fixes..


---


# [OpenVSP Version 3.13.3](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.13.3)

2017-09-16

Lots of fixes, a few features, and some tweaks all around.  Much of this
came up at the workshop -- too bad you missed it.

VSPAERO is updated to 4.2 -- this version includes a leading edge suction/
vortex lift model as well as a bunch of fixes.

I'm also updating the compiler used to build the Windows releases.  It may
make some things faster, but it may also cause problems on some old
machines.  Please report any problems.

Features:
 - VSPAERO 4.2
  . Vortex lift model / leading edge suction model integrated.
 - DegenGeom enhancements
  . LE radius, thickness, and location from curve not mesh
  . Handle prop like wing in DegenGeom
 - Change Prop blade duplication to speed updates
 - Add invert airfoil for VKT airfoils
 - Add UpdateGeom to API to facilitate rel vs. abs positioning in scripts
 - Allow control surfaces to be in multiple groups
 - 'AJ' usability improvments / fixes
  . Edit min/max value of User Parms
  . User Parm adjust screen wider for long names
  . Advanced link screen wider for long names
  . Fix odd _0 appended to user parm group names
  . Sort and divide user parm adjust tab by group
  . Clarify parm linking GUI
  . Sort parm links

Bug Fixes:
 - VSPAERO 4.2
  . Ground effect, wakes and CG location
  . Various updates and fixes in viewer
  . Control surface fixes - multiple controls on a single wing
  . Large control surface deflection improvements
  . Changed calculation of CpMin
 - BOR diameter was radius
 - Make BOR parms linkable
 - Initial position of children of hinges
 - Parasite drag crashing with blanks
 - Cf equation inputs and defaults
 - Cf calculation for mixed laminar/turbulent flow


---


# [OpenVSP Version 3.13.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.13.2)

2017-08-28

Just in time for the workshop an update with some features and lots of bug
fixes.

This adds a built-in body of revolution -- in case the Duct custom
components were not enough for your needs.  Also includes Spin control for
fuselage and stack geoms.

Lots of updates to VSPAERO -- ground effect and some viewer updates, but
mostly a whole host of bug fixes for issues that people have been running
into.

Plenty of good stuff here.  Everyone should update.

Features:
 - Body of revolution component
 - Spin control for fuselage and stack
 - VSPAERO Update to 4.1.1
  . Added ground effect model
  . Reflect symmetric half geometry in viewer
  . Show XYZ planes in viewer

Bug Fixes:
 - Clean up logic in STEP/IGES export - may help some importers
 - VSPAERO GUI now creates half-mesh for panel method with symmetry
 - VSPAERO Update to 4.1.1
  . Fix drag integration inconsistency for alpha vs. rotation
  . Fix memory crash w/ periodic wakes (ducts)
  . Fix pressure anomaly w/ periodic wakes (ducts)
  . Fix problem with symmetry calculations
  . Fix high AR Cp near centerline
  . Fix high AR Cp out span
  . Fix slicer on Windows
  . Set minimum Mach to 0.001 -- fixes Cp min/max range in viewer


---


# [OpenVSP Version 3.13.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.13.1)

2017-08-02

This release is all about bug fixes and cleanups related to the recent
VSPAERO updates and GUI improvements.  Most of these problems have been
encountered by users -- so update and don't have the problems.

Features:
 - Remove temp num points to 1 for VSPAERO calculations

Bug Fixes:
 - Correctly visualize control surface within VSPAERO
 - VSPAERO will read wings with any amount of subsurfaces correctly
 - Fix crash caused by out of bounds index for rotors in VSPAEROScreen
 - Fix build problems for FreeBSD
 - Fix case problems with #includes


---


# [OpenVSP Version 3.13.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.13.0)

2017-07-22

This release is mostly about updating VSPAERO to version 4.1.  This
includes an unsteady formulation for control derivatives and some general
speed and robustness improvements.  The GUI integration of VSPAERO has
been updated to match v4.1.

New geometry modeling options in this version include ellipsoids and
von Karman Trefftz airfoils and CST airfoils are now available for
Fuselage and Stack.

In addition, lots of bug fixes all around.  Too many to list to VSPAERO
integration and the parasite drag buildup tool.  In addition, a tricky
bug in IGES/STEP export and a really old bug that nobody has ever noticed.

All around, you'll certainly want to update.

Features:
 - Update VSPAERO to v4.1 -- unsteady formulation and lots of fixes
 - Update VSPAERO integration to support v4.1
 - Add ellipsoid geom type
 - Add von Karman-Trefftz airfoil type
 - Add CST Airfoils to Fuselage and Stack

Bug Fixes:
 - Fix bugs in parasite drag
 - Fix split surface problem for IGES/STEP export
 - Fix VSPAERO rotor induced velocity terms
 - Fix VSPAERO integration issues
 - Simplify file insert to avoid crashes
 - Fix bug where DisplayNames sometimes don't get updated


---


# [OpenVSP Version 3.12.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.12.1)

2017-06-19

This quick bug fix release takes care of a bug caused by using different
compilers for release from development.

Our Windows developers use VS2015, while the releases are prepared with
VS2010.  VS2010 doesn't support some newer features that were used.


---


# [OpenVSP Version 3.12.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.12.0)

2017-06-17

This version of OpenVSP does much to ease use of VSPAERO -- especially for
complex cases.  You no longer need to edit/tweak the setup file yourself,
everything is controlled from the GUI.  Generating the DegenGeom file has
been streamlined, so you don't have to explicitly write the geometry
either.

Visualization of the thin-surface VLM geometry has been added, this should
make it easier to understand how VSPAERO will interpret your geometry.

The parasite drag buildup tool is finally included.  It allows the user to
conduct a traditional form-factor/wetted area based drag buildup.  Many
options are available to match your preferred methods.

Control surface subsurfaces have been enhanced to produce more realistic
shapes matching designers intent.

Don't forget the OpenVSP Workshop Aug 30-Sept 1.

Expect 3.13.0 soon.  It will include VSPAERO 4.1, incorporating bug fixes
and an unsteady mode.

Features:
 - Parasite drag buildup tool
 - Full control of VSPAERO from OpenVSP GUI
 - DegenGeom preview visualization
 - Advanced control surface modeling

Bug Fixes:
 - Advanced links not loading from file with 'Loop' policy geoms
 - Crash conformal components with some file airfoils
 - CompGeom when subsurfaces fall on tessellation line
 - DegenGeom handling of tip caps with blended wings
 - CustomGeom name matching GUI mismatch


---


# [OpenVSP Version 3.11.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.11.0)

2017-03-08

This release is one of the most feature-packed updates yet.  It includes
two long-requested features that will transform how OpenVSP is used.  There
are another half dozen smaller updates that would justify the version
number bump on their own.  There is something here for everyone -- you are
going to want to update.

Blended wings are finally here.  OpenVSP v2 had a BWB component that I was
never happy with for many reasons.  It is one of the things that didn't
make the cut for v3 because I didn't want to support an ugly hack forever.
There have been two significant attempts at blended wings in the v3 era.
The first was based on early skinning and was a total disaster.  The second
brought us slew control in skinning; while improved, it mostly proved
that a skinning-based approach wouldn't work.

Wing blending works by giving the user enhanced control over the leading
and/or trailing edges of a wing section.  Instead of a straight line, the
LE/TE can become a smoothly varying curve.  Through suficient application
of magic, the rest of the airfoil follows along.

The most simple way to modify a LE/TE is to change its local sweep or
dihedral angle.  You can also set a LE/TE direction to 'match' an
adjoining wing section.  These options should enable the user to quickly
build their design intent into a blended wing with a minimum of parameters.
A brief video tutorial on the blended wing component is available to
get you started https://youtu.be/bGRYZr7rS94 .

Conformal components are an entirely new addition to OpenVSP.  Where most
OpenVSP components' shape is independently defined, conformal components
derive their shape from their parent -- they conform to their parent.
This makes them ideal for modeling fuel tanks and payload bays.  Users
will soon find myriad application including layout and mass properties
problems.  To create a conformal component, select the desired parent
component and 'Add' a conformal component.  Then adjust the offset and
U/W trim as desired.

Other new features include XPatch export of triangle meshes and SVG export
for 2D drawings.  CompGeom (and related tools) should be faster in one
step.  CompGeom now calculates and reports the area of subsurfaces.

Lots of little bugs were fixed too -- but if you weren't convinced to
update by now, I doubt they'll do it for you.  Just go ahead and update,
trust me.

Features
 - Blended Wings
 - Conformal Components
 - XPatch export
 - Improved DXF 2D export (outlines)
 - SVG 2D export
 - SubSurface areas in CompGeom
 - CompGeom faster
 - IGES/STEP surface splitting along U/W-Const SubSurface lines
 - Mesh STL files can write surfaces and/or slices

Bug Fixes
 - WaveDrag set choice glitch - thanks Alex Ziebart & Mike Raber
 - Set editor conflict
 - CFDMesh with Shown set
 - Update CFDMesh when meshing fails
 - Prop curve indexing problem when enforcing order (possible crash)
 - WaveDrag reference area flag Parm in wrong group
 - Set VSPAERO analysis method from API
 - CFDMesh threading console update
 - Highlight Selected Set in Set Editor


---


# [OpenVSP Version 3.10.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.10.0)

2017-01-07

Just in time for SciTech, this release has been a long time coming.  A few
things had to be dropped at the last minute and there are a bunch more
features pending in the wings...  So expect more exciting releases in the
near future.

There are no huge features this time, but enough significant changes to
warrant the bump to .10.  The FLTK graphics library has been updated to
1.3.4-1, the first stable FLTK release in years.  The Angelscript scripting
language has also been updated to the lastest version.

The clustering function for wings and propellers (LE/TE and root/tip) has
been changed to one used widely by structured meshers. It gives more
gradual point spacing growth resulting in much higher quality surface
meshes.

The minimum clustering parameter is now 0.0001 instead of 0.0.  Such a
small value will likely be too small.  You will need to play with existing
models to get satisfactory results.

I've done some profiling to identify some slow spots and have made
changes to speed things up.  Hopefully it is noticible, but results may
differ by platform.

There is also now an option to omit blunt airfoil TE panels from IGES/STEP
surface export -- I won't name names, but users of a particular CFD meshing
tool have asked us to add this option to make our surfaces less watertight.

Features
 - Improved tessellation clustering function
 - Tessellation spacing quality metrics
 - Option to omit blunt TE patches from STEP/IGES surface export.
 - CFDMesh faster
 - FitModel faster
 - Propeller, wing, & fuselage components faster
 - Move CFDMesh to separate thread to improve console updating

Bug Fixes
 - VSPAERO integration fixes
 - CloseTE tolerance scales with chord to fix behavior on small models.
 - Tighten degenerate curve test in tesselation and surface export.
 - Fix PrintAnalysisInputs and PrintResults API functions
 - Fix bug editing propeller spline points


---


# [OpenVSP Version 3.9.1 with VSPAERO 3.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.9.1)

2016-09-05

This release is all bug fixes.  Some of the bugs are longstanding, but most
relate to the big VSPAERO and integration update in 3.9.0.  Everyone
should update.

Bug Fixes:
 - Fix bugs with unenforced limits of propeller curves
 - Fix feather/collective angle for reversed propeller
 - Fix false camber for blunt TE/LE airfoils in VLM VSPAERO analysis
 - Fix problem where custom materials got re-written to file on insert
 - Fix error writing supersonic VSPAERO results to file
 - Fix various errors with VSPAERO integration


---


# [OpenVSP Version 3.9.0 with VSPAERO Version 3.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.9.0)

2016-08-21

Just in time for the Workshop, this release has something for everyone.
VSPAERO, Hinges, Group Transform, AutoCAD Export, and bug fixes.

VSPAERO v3.0 is the most significant update to the VSPAERO solver to date.
It includes a thick-surface panel method, control surfaces in VLM mode,
built-in batch runs, improved speed, and more.

The VSPAERO integration into OpenVSP has been updated to support the new
version of VSPAERO.  API support also updated.

A one-axis kinematic joint has been added.  This is like a blank component,
but that can translate or rotate.  Its children are forced to be attached
to the joint.  This component simplifies modeling control surfaces,
landing gear, folding wings, and similar common major assemblies.

Group transformation/modification has been added.  Select a group of
components and you can translate/scale them as a group.  You can also
change their material or color, or rotate them about their own origins.
This is one of the last v2 features that was missing in v3.  Thanks much
to Alex Gary for getting this going.

Feature lines can now be exported as an AutoCAD DXF file.  This traditional
drafting file can be imported into just about any Drawing or CAD program
as a starting point for a general arrangement or inboard profile drawing.
Welcome ESAero intern Justin Gravett to the development team.

Of course a few bug fixes are thrown in too.  Mostly some API updates that
broke Python or introduced features that never worked correctly.

Features:
 - VSPAERO v3.0 - Panel method, batch runs, control surfaces, much more
 - Improved VSPAERO integration
 - One-Axis Kinematic Joint (Hinge/Slider)
 - Group Transformation/Scale -- Thanks Alex Gary
 - 2D AutoCad Export of feature lines

Bug Fixes:
 - Change to Geom type name lookup instead of type ID number
 - Fix Python API
 - Fix VarPresets API to built-in scripting


---


# [OpenVSP Version 3.8.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.8.1)

2016-08-01

Propellers for all!

This release adds a new propeller component and fixes a pair of unrelated
bugs.  Though it may seem like a small change, the propeller component
constructs its surface differently from other components.  It represents
a surprisingly large amount of work.

Instead of defining the blade shape (chord, twist) at specified stations,
these are instead specified as continuious curves.

The propeller component comes with a bunch of related accessories -- the
blade activity factor is calculated on-the-fly, there is a one-click option
to switch to a counter-rotating prop, propeller folding is built-in -- with
a user-defined fold axis, there is a blade element export/import
capability, and the BEM data is available through the API.

Features:
 - New propeller component
 - Blade element import/export and API

Bug Fixes:
 - Fix DegenGeom crash on Matlab export
 - Fix GUI output of mass properties calculation


---


# [OpenVSP Version 3.8.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.8.0)

2016-07-15

Don't forget to register for the OpenVSP Workshop (Aug 23-25)!

This version is more focused than many of our releases.  It is really
all about upgrading the VSPAERO integration (and one bugfix).

The VSPAERO GUI has been comprehensively updated.  You can now run alpha/
beta/Mach sweeps from the GUI.  Results are automatically parsed and are
ready for review from the GUI.  VSPAERO is now available through the
Analysis Manager in the API.  The results are parsed and are available
through the Results Manager in the API.

In addition, a bug fix for files saved since version 3.7 that use super
ellipse cross sections.

Features:
 - Overhaul of VSPAERO integration
 - VSPAERO alpha/beta/Mach sweeps from GUI
 - Visualize VSPAERO results in GUI
 - VSPAERO available through API
 - Changed default wing LE/TE clustering to 0.25
 - Build on OSX on Travis-CI
 - Improve automated testing

Bug fixes:
 - Duplicate parameter name in superellipse cross section


---


# [OpenVSP 3.7.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.7.2)

2016-06-24

Another quick couple of bug fixes associated with the variable presets
capability.  Not quite as critical as the 3.7.1 fixes, but everyone should
update nonetheless.

Bug Fixes:
 - Fix crash when deleting empty var presets group


---


# [OpenVSP 3.7.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.7.1)

2016-06-24

This release fixes a major bug that slipped into 3.7.0.  Everyone must
update to this version.

Bug fixes:
 - Fix crash caused by reading file without VarPresets field.


---


# [OpenVSP Version 3.7.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.7.0)

2016-06-23

This version includes two significant new features that have been in the
works for some time.  These are the estimation of supersonic wave drag
by the area rule and a capability to save sets of variable settings in a
given model.

The wave drag capability has been much requested and long awaited.  Since
its demonstration at the 2015 Workshop, a number of slick improvements have
been made.  Thanks to Michael Waddington for all the work and to Sriram
Rallabhandi for the C implementation of the Eminton Lord calculations.

The variable preset capability has a number of use cases; it allows you to
save the model state and return to that state quickly and easily.  Perhaps
you have a model with takeoff, landing, and cruise configurations.  Perhaps
you adjust the tessellation for VSPAERO, but want different resolution for
mass properties, and different again for Cart3D.  Or, if you're doing a
design study, you may want to save the baseline model and different
optimal solutions (weight optimal, fuel optimal, cost optimal, etc).
Thanks to Bryan Schmidt for all the work on this feature.

Features:
 - Wave drag estimation by area distribution and Eminton Lord calculation
 - Variable presets for saving model state


---


# [OpenVSP Version 3.6.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.6.2)

2016-06-10

A small bugfix update just in time for AVIATION.  This release fixes
three problems that have bitten users.  Get it now and don't get bit!
See you in DC.

Bug Fixes:
 - Projected area direction setting from API.  Thanks James Haley
 - Crash in 3.6.1 with zero-width cross sections.
 - Crash exporting split-surface IGES files with degenerate patches.


---


# [OpenVSP Version 3.6.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.6.1)

2016-05-29

One big feature, a handful of enhancements, and a bunch of bug fixes make
this version well worth the update.

The big feature is a projected area calculator.  Use it to calculate
the projected area of a component or a set of components.  Project in the
X, Y, or Z directions -- or along an arbitrary vector -- or along a
vector normal to the u=0, w=0 point on a given component.  You can also
bound the projected area by a component or a set.

The resultant area is triangulated and added to the model as a MeshGeom for
visualization or export to STL.  Although only the final area is reported
in the GUI, component areas and the area outline are available in the
output file or via the API.

So, whether you want frontal area for a drag estimate, rotor downwash area
for a download calculation, or you just want the projected area of a
complex wing made of multiple components -- you're in business.

The super ellipse and rounded rectangle cross section types have been
enhanced to creation of a bunch more related shapes.  Thanks to Frank
Zhang for those enhancements.

The bug fixes include some old and some new.  All good stuff.

Features:
 - Projected area calculator with many use cases
 - Enhanced superellipse curve type - Frank Zhang
 - Enhanced rounded rectangle curve type - Frank Zhang

Bug Fixes:
 - Protect against NULL pointer dereference from API or Custom Components
 - Fix crash with airfoils trimmed at barely different spots (thx Jason W.)
 - Fix longstanding bug with handling of rounded corners for General XSec
 - Fix deactivated sliders not updating
 - Set unused capping parameters to value of least surprise


---


# [OpenVSP Version 3.6.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.6.0)

2016-05-05

It has been a while since the last release, I'm sure you'll find that this
version has lots of things to make it worth the wait.  There are a lot of changes here--and there may be some lingering bugs.
Please dig in and report success/failures to the Google Group.

Many of the embedded libraries have been updated, so if you compile
yourself, you'll need to refresh your build.

SnapTo has been added.  This feature lets you adjust any parameter until
the part snaps to a collision (or offset) with another part.

Leading edge modifications options matching the trailing edge modifications
have been added.  Likewise, leading edge control surface subsurfaces.

Multiple kinds of edge treatment have been added (not just flat).  So, if
you ever wanted to make a diamond airfoil with finite-radius LE/TE and
an edge wingtip, now you can.

Plenty more features - and too many fixes to detail.  The only logical
step is to download it and give it a try.

Features:
 - SnapTo, drive parameters by component collision/offset
 - Thicken airfoil LE like TE
 - Trim airfoil LE like TE
 - Round, edge, and sharp LE and TE closeout
 - Round, edge, and sharp wingtips (incl. v2 import round tips)
 - Option for rectangle XSec corner parameters to match
 - AnalysisMgr to streamline API additions
 - Leading edge control surface subsurfaces for wings
 - Write X,Y,Z intersection curves to SRF file for GridTool users

Bug Fixes:
 - Make all parameters linkable
 - Flip normal vectors for Plot3D as appropriate (thanks Joe Derlaga)
 - Many and various build fixes (thanks Carmine, fernape, George, et. al.)
 - Fix 0/1 subsurface indexing in DegenGeom for Matlab
 - Updates to Python wrapper (thanks Santiago Balestrini-Robinson)
 - Fix handful of OpenGL errors
 - Fux null pointer dereference (thanks Richard Harrison)
 - Fix flaky drag-n-drop in FitModel


---


# [OpenVSP Version 3.5.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.5.2)

2016-02-13

Another collection of small fixes and features, with one or two that are
sure to be crowd pleasers.  Get it now and don't be left out!

Most of the changes fall into two categories.  Sub-Surfaces and the main
GUI.

The first sub-surface work allows multi-surface geoms (common with custom
components) to assign sub-surfaces on a per-surface basis (rather than to
all surfaces).

In addition, sub-surfaces are now output to DegenGeom files.  This kind
of extension of the DG file format was always expected, so hopefully
parsers have been written to 'just work'.

Sub-surfaces are output as a u,v polyline, it is up to the DegenGeom user
to figure out what that means for their stick or plate representation.
Sub-surface names are also written out as well as a flag indicating which
side of the polyline is the 'inside' of the subsurface.

The main window has been refactored such that it is now generated on the
fly.  This will make future updates much easier.  While I was there, I
made a bunch of other improvements and cleanups.

The Undo button is now located under the Edit menu, and Ctrl-Z
(Cmd-Z on Mac) will trigger Undo.  Lots of menu commands now have sensible
shortcut keys that make common actions easier and also document some
little-known capability.

On Mac, the main menu bar now appears in the top window bar like other
native applications.  This may take some getting used to.

If you're still looking for a reason to update, here it is.  With this
version, a press of the 'f' key will fit the visible model (zoom and
center) to the screen (and change the rotation center to the center of
the visible model).

Features
 - Per-surface sub-surfaces
 - Sub-surfaces written to DegenGeom files
 - Programmatic main window
 - Fit visible model to window with 'f' key
 - Menu rearrangement with lots of shortcut keys

Bug fixes
 - Fix CFDMesh problem with vertical tails (thanks Carmine Vassallo)
 - Fix problem passing shortcut keys to all listeners


---


# [OpenVSP 3.5.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.5.1)

2016-01-23

This version has lots of fixes and a handful of features.  There is
something here for everyone, so update -- you don't want to be left out.

The addition of tessellation clustering control for wings (chordwise and
spanwise) will be important to lots of users.  This only affects the
wireframe and things related to it (DegenGeom, VSPAERO, CompGeom, etc).
It does not affect the underlying shape or the surface parameterization.

To do this, NACA 4-digit and CST airfoils types had to be re-parameterized
to an equal-arc-length basis.  This will change loft with mixed airfoil
types and possibly attachment points.  An option to recover the old
behavior is included.

Cluster is controlled with two parameters (LE/TE or Root/Tip).  If you
set them =1.0, you get uniform spacing (the default).  Smaller numbers
cluster, larger numbers spread things out.  Feedback on the level of
control and the growth rate is appreciated.  This may need tweaking down
the road.

The libJPEG library has given us no end of trouble over the years and I've
finally had enough.  I ripped it out and have replaced it with a smaller
and simpler library.  Hopefully this fixes all the problems users have
had with image files.  This will be mostly user invisible, but you can
no longer do JPEG screenshots -- PNG only.  Also, you now have your choice
of image formats for background images (JPG, PNG, GIF, BMP, TGA).

Features
 - Chordwise tessellation clustering control
 - Spanwise tessellation clustering control
 - Equal arc length skinning for four digit and CST airfoils
 - Add selection Show Only button.  Suggested by Jason W.
 - More file formats supported for background (jpg, png, bmp, gif, tga)
 - Make XSec highlight curves adaptive
 - Improve polygon offset to improve feature line rendering

Fixes
 - Fix update custom components change position/rotation. Thanks Erik O.
 - Fix problem with wing list update in VSPAERO GUI
 - Fix recent problems with logslider
 - Fix fuselages with many file XSecs
 - Fix O(n^2) problems in fuselage import and read
 - Get rid of libJPEG


---


# [OpenVSP Version 3.5.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.5.0)

2016-01-02

A little late for New Years, but just in time for SciTech!  I hope to see
you in San Diego next week.

OpenVSP 3.5.0 has a bunch of features across the board that should improve
usability for a lot of you.  High on the list is the ability to import
files from OpenVSP v2.  This import isn't perfect*, but it should do the
lions share of moving a model from v2 to v3.

On the good looks side of things, this version adds support for Retina
display on MacOS, PNG format screen shots, and also saving screen shots at
greater than on-screen resolution.  This should help improve publication
quality figures from OpenVSP.

Features:
 - v2 Import
 - Retina display on MacOS
 - Super-resolution screen shots
 - PNG format screen shots
 - Improved and quantitative view control (repeatable views)
 - Geom Browser converted to programmatic GUI

Bug Fixes:
 - Change how point cloud files are saved, prevent out of memory on read
 - Fix O(n^2) bugs with points and triangles in XML (copy/paste and save)

* Components that don't yet exist in v3 can not convert.  Also, some
parameters like skinning tangent strengths don't convert 1:1, so a
reasonable approximate value is assigned.


---


# [OpenVSP Version 3.4.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.4.1)

2015-12-13

This bug fix release takes care of handful of problems that some users
have hit.  One recent bug, the others have been around a while.

Thanks to Jason Welstead for characterizing one bug and to Alex Gary for
fixing it.

Bug fixes:
 - Negative volume flag not working for Custom Geoms.
 - Crash on minimize/restore of main window on MacOS.
 - Mixed tolerances in CompGeom causing removal of watertight shapes.
 - Force true closure of six-series airfoils.


---


# [OpenVSP 3.4.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.4.0)

2015-11-17

CST for everyone!

OpenVSP 3.4.0 comes with native support for Brenda Kulfan's CST Airfoil
representation.  This is an oft-requested and long awaited feature by
those doing aero optimization.

CST support is limited to the airfoil class (N1=0.5, N2=1.0) and is only
available for Wing-type components for now.

CST airfoils can be promoted exactly or demoted to the closest possible
approximation of reduced order.  Existing VSP airfoils can be converted
to CST using a least squares fit of a specified order.

Airfoils may now be modified via scale, shift, and rotate.  These mods
are used by the CST fitting to allow the CST curve to be defined on
x=[0,1] with start/end at y=0.

These modifications will also allow some interesting alternative uses.
With great power comes great responsibility.  Do not use these mods
to achieve wing shapes possible through 'normal' parameters -- you will
come to regret it.

Features:
 - CST Airfoil class support
 - Airfoil scale, shift, rotate modifications
 - Promote/demote CST airfoils
 - Best least squares CST conversion of existing airfoil types
 - Extend API to support CST airfoils

Bug fixes:
 - Write 1/0 Type parameter for *.hrm files
 - Modify SuperBuild to auto-build *.zip package every time


---


# [OpenVSP 3.3.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.3.2)

2015-11-05

This is a quick release to fix a bug with 6-Series airfoils on Windows
introduced by 3.3.1.

Bug fix:
 - Fix problem indexing past end of vector in 6-Series.  Thanks James Haley.


---


# [OpenVSP Version 3.3.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.3.1)

2015-11-02

Small but mighty!

OpenVSP 3.3.1 may 'only' be a bugfix release, but it fixes some really
big and longstanding bugs.  The fixes are so significant that I
considered calling it 3.4.0 -- or even 4.0.0!

Whatever you do, update to this version, you'll be glad you did.  Shaded
rendering is significantly improved, a few performance improvements, and
the problems with 6-series and file airfoils have been fixed.

Bug fixes:
 - Fix various problems with 6-Series and File Airfoil types.
 - Multiple normal vectors at corners and edges for better shading.
 - Improve feature line rendering to reduce artifacts.
 - Eliminate transparent surface rendering artifacts.
 - Tweak adaptive feature lines to improve consistency of results.
 - Add OpenVSP version number to output IGES files.
 - Tweak target mesh length calculation to speed CFDMesh.


---


# [OpenVSP Version 3.3.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.3.0)

2015-10-09

October surprise!  OpenVSP can now export IGES files!

OpenVSP 3.3.0 is the latest feature release of everyone's favorite
parametric modeling tool.  IGES support was made possible via libIGES, a
small library created by Cirilo Bernardo.

I've bumped the version number not because this change is monumental,
but because it will require special attention for anyone building VSP
themselves (they must re-build the Libraries to build libIGES).

Unfortunately, IGES file export will not be a CAD-interchange-cure-all.
IGES files will be subject to many of the same pitfalls as STEP files, but
hopefully this will provide another avenue to work with other modelers and
downstream tools.

In addition to IGES files, there are some other features and bug fixes,
everyone should update and enjoy!

Features:
 - IGES file export made possible by libIGES github.com/cbernardo/libIGES
 - Adaptive feature lines (Should be prettier)
 - Visualization of blank component axes (Turn off feature lines to hide)
 - Split surfaces by default for STEP files (Match IGES, more compatible)
 - Add STEP/IGES export units enum to API

Bug fixes:
 - Wing root incidence not updating (thanks Travis, Michael, and Brandon)
 - Wing twist limits too limiting (thanks Michael and Brandon)
 - Symmetrical diagonals for CompGeom & friends (thanks Dave Kinney)


---


# [OpenVSP Version 3.2.3](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.2.3)

2015-09-20

OpenVSP 3.2.3 is a periodic update.  Nothing huge here shoving it out the
door, but enough good things have accumulated in the last month that a
release should be useful to people.  Both fixes and features, everyone
should update.

Features:
 - Added slew control to skinning.  Slew controls the direction of surface
    curves 'in' the surface - where Angle is 'out' of the surface.
 - Change default skinning parameters to simulate v2's Fuse2 component.
 - Export wireframe as Plot3D file (thanks Joe Derlaga)
 - Improve nearest point search algorithms in Code-Eli
    should help CFDMesh and FitModel.
 - Cache derivative calculations in Code-Eli - should speed CFDMesh some
 - Added reversed triangle detection and removal to CFDMesh
 - Tweaked CFDMesh tolerances and parameters for robustness

Bug fixes:
 - Refactor remove interior tris.  Fix complex negative volume cases.
 - CFDMesh source names not saving (thanks @SteveDoyle2)
 - Feature line update.
 - Fix API specification causing problems for SWIG (thanks Alex & Travis)
 - Misc. code cleanups (thanks elfring)


---


# [OpenVSP Version 3.2.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.2.2)

2015-08-18

OpenVSP 3.2.2 is another bugfix release.  Some small, but a few that have
been longstanding problems.  Everyone should update to get the benefit of
these fixes.

Features:
 - Specify units when writing STEP files.
 - Specify dimensional tolerance when writing STEP files.

Bug fixes:
 - Fix wing updating with zero tip chord, zero taper, etc.
 - VSPAERO Remove additional swirl component in front of actuator disk.
 - Fix airfoil trimming bug introduced in 3.2.1
 - Eliminate divide by zero for trim/closure parameters with zero chord.
 - Fix bug introduced when DegenGeom GUI was made programmatic.


---


# [OpenVSP Version 3.2.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.2.1)

2015-07-31

OpenVSP 3.2.1 is a release of mostly bugfixes.  Most of the fixes are small
items expected to accompany a big release like 3.2.0.  However, the most
significant fix has been a problem with all of v3.  So, although it is
small, everyone shoudl go ahead and update.

Improvements:
 - Background control GUI converted to be programmatic

Bug Fixes:
 - Fix inconsistent orientation of triangles from CompGeom
 - Fix orientation of symmetrical actuator disk normal vectors
 - Improve feature lines and XSec highlights with blunt trailing edges
 - Don't write un-needed Matlab DegenGeom file for VSPAERO


---


# [OpenVSP Version 3.2.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.2.0)

2015-07-24

OpenVSP 3.2.0 is the first feature release in a while -- there is a lot
of great stuff included to make it well worth the wait.

Blunt trailing edges -- Wing section TE's can now be blunt.  Airfoil files
with a gap between the TE points will be left that way (re-read your files
on existing models to get this behavior).  Airfoil curves can be modified
to thicken or sharpen the TE; they can also be trimmed to blunt the TE.
The sharp corners have parameter values W=0.001 and 0.999, use these values
when creating subsurfaces or attaching to the corner.

A negative volume flag has been added to components to cause them to poke
holes in CFDMesh.  In all other cases, these components have normal
behavior.  This should be useful for modeling engine flow-through.

Actuator disk support has been added.  The disk is described in DegenGeom
for VSPAERO and a thin membrane is meshed in CFDMesh.  Some volume meshers
should be able to mesh up to both sides of the disk.  The actuator disk
is immune to negative volume, so you can put a disk in your flow-through
duct if you want to.

VSPAERO has some great improvements as well.  Easy actuator disk
integration, velocity surveys, and some load integration bug fixes.

Improvements and additions since 3.1.2:
 - Blunt TE airfoil support
 - Negative volumes for CFDMesh
 - VSPAERO auto-setup of actuator disks
 - VSPAERO field velocity survey calcualtions
 - Actuator disk component (example custom component)
 - Improved actuator disk integration with VSPAERO
 - Control surface subsurfaces
 - Drag-N-Drop for picking parameters
 - Save/Import for FitModel optimization problem formulation
 - Programmatic GUI for CFDMesh, Parameter Linking, CompGeom, and DegenGeom
 - Custom component support for actuator disk and negative volumes

Bug Fixes:
 - VSPAERO Fix some pressure integration bugs
 - Split symm plane to improve CFDMesh mesh quality with huge domains
 - Tweak tolerance for short intersection edges in CFDMesh
 - Set z-buffer distance dynamically to handle large models

One last note, with this version we also add two new developers to the
team, thanks much to Brandon and Mitch for their contributions.


---


# [OpenVSP Version 3.1.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.1.2)

2015-05-23

OpenVSP 3.1.2 fixes a few bugs and makes the API more complete.

Features and additions
 - Add control of wing root incidence.  Thanks Jeff Viken.
 - Add container routines to API for User Parms.  Thanks Erik Olson & James Haley.
 - Add file airfoil routines to API.  Thanks Yohann Delannay.

Bug fixes
 - Fix problem with slashes in file names.  Thanks Bill Fredericks.
 - Include OpenMP dll's on Windows.  Thanks Bill & Brandon Litherland.
 - Fix VSPAERO problem with long path -setup.  Thanks Brandon.
 - Fix nonexistant component IDs in VSPAERO Manager.  Thanks Brandon.


---


# [OpenVSP Version 3.1.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.1.1)

2015-05-02

OpenVSP 3.1.1 fixes a bunch of small issues that quickly cropped up in
3.1.0.  Thanks to everyone who helped find and diagnose these problems.

Bug fixes
 - Built-in favorite paths not appending trailing /.  Reported by Alex Gary.
 - Correct omission of SECSWEEP_WSECT_DRIVER from API.  Reported by Yohann Delannay.
 - Fix issue of displaying duplicate custom components.  Reported by Jim Fenbert.
 - Force static linking of GCC supplied libraries.  Reported by Bryan Morrisey.
 - Improve status checking and button de/activate for VSPAERO.
 - Properly quote Windows commands with spaces.  Reported by Alex Gary.


---


# [OpenVSP Version 3.1.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.1.0)

2015-04-28

OpenVSP 3.1.0 brings the much awaited release of VSPAERO, the new vortex
lattice solver that has been developed to be a part of OpenVSP.  There
are a few other improvements and fixes, but I'm certain everyone will
want to update to get their hands on VSPAERO.

VSPAERO is a thin-surface code for inviscid subsonic and supersonic
aerodynamics.  It is multithreaded to make the most of common multi-core
computers and uses a mesh agglomeration scheme to improve scaling with
large meshes.  The matrix is solved with an iterative GMRES solver.
VSPAERO includes a simple actuator disk model to represent propulsion-
airframe interaction and also the ability to calculate common stability
derivatives.

Improvements and additions since 3.0.4
 - VSPAERO vortex lattice solver and viewer
 - VSPAERO integration into OpenVSP GUI
 - Initial use of Travis-CI continuious integration build/test server
 - Search multiple paths for CustomScripts, working, home, then vsp's location
 - Add home and vsp's location to favorites pull-down in file select dialog

Bug fixes
 - Reading wings with sweep location=1.0 from file (reported by Nick Borer)
 - Wing parameter limits now allow tiny segments (reported by Nick Borer)
 - Fix Linux build problem introduced at the last moment before 3.0.4


---


# [OpenVSP Version 3.0.4](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.0.4)

2015-02-17

OpenVSP 3.0.4 is mostly a bug fix release.  This should fix a bunch of
issues with DegenGeom in preparation for things to come.  Also fixed
are a bunch of problems with 6-series and file airfoil types.  Anyone
using v3 should certainly update.

Bug fixes in this release include:
 - Six series leading edge creep
 - Crash for very thin six series airfoils
 - Fix problem copy/pasting file airfoils
 - Update overall wing aspect ratio display
 - Don't deselect Geom when executing DegenGeom
 - Fix sign assumption in z camber of DegenGeom for reflex or inverted foil
 - Make export file names match primary file name
 - Save Vehicle level parameters including file save options
 - Fix wing updating when total parameters changed
 - Enable extracting point cloud from slices
 - Write out slices to STL file
 - Increase upper bound on Wing Area parameters to support aircraft in mm
 - Don't write out flat wing caps to DegenGeom
 - Don't drop point data for symmetrical DegenGeom
 - Build fixes for Linux
 - Fix mismatched limits for 4-digit airfoil parameters
 - Update X3D export for materials and flipped normal vectors
 - Add version check to ReadXMLFile
 - Add base file name getter to API


---


# [OpenVSP Version 3.0.3](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.0.3)

2015-01-23

OpenVSP 3.0.3 includes a handful of bug fixes and small improvements to
the 3.0 series.  Versions 3.0.1 and 3.0.2 included quick fixes for urgent
last minute bugs -- but they did not include a complete release note.

API Users Note:
The calling arguments in four functions were changed: CutXSec, CopyXSec,
PasteXSec, and InsertXSec.  See Wiki for details.

Improvements since 3.0.0 include:
 - Extend API to allow changing Wing Driver Groups
 - Add VSPExit to API to return exit value to host batch environment
 - Add DegenGeom to API / Scripting
 - API now references Cut/Copy/Past/Insert XSec to Geom not XSecSurf

Bug fixes since 3.0.0 include:
 - Fix crash caused by dynamic Parm GUIs and undo-friendly sliders (3.0.1)
 - Fix crash when mixing 4-digit airfoils with other XSec types (3.0.2)
 - Fix ability for API to find parameters in XSecSurfs (3.0.2)
 - Fix update of FractionParms in FuselageGeom
 - Fix updates of Wing Geom through API, Design GUI, Links, or FitModel
 - Fix handling JPEG image skew for non multiple of 4 pixel widths
 - Fix JPEG background scaling behavior to maintain image aspect ratio
 - Fix problem with Python wrapper build on Windows


---


# [Bump version to 3.0.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.0.2)

2015-01-06

Release new version with a couple of quick fixes.


---


# [Bump version to 3.0.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.0.1)

2015-01-06



---


# [OpenVSP Version 3.0.0!](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_3.0.0)

2015-01-03

Version 3.0.0 compared to version 2.3.0 includes too many changes and
improvements to list.  OpenVSP v3 is a near total re-write that has
been three years in the works.  Many thanks to the entire core OpenVSP
team.  This is what you've all been waiting for!

Since 2.9.7, version 3.0.0 has a handful of small fixes and features,
and one major new feature that has long been in development.

Improvements include:
 - Fit Model tool to perform least squares fit of VSP model to points
 - Point Cloud Geoms to support Fit Model
 - Import of point clouds as well as conversion from MeshGeom
 - Improved Undo, where slider actions are not recorded
 - SubSurface support in API
 - Improvements to advanced parameter linking.


---


# [Bump version to 2.9.8 as prelude to 3.0.0.](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.9.8)

2015-01-03

Otherwise identical to 3.0.0, 2.9.8 exists to serve anyone who may have
some sort of issue with the major version number change.


---


# [OpenVSP 2.9.7 fifth Beta for version 3](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.9.7)

2014-12-11

This version includes some nice improvements, but also a bunch of small
bug fixes that have been preventing people from getting work done.  If
you've been hitting these, you'll certainly want to update.

Improvements include:
 - Issue error when you try to set secondary & primary sweep location equal
 - New NACA 4-Digit airfoil code, much improved representation of shape
 - Custom components now support scaling
 - Custom components now support default sources
 - Custom components now support set center of rotation
 - Custom components now support surface type flag setting
 - Can now edit name, group, description of user parameters

Bug fixes include:
 - Fix scripting on Linux - scripting, custom components, advanced links
 - Improve robustness of triangle splitting (CompGeom, DegenGeom, etc)
 - Use strict IEEE FP on Windows for Triangle, fix crashes in CompGeom
 - Initialize strings to \0, fix DegenGeom crash on Windows
 - Advanced parameter links force update on compile
 - Prevent AWave slice angle from going to 0 (Mach=infinity).
 - Set file name when opening files from command line.  Thanks Jim.
 - Remove assumption on # of user parms.  Thanks Erik.


---


# [OpenVSP 2.9.6 fourth Beta for version 3](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.9.6)

2014-10-29

This version includes more small fixes and feature refinements as we inch
closer to version 3.0.

Improvements include:
 - Symmetry about ancestor's coordinate system
 - Default Source support for CFDMesh
 - Rearrange Geom Browser tab to show larger geom trees
 - Added Delete, Set Show/NoShow/ShowOnly, and surface None buttons
 - Expand custom geom #include statements and save with geom
 - Added GetCustomXSecLoc/Rot and GDEV_SCROLL_TAB

Bug fixes include:
 - Numerous surface parameter [0,1] bugs that could cause crashes.
 - Consistent triangle orientation for NASCART import/export
 - Fix skinning bugs & tweak GUI
 - Fix wing scaling


---


# [OpenVSP 2.9.5 third Beta for version 3](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.9.5)

2014-10-07

This version includes a large number of changes both features and fixes.
Some of these fixes took longer than expected -- and one of the features
is pretty big, so this release took much longer than anticipated.  Look
for subsequent releases to be smaller and more frequent as the bugs are
chased out.

Warning:  One bug fix required an incompatible change in how skinning is
calculated.  If you have used a Fuselage or Stack component with skinning,
you will need to adjust your tangent strengths -- the geometry will be
wrong when you open it.  I am very sorry for this inconvenience.

Note:  The file extension for custom components has been changed from
.as to .vsppart.  Similarly, the file extension for scripting has been
changed from .as to .vspscript.  These changes should prevent future
confusion.

Significant improvements include:
 - Add Advanced Parameter Linking
 - Add STEP file options for compatibility with SolidWorks and Pointwise
 - Add multi-solid STL file options for tagged meshes to Pointwise
 - Add command-line script mode
 - Add no-graphics vspscript executable
 - Add Adjustable center of rotation for Wings
 - Add 'Loop' mode for Stack components

Notable bug fixes include:
 - Major redesign of how CFDMesh handles underlying surface fixing bugs
 - CFDMesh GUI Source and Wake geom handling
 - Saving Wake and Far Field settings for CFDMesh
 - Scaling of Custom Components
 - Zero chord wing crashes
 - DegenGeom and wing cap crashes
 - CFDMesh source crashes
 - Small errors in DegenGeom files noticed by Erik Olson
 - Fixed lots more bugs


---


# [OpenVSP 2.9.4 second Beta for version 3](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.9.4)

2014-08-19

This version includes a whole host of small bug fixes and tweaks over
2.9.3.


---


# [OpenVSP 2.9.3 first Beta for version 3](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.9.3)

2014-08-14

This version has pretty much all the features that are targeting 3.0.0 and
should be getting close to a generally usable state.  It still has some
rough edges, but is getting much closer.

Significant improvements include:
 - Run scripts from command line and/or GUI
 - Expansion of Custom Component capabilities
 - Fuselage XSec ordering policies -- better support for nacelles.
 - Closed wing end-caps
 - STEP file output
 - Material editor
 - Feature detection for CFDMesh
 - Feature visualization
 - Lots of GUI usability improvements
 - New secondary sweep wing driver
 - Lots of bug fixes.


---


# [OpenVSP 2.9.2, third public alpha for version 3](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.9.2)

2014-07-27

Long in coming, but worth the wait.  This third alpha has many and vast
improvements from 2.9.1.  While there are still bugs and small feature
holes, all the major features are in there.  This is getting close to what
3.0.0 will be.

Major improvements include:
 - Degenerate geometry support to enable low-order analysis tools.
 - Fuselage and stack component skinning
 - View clipping
 - Subsurface visualization and polishing
 - Active XSec highlighting & view
 - Saving CFDMesh settings
 - Custom geometry improvements
 - Start of advanced parameter linking
 - Fix wing sweep/twist interaction, fix dihedral implementation
 - Fix problems with JPEG files

The 2.9 series will be the alpha and beta versions leading to 3.0.0, which
will be the first production release for this branch.


---


# [OpenVSP 2.9.1, second public alpha for version 3.](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.9.1)

2014-05-28

Long overdue, this second alpha has tons of changes from 2.9.0.  This
version still has lots of bugs and missing features, but it should be a
dramatic improvement -- or at least a much better preview of 3.0.0.

Major improvements include:
 - Subsurfaces
 - Custom Components
 - Greatly expanded API
 - Preliminary Wing and Stack Geom types
 - Airfoils and other XSec types implemented
 - Pick Geom (p) and Pick rotation (r) modes
 - Rulers, materials, and lighting
 - Lots of bug fixes

The 2.9 series will be the alpha and beta versions leading to 3.0.0, which
will be the first production release for this branch.


---


# [OpenVSP 2.9.0, first public alpha for version 3.](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.9.0)

2014-02-12

For the past two years, a dedicated team has been re-writing VSP from
the ground up.  This is the first public release from that effort.

The core v3 team responsible for this work so far is:
Alex Gary, J.R. Gloudemans, Yue Han, David Marshall, and Rob McDonald

The 2.9 series will be the alpha and beta versions leading to 3.0.0, which
will be the first production release for this branch.

Unfortunately, this development was done in a way that the development
history is not a meaningful continuation of the 2.X history.  Consequently,
this commit starts a new history from the beginning.  Do not try to work
across the branches, no good can result.


---


# [Bump version to 2.3.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.3.0)

2013-12-10

Version 2.3.0 contains one significant feature and a small bug fix.
Version 2.3.0 is likely the last 2.X feature release for VSP.  We will
continue making 2.3.Y bug fix releases for a long time.

For some time, almost all developer effort has been focused on the pending
3.X project.  VSP v3 is a near total rewrite keeping everything you love
about VSP, but ushering in new features galore and a more maintainable
code base.  We hope to release a not-for-production-use alpha version of
v3 for users to begin testing soon.

Fixes/features in this release:
 - Add ability to model outer domain boundary in CFD Mesh.
 - Match multi-body edge shares in CFD Mesh.  (Thanks Sriram R.)

You can now generate a CFD Mesh including the outer boundary.  Some volume
grid generators need to start with a fully enclosed mesh, so we now support
that mode of operation.  The outer boundary can be either an explicit VSP
component or a box generated for you.  You can model the full domain, or
just the +Y half domain -- VSP will mesh the symmetry plane for you.  There
are separate mesh size controls which apply to the outer boundary.

In CompGeom, VSP tries to match pairs of non watertight geometries to form
a single watergith geometry.  In CFD Mesh, VSP tries to pair and stitch
any perfectly coincident boundary edges.  In cases where there were three
or more components that daisy chained together, VSP do this incorrectly.
With this bug fix, you can now daisy chain an arbitrary number of
components to form a watertight body -- so long as the boundaries match
exactly.


---


# [Bump version to 2.2.5](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.2.5)

2013-09-24

Version 2.2.5 contains a collection of bug fixes and small features.  Some
of the features look like bug fixes -- and some of the fixes look like
small features.

Fixes/features in this release:
 - Save CFD export options
 - Automatically add *.vsp extension when saving
 - Add *.ram files to file-open dialog.
 - Write propellers to XSec *.hrm files
 - Fixed PovRay *.inc file bug
 - Fix FEA elements not getting written
 - Fix 64-bit GLFont support and embed font in executable
 - Fix parameter linking and design files for propeller


---


# [Bump version to 2.2.4](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.2.4)

2013-06-14

Version 2.2.4 contains a collection of bug fixes that improve the CFD
mesher.  Various users have run into these problems, so if you use
CFDMesh, you should certainly upgrade.

Bug fixes in this release:
 - Guarantee attachment of wakes to constructing edge
 - Relax tolerance to better match wakes down body splits
 - Rewrite curve tessellation integration for quality in extreme cases
 - Introduce curve split sources for mesh quality near intersections
 - Size curve distance table to match underlying topology

The wake changes should help guarantee mesh topology when wakes are turned
on.  There were certain cases that could arise where a wake would not
properly attach to the generating trailing edge, or the edge of a body
that aligned with the wake.  Both of these situations should be improved.

The rest of the changes affect the quality of the tessellation along
intersection and border curves.  The VSP mesher first lays out the curves,
and then the surface mesh is generated without changing the curve points.
Consequently, poor quality curve tessellation means a poor quality surface
mesh.  If you have experienced strange unexplained mesh quality problems,
these fixes should help.


---


# [Bump version to 2.2.3](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.2.3)

2013-04-10

Version 2.2.3 contains a triplet of small bug fixes that have bitten
some users.  I have been putting these off waiting for an upcoming
feature release, but now seems to be the time to get these out there.

Bug fixes in this release:
 - Threaded version check
 - Fix CG contribution of point mass components
 - Fix CFDMesh half models when a surface is actually cut

Placing the version check in its own thread allows VSP execution to
continue during the version check.  This is significant for situations
where the computer has internet connectivity, but the VSP web server is
experiencing problems.  The HTTP library used by VSP does not let you
change the timeout from 60 seconds, so in these situations, VSP paused
for 60 seconds on startup.

The CG contribution of point mass components was calculated using their
local translation -- ignoring the translation of any components they were
attached to.  This fix makes their contribution use their actual global
x,y,z position.

When the half model option is selected for CFDMesh, VSP makes two passes at
eliminating geometry in the -Y domain.  First, before any meshing occurs,
surfaces which fully lie in that domain are eliminated.  Then, the
remaining surfaces are sliced by a Y=0 plane.  That intersection line is
obeyed by the mesher, and any triangles whose center point lies in the
wrong domain are eliminated after mesh generation (when interior triangles
are culled).

The second process needs the cutting plane to be a fully-fledged plane,
including a curvature based target map.  Prior testing had only exercised
the first code path (which could complete with an incomplete cutting plane).
This bug basically amounted to some null pointer dereferences and improper
allocation/setup for the cutting plane.


---


# [Bump version to 2.2.2](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.2.2)

2013-02-18

Version 2.2.2 has been a long time coming.  I have been waiting on a few
features to make a 2.3.0 release, but finally decided it was better to get
these fixes out sooner than later.

The mesh growth limiting step of CFDMesh 'Build Target Mesh' has been
totally re-written.  This new version should be _vastly_ faster for just
about anyone out there.  The new BTM algorithm follows the surface
topology rather than doing a full spatial search.  For the rare cases
where a full spatial search is still required, there is an optional flag
to enable it.  Even that mode should still be much faster than before.

Bug fixes in this release:
 - Don't close already closed file in readVspAirfoil (Linux crash)
 - Fix MS_Wing chord update on scale
 - Fix warnings on various compilers
 - Update example files to silence warnings on load


---


# [Bump version to 2.2.1](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.2.1)

2012-12-06

Version 2.2.1 contains a number of small bug fixes.  One of thes fixes is
particularly important -- everyone should upgrade.

The nastiest bug caused components added from the 'external' default
component set to have the ID number of that component.  If multiple such
components were added, this could cause duplicate IDs in the model.  IDs
are expected to be unique, so this causes significant problems.

The external BWB defaults that ship with VSP include a MS_Wing component
with ID 42078344.  This is the most likely number to be duplicated.

If your model has duplicate IDs, you may see strange behavior including
problems with parent/child relationships, parameter linking, and wetted
area/volume totals from CompGeom.

In addition to a fix to prevent this from happening, a check for duplicate
IDs has been added to VSP.  If you open a model with duplicate IDs, a warning
message will be printed to the console and new IDs will be created for your
model.  Unfortunately, this may cause some parent/child and parameter links
to be broken.  If you see this message, check your model carefully.

Bug fixes in this release:
 - Bug when editing a propeller that could crash VSP
 - Parameter linked fuselage variables could mysteriously take wrong values
 - Components created from 'external' default components could have duplicate IDs


---


# [Bump version to 2.2.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.2.0)

2012-09-13

Version 2.2.0 includes many features and improvements in all corners of
the program.  There should be something here for everyone, so go ahead
and upgrade.

My favorite feature is the 'r' key.  Place the mouse over the model and
press 'r' to change the center of rotation.

Many users will appreciate that we can now read both of the file formats
used in the UIUC airfoil database.  Please let us know if you find any
airfoil files in the database that you can't read.

Users doing trade or optimization studies will want to check out the
design and XDDM support.  These features should make it very easy to
use VSP in your next design study.

In addition, there were a whole host of small fixes and improvements.
Some of these bugs were long standing and quite annoying -- it is great
to see them gone.

Major features of this version include:
 - User interface improvements
    Add axis orientation arrows
    Center view menu option (or c key)
    Set center of rotation (r key)
    Added help menu for mouse/key usage
 - Read Lednicer airfoil files from UIUC database
    (in addition to Selig and VSP format airfoil files)
 - Add support for design files to facilitate trade/optimization studies
 - Add support for Cart3D Optimization Framework XDDM files
 - Wake improvements
    Visualize wake
    Write wakes to separate solid in STL files
    Separately tag wake components in TRI files
    Orient wake normals consistently

Bug fixes and minor features include:
 - Calculate viewpoints and add them to X3D files
 - Fix angle calculation for co-linear vectors
 - Make component ID's persistent
 - Increase wake tag increment from 100 to 10000
 - Increase precision in XSec files
 - Fix rounded-tip flap/slat bug
    (https://github.com/OpenVSP/OpenVSP/issues/11)
 - Prevent CFDMesh from adding lots of tiny triangles when short edges
    are forced by surface topology
 - Change 'Twist' label to 'WashOut' to better indicate sign convention
 - Use fast nearest neighbors to merge duplicate nodes
 - Expand propeller cone angle limits
 - Add test for Vorlax to prevent entry to VorView menu
 - Fix fuselage edit cross section bug
    (https://github.com/OpenVSP/OpenVSP/issues/8)
 - Sane defaults for user views
 - Fix MSWing blended cross section bug
 - Increase file load speed by replacing dyn_array with deque in ms_wing


---


# [Bump version to 2.1.0](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.1.0)

2012-07-02

Major features of this version include:
 - CBAero Wake modeling
 - Curvature based meshing
 - Re-worked CFD mesh GUI
 - X3D file output
 - Online version check

Bug fixes and minor features include:
 - Scale wing area by linear dimension instead of area
 - Don't write commas in Cart3D files
 - Batch mode mass properties calculation


---


# [Bump version to 2.0.3.](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.0.3)

2012-02-22



---


# [Bump version to 2.0.2.](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.0.2)

2012-02-21



---


# [Bump version to 2.0.1.](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.0.1)

2012-02-02



---


# [Add community web site to About box.](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.0_Community)

2012-01-10



---


# [Official initial release of OpenVSP.](https://github.com/OpenVSP/OpenVSP/releases/tag/OpenVSP_2.0.0)

2012-01-10



---


