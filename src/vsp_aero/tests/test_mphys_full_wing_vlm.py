import os

import numpy as np
import openmdao.api as om
from mphys.multipoint import Multipoint
from mphys.scenario_aerodynamic import ScenarioAerodynamic

from vspaero.mphys import SteadyBuilder
from openmdao_analysis_base_test import OpenMDAOTestCase

"""
Tests a steady rectangular wing analysis using VLM solver with mphys/openmdao wrapper.
Tests inviscid function values and problem's partial/total sensitivities against FD.
"""

# VSP model
base_dir = os.path.dirname(os.path.abspath(__file__))
vsp_file = os.path.join(base_dir, "./input_files/coarse_rect_wing.vsp3")

class ProblemTest(OpenMDAOTestCase.OpenMDAOTest):
    # Historical reference values for function outputs
    FUNC_REFS = {
        "analysis.CL": 0.49436302,
        "analysis.CD": 0.00631034
    }
    # Inputs to check total sensitivities wrt
    WRT = ["flight_vars.aoa", "flight_vars.mach", "mesh.x_aero0"]

    def setup_problem(self):
        """
        Setup openmdao problem object we will be testing.
        """

        class Top(Multipoint):
            def setup(self):

                # Create mphys builder for aero solver
                aero_builder = SteadyBuilder(vsp_file,
                                             options={"num_wake_nodes": 2,
                                                      "check_partials": True,
                                                      "write_solution": False})
                aero_builder.initialize(self.comm)

                # Create mesh component and connect with solver
                self.add_subsystem("mesh", aero_builder.get_mesh_coordinate_subsystem())

                dvs = self.add_subsystem("flight_vars", om.IndepVarComp())
                dvs.add_output("aoa", 5.0)
                dvs.add_output("mach", 0.3)

                self.mphys_add_scenario("analysis", ScenarioAerodynamic(aero_builder=aero_builder))
                self.connect("mesh.x_aero0", "analysis.x_aero")
                self.connect("flight_vars.aoa", "analysis.aoa")
                self.connect("flight_vars.mach", "analysis.mach")

        prob = om.Problem()
        prob.model = Top()

        return prob
