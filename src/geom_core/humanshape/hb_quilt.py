#
# This file is released under the terms of the NASA Open Source Agreement (NOSA)
# version 1.3 as detailed in the LICENSE file which accompanies this software.
#
# hb_quilt.py
#
# Closed tube quilts: a bicubic B-spline, periodic around (u) and clamped along (v),
# whose first and last control rows collapse to single points (poles).  Each part is
# fitted to its region of the mesh by one fixed linear operator applied to
# Yaug = [full mesh vertices; skeleton joints], so its control net is
# P = Pbar + Q_P X exactly, as the mesh is.
#

import numpy as np
from scipy.interpolate import BSpline
from scipy.linalg import null_space, cho_factor, cho_solve
from scipy.sparse import csr_matrix, hstack, vstack
from scipy.spatial import ConvexHull
from scipy.ndimage import maximum_filter

MIR = np.array([1.0, -1.0, 1.0])
DEG = 3


def unit(v):
    return v / np.linalg.norm(v)


def periodic_knots(n):
    """Uniform knots of a periodic cubic with n control points, unrolled: n + 7 values,
    control point j of the unrolled net is j mod n, the domain is [0, 1]."""
    return np.arange(-DEG, n + DEG + 1) / n


def clamped_knots(n):
    return np.concatenate([np.zeros(DEG), np.linspace(0, 1, n - DEG + 1), np.ones(DEG)])


def periodic_design(u, n):
    B = BSpline.design_matrix(np.mod(u, 1.0), periodic_knots(n), DEG).toarray()
    Bp = B[:, :n].copy()
    Bp[:, :DEG] += B[:, n:n + DEG]
    return Bp


def clamped_design(v, n, deriv=0):
    t = clamped_knots(n)
    vv = np.clip(v, 0, 1)
    if deriv == 0:
        return BSpline.design_matrix(np.minimum(vv, 1 - 1e-13), t, DEG).toarray()
    out = np.zeros((len(v), n))
    for i in range(n):
        c = np.zeros(n)
        c[i] = 1
        out[:, i] = BSpline(t, c, DEG).derivative(deriv)(vv)
    return out


def tensor(Bu, Bv):
    return (Bu[:, :, None] * Bv[:, None, :]).reshape(len(Bu), -1)


def second_difference(n, per):
    rows = []
    for i in (range(n) if per else range(1, n - 1)):
        r = np.zeros(n)
        r[(i - 1) % n] += 1
        r[i] -= 2
        r[(i + 1) % n] += 1
        rows.append(r)
    return np.array(rows)


def barycentric(nsub):
    W = []
    for i in range(nsub + 1):
        for j in range(nsub + 1 - i):
            W.append((i / nsub, j / nsub, (nsub - i - j) / nsub))
    return np.array(W)


def mirror_perm(nu):
    """Control point index map of u -> 1 - u for the periodic basis."""
    uu = np.linspace(0.013, 0.987, 97)
    A = periodic_design(uu, nu)
    Bm = periodic_design(1 - uu, nu)
    perm = np.array([np.argmin(np.abs(Bm - A[:, [i]]).sum(0)) for i in range(nu)])
    assert np.abs(Bm[:, perm] - A).max() < 1e-12, 'u basis not mirror symmetric'
    return perm


class Capsule:
    """u = angle about the axis a->b (0 at the 'front' vector), v = normalized length along
    the capsule profile: spherical cap about a (radius ra), cylinder, spherical cap about b
    (radius rb).  An optional warp of v clusters the rows of the net."""

    def __init__(self, a, b, front, ra, rb):
        self.a = np.asarray(a, float)
        self.b = np.asarray(b, float)
        self.L = np.linalg.norm(self.b - self.a)
        self.e3 = (self.b - self.a) / self.L
        f = np.asarray(front, float)
        e1 = f - (f @ self.e3) * self.e3
        self.e1 = e1 / np.linalg.norm(e1)
        self.e2 = np.cross(self.e3, self.e1)
        self.ra, self.rb = ra, rb
        self.warp = None
        self.tot = ra * np.pi / 2 + self.L + rb * np.pi / 2

    def __call__(self, Pts):
        r = Pts - self.a
        s = r @ self.e3
        x, y = r @ self.e1, r @ self.e2
        u = np.mod(np.arctan2(y, x) / (2 * np.pi), 1.0)
        rad = np.hypot(x, y)
        arc = self.ra * np.pi / 2 + s
        m = s < 0
        arc[m] = self.ra * np.arctan2(np.hypot(x[m], y[m]), -s[m])
        sb = s - self.L
        m = sb > 0
        arc[m] = self.ra * np.pi / 2 + self.L + self.rb * np.arctan2(sb[m], rad[m])
        v = arc / self.tot
        if self.warp is not None:
            v = np.interp(v, self.warp[0], self.warp[1])
        return u, v

    def set_warp(self, bumps, n=2001):
        """bumps: (v_centre, half_width, gain); rows (uniform in the warped v) cluster there."""
        x = np.linspace(0, 1, n)
        dens = np.ones(n)
        for c, w, g in bumps:
            dens += (g - 1) * np.exp(-0.5 * ((x - c) / w) ** 2)
        cdf = np.r_[0, np.cumsum(0.5 * (dens[1:] + dens[:-1]) * np.diff(x))]
        self.warp = (x, cdf / cdf[-1])


class PartSpec:
    """One closed part.

    end0, end1: ('pole', 'ring') -- a visible pole whose first ring is an affine image of a
    regular polygon (G1 at the pole); or ('nose', cut, joint, kappa) -- a hidden end past the
    cut ring, inside the neighbour, closing to a pole between the ring centroid and the joint
    (or the mean of mesh vertices ('verts', idx)).  Hidden ends and the holes where child
    parts attach are not fitted: free scales the smoothing of control points whose data
    weight is below reach times the median.
    hull: (s0, s1, ds) mm along the axis -- replace the data there by the convex hull of
    each cross section (fused toes).  envelope: keep only samples near the local
    maximum radius (thumb and fingers).  overlap: {cut name: depth} -- also fit the
    neighbour's samples up to depth mm past that cut, so that the two parts meet tangent
    over a shared band rather than each closing out at the cut."""

    def __init__(self, name, regions, side, cap, nu, nv, end0, end1, lam=1e-4, wt=0.5, nsub=4,
                 envelope=0.0, hull=None, free=300.0, reach=0.3, overlap=None):
        self.name, self.regions, self.side, self.cap, self.nu, self.nv = name, set(regions), side, cap, nu, nv
        self.end0, self.end1, self.lam, self.wt, self.nsub = end0, end1, lam, wt, nsub
        self.envelope = envelope
        self.hull = hull
        self.free, self.reach = free, reach
        self.overlap = overlap or {}


def sample_matrix(Y, T, names, cuts, nsub, regions, overlap=None):
    """Barycentric samples of the triangles of the regions, as rows over the mesh vertices;
    a triangle across a cut is split per sample by the cut plane.  overlap {cut name: depth}
    adds the samples of the region across that cut up to depth past it."""
    W = barycentric(nsub)
    overlap = overlap or {}
    cutn = {c.name: c for c in cuts}
    near = set(regions)
    for cn in overlap:
        near |= {cutn[cn].parent, cutn[cn].child}
    tri_any = np.isin(names[T], list(near)).any(1)
    Tt = T[tri_any]
    Pts = np.einsum('sk,tkc->tsc', W, Y[Tt])
    lab = np.empty(Pts.shape[:2], dtype=object)
    nm = names[Tt]
    same = (nm[:, 0] == nm[:, 1]) & (nm[:, 0] == nm[:, 2])
    lab[same] = nm[same, 0][:, None]
    cutd = {(c.parent, c.child): c for c in cuts}
    for t in np.where(~same)[0]:
        s = set(nm[t])
        done = False
        for (pa, ch), c in cutd.items():
            if s <= {pa, ch}:
                lab[t] = np.where(c.side(Pts[t]) > 0, ch, pa)
                done = True
                break
        if not done:
            vals, cnt = np.unique(nm[t], return_counts=True)
            lab[t] = vals[np.argmax(cnt)]
    keep = np.isin(lab, list(regions))
    for cn, depth in overlap.items():
        c = cutn[cn]
        other = c.child
        sign = 1.0
        if c.child in regions:
            other = c.parent
            sign = -1.0
        h = sign * c.side(Pts.reshape(-1, 3)).reshape(Pts.shape[:2])
        act = c.active(Pts.reshape(-1, 3)).reshape(Pts.shape[:2])
        keep |= (lab == other) & (h > 0.0) & (h < depth) & act
    ti, si = np.where(keep)
    area = 0.5 * np.linalg.norm(np.cross(Y[Tt[:, 1]] - Y[Tt[:, 0]], Y[Tt[:, 2]] - Y[Tt[:, 0]]), axis=1)
    rows = np.repeat(np.arange(len(ti)), 3)
    M = csr_matrix((W[si].ravel(), (rows, Tt[ti].ravel())), shape=(len(ti), len(Y)))
    return M, Pts[ti, si], area[ti] / len(W), Tt[ti]


class Part:
    """The fit operator of one part, built on the reference body."""

    def __init__(self, spec, Yr, Sr, T, names, cuts, ring_fn):
        self.spec = s = spec
        nv_aug = len(Yr) + len(Sr)
        Yaug = np.vstack([Yr, Sr])
        cutd = {c.name: c for c in cuts}
        M, Pd, wd, Ts = sample_matrix(Yr, T, names, cuts, s.nsub, s.regions, s.overlap)
        u, v = s.cap(Pd)
        self.data_area = wd.copy()
        # Samples of triangles that fold over in (u, v) -- walls of the gaps between fingers or
        # toes that face back toward the axis -- are dropped.
        uu, vv = s.cap(Yr[Ts.ravel()])
        uu = uu.reshape(-1, 3)
        vv = vv.reshape(-1, 3)
        uu = uu[:, :1] + ((uu - uu[:, :1] + 0.5) % 1.0 - 0.5)
        sa = (uu[:, 1] - uu[:, 0]) * (vv[:, 2] - vv[:, 0]) - (uu[:, 2] - uu[:, 0]) * (vv[:, 1] - vv[:, 0])
        self.folded = (sa * np.sign(np.median(sa))) <= 0
        wd = wd * (~self.folded)
        if s.envelope:
            cp = s.cap
            r = Pd - cp.a
            sx = r @ cp.e3
            foot = cp.a + np.outer(np.clip(sx, 0, cp.L), cp.e3)
            rad = np.linalg.norm(Pd - foot, axis=1)
            ub = np.floor(u * 6 * s.nu).astype(int)
            vb = np.floor(v * 4 * s.nv).astype(int)
            grid = np.zeros((6 * s.nu, 4 * s.nv + 1))
            np.maximum.at(grid, (ub % (6 * s.nu), np.clip(vb, 0, 4 * s.nv)), rad)
            gm = maximum_filter(grid, size=(5, 3), mode=('wrap', 'nearest'))
            wd = wd * ~(rad < s.envelope * gm[ub % (6 * s.nu), np.clip(vb, 0, 4 * s.nv)])
        M = hstack([M, csr_matrix((M.shape[0], len(Sr)))]).tocsr()
        rowsM = [M]
        U = [u]
        V = [v]
        Wt = [wd]
        if s.hull is not None:
            Hm, hw = self.hull_rows(Yr, T, names, nv_aug, Pd, wd)
            uh, vh = s.cap(Hm @ Yaug)
            rowsM.append(Hm)
            U.append(uh)
            V.append(vh)
            Wt.append(hw)
        self.data_M = M

        def ringinfo(cut):
            a, b, f = ring_fn(cut)
            n = len(a)
            R = csr_matrix((np.r_[1 - f, f], (np.r_[np.arange(n), np.arange(n)], np.r_[a, b])), shape=(n, nv_aug))
            return R, R @ Yaug

        self.nfit = sum(r.shape[0] for r in rowsM)
        # A hidden end is not fitted to anything past its cut: only its pole is placed, between
        # the ring centroid and the joint, and the smoothing carries the surface there from the
        # data, tangent to them.  A hole of a child part is spanned by the smoothing alone.
        for endi, end in ((0, s.end0), (1, s.end1)):
            if end[0] != 'nose':
                continue
            _, cname, jidx, kappa = end[:4]
            R0, Pr0 = ringinfo(cutd[cname])
            n0 = R0.shape[0]
            cR = csr_matrix(np.full((1, n0), 1.0 / n0)) @ R0
            if isinstance(jidx, tuple):
                vi = np.asarray(jidx[1])
                Jrow = csr_matrix((np.full(len(vi), 1.0 / len(vi)), (np.zeros(len(vi), int), vi)), shape=(1, nv_aug))
            else:
                Jrow = csr_matrix(([1.0], ([0], [len(Yr) + jidx])), shape=(1, nv_aug))
            crow = cR + kappa * (Jrow - cR)
            perim = np.linalg.norm(np.roll(Pr0, 1, 0) - Pr0, axis=1).sum()
            rowsM.append(crow.tocsr())
            U.append(np.zeros(1))
            V.append(np.full(1, float(endi)))
            Wt.append(np.full(1, s.wt * perim * perim))
        self.Wmat = vstack(rowsM).tocsr()
        self.u = np.concatenate(U)
        self.v = np.concatenate(V)
        self.w = np.concatenate(Wt)
        self.build()

    def hull_rows(self, Yr, T, names, nv_aug, Pd, wd):
        """Fused-toe envelope: in the axial range, the data become samples on the convex
        hull of each cross section; the hull combinatorics are frozen on the reference body."""
        s = self.spec
        s0, s1, ds = s.hull
        cp = s.cap
        sx = (Pd - cp.a) @ cp.e3
        wd[(sx > s0) & (sx < s1)] = 0.0
        tri = T[np.isin(names[T], list(s.regions)).all(1)]
        hrows = []
        hw = []
        for sk in np.arange(s0 + ds / 2, s1, ds):
            h = (Yr - cp.a) @ cp.e3 - sk
            ht = h[tri]
            m = (ht.min(1) < 0) & (ht.max(1) > 0)
            E = set()
            for t in tri[m]:
                for i in range(3):
                    a, b = t[i], t[(i + 1) % 3]
                    if h[a] * h[b] < 0:
                        E.add((min(a, b), max(a, b)))
            E = np.array(sorted(E))
            if len(E) < 3:
                continue
            f = h[E[:, 0]] / (h[E[:, 0]] - h[E[:, 1]])
            Q = Yr[E[:, 0]] + f[:, None] * (Yr[E[:, 1]] - Yr[E[:, 0]])
            q2 = np.c_[(Q - cp.a) @ cp.e1, (Q - cp.a) @ cp.e2]
            H = ConvexHull(q2).vertices
            for i in range(len(H)):
                ia, ib = H[i], H[(i + 1) % len(H)]
                L = np.linalg.norm(Q[ib] - Q[ia])
                n = max(1, int(np.ceil(L / 2.0)))
                for j in range(n):
                    g = (j + 0.5) / n
                    r = np.zeros(nv_aug)
                    r[E[ia, 0]] += (1 - g) * (1 - f[ia])
                    r[E[ia, 1]] += (1 - g) * f[ia]
                    r[E[ib, 0]] += g * (1 - f[ib])
                    r[E[ib, 1]] += g * f[ib]
                    hrows.append(r)
                    hw.append(L / n * ds)
        return csr_matrix(np.array(hrows)), np.array(hw)

    def build(self):
        s = self.spec
        nu, nv = s.nu, s.nv
        Bm = tensor(periodic_design(self.u, nu), clamped_design(self.v, nv))
        Rg = np.kron(second_difference(nu, True), np.eye(nv))
        Rv = np.kron(np.eye(nu), second_difference(nv, False))
        R = Rg.T @ Rg + Rv.T @ Rv
        BtW = Bm.T * self.w[None, :]
        Mn = BtW @ Bm
        # The smoothing is stronger by up to s.free where the data do not reach a control
        # point (hidden ends, holes), so those are faired rather than fitted.
        cov = BtW[:, :self.nfit].sum(1)
        ref = np.median(cov[cov > 0])
        f = 1.0 + s.free * np.clip(1.0 - cov / (s.reach * ref), 0.0, 1.0)
        ci = np.arange(nu * nv).reshape(nu, nv)
        Rf = Rg.T @ (f[ci.ravel()][:, None] * Rg) + Rv.T @ (f[ci[:, 1:-1].ravel()][:, None] * Rv)
        Mn = Mn + s.lam * np.trace(Mn) / np.trace(R) * Rf
        self.Mn = Mn
        self.K = np.asarray((self.Wmat.T @ BtW.T).T)
        M3 = np.kron(Mn, np.eye(3))
        self.E = self.reduction()
        self.cho = cho_factor(self.E.T @ M3 @ self.E)

    def reduction(self):
        """Columns span the admissible nets (flattened ncp x 3): pole rows collapsed, first ring
        at a visible pole an affine image of a regular polygon, and mirror symmetry of centre
        parts."""
        s = self.spec
        nu, nv = s.nu, s.nv
        ncp = nu * nv

        def idx(i, j):
            return i * nv + j

        C = []

        def eq(i1, j1, i2, j2, sgn=np.ones(3)):
            for k in range(3):
                r = np.zeros(ncp * 3)
                r[idx(i1, j1) * 3 + k] = 1
                r[idx(i2, j2) * 3 + k] -= sgn[k]
                C.append(r)

        for j in (0, nv - 1):
            for i in range(1, nu):
                eq(i, j, 0, j)
        for j, end in ((0, s.end0), (nv - 1, s.end1)):
            if end[0] == 'pole':
                # P1_i = P0 + a cos t_i + b sin t_i: rows orthogonal to span{1, cos, sin}
                jj = 1 if j == 0 else nv - 2
                th = 2 * np.pi * (np.arange(nu) - 1) / nu
                Nm = null_space(np.c_[np.ones(nu), np.cos(th), np.sin(th)].T)
                for k in range(3):
                    for col in Nm.T:
                        r = np.zeros(ncp * 3)
                        for i in range(nu):
                            r[idx(i, jj) * 3 + k] = col[i]
                        C.append(r)
        if s.side == 'C':
            perm = mirror_perm(nu)
            for i in range(nu):
                for j in range(nv):
                    if perm[i] == i:
                        r = np.zeros(ncp * 3)
                        r[idx(i, j) * 3 + 1] = 1
                        C.append(r)
                    elif perm[i] > i:
                        eq(perm[i], j, i, j, MIR)
        return null_space(np.array(C), rcond=1e-10)

    def fit(self, Yaug):
        rhs = self.E.T @ (self.K @ Yaug).ravel()
        z = cho_solve(self.cho, rhs)
        return self.exact((self.E @ z).reshape(self.spec.nu, self.spec.nv, 3))

    def exact(self, P):
        """Poles bitwise single points, centre parts bitwise mirror symmetric."""
        P = P.copy()
        s = self.spec
        P[:, 0] = P[0, 0]
        P[:, -1] = P[0, -1]
        if s.side == 'C':
            P[:, 0, 1] = 0.0
            P[:, -1, 1] = 0.0
            perm = mirror_perm(s.nu)
            for i in range(s.nu):
                if perm[i] == i:
                    P[i, :, 1] = 0.0
                elif perm[i] > i:
                    P[perm[i]] = P[i] * MIR
            P[:, 0] = P[0, 0]
            P[:, -1] = P[0, -1]
        return P

    def operator(self):
        """Dense map Yaug (flattened) -> P (flattened)."""
        G = np.kron(self.K, np.eye(3))
        return self.E @ cho_solve(self.cho, self.E.T @ G)


def evaluate(P, nug, nvg):
    """Grid of surface points: nug around (periodic, no repeat), nvg along (poles included)."""
    nu, nv = P.shape[:2]
    uu = np.linspace(0, 1, nug, endpoint=False)
    vv = np.linspace(0, 1, nvg)
    return np.einsum('ai,bj,ijc->abc', periodic_design(uu, nu), clamped_design(vv, nv), P)


def closed_tris(nug, nvg):
    """Triangles of an evaluation grid as a closed mesh (poles as fans); see grid_verts."""
    T = []

    def vid(i, j):
        if j == 0:
            return 0
        if j == nvg - 1:
            return 1 + (nvg - 2) * nug
        return 1 + (j - 1) * nug + (i % nug)

    for i in range(nug):
        i2 = (i + 1) % nug
        T.append((0, vid(i2, 1), vid(i, 1)))
        for j in range(1, nvg - 2):
            a, b, c, d = vid(i, j), vid(i2, j), vid(i2, j + 1), vid(i, j + 1)
            T.append((a, b, c))
            T.append((a, c, d))
        T.append((vid(i, nvg - 2), vid(i2, nvg - 2), vid(0, nvg - 1)))
    return np.array(T)


def grid_verts(G):
    return np.vstack([G[0, 0][None], G[:, 1:-1].transpose(1, 0, 2).reshape(-1, 3), G[0, -1][None]])
