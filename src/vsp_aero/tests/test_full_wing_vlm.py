import os
import numpy as np
from vspaero import pyVSPAero, functions
from pyvspaero_analysis_base_test import PyVSPAeroTestCase

"""
Tests a steady rectangular wing analysis using VLM solver.
Test includes two cases: an incompressible (M=0.0) and compressible analysis (M=0.8).
Tests inviscid/viscous function values and sensitivities against FD.
"""

base_dir = os.path.dirname(os.path.abspath(__file__))
vsp_file = os.path.join(base_dir, "./input_files/rect_wing.vsp3")

class ProblemTest(PyVSPAeroTestCase.PyVSPAeroTest):

    FUNC_REFS = {'incompressible_CL': np.array([0.45570527]),
                 'incompressible_CD': np.array([0.01777621]),
                 'incompressible_CMx': np.array([3.52332335e-13]),
                 'incompressible_CMy': np.array([0.03623004]),
                 'incompressible_CMz': np.array([6.76424194e-15]),
                 'incompressible_CLi': np.array([0.45674373]),
                 'incompressible_CDi': np.array([0.00624219]),
                 'incompressible_CMxi': np.array([3.50692808e-13]),
                 'incompressible_CMyi': np.array([0.03622998]),
                 'incompressible_CMzi': np.array([8.26880231e-15]),
                 'compressible_CL': np.array([0.7959922]),
                 'compressible_CD': np.array([0.03303317]),
                 'compressible_CMx': np.array([-7.61046906e-15]),
                 'compressible_CMy': np.array([0.0663429]),
                 'compressible_CMz': np.array([-1.76259007e-14]),
                 'compressible_CLi': np.array([0.79753307]),
                 'compressible_CDi': np.array([0.01873021]),
                 'compressible_CMxi': np.array([-1.05071507e-14]),
                 'compressible_CMyi': np.array([0.06634282]),
                 'compressible_CMzi': np.array([-1.76406112e-14])}

    def setup_vspaero_problems(self):
        """
        Setup vspaero problems objects that describe different problem types we will be testing.
        Must be defined in child class that inherits from this class.
        """
        VLMAssembler = pyVSPAero(vsp_file)
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
        prob1 = VLMAssembler.create_steady_problem("incompressible", aoa=5.0, x_cg=0.25)
        prob2 = VLMAssembler.create_steady_problem("compressible", aoa=6.0, x_cg=0.25, mach=0.8)
        all_problems = [prob1, prob2]
        return all_problems
