#
# This file is released under the terms of the NASA Open Source Agreement (NOSA)
# version 1.3 as detailed in the LICENSE file which accompanies this software.
#
# Tests of the Human's half mesh as compiled into OpenVSP.
#
#     python3 -m pytest src/geom_core/humanshape/tests
#

import os
import sys

import numpy as np
import igl

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))

from hb_data import Body, XREF
from repair_toes import check_half_mesh


def test_the_half_mesh_is_a_closed_manifold_with_its_mirror():
    b = Body()
    check_half_mesh(b.D['tris'], b.D['male_verts'])


def test_no_face_of_the_feet_is_inverted_or_folded():
    # A face whose normal points into the body, or with the body on both sides.
    b = Body()
    F = b.tf
    for sex in ('male', 'female'):
        V = b.mesh(sex, XREF[sex])
        A, B, C = V[F[:, 0]], V[F[:, 1]], V[F[:, 2]]
        n = np.cross(B - A, C - A)
        n /= np.linalg.norm(n, axis=1)[:, None]
        c = (A + B + C) / 3
        foot = c[:, 2] < 60
        out = igl.winding_number(V, F, c[foot] + 0.2 * n[foot])
        assert (out < 0.5).all(), sex
