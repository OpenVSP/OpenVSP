import os
import time
import openmdao.api as om
from pygeo.mphys import OM_DVGEOCOMP
import openvsp

from vspaero.pyvspaero import pyVSPAero
from vspaero import functions

# Setup VLM assembler
vsp_file = os.path.join(os.path.dirname(__file__), "./rect_wing.vsp3")

class FlightCase(om.ExplicitComponent):
    def initialize(self):
        self.options.declare("vsp_file", recordable=False)

    def setup(self):
        # VSP model
        vsp_file = self.options["vsp_file"]

        VLMAssembler = pyVSPAero(vsp_file)
        # Add output functions
        VLMAssembler.add_function("CL", functions.CLi)
        VLMAssembler.add_function("CD", functions.CDi)
        self.assembler = VLMAssembler

        # Create a steady problem instance
        self.problem = VLMAssembler.create_steady_problem(self.name, options={"symmetry": "Y"})

        mesh_xyz = self.problem.get_geometry()

        self.add_input("aoa", val=5.0)
        self.add_input("S_ref", val=10.0)
        self.add_input("x_aero", val=mesh_xyz, shape=mesh_xyz.shape)
        for out_name in self.problem.func_list:
            self.add_output(out_name, val=1.0)

        self.declare_partials("*", "*")

    def get_mesh(self):
        return self.problem.get_geometry()

    def compute(self, inputs, outputs):
        self.problem.set_flight_vars(**inputs)
        self.problem.set_geometry(inputs["x_aero"])

        self.problem.solve()

        funcs = {}
        self.problem.eval_functions(funcs)

        for out_name in outputs:
            output_key = f"{self.name}_{out_name}"
            outputs[out_name] = funcs[output_key]

        self.problem.write_solution(output_dir="./output")

    def compute_partials(self, inputs, J):
        """ Jacobian of partial derivatives."""

        self.problem.set_flight_vars(**inputs)
        self.problem.set_geometry(inputs["x_aero"])

        funcs_sens = {}
        self.problem.eval_functions_sens(funcs_sens)

        for out_name in self.problem.func_list:
            for in_name in inputs:
                output_key = f"{self.name}_{out_name}"
                if in_name == "x_aero":
                    J[out_name, in_name] = funcs_sens[output_key]["xyz"]
                else:
                    J[out_name, in_name] = funcs_sens[output_key][in_name]

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


class Top(om.Group):
    def setup(self):

        cruise_component = FlightCase(vsp_file=vsp_file)

        self.add_subsystem("initial_mesh", om.IndepVarComp())

        # add the geometry component, we dont need a builder because we do it here.
        self.add_subsystem("geometry", OM_DVGEOCOMP(file=vsp_file, type="vsp"), promotes_inputs=["WingGeom*"])
        # add pointset
        self.geometry.nom_add_discipline_coords("aero")

        self.add_subsystem("planform", Planform(vsp_file=vsp_file), promotes_inputs=['*'])
        self.connect("planform.S_ref", "cruise.S_ref")

        self.add_subsystem("cruise", cruise_component)
        self.connect("geometry.x_aero0", "cruise.x_aero")

    def configure(self):
        self.initial_mesh.add_output("x_aero0", self.cruise.get_mesh())
        self.connect("initial_mesh.x_aero0", "geometry.x_aero_in")

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
prob.model.add_constraint("planform.S_ref", equals=10.0, scaler=0.1)
prob.model.add_objective("cruise.CD", scaler=1e4)

prob.setup()

# Create a n^2 diagram for user to view model connections
om.n2(prob, show_browser=False)
prob.run_model()
print("CL", prob["cruise.CL"][0])
print("CD", prob["cruise.CD"][0])

#prob.check_totals()
#sens = prob.check_partials(excludes=["planform"], step=1e-6, form="central")

#xyz = prob["cruise.x_aero"]
#print(xyz)

prob.run_driver()