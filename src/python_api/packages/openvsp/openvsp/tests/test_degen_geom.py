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

import openvsp as vsp
from unittest import TestCase
import numpy.testing as npt
import numpy as np
from utilities import RunManager

class TestOpenVSP(TestCase):
    def test_degen_geom(self):
        # Test analysis manager
        vsp.VSPRenew()
        vsp.ClearVSPModel()

        # Print all types
        print(vsp.GetGeomTypes())

        prop_id = vsp.AddGeom("PROP")
        blank_id = vsp.AddGeom("BLANK")
        disk_id = vsp.AddGeom("Disk")
        vsp.SetParmVal(blank_id, "Point_Mass_Flag", "Mass", 1)
        vsp.SetParmVal(blank_id, "Point_Mass", "Mass", 5.0)
        wing_id = vsp.AddGeom("WING")
        pod_id = vsp.AddGeom("POD")
        vsp.AddSubSurf(pod_id, vsp.SS_RECTANGLE)
        vsp.AddSubSurf(wing_id, vsp.SS_CONTROL)

        vsp.Update()

        # Run Degen Geom
        print(vsp.FindGeoms())
        print(vsp.GetAnalysisInputNames("DegenGeom"))
        vsp.SetAnalysisInputDefaults("DegenGeom")
        vsp.SetIntAnalysisInput("DegenGeom", "WriteMFileFlag", [0], 0)
        vsp.SetIntAnalysisInput("DegenGeom", "WriteCSVFlag", [0], 0)
        vsp.PrintAnalysisInputs("DegenGeom")
        degen_results_id = vsp.ExecAnalysis("DegenGeom")

        print(vsp.GetAllResultsNames())
        vsp.PrintResults(degen_results_id)
        blank_ids = vsp.GetStringResults(degen_results_id, "Degen_BlankGeoms")
        degen_ids = vsp.GetStringResults(degen_results_id, "Degen_DegenGeoms")

        for blank_id in blank_ids:
            vsp.PrintResults(blank_id)

        for degen_id in degen_ids:
            vsp.PrintResults(degen_id)
            t = vsp.GetStringResults(degen_id, "type", 0)[0]
            if t == "DISK":
                disk_id = vsp.GetStringResults(degen_id, "disk", 0)[0]
                vsp.PrintResults(disk_id)
            surf_id = vsp.GetStringResults(degen_id, "surf", 0)[0]
            vsp.PrintResults(surf_id)
            areas = vsp.GetDoubleMatResults(surf_id, "area")

            plate_ids = vsp.GetStringResults(degen_id, "plates")
            for plate_id in plate_ids:
                vsp.PrintResults(plate_id)

            stick_ids = vsp.GetStringResults(degen_id, "sticks")
            for stick_id in stick_ids:
                vsp.PrintResults(stick_id)

            if t != "DISK":
                point_id = vsp.GetStringResults(degen_id, "point")[0]
                vsp.PrintResults(point_id)

            subsurf_ids = vsp.GetStringResults(degen_id, "subsurfs")
            for ss_id in subsurf_ids:
                vsp.PrintResults(ss_id)

            hinge_ids = vsp.GetStringResults(degen_id, "hinges")
            for hinge_id in hinge_ids:
                vsp.PrintResults(hinge_id)

        self.assertTrue(True)


    def test_double_mat(self):
        vsp.VSPRenew()
        vsp.ClearVSPModel()
        pod_id = vsp.AddGeom("POD")

        vsp.Update()

        degen_results_id = vsp.ExecAnalysis("DegenGeom")
        degen_ids = vsp.GetStringResults(degen_results_id, "Degen_DegenGeoms")

        for degen_id in degen_ids:
            surf_id = vsp.GetStringResults(degen_id, "surf", 0)[0]
            vsp.PrintResults(surf_id)
            areas = vsp.GetDoubleMatResults(surf_id, "area")

            print(areas[0][0])
        self.assertTrue(True)

    def test_simple_prop_degen(self):
        vsp.VSPRenew()
        vsp.ClearVSPModel()
        prop_id = vsp.AddGeom("PROP")
        vsp.Update()

        print(vsp.GetAnalysisInputNames("DegenGeom"))
        vsp.SetAnalysisInputDefaults("DegenGeom")
        vsp.PrintAnalysisInputs("DegenGeom")
        vsp.SetIntAnalysisInput("DegenGeom", "WriteCSVFlag", [0], 0)
        vsp.SetIntAnalysisInput("DegenGeom", "WriteMFileFlag", [0], 0)
        vsp.PrintAnalysisInputs("DegenGeom")

        self.assertTrue(True)

    def test_copy_paste_ids(self):
        vsp.VSPRenew()
        vsp.ClearVSPModel()
        prop_id1 = vsp.AddGeom("PROP")
        vsp.Update()

        vsp.CopyGeomToClipboard(prop_id1)
        prop_id2 = vsp.PasteGeomClipboard()

        self.assertTrue(prop_id1 != prop_id2[0])

    def test_parse_degen_geom(self):
        vsp.VSPRenew()
        vsp.ClearVSPModel()
        wing_id = vsp.AddGeom("WING")

        vsp.Update()

        vsp.SetIntAnalysisInput("DegenGeom", "WriteCSVFlag", [0], 0)
        vsp.SetIntAnalysisInput("DegenGeom", "WriteMFileFlag", [0], 0)

        degen_results_id = vsp.ExecAnalysis("DegenGeom")
        for degen_obj in vsp.parse_degen_geom(degen_results_id):
            print(degen_obj)

    def test_parasite_drag(self):
        import numpy as np
        vsp.VSPRenew()
        vsp.ClearVSPModel()
        wing_id = vsp.AddGeom("WING")
        pod_id = vsp.AddGeom("POD")
        sref = vsp.GetParmVal(wing_id, "TotalArea", "WingGeom")

        res = vsp.parasitedrag_sweep(speeds=np.linspace(10, 250, 10), alts_ft=[0, 10000, 20000, 50000], sref=sref,
                                     length_unit=vsp.LEN_FT, speed_unit=vsp.V_UNIT_MPH)

        res.plot()

    def test_surface_patches(self):
        vsp.VSPRenew()
        vsp.ClearVSPModel()
        vsp.AddGeom("PROP")

        vsp.Update()

        components = vsp.export_surface_patches(vsp.SET_ALL, remove_degenerate=True)
        vsp.plot_surface_components(components)

    def test_control_surface_degen_geom(self):
        vsp.VSPRenew()

    def test_degen_transform_mat(self):
        import degen_geom as dg
        vsp.VSPRenew()
        vsp.ClearVSPModel()
        prop_id = vsp.AddGeom("PROP")

        vsp.SetParmVal(prop_id, "Y_Rel_Location", "XForm", 60.0)
        vsp.SetParmVal(prop_id, "Y_Rel_Rotation", "XForm", 90.0)
        vsp.SetParmVal(prop_id, "X_Rel_Rotation", "XForm", 10.0)
        vsp.SetParmVal(prop_id, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ)

        vsp.Update()

        vsp.SetIntAnalysisInput("DegenGeom", "WriteCSVFlag", [0], 0)
        vsp.SetIntAnalysisInput("DegenGeom", "WriteMFileFlag", [0], 0)

        degen_results_id = vsp.ExecAnalysis("DegenGeom")
        degen_objects = vsp.parse_degen_geom(degen_results_id)

        degen_mgr = dg.DegenGeomMgr(degen_objects)
        dg_prop_comp = degen_mgr.degen_objs[prop_id]

        orig_copy = dg_prop_comp.copies[0]

        origin = orig_copy[0].transmat.get_translations()
        self.assertAlmostEqual(origin[0], 0.0, places=6)
        self.assertAlmostEqual(origin[1], 60.0, places=6)
        self.assertAlmostEqual(origin[2], 0.0, places=6)

        angles = orig_copy[0].transmat.get_angles()
        self.assertAlmostEqual(angles[0], 10.0, places=6)
        self.assertAlmostEqual(angles[1], 90.0, places=6)
        self.assertAlmostEqual(angles[2], 0.0, places=6)

        sym_copy = dg_prop_comp.copies[1]
        origin = sym_copy[0].transmat.get_translations()
        self.assertAlmostEqual(origin[0], 0.0, places=6)
        self.assertAlmostEqual(origin[1], -60.0, places=6)
        self.assertAlmostEqual(origin[2], 0.0, places=6)

        angles = sym_copy[0].transmat.get_angles()
        self.assertAlmostEqual(angles[0], -180.0+10.0, places=6)
        self.assertAlmostEqual(angles[1], 90.0, places=6)
        self.assertAlmostEqual(angles[2], 0.0, places=6)

        self.assertTrue(True)

    def test_get_prop_info(self):
        import matplotlib.pyplot as plt
        vsp.VSPRenew()
        vsp.ClearVSPModel()

        prop_id = vsp.AddGeom("PROP")

        valid_location = np.array([-30.0, 60.0, 0.0]).reshape((3, 1))

        vsp.SetParmVal(prop_id, "Y_Rel_Location", "XForm", valid_location[1, 0])
        vsp.SetParmVal(prop_id, "X_Rel_Location", "XForm", valid_location[0, 0])
        vsp.SetParmVal(prop_id, "Y_Rel_Rotation", "XForm", 0.0)
        vsp.SetParmVal(prop_id, "X_Rel_Rotation", "XForm", 0.0)
        vsp.SetParmVal(prop_id, "Sym_Planar_Flag", "Sym", vsp.SYM_XZ | vsp.SYM_YZ)

        valid_location_sym = np.array(valid_location)
        valid_location_sym[1] *= -1.0
        valid_location_aft = np.array(valid_location)
        valid_location_aft[0] *= -1.0
        valid_location_aft_sym = np.array(valid_location_sym)
        valid_location_aft_sym[0] *= -1.0

        vsp.Update()

        prop_info = vsp.get_propeller_thrust_vectors(vsp.SET_ALL)

        # Plot prop objects
        vsp.plot_propeller_info(prop_info, vector_scale=30.0, markersize=5)
        plt.draw()
        plt.show()

        valid_thrust_dir = np.array([-1.0, 0.0, 0.0]).reshape((3, 1))
        valid_thrust_dir_aft_props = np.array([1.0, 0.0, 0.0]).reshape((3, 1))

        # Check thrust vectors
        npt.assert_allclose(prop_info[0].thrust_vector, valid_thrust_dir)
        npt.assert_allclose(prop_info[1].thrust_vector, valid_thrust_dir)
        npt.assert_allclose(prop_info[2].thrust_vector, valid_thrust_dir_aft_props)
        npt.assert_allclose(prop_info[3].thrust_vector, valid_thrust_dir_aft_props)

        # Check origins
        npt.assert_allclose(prop_info[0].hub_center, valid_location)
        npt.assert_allclose(prop_info[1].hub_center, valid_location_sym)
        npt.assert_allclose(prop_info[2].hub_center, valid_location_aft)
        npt.assert_allclose(prop_info[3].hub_center, valid_location_aft_sym)

        # Check rotation directions
        self.assertTrue(prop_info[0].rotation_direction == 1)
        self.assertTrue(prop_info[1].rotation_direction == -1)
        self.assertTrue(prop_info[2].rotation_direction == -1)
        self.assertTrue(prop_info[3].rotation_direction == 1)


if __name__ == "__main__":
    t = TestOpenVSP()
    t.test_parse_degen_geom()
    t.test_degen_geom()
    t.test_parasite_drag()
    t.test_simple_prop_degen()
    t.test_surface_patches()
    t.test_degen_transform_mat()

