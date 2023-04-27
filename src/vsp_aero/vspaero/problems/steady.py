"""
pySteady_problem
"""

# =============================================================================
# Imports
# =============================================================================
import os

import numpy as np
from .base import BaseProblem, wrt_dict
from .. import functions


class SteadyProblem(BaseProblem):
    """
    Class for VSPAero steady problem types. Contains methods common to all VSPAero problems.
    """

    # Default options for class
    default_options = {
        'max_turning_angle': [float, -1.0, 'Ask Dave???.'],

        # Solution Options
        'symmetry': [str, 'N', "String flag declaring if problem size should be reduced using geometrical symmetry.\n"
                               "\t Acceptable values are:\n"
                               "\t\t N = No symmetry (full model)\n"
                               "\t\t X = Symmetry along 'x'-axis (half model)\n"
                               "\t\t Y = Symmetry along 'y'-axis (half model)\n"
                               "\t\t Z = Symmetry along 'z'-axis (half model)\n"],
        'far_dist': [float, -1., "Defines how far downstream the wake should be shed. "
                                 "If a negative value is specified, "
                                 "VSPAero well determine an appropriate distance through heuristics.\n"],
        'num_wake_nodes': [int, 64, 'Number of nodes to discretize wake with in the downstream direction. '
                                    'Must be power of 2.'],
        'wake_iters': [int, 0, 'Number of iterations required to solve wake.'],
        'gmres_tol': [float, 1e-8, 'Tolerance for Flow/Adjoint GMRES solver.'],

        # Output Options
        'output_dir': [str, './', 'Output directory for F5 file writer.'],
        'write_solution': [bool, True, 'Flag for suppressing all f5 file writing.'],
        'number_solutions': [bool, True, 'Flag for attaching solution counter index to f5 files.'],
        'debug_print': [bool, False, 'Flag for printing VSPAERO Solver information to stdout.'],

    }

    is_unsteady = False

    def __init__(self, name, openvsp_model, func_list, solver_type, options=None):

        # Problem name
        self.name = name

        # Default setup for common problem class objects
        BaseProblem.__init__(self, openvsp_model, func_list, solver_type, options)

        # Create problem-specific variables
        self._setup_assembler()

        #self.assembler.solve()

        return

    def eval_functions(self, funcs, eval_funcs=None,
                      ignore_missing=False):
        """
        This is the main routine for returning useful information from
        pyvspaero. The functions corresponding to the strings in
        `eval_funcs` are evaluated and updated into the provided
        dictionary.

        Parameters
        ----------
        funcs: dict
            Dictionary into which the functions are saved.
        eval_funcs: str or iterable object containing strings.
            If not none, use these functions to evaluate.
        ignore_missing: bool
            Flag to supress checking for a valid function. Please use
            this option with caution.

        Examples
        --------
        >>> funcs = {}
        >>> steady_problem.solve()
        >>> steady_problem.eval_functions(funcs, ['CL'])
        >>> funcs
        >>> # Result will look like (if SteadyProblem has name of 'cruise'):
        >>> # {'cruise_CL':12354.10}
        """

        if eval_funcs is None:
            eval_funcs = sorted(list(self.func_list.keys()))
        else:
            eval_funcs = sorted(list(eval_funcs))

        if not ignore_missing:
            for f in eval_funcs:
                if f not in self.func_list:
                    raise self._vspaero_error(f"Supplied function '{f}' has not been added "
                                          "using add_function().")

        # Get current states
        gamma = self.assembler.get_forward_solution_vector()
        # Calculate updated function values
        self.assembler.calculate_optimization_functions(gamma)

        # Loop through each function and append to output
        for fidx, f in enumerate(self.func_list):
            if f in eval_funcs:
                key = self.name + '_%s' % f
                fun_len = self.assembler.get_optimization_function_length(fidx)
                funcs[key] = np.zeros(fun_len)
                self.assembler.get_function_value(fidx, funcs[key])

    def eval_functions_sens(self, funcs_sens, eval_funcs=None):
        """
        This is the main routine for returning useful (sensitivity)
        information from the problem. The function derivatives
        corresponding to the strings in `eval_funcs` are evaluated and
        updated into the provided dictionary. The derivitives with
        respect to all design variables and node locations are computed.

        Parameters
        ----------
        funcs_sens: dict
            Dictionary into which the derivatives are saved.
        eval_funcs: str or iterable object containing strings
            The functions the user wants returned

        Examples
        --------
        >>> funcs_sens = {}
        >>> steady_problem.eval_functions_sens(funcs_sens, ['cruise_CL'])
        >>> funcs_sens
        >>> # Result will look like (if SteadyProblem has name of 'cruise'):
        >>> # {'cruise_CL':{'xyz':[1.234, ..., 7.89], 'aoa':[3.14], ...}}
        """

        if eval_funcs is None:
            eval_funcs = sorted(list(self.func_list.keys()))
        elif isinstance(eval_funcs, str):
            eval_funcs = [eval_funcs]
        else:
            eval_funcs = sorted(list(eval_funcs))

        # Check that the functions are all ok.
        for f in eval_funcs:
            if f not in self.func_list:
                raise self._vspaero_error("Supplied function has not been added "
                                          "using add_function()")

        self.solve_adjoint()

        for fidx, f in enumerate(self.func_list):
            if f in eval_funcs:
                key = f"{self.name}_{f}"
                funcs_sens[key] = {}

                funcs_sens[key]['xyz'] = np.zeros(3*self.num_nodes)
                self.assembler.get_function_gradients(fidx, funcs_sens[key]['xyz'])
                for flight_var in wrt_dict:
                    var_id = wrt_dict[flight_var]
                    funcs_sens[key][flight_var] = np.atleast_1d(self.assembler.func_input_gradients(fidx, var_id))

    def get_residual(self):
        """
        This routine is used to evaluate directly the aerodynamic
        residual. Only typically used with aerostructural analysis.

        Returns
        -------
        res : numpy.ndarray
            Array holding current residual vector.
        """
        gamma = self.get_states()
        res = np.zeros_like(gamma)
        self.assembler.calculate_forward_residual(gamma, res)
        return res

    def get_states(self):
        """
        This routine is used to evaluate directly the aerodynamic
        residual. Only typically used with aerostructural analysis.

        Returns
        -------
        prod : numpy.ndarray
            Array holding current state variable vector for VLM circulation strength.
        """
        gamma = self.assembler.get_forward_solution_vector()
        return gamma

    def set_states(self, gamma):
        """
        This routine is used to directly update the aerodynamic
        state variables. Only typically used with aerostructural analysis.

        Parameters
        ----------
        gamma: np.ndarray
            Dictionary into which the derivatives are saved.
        """
        self.assembler.set_forward_solution_vector(gamma)
        return

    def get_nodal_forces(self):
        """
        This routine is used to evaluate directly the aerodynamic
        forces at the mesh nodes. Only typically used with aerostructural analysis.

        Returns
        -------
        forces : numpy.ndarray
            Array holding current aerodynamic forces at VLM nodes.
        """
        forces = self.assembler.get_nodal_forces()
        return forces

    def eval_trans_jac_vec_product(self, psi):
        """
        This routine is used to evaluate the matrix-vector product of a vector
        with the transpose of the problem Jacobian.
        Only typically used with aerostructural analysis.

        Returns
        -------
        prod : numpy.ndarray
            Array holding matrix vector product.
        """
        prod = np.zeros_like(psi)
        self.assembler.calculate_adjoint_matrix_vector_product(psi, prod)
        return prod

    def eval_function_partials(self, funcs_sens, eval_funcs=None):
        """
        This is the main routine for returning partial sensitivity
        information from problem. The derivatives of the functions
        corresponding to the strings in `eval_funcsg` are evaluated and
        updated into the provided dictionary. The derivatives with
        respect to all design variables, node locations, and state
        variables are computed.

        Parameters
        ----------
        funcs_sens: dict
            Dictionary into which the partial derivatives are saved.
        eval_funcs: str or iterable object containing strings
            The functions the user wants returned
        """

        if eval_funcs is None:
            eval_funcs = sorted(list(self.func_list.keys()))
        elif isinstance(eval_funcs, str):
            eval_funcs = [eval_funcs]
        else:
            eval_funcs = sorted(list(eval_funcs))

        # Check that the functions are all ok.
        for f in eval_funcs:
            if f not in self.func_list:
                raise self._vspaero_error("Supplied function has not been added "
                                          "using add_function()")

        num_eqs = self.get_num_adjoint_eqs()

        for fidx, f in enumerate(self.func_list):
            if f in eval_funcs:
                key = f"{self.name}_{f}"
                funcs_sens[key] = {}

                funcs_sens[key]['xyz'] = np.zeros(3*self.num_nodes)
                funcs_sens[key]["gamma"] = np.zeros(num_eqs)
                input_sens = np.zeros(functions.NUMBER_OF_INPUTS+1)

                self.assembler.calculate_optimization_function_input_partials(fidx, funcs_sens[key]['xyz'],
                                                                              input_sens[1:], funcs_sens[key]["gamma"])
                # Convert input sens array into dictionary keys
                for var_name in wrt_dict:
                    input_idx = wrt_dict[var_name]
                    funcs_sens[key][var_name] = input_sens[input_idx]


        return funcs_sens

    def calculate_adjoint_residual_partial_products(self, psi):
        """
        This routine is used to evaluate the adjoint product contribution to
        the total design sensitivity. Typically, only used with
        aerostructural analysis.

        The contribution is given by the following formula:

            prod_sens = psi^T . pRpx

        Parameters
        ----------
        psi: numpy.ndarray
            Array holding adjoint vector.

        Returns
        -------
        prod_sens : dict
            Array holding matrix vector product.
        """
        prod_sens = {'xyz': np.zeros(3 * self.num_nodes)}
        input_prod_sens = np.zeros(functions.NUMBER_OF_INPUTS+1)
        self.assembler.calculate_adjoint_residual_partial_products(psi, prod_sens['xyz'], input_prod_sens[1:])

        # Convert input sens array into dictionary keys
        for var_name in wrt_dict:
            input_idx = wrt_dict[var_name]
            prod_sens[var_name] = input_prod_sens[input_idx]

        return prod_sens

    def calculate_nodal_force_partial_products(self, pf_pforces):
        """
        This routine is used to evaluate the nodal force contribution to
        the total design sensitivity. Typically, only used with
        aerostructural analysis.

        Parameters
        ----------
        pf_pforces: numpy.ndarray
            Array holding partial sensitivity of output function wrt nodal forces.

        Returns
        -------
        prod_sens : dict
            Dictionary holding sensitivities.
        """
        num_eqs = self.get_num_adjoint_eqs()
        prod_sens = {'xyz': np.zeros(3 * self.num_nodes), 'gamma': np.zeros(num_eqs)}
        input_prod_sens = np.zeros(functions.NUMBER_OF_INPUTS + 1)
        self.assembler.calculate_nodal_force_partial_products(pf_pforces, prod_sens['xyz'], input_prod_sens[1:],
                                                              prod_sens["gamma"])

        # Convert input sens array into dictionary keys
        for var_name in wrt_dict:
            input_idx = wrt_dict[var_name]
            prod_sens[var_name] = input_prod_sens[input_idx]
        return prod_sens

    def solve_adjoint_for_rhs(self, adj_rhs):
        """
        Solve adjoint linear system for user-defined right-hand side array.
        Typically, only used with aerostructural analysis.

        Returns
        -------
        psi : numpy.ndarray
            Array holding adjoint solution vector.
        """
        psi = np.zeros_like(adj_rhs)
        self.assembler.solve_adjoint_linear_system(psi, adj_rhs)
        return psi

    def write_solution(self, output_dir=None, base_name=None, number=None):
        """
        This is a generic shell function that writes the output
        file(s). The intent is that the user or calling program can
        call this function and pyVSPAero writes all the files that the
        user has defined.

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

        # Call parent method to write out adb files
        BaseProblem.write_solution(self, output_dir, base_name, number)

        # Write out tecplot file
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
            base = os.path.join(output_dir, base_name) + '.dat'
            self._write_tecplot_file(base)

    def _write_tecplot_file(self, file_name):
        """
        Write solution data to a TecPlot plt file for post-processing.
        This data includes: nodal coordinates, circulation, and nodal forces.

        Parameters
        ----------
        file_name : str or None
            File name to write Tecplot data to.
        """

        tec_file = open(file_name, "w")

        # get connectivity of vortex loops
        conn, ptr = self.assembler.get_nodal_connectivity()
        # Node locations and forces
        xyz = self.get_geometry()
        xyz = xyz.reshape(self.num_nodes, 3)
        forces = self.get_nodal_forces()
        forces = forces.reshape(self.num_nodes, 3)

        # Convert circulation from element-centered value to nodal-centered
        circulations = self.get_states()
        elem_count = [np.count_nonzero(conn == node_j) for node_j in range(self.num_nodes)]
        nodal_circulations = np.zeros(self.num_nodes)
        num_loops = self.get_num_loops()
        # nodal circulation is the average of each element its connected to
        for elem_i in range(num_loops):
            attached_nodes = conn[ptr[elem_i]:ptr[elem_i+1]]
            for node_j in attached_nodes:
                nodal_circulations[node_j] += circulations[elem_i] / elem_count[node_j]

        # Write out header
        tec_file.write('title = "VSPAero: Aerodynamic Solution"\n')
        tec_file.write('variables = "x", "y", "z", "gamma", "fx", "fy", "fz"\n')
        tec_file.write(f'zone n={self.num_nodes}, e={num_loops}, f=fepoint, et=quadrilateral T="{self.name}"\n')
        tec_file.write('\n')

        # Write out nodal variables
        for node_i in range(self.num_nodes):
            tec_file.write(f'{xyz[node_i, 0]} {xyz[node_i, 1]} {xyz[node_i, 2]} {nodal_circulations[node_i]} '
                           f'{forces[node_i, 0]} {forces[node_i, 1]} {forces[node_i, 2]} \n')
        tec_file.write('\n')

        # Write out connectivity
        for elem_i in range(num_loops):
            attached_nodes = conn[ptr[elem_i]:ptr[elem_i + 1]]
            # tecplot numbering is 1 based
            attached_nodes += 1
            num_attached = len(attached_nodes)
            # We'll treat tri elements as degenerate quads (last node repeated twice)
            if num_attached == 3:
                tec_file.write(f'{attached_nodes[0]} {attached_nodes[1]} {attached_nodes[2]} {attached_nodes[2]} \n')
            elif num_attached == 4:
                # Check which node order gives an unskewed quad
                n = attached_nodes - 1
                det1 = np.cross(xyz[n[2]] - xyz[n[0]], xyz[n[3]] - xyz[n[1]])
                det2 = np.cross(xyz[n[3]] - xyz[n[0]], xyz[n[2]] - xyz[n[1]])
                if np.linalg.norm(det1) > np.linalg.norm(det2):
                    tec_file.write(f'{attached_nodes[0]} {attached_nodes[1]} {attached_nodes[2]} {attached_nodes[3]} \n')
                else:
                    tec_file.write(f'{attached_nodes[0]} {attached_nodes[1]} {attached_nodes[3]} {attached_nodes[2]} \n')
            else:
                pass

        tec_file.close()
