import unittest
import numpy as np
from openmdao.utils.assert_utils import assert_check_totals

"""
This is a base class for running openmdao unit test cases.
This base class will test function evaluations, partial, and total 
sensitivities for the user-specified openmdao problems implemented by
the child test case. When the user creates a new test based 
on this class a `setup_problem` method is required to be defined in the child class.

See the virtual method implementations for each method 
below for more details.
NOTE: The child class must NOT implement its own setUp method 
for the unittest class. This is handled in the base class.
"""


class OpenMDAOTestCase:
    class OpenMDAOTest(unittest.TestCase):
        rtol = 1e-5
        atol = 1e-6
        dh = 1e-6
        fd_method = "fd"
        fd_form = "central"

        FUNC_REFS = {}
        WRT = []

        def setUp(self):

            # Setup user-specified openmdao problem for this test
            self.prob = self.setup_problem()
            self.prob.setup(mode="rev")

        def setup_problem(self):
            """
            Setup openmdao problem object we will be testing.
            Must be defined in child class that inherits from this class.
            """
            raise NotImplementedError(
                "Child class must implement a 'setup_problem' method"
            )
            return

        def test_solve(self):
            """
            Test linear solve and function evaluations
            """
            # solve
            self.prob.run_model()

            # Test functions values against historical values
            for func_name in self.FUNC_REFS:
                with self.subTest(function=func_name):
                    np.testing.assert_allclose(
                        self.prob[func_name], self.FUNC_REFS[func_name],
                        rtol=self.rtol,
                        atol=self.atol,
                    )

        def test_partials(self):
            """
            Test partial sensitivities using fd/cs
            """
            # solve
            self.prob.run_model()

            # Test functions values against historical values
            data = self.prob.check_partials(
                compact_print=True,
                out_stream=None,
                method=self.fd_method,
                form=self.fd_form,
                step=self.dh,
            )
            self.assert_check_partials(data)

        def test_totals(self):
            """
            Test total sensitivities using fd/cs
            """
            # solve
            self.prob.run_model()

            # Test functions total sensitivities
            of = list(self.FUNC_REFS.keys())
            for var_wrt in self.WRT:
                with self.subTest(wrt=var_wrt):
                    for var_of in of:
                        with self.subTest(of=var_of):
                            data = self.prob.check_totals(
                                of=var_of,
                                wrt=var_wrt,
                                compact_print=True,
                                out_stream=None,
                                method=self.fd_method,
                                form=self.fd_form,
                                step=self.dh,
                            )
                            self.assert_check_totals(data)

        def assert_check_partials(self, data):
            """
            Check partial sensitivity vector calculated analytically vs fd approximation.
            """
            for component in data:
                for func_name, wrt in data[component]:
                    with self.subTest(component=component):
                        with self.subTest(wrt=wrt):
                            with self.subTest(of=func_name):
                                np.testing.assert_allclose(
                                    data[component][func_name, wrt]["J_rev"],
                                    data[component][func_name, wrt]["J_fd"],
                                    rtol=self.rtol,
                                    atol=self.atol,
                                )

        def assert_check_totals(self, data):
            """
            Check total sensitivity vector calculated analytically vs fd approximation.
            """
            assert_check_totals(data, atol=self.atol, rtol=self.rtol)
