# VSPAERO Isollated Rotor Example Case

This test case analyzes a stimple 5-bladed propeller (or rotor) in near-static operation.
VSPAERO can analyze true hover, but by allowing some freestream velocity, we can better
compare with some other tools like XRotor that require some finite velocity.

VSPAERO supports two ways of modeling the rotating blades of an isolated rotor.  The first
(steady rotating frame) takes advantage of symmetry and some unusual characteristics of
potential theory formulation.  The second (unsteady rotating blades) applies a much more
general approach to this simple case.

Instead of rotating the blades, the steady rotating frame approach holds the blades still
and rotates the freestream around the X-axis.  This requires the model and flow to be
symmetrical about (and aligned with) the X-axis. 

Both examples start from identical OpenVSP `*.vsp3` models and identical `*_DegenGeom.csv`
files, located in this directory.

* [Steady Rotating Frame](./Steady_Rotating_Frame/README.md)
* [Unsteady Rotating Blades](./Unsteady_Rotating_Blades/README.md)
