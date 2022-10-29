# distutils: language = c++

from VSP_Optimizer cimport VSP_OPTIMIZER
import functions

cdef extern from "Solver/VSP_Optimizer.H":
    cdef int PYTHON_ARRAYS

cdef extern from "omp.h":
    cdef int omp_get_max_threads()

cimport numpy as np
import numpy as np

# This decorator activates and deactivates the Adept stack immediately before and after a VSP_Optimizer call
# This is necessary to prevent conflicts between different stacks when multiple VSP_Optimizer instance are being used
cdef _adept_stack_safe(method):
    def stack_safe_method(self, *args, **kwargs):
        self._activate_auto_diff_stack()
        if self.debug_print:
            # Run w/ vspaero stdout turned on
            out = method(self, *args, **kwargs)
        else:
            # Run w/ vspaero stdout turned off
            self._suppress_stdout()
            out = method(self, *args, **kwargs)
            self._resume_stdout()
        self._deactivate_auto_diff_stack()
        return out
    return stack_safe_method

cdef class pyVSPOptimizer:
    cdef VSP_OPTIMIZER* vsp_optimizer  # Holds a C++ instance we're wrapping.
    cdef int _debug_print
    cdef int _is_setup

    def __cinit__(self):
        self.vsp_optimizer = new VSP_OPTIMIZER()
        self._debug_print = False
        self._is_setup = False

    def __dealloc__(self):
        del self.vsp_optimizer

    def __init__(self):
        # Always want "PYTHON_ARRAYS" (arrays indexed from 0 to N-1).
        self.vsp_optimizer.SetArrayOffSetType(PYTHON_ARRAYS)
        # Default the number of threads to max threads
        cdef int nthreads = omp_get_max_threads()
        self.vsp_optimizer.SetNumberOfThreads(nthreads)
        self.vsp_optimizer.DeactivateAutoDiffStack()

    @property
    def debug_print(self):
        return self._debug_print

    # a setter function
    def set_debug_print(self, flag):
        self._debug_print = flag

    @_adept_stack_safe
    def setup(self, fname_prefix):
        fp_bytes = fname_prefix.encode('utf-8') + b'\x00'
        cdef char* fp_cstring = fp_bytes
        self.vsp_optimizer.Setup(fp_cstring)
        self._is_setup = True

    @property
    def is_setup(self):
        return self._is_setup

    def _activate_auto_diff_stack(self):
        return self.vsp_optimizer.ActivateAutoDiffStack()

    def _deactivate_auto_diff_stack(self):
        return self.vsp_optimizer.DeactivateAutoDiffStack()

    def _suppress_stdout(self):
        return self.vsp_optimizer.SupressStdout()

    def _resume_stdout(self):
        return self.vsp_optimizer.ResumeStdout()

    @_adept_stack_safe
    def get_number_of_threads(self):
        return self.vsp_optimizer.GetNumberOfThreads()

    @_adept_stack_safe
    def set_number_of_threads(self, int n):
        self.vsp_optimizer.SetNumberOfThreads(n)

    @_adept_stack_safe
    def get_unsteady_analysis(self):
        return self.vsp_optimizer.GetUnsteadyAnalysis()

    @_adept_stack_safe
    def set_unsteady_analysis(self, int i):
        self.vsp_optimizer.SetUnsteadyAnalysis(i)

    def get_number_of_nodes(self):
        return self.vsp_optimizer.NumberOfNodes()

    @_adept_stack_safe
    def get_number_of_loops(self):
        return self.vsp_optimizer.NumberOfLoops()

    @_adept_stack_safe
    def get_number_of_optimization_functions(self):
        return self.vsp_optimizer.GetNumberOfOptimizationFunctions()

    @_adept_stack_safe
    def set_number_of_optimization_functions(self, int n):
        self.vsp_optimizer.SetNumberOfOptimizationFunctions(n)

    @_adept_stack_safe
    def update_geometry(self, np.ndarray[double, ndim=2] nodes):
        # nodes should be a 2D array of shape (self.nnodes, 3).
        # The `<double *> nodes.data` appears to flatten `nodes`, which is nice.
        self.vsp_optimizer.UpdateGeometry(<double *> nodes.data)

    @_adept_stack_safe
    def get_optimization_function(self, c):
        o = self.vsp_optimizer.GetOptimizationFunction(c+1)
        return o

    @_adept_stack_safe
    def set_optimization_function(self, c, func_id):
        self.vsp_optimizer.SetOptimizationFunction(c+1, func_id)

    @_adept_stack_safe
    def get_optimization_functions(self):
        noutputs = self.get_number_of_optimization_functions()
        o = np.empty(noutputs, dtype=int)
        for c in range(noutputs):
            o[c] = self.vsp_optimizer.GetOptimizationFunction(c+1)
        return o

    @_adept_stack_safe
    def set_optimization_functions(self, np.ndarray[int, ndim=1] func_ids):
        self.set_number_of_optimization_functions(len(func_ids))
        for c in range(len(func_ids)):
            self.vsp_optimizer.SetOptimizationFunction(c+1, func_ids[c])

    @_adept_stack_safe
    def get_optimization_function_length(self, c):
        return self.vsp_optimizer.OptimizationFunctionLength(c+1)

    @_adept_stack_safe
    def get_optimization_sets(self):
        noutputs = self.get_number_of_optimization_functions()
        s = np.empty(noutputs, dtype=int)
        for c in range(noutputs):
            s[c] = self.vsp_optimizer.GetOptimizationSet(c+1)
        return s

    @_adept_stack_safe
    def get_optimization_number_of_time_steps(self):
        noutputs = self.get_number_of_optimization_functions()
        nt = np.empty(noutputs, dtype=int)
        for c in range(noutputs):
            nt[c] = self.vsp_optimizer.OptimizationNumberOfTimeSteps(c+1)
        return nt

    @_adept_stack_safe
    def solve(self):
        self.vsp_optimizer.Solve()

    @_adept_stack_safe
    def solve_forward(self):
        self.vsp_optimizer.SolveForward()

    @_adept_stack_safe
    def set_gradient_vector(self, c, np.ndarray[double, ndim=1] vec):
        self.vsp_optimizer.SetGradientVector(c+1, <double *> vec.data)

    @_adept_stack_safe
    def solve_adjoint(self):
        self.vsp_optimizer.SolveAdjoint()

    @_adept_stack_safe
    def calculate_mat_vec_prod_rhs(self, np.ndarray[double, ndim=1] vec_in, np.ndarray[double, ndim=1] vec_out, np.ndarray[double, ndim=1] rhs):
        # TODO: check that all of these vectors have len == self.nloops.
        # TODO: how do we tell what case/optimization function we're working with?
        self.vsp_optimizer.CalculateMatrixVectorProductAndRightHandSide(<double *> vec_in.data, <double *> vec_out.data, <double *> rhs.data)

    @_adept_stack_safe
    def calculate_adjoint_mat_vec_prod_rhs(self, np.ndarray[double, ndim=1] vec_in, np.ndarray[double, ndim=1] vec_out, np.ndarray[double, ndim=1] rhs):
        # TODO: check that all of these vectors have len == self.nloops.
        # TODO: how do we tell what case/optimization function we're working with?
        self.vsp_optimizer.CalculateAdjointMatrixVectorProductAndRightHandSide(<double *> vec_in.data, <double *> vec_out.data, <double *> rhs.data)

    @_adept_stack_safe
    def get_function_value(self, int c, np.ndarray[double, ndim=1] vec):
        # TODO: just using the vector form of VSP_Optimizer.GetFunctionValue.
        return self.vsp_optimizer.GetFunctionValue(c+1, <double *> vec.data)

    @_adept_stack_safe
    def get_unsteady_function_value(self, c, t, np.ndarray[double, ndim=1] vec):
        # TODO: just using the vector form of VSP_Optimizer.GetFunctionValue.
        return self.vsp_optimizer.GetUnsteadyFunctionValue(c+1, t, <double *> vec.data)

    @_adept_stack_safe
    def get_function_gradients(self, c, np.ndarray[double, ndim=1] grads):
        self.vsp_optimizer.GetFunctionGradients(c+1, <double *> grads.data)

    # TODO: does this get us just the geometry for the optimization set (aka the body we're calculating the optimization function for)?
    @_adept_stack_safe
    def node_x(self, i):
        return self.vsp_optimizer.NodeX(i)

    @_adept_stack_safe
    def node_y(self, i):
        return self.vsp_optimizer.NodeY(i)

    @_adept_stack_safe
    def node_z(self, i):
        return self.vsp_optimizer.NodeZ(i)

    @_adept_stack_safe
    def set_geometry(self, np.ndarray[double, ndim=1] xyz):
        self.vsp_optimizer.UpdateGeometry(<double *> xyz.data)

    @_adept_stack_safe
    def get_geometry(self):
        cdef int nnodes = self.get_number_of_nodes()
        xyz = np.zeros(3 * nnodes)
        cdef double[:] xyz_view = xyz
        for i in range(nnodes):
            xyz_view[3 * i + 0] = self.vsp_optimizer.NodeX(i)
            xyz_view[3 * i + 1] = self.vsp_optimizer.NodeY(i)
            xyz_view[3 * i + 2] = self.vsp_optimizer.NodeZ(i)
        return xyz

    @_adept_stack_safe
    def gradient_x(self, c, i):
        return self.vsp_optimizer.GradientX(c+1, i)

    @_adept_stack_safe
    def gradient_y(self, c, i):
        return self.vsp_optimizer.GradientY(c+1, i)

    @_adept_stack_safe
    def gradient_z(self, c, i):
        return self.vsp_optimizer.GradientZ(c+1, i)

    @_adept_stack_safe
    def func_input_gradients(self, c, var_id):
        # TODO: this is not a great name. I think `dF_dInputVariable` returns the derivative of the output function wrt settings in the .vspaero input file (eg rotor RPM, freestream velocity).
        # Convert sens from rad to deg
        if var_id in [functions.WRT_ALPHA, functions.WRT_BETA]:
            return (np.pi/180.0) * self.vsp_optimizer.dF_dInputVariable(c+1, var_id)
        else:
            return self.vsp_optimizer.dF_dInputVariable(c+1, var_id)

    @_adept_stack_safe
    def unsteady_func_input_gradients(self, c, t, var_id):
        # TODO: this is not a great name. I think `dF_dInputVariable` returns the derivative of the output function wrt settings in the .vspaero input file (eg rotor RPM, freestream velocity).
        return self.vsp_optimizer.dF_dInputVariable(c+1, t, var_id)

    @_adept_stack_safe
    def set_mach_number(self, mach):
        self.vsp_optimizer.SetMachNumber(mach)

    @_adept_stack_safe
    def set_aoa(self, alpha):
        self.vsp_optimizer.SetAoADegrees(alpha)

    @_adept_stack_safe
    def set_beta(self, beta):
        self.vsp_optimizer.SetBetaDegrees(beta)

    @_adept_stack_safe
    def set_vinf(self, vinf):
        self.vsp_optimizer.SetVinf(vinf)

    @_adept_stack_safe
    def set_density(self, rho):
        self.vsp_optimizer.SetDensity(rho)

    @_adept_stack_safe
    def set_recref(self, recref):
        self.vsp_optimizer.SetReCref(recref)

    @_adept_stack_safe
    def set_rotational_rate(self, np.ndarray[double, ndim=1] omega):
        p = omega[0]
        q = omega[1]
        r = omega[2]
        self.vsp_optimizer.SetRotationalRate_p(p)
        self.vsp_optimizer.SetRotationalRate_q(q)
        self.vsp_optimizer.SetRotationalRate_r(r)

    @_adept_stack_safe
    def set_sref(self, sref):
        self.vsp_optimizer.SetSref(sref)

    @_adept_stack_safe
    def set_cref(self, cref):
        self.vsp_optimizer.SetCref(cref)

    @_adept_stack_safe
    def set_bref(self, bref):
        self.vsp_optimizer.SetBref(bref)

    @_adept_stack_safe
    def set_x_cg(self, x):
        self.vsp_optimizer.SetXcg(x)

    @_adept_stack_safe
    def set_y_cg(self, y):
        self.vsp_optimizer.SetYcg(y)

    @_adept_stack_safe
    def set_z_cg(self, z):
        self.vsp_optimizer.SetZcg(z)

    @_adept_stack_safe
    def set_clo2d(self, clo2d):
        self.vsp_optimizer.SetClo2D(clo2d)

    @_adept_stack_safe
    def set_clmax(self, clmax):
        self.vsp_optimizer.SetClMax(clmax)

    @_adept_stack_safe
    def set_wake_iterations(self, wake_iters):
        self.vsp_optimizer.SetWakeIterations(wake_iters)

    @_adept_stack_safe
    def set_num_wake_nodes(self, num_nodes):
        self.vsp_optimizer.SetNumWakeNodes(num_nodes)

    @_adept_stack_safe
    def set_far_dist(self, far_dist):
        self.vsp_optimizer.SetFarDist(far_dist)

    @_adept_stack_safe
    def set_gmres_tolerance(self, tol):
        self.vsp_optimizer.SetGMRESToleranceFactor(tol)
