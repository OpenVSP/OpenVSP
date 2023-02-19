import os

import numpy as np
from vspaero import pyVSPAero, functions
from pyvspaero_analysis_base_test import PyVSPAeroTestCase

"""
Tests a steady cylindrical fuselage analysis in isolation using the panel solver.
Test includes two cases: an incompressible (M=0.0) and compressible analysis (M=0.8).
Tests inviscid/viscous function values and sensitivities against FD.
"""

base_dir = os.path.dirname(os.path.abspath(__file__))
vsp_file = os.path.join(base_dir, "./input_files/fuselage.vsp3")

class ProblemTest(PyVSPAeroTestCase.PyVSPAeroTest):
    FUNC_REFS = {'incompressible_CL': np.array([0.000000000000]),
                 'incompressible_CD': np.array([0.020644420525]),
                 'incompressible_CMx': np.array([0.000000000000]),
                 'incompressible_CMy': np.array([0.106766373781]),
                 'incompressible_CMz': np.array([-0.000000000000]),
                 'incompressible_CLi': np.array([-0.000000000000]),
                 'incompressible_CDi': np.array([0.000000367790]),
                 'incompressible_CMxi': np.array([0.000000000000]),
                 'incompressible_CMyi': np.array([0.106766373781]),
                 'incompressible_CMzi': np.array([-0.000000000000]),
                 'compressible_CL': np.array([0.000000000000]),
                 'compressible_CD': np.array([0.020644420525]),
                 'compressible_CMx': np.array([0.000000000000]),
                 'compressible_CMy': np.array([-0.525242924582]),
                 'compressible_CMz': np.array([-0.000000000000]),
                 'compressible_CLi': np.array([0.000000000000]),
                 'compressible_CDi': np.array([0.000000000000]),
                 'compressible_CMxi': np.array([0.000000000000]),
                 'compressible_CMyi': np.array([-0.525242924582]),
                 'compressible_CMzi': np.array([-0.000000000000])}

    def setup_vspaero_problems(self):
        """
        Setup vspaero problems objects that describe different problem types we will be testing.
        Must be defined in child class that inherits from this class.
        """
        VLMAssembler = pyVSPAero(vsp_file, options={"solver_type": "Panel"}, s_ref=56.25, b_ref=3.0, c_ref=3.0)
        # Add output functions
        VLMAssembler.add_function("CL", functions.CL)
        VLMAssembler.add_function("CD", functions.CD)
        VLMAssembler.add_function("CMx", functions.CMX)
        VLMAssembler.add_function("CMy", functions.CMY)
        VLMAssembler.add_function("CMz", functions.CMZ)
        VLMAssembler.add_function("CLi", functions.CLi)
        VLMAssembler.add_function("CDi", functions.CDi)
        VLMAssembler.add_function("CMxi", functions.CMXi)
        VLMAssembler.add_function("CMyi", functions.CMYi)
        VLMAssembler.add_function("CMzi", functions.CMZi)

        # Create a steady problem instance
        prob1 = VLMAssembler.create_steady_problem("incompressible", aoa=5.0, x_cg=15.0)
        prob2 = VLMAssembler.create_steady_problem("compressible", aoa=6.0, x_cg=15.0, mach=0.8)
        all_problems = [prob1, prob2]
        return all_problems
