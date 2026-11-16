#!/usr/bin/env python3
#
# This file is released under the terms of the NASA Open Source Agreement (NOSA)
# version 1.3 as detailed in the LICENSE file which accompanies this software.
#
# human_tets.py
#
# Generate ../HumanTetData.h: the tetrahedral mesh that fills the Human, which HumanGeom poses by
# volumetric as-rigid-as-possible deformation (ARAP) or by Fast Automatic Skinning Transformations
# (FAST; Jacobson, Baran, Kavan, Popovic and Sorkine, SIGGRAPH 2012), both as libigl implements them.
#
# One tet mesh serves every body of both sexes; its connectivity is fixed and its vertices are linear
# in the anthropometric vector X, as the skin is.  It is built on the androgynous body (the mean of the
# 50th percentile male and female, which share the skin's connectivity) from the skin vertices, points
# along the bones, stations off them, and a face-centred cubic lattice inside the skin, away from the
# skin and those points.  The points are mirror symmetric; the Delaunay tetrahedralization (Qhull,
# through scipy) of the half on the right is mirrored, so the mesh is too.  The tets whose centroids
# and edge midpoints are inside the skin are kept, and then those of quality (6 sqrt(2) V / l_rms^3, 1
# for a regular tet) at least MIN_QUALITY.  A skin vertex is its mesh vertex; a bone point is a fixed
# fraction of its segment between two joints; a station or lattice point moves with the
# cotangent-harmonic extension of the skin's and bone points' displacement from the androgynous body,
# a fixed linear map, so each sex's are Pbar + Q X.  A skin vertex that no tet uses rides on the
# nearest point of the tet mesh's boundary.
#
# On that mesh, once: the tet vertices held to each handle, the bone points, the stations (points off
# each bone that hold the roll about it) and every tet vertex of the head, which moves as one body;
# bounded biharmonic weights (Jacobson, Baran, Popovic and Sorkine 2011) on the handles, held there;
# and FAST's additional weights (its section 3.3), cubic B-spline bumps in weight space that vanish at
# every handle.  ARAP holds the same tet vertices.  A long bone that twists (upper arm, forearm, thigh,
# neck, spine) carries two handles, each adding a fraction of the twist to the bone's transform, so
# the twist spreads along the bone.
#
#     python3 human_tets.py [--out FILE] [--lattice MM]
#
# Needs NumPy, SciPy and libigl's Python bindings.  Takes under a minute.
#

import argparse
import os
import sys
import time

import numpy as np
import igl
import scipy.sparse as sp
from scipy.sparse.csgraph import connected_components
from scipy.spatial import Delaunay, cKDTree

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from hb_data import Body, J, B, XREF, input_digests

# Lattice cube edge (mm; neighbouring lattice points are LATTICE / sqrt(2) apart), its jitter (fraction
# of the edge, so Qhull splits each octahedron the same way), and the distance (mm) lattice points keep
# from the skin's triangles and from the bone points.
LATTICE = 40.0
JITTER = 0.001
SEED = 0
SKIN_CLEAR = 0.394 * LATTICE
# Bone points lie between the fractions HANDLE_T of their segments, at most BONE_STEP (mm) apart, and
# at least SKIN_GAP (mm) inside the skin.
HANDLE_T = (0.2, 0.8)
BONE_STEP = LATTICE / 3.0
SKIN_GAP = 2.0
# Tets with an edge midpoint more than MID_GAP (mm) outside the skin bridge a gap between limbs.
MID_GAP = 0.8
MIN_QUALITY = 0.1
# Bounded biharmonic weights' active set iterations, and the smallest weight kept.
BBW_MAX_ITER = 200
MIN_WEIGHT = 1e-6
# FAST's additional weights (in mirror pairs), the weight-space samples their seeds are spread over,
# and the smallest kept; FAST's rotation groups (in mirror pairs).
NUM_EXTRA = 30
EXTRA_SAMPLES = 4000
EXTRA_ROUNDS = 30
MIN_EXTRA = 1e-6
FAST_GROUPS = 100
# A station is a point off a handle's bone, at STATION_R of the way from the bone to the skin, that
# ARAP holds with the bone points so the roll about the bone is held.
STATION_R = 0.5

# Partial rotations, in HumanTetRig's HumanTetPartial order: a handle may add a fraction of one to its
# bone's transform, to spread a twist or bend along a bone.
PARTIALS = ['TORSO', 'NECK', 'LSHOULDER_IE', 'LPRONATION', 'LHIP_IE', 'RSHOULDER_IE', 'RPRONATION', 'RHIP_IE']

# Handles: (name, bone whose transform it takes, segments (joint a, joint c, t0, t1) whose bone points
# between fractions t0 and t1 it holds, the segments that carry a station at the middle of that span,
# the partial rotation it adds, the fraction of it).  A long bone that twists is split in two at its
# middle, each half taking the fraction of the twist at its centre; a fraction is relative to its
# bone's transform, which carries all of the rotation or none of it.  The torso's fractions are set by
# torso_fractions.
_T0, _TM, _T1 = HANDLE_T[0], 0.5, HANDLE_T[1]
HANDLES = [('UPSPINE_A', 'UPSPINE', [('ORIGIN', 'BACK', _T0, _TM)], [0], 'TORSO', None),
           ('UPSPINE_B', 'UPSPINE', [('ORIGIN', 'BACK', _TM, _T1)], [0], 'TORSO', None),
           ('LOWSPINE', 'LOWSPINE', [('BACK', 'WAIST', _T0, _T1)], [0], 'TORSO', None),
           ('PELVIS', 'LOWSPINE', [('WAIST', 'LHIP', _T0, _T1), ('WAIST', 'RHIP', _T0, _T1)], [0, 1], None, 0.0),
           ('NECK_A', 'NECK', [('ORIGIN', 'HEAD', _T0, _TM)], [0], 'NECK', None),
           ('NECK_B', 'NECK', [('ORIGIN', 'HEAD', _TM, _T1)], [0], 'NECK', None),
           ('SKULL', 'NECK', [('HEAD', 'DES_EYE', _T0, _T1)], [0], None, 0.0)]
for _s in 'LR':
    HANDLES += [(_s + 'CLAV', _s + 'CLAV', [('ORIGIN', _s + 'SHOULDER', _T0, _T1)], [0], None, 0.0),
                (_s + 'BICEP_A', _s + 'BICEP', [(_s + 'SHOULDER', _s + 'ELBOW', _T0, _TM)], [0], _s + 'SHOULDER_IE', -0.65),
                (_s + 'BICEP_B', _s + 'BICEP', [(_s + 'SHOULDER', _s + 'ELBOW', _TM, _T1)], [0], _s + 'SHOULDER_IE', -0.35),
                (_s + 'FOREARM_A', _s + 'FOREARM', [(_s + 'ELBOW', _s + 'WRIST', _T0, _TM)], [0], _s + 'PRONATION', 0.35),
                (_s + 'FOREARM_B', _s + 'FOREARM', [(_s + 'ELBOW', _s + 'WRIST', _TM, _T1),
                                                    (_s + 'WRIST', _s + 'WRISTAX', _T0, _T1)], [0], _s + 'PRONATION', 0.65),
                (_s + 'HAND', _s + 'HAND', [(_s + 'WRIST', _s + 'FINGER', _T0, _T1)], [0], None, 0.0),
                (_s + 'THIGH_A', _s + 'THIGH', [(_s + 'HIP', _s + 'KNEE', _T0, _TM)], [0], _s + 'HIP_IE', -0.65),
                (_s + 'THIGH_B', _s + 'THIGH', [(_s + 'HIP', _s + 'KNEE', _TM, _T1)], [0], _s + 'HIP_IE', -0.35),
                (_s + 'SHIN', _s + 'SHIN', [(_s + 'KNEE', _s + 'ANKLE', _T0, _T1)], [0], None, 0.0),
                (_s + 'FOOT', _s + 'FOOT', [(_s + 'ANKLE', _s + 'TOE', _T0, _T1)], [0], None, 0.0)]
HNAME = [h[0] for h in HANDLES]


def set_lattice(edge):
    """Scale the lattice and the lengths that follow it to a cube edge (mm)."""
    global LATTICE, SKIN_CLEAR, BONE_STEP
    LATTICE = edge
    SKIN_CLEAR = 0.394 * edge
    BONE_STEP = edge / 3.0


def log(*a):
    print(*a, flush=True)


def skin_dist(V, F, P):
    return np.sqrt(igl.point_mesh_squared_distance(P, V, F)[0])


def tet_volumes(P, T):
    e1, e2, e3 = (P[T[:, i]] - P[T[:, 0]] for i in (1, 2, 3))
    return np.einsum('ij,ij->i', e1, np.cross(e2, e3)) / 6.0


def tet_quality(P, T):
    E = [(0, 1), (0, 2), (0, 3), (1, 2), (1, 3), (2, 3)]
    l2 = sum(np.sum((P[T[:, a]] - P[T[:, b]]) ** 2, 1) for a, b in E) / 6.0
    return 6.0 * np.sqrt(2.0) * tet_volumes(P, T) / l2 ** 1.5


# The head moves as one body: every tet vertex above the plane through the throat under the chin and
# the nape (HEAD_PLANE, a point and its upward normal, set by set_head) is held to SKULL.  Both are
# found on the skin's midline (within MIDLINE mm of it) in the band HEAD_BAND mm below the HEAD joint:
# the chin's underside, its lowest point at least CHIN_AHEAD mm in front of HEAD, and the nape, the
# shallowest point behind HEAD.  The neck's handles share the head's rotation over the neck below the
# plane, from NECK_SPAN of the way along ORIGIN-HEAD to its crossing, split at NECK_SPLIT.
HEAD_PLANE = None
HEAD_BAND = 150.0
CHIN_AHEAD = 60.0
NECK_SPAN = (0.2, 0.9)
NECK_SPLIT = 0.55
MIDLINE = 6.0


def set_head(V, S):
    """The head plane from the template's midline profile near the HEAD joint.  The neck handles' spans
    follow, and their fractions of the head's rotation, by their centres along ORIGIN to the plane, are
    returned relative to NECK's transform, which carries all of it."""
    global HEAD_PLANE
    o, h = S[J['ORIGIN']], S[J['HEAD']]
    mid = V[np.abs(V[:, 1]) < MIDLINE]
    band = mid[(mid[:, 2] > h[2] - HEAD_BAND) & (mid[:, 2] < h[2])]
    front = band[band[:, 0] < h[0] - CHIN_AHEAD]
    chin = front[np.argmin(front[:, 2])]
    back = band[band[:, 0] > h[0]]
    nape = back[np.argmin(back[:, 0])]
    d = nape - chin
    n = np.array([-d[2], 0.0, d[0]])
    n /= np.linalg.norm(n)
    chin = chin * np.array([1.0, 0.0, 1.0])
    HEAD_PLANE = (chin, n)
    tc = ((chin - o) @ n) / ((h - o) @ n)
    t0, t1 = NECK_SPAN[0] * tc, NECK_SPAN[1] * tc
    tm = NECK_SPLIT * tc
    f = {}
    for k, hd in enumerate(HANDLES):
        if hd[0] == 'NECK_A':
            HANDLES[k] = (hd[0], hd[1], [('ORIGIN', 'HEAD', t0, tm)], hd[3], hd[4], hd[5])
            f[k] = 0.5 * (t0 + tm) / tc - 1.0
        elif hd[0] == 'NECK_B':
            HANDLES[k] = (hd[0], hd[1], [('ORIGIN', 'HEAD', tm, t1)], hd[3], hd[4], hd[5])
            f[k] = 0.5 * (tm + t1) / tc - 1.0
    log('head plane through the chin %s and the nape %s; neck crosses it at %.2f of ORIGIN-HEAD' % (
        np.round(chin, 1), np.round(nape, 1), tc))
    return f


def in_head(P):
    return (P - HEAD_PLANE[0]) @ HEAD_PLANE[1] > 0.0


def torso_fractions(S):
    """The torso twist's fraction at each spine handle's centre, by arc length from ORIGIN (none) to
    WAIST (all), relative to the handle's bone: UPSPINE carries none of it, LOWSPINE all."""
    o, b, w = S[J['ORIGIN']], S[J['BACK']], S[J['WAIST']]
    l1, l2 = np.linalg.norm(b - o), np.linalg.norm(w - b)
    f = {}
    for k, h in enumerate(HANDLES):
        if h[4] == 'TORSO':
            a, c, t0, t1 = h[2][0]
            t = 0.5 * (t0 + t1)
            if h[1] == 'UPSPINE':
                f[k] = t * l1 / (l1 + l2)
            else:
                f[k] = (l1 + t * l2) / (l1 + l2) - 1.0
    return f


def bone_points(V, F, S):
    """Points along each handle's segments inside and off the skin: positions, handles, (a, c, t)."""
    pts, owner, seg = [], [], []
    for k, (name, bone, segs, st, part, frac) in enumerate(HANDLES):
        for a, c, t0, t1 in segs:
            pa, pc = S[J[a]], S[J[c]]
            n = max(3, int(np.linalg.norm(pc - pa) / BONE_STEP))
            t = np.linspace(HANDLE_T[0], HANDLE_T[1], n)
            if name.startswith('NECK'):
                # The neck's spans lie below the head plane, sampled on their own; the first stops short
                # of the second, as the other split bones' do.
                t = np.linspace(t0, t1, max(2, int((t1 - t0) * np.linalg.norm(pc - pa) / BONE_STEP) + 1),
                                endpoint=(name == 'NECK_B'))
            elif t1 < HANDLE_T[1]:
                t = t[(t >= t0) & (t < t1)]
            else:
                t = t[t >= t0]
            P = pa + t[:, None] * (pc - pa)
            ok = (igl.fast_winding_number(V, F, P) > 0.5) & (skin_dist(V, F, P) > SKIN_GAP)
            pts.append(P[ok])
            owner += [k] * int(ok.sum())
            seg += [(J[a], J[c], tt) for tt in t[ok]]
    owner = np.array(owner)
    missing = [HNAME[k] for k in range(len(HANDLES)) if not (owner == k).any()]
    assert not missing, 'handles with no bone points: %s' % missing
    return np.vstack(pts), owner, seg


def side(name):
    """'L' or 'R' for a handle of one side, '' for one on the centre."""
    if name[0] in 'LR' and 'L' + name[1:] in HNAME and 'R' + name[1:] in HNAME:
        return name[0]
    return ''


def mirror_name(name):
    if side(name) == 'L':
        return 'R' + name[1:]
    if side(name) == 'R':
        return 'L' + name[1:]
    return name


def stations(V, F, S):
    """Each handle's stations: a point off its bone at the middle of a segment's span, on the side of
    the forward or up direction most across the bone, STATION_R of the way to the skin.  Made on the
    right (+y) side and the centre, and mirrored to the left so the set is mirror symmetric."""
    pts, owner = [], []
    for k, (name, bone, segs, st, part, frac) in enumerate(HANDLES):
        if side(name) == 'L':
            continue
        for i in st:
            a, c, t0, t1 = segs[i]
            pa, pc = S[J[a]], S[J[c]]
            u = (pc - pa) / np.linalg.norm(pc - pa)
            p = pa + 0.5 * (t0 + t1) * (pc - pa)
            if p[1] < 0.0:
                continue
            ref = [np.array([-1.0, 0.0, 0.0]), np.array([0.0, 0.0, 1.0])]
            r = min(ref, key=lambda v: abs(v @ u))
            d = r - (r @ u) * u
            d /= np.linalg.norm(d)
            s = np.arange(0.5, 400.0, 0.5)
            inside = igl.fast_winding_number(V, F, p + s[:, None] * d) > 0.5
            reach = s[np.argmin(inside)]
            q = p + STATION_R * reach * d
            pts.append(q)
            owner.append(k)
            if q[1] > 0.0:
                pts.append(q * np.array([1.0, -1.0, 1.0]))
                owner.append(HNAME.index(mirror_name(name)))
    return np.array(pts), np.array(owner)


def lattice_points(V, F, bp):
    """The face-centred cubic lattice inside the skin, SKIN_CLEAR from it and from the bone points and
    stations bp, mirror symmetric in y."""
    rng = np.random.default_rng(SEED)
    lo, hi = V.min(0), V.max(0)
    ax = [np.arange(np.floor(lo[i] / LATTICE), np.ceil(hi[i] / LATTICE) + 1) * LATTICE for i in range(3)]
    G = np.stack(np.meshgrid(*ax, indexing='ij'), -1).reshape(-1, 3)
    h = 0.5 * LATTICE
    G = np.vstack([G, G + [h, h, 0.0], G + [h, 0.0, h], G + [0.0, h, h]])
    G = G[G[:, 1] >= 0.0]
    jit = JITTER * LATTICE * (rng.random(G.shape) - 0.5)
    jit[G[:, 1] == 0.0, 1] = 0.0
    G = G + jit
    G = G[igl.fast_winding_number(V, F, G) > 0.5]
    G = G[(skin_dist(V, F, G) > SKIN_CLEAR) & (cKDTree(bp).query(G)[0] > SKIN_CLEAR)]
    return np.vstack([G, G[G[:, 1] > 0.0] * np.array([1.0, -1.0, 1.0])])


def tet_mesh(V, F, S):
    """The tet mesh on body (V, S): vertices (skin, then bone points, then stations and lattice points,
    those a tet uses), tets, the tet vertex of each skin vertex (-1 if none), the bone points' tet
    vertices, handles and segments, the stations' tet vertices and handles, and the tet vertices of
    the stations and lattice points (which follow the body alike)."""
    bp, owner, seg = bone_points(V, F, S)
    st, st_owner = stations(V, F, S)
    G = lattice_points(V, F, np.vstack([bp, st]))
    P = np.vstack([V, bp, st, G])
    # The points are mirror symmetric in y; triangulate the y >= 0 half and mirror its tets so the
    # mesh is too.
    d, mir = cKDTree(P).query(P * np.array([1.0, -1.0, 1.0]))
    assert d.max() < 1e-6, 'the points are not mirror symmetric'
    half = np.where(P[:, 1] >= 0.0)[0]
    T = half[Delaunay(P[half]).simplices]
    T = T[igl.fast_winding_number(V, F, P[T].mean(1)) > 0.5]
    M = np.vstack([0.5 * (P[T[:, i]] + P[T[:, j]]) for i, j in ((0, 1), (0, 2), (0, 3), (1, 2), (1, 3), (2, 3))])
    ok = igl.fast_winding_number(V, F, M) > 0.5
    ok[~ok] = igl.point_mesh_squared_distance(M[~ok], V, F)[0] < MID_GAP ** 2
    T = T[ok.reshape(6, -1).all(0)]
    T = np.vstack([T, mir[T]])
    neg = tet_volumes(P, T) < 0
    T[neg] = T[neg][:, [0, 2, 1, 3]]
    T = T[tet_quality(P, T) >= MIN_QUALITY]
    # Only the pieces that hold a bone point.
    E = np.vstack([T[:, [0, i]] for i in (1, 2, 3)])
    _, lab = connected_components(sp.coo_matrix((np.ones(len(E)), (E[:, 0], E[:, 1])), shape=(len(P), len(P))),
                                  directed=False)
    nv, nb = len(V), len(bp)
    T = T[np.isin(lab[T[:, 0]], lab[nv:nv + nb])]
    used = np.zeros(len(P), bool)
    used[T.ravel()] = True
    ns = len(st)
    assert used[nv:nv + nb + ns].all(), 'a bone point or station is in no tet'
    new = np.full(len(P), -1)
    new[used] = np.arange(used.sum())
    grid = np.arange(nv + nb, len(P))
    # The head's tet vertices, held to SKULL; no other handle's bone point or station is in it.
    head = used & in_head(P)
    skull = HNAME.index('SKULL')
    other = np.concatenate([np.arange(nv, nv + nb)[owner != skull], nv + nb + np.arange(ns)[st_owner != skull]])
    assert not head[other].any(), 'a bone point or station of another handle is in the head'
    return dict(TV=P[used], TT=new[T], skin_tv=new[:nv], bs_tv=new[nv:nv + nb], bs_owner=owner, bs_seg=seg,
                st_tv=new[nv + nb:nv + nb + ns], st_owner=st_owner, grid_tv=new[grid][used[grid]],
                head_tv=new[np.where(head)[0]])


def interior_linear(tm, body):
    """Each sex's lattice points as Pbar (n, 3) + Q (n, 3, 6) X: the template position plus the
    cotangent-harmonic extension of the skin's and bone points' displacement from the template."""
    TV, TT = tm['TV'], tm['TT']
    gi = tm['grid_tv']
    bnd = np.setdiff1d(np.arange(len(TV)), gi)
    cols = []
    for sex in ('male', 'female'):
        Yb, Q = body.mesh_lin(sex)
        Sb, SQ = body.skel_lin(sex)
        Q, SQ = Q.reshape(-1, 3, 6), SQ.reshape(-1, 3, 6)
        Bbar = np.zeros((len(TV), 3))
        BQ = np.zeros((len(TV), 3, 6))
        on = tm['skin_tv'] >= 0
        Bbar[tm['skin_tv'][on]] = Yb[on]
        BQ[tm['skin_tv'][on]] = Q[on]
        for tv, (a, c, t) in zip(tm['bs_tv'], tm['bs_seg']):
            Bbar[tv] = (1 - t) * Sb[a] + t * Sb[c]
            BQ[tv] = (1 - t) * SQ[a] + t * SQ[c]
        cols.append(Bbar[bnd] - TV[bnd])
        cols.append(BQ[bnd].reshape(len(bnd), 18))
    H = igl.harmonic(TV, TT, bnd.astype(np.int64), np.hstack(cols), 1)[gi]
    out = {}
    for k, sex in enumerate(('male', 'female')):
        Hk = H[:, 21 * k:21 * (k + 1)]
        out[sex] = (TV[gi] + Hk[:, :3], Hk[:, 3:].reshape(-1, 3, 6))
    return out


def tet_verts(tm, lin, body, sex, X):
    TV = np.zeros_like(tm['TV'])
    Y, S = body.mesh(sex, X), body.skel(sex, X)
    on = tm['skin_tv'] >= 0
    TV[tm['skin_tv'][on]] = Y[on]
    for tv, (a, c, t) in zip(tm['bs_tv'], tm['bs_seg']):
        TV[tv] = (1 - t) * S[a] + t * S[c]
    TV[tm['grid_tv']] = lin[sex][0] + np.einsum('nck,k->nc', lin[sex][1], X)
    return TV


def bbw_weights(tm):
    b, owner = held(tm)
    bc = np.zeros((len(b), len(HANDLES)))
    bc[np.arange(len(b)), owner] = 1.0
    W = igl.bbw(tm['TV'], tm['TT'], b.astype(np.int64), bc, partition_unity=True, max_iter=BBW_MAX_ITER)
    W = np.clip(W, 0.0, 1.0)
    W /= W.sum(1, keepdims=True)
    W[W < MIN_WEIGHT] = 0.0
    return W / W.sum(1, keepdims=True)


def bspline(t):
    """Cubic B-spline bump, 1 at t = 0, 0 for t >= 1."""
    u = 2.0 * np.abs(t)
    out = np.zeros_like(u)
    a = u < 1.0
    b = (u >= 1.0) & (u < 2.0)
    out[a] = 2.0 / 3.0 - u[a] ** 2 + 0.5 * u[a] ** 3
    out[b] = (2.0 - u[b]) ** 3 / 6.0
    return out * 1.5


def handle_mirror():
    """Each handle's mirror image: a weight vector's mirror is w[handle_mirror()]."""
    return np.array([HNAME.index(mirror_name(n)) for n in HNAME])


def extra_weights(W):
    """FAST's additional weights: NUM_EXTRA / 2 seeds and their mirror images (left and right handles'
    weights swapped), spread over the vertices' weights by farthest points (corners counted at half
    distance), each a cubic B-spline bump of radius twice its distance to the nearest other seed or
    corner, so that it vanishes at every handle.  The weights of a vertex's mirror image are its own,
    mirrored, since the handles' are."""
    rng = np.random.default_rng(SEED)
    hm = handle_mirror()
    E = np.eye(W.shape[1])
    cand = W[rng.choice(len(W), min(EXTRA_SAMPLES, len(W)), replace=False)]
    half = NUM_EXTRA // 2
    s = cand[rng.choice(len(cand), half, replace=False)].copy()
    dcorner = 0.5 * np.linalg.norm(cand[:, None, :] - E[None], axis=2).min(1)
    dself = np.linalg.norm(cand - cand[:, hm], axis=1)
    for _ in range(EXTRA_ROUNDS):
        moved = 0.0
        for i in range(half):
            o = np.delete(s, i, 0)
            o = np.vstack([o, o[:, hm]])
            d = np.minimum(np.minimum(dcorner, dself), np.linalg.norm(cand[:, None, :] - o[None], axis=2).min(1))
            new = cand[np.argmax(d)]
            moved = max(moved, np.linalg.norm(new - s[i]))
            s[i] = new
        if moved == 0.0:
            break
    s = np.vstack([s, s[:, hm]])
    We = np.zeros((len(W), NUM_EXTRA))
    for i in range(NUM_EXTRA):
        r = 2.0 * min(0.5 * np.linalg.norm(s[i] - E, axis=1).min(), np.linalg.norm(np.delete(s, i, 0) - s[i], axis=1).min())
        We[:, i] = bspline(np.linalg.norm(W - s[i], axis=1) / r)
    We[We < MIN_EXTRA] = 0.0
    return We


def mirror_maps(tm):
    """Each tet vertex's mirror image, and each tet's."""
    TV, TT = tm['TV'], tm['TT']
    d, mv = cKDTree(TV).query(TV * np.array([1.0, -1.0, 1.0]))
    assert d.max() < 1e-9 and len(set(mv.tolist())) == len(mv), 'the tet vertices are not mirror symmetric'
    key = {tuple(sorted(t)): i for i, t in enumerate(TT.tolist())}
    mt = np.array([key[tuple(sorted(mv[t]))] for t in TT])
    return mv, mt


def fast_groups(tm, W, We):
    """FAST's rotation groups, as igl::partition makes them on the right half: farthest points in the
    weights, each vertex in the nearest one's group, each tet in its vertices' most common group; a
    left tet is in its mirror's group plus FAST_GROUPS / 2."""
    TV, TT = tm['TV'], tm['TT']
    mv, mt = mirror_maps(tm)
    A = np.hstack([W, We])
    right = np.where(TV[:, 1] >= 0.0)[0]
    Ar = A[right]
    k = FAST_GROUPS // 2
    seeds = [int(np.argmax(np.linalg.norm(Ar - Ar.mean(0), axis=1)))]
    dist = np.linalg.norm(Ar - Ar[seeds[0]], axis=1)
    for _ in range(k - 1):
        seeds.append(int(np.argmax(dist)))
        dist = np.minimum(dist, np.linalg.norm(Ar - Ar[seeds[-1]], axis=1))
    lab = np.full(len(TV), -1)
    lab[right] = np.argmin(np.linalg.norm(Ar[:, None, :] - Ar[seeds][None], axis=2), axis=1)
    rtet = (TV[TT][:, :, 1] >= 0.0).all(1)
    assert rtet[mt[~rtet]].all(), 'a left tet has no right mirror'
    G = np.zeros(len(TT), int)
    for t in np.where(rtet)[0]:
        G[t] = np.bincount(lab[TT[t]], minlength=k).argmax()
    for t in np.where(~rtet)[0]:
        G[t] = G[mt[t]] + k
    return G


def held(tm):
    """The tet vertices held to a handle, and their handles: the bone points, the stations, and the rest of
    the head's tet vertices, held to SKULL."""
    tv = np.concatenate([tm['bs_tv'], tm['st_tv']])
    hh = np.concatenate([tm['bs_owner'], tm['st_owner']])
    extra = np.setdiff1d(tm['head_tv'], tv)
    return np.concatenate([tv, extra]), np.concatenate([hh, np.full(len(extra), HNAME.index('SKULL'))])


def off_tets(tm, V):
    """Skin vertices no tet uses: two boundary triangles of the tet mesh, whose mean carries the vertex,
    and the barycentric coordinates in each of the nearest point on the first.  The triangles are the
    nearest one twice, or for a vertex on the centre plane the nearest one and its mirror, so the
    vertex stays on the plane."""
    miss = np.where(tm['skin_tv'] < 0)[0]
    BF = igl.boundary_facets(tm['TT'])
    if isinstance(BF, tuple):
        BF = BF[0]
    _, fi, cp = igl.point_mesh_squared_distance(V[miss], tm['TV'], BF)
    tri = BF[fi]
    bary = igl.barycentric_coordinates(cp, *(tm['TV'][tri[:, i]] for i in range(3)))
    mv, _ = mirror_maps(tm)
    tri2 = tri.copy()
    plane = V[miss, 1] == 0.0
    tri2[plane] = mv[tri[plane]]
    return miss, np.hstack([tri, tri2]), bary


def surface_points():
    """The Bezier surface's control points on the androgynous body, in the order HumanGeom builds its
    nets from ../HumanBezierData.h (BuildHumanBezierNets): each part, its left mirror after a right
    part, around index then along."""
    import re
    text = open(os.path.join(os.path.dirname(HERE), 'HumanBezierData.h')).read()

    def array(name):
        m = re.search(r'static const double %s\[\d+\] = \{(.*?)\};' % name, text, re.S)
        return np.array([float(v) for v in m.group(1).replace(',', ' ').split()])

    table = re.search(r'hb_male_parts\[HUMAN_BEZ_NUM_PART\] = \{(.*?)\n\};', text, re.S).group(1)
    pts = []
    partner = []
    for m in re.finditer(r'\{ "(\w+)", (true|false), (\d+), (\d+), \w+, \w+, hb_(\w+)_male_pbar, \w+ \}', table):
        centre, nu, nv, nm = m.group(2) == 'true', int(m.group(3)), int(m.group(4)), m.group(5)
        P = 0.0
        for sex in ('male', 'female'):
            pb = array('hb_%s_%s_pbar' % (nm, sex)).reshape(nu, nv, 3)
            q = array('hb_%s_%s_q' % (nm, sex)).reshape(nu, nv, 3, 6)
            P = P + 0.5 * (pb + q @ XREF[sex])
        # A point's mirror image is the point at around index ( nu + 2 - i ) mod nu of the left net,
        # or of a centre net itself.
        base = sum(len(q) for q in pts)
        mi = ((nu + 2 - np.arange(nu)) % nu)[:, None] * nv + np.arange(nv)[None, :]
        pts.append(P.reshape(-1, 3))
        if centre:
            partner.append(base + mi.ravel())
        else:
            M = P[(nu + 2 - np.arange(nu)) % nu] * np.array([1.0, -1.0, 1.0])
            pts.append(M.reshape(-1, 3))
            partner.append(base + nu * nv + mi.ravel())
            partner.append(base + mi.ravel())
    return np.vstack(pts), np.concatenate(partner)


def embed(tm, P):
    """The tet each point is in, or else the one it is least far outside (its smallest barycentric
    coordinate the largest), among the tets with the nearest centroids."""
    TV, TT = tm['TV'], tm['TT']
    C = TV[TT].mean(1)
    _, cand = cKDTree(C).query(P, k=48)
    best = np.zeros(len(P), int)
    worst = np.zeros(len(P))
    for i in range(len(P)):
        T = TT[cand[i]]
        A = (TV[T[:, 1:]] - TV[T[:, :1]]).transpose(0, 2, 1)
        b = np.linalg.solve(A, (P[i] - TV[T[:, 0]])[:, :, None])[:, :, 0]
        bary = np.hstack([1.0 - b.sum(1, keepdims=True), b])
        k = np.argmax(bary.min(1))
        best[i] = cand[i][k]
        worst[i] = bary[k].min()
    return best, worst


def mirror_embedding(tm, P, tet, partner):
    """Two tets per control point, whose mean carries it, so that the surface stays mirror symmetric:
    a point on the centre plane (a pole's, or its own image) takes a tet and that tet's mirror, and
    any other point and its mirror image take mirror tets."""
    _, mt = mirror_maps(tm)
    out = np.zeros((len(tet), 2), int)
    for i in range(len(tet)):
        j = partner[i]
        if P[i, 1] == 0.0:
            out[i] = (tet[i], mt[tet[i]])
        elif j < i:
            out[i] = mt[out[j]]
        else:
            out[i] = (tet[i], tet[i])
    return out


def compute(body):
    t0 = time.time()
    V = 0.5 * (body.mesh('male', XREF['male']) + body.mesh('female', XREF['female']))
    S = 0.5 * (body.skel('male', XREF['male']) + body.skel('female', XREF['female']))
    neck = set_head(V, S)
    tm = tet_mesh(V, body.tf, S)
    q = tet_quality(tm['TV'], tm['TT'])
    log('androgynous tet mesh: %d vertices (%d skin, %d bone points, %d stations, %d lattice), %d tets, quality 1%% '
        '%.3f median %.3f, %d skin vertices in no tet (%.0f s)' % (
            len(tm['TV']), (tm['skin_tv'] >= 0).sum(), len(tm['bs_tv']), len(tm['st_tv']),
            len(tm['grid_tv']) - len(tm['st_tv']), len(tm['TT']), np.percentile(q, 1), np.median(q),
            (tm['skin_tv'] < 0).sum(), time.time() - t0))
    lin = interior_linear(tm, body)
    t0 = time.time()
    W = bbw_weights(tm)
    We = extra_weights(W)
    groups = fast_groups(tm, W, We)
    hv, hh = held(tm)
    log('bbw and extra weights %.0f s; ARAP holds %d tet vertices (%d stations, %d in the head)' % (
        time.time() - t0, len(hv), len(tm['st_tv']), len(tm['head_tv'])))
    miss, tri, bary = off_tets(tm, V)
    cp, partner = surface_points()
    cp_tet, cp_min = embed(tm, cp)
    log('surface control points: %d, %d inside a tet; the rest at a smallest barycentric coordinate of %.2f '
        'median, %.2f worst' % (len(cp), (cp_min >= -1e-9).sum(), np.median(cp_min[cp_min < -1e-9]), cp_min.min()))
    cp_tet = mirror_embedding(tm, cp, cp_tet, partner)
    frac = [h[5] for h in HANDLES]
    for k, f in torso_fractions(S).items():
        frac[k] = f
    for k, f in neck.items():
        frac[k] = f
    return dict(tm=tm, lin=lin, W=W, We=We, hv=hv, hh=hh, miss=miss, tri=tri, bary=bary, frac=frac,
                cp_tet=cp_tet,
                groups=groups)


def fmt(x):
    return '%.9g' % x


def emit(lines, ctype, name, A, cols=None):
    A = np.asarray(A)
    flat = A.ravel()
    if cols is None:
        dims = '[%d]' % max(1, len(flat))
        rows = [flat[i:i + 10] for i in range(0, len(flat), 10)]
    else:
        dims = '[%d][%d]' % (max(1, len(flat) // cols), cols)
        rows = [flat[i:i + cols] for i in range(0, len(flat), cols)]
    f = fmt
    if ctype == 'int':
        f = lambda v: '%d' % v
    lines.append('static const %s %s%s = {' % (ctype, name, dims))
    if not len(flat):
        rows = [np.zeros(cols or 1)]
    for r in rows:
        if cols is None:
            lines.append('    ' + ', '.join(f(v) for v in r) + ',')
        else:
            lines.append('    { ' + ', '.join(f(v) for v in r) + ' },')
    lines.append('};')


def emit_csr(lines, name, W):
    start, col, val = [0], [], []
    for r in W:
        nz = np.nonzero(r)[0]
        col += list(nz)
        val += list(r[nz])
        start.append(len(col))
    emit(lines, 'int', name + '_start', start)
    emit(lines, 'int', name + '_col', col)
    emit(lines, 'double', name + '_val', val)


def write_header(path, d):
    tm = d['tm']
    L = ['//', '// This file is released under the terms of the NASA Open Source Agreement (NOSA)',
         '// version 1.3 as detailed in the LICENSE file which accompanies this software.', '//',
         '// HumanTetData.h: generated by src/geom_core/humanshape/human_tets.py.  Do not edit.', '//',
         '// The tet mesh that fills every Human: its tets, how each tet vertex follows the body (a skin vertex,',
         '// a fraction of a bone segment, or per sex Pbar + Q X), its weights on the bone handles and FAST\'s',
         '// additional weights, the tet vertices ARAP holds to each handle, and how the skin vertices no tet',
         '// uses ride on the tet mesh\'s boundary.', '//'] + input_digests([
        'MaleGeomDataReduced.h',
        'FemaleGeomDataReduced.h',
        'HumanBezierData.h',
    ]) + ['//', '',
         '#if !defined(VSP_HUMAN_TET_DATA__INCLUDED_)', '#define VSP_HUMAN_TET_DATA__INCLUDED_', '',
         '#define HUMAN_TET_NUM_HANDLES %d' % len(HANDLES),
         '#define HUMAN_TET_NUM_PARTIALS %d' % len(PARTIALS),
         '#define HUMAN_TET_NUM_EXTRA %d' % d['We'].shape[1],
         '#define HUMAN_TET_NUM_GROUPS %d' % FAST_GROUPS,
         '#define HUMAN_TET_NTV %d' % len(tm['TV']),
         '#define HUMAN_TET_NTT %d' % len(tm['TT']),
         '#define HUMAN_TET_NSKIN %d' % len(tm['skin_tv']),
         '#define HUMAN_TET_NBONE %d' % len(tm['bs_tv']),
         '#define HUMAN_TET_NGRID %d' % len(tm['grid_tv']),
         '#define HUMAN_TET_NHOLD %d' % len(d['hv']),
         '#define HUMAN_TET_NOFF %d' % len(d['miss']),
         '#define HUMAN_TET_NCP %d' % len(d['cp_tet']),
         '',
         '// The bone whose transform each handle takes.']
    emit(L, 'int', 'human_tet_handle_bone', [B[h[1]] for h in HANDLES])
    L.append('// The partial rotation each handle adds to its bone\'s transform (HumanTetPartial, -1 if none),')
    L.append('// and the fraction of it.')
    emit(L, 'int', 'human_tet_handle_partial', [PARTIALS.index(h[4]) if h[4] else -1 for h in HANDLES])
    emit(L, 'double', 'human_tet_handle_frac', d['frac'])
    L.append('// Tets, positively oriented.')
    emit(L, 'int', 'human_tet_tets', tm['TT'], 4)
    L.append('// The tet vertex of each full-mesh skin vertex, -1 if no tet uses it.')
    emit(L, 'int', 'human_tet_skin_tv', tm['skin_tv'])
    L.append('// Bone points: tet vertex, joints a and c, and t, the point being S[a] + t ( S[c] - S[a] ).')
    emit(L, 'int', 'human_tet_bone_tv', tm['bs_tv'])
    emit(L, 'int', 'human_tet_bone_joints', np.array([(a, c) for a, c, t in tm['bs_seg']]), 2)
    emit(L, 'double', 'human_tet_bone_t', np.array([t for a, c, t in tm['bs_seg']]))
    L.append('// Lattice points: tet vertex, and per sex Pbar (mm) and Q, the point being Pbar + Q X.')
    emit(L, 'int', 'human_tet_grid_tv', tm['grid_tv'])
    for sex in ('male', 'female'):
        emit(L, 'double', 'human_tet_%s_grid_pbar' % sex, d['lin'][sex][0], 3)
        emit(L, 'double', 'human_tet_%s_grid_q' % sex, d['lin'][sex][1].reshape(-1, 18), 18)
    L.append('// Handle weights and FAST\'s additional weights per tet vertex (CSR).')
    emit_csr(L, 'human_tet_w', d['W'])
    emit_csr(L, 'human_tet_we', d['We'])
    L.append('// FAST\'s rotation group of each tet; mirror symmetric.')
    emit(L, 'int', 'human_tet_group', d['groups'])
    L.append('// ARAP\'s fixed tet vertices and their handles.')
    emit(L, 'int', 'human_tet_hold_tv', d['hv'])
    emit(L, 'int', 'human_tet_hold_handle', d['hh'])
    L.append('// Skin vertices no tet uses: the vertex, two boundary triangles of tet vertices whose mean carries')
    L.append('// it, barycentric coordinates in each.')
    emit(L, 'int', 'human_tet_off_vert', d['miss'])
    emit(L, 'int', 'human_tet_off_tri', d['tri'], 6)
    emit(L, 'double', 'human_tet_off_bary', d['bary'], 3)
    L.append('// The two tets whose mean carries each control point of the Bezier surface, in the order')
    L.append('// HumanGeom builds its nets (human_bezier.py, HumanBezierData.h); mirror symmetric.')
    emit(L, 'int', 'human_tet_cp_tet', d['cp_tet'], 2)
    L += ['', '#endif // !defined(VSP_HUMAN_TET_DATA__INCLUDED_)', '']
    with open(path, 'w') as f:
        f.write('\n'.join(L))


def check(body, d):
    """Inverted tets over the input box's corners and the 5th/95th percentile bodies, per sex."""
    tm = d['tm']
    for sex, sta, bmi in (('male', (1634.0, 1881.0), (20.7, 39.8)), ('female', (1498.0, 1735.0), (19.6, 43.3))):
        inv = []
        for s in sta:
            for b in bmi:
                for sit in (0.45, 0.57):
                    for age in (18.0, 80.0):
                        X = np.array([s, b, sit, age, b * age, 1.0])
                        inv.append(int((tet_volumes(tet_verts(tm, d['lin'], body, sex, X), tm['TT']) <= 0).sum()))
        v50 = int((tet_volumes(tet_verts(tm, d['lin'], body, sex, XREF[sex]), tm['TT']) <= 0).sum())
        log('%s: inverted tets at the 50th percentile %d; over 16 extreme bodies worst %d, median %d' % (
            sex, v50, max(inv), int(np.median(inv))))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--out', default=os.path.join(os.path.dirname(HERE), 'HumanTetData.h'), help='header to write')
    ap.add_argument('--lattice', type=float, default=LATTICE, help='lattice cube edge (mm)')
    a = ap.parse_args()
    set_lattice(a.lattice)
    body = Body()
    d = compute(body)
    check(body, d)
    write_header(a.out, d)
    log('wrote %s (%.1f MB)' % (a.out, os.path.getsize(a.out) / 1e6))


if __name__ == '__main__':
    main()
