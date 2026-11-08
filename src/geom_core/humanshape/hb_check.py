#
# This file is released under the terms of the NASA Open Source Agreement (NOSA)
# version 1.3 as detailed in the LICENSE file which accompanies this software.
#
# hb_check.py
#
# Measurements of the generated surface (human_bezier.py --check): linearity in X and fit
# to the mesh over the input box.
# Distances are in mm, from the mesh samples of a part to that part's surface.
#

import numpy as np
from scipy.spatial import cKDTree

from hb_data import XREF, xvec, J
from hb_quilt import evaluate, grid_verts, closed_tris, periodic_design, clamped_design

# Stature and BMI 5th and 95th percentiles of HumanGeom's tables.
PCT = {'male': dict(sta=(1634, 1881), bmi=(20.7, 39.8)), 'female': dict(sta=(1498, 1735), bmi=(19.6, 43.3))}


def corners(sex):
    s = PCT[sex]
    out = []
    for a in s['sta']:
        for b in s['bmi']:
            for c in (0.49, 0.55):
                for d in (18, 80):
                    out.append(((a, b, c, d), xvec(a, b, c, d)))
    return out


def point_tri_dist(Pt, A, Bv, C):
    ab = Bv - A
    ac = C - A
    ap = Pt - A
    d1 = np.einsum('ij,ij->i', ab, ap)
    d2 = np.einsum('ij,ij->i', ac, ap)
    bp = Pt - Bv
    d3 = np.einsum('ij,ij->i', ab, bp)
    d4 = np.einsum('ij,ij->i', ac, bp)
    cp = Pt - C
    d5 = np.einsum('ij,ij->i', ab, cp)
    d6 = np.einsum('ij,ij->i', ac, cp)
    va = d3 * d6 - d5 * d4
    vb = d5 * d2 - d1 * d6
    vc = d1 * d4 - d3 * d2
    den = va + vb + vc
    den[den == 0] = 1e-30
    Q = A + ab * (vb / den)[:, None] + ac * (vc / den)[:, None]
    m = (d1 <= 0) & (d2 <= 0)
    Q[m] = A[m]
    m = (d3 >= 0) & (d4 <= d3)
    Q[m] = Bv[m]
    m = (d6 >= 0) & (d5 <= d6)
    Q[m] = C[m]

    def safe(x):
        return np.where(x == 0, 1e-30, x)

    m = (vc <= 0) & (d1 >= 0) & (d3 <= 0)
    Q[m] = (A + ab * (d1 / safe(d1 - d3))[:, None])[m]
    m = (vb <= 0) & (d2 >= 0) & (d6 <= 0)
    Q[m] = (A + ac * (d2 / safe(d2 - d6))[:, None])[m]
    m = (va <= 0) & ((d4 - d3) >= 0) & ((d5 - d6) >= 0)
    t = (d4 - d3) / safe((d4 - d3) + (d5 - d6))
    Q[m] = (Bv + (C - Bv) * t[:, None])[m]
    return np.linalg.norm(Pt - Q, axis=1)


def dist_to_tris(Pt, V, T, k=24):
    tree = cKDTree(V[T].mean(1))
    _, idx = tree.query(Pt, k=k)
    best = np.full(len(Pt), np.inf)
    for j in range(k):
        t = T[idx[:, j]]
        best = np.minimum(best, point_tri_dist(Pt, V[t[:, 0]], V[t[:, 1]], V[t[:, 2]]))
    return best


def net_error(part, P, Yaug):
    """Area weighted RMS, p99 and max of the part's mesh samples to its surface."""
    P = np.asarray(P).reshape(part.spec.nu, part.spec.nv, 3)
    G = evaluate(P, 6 * part.spec.nu, 5 * part.spec.nv)
    d = dist_to_tris(part.data_M @ Yaug, grid_verts(G), closed_tris(*G.shape[:2]))
    w = part.data_area
    return np.sqrt((w * d * d).sum() / w.sum()), np.percentile(d, 99), d.max()


def report(body, parts, data, log):
    log('')
    log('== checks ==')
    for sex, lin in data.items():
        Xa = xvec(1700, 30, 0.5, 40)
        Xb = xvec(1650, 22, 0.53, 40)
        dl = 0.0
        for ip, p in enumerate(parts):
            Pb, Qp = lin[ip]
            for X in (Xa, Xb):
                Ya = np.vstack([body.mesh(sex, X), body.skel(sex, X)])
                dl = max(dl, np.abs(Pb + (Qp @ X).reshape(-1, 3) - p.fit(Ya).reshape(-1, 3)).max())
        log('%s: Pbar + Q_P X against the fit of the mesh at X: max %.1e mm' % (sex, dl))

    log('')
    log('fit error, mesh samples to own surface, mm: rms / p99 / max (worst over the 5-95 % corners)')
    for sex, lin in data.items():
        for ip, p in enumerate(parts):
            Pb, Qp = lin[ip]
            ref = net_error(p, Pb + (Qp @ XREF[sex]).reshape(-1, 3), np.vstack([body.mesh(sex, XREF[sex]),
                                                                               body.skel(sex, XREF[sex])]))
            worst = np.zeros(3)
            for _, X in corners(sex):
                Ya = np.vstack([body.mesh(sex, X), body.skel(sex, X)])
                worst = np.maximum(worst, net_error(p, Pb + (Qp @ X).reshape(-1, 3), Ya))
            log('  %-6s %-8s 50th %.2f / %.1f / %.1f   corners %.2f / %.1f / %.1f'
                % (sex, p.spec.name, ref[0], ref[1], ref[2], worst[0], worst[1], worst[2]))


def eye_report(body, parts, data, eyes, order, log):
    """Distance from the eye at its fixed point of the head surface to the skeleton's eye, over
    the corners of the input box."""
    ip = eyes['male'][0]
    p = parts[ip]
    nu, nv = p.spec.nu, p.spec.nv
    for sex, lin in data.items():
        Pb, Qp = lin[ip]
        uw = eyes[sex][1]
        worst = 0.0
        for _, X in corners(sex):
            P = (Pb + (Qp @ X).reshape(-1, 3))[order(p)].reshape(nu, nv, 3)
            S = body.skel(sex, X)
            for k, (u, w) in (('REYE', uw[0:2]), ('LEYE', uw[2:4])):
                e = np.einsum('i,j,ijc->c', periodic_design(np.array([w / nu]), nu)[0],
                              clamped_design(np.array([u / (nv - 3)]), nv)[0], P)
                worst = max(worst, np.linalg.norm(e - S[J[k]]))
        log('%s: eyes at fixed surface points, largest distance to the skeleton eye %.2f mm' % (sex, worst))
