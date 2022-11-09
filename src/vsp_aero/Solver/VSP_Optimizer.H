//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////
#ifndef VSP_OPTIMIZER_H
#define VSP_OPTIMIZER_H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <fcntl.h>

#ifndef WIN32
#include <sys/wait.h>
#include <unistd.h>
#else
#define open            _open
#define O_WRONLY        _O_WRONLY
#define close           _close
#endif

#include "VSPAERO_TYPES.H"
#include "VSP_Solver.H"
#include "ControlSurfaceGroup.H"

#undef BOUNDARY_CONDITION_DATA_H
#undef COMPONENT_GROUP_H
#undef CONTROL_SURFACE_H
#undef CONTROL_SURFACE_GROUP_H
#undef ENGINE_FACE_H
#undef FEM_NODE_H
#undef GRADIENT_H
#undef INTERACTION_CLASSES_H
#undef LOOP_INTERACTION_ENTRY_H
#undef MATPRECON_H
#undef MERGESORT_H
#undef QUAD_CELL_H
#undef QUAD_EDGE_H
#undef QUAD_NODE_H
#undef QUAD_TREE_H
#undef ROTOR_DISK_H
#undef SEARCH_H
#undef SEARCH_LEAF_H
#undef SPAN_LOAD_DATA_H
#undef SPAN_LOAD_ROTOR_DATA_H
#undef VSPAERO_TYPES_H
#undef VSP_AGGLOM_H
#undef VSP_EDGE_H
#undef VSP_GEOM_H
#undef VSP_GRID_H
#undef VSP_LOOP_H
#undef VSP_NODE_H
#undef VSP_SOLVER_H
#undef VSP_SURFACE_H
#undef VORTEX_SHEET_LOOP_INTERACTION_ENTRY_H
#undef VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET_H
#undef VORTEX_BOUND_H
#undef VORTEX_SHEET_H
#undef VORTEX_TRAIL_H
#undef WOPWOP_H
#undef BINARYIO_H
#undef MATRIX_H
#undef QUAT_H
#undef UTILS_H
#undef VSPAERO_DOUBLE
#undef AUTODIFF_IS_OFF
#undef OPTIMIZATION_FUNCTION_H

#define AUTODIFF
#include "VSPAERO_TYPES.H"
#include "VSP_Solver.H"
#include "ControlSurfaceGroup.H"
#undef AUTODIFF

#define MAXRUNCASES 10

#define DEFAULT_ARRAYS 0
#define PYTHON_ARRAYS  1

class VSP_OPTIMIZER {

private:

    char FileName_[2000];
    
    double Sref_;
    double Cref_;
    double Bref_;
    double Xcg_;
    double Ycg_;
    double Zcg_;
    double Mach_;
    double AoA_;
    double Beta_;
    double Rho_;
    double Vinf_;
    double Vref_;
    double Machref_;
    double ReCref_;
    double ClMax_;
    double Clo2D_;
    double MaxTurningAngle_;
    double FarDist_;
    double TimeStep_;
    double HeightAboveGround_;
    double BladeRPM_;
    double WakeRelax_;
    double GMRESReductionFactor_;
    double CoreSizeFactor_;
        
    // Number of Machs, AoAs, and Betas
    
    int NumberOfMachs_;
    int NumberOfAoAs_;
    int NumberOfBetas_;
    int NumberOfReCrefs_;
    
    // Mach, AoA, and Beta Lists
    
    double   MachList_[MAXRUNCASES];
    double    AoAList_[MAXRUNCASES];
    double   BetaList_[MAXRUNCASES];
    double ReCrefList_[MAXRUNCASES];
    
    // Control surfaces
    
    int NumberOfControlGroups_;
    VSPAERO_SOLVER::CONTROL_SURFACE_GROUP *ControlSurfaceGroup_;
    
    int WakeIterations_                ;
    int NumberOfRotors_                ;
    int NumStabCases_                  ;
    int NumberOfThreads_               ;
    int StabControlRun_                ;
    int SetFreeStream_                 ;
    int SaveRestartFile_               ;
    int DoRestartRun_                  ;
    int DoSymmetry_                    ;
    int SetFarDist_                    ;
    int Symmetry_                      ;
    int NumberOfWakeNodes_             ;
    int DumpGeom_                      ;
    int NoWakeIteration_               ;
    int NumberofSurveyPoints_          ;
    int NumberOfSurveyTimeSteps_       ;
    int LoadFEMDeformation_            ;
    int DoGroundEffectsAnalysis_       ;
    int Write2DFEMFile_                ;
    int DoUnsteadyAnalysis_            ;
    int StartFromSteadyState_          ;
    int UnsteadyAnalysisType_          ;
    int NumberOfTimeSteps_             ;
    int NumberOfTimeSamples_           ;
    int RotorAnalysisRun_              ;
    int CreateNoiseFiles_              ;
    int NoPanelSpanWiseLoading_        ;
    int DoAdjointSolve_                ;
    int OptimizationSolve_             ;
    int SetupHighLiftFile_             ;
    int RestartAndInterrogateSolution_ ;
    int NumberOfQuadTrees_             ;
    int NumberOfInlets_                ;
    int NumberOfNozzles_               ;
    int DoComplexDiffTest              ;
    int DoFiniteDiffTest               ;
    int NumberOfOptimizationFunctions_ ;
    int OptimizationFunction_[1001]    ;
    int OptimizationSet_[1001]         ;

    template <typename T>
    void LoadCaseFile(T &VSP);
    
    double DOUBLE(double a);
    double DOUBLE(adept::adouble a);
    
    VSPAERO_SOLVER::VSP_SOLVER *Solver_;
    
    VSPAERO_SOLVER::VSP_SOLVER &Solver(void) { return Solver_[0]; };

    adept::Stack AutoDiffStack;

    VSPAERO_ADJOINT::VSP_SOLVER *Adjoint_;
    
    VSPAERO_ADJOINT::VSP_SOLVER &Adjoint(void) { return Adjoint_[0]; };
    
    int ArrayOffSet_;
    int MaxTempArraySize_;
    
    double *TempArray_[3];

    int stdoutfd;

    void ShiftInputVector(double *Vec1, double *Vec2, int Length);
    void ShiftOutputVector(double *Vec1, double *Vec2, int Length);

public:

    VSP_OPTIMIZER(void);
   ~VSP_OPTIMIZER(void);
    VSP_OPTIMIZER(const VSP_OPTIMIZER &Optimizer);

    /** Activate/deactivate the Adept stack in case we may be running multiple optimizer insatances**/
    void ActivateAutoDiffStack(void) { AutoDiffStack.activate(); };
    void DeactivateAutoDiffStack(void) { AutoDiffStack.deactivate(); };

    /** Set the number of CPU threads used by OPENMP in forward solve **/
    
    int &NumberOfThreads(void) { return NumberOfThreads_; };
    // Cython doesn't support treating returned references as lvalues: https://cython.readthedocs.io/en/latest/src/userguide/wrapping_CPlusPlus.html#c-left-values
    int GetNumberOfThreads(void) { return NumberOfThreads_; };
    void SetNumberOfThreads(int i) { NumberOfThreads_ = i; };
    
    /** Tell the solvers this is an unsteady, time accurate, solve **/
    
    int &DoUnsteadyAnalysis(void) { return DoUnsteadyAnalysis_; };
    // Cython doesn't support treating returned references as lvalues: https://cython.readthedocs.io/en/latest/src/userguide/wrapping_CPlusPlus.html#c-left-values
    int GetUnsteadyAnalysis(void) { return DoUnsteadyAnalysis_; };
    void SetUnsteadyAnalysis(int i) { DoUnsteadyAnalysis_ = i; };
    
    /** Number of mesh nodes **/
    
    int NumberOfNodes(void) { return Solver().VSPGeom().Grid(0).NumberOfNodes(); };
    
    /** Number Of mesh loops **/
    
    int NumberOfLoops(void) { return Solver().VSPGeom().Grid(1).NumberOfLoops(); };
    
    /** Update the geometry (mesh) **/
    
    void UpdateGeometry(double *NodeXYZ);
        
    /** Set number of optimization functions.. ie, CL, CD, CT, SpanLoading **/
    
    int &NumberOfOptimizationFunctions(void) { return NumberOfOptimizationFunctions_; };
    // Cython doesn't support treating returned references as lvalues: https://cython.readthedocs.io/en/latest/src/userguide/wrapping_CPlusPlus.html#c-left-values
    int GetNumberOfOptimizationFunctions(void) { return NumberOfOptimizationFunctions_; };
    void SetNumberOfOptimizationFunctions(int n) { NumberOfOptimizationFunctions_ = n; };
 
    /** Set the Optimization function for case i
     * 
     * Usage OptimizationFunction(Case) = Value, where Value is one of the following:
     *
     * OPT_CL:           Vehicle Lift Force Coefficient
     * OPT_CD:           Vehicle Drag Force Coefficient
     * OPT_CS:           Vehicle Side Force Coefficient
     *                  
     * OPT_CMx:          Vehicle Pitching Moment about the X axis
     * OPT_CMy:          Vehicle Pitching Moment about the Y axis
     * OPT_CMz:          Vehicle Pitching Moment about the Z axis
     *                  
     * OPT_CD_CL_CM:     Trim Vehicle, Minimize Drag, and Drive CL to user specified value
     *                  
     * OPT_ROTOR_CT:     Rotor Thrust Coefficient
     * OPT_ROTOR_CP:     Rotor Power Coefficient
     *                  
     * OPT_WING_LOAD:    Spanwise wing load, ie Cl vs Span
     *                  
     * OPT_RESIDUAL:     Residual vector for linear system
     *
     * OPT_WING_CX:      X component of spanwise wing load
     * OPT_WING_CY:      Y component of spanwise wing load
     * OPT_WING_CZ:      Z component of spanwise wing load
     *
     **/
             
    int &OptimizationFunction(int Case) { return OptimizationFunction_[Case]; };
    // Cython doesn't support treating returned references as lvalues: https://cython.readthedocs.io/en/latest/src/userguide/wrapping_CPlusPlus.html#c-left-values
    int GetOptimizationFunction(int Case) { return OptimizationFunction_[Case]; };
    void SetOptimizationFunction(int Case, int Value) { OptimizationFunction_[Case] = Value; };
    /** Set the optimization set, this may be the wing set, or rotor set **/
    
    int &OptimizationSet(int Case) { return OptimizationSet_[Case]; };
    int GetOptimizationSet(int Case) { return OptimizationSet_[Case]; };
    void SetOptimizationSet(int Case, int Value) { OptimizationSet_[Case] = Value; };
        
    /** Get function length for optimization function # Case...  **/
    
    int OptimizationFunctionLength(int Case) { return Solver().OptimizationFunctionLength(Case); };

    /** Get total vector length for optimization function # Case... this is generally the function length X Number of time sample points for an unsteady case **/
    
    int OptimizationNumberOfTimeSteps(int Case) { return Solver().OptimizationNumberOfTimeSteps(Case); };
    
    /** Setup everything for both the forward solver and adjoint **/
    
    void Setup(char *FileName);
    
    /** Set the free stream Mach number ***/
    
    void SetMachNumber(double Mach);
    
    /** Set the free stream angle of attack **/
    
    void SetAoADegrees(double Alpha);
    
    /*** Set free stream side slip angle ***/
    
    void SetBetaDegrees(double Beta);
    
    /** Set free stream velocity **/
    
    void SetVinf(double Vinf);
    
    /** Set free stream density **/
    
    void SetDensity(double Density);
    
    /** Set Reynold's number based on Cref **/
    
    void SetReCref(double ReCref);
    
    /** Set roll rate about x axis **/
    
    void SetRotationalRate_p(double P);
    
    /** Set roll rate about y axis ***/
    
    void SetRotationalRate_q(double Q);
    
    /** Set roll rate about z axis **/
    
    void SetRotationalRate_r(double R);
    
    /** Set wing reference area **/
    
    void SetSref(double Sref);
    
    /** Set wing reference chord **/
    
    void SetCref(double Cref);
    
    /** Set wing reference span **/
    
    void SetBref(double Bref);
    
    /** Set wing X cg location **/
    
    void SetXcg(double Xcg);
    
    /** Set wing Y cg location **/
    
    void SetYcg(double Ycg);
    
    /** Set wing Z cg location **/
    
    void SetZcg(double Zcg);
    
    /** Set 2D CLo offset for viscous drag estimates on wings 
     * Ie... Cdvisc ~ k*(Cl-Clo2D)^2 ... hence Clo2D allows the
     * user to offset the minimum viscous profile drag for lifting
     * surfaces to a non zero (local/2d) Cl **/
    
    void SetClo2D(double Clo2D);
    
    /** Set 2D ClMax limit flag 
     * if ClMax <  0.   ... no ClMax limit applied 
     * if ClMax >= 0.   ... ClMax is the 2D ClMax limit for wing sectional Cl's 
     * if ClMax == -999 ... ClMax limited is calculated internally to the code based
     * on the local Re number (based on ReCref and the local chord) and a limit
     * local Mach number based loosely on Carlson's methods **/
    
    void SetClMax(double ClMax);
    
    /** Set number of wake iterations **/
    
    void SetWakeIterations(int WakeIterations);
    
    /** Set number of wake nodes, this must be a power of 2... ie 32, 64, 128 ... **/
    
    void SetNumWakeNodes(int NumWakeNodes);
    
    /** Set wake far distance ... this is the distance from the trailing edge that is in the adapted wake region **/
    
    void SetFarDist(double FarDist);

    /** Set tolerance factor for GMRES solver, this scales the default residual reduction
     * ie... ResidualReduction = DefaultResidualReduction * User_GMRES_ToleranceFactor_**/

    void SetGMRESToleranceFactor(double tolFactor);
    
    /** Set the symmetry flag 
     * N ~ no symmetry
     * X ~ symmetry about X = 0
     * Y ~ symmetry about Y = 0
     * Z ~ symmetry about Z = 0
     * 
     **/

    /** Solve both the forward solve and adjoint problem **/
    
    void Solve(void);

    /** Solve the forward problem **/
    
    void SolveForward(void);
    
    /** Set any user supplied initial gradient vectors ... this should be called after SolveForward() and before SolveAdjoint() 
     * partly because you will need to know the length for Vec and you won't until you run the forward solve ... **/
    
    void SetGradientVector(int Case, double *Vec);
        
    /** Solve the adjoint problem **/
    
    void SolveAdjoint(void);
    
    /** Do a matrix-vector multiply and get the RHS for the forward problem... please do a forward solve first! **/
    
    void CalculateMatrixVectorProductAndRightHandSide(double *VecIn, double *VecOut, double *RHS);
    
    /** Do a adjoint matrix-vector multiply and get the RHS for the forward problem... please do a forward and adjoint solve first! **/
    
    void CalculateAdjointMatrixVectorProductAndRightHandSide(double *VecIn, double *VecOut, double *RHS);
    
    /** Get the function value for a single case run, this returns a scalar in Vec **/

    void GetFunctionValue(double &Vec);    

    /** Get the function value for optimization run # Case, this returns a scalar in Vec **/
    
    void GetFunctionValue(int Case, double &Vec);

    /** Get the function value for a single case run, this returns a vector in Vec **/

    void GetFunctionValue(double *Vec);    
    
    /** Get the function value for optimization run # Case, this returns a vector in Vec **/
    
    void GetFunctionValue(int Case, double *Vec);

    /** Get the unsteady function value for optimization run # Case, and integration time step TIME...this returns a vector in Vec **/
    
    void GetUnsteadyFunctionValue(int Case, int Time, double *Vec);
    
    /** Get the unsteady function value for optimization run # 1, and integration time step Time...this returns a vector in Vec **/
    
    void GetUnsteadyFunctionValue(int Time, double *Vec);    
            
    /** Get gradients for single case run, Gradients is a 3*NumberOfNodes length vector containing gradients wrt xyz **/
        
    void GetFunctionGradients(double *Gradients);
    
    /** Get gradients for optimization run # Case, Gradients is a 3*NumberOfNodes length vector containing gradients wrt xyz **/
    
    void GetFunctionGradients(int Case, double *Gradients);
    
    /** Get gradients for optimization run # Case and integration time step Time, Gradients is a 3*NumberOfNodes length vector containing gradients wrt xyz **/
    
    void GetFunctionGradients(int p, int Time, double *Gradients);

    /** Component ID of node i **/
    
    int NodeComponentID(int i) { return Solver().VSPGeom().Grid(0).NodeList(i + ArrayOffSet_).ComponentID(); };
    
    /** Surface ID of node i **/
    
    int NodeSurfaceID(int i) { return Solver().VSPGeom().Grid(0).NodeList(i + ArrayOffSet_).SurfaceID(); };
    
    /** X coordinate of node i **/
    
    double NodeX(int i) { return Solver().VSPGeom().Grid(0).NodeList(i + ArrayOffSet_).x(); };
    
    /** Y coordinate of node i **/
    
    double NodeY(int i) { return Solver().VSPGeom().Grid(0).NodeList(i + ArrayOffSet_).y(); };
    
    /** Z coordinate of node i **/
    
    double NodeZ(int i) { return Solver().VSPGeom().Grid(0).NodeList(i + ArrayOffSet_).z(); };
    
    /** Gradient of objective function wrt x for node i **/

    double GradientX(int i) { return Adjoint().dF_dMesh_X(1,i + ArrayOffSet_); };
    
    /** Gradient of objective function wrt y for node i **/
    
    double GradientY(int i) { return Adjoint().dF_dMesh_Y(1,i + ArrayOffSet_); };

    /** Gradient of objective function wrt z for node i **/

    double GradientZ(int i) { return Adjoint().dF_dMesh_Z(1,i + ArrayOffSet_); };

    /** Gradient of objective function # Case, wrt x for node i **/

    double GradientX(int Case, int i) { return Adjoint().dF_dMesh_X(Case,i + ArrayOffSet_); };
    
    /** Gradient of objective function # Case, wrt y for node i **/
    
    double GradientY(int Case, int i) { return Adjoint().dF_dMesh_Y(Case,i + ArrayOffSet_); };

    /** Gradient of objective function # Case, wrt z for node i **/

    double GradientZ(int Case, int i) { return Adjoint().dF_dMesh_Z(Case,i + ArrayOffSet_); };

    /** Gradient of objective function # Case, wrt x for node i at averaging time, Time **/

    double GradientX(int Case, int Time, int i) { return Adjoint().dF_dMesh_X(Case,Time,i + ArrayOffSet_); };
    
    /** Gradient of objective function # Case, wrt y for node i at averaging time, Time **/
    
    double GradientY(int Case, int Time, int i) { return Adjoint().dF_dMesh_Y(Case,Time,i + ArrayOffSet_); };

    /** Gradient of objective function # Case, wrt z for node i at averaging time, Time **/

    double GradientZ(int Case, int Time, int i) { return Adjoint().dF_dMesh_Z(Case,Time,i + ArrayOffSet_); };
    
    /** Gradient of objective function wrt input variable **/

    double dF_dInputVariable(int InputVariable) { return Adjoint().dF_dInputVariable(InputVariable); };
        
    /** Gradient of objective function # Case, wrt input variable **/

    double dF_dInputVariable(int Case, int InputVariable) { return Adjoint().dF_dInputVariable(Case,InputVariable); };
    
    /** Gradient of objective function # Case, wrt input variable at averaging time, Time **/

    double dF_dInputVariable(int Case, int Time, int InputVariable) { return Adjoint().dF_dInputVariable(Case,Time,InputVariable); };
        
    /** Get the pressures at the nodes of the mesh, returns a vector of length NumberOfNodes **/
    
    void GetNodalPressures(double *Pressure);
    
    /** Calculate partial F wrt pressure for user supplied function F... both vectors are of length NumberOfNodes **/
    
    void GetpFupMesh(double *pFU_pP, double *pFU_pMesh);
    
    /** Set array assumptions
     * 
     * UsageCase PYTHON_ARRAYS  ... API arrays are assumed to run from 0 to N-1
     * UsageCase DEFAULT_ARRAYS ... API arrays are assumed to run from 1 to N
     * 
     **/
    
    void SetArrayOffSetType(int ArrayType) { ArrayOffSet_ = ArrayType; };
    
    /** Write out a cart3d tri file **/
    
    void WriteOutCart3dTriFile(void) { Solver().WriteOutCart3dTriFile(); };

    /** Suppress/resume stdout **/

    void SuppressStdout(void);
    void ResumeStdout(void);

};

#endif

