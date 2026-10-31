---
title:  'NGon Mesh'
---

An NGon Mesh holds a polygon mesh: the surface of the model as faces with any number of sides, with the wakes that
shed from its trailing edges marked.  It is the geometry VSPAERO runs on, and it is made from a [Mesh](Mesh.md), most
often by VSPAERO itself when it prepares the geometry.  Its job is to be inspected and written to a VSPAERO geometry
file (.vspgeom); it has no shape parameters, and it is not saved with the model.

| Topic                                                  | Tab   |
|:-------------------------------------------------------|:------|
| [Quick Start](#quick-start)                            |       |
| [Where an NGon Mesh Comes From](#where-an-ngon-mesh-comes-from) |   |
| [How the Polygons Are Made](#how-the-polygons-are-made) |      |
| [Checking the Mesh](#checking-the-mesh)                | Other |
| [Coarser Meshes](#coarser-meshes)                      | Other |
| [Writing a VSPGEOM File](#writing-a-vspgeom-file)      | Other |
| [Limits](#limits)                                      |       |
| [Scripting](#scripting)                                |       |

The Gen, XForm and Mass tabs are the ones every component has; see [Gen](GenTab.md),
[XForm](XFormTab.md) and [Mass](MassTab.md).  An NGon Mesh has no symmetry or sub-surfaces, and the
Mass tab's density controls are greyed.

## Quick Start

1. Prepare the VSPAERO geometry (*Prepare Solver*), or click *Convert* under Convert to NGon Mesh on a mesh's Other tab.
2. Turn on *Show Non Manifold Edges* on the NGon mesh's Other tab, and fix the model where it shows defects.
3. Click *Write VSPGEOM* to keep the mesh, which the model file does not save.

## Where an NGon Mesh Comes From

An NGon mesh is always converted from triangles, in one of three places.

| Source                                          | Notes                                                       |
|:------------------------------------------------|:------------------------------------------------------------|
| VSPAERO, *Prepare Solver* or *Launch Solver*    | CompGeom of the chosen sets (the thin set as camber surfaces), converted, then written to the .vspgeom file VSPAERO reads.  The CompGeom mesh is deleted and the NGon mesh is left in the model, shown. |
| Mesh, Other tab, *Convert* to NGon Mesh         | Any mesh, converted, without the cleanup passes below.      |
| DegenGeom, *Make Mesh* with *Type* N-Gon        | The degenerate geometry (plates and camber surfaces), converted, without the cleanup passes below. |

The NGon mesh is named after the mesh it came from, with `_NGon` added.  Each time VSPAERO prepares the
geometry it replaces the NGon mesh it made the time before.

## How the Polygons Are Made

The conversion starts from the triangles and rebuilds the faces the components were tessellated with.

- Points that coincide are merged, so the faces share their edges.
- Triangles that came from one quadrilateral of a component's tessellation are joined back into it.  Where an
  intersection cut a quadrilateral, the pieces left on the surface become polygons of their own.
- Points that lie on a straight edge between two others are removed, and edges of zero length dropped.
- A face whose boundary doubles back on itself is split at the point where it turns.
- Along the curves where components intersect and around sub-surface outlines, a segment much shorter than its
  neighbours is collapsed into one end, so slivers do not line the curve (*Cull Tiny Edges*).  The ends of a curve,
  its junctions and its corners are kept.
- The fans of thin triangles where a row of the tessellation closes to a point -- a body's nose and tail, the leading
  and trailing edges of a wing tip cap -- are merged into a few polygons (*Merge Degen Tri Regions*).
- The trailing edges of wings are found and marked as wakes.  Body wakes, behind blunt bases, are found too when
  VSPAERO's *Find Body Wakes* is on.

The two cleanup passes, Cull Tiny Edges and Merge Degen Tri Regions, are for the meshes VSPAERO reads.  They run only
when VSPAERO prepares the geometry, where they are options, off by default, in the *Mesh Options* group of VSPAERO's
Overview tab, beside *X-Z Symmetry* and *Cull Orphans*; a mesh converted from the Mesh screen, from DegenGeom or by
`CreateNGonMeshGeom` is not cleaned.  The slider beside *Cull Tiny Edges* is the ratio, from 0 to 0.5: a segment shorter
than that fraction of its shorter neighbour is collapsed.

*Cull Orphans* drops small pieces of the thin set's camber surfaces left by intersections: each piece whose area is
below the set fraction (0.03 by default) of the area of its surface.  Pieces of thick surfaces are never dropped.  It
is off by default, and greyed when VSPAERO has no thin set.  The NGon meshes made outside VSPAERO are never culled.

*Find Body Wakes*, and *Num Ref.* below, are on VSPAERO's Experimental tab, which is shown only when OpenVSP runs in
experimental mode.  The settings are saved with the model and can be set from a script whether or not the tab is
shown.

## Checking the Mesh

The 3D view draws the faces with their outlines, colored by tag (component and sub-surface), and the wakes as heavy
colored lines along the edges they shed from.

*Show Non Manifold Edges* replaces the wakes with the mesh's defects:

| Drawn as      | Defect                                                                                 |
|:--------------|:---------------------------------------------------------------------------------------|
| Blue lines    | Edges with only one face: holes or open boundaries in what should be a closed surface. |
| Red lines     | Edges shared by three or more faces: surfaces that meet without being joined.         |
| Orange lines  | Loops of edges that lie along one straight line.                                       |
| Black points  | Points where a face's boundary doubles back on itself.                                 |

A closed, clean surface shows none of them.  A thin surface, such as a camber surface from VSPAERO's thin set, is
open by design, and its edges show blue.  Blue lines inside a closed surface can also be a seam whose two sides were
not joined: two edges between the same two points, each with one face.  A VSPGEOM file, which lists faces by their
points, shows such a seam as closed, so check it here.  A component put in both VSPAERO's thick set and its thin set
makes such seams; put each component in one set only.

*Prepare Solver* reports the defects of the mesh it makes in its results: the number of edges with only one face, the
number with three or more, and the number of pairs of points joined by more than one edge.  A model made only of
closed, thick components should report zero for all three.

*Report* prints the mesh's tables -- tag names and IDs, which tags are thick or thin surfaces, and their types --
to the console OpenVSP was started from.

*Triangulate* splits every polygon into triangles.  It is for a solver that needs triangles; VSPAERO takes the
polygons as they are.

## Coarser Meshes

VSPAERO's *Num Ref.* asks for coarser versions of the mesh along with it, each built by merging the faces of the one
before.  The *Mesh* counter on the Other tab chooses which one is drawn and written: 0 is the finest, the mesh the
conversion made, and each step up is coarser.  A mesh converted from the Mesh screen has only the finest.

## Writing a VSPGEOM File

*Write VSPGEOM* writes the mesh, as it is placed, to a VSPAERO geometry file, with the files VSPAERO reads beside
it:

| File                    | Holds                                                    |
|:------------------------|:---------------------------------------------------------|
| `name.vspgeom`          | The points, faces and wakes.                             |
| `name.vkey`             | What each tag means: component, surface and sub-surface. |
| `name.csf`              | The control surfaces, from the sub-surfaces that are control surfaces. |
| `name*.tag`, `.taglist` | The faces of each tag, for VSPAERO's per-group results.  |

Any of these files already there with the same base name are deleted first, so a set of files never mixes two
meshes.

## Limits

- A mesh opened from a model file has lost the tables that say which component and surface each triangle came from,
  so converting it finds no wakes.  Convert a mesh in the session that made it, or make the mesh again first.
- An NGon mesh is not saved with the model.  When the file is opened again the component is there but empty;
  write the VSPGEOM file to keep the mesh, or make it again from its source.
- Its faces cannot be edited.  Change the components, or their tessellation, and make it again.

## Scripting

| Function                          | Use                                                        |
|:----------------------------------|:-----------------------------------------------------------|
| `CreateNGonMeshGeom( mesh_id )`   | Convert a mesh to an NGon mesh; returns the new ID.        |

The *Mesh* counter is the Parm `ActiveMesh` in the group `RefMesh`, and *Show Non Manifold Edges* is
`ShowNonManifoldEdges` in the group `Draw`.  *Triangulate*, *Report* and *Write VSPGEOM* are not in the
API.  `ExportFile` with `EXPORT_VSPGEOM` writes the files as *Write VSPGEOM* does when the set it is given holds an
NGon mesh; otherwise it writes the set's triangles.

The results of VSPAERO's geometry preparation (the `VSPAEROComputeGeometry` analysis, result `VSPAERO_Geom`) hold the
defect counts as `Num_Edges_One_Face`, `Num_Edges_Many_Faces` and `Num_Duplicate_Edges`, each -1 when no NGon mesh was
made.  The Mesh Options are the analysis inputs `CullFracFlag`, `CullFrac`, `CleanIntersectionsFlag`,
`CleanIntersectionsLenRatio` and `MergeDegenTriRegionsFlag`.
