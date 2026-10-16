---
title:  'Gen Tab'
---

Every component (Geom) in OpenVSP is edited in a window whose first tabs are the same for all of them: Gen,
[XForm](XFormTab.md), [Mass](MassTab.md) and [Sub](SubTab.md).  The Gen tab holds what does not depend on the Geom's
shape: its name and look, how finely it is tessellated, whether it cuts the Geoms it overlaps, which sets it belongs
to, and its attributes.  The tabs after the common ones shape the particular Geom and are described on its own page.

| Topic                                   |
|:----------------------------------------|
| [Name, Color and Material](#name-color-and-material) |
| [Tessellation](#tessellation)           |
| [Negative Volume](#negative-volume)     |
| [Sets](#sets)                           |
| [Attributes](#attributes)               |
| [Scripting](#scripting)                 |

## Name, Color and Material

The *Name* is the Geom's name in the Geom Browser, in analysis results and in exported files.  *Color* and
*Material* set only how it is drawn.

## Tessellation

OpenVSP's surfaces are smooth; *Num_U* and *Num_W* set how finely they are cut into the triangles and quadrilaterals
that the 3D view draws and that the mesh-based analyses use (CompGeom, Mass Properties, VSPAERO, the mesh exports).
*Num_U* is the number of points along each span of the surface between two of its cross sections (a pod has two
such spans, nose to middle and middle to tail), and *Num_W* the number of points around each section.  Fuselage and
Stack set the points along each span section by section on the XSec tab instead (see
[Fuselage and Stack](FuselageStack.md#tessellation-along-the-body)), and Wing on its Sect tab (see
[Wing](Wing.md#tessellation)), so their *Num_U* is greyed.

Choose the tessellation for the use.  Finer is not always better: a surface smooth enough for 3D printing or an
accurate wetted area is far finer than VSPAERO needs, and only makes it slower.  One model can hold both (see
[Sets](#sets)).

*Num_W* is rounded up to one more than a multiple of four (of eight for Fuselage, Stack and Propeller), so that
the tessellation always passes through the points a quarter of the way round each section: the top, bottom and sides
of a body, or the leading and trailing edges of a wing.

## Negative Volume

Almost every Geom with a surface is closed on its own, and Geoms are joined by overlapping them.  The model keeps them
as separate surfaces, and the 3D view draws them overlapping; only the analyses that need one outer surface, such as
CompGeom, Mass Properties and CFD Mesh, intersect them and keep their union.  So let parts overlap rather than just
meet: carry a wing's root inside the fuselage instead of stopping it at the fuselage's surface.

A Geom marked *Negative Volume*, under *CFDMesh Negative Volume* on the tab, is a cutter: in analyses that intersect the
model, such as CompGeom, Mass Properties and CFD Mesh, its volume is removed from the Geoms it overlaps.  The part of
its surface inside them becomes the wall of the cut, and the part outside is dropped.  Use it for a cut-out such as a
wheel well or a cavity.  It is greyed for a Geom whose surface the analyses already ignore, such as a propeller drawn as
a disk.

Make a cutter pass through the surface it opens, rather than end on it.  Where a face of the cutter lies exactly on
the surface of the Geom it cuts, which side of it each point lies on is ambiguous, and the cut can come out wrong.
Carried a little past the surface, the same cutter cuts cleanly.

## Sets

The list holds every set in the model (named in the Set Editor, see [Set Editor](SetEdit.md)), and the Geom is in each
set that is checked.  Analyses and exports each work on one set, so sets are how a Geom is left out of or included in
each of them.  Sets also let one model hold several versions of a part, each set up for a different use, such as a
finely tessellated fuselage for wetted area in one set and a coarse one for VSPAERO in another; each analysis is then
run on the set that holds its version.

*Shown* and *Not_Shown* are the two sets the 3D view uses: a Geom is always in exactly one of them, so checking one
unchecks the other.

## Attributes

The lower part of the Gen tab is an attribute editor for the Geom, and each sub-surface has one for itself on the
[Sub](SubTab.md) tab.  Attributes are named values of any kind (text, numbers, vectors, matrices, references to Parms,
groups of other attributes) that OpenVSP stores in the model file and the API can read, for notes or for data a
downstream tool needs.  They do not change the geometry.  See [Attribute Explorer](AttributeExplorer.md).

## Scripting

The tessellation and Negative Volume controls are Parms, set and read with `SetParmVal` and `GetParmVal` by name and
group; `GetParmActiveFlag` tells whether a control is active or greyed, as of the last Update.  The name, color,
material and sets are not Parms and have functions of their own.

| Control                       | Parm                                                       | Group        |
|:------------------------------|:-----------------------------------------------------------|:-------------|
| Num_U, Num_W                  | `Tess_U`, `Tess_W`                                         | Shape        |
| Negative Volume               | `Negative_Volume_Flag`                                     | Negative_Volume_Props |

| Control                       | Function                                                   |
|:------------------------------|:-----------------------------------------------------------|
| Name                          | `SetGeomName`, `GetGeomName`                               |
| Color                         | `SetGeomWireColor`, `GetGeomWireColor`                     |
| Material                      | `SetGeomMaterialName`, `GetGeomMaterialName`               |
| Sets                          | `SetSetFlag`, `GetSetFlag`                                 |
