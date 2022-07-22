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

#ifndef WIN32
#include <sys/wait.h>
#include <unistd.h>
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

class VSP_OPTIMIZER {

private:

    char FileName_[2000];
    
    double Sref_;
    double Cref_;
    double Bref_;
    double Xcg_;
    double Ycg_;
    double Zcg_;
    double Cl_;
    double Cd_;
    double Cm_;
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
    double ReducedFrequency_;
    double UnsteadyAngleMax_;
    double UnsteadyHMax_;     
    double HeightAboveGround_;
    double BladeRPM_;
    
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
    
    // Sability and control Mach, AoA, and Beta Lists
    
    double Stab_MachList_[MAXRUNCASES];
    double  Stab_AoAList_[MAXRUNCASES];
    double Stab_BetaList_[MAXRUNCASES];
    
    // Stability Rotational rates list
    
    double RotationalRate_pList_[MAXRUNCASES];
    double RotationalRate_qList_[MAXRUNCASES];
    double RotationalRate_rList_[MAXRUNCASES];
    
    // Deltas for calculating derivaties
    
    double Delta_AoA_;
    double Delta_Beta_;
    double Delta_Mach_;
    double Delta_P_;
    double Delta_Q_;
    double Delta_R_;
    double Delta_Control_;
    
    // Raw stability data
    
    double CFxForCase[MAXRUNCASES];
    double CFyForCase[MAXRUNCASES];
    double CFzForCase[MAXRUNCASES];
    
    double CMxForCase[MAXRUNCASES];
    double CMyForCase[MAXRUNCASES];
    double CMzForCase[MAXRUNCASES];
    
    double CLForCase[MAXRUNCASES];
    double CDForCase[MAXRUNCASES];
    double CSForCase[MAXRUNCASES];
    
    double CDtForCase[MAXRUNCASES];
    
    double CMlForCase[MAXRUNCASES];
    double CMmForCase[MAXRUNCASES];
    double CMnForCase[MAXRUNCASES];
    
    double CDoForCase[MAXRUNCASES];
    
    double OptimizationFunctionForCase[MAXRUNCASES];
    
    // Stability derivatives
    
    double dCFx_wrt[MAXRUNCASES];
    double dCFy_wrt[MAXRUNCASES];
    double dCFz_wrt[MAXRUNCASES];
    
    double dCMx_wrt[MAXRUNCASES];
    double dCMy_wrt[MAXRUNCASES];
    double dCMz_wrt[MAXRUNCASES];
    
    double dCL_wrt[MAXRUNCASES];
    double dCD_wrt[MAXRUNCASES];
    double dCS_wrt[MAXRUNCASES];
    
    double dCMl_wrt[MAXRUNCASES];
    double dCMm_wrt[MAXRUNCASES];
    double dCMn_wrt[MAXRUNCASES];
    
    FILE *StabFile;
    FILE *VorviewFlt;
    
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


public:

    VSP_OPTIMIZER(void);
   ~VSP_OPTIMIZER(void);
    VSP_OPTIMIZER(const VSP_OPTIMIZER &Optimizer);

    // Set the number of CPU threads
    
    int &NumberOfThreads(void) { return NumberOfThreads_; };
    
    // Tell the solvers this is an unsteady, time accurate, solve
    
    int &DoUnsteadyAnalysis(void) { return DoUnsteadyAnalysis_; };
    
    // Number of mesh nodes
    
    int NumberOfNodes(void) { return Solver().VSPGeom().Grid(0).NumberOfNodes(); };
    
    // Number Of mesh loops
    
    int NumberOfLoops(void) { return Solver().VSPGeom().Grid(1).NumberOfLoops(); };
    
    // Update the geometry (mesh)
    
    void UpdateGeometry(double *NodeXYZ);
        
    // Set number of optimization functions.. ie, CL, CD, CT, SpanLoading
    
    int &NumberOfOptimizationFunctions(void) { return NumberOfOptimizationFunctions_; };
 
    // Set the Optimization function for case i
    
    int &OptimizationFunction(int Case) { return OptimizationFunction_[Case]; };

         // Usage OptimizationFunction(Case) = Value, where Value is one of the following:
         //
         // OPT_CL           Vehicle Lift Force Coefficient
         // OPT_CD           Vehicle Drag Force Coefficient
         // OPT_CS           Vehicle Side Force Coefficient
         //                  
         // OPT_CMx          Vehicle Pitching Moment about the X axis
         // OPT_CMy          Vehicle Pitching Moment about the Y axis
         // OPT_CMz          Vehicle Pitching Moment about the Z axis
         //                  
         // OPT_CD_CL_CM     Trim Vehicle, Minimize Drag, and Drive CL to user specified value
         //                  
         // OPT_ROTOR_CT     Rotor Thrust Coefficient
         // OPT_ROTOR_CP     Rotor Power Coefficient
         //                  
         // OPT_WING_LOAD    Spanwise wing load, ie Cl vs Span
         //                  
         // OPT_RESIDUAL     Residual vector for linear system
         
         
    // Set the optimization set, this may be the wing set, or rotor set
    
    int &OptimizationSet(int Case) { return OptimizationSet_[Case]; };
        
    // Get vector length for optimization function # Case... 
    
    int OptimizationFunctionLength(int Case) { return Solver().OptimizationFunctionLength(Case); };
    
    // Setup everything for both the forward solver and adjoint
    
    void Setup(char *FileName);
    
    // Solve both the forward solve and adjoint problem
    
    void Solve(void);
    
    // Solve both the forward solver and the adjoint with a user supplied gradient vector.. Vec is of length FunctionLength
    
    void Solve(double *Vec);    
    
    // Solve the forward problem
    
    void SolveForward(void);
    
    // Set any user supplied initial gradient vectors
    
    void SetGradientVector(int Case, double *Vec);
        
    // Solve the adjoint problem
    
    void SolveAdjoint(void);
    
    // Do a matrix-vector multiply and get the RHS for the forward problem... please do a forward solve first!
    
    void CalculateMatrixVectorProductAndRightHandSide(double *VecIn, double *VecOut, double *RHS);
    
    // Do a adjoint matrix-vector multiply and get the RHS for the forward problem... please do a forward and adjoint solve first!
    
    void CalculateAdjointMatrixVectorProductAndRightHandSide(double *VecIn, double *VecOut, double *RHS);
    
    // Get the function value for a single case run, this returns a scalar in Vec

    void GetFunctionValue(double &Vec);    
    
    // Get the function value for optimization run # Case, this returns a scalar in Vec
    
    void GetFunctionValue(int Case, double &Vec);

    // Get the function value for a single case run, this returns a vector in Vec

    void GetFunctionValue(double *Vec);    
    
    // Get the function value for optimization run # Case, this returns a vector in Vec
    
    void GetFunctionValue(int Case, double *Vec);
        
    // Get gradients for single case run, Gradients is a 3*NumberOfNodes length vector containing gradients wrt xyz
        
    void GetFunctionGradients(double *Gradients);
    
    // Get gradients for optimization run # Case, Gradients is a 3*NumberOfNodes length vector containing gradients wrt xyz
    
    void GetFunctionGradients(int Case, double *Gradients);

    // Geometry/mesh xyz node values
    
    double NodeX(int i) { return Solver().VSPGeom().Grid(0).NodeList(i).x(); };
    double NodeY(int i) { return Solver().VSPGeom().Grid(0).NodeList(i).y(); };
    double NodeZ(int i) { return Solver().VSPGeom().Grid(0).NodeList(i).z(); };
    
    // Gradients wrt X, Y, or Z for single case run

    double GradientX(int i) { return Adjoint().dF_dMesh_X(1,i); };
    double GradientY(int i) { return Adjoint().dF_dMesh_Y(1,i); };
    double GradientZ(int i) { return Adjoint().dF_dMesh_Z(1,i); };

    // Gradients wrt X, Y, or Z for optimization run # Case

    double GradientX(int Case, int i) { return Adjoint().dF_dMesh_X(Case,i); };
    double GradientY(int Case, int i) { return Adjoint().dF_dMesh_Y(Case,i); };
    double GradientZ(int Case, int i) { return Adjoint().dF_dMesh_Z(Case,i); };

    // Get the pressures at the nodes of the mesh, returns a vector of length NumberOfNodes
    
    void GetNodalPressures(double *Pressure);
    
    // Calculate partial F wrt pressure for user supplied function F... both vectors are of length NumberOfNodes
    
    void GetpFupMesh(double *pFU_pP, double *pFU_pMesh);
    
};

#endif

