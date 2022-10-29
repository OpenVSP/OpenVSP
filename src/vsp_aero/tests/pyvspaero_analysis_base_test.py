import numpy as np
import unittest
from vspaero.problems.base import wrt_dict as WRT_DICT

"""
This is a base class for running pyvspaero unit test cases.
This base class will test function evaluations and total 
sensitivities for the user-specified problems implemented by
the child test case. When the user creates a new test based 
on this class only a 'setup_vspaero_problems' method is
required to be defined in the child class. See the virtual method 
implementations for this method below for more details.

NOTE: The child class must NOT implement its own setUp method 
for the unittest class. This is handled in the base class.
"""


class PyVSPAeroTestCase:
    class PyVSPAeroTest(unittest.TestCase):
        def setUp(self):

            # Default fd step size and tolerances
            # Can be overridden in child class
            self.rtol = 1e-4
            self.atol = 1e-4
            self.dh = 1e-5

            # Setup user-specified assembler for this test
            self.problems = self.setup_vspaero_problems()

            # Get the design variable values
            self.dv0 = {}
            self.dv1 = {}
            for problem in self.problems:
                problem.get_flight_vars(self.dv0)
            # Initial nodal location vector
            self.xyz0 = self.problems[0].get_geometry()
            # Create temporary node vec for doing fd
            self.xyz1 = np.zeros_like(self.xyz0)
            self.xyz_pert = self.xyz0.copy()
            self.xyz_pert /= np.linalg.norm(self.xyz_pert)

        def setup_vspaero_problems(self):
            """
            Setup vspaero problems objects that describe different problem types we will be testing.
            Must be defined in child class that inherits from this class.
            """
            raise NotImplementedError(
                "Child class must implement a 'setup_vspaero_problems' method"
            )
            return

        def test_solve(self):
            """
            Test linear solve and function evaluations
            """
            # solve
            funcs = self.run_solve()

            # Test functions values against historical values
            for prob in self.problems:
                with self.subTest(problem=prob.name):
                    func_names = prob.get_function_names()
                    for func_name in func_names:
                        with self.subTest(function=func_name):
                            func_key = f"{prob.name}_{func_name}"
                            np.testing.assert_allclose(
                                funcs[func_key],
                                self.FUNC_REFS[func_key],
                                rtol=self.rtol,
                                atol=self.atol,
                            )

        def test_total_dv_sensitivities(self):
            """
            Test total flight variable sensitivity through adjoint against fd/cs
            """
            # Initial solve
            funcs = self.run_solve()

            # Compute the total derivative w.r.t. flight design variables using adjoint
            func_sens = self.run_sensitivities()

            # Tests cs/fd against sensitivity from adjoint
            for prob in self.problems:
                with self.subTest(problem=prob.name):
                    func_names = prob.get_function_names()
                    symm_flag = prob.get_option("symmetry")
                    # Loop through each flight variable
                    for dv_name in WRT_DICT:
                        with self.subTest(dv_name=dv_name):
                            # If symmetry is set to "y", beta var must always be 0
                            if symm_flag.lower() == "y" and dv_name == "beta":
                                pass
                            else:
                                dv_key = f"{prob.name}_{dv_name}"
                                # Compute the total derivative w.r.t. flight vars using fd/cs
                                self.dv1 = self.perturb_dv_dict(self.dv0, dv_key)
                                # Run perturbed solution
                                funcs_pert = self.run_solve(dv=self.dv1)
                                # Loop through each function
                                for func_name in func_names:
                                    with self.subTest(function=func_name):
                                        func_key = f"{prob.name}_{func_name}"
                                        # project exact sens
                                        dfddv_proj = func_sens[func_key][dv_name]
                                        # Compute approximate sens
                                        f_dv_sens_approx = self.compute_fd_approx(
                                            funcs_pert[func_key], funcs[func_key]
                                        )
                                        np.testing.assert_allclose(
                                            dfddv_proj,
                                            f_dv_sens_approx,
                                            rtol=self.rtol,
                                            atol=self.atol,
                                        )

        def test_total_xyz_sensitivities(self):
            """
            Test total xpt sensitivity through adjoint against fd/cs
            """
            # Initial solve
            funcs = self.run_solve()

            # Compute the total derivative w.r.t. mesh design variables using adjoint
            func_sens = self.run_sensitivities()

            # Compute the total derivative w.r.t. nodal xpt locations using fd/cs
            self.xyz1 = self.perturb_xyz_vec(self.xyz0, self.xyz_pert)
            # Run perturbed solution
            funcs_pert = self.run_solve(xyz=self.xyz1)

            # Tests cs/fd against sensitivity from adjoint
            for prob in self.problems:
                with self.subTest(problem=prob.name):
                    func_names = prob.get_function_names()
                    for func_name in func_names:
                        with self.subTest(function=func_name):
                            func_key = f"{prob.name}_{func_name}"
                            # project exact sens
                            dfdx_proj = func_sens[func_key]["xyz"].dot(self.xyz_pert)
                            # Compute approximate sens
                            f_xpt_sens_approx = self.compute_fd_approx(
                                funcs_pert[func_key], funcs[func_key]
                            )
                            np.testing.assert_allclose(
                                dfdx_proj,
                                f_xpt_sens_approx,
                                rtol=self.rtol,
                                atol=self.atol,
                            )

        def run_solve(self, dv=None, xyz=None):
            """
            Run a linear solve at specified design point and return functions of interest
            """
            if dv is None:
                dv = self.dv0

            if xyz is None:
                xyz = self.xyz0

            # Solve each problem and evaluate functions
            funcs = {}
            for prob in self.problems:
                # Set the design variables
                prob.set_flight_vars(**dv)
                # Set node locations
                prob.set_geometry(xyz)
                # Solve problem
                prob.solve()
                # Evaluate functions
                prob.eval_functions(funcs)

            return funcs

        def run_sensitivities(self, dv=None, xyz=None):
            """
            Run a sensitivity solve at specified design point and return sens of  functions of interest
            """
            if dv is None:
                dv = self.dv0

            if xyz is None:
                xyz = self.xyz0

            # Solve each problem and evaluate function sens
            funcs_sens = {}
            funcs = {}
            for prob in self.problems:
                # Set the design variables
                prob.set_flight_vars(**dv)
                # Set node locations
                prob.set_geometry(xyz)
                # Solve problem
                prob.solve()
                # Evaluate functions
                prob.eval_functions(funcs)
                prob.eval_functions_sens(funcs_sens)

            return funcs_sens

        def perturb_dv_dict(self, dv_dict, dv_key):
            """
            Perform fd perturbation on vspaero dv dict as follows
            dv_out = dv_in + dh
            """
            dv_out = dv_dict.copy()
            dv_out[dv_key] += self.dh
            return dv_out

        def perturb_xyz_vec(self, vec_in, vec_pert):
            """
            Perform fd perturbation on vspaero vector as follows
            vec_out = vec_in + dh * vec_pert
            """
            vec_out = vec_in + self.dh * vec_pert
            return vec_out

        def compute_fd_approx(self, vec1, vec0):
            """
            Perform fd calculation to approximate sensitivities

            difference performed as follows:
                sens = (vec1 - vec0) / dh, in real mode
            """
            sens_approx = (vec1 - vec0) / self.dh
            return sens_approx
