# Model Changes in OpenVSP 3.56

OpenVSP 3.56 changes what six sets of parameters mean, and fixes others that did not do what they said.  A model saved
by an earlier version opens as it was saved, with no conversion, so the models listed here change shape.  Each entry
says which models are affected and how to keep the earlier shape.

## Parameters whose meaning changed

### Propeller: reverse rotation

Feather Offset, and the fold axis's Offset/R, Azimuth and Elevation, now mirror with Reverse Rotation, so a reversed
propeller is the mirror image of the normal one.

- Affected: propellers with Reverse Rotation and any of these set.
- To keep the earlier shape: change their signs.

### Propeller: feather bearing

A blade now feathers at its feather bearing, Radial/R 0.15, instead of outboard of every hinge.

- Affected: propellers with both a Fold and a Feather angle whose fold hinge Radial/R is above 0.15 (the default is
  0.20).  The blade now folds about a feathered hinge.
- To keep the earlier shape: set the feather bearing Radial/R to 1.0.

### Auxiliary: rotor tip path

Automatic r_flap/R is new, and on by default: r_flap/R follows the propeller's flap hinge, Radial/R 0.05 by default.

- Affected: every rotor tip path on a propeller.
- To keep the earlier shape: turn Automatic r_flap/R off and enter r_flap/R again.

### Wing: blending

A positive inboard Dihedral, at the leading or the trailing edge, now raises the edge going outboard, as the outboard
Dihedral does.

- Affected: sections blended with an inboard Dihedral.  An outboard edge matched to the inboard angles is now smooth
  instead of creased.
- To keep the earlier shape: change the sign of the inboard Dihedral.

### Fuselage and Stack: chevrons

The Right Amplitude and Left Amplitude now drive the sides they name; each used to drive the other.

- Affected: chevrons whose right and left amplitudes differ.
- To keep the earlier shape: swap the two values.

### Gear: retraction mechanism

The strut now turns about the knee axis the 3D view draws, which the Bend Angle turns with the upper strut.

- Affected: gears with a Bend Angle other than 180 whose knee axis is not along the trunnion axis.  They retract
  differently: 29.6 degrees apart at Bend Angle 120 and Azimuth Angle 60.
- To keep the earlier motion: set the knee Azimuth Angle and Elevation Angle again.

### Values kept outside a component

Variable presets and Modes, links and advanced links, design variable files and scripts keep their old numbers, which
take the new meanings.  A model saved by OpenVSP 3.56 and opened in an earlier version takes the earlier meanings
there, and changes shape.

## Fixes that change saved models

- Propeller: a plain flap set on the sections, in models saved since 3.46.0, now shows; it had no effect.
- Propeller: flaps given in model units are sized in the same update as a Diameter or chord change, not one update
  late.
- Wing and other lifting surfaces: plain flaps whose chord and arc reach past 0.95 of the section are laid out
  slightly differently, which can move the loft between sections a little.
- Engine model: a flowpath with an inlet and an outlet in Inlet Mode FLOWTHROUGH NEGATIVE ONLY is now negative; it was
  positive.
- Gear: a tire shoulder width or height given in model units keeps the value given; it was recomputed on every update.
- Hinge: a Hinge oriented by Vectors with the Axis and Reference X and Z, Y and X, or Z and Y now lies on the
  direction given; its frame was left-handed, and the hinge axis pointed elsewhere.
- Auxiliary: a 3pt Composite Clearance Envelope's main gear lies exactly the Main Gear Offset from the track; it fell
  slightly short (by about 0.25% for 24 in on a 40 ft wheelbase).
- Custom components: a script without ComputeCenter, with a nonzero Origin and a last section past the next-to-last,
  now takes the rotation origin from the last section (OnOffExample at Origin 1: 1.5 becomes 3.0).
- Human: components attached to a Human, such as a cone at the design eye, follow the Human's pose and size as they
  change.
- 6-series airfoil: a section with Series one past 65A, set by a script, is built as 65A with A 0.8; it built a
  63-series section.
- Conformal: a Conformal attached to a Gear opens with no parent, and is empty.
