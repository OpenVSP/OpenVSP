import os
import numpy as np
from numpy.testing import assert_almost_equal
import unittest

from vspaero.optimizer import pyVSPOptimizer
from vspaero.functions import CL


class WingTest(unittest.TestCase):
    def setUp(self):

        # Run `vspaero_opt`
        d = os.path.dirname(os.path.abspath(__file__))
        os.system(f"{d}/../../Solver/vspaero_opt 1 hershey")

        self._fname_prefix = fname_prefix = "hershey"
        self._vspopt = vspopt = pyVSPOptimizer()

        vspopt.set_number_of_threads(4)
        vspopt.set_unsteady_analysis(0)
        vspopt.set_number_of_optimization_functions(1)
        vspopt.set_optimization_functions(np.array([CL], dtype=np.int32))
        vspopt.setup(fname_prefix)
        vspopt.solve()

    def test_output(self):
        vspopt = self._vspopt

        # Only one output for this case.
        p = 0

        # Read the value of the output function (CL) for this case as calculated by `vspaero_opt 1 hershey`.
        with open("hershey.opt.gradient", "r") as f:
            nnodes = int(f.readline())
            self.assertEqual(vspopt.get_number_of_nodes(), nnodes)
            func_expected = float(f.readline().strip().split(' ')[-1])

        # Get the value of the output function from the Python wrapper.
        func = np.empty(1, dtype=np.float64)
        vspopt.get_function_value(p, func)
        assert_almost_equal(func[0], func_expected, 6)

        # Read the value of the derivatives for this case as calculated by `vspaero_opt 1 hershey`.
        data = np.loadtxt("hershey.opt.gradient", skiprows=3)
        dfdx_expected = data[:, 4:]

        # Get the derivatives found here.
        vspopt = self._vspopt
        nnodes = vspopt.get_number_of_nodes()
        dfdx_actual = np.zeros((nnodes, 3))
        for i in range(nnodes):
            dfdx_actual[i, 0] = vspopt.gradient_x(p, i)
            dfdx_actual[i, 1] = vspopt.gradient_y(p, i)
            dfdx_actual[i, 2] = vspopt.gradient_z(p, i)

        assert_almost_equal(dfdx_actual, dfdx_expected)


if __name__ == '__main__':
    unittest.main()
