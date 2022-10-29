"""
pyBase_problem
"""

# =============================================================================
# Imports
# =============================================================================
import os
import shutil
from ..optimizer import pyVSPOptimizer
from ..utilities import BaseUI
import tempfile
import vspaero.functions

wrt_dict = {"aoa": vspaero.functions.WRT_ALPHA,
            "beta": vspaero.functions.WRT_BETA,
            "mach": vspaero.functions.WRT_MACH,
            "vinf": vspaero.functions.WRT_VINF,
            "rho": vspaero.functions.WRT_DENSITY,
            "recref": vspaero.functions.WRT_RECREF}

class BaseProblem(BaseUI):
    """
    Base class for VSPAero problem types. Contains methods common to all VSPAero problems.
    """

    def __init__(self, openvsp_model, func_list, solver_type, options=None):

        # OpenVSP model object
        self.openvsp_model = openvsp_model
        # VSPAero "optimizer" class
        self.assembler = pyVSPOptimizer()
        # Set flag indicating if this is a steady/unsteady problem
        self.assembler.set_unsteady_analysis(self.is_unsteady)
        # VLM or PANEL
        self.solver_type = solver_type
        # Create a temp directory to hold files to initialize VSPAero
        self.tmpdir = tempfile.TemporaryDirectory()

        self.func_list = func_list
        noutputs = 0
        for func_idx, func in enumerate(func_list):
            func_id = func_list[func]
            self.assembler.set_optimization_function(func_idx, func_id)
            noutputs += 1
            self.assembler.set_number_of_optimization_functions(noutputs)

        # Set default options
        self.set_options_to_default()
        # Update w/ user-defined options
        if options:
            for key in options:
                self.set_option(key, options[key])

        self.sref = 1.
        self.cref = 1.
        self.bref = 1.
        self.x_cg = 0.0
        self.y_cg = 0.0
        self.z_cg = 0.0
        self.mach = 1e-16
        self.aoa = 0.0
        self.beta = 0.0
        self.vinf = 1.0
        self.rho = 1.0
        self.recref = 1000000.0
        self.clmax = -1.0

        self.call_counter = -1

        return

    def set_flight_vars(self, **kwargs):
        flight_vars = kwargs
        for var in flight_vars:
            if "sref" == var.lower() or var == f"{self.name}_Sref":
                self.sref = flight_vars[var]
                self.assembler.set_sref(self.sref)
            if "cref" == var.lower() or var == f"{self.name}_cref":
                self.cref = flight_vars[var]
                self.assembler.set_cref(self.cref)
            if "bref" == var.lower() or var == f"{self.name}_bref":
                self.bref = flight_vars[var]
                self.assembler.set_bref(self.bref)
            if "x_cg" == var.lower() or var == f"{self.name}_x_cg":
                self.x_cg = flight_vars[var]
                self.assembler.set_x_cg(self.x_cg)
            if "y_cg" == var.lower() or var == f"{self.name}_y_cg":
                self.y_cg = flight_vars[var]
                self.assembler.set_y_cg(self.y_cg)
            if "z_cg" == var.lower() or var == f"{self.name}_z_cg":
                self.z_cg = flight_vars[var]
                self.assembler.set_z_cg(self.z_cg)
            if "mach" == var.lower() or var == f"{self.name}_mach":
                self.mach = flight_vars[var]
                self.assembler.set_mach_number(self.mach)
            if "aoa" == var.lower() or var == f"{self.name}_aoa":
                self.aoa = flight_vars[var]
                self.assembler.set_aoa(self.aoa)
            if "beta" == var.lower() or var == f"{self.name}_beta":
                self.beta = flight_vars[var]
                self.assembler.set_beta(self.beta)
            if "vinf" == var.lower() or var == f"{self.name}_vinf":
                self.vinf = flight_vars[var]
                self.assembler.set_vinf(self.vinf)
            if "rho" == var.lower() or var == f"{self.name}_rho":
                self.rho = flight_vars[var]
                self.assembler.set_density(self.rho)
            if "recref" == var.lower() or var == f"{self.name}_recref":
                self.recref = flight_vars[var]
                self.assembler.set_recref(self.recref)
            if "clmax" == var.lower() or var == f"{self.name}_clmax":
                self.clmax = flight_vars[var]
                self.assembler.set_clmax(self.clmax)

    def get_flight_vars(self, flight_vars=None):
        if flight_vars is None or isinstance(flight_vars, dict):
            pass
        else:
            raise self._vspaero_error("'flight_vars' must be of type dict in 'get_flight_vars' method.")

        new_vars = {f"{self.name}_Sref": self.sref,
                    f"{self.name}_cref": self.cref,
                    f"{self.name}_bref": self.bref,
                    f"{self.name}_x_cg": self.x_cg,
                    f"{self.name}_y_cg": self.y_cg,
                    f"{self.name}_z_cg": self.z_cg,
                    f"{self.name}_mach": self.mach,
                    f"{self.name}_aoa": self.aoa,
                    f"{self.name}_beta": self.beta,
                    f"{self.name}_vinf": self.vinf,
                    f"{self.name}_rho": self.rho,
                    f"{self.name}_recref": self.recref,
                    f"{self.name}_clmax": self.clmax}

        if flight_vars is not None:
            flight_vars.update(new_vars)
        return new_vars

    def _setup_assembler(self):
        fname_prefix = f"{self.tmpdir.name}/{self.name}"
        # Write mesh file to temp directory
        self.write_mesh_file(f"{fname_prefix}.vspgeom")
        # Write flight conditions file to temp directory
        self.write_case_file(f"{fname_prefix}.vspaero")
        # Setup VSPAero Optimizer class from temp directory files
        self.assembler.setup(fname_prefix)
        # Set default options to assembler
        self._update_assembler_options()

    def __del__(self):
        """
        Delete temporary directory on deconstruct of class.
        """
        self.tmpdir.cleanup()

    def get_function_names(self):
        """
        Return a list of the current function key names
        """
        return list(self.func_list.keys())

    def write_mesh_file(self, file_name):
        """
        Write file which holds mesh information for VSPAero model.

        Parameters
        ----------
        file_name : str
            Name of file to write to.
        """
        # Check the solver type
        if self.solver_type.upper() == "PANEL":
            analysis_method = self.openvsp_model.PANEL
            alternate_input = False
            if file_name.endswith("tri"):
                file_name = f"{file_name[:-3]}.vspgeom"
        else:
            analysis_method = self.openvsp_model.VORTEX_LATTICE
            alternate_input = True

        # Check the symmetry flag
        symm = self.get_option("symmetry")
        # Only compare first character
        if symm[0].lower() == "y":
            symm_flag = True
        elif symm[0].lower() == "n":
            symm_flag = False
        else:
            self._vspaero_error(f"Provided symmetry option '{symm}' not supported. "
                                f"Symmetry must be either 'N' or 'Y'.")
        # Set mesh parameters
        self.openvsp_model.SetComputationFileName(self.openvsp_model.VSPAERO_VSPGEOM_TYPE, file_name)
        self.openvsp_model.SetIntAnalysisInput("VSPAEROComputeGeometry", "AnalysisMethod", [analysis_method])
        self.openvsp_model.SetIntAnalysisInput("VSPAEROComputeGeometry", "AlternateInputFormatFlag", [alternate_input])
        self.openvsp_model.SetIntAnalysisInput("VSPAEROComputeGeometry", "GeomSet", [self.VSPAERO_VSP_SET])
        self.openvsp_model.SetIntAnalysisInput("VSPAEROComputeGeometry", "Symmetry", [symm_flag])
        # Create mesh
        result_id = self.openvsp_model.ExecAnalysis("VSPAEROComputeGeometry")
        # Delete mesh from vsp model
        mesh_id = self.openvsp_model.GetStringResults(result_id, "Mesh_GeomID")
        self.openvsp_model.DeleteGeomVec(mesh_id)

    def write_case_file(self, file_name):
        """
        Write file which holds VSPAero problem information.

        Parameters
        ----------
        file_name : str
            Name of file to write to.
        """
        file_handle = open(file_name, "w")
        file_handle.write(f"Sref = {self.sref}\n")
        file_handle.write(f"Cref = {self.cref}\n")
        file_handle.write(f"Bref = {self.bref}\n")
        file_handle.write(f"X_cg = {self.x_cg}\n")
        file_handle.write(f"Y_cg = {self.y_cg}\n")
        file_handle.write(f"Z_cg = {self.z_cg}\n")
        file_handle.write(f"Mach = {self.mach}\n")
        file_handle.write(f"AoA = {self.aoa}\n")
        file_handle.write(f"Beta = {self.beta}\n")
        file_handle.write(f"Vinf = {self.vinf}\n")
        file_handle.write(f"Rho = {self.rho}\n")
        file_handle.write(f"ReCref = {self.recref}\n")
        file_handle.write(f"ClMax = {self.clmax}\n")

        mta = self.get_option("max_turning_angle")
        file_handle.write(f"MaxTurningAngle = {mta}\n")
        sym = self.get_option("symmetry")
        file_handle.write(f"Symmetry = {sym}\n")
        far_dist = self.get_option("far_dist")
        file_handle.write(f"FarDist = {far_dist}\n")
        num_wake_nodes = self.get_option("num_wake_nodes")
        file_handle.write(f"NumWakeNodes = {num_wake_nodes}\n")
        wake_iters = self.get_option("wake_iters")
        file_handle.write(f"WakeIters = {wake_iters}\n")
        file_handle.write("NumberOfRotors = 0\n")
        file_handle.write("NumberOfControlGroups = 0\n")
        file_handle.close()

    def set_option(self, name, value):
        """
        Set a solver option value. The name is not case sensitive.

        Parameters
        ----------
        name : str
            Name of option to modify

        value : depends on option
            New option value to set
        """
        # Default setOption for common problem class objects
        BaseUI.set_option(self, name, value)

        # Update tolerances
        if "debug_print" in name.lower():
            debug_flag = self.get_option("debug_print")
            self.assembler.set_debug_print(debug_flag)

        if self.assembler.is_setup:
            if "gmres_tol" in name.lower():
                tol = self.get_option("gmres_tol")
                self.assembler.set_gmres_tolerance(tol)

    def _update_assembler_options(self):
        """
        Makes sure VSP_Optimizer class has option values initialized to correct values after setup.x
        """
        for option_name in self.options:
            self.set_option(option_name, self.options[option_name][1])

    def solve(self):
        self.assembler.solve_forward()
        self.call_counter += 1

    def solve_adjoint(self):
        self.assembler.solve_adjoint()

    def write_solution(self, output_dir=None, base_name=None, number=None):
        """
        This is a generic shell function that writes the output
        file(s).  The intent is that the user or calling program can
        call this function and pyVSPAero writes all the files that the
        user has defined. It is recommended that this function is used
        along with the associated logical flags in the options to
        determine the desired writing procedure

        Parameters
        ----------
        output_dir : str or None
            Use the supplied output directory
        base_name : str or None
            Use this supplied string for the base filename. Typically
            only used from an external solver.
        number : int or None
            Use the user supplied number to index solution. Again, only
            typically used from an external solver
        """

        # Check input
        if output_dir is None:
            output_dir = self.get_option('output_dir')

        if base_name is None:
            base_name = self.name

        # If we are numbering solution, it saving the sequence of
        # calls, add the call number
        if number is not None:
            # We need number based on the provided number:
            base_name = base_name + '_%3.3d' % number
        else:
            # if number is none, i.e. standalone, but we need to
            # number solutions, use internal counter
            if self.get_option('number_solutions'):
                base_name = base_name + '_%3.3d' % self.call_counter

        # Unless the writeSolution option is off write actual file:
        if self.get_option('write_solution'):
            tmp_file = os.path.join(self.tmpdir.name, f"{self.name}.adb")
            base = os.path.join(output_dir, base_name) + '.adb'
            shutil.copyfile(tmp_file, base)
            shutil.copyfile(f"{tmp_file}.cases", f"{base}.cases")

    def get_num_nodes(self):
        return self.assembler.get_number_of_nodes()

    @property
    def num_nodes(self):
        return  self.get_num_nodes()

    def get_geometry(self):
        return self.assembler.get_geometry()

    def set_geometry(self, xyz):
        return self.assembler.set_geometry(xyz)


