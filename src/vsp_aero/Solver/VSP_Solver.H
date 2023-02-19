//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////
 
#ifndef VSP_SOLVER_H
#define VSP_SOLVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "VSP_Geom.H"
#include "Vortex_Trail.H"
#include "Vortex_Sheet.H"
#include "RotorDisk.H"
#include "ControlSurfaceGroup.H"
#include "VSPAERO_OMP.H"
#include "time.H"
#include "quat.H"
#include "MatPrecon.H"
#include "Gradient.H"
#include "MergeSort.H"
#include "Interaction.H"
#include "InteractionLoop.H"
#include "VortexSheetInteractionLoop.H"
#include "VortexSheetVortex_To_VortexInteractionSet.H"
#include "ComponentGroup.H"
#include "SpanLoadData.H"
#include "QuadTree.H"
#include "EngineFace.H"
#include "OptimizationFunction.H"
#include "AdjointGradient.H"

#include "START_NAME_SPACE.H"

#define SOLVER_JACOBI 1
#define SOLVER_GMRES  2

#define JACOBI 1
#define SSOR   2
#define MATCON 3

#define SYM_X 1
#define SYM_Y 2
#define SYM_Z 3

#define FORCE_UNSTEADY 1

#define IMPULSE_ANALYSIS   1
#define HEAVE_ANALYSIS     2
#define P_ANALYSIS         3
#define Q_ANALYSIS         4
#define R_ANALYSIS         5

#define ALL_LOOPS          1
#define RELATIVE_MOTION    2
#define NO_RELATIVE_MOTION 3

#define FIXED_LOOPS  0
#define MOVING_LOOPS 1

#define NOISE_LINEAR_INTERPOLATION          1
#define NOISE_QUADRATIC_INTERPOLATION       2
#define NOISE_CUBIC_INTERPOLATION           3
#define NOISE_QUARTIC_INTERPOLATION         4
#define NOISE_CUBIC_HERMITE_INTERPOLATION   5
#define NOISE_QUINTIC_HERMITE_INTERPOLATION 6

#define GEOMETRY_UPDATE_DO_ALL      1
#define GEOMETRY_UPDATE_DO_STARTUP  2
#define GEOMETRY_UPDATE_DO_ADJOINT  3

// Definition of the VSP_SOLVER class

class VSP_SOLVER {

private:

    // Timing stuff
    
    VSPAERO_DOUBLE TotalTime[5];
    
    VSPAERO_DOUBLE StartTime_;
    VSPAERO_DOUBLE StartSolveTime_;

    // Filename
    
    char FileName_[2000];
    
    int iFix_;
    
    // Geometry type
    
    int ModelType_;
    
    // Surface type

    int SurfaceType_;
    
    // Debug
    
    int Verbose_;
    
    // Test run
    
    int DumpGeom_;
    
    // Create boundary conditions file
    
    int CreateHighLiftFile_;
    
    // Write out 2d FEM output file
    
    int Write2DFEMFile_;
    
    // Turn off KT correction
    
    int KarmanTsienCorrection_;    

    // Function data
    
    void init(void);
    
    // Multigrid level we are solving on
    
    int MGLevel_;
    
    // VSP Geometry
    
    VSP_GEOM VSPGeom_;
    
    // VSP Solver
    
    int NumberOfThreads_;    
    int NumberOfMGLevels_;
    int NumberOfEquations_;
    int NumberOfKelvinConstraints_;
    int *LoopInKelvinConstraintGroup_;
    int *LoopIsOnBaseRegion_;
    
    VSPAERO_DOUBLE KelvinLambda_;
    
    // Rotor Geoemtry
    
    int NumberOfRotors_;
    
    ROTOR_DISK *RotorDisk_;
    
    // Inlet / Nozzle flags
    
    int NumberOfEngineFaces_;
    int *SurfaceIsOnEngineFace_;
    
    ENGINE_FACE *EngineFace_;
    
    // Blade analysis
    
    int RotorAnalysis_;
    VSPAERO_DOUBLE BladeRPM_;

    // Control Surface Group information (used for case header)

    int NumberOfControlGroups_;
    CONTROL_SURFACE_GROUP *ControlSurfaceGroup_;

    // Velocity survey points
    
    int NumberOfSurveyTimeSteps_;
    int NumberofSurveyPoints_;
    VSPAERO_DOUBLE SurveyPointsStartTime_;    
    VSP_NODE *SurveyPointList_;    
    
    // Quad tree survey
    
    int NumberOfQuadTrees_;
    int *QuadTreeDirection_;
    VSPAERO_DOUBLE *QuadTreeValue_;
    QUAD_TREE *QuadTreeList_;
    FILE *QUADTREECaseListFile_;
    
    // Solver routines and data
    
    int TimeAnalysisType_;
    int TimeAccurate_;
    int StartFromSteadyState_;
    int NumberOfTimeSteps_;
    int NumberOfTimeSamples_;
    int Time_;
    int WakeStartingTime_;
    int DoHoverRampFreeStream_;
    int PanelSpanWiseLoading_;
    int AveragingHasStarted_;
    int NumberOfAveragingSets_;
    int FlowIs2D_;
    
    VSPAERO_DOUBLE SigmaAvg_;
    VSPAERO_DOUBLE TimeStep_;
    VSPAERO_DOUBLE CurrentTime_;
    VSPAERO_DOUBLE HoverRampFreeStreamVelocity_;
    VSPAERO_DOUBLE OriginalVinfHoverRamp_;
    
    VSPAERO_DOUBLE ReducedFrequency_;
    VSPAERO_DOUBLE Unsteady_AngleRate_; // Rad/s
    VSPAERO_DOUBLE Unsteady_Angle_;     // Rad
    VSPAERO_DOUBLE Unsteady_AngleMax_;  // Rad    
    VSPAERO_DOUBLE Unsteady_H_;
    VSPAERO_DOUBLE Unsteady_HMax_;

    int *GeometryComponentIsFixed_;    
    int *GeometryGroupID_;
    
    int NumberOfComponentGroups_;
    COMPONENT_GROUP *ComponentGroupList_;
    
    int AllComponentsAreFixed_;
    int ThereIsRelativeComponentMotion_;
    
    VSPAERO_DOUBLE FreeStreamVelocity_[3];
    VSPAERO_DOUBLE WakeAngle_[3];
    
    int NumberOfWakeTrailingNodes_;
    
    int SetFarFieldDist_;
    VSPAERO_DOUBLE FarFieldDist_;
    
    VSPAERO_DOUBLE RotationalRate_[3];
    
    VSPAERO_DOUBLE **UnsteadyTrailingWakeVelocity_;
    VSPAERO_DOUBLE **LocalBodySurfaceVelocityForLoop_;
    VSPAERO_DOUBLE **LocalBodySurfaceVelocityForEdge_;
    
    int NumberOfSurfaceVortexEdges_;
    int NumberOfTrailingVortexEdges_;
    int NumberOfVortexLoops_;
    int NumberOfSurfaceNodes_;
    
    int NoWakeIteration_;
    int WakeIterations_;
    int CurrentWakeIteration_;
    int GMRESTightConvergence_;
    
    int DoSymmetryPlaneSolve_;

    int Preconditioner_;
    
    VSPAERO_DOUBLE User_GMRES_ToleranceFactor_;
    
    VSPAERO_DOUBLE WakeRelax_;
    
    VSPAERO_DOUBLE CoreSizeFactor_;

    VSPAERO_DOUBLE *Gamma_[3];
    VSPAERO_DOUBLE &Gamma(int i) { return Gamma_[0][i]; };
    VSPAERO_DOUBLE &GammaNM0(int i) { return Gamma_[0][i]; };    
    VSPAERO_DOUBLE &GammaNM1(int i) { return Gamma_[1][i]; };
    VSPAERO_DOUBLE &GammaNM2(int i) { return Gamma_[2][i]; };
 
    int NumberOfNoiseInterpolationPoints_;
    
    VSPAERO_DOUBLE *GammaNoise_[9];
    VSPAERO_DOUBLE *FxNoise_[9];
    VSPAERO_DOUBLE *FyNoise_[9];
    VSPAERO_DOUBLE *FzNoise_[9];
    VSPAERO_DOUBLE *dCpUnsteadyNoise_[9];
    VSPAERO_DOUBLE *UNoise_[9];
    VSPAERO_DOUBLE *VNoise_[9];
    VSPAERO_DOUBLE *WNoise_[9];

    VSPAERO_DOUBLE *RightHandSide_;
    VSPAERO_DOUBLE *Residual_;
    VSPAERO_DOUBLE *Diagonal_;
    VSPAERO_DOUBLE *MatrixVectorProduct_;
    VSPAERO_DOUBLE MaxDiagonal_;
    VSPAERO_DOUBLE *Delta_;
    VSPAERO_DOUBLE JacobiRelaxationFactor_;
    VSPAERO_DOUBLE L2Residual_;
    VSPAERO_DOUBLE KTResidual_[2];
    char ConvergenceLine_[2000];
    
    int NumberOfMatrixPreconditioners_;    
    MATPRECON *MatrixPreconditionerList_;
    
    GRADIENT *VorticityGradient_;
    
    VSPAERO_DOUBLE AngleOfAttack_;
    VSPAERO_DOUBLE AngleOfBeta_;
    VSPAERO_DOUBLE AngleOfAttackZero_;
    VSPAERO_DOUBLE AngleOfBetaZero_;
    VSPAERO_DOUBLE Mach_;
    VSPAERO_DOUBLE Machref_;
    VSPAERO_DOUBLE Vinf_;
    VSPAERO_DOUBLE Vref_;
    VSPAERO_DOUBLE Vmax_;
    VSPAERO_DOUBLE Density_;
    VSPAERO_DOUBLE ReCref_;
    VSPAERO_DOUBLE LastMach_;

    VSPAERO_DOUBLE Sref_;
    VSPAERO_DOUBLE Cref_;
    VSPAERO_DOUBLE Bref_;
    VSPAERO_DOUBLE AverageVehicleChord_;
    
    VSPAERO_DOUBLE XYZcg_[3];

    VSPAERO_DOUBLE  QMax_;
    VSPAERO_DOUBLE CpMin_;
    VSPAERO_DOUBLE CpMax_;
    VSPAERO_DOUBLE CpBase_;
    VSPAERO_DOUBLE DCpMax_;
    
    VSPAERO_DOUBLE CL_[3];
    VSPAERO_DOUBLE CD_[3];
    VSPAERO_DOUBLE CS_[3];

    VSPAERO_DOUBLE CFx_[3];
    VSPAERO_DOUBLE CFy_[3];
    VSPAERO_DOUBLE CFz_[3];

    VSPAERO_DOUBLE CMx_[3];
    VSPAERO_DOUBLE CMy_[3];
    VSPAERO_DOUBLE CMz_[3];
   
    VSPAERO_DOUBLE CDTrefftz_[3];    
    
    VSPAERO_DOUBLE *CL_Unsteady_;
    VSPAERO_DOUBLE *CD_Unsteady_;
    VSPAERO_DOUBLE *CS_Unsteady_;    
    VSPAERO_DOUBLE *CFx_Unsteady_;
    VSPAERO_DOUBLE *CFy_Unsteady_;
    VSPAERO_DOUBLE *CFz_Unsteady_;
    VSPAERO_DOUBLE *CMx_Unsteady_;
    VSPAERO_DOUBLE *CMy_Unsteady_;
    VSPAERO_DOUBLE *CMz_Unsteady_;
    
    VSPAERO_DOUBLE MaxTurningAngle_;
    VSPAERO_DOUBLE Clo_2d_;
    VSPAERO_DOUBLE Clmax_2d_;
    VSPAERO_DOUBLE CFxo_[3];
    VSPAERO_DOUBLE CFyo_[3];
    VSPAERO_DOUBLE CFzo_[3];
    VSPAERO_DOUBLE CLo_[3];
    VSPAERO_DOUBLE CSo_[3];
    VSPAERO_DOUBLE CDo_[3];
    VSPAERO_DOUBLE CMxo_[3];
    VSPAERO_DOUBLE CMyo_[3];
    VSPAERO_DOUBLE CMzo_[3];
    
    // Span loading data

    int SpanLoadingData_;
    
    int StartOfSpanLoadDataSets_;
    int NumberOfSpanLoadDataSets_;
    
    SPAN_LOAD_DATA *SpanLoadData_;
    
    SPAN_LOAD_DATA &SpanLoadData(int i) { return SpanLoadData_[i]; };

    VSPAERO_DOUBLE Xmin_, Xmax_;
    VSPAERO_DOUBLE Ymin_, Ymax_;
    VSPAERO_DOUBLE Zmin_, Zmax_;

    // Pointer list to vortex/grid edges
    
    VSP_EDGE **SurfaceVortexEdge_;
    
    VSP_EDGE &SurfaceVortexEdge(int i) { return *(SurfaceVortexEdge_[i]); };
 
    // Pointer list to vortex/grid loops
    
    VSP_LOOP **VortexLoop_;
    
    VSP_LOOP &VortexLoop(int i) { return *(VortexLoop_[i]); };
    
    // Trailing vortices
    
    VORTEX_TRAIL *TrailingVortexEdge_;
 
    VORTEX_TRAIL &TrailingVortexEdge(int i) { return TrailingVortexEdge_[i]; };
    
    // Vortex Sheets
    
    int NumberOfVortexSheets_;
    
    VORTEX_SHEET **VortexSheet_;
    
    VORTEX_SHEET &VortexSheet(int c, int i) { return VortexSheet_[c][i]; };

    VORTEX_SHEET &VortexSheet(int i) { return VortexSheet_[0][i]; };

    // Vortex/grid edge interaction lists

    int NumberOfInteractionLoops_[2];
    
    LOOP_INTERACTION_ENTRY *InteractionLoopList_[2];
    
    // Vortex Sheet/grid interaction lists
    
    int *NumberOfVortexSheetInteractionLoops_;
    
    VORTEX_SHEET_LOOP_INTERACTION_ENTRY **VortexSheetInteractionLoopList_;
    
    // Vortex sheet/sheet interaction lists
    
    int *NumberOfVortexSheetInteractionEdges_;

    VORTEX_SHEET_LOOP_INTERACTION_ENTRY **VortexSheetInteractionTrailingVortexList_;
        
    VORTEX_SHEET_VORTEX_TO_VORTEX_INTERACTION_SET *VortexSheetVortexToVortexSet_;
    
    // Surface vortex edge to edge interaction lists
    
    int ThereIsEdgeToEdgeInteractionDataForLoopType_[2];
    int *NumberOfInteractionEdgesForEdge_[2];    
    VSP_EDGE ***VortexEdgeInteractionList_[2];

    // Initialize the local free stream conditions
    
    void InitializeFreeStream(void);
    
    // Calculate the diagonal of the influence matrix
    
    void CalculateDiagonal(void);
    
    // Calculate nearest neighbor coeffients of the influence matrix
    
    void CalculateNeighborCoefs(void);
    
    // Calculate the matrix preconditioners
    
    int CalculateNumberOfFineLoops(int Level, VSP_LOOP &Loop, int *LoopList);
    
    void CreateMatrixPreconditionersDataStructure(void);

    void CreateMatrixPreconditioners(void);

    // Multi Grid Routines

    void RestrictSolutionFromGrid(int Level);
    
    void RestrictKTFactorFromGrid(int Level);
    
    void ProlongateSolutionFromGrid(int Level);
    
    void InterpolateSolutionFromGrid(int Level);

    // Calculate matrix-vector products, and preconditioning

    void DoPreconditionedMatrixMultiply(VSPAERO_DOUBLE *vec_in, VSPAERO_DOUBLE *vec_out);
    
    void DoMatrixPrecondition(VSPAERO_DOUBLE *vec_in);
 
    VSPAERO_DOUBLE *MatrixVecTemp_;
    
    void DoMatrixMultiply(VSPAERO_DOUBLE *vec_in, VSPAERO_DOUBLE *vec_out);
    
    void MatrixMultiply(VSPAERO_DOUBLE *vec_in, VSPAERO_DOUBLE *vec_out);    

    void ZeroLoopVelocities(void);
   
    void ProlongateVelocity(void);
    
    void ProlongateUnsteadyVelocity(void);
    
    void RestrictFreeStreamVelocity(void);
     
    void MatrixTransposeMultiply(VSPAERO_DOUBLE *vec_in, VSPAERO_DOUBLE *vec_out);
    
    // GMRES routines
    
    void Do_GMRES_Solve(void);

    void GMRES_Solver(int Neq,                           // Number of Equations, 0 <= i < Neq
                      int IterMax,                       // Max number of outer iterations
                      int NumRestart,                    // Max number of inner (restart) iterations
                      int Verbose,                       // Output flag, verbose = 0, or 1
                      VSPAERO_DOUBLE *x,                 // Initial guess and solution vector
                      VSPAERO_DOUBLE *RightHandSide,     // Right hand side of Ax = b
                      VSPAERO_DOUBLE ErrorMax,           // Maximum error tolerance
                      VSPAERO_DOUBLE ErrorReduction,     // Residual reduction factor
                      VSPAERO_DOUBLE &ResFinal,          // Final log10 of residual reduction
                      int    &IterFinal);                // Final iteration count      

    VSPAERO_DOUBLE VectorDot(int Neq, VSPAERO_DOUBLE *r, VSPAERO_DOUBLE *s);
    
    void ApplyGivensRotation(VSPAERO_DOUBLE c, VSPAERO_DOUBLE s, int k, VSPAERO_DOUBLE *g);

    void CalculateVelocities(void);
    void CalculateEdgeVelocities(void);
    
    void CalculateRightHandSide(void);
    
    // Adjoint stuff

    void CreateAdjointMatrix(void);
    
    MATRIX AdjointMatrix_;
    
    int NumberOfOptimizationFunctions_;
    int OptimizationCase_;
    int DoAdjointSolve_;
    int OptimizationSolve_;    
    int AdjointMatrixSolve_;    
    int NumberOfTotalWakeResidualNodes_;
    int NumberOfAdjointEquations_;
    
    OPTIMIZATION_FUNCTION OptimizationFunctionList_[1001];   
    
    // Gradient data with respect to mesh nodes **/
    
    VSPAERO_DOUBLE *pF_pMesh_;
    VSPAERO_DOUBLE *pR_pMesh_;    
    VSPAERO_DOUBLE *pF_pSoln_;
    
    ADJOINT_GRADIENT MeshGradientData_;
    
    // Gradient data with respect to input variables
    
    int NumberOfOptimizationInputIndepdendentVariables_;

    VSPAERO_DOUBLE *pR_pInputVariable_;
    VSPAERO_DOUBLE *pF_pInputVariable_;
    
    ADJOINT_GRADIENT SolutionGradientData_;
    
    // Adjoint solution vector
    
    VSPAERO_DOUBLE **Psi_;
    
    int OptimizationNumberOfIntegrationTimeSteps_;
    VSPAERO_DOUBLE OptimizationIntegrationTime_;
        
    void Optimization_AdjointSolve(void);        
    void Optimization_CalculateGradientOfDesignFunction(void);
    void Optimization_Calculate_pF_pMesh(void);
    void Optimization_Calculate_pF_pGamma(void);
    void Optimization_GMRES_AdjointSolve(void);
    void Optimization_DoAdjointMatrixMultiply(VSPAERO_DOUBLE *vec_in, VSPAERO_DOUBLE *vec_out);
    void Optimization_Calculate_pR_pMesh(void);
    void Optimization_Calculate_Total_Gradient(void);
    void StoreOptimizationFunction(void);
    
    // Calculate gradients wrt rotor rpm
    
    VSPAERO_DOUBLE *Optimization_Calculate_pF_pRotorOmega(int Group);
    
    // Setup stuff
    
    static double FarAway_;
    
    VSPAERO_DOUBLE SmoothFactor_;
    
    void DetermineNumberOfKelvinConstrains(void);

    void Setup_VortexLoops(void);
   
    void Setup_VortexEdges(void);
    
    void InitializeTrailingVortices(void);
    
    void UpdateTrailingVortices(void);
    
    void CreateSurfaceVorticesInteractionList(int LoopType);
    
    void UpdateWakeVortexInteractionLists(void);

    VORTEX_SHEET_LOOP_INTERACTION_ENTRY* CreateVortexSheetInteractionList(int v, int &NumberOfVortexSheetInteractionLoops);
    
    VORTEX_SHEET_LOOP_INTERACTION_ENTRY* CreateVortexTrailInteractionList(int v, int w, int t, int &NumberOfVortexSheetInteractionEdges);

    void CreateInteractionListForSurfaceEdges(int LoopType);
    
    void CalculateSurfaceInducedVelocityAtPoint(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[3]);
    
    int CalculateSurfaceInducedVelocityAtOffBodyPoint(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[3]);
    
    void CalculateWingSurfaceInducedVelocityAtPoint(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[3]);

    void CalculateSurfaceInducedVelocityAtPoint(int GeomID, int ComponentID, int pLoop, VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[3]);
     
    void CalculateSurfaceInducedVelocityAtPoint(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE CoreWidth);

    void AdjustNearSurfaceVelocities(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE q[3], int NumberOfEdges, VSP_EDGE **InteractionList);
    
    VSP_EDGE **CreateInteractionList(int GeomID, int ComponentID, int pLoop, int InteractionType, VSPAERO_DOUBLE xyz[3], int &NumberOfInteractionEdges);

    int NodeIsInsideLoop(VSP_LOOP &Loop, VSPAERO_DOUBLE xyz[3]);

    int FirstTimeSetup_;
    int FirstTimeGradientSetup_;
    int MaxStackSize_;
    int ***EdgeIsUsed_;    
    int *SearchID_;
    
    VSPAERO_DOUBLE AdjointTime_;
    
    VSP_EDGE ***TempInteractionList_;

    STACK_ENTRY **LoopStackList_;    
        
    int SolverType_;
    int DoRestart_;
    int SaveRestartFile_;
    int ExternalCoupledSolve_;
    
    // Solve stuff
    
    // Solve the linear system 
    
    void SolveLinearSystem(void);
        
    // Wake update 
    
    void UpdateWakeLocations(void);
    
    void UpdateWakeLocations(VSPAERO_DOUBLE *VecIn);
    
    // Calculate the unsteady wake velocities
    
    void CalculateUnsteadyWakeVelocities(void);

    // Update the location of the geometry ... or parts of it
    
    void UpdateGeometryLocation(int DoStartUp);

    // Reset the geometry to it's initial state
    
    void ResetGeometry(void);

    // Update the local values of the circulation strengths for each vortex egde
    
    void UpdateVortexEdgeStrengths(int Level, int UpdateType);
    
    // Zero out vortex strengths
    
    void ZeroVortexState(void);
    
    // Update the convected wake distance
    
    void UpdateWakeConvectedDistance(void);
    
    // Calculate Residual
    
    void CalculateResidual(void);
    
    // Smooth out the principal part of the integral for supersonic flows
    
    void CalculatePrincipalPart(void);
    
    void SmoothPrincipalPart(void);
            
    // Force and moment calculations
    
    void CalculateForces(void);
    void CalculatePressureForces(void);
    void CalculateTrefftzForces(void);
    void CalculateKuttaJukowskiForces(void);
    void CalculateUnsteadyForces(void);    
    void IntegrateForcesAndMoments(void);
    void CalculateDeltaCPs(void);
    void CalculateVorticityGradient(void);    
    void CalculateSurfacePressures(void);
    void CalculateOptimizationFunctions(void);
    void CalculateOptimizationForces(void);

    void CreateVorticityGradientDataStructure(void);
    void CreateVorticityGradientDataStructureOld(void);

    void CalculateLeastSquaresCoefficients(int Loop1);
                                            
    void CalculateCLmaxLimitedForces(void);

    int SurfaceVortexEdgeIsBetweenPlanes(VSPAERO_DOUBLE *Normal1, VSPAERO_DOUBLE *Normal1Point,
                                         VSPAERO_DOUBLE *Normal2, VSPAERO_DOUBLE *Normal2Point,
                                         int Edge, VSPAERO_DOUBLE &Weight);
    
    void CalculateSpanWiseLoading(void);
    void CreateFEMLoadFile(int Case);
    void CreateFEMLoadFileFromVLMSolve(int Case);
    void CreateFEMLoadFileFromPanelSolve(int Case);
    
    // Quad Tree Slicing
    
    void CreateQuadTreePlaneDataStructures(void);
    void CalculateQuadTreeVelocitySurvey(int Case);
    
    int InsideBody(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE distance);       

    // Set solver type... not being used 
    
    int &SolverType(void) { return SolverType_; };
    
    // Number of time samples... not being used 
           
    int &NumberOfTimeSamples(void) { return NumberOfTimeSamples_; };
           
    // Field surveys
    
    void CalculateVelocitySurvey(int Case);
        
    // ADB file
    
    int DoSolutionInterrogation_;
    
    FILE *ADBFile_;
    FILE *ADBCaseListFile_;
    
    // Input ADB file ... for noise post-processing
    
    FILE *InputADBFile_;
    
    char CaseString_[2000];

    // Restart files
    
    void WriteRestartFile(void);
    void LoadRestartFile(void);
    
    // Status file
    
    FILE *StatusFile_;
    
    // Loads file
    
    FILE *LoadFile_;
    
    VSPAERO_DOUBLE Time1_;
    VSPAERO_DOUBLE Time2_;
    
    // FEM file (beam)
    
    FILE *FEMLoadFile_;
    
    // FEM Analysis
    
    int LoadDeformationFile_;
    
    // 2D Fem loads files
    
    FILE *FEM2DLoadFile_;
    
    void WriteFEM2DGeometry(void);
    void WriteFEM2DSolution(void);
    
    // Nodal quantities of stuff
    
    VSPAERO_DOUBLE *NodalCp_;
    
    // Survey points file
    
    FILE *SurveyFile_;

    // File IO
    
    void WriteOutAerothermalDatabaseHeader(void);    
    void WriteOutAerothermalDatabaseGeometry(void);
    void WriteOutAerothermalDatabaseSolution(void);

    void ReadInAerothermalDatabaseHeader(void);    
    void ReadInAerothermalDatabaseGeometry(void);
    void ReadInAerothermalDatabaseSolution(int TimeCase);

    void InterpolateInTime(VSPAERO_DOUBLE Time, VSPAERO_DOUBLE **ArrayIn, VSPAERO_DOUBLE *ArrayOut, int NumValues);
    void InterpolateExistingSolution(VSPAERO_DOUBLE Time);


    void SavePeriodicNoiseSolution(void);
    void RestorePeriodicNoiseSolution(void);
  
    void SaveVortexState(void);
    
    void SaveWakeShapeState(void);
    
    void RestoreWakeShapeState(void);
    
    // Carlson's attainable leading edge suction model
    
    VSPAERO_DOUBLE CalculateLeadingEdgeSuctionFraction(VSPAERO_DOUBLE Mach, VSPAERO_DOUBLE ToC, VSPAERO_DOUBLE RoC, VSPAERO_DOUBLE EtaToC, VSPAERO_DOUBLE AoA, VSPAERO_DOUBLE Sweep);
  
    // Group force coefficient file(s)
    
    FILE **GroupFile_;

    // Rotor coefficient file(s)
    
    FILE **RotorFile_;
    
    // PSU WOP WOP Coupling
    
    // Set psuwopwop periodicity type
    // WOPWOP_CONSTANT  1
    // WOPWOP_PERIODIC  2
    // WOPWOP_APERIODIC 3
     
    int &WopWopPeriodicity(void) { return WopWopPeriodicity_; };
    
    int &WopWopWriteOutADBFile(void) { return WopWopWriteOutADBFile_; };    
    
    VSPAERO_DOUBLE &WopWopUserStartTime(void) { return WopWopUserStartTime_; };
    VSPAERO_DOUBLE &WopWopUserFinishTime(void) { return WopWopUserFinishTime_; };
        
    int NoiseAnalysis_;
    int NoiseTimeRefinement_;
    int NoiseTime_;
    int NumberOfNoiseTimeSteps_;
    int SteadyStateNoise_;
    int NoiseInterpolation_;
    
    VSPAERO_DOUBLE NoiseTimeStep_;
    VSPAERO_DOUBLE NoiseTimeShift_;
    VSPAERO_DOUBLE CurrentNoiseTime_;

    int WopWopEnglishUnits_;
    int WopWopFlyBy_;
    int WopWopPeriodicity_;
    int WopWopNumberOfRotors_;
    int WopWopNumberOfWings_;
    int WopWopNumberOfBodies_;
    int WopWopNumberOfTotalBlades_;
    int WopWopNumberOfTotalWingSurfaces_;
    int WopWopNumberOfTtoalBodySurfaces_;
    int WopWopNumberOfTimeSteps_;
    int WopWopComponentGroupStart_;
    int WopWopComponentGroupEnd_;
    int WopWopLongestPeriodGroup_;
    int WopWopWriteOutADBFile_;
    
    VSPAERO_DOUBLE WopWopDensityConversion_;
    VSPAERO_DOUBLE WopWopLengthConversion_;
    VSPAERO_DOUBLE WopWopAreaConversion_;
    VSPAERO_DOUBLE WopWopPressureConversion_;     
    VSPAERO_DOUBLE WopWopForceConversion_;
    VSPAERO_DOUBLE WopWopdTau_;
    VSPAERO_DOUBLE WopWopOmegaMin_;
    VSPAERO_DOUBLE WopWopOmegaMax_;
    VSPAERO_DOUBLE WopWopLongestPeriod_;
    VSPAERO_DOUBLE WopWopShortestPeriod_;
    VSPAERO_DOUBLE WopWopObserverTime_;
    VSPAERO_DOUBLE WopWopUserStartTime_;
    VSPAERO_DOUBLE WopWopUserFinishTime_;
    
    FILE *PSUWopWopNameListFile_;
    
    // Main driver routines
    
    void SetupPSUWopWopData(void);
    void WriteOutPSUWopWopFileHeadersForGroup(int c);
    void WriteOutPSUWopWopUnsteadyDataForGroup(int c);

    // Namelist files
    
    void WriteOutPSUWopWopCaseAndNameListFiles(void);
    void WriteOutPSUWopWopCaseAndNameListFilesForFlyBy(void);
        void WriteOutPSUWopWopCaseAndNameListFilesForFlyByOld(void);

    void WriteOutPSUWopWopCaseAndNameListFilesForFootPrint(void);

    // Compact Geometry
    
    void WriteOutPSUWopWopCompactGeometryHeaderForGroup(int c);
    void WriteOutPSUWopWopCompactGeometryDataForGroup(int c);
    
    // Compact Loading
    
    void WriteOutPSUWopWopCompactLoadingHeaderForGroup(int c);
    void WriteOutPSUWopWopCompactLoadingDataForGroup(int c);

    // Thickness Geometry

    void WriteOutPSUWopWopThicknessGeometryHeaderForGroup(int c);    
    void WriteOutPSUWopWopThicknessGeometryDataForGroup(int c); 
    
    // Full Geometry

    void WriteOutPSUWopWopLoadingGeometryHeaderForGroup(int c);        
    void WriteOutPSUWopWopLoadingGeometryDataForGroup(int c);   
    
    // Full Loading
    
    void WriteOutPSUWopWopLoadingHeaderForGroup(int c);        
    void WriteOutPSUWopWopLoadingDataForGroup(int c);    
    
    // Pegg and BPM data

    void WriteOutPSUWopWopPeggNamelist(void);
    void WriteOutPSUWopWopBPMHeaderForGroup(int c);    
    void WriteOutPSUWopWopBPMDataForGroup(int c);
        
public:

    // Constructor, Destructor, Copy

    VSP_SOLVER(void);
   ~VSP_SOLVER(void);
    VSP_SOLVER(const VSP_SOLVER &Solver_);

    // Copy function

    VSP_SOLVER& operator=(const VSP_SOLVER &Solver_);

    /** Get Model type.. Panel or VLM **/

    int ModelType(void) { return ModelType_; };
    
    /** Get Model subtype... CART3D, VSPGEOM, or DegenGeom **/
    
    int ModelSubType(void) { return SurfaceType_; };
    
    /** Turn off wake iterations **/
    
    int &NoWakeIteration(void) { return NoWakeIteration_; };
    
    /** Set number of wake iterations **/
    
    int &WakeIterations(void) { return WakeIterations_; };
    
    /** Adjust the wake core size model up or down, default value is 1. **/
    
    VSPAERO_DOUBLE &CoreSizeFactor(void) { return CoreSizeFactor_; };
    
    /** Turn on tight GMRES convergence **/
    
    int &GMRESTightConvergence(void) { return GMRESTightConvergence_; };
    
    /** User GMRES residual reduction factor... this scales the default residual reduction
     * ie... ResidualReductrion = DefaultResidualReduction * User_GMRES_ToleranceFactor_
     **/
     
    VSPAERO_DOUBLE &User_GMRES_ToleranceFactor(void) { return User_GMRES_ToleranceFactor_; };

    /** Wake relaxation factor **/
    
    VSPAERO_DOUBLE &WakeRelax(void) { return WakeRelax_; };
        
    /** Set angle of attack **/
    
    VSPAERO_DOUBLE &AngleOfAttack(void) { return AngleOfAttack_; };
    
    /** Set angle of sidesip **/
    
    VSPAERO_DOUBLE &AngleOfBeta(void) { return AngleOfBeta_; };
   
    /** Zero angle of attack, for unsteady periodic runs **/
    
    VSPAERO_DOUBLE &AngleOfAttackZero(void) { return AngleOfAttackZero_; };
    
    /** Zero side slip angle, for unsteady periodic runs **/
    
    VSPAERO_DOUBLE &AngleOfBetaZero(void) { return AngleOfBetaZero_; };
    
    /** Set free stream Mach number **/
        
    VSPAERO_DOUBLE &Mach(void) { return Mach_; };
    
    /** Set free stream velocity ... note that units are up to user to be consistent with geometry! **/
    
    VSPAERO_DOUBLE &Vinf(void) { return Vinf_; };

    /** Set reference Mach number, this requires Vref to be set as well **/
    
    VSPAERO_DOUBLE &Machref(void) { return Machref_; };
    
    /** Set reference velocity.. this is often the tip velocity for a rotor ... should be consistent with Machref **/
    
    VSPAERO_DOUBLE &Vref(void) { return Vref_; };
    
    /** Set free stream density **/
    
    VSPAERO_DOUBLE &Density(void) { return Density_; };
    
    /** Set free stream Reynolds number based on the reference chord, Cref **/
    
    VSPAERO_DOUBLE &ReCref(void) { return ReCref_; };
    
    /** Set Xcg loation **/

    VSPAERO_DOUBLE &Xcg(void) { return XYZcg_[0]; };
    
    /** Set Ycg loation **/
    
    VSPAERO_DOUBLE &Ycg(void) { return XYZcg_[1]; };

    /** Set Zcg loation **/

    VSPAERO_DOUBLE &Zcg(void) { return XYZcg_[2]; };    
    
    /** Set rotational rate about x axis **/
    
    VSPAERO_DOUBLE &RotationalRate_p(void) { return RotationalRate_[0]; };
    
    /** Set rotational rate about y axis **/
    
    VSPAERO_DOUBLE &RotationalRate_q(void) { return RotationalRate_[1]; };

    /** Set rotational rate about z axis **/

    VSPAERO_DOUBLE &RotationalRate_r(void) { return RotationalRate_[2]; };    
    
    /** Set the angular rate, in radian/sec, for component group = Group **/
    
    VSPAERO_DOUBLE &GroupOmegaRate(int Group) { return ComponentGroupList_[Group].Omega(); };

    /** Set number of rotors ... aka... acuator disks **/
    
    void SetNumberOfRotors(int NumberOfRotors) { NumberOfRotors_ = NumberOfRotors; RotorDisk_ = new ROTOR_DISK[NumberOfRotors_ + 1]; };
    
    /** Access to the acuator disk objects **/
    
    ROTOR_DISK &RotorDisk(int i) { return RotorDisk_[i]; };
   
    /** Set the number of inlets and nozzles **/
    
    void SetNumberOfEngineFaces(int NumberOfEngineFaces) { NumberOfEngineFaces_ = NumberOfEngineFaces ; EngineFace_ = new ENGINE_FACE[NumberOfEngineFaces_ + 1]; };
    
    /** Get number of inlets/nozzles **/
    
    int NumberOfEngineFaces(void) { return NumberOfEngineFaces_; };
    
    /** Access to the nozzle/inlet objects **/
    
    ENGINE_FACE &EngineFace(int i) { return EngineFace_[i]; };

    /** Pass in a pointer to the control surface group(s) that were read in/set up in the vspaero main routine... **/
    
    void SetControlSurfaceGroup( CONTROL_SURFACE_GROUP * csg, int NumberOfControlGroups ) { ControlSurfaceGroup_ = csg; NumberOfControlGroups_ = NumberOfControlGroups;  };

    /** Access to the control surface group objects **/
    
    CONTROL_SURFACE_GROUP &ControlSurfaceGroup( int i ) { return ControlSurfaceGroup_[i]; };
    
    /** Turn on and set symmetry direction **/
    
    void DoSymmetryPlaneSolve(int Direction) { DoSymmetryPlaneSolve_ = VSPGeom_.DoSymmetryPlaneSolve() = Direction; };

    /** Set the farfield distance for the trailing wakes.. over rides defaults **/
    
    void SetFarFieldDist(VSPAERO_DOUBLE FarDist) { SetFarFieldDist_ = 1; FarFieldDist_ = FarDist; };
    
    /** Set number of trailing nodes to define the wakes... must be a power of 2 **/
    
    void SetNumberOfWakeTrailingNodes(int NumberOfNodes) { NumberOfWakeTrailingNodes_ = NumberOfNodes; };
        
    /** Set the number of survey points **/
    
    void SetNumberOfSurveyPoints(int NumberOfSurveyPoints) { NumberofSurveyPoints_ = NumberOfSurveyPoints; SurveyPointList_ = new VSP_NODE[NumberofSurveyPoints_ + 1]; };
    
    /** Access to the survey points objects **/
    
    VSP_NODE &SurveyPointList(int i) { return SurveyPointList_[i]; };
    
    /** Set the number of final time steps to do a survey for unsteady calculatons **/
    
    int &NumberOfSurveyTimeSteps(void) { return NumberOfSurveyTimeSteps_; };
    
    /** Set number of quad trees for flow field interrogation **/
    
    void SetNumberOfQuadTrees(int NumberOfQuadTrees) { NumberOfQuadTrees_ = NumberOfQuadTrees; QuadTreeDirection_ = new int[NumberOfQuadTrees_ + 1]; QuadTreeValue_ = new VSPAERO_DOUBLE[NumberOfQuadTrees_ + 1]; };

    /** Set quad tree direction ... x, y, or z --> 1, 2, or 3 **/
    
    int &QuadTreeDirection(int i) { return QuadTreeDirection_[i]; };
    
    /** Set quad tree plane value... ie x, y, or z location **/
    
    VSPAERO_DOUBLE &QuadTreeValue(int i) { return QuadTreeValue_[i]; };
    
    /** Output the force and moment data for group Group ... **/
    
    void OutputForcesAndMomentsForGroup(int Group);
    
    /** Calculate rotor coefficients given thrust, moment, diameter, rpm... **/
     
    void CalculateRotorCoefficientsFromForces(VSPAERO_DOUBLE Thrust, VSPAERO_DOUBLE Moment, VSPAERO_DOUBLE Diameter, VSPAERO_DOUBLE RPM,
                                              VSPAERO_DOUBLE &J, VSPAERO_DOUBLE &CT, VSPAERO_DOUBLE &CQ, VSPAERO_DOUBLE &CP, VSPAERO_DOUBLE &EtaP,
                                              VSPAERO_DOUBLE &CT_h, VSPAERO_DOUBLE &CQ_h, VSPAERO_DOUBLE &CP_h, VSPAERO_DOUBLE &FOM);

    /** Solution vector for the forward solve ... ie the vorticity **/

    double ForwardSolutionVector(int i) { return DOUBLE(Gamma_[0][i]); };

    /** Solution vector for the adjoint solve ... ie Psi **/

    double AdjointSolutionVector(int i) { return DOUBLE(Psi_[1][i]); };
        
    /** P'th solution vector for the adjoint solve ... ie Psi **/
        
    double AdjointSolutionVector(int p, int i) { return DOUBLE(Psi_[p][i]); };
        
    /** Reference area **/
    
    VSPAERO_DOUBLE &Sref(void) { return Sref_; };
    
    /** Reference chord **/
    
    VSPAERO_DOUBLE &Cref(void) { return Cref_; };
    
    /** Reference Span **/
    
    VSPAERO_DOUBLE &Bref(void) { return Bref_; };    
 
    /** Inviscid Force coefficient, in x axis direction ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/

    VSPAERO_DOUBLE CFx(int Type) { return CFx_[Type]; };
    
    /** Inviscid Force coefficient, in y axis directio ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/
    
    VSPAERO_DOUBLE CFy(int Type) { return CFy_[Type]; };

    /** Inviscid Force coefficient, in z axis directio ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/

    VSPAERO_DOUBLE CFz(int Type) { return CFz_[Type]; };

    /** Inviscid Lift coefficient ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/

    VSPAERO_DOUBLE  CL(int Type) { return  CL_[Type]; };
    
    /** Inviscid Drag coefficient ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/
    
    VSPAERO_DOUBLE  CD(int Type) { return  CD_[Type]; };
    
    /** Inviscid Side force coefficient ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/
    
    VSPAERO_DOUBLE  CS(int Type) { return  CS_[Type]; };
    
    /** Inviscid Moment coefficient, about x axis ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/
    
    VSPAERO_DOUBLE CMx(int Type) { return CMx_[Type]; };
    
    /** Inviscid Moment coefficient, about y axis ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/
    
    VSPAERO_DOUBLE CMy(int Type) { return CMy_[Type]; };

    /** Inviscid Moment coefficient, about z axis ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/

    VSPAERO_DOUBLE CMz(int Type) { return CMz_[Type]; };
    
    /** Inviscid Trailing edge (sorta kinda Trefftz analysis) induced drag ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/

    VSPAERO_DOUBLE CDTrefftz(int Type) { return CDTrefftz_[Type]; };

    /** Inviscid Current Force coefficient, in x axis direction ... Steady/Unsteady **/

    VSPAERO_DOUBLE CFx(void) { return CFx_[0]; };
    
    /** Inviscid Current Force coefficient, in y axis direction ... Steady/Unsteady **/
    
    VSPAERO_DOUBLE CFy(void) { return CFy_[0]; };

    /** Inviscid Current Force coefficient, in z axis direction ... Steady/Unsteady **/

    VSPAERO_DOUBLE CFz(void) { return CFz_[0]; };

    /** Inviscid Current Inviscid Lift coefficient ... Steady/Unsteady **/
    
    VSPAERO_DOUBLE  CL(void) { return  CL_[0]; };
    
    /** Inviscid Current Inviscid Drag coefficient ... Steady/Unsteady **/

    VSPAERO_DOUBLE  CD(void) { return  CD_[0]; };
    
    /**  InviscidCurrent Inviscid Side coefficient ... Steady/Unsteady **/
    
    VSPAERO_DOUBLE  CS(void) { return  CS_[0]; };
    
    /** Inviscid Curren Inviscidt moment coefficient about x axis... Steady/Unsteady **/
    
    VSPAERO_DOUBLE CMx(void) { return CMx_[0]; };
    
    /** Inviscid Current Inviscid moment coefficient about y axis... Steady/Unsteady **/
    
    VSPAERO_DOUBLE CMy(void) { return CMy_[0]; };

    /** Inviscid Current Inviscid moment coefficient about z axis... Steady/Unsteady **/

    VSPAERO_DOUBLE CMz(void) { return CMz_[0]; };
    
    /** Inviscid Current Trailing edge (sorta kinda Trefftz analysis) induced drag... Steady/Unsteady **/
    
    VSPAERO_DOUBLE CDTrefftz(void) { return CDTrefftz_[0]; };
    
    /** Unsteady Inviscid Lift coefficient at time step i **/
  
    VSPAERO_DOUBLE  CL_Unsteady(int i) { return  CL_Unsteady_[i]; };
    
    /** Unsteady Inviscid Drag coefficient at time step i **/
    
    VSPAERO_DOUBLE  CD_Unsteady(int i) { return  CD_Unsteady_[i]; };

    /** Unsteady Inviscid Side coefficient at time step i **/

    VSPAERO_DOUBLE  CS_Unsteady(int i) { return  CS_Unsteady_[i]; };   

    /** Unsteady Inviscid force coefficient in x axis direction at time step i **/

    VSPAERO_DOUBLE CFx_Unsteady(int i) { return CFx_Unsteady_[i]; };

    /** Unsteady Inviscid force coefficient in y axis direction at time step i **/

    VSPAERO_DOUBLE CFy_Unsteady(int i) { return CFy_Unsteady_[i]; };

    /** Unsteady Inviscid force coefficient in z axis direction at time step i **/

    VSPAERO_DOUBLE CFz_Unsteady(int i) { return CFz_Unsteady_[i]; };

    /** Unsteady Inviscid moment coefficient along x axis at time step i **/

    VSPAERO_DOUBLE CMx_Unsteady(int i) { return CMx_Unsteady_[i]; };
    
    /** Unsteady Inviscid moment coefficient along y axis at time step i **/

    VSPAERO_DOUBLE CMy_Unsteady(int i) { return CMy_Unsteady_[i]; };

    /** Unsteady Inviscid moment coefficient along z axis at time step i **/

    VSPAERO_DOUBLE CMz_Unsteady(int i) { return CMz_Unsteady_[i]; }; 

    /** Viscous force coefficient in x axis direction **/
        
    VSPAERO_DOUBLE CFxo(void) { return CFxo_[0]; };

    /** Viscous force coefficient in y axis direction **/

    VSPAERO_DOUBLE CFyo(void) { return CFyo_[0]; };
    
    /** Viscous force coefficient in z axis direction **/
    
    VSPAERO_DOUBLE CFzo(void) { return CFzo_[0]; };

    /** Viscous moment coefficient along x axis **/
        
    VSPAERO_DOUBLE CMxo(void) { return CMxo_[0]; };
    
    /** Viscous moment coefficient along y axis **/
    
    VSPAERO_DOUBLE CMyo(void) { return CMyo_[0]; };

    /** Viscous moment coefficient along z axis **/

    VSPAERO_DOUBLE CMzo(void) { return CMzo_[0]; };
 
    /** Viscous lift coefficient **/

    VSPAERO_DOUBLE CLo(void) { return CLo_[0]; };
    
    /** Viscous drag coefficient **/

    VSPAERO_DOUBLE CDo(void) { return CDo_[0]; };
            
    /** Viscous side force coefficient **/

    VSPAERO_DOUBLE CSo(void) { return CSo_[0]; };
        
    /** Viscous force coefficient in x axis direction... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/ 
        
    VSPAERO_DOUBLE CFxo(int Type) { return CFxo_[Type]; };

    /** Viscous force coefficient in y axis direction... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/

    VSPAERO_DOUBLE CFyo(int Type) { return CFyo_[Type]; };
    
    /** Viscous force coefficient in z axis direction... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/ 
    
    VSPAERO_DOUBLE CFzo(int Type) { return CFzo_[Type]; };

    /** Viscous moment coefficient along x axis... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/
        
    VSPAERO_DOUBLE CMxo(int Type) { return CMxo_[Type]; };
    
    /** Viscous moment coefficient along y axis... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/ 
    
    VSPAERO_DOUBLE CMyo(int Type) { return CMyo_[Type]; };

    /** Viscous moment coefficient along z axi... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/

    VSPAERO_DOUBLE CMzo(int Type) { return CMzo_[Type]; };    
    
    /** Viscous lift coefficient ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/

    VSPAERO_DOUBLE CLo(int Type) { return CLo_[Type]; };

    /** Viscous drag coefficient ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/

    VSPAERO_DOUBLE CDo(int Type) { return CDo_[Type]; };
    
    /** Viscous side force coefficient ... Type == 0: Steady/Unsteady ... Type == 2: Averaged **/

    VSPAERO_DOUBLE CSo(int Type) { return CSo_[Type]; };
        
    /** Zero angle of attack lift coefficient for 2D viscous drag estimate... **/

    VSPAERO_DOUBLE &Clo2D(void) { return Clo_2d_; };
    
    /** 2D ClMax for stall calculations **/
    
    VSPAERO_DOUBLE &ClMax(void) { return Clmax_2d_; };
    
    /** Max turning angle for stall calculations .. this is no longer being used... **/
    
    VSPAERO_DOUBLE &MaxTurningAngle(void) { return MaxTurningAngle_; };
    
    /** Read in the VSP geometry file **/
    
    void ReadFile(char *FileName) { snprintf(FileName_,sizeof(FileName_)*sizeof(char),"%s",FileName); VSPGeom_.LoadDeformationFile() = LoadDeformationFile_;  VSPGeom_.ReadFile(FileName,ModelType_,SurfaceType_); };

    /** Read in the FEM deformation file **/
    
    void LoadFEMDeformationData(void) { VSPGeom_.LoadFEMDeformationData(FileName_); };
    
    /** Turn on ground effects analysis **/
    
    int &DoGroundEffectsAnalysis(void) { return VSPGeom_.DoGroundEffectsAnalysis(); };
    
    /** Set the vehicle rotation vector (r1,r2,r3 **/
    
    VSPAERO_DOUBLE &VehicleRotationAngleVector(int i) { return VSPGeom_.VehicleRotationAngleVector(i); };
    
    /** Set the point about which to rotate the vehicle **/
    
    VSPAERO_DOUBLE &VehicleRotationAxisLocation(int i) { return VSPGeom_.VehicleRotationAxisLocation(i); };
    
    /** Set the height of the vehicle above the ground ...be careful you don't rotate a vehicle into the ground plane! **/
    
    VSPAERO_DOUBLE &HeightAboveGround(void) { return VSPGeom_.HeightAboveGround(); };
    
    /** Access to VSP geometry object **/
    
    VSP_GEOM &VSPGeom(void) { return VSPGeom_; };

    /** Set up the problem... **/
    
    void Setup(void);
    
    /** Solve on the current geometry, flow condition(s) **/
 
    void Solve(void) { Solve(0); };
    
    /** Solve case, Case **/
    
    void Solve(int Case);

    /** Recalcalculate the forces... something has been changed, usually the Re # **/
    
    void ReCalculateForces(void);    
    
    /** Write out the noise files for psuwopwop **/
    
    void WriteOutNoiseFiles(int Case);  
    
    /** Write out time accurate noise files for psuwopwop **/
    
    void WriteOutTimeAccurateNoiseFiles(int Case);  
    
    /** Write out steady state noise files for psuwopwop **/
    
    void WriteOutSteadyStateNoiseFiles(int Case);  

#if not defined AUTODIFF && not defined COMPLEXDIFF

    /** Do a matrix vector product using a user supplied vector... and also return the right hand side **/

    void CalculateMatrixVectorProductAndRightHandSide(double *VecIn, double *VecOut, double *RHS);

#endif

#if not defined AUTODIFF && not defined COMPLEXDIFF

    /** Calculate residual given user supplied vector of solution vector, gamma **/

    void CalculateForwardResidual(double *VecIn, double *VecOut);

#endif

#if not defined AUTODIFF && not defined COMPLEXDIFF

    /** Calculate optimization functions given user supplied vector of solution vector, gamma **/

    void CalculateOptimizationFunctions(double *VecIn);

#endif

#if defined AUTODIFF

    /** Do a matrix vector product using a user supplied vector... and also return the right hand side **/

    void CalculateAdjointMatrixVectorProductAndRightHandSide(double *VecIn, double *VecOut, double *RHS);
    
    /** Solve the adjoint equation for user defined right hand side **/
    
    void SolveAdjointLinearSystem(double *Psi, double *RHS);

    /** Calculate partial derivatives of optimization case = Case wrt the mesh and the input variables **/
    
    void CalculateOptimizationFunctionPartials(int Case, double *pF_pMesh, double *pF_pInputVariable);
    
    /** Solve for the adjoint residual partial products given the vector Psi **/
    
    void CalculateAdjointResidualPartialProducts(double *Psi, double *pR_pMesh, double *pR_pInputVariable);
        
#endif

    // Apply matrix preconditioner... the preconditioner applied depends on whether we are doing a forward or adjoint solve **/
    
    void ApplyMatrixPrecondition(double *Vec);
    
    /** Return the L2 residual **/
    
    VSPAERO_DOUBLE L2Residual(void) { return L2Residual_; };

    /** Do a restart using a previous solution file **/
        
    int &DoRestart(void) { return DoRestart_; };
    
    /** Save a restart file **/
    
    int &SaveRestartFile(void) { return SaveRestartFile_; };
    
    /** Output a status file **/
    
    void OutputStatusFile(int Case);
    
    /** Output zero lift drag to the status file **/
    
    void OutputZeroLiftDragToStatusFile(void);
    
    /** Read in the geometry, do a minimum of set up and then write out an adb file we can view in Viewer ... no solve **/
    
    int &DumpGeom(void) { return DumpGeom_; };
    
    /** Create a default boundary conditions setup file **/
    
    int &CreateHighLiftFile(void) { return CreateHighLiftFile_; };

    /** Load in the fem deformation file **/
    
    int &LoadFEMDeformation(void) { return LoadDeformationFile_; };
    
    /** Write out 2D FEM load file **/
    
    int &Write2DFEMFile(void) { return Write2DFEMFile_; };

    /** Turn on the Karman Tsien compressibiltiy correction **/
    
    int &KarmanTsienCorrection(void) { return KarmanTsienCorrection_; };

    /** Set the time accurate anlsysis type 
     * 
     * P_ANALYSIS ... unsteady pitch analysis
     * Q_ANALYSIS ... unsteady roll analysis
     * R_ANALYSIS ... unsteady yaw analysis
     * 
     **/
    
    int &TimeAnalysisType(void) { return TimeAnalysisType_; };
    
    /** Set the analysis to time accurate mode **/
    
    int &TimeAccurate(void) { return TimeAccurate_; };
    
    /** Run a steady state analysis first... then start time accurate analysis from there **/
    
    int &StartFromSteadyState(void) { return StartFromSteadyState_; };    
    
    /** Set the number of time step 
     * 
     * -1 indicates the code will determine number of time steps based on rotor RPMS
     * -1 means one rotor revolutions
     * -2 means two rotor revolutions...
     * 
     */
     
    int &NumberOfTimeSteps(void) { return NumberOfTimeSteps_; };

    /** Set the time step for an unsteady analysis
     * 
     * -1 means the code will set the time step based on rotor RPMS
     * -1 means code will do 18 revs on the fastest rotor
     * -2 means code will do 36 revs on the fastest rotor...
     * 
     **/
     
    VSPAERO_DOUBLE &TimeStep(void) { return TimeStep_; };
    
    /** Starts time accurate solve with a finite free stream and 
     * slowly tapers that to zero to help with solving stiff
     * hover cases
     *
     **/

    int &DoHoverRampFreeStream(void) { return DoHoverRampFreeStream_; };

    /** Hover ramp free stream velocity to start with **/
        
    VSPAERO_DOUBLE &HoverRampFreeStreamVelocity(void) { return HoverRampFreeStreamVelocity_; };
    
    /** Force the solver into 2D mode ... mostly for testing purposes **/
    
    int &FlowIs2D(void) { return FlowIs2D_; };
    
    /** Turn off the calculation of spanwise loading for a panel solver...
     * 
     * this is here since the code can get confused for cart3d tri file
     * geometries that just really don't have the information to figure
     * out how to calculate spanwise loading information... without this
     * the code might just quit after bashing it's head into the ground
     **/
     
    int &PanelSpanWiseLoading(void) { return PanelSpanWiseLoading_; };
    
    /** Set reduced frequency for an unsteady pitch/yaw analysis **/
    
    VSPAERO_DOUBLE &ReducedFrequency(void) { return ReducedFrequency_; };
    
    /** Get back the actual unsteady angle rate **/
    
    VSPAERO_DOUBLE Unsteady_AngleRate(void) { return Unsteady_AngleRate_; };
    
    /** Set the unsteady maximum angle... pitch or yaw **/
    
    VSPAERO_DOUBLE &Unsteady_AngleMax(void) { return Unsteady_AngleMax_; };
    
    /** Set the unsteady max heave value **/
    
    VSPAERO_DOUBLE &Unsteady_HMax(void) { return Unsteady_HMax_; };
        
    /** Run a hacked blade analysis where the world spins and the rotor doesn't **/
            
    int &RotorAnalysis(void) { return RotorAnalysis_; };
    
    /** Set the blade analysis rotor RPM **/
    
    VSPAERO_DOUBLE &BladeRPM(void) { return BladeRPM_; };

    /** Interrogate an existing solution **/
    
    int &DoSolutionInterrogation(void) { return DoSolutionInterrogation_; };


    /** Load in restart file and interrogate the solution 
    * 
    * this lets you add in survey points, cut planes after you've done a run
    * and then just interrogate the solution without have to solve it
    * again... very useful for unsteady cases
    * 
    **/ 
     
    void RestartAndInterrogateSolution(int Case);
        
    /** Do a noise analysis... this is a post processing step on an existing solution **/
    
    int &NoiseAnalysis(void) { return NoiseAnalysis_; };
    
    /** Create files for an assumed steady state analysis... ie, not time accurate **/
    
    int &SteadyStateNoise(void) { return SteadyStateNoise_; };    
   
    /** Input geometry and flight conditions are in english units...
     * 
     * ... so we will convert to SI for output to psuwopwop
     * 
     **/

    int &WopWopEnglishUnits(void) { return WopWopEnglishUnits_; };   

    /** Do a psuwopwop flyby analysis **/

    void DoWopWopFlyBy(void) { WopWopFlyBy_ = 1; };
    
    /** Do a psuwopwop foot print analysis **/
    
    void DoWopWopFootPrint(void) { WopWopFlyBy_ = 0; };
    
    /** Number of nodes on the finest grid level **/
    
    int NumberOfGridNodes(void) { return VSPGeom().Grid(0).NumberOfNodes(); };
    
    /** Number of vortex loop on the finest mesh level **/
    
    int NumberOfVortexLoops(void) { return NumberOfVortexLoops_; };
    
    /** Total number of adjoint equations **/
    
    int NumberOfAdjointEquations(void) { return NumberOfAdjointEquations_; };

    /** Number of wake residual equations **/
    
    int NumberOfAdjointWakeResidualEquations(void) { return NumberOfTotalWakeResidualNodes_; };  

    /** Do an adjoint solve **/
    
    void Optimization_Solve(void) { Optimization_Solve(0); };
    
    /** Do an adjoint solve on case **/
    
    void Optimization_Solve(int Case);    
        
    /** Set the number of optimization functions **/
    
    int &NumberOfOptimizationFunctions(void) { return NumberOfOptimizationFunctions_; };

    /** Set optimization function 'i' to value 'OptFunctionValue' **/
    
    void SetOptimizationFunction(int i, int OptFunctionValue);
    
    /** Set optimization function 'i' to value 'OptFunctionValue' and associate with rotor, or wing 'Set' **/
    
    void SetOptimizationFunction(int i, int OptFunctionValue, int Set );
    
    /** Set optimization gradient vector for the one and only optimization function... assume there just 1! **/
    
    void SetOptimizationFunctionInputGradientVector(double *Vec) { SetOptimizationFunctionInputGradientVector(1,1,1,1,Vec); };
   
    /** Set optimization gradient vector for Case, with Length, TimeSteps, Set... **/
    
    void SetOptimizationFunctionInputGradientVector(int Case, int Length, int TimeSteps, int Set, double *Vec);

    /** Determine the optimization function length based on what it is... **/
    
    void CalculateOptimizationFunctionLength(void);
   
    /** Get optimization function lenght for opt function Case **/
    
    int OptimizationFunctionLength(int Case) { return OptimizationFunctionList_[Case].FunctionLength(); };
    
    /** Get total vector length for optimization case **/
    
    int OptimizationVectorLength(int Case) { return OptimizationFunctionList_[Case].VectorLength(); };
    
    /** Get number of averaging time steps for optimization case **/
    
    int OptimizationNumberOfTimeSteps(int Case) { return OptimizationFunctionList_[Case].NumberOfTimeSteps(); };
    
    /** Do the adjoint solve **/

    int &DoAdjointSolve(void) { return DoAdjointSolve_; };
    
    /** This is an optimization solve
     * 
     * this must be set to let the solver know it needs to evaluate the 
     * optimization functions... for both the foward and adjoint solvers
     * 
     **/

    int &OptimizationSolve(void) { return OptimizationSolve_; };

// Total Gradients
    
    /** Total gradient of objective function 1 wrt x for node i **/
    
    double dF_dMesh_X(int i) { return DOUBLE(MeshGradientData_.dF_dMesh(0,1,3*i-2)); };
    
    /** Total gradient of objective function 1 wrt y for node i **/

    double dF_dMesh_Y(int i) { return DOUBLE(MeshGradientData_.dF_dMesh(0,1,3*i-1)); };

    /** Total gradient of objective function 1 wrt z for node i **/
 
    double dF_dMesh_Z(int i) { return DOUBLE(MeshGradientData_.dF_dMesh(0,1,3*i  )); };
    
    /** Total gradient of objective function p wrt x for node i **/
  
    double dF_dMesh_X(int p, int i) { return DOUBLE(MeshGradientData_.dF_dMesh(0,p,3*i-2)); };
    
    /** Total gradient of objective function p wrt y for node i **/
 
    double dF_dMesh_Y(int p, int i) { return DOUBLE(MeshGradientData_.dF_dMesh(0,p,3*i-1)); };

    /** Total gradient of objective function p wrt z for node i **/

    double dF_dMesh_Z(int p, int i) { return DOUBLE(MeshGradientData_.dF_dMesh(0,p,3*i  )); };
    
    /** Unsteady total gradient of objective function p, at time Time, wrt x for node i **/
   
    double dF_dMesh_X(int p, int Time, int i) { return DOUBLE(MeshGradientData_.dF_dMesh(Time,p,3*i-2)); };
    
    /** Unsteady total gradient of objective function p, at time Time, wrt y for node i **/

    double dF_dMesh_Y(int p, int Time, int i) { return DOUBLE(MeshGradientData_.dF_dMesh(Time,p,3*i-1)); };

    /** Unsteady total gradient of objective function p, at time Time, wrt z for node i **/
       
    double dF_dMesh_Z(int p, int Time, int i) { return DOUBLE(MeshGradientData_.dF_dMesh(Time,p,3*i  )); };
    
// Partial gradients with respect to the mesh

    /** Partial gradient of objective function 1 wrt mesh x for node i **/
    
    double pF_pMesh_X(int i) { return DOUBLE(MeshGradientData_.pF_pMesh(0,1,3*i-2)); };
    
    /** Partial gradient of objective function 1 wrt mesh y for node i **/

    double pF_pMesh_Y(int i) { return DOUBLE(MeshGradientData_.pF_pMesh(0,1,3*i-1)); };

    /** Partial gradient of objective function 1 wrt mesh z for node i **/
 
    double pF_pMesh_Z(int i) { return DOUBLE(MeshGradientData_.pF_pMesh(0,1,3*i  )); };
    
    /** Partial gradient of objective function p wrt mesh x for node i **/
      
    double pF_pMesh_X(int p, int i) { return DOUBLE(MeshGradientData_.pF_pMesh(0,p,3*i-2)); };
    
    /** Partial gradient of objective function p wrt mesh y for node i **/
 
    double pF_pMesh_Y(int p, int i) { return DOUBLE(MeshGradientData_.pF_pMesh(0,p,3*i-1)); };

    /** Partial gradient of objective function p wrt mesh z for node i **/

    double pF_pMesh_Z(int p, int i) { return DOUBLE(MeshGradientData_.pF_pMesh(0,p,3*i  )); };
    
    /** Unsteady partial gradient of objective function p, at time Time, wrt mesh x for node i **/
   
    double pF_pMesh_X(int p, int Time, int i) { return DOUBLE(MeshGradientData_.pF_pMesh(Time,p,3*i-2)); };
    
    /** Unsteady partial gradient of objective function p, at time Time, wrt mesh y for node i **/

    double pF_pMesh_Y(int p, int Time, int i) { return DOUBLE(MeshGradientData_.pF_pMesh(Time,p,3*i-1)); };

    /** Unsteady partial gradient of objective function p, at time Time, wrt mesh z for node i **/
       
    double pF_pMesh_Z(int p, int Time, int i) { return DOUBLE(MeshGradientData_.pF_pMesh(Time,p,3*i  )); };

// Partial gradients with respect to the solution

    /** Partial gradient of objective function 1 wrt solution, wrt x for node i **/
    
    double pR_pMesh_X(int i) { return DOUBLE(MeshGradientData_.pR_pMesh(0,1,3*i-2)); };
    
    /** Partial gradient of objective function 1 wrt solution, wrt y for node i **/

    double pR_pMesh_Y(int i) { return DOUBLE(MeshGradientData_.pR_pMesh(0,1,3*i-1)); };

    /** Partial gradient of objective function 1 wrt solution, wrt z for node i **/
 
    double pR_pMesh_Z(int i) { return DOUBLE(MeshGradientData_.pR_pMesh(0,1,3*i  )); };
    
    /** Partial gradient of objective function p wrt solution, wrt x for node i **/
      
    double pR_pMesh_X(int p, int i) { return DOUBLE(MeshGradientData_.pR_pMesh(0,p,3*i-2)); };
    
    /** Partial gradient of objective function p wrt solution, wrt y for node i **/
 
    double pR_pMesh_Y(int p, int i) { return DOUBLE(MeshGradientData_.pR_pMesh(0,p,3*i-1)); };

    /** Partial gradient of objective function p wrt solution, wrt z for node i **/

    double pR_pMesh_Z(int p, int i) { return DOUBLE(MeshGradientData_.pR_pMesh(0,p,3*i  )); };
    
    /** Unsteady partial gradient of objective function p, at time Time, wrt solution, wrt x for node i **/
   
    double pR_pMesh_X(int p, int Time, int i) { return DOUBLE(MeshGradientData_.pR_pMesh(Time,p,3*i-2)); };
    
    /** Unsteady partial gradient of objective function p, at time Time, wrt solution, wrt y for node i **/

    double pR_pMesh_Y(int p, int Time, int i) { return DOUBLE(MeshGradientData_.pR_pMesh(Time,p,3*i-1)); };

    /** Unsteady partial gradient of objective function p, at time Time, wrt solution, wrt z for node i **/
       
    double pR_pMesh_Z(int p, int Time, int i) { return DOUBLE(MeshGradientData_.pR_pMesh(Time,p,3*i  )); };

// Total gradients
                
    /** Total gradient of objective function 1 with resepect to input variable InputVariable 
     *
     * OPT_GRADIENT_WRT_ALPHA 
     * OPT_GRADIENT_WRT_BETA         
     * OPT_GRADIENT_WRT_MACH         
     * OPT_GRADIENT_WRT_VINF         
     * OPT_GRADIENT_WRT_DENSITY      
     * OPT_GRADIENT_WRT_RECREF       
     * OPT_GRADIENT_WRT_P_RATE       
     * OPT_GRADIENT_WRT_Q_RATE       
     * OPT_GRADIENT_WRT_R_RATE       
     * **/
     
    double dF_dInputVariable(int InputVariable) { return DOUBLE(SolutionGradientData_.dF_dVariable(0,1,InputVariable)); };
 
    /** Total gradient of objective function p with resepect to input variable InputVariable 
     *
     * OPT_GRADIENT_WRT_ALPHA 
     * OPT_GRADIENT_WRT_BETA         
     * OPT_GRADIENT_WRT_MACH         
     * OPT_GRADIENT_WRT_VINF         
     * OPT_GRADIENT_WRT_DENSITY      
     * OPT_GRADIENT_WRT_RECREF       
     * OPT_GRADIENT_WRT_P_RATE       
     * OPT_GRADIENT_WRT_Q_RATE       
     * OPT_GRADIENT_WRT_R_RATE       
     * **/    
    
    double dF_dInputVariable(int p, int InputVariable) { return DOUBLE(SolutionGradientData_.dF_dVariable(0,p,InputVariable)); };
    
    /** Unsteady total gradient of objective function p, at time Time, with resepect to input variable InputVariable
     *
     * OPT_GRADIENT_WRT_ALPHA 
     * OPT_GRADIENT_WRT_BETA         
     * OPT_GRADIENT_WRT_MACH         
     * OPT_GRADIENT_WRT_VINF         
     * OPT_GRADIENT_WRT_DENSITY      
     * OPT_GRADIENT_WRT_RECREF       
     * OPT_GRADIENT_WRT_P_RATE       
     * OPT_GRADIENT_WRT_Q_RATE       
     * OPT_GRADIENT_WRT_R_RATE       
     * **/
            
    double dF_dInputVariable(int p, int Time, int InputVariable) { return DOUBLE(SolutionGradientData_.dF_dVariable(Time,p,InputVariable)); };

// Partial gradients wrt mesh
                
    /** Partial gradient of objective function 1 with resepect to input variable InputVariable 
     *
     * OPT_GRADIENT_WRT_ALPHA 
     * OPT_GRADIENT_WRT_BETA         
     * OPT_GRADIENT_WRT_MACH         
     * OPT_GRADIENT_WRT_VINF         
     * OPT_GRADIENT_WRT_DENSITY      
     * OPT_GRADIENT_WRT_RECREF       
     * OPT_GRADIENT_WRT_P_RATE       
     * OPT_GRADIENT_WRT_Q_RATE       
     * OPT_GRADIENT_WRT_R_RATE       
     * **/
     
    double pF_pInputVariable(int InputVariable) { return DOUBLE(SolutionGradientData_.pF_pVariable(0,1,InputVariable)); };
 
    /** Partial gradient of objective function p with resepect to input variable InputVariable 
     *
     * OPT_GRADIENT_WRT_ALPHA 
     * OPT_GRADIENT_WRT_BETA         
     * OPT_GRADIENT_WRT_MACH         
     * OPT_GRADIENT_WRT_VINF         
     * OPT_GRADIENT_WRT_DENSITY      
     * OPT_GRADIENT_WRT_RECREF       
     * OPT_GRADIENT_WRT_P_RATE       
     * OPT_GRADIENT_WRT_Q_RATE       
     * OPT_GRADIENT_WRT_R_RATE       
     * **/    
    
    double pF_pInputVariable(int p, int InputVariable) { return DOUBLE(SolutionGradientData_.pF_pVariable(0,p,InputVariable)); };
    
    /** Unsteady partial gradient of objective function p, at time Time, with resepect to input variable InputVariable
     *
     * OPT_GRADIENT_WRT_ALPHA 
     * OPT_GRADIENT_WRT_BETA         
     * OPT_GRADIENT_WRT_MACH         
     * OPT_GRADIENT_WRT_VINF         
     * OPT_GRADIENT_WRT_DENSITY      
     * OPT_GRADIENT_WRT_RECREF       
     * OPT_GRADIENT_WRT_P_RATE       
     * OPT_GRADIENT_WRT_Q_RATE       
     * OPT_GRADIENT_WRT_R_RATE       
     * **/
            
    double pF_pInputVariable(int p, int Time, int InputVariable) { return DOUBLE(SolutionGradientData_.pF_pVariable(Time,p,InputVariable)); };
        
// Partial gradients wrt residual
                
    /** Partial gradient of objective function 1 with resepect to residual - vector product
     *
     * OPT_GRADIENT_WRT_ALPHA 
     * OPT_GRADIENT_WRT_BETA         
     * OPT_GRADIENT_WRT_MACH         
     * OPT_GRADIENT_WRT_VINF         
     * OPT_GRADIENT_WRT_DENSITY      
     * OPT_GRADIENT_WRT_RECREF       
     * OPT_GRADIENT_WRT_P_RATE       
     * OPT_GRADIENT_WRT_Q_RATE       
     * OPT_GRADIENT_WRT_R_RATE       
     * **/
     
    double pR_pInputVariable(int InputVariable) { return DOUBLE(SolutionGradientData_.pR_pVariable(0,1,InputVariable)); };
 
    /** Partial gradient of objective function p with resepect to residual - vector product
     *
     * OPT_GRADIENT_WRT_ALPHA 
     * OPT_GRADIENT_WRT_BETA         
     * OPT_GRADIENT_WRT_MACH         
     * OPT_GRADIENT_WRT_VINF         
     * OPT_GRADIENT_WRT_DENSITY      
     * OPT_GRADIENT_WRT_RECREF       
     * OPT_GRADIENT_WRT_P_RATE       
     * OPT_GRADIENT_WRT_Q_RATE       
     * OPT_GRADIENT_WRT_R_RATE       
     * **/    
    
    double pR_pInputVariable(int p, int InputVariable) { return DOUBLE(SolutionGradientData_.pR_pVariable(0,p,InputVariable)); };
    
    /** Unsteady partial gradient of objective function p, at time Time, with resepect to residual - vector product
     *
     * OPT_GRADIENT_WRT_ALPHA 
     * OPT_GRADIENT_WRT_BETA         
     * OPT_GRADIENT_WRT_MACH         
     * OPT_GRADIENT_WRT_VINF         
     * OPT_GRADIENT_WRT_DENSITY      
     * OPT_GRADIENT_WRT_RECREF       
     * OPT_GRADIENT_WRT_P_RATE       
     * OPT_GRADIENT_WRT_Q_RATE       
     * OPT_GRADIENT_WRT_R_RATE       
     * **/
            
    double pR_pInputVariable(int p, int Time, int InputVariable) { return DOUBLE(SolutionGradientData_.pR_pVariable(Time,p,InputVariable)); };
                 
    /** Value of objective function **/
        
    VSPAERO_DOUBLE OptimizationFunction(void);
    
    /** Value of objective function Case **/
    
    VSPAERO_DOUBLE OptimizationFunction(int Case);

#if defined AUTODIFF || defined COMPLEXDIFF

    /** Value of objective function 1 ... returns VSPAERO_DOUBLE **/

    void OptimizationFunction(VSPAERO_DOUBLE &Vec) { OptimizationFunction(1, Vec); };
    
    /** Value of objective function Case ... returns VSPAERO_DOUBLE **/
    
    void OptimizationFunction(int Case, VSPAERO_DOUBLE &Vec);

    /** Value of objective vector function ... returns VSPAERO_DOUBLE **/

    void OptimizationFunction(VSPAERO_DOUBLE *Vec) { OptimizationFunction(1,Vec); };   

    /** Value of vector objective function case ... returns VSPAERO_DOUBLE **/

    void OptimizationFunction(int Case, VSPAERO_DOUBLE *Vec);

    /** Value of vector objective function case, and time Time ... returns VSPAERO_DOUBLE **/

    void OptimizationFunction(int Case, int Time, VSPAERO_DOUBLE *Vec);

#endif

    /** Value of objective function 1 ... returns double **/

    void OptimizationFunction(double &Vec) { OptimizationFunction(1, Vec); };

    /** Value of objective function Case  ... returns double **/

    void OptimizationFunction(int Case, double &Vec);    

    /** Value of objective vector function  ... returns double **/

    void OptimizationFunction(double *Vec) { OptimizationFunction(1, Vec); };  

    /** Value of vector objective function case  ... returns double **/

    void OptimizationFunction(int Case, double *Vec);

    /** Value of vector objective function case, and time Time  ... returns double **/

    void OptimizationFunction(int Case, int Time, double *Vec);
                    
    /** Calculate nodal pressures on the mesh for FEM analysis **/
    
    void CalculateNodalPressures(void);
    
    /** Pressure at node i of the mesh **/
    
    VSPAERO_DOUBLE NodalCp(int i) { return NodalCp_[i]; };
    
    /** Number of spanwise stations for wing **/
    
    int NumberOfSpanStationsForWing(int Wing) { return SpanLoadData(Wing).NumberOfSpanStations(); };
        
    /** i'th spanwise chord value for wing = Wing **/
    
    VSPAERO_DOUBLE SpanWiseChordForWing(int Wing, int i) { return SpanLoadData(Wing).Span_Chord(i); };

    /** i'th spanwise area value for wing = Wing **/
    
    VSPAERO_DOUBLE SpanWiseAreaForWing(int Wing, int i) { return SpanLoadData(Wing).Span_Area(i); };
                
    /** Calculate partial of user input function F wrt to pressures **/
    
    void Optimization_Calculate_pFu_pMesh(double *pUF_pP, double *pUF_pMesh);

    /** Over ride default edge by edge ssor preconditioner **/
    
    int &Preconditioner(void ) { return Preconditioner_; };

    /** Set the user case string **/
    
    char *CaseString(void) { return CaseString_; };
        
    /** Write a Generic File header to FILE id fid **/
    
    void WriteCaseHeader(FILE *fid);
    
    /** Write out a cart3d tri file of the top level mesh **/
    
    void WriteOutCart3dTriFile(void);

};

#include "END_NAME_SPACE.H"

#endif
