import os

import numpy as np
from vspaero import pyVSPAero, functions
from pyvspaero_analysis_base_test import PyVSPAeroTestCase

"""
Tests a steady rectangular wing analysis using the panel solver.
Test includes two cases: an incompressible (M=0.0) and compressible analysis (M=0.8).
Tests inviscid/viscous function values and sensitivities against FD.
"""

base_dir = os.path.dirname(os.path.abspath(__file__))
vsp_file = os.path.join(base_dir, "./input_files/rect_wing.vsp3")

class ProblemTest(PyVSPAeroTestCase.PyVSPAeroTest):
    FUNC_REFS = {'incompressible_CL': np.array([0.52339084]),
                 'incompressible_CD': np.array([0.02484065]),
                 'incompressible_CMx': np.array([-1.76239215e-05]),
                 'incompressible_CMy': np.array([0.02427089]),
                 'incompressible_CMz': np.array([-2.01219606e-08]),
                 'incompressible_CLi': np.array([0.52452745]),
                 'incompressible_CDi': np.array([0.01152177]),
                 'incompressible_CMxi': np.array([-1.76234806e-05]),
                 'incompressible_CMyi': np.array([0.02414508]),
                 'incompressible_CMzi': np.array([-9.22291076e-08]),
                 'compressible_CL': np.array([0.89851768]),
                 'compressible_CD': np.array([0.04928578]),
                 'compressible_CMx': np.array([-2.42831114e-05]),
                 'compressible_CMy': np.array([0.0446151]),
                 'compressible_CMz': np.array([3.30438716e-07]),
                 'compressible_CLi': np.array([0.90031468]),
                 'compressible_CDi': np.array([0.03167503]),
                 'compressible_CMxi': np.array([-2.42832223e-05]),
                 'compressible_CMyi': np.array([0.04445326]),
                 'compressible_CMzi': np.array([1.91943972e-07])}

    def setup_vspaero_problems(self):
        """
        Setup vspaero problems objects that describe different problem types we will be testing.
        Must be defined in child class that inherits from this class.
        """
        VLMAssembler = pyVSPAero(vsp_file, options={"solver_type": "Panel"})
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
