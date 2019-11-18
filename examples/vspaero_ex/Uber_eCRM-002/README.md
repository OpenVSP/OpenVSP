# VSPAERO Multi-Rotor Vehicle Example Case

In this case, VSPAERO uses a very general unsteady solver to solve a flow that includes
rotating objects.  The motion of those rotating objects is described in a `*.groups` file.
This case is relatively complex, with nine groups defined.  The `*.groups` file
is correspondingly longer, but still straightforward.

To run this case, place the three `uber2_DegenGeom.*` files from this directory in the
same directory.  Then run the following command from the command line in that directory.

```
vspaero -omp 4 -unsteady uber2_DegenGeom
```

Once complete, `vspaero` will write several output files that include solution history,
resulting forces/moments, etc.  To visualize the solution, run the following command from
the same directory.

```
vspviewer uber2_DegenGeom
```
