# HumanGeom anthropometric data

This directory holds the **original, full-resolution** anthropometric data for
`HumanGeom`, the script that reduces it into the form actually compiled into
OpenVSP, and the program that generates the Human's Bezier surface data.

## Files

| File | Role |
| ---- | ---- |
| `MaleGeomData.h`, `FemaleGeomData.h` | Original 200-principal-component data. **Archived only -- no longer compiled into OpenVSP.** Kept as the source of truth for regeneration. |
| `reduce_geomdata.py` | Converter. Reads the originals, collapses the two-stage PCA, and writes the compiled headers. |
| `matlab/` | The MATLAB scripts that made the originals from the UMTRI model files; see `matlab/README.md`. |
| `repair_toes.py` | Repairs the UMTRI fine mesh: stray vertices and tangled faces on the right foot's second and third toes, then the left half made the exact mirror image of the right (the model is symmetric but for the left toes' and eyes', mouth's and ears' faces).  Writes the repair into the originals (`--write`; run `reduce_geomdata.py` after it) and, with `--fine-out DIR --model-out`, a whole repaired copy of the UMTRI model files for a future reduction.  It needs the UMTRI model files. |
| `../MaleGeomDataReduced.h`, `../FemaleGeomDataReduced.h` | **Generated** headers that `HumanGeom.cpp` includes and compiles. Do not edit by hand. |

## What the reduction does

The original model reconstructs a body mesh (and skeleton) from a two-stage
principal-component fit:

```
score[200] = coeffs[200][6] . X[6]
Y          = Ybar + P[N][200] . score[200]
```

where `X = { stature, BMI, sit-fraction, age, BMI*age, 1 }` is the six-element
anthropometric input vector and `P` is the principal-component matrix
(`*_half_pcs` for the mesh, `*_skel_pcs` for the skeleton).

Because matrix multiplication is associative and `X` has only six entries, the
two stages fold into a single precomputed linear map `Q = P . coeffs`:

```
Y = Ybar + (P . coeffs) . X = Ybar + Q . X       Q[N][6]
```

This makes the compiled data about **33x smaller** (~14 MB -> ~0.43 MB) and the
per-update reconstruction about **33x cheaper**, with no meaningful change in the
resulting geometry. The product is formed in double precision (the literals in
the originals carry full double-precision digits even though the trailing `f`
stores them as `float`), then written back out as double-precision text with the
`f` restored so MSVC still compiles the tables quickly.

Some arrays (`*_skel_verts`, `*_skel_pcs`) are declared larger than the data they
initialize; C++ zero-fills the remaining rows. The converter reproduces this
exactly by emitting the same number of rows.

## Regenerating (e.g. after an upstream data update)

1. Replace `MaleGeomData.h` / `FemaleGeomData.h` here with the new upstream data
   (same array names, dimensions, and `HumanGeom::` qualification).
2. Run the converter:

   ```
   python3 reduce_geomdata.py
   ```

   It rewrites `../MaleGeomDataReduced.h` and `../FemaleGeomDataReduced.h`.
3. Rebuild OpenVSP.

The converter needs NumPy (used only for the double-precision matrix multiply).

## The Bezier surface: `human_bezier.py`

`HumanGeom` can also represent the body by closed piecewise Bezier surfaces (the
`Representation` Parm).  Their data are generated offline into
`../HumanBezierData.h` by `human_bezier.py`; building OpenVSP does not need Python.

| File | Role |
| ---- | ---- |
| `human_bezier.py` | Main program.  Builds the parts, fits them, writes `../HumanBezierData.h`; `--check` also prints the measured errors. |
| `hb_data.py` | Reads `../MaleGeomDataReduced.h` and `../FemaleGeomDataReduced.h`; the closed full mesh and the skeleton. |
| `hb_layout.py` | The parts: cut planes, regions, and each part's axis, net size and ends. |
| `hb_quilt.py` | The closed B-spline quilts and the linear fit operator. |
| `hb_check.py` | The measurements of `--check`. |
| `../HumanBezierData.h` | **Generated.**  Do not edit by hand. |

### The parts

The body is twenty overlapping closed parts: trunk, head and neck, and per side
arm, palm, thumb, four fingers, leg and foot (fused toes).  Each is one bicubic
B-spline net, periodic around and clamped along, whose first and last rows
collapse to poles, so each part is a complete closed quilt with degenerate patches
at its poles.  The surface is C2 everywhere but at the poles; a visible pole's
first ring is an affine image of a regular polygon, which gives the pole a tangent
plane.  Where a part meets its neighbour it continues inside it (a hidden "nose")
and closes to a pole; where a child part attaches, the parent spans the hole.
Neither the noses nor the holes are fitted to anything: past the data the surface
is faired (the smoothing is raised up to `free` times on control points the data
do not reach), so it leaves the body tangent to it and closes quickly.  A finger
also fits the palm's skin for 20 mm below its base (the thumb 10 mm) before its
nose closes, so the two meet tangent over a shared band rather than leaving a
valley.  The union
of the parts is the body; `CompGeom` and `CFDMesh` find the intersection curves.

The cut planes and each part's parameterization are fixed on one reference body,
so the fit of every body is the same linear map of the mesh vertices and joints.
The control points are therefore exactly linear in the anthropometric vector X:

```
P = Pbar + Q_P . X
```

Centre parts are mirror symmetric nets; each left part is the mirror image of its
right part.  The fingers are cut off the palm where each parts from it; the feet
are simplified (convex hulls of their cross sections).  More detailed parts would
change `hb_layout.py` only.

The eyes are fixed points of the head's surface, (u, w) found on each sex's
reference body, so they move with the surface.

### Regenerating

```
python3 human_bezier.py            # writes ../HumanBezierData.h, under a minute
python3 human_bezier.py --check    # also measures the fit (much longer)
```

Regenerate after changing the anthropometric tables or the part layout.  The
generator needs NumPy and SciPy.

## Tests

```
python3 -m pytest tests
```

checks the half mesh and that the generated headers are current: each records the
SHA-1 of the headers it was made from, so a header left stale after its inputs
changed fails `tests/test_generated.py`.

## Attribution

This anthropometric model is based on the model from Human Shapes
(http://humanshape.org/), used with permission of Prof. Matthew P. Reed,
University of Michigan. See the header comments in the archived original files
for the full derivation of the mesh, skeleton, and principal-component fit.
