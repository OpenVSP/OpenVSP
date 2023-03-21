"""
Use Mphys/OpenVSP interface to perform inviscid drag minimization of
an initially rectangular wing with respect to the chord distribution,
subject to a lift and reference area constraint. Similar to the twist optimization,
the expected result from lifting line theory should produce an elliptical lift distrbution.
Check output directory for Tecplot solution files.
"""

import os

import openmdao.api as om
import openvsp
from mphys import Multipoint
from mphys.scenario_aerodynamic import ScenarioAerodynamic
from pygeo.mphys import OM_DVGEOCOMP

from vspaero.mphys import SteadyBuilder


class Planform(om.ExplicitComponent):

    def initialize(self):
        self.options.declare("vsp_file", recordable=False)

    def setup(self):
        # VSP model
        vsp_file = self.options["vsp_file"]
        self.add_input("WingGeom:XSec_1:Root_Chord", val=1)
        self.add_input("WingGeom:XSec_2:Root_Chord", val=1)
        self.add_input("WingGeom:XSec_3:Root_Chord", val=1)
        self.add_input("WingGeom:XSec_4:Root_Chord", val=1)
        self.add_input("WingGeom:XSec_4:Tip_Chord", val=1)
        self.add_output("S_ref", val=1.0)
        self.declare_partials("S_ref", "*", method="fd")
        self.vsp_model = openvsp.VSPVehicle(vsp_file)

    def compute(self, inputs, outputs):
        for input_key in inputs:
            val = inputs[input_key][0]
            geom_name, group_name, parm_name = input_key.split(":")
            container_id = self.vsp_model.FindContainer(geom_name, 0)
            self.vsp_model.SetParmValUpdate(container_id, parm_name, group_name, val)
        self.vsp_model.Update()
        parm_id = self.vsp_model.FindParm(container_id, "TotalArea", geom_name)
        outputs["S_ref"] = self.vsp_model.GetParmVal(parm_id)


class Top(Multipoint):
    def setup(self):
        # VSP model
        vsp_file = os.path.join(os.path.dirname(__file__), "rect_wing.vsp3")

        # Create mphys builder for aero solver
        aero_builder = SteadyBuilder(vsp_file, options={"symmetry": "Y"})
        aero_builder.initialize(self.comm)

        # Create mesh component and connect with solver
        self.add_subsystem("mesh", aero_builder.get_mesh_coordinate_subsystem())

        # add the geometry component, we don't need a builder because we do it here.
        self.add_subsystem("geometry", OM_DVGEOCOMP(file=vsp_file, type="vsp"), promotes_inputs=["WingGeom*"])
        # add pointset
        self.geometry.nom_add_discipline_coords("aero")

        self.add_subsystem("planform", Planform(vsp_file=vsp_file), promotes_inputs=['*'])
        self.connect("planform.S_ref", "cruise.S_ref")

        self.mphys_add_scenario("cruise", ScenarioAerodynamic(aero_builder=aero_builder))
        self.connect("mesh.x_aero0", "geometry.x_aero_in")
        self.connect("geometry.x_aero0", "cruise.x_aero")

    def configure(self):
        # create geometric DV setup
        self.geometry.nom_addVSPVariable("WingGeom", "XSec_1", "Root_Chord", scaledStep=False)
        self.geometry.nom_addVSPVariable("WingGeom", "XSec_2", "Root_Chord", scaledStep=False)
        self.geometry.nom_addVSPVariable("WingGeom", "XSec_3", "Root_Chord", scaledStep=False)
        self.geometry.nom_addVSPVariable("WingGeom", "XSec_4", "Root_Chord", scaledStep=False)
        self.geometry.nom_addVSPVariable("WingGeom", "XSec_4", "Tip_Chord", scaledStep=False)


prob = om.Problem()
prob.model = Top()

# Set optimizer as model driver
prob.driver = om.pyOptSparseDriver(debug_print=['objs', 'nl_cons', "desvars"])
prob.driver.options['optimizer'] = "SNOPT"
prob.driver.opt_settings['Major iterations limit'] = 200
prob.driver.opt_settings["Nonderivative linesearch"] = None
prob.driver.opt_settings["Major optimality tolerance"] = 1e-4
prob.driver.opt_settings["Major feasibility tolerance"] = 1e-5
prob.driver.opt_settings["Hessian updates"] = 10

# Setup problem and add design variables, constraint, and objective
prob.model.add_design_var("WingGeom:XSec_1:Root_Chord", lower=1e-3, upper=5.0)
prob.model.add_design_var("WingGeom:XSec_2:Root_Chord", lower=1e-3, upper=5.0)
prob.model.add_design_var("WingGeom:XSec_3:Root_Chord", lower=1e-3, upper=5.0)
prob.model.add_design_var("WingGeom:XSec_4:Root_Chord", lower=1e-3, upper=5.0)
prob.model.add_design_var("WingGeom:XSec_4:Tip_Chord", lower=1e-3, upper=5.0)
prob.model.add_design_var("cruise.aoa", lower=-10.0, upper=10.0)
prob.model.add_constraint("cruise.CL", equals=0.5)
prob.model.add_constraint("planform.S_ref", equals=10.0)
prob.model.add_objective("cruise.CD", scaler=1e4)

prob.setup()

# Create a n^2 diagram for user to view model connections
#om.n2(prob)
prob["cruise.aoa"] = 5.0
prob.run_model()
print("CL", prob["cruise.CL"][0])
print("CD", prob["cruise.CD"][0])
prob.check_totals(["cruise.CD", "planform.S_ref"], ["cruise.aoa", "WingGeom:XSec_3:Root_Chord"])

#prob.run_driver()
# Write optimized geometry to vsp file
#prob.model.geometry.DVGeo.writeVSPFile("opt_chord.vsp3")


