#
# This file is released under the terms of the NASA Open Source Agreement (NOSA)
# version 1.3 as detailed in the LICENSE file which accompanies this software.
#
# hb_data.py
#
# Inputs of the HumanGeom Bezier surface generator: the compiled anthropometric
# tables (../MaleGeomDataReduced.h, ../FemaleGeomDataReduced.h), the closed
# full mesh made from the half mesh and its mirror, and the 25 joint skeleton.
#
# Every number is rounded to float on reading, because HumanGeom compiles the
# tables as float (REAL_T); the fit then sees the mesh HumanGeom builds.
#

import hashlib
import os
import re

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
GEOM_CORE = os.path.dirname(HERE)

NUM_MESH_VERT = 2943
NUM_MESH_TRI = 5768
NUM_SKEL = 25

# HumanGeom::joints
JOINTS = ['ORIGIN', 'BACK', 'WAIST', 'HEAD', 'LHIP', 'LKNEE', 'LANKLE', 'LTOE', 'RHIP', 'RKNEE', 'RANKLE',
          'RTOE', 'LSHOULDER', 'LELBOW', 'LWRIST', 'LFINGER', 'RSHOULDER', 'RELBOW', 'RWRIST', 'RFINGER',
          'REYE', 'LEYE', 'DES_EYE', 'RWRISTAX', 'LWRISTAX']
J = {n: i for i, n in enumerate(JOINTS)}

# HumanGeom::bones; bone b ends at joint b + 1.
BONES = ['UPSPINE', 'LOWSPINE', 'NECK', 'LPELVIS', 'LTHIGH', 'LSHIN', 'LFOOT', 'RPELVIS', 'RTHIGH', 'RSHIN',
         'RFOOT', 'LCLAV', 'LBICEP', 'LFOREARM', 'LHAND', 'RCLAV', 'RBICEP', 'RFOREARM', 'RHAND', 'SKULL',
         'LORBITAL', 'RORBITAL', 'RWRISTBASE', 'LWRISTBASE']
B = {n: i for i, n in enumerate(BONES)}


_NUM_RE = re.compile(r'[-+]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?f?')


def _block(text, member):
    m = re.search(r'HumanGeom::m_' + re.escape(member) + r'\b[^=;]*=\s*\{', text)
    if not m:
        raise RuntimeError('array m_%s not found' % member)
    start = m.end()
    end = text.index('};', start)
    return [t.rstrip('f') for t in _NUM_RE.findall(text[start:end])]


def _floats(text, member, cols):
    v = np.array([float(t) for t in _block(text, member)], dtype=np.float64)
    return v.astype(np.float32).astype(np.float64).reshape(-1, cols)


def input_digests(names):
    """Comment lines naming the SHA-1 of each input header (in ../), which a generated header
    records so that tests/test_generated.py can tell when it is stale."""
    lines = []
    for name in names:
        with open(os.path.join(GEOM_CORE, name), 'rb') as f:
            lines.append('// Input %s SHA-1 %s' % (name, hashlib.sha1(f.read()).hexdigest()))
    return lines


def read_tables():
    """The compiled tables of both sexes, as float64 arrays of float values."""
    out = {}
    for sex in ('male', 'female'):
        text = open(os.path.join(GEOM_CORE, sex.capitalize() + 'GeomDataReduced.h')).read()
        out[sex + '_verts'] = _floats(text, sex + '_half_verts', 3)
        out[sex + '_Q'] = _floats(text, sex + '_half_Q', 6)
        out[sex + '_skel'] = _floats(text, sex + '_skel_verts', 3)
        out[sex + '_skelQ'] = _floats(text, sex + '_skel_Q', 6)
        if sex == 'male':
            out['tris'] = np.array([int(t) for t in _block(text, 'half_tris')]).reshape(-1, 3)
            out['skel_indx'] = np.array([int(t) for t in _block(text, 'skel_indx')])
    assert out['male_verts'].shape == (NUM_MESH_VERT, 3)
    assert out['tris'].shape == (NUM_MESH_TRI, 3)
    return out


def xvec(stature, bmi, sitfrac, age):
    """HumanGeom's anthropometric input vector."""
    return np.array([stature, bmi, sitfrac, age, bmi * age, 1.0])


# The 50th percentile body of each sex at HumanGeom's default sitting fraction and age.
XREF = {'male': xvec(1756.0, 27.7, 0.51, 30.0), 'female': xvec(1619.0, 27.7, 0.51, 30.0)}

# The body the cut planes and part axes of hb_layout were placed on.
XLAYOUT = xvec(1756.0, 27.7, 0.52, 30.0)


class Body:
    """The closed full mesh: the half mesh, then the mirror of its vertices off the centre plane.

    The vertex order is HumanGeom's full mesh (SetupFullMesh): the half mesh's vertices, then the
    mirrors of those off the centre plane in order."""

    def __init__(self, tables=None):
        if tables is None:
            tables = read_tables()
        self.D = D = tables
        self.tris = D['tris']
        self.nh = nh = len(D['male_verts'])
        self.cen = cen = D['male_verts'][:, 1] == 0.0
        for sex in ('male', 'female'):
            assert np.array_equal(cen, D[sex + '_verts'][:, 1] == 0.0)
            assert not D[sex + '_Q'].reshape(nh, 3, 6)[cen, 1, :].any(), 'a centre vertex leaves the centre plane'
        mir = np.full(nh, -1)
        mir[~cen] = nh + np.arange((~cen).sum())
        mir[cen] = np.where(cen)[0]
        self.nf = nh + int((~cen).sum())
        self.src = np.r_[np.arange(nh), np.where(~cen)[0]]
        self.sgn = np.r_[np.ones(nh), -np.ones((~cen).sum())]
        self.tf = np.vstack([self.tris, mir[self.tris][:, ::-1]])
        # full-mesh vertex -> its mirror image
        self.mirf = np.r_[mir, np.where(~cen)[0]]

    def half(self, sex, X):
        return self.D[sex + '_verts'] + (self.D[sex + '_Q'] @ X).reshape(-1, 3)

    def full_from_half(self, Yh):
        Y = Yh[self.src].copy()
        Y[:, 1] *= self.sgn
        return Y

    def mesh(self, sex, X):
        """Full mesh at X, in mm, in the data pose."""
        return self.full_from_half(self.half(sex, X))

    def skel(self, sex, X):
        """The 25 joints at X, in mm, in the data pose."""
        D = self.D
        S = D[sex + '_skel'] + (D[sex + '_skelQ'] @ X).reshape(-1, 3)
        out = np.zeros((NUM_SKEL, 3))
        for i, k in enumerate(D['skel_indx']):
            p = S[abs(k)].copy()
            if k < 0:
                p[1] *= -1
            out[i] = p
        return out

    def mesh_lin(self, sex):
        """(Ybar, Q): full mesh = Ybar + (Q @ X).reshape(-1, 3)."""
        D = self.D
        Yb = self.full_from_half(D[sex + '_verts'])
        Qh = D[sex + '_Q'].reshape(self.nh, 3, 6)
        Q = Qh[self.src].copy()
        Q[:, 1, :] *= self.sgn[:, None]
        return Yb, Q.reshape(-1, 6)

    def skel_lin(self, sex):
        """(Sbar, SQ) of the 25 joints."""
        D = self.D
        Sb = D[sex + '_skel']
        SQ = D[sex + '_skelQ'].reshape(-1, 3, 6)
        outb = np.zeros((NUM_SKEL, 3))
        outq = np.zeros((NUM_SKEL, 3, 6))
        for i, k in enumerate(D['skel_indx']):
            outb[i] = Sb[abs(k)]
            outq[i] = SQ[abs(k)]
            if k < 0:
                outb[i, 1] *= -1
                outq[i, 1] *= -1
        return outb, outq.reshape(-1, 6)
