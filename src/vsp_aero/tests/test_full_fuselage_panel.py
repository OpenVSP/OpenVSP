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
    FUNC_REFS = {'incompressible_CL': np.array([-0.00365325]),
                 'incompressible_CD': np.array([0.02038826]),
                 'incompressible_CMx': np.array([6.92285735e-16]),
                 'incompressible_CMy': np.array([0.11805778]),
                 'incompressible_CMz': np.array([-1.41161157e-14]),
                 'incompressible_CLi': np.array([-0.00365325]),
                 'incompressible_CDi': np.array([-0.00025616]),
                 'incompressible_CMxi': np.array([6.92285735e-16]),
                 'incompressible_CMyi': np.array([0.11805778]),
                 'incompressible_CMzi': np.array([-1.41161157e-14]),
                 'compressible_CL': np.array([-0.01505233]),
                 'compressible_CD': np.array([0.0191199]),
                 'compressible_CMx': np.array([2.26436154e-15]),
                 'compressible_CMy': np.array([-0.48437012]),
                 'compressible_CMz': np.array([-4.44211581e-13]),
                 'compressible_CLi': np.array([-0.01505233]),
                 'compressible_CDi': np.array([-0.00152452]),
                 'compressible_CMxi': np.array([2.26436154e-15]),
                 'compressible_CMyi': np.array([-0.48437012]),
                 'compressible_CMzi': np.array([-4.44211581e-13])}

    def setup_vspaero_problems(self):
        """
        Setup vspaero problems objects that describe different problem types we will be testing.
        Must be defined in child class that inherits from this class.
        """
        VLMAssembler = pyVSPAero(vsp_file, options={"solver_type": "Panel"}, sref=56.25, bref=3.0, cref=3.0)
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
