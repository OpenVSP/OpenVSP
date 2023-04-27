"""
Class definition for the Mphys builder for the aero solver.
"""

import copy
import numpy as np
import openmdao.api as om

try:
    from mphys.builder import Builder
    from mphys.distributed_converter import DistributedConverter, DistributedVariableDescription
except ImportError:
    pass

from .. import pyVSPAero
from vspaero.problems.base import wrt_dict
from ..problems import SteadyProblem
from .. import functions

FLIGHT_VARS_NAME = list(wrt_dict.keys())


class AeroMesh(om.IndepVarComp):
    """
    Component to read the initial mesh coordinates with OAS.
    Only the root will be responsible for this information.
    The mesh will be broadcasted to all other processors in a following step.
    """

    def initialize(self):
        self.options.declare("problem", default=None, desc="pyvspaero steady problem", types=SteadyProblem,
                             recordable=False)
        self.options.declare("init_geometry", desc="initial node locations", recordable=False)

    def setup(self):
        if self.comm.rank == 0:
            self.problem = self.options["problem"]
            xpts = self.options["init_geometry"]
        else:
            xpts = np.zeros(0)
        self.add_output(
            "x_aero0",
            distributed=True,
            val=xpts,
            shape=xpts.size,
            units="m",
            desc="aero node coordinates",
            tags=["mphys_coordinates"],
        )


class AeroSolverComponent(om.ImplicitComponent):
    """
    Component to perform VSPAero steady analysis

    Assumptions:
        - The VSPAero steady residual is R = AIC * gamma - rhs = 0

    """

    def initialize(self):
        self.options.declare("problem", default=None, desc="pyvspaero steady problem", types=SteadyProblem,
                             recordable=False)
        self.options.declare("init_flight_vars", recordable=False)
        self.options.declare("check_partials", default=False)

    def setup(self):
        self.problem = self.options["problem"]
        init_vals = self.options["init_flight_vars"]
        self.check_partials = self.options["check_partials"]

        # OpenMDAO setup
        self.nnodes = self.problem.get_num_nodes()
        self.nloops = self.problem.get_num_loops()

        # inputs
        # Flight vars
        for var_name in FLIGHT_VARS_NAME:
            self.add_input(
                var_name,
                val=init_vals[f"{self.problem.name}_{var_name}"],
                distributed=False,
                shape=1,
                desc="vspaero flight variables",
                tags=["mphys_coupling"],
            )
        # node locations
        self.add_input(
            "x_aero",
            distributed=False,
            shape=self.nnodes * 3,
            units="m",
            desc="flattened aero mesh coordinates for all oas surfaces",
            tags=["mphys_coupling"],
        )

        # outputs
        self.add_output(
            "circulations",
            distributed=False,
            shape=self.nloops,
            desc="aerodynamic state vector",
            tags=["mphys_coupling"],
        )

    def _update_internal(self, inputs, outputs=None):
        self.problem.set_flight_vars(**inputs)
        self.problem.set_geometry(inputs["x_aero"])
        if outputs is not None:
            self.problem.set_states(outputs["circulations"])

    def apply_nonlinear(self, inputs, outputs, residuals):
        self._update_internal(inputs, outputs)
        residuals["circulations"] = self.problem.get_residual()

    def solve_nonlinear(self, inputs, outputs):
        self._update_internal(inputs)
        self.problem.solve()
        outputs["circulations"] = self.problem.get_states()

    def solve_linear(self, d_outputs, d_residuals, mode):
        if mode == "fwd":
            if self.check_partials:
                print("solver fwd")
            else:
                raise ValueError("forward mode requested but not implemented")

        if mode == "rev":
            nloops = self.problem.get_num_loops()
            neqs = self.problem.get_num_adjoint_eqs()
            rhs = np.zeros(neqs)
            rhs[:nloops] = d_outputs["circulations"]
            d_residuals["circulations"] = self.problem.solve_adjoint_for_rhs(rhs)[:nloops]

    def apply_linear(self, inputs, outputs, d_inputs, d_outputs, d_residuals, mode):
        self._update_internal(inputs, outputs)
        if mode == "fwd":
            if not self.check_partials:
                raise ValueError("forward mode requested but not implemented")

        if mode == "rev":
            nloops = self.problem.get_num_loops()
            neqs = self.problem.get_num_adjoint_eqs()

            if "circulations" in d_residuals:
                psi = np.zeros(neqs)
                psi[:nloops] = d_residuals["circulations"]

                # Compute Jac^T * psi
                if "circulations" in d_outputs:
                    d_outputs["circulations"] = self.problem.eval_trans_jac_vec_product(psi)[:nloops]

                # Check if any flight vars are included in d_inputs
                include_flight_vars = False
                for var_name in FLIGHT_VARS_NAME:
                    if var_name in d_inputs:
                        include_flight_vars = True
                        break

                # Compute psi * pRpx
                if "x_aero" in d_inputs or include_flight_vars:
                    sens_dict = self.problem.calculate_adjoint_residual_partial_products(psi)

                    if "x_aero" in d_inputs:
                        d_inputs["x_aero"] += sens_dict["xyz"]
                    if include_flight_vars:
                        for var_name in FLIGHT_VARS_NAME:
                            if var_name in d_inputs:
                                d_inputs[var_name] += sens_dict[var_name]


class VLMForces(om.ExplicitComponent):
    """
    Component to compute VSPAero nodal forces
    """

    def initialize(self):
        self.options.declare("problem", default=None, desc="pyvspaero steady problem", types=SteadyProblem,
                             recordable=False)
        self.options.declare("init_flight_vars", recordable=False)
        self.options.declare("init_geometry", desc="initial node locations", recordable=False)
        self.options.declare("check_partials")

        self.problem = None

        self.check_partials = False

    def setup(self):
        self.problem = self.options["problem"]
        init_vals = self.options["init_flight_vars"]

        self.check_partials = self.options["check_partials"]

        # OpenMDAO setup
        self.nnodes = self.problem.get_num_nodes()
        self.nloops = self.problem.get_num_loops()

        # inputs
        # Flight vars
        for var_name in FLIGHT_VARS_NAME:
            self.add_input(
                var_name,
                val=init_vals[f"{self.problem.name}_{var_name}"],
                distributed=False,
                shape=1,
                desc="vspaero flight variables",
                tags=["mphys_coupling"],
            )
        # node locations
        self.add_input(
            "x_aero",
            distributed=False,
            shape=self.nnodes * 3,
            units="m",
            desc="flattened aero mesh coordinates for all vlm surfaces",
            tags=["mphys_coupling"],
        )

        # Circulation state variable
        self.add_input(
            "circulations",
            distributed=False,
            shape=self.nloops,
            desc="vlm state vector",
            tags=["mphys_coupling"],
        )

        # Add user-defined problem outputs
        self.add_output("f_aero",
                        distributed=False,
                        shape=self.nnodes * 3,
                        desc="vlm nodal force vector",
                        tags=["mphys_coupling"]
        )

        self.symm_flag = self.problem.get_option("Symmetry").lower()
        init_geom = self.options["init_geometry"].reshape(self.nnodes, 3)

        if self.symm_flag == "x":
            self.symm_nodes = np.abs(init_geom[:, 0]) <= 1e-15
        elif self.symm_flag == "y":
            self.symm_nodes = np.abs(init_geom[:, 1]) <= 1e-15
        elif self.symm_flag == "z":
            self.symm_nodes = np.abs(init_geom[:, 2]) <= 1e-15
        else: # self.sym_flag = 'n'
            self.symm_nodes = None

    def _update_internal(self, inputs):
        self.problem.set_flight_vars(**inputs)
        self.problem.set_geometry(inputs["x_aero"])
        self.problem.set_states(inputs["circulations"])

    def compute(self, inputs, outputs):
        self._update_internal(inputs)

        # Evaluate forces
        f_aero = self.problem.get_nodal_forces()
        f_aero = f_aero.reshape(self.nnodes, 3)

        # Zero out the normal forces at symmetry plane
        if self.symm_flag == "x":
            f_aero[self.symm_nodes, 0] = 0.0
        elif self.symm_flag == "y":
            f_aero[self.symm_nodes, 1] = 0.0
        elif self.symm_flag == "z":
            f_aero[self.symm_nodes, 2] = 0.0

        outputs["f_aero"] = f_aero.flatten()

    def compute_jacvec_product(self, inputs, d_inputs, d_outputs, mode):
        if mode == "fwd":
            if not self.check_partials:
                raise ValueError("VSPAero forward mode requested but not implemented")
        if mode == "rev":
            # always update internal because multiple scenarios could use the same problem object,
            # and we need to load this scenario's state back into the problem before doing derivatives
            self._update_internal(inputs)

            # Check if any flight vars are included in d_inputs
            include_flight_vars = False
            for var_name in FLIGHT_VARS_NAME:
                if var_name in d_inputs:
                    include_flight_vars = True
                    break

            if "f_aero" in d_outputs:
                d_func = d_outputs["f_aero"].copy()
                d_func = d_func.reshape(self.nnodes, 3)

                # Zero out the normal forces at symmetry plane
                if self.symm_flag == "x":
                    d_func[self.symm_nodes, 0] = 0.0
                elif self.symm_flag == "y":
                    d_func[self.symm_nodes, 1] = 0.0
                elif self.symm_flag == "z":
                    d_func[self.symm_nodes, 2] = 0.0

                d_func = d_func.flatten()

                # Compute function partials
                if "x_aero" in d_inputs or "circulations" in d_inputs or include_flight_vars:
                    sens_dict = self.problem.calculate_nodal_force_partial_products(d_func)

                    if "x_aero" in d_inputs:
                        d_inputs["x_aero"] += sens_dict["xyz"]
                    if include_flight_vars:
                        for var_name in FLIGHT_VARS_NAME:
                            if var_name in d_inputs:
                                d_inputs[var_name] += sens_dict[var_name]

                    if "circulations" in d_inputs:
                        nloops = self.problem.get_num_loops()
                        d_inputs["circulations"][:] += sens_dict["gamma"][:nloops]

class AeroCouplingGroup(om.Group):
    """
    Group that wraps the aerodynamic states into the Mphys's broader coupling group.

    This is done in four steps:

        1. The deformed aero coordinates are read in as a distributed flattened array.

        2. The VLM problem is then solved based on the deformed mesh.

        3. The serial force vector is converted to a distributed array and
        provided as output to the rest of the Mphys coupling groups.
    """

    def initialize(self):
        self.options.declare("problem", default=None, desc="pyvspaero steady problem", types=SteadyProblem,
                             recordable=False)
        self.options.declare("init_flight_vars", recordable=False)
        self.options.declare("check_partials", default=False)
        self.options.declare("init_geometry", desc="initial node locations", recordable=False)

    def setup(self):
        self.problem = self.options["problem"]

        nnodes = self.problem.get_num_nodes()

        # Convert distributed mphys mesh input into a serial vector VSPAero can use
        vars = [DistributedVariableDescription(name="x_aero", shape=(nnodes * 3), tags=["mphys_coordinates"])]

        self.add_subsystem("collector", DistributedConverter(distributed_inputs=vars), promotes_inputs=["x_aero"])
        self.connect("collector.x_aero_serial", ["states.x_aero", "forces.x_aero"])

        prom_in = FLIGHT_VARS_NAME.copy()
        idx = prom_in.index("vref")
        prom_in[idx] = ("vref", "vinf")

        # VLM aero states component
        self.add_subsystem(
            "states",
            AeroSolverComponent(problem=self.problem,
                                init_flight_vars=self.options["init_flight_vars"],
                                check_partials=self.options["check_partials"]),
            promotes_inputs=prom_in,
            promotes_outputs=["*"],
        )

        # VLM aero forces component
        self.add_subsystem(
            "forces",
            VLMForces(problem=self.problem,
                      init_flight_vars=self.options["init_flight_vars"],
                      init_geometry=self.options["init_geometry"],
                      check_partials=self.options["check_partials"]),
            promotes_inputs=["circulations"] + prom_in,
        )

        # Convert serial force vector to distributed, like mphys expects
        vars = [DistributedVariableDescription(name="f_aero", shape=(nnodes * 3), tags=["mphys_coupling"])]

        self.add_subsystem("distributor", DistributedConverter(distributed_outputs=vars), promotes_outputs=["f_aero"])
        self.connect("forces.f_aero", "distributor.f_aero_serial")


class AeroFuncsGroup(om.Group):
    """
    Group to contain the total aerodynamic performance functions
    to be evaluated after the coupled states are solved.
    """

    def initialize(self):
        self.options.declare("problem", default=None, desc="pyvspaero steady problem", types=SteadyProblem,
                             recordable=False)
        self.options.declare("init_flight_vars", recordable=False)
        self.options.declare("write_solution", default=True)
        self.options.declare("check_partials", default=False)
        self.options.declare("output_dir")
        self.options.declare("scenario_name", default=None)

    def setup(self):
        self.problem = self.options["problem"]

        nnodes = self.problem.get_num_nodes()

        # Convert distributed mphys mesh input into a serial vector VSPAero can use
        vars = [DistributedVariableDescription(name="x_aero", shape=(nnodes * 3), tags=["mphys_coordinates"])]

        self.add_subsystem("collector", DistributedConverter(distributed_inputs=vars), promotes_inputs=["x_aero"])
        self.connect("collector.x_aero_serial", "functions.x_aero")

        prom_in = ["circulations"] + FLIGHT_VARS_NAME
        idx = prom_in.index("vref")
        prom_in[idx] = ("vref", "vinf")

        # VLM aero functions component
        self.add_subsystem(
            "functions",
            VLMFunctions(problem=self.problem,
                         init_flight_vars=self.options["init_flight_vars"],
                         check_partials=self.options["check_partials"],
                         write_solution=self.options["write_solution"],
                         output_dir=self.options["output_dir"],
                         scenario_name=self.options["scenario_name"]),
            promotes_inputs=prom_in,
            promotes_outputs=["*"],
        )


class VLMFunctions(om.ExplicitComponent):
    """
    Component to compute VSPAero functions
    """

    def initialize(self):
        self.options.declare("problem", default=None, desc="pyvspaero steady problem", types=SteadyProblem,
                             recordable=False)
        self.options.declare("init_flight_vars", recordable=False)
        self.options.declare("check_partials", default=False)
        self.options.declare("scenario_name", default=None)
        self.options.declare("write_solution")
        self.options.declare("output_dir")

        self.problem = None

        self.check_partials = False

    def setup(self):
        self.problem = self.options["problem"]
        init_vals = self.options["init_flight_vars"]
        self.check_partials = self.options["check_partials"]
        self.write_solution = self.options["write_solution"]
        self.solution_counter = 0

        # OpenMDAO setup
        self.nnodes = self.problem.get_num_nodes()
        self.nloops = self.problem.get_num_loops()

        # inputs
        # Flight vars
        for var_name in FLIGHT_VARS_NAME:
            self.add_input(
                var_name,
                val=init_vals[f"{self.problem.name}_{var_name}"],
                distributed=False,
                shape=1,
                desc="vspaero flight variables",
                tags=["mphys_coupling"],
            )
        # node locations
        self.add_input(
            "x_aero",
            distributed=False,
            shape=self.nnodes * 3,
            units="m",
            desc="flattened aero mesh coordinates for all vlm surfaces",
            tags=["mphys_coupling"],
        )

        # Circulation state variable
        self.add_input(
            "circulations",
            distributed=False,
            shape=self.nloops,
            desc="vlm state vector",
            tags=["mphys_coupling"],
        )

        # Add user-defined problem outputs
        func_names = self.problem.get_function_names()
        for func_name in func_names:
            self.add_output(
                func_name, distributed=False, shape=1, tags=["mphys_result"]
            )

    def _update_internal(self, inputs):
        self.problem.set_flight_vars(**inputs)
        self.problem.set_geometry(inputs["x_aero"])
        self.problem.set_states(inputs["circulations"])

    def compute(self, inputs, outputs):
        self._update_internal(inputs)

        # Evaluate functions
        funcs = {}
        self.problem.eval_functions(funcs, eval_funcs=outputs.keys())
        for func_name in outputs:
            # Add struct problem name from key
            key = self.problem.name + "_" + func_name
            outputs[func_name] = funcs[key]

        if self.write_solution:
            # write the solution files.
            self.problem.write_solution(output_dir=self.options["output_dir"],
                                        base_name=self.options["scenario_name"],
                                        number=self.solution_counter)
            self.solution_counter += 1

    def compute_jacvec_product(self, inputs, d_inputs, d_outputs, mode):
        if mode == "fwd":
            if not self.check_partials:
                raise ValueError("VSPAero forward mode requested but not implemented")
        if mode == "rev":
            # always update internal because multiple scenarios could use the same problem object,
            # and we need to load this scenario's state back into the problem before doing derivatives
            self._update_internal(inputs)

            # Check if any flight vars are included in d_inputs
            include_flight_vars = False
            for var_name in FLIGHT_VARS_NAME:
                if var_name in d_inputs:
                    include_flight_vars = True
                    break

            for func_name in d_outputs:
                d_func = d_outputs[func_name]
                func_key = f"{self.problem.name}_{func_name}"

                # Compute function partials
                if "x_aero" in d_inputs or "circulations" in d_inputs or include_flight_vars:
                    sens_dict = {}
                    self.problem.eval_function_partials(sens_dict, eval_funcs=func_name)

                    if "x_aero" in d_inputs:
                        d_inputs["x_aero"] += d_func * sens_dict[func_key]["xyz"]
                    if include_flight_vars:
                        for var_name in FLIGHT_VARS_NAME:
                            if var_name in d_inputs:
                                d_inputs[var_name] += d_func * sens_dict[func_key][var_name]

                    if "circulations" in d_inputs:
                        nloops = self.problem.get_num_loops()
                        d_inputs["circulations"][:] += d_func * sens_dict[func_key]["gamma"][:nloops]


class SteadyBuilder(Builder):
    """
    Mphys builder class responsible for setting up components of VSPAero's aerodynamic solver.
    """

    def_options = {"output_dir": "./", "write_solution": True, "check_partials": False}

    def __init__(self, vsp_file, comps=None, options=None):
        self.vsp_file = vsp_file
        # VSP component names to include in analysis, defaults to all
        self.comps = comps
        # Copy default options
        self.options = copy.deepcopy(self.def_options)
        # Update with user-defined options
        if options:
            self.options.update(options)

    def initialize(self, comm):
        self.comm = comm

        steady_options = copy.deepcopy(self.options)

        # Load optional user-defined callback function for setting up output functions (CL, CD, etc.)
        if "function_setup" in steady_options:
            function_setup = steady_options.pop("function_setup")
        else:
            function_setup = None

        self.check_partials = steady_options.pop("check_partials")

        # Create pyvspaero instance
        self.vlm_assembler = pyVSPAero(self.vsp_file, comps=self.comps)

        # Do any pre-initialize setup requested by user
        if function_setup is not None:
            function_setup(self.vlm_assembler)
        else:
            self.vlm_assembler.add_function("CL", functions.CLi)
            self.vlm_assembler.add_function("CD", functions.CDi)

        # Set up elements and pyvspaero assembler
        self.problem = self.vlm_assembler.create_steady_problem("steady_analysis", options=steady_options)

        # Save a copy of the initial flight vars and geometry
        self.dv0 = self.problem.get_flight_vars()
        self.xyz0 = self.problem.get_geometry()

    def get_coupling_group_subsystem(self, scenario_name=None):
        return AeroCouplingGroup(problem=self.problem, init_flight_vars=self.dv0, init_geometry=self.xyz0,
                                 check_partials=self.check_partials)

    def get_mesh_coordinate_subsystem(self, scenario_name=None):
        return AeroMesh(problem=self.problem, init_geometry=self.xyz0)

    def get_post_coupling_subsystem(self, scenario_name=None):
        return AeroFuncsGroup(
            problem=self.problem,
            init_flight_vars=self.dv0,
            write_solution=self.options["write_solution"],
            output_dir=self.options["output_dir"],
            scenario_name=scenario_name,
            check_partials=self.check_partials
        )

    def get_ndof(self):
        """
        Tells Mphys this is a 3D problem.
        """
        return 3

    def get_number_of_nodes(self):
        """
        Get the number of nodes on root proc
        """
        if self.comm.rank == 0:
            return self.problem.get_num_nodes()
        return 0
