# VSPAERO Isolated Rotor Unsteady Rotating Blade Example Case

In this case, VSPAERO uses a very general unsteady solver to solve a flow that includes
rotating objects.  The motion of those rotating objects is described in a `*.groups` file.
This case is relatively simple, with just one rotating component, so the `*.groups` file
is correspondingly simple.

To run this case, place the `prop_DegenGeom.csv` file from the parent directory in the
same place as the `prop_DegenGeom.vspaero` and `prop_DegenGeom.groups` files from this
directory.  Then run the following command from the command line in that directory.

```
vspaero -omp 4 -unsteady prop_DegenGeom
```

Once complete, `vspaero` will write several output files that include solution history,
resulting forces/moments, etc.  To visualize the solution, run the following command from
the same directory.

```
vspviewer prop_DegenGeom
```
