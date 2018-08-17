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

#define SOLVER_JACOBI 1
#define SOLVER_GMRES  2

#define JACOBI 1
#define SSOR   2
#define MATCON 3

#define SYM_X 1
#define SYM_Y 2
#define SYM_Z 3

#define FORCE_AVERAGE  1
#define FORCE_UNSTEADY 2

#define IMPULSE_ANALYSIS 1
#define HEAVE_ANALYSIS   2
#define P_ANALYSIS       3
#define Q_ANALYSIS       4
#define R_ANALYSIS       5
#define PATH_ANALYSIS    6

// Small class for stack list

class STACK_ENTRY {

public:

    int Level;
    int Loop;

};

// Definition of the VSP_SOLVER class

class VSP_SOLVER {

private:

    // Filename
    
    char FileName_[2000];
    
    int iFix_;
    
    // Geometry type
    
    int ModelType_;
    
    // Debug
    
    int Verbose_;
    
    // Test run
    
    int DumpGeom_;
    
    // Write out 2d FEM output file
    
    int Write2DFEMFile_;
    
    // Turn off KT correction
    
    int NoKarmanTsienCorrection_;
   
    // Function data
    
    void init(void);
    
    // Multigrid level we are solving on
    
    int MGLevel_;
    
    // VSP Geometry
    
    VSP_GEOM VSPGeom_;
    
    // VSP Solver
    
    int NumberOfMGLevels_;
    int NumberOfEquations_;
    int NumberOfKelvinConstraints_;
    int *LoopInKelvinConstraintGroup_;
    int *LoopIsOnBaseRegion_;
    
    // Rotor Geoemtry
    
    int NumberOfRotors_;
    
    ROTOR_DISK *RotorDisk_;
    
    // Blade analysis
    
    int RotorAnalysis_;
    double BladeRPM_;

    // Control Surface Group information (used for case header)

    int NumberOfControlGroups_;
    CONTROL_SURFACE_GROUP *ControlSurfaceGroup_;

    // Velocity survey points
    
    int NumberofSurveyPoints_;
    VSP_NODE *SurveyPointList_;    
    
    // Solver routines and data
    
    int TimeAnalysisType_;
    int TimeAccurate_;
    int NumberOfTimeSteps_;
    int NumberOfTimeSamples_;
    int Time_;
    double TimeStep_;
    double CurrentTime_;
    
    double ReducedFrequency_;
    double Unsteady_AngleRate_; // Rad/s
    double Unsteady_Angle_;     // Rad
    double Unsteady_AngleMax_;  // Rad    
    double Unsteady_H_;
    double Unsteady_HMax_;

    double FreeStreamVelocity_[3];
    double WakeAngle_[3];
    
    int NumberOfWakeTrailingNodes_;
    
    int SetFarFieldDist_;
    double FarFieldDist_;
    
    double RotationalRate_[3];
    
    double **LocalFreeStreamVelocity_;
    double **UnsteadyTrailingWakeVelocity_;
    double **LocalBodySurfaceVelocity_;
    
    int NumberOfSurfaceVortexEdges_;
    int NumberOfTrailingVortexEdges_;
    int NumberOfVortexLoops_;
    int NumberOfSurfaceNodes_;
    
    int NoWakeIteration_;
    int WakeIterations_;
    int CurrentWakeIteration_;
    
    int DoSymmetryPlaneSolve_;

    int Preconditioner_;
    
    int CalculateVortexLift_;
    int CalculateLeadingEdgeSuction_;

    double *Gamma_;
    double *GammaNM1_;
    double *GammaNM2_;
    double *RightHandSide_;
    double *Residual_;
    double *Diagonal_;
    double MaxDiagonal_;
    double *Delta_;
    double JacobiRelaxationFactor_;
    double L2Residual_;
    
    int NumberOfMatrixPreconditioners_;    
    MATPRECON *MatrixPreconditionerList_;
    
    GRADIENT *VorticityGradient_;
    
    double AngleOfAttack_;
    double AngleOfBeta_;
    double AngleZero_;
    double Mach_;
    double Vinf_;
    double Density_;
    double ReCref_;

    double Sref_;
    double Cref_;
    double Bref_;
    
    double XYZcg_[3];
            
    int ForceType_;
    int AveragingIteration_;
    int NumberOfAveragingSteps_;
    
    double  QMax_;
    double CpMin_;
    double CpMax_;
    double CpBase_;
    double DCpMax_;
    
    double CL_[3];
    double CD_[3];
    double CS_[3];    
    double CFx_[3];
    double CFy_[3];
    double CFz_[3];
    double CMx_[3];
    double CMy_[3];
    double CMz_[3];
    
    double *CL_Unsteady_;
    double *CD_Unsteady_;
    double *CS_Unsteady_;    
    double *CFx_Unsteady_;
    double *CFy_Unsteady_;
    double *CFz_Unsteady_;
    double *CMx_Unsteady_;
    double *CMy_Unsteady_;
    double *CMz_Unsteady_;
    
    double MaxTurningAngle_;
    double Clmax_2d_;
    double CDo_;
        
    double **Span_Cx_;
    double **Span_Cy_;
    double **Span_Cz_;
    double **Span_Cxi_;
    double **Span_Cyi_;
    double **Span_Czi_;    
    double **Span_Cmx_;
    double **Span_Cmy_;
    double **Span_Cmz_;
    double **Span_Cn_;
    double **Span_Cl_;
    double **Span_Cs_;
    double **Span_Cd_;
    double **Span_LE_;
    double **Span_Yavg_;
    double **Span_Area_;
    double **Local_Vel_;

    double Xmin_, Xmax_;
    double Ymin_, Ymax_;
    double Zmin_, Zmax_;
    
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
    
    VORTEX_SHEET *VortexSheet_;
    
    VORTEX_SHEET &VortexSheet(int i) { return VortexSheet_[i]; };
    
    // Vortex/grid edge interaction lists
    
    int *NumberOfVortexEdgesForInteractionListEntry_;

    VSP_EDGE ***SurfaceVortexEdgeInteractionList_;
    
    int NumberOfVortexEdgesForInteractionListEntry(int i) { return NumberOfVortexEdgesForInteractionListEntry_[i]; };
    
    VSP_EDGE &SurfaceVortexEdgeInteractionList(int i, int j) { return *(SurfaceVortexEdgeInteractionList_[i][j]); };
   
    void CalculateMPVelocity(void);

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

    void ProlongateSolutionFromGrid(int Level);
    
    void InterpolateSolutionFromGrid(int Level);

    // Calculate matrix-vector products, and preconditioning

    void DoPreconditionedMatrixMultiply(double *vec_in, double *vec_out);
    
    void DoMatrixPrecondition(double *vec_in);
 
    double *MatrixVecTemp_;
    
    void DoMatrixMultiply(double *vec_in, double *vec_out);
    
    void MatrixMultiply(double *vec_in, double *vec_out);    
 
    void MatrixTransposeMultiply(double *vec_in, double *vec_out);
    
    // GMRES routines
    
    void Do_GMRES_Solve(void);

    void GMRES_Solver(int Neq,                   // Number of Equations, 0 <= i < Neq
                      int IterMax,               // Max number of outer iterations
                      int NumRestart,            // Max number of inner (restart) iterations
                      int Verbose,               // Output flag, verbose = 0, or 1
                      double *x,                 // Initial guess and solution vector
                      double *RightHandSide,     // Right hand side of Ax = b
                      double ErrorMax,           // Maximum error tolerance
                      double ErrorReduction,     // Residual reduction factor
                      double &ResFinal,          // Final log10 of residual reduction
                      int    &IterFinal);        // Final iteration count      

    double VectorDot(int Neq, double *r, double *s);
    
    void ApplyGivensRotation(double c, double s, int k, double *g);

    void CalculateVelocities(void);
    
    void CalculateRightHandSide(void);
       
    // Setup stuff
    
    void DetermineNumberOfKelvinConstrains(void);

    void Setup_VortexLoops(void);
   
    void Setup_VortexEdges(void);
    
    void InitializeTrailingVortices(void);

    void CreateSurfaceVorticesInteractionList(void);
    
    void CalculateSurfaceInducedVelocityAtPoint(double xyz[3], double q[3]);

    void CalculateWingSurfaceInducedVelocityAtPoint(double xyz[3], double q[3]);
    
    VSP_EDGE **CreateInteractionList(int ComponentID, double xyz[3], int &NumberOfInteractionEdges);
    
    int FirstTimeSetup_;
    int MaxStackSize_;
    int **EdgeIsUsed_;    
    int SearchID_;
    
    STACK_ENTRY *LoopStackList_;    
        
    int SolverType_;
    int DoRestart_;
    int SaveRestartFile_;
    
    // ADB file
    
    FILE *ADBFile_;
    FILE *ADBCaseListFile_;
    
    char CaseString_[2000];

    // Restart files
    
    void WriteRestartFile(void);
    void LoadRestartFile(void);
    
    // Status file
    
    FILE *StatusFile_;
    
    // Loads file
    
    FILE *LoadFile_;
    
    double Time1_;
    double Time2_;
    
    // FEM file (beam)
    
    FILE *FEMLoadFile_;
    
    // FEM Analysis
    
    int LoadDeformationFile_;
    
    // 2D Fem loads files
    
    FILE *FEM2DLoadFile_;
    
    void WriteFEM2DGeometry(void);
    void WriteFEM2DSolution(void);
    
    // File IO
    
    void WriteOutAerothermalDatabaseHeader(void);    
    void WriteOutAerothermalDatabaseGeometry(void);
    void WriteOutAerothermalDatabaseSolution(void);
    
    // Unsteady routines
    
    void SaveVortexState(void);
    
    // Carlson's attainable leading edge suction model
    
    double CalculateLeadingEdgeSuctionFraction(double Mach, double ToC, double RoC, double EtaToC, double AoA, double Sweep);
    
public:

    // Constructor, Destructor, Copy

    VSP_SOLVER(void);
   ~VSP_SOLVER(void);
    VSP_SOLVER(const VSP_SOLVER &Solver_);

    // Copy function

    VSP_SOLVER& operator=(const VSP_SOLVER &Solver_);

    // Solver routines

    int ModelType(void) { return ModelType_; };
    
    int &NoWakeIteration(void) { return NoWakeIteration_; };
    int &WakeIterations(void) { return WakeIterations_; };
    
    double &AngleOfAttack(void) { return AngleOfAttack_; };
    double &AngleOfBeta(void) { return AngleOfBeta_; };
    double &Mach(void) { return Mach_; };
    double &Vinf(void) { return Vinf_; };
    double &Density(void) { return Density_; };
    double &ReCref(void) { return ReCref_; };

    double &Xcg(void) { return XYZcg_[0]; };
    double &Ycg(void) { return XYZcg_[1]; };
    double &Zcg(void) { return XYZcg_[2]; };    
    
    double &RotationalRate_p(void) { return RotationalRate_[0]; };
    double &RotationalRate_q(void) { return RotationalRate_[1]; };
    double &RotationalRate_r(void) { return RotationalRate_[2]; };    

    void SetNumberOfRotors(int NumberOfRotors) { NumberOfRotors_ = NumberOfRotors; RotorDisk_ = new ROTOR_DISK[NumberOfRotors_ + 1]; };
    ROTOR_DISK &RotorDisk(int i) { return RotorDisk_[i]; };

    // Control Surface Group information (used for case header)
    void SetControlSurfaceGroup( CONTROL_SURFACE_GROUP * csg, int NumberOfControlGroups ) { ControlSurfaceGroup_ = csg; NumberOfControlGroups_ = NumberOfControlGroups;  };
    CONTROL_SURFACE_GROUP &ControlSurfaceGroup( int i ) { return ControlSurfaceGroup_[i]; };
    
    void DoSymmetryPlaneSolve(int Direction) { DoSymmetryPlaneSolve_ = VSPGeom_.DoSymmetryPlaneSolve() = Direction; };

    void SetFarFieldDist(double FarDist) { SetFarFieldDist_ = 1; FarFieldDist_ = FarDist; };
    
    void SetNumberOfWakeTrailingNodes(int NumberOfNodes) { NumberOfWakeTrailingNodes_ = NumberOfNodes; };
    
    void SetNumberOfSurveyPoints(int NumberOfSurveyPoints) { NumberofSurveyPoints_ = NumberOfSurveyPoints; SurveyPointList_ = new VSP_NODE[NumberofSurveyPoints_ + 1]; };
    VSP_NODE &SurveyPointList(int i) { return SurveyPointList_[i]; };

    // Reference areas and lengths 
    
    double &Sref(void) { return Sref_; };
    double &Cref(void) { return Cref_; };
    double &Bref(void) { return Bref_; };    
 
    // Force and moment coefficients

    double CFx(int Type) { return CFx_[Type]; };
    double CFy(int Type) { return CFy_[Type]; };
    double CFz(int Type) { return CFz_[Type]; };
    double  CL(int Type) { return  CL_[Type]; };
    double  CD(int Type) { return  CD_[Type]; };
    double  CS(int Type) { return  CS_[Type]; };
    double CMx(int Type) { return CMx_[Type]; };
    double CMy(int Type) { return CMy_[Type]; };
    double CMz(int Type) { return CMz_[Type]; };

    double CFx(void) { return CFx_[ForceType_]; };
    double CFy(void) { return CFy_[ForceType_]; };
    double CFz(void) { return CFz_[ForceType_]; };
    double  CL(void) { return  CL_[ForceType_]; };
    double  CD(void) { return  CD_[ForceType_]; };
    double  CS(void) { return  CS_[ForceType_]; };
    double CMx(void) { return CMx_[ForceType_]; };
    double CMy(void) { return CMy_[ForceType_]; };
    double CMz(void) { return CMz_[ForceType_]; };
    
    double  CL_Unsteady(int i) { return  CL_Unsteady_[i]; };
    double  CD_Unsteady(int i) { return  CD_Unsteady_[i]; };
    double  CS_Unsteady(int i) { return  CS_Unsteady_[i]; };   
    double CFx_Unsteady(int i) { return CFx_Unsteady_[i]; };
    double CFy_Unsteady(int i) { return CFy_Unsteady_[i]; };
    double CFz_Unsteady(int i) { return CFz_Unsteady_[i]; };
    double CMx_Unsteady(int i) { return CMx_Unsteady_[i]; };
    double CMy_Unsteady(int i) { return CMy_Unsteady_[i]; };
    double CMz_Unsteady(int i) { return CMz_Unsteady_[i]; }; 
    
    // Zero lift drag

    double CDo(void) { return CDo_; };
    
    // Local, 2D, Clmax te
    
    double &ClMax(void) { return Clmax_2d_; };
    
    double &MaxTurningAngle(void) { return MaxTurningAngle_; };
    
    // Read in the VSP geometry file
    
    void ReadFile(char *FileName) { sprintf(FileName_,"%s",FileName); VSPGeom_.LoadDeformationFile() = LoadDeformationFile_; ModelType_ = VSPGeom_.ReadFile(FileName); };    

    // Read in the FEM deformation file
    
    void LoadFEMDeformationData(void) { VSPGeom_.LoadFEMDeformationData(FileName_); };
    
    // Ground effects analysis
    
    int &DoGroundEffectsAnalysis(void) { return VSPGeom_.DoGroundEffectsAnalysis(); };
    
    double &VehicleRotationAngleVector(int i) { return VSPGeom_.VehicleRotationAngleVector(i); };
    
    double &VehicleRotationAxisLocation(int i) { return VSPGeom_.VehicleRotationAxisLocation(i); };
    
    double &HeightAboveGround(void) { return VSPGeom_.HeightAboveGround(); };
    
    // Access to VSP geometry
    
    VSP_GEOM &VSPGeom(void) { return VSPGeom_; };

    // Solver setup and solve
    
    void Setup(void);
    void Solve(void) { Solve(0); };
    void Solve(int Case);
    void SolveLinearSystem(void);
    void ReCalculateForces(void);
    
    // Wake update 
    
    void UpdateWakeLocations(void);
    
    // Convect wake vorticity along streamlines
    
    void ConvectWakeVorticity(void);
    
    // Calculate the body surface velocities
    
    void CalculateSurfaceMotion(void);
    
    // Update the location of the geometry ... or parts of it
    
    void UpdateGeometryLocation(void);

    // Update the local values of the circulation strengths for each vortex egde
    
    void UpdateVortexEdgeStrengths(int Level, int UpdateType);
    
    // Zero out vortex strengths
    
    void ZeroVortexState(void);
    
    // Calculate Residual
    
    void CalculateResidual(void);
    
    double L2Residual(void) { return L2Residual_; };

    // Force and moment calculations
    
    void CalculateForces(void);
    void CalculatePressureForces(void);
    void CalculateTrefftzForces(void);
    void CalculateTrefftzForcesOld(void);
    
    void CalculateKuttaJukowskiForces(void);
    void CalculateUnsteadyForces(void);    
    void IntegrateForcesAndMoments(int UnsteadyEvaluation);
    void CalculateDeltaCPs(void);
    void CalculateVorticityGradient(void);    
    void CalculateSurfacePressures(void);

void CreateVorticityGradientDataStructure(void);

                                                   
    void CalculateCLmaxLimitedForces(int UnsteadyEvaluation);
    void CalculateVortexBurstLocation(void);

    void CalculateSpanWiseLoading(void);
    void CreateFEMLoadFile(int Case);
    void CreateFEMLoadFileFromVLMSolve(int Case);
    void CreateFEMLoadFileFromPanelSolve(int Case);

    // Field surveys
    
    void CalculateVelocitySurvey(void);
    
    // Set solver method
    
    int &SolverType(void) { return SolverType_; };
    
    // Force a restart
        
    int &DoRestart(void) { return DoRestart_; };
    int &SaveRestartFile(void) { return SaveRestartFile_; };
    
    // Output results file
    
    void OutputStatusFile(int Type);
    void OutputZeroLiftDragToStatusFile(void);
    
    // Force geometry dump, and no solve
    
    int &DumpGeom(void) { return DumpGeom_; };
    
    // Use force averaging
    
    int &ForceType(void) { return ForceType_; };
    int &AveragingIteration(void) { return AveragingIteration_; };
    
    // FEM Analysis
    
    int &LoadFEMDeformation(void) { return LoadDeformationFile_; };
    
    int &Write2DFEMFile(void) { return Write2DFEMFile_; };
    
    int &NoKarmanTsienCorrection(void) { return NoKarmanTsienCorrection_; };
    
    // Unsteady analysis
    
    int &TimeAnalysisType(void) { return TimeAnalysisType_; };
    int &TimeAccurate(void) { return TimeAccurate_; };
    int &NumberOfTimeSteps(void) { return NumberOfTimeSteps_; };
    int &NumberOfTimeSamples(void) { return NumberOfTimeSamples_; };
    
    double &ReducedFrequency(void) { return ReducedFrequency_; };
    double Unsteady_AngleRate(void) { return Unsteady_AngleRate_; };
    double &Unsteady_AngleMax(void) { return Unsteady_AngleMax_; };
    double &Unsteady_HMax(void) { return Unsteady_HMax_; };
    
    // Blade analysis
            
    int &RotorAnalysis(void) { return RotorAnalysis_; };
    double &BladeRPM(void) { return BladeRPM_; };
    
    // Over ride default edge by edge ssor preconditioner
    
    int &Preconditioner(void ) { return Preconditioner_; };
    
    // Force calculation of leading edge suction and/or vortex lift 
    
    int &CalculateVortexLift(void) { return CalculateVortexLift_; };
    
    int &CalculateLeadingEdgeSuction(void) { return CalculateLeadingEdgeSuction_; };
    
    // User case string
    
    char *CaseString(void) { return CaseString_; };
        
    // Generic File header
    
    void WriteCaseHeader(FILE *fid);

};

#endif
