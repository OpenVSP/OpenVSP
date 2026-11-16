#!/usr/bin/env python3
#
# This file is released under the terms of the NASA Open Source Agreement (NOSA)
# version 1.3 as detailed in the LICENSE file which accompanies this software.
#
# human_bezier.py
#
# Generate ../HumanBezierData.h: the piecewise Bezier surface of HumanGeom.
#
# The body is a set of overlapping closed parts (trunk, head and neck, and per side
# arm, palm, thumb, four fingers, leg and foot).  Each part is a bicubic B-spline net,
# periodic around and clamped along, with both end rows collapsed to poles.  Its
# control points are linear in the anthropometric vector X, P = Pbar + Q_P X, exactly
# as the mesh is, because each net is one fixed linear fit of the mesh and skeleton.
# HumanGeom poses the nets through its tet mesh (human_tets.py).
# See README.md.
#
# Usage:
#     python3 human_bezier.py [--out FILE] [--check]
#
# Needs NumPy and SciPy.
#

import argparse
import os
import sys
import time

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from hb_data import Body, XREF, XLAYOUT, J, input_digests
from hb_quilt import Part, periodic_knots, clamped_knots, periodic_design, clamped_design
from hb_layout import Layout
import hb_check


def log(*a):
    print(*a, flush=True)


def build_parts(body):
    Yl = body.mesh('male', XLAYOUT)
    Sl = body.skel('male', XLAYOUT)
    lay = Layout(Yl, Sl, body.tf, body.mirf)
    parts = []
    for sp in lay.specs():
        parts.append(Part(sp, Yl, Sl, body.tf, lay.names, lay.cuts, lay.ring))
        log('  part %-8s net %2d x %2d (around x along), %4d free coordinates'
            % (sp.name, sp.nu, sp.nv, parts[-1].E.shape[1]))
    return parts


def linear_nets(body, parts, sex):
    """Pbar (ncp, 3) and Q_P (ncp * 3, 6) of every part."""
    Yb, Q = body.mesh_lin(sex)
    Sb, SQ = body.skel_lin(sex)
    Ab = np.vstack([Yb, Sb]).ravel()
    AQ = np.vstack([Q, SQ])
    out = []
    for p in parts:
        F = p.operator()
        Pb = p.exact((F @ Ab).reshape(p.spec.nu, p.spec.nv, 3)).reshape(-1, 3)
        Qp = (F @ AQ).reshape(p.spec.nu, p.spec.nv, 3, 6)
        # Poles and centre symmetry exactly, column by column of Q_P.
        Qx = np.stack([p.exact(Qp[..., k]) for k in range(6)], -1)
        out.append((Pb, Qx.reshape(-1, 6)))
    return out


def vsp_order(part):
    """Index map from the stored order to the fit's: the around direction is reversed, so that
    d/d(along) x d/d(around) points out of the body, as OpenVSP's surfaces do.  Control point i
    of the periodic basis peaks at u = (i - 1) / nu, so stored i is fit (2 - i) mod nu, which
    takes u to w = -u: the fit's u = 0 (on a centre part's symmetry plane) stays at w = 0."""
    nu, nv = part.spec.nu, part.spec.nv
    return (np.mod(2 - np.arange(nu), nu)[:, None] * nv + np.arange(nv)[None, :]).ravel()


def eye_params(body, parts, lin, sex):
    """The eyes as fixed points of the stored head net, (u, w) of its VspSurf for the right then
    the left eye: the points of the reference body's surface nearest its eye joints."""
    from scipy.optimize import minimize
    ip = [p.spec.name for p in parts].index('head')
    p = parts[ip]
    nu, nv = p.spec.nu, p.spec.nv
    Pb, Qp = lin[ip]
    X = XREF[sex]
    P = (Pb + (Qp @ X).reshape(-1, 3))[vsp_order(p)].reshape(nu, nv, 3)
    S = body.skel(sex, X)

    def pnt(a):
        return np.einsum('i,j,ijc->c', periodic_design(np.array([a[0]]), nu)[0],
                         clamped_design(np.array([a[1]]), nv)[0], P)

    out = []
    for k in ('REYE', 'LEYE'):
        e = S[J[k]]
        uu, vv = np.meshgrid(np.linspace(0, 1, 200, endpoint=False), np.linspace(0, 1, 201), indexing='ij')
        G = np.einsum('ai,bj,ijc->abc', periodic_design(uu[:, 0], nu), clamped_design(vv[0], nv), P)
        a0 = np.unravel_index(np.argmin(np.linalg.norm(G - e, axis=2)), G.shape[:2])
        r = minimize(lambda a: np.sum((pnt(a) - e) ** 2), [uu[a0], vv[a0]], method='Nelder-Mead',
                     options=dict(xatol=1e-10, fatol=1e-12))
        u, v = r.x[0] % 1.0, r.x[1]
        out += [v * (nv - 3), u * nu]
    return ip, out


# ----------------------------------------------------------------------------
# C++ output
# ----------------------------------------------------------------------------

def fmt(x):
    if x == 0.0:
        return '0.0'
    return '%.10g' % x


def emit_array(lines, ctype, name, A, cols):
    A = np.asarray(A)
    flat = A.ravel()
    lines.append('static const %s %s[%d] = {' % (ctype, name, len(flat)))
    rows = (len(flat) + cols - 1) // cols
    for r in range(rows):
        vals = flat[r * cols:(r + 1) * cols]
        if ctype == 'int':
            body = ', '.join('%d' % v for v in vals)
        else:
            body = ', '.join(fmt(v) for v in vals)
        lines.append('    ' + body + (',' if r < rows - 1 else ''))
    lines.append('};')
    lines.append('')


def write_header(path, parts, data, eyes):
    lines = []
    lines.append('#if !defined(VSP_HUMAN_BEZIER_DATA__INCLUDED_)')
    lines.append('#define VSP_HUMAN_BEZIER_DATA__INCLUDED_')
    lines.append('')
    lines.append('//////////////////////////////////////////////////////////////////////////////')
    lines.append('// GENERATED FILE -- DO NOT EDIT BY HAND.')
    lines.append('//')
    lines.append('// Produced by src/geom_core/humanshape/human_bezier.py from MaleGeomDataReduced.h and')
    lines.append('// FemaleGeomDataReduced.h.  See src/geom_core/humanshape/README.md.')
    lines.extend(input_digests(['MaleGeomDataReduced.h', 'FemaleGeomDataReduced.h']))
    lines.append('//')
    lines.append('// The piecewise Bezier surface of HumanGeom, as closed parts.  Each stored part is a')
    lines.append('// bicubic B-spline net of nu x nv control points, indexed [i around][j along], periodic')
    lines.append('// around (the unrolled knots uknots, nu + 7 values, take control point i mod nu) and')
    lines.append('// clamped along (vknots, nv + 4 values).  Rows j = 0 and j = nv - 1 are poles.')
    lines.append('// d/d(along) x d/d(around) points out of the body.  Centre parts are mirror')
    lines.append('// symmetric about w = 0 and w = 0.5; each right part (_R) also gives the left one, its')
    lines.append('// mirror image with around index i taken to (2 - i) mod nu, so that w goes to -w.')
    lines.append('//')
    lines.append('// Per sex and part, with X = { stature, BMI, sit-fraction, age, BMI*age, 1 }, the net in')
    lines.append('// the data pose is P0[c][k] = pbar[3 c + k] + sum_m q[6 ( 3 c + k ) + m] X[m].')
    lines.append('//////////////////////////////////////////////////////////////////////////////')
    lines.append('')
    lines.append('#define HUMAN_BEZ_NUM_PART %d' % len(parts))
    lines.append('')
    table = {sex: [] for sex in data}
    for ip, p in enumerate(parts):
        s = p.spec
        nm = s.name.replace('_R', '')
        lines.append('//==== %s ====//' % s.name)
        lines.append('')
        emit_array(lines, 'double', 'hb_%s_uknots' % nm, periodic_knots(s.nu), 8)
        emit_array(lines, 'double', 'hb_%s_vknots' % nm, clamped_knots(s.nv), 8)
        order = vsp_order(p)
        for sex, lin in data.items():
            Pb, Qp = lin[ip]
            pre = 'hb_%s_%s' % (nm, sex)
            emit_array(lines, 'double', pre + '_pbar', Pb[order], 3)
            emit_array(lines, 'double', pre + '_q', Qp.reshape(-1, 3, 6)[order], 6)
            table[sex].append('    { "%s", %s, %d, %d, hb_%s_uknots, hb_%s_vknots, %s_pbar, %s_q }'
                              % (s.name, 'true' if s.side == 'C' else 'false', s.nu, s.nv, nm, nm, pre, pre))
    lines.append('// The eyes: the head part, and per sex (u, w) of its surface for the right then left eye.')
    lines.append('#define HUMAN_BEZ_EYE_PART %d' % eyes['male'][0])
    for sex in data:
        lines.append('static const double hb_%s_eye_uw[4] = { %s };' % (sex, ', '.join(fmt(x) for x in eyes[sex][1])))
    lines.append('')
    for sex in data:
        lines.append('static const HumanBezPartData hb_%s_parts[HUMAN_BEZ_NUM_PART] = {' % sex)
        lines.append(',\n'.join(table[sex]))
        lines.append('};')
        lines.append('')
    lines.append('#endif // !defined(VSP_HUMAN_BEZIER_DATA__INCLUDED_)')
    with open(path, 'w', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')


def main():
    ap = argparse.ArgumentParser(description='Generate ../HumanBezierData.h, the Human\'s Bezier surface data.')
    ap.add_argument('--out', default=os.path.join(os.path.dirname(HERE), 'HumanBezierData.h'))
    ap.add_argument('--check', action='store_true', help='also measure the fit')
    args = ap.parse_args()

    t0 = time.time()
    body = Body()
    log('parts (fitted on the layout body):')
    parts = build_parts(body)
    data = {}
    for sex in ('male', 'female'):
        log('%s: nets linear in X' % sex)
        data[sex] = linear_nets(body, parts, sex)
    eyes = {sex: eye_params(body, parts, data[sex], sex) for sex in data}
    write_header(args.out, parts, data, eyes)
    log('wrote %s (%.0f s)' % (args.out, time.time() - t0))
    if args.check:
        hb_check.eye_report(body, parts, data, eyes, vsp_order, log)
        hb_check.report(body, parts, data, log)


if __name__ == '__main__':
    main()
