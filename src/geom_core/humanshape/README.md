# HumanGeom anthropometric data

This directory holds the **original, full-resolution** anthropometric data for
`HumanGeom` and the script that reduces it into the form actually compiled into
OpenVSP.

## Files

| File | Role |
| ---- | ---- |
| `MaleGeomData.h`, `FemaleGeomData.h` | Original 200-principal-component data. **Archived only -- no longer compiled into OpenVSP.** Kept as the source of truth for regeneration. |
| `reduce_geomdata.py` | Converter. Reads the originals, collapses the two-stage PCA, and writes the compiled headers. |
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

## Attribution

This anthropometric model is based on the model from Human Shapes
(http://humanshape.org/), used with permission of Prof. Matthew P. Reed,
University of Michigan. See the header comments in the archived original files
for the full derivation of the mesh, skeleton, and principal-component fit.
