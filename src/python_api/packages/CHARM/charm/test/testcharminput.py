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
    unittest.main(verbosity=2)
