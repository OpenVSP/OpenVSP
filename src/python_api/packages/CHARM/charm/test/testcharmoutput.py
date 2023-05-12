# Copyright (c) 2023 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.  All Other
# Rights Reserved.

# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at

#      http://www.apache.org/licenses/LICENSE-2.0

#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#  License for the specific language governing permissions and limitations under
#  the License.

import unittest
import os

path = os.path.dirname(__file__)


class Test(unittest.TestCase):

    def test_parse_perf_file(self):
        import charm.output as charmo

        perffile = os.path.join(path, "test_run", "testperf.dat")

        perfdata = charmo.CharmPerfData(filename=perffile)

        print(perfdata.rotor_metadata)
        print(perfdata.row_data["CL2D"])

    def test_parse_log_file(self):
        import charm.output as charmo

        rotor_results = charmo._parse_log_file(os.path.join(path, "test_run", "test.log"))

        shaft_thrust_total_exp = 1049.41
        shaft_thrust_total = rotor_results[0][1].shaft_thrust_total
        self.assertEqual(shaft_thrust_total_exp, shaft_thrust_total, msg="Rotor shaft frame total thrust mismatch.")

        shaft_power_total_exp = 216.906
        shaft_power_total = rotor_results[0][1].shaft_power_total
        self.assertEqual(shaft_power_total_exp, shaft_power_total, msg="Rotor shaft frame total power mismatch.")

        shaft_h_force_exp = -1.91
        shaft_h_force = rotor_results[0][1].shaft_h_force
        self.assertEqual(shaft_h_force_exp, shaft_h_force, msg="Rotor shaft frame H-force mismatch.")

        shaft_y_force_exp = -0.76
        shaft_y_force = rotor_results[0][1].shaft_y_force
        self.assertEqual(shaft_y_force_exp, shaft_y_force, msg="Rotor shaft frame Y-force mismatch.")

        shaft_roll_mom_exp = 12.431
        shaft_roll_mom = rotor_results[0][1].shaft_roll_mom
        self.assertEqual(shaft_roll_mom_exp, shaft_roll_mom, msg="Rotor shaft frame roll moment mismatch.")

        shaft_pitch_mom_exp = -9.689
        shaft_pitch_mom = rotor_results[0][1].shaft_pitch_mom
        self.assertEqual(shaft_pitch_mom_exp, shaft_pitch_mom, msg="Rotor shaft frame pitch moment mismatch.")

        shaft_yaw_mom_exp = 784.400
        shaft_yaw_mom = rotor_results[0][1].shaft_yaw_mom
        self.assertEqual(shaft_yaw_mom_exp, shaft_yaw_mom, msg="Rotor shaft frame yaw moment mismatch.")

        wind_thrust_total_exp = -56.83
        wind_thrust_total = rotor_results[0][1].wind_thrust_total
        self.assertEqual(wind_thrust_total_exp, wind_thrust_total, msg="Wind frame total thrust mismatch.")

        wind_power_total_exp = 216.91
        wind_power_total = rotor_results[0][1].wind_power_total
        self.assertEqual(wind_power_total_exp, wind_power_total, msg="Wind frame total power mismatch.")

        wind_x_force_exp = -1047.87
        wind_x_force = rotor_results[0][1].wind_x_force
        self.assertEqual(wind_x_force_exp, wind_x_force, msg="Wind frame X-force mismatch.")

        wind_side_force_exp = -0.76
        wind_side_force = rotor_results[0][1].wind_side_force
        self.assertEqual(wind_side_force_exp, wind_side_force, msg="Wind frame side force mismatch.")

        aircraft_roll_mom_exp = -784.400
        aircraft_roll_mom = rotor_results[0][1].aircraft_roll_mom
        self.assertEqual(aircraft_roll_mom_exp, aircraft_roll_mom, msg="Aircraft frame hub roll moment mismatch.")

        aircraft_pitch_mom_exp = -9.689
        aircraft_pitch_mom = rotor_results[0][1].aircraft_pitch_mom
        self.assertEqual(aircraft_pitch_mom_exp, aircraft_pitch_mom, msg="Aircraft frame hub pitch moment mismatch.")

        aircraft_yaw_mom_exp = 12.431
        aircraft_yaw_mom = rotor_results[0][1].aircraft_yaw_mom
        self.assertEqual(aircraft_yaw_mom_exp, aircraft_yaw_mom, msg="Aircraft frame hub yaw moment mismatch.")

        collective_exp = 1.47
        collective = rotor_results[0][1].collective
        self.assertEqual(collective_exp, collective, msg="Rotor collective mismatch.")

        tip_speed_exp = 604.859
        tip_speed = rotor_results[0][1].tip_speed
        self.assertEqual(tip_speed_exp, tip_speed, msg="Rotor tip speed mismatch.")

        fom_exp = 0.0  # Figure of merit not printed in this example log file
        fom = rotor_results[0][1].fom
        self.assertEqual(fom_exp, fom, msg="FOM mismatch.")

        sigma_exp = 0.150784
        sigma = rotor_results[0][1].sigma
        self.assertEqual(sigma_exp, sigma, msg="Rotor solidity mismatch.")

        wind_ideal_ind_power_exp = 101.51
        wind_ideal_ind_power = rotor_results[0][1].wind_ideal_ind_power
        self.assertEqual(wind_ideal_ind_power_exp, wind_ideal_ind_power, msg="Wind frame ideal induced power mismatch.")

        wind_rotor_eff_exp = 0.746
        wind_rotor_eff = rotor_results[0][1].wind_rotor_eff
        self.assertEqual(wind_rotor_eff_exp, wind_rotor_eff, msg="Wind frame rotor efficiency mismatch.")

        aircraft_x_force_exp = -1049.410
        aircraft_x_force = rotor_results[0][1].aircraft_x_force
        self.assertEqual(aircraft_x_force_exp, aircraft_x_force, msg="Aircraft frame hub X-force mismatch.")

        aircraft_y_force_exp = 0.223
        aircraft_y_force = rotor_results[0][1].aircraft_y_force
        self.assertEqual(aircraft_y_force_exp, aircraft_y_force, msg="Aircraft frame hub Y-force mismatch.")

        aircraft_z_force_exp = -2.509
        aircraft_z_force = rotor_results[0][1].aircraft_z_force
        self.assertEqual(aircraft_z_force_exp, aircraft_z_force, msg="Aircraft frame hub Z-force mismatch.")

    def test_parse_charm_run(self):
        import charm.output as charmo

        results = charmo.parse_charm_run(os.path.join(path, "test_run"), "test")

        rotor_eff_wind_exp = 0.746
        rotor_eff_wind = results.rotor_results[0].rotor_eff_wind[-1]
        self.assertEqual(rotor_eff_wind_exp, rotor_eff_wind, msg="Wind frame rotor efficiency mismatch.")

        ideal_ind_power_exp = 101.51
        ideal_ind_power = results.rotor_results[0].ideal_ind_power_wind[-1]
        self.assertEqual(ideal_ind_power_exp, ideal_ind_power, msg="Wind frame ")


if __name__ == "__main__":
    t = Test()
    # t.test_parse_perf_file()
    # t.test_parse_log_file()
    # t.test_parse_charm_run()
    unittest.main(verbosity=2)
