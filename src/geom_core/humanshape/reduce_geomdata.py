#!/usr/bin/env python3
#
# This file is released under the terms of the NASA Open Source Agreement (NOSA)
# version 1.3 as detailed in the LICENSE file which accompanies this software.
#
# reduce_geomdata.py
#
# Collapse the two-stage HumanGeom anthropometric PCA into a single linear map.
# ----------------------------------------------------------------------------
#
# The original HumanGeom model (archived alongside this script in
# MaleGeomData.h / FemaleGeomData.h) reconstructs a body mesh in two stages:
#
#     score[200] = coeffs[200][6] . X[6]              (X = anthropometric vars)
#     Y          = Ybar + P[N][200] . score[200]      (P = *_half_pcs / *_skel_pcs)
#
# where X is the six-element anthropometric input vector
#     X = { stature, BMI, sit-fraction, age, BMI*age, 1 }.
#
# Because matrix multiplication is associative and X has only six entries, the
# two stages can be folded together offline into a single matrix:
#
#     Y = Ybar + P . (coeffs . X) = Ybar + (P . coeffs) . X = Ybar + Q . X
#
# with  Q[N][6] = P[N][200] . coeffs[200][6].
#
# This script reads the ORIGINAL data files, parses every constant at full
# double precision (the literals carry ~20 significant digits even though the
# trailing 'f' makes the compiler store them as float -- the 'f' exists only so
# MSVC compiles these enormous tables quickly), performs the P . coeffs product
# in double precision with NumPy, and writes the reduced Q tables back out as
# double-precision text with the 'f' suffix restored.  The result is a drop-in
# replacement that is ~33x smaller and ~33x cheaper to evaluate per update, with
# no meaningful change in the computed geometry.
#
# The original 200-component tables are kept (in this directory) but are no
# longer compiled into OpenVSP.  If Prof. Reed's upstream data source is ever
# updated, replace MaleGeomData.h / FemaleGeomData.h here and re-run this script
# to regenerate the compiled MaleGeomDataReduced.h / FemaleGeomDataReduced.h.
#
# Usage:
#     python3 reduce_geomdata.py
#
# Rob McDonald
#

import os
import re
import sys

import numpy as np

# ---------------------------------------------------------------------------
# Model dimensions (must match the #defines in HumanGeom.h).
# ---------------------------------------------------------------------------
NUM_MESH_TRI = 5768
NUM_MESH_VERT = 2943
NUM_SKEL = 25
NUM_SKEL_VERT = 17

NPC = 200   # number of principal components (the dimension being collapsed away)
NVAR = 6    # number of anthropometric input variables

HERE = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.dirname(HERE)   # src/geom_core -- where the compiled headers live

# Matches an integer or floating point literal with an optional 'f' suffix.
_NUM_RE = re.compile(r'[-+]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?f?')


def read_block(text, member):
    """Return the text between '= {' and the terminating '};' for m_<member>."""
    m = re.search(r'\bm_' + re.escape(member) + r'\b[^=]*=\s*\{', text)
    if not m:
        raise RuntimeError('array m_%s not found' % member)
    start = m.end()
    end = text.index('};', start)
    return text[start:end]


def parse_floats(text, member, max_rows, cols):
    """Parse m_<member> as an (n x cols) float64 matrix at full precision.

    Some arrays are declared [max_rows][cols] but the initializer supplies
    fewer rows; C++ zero-fills the remainder.  We honor that: we parse only the
    rows that are actually present and let the emitted header be zero-filled by
    the compiler in exactly the same way, so the compiled result is identical.
    """
    toks = _NUM_RE.findall(read_block(text, member))
    vals = [float(t[:-1]) if t.endswith('f') else float(t) for t in toks]
    if len(vals) % cols != 0:
        raise RuntimeError('m_%s: %d values is not a multiple of %d columns'
                           % (member, len(vals), cols))
    nrows = len(vals) // cols
    if nrows > max_rows:
        raise RuntimeError('m_%s: %d rows exceeds declared %d'
                           % (member, nrows, max_rows))
    if nrows < max_rows:
        print('  note: m_%s initializes %d of %d rows (rest zero-filled)'
              % (member, nrows, max_rows))
    return np.array(vals, dtype=np.float64).reshape(nrows, cols)


def parse_ints(text, member, max_count):
    """Parse m_<member> as a flat list of ints (<= max_count present)."""
    toks = _NUM_RE.findall(read_block(text, member))
    if len(toks) > max_count:
        raise RuntimeError('m_%s: %d values exceeds declared %d'
                           % (member, len(toks), max_count))
    return [int(t) for t in toks]


def fmt_float(x):
    """Full round-trip double precision text, with the 'f' float suffix."""
    return '%.17ef' % x


def emit_matrix_float(qualified_name, dims, mat):
    """Emit 'const REAL_T HumanGeom::<name><dims> = { {..}, .. };'."""
    lines = ['const REAL_T HumanGeom::%s%s = {' % (qualified_name, dims)]
    n = mat.shape[0]
    for i in range(n):
        row = ', '.join(fmt_float(v) for v in mat[i])
        term = '}};' if i == n - 1 else '},'
        lines.append('{' + row + term)
    return '\n'.join(lines) + '\n'


def emit_matrix_int(qualified_name, dims, rows):
    """Emit a 2D int table (rows is a list of tuples/lists)."""
    lines = ['const int HumanGeom::%s%s = {' % (qualified_name, dims)]
    n = len(rows)
    for i, r in enumerate(rows):
        body = ', '.join(str(v) for v in r)
        term = '}};' if i == n - 1 else '},'
        lines.append('{' + body + term)
    return '\n'.join(lines) + '\n'


def emit_vector_int(qualified_name, dims, vals):
    """Emit a 1D int table on a single line."""
    body = ', '.join(str(v) for v in vals)
    return 'const int HumanGeom::%s%s = {%s};\n' % (qualified_name, dims, body)


def doc_comment(gender, shared):
    """Generated-file banner carrying the original attribution and the math."""
    lines = []
    lines.append('/' + '/' * 77)
    lines.append('// GENERATED FILE -- DO NOT EDIT BY HAND.')
    lines.append('//')
    lines.append('// Produced by src/geom_core/humanshape/reduce_geomdata.py from the original')
    lines.append('// full-resolution data in src/geom_core/humanshape/%sGeomData.h.'
                 % ('Male' if gender == 'male' else 'Female'))
    lines.append('// To regenerate (e.g. after an upstream data update), re-run that script.')
    lines.append('//')
    lines.append('// This anthropometric model is based on the model from Human Shapes')
    lines.append('// (http://humanshape.org/), used with permission of Prof. Matthew P. Reed,')
    lines.append('// University of Michigan.  Prof. Reed provided the data files and example')
    lines.append('// Python code to guide this work.  See the archived originals for the full')
    lines.append('// derivation of the mesh, skeleton, and principal component fit.')
    lines.append('//')
    lines.append('// The original model reconstructs geometry from a two-stage PCA:')
    lines.append('//')
    lines.append('//     score[200] = coeffs[200][6] . X[6]')
    lines.append('//     Y          = Ybar + P[N][200] . score[200]')
    lines.append('//')
    lines.append('// where X = { stature, BMI, sit-fraction, age, BMI*age, 1 } is the six-element')
    lines.append('// anthropometric input vector and P is the principal component matrix')
    lines.append('// (*_half_pcs for the mesh, *_skel_pcs for the skeleton).  Since X has only')
    lines.append('// six entries and matrix multiply is associative, the two stages fold into a')
    lines.append('// single precomputed linear map Q = P . coeffs:')
    lines.append('//')
    lines.append('//     Y = Ybar + (P . coeffs) . X = Ybar + Q . X       Q[N][6]')
    lines.append('//')
    lines.append('// The product was formed in double precision from the full-precision text of')
    lines.append('// the archived originals, then written here as double-precision text with the')
    lines.append('// trailing \'f\' restored.  The \'f\' keeps each literal a float (so MSVC compiles')
    lines.append('// these tables quickly) exactly as in the originals; only the storage type is')
    lines.append('// float, while the reduction itself was computed in double precision.')
    lines.append('//')
    lines.append('// Relative to the originals this is about 33x less static data and about 33x')
    lines.append('// fewer multiplies per HumanGeom update, with no meaningful change in geometry.')
    if shared:
        lines.append('//')
        lines.append('// This (male) file also carries the gender-neutral connectivity tables')
        lines.append('// (m_half_tris, m_skel_indx), matching the original layout.')
    lines.append('//')
    lines.append('// Rob McDonald')
    lines.append('/' + '/' * 77)
    return '\n'.join(lines) + '\n'


def build_reduced(gender, src_text, shared_text):
    """Return the full text of the reduced header for one gender."""
    g = gender
    coeffs = parse_floats(src_text, '%s_coeffs' % g, NPC, NVAR)              # [200][6]
    half_pcs = parse_floats(src_text, '%s_half_pcs' % g, NUM_MESH_VERT * 3, NPC)
    skel_pcs = parse_floats(src_text, '%s_skel_pcs' % g, NUM_SKEL_VERT * 3, NPC)
    half_verts = parse_floats(src_text, '%s_half_verts' % g, NUM_MESH_VERT, 3)
    skel_verts = parse_floats(src_text, '%s_skel_verts' % g, NUM_SKEL_VERT, 3)

    # The collapse: Q = P . coeffs, formed in double precision.
    half_Q = half_pcs @ coeffs        # [8829][6]
    skel_Q = skel_pcs @ coeffs        # [51][6]

    guard = 'VSP_%s_GEOM_DATA_REDUCED__INCLUDED_' % g.upper()
    shared = (g == 'male')

    parts = []
    parts.append('#if !defined(%s)' % guard)
    parts.append('#define %s' % guard)
    parts.append('')
    parts.append(doc_comment(g, shared))

    if shared:
        # Gender-neutral connectivity tables live only in the male file, matching
        # the original layout.  They are copied through unchanged.
        skel_indx = parse_ints(shared_text, 'skel_indx', NUM_SKEL)
        tri_flat = parse_ints(shared_text, 'half_tris', NUM_MESH_TRI * 3)
        if len(tri_flat) % 3 != 0:
            raise RuntimeError('m_half_tris: %d values is not a multiple of 3'
                               % len(tri_flat))
        tris = [tri_flat[i * 3:i * 3 + 3] for i in range(len(tri_flat) // 3)]
        parts.append(emit_vector_int('m_skel_indx', '[NUM_SKEL]', skel_indx))
        parts.append('')
        parts.append(emit_matrix_int('m_half_tris', '[NUM_MESH_TRI][3]', tris))
        parts.append('')

    parts.append(emit_matrix_float('m_%s_half_verts' % g, '[NUM_MESH_VERT][3]', half_verts))
    parts.append('')
    parts.append(emit_matrix_float('m_%s_skel_verts' % g, '[NUM_SKEL_VERT][3]', skel_verts))
    parts.append('')
    parts.append('// Q = half_pcs . coeffs -- maps the 6 anthropometric variables directly to')
    parts.append('// mesh vertex-coordinate offsets (see banner above).')
    parts.append(emit_matrix_float('m_%s_half_Q' % g, '[NUM_MESH_VERT*3][6]', half_Q))
    parts.append('')
    parts.append('// Q = skel_pcs . coeffs -- same collapse for the skeleton joints.')
    parts.append(emit_matrix_float('m_%s_skel_Q' % g, '[NUM_SKEL_VERT*3][6]', skel_Q))
    parts.append('')
    parts.append('#endif // %s' % guard)
    parts.append('')
    return '\n'.join(parts)


def main():
    male_src_path = os.path.join(HERE, 'MaleGeomData.h')
    female_src_path = os.path.join(HERE, 'FemaleGeomData.h')

    with open(male_src_path, 'r') as fh:
        male_text = fh.read()
    with open(female_src_path, 'r') as fh:
        female_text = fh.read()

    male_out = build_reduced('male', male_text, male_text)
    female_out = build_reduced('female', female_text, None)

    male_out_path = os.path.join(OUT_DIR, 'MaleGeomDataReduced.h')
    female_out_path = os.path.join(OUT_DIR, 'FemaleGeomDataReduced.h')

    with open(male_out_path, 'w') as fh:
        fh.write(male_out)
    with open(female_out_path, 'w') as fh:
        fh.write(female_out)

    print('wrote %s (%.1f MB)' % (male_out_path, os.path.getsize(male_out_path) / 1e6))
    print('wrote %s (%.1f MB)' % (female_out_path, os.path.getsize(female_out_path) / 1e6))


if __name__ == '__main__':
    sys.exit(main())
