---
title:  'Geometry Analysis'
---

The Geometry Analysis Manager allows the user to set up and execute several geometric analysis tasks typical of
the aircraft design process.

Each analysis task is configured with the geometry or geometries required.  The primary geometry can be either a
Set, Mode, or an individual component.  The secondary geometry (when required) can be either a Set, an individual
component, or (in special cases) direct specification of a point or plane.

Some of the analysis tasks require a specific type of geometry for the analysis.  For example, the Plane 2pt Angle
analysis used to calculate the tail strike angle requires a 2pt Ground Plane Auxiliary Geometry for its secondary
input.  These requirements are summarized in the following table.

| Geometry Analysis Type                                        | Primary Geometry | Secondary Geometry                 |
|:--------------------------------------------------------------|:-----------------|------------------------------------|
| [External](#external)                                         | Any              | Any                                |
| [Packaging](#packaging)                                       | Any              | Any                                |
| [Self External](#self-external)                               | Any              | N/A                                |
| [Plane Distance](#plane-distance)                             | Any              | ZGround, Gear, or 3pt Ground Plane |
| [Plane 2pt Angle](#plane-2pt-angle)                           | Any              | 2pt Ground Plane                   |
| [Tipback](#tipback)                                           | 2pt Ground Plane | N/A                                |
| [Plane 1pt Angle](#plane-1pt-angle)                           | Any              | 1pt Ground Plane                   |
| [Weight Distribution](#weight-distribution)                   | 3pt Ground Plane | N/A                                |
| [Tipover](#tipover)                                           | 2pt Ground Plane | N/A                                |
| [Ground Maneuverability](#ground-manueverability)             | Any              | 3pt Ground Plane                   |
| [From Point Visibility](#from-point-visibility)               | Any              | XYZ, HumanGeom, or Geom            |
| [Composite Clearance Envelope](#composite-clearance-envelope) | Any              | 3pt Composite Clearance Envelope   |
| [Swept Volume](#swept-volume)                                 | Any              | Any or HingeGeom                   |
| [Look At Visibility](#look-at-visibility)                     | Any              | N/A                                |


## External

Check that the primary and secondary geometries are external to one another.  Before checking for interference,
a CompGeom type analysis is run on both primary and secondary geometry sets to compute each trimmed OML -- this
step allows the geometry sets to include negative components.

## Packaging

Check that the secondary geometry is contained within the primary geometry.  Before checking for interference,
a CompGeom type analysis is run on both primary and secondary geometry sets to compute each trimmed OML -- this
step allows the geometry sets to include negative components.

## Self External

Check that all surfaces of the primary geometry are external to one another.  No CompGeom type analysis is run
on the geometry, so the effect of negative components can not be considered.  No secondary geometry is required.

This analysis should be used instead of the [External](#external) analysis when you want to check external
interference between a number of simple components or between multiple surfaces of a single geometry -- as
created by either symmetry or a complex component like a propeller.

## Plane Distance

Calculate the minimum and maximum height of the primary geometry above a plane specified by the secondary geometry.
Before calculating distance, a CompGeom type analysis is run on the primary geometry set to compute each trimmed
OML -- this step allows the geometry set to include negative components.  The secondary geometry is used to specify
the static reference plane.  The static reference plane can be specified as a Z-constant plane, as the nominal ground
plane from a landing gear component, or by a 3pt Ground Plane auxiliary geometry.

## Plane 2pt Angle

Calculate the angle of first contact betwen the primary geometry and a ground plane rotating about an axis specified
by the secondary geometry.  Before calculating the angle, a CompGeom type analysis is run on the primary geometry set
to compute each trimmed OML -- this step allows the geometry set to include negative components.  The secondary
geometry must be specified by a 2pt Ground Plane auxiliary geometry.

If the specified contact points are bogies with multiple wheels in tandem, rotation will occur about the line connecting
the bogie pivots up to the maximum bogie pivot angle.  Beyond that limit, rotation will occur about the line connecting
the appropriate wheel axles while the bogies are at maximum pivot.  If the contact points do not have multiple wheels
in tandem, rotation will occur about the line connecting the wheel axles.

## Tipback

Calculate the angle from vertical to the center of gravity about the line connecting the landing gear pivots.  The
pivot point and center of gravity envelope are specified by a 2pt Ground Plane auxiliary geometry.  No secondary
geometry is required. The tipback angle is calculated to all eight possible corners of the CG envelope as well as
the nominal CG position.

## Plane 1pt Angle

Calculate the angle of first contact betwen the primary geometry and a ground plane rotating about a roll axis specified
by the secondary geometry.  Before calculating the angle, a CompGeom type analysis is run on the primary geometry set
to compute each trimmed OML -- this step allows the geometry set to include negative components.  The secondary
geometry must be specified by a 1pt Ground Plane auxiliary geometry.

The roll axis goes through the outboard contact point of the specified landing gear in the forward direction in the
ground plane.

## Weight Distribution

Calculate the fraction of the weight reacted by each contact point specified by the primary geometry.  The contact
points and center of gravity envelope are specified by a 3pt Ground Plane auxiliary geometry.  No secondary geometry
is required.

## Tipover

Calculate the angle from vertical to the center of gravity about the line connecting the landing gear contact points.
The contact points and center of gravity envelope are specified by a 2pt Ground Plane auxiliary geometry.  No secondary
geometry is required. The tipover angle is calculated to all eight possible corners of the CG envelope as well as
the nominal CG position.

## Ground Manueverability

Calculate the largest arc swept by the primary geometry as well as the ground tracks of the landing gear contact points
specified by the secondary geometry.  Before calculating a CompGeom type analysis is run on the primary
geometry set to compute each trimmed OML -- this step allows the geometry set to include negative components.  The
secondary geometry must be specified by a 3pt Ground Plane auxiliary geometry.  The contact point with the largest
allowed turning angle is treated as the steerable gear.  The steering angle to produce the tightest possible turn
is used (up to the turning angle limit).

## From Point Visibility

Calculate the visible (or occluded) domain from a specific point in space.  Before calculating visibility,
a CompGeom type analysis is run on the primary geometry set to compute each trimmed OML -- this
step allows the geometry set to include negative components.  The secondary geometry is used to specify
the viewpoint.

## Composite Clearance Envelope

Calculate the minimum ground clearance between the primary geometry and a composite clearance envelope specified by
the secondary geometry.  Before calculating clearance, a CompGeom type analysis is run on the primary geometry set
to compute each trimmed OML -- this step allows the geometry set to include negative components.  The secondary
geometry must be specified by a 3pt Composite Clearance Envelope auxiliary geometry.

## Swept Volume

Perform an [External](#external) interference check where the linear swept volume of the secondary geometry is computed
before interference is checked.  Like the [External](#external) analysis, a CompGeom type analysis is run on both
primary and secondary geometry sets to compute each trimmed OML -- this step allows the geometry sets to include
negative components.

If the secondary geometry is a HingeGeom with linear motion enabled, the swept volume's displacement is obtained from
the HingeGeom.  Otherwise, the direction of displacement is provided by the user.

Positive and negative displersion angles can be specified.  These angles rotate the swept volume's direction in the
X, Y, or Z directions.  For typical problems, the X dispersion will control dispersion from the front view and the
Y dispersion will control dispersion from the side view.

## Look At Visibility

Calculate the model's visibility from a specific direction.  Before calculating visibility,
a CompGeom type analysis is run on the primary geometry set to compute each trimmed OML -- this
step allows the geometry set to include negative components.

The visible wetted area, visible projected area, and the equivalent solar areas are calculated on a per-surface,
per-tag, and per-subsurface basis.

The view direction is specified as azimuth and elevation angles from the model's perspective -- i.e. positive up
and to the right from the pilot's perspective.

The equivalent solar area uses the user specified index of refraction of the glass (or optical coating) of the solar
cell to compute the Fresnel reflectance correction to the solar cell's effective area.
