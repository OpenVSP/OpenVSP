# VSPAERO Isolated Rotor Steady Rotating Frame Example Case

Instead of rotating the blades, the steady rotating frame approach used in this example
holds the blades still and rotates the freestream around the X-axis.  This requires the
model and flow to be symmetrical about (and aligned with) the X-axis.  The resulting
flow is steady in the frame of the blades.  In this case, wake relaxation is used in the
solution process.  However, the wake relaxation does not represent a time-accurate phenemona. 

The `prop_DegenGeom.csv` file in this directory can be re-created by running the DegenGeom
analysis in OpenVSP.

Run the following command from the command line in the directory with the `prop_DegenGeom.csv`
and `prop_DegenGeom.vspaero` files.

```
vspaero -omp 4 -rotor 859.22 prop_DegenGeom
```

Once complete, `vspaero` will write several output files that include solution history,
resulting forces/moments, etc.  To visualize the solution, run the following command from
the same directory.

```
vspviewer prop_DegenGeom
```
