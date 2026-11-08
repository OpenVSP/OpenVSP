#
# This file is released under the terms of the NASA Open Source Agreement (NOSA)
# version 1.3 as detailed in the LICENSE file which accompanies this software.
#
# repair_toes.py
#
# Repairs the UMTRI fine mean mesh and carries the repair into MaleGeomData.h and
# FemaleGeomData.h, the archived originals.  Run reduce_geomdata.py after it.
#
# The defects are on the right foot's second and third toes.  Each is a set of bad vertices.
# Where the triangles are tangled, the faces around the bad vertices are deleted and the hole
# re-triangulated.  The bad vertices then become the biharmonic interpolation of the two rings
# of vertices around them, held fixed: the squared uniform Laplacian is least over the bad
# vertices and the first ring, so the patch carries the curvature of its surroundings (a
# harmonic fill would flatten it, and sink into a toe).  The same weights apply to the mean
# position and to every principal component row, so the repair is the same linear map for
# every body.
#
# The UMTRI model is mirror symmetric, positions and principal components alike, but for the
# faces of the left toes, eyes, mouth and ears, which are tangled where the right side's are
# clean.  So the left half is then made the mirror image of the repaired right half.
#
# OpenVSP's half mesh keeps the vertices process_human.m kept (the fine mesh's original
# indices, in increasing order, found here by matching mean_vector.tsv); its triangles around
# a re-triangulated defect are re-made the same way, which keeps their number.  The tool can
# run again on its own output: every value is recomputed from the UMTRI files.
#
# Needs the UMTRI model files the MATLAB scripts read (see matlab/README.md):
#
#     python3 repair_toes.py --umtri ".../Parametric Person/Michigan" [--write]
#                            [--fine-out DIR [--model-out]]
#
# Without --write it changes nothing.  --fine-out writes the repaired fine meshes as OBJ and a
# preview of the half mesh; --model-out also writes a whole repaired copy of the UMTRI model
# files (mean_vector.tsv, pcs.tsv and the mesh) there, the source for any future reduction.
#

import argparse
import os
import re

import numpy as np
from scipy.optimize import linear_sum_assignment
from scipy.spatial import Delaunay, cKDTree

HERE = os.path.dirname(os.path.abspath(__file__))
NPC = 200
NUM_ANTHRO = {'male': 49, 'female': 51}
NUM_LANDMARK = 93
MODEL = {'male': 'Male_800Subj_200PC_4Var_2017_06_26', 'female': 'Female_800Subj_200PC_4Var_2017_06_26'}
OBJ = {'male': 'mean_mesh.obj', 'female': 'mean_mesh_tris.obj'}

# The defects, as fine mesh vertices (OpenVSP's half mesh index in brackets where it keeps
# the vertex).  Second toe: 13592 [2551] stands 3.7 mm out of the toe toward the third, with
# its neighbour 13559, and the faces around 13499, 13484, 14227 and 14180 near its tip are
# tangled.  Third toe: the faces around 13669 [2592], 13731 [2611], 13655 and 13726 are
# tangled, and so are those around 13668, and at its tip around 13680, 13628 and 13677 (which
# lies between the first two and is in a folded face).
SITES = [{'bad': [13592, 13559], 'retriangulate': True},
         {'bad': [13499], 'retriangulate': True},
         {'bad': [13484], 'retriangulate': True},
         {'bad': [14227], 'retriangulate': True},
         {'bad': [14180], 'retriangulate': True},
         {'bad': [13669, 13731, 13655, 13726], 'retriangulate': True},
         {'bad': [13668], 'retriangulate': True},
         {'bad': [13680, 13628, 13677], 'retriangulate': True}]

_NUM = re.compile(r'[-+]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?')


def read_obj_faces(path):
    F = []
    for line in open(path):
        t = line.split()
        if t and t[0] == 'f':
            F.append([int(x.split('/')[0]) - 1 for x in t[1:4]])
    return np.array(F)


def read_rows(path, rows):
    """The given rows of a whitespace separated table."""
    want = set(rows)
    out = {}
    with open(path) as f:
        for i, line in enumerate(f):
            if i in want:
                out[i] = np.array([float(x) for x in line.split()])
    return np.array([out[r] for r in rows])


def boundary_loop(F, bad):
    """The faces touching the bad vertices, and the loop of edges around them."""
    inc = np.isin(F, list(bad)).any(1)
    E = set()
    for f in F[inc]:
        for k in range(3):
            E.add((int(f[k]), int(f[(k + 1) % 3])))
    nxt = dict((a, b) for a, b in E if (b, a) not in E)
    start = next(iter(nxt))
    loop = [start]
    while nxt[loop[-1]] != start:
        loop.append(nxt[loop[-1]])
    assert len(loop) == len(nxt), 'the faces around a defect do not bound a disc'
    return inc, loop


def retriangulate(X, loop, inner):
    """Triangles over the loop and inner vertices, in the loop's orientation: the constrained
    Delaunay of their projection on the loop's best plane, kept inside the loop."""
    pts = np.array(loop + inner)
    P = X[pts]
    c = P.mean(0)
    _, _, Vt = np.linalg.svd(P - c)
    uv = (P - c) @ Vt[:2].T
    tri = Delaunay(uv).simplices
    poly = uv[:len(loop)]

    def inside(q):
        x, y = q
        n = len(poly)
        res = False
        for i in range(n):
            a, b = poly[i], poly[(i + 1) % n]
            if (a[1] > y) != (b[1] > y) and x < a[0] + (y - a[1]) * (b[0] - a[0]) / (b[1] - a[1]):
                res = not res
        return res
    tri = tri[[inside(uv[t].mean(0)) for t in tri]]
    # Orient as the loop runs.
    area = 0.5 * np.sum(poly[:, 0] * np.roll(poly[:, 1], -1) - np.roll(poly[:, 0], -1) * poly[:, 1])
    a = uv[tri]
    s = (a[:, 1, 0] - a[:, 0, 0]) * (a[:, 2, 1] - a[:, 0, 1]) - (a[:, 2, 0] - a[:, 0, 0]) * (a[:, 1, 1] - a[:, 0, 1])
    flip = (s * area) < 0
    tri[flip] = tri[flip][:, ::-1]
    nb = len(loop) + len(inner)
    assert len(tri) == 2 * len(inner) + len(loop) - 2, 'patch is not a disc triangulation'
    assert nb == len(np.unique(tri)), 'patch misses a vertex'
    return pts[tri]


def harmonic_weights(F, bad, loop):
    """A with X[bad] = A @ X[loop]: the uniform Laplacian of the faces around the bad
    vertices, solved with the loop fixed."""
    idx = {v: i for i, v in enumerate(bad)}
    lidx = {v: i for i, v in enumerate(loop)}
    nb = {v: set() for v in bad}
    for f in F:
        for k in range(3):
            a, b = int(f[k]), int(f[(k + 1) % 3])
            if a in nb:
                nb[a].add(b)
            if b in nb:
                nb[b].add(a)
    L = np.zeros((len(bad), len(bad)))
    R = np.zeros((len(bad), len(loop)))
    for v in bad:
        i = idx[v]
        L[i, i] = len(nb[v])
        for u in nb[v]:
            if u in idx:
                L[i, idx[u]] -= 1.0
            else:
                R[i, lidx[u]] += 1.0
    return np.linalg.solve(L, R)


def biharmonic_weights(F, bad):
    """A and the fixed vertices K with X[bad] = A @ X[K]: least squares of the uniform Laplacian
    over the bad vertices and their first ring, with the first and second rings fixed."""
    nb = {}
    for f in F:
        for k in range(3):
            nb.setdefault(int(f[k]), set()).update((int(f[(k + 1) % 3]), int(f[(k + 2) % 3])))
    bad = list(bad)
    ring1 = sorted(set().union(*(nb[v] for v in bad)) - set(bad))
    ring2 = sorted(set().union(*(nb[v] for v in ring1)) - set(bad) - set(ring1))
    K = ring1 + ring2
    col = {v: i for i, v in enumerate(bad + K)}
    rows = []
    for v in bad + ring1:
        r = np.zeros(len(col))
        r[col[v]] = 1.0
        for u in nb[v]:
            r[col[u]] -= 1.0 / len(nb[v])
        rows.append(r)
    M = np.array(rows)
    nbad = len(bad)
    A = -np.linalg.lstsq(M[:, :nbad], M[:, nbad:], rcond=None)[0]
    return A, K


def repair_fine(Ff, X):
    """The repaired fine faces, and per site (bad, loop, A)."""
    Ff = Ff.copy()
    out = []
    for site in SITES:
        bad = site['bad']
        inc, loop = boundary_loop(Ff, bad)
        if site['retriangulate']:
            # Place the bad vertices on the old faces, triangulate there, then again on the new faces.
            A = harmonic_weights(Ff, bad, loop)
            Xs = X.copy()
            Xs[bad] = A @ X[loop]
            new = retriangulate(Xs, loop, bad)
            Ff = np.vstack([Ff[~inc], new])
        A, K = biharmonic_weights(Ff, bad)
        out.append((bad, K, A))
    return Ff, out


def check_half_mesh(T, V):
    """Every edge is shared by two faces with opposite senses, except on the centre plane, and no
    face repeats."""
    seen = {}
    for f in T:
        for k in range(3):
            e = (int(f[k]), int(f[(k + 1) % 3]))
            assert e not in seen, 'edge %s used twice in one sense' % (e,)
            seen[e] = True
    for (a, b) in seen:
        if (b, a) not in seen:
            assert V[a, 1] == 0.0 and V[b, 1] == 0.0, 'open edge %d-%d off the centre plane' % (a, b)
    assert len({tuple(sorted(f)) for f in T.tolist()}) == len(T), 'a face repeats'


def mirror_partners(Vs):
    """The right or centre vertex whose mirror image each vertex is (itself for those), one map
    for all the meshes Vs (they share their faces).  The UMTRI model is mirror symmetric but
    for a few spots on the head where the two sides' vertices differ; there the left vertices
    take the unmatched right vertices nearest their mirror images, summed over the meshes."""
    flip = np.array([1.0, -1.0, 1.0])
    V = Vs[0]
    left = V[:, 1] < 0.0
    ok = left.copy()
    m0 = None
    for X in Vs:
        d, m = cKDTree(X).query(X * flip)
        if m0 is None:
            m0 = m
        ok &= (d < 1e-4) & (m == m0) & (X[m, 1] > 0.0)
    src = np.arange(len(V))
    src[ok] = m0[ok]
    rest_l = np.nonzero(left & ~ok)[0]
    rest_r = np.setdiff1d(np.nonzero(V[:, 1] > 0.0)[0], src[ok])
    assert len(rest_l) == len(rest_r), 'the two sides have different vertex counts'
    if len(rest_l):
        cost = sum(np.linalg.norm((X[rest_l] * flip)[:, None, :] - X[rest_r][None, :, :], axis=2) for X in Vs)
        i, j = linear_sum_assignment(cost)
        src[rest_l[i]] = rest_r[j]
    assert len(np.unique(src[left])) == left.sum()
    return src


def symmetric_faces(F, src, V):
    """The right half's faces and their mirror images on the left vertices."""
    right_or_centre = V[:, 1] >= 0.0
    Fr = F[right_or_centre[F].all(1)]
    inv = np.arange(len(V))
    left = np.nonzero(V[:, 1] < 0.0)[0]
    inv[src[left]] = left
    return np.vstack([Fr, inv[Fr][:, ::-1]])


def write_model(umtri, out, s, faces, fix, src):
    """The repaired UMTRI model files of one sex: the fine vertices' rows of mean_vector.tsv
    and pcs.tsv replaced (fix: vertex -> (mean (3,), pc rows (3, NPC))), the left half the
    mirror image of the right, and the faces."""
    base = NUM_ANTHRO[s] + 3 * NUM_LANDMARK
    src_dir = os.path.join(umtri, MODEL[s])
    dst_dir = os.path.join(out, MODEL[s])
    os.makedirs(dst_dir, exist_ok=True)
    flip = np.array([1.0, -1.0, 1.0])
    for name, col in (('mean_vector.tsv', None), ('pcs.tsv', NPC)):
        lines = open(os.path.join(src_dir, name)).read().split('\n')
        nv = (len([l for l in lines if l.strip()]) - base) // 3
        vals = {}
        for v in range(nv):
            for c in range(3):
                vals[(v, c)] = lines[base + 3 * v + c]

        def fmt(x):
            if col is None:
                return '%.17g' % x[0]
            return '\t'.join('%.17g' % y for y in x)

        def row(v, c):
            if v in fix:
                return np.atleast_1d(fix[v][0 if col is None else 1][c])
            return np.array(vals[(v, c)].split(), float)
        for v in range(nv):
            r = src[v]
            if r != v or v in fix:
                for c in range(3):
                    x = row(r, c)
                    if r != v:
                        x = x * flip[c]
                    lines[base + 3 * v + c] = fmt(x)
        with open(os.path.join(dst_dir, name), 'w', newline='\n') as f:
            f.write('\n'.join(lines))
    with open(os.path.join(dst_dir, OBJ[s]), 'w', newline='\n') as f:
        f.write('# UMTRI mean mesh, repaired by OpenVSP humanshape/repair_toes.py\n')
        for t in faces:
            f.write('f %d %d %d\n' % tuple(t + 1))


def match_half(Vh, Vf):
    """The fine vertex of each half mesh vertex, in increasing order.  A vertex an earlier run
    repaired no longer matches its fine vertex; it is the nearest site vertex that fits the
    order."""
    d, idx = cKDTree(Vf).query(Vh)
    ok = d < 1e-3
    site = sorted(v for st in SITES for v in st['bad'])
    for r in np.nonzero(~ok)[0]:
        lo = idx[r - 1] if r > 0 else -1
        hi = next((idx[k] for k in range(r + 1, len(idx)) if ok[k]), len(Vf))
        cand = [v for v in site if lo < v < hi]
        assert cand, 'half vertex %d matches no fine vertex' % r
        # A repair moves a vertex a few mm at most: the nearest candidate.
        idx[r] = min(cand, key=lambda v: np.linalg.norm(Vf[v] - Vh[r]))
    assert (np.diff(idx) > 0).all(), 'half mesh is not a subset of the fine mesh in order'
    return idx


def block(text, member):
    m = re.search(r'\bm_' + re.escape(member) + r'\b[^=]*=\s*\{\n?', text)
    start = m.end()
    end = text.index('};', start)
    return start, end


def replace_rows(text, member, rows):
    """Replace rows (index -> list of numbers or ints) of a one-row-per-line table."""
    start, end = block(text, member)
    lines = text[start:end].split('\n')
    for i, vals in rows.items():
        line = lines[i]
        lead = line[:line.index('{')]
        tail = line[line.rindex('}') + 1:]
        if isinstance(vals[0], (int, np.integer)):
            body = ', '.join('%d' % v for v in vals)
        else:
            body = ', '.join('%.19ef' % v for v in vals)
        lines[i] = lead + '{' + body + '}' + tail
    return text[:start] + '\n'.join(lines) + text[end:]


def table(text, member, cols, kind=float):
    start, end = block(text, member)
    vals = [kind(t) for t in _NUM.findall(text[start:end])]
    return np.array(vals).reshape(-1, cols)


def main():
    ap = argparse.ArgumentParser(description='Repair the toes of the UMTRI fine mesh.')
    ap.add_argument('--umtri', required=True, help='folder holding the Male_ and Female_ model folders')
    ap.add_argument('--fine-out', help='write the repaired fine meshes here as OBJ, and a preview')
    ap.add_argument('--write', action='store_true', help='write MaleGeomData.h and FemaleGeomData.h')
    ap.add_argument('--model-out', action='store_true',
                    help='with --fine-out, also write the whole repaired UMTRI model (mean_vector.tsv, pcs.tsv, mesh)')
    args = ap.parse_args()

    paths = {s: os.path.join(HERE, '%sGeomData.h' % s.capitalize()) for s in ('male', 'female')}
    texts = {s: open(paths[s]).read() for s in paths}
    Ff = read_obj_faces(os.path.join(args.umtri, MODEL['male'], OBJ['male']))
    assert np.array_equal(Ff, read_obj_faces(os.path.join(args.umtri, MODEL['female'], OBJ['female'])))
    Th = table(texts['male'], 'half_tris', 3, int)

    fine_orig = {}
    fine_mean = {}
    red2fine = None
    for s in ('male', 'female'):
        C = np.loadtxt(os.path.join(args.umtri, MODEL[s], 'mean_vector.tsv'))
        fine_mean[s] = C[NUM_ANTHRO[s] + 3 * NUM_LANDMARK:].reshape(-1, 3)
        fine_orig[s] = fine_mean[s].copy()
        Vh = table(texts[s], s + '_half_verts', 3)
        idx = match_half(Vh, fine_mean[s])
        if red2fine is None:
            red2fine = idx
        assert np.array_equal(red2fine, idx)
    fine2red = {int(f): r for r, f in enumerate(red2fine)}

    Ff_new, sites = repair_fine(Ff, fine_mean['male'])
    check_half_mesh(Ff_new, fine_mean['male'] + np.array([0.0, 1e9, 0.0]))
    Th_cur = Th.copy()
    fixes = {'male': {}, 'female': {}}
    for (bad, loop, A), spec in zip(sites, SITES):
        print('site %s: from %d fixed vertices' % (bad, len(loop)))
        for s in ('male', 'female'):
            Xm = fine_mean[s]
            base = NUM_ANTHRO[s] + 3 * NUM_LANDMARK
            rows = [base + 3 * v + c for v in loop for c in range(3)]
            P = read_rows(os.path.join(args.umtri, MODEL[s], 'pcs.tsv'), rows).reshape(len(loop), 3, NPC)
            Xb = A @ Xm[loop]
            Pb = np.einsum('il,lcp->icp', A, P)
            for i, v in enumerate(bad):
                print('  %s fine %d moves %.2f mm' % (s, v, np.linalg.norm(Xb[i] - Xm[v])))
                Xm[v] = Xb[i]
                fixes[s][v] = (Xb[i], Pb[i])
                r = fine2red.get(v)
                if r is not None:
                    texts[s] = replace_rows(texts[s], s + '_half_verts', {r: list(Xb[i])})
                    texts[s] = replace_rows(texts[s], s + '_half_pcs', {3 * r + c: list(Pb[i, c]) for c in range(3)})
        # The half mesh's faces around the defect, re-made over the same vertices.
        rbad = [fine2red[v] for v in bad if v in fine2red]
        if spec['retriangulate'] and rbad:
            inc, rloop = boundary_loop(Th_cur, rbad)
            Xr = fine_mean['male'][red2fine]
            new = retriangulate(Xr, rloop, rbad)
            old = np.nonzero(inc)[0]
            assert len(new) == len(old)
            for i, t in zip(old, new):
                Th_cur[i] = t
            print('  half mesh: %d faces re-made around %s' % (len(old), rbad))
    check_half_mesh(Th_cur, fine_mean['male'][red2fine])
    # The fine mesh made mirror symmetric: the left half is the mirror image of the repaired right.
    src = mirror_partners([fine_orig['male'], fine_orig['female']])
    Ff_sym = symmetric_faces(Ff_new, src, fine_orig['male'])
    check_half_mesh(Ff_sym, fine_mean['male'] + np.array([0.0, 1e9, 0.0]))
    flip = np.array([1.0, -1.0, 1.0])
    for s in ('male', 'female'):
        left = fine_mean[s][:, 1] < 0.0
        moved = np.linalg.norm(fine_mean[s][src] * flip - fine_mean[s], axis=1)[left]
        print('%s: left half mirrored from the right; %d vertices move, up to %.2f mm'
              % (s, (moved > 1e-6).sum(), moved.max()))
        fine_mean[s][left] = fine_mean[s][src[left]] * flip
    Ff_new = Ff_sym
    new_tris = {int(i): [int(x) for x in Th_cur[i]] for i in np.nonzero((Th_cur != Th).any(1))[0]}
    if new_tris:
        texts['male'] = replace_rows(texts['male'], 'half_tris', new_tris)
    if args.write:
        for s in paths:
            with open(paths[s], 'w', newline='\n') as f:
                f.write(texts[s])
            print('wrote %s' % paths[s])
    if args.fine_out:
        os.makedirs(args.fine_out, exist_ok=True)
        Th_new = Th_cur
        np.savez(os.path.join(args.fine_out, 'preview.npz'), fine_F=Ff, fine_F_new=Ff_new, fine_V=fine_orig['male'],
                 fine_V_new=fine_mean['male'], half_F=Th, half_F_new=Th_new, half_V=fine_orig['male'][red2fine],
                 half_V_new=fine_mean['male'][red2fine], red2fine=red2fine)
        for s in ('male', 'female'):
            fn = os.path.join(args.fine_out, '%s_mean_mesh_repaired.obj' % s)
            with open(fn, 'w') as f:
                for p in fine_mean[s]:
                    f.write('v %.6f %.6f %.6f\n' % tuple(p))
                for t in Ff_new:
                    f.write('f %d %d %d\n' % tuple(t + 1))
            print('wrote %s' % fn)
            if args.model_out:
                write_model(args.umtri, args.fine_out, s, Ff_new, fixes[s], src)
                print('wrote the repaired %s model files in %s' % (s, os.path.join(args.fine_out, MODEL[s])))


if __name__ == '__main__':
    main()
