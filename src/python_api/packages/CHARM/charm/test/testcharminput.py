import unittest
import openvsp as vsp
import os

path = os.path.dirname(__file__)


class Test(unittest.TestCase):

    def test_create_default_wing_settings(self):
        import charm.input_automation as charm

        dg_mgr, _, _ = create_geom()
        rw_filename = "test_rw.inp"
        rw_file = os.path.join(path, rw_filename)

        wing_settings = charm.build_default_wing_settings(dg_mgr, default_template=rw_file)
        print(wing_settings)
        self.assertTrue(True)

    def test_create_default_rotor_settings(self):
        import charm.input_automation as charm
        dg_mgr, _, _ = create_geom()
        rw_filename = "test_rw.inp"
        rw_file = os.path.join(path, rw_filename)

        rotor_settings = charm.build_default_rotor_settings(dg_mgr, default_rpm=300.0, default_template=rw_file)
        print(rotor_settings.rpm)
        print(rotor_settings.rotor_wake_template)

    def test_create_charm_blade_geom_file_from_propeller(self):
        import charm.input_automation as charm
        dg_mgr, _, rotor_id = create_geom()
        blade_geom = charm.create_charm_blade_geom_file_from_propeller(dg_mgr.degen_objs[rotor_id])
        print(blade_geom)
        self.assertTrue(True)

    def test_build_default_trim_settings(self):
        import charm.input_automation as charm
        import numpy as np

        trim_settings = charm.build_default_trim_settings(trim_template=os.path.join(path, "test_trim.inp"))

        expFsMs = [1, 1, 1, 1, 1, 1]
        self.assertEqual(expFsMs, trim_settings.forces_moments)

        expTargets = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        self.assertEqual(expTargets, trim_settings.targets)

        expTol = [.0002, .0002, .0002, .0002, .0002, .0002]
        self.assertEqual(expTol, trim_settings.tol)

        expPilotCntrls = [1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0]
        self.assertEqual(expPilotCntrls, trim_settings.pilot_cntrls)

        expAttitudeCntls = [1, 1, 0]
        self.assertEqual(expAttitudeCntls, trim_settings.attitude_cntrls)

        expNumComp = 2
        self.assertEqual(expNumComp, trim_settings.num_aero_comps)

        expAeroCompCntrls = np.array([[1, 0, 1, 2, 3, 0, 0, 0, 0, 0],
                                      [2, 0, 4, 0, 0, 0, 0, 0, 0, 0]])
        self.assertSequenceEqual(expAeroCompCntrls.tolist(), trim_settings.aero_comp_cntrls.tolist())

        expCntrlGains = np.array([[1, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
                                  [2, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
                                  [3, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
                                  [4, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]])
        self.assertSequenceEqual(expCntrlGains.tolist(), trim_settings.cntrl_gains.tolist())

        expInitValues = [20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        self.assertEqual(expInitValues, trim_settings.init_cntrls)

# TODO: Need to create test that doesn't include trim inputs (uses test_rc.inp instead of test_rctrim.inp)
    def test_read_run_characteristics_template(self):
        import charm.input_automation as charm

        template_filename = os.path.join(path, "test_rctrim.inp")
        test_run_char_dict = charm.read_run_characteristics_template(template_filename=template_filename)

        expNPSI = 24
        self.assertEqual(expNPSI, test_run_char_dict['NPSI'])

        expNROTOR = 2
        self.assertEqual(expNROTOR, test_run_char_dict['NROTOR'])

        expCG = [1.0, 2.0, 3.0]
        self.assertEqual(expCG, [test_run_char_dict["XCG"],
                                 test_run_char_dict["YCG"],
                                 test_run_char_dict["ZCG"]])


def create_geom():
    vsp.ClearVSPModel()
    vsp.DeleteAllResults()

    wing_id = vsp.AddGeom("WING")
    rotor_id = vsp.AddGeom("PROP")
    vsp.SetParmVal(wing_id, "X_Rel_Location", "XForm", 10.0)
    vsp.Update()

    dg_mgr = vsp.run_degen_geom(set_index=vsp.SET_ALL)

    vsp.WriteVSPFile(os.path.join(path, "testfile.vsp3"))

    return dg_mgr, wing_id, rotor_id


if __name__ == "__main__":
    t = Test()
    # t.test_create_default_wing_settings()
    # t.test_create_default_rotor_settings()
    # t.test_create_charm_blade_geom_file_from_propeller()
    # t.test_build_default_trim_settings()
    # t.test_read_run_characteristics_template()
    unittest.main(verbosity=2)
