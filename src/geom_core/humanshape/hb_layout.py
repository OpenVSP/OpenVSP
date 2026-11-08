#
# This file is released under the terms of the NASA Open Source Agreement (NOSA)
# version 1.3 as detailed in the LICENSE file which accompanies this software.
#
# hb_layout.py
#
# The decomposition of the body into overlapping closed parts: trunk, head and neck,
# and per side arm, palm, thumb, four fingers, leg and foot.  Cut planes split the mesh
# into regions; each part is fitted to its region and continues past each cut into its
# neighbour (a hidden "nose"), or spans a hole where a child part attaches.
#
# Everything here is fixed on the reference body (the 50th percentile male), so the fit
# of every body is the same linear operator.  Right parts are specified; left parts are
# their mirror images.
#

import numpy as np
from scipy.sparse import coo_matrix
from scipy.sparse.csgraph import connected_components

from hb_data import J
from hb_quilt import Capsule, PartSpec, unit

MIRV = np.array([1.0, -1.0, 1.0])


class Cut:
    """A cut plane through point p with normal n (child side positive), active within radius rc
    of p, between a parent and a child region."""

    def __init__(self, name, p, n, rc, parent, child):
        self.name, self.parent, self.child, self.rc = name, parent, child, rc
        self.p = np.asarray(p, float)
        self.n = unit(np.asarray(n, float))

    def mirrored(self):
        def f(s):
            return s.replace('_R', '_L')
        return Cut(f(self.name), self.p * MIRV, self.n * MIRV, self.rc, f(self.parent), f(self.child))

    def side(self, Pts):
        return (Pts - self.p) @ self.n

    def active(self, Pts):
        d = Pts - self.p
        s = d @ self.n
        return np.linalg.norm(d - np.outer(s, self.n), axis=1) < self.rc


def _edges(T):
    E = np.vstack([T[:, [0, 1]], T[:, [1, 2]], T[:, [2, 0]]])
    return np.unique(np.sort(E, 1), axis=0)


def assign(Y, T, cuts, seeds):
    """Region name of every mesh vertex: drop the edges an active cut crosses, take the
    connected components, name each by the seed point nearest one of its vertices."""
    E = _edges(T)
    keep = np.ones(len(E), bool)
    for c in cuts:
        sa, sb = c.side(Y[E[:, 0]]), c.side(Y[E[:, 1]])
        mid = 0.5 * (Y[E[:, 0]] + Y[E[:, 1]])
        keep &= ~((sa * sb < 0) & c.active(mid))
    n = len(Y)
    A = coo_matrix((np.ones(keep.sum()), (E[keep, 0], E[keep, 1])), shape=(n, n))
    _, lab = connected_components(A, directed=False)
    names = np.array([''] * n, dtype=object)
    for nm, sp in seeds.items():
        v = np.argmin(np.linalg.norm(Y - sp, axis=1))
        names[lab == lab[v]] = nm
    return names


def ring(Y, T, names, cut):
    """Edges of the loop between a cut's child and parent regions: (i in child, j in parent, frac)."""
    E = _edges(T)
    a, b = E[:, 0], E[:, 1]
    m = (((names[a] == cut.child) & (names[b] == cut.parent)) | ((names[a] == cut.parent) & (names[b] == cut.child)))
    a, b = a[m], b[m]
    sw = names[a] != cut.child
    a2 = np.where(sw, b, a)
    b2 = np.where(sw, a, b)
    ha, hb = cut.side(Y[a2]), cut.side(Y[b2])
    return a2, b2, np.clip(ha / (ha - hb), 0, 1)


def finger_cuts(Y, T, names, wr, h3):
    """Cuts of the four fingers off the palm, index first.  Each finger is cut square to the
    hand at the least distance from the wrist at which it is a separate piece of the hand."""
    hand = names == 'hand_R'
    s = (Y - wr) @ h3
    Eh = _edges(T)

    def pieces(s0):
        m = hand & (s > s0)
        E = Eh[m[Eh].all(1)]
        A = coo_matrix((np.ones(len(E)), (E[:, 0], E[:, 1])), shape=(len(Y), len(Y)))
        _, lab = connected_components(A, directed=False)
        lab = np.where(m, lab, -1)
        return [np.where(lab == c)[0] for c in np.unique(lab[lab >= 0]) if (lab == c).sum() > 5]

    # The tips, from a station past every web.
    tips = [c[np.argmax(s[c])] for c in pieces(s[hand].min() + 0.65 * np.ptp(s[hand]))]
    tips = sorted(tips, key=lambda t: Y[t][0])
    assert len(tips) == 4, 'expected four fingers'
    cuts = {}
    seeds = {}
    levels = np.arange(np.floor(s[hand].min()), s[tips].min(), 1.0)
    for k, tip in enumerate(tips):
        for s0 in levels:
            c = next(c for c in pieces(s0) if tip in c)
            if not np.isin([t for t in tips if t != tip], c).any():
                break
        # The edges from the piece to the palm, and the plane between their ends.
        inc = np.isin(Eh, c)
        E = Eh[inc[:, 0] != inc[:, 1]]
        sl = s0
        mid = 0.5 * (Y[E[:, 0]] + Y[E[:, 1]])
        p = wr + h3 * sl
        p = p + (mid.mean(0) - p) - ((mid.mean(0) - p) @ h3) * h3
        q = mid - p
        rad = np.linalg.norm(q - np.outer(q @ h3, h3), axis=1).max()
        name = 'finger%d_R' % (k + 1)
        cuts[name] = Cut(name, p, h3, rad + 1.0, 'hand_R', name)
        seeds[name] = Y[tip]
    return cuts, seeds


class Layout:
    """Cuts, seeds and part specifications, fixed on the reference body (Yr, Sr)."""

    def __init__(self, Yr, Sr, T, mirf):
        self.Yr, self.Sr, self.T = Yr, Sr, T
        S = Sr
        wr, fi = S[J['RWRIST']], S[J['RFINGER']]
        # hand frame: e3 wrist to finger, e1 forward
        h3 = unit(fi - wr)
        h1 = np.array([-1.0, 0, 0])
        h1 -= h1 @ h3 * h3
        h1 = unit(h1)
        h2 = np.cross(h3, h1)
        hm = (np.linalg.norm(Yr - wr, axis=1) < 220) & (Yr[:, 1] > 300) & (((Yr - wr) @ h3) > -20)
        thumb_tip = Yr[np.where(hm)[0][np.argmax(((Yr - wr) @ h1)[hm])]]
        thumb_base = wr + 30 * h1 - 12 * h2 + 22 * h3
        thumb_ax = unit(thumb_tip - thumb_base)
        sh, el = S[J['RSHOULDER']], S[J['RELBOW']]
        hp, kn, an, to = S[J['RHIP']], S[J['RKNEE']], S[J['RANKLE']], S[J['RTOE']]
        og, hd = S[J['ORIGIN']], S[J['HEAD']]
        farm = unit(wr - el)
        thigh = unit(kn - hp)
        neck_ax = unit(hd - og)
        self.cuts_R = {
            'neck': Cut('neck', og + neck_ax * (1500 - og[2]) / neck_ax[2], neck_ax, 110, 'torso', 'head'),
            'shoulder_R': Cut('shoulder_R', [130.0, 197.6, 1356.0], [0, 1.0, -0.1], 120, 'torso', 'arm_R'),
            'wrist_R': Cut('wrist_R', wr + farm * 5, farm, 70, 'arm_R', 'hand_R'),
            'thumb_R': Cut('thumb_R', thumb_base + thumb_ax * 0.5 * np.linalg.norm(thumb_tip - thumb_base),
                           thumb_ax, 28, 'hand_R', 'thumb_R'),
            'hip_R': Cut('hip_R', hp + thigh * 90, thigh + np.array([0, 0.55, 0]), 165, 'torso', 'leg_R'),
            'ankle_R': Cut('ankle_R', [an[0], an[1], an[2] + 35], [0, 0, -1.0], 80, 'leg_R', 'foot_R'),
        }
        seeds = {'torso': np.array([130, 0, 1100.0]), 'head': hd + np.array([0, 0, 80.0]),
                 'arm_R': el + np.array([0, 40, 0]), 'hand_R': wr + 60 * h3, 'thumb_R': thumb_tip, 'leg_R': kn, 'foot_R': to}
        self.cuts = list(self.cuts_R.values()) + [c.mirrored() for c in self.cuts_R.values() if c.name.endswith('_R')]
        self.seeds = dict(seeds)
        self.seeds.update({k.replace('_R', '_L'): v * MIRV for k, v in seeds.items() if k.endswith('_R')})
        self.names = assign(Yr, T, self.cuts, self.seeds)
        # The fingers, cut where each parts from the palm, then the regions again with them.
        fcuts, fseeds = finger_cuts(Yr, T, self.names, wr, h3)
        self.cuts_R.update(fcuts)
        self.cuts += list(fcuts.values()) + [c.mirrored() for c in fcuts.values()]
        self.seeds.update(fseeds)
        self.seeds.update({k.replace('_R', '_L'): v * MIRV for k, v in fseeds.items()})
        self.names = assign(Yr, T, self.cuts, self.seeds)
        # The regions are mirror images of each other.
        assert all(self.names[mirf[i]] == self.names[i].replace('_R', '#').replace('_L', '_R').replace('#', '_L')
                   for i in range(len(Yr)))
        self.h = (wr, h1, h2, h3)
        self.thumb = (thumb_tip, thumb_ax)

    def ring(self, cut):
        return ring(self.Yr, self.T, self.names, cut)

    def specs(self):
        """The right and centre parts.  Order is that of the generated data."""
        c = self.cuts_R
        Yr, S = self.Yr, self.Sr
        wr, h1, h2, h3 = self.h
        thumb_tip, thumb_ax = self.thumb
        sh, el = S[J['RSHOULDER']], S[J['RELBOW']]
        kn = S[J['RKNEE']]
        foot = Yr[self.names == 'foot_R']
        heel = foot[np.argmax(foot[:, 0])]
        toe = foot[np.argmin(foot[:, 0])]
        head_c = np.array([85.0, 0, 1665.0])
        thumb_tipv = np.where(np.linalg.norm(Yr - thumb_tip, axis=1) < 14)[0]

        sp = []
        sp.append(PartSpec('torso', ['torso'], 'C', Capsule([120, 0, 880], [120, 0, 1400], [-1, 0, 0], 110, 130), 32, 34,
                           ('pole', 'ring'), ('nose', 'neck', J['HEAD'], 0.45), lam=1e-3, free=100.0, reach=0.1))
        sp.append(PartSpec('head', ['head'], 'C', Capsule(c['neck'].p, head_c, [-1, 0, 0], 60, 95), 24, 24,
                           ('nose', 'neck', J['HEAD'], -0.5), ('pole', 'ring')))
        arm_cap = Capsule(sh, c['wrist_R'].p, [-1, 0, 0], 60, 40)
        arm_cap.set_warp([(arm_cap(el[None])[1][0], 0.06, 3.0)])
        sp.append(PartSpec('arm_R', ['arm_R'], 'R', arm_cap, 16, 24,
                           ('nose', 'shoulder_R', J['ORIGIN'], 0.3), ('nose', 'wrist_R', J['RFINGER'], 0.25)))
        sp.append(PartSpec('hand_R', ['hand_R'], 'R', Capsule(c['wrist_R'].p, wr + 75 * h3, h1, 40, 35), 20, 20,
                           ('nose', 'wrist_R', J['RELBOW'], 0.15), ('pole', 'ring'), lam=1e-3))
        # The thumb fits the palm's skin for 10 mm below its base, as the fingers do (its broad
        # base sticks out of the body with more).
        sp.append(PartSpec('thumb_R', ['thumb_R'], 'R', Capsule(c['thumb_R'].p, thumb_tip - 12 * thumb_ax, h1, 20, 12),
                           8, 10, ('nose', 'thumb_R', ('verts', thumb_tipv), -0.75), ('pole', 'ring'), lam=1e-3,
                           envelope=0.85, overlap={'thumb_R': 10.0}))
        # Each finger also fits the palm's skin for 20 mm below its base, so that it meets the
        # palm tangent there, and closes out three quarters of its length inside the palm.
        for k in range(1, 5):
            fc = c['finger%d_R' % k]
            fv = np.where(self.names == fc.child)[0]
            tip = Yr[fv[np.argmax((Yr[fv] - fc.p) @ fc.n)]]
            tipv = fv[np.linalg.norm(Yr[fv] - tip, axis=1) < 10]
            fax = unit(tip - fc.p)
            sp.append(PartSpec(fc.child, [fc.child], 'R', Capsule(fc.p, tip - 9 * fax, h1, 15, 9), 8, 10,
                               ('nose', fc.name, ('verts', tipv), -0.75), ('pole', 'ring'), lam=1e-3, envelope=0.85,
                               overlap={fc.name: 20.0}))
        leg_cap = Capsule(c['hip_R'].p, c['ankle_R'].p, [-1, 0, 0], 90, 45)
        leg_cap.set_warp([(leg_cap(kn[None])[1][0], 0.06, 3.0)])
        sp.append(PartSpec('leg_R', ['leg_R'], 'R', leg_cap, 16, 28,
                           ('nose', 'hip_R', J['RHIP'], 1.0), ('nose', 'ankle_R', J['RANKLE'], 1.4)))
        a = heel + np.array([-40.0, 0, 0])
        a[2] = 40
        sp.append(PartSpec('foot_R', ['foot_R'], 'R', Capsule(a, toe + np.array([20.0, 0, 0]), [0, 0, -1], 40, 20),
                           20, 20, ('pole', 'ring'), ('pole', 'ring'), lam=1e-3, hull=(140, 260, 4)))
        return sp
