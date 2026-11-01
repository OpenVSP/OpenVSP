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
| [Wetted Area and Volume](#wetted-area-and-volume)             | Any              | N/A                                |
| [Planar Slice](#planar-slice)                                 | Any              | N/A                                |
| [Projected Area](#projected-area)                             | Any              | Any                                |
| [Mass Properties](#mass-properties)                           | Any              | N/A                                |
| [External Interference](#external-interference)               | Any              | Any                                |
| [Self External Interference](#self-external-interference)     | Any              | N/A                                |
| [Packaging Interference](#packaging-interference)             | Any              | Any                                |
| [Plane Min/Max Distance](#plane-minmax-distance)              | Any              | ZGround, Gear, or 3pt Ground Plane |
| [Plane 2pt Angle Contact](#plane-2pt-angle-contact)           | Any              | 2pt Ground Plane                   |
| [Plane 1pt Angle Contact](#plane-1pt-angle-contact)           | Any              | 1pt Ground Plane                   |
| [Tipback Angle](#tipback-angle)                               | 2pt Ground Plane | N/A                                |
| [Tipover Angle](#tipover-angle)                               | 2pt Ground Plane | N/A                                |
| [Weight Distribution](#weight-distribution)                   | 3pt Ground Plane | N/A                                |
| [Ground Maneuverability](#ground-maneuverability)             | Any              | 3pt Ground Plane                   |
| [Composite Clearance Envelope](#composite-clearance-envelope) | Any              | 3pt Composite Clearance Envelope   |
| [From Point Visibility](#from-point-visibility)               | Any              | XYZ, HumanGeom, or Geom            |
| [Look At Visibility](#look-at-visibility)                     | Any              | N/A                                |
| [Swept Volume](#swept-volume)                                 | Any              | Any or HingeGeom                   |
| [Risk Angle](#risk-angle)                                     | Any              | Rotor Fragment or Thrown Blade     |
| [Aero Center](#aero-center)                                   | Wing             | N/A                                |

## Wetted Area and Volume

Calculate the wetted area and volume of the primary geometry using CompGeom to compute the trimmed OML -- this
step allows the geometry set to include negative components.

## Planar Slice

Compute the cross sectional area distribution of the primary geometry.  Although the OML is not calculated, this
computation does honor negative components.

## Projected Area

Compute the projected area of the primary geometry in a specified direction.  The bounded projected area can also
be computed where the boundary is specified by the secondary geometry.  The convex hull of either the primary or
secondary geometry can also be computed.

## Mass Properties

Compute the mass, center of mass, and inertias of the primary geometry.  The trimmed OML of the geometry contributes
to the mass properties through a per-area density value assigned to each component.  The volume of each component
contributes through a per-volume density value assigned to each component.  Where multiple components overlap, the
density value for the highest priority component is used.  Per-length density values are used to calculate the
mass property contribution of Routing Geoms.  Any object can also include specified mass properties that
will be included.

## External Interference

Check that the primary and secondary geometries are external to one another.  Before checking for interference,
a CompGeom type analysis is run on both primary and secondary geometry sets to compute each trimmed OML -- this
step allows the geometry sets to include negative components.

## Self External Interference

Check that all surfaces of the primary geometry are external to one another.  No CompGeom type analysis is run
on the geometry, so the effect of negative components can not be considered.  No secondary geometry is required.

This analysis should be used instead of the [External Interference](#external-interference) analysis when you want
to check external interference between a number of simple components or between multiple surfaces of a single
geometry -- as created by either symmetry or a complex component like a propeller.

## Packaging Interference

Check that the secondary geometry is contained within the primary geometry.  Before checking for interference,
a CompGeom type analysis is run on both primary and secondary geometry sets to compute each trimmed OML -- this
step allows the geometry sets to include negative components.

## Plane Min/Max Distance

Calculate the minimum and maximum height of the primary geometry above a plane specified by the secondary geometry.
Before calculating distance, a CompGeom type analysis is run on the primary geometry set to compute the trimmed
OML -- this step allows the geometry set to include negative components.  The secondary geometry is used to specify
the static reference plane.  The static reference plane can be specified as a Z-constant plane, as the nominal ground
plane from a landing gear component, or by a 3pt Ground Plane auxiliary geometry.

## Plane 2pt Angle Contact

Calculate the angle of first contact betwen the primary geometry and a ground plane rotating about an axis specified
by the secondary geometry.  Before calculating the angle, a CompGeom type analysis is run on the primary geometry set
to compute each trimmed OML -- this step allows the geometry set to include negative components.  The secondary
geometry must be specified by a 2pt Ground Plane auxiliary geometry.

If the specified contact points are bogies with multiple wheels in tandem, rotation will occur about the line connecting
the bogie pivots up to the maximum bogie pivot angle.  Beyond that limit, rotation will occur about the line connecting
the appropriate wheel axles while the bogies are at maximum pivot.  If the contact points do not have multiple wheels
in tandem, rotation will occur about the line connecting the wheel axles.

## Plane 1pt Angle Contact

Calculate the angle of first contact betwen the primary geometry and a ground plane rotating about a roll axis specified
by the secondary geometry.  Before calculating the angle, a CompGeom type analysis is run on the primary geometry set
to compute each trimmed OML -- this step allows the geometry set to include negative components.  The secondary
geometry must be specified by a 1pt Ground Plane auxiliary geometry.

The roll axis goes through the outboard contact point of the specified landing gear in the forward direction in the
ground plane.

## Tipback Angle

Calculate the angle from vertical to the center of gravity about the line connecting the landing gear pivots.  The
pivot point and center of gravity envelope are specified by a 2pt Ground Plane auxiliary geometry.  No secondary
geometry is required. The tipback angle is calculated to all eight possible corners of the CG envelope as well as
the nominal CG position.

## Tipover Angle

Calculate the angle from vertical to the center of gravity about the line connecting the landing gear contact points.
The contact points and center of gravity envelope are specified by a 2pt Ground Plane auxiliary geometry.  No secondary
geometry is required. The tipover angle is calculated to all eight possible corners of the CG envelope as well as
the nominal CG position.

## Weight Distribution

Calculate the fraction of the weight reacted by each contact point specified by the primary geometry.  The contact
points and center of gravity envelope are specified by a 3pt Ground Plane auxiliary geometry.  No secondary geometry
is required.

## Ground Maneuverability

Calculate the largest arc swept by the primary geometry as well as the ground tracks of the landing gear contact points
specified by the secondary geometry.  Before calculating a CompGeom type analysis is run on the primary
geometry set to compute each trimmed OML -- this step allows the geometry set to include negative components.  The
secondary geometry must be specified by a 3pt Ground Plane auxiliary geometry.  The contact point with the largest
allowed turning angle is treated as the steerable gear.  The steering angle to produce the tightest possible turn
is used (up to the turning angle limit).

## Composite Clearance Envelope

Calculate the minimum ground clearance between the primary geometry and a composite clearance envelope specified by
the secondary geometry.  Before calculating clearance, a CompGeom type analysis is run on the primary geometry set
to compute each trimmed OML -- this step allows the geometry set to include negative components.  The secondary
geometry must be specified by a 3pt Composite Clearance Envelope auxiliary geometry.

## From Point Visibility

From Point Visibility can work in continuous or discrete mode.  In continuous mode, it will
calculate the visible (or occluded) domain from a specific point in space.  In discrete mode, it will
calculate the visibility from a specific point in space along a set of specified directions.

Before calculating visibility, a CompGeom type analysis is run on the primary geometry set to compute the
trimmed OML -- this step allows the geometry set to include negative components.  The secondary geometry is
used to specify the viewpoint.

## Look At Visibility

Calculate the model's visibility from a specific direction.  Before calculating visibility,
a CompGeom type analysis is run on the primary geometry set to compute the trimmed OML -- this
step allows the geometry set to include negative components.

The visible wetted area, visible projected area, and the equivalent solar areas are calculated on a per-surface,
per-tag, and per-subsurface basis.

The view direction is specified as azimuth and elevation angles from the model's perspective -- i.e. positive up
and to the right from the pilot's perspective.

The equivalent solar area uses the user specified index of refraction of the glass (or optical coating) of the solar
cell to compute the Fresnel reflectance correction to the solar cell's effective area.

## Swept Volume

Perform an [External Interference](#external-interference) check where the linear swept volume of the
secondary geometry is computed before interference is checked.  Like the
[External Interference](#external-interference) analysis, a CompGeom type analysis is run on both
primary and secondary geometry sets to compute each trimmed OML -- this step allows the geometry sets to include
negative components.

If the secondary geometry is a HingeGeom with linear motion enabled, the swept volume's displacement is obtained from
the HingeGeom.  Otherwise, the direction of displacement is provided by the user.

Positive and negative displersion angles can be specified.  These angles rotate the swept volume's direction in the
X, Y, or Z directions.  For typical problems, the X dispersion will control dispersion from the front view and the
Y dispersion will control dispersion from the side view.

## Risk Angle

Calculate the translational risk angle (in degrees) for a fragment thrown from rotating machinery interfering with
a geometry as described in FAA AC 20-128A.  If the fragment trajectory misses the target geometry, the risk angle
is zero degrees.  The bounding release angles of the fragment are also calculated.

Before calculating the risk angle, a CompGeom type analysis is run on the primary geometry set to compute the
trimmed OML -- this step allows the geometry set to include negative components.  The secondary geometry is
used to describe the fragment, it must be specified by either an AC 20-128A Rotor Fragment or AC 25.905-1 Thrown Blade
auxiliary geometry.

## Aero Center

Calculate the incompressible aerodynamic center of a single wing geom.  This analysis uses VSPAERO to run a
simple analysis at zero and one degree angles of attack.  The change in force and moment coefficients from
those cases are used to calculate the location of the aerodynamic center of the isolated lifting surface.

This analysis uses a thin-surface representation utilizing the component's tessellated resolution.  The
analysis is run at Mach=0 and ReCref=1e7 with a rigid wake.  Only the inviscid force and moment coefficients
are used in the calculation.
