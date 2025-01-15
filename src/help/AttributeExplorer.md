---
title:  'Attribute Explorer'
---

The Attribute Explorer allows the user to view and manipulate Attributes.

Attributes allow the user to read and write additional model information. Attributes are process-agnostic in
OpenVSP, and are free to represent virtually any kind of information, ranging from user notes on design intent
to input parameters for downstream engineering analysis tools.

Attributes are stored in Attribute Collections or Attribute Groups; most OpenVSP objects have an Attribute
Collection, such that a user may store Attributes in the Vehicle, its Geoms, Subsurface, Links, Advanced Links,
Measures, Parms, Cross sections, Wing Sections, and even in other Attributes. Any OpenVSP object supporting
Attributes will have a minimized attribute explorer at the bottom of its GUI window. These attributes are
accessible both in GUI and API commands, allowing for user edits and scripted operations of integrated design
toolchains.

## Attribute Types

Attributes can store a variety of datatypes.

| Attribute Types |
|:----------------|
| Boolean         |
| Integer         |
| Double          |
| String          |
| Parms           |
| Vec3D           |
| Integer Matrix  |
| Double Matrix   |
| Attribute Group |

Vec3D and Matrix type Attributes support copy/paste with standard spreadsheet software for ease of use. They also
support renamable column and row headers by creating a string attribute of the name "row_{name of matrix attribute}"
and entering a comma-delimited series of header names. This name attribute must be within the same collection/group
as the matrix/Vec3D attribute for this to work.

The Parm type and Attribute Group type also have some eccentricities.

To ensure consistency in user workflow and API access methods, the Parm type attribute can be used to reference
the value of any Parm in the OpenVSP vehicle. In the Explorer GUI, it has a Parm Picker with dropdown menus, and
the user may simply drag and drop a ParmID onto the GUI space to copy it easily. Once referencing a valid Parm, the
GUI mirrors the slider used by that Parm, permitting the user to drive the parm from the Attribute Explorer. Linking
a user-defined parm with several parm-type attributes can powerfully drive metadata such as material properties or
design variables for downstream analysis tools.

To encourage clean organization of user Attributes, Attribute Groups permit hierarhicial nesting of attributes.
A user may use attribute groups to organize the attributes by intended downstream analysis tool, or enable multiple
attributes of the same name (attributes must have a unique name within each collection or group).

## Attributable Objects

Attributes can be added to many OpenVSP objects.

| Attributable Objects |
|:---------------------|
| Vehicle              |
| Geom                 |
| SubSurface           |
| Xsec                 |
| Wing Section         |
| Airfoil Section      |
| Parm                 |
| Measure              |
| Link                 |
| AdvLink              |
| Mode                 |
| GeomSet              |
| VarPreset            |
| Attribute            |

## GUI Operations

The GUI has been designed to enable clean, easy Attributes work. Copy/Cut/Paste commands exist for moving attributes
in and out of different collections and groups. Cut/Copy/Paste/Delete all have hotkeys available for ease of use.
Most GUI operations available for Attributes work for multiple selections of attributes and collections at once- e.g.,
a user can copy a single attribute and paste into multiple collections/groups.

Due to the vast quantity of attributable objects in an OpenVSP vehicle, the explorer only shows attribute collections
either if they are populated with attributes or if their objects' associated GUI window has been opened. For example,
a user would add a Geom, and then select that Geom to open its GUI window- once that window is opened the explorer shows
the empty attribute collection for user access. Multiple geoms may be selected at once for expediency.

The Attribut Explorer also has search functions available for sorting through complicated models. A user may search by
name string, with or without case sensitivity

## API Usage

Attributes support data transfer into and out of OpenVSP via the API. This supports use of attribute data for upstream
scripted model generation and for downstream analysis toolchains. The API is fully documented in the docs.