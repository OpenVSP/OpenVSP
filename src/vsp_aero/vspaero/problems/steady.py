"""
pySteady_problem
"""

# =============================================================================
# Imports
# =============================================================================
import numpy as np
from .base import BaseProblem, wrt_dict


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

        return

    def eval_functions(self, funcs, eval_funcs=None,
                      ignore_missing=False):
        """
        This is the main routine for returning useful information from
        pyvspaero. The functions corresponding to the strings in
        EVAL_FUNCS are evaluated and updated into the provided
        dictionary.

        Parameters
        ----------
        funcs : dict
            Dictionary into which the functions are saved.
        eval_funcs : iterable object containing strings.
            If not none, use these functions to evaluate.
        ignore_missing : bool
            Flag to suppress checking for a valid function. Please use
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

        for fidx, f in enumerate(self.func_list):
            if f in eval_funcs:
                key = self.name + '_%s' % f
                fun_len = self.assembler.get_optimization_function_length(fidx)
                funcs[key] = np.zeros(fun_len)
                self.assembler.get_function_value(fidx, funcs[key])

    def eval_functions_sens(self, funcs_sens, eval_funcs=None):
        """
        This is the main routine for returning useful (sensitivity)
        information from problem. The derivatives of the functions
        corresponding to the strings in EVAL_FUNCS are evaluated and
        updated into the provided dictionary. The derivatives with
        respect to all design variables and node locations are computed.

        Parameters
        ----------
        funcs_sens : dict
            Dictionary into which the derivatives are saved.
        eval_funcs : iterable object containing strings
            The functions the user wants returned

        Examples
        --------
        >>> funcs_sens = {}
        >>> steady_problem.evalFunctionsSens(funcs_sens, ['cruise_CL'])
        >>> funcs_sens
        >>> # Result will look like (if SteadyProblem has name of 'cruise'):
        >>> # {'cruise_CL':{'xyz':[1.234, ..., 7.89], 'aoa':[3.14], ...}}
        """

        if eval_funcs is None:
            eval_funcs = sorted(list(self.func_list.keys()))
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
