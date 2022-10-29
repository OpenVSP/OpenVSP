import os
import numpy as np
from vspaero import pyVSPAero, functions
from pyvspaero_analysis_base_test import PyVSPAeroTestCase

"""
Tests a steady rectangular wing analysis using VLM solver with symmetry conditions turned on for xz plane.
Test includes two cases: an incompressible (M=0.0) and compressible analysis (M=0.8).
Tests inviscid/viscous function values and sensitivities against FD.
"""

base_dir = os.path.dirname(os.path.abspath(__file__))
vsp_file = os.path.join(base_dir, "./input_files/rect_wing.vsp3")

class ProblemTest(PyVSPAeroTestCase.PyVSPAeroTest):
    FUNC_REFS = {'incompressible_CL': np.array([0.45605449]),
                 'incompressible_CD': np.array([0.01778004]),
                 'incompressible_CMx': np.array([0.]),
                 'incompressible_CMy': np.array([0.03634075]),
                 'incompressible_CMz': np.array([0.]),
                 'incompressible_CLi': np.array([0.45709313]),
                 'incompressible_CDi': np.array([0.00624397]),
                 'incompressible_CMxi': np.array([0.]),
                 'incompressible_CMyi': np.array([0.03634069]),
                 'incompressible_CMzi': np.array([0.]),
                 'compressible_CL': np.array([0.79712391]),
                 'compressible_CD': np.array([0.03305776]),
                 'compressible_CMx': np.array([0.]),
                 'compressible_CMy': np.array([0.06674775]),
                 'compressible_CMz': np.array([0.]),
                 'compressible_CLi': np.array([0.79866614]),
                 'compressible_CDi': np.array([0.01874219]),
                 'compressible_CMxi': np.array([0.]),
                 'compressible_CMyi': np.array([0.06674767]),
                 'compressible_CMzi': np.array([0.])}

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
        prob1 = VLMAssembler.create_steady_problem("incompressible", aoa=5.0, x_cg=0.25, options={"symmetry": "Y"})
        prob2 = VLMAssembler.create_steady_problem("compressible", aoa=6.0, x_cg=0.25, mach=0.8, options={"symmetry": "Y"})
        all_problems = [prob1, prob2]
        return all_problems
