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
    VSPAERO_DOUBLE CFxo_;
    VSPAERO_DOUBLE CFyo_;
    VSPAERO_DOUBLE CFzo_;
    VSPAERO_DOUBLE CDo_[3];
    VSPAERO_DOUBLE CMxo_;
    VSPAERO_DOUBLE CMyo_;
    VSPAERO_DOUBLE CMzo_;
    
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
    int AdjointSolve_;    
    int FreezeMeshGradients_;
    
    OPTIMIZATION_FUNCTION OptimizationFunctionList_[1001];   
    
    VSPAERO_DOUBLE *pF_pMesh_;
    VSPAERO_DOUBLE *pR_pMesh_;    
    VSPAERO_DOUBLE *pF_pGamma_;
    VSPAERO_DOUBLE **dF_dMesh_;
    
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

    // Setup stuff
    
    static double FarAway_;
    
    VSPAERO_DOUBLE SmoothFactor_;
    
    void DetermineNumberOfKelvinConstrains(void);

    void Setup_VortexLoops(void);
   
    void Setup_VortexEdges(void);
    
    void InitializeTrailingVortices(void);

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
    int MaxStackSize_;
    int ***EdgeIsUsed_;    
    int *SearchID_;
    
    VSPAERO_DOUBLE AdjointTime_;
    
    VSP_EDGE ***TempInteractionList_;

    STACK_ENTRY **LoopStackList_;    
        
    int SolverType_;
    int DoRestart_;
    int SaveRestartFile_;
    
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
    
    // Carlson's attainable leading edge suction model
    
    VSPAERO_DOUBLE CalculateLeadingEdgeSuctionFraction(VSPAERO_DOUBLE Mach, VSPAERO_DOUBLE ToC, VSPAERO_DOUBLE RoC, VSPAERO_DOUBLE EtaToC, VSPAERO_DOUBLE AoA, VSPAERO_DOUBLE Sweep);
  
    // Group force coefficient file(s)
    
    FILE **GroupFile_;

    // Rotor coefficient file(s)
    
    FILE **RotorFile_;
    
    // PSU WOP WOP Coupling
    
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

    // Solver routines

    int ModelType(void) { return ModelType_; };
    int ModelSubType(void) { return SurfaceType_; };
    
    int &NoWakeIteration(void) { return NoWakeIteration_; };
    int &WakeIterations(void) { return WakeIterations_; };
    int &GMRESTightConvergence(void) { return GMRESTightConvergence_; };
    
    VSPAERO_DOUBLE &AngleOfAttack(void) { return AngleOfAttack_; };
    VSPAERO_DOUBLE &AngleOfBeta(void) { return AngleOfBeta_; };
    VSPAERO_DOUBLE &AngleOfAttackZero(void) { return AngleOfAttackZero_; };
    VSPAERO_DOUBLE &AngleOfBetaZero(void) { return AngleOfBetaZero_; };
    VSPAERO_DOUBLE &Mach(void) { return Mach_; };
    VSPAERO_DOUBLE &Machref(void) { return Machref_; };
    VSPAERO_DOUBLE &Vinf(void) { return Vinf_; };
    VSPAERO_DOUBLE &Vref(void) { return Vref_; };
    VSPAERO_DOUBLE &Density(void) { return Density_; };
    VSPAERO_DOUBLE &ReCref(void) { return ReCref_; };

    VSPAERO_DOUBLE &Xcg(void) { return XYZcg_[0]; };
    VSPAERO_DOUBLE &Ycg(void) { return XYZcg_[1]; };
    VSPAERO_DOUBLE &Zcg(void) { return XYZcg_[2]; };    
    
    VSPAERO_DOUBLE &RotationalRate_p(void) { return RotationalRate_[0]; };
    VSPAERO_DOUBLE &RotationalRate_q(void) { return RotationalRate_[1]; };
    VSPAERO_DOUBLE &RotationalRate_r(void) { return RotationalRate_[2]; };    

    // Rotors
    
    void SetNumberOfRotors(int NumberOfRotors) { NumberOfRotors_ = NumberOfRotors; RotorDisk_ = new ROTOR_DISK[NumberOfRotors_ + 1]; };
    ROTOR_DISK &RotorDisk(int i) { return RotorDisk_[i]; };
   
    // Inlet / Nozzles
    
    void SetNumberOfEngineFaces(int NumberOfEngineFaces) { NumberOfEngineFaces_ = NumberOfEngineFaces ; EngineFace_ = new ENGINE_FACE[NumberOfEngineFaces_ + 1]; };
    int NumberOfEngineFaces(void) { return NumberOfEngineFaces_; };
    ENGINE_FACE &EngineFace(int i) { return EngineFace_[i]; };

    // Control Surface Group information (used for case header)
    
    void SetControlSurfaceGroup( CONTROL_SURFACE_GROUP * csg, int NumberOfControlGroups ) { ControlSurfaceGroup_ = csg; NumberOfControlGroups_ = NumberOfControlGroups;  };
    CONTROL_SURFACE_GROUP &ControlSurfaceGroup( int i ) { return ControlSurfaceGroup_[i]; };
    
    void DoSymmetryPlaneSolve(int Direction) { DoSymmetryPlaneSolve_ = VSPGeom_.DoSymmetryPlaneSolve() = Direction; };

    void SetFarFieldDist(VSPAERO_DOUBLE FarDist) { SetFarFieldDist_ = 1; FarFieldDist_ = FarDist; };
    
    void SetNumberOfWakeTrailingNodes(int NumberOfNodes) { NumberOfWakeTrailingNodes_ = NumberOfNodes; };
        
    // Survey points
    
    void SetNumberOfSurveyPoints(int NumberOfSurveyPoints) { NumberofSurveyPoints_ = NumberOfSurveyPoints; SurveyPointList_ = new VSP_NODE[NumberofSurveyPoints_ + 1]; };
    
    VSP_NODE &SurveyPointList(int i) { return SurveyPointList_[i]; };
    
    int &NumberOfSurveyTimeSteps(void) { return NumberOfSurveyTimeSteps_; };
    
    // Quad tree surfaces
    
    void SetNumberOfQuadTrees(int NumberOfQuadTrees) { NumberOfQuadTrees_ = NumberOfQuadTrees; QuadTreeDirection_ = new int[NumberOfQuadTrees_ + 1]; QuadTreeValue_ = new VSPAERO_DOUBLE[NumberOfQuadTrees_ + 1]; };

    int &QuadTreeDirection(int i) { return QuadTreeDirection_[i]; };
    VSPAERO_DOUBLE &QuadTreeValue(int i) { return QuadTreeValue_[i]; };
    
    // Calculate rotor coefficients for a rotor blade analysis
    
    void OutputForcesAndMomentsForGroup(int Group);
     
    void CalculateRotorCoefficientsFromForces(VSPAERO_DOUBLE Thrust, VSPAERO_DOUBLE Moment, VSPAERO_DOUBLE Diameter, VSPAERO_DOUBLE RPM,
                                              VSPAERO_DOUBLE &J, VSPAERO_DOUBLE &CT, VSPAERO_DOUBLE &CQ, VSPAERO_DOUBLE &CP, VSPAERO_DOUBLE &EtaP,
                                              VSPAERO_DOUBLE &CT_h, VSPAERO_DOUBLE &CQ_h, VSPAERO_DOUBLE &CP_h, VSPAERO_DOUBLE &FOM);

    // Reference areas and lengths 
    
    VSPAERO_DOUBLE &Sref(void) { return Sref_; };
    VSPAERO_DOUBLE &Cref(void) { return Cref_; };
    VSPAERO_DOUBLE &Bref(void) { return Bref_; };    
 
    // Force and moment coefficients

    VSPAERO_DOUBLE CFx(int Type) { return CFx_[Type]; };
    VSPAERO_DOUBLE CFy(int Type) { return CFy_[Type]; };
    VSPAERO_DOUBLE CFz(int Type) { return CFz_[Type]; };
    VSPAERO_DOUBLE  CL(int Type) { return  CL_[Type]; };
    VSPAERO_DOUBLE  CD(int Type) { return  CD_[Type]; };
    VSPAERO_DOUBLE  CS(int Type) { return  CS_[Type]; };
    VSPAERO_DOUBLE CMx(int Type) { return CMx_[Type]; };
    VSPAERO_DOUBLE CMy(int Type) { return CMy_[Type]; };
    VSPAERO_DOUBLE CMz(int Type) { return CMz_[Type]; };

    VSPAERO_DOUBLE CDTrefftz(int Type) { return CDTrefftz_[Type]; };

    VSPAERO_DOUBLE CFx(void) { return CFx_[0]; };
    VSPAERO_DOUBLE CFy(void) { return CFy_[0]; };
    VSPAERO_DOUBLE CFz(void) { return CFz_[0]; };
    VSPAERO_DOUBLE  CL(void) { return  CL_[0]; };
    VSPAERO_DOUBLE  CD(void) { return  CD_[0]; };
    VSPAERO_DOUBLE  CS(void) { return  CS_[0]; };
    VSPAERO_DOUBLE CMx(void) { return CMx_[0]; };
    VSPAERO_DOUBLE CMy(void) { return CMy_[0]; };
    VSPAERO_DOUBLE CMz(void) { return CMz_[0]; };
    
    VSPAERO_DOUBLE CDTrefftz(void) { return CDTrefftz_[0]; };
  
    VSPAERO_DOUBLE  CL_Unsteady(int i) { return  CL_Unsteady_[i]; };
    VSPAERO_DOUBLE  CD_Unsteady(int i) { return  CD_Unsteady_[i]; };
    VSPAERO_DOUBLE  CS_Unsteady(int i) { return  CS_Unsteady_[i]; };   
    VSPAERO_DOUBLE CFx_Unsteady(int i) { return CFx_Unsteady_[i]; };
    VSPAERO_DOUBLE CFy_Unsteady(int i) { return CFy_Unsteady_[i]; };
    VSPAERO_DOUBLE CFz_Unsteady(int i) { return CFz_Unsteady_[i]; };
    VSPAERO_DOUBLE CMx_Unsteady(int i) { return CMx_Unsteady_[i]; };
    VSPAERO_DOUBLE CMy_Unsteady(int i) { return CMy_Unsteady_[i]; };
    VSPAERO_DOUBLE CMz_Unsteady(int i) { return CMz_Unsteady_[i]; }; 
    
    VSPAERO_DOUBLE CFxo(void) { return CFxo_; };
    VSPAERO_DOUBLE CFyo(void) { return CFyo_; };
    VSPAERO_DOUBLE CFzo(void) { return CFzo_; };
        
    VSPAERO_DOUBLE CMxo(void) { return CMxo_; };
    VSPAERO_DOUBLE CMyo(void) { return CMyo_; };
    VSPAERO_DOUBLE CMzo(void) { return CMzo_; };
    
    // Zero lift drag

    VSPAERO_DOUBLE CDo(int Type) { return CDo_[Type]; };
    VSPAERO_DOUBLE CDo(void) { return CDo_[0]; };
    
    // Local, 2D, Clmax te

    VSPAERO_DOUBLE &Clo2D(void) { return Clo_2d_; };
    VSPAERO_DOUBLE &ClMax(void) { return Clmax_2d_; };
    
    VSPAERO_DOUBLE &MaxTurningAngle(void) { return MaxTurningAngle_; };
    
    // Read in the VSP geometry file
    
    void ReadFile(char *FileName) { sprintf(FileName_,"%s",FileName); VSPGeom_.LoadDeformationFile() = LoadDeformationFile_;  VSPGeom_.ReadFile(FileName,ModelType_,SurfaceType_); };    

    // Read in the FEM deformation file
    
    void LoadFEMDeformationData(void) { VSPGeom_.LoadFEMDeformationData(FileName_); };
    
    // Ground effects analysis
    
    int &DoGroundEffectsAnalysis(void) { return VSPGeom_.DoGroundEffectsAnalysis(); };
    
    VSPAERO_DOUBLE &VehicleRotationAngleVector(int i) { return VSPGeom_.VehicleRotationAngleVector(i); };
    
    VSPAERO_DOUBLE &VehicleRotationAxisLocation(int i) { return VSPGeom_.VehicleRotationAxisLocation(i); };
    
    VSPAERO_DOUBLE &HeightAboveGround(void) { return VSPGeom_.HeightAboveGround(); };
    
    // Access to VSP geometry
    
    VSP_GEOM &VSPGeom(void) { return VSPGeom_; };

    // Solver setup and solve
    
    void Setup(void);
    void Solve(void) { Solve(0); };
    void Solve(int Case);
    void SolveLinearSystem(void);
    void ReCalculateForces(void);    
    void WriteOutNoiseFiles(int Case);  
    void WriteOutTimeAccurateNoiseFiles(int Case);  
    void WriteOutSteadyStateNoiseFiles(int Case);  
      
    // Wake update 
    
    void UpdateWakeLocations(void);
    
    // Calculate the unsteady wake velocities
    
    void CalculateUnsteadyWakeVelocities(void);
    
    // Calculate the body surface velocities
    
    void CalculateSurfaceMotion(void);
    
    // Update the location of the geometry ... or parts of it
    
    void UpdateGeometryLocation(int DoStartUp);
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
        
    // Do a matrix vector product using a user supplied vector... and also return the right hand side

#if not defined AUTODIFF && not defined COMPLEXDIFF
    void CalculateMatrixVectorProductAndRightHandSide(double *VecIn, double *VecOut, double *RHS);
#endif

#if defined AUTODIFF
void CalculateAdjointMatrixVectorProductAndRightHandSide(double *VecIn, double *VecOut, double *RHS);
#endif
    
    VSPAERO_DOUBLE L2Residual(void) { return L2Residual_; };

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

    // Field surveys
    
    void CalculateVelocitySurvey(int Case);
    
    // Set solver method
    
    int &SolverType(void) { return SolverType_; };
    
    // Force a restart
        
    int &DoRestart(void) { return DoRestart_; };
    int &SaveRestartFile(void) { return SaveRestartFile_; };
    
    // Output results file
    
    void OutputStatusFile(int Case);
    void OutputZeroLiftDragToStatusFile(void);
    
    // Force geometry dump, and no solve
    
    int &DumpGeom(void) { return DumpGeom_; };
    
    // Create a boundary conditions setup file
    
    int &CreateHighLiftFile(void) { return CreateHighLiftFile_; };

    // FEM Analysis
    
    int &LoadFEMDeformation(void) { return LoadDeformationFile_; };
    
    int &Write2DFEMFile(void) { return Write2DFEMFile_; };

    int &KarmanTsienCorrection(void) { return KarmanTsienCorrection_; };
    
    // Quad Tree Slicing
    
    void CreateQuadTreePlaneDataStructures(void);
    void CalculateQuadTreeVelocitySurvey(int Case);
    
    int InsideBody(VSPAERO_DOUBLE xyz[3], VSPAERO_DOUBLE distance);
        
    // Unsteady analysis
    
    int &TimeAnalysisType(void) { return TimeAnalysisType_; };
    int &TimeAccurate(void) { return TimeAccurate_; };
    int &StartFromSteadyState(void) { return StartFromSteadyState_; };    
    int &NumberOfTimeSteps(void) { return NumberOfTimeSteps_; };
    int &NumberOfTimeSamples(void) { return NumberOfTimeSamples_; };
    int &DoHoverRampFreeStream(void) { return DoHoverRampFreeStream_; };
    int &PanelSpanWiseLoading(void) { return PanelSpanWiseLoading_; };
    
    VSPAERO_DOUBLE &HoverRampFreeStreamVelocity(void) { return HoverRampFreeStreamVelocity_; };
    
    VSPAERO_DOUBLE &ReducedFrequency(void) { return ReducedFrequency_; };
    VSPAERO_DOUBLE Unsteady_AngleRate(void) { return Unsteady_AngleRate_; };
    VSPAERO_DOUBLE &Unsteady_AngleMax(void) { return Unsteady_AngleMax_; };
    VSPAERO_DOUBLE &Unsteady_HMax(void) { return Unsteady_HMax_; };
    
    VSPAERO_DOUBLE &TimeStep(void) { return TimeStep_; };
    
    // Blade analysis
            
    int &RotorAnalysis(void) { return RotorAnalysis_; };
    VSPAERO_DOUBLE &BladeRPM(void) { return BladeRPM_; };
    
    // Noise Analysis
    
    int &NoiseAnalysis(void) { return NoiseAnalysis_; };
    int &SteadyStateNoise(void) { return SteadyStateNoise_; };    
    int &WopWopEnglishUnits(void) { return WopWopEnglishUnits_; };    
    int &WopWopPeriodicity(void) { return WopWopPeriodicity_; };
    int &WopWopWriteOutADBFile(void) { return WopWopWriteOutADBFile_; };
    
    void DoWopWopFlyBy(void) { WopWopFlyBy_ = 1; };
    void DoWopWopFootPrint(void) { WopWopFlyBy_ = 0; };
    
    VSPAERO_DOUBLE &WopWopUserStartTime(void) { return WopWopUserStartTime_; };
    VSPAERO_DOUBLE &WopWopUserFinishTime(void) { return WopWopUserFinishTime_; };
    
    // Solve and Adjoint stuff
    
    void Optimization_Solve(void) { Optimization_Solve(0); };
    
    void Optimization_Solve(int Case);    
        
    void RestartAndInterrogateSolution(int Case);
    
    int &NumberOfOptimizationFunctions(void) { return NumberOfOptimizationFunctions_; };

    void SetOptimizationFunction(int i, int OptFunctionValue);
    void SetOptimizationFunction(int i, int OptFunctionValue, int Set );
    void SetOptimizationFunctionInputGradientVector(double *Vec) { SetOptimizationFunctionInputGradientVector(1,1,1,Vec); };
    void SetOptimizationFunctionInputGradientVector(int Case, int Length, int Set, double *Vec);

    void CalculateOptimizationFunctionLength(void);
   
    int OptimizationFunctionLength(int Case) { return OptimizationFunctionList_[Case].FunctionLength(); };

    int &DoAdjointSolve(void) { return DoAdjointSolve_; };

    int &OptimizationSolve(void) { return OptimizationSolve_; };
    
    int &DoSolutionInterrogation(void) { return DoSolutionInterrogation_; };
 
    double dF_dMesh_X(int i) { return DOUBLE(dF_dMesh_[1][3*i-2]); };
    double dF_dMesh_Y(int i) { return DOUBLE(dF_dMesh_[1][3*i-1]); };
    double dF_dMesh_Z(int i) { return DOUBLE(dF_dMesh_[1][3*i  ]); };

    double dF_dMesh_X(int p, int i) { return DOUBLE(dF_dMesh_[p][3*i-2]); };
    double dF_dMesh_Y(int p, int i) { return DOUBLE(dF_dMesh_[p][3*i-1]); };
    double dF_dMesh_Z(int p, int i) { return DOUBLE(dF_dMesh_[p][3*i  ]); };
        
    VSPAERO_DOUBLE OptimizationFunction(void);
    VSPAERO_DOUBLE OptimizationFunction(int Case);

#if defined AUTODIFF || defined COMPLEXDIFF
    void OptimizationFunction(VSPAERO_DOUBLE &Vec) { OptimizationFunction(1, Vec); };
    void OptimizationFunction(int Case, VSPAERO_DOUBLE &Vec);
    void OptimizationFunction(VSPAERO_DOUBLE *Vec) { OptimizationFunction(1,Vec); };   
    void OptimizationFunction(int Case, VSPAERO_DOUBLE *Vec);
#endif

    void OptimizationFunction(double &Vec) { OptimizationFunction(1, Vec); };
    void OptimizationFunction(int Case, double &Vec);    
    void OptimizationFunction(double *Vec) { OptimizationFunction(1, Vec); };  
    void OptimizationFunction(int Case, double *Vec);
                
    // Nodal quantities of stuff
    
    void CalculateNodalPressures(void);
    void Optimization_Calculate_pFu_pMesh(double *pUF_pP, double *pUF_pMesh);
    VSPAERO_DOUBLE *Optimization_Calculate_pF_pRotorOmega(int Group);
    
    VSPAERO_DOUBLE NodalCp(int i) { return NodalCp_[i]; };
        
    // Over ride default edge by edge ssor preconditioner
    
    int &Preconditioner(void ) { return Preconditioner_; };

    // User case string
    
    char *CaseString(void) { return CaseString_; };
        
    // Generic File header
    
    void WriteCaseHeader(FILE *fid);

};

#include "END_NAME_SPACE.H"

#endif
