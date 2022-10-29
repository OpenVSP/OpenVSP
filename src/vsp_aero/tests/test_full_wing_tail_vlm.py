import os
import numpy as np
from vspaero import pyVSPAero, functions
from pyvspaero_analysis_base_test import PyVSPAeroTestCase

"""
Tests a steady wing and small tail analysis using VLM solver.
This case verifies that VSPAERO handles multi-body cases consistently.
Test includes two cases: an incompressible (M=0.0) and compressible analysis (M=0.8).
Tests inviscid function values and sensitivities against FD.
"""

base_dir = os.path.dirname(os.path.abspath(__file__))
vsp_file = os.path.join(base_dir, "./input_files/wing_and_tail.vsp3")

class ProblemTest(PyVSPAeroTestCase.PyVSPAeroTest):

    FUNC_REFS = {'incompressible_CLi': np.array([0.53991878]),
                 'incompressible_CDi': np.array([0.00847457]),
                 'incompressible_CMxi': np.array([7.28181598e-12]),
                 'incompressible_CMyi': np.array([-0.27542922]),
                 'incompressible_CMzi': np.array([-7.35694769e-13]),
                 'compressible_CLi': np.array([0.91251037]),
                 'compressible_CDi': np.array([0.02388441]),
                 'compressible_CMxi': np.array([6.94843727e-11]),
                 'compressible_CMyi': np.array([-0.33808139]),
                 'compressible_CMzi': np.array([1.50429821e-12])}

    def setup_vspaero_problems(self):
        """
        Setup vspaero problems objects that describe different problem types we will be testing.
        Must be defined in child class that inherits from this class.
        """
        VLMAssembler = pyVSPAero(vsp_file)
        # Add output functions
        VLMAssembler.add_function("CLi", functions.CLi)
        VLMAssembler.add_function("CDi", functions.CDi)
        VLMAssembler.add_function("CMxi", functions.CMXi)
        VLMAssembler.add_function("CMyi", functions.CMYi)
        VLMAssembler.add_function("CMzi", functions.CMZi)

        # Create a steady problem instance
        prob1 = VLMAssembler.create_steady_problem("incompressible", aoa=5.0, x_cg=0.5, z_cg=-0.25)
        prob2 = VLMAssembler.create_steady_problem("compressible", aoa=6.0, x_cg=0.5, z_cg=-0.25, mach=0.8)
        all_problems = [prob1, prob2]
        return all_problems
