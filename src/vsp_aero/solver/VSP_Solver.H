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
#include "VSP_DegenGeom.H"
#include "Vortex_Trail.H"
#include "RotorDisk.H"
#include "VSPAERO_OMP.H"

#define SOLVER_JACOBI 1
#define SOLVER_GMRES  2

#define SYM_X 1
#define SYM_Y 2
#define SYM_Z 3

#define FORCE_AVERAGE 1

// Small class for stack list

class STACK_ENTRY {

public:

    int Level;
    int Loop;

};

// Definition of the VSP_SOLVER class

class VSP_SOLVER {

private:

    friend class VSP_MGSOLVER;
    
    // Filename
    
    char FileName_[2000];
    
    // Debug
    
    int Verbose_;
    
    // Test run
    
    int DumpGeom_;
    
    // Function data
    
    void init(void);
    
    // Multigrid level we are solving on
    
    int MGLevel_;
    
    // VSP Geometry
    
    VSP_DEGEN_GEOM VSPGeom_;
    
    // VSP Solver
    
    int NumberOfMGLevels_;
    
    // Rotor Geoemtry
    
    int NumberOfRotors_;
    
    ROTOR_DISK *RotorDisk_;
    
    // Velocity survey points

    int NumberofSurveyPoints_;
    VSP_NODE *SurveyPointList_;

    // Solver routines and data

    double FreeStreamVelocity_[3];
    double WakeAngle_[3];
    
    int NumberOfWakeTrailingNodes_;
    
    int SetFarFieldDist_;
    double FarFieldDist_;
    
    double RotationalRate_[3];
    
    double **LocalFreeStreamVelocity_;
    
    int NumberOfSurfaceVortexEdges_;
    int NumberOfTrailingVortexEdges_;
    int NumberOfVortexLoops_;
    
    int NoWakeIteration_;
    int WakeIterations_;
    int CurrentWakeIteration_;
    
    int DoSymmetryPlaneSolve_;

    double *Gamma_;
    double *RightHandSide_;
    double *Residual_;
    double *Diagonal_;
    double *GammaOld_;
    double *Delta_;
    double JacobiRelaxationFactor_;
    double L2Residual_;
    
    double AngleOfAttack_;
    double AngleOfBeta_;
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
    
    double CL_[2];
    double CD_[2];
    double CS_[2];    
    double CFx_[2];
    double CFy_[2];
    double CFz_[2];
    double CMx_[2];
    double CMy_[2];
    double CMz_[2];

    double MaxTurningAngle_;
    double Clmax_2d_;
    double CDo_;
        
    double **Span_Cx_;
    double **Span_Cy_;
    double **Span_Cz_;
    double **Span_Cmx_;
    double **Span_Cmy_;
    double **Span_Cmz_;
    double **Span_Cn_;
    double **Span_Cl_;
    double **Span_Cs_;
    double **Span_Cd_;
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
    
    // Multi Grid Routines

    void RestrictSolutionFromGrid(int Level);

    void ProlongateSolutionFromGrid(int Level);
    
    void InterpolateSolutionFromGrid(int Level);

    // Calculate matrix-vector products, and preconditioning

    void DoPreconditionedMatrixMultiply(double *vec_in, double *vec_out);
    
    void DoMatrixPrecondition(double *vec_in);
    
    void DoMatrixMultiply(double *vec_in, double *vec_out);

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
       
    // Setup stuff
    
    void Size_Lists(void);

    void Setup_VortexLoops(void);
   
    void Setup_VortexEdges(void);
    
    void InitializeTrailingVortices(void);
    
    void CreateInteractionList(void);
   
    void CreateSurfaceVorticesInteractionList(void);
    
    void CalculateSurfaceInducedVelocityAtPoint(double xyz[3], double q[3]);

    void CalculateWingSurfaceInducedVelocityAtPoint(double xyz[3], double q[3]);
    
    VSP_EDGE **CreateInteractionList(double xyz[3], int &NumberOfInteractionEdges);
    
    int FirstTimeSetup_;
    int MaxStackSize_;
    int **EdgeIsUsed_;    
    int SearchID_;
    
    STACK_ENTRY *LoopStackList_;    
        
    int SolverType_;
    int FirstTimeSolve_;    
    int DoRestart_;
    int SaveRestartFile_;

    // Restart files
    
    void WriteRestartFile(void);
    void LoadRestartFile(void);
    
    // Status file
    
    FILE *StatusFile_;
    
public:

    // Constructor, Destructor, Copy

    VSP_SOLVER(void);
   ~VSP_SOLVER(void);
    VSP_SOLVER(const VSP_SOLVER &Solver_);

    // Copy function

    VSP_SOLVER& operator=(const VSP_SOLVER &Solver_);

    // Solver routines
    
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
    
    void DoSymmetryPlaneSolve(int Direction) { DoSymmetryPlaneSolve_ = Direction; };

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
    
    // Zero lift drag

    double CDo(void) { return CDo_; };
    
    // Local, 2D, Clmax te
    
    double &ClMax(void) { return Clmax_2d_; };
    
    double &MaxTurningAngle(void) { return MaxTurningAngle_; };

    // Read in the VSP degenerate geometry file
    
    void ReadFile(char *FileName) { sprintf(FileName_,"%s",FileName); VSPGeom_.ReadFile(FileName); };    
    
    // Access to DegenGeom geometry

    VSP_DEGEN_GEOM &VSPGeom(void) { return VSPGeom_; };

    // Solver setup and solve
    
    void Setup(void);
    void Solve(void) { Solve(0); };
    void Solve(int Case);
    void SolveLinearSystem(void);
    
    // Wake update 
    
    void UpdateWakeLocations(void);
 
    // Update the local values of the circulation strengths for each vortex egde
    
    void UpdateVortexEdgeStrengths(int Level);
    
    // Calculate Residual
    
    void CalculateResidual(void);
    
    double L2Residual(void) { return L2Residual_; };

    // Force and moment calculations

    void CalculateForces(void);
    void CalculatePressureForces(void);
    void CalculateInducedDrag(void);
    void CalculateDeltaCPs(void);
    void CalculateCLmaxLimitedForces(void);

    void CalculateSpanWiseLoading(void);
    void WriteOutAerothermalDatabaseFiles(void);

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

};

#endif
