# Copyright (c) 2018 Uber Technologies, Inc.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import degen_geom as dg
import openvsp as vsp

def parse_degen_geom(degen_geom_res_id):
    # Get all of the degen geom results managers ids
    degen_ids = vsp.GetStringResults(degen_geom_res_id, "Degen_DegenGeoms")

    degen_objects = []
    # loop over all degen objects
    for degen_id in degen_ids:
        res = vsp.parse_results_object(degen_id)
        degen_obj = dg.DegenGeom(res)

        surf_id = res.surf[0]
        res = vsp.parse_results_object(surf_id)
        degen_obj.surf = dg.DegenSurf(res)

        stick_ids = vsp.GetStringResults(degen_id, 'sticks')
        for stick_id in stick_ids:
            res = vsp.parse_results_object(stick_id)
            degen_obj.sticks.append(dg.DegenStick(res))

        hinge_ids = vsp.GetStringResults(degen_id, "hinges")
        for hinge_id in hinge_ids:
            res = vsp.parse_results_object(hinge_id)
            degen_obj.hinge_lines.append(dg.DegenHinge(res))

        point_ids = vsp.GetStringResults(degen_id, "point")
        for point_id in point_ids:
            res = vsp.parse_results_object(point_id)
            degen_obj.point = dg.DegenPoint(res)

        disk_ids = vsp.GetStringResults(degen_id, "disk")
        for disk_id in disk_ids:
            res = vsp.parse_results_object(disk_id)
            degen_obj.disk = dg.DegenDisk(res)

        degen_objects.append(degen_obj)

    return degen_objects
