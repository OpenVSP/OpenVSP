# VSPAERO Isollated Rotor Example Case

This test case analyzes a stimple 5-bladed propeller (or rotor) in near-static operation.
VSPAERO can analyze true hover, but by allowing some freestream velocity, we can better
compare with some other tools like XRotor that require some finite velocity.

VSPAERO supports two ways of modeling the rotating blades of an isolated rotor.  The first
(steady rotating frame) takes advantage of symmetry and some unusual characteristics of
potential theory formulation.  The second (unsteady rotating blades) applies a much more
general approach to this simple case.

Both examples start from the OpenVSP `prop.vsp3` model located in this directory.

## Steady Rotating Frame

Instead of rotating the blades, the steady rotating frame approach holds the blades still
and rotates the freestream around the X-axis.  This requires the model and flow to be
symmetrical about (and aligned with) the X-axis.  This mode of operation is not supported
by the OpenVSP GUI and can only be accessed from the command line.

Detailed instructions are available via the link for running a
[steady rotating frame](./Steady_Rotating_Frame/) example case.

## Unsteady Rotating Blades

In this case, VSPAERO uses a very general unsteady solver to solve a flow that includes
rotating objects.  The motion of those rotating objects is described in a `*.groups` file.
All of the required input files and execution of vspaero are now handled by the OpenVSP GUI.
To run this case, open the `prop.vsp3` file in this directory, go to the `VSPAERO...` GUI
and click `Launch Solver`.
