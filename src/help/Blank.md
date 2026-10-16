---
title:  'Blank'
---

The Blank component has no shape.  It is a coordinate frame -- an origin and a set of axes -- placed in the model like
any other component, and drawn in the 3D view as an axis triad at its origin (red $X$, green $Y$, blue $Z$), with one
triad at each symmetric copy.  With *Pick* on in the Geom Browser, click the triad to select the Blank.

| Topic                                       | Tab              |
|:--------------------------------------------|:-----------------|
| [What a Blank Is For](#what-a-blank-is-for) |                  |
| [Point Mass](#point-mass)                   | Mass             |
| [Shared Tabs](#shared-tabs)                 | Gen, XForm, Mass |
| [Scripting](#scripting)                     |                  |

## What a Blank Is For

A Blank gives a model a frame that belongs to no part of the airframe.  It is used to:

- Hold a group of components together.  Components attached to a Blank move with it, so an engine installation, a
  cockpit or a payload bay built under a Blank is moved or turned by changing the Blank alone.  Put a Blank at the top
  of any assembly whose origin should not be on one of its parts.
- Carry a mass that has no shape, such as equipment, fuel or passengers (see [Point Mass](#point-mass)).
- Mark a point for components that refer to a point rather than a surface.  A [Routing](Routing.md) point on a Blank
  lies at the Blank's origin, so a route can run through free space.  An [Auxiliary](Auxiliary.md) super cone or rotor
  fragment zone hung from a Blank sits at the Blank's origin with the Blank's axes, which is more flexible than the
  origin of the component it describes.

A Blank has no surface, so it is left out of every analysis that works on shape (CompGeom, DegenGeom, CFD meshing,
export).  Children attached to it by UW, RST, LMN or $\eta$MN coordinates fall back to its origin and axes.  A Blank
cannot be flipped: its axes always stay right-handed.

## Point Mass

A Blank's only mass is the concentrated mass of the *Specified Mass Properties* on the Mass tab (see
[Mass](MassTab.md)); the density, priority and thin-shell settings are greyed, since a Blank has no volume or area for
them to act on.  The center of gravity is measured from the Blank's origin in its own axes, so a Blank placed at an item
of equipment carries that item's mass with its center of gravity at zero.  While the Blank is selected and has a mass,
the 3D view marks the center of gravity with a blue point.

## Shared Tabs

The Gen, XForm and Mass tabs are common to every component; see [Gen](GenTab.md), [XForm](XFormTab.md)
and [Mass](MassTab.md).  A Blank has no Sub tab, since it has no surface to carry sub-surfaces.

## Scripting

A script adds a Blank with `AddGeom( "BLANK" )` and sets its point mass through the parms `PointMass`, `CGx`, `CGy`,
`CGz` and `Ixx` to `Iyz` in the group `Mass_Props`.
