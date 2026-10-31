---
title:  'Mesh'
---

A Mesh component holds a fixed set of triangles.  It is how OpenVSP brings in geometry from other tools -- STL,
Cart3D and NASCART files -- and it is what several analyses leave behind: the intersected and trimmed outer mold
line from CompGeom, the slices from Mass Properties and Planar Slice, a projected area.  A mesh can be moved,
turned, scaled, saved and used in further analyses, but its shape cannot be edited: it has no parameters of its own
beyond how it is drawn.

| Topic                                            | Tab   |
|:-------------------------------------------------|:------|
| [Quick Start](#quick-start)                      |       |
| [Where a Mesh Comes From](#where-a-mesh-comes-from) |    |
| [Importing a Mesh](#importing-a-mesh)            |       |
| [Tags](#tags)                                    |       |
| [Converting a Mesh](#converting-a-mesh)          | Other |
| [Mesh and Slices](#mesh-and-slices)              | Other |
| [Using a Mesh](#using-a-mesh)                    |       |
| [Scripting](#scripting)                          |       |

The Gen, XForm and Mass tabs are the ones every component has; see [Gen](GenTab.md),
[XForm](XFormTab.md) and [Mass](MassTab.md).  A Mesh has no Sub tab.

## Quick Start

### An imported mesh

1. Choose *File > Import* and click the file's format: Stereolith (.stl), Cart3D (.tri), NASCART (.dat) or XSec as Tri
   Mesh (.hrm).
2. Place it with the XForm tab, and scale it to the model's units with *Scale*.
3. Set its density on the Mass tab if it is to count in the mass properties.

### The result of an analysis

1. Run the analysis -- CompGeom, Mass Properties, Planar Slice, Projected Area.  The mesh it makes is added to the
   model.  Run from their screens, CompGeom, Mass Properties, Planar Slice and Wave Drag hide the components the
   mesh was made from.
2. Inspect it in the 3D view: turn on *Sub* in the Geom Browser to color it by component and sub-surface.
3. Delete it when done, or keep it as a record: it is saved with the model.

## Where a Mesh Comes From

A mesh is made by an import, by an analysis, or from another component.

| Source                                         | The mesh                                                        |
|:-----------------------------------------------|:----------------------------------------------------------------|
| File > Import                                  | The file's triangles.                                           |
| CompGeom                                       | The outer mold line: every component intersected with every other, inside parts removed, negative volumes cut out. |
| Mass Properties                                | The components' triangles, and the slices the mass was integrated over. |
| Planar Slice                                   | The components' triangles, and the cutting planes' cross sections. |
| Wave Drag                                      | The components' triangles, and the slices at the Mach angle.    |
| Projected Area                                 | The projected area, flat, and its outline.                      |
| Geometry Analysis, *Make MeshGeom*             | The triangles the analysis case worked on, and any slices.      |
| DegenGeom, *Make Mesh*                         | The degenerate geometry as triangles: plates for bodies, camber surfaces for wings. |
| Point Cloud, *Create Convex Hull Mesh*         | The convex hull of the points.                                  |
| File > Export, the mesh formats                | The triangles of the exported set.                              |

Mass Properties removes the mesh it made the previous time it ran, so only the latest is kept.  The mesh exports
(STL, Cart3D, OBJ and others) also build a mesh, and leave it in the model with every other component hidden.  An
export whose set already holds a mesh writes that mesh instead of building a new one, so a second export after the
model has changed writes the old shape; delete the mesh before exporting again.

## Importing a Mesh

The importers read triangles and nothing else: no units, no names of parts and no surface curvature.  OpenVSP's
model has no units, so a mesh comes in at the numbers in the file; scale it to match the model (*Scale* on the XForm
tab).

| Format            | Notes                                                                                     |
|:------------------|:------------------------------------------------------------------------------------------|
| Stereolith (.stl) | ASCII or binary, told apart by the file's content.  Several solids in one file make one mesh, in ASCII or as binary blocks, one per solid, as DATCOM writes them.  A binary file whose size does not match its facet count, such as one cut short, is refused.  The facet normals are read as given. |
| Cart3D (.tri)     | Formatted or unformatted, single or double precision, told apart by the file's content.  The component number of each triangle is kept as its tag. |
| NASCART (.dat)    | Rotated as it is read, from NASCART's axes ($Y$ up) to OpenVSP's ($Z$ up): the file's $(x, y, z)$ becomes $(x, -z, y)$. |
| XSec (.hrm)       | OpenVSP's cross section format.  Each component's grid of points is split into triangles, two per quadrilateral, and all components go into one mesh.  To keep the components apart as grids, import the same file as XSec as Wireframe instead (see [Wireframe](Wire.md)). |

A file that holds no triangles is not imported.

## Tags

Each triangle of a mesh can carry tags: numbers that say which component and which sub-surfaces it came from.  A
CompGeom mesh is tagged by component and, when sub-surfaces are intersected, by sub-surface; a Cart3D file brings
its own component numbers.  With *Sub* on in the Geom Browser, the mesh is drawn one color per combination of tags,
going round the color wheel.  *Start Color Degree* on the Other tab sets where on the wheel the first color is: 0
is red, 120 green and 240 blue.  It also colors the outlines a projected area draws.

## Converting a Mesh

The Other tab converts a mesh into two other components.  Each is a new component; the mesh is left as it was.

- *Convert* under Convert to Point Cloud makes a [Point Cloud](PtCloud.md) of the mesh's vertices, each once, where the
  mesh is placed.  The vertices of the slices are included when *Slices* is on.
- *Convert* under Convert to NGon Mesh makes an [NGon Mesh](NGonMesh.md): the triangles merged into polygons, with the
  wing wakes found, ready to write as a VSPAERO geometry file.  The NGon mesh is made where the mesh is placed.  The
  cleanup passes that collapse short segments along intersections and merge the fans of thin triangles at poles and
  tip caps are not run (see [How the Polygons Are Made](NGonMesh.md#how-the-polygons-are-made)); they are options of
  VSPAERO's own conversion.
  A mesh opened from a model file has lost the tables that say which component each triangle came from, so its
  conversion finds no wakes; convert a mesh in the session that made it.

## Mesh and Slices

A mesh from Mass Properties, Planar Slice or Wave Drag holds two sets of triangles: the components' mesh and the
slices.  *Mesh* and *Slices* on the Other tab choose which are drawn.  They also choose what an STL export of the mesh
writes and what a point cloud made from it holds, so turn *Slices* off to export the shape alone.

## Using a Mesh

A mesh takes part wherever triangles are enough.

- It is placed like any component: location, rotation, attachment, *Scale* and *Flip* on the XForm tab.  It makes
  no symmetric copies, so the XForm tab's symmetry controls are greyed, and so is *Rot Origin(X)*.
- CompGeom, Mass Properties, the slicing analyses, Projected Area, interference checks and the mesh exports take it
  as they take any component.  A mesh run through CompGeom again is intersected with the other components like any
  of them; it should be closed for the result to be right.
- Mass Properties gives it the density on its Mass tab, like any component.
- VSPAERO takes it as one or more components.
- It is saved with the model, triangles and all.

It is not a surface.  It has no sub-surfaces, its degenerate geometry is empty, and it takes no part
where a surface is needed: CFD Mesh, FEA Mesh, Fit Model, or as the boundary or direction of a projection.  Its
triangles cannot be edited in OpenVSP; change the geometry it came from and run the analysis again, or edit the file
and import it again.

## Scripting

A mesh is made by importing a file or by running an analysis; each returns the new mesh's ID.

| Function                                 | Use                                                         |
|:-----------------------------------------|:------------------------------------------------------------|
| `ImportFile( file, IMPORT_STL, "" )`     | Import a file as a mesh (`IMPORT_CART3D_TRI`, `IMPORT_NASCART`, `IMPORT_XSEC_MESH` for the other formats). |
| `ComputeCompGeom( set, half, types )`    | Run CompGeom and return the mesh's ID.                      |
| `CreatePtCloudGeom( mesh_id )`           | Convert to a point cloud, as the Convert to Point Cloud button.      |
| `CreateNGonMeshGeom( mesh_id )`          | Convert to an NGon mesh, as the Convert to NGon Mesh button.         |

The Other tab's toggles are the Parms `MeshFlag`, `SliceFlag` and `StartColorDegree` in the group `Draw`.  The
scale the mesh has been given in total is `Scale_From_Original` in the group `XForm`.

```python
import openvsp as vsp

mesh_id = vsp.ImportFile( "part.stl", vsp.IMPORT_STL, "" )
vsp.SetParmVal( mesh_id, "Scale", "XForm", 0.0254 )     # inches to meters
vsp.Update()
```
