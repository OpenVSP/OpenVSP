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
#include "memory.H"
#include "quat.H"
#include "MatPrecon.H"
#include "Gradient.H"
#include "MergeSort.H"
#include "Interaction.H"
#include "InteractionLoop.H"
#include "FastMatrix.H"
#include "ComponentGroup.H"
#include "QuadTree.H"
#include "EngineFace.H"
#include "OptimizationFunction.H"
#include "AdjointGradient.H"
#include "SaveState.H"
#include "CharSizes.H"

#include "START_NAME_SPACE.H"

#define SOLVER_JACOBI 1
#define SOLVER_GMRES  2

#define JACOBI 1
#define MATCON 2

#define FORCE_UNSTEADY 1

#define IMPULSE_ANALYSIS 1
#define HEAVE_ANALYSIS   2
#define P_ANALYSIS       3
#define Q_ANALYSIS       4
#define R_ANALYSIS       5

#define NOISE_LINEAR_INTERPOLATION          1
#define NOISE_QUADRATIC_INTERPOLATION       2
#define NOISE_CUBIC_INTERPOLATION           3
#define NOISE_QUARTIC_INTERPOLATION         4
#define NOISE_CUBIC_HERMITE_INTERPOLATION   5
#define NOISE_QUINTIC_HERMITE_INTERPOLATION 6

#define GEOMETRY_UPDATE_DO_ALL     1
#define GEOMETRY_UPDATE_DO_STARTUP 2
#define GEOMETRY_UPDATE_DO_ADJOINT 3

#define ADJOINT_INVISCID_AND_VISCOUS_FORCES          0
#define ADJOINT_WAKE_AND_VISCOUS_FORCES              1
#define ADJOINT_INVISCID_AND_WAKE_AND_VISCOUS_FORCES 3
#define ADJOINT_INVISCID_FORCES                      4
#define ADJOINT_VISCOUS_FORCES                       5
#define ADJOINT_WAKE_FORCES                          6
#define ADJOINT_TOTAL_FORCES                         7
#define ADJOINT_TOTAL_FORCES_USING_WAKE_FORCES       8

// Definition of the VSP_SOLVER class

class VSP_SOLVER {

private:

    // Timing stuff
    
    double TotalTime[5];
    
    double StartTime_;
    double StartSolveTime_;

    // Filename
    
    char FileName_[MAX_CHAR_SIZE];
    
    int iFix_;

    // Debug
    
    int Verbose_;
    
    // Test run
    
    int DumpGeom_;
    
    // Create boundary conditions file
    
    int CreateHighLiftFile_;
    
    // Write out 2d FEM output file
    
    int Write2DFEMFile_;
    
    // Write out tecplot file
    
    int WriteTecplotFile_;
    
    // Turn off KT correction
    
    int KarmanTsienCorrection_;    

    // Function data
    
    void init(void);
    
    // Multigrid level we are solving on
    
    int MGLevel_;
    
    // User specified mglevel to perform solve on
    
    int SolveOnMGLevel_;
    
    // VSP Geometry
    
    VSP_GEOM VSPGeom_;
    
    // VSP Solver
    
    int NumberOfThreads_;    
    int NumberOfMGLevels_;
    int NumberOfEquations_;
   
    // Kelvin constraints
    
    int NumberOfKelvinConstraints_;
    int *LoopInKelvinConstraintGroup_;
    int *LoopIsOnBaseRegion_;
    
    double *KelvinGroupSum_;    
    double KelvinLambda_;
    
    // Rotor Geoemtry
    
    int NumberOfRotors_;
    
    ROTOR_DISK *RotorDisk_;
    
    // Inlet / Nozzle flags
    
    int NumberOfEngineFaces_;
    int *SurfaceIsOnEngineFace_;
    
    ENGINE_FACE *EngineFace_;

    // Control Surface Group information (used for case header)

    int NumberOfControlGroups_;
    CONTROL_SURFACE_GROUP *ControlSurfaceGroup_;

    // Velocity survey points
    
    int NumberOfSurveyTimeSteps_;
    int NumberofSurveyPoints_;
    double SurveyPointsStartTime_;    
    VSP_NODE *SurveyPointList_;    
    
    // Quad tree survey
    
    int NumberOfQuadTrees_;
    int *QuadTreeDirection_;
    int QuadTreeBufferLevels_;
    double *QuadTreeValue_;
    QUAD_TREE *QuadTreeList_;
    FILE *QUADTREECaseListFile_;
    
    // Solver routines and data
    
    int TimeAnalysisType_;
    int TimeAccurate_;
    int StartFromSteadyState_;
    int NumberOfTimeSteps_;
    int StartAveragingTimeStep_;
    int NumberOfTimeSamples_;
    int Time_;
    int WakeStartingTime_;
    int PanelSpanWiseLoading_;
    int AveragingHasStarted_;
    int NumberOfAveragingSets_;
    int StallModelIsOn_;
    int NoADBFile_;
    
    double SigmaAvg_;
    double DeltaTime_;
    double CurrentTime_;

    double ReducedFrequency_;
    double Unsteady_AngleRate_; // Rad/s
    double Unsteady_Angle_;     // Rad
    double Unsteady_AngleMax_;  // Rad    
    double Unsteady_H_;
    double Unsteady_HMax_;

    double WopWopOmegaMin_;
    double WopWopOmegaMax_;
    
    int NoiseAnalysis_;
    int NoiseTimeRefinement_;
    int NoiseTime_;
    int NumberOfNoiseTimeSteps_;
    int SteadyStateNoise_;
    int NoiseInterpolation_;
    
    double NoiseDeltaTime_;
    double NoiseTimeShift_;
    double CurrentNoiseTime_;
        
    int NumberOfComponentGroups_;

    double FreeStreamVelocity_[3];
    
    double WakeAngle_[3];

    double RotationalRate_[3];
    
    double **LocalBodySurfaceVelocityForLoop_;
    double **LocalBodySurfaceVelocityForEdge_;
    
    int ImplicitWake_;    
    int ImplicitWakeStartIteration_;
    int FreezeWakeAtIteration_;
    int WakeIterations_;
    int CurrentWakeIteration_;
    int FreezeWakeRootVortices_;
    int GMRESTightConvergence_;
    int FreezeMultiPoleAtIteration_;
    int DoSymmetryPlaneSolve_;

    int Preconditioner_;    
    int UpdateMatrixPreconditioner_;
    int UseWakeNodeMatrixPreconditioner_;
        
    int DoVortexStretching_;
    
    double ForwardGMRESConvergenceFactor_;
    double AdjointGMRESConvergenceFactor_;
    
    double NonLinearConvergenceFactor_;
    
    double WakeRelax_;
    
    double CoreSizeFactor_;
    
    double UserSpecifiedCutOffFactor_;

    double *Gamma_[3];
    double &Gamma(int i) { return Gamma_[0][i]; };
    double &GammaNM0(int i) { return Gamma_[0][i]; };
    double &GammaNM1(int i) { return Gamma_[1][i]; };
    double &GammaNM2(int i) { return Gamma_[2][i]; };
 
    double *DeltaGamma_;
    double &dGamma(int i) { return DeltaGamma_[i]; };
    
    double *DeltaXYZ_;
    double &dXYZ(int i) { return DeltaXYZ_[i]; };    
    double &dX(int i) { return DeltaXYZ_[3*i-2]; };    
    double &dY(int i) { return DeltaXYZ_[3*i-1]; };    
    double &dZ(int i) { return DeltaXYZ_[3*i  ]; };    
 
    int NumberOfNoiseInterpolationPoints_;
    
    double *GammaNoise_[9];
    double *FxNoise_[9];
    double *FyNoise_[9];
    double *FzNoise_[9];
    double *dCpUnsteadyNoise_[9];
    double *UNoise_[9];
    double *VNoise_[9];
    double *WNoise_[9];

    double *RightHandSide_;
    double *Residual_;
    double *Diagonal_;
    double *MatrixVectorProduct_;
    double MaxDiagonal_;
    double *Delta_;
    double JacobiRelaxationFactor_;
    double L2Residual_;
    double KTResidual_[2];
    
    double MaxResidual_;
    double WakeResidual_;
    
    char ConvergenceLine_[MAX_CHAR_SIZE];
    
    int NumberOfVortexLoopMatrixPreconditioners_;    
    MATPRECON *VortexLoopMatrixPreconditionerList_;

    int NumberOfWakeNodeMatrixPreconditioners_;    
    MATPRECON *WakeNodeMatrixPreconditionerList_;
        
    GRADIENT *VorticityGradient_;
    
    double AngleOfAttack_;
    double AngleOfBeta_;
    double AngleOfAttackZero_;
    double AngleOfBetaZero_;
    double Mach_;
    double Machref_;
    double Vinf_;
    double Vref_;
    double Vmax_;
    double Density_;
    double ReCref_;
    double LastMach_;

    double Sref_;
    double Cref_;
    double Bref_;
    
    double XYZcg_[3];

    double  QMax_;
    double CpMin_;
    double CpMax_;
    double CpBase_;
    double DCpMax_;
    double CpCrit_;
    
    // Invisicid forces and moments
    
    double CLi_;
    double CDi_;
    double CSi_;

    double CFix_;
    double CFiy_;
    double CFiz_;

    double CMix_;
    double CMiy_;
    double CMiz_;
   
    // Viscous forces and moments
   
    double CLo_;
    double CSo_;
    double CDo_;
       
    double CFox_;
    double CFoy_;
    double CFoz_;
        
    double CMox_;
    double CMoy_;
    double CMoz_;
     
    // Trefftz plane-ish forces
   
    double CLw_;
    double CSw_;
    double CDw_;
       
    double CFwx_;
    double CFwy_;
    double CFwz_;
    
    // 2D aero stuff

    double Clo_2d_;
    double Clmax_2d_;
    
    // Stall factor minimum
    
    double MinStallFactor_;

    double **NodalForces_;

    double Xmin_, Xmax_;
    double Ymin_, Ymax_;
    double Zmin_, Zmax_;

    // Fast Matrix routines
    
    FAST_MATRIX FastMatrix_;

    // Initialize the local free stream conditions
        
    void InitializeFreeStream(void);
    
    void UpdateLoopFreeStreamVelocities(void);

    void UpdateEdgeFreeStreamVelocities(void);
    
    void Calculate_Total_FreeStream_Velocity_at_pXYZ(double xyzp[3], double qp[4]);
    
    void Calculate_FreeStream_Velocity_From_RotationRates_at_pXYZ(double xyzp[3], double qp[4]);
    
    void Calculate_FreeStreamVelocity_From_Rotors_And_Engines_At_pXYZ(double xyzp[3], double qp[4]);
    
    void CalculateBodyVelocitiesForQuasiUnsteadyAnalysis(void);

    // Calculate the diagonal of the influence matrix
    
    void CalculateDiagonal(void);

    // Calculate the matrix preconditioners
    
    int CalculateNumberOfFineLoops(int Level, VSP_LOOP &Loop, int *LoopList);
    
    void CreateMatrixPreconditionersDataStructure(void);

    void CreateMatrixPreconditionersDataStructureForVortexLoops(void);

    void CreateMatrixPreconditionersDataStructureForWakeNodes(void);

    void CreateMatrixPreconditioners(void);

    void CreateVortexLoopMatrixPreconditioners(void);
    
    void CreateWakeNodeMatrixPreconditioners(void);

    // Multi Grid Routines

    void RestrictSolutionFromGrid(int Level);

    void RestrictVortexCoreWidthFromGrid(int Level);

    void RestrictAdjointSolutionFromGrid(int Level);
    
    void RestrictKTFactorFromGrid(int Level);
    
    void ProlongateSolutionFromGrid(int Level);
    
    void InterpolateSolutionFromGrid(int Level);
    
    void RestrictPerturbationSolutionFromGrid(int Level);
    
    // Calculate matrix-vector products, and preconditioning

    void DoPreconditionedMatrixMultiply(double *vec_in, double *vec_out);

    void DoForwardPreconditionedMatrixMultiply(double *vec_in, double *vec_out);
    
    void DoAdjointPreconditionedMatrixMultiply(double *vec_in, double *vec_out);
    
    void DoForwardSolvePrecondition(double *vec_in);
    
    void DoAdjointMatrixPrecondition(double *vec_in);
     
    double *MatrixVecTemp_;

    // Forward matrix multiply routines
    
    void DoForwardMatrixMultiply(double *vec_in, double *vec_out);    
    
    void CalculateGammaPerturbationLoopVelocities(void);
    
    void CalculateMeshPerturbationLoopVelocities(void);
    
    void CalculateGammaPerturbationEdgeVelocities(void);
    
    void CalculateMeshPerturbationEdgeVelocities(void);
    
    // Adjoint matrix multiply routines

    void CreateAdjointCoarseEdgeList(void);
      
    void TransposeMatrixMultiply(double *vec_in, double *vec_out);

    void CalculateGammaPerturbationLoopVelocities_Tranpose(double *vec_in, double *vec_out);

    void CalculateMeshPerturbationLoopVelocities_Tranpose(double *vec_in, double *vec_out);

    void CalculateGammaPerturbationEdgeVelocities_Tranpose(double *vec_in, double *vec_out);

    void CalculateMeshPerturbationEdgeVelocitiesOverLoops_Tranpose(double *vec_in, double *vec_out);

    void CalculateKstallPerturbationLoopVelocities_Tranpose(double *vec_in, double *vec_out);

    void CalculateKstallPerturbationEdgeVelocities_Tranpose(double *vec_in, double *vec_out);
    
    void CalculateVortexStretchingRatioPerturbationLoopVelocities_Tranpose(double *vec_in, double *vec_out);
        
    void CalculateVortexStretchingRatioPerturbationEdgeVelocities_Tranpose(double *vec_in, double *vec_out);
    
    void CalculateMeshPerturbationEdgeVelocitiesOverEdges_Tranpose(double *vec_in, double *vec_out);

    void CalculateMachPerturbationVelocities(void);

    void CalculateLoopMachPerturbationVelocities(void);
    
    void CalculateEdgeMachPerturbationVelocities(void);

    void CalculateStallGammaPerturbationLoopVelocities(void);

    void CalculateStallGammaPerturbationEdgeVelocities(void);
        
    void CalculateVortexStretchingRatioGammaPerturbationLoopVelocities(void);
    
    void CalculateVortexStretchingRatioGammaPerturbationEdgeVelocities(void);

    void ProlongateMachLoopVelocity(void);

    void ProlongateMachEdgeVelocity(void);

    double CalculateMeshPiAdjointExpansionOverLoops(int &Level, int &Loop, double q[3]);

    double CalculateGammaPsiAdjointExpansionOverLoops(int &Level, int &Loop, double q[3]);
    
    double CalculateAdjointExpansionOverEdges(int &Level, int &Loop, double q[3][3]);

    void CalculateAdjointWakeGammaMatrixMultiply(int LevelC, int EdgeC, double q[3], double Fact, double  &Coef);

    void CalculateAdjointWakeMeshMatrixMultiply(int LevelC, int EdgeC, 
                                                                      double &dq0_dx1,
                                                                      double &dq0_dy1,
                                                                      double &dq0_dz1,
                                                                      double &dq0_dx2,
                                                                      double &dq0_dy2,
                                                                      double &dq0_dz2,
                                                                      
                                                                      double &dq1_dx1,
                                                                      double &dq1_dy1,
                                                                      double &dq1_dz1,
                                                                      double &dq1_dx2,
                                                                      double &dq1_dy2,
                                                                      double &dq1_dz2,
                                                                      
                                                                      double &dq2_dx1,
                                                                      double &dq2_dy1,
                                                                      double &dq2_dz1,
                                                                      double &dq2_dx2,
                                                                      double &dq2_dy2,
                                                                      double &dq2_dz2,
                                                                      
                                                                      double &dq0_dxp,
                                                                      double &dq0_dyp,
                                                                      double &dq0_dzp,
                                                                      
                                                                      double &dq1_dxp,
                                                                      double &dq1_dyp,
                                                                      double &dq1_dzp,
                                                                      
                                                                      double &dq2_dxp,
                                                                      double &dq2_dyp,
                                                                      double &dq2_dzp, double Fact, double Coef[9]);
                                                                                  
    void CalculateAdjointForceGradient(int LevelC, int EdgeC, double q[3], double Fact, double *Forces, double *Moments, double *qw, double *WakeForces, int NumCases);
 
    void CalculateAdjointForcesOverEdges(int &Level, int &Loop, int iLevel, int iLoop);
   
    void DistributeAdjointExpansionOverLoops(int &iLevel, int &iLoop, double &Coef, double *Vec);

    void CalculateForceGradients(void);

    void CalculateForceGradients_wrt_Gamma(void);
    
    void CalculateForceGradients_wrt_Mesh(void);

    void CalculateForceGradients_wrt_StallFactor(void);
    
    void CalculateForceGradients_wrt_VortexStretchingFactor(void); 
    
    void CalculateTimeAccurateAdjointCouplingTerms(int ForceCase, int Dir, int UnsteadyAdjointCase);
    
    void CalculateTimeAccurateForceCouplingTerms_wrt_Gamma(void);
        
    void CalculateViscousForceGradients(void);

    void DistributeForceGammaVelocities(int LevelC, int LoopC);

    void DistributeForceMeshVelocities(int LevelC, int LoopC);

    void ZeroForceGammaVelocities(int LevelC, int LoopC);

    void ZeroForceMeshVelocities(int LevelC, int LoopC);
 
    void ProlongateGammaForces(void);
 
    void ProlongateMeshForces(void);

    void DistributeAdjointForcesOverLoops(int &iLevel, int &iLoop, double Forces[3], double *pF_pS);

    void CalculateAdjointGammaForcesOverEdges(int &Level, int &Loop, int iLevel, int iLoop);

    void CalculateAdjointMeshForcesOverEdges(int &Level, int &Loop, int iLevel, int Node1, int Node2);

    void DistributeAdjointForcesOverEdges(int &Level, int &Loop, double Forces[3], double *pF_pM);

    void DistributeAdjointExpansionOverNodes(int &Level, int &Node, double Coef[6], double *Vec);

    void CalculateMeshAdjointExpansionOverLoops(int &Level, int &Loop,
                                                double &dq0_dx1,
                                                double &dq0_dy1,
                                                double &dq0_dz1,
                                                double &dq0_dx2,
                                                double &dq0_dy2,
                                                double &dq0_dz2,
                                                            
                                                double &dq1_dx1,
                                                double &dq1_dy1,
                                                double &dq1_dz1,
                                                double &dq1_dx2,
                                                double &dq1_dy2,
                                                double &dq1_dz2,
                                                           
                                                double &dq2_dx1,
                                                double &dq2_dy1,
                                                double &dq2_dz1,
                                                double &dq2_dx2,
                                                double &dq2_dy2,
                                                double &dq2_dz2,
                                                
                                                double &dq0_dxp,
                                                double &dq0_dyp,
                                                double &dq0_dzp,
                                                   
                                                double &dq1_dxp,
                                                double &dq1_dyp,
                                                double &dq1_dzp,
                                                      
                                                double &dq2_dxp,
                                                double &dq2_dyp,
                                                double &dq2_dzp, 
                                                 
                                                double &dq0_dMach,
                                                double &dq1_dMach,
                                                double &dq2_dMach,
                                                                                                                                          
                                                double Coef[9]);

    void CalculatePsiT_PartialResidualPartialMesh(void);

    void CalculatePsiT_PartialResidualPartialUserInputs(int ForceCase, int Dir, double InviscidWeight, double ViscousWeight, double WakeWeight);
                                                                                                                          
    void CalculatePsiT_PartialResidualPartialFreeStream(int ForceCase, int Dir, double InviscidWeight, double ViscousWeight, double WakeWeight);

    void CalculatePsiT_PartialResidualPartialMesh_VorticityEquations(void);

    void CalculatePsiT_PartialResidualPartialMesh_WakeEquations(void);

    void CalculatePsiT_PartialResidualPartialMesh_StallEquations(void);

    void CalculatePsiT_PartialResidualPartialMesh_VortexStretchingRatioEquations(void);

    void TestCalculateLoopNormalGradients_wrt_Mesh(int Level, int Loop);
                                                     
    // Velocities
    
    void ZeroLoopVelocities(void);

    void ZeroEdgeVelocities(void);
   
    void ProlongateLoopVelocity(void);
    
    void ProlongateEdgeVelocity(void);
    
    // Perturbation velocities
    
    void ZeroLoopPerturbationVelocities(void);

    void ZeroEdgePerturbationVelocities(void);
   
    void ProlongateLoopPerturbationVelocity(void);
    
    void ProlongateEdgePerturbationVelocity(void);    

    void RestrictFreeStreamVelocity(void);

    // GMRES routines
    
    void Do_GMRES_Solve(void);

    void GMRES_Solver(int Neq,                           // Number of Equations, 0 <= i < Neq
                      int IterMax,                       // Max number of outer iterations
                      int NumRestart,                    // Max number of inner (restart) iterations
                      int Verbose,                       // Output flag, verbose = 0, or 1
                      double *x,                 // Initial guess and solution vector
                      double *RightHandSide,     // Right hand side of Ax = b
                      double ErrorMax,           // Maximum error tolerance
                      double ErrorReduction,     // Residual reduction factor
                      double &ResFinal,          // Final log10 of residual reduction
                      int    &IterFinal);                // Final iteration count      

    double VectorDot(int Neq, double *r, double *s);
    
    void ApplyGivensRotation(double c, double s, int k, double *g);

    void CalculateVelocities(int UpdateType);
    
    void CalculateLoopVelocities(void);
        
    void CalculateEdgeVelocities(void);
    
    void CalculateRightHandSide(void);
 
    // Adjoint stuff

    void CreateAdjointMatrix(void);

    void CreateAdjointMatrixFull(void);
    
    // Adjoint test code
    
    void TestForceGradients(void);

    void TestInducedForceGradients(void);

    void TestJacobianMatrix(void);

    void TestMatrixPreconditioner(void);

    void TestPartialResdiual_pMesh(void);
    
    void TestPartialResdiual_pMesh_Full(void);

    void TestPartialResdiual_pVinf(void);

    void TestEdgeCentroidGradients(void);
    
    int UnsteadyFDTest_;
    double TestTotalUnsteadyForceIm[3], TestTotalUnsteadyForceVm[3];
    double TestTotalUnsteadyForceIp[3], TestTotalUnsteadyForceVp[3];
 
    double TestTotalUnsteadyMomentIm[3], TestTotalUnsteadyMomentVm[3];
    double TestTotalUnsteadyMomentIp[3], TestTotalUnsteadyMomentVp[3];
            
    void TestTotalUnsteadyForceGradients(void);
            
    int NumberOfUnsteadyAdjointCases_;
    int DoAdjointSolve_;
    int OptimizationSolve_;    
    int AdjointMatrixSolve_;
    
    // Flag to set adjoint force direction
    
    int DoAdjointSolveForThisForceMomentCase_[7];
    
    // Flag to set the adjoint force type... viscous or inviscid...
    
    int AdjointSolutionForceType_;    
    
    // Invsicid force gradients wrt inputs
    
    double DFi_DAlpha_[3];
    double DFi_DBeta_[3];
    double DFi_DP_[3];
    double DFi_DQ_[3];
    double DFi_DR_[3];
    double DFi_DMach_[3];
    double DFi_DCG_[3][3];
    double DFi_DRe_[3];
                 
    double DMi_DAlpha_[3];
    double DMi_DBeta_[3];
    double DMi_DP_[3];
    double DMi_DQ_[3];
    double DMi_DR_[3];
    double DMi_DMach_[3];
    double DMi_DCG_[3][3];
    double DMi_DRe_[3];

    // Viscous force gradients wrt inputs
    
    double DFo_DAlpha_[3];
    double DFo_DBeta_[3];
    double DFo_DP_[3];
    double DFo_DQ_[3];
    double DFo_DR_[3];
    double DFo_DMach_[3];
    double DFo_DCG_[3][3];
    double DFo_DRe_[3];
                     
    double DMo_DAlpha_[3];
    double DMo_DBeta_[3];
    double DMo_DP_[3];
    double DMo_DQ_[3];
    double DMo_DR_[3];
    double DMo_DMach_[3];
    double DMo_DCG_[3][3];
    double DMo_DRe_[3];

    // Wake force gradients wrt inputs
    
    double DFw_DAlpha_[3];
    double DFw_DBeta_[3];
    double DFw_DP_[3];
    double DFw_DQ_[3];
    double DFw_DR_[3];
    double DFw_DMach_[3];
    double DFw_DCG_[3][3];
    double DFw_DRe_[3];

    // Aero Invsicid force gradients wrt inputs
    
    double DAeroFi_DAlpha_[3];
    double DAeroFi_DBeta_[3];
    double DAeroFi_DP_[3];
    double DAeroFi_DQ_[3];
    double DAeroFi_DR_[3];
    double DAeroFi_DMach_[3];
    double DAeroFi_DCG_[3][3];
    double DAeroFi_DRe_[3];
                 
    double DAeroMi_DAlpha_[3];
    double DAeroMi_DBeta_[3];
    double DAeroMi_DP_[3];
    double DAeroMi_DQ_[3];
    double DAeroMi_DR_[3];
    double DAeroMi_DMach_[3];
    double DAeroMi_DCG_[3][3];
    double DAeroMi_DRe_[3];

    // Aero Viscous force gradients wrt inputs
    
    double DAeroFo_DAlpha_[3];
    double DAeroFo_DBeta_[3];
    double DAeroFo_DP_[3];
    double DAeroFo_DQ_[3];
    double DAeroFo_DR_[3];
    double DAeroFo_DMach_[3];
    double DAeroFo_DCG_[3][3];
    double DAeroFo_DRe_[3];
                     
    double DAeroMo_DAlpha_[3];
    double DAeroMo_DBeta_[3];
    double DAeroMo_DP_[3];
    double DAeroMo_DQ_[3];
    double DAeroMo_DR_[3];
    double DAeroMo_DMach_[3];
    double DAeroMo_DCG_[3][3];
    double DAeroMo_DRe_[3];

    // Aero Wake force gradients wrt inputs
    
    double DAeroFw_DAlpha_[3];
    double DAeroFw_DBeta_[3];
    double DAeroFw_DP_[3];
    double DAeroFw_DQ_[3];
    double DAeroFw_DR_[3];
    double DAeroFw_DMach_[3];
    double DAeroFw_DCG_[3][3];
    double DAeroFw_DRe_[3];
    
    // Gradient data with respect to mesh nodes **/
    
    double *pF_pMesh_;
    double *PsiT_pR_pMesh_;    
    double *pF_pSoln_;
    double *pF_pSoln_NP1_[7];

    // Adjoint solution vector
    
    double ***Psi_;
    double *CurrentPsi_;
    
    void Optimization_GMRES_AdjointSolve(void);    
    void DoAdjointMatrixMultiply(double *vec_in, double *vec_out);

    // Setup stuff
    
    static double FarAway_;
    
    double SmoothFactor_;
    
    void DetermineNumberOfKelvinConstrains(void);

    void Setup_VortexLoops(void);
   
    void Setup_VortexEdges(void);
    
    void InitializeTrailingVortices(void);
 
    void CreateSurfaceVorticesInteractionList(void);
 
    void CalculateSurfaceInducedVelocityAtPoint(double xyz[3], double q[3]);
    
    int CalculateSurfaceInducedVelocityAtOffBodyPoint(double xyz[3], double q[3]);

    void CalculateSurfaceInducedVelocityAtPoint(int ComponentID, int pLoop, double xyz[3], double q[3]);
     
    void CalculateSurfaceInducedVelocityAtPoint(double xyz[3], double q[3], double CoreWidth);

    void AdjustNearSurfaceVelocities(double xyz[3], double q[3], int NumberOfEdges, VSP_EDGE **InteractionList);

    INTERACTION_LISTS *CreateInteractionList(int ComponentID, int pLoop, double xyz[3]);

    int NodeIsInsideLoop(VSP_LOOP &Loop, double xyz[3]);

    int FirstTimeSetup_;
    int FirstTimeGradientSetup_;
    int MaxStackSize_;
    int ***EdgeIsUsed_;    
    int ***LoopIsUsed_;
    int *SearchID_;
    int UserAdjointComponentList_[10000];
    int *ComponentIsInAdjointForceAndMomentList_;
        
    double AdjointTime_;
    
    // Interaction data
    
    INTERACTION_LISTS *TempInteractionList_;

    STACK_ENTRY **LoopStackList_;    
        
    int DoRestart_;
    int SaveRestartFile_;
    
    int RestartFromPreviousSolve_;
    
    // Solve stuff
    
    // Solve the forward linear system 
    
    void SolveForwardLinearSystem(void);
        
    // Solve the adjoint linear system 
        
    void SolveAdjointLinearSystem(void);

    // Zero out force and moment gradients
        
    void ZeroForceAndMomentGradients(void);

    // Average force and moment gradients
    
    void AverageForceAndMomentGradients(int NumberOfCases);
    
    // Calculate the force coefficients (cl, cd, cm... etc) wrt user inputs like alpha, beta...
    
    void CalculateForceCoefficientGradients_wrt_Inputs(void);
    
    // Calculate the force coefficients (cl, cd, cm... etc) wrt mesh nodes
    
    void CalculateForceCoefficientGradients_wrt_Mesh(void);
    
    // Calculate control surface derivatives
        
    void CalculateControlSurfaceDerivatives(void);
    
    // Write out the mesh force gradients
    
    void WriteOutMeshGradients(void);

    // Wake update 
    
    void UpdateWakeLocations(void);

    void UpdateWakeLocations(double *VecIn);
    
    // Update the wake convected distance

    void UpdateWakeConvectedDistance(void);
    
    // Update the location of the geometry for time accurate solutions
    
    void UpdateGeometryLocation(void);

    // Calculate the body velocities for moving geometries
        
    void CalculateLocalBodyVelocitiesFromMovingGeometries(int Group);

    // Convect the wake vorticity, and wake node locations
    
    void ConvectUnsteadyWakes(void);    
    
    // Apply the vortex stretching model
    
    void ApplyVortexStretchingModel(void);
    
    // Update the local values of the circulation strengths for each vortex egde

    void UpdateVortexEdgeStrengths(int Level, int UpdateType);
    
    // Update the local values of the perturbation circulation strengths for each vortex egde
    
    void UpdatePerturbationVortexEdgeStrengths(int Level);
    
    // Update the vortex core widths across all the wake loops and edges
    
    void UpdateVortexCoreWidths(int Level, int UpdateType);

    // Zero out the vortex core widths in the wake loops and edges
        
    void ZeroVortexCoreWidths(int Level, int UpdateType);

    // Zero out vortex strengths
    
    void ZeroVortexState(void);

    // Calculate Residual
    
    void CalculateResidual(void);

    // Stall function

    void StallFunction(double X, double &F, double &pF_pX);
    
    // Smooth out the principal part of the integral for supersonic flows
    
    void CalculatePrincipalPart(void);
    
    void SmoothPrincipalPart(void);
            
    // Force and moment calculations
    
    void CalculateForces(void);
    
    void CalculatePressureForces(void);
    
    void CalculateKuttaJukowskiForces(void);
    
    void CalculateUnsteadyForces(void);    

    void IntegrateForcesAndMoments(void);
    
    void CalculateDeltaCPs(void);
    
    void CalculateVorticityGradient(void);    
    
    void CalculateSurfacePressures(void);
    
    void CalculateOptimizationForces(void);

    void CreateVorticityGradientDataStructure(void);

    void CalculateLeastSquaresCoefficients(int Loop1);

    void CalculateSpanWiseLoading(void);

    // Quad Tree Slicing
    
    void CreateQuadTreePlaneDataStructures(void);
    
    void CalculateQuadTreeVelocitySurvey(int Case);
    
    int InsideBody(double xyz[3], double distance);       

    // Field surveys
    
    void CalculateVelocitySurvey(int Case);
        
    // ADB file
    
    int DoSolutionInterrogation_;
    
    FILE *ADBFile_;
    FILE *ADBCaseListFile_;
    
    // Input ADB file ... for noise post-processing
    
    FILE *InputADBFile_;
    
    char CaseString_[MAX_CHAR_SIZE];
    
    // Save states
    
    int MaxNumberOfSavedStates_;

    SAVE_STATE *SavedState_;
    
    SAVE_STATE &SavedState(int i) { return SavedState_[i]; };
    
    void CreateSavedStateStack(int StackSize);
    
    void SaveCurrentState(int SaveStateTime);
    
    void RestoreStateToTime(int SaveStateTime);
    
    void ZeroSolutionState(void);
    
    // Restart files
    
    void WriteRestartFile(void);
    
    void LoadRestartFile(void);
    
    // Status file
    
    FILE *StatusFile_;
    
    // Loads file
    
    FILE *LoadFile_;
    
    double Time1_;
    
    double Time2_;
    
    // 2D Fem loads files
    
    FILE *FEM2DLoadFile_;
    
    void WriteFEM2DGeometry(void);
    
    void WriteFEM2DSolution(void);
    
    // Tecplot file
    
    void WriteOutTecPlotFile(void);

    // Nodal quantities of stuff
    
    double *NodalCp_;
    
    // Survey points file
    
    FILE *SurveyFile_;

    // File IO
    
    void WriteOutAerothermalDatabaseHeader(void);    
    
    void WriteOutAerothermalDatabaseGeometry(void);
    
    void WriteOutAerothermalDatabaseSolution(void);

    void ReadInAerothermalDatabaseHeader(void);    
    
    void ReadInAerothermalDatabaseGeometry(void);
    
    void ReadInAerothermalDatabaseSolution(int TimeCase);

    void InterpolateInTime(double Time, double **ArrayIn, double *ArrayOut, int NumValues);
    
    void InterpolateExistingSolution(double Time);

    void SavePeriodicNoiseSolution(void);

    void RestorePeriodicNoiseSolution(void);

    // Carlson's attainable leading edge suction model
    
    double CalculateLeadingEdgeSuctionFraction(double Mach, double ToC, double RoC, double EtaToC, double AoA, double Sweep);
  
    // Group force coefficient file(s)
    
    FILE **GroupFile_;

    // Rotor coefficient file(s)
    
    FILE **RotorFile_;

public:

    // Constructor, Destructor, Copy

    VSP_SOLVER(void);
   ~VSP_SOLVER(void);
    VSP_SOLVER(const VSP_SOLVER &Solver_);

    // Copy function

    VSP_SOLVER& operator=(const VSP_SOLVER &Solver_);

    /** Turn on implicit wake **/
    
    int &ImplicitWake(void) { return ImplicitWake_; };
    
    /** Iteration at which to start implicit wake solve **/
    
    int &ImplicitWakeStartIteration(void) { return ImplicitWakeStartIteration_; };

    /** Set number of wake iterations **/
    
    int &WakeIterations(void) { return WakeIterations_; };
    
    /** Set the multipole far away value **/
    
    double &FarAway(void) { return FarAway_; };
    
    /** Adjust the wake core size model up or down, default value is 1. **/
    
    double &CoreSizeFactor(void) { return CoreSizeFactor_; };
    
    /** Set the velocity kernel cutoff value **/
    
    double &UserSpecifiedCutOffFactor(void) { return UserSpecifiedCutOffFactor_; };
    
    /** Turn on vortex stretching model **/
    
    int &DoVortexStretching(void) { return DoVortexStretching_; };
    
    /** Turn on tight GMRES convergence **/
    
    int &GMRESTightConvergence(void) { return GMRESTightConvergence_; };

    /** User Forward GMRES residual reduction factor... this scales the default residual reduction
     * ie... ResidualReduction = DefaultResidualReduction * ForwardGMRESConvergenceFactor_
     **/
     
    double &ForwardGMRESConvergenceFactor(void) { return ForwardGMRESConvergenceFactor_; };

    /** User Adjoint GMRES residual reduction factor... this scales the default residual reduction
     * ie... ResidualReduction = DefaultResidualReduction * ForwardGMRESConvergenceFactor_
     **/
     
    double &AdjointGMRESConvergenceFactor(void) { return AdjointGMRESConvergenceFactor_; };
    
    /** User non-linear residual reduction factor on solution of overall nonlinear system
     * ... this scales the default residual reduction
     * ie... ResidualReduction = DefaultResidualReduction * NonLinearConvergenceFactor_
     **/
     
    double &NonLinearConvergenceFactor(void) { return NonLinearConvergenceFactor_; };
        
    /** Wake relaxation factor **/
    
    double &WakeRelax(void) { return WakeRelax_; };
        
    /* Use previous solution as initial guess ... don't use this if the geometry has changed, eg in an optimization run **/
    
    int &RestartFromPreviousSolve(void) { return RestartFromPreviousSolve_; };
 
    /** Set angle of attack **/
    
    double &AngleOfAttack(void) { return AngleOfAttack_; };
    
    /** Set angle of sidesip **/
    
    double &AngleOfBeta(void) { return AngleOfBeta_; };
   
    /** Zero angle of attack, for unsteady periodic runs **/
    
    double &AngleOfAttackZero(void) { return AngleOfAttackZero_; };
    
    /** Zero side slip angle, for unsteady periodic runs **/
    
    double &AngleOfBetaZero(void) { return AngleOfBetaZero_; };
    
    /** Set free stream Mach number **/
        
    double &Mach(void) { return Mach_; };
    
    /** Set free stream velocity ... note that units are up to user to be consistent with geometry! **/
    
    double &Vinf(void) { return Vinf_; };

    /** Set reference Mach number, this requires Vref to be set as well **/
    
    double &Machref(void) { return Machref_; };
    
    /** Set reference velocity.. this is often the tip velocity for a rotor ... should be consistent with Machref **/
    
    double &Vref(void) { return Vref_; };
    
    /** Set free stream density **/
    
    double &Density(void) { return Density_; };
    
    /** Set free stream Reynolds number based on the reference chord, Cref **/
    
    double &ReCref(void) { return ReCref_; };
    
    /** Set Xcg loation **/

    double &Xcg(void) { return XYZcg_[0]; };
    
    /** Set Ycg loation **/
    
    double &Ycg(void) { return XYZcg_[1]; };

    /** Set Zcg loation **/

    double &Zcg(void) { return XYZcg_[2]; };    
    
    /** Set rotational rate about x axis **/
    
    double &RotationalRate_p(void) { return RotationalRate_[0]; };
    
    /** Set rotational rate about y axis **/
    
    double &RotationalRate_q(void) { return RotationalRate_[1]; };

    /** Set rotational rate about z axis **/

    double &RotationalRate_r(void) { return RotationalRate_[2]; };    
    
    /** Set the angular rate, in radian/sec, for component group = Group **/
    
//    double &GroupOmegaRate(int Group) { return ComponentGroupList_[Group].Omega(); };

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
    
    int &DoSymmetryPlaneSolve(void) { return DoSymmetryPlaneSolve_; };
    
    /** Freeze the multipole expansion update **/
    
    int &FreezeMultiPoleAtIteration(void) { return FreezeMultiPoleAtIteration_; };
    
    /** Freeze the wake after this number of iterations **/
    
    int &FreezeWakeAtIteration(void) { return FreezeWakeAtIteration_; };
    
    /** Freeze the root wake vortices **/
    
    int &FreezeWakeRootVortices(void) { return FreezeWakeRootVortices_; };

    /** Set the farfield distance for the trailing wakes.. over rides defaults **/
    
    void SetFarFieldDist(double FarDist) { VSPGeom().SetFarFieldDist() = 1; VSPGeom().FarFieldDist() = FarDist; };
    
    /** Set number of trailing nodes to define the wakes... must be a power of 2 **/
    
    void SetNumberOfWakeTrailingNodes(int NumberOfNodes) { VSPGeom().SetNumberOfWakeTrailingNodes(NumberOfNodes); };
        
    /** Set the number of survey points **/
    
    void SetNumberOfSurveyPoints(int NumberOfSurveyPoints) { NumberofSurveyPoints_ = NumberOfSurveyPoints; SurveyPointList_ = new VSP_NODE[NumberofSurveyPoints_ + 1]; };
    
    /** Access to the survey points objects **/
    
    VSP_NODE &SurveyPointList(int i) { return SurveyPointList_[i]; };
    
    /** Set the number of final time steps to do a survey for unsteady calculatons **/
    
    int &NumberOfSurveyTimeSteps(void) { return NumberOfSurveyTimeSteps_; };
    
    /** Set number of quad trees for flow field interrogation **/
    
    void SetNumberOfQuadTrees(int NumberOfQuadTrees) { NumberOfQuadTrees_ = NumberOfQuadTrees; QuadTreeDirection_ = new int[NumberOfQuadTrees_ + 1]; QuadTreeValue_ = new double[NumberOfQuadTrees_ + 1]; };

    /** Set number of buffer levels for quad tree... the higher the level, the more cells **/
    
    int &QuadTreeBufferLevels(void) { return QuadTreeBufferLevels_; };

    /** Set quad tree direction ... x, y, or z --> 1, 2, or 3 **/
    
    int &QuadTreeDirection(int i) { return QuadTreeDirection_[i]; };
    
    /** Set quad tree plane value... ie x, y, or z location **/
    
    double &QuadTreeValue(int i) { return QuadTreeValue_[i]; };
    
    /** Output the force and moment data for group Group ... **/
    
    void OutputForcesAndMomentsForGroup(int Group);
    
    /** Calculate rotor coefficients given thrust, moment, diameter, rpm... **/
     
    void CalculateRotorCoefficientsFromForces(double Thrust, double Moment, double Diameter, double RPM,
                                              double &J, double &CT, double &CQ, double &CP, double &EtaP,
                                              double &CT_h, double &CQ_h, double &CP_h, double &FOM);

    /** Solution vector for the forward solve ... ie the vorticity **/

    double &ForwardSolutionVector(int i) { return Gamma_[0][i]; };

    /** Solution vector for the adjoint solve ... ie Psi **/

  // double AdjointSolutionVector(int i) { return double(Psi_[1][i]); };
  //     
  // /** P'th solution vector for the adjoint solve ... ie Psi **/
  //     
  // double AdjointSolutionVector(int p, int i) { return double(Psi_[p][i]); };
        
    /** Reference area **/
    
    double &Sref(void) { return Sref_; };
    
    /** Reference chord **/
    
    double &Cref(void) { return Cref_; };
    
    /** Reference Span **/
    
    double &Bref(void) { return Bref_; };    
 
    /** Total x Force coefficient, in x axis direction **/

    double CFtx(void) { return CFix_ + CFox_; };
    
    /** Total y Force coefficient, in y axis direction **/
    
    double CFty(void) { return CFiy_ + CFoy_; };

    /** Total z Force coefficient, in z axis direction **/

    double CFtz(void) { return CFiz_ + CFoz_; };

    /** Total Lift coefficient **/
    
    double  CLt(void) { return CLi_ + CLo_; };
    
    /** Total Drag coefficient **/

    double  CDt(void) { return CDi_ + CDo_; };
    
    /**  Total Side coefficient **/
    
    double  CSt(void) { return CSi_ + CSo_; };
    
    /** Total x moment coefficient about x axis **/
    
    double CMtx(void) { return CMix_ + CMox_; };
    
    /** Total y moment coefficient about y axis **/
    
    double CMty(void) { return CMiy_ + CMoy_; };

    /** Total z moment coefficient about z axis **/

    double CMtz(void) { return CMiz_ + CMoz_; };
    
    /** Total rolling moment coefficient about x axis **/
    
    double CMlt(void) { return -CMix_ - CMox_; };
    
    /** Total pitching moment coefficient about y axis **/
    
    double CMmt(void) { return CMiy_ + CMoy_; };

    /** Total rolling moment coefficient about z axis **/

    double CMnt(void) { return -CMiz_ - CMoz_; };
    
    /** Inviscid x Force coefficient, in x axis direction **/

    double CFix(void) { return CFix_; };
    
    /** Inviscid y Force coefficient, in y axis direction **/
    
    double CFiy(void) { return CFiy_; };

    /** Inviscid z Force coefficient, in z axis direction **/

    double CFiz(void) { return CFiz_; };

    /** Inviscid Lift coefficient **/
    
    double  CLi(void) { return  CLi_; };
    
    /** Inviscid Drag coefficient **/

    double  CDi(void) { return  CDi_; };
    
    /**  Inviscid Side coefficient **/
    
    double  CSi(void) { return  CSi_; };
    
    /** Inviscid x moment coefficient about x axis **/
    
    double CMix(void) { return CMix_; };
    
    /** Inviscid y moment coefficient about y axis **/
    
    double CMiy(void) { return CMiy_; };

    /** Inviscid z moment coefficient about z axis **/

    double CMiz(void) { return CMiz_; };
    
    /** Inviscid rolling moment coefficient about x axis **/
    
    double CMli(void) { return -CMix_; };
    
    /** Inviscid pitching moment coefficient about y axis **/
    
    double CMmi(void) { return CMiy_; };

    /** Inviscid rolling moment coefficient about z axis **/

    double CMni(void) { return -CMiz_; };
    
    /** Viscous force coefficient in x axis direction **/
        
    double CFox(void) { return CFox_; };

    /** Viscous force coefficient in y axis direction **/

    double CFoy(void) { return CFoy_; };
    
    /** Viscous force coefficient in z axis direction **/
    
    double CFoz(void) { return CFoz_; };

    /** Viscous moment coefficient along x axis **/
        
    double CMox(void) { return CMox_; };
    
    /** Viscous moment coefficient along y axis **/
    
    double CMoy(void) { return CMoy_; };

    /** Viscous moment coefficient along z axis **/

    double CMoz(void) { return CMoz_; };

    /** Viscous moment coefficient along x axis **/
        
    double CMlo(void) { return -CMox_; };
    
    /** Viscous moment coefficient along y axis **/
    
    double CMmo(void) { return CMoy_; };

    /** Viscous moment coefficient along z axis **/

    double CMno(void) { return -CMoz_; };
     
    /** Viscous lift coefficient **/

    double CLo(void) { return CLo_; };
    
    /** Viscous drag coefficient **/

    double CDo(void) { return CDo_; };
            
    /** Viscous side force coefficient **/

    double CSo(void) { return CSo_; };
    
    /** Wake + free stream induced force coefficient in x axis direction **/
        
    double CFiwx(void) { return CFwx_; };

    /** Wake + free stream induced force coefficient in y axis direction **/

    double CFiwy(void) { return CFwy_; };
    
    /** Wake + free stream induced force coefficient in z axis direction **/
    
    double CFiwz(void) { return CFwz_; };
    
    /** Wake + free stream induced lift coefficient **/

    double CLiw(void) { return CLw_; };
    
    /** Wake + free induced drag coefficient **/

    double CDiw(void) { return CDw_; };
            
    /** Wake + free induced side force coefficient **/

    double CSiw(void) { return CSw_; };

    /** Viscous + Wake + free stream induced force coefficient in x axis direction **/
        
    double CFtwx(void) { return CFwx_ + CFox_; };

    /** Viscous + Wake + free stream induced force coefficient in y axis direction **/

    double CFtwy(void) { return CFwy_ + CFoy_; };
    
    /** Viscous + Wake + free stream induced force coefficient in z axis direction **/
    
    double CFtwz(void) { return CFwz_ + CFoz_; };
    
    /** Viscous + Wake + free stream induced lift coefficient **/

    double CLtw(void) { return CLw_ + CLo_; };
    
    /** Viscous + Wake + free induced drag coefficient **/

    double CDtw(void) { return CDw_ + CDo_; };
            
    /** Viscous + Wake + free induced side force coefficient **/

    double CStw(void) { return CSw_ + CSo_; };    
    
// Total forces and moments gradients

    /** Partial of total Fx,y,z wrt alpha **/

    double pCFtx_pAlpha(void) { return DFi_DAlpha_[0] + DFo_DAlpha_[0]; };
    double pCFty_pAlpha(void) { return DFi_DAlpha_[1] + DFo_DAlpha_[1]; };
    double pCFtz_pAlpha(void) { return DFi_DAlpha_[2] + DFo_DAlpha_[2]; };
    
    /** Partial of total Fx,y,z wrt beta **/
    
    double pCFtx_pBeta(void) { return DFi_DBeta_[0] + DFo_DBeta_[0]; };
    double pCFty_pBeta(void) { return DFi_DBeta_[1] + DFo_DBeta_[1]; };
    double pCFtz_pBeta(void) { return DFi_DBeta_[2] + DFo_DBeta_[2]; };
    
    /** Partial of total Fx,y,z wrt p roll rate **/
    
    double pCFtx_pP(void) { return DFi_DP_[0] + DFo_DP_[0]; };
    double pCFty_pP(void) { return DFi_DP_[1] + DFo_DP_[1]; };
    double pCFtz_pP(void) { return DFi_DP_[2] + DFo_DP_[2]; };
    
    /** Partial of total Fx,y,z wrt q pitch rate **/
    
    double pCFtx_pQ(void) { return DFi_DQ_[0] + DFo_DQ_[0]; };
    double pCFty_pQ(void) { return DFi_DQ_[1] + DFo_DQ_[1]; };
    double pCFtz_pQ(void) { return DFi_DQ_[2] + DFo_DQ_[2]; };
    
    /** Partial of total Fx,y,z wrt r yaw rate **/
    
    double pCFtx_pR(void) { return DFi_DR_[0] + DFo_DR_[0]; };
    double pCFty_pR(void) { return DFi_DR_[1] + DFo_DR_[1]; };
    double pCFtz_pR(void) { return DFi_DR_[2] + DFo_DR_[2]; };
    
    /** Partial of total Fx,y,z wrt Mach **/
    
    double pCFtx_pMach(void) { return DFi_DMach_[0] + DFo_DMach_[0]; };
    double pCFty_pMach(void) { return DFi_DMach_[1] + DFo_DMach_[1]; };
    double pCFtz_pMach(void) { return DFi_DMach_[2] + DFo_DMach_[2]; };
    
    /** Partial of total Fx,y,z wrt x location of CG **/
    
    double pCFtx_pCGx(void) { return DFi_DCG_[0][0] + DFo_DCG_[0][0]; };
    double pCFty_pCGx(void) { return DFi_DCG_[1][0] + DFo_DCG_[1][0]; };
    double pCFtz_pCGx(void) { return DFi_DCG_[2][0] + DFo_DCG_[2][0]; };

    /** Partial of total Fx,y,z wrt y location of CG **/
    
    double pCFtx_pCGy(void) { return DFi_DCG_[0][1] + DFo_DCG_[0][1]; };
    double pCFty_pCGy(void) { return DFi_DCG_[1][1] + DFo_DCG_[1][1]; };
    double pCFtz_pCGy(void) { return DFi_DCG_[2][1] + DFo_DCG_[2][1]; };

    /** Partial of total or total Fx,y,z wrt z location of CG **/
    
    double pCFtx_pCGz(void) { return DFi_DCG_[0][2] + DFo_DCG_[0][2]; };
    double pCFty_pCGz(void) { return DFi_DCG_[1][2] + DFo_DCG_[1][2]; };
    double pCFtz_pCGz(void) { return DFi_DCG_[2][2] + DFo_DCG_[2][2]; };
    
    /** Partial of total Fx,y,z wrt Re **/
            
    double pCFtx_pRe(void) { return DFi_DRe_[0] + DFo_DRe_[0]; };
    double pCFty_pRe(void) { return DFi_DRe_[1] + DFo_DRe_[1]; };
    double pCFtz_pRe(void) { return DFi_DRe_[2] + DFo_DRe_[2]; };
                
    /** Partial of total Mx,y,z wrt alpha **/
                 
    double pCMtx_pAlpha(void) { return DMi_DAlpha_[0] + DMo_DAlpha_[0]; };
    double pCMty_pAlpha(void) { return DMi_DAlpha_[1] + DMo_DAlpha_[1]; };
    double pCMtz_pAlpha(void) { return DMi_DAlpha_[2] + DMo_DAlpha_[2]; };
    
    /** Partial of inviscid Mx,y,z wrt beta **/
    
    double pCMtx_pBeta(void) { return DMi_DBeta_[0] + DMo_DBeta_[0]; };
    double pCMty_pBeta(void) { return DMi_DBeta_[1] + DMo_DBeta_[1]; };
    double pCMtz_pBeta(void) { return DMi_DBeta_[2] + DMo_DBeta_[2]; };
    
    /** Partial of total Mx,y,z wrt p roll rate **/
    
    double pCMtx_pP(void) { return DMi_DP_[0] + DMo_DP_[0]; };
    double pCMty_pP(void) { return DMi_DP_[1] + DMo_DP_[1]; };
    double pCMtz_pP(void) { return DMi_DP_[2] + DMo_DP_[2]; };
    
    /** Partial of total Fx,y,z wrt q pitch rate **/
    
    double pCMtx_pQ(void) { return DMi_DQ_[0] + DMo_DQ_[0]; };
    double pCMty_pQ(void) { return DMi_DQ_[1] + DMo_DQ_[1]; };
    double pCMtz_pQ(void) { return DMi_DQ_[2] + DMo_DQ_[2]; };
    
    /** Partial of total Mx,y,z wrt r yaw rate **/
    
    double pCMtx_pR(void) { return DMi_DR_[0] + DMo_DR_[0]; };
    double pCMty_pR(void) { return DMi_DR_[1] + DMo_DR_[1]; };
    double pCMtz_pR(void) { return DMi_DR_[2] + DMo_DR_[2]; };
    
    /** Partial of total Mx,y,z wrt Mach **/
    
    double pCMtx_pMach(void) { return DMi_DMach_[0] + DMo_DMach_[0]; };
    double pCMty_pMach(void) { return DMi_DMach_[1] + DMo_DMach_[1]; };
    double pCMtz_pMach(void) { return DMi_DMach_[2] + DMo_DMach_[2]; };
    
    /** Partial of total Mx,y,z wrt x location of CG **/
    
    double pCMtx_pCGx(void) { return DMi_DCG_[0][0] + DMo_DCG_[0][0]; };
    double pCMty_pCGx(void) { return DMi_DCG_[1][0] + DMo_DCG_[1][0]; };
    double pCMtz_pCGx(void) { return DMi_DCG_[2][0] + DMo_DCG_[2][0]; };

    /** Partial of total Mx,y,z wrt y location of CG **/
    
    double pCMtx_pCGy(void) { return DMi_DCG_[0][1] + DMo_DCG_[0][1]; };
    double pCMty_pCGy(void) { return DMi_DCG_[1][1] + DMo_DCG_[1][1]; };
    double pCMtz_pCGy(void) { return DMi_DCG_[2][1] + DMo_DCG_[2][1]; };

    /** Partial of total Mx,y,z wrt y location of CG **/
    
    double pCMtx_pCGz(void) { return DMi_DCG_[0][2] + DMo_DCG_[0][2]; };
    double pCMty_pCGz(void) { return DMi_DCG_[1][2] + DMo_DCG_[1][2]; };
    double pCMtz_pCGz(void) { return DMi_DCG_[2][2] + DMo_DCG_[2][2]; };

    /** Partial of total Mx,y,z wrt Re **/
            
    double pCMtx_pRe(void) { return DMi_DRe_[0] + DMo_DRe_[0]; };
    double pCMty_pRe(void) { return DMi_DRe_[1] + DMo_DRe_[1]; };
    double pCMtz_pRe(void) { return DMi_DRe_[2] + DMo_DRe_[2]; };

// Inviscid forces and moments gradients

    /** Partial of inviscid Fx,y,z wrt alpha **/

    double pCFix_pAlpha(void) { return DFi_DAlpha_[0]; };
    double pCFiy_pAlpha(void) { return DFi_DAlpha_[1]; };
    double pCFiz_pAlpha(void) { return DFi_DAlpha_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt beta **/
    
    double pCFix_pBeta(void) { return DFi_DBeta_[0]; };
    double pCFiy_pBeta(void) { return DFi_DBeta_[1]; };
    double pCFiz_pBeta(void) { return DFi_DBeta_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt p roll rate **/
    
    double pCFix_pP(void) { return DFi_DP_[0]; };
    double pCFiy_pP(void) { return DFi_DP_[1]; };
    double pCFiz_pP(void) { return DFi_DP_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt q pitch rate **/
    
    double pCFix_pQ(void) { return DFi_DQ_[0]; };
    double pCFiy_pQ(void) { return DFi_DQ_[1]; };
    double pCFiz_pQ(void) { return DFi_DQ_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt r yaw rate **/
    
    double pCFix_pR(void) { return DFi_DR_[0]; };
    double pCFiy_pR(void) { return DFi_DR_[1]; };
    double pCFiz_pR(void) { return DFi_DR_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt Mach **/
    
    double pCFix_pMach(void) { return DFi_DMach_[0]; };
    double pCFiy_pMach(void) { return DFi_DMach_[1]; };
    double pCFiz_pMach(void) { return DFi_DMach_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt x location of CG **/
    
    double pCFix_pCGx(void) { return DFi_DCG_[0][0]; };
    double pCFiy_pCGx(void) { return DFi_DCG_[1][0]; };
    double pCFiz_pCGx(void) { return DFi_DCG_[2][0]; };

    /** Partial of inviscid Fx,y,z wrt y location of CG **/
    
    double pCFix_pCGy(void) { return DFi_DCG_[0][1]; };
    double pCFiy_pCGy(void) { return DFi_DCG_[1][1]; };
    double pCFiz_pCGy(void) { return DFi_DCG_[2][1]; };

    /** Partial of inviscid or total Fx,y,z wrt z location of CG **/
    
    double pCFix_pCGz(void) { return DFi_DCG_[0][2]; };
    double pCFiy_pCGz(void) { return DFi_DCG_[1][2]; };
    double pCFiz_pCGz(void) { return DFi_DCG_[2][2]; };
    
    /** Partial of inviscid Fx,y,z wrt Re **/
            
    double pCFix_pRe(void) { return DFi_DRe_[0]; };
    double pCFiy_pRe(void) { return DFi_DRe_[1]; };
    double pCFiz_pRe(void) { return DFi_DRe_[2]; };
                
    /** Partial of inviscid Mx,y,z wrt alpha **/
                 
    double pCMix_pAlpha(void) { return DMi_DAlpha_[0]; };
    double pCMiy_pAlpha(void) { return DMi_DAlpha_[1]; };
    double pCMiz_pAlpha(void) { return DMi_DAlpha_[2]; };
    
    /** Partial of inviscid Mx,y,z wrt beta **/
    
    double pCMix_pBeta(void) { return DMi_DBeta_[0]; };
    double pCMiy_pBeta(void) { return DMi_DBeta_[1]; };
    double pCMiz_pBeta(void) { return DMi_DBeta_[2]; };
    
    /** Partial of inviscid Mx,y,z wrt p roll rate **/
    
    double pCMix_pP(void) { return DMi_DP_[0]; };
    double pCMiy_pP(void) { return DMi_DP_[1]; };
    double pCMiz_pP(void) { return DMi_DP_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt q pitch rate **/
    
    double pCMix_pQ(void) { return DMi_DQ_[0]; };
    double pCMiy_pQ(void) { return DMi_DQ_[1]; };
    double pCMiz_pQ(void) { return DMi_DQ_[2]; };
    
    /** Partial of inviscid Mx,y,z wrt r yaw rate **/
    
    double pCMix_pR(void) { return DMi_DR_[0]; };
    double pCMiy_pR(void) { return DMi_DR_[1]; };
    double pCMiz_pR(void) { return DMi_DR_[2]; };
    
    /** Partial of inviscid Mx,y,z wrt Mach **/
    
    double pCMix_pMach(void) { return DMi_DMach_[0]; };
    double pCMiy_pMach(void) { return DMi_DMach_[1]; };
    double pCMiz_pMach(void) { return DMi_DMach_[2]; };
    
    /** Partial of inviscid Mx,y,z wrt x location of CG **/
    
    double pCMix_pCGx(void) { return DMi_DCG_[0][0]; };
    double pCMiy_pCGx(void) { return DMi_DCG_[1][0]; };
    double pCMiz_pCGx(void) { return DMi_DCG_[2][0]; };

    /** Partial of inviscid Mx,y,z wrt y location of CG **/
    
    double pCMix_pCGy(void) { return DMi_DCG_[0][1]; };
    double pCMiy_pCGy(void) { return DMi_DCG_[1][1]; };
    double pCMiz_pCGy(void) { return DMi_DCG_[2][1]; };

    /** Partial of inviscid Mx,y,z wrt y location of CG **/
    
    double pCMix_pCGz(void) { return DMi_DCG_[0][2]; };
    double pCMiy_pCGz(void) { return DMi_DCG_[1][2]; };
    double pCMiz_pCGz(void) { return DMi_DCG_[2][2]; };

    /** Partial of inviscid Mx,y,z wrt Re **/
            
    double pCMix_pRe(void) { return DMi_DRe_[0]; };
    double pCMiy_pRe(void) { return DMi_DRe_[1]; };
    double pCMiz_pRe(void) { return DMi_DRe_[2]; };
    
// Viscous forces and moments gradients

    /** Partial of viscous Fx,y,z wrt alpha **/

    double pCFox_pAlpha(void) { return DFo_DAlpha_[0]; };
    double pCFoy_pAlpha(void) { return DFo_DAlpha_[1]; };
    double pCFoz_pAlpha(void) { return DFo_DAlpha_[2]; };
    
    /** Partial of viscous Fx,y,z wrt beta **/
    
    double pCFox_pBeta(void) { return DFo_DBeta_[0]; };
    double pCFoy_pBeta(void) { return DFo_DBeta_[1]; };
    double pCFoz_pBeta(void) { return DFo_DBeta_[2]; };
    
    /** Partial of viscous Fx,y,z wrt p roll rate **/
    
    double pCFox_pP(void) { return DFo_DP_[0]; };
    double pCFoy_pP(void) { return DFo_DP_[1]; };
    double pCFoz_pP(void) { return DFo_DP_[2]; };
    
    /** Partial of viscous Fx,y,z wrt q pitch rate **/
    
    double pCFox_pQ(void) { return DFo_DQ_[0]; };
    double pCFoy_pQ(void) { return DFo_DQ_[1]; };
    double pCFoz_pQ(void) { return DFo_DQ_[2]; };
    
    /** Partial of viscous Fx,y,z wrt r yaw rate **/
    
    double pCFox_pR(void) { return DFo_DR_[0]; };
    double pCFoy_pR(void) { return DFo_DR_[1]; };
    double pCFoz_pR(void) { return DFo_DR_[2]; };
    
    /** Partial of viscous Fx,y,z wrt Mach **/
    
    double pCFox_pMach(void) { return DFo_DMach_[0]; };
    double pCFoy_pMach(void) { return DFo_DMach_[1]; };
    double pCFoz_pMach(void) { return DFo_DMach_[2]; };
    
    /** Partial of viscous Fx,y,z wrt x location of CG **/
    
    double pCFox_pCGx(void) { return DFo_DCG_[0][0]; };
    double pCFoy_pCGx(void) { return DFo_DCG_[1][0]; };
    double pCFoz_pCGx(void) { return DFo_DCG_[2][0]; };

    /** Partial of viscous Fx,y,z wrt y location of CG **/
    
    double pCFox_pCGy(void) { return DFo_DCG_[0][1]; };
    double pCFoy_pCGy(void) { return DFo_DCG_[1][1]; };
    double pCFoz_pCGy(void) { return DFo_DCG_[2][1]; };

    /** Partial of viscous Fx,y,z wrt z location of CG **/
    
    double pCFox_pCGz(void) { return DFo_DCG_[0][2]; };
    double pCFoy_pCGz(void) { return DFo_DCG_[1][2]; };
    double pCFoz_pCGz(void) { return DFo_DCG_[2][2]; };
    
    /** Partial of viscous Fx,y,z wrt Re **/
            
    double pCFox_pRe(void) { return DFo_DRe_[0]; };
    double pCFoy_pRe(void) { return DFo_DRe_[1]; };
    double pCFoz_pRe(void) { return DFo_DRe_[2]; };
                
    /** Partial of viscous Mx,y,z wrt alpha **/
                 
    double pCMox_pAlpha(void) { return DMo_DAlpha_[0]; };
    double pCMoy_pAlpha(void) { return DMo_DAlpha_[1]; };
    double pCMoz_pAlpha(void) { return DMo_DAlpha_[2]; };
    
    /** Partial of viscous Mx,y,z wrt beta **/
    
    double pCMox_pBeta(void) { return DMo_DBeta_[0]; };
    double pCMoy_pBeta(void) { return DMo_DBeta_[1]; };
    double pCMoz_pBeta(void) { return DMo_DBeta_[2]; };
    
    /** Partial of viscous Mx,y,z wrt p roll rate **/
    
    double pCMox_pP(void) { return DMo_DP_[0]; };
    double pCMoy_pP(void) { return DMo_DP_[1]; };
    double pCMoz_pP(void) { return DMo_DP_[2]; };
    
    /** Partial of viscous Fx,y,z wrt q pitch rate **/
    
    double pCMox_pQ(void) { return DMo_DQ_[0]; };
    double pCMoy_pQ(void) { return DMo_DQ_[1]; };
    double pCMoz_pQ(void) { return DMo_DQ_[2]; };
    
    /** Partial of viscous Mx,y,z wrt r yaw rate **/
    
    double pCMox_pR(void) { return DMo_DR_[0]; };
    double pCMoy_pR(void) { return DMo_DR_[1]; };
    double pCMoz_pR(void) { return DMo_DR_[2]; };
    
    /** Partial of viscous Mx,y,z wrt Mach **/
    
    double pCMox_pMach(void) { return DMo_DMach_[0]; };
    double pCMoy_pMach(void) { return DMo_DMach_[1]; };
    double pCMoz_pMach(void) { return DMo_DMach_[2]; };
    
    /** Partial of viscous Mx,y,z wrt x location of CG **/
    
    double pCMox_pCGx(void) { return DMo_DCG_[0][0]; };
    double pCMoy_pCGx(void) { return DMo_DCG_[1][0]; };
    double pCMoz_pCGx(void) { return DMo_DCG_[2][0]; };

    /** Partial of viscous Mx,y,z wrt y location of CG **/
    
    double pCMox_pCGy(void) { return DMo_DCG_[0][1]; };
    double pCMoy_pCGy(void) { return DMo_DCG_[1][1]; };
    double pCMoz_pCGy(void) { return DMo_DCG_[2][1]; };

    /** Partial of viscous Mx,y,z wrt y location of CG **/
    
    double pCMox_pCGz(void) { return DMo_DCG_[0][2]; };
    double pCMoy_pCGz(void) { return DMo_DCG_[1][2]; };
    double pCMoz_pCGz(void) { return DMo_DCG_[2][2]; };

    /** Partial of viscous Mx,y,z wrt Re **/
            
    double pCMox_pRe(void) { return DMo_DRe_[0]; };
    double pCMoy_pRe(void) { return DMo_DRe_[1]; };
    double pCMoz_pRe(void) { return DMo_DRe_[2]; };
         
// Total forces using wake forces gradients

    /** Partial of total Fx,y,z wrt alpha using wake forces **/

    double pCFtwx_pAlpha(void) { return DFw_DAlpha_[0] + DFo_DAlpha_[0]; };
    double pCFtwy_pAlpha(void) { return DFw_DAlpha_[1] + DFo_DAlpha_[1]; };
    double pCFtwz_pAlpha(void) { return DFw_DAlpha_[2] + DFo_DAlpha_[2]; };
    
    /** Partial of total Fx,y,z wrt beta using wake forces **/
    
    double pCFtwx_pBeta(void) { return DFw_DBeta_[0] + DFo_DBeta_[0]; };
    double pCFtwy_pBeta(void) { return DFw_DBeta_[1] + DFo_DBeta_[1]; };
    double pCFtwz_pBeta(void) { return DFw_DBeta_[2] + DFo_DBeta_[2]; };
    
    /** Partial of total Fx,y,z wrt p roll rate using wake forces **/
    
    double pCFtwx_pP(void) { return DFw_DP_[0] + DFo_DP_[0]; };
    double pCFtwy_pP(void) { return DFw_DP_[1] + DFo_DP_[1]; };
    double pCFtwz_pP(void) { return DFw_DP_[2] + DFo_DP_[2]; };
    
    /** Partial of total Fx,y,z wrt q pitch rate using wake forces **/
    
    double pCFtwx_pQ(void) { return DFw_DQ_[0] + DFo_DQ_[0]; };
    double pCFtwy_pQ(void) { return DFw_DQ_[1] + DFo_DQ_[1]; };
    double pCFtwz_pQ(void) { return DFw_DQ_[2] + DFo_DQ_[2]; };
    
    /** Partial of total Fx,y,z wrt r yaw rate using wake forces **/
    
    double pCFtwx_pR(void) { return DFw_DR_[0] + DFo_DR_[0]; };
    double pCFtwy_pR(void) { return DFw_DR_[1] + DFo_DR_[1]; };
    double pCFtwz_pR(void) { return DFw_DR_[2] + DFo_DR_[2]; };
    
    /** Partial of total Fx,y,z wrt Mach using wake forces **/
    
    double pCFtwx_pMach(void) { return DFw_DMach_[0] + DFo_DMach_[0]; };
    double pCFtwy_pMach(void) { return DFw_DMach_[1] + DFo_DMach_[1]; };
    double pCFtwz_pMach(void) { return DFw_DMach_[2] + DFo_DMach_[2]; };
    
    /** Partial of total Fx,y,z wrt x location of CG using wake forces **/
    
    double pCFtwx_pCGx(void) { return DFw_DCG_[0][0] + DFo_DCG_[0][0]; };
    double pCFtwy_pCGx(void) { return DFw_DCG_[1][0] + DFo_DCG_[1][0]; };
    double pCFtwz_pCGx(void) { return DFw_DCG_[2][0] + DFo_DCG_[2][0]; };

    /** Partial of total Fx,y,z wrt y location of CG using wake forces **/
    
    double pCFtwx_pCGy(void) { return DFw_DCG_[0][1] + DFo_DCG_[0][1]; };
    double pCFtwy_pCGy(void) { return DFw_DCG_[1][1] + DFo_DCG_[1][1]; };
    double pCFtwz_pCGy(void) { return DFw_DCG_[2][1] + DFo_DCG_[2][1]; };

    /** Partial of total or total Fx,y,z wrt z location of CG using wake forces **/
    
    double pCFtwx_pCGz(void) { return DFw_DCG_[0][2] + DFo_DCG_[0][2]; };
    double pCFtwy_pCGz(void) { return DFw_DCG_[1][2] + DFo_DCG_[1][2]; };
    double pCFtwz_pCGz(void) { return DFw_DCG_[2][2] + DFo_DCG_[2][2]; };
    
    /** Partial of total Fx,y,z wrt Re using wake forces **/
            
    double pCFtwx_pRe(void) { return DFw_DRe_[0] + DFo_DRe_[0]; };
    double pCFtwy_pRe(void) { return DFw_DRe_[1] + DFo_DRe_[1]; };
    double pCFtwz_pRe(void) { return DFw_DRe_[2] + DFo_DRe_[2]; };        
         
// Inviscid forces using wake forces gradients

    /** Partial of total Fx,y,z wrt alpha using wake forces **/

    double pCFiwx_pAlpha(void) { return DFw_DAlpha_[0]; };
    double pCFiwy_pAlpha(void) { return DFw_DAlpha_[1]; };
    double pCFiwz_pAlpha(void) { return DFw_DAlpha_[2]; };
    
    /** Partial of total Fx,y,z wrt beta using wake forces **/
    
    double pCFiwx_pBeta(void) { return DFw_DBeta_[0]; };
    double pCFiwy_pBeta(void) { return DFw_DBeta_[1]; };
    double pCFiwz_pBeta(void) { return DFw_DBeta_[2]; };
    
    /** Partial of total Fx,y,z wrt p roll rate using wake forces **/
    
    double pCFiwx_pP(void) { return DFw_DP_[0]; };
    double pCFiwy_pP(void) { return DFw_DP_[1]; };
    double pCFiwz_pP(void) { return DFw_DP_[2]; };
    
    /** Partial of total Fx,y,z wrt q pitch rate using wake forces **/
    
    double pCFiwx_pQ(void) { return DFw_DQ_[0]; };
    double pCFiwy_pQ(void) { return DFw_DQ_[1]; };
    double pCFiwz_pQ(void) { return DFw_DQ_[2]; };
    
    /** Partial of total Fx,y,z wrt r yaw rate using wake forces **/
    
    double pCFiwx_pR(void) { return DFw_DR_[0]; };
    double pCFiwy_pR(void) { return DFw_DR_[1]; };
    double pCFiwz_pR(void) { return DFw_DR_[2]; };
    
    /** Partial of total Fx,y,z wrt Mach using wake forces **/
    
    double pCFiwx_pMach(void) { return DFw_DMach_[0]; };
    double pCFiwy_pMach(void) { return DFw_DMach_[1]; };
    double pCFiwz_pMach(void) { return DFw_DMach_[2]; };
    
    /** Partial of total Fx,y,z wrt x location of CG using wake forces **/
    
    double pCFiwx_pCGx(void) { return DFw_DCG_[0][0]; };
    double pCFiwy_pCGx(void) { return DFw_DCG_[1][0]; };
    double pCFiwz_pCGx(void) { return DFw_DCG_[2][0]; };

    /** Partial of total Fx,y,z wrt y location of CG using wake forces **/
    
    double pCFiwx_pCGy(void) { return DFw_DCG_[0][1]; };
    double pCFiwy_pCGy(void) { return DFw_DCG_[1][1]; };
    double pCFiwz_pCGy(void) { return DFw_DCG_[2][1]; };

    /** Partial of total or total Fx,y,z wrt z location of CG **/
    
    double pCFiwx_pCGz(void) { return DFw_DCG_[0][2]; };
    double pCFiwy_pCGz(void) { return DFw_DCG_[1][2]; };
    double pCFiwz_pCGz(void) { return DFw_DCG_[2][2]; };
    
    /** Partial of total Fx,y,z wrt Re using wake forces **/
            
    double pCFiwx_pRe(void) { return DFw_DRe_[0]; };
    double pCFiwy_pRe(void) { return DFw_DRe_[1]; };
    double pCFiwz_pRe(void) { return DFw_DRe_[2]; };                
        
// Aero total forces and moments

    /** Partial of total CL, CD, CS wrt alpha **/

    double pCLt_pAlpha(void) { return DAeroFi_DAlpha_[0] + DAeroFo_DAlpha_[0]; };
    double pCDt_pAlpha(void) { return DAeroFi_DAlpha_[1] + DAeroFo_DAlpha_[1]; };
    double pCSt_pAlpha(void) { return DAeroFi_DAlpha_[2] + DAeroFo_DAlpha_[2]; };
    
    /** Partial of total CL, CD, CS wrt beta **/
    
    double pCLt_pBeta(void) { return DAeroFi_DBeta_[0] + DAeroFo_DBeta_[0]; };
    double pCDt_pBeta(void) { return DAeroFi_DBeta_[1] + DAeroFo_DBeta_[1]; };
    double pCSt_pBeta(void) { return DAeroFi_DBeta_[2] + DAeroFo_DBeta_[2]; };
    
    /** Partial of total CL, CD, CS wrt p roll rate **/
    
    double pCLt_pP(void) { return DAeroFi_DP_[0] + DAeroFo_DP_[0]; };
    double pCDt_pP(void) { return DAeroFi_DP_[1] + DAeroFo_DP_[1]; };
    double pCSt_pP(void) { return DAeroFi_DP_[2] + DAeroFo_DP_[2]; };
    
    /** Partial of total CL, CD, CSwrt q pitch rate **/
    
    double pCLt_pQ(void) { return DAeroFi_DQ_[0] + DAeroFo_DQ_[0]; };
    double pCDt_pQ(void) { return DAeroFi_DQ_[1] + DAeroFo_DQ_[1]; };
    double pCSt_pQ(void) { return DAeroFi_DQ_[2] + DAeroFo_DQ_[2]; };
    
    /** Partial of total CL, CD, CS wrt r yaw rate **/
    
    double pCLt_pR(void) { return DAeroFi_DR_[0] + DAeroFo_DR_[0]; };
    double pCDt_pR(void) { return DAeroFi_DR_[1] + DAeroFo_DR_[1]; };
    double pCSt_pR(void) { return DAeroFi_DR_[2] + DAeroFo_DR_[2]; };
    
    /** Partial of total CL, CD, CS wrt Mach **/
    
    double pCLt_pMach(void) { return DAeroFi_DMach_[0] + DAeroFo_DMach_[0]; };
    double pCDt_pMach(void) { return DAeroFi_DMach_[1] + DAeroFo_DMach_[1]; };
    double pCSt_pMach(void) { return DAeroFi_DMach_[2] + DAeroFo_DMach_[2]; };
    
    /** Partial of total CL, CD, CSwrt x location of CG **/
    
    double pCLt_pCGx(void) { return DAeroFi_DCG_[0][0] + DAeroFo_DCG_[0][0]; };
    double pCDt_pCGx(void) { return DAeroFi_DCG_[1][1] + DAeroFo_DCG_[1][0]; };
    double pCSt_pCGx(void) { return DAeroFi_DCG_[2][2] + DAeroFo_DCG_[2][0]; };
                      
    /** Partial of total CL, CD, CS wrt y location of CG **/
    
    double pCLt_pCGy(void) { return DAeroFi_DCG_[0][1] + DAeroFo_DCG_[0][1]; };
    double pCDt_pCGy(void) { return DAeroFi_DCG_[1][1] + DAeroFo_DCG_[1][1]; };
    double pCSt_pCGy(void) { return DAeroFi_DCG_[2][1] + DAeroFo_DCG_[2][1]; };

    /** Partial of total CL, CD, CS wrt z location of CG **/
    
    double pCLt_pCGz(void) { return DAeroFi_DCG_[0][2] + DAeroFo_DCG_[0][2]; };
    double pCDt_pCGz(void) { return DAeroFi_DCG_[1][2] + DAeroFo_DCG_[1][2]; };
    double pCSt_pCGz(void) { return DAeroFi_DCG_[2][2] + DAeroFo_DCG_[2][2]; };
    
    /** Partial of total CL, CD, CS wrt Re **/
            
    double pCLt_pRe(void) { return DAeroFi_DRe_[0] + DAeroFo_DRe_[0]; };
    double pCDt_pRe(void) { return DAeroFi_DRe_[1] + DAeroFo_DRe_[1]; };
    double pCSt_pRe(void) { return DAeroFi_DRe_[2] + DAeroFo_DRe_[2]; };
                
    /** Partial of total CMl, Cmm, Cmn wrt alpha **/
                 
    double pCMlt_pAlpha(void) { return DAeroMi_DAlpha_[0] + DAeroMo_DAlpha_[0]; };
    double pCMmt_pAlpha(void) { return DAeroMi_DAlpha_[1] + DAeroMo_DAlpha_[1]; };
    double pCMnt_pAlpha(void) { return DAeroMi_DAlpha_[2] + DAeroMo_DAlpha_[2]; };
    
    /** Partial of total CMl, Cmm, Cmn wrt beta **/
    
    double pCMlt_pBeta(void) { return DAeroMi_DBeta_[0] + DAeroMo_DBeta_[0]; };
    double pCMmt_pBeta(void) { return DAeroMi_DBeta_[1] + DAeroMo_DBeta_[1]; };
    double pCMnt_pBeta(void) { return DAeroMi_DBeta_[2] + DAeroMo_DBeta_[2]; };
    
    /** Partial of total CMl, Cmm, Cmn wrt p roll rate **/
    
    double pCMlt_pP(void) { return DAeroMi_DP_[0] + DAeroMo_DP_[0]; };
    double pCMmt_pP(void) { return DAeroMi_DP_[1] + DAeroMo_DP_[1]; };
    double pCMnt_pP(void) { return DAeroMi_DP_[2] + DAeroMo_DP_[2]; };
    
    /** Partial of total CMl, Cmm, Cmn wrt q pitch rate **/
    
    double pCMlt_pQ(void) { return DAeroMi_DQ_[0] + DAeroMo_DQ_[0]; };
    double pCMmt_pQ(void) { return DAeroMi_DQ_[1] + DAeroMo_DQ_[1]; };
    double pCMnt_pQ(void) { return DAeroMi_DQ_[2] + DAeroMo_DQ_[2]; };
    
    /** Partial of total CMl, Cmm, Cmn wrt r yaw rate **/
    
    double pCMlt_pR(void) { return DAeroMi_DR_[0] + DAeroMo_DR_[0]; };
    double pCMmt_pR(void) { return DAeroMi_DR_[1] + DAeroMo_DR_[1]; };
    double pCMnt_pR(void) { return DAeroMi_DR_[2] + DAeroMo_DR_[2]; };
    
    /** Partial of total CMl, Cmm, Cmnwrt Mach **/
    
    double pCMlt_pMach(void) { return DAeroMi_DMach_[0] + DAeroMo_DMach_[0]; };
    double pCMmt_pMach(void) { return DAeroMi_DMach_[1] + DAeroMo_DMach_[1]; };
    double pCMnt_pMach(void) { return DAeroMi_DMach_[2] + DAeroMo_DMach_[2]; };
    
    /** Partial of total CMl, Cmm, Cmn wrt x location of CG **/
    
    double pCMlt_pCGx(void) { return DAeroMi_DCG_[0][0] + DAeroMo_DCG_[0][0]; };
    double pCMmt_pCGx(void) { return DAeroMi_DCG_[1][0] + DAeroMo_DCG_[1][0]; };
    double pCMnt_pCGx(void) { return DAeroMi_DCG_[2][0] + DAeroMo_DCG_[2][0]; };

    /** Partial of total CMl, Cmm, Cmn wrt y location of CG **/
    
    double pCMlt_pCGy(void) { return DAeroMi_DCG_[0][1] + DAeroMo_DCG_[0][1]; };
    double pCMmt_pCGy(void) { return DAeroMi_DCG_[1][1] + DAeroMo_DCG_[1][1]; };
    double pCMnt_pCGy(void) { return DAeroMi_DCG_[2][1] + DAeroMo_DCG_[2][1]; };

    /** Partial of total CMl, Cmm, Cmn wrt y location of CG **/
    
    double pCMlt_pCGz(void) { return DAeroMi_DCG_[0][2] + DAeroMo_DCG_[0][2]; };
    double pCMmt_pCGz(void) { return DAeroMi_DCG_[1][2] + DAeroMo_DCG_[1][2]; };
    double pCMnt_pCGz(void) { return DAeroMi_DCG_[2][2] + DAeroMo_DCG_[2][2]; };

    /** Partial of total CMl, Cmm, Cmn wrt Re **/
            
    double pCMlt_pRe(void) { return DAeroMi_DRe_[0] + DAeroMo_DRe_[0]; };
    double pCMmt_pRe(void) { return DAeroMi_DRe_[1] + DAeroMo_DRe_[1]; };
    double pCMnt_pRe(void) { return DAeroMi_DRe_[2] + DAeroMo_DRe_[2]; };
    
// Aero Inviscid forces and moments gradients

    /** Partial of inviscid Fx,y,z wrt alpha **/

    double pCLi_pAlpha(void) { return DAeroFi_DAlpha_[0]; };
    double pCDi_pAlpha(void) { return DAeroFi_DAlpha_[1]; };
    double pCSi_pAlpha(void) { return DAeroFi_DAlpha_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt beta **/
    
    double pCLi_pBeta(void) { return DAeroFi_DBeta_[0]; };
    double pCDi_pBeta(void) { return DAeroFi_DBeta_[1]; };
    double pCSi_pBeta(void) { return DAeroFi_DBeta_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt p roll rate **/
    
    double pCLi_pP(void) { return DAeroFi_DP_[0]; };
    double pCDi_pP(void) { return DAeroFi_DP_[1]; };
    double pCSi_pP(void) { return DAeroFi_DP_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt q pitch rate **/
    
    double pCLi_pQ(void) { return DAeroFi_DQ_[0]; };
    double pCDi_pQ(void) { return DAeroFi_DQ_[1]; };
    double pCSi_pQ(void) { return DAeroFi_DQ_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt r yaw rate **/
    
    double pCLi_pR(void) { return DAeroFi_DR_[0]; };
    double pCDi_pR(void) { return DAeroFi_DR_[1]; };
    double pCSi_pR(void) { return DAeroFi_DR_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt Mach **/
    
    double pCLi_pMach(void) { return DAeroFi_DMach_[0]; };
    double pCDi_pMach(void) { return DAeroFi_DMach_[1]; };
    double pCSi_pMach(void) { return DAeroFi_DMach_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt x location of CG **/
    
    double pCLi_pCGx(void) { return DAeroFi_DCG_[0][0]; };
    double pCDi_pCGx(void) { return DAeroFi_DCG_[1][0]; };
    double pCSi_pCGx(void) { return DAeroFi_DCG_[2][0]; };

    /** Partial of inviscid Fx,y,z wrt y location of CG **/
    
    double pCLi_pCGy(void) { return DAeroFi_DCG_[0][1]; };
    double pCDi_pCGy(void) { return DAeroFi_DCG_[1][1]; };
    double pCSi_pCGy(void) { return DAeroFi_DCG_[2][1]; };

    /** Partial of inviscid Fx,y,z wrt z location of CG **/
    
    double pCLi_pCGz(void) { return DAeroFi_DCG_[0][2]; };
    double pCDi_pCGz(void) { return DAeroFi_DCG_[1][2]; };
    double pCSi_pCGz(void) { return DAeroFi_DCG_[2][2]; };
    
    /** Partial of inviscid Fx,y,z wrt Re **/
            
    double pCLi_pRe(void) { return DAeroFi_DRe_[0]; };
    double pCDi_pRe(void) { return DAeroFi_DRe_[1]; };
    double pCSi_pRe(void) { return DAeroFi_DRe_[2]; };
                
    /** Partial of inviscid Mx,y,z wrt alpha **/
                 
    double pCMli_pAlpha(void) { return DAeroMi_DAlpha_[0]; };
    double pCMmi_pAlpha(void) { return DAeroMi_DAlpha_[1]; };
    double pCMni_pAlpha(void) { return DAeroMi_DAlpha_[2]; };
    
    /** Partial of inviscid Mx,y,z wrt beta **/
    
    double pCMli_pBeta(void) { return DAeroMi_DBeta_[0]; };
    double pCMmi_pBeta(void) { return DAeroMi_DBeta_[1]; };
    double pCMni_pBeta(void) { return DAeroMi_DBeta_[2]; };
    
    /** Partial of inviscid Mx,y,z wrt p roll rate **/
    
    double pCMli_pP(void) { return DAeroMi_DP_[0]; };
    double pCMmi_pP(void) { return DAeroMi_DP_[1]; };
    double pCMni_pP(void) { return DAeroMi_DP_[2]; };
    
    /** Partial of inviscid Fx,y,z wrt q pitch rate **/
    
    double pCMli_pQ(void) { return DAeroMi_DQ_[0]; };
    double pCMmi_pQ(void) { return DAeroMi_DQ_[1]; };
    double pCMni_pQ(void) { return DAeroMi_DQ_[2]; };
    
    /** Partial of inviscid Mx,y,z wrt r yaw rate **/
    
    double pCMli_pR(void) { return DAeroMi_DR_[0]; };
    double pCMmi_pR(void) { return DAeroMi_DR_[1]; };
    double pCMni_pR(void) { return DAeroMi_DR_[2]; };
    
    /** Partial of inviscid Mx,y,z wrt Mach **/
    
    double pCMli_pMach(void) { return DAeroMi_DMach_[0]; };
    double pCMmi_pMach(void) { return DAeroMi_DMach_[1]; };
    double pCMni_pMach(void) { return DAeroMi_DMach_[2]; };
    
    /** Partial of inviscid Mx,y,z wrt x location of CG **/
    
    double pCMli_pCGx(void) { return DAeroMi_DCG_[0][0]; };
    double pCMmi_pCGx(void) { return DAeroMi_DCG_[1][0]; };
    double pCMni_pCGx(void) { return DAeroMi_DCG_[2][0]; };

    /** Partial of inviscid Mx,y,z wrt y location of CG **/
    
    double pCMli_pCGy(void) { return DAeroMi_DCG_[0][1]; };
    double pCMmi_pCGy(void) { return DAeroMi_DCG_[1][1]; };
    double pCMni_pCGy(void) { return DAeroMi_DCG_[2][1]; };

    /** Partial of inviscid Mx,y,z wrt y location of CG **/
    
    double pCMli_pCGz(void) { return DAeroMi_DCG_[0][2]; };
    double pCMmi_pCGz(void) { return DAeroMi_DCG_[1][2]; };
    double pCMni_pCGz(void) { return DAeroMi_DCG_[2][2]; };

    /** Partial of inviscid Mx,y,z wrt Re **/
            
    double pCMli_pRe(void) { return DAeroMi_DRe_[0]; };
    double pCMmi_pRe(void) { return DAeroMi_DRe_[1]; };
    double pCMni_pRe(void) { return DAeroMi_DRe_[2]; };

// Aero Viscous forces and moments gradients

    /** Partial of viscous Fx,y,z wrt alpha **/

    double pCLo_pAlpha(void) { return DAeroFo_DAlpha_[0]; };
    double pCDo_pAlpha(void) { return DAeroFo_DAlpha_[1]; };
    double pCSo_pAlpha(void) { return DAeroFo_DAlpha_[2]; };
    
    /** Partial of viscous Fx,y,z wrt beta **/
    
    double pCLo_pBeta(void) { return DAeroFo_DBeta_[0]; };
    double pCDo_pBeta(void) { return DAeroFo_DBeta_[1]; };
    double pCSo_pBeta(void) { return DAeroFo_DBeta_[2]; };
    
    /** Partial of viscous Fx,y,z wrt p roll rate **/
    
    double pCLo_pP(void) { return DAeroFo_DP_[0]; };
    double pCDo_pP(void) { return DAeroFo_DP_[1]; };
    double pCSo_pP(void) { return DAeroFo_DP_[2]; };
    
    /** Partial of viscous Fx,y,z wrt q pitch rate **/
    
    double pCLo_pQ(void) { return DAeroFo_DQ_[0]; };
    double pCDo_pQ(void) { return DAeroFo_DQ_[1]; };
    double pCSo_pQ(void) { return DAeroFo_DQ_[2]; };
    
    /** Partial of viscous Fx,y,z wrt r yaw rate **/
    
    double pCLo_pR(void) { return DAeroFo_DR_[0]; };
    double pCDo_pR(void) { return DAeroFo_DR_[1]; };
    double pCSo_pR(void) { return DAeroFo_DR_[2]; };
    
    /** Partial of viscous Fx,y,z wrt Mach **/
    
    double pCLo_pMach(void) { return DAeroFo_DMach_[0]; };
    double pCDo_pMach(void) { return DAeroFo_DMach_[1]; };
    double pCSo_pMach(void) { return DAeroFo_DMach_[2]; };
    
    /** Partial of viscous Fx,y,z wrt x location of CG **/
    
    double pCLo_pCGx(void) { return DAeroFo_DCG_[0][0]; };
    double pCDo_pCGx(void) { return DAeroFo_DCG_[1][0]; };
    double pCSo_pCGx(void) { return DAeroFo_DCG_[2][0]; };

    /** Partial of viscous Fx,y,z wrt y location of CG **/
    
    double pCLo_pCGy(void) { return DAeroFo_DCG_[0][1]; };
    double pCDo_pCGy(void) { return DAeroFo_DCG_[1][1]; };
    double pCSo_pCGy(void) { return DAeroFo_DCG_[2][1]; };

    /** Partial of viscous Fx,y,z wrt z location of CG **/
    
    double pCLo_pCGz(void) { return DAeroFo_DCG_[0][2]; };
    double pCDo_pCGz(void) { return DAeroFo_DCG_[1][2]; };
    double pCSo_pCGz(void) { return DAeroFo_DCG_[2][2]; };
    
    /** Partial of viscous Fx,y,z wrt Re **/
            
    double pCLo_pRe(void) { return DAeroFo_DRe_[0]; };
    double pCDo_pRe(void) { return DAeroFo_DRe_[1]; };
    double pCSo_pRe(void) { return DAeroFo_DRe_[2]; };
                
    /** Partial of viscous Mx,y,z wrt alpha **/
                 
    double pCMlo_pAlpha(void) { return DAeroMo_DAlpha_[0]; };
    double pCMmo_pAlpha(void) { return DAeroMo_DAlpha_[1]; };
    double pCMno_pAlpha(void) { return DAeroMo_DAlpha_[2]; };
    
    /** Partial of viscous Mx,y,z wrt beta **/
    
    double pCMlo_pBeta(void) { return DAeroMo_DBeta_[0]; };
    double pCMmo_pBeta(void) { return DAeroMo_DBeta_[1]; };
    double pCMno_pBeta(void) { return DAeroMo_DBeta_[2]; };
    
    /** Partial of viscous Mx,y,z wrt p roll rate **/
    
    double pCMlo_pP(void) { return DAeroMo_DP_[0]; };
    double pCMmo_pP(void) { return DAeroMo_DP_[1]; };
    double pCMno_pP(void) { return DAeroMo_DP_[2]; };
    
    /** Partial of viscous Fx,y,z wrt q pitch rate **/
    
    double pCMlo_pQ(void) { return DAeroMo_DQ_[0]; };
    double pCMmo_pQ(void) { return DAeroMo_DQ_[1]; };
    double pCMno_pQ(void) { return DAeroMo_DQ_[2]; };
    
    /** Partial of viscous Mx,y,z wrt r yaw rate **/
    
    double pCMlo_pR(void) { return DAeroMo_DR_[0]; };
    double pCMmo_pR(void) { return DAeroMo_DR_[1]; };
    double pCMno_pR(void) { return DAeroMo_DR_[2]; };
    
    /** Partial of viscous Mx,y,z wrt Mach **/
    
    double pCMlo_pMach(void) { return DAeroMo_DMach_[0]; };
    double pCMmo_pMach(void) { return DAeroMo_DMach_[1]; };
    double pCMno_pMach(void) { return DAeroMo_DMach_[2]; };
    
    /** Partial of viscous Mx,y,z wrt x location of CG **/
    
    double pCMlo_pCGx(void) { return DAeroMo_DCG_[0][0]; };
    double pCMmo_pCGx(void) { return DAeroMo_DCG_[1][0]; };
    double pCMno_pCGx(void) { return DAeroMo_DCG_[2][0]; };

    /** Partial of viscous Mx,y,z wrt y location of CG **/
    
    double pCMlo_pCGy(void) { return DAeroMo_DCG_[0][1]; };
    double pCMmo_pCGy(void) { return DAeroMo_DCG_[1][1]; };
    double pCMno_pCGy(void) { return DAeroMo_DCG_[2][1]; };

    /** Partial of viscous Mx,y,z wrt y location of CG **/
    
    double pCMlo_pCGz(void) { return DAeroMo_DCG_[0][2]; };
    double pCMmo_pCGz(void) { return DAeroMo_DCG_[1][2]; };
    double pCMno_pCGz(void) { return DAeroMo_DCG_[2][2]; };

    /** Partial of viscous Mx,y,z wrt Re **/
            
    double pCMlo_pRe(void) { return DAeroMo_DRe_[0]; };
    double pCMmo_pRe(void) { return DAeroMo_DRe_[1]; };
    double pCMno_pRe(void) { return DAeroMo_DRe_[2]; };

// Aero total forces using wake forces

    /** Partial of total CL, CD, CS wrt alpha using wake forces **/

    double pCLtw_pAlpha(void) { return DAeroFw_DAlpha_[0] + DAeroFo_DAlpha_[0]; };
    double pCDtw_pAlpha(void) { return DAeroFw_DAlpha_[1] + DAeroFo_DAlpha_[1]; };
    double pCStw_pAlpha(void) { return DAeroFw_DAlpha_[2] + DAeroFo_DAlpha_[2]; };
    
    /** Partial of total CL, CD, CS wrt beta using wake forces **/
    
    double pCLtw_pBeta(void) { return DAeroFw_DBeta_[0] + DAeroFo_DBeta_[0]; };
    double pCDtw_pBeta(void) { return DAeroFw_DBeta_[1] + DAeroFo_DBeta_[1]; };
    double pCStw_pBeta(void) { return DAeroFw_DBeta_[2] + DAeroFo_DBeta_[2]; };
    
    /** Partial of total CL, CD, CS wrt p roll rate using wake forces **/
    
    double pCLtw_pP(void) { return DAeroFw_DP_[0] + DAeroFo_DP_[0]; };
    double pCDtw_pP(void) { return DAeroFw_DP_[1] + DAeroFo_DP_[1]; };
    double pCStw_pP(void) { return DAeroFw_DP_[2] + DAeroFo_DP_[2]; };
    
    /** Partial of total CL, CD, CSwrt q pitch rate using wake forces **/
    
    double pCLtw_pQ(void) { return DAeroFw_DQ_[0] + DAeroFo_DQ_[0]; };
    double pCDtw_pQ(void) { return DAeroFw_DQ_[1] + DAeroFo_DQ_[1]; };
    double pCStw_pQ(void) { return DAeroFw_DQ_[2] + DAeroFo_DQ_[2]; };
    
    /** Partial of total CL, CD, CS wrt r yaw rate using wake forces **/
    
    double pCLtw_pR(void) { return DAeroFw_DR_[0] + DAeroFo_DR_[0]; };
    double pCDtw_pR(void) { return DAeroFw_DR_[1] + DAeroFo_DR_[1]; };
    double pCStw_pR(void) { return DAeroFw_DR_[2] + DAeroFo_DR_[2]; };
    
    /** Partial of total CL, CD, CS wrt Mach using wake forces **/
    
    double pCLtw_pMach(void) { return DAeroFw_DMach_[0] + DAeroFo_DMach_[0]; };
    double pCDtw_pMach(void) { return DAeroFw_DMach_[1] + DAeroFo_DMach_[1]; };
    double pCStw_pMach(void) { return DAeroFw_DMach_[2] + DAeroFo_DMach_[2]; };
    
    /** Partial of total CL, CD, CSwrt x location of CG using wake forces **/
    
    double pCLtw_pCGx(void) { return DAeroFw_DCG_[0][0] + DAeroFo_DCG_[0][0]; };
    double pCDtw_pCGx(void) { return DAeroFw_DCG_[1][1] + DAeroFo_DCG_[1][0]; };
    double pCStw_pCGx(void) { return DAeroFw_DCG_[2][2] + DAeroFo_DCG_[2][0]; };
                      
    /** Partial of total CL, CD, CS wrt y location of CG using wake forces **/
    
    double pCLtw_pCGy(void) { return DAeroFw_DCG_[0][1] + DAeroFo_DCG_[0][1]; };
    double pCDtw_pCGy(void) { return DAeroFw_DCG_[1][1] + DAeroFo_DCG_[1][1]; };
    double pCStw_pCGy(void) { return DAeroFw_DCG_[2][1] + DAeroFo_DCG_[2][1]; };

    /** Partial of total CL, CD, CS wrt z location of CG using wake forces **/
    
    double pCLtw_pCGz(void) { return DAeroFw_DCG_[0][2] + DAeroFo_DCG_[0][2]; };
    double pCDtw_pCGz(void) { return DAeroFw_DCG_[1][2] + DAeroFo_DCG_[1][2]; };
    double pCStw_pCGz(void) { return DAeroFw_DCG_[2][2] + DAeroFo_DCG_[2][2]; };
    
    /** Partial of total CL, CD, CS wrt Re using wake forces **/
            
    double pCLtw_pRe(void) { return DAeroFw_DRe_[0] + DAeroFo_DRe_[0]; };
    double pCDtw_pRe(void) { return DAeroFw_DRe_[1] + DAeroFo_DRe_[1]; };
    double pCStw_pRe(void) { return DAeroFw_DRe_[2] + DAeroFo_DRe_[2]; };    
    
// Inviscid total forces using wake forces

    /** Partial of total CL, CD, CS wrt alpha using wake forces **/

    double pCLiw_pAlpha(void) { return DAeroFw_DAlpha_[0]; };
    double pCDiw_pAlpha(void) { return DAeroFw_DAlpha_[1]; };
    double pCSiw_pAlpha(void) { return DAeroFw_DAlpha_[2]; };
    
    /** Partial of total CL, CD, CS wrt beta using wake forces **/
    
    double pCLiw_pBeta(void) { return DAeroFw_DBeta_[0]; };
    double pCDiw_pBeta(void) { return DAeroFw_DBeta_[1]; };
    double pCSiw_pBeta(void) { return DAeroFw_DBeta_[2]; };
    
    /** Partial of total CL, CD, CS wrt p roll rate using wake forces **/
    
    double pCLiw_pP(void) { return DAeroFw_DP_[0]; };
    double pCDiw_pP(void) { return DAeroFw_DP_[1]; };
    double pCSiw_pP(void) { return DAeroFw_DP_[2]; };
    
    /** Partial of total CL, CD, CSwrt q pitch rate using wake forces **/
    
    double pCLiw_pQ(void) { return DAeroFw_DQ_[0]; };
    double pCDiw_pQ(void) { return DAeroFw_DQ_[1]; };
    double pCSiw_pQ(void) { return DAeroFw_DQ_[2]; };
    
    /** Partial of total CL, CD, CS wrt r yaw rate using wake forces **/
    
    double pCLiw_pR(void) { return DAeroFw_DR_[0]; };
    double pCDiw_pR(void) { return DAeroFw_DR_[1]; };
    double pCSiw_pR(void) { return DAeroFw_DR_[2]; };
    
    /** Partial of total CL, CD, CS wrt Mach using wake forces **/
    
    double pCLiw_pMach(void) { return DAeroFw_DMach_[0]; };
    double pCDiw_pMach(void) { return DAeroFw_DMach_[1]; };
    double pCSiw_pMach(void) { return DAeroFw_DMach_[2]; };
    
    /** Partial of total CL, CD, CSwrt x location of CG using wake forces **/
    
    double pCLiw_pCGx(void) { return DAeroFw_DCG_[0][0]; };
    double pCDiw_pCGx(void) { return DAeroFw_DCG_[1][1]; };
    double pCSiw_pCGx(void) { return DAeroFw_DCG_[2][2]; };
                      
    /** Partial of total CL, CD, CS wrt y location of CG using wake forces **/
    
    double pCLiw_pCGy(void) { return DAeroFw_DCG_[0][1]; };
    double pCDiw_pCGy(void) { return DAeroFw_DCG_[1][1]; };
    double pCSiw_pCGy(void) { return DAeroFw_DCG_[2][1]; };

    /** Partial of total CL, CD, CS wrt z location of CG using wake forces **/
    
    double pCLiw_pCGz(void) { return DAeroFw_DCG_[0][2]; };
    double pCDiw_pCGz(void) { return DAeroFw_DCG_[1][2]; };
    double pCSiw_pCGz(void) { return DAeroFw_DCG_[2][2]; };
    
    /** Partial of total CL, CD, CS wrt Re using wake forces **/
            
    double pCLiw_pRe(void) { return DAeroFw_DRe_[0]; };
    double pCDiw_pRe(void) { return DAeroFw_DRe_[1]; };
    double pCSiw_pRe(void) { return DAeroFw_DRe_[2]; };        
    
    /** Zero angle of attack lift coefficient for 2D viscous drag estimate... **/

    double &Clo2D(void) { return Clo_2d_; };
    
    /** Set maximum 2D Clmax **/
    
    double &Clmax_2d(void) { return Clmax_2d_; };
    
    /** Minimum value of stall factor... a value < 1 means some wing section has stalled **/
    
    double MinStallFactor(void) { return MinStallFactor_; };

    /** Read in the VSP geometry file **/
    
    void ReadFile(char *FileName) { snprintf(FileName_,sizeof(FileName_)*sizeof(char),"%s",FileName);  VSPGeom_.DoSymmetryPlaneSolve() = DoSymmetryPlaneSolve_ ; VSPGeom_.ReadFile(FileName); };

    /** Turn on ground effects analysis **/
    
    int &DoGroundEffectsAnalysis(void) { return VSPGeom_.DoGroundEffectsAnalysis(); };
    
    /** Set the vehicle rotation vector (r1,r2,r3 **/
    
    double &VehicleRotationAngleVector(int i) { return VSPGeom_.VehicleRotationAngleVector(i); };
    
    /** Set the point about which to rotate the vehicle **/
    
    double &VehicleRotationAxisLocation(int i) { return VSPGeom_.VehicleRotationAxisLocation(i); };
    
    /** Set the height of the vehicle above the ground ...be careful you don't rotate a vehicle into the ground plane! **/
    
    double &HeightAboveGround(void) { return VSPGeom_.HeightAboveGround(); };
    
    /** Access to VSP geometry object **/
    
    VSP_GEOM &VSPGeom(void) { return VSPGeom_; };

    /** Set up the problem... **/
    
    void Setup(void);
    
    /** Solve on the current geometry, flow condition(s) **/
 
    void Solve(void) { Solve(0); };
    
    /** Solve case, Case **/
    
    void Solve(int Case);
    
    /** Update the mesh geometry data structures **/
    
    void UpdateMeshes(void);
   
    /** Recalcalculate the forces... something has been changed, usually the Re # **/
    
    void ReCalculateForces(void) { CalculateForces(); };
    
//
//    /** Do a matrix vector product using a user supplied vector **/
//
//    void CalculateForwardMatrixVectorProduct(double *VecIn, double *VecOut);
//
//    /** Calculate the right hand side of the forward equation **/
//
//    void CalculateForwardRightHandSide(double *RHS);
//
//    /** Calculate residual given user supplied vector of solution vector, gamma **/
//
//    void CalculateForwardResidual(double *VecIn, double *VecOut);
//
//    /** Get aerodynamic forces at nodes **/
//
//    void GetNodalForces(double *VecIn);
//
//    /** Do a transpose matrix vector product using a user supplied vector **/
//
//    void CalculateAdjointMatrixVectorProduct(double *VecIn, double *VecOut);
//
//    /** Calculate partial of functions wrt gamma **/
//
//    void CalculateAdjointRightHandSide(int OptimizationCase, double *RHS);
//    
//    /** Solve the adjoint equation for user defined right hand side **/
//    
//    void SolveAdjointLinearSystem(double *Psi, double *RHS);
//
//    /** Calculate partial derivatives of optimization case = Case wrt the mesh, the input variables, and gamma **/
//    
//    void CalculateOptimizationFunctionPartials(int OptimizationCase, double *pF_pMesh, double *pF_pInputVariable, double *pF_pGamma);
//    
//    /** Calculate the adjoint residual partial products given the vector Psi **/
//    
//    void CalculateAdjointResidualPartialProducts(double *Psi, double *PsiT_pR_pMesh, double *pR_pInputVariable);
//        
//    /** Calculate the mesh, the input variables, and gamma partial products given the vector pF_pForces **/
//
//    void CalculateNodalForcePartialProducts(double *pF_pForces, double* pF_pMesh, double *pF_pInputVariable, double *pF_pGamma);
//

    /** Return the L2 residual **/
    
    double L2Residual(void) { return L2Residual_; };

    /** Do a restart using a previous solution file **/
        
    int &DoRestart(void) { return DoRestart_; };
    
    /** Save a restart file **/
    
    int &SaveRestartFile(void) { return SaveRestartFile_; };
    
    /** Output a status file **/
    
    void OutputStatusFile(int FinalIteration);

    /** Read in the geometry, do a minimum of set up and then write out an adb file we can view in Viewer ... no solve **/
    
    int &DumpGeom(void) { return DumpGeom_; };
    
    /** Create a default boundary conditions setup file **/
    
    int &CreateHighLiftFile(void) { return CreateHighLiftFile_; };

    /** Write out 2D FEM load file **/
    
    int &Write2DFEMFile(void) { return Write2DFEMFile_; };
    
    /** Write out tecplot file **/
    
    int &WriteTecplotFile(void) { return WriteTecplotFile_; };
         
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

    /** Set the time step to start averaging time accurate forces **/
    
    int &StartAveragingTimeStep(void) { return StartAveragingTimeStep_; };
    
    /** Set the ime step **/
     
    double &DeltaTime(void) { return DeltaTime_; };
    
    /** Starts time accurate solve with a finite free stream and 
     * slowly tapers that to zero to help with solving stiff
     * hover cases
     *
     **/

    /** Turn on the stall model **/
    
    int &StallModelIsOn(void) { return StallModelIsOn_; };
    
    /** Turn off writing of ADB file **/
    
    int &NoADBFile(void) { return NoADBFile_; };
    
    /** Perform the solve on MGLevel = SolveOnMGLevel ... this has the solver
     * run using SolveOnMGLevel_ level as the finest grid level. This allows
     * the user to get a flow solution on one of the coarser, agglomerated, grids.
     * There MUST be at least 1 level of coarse mesh than SolveOnMGLevel_ otherwise
     * the solve will fail **/
     
    void SolveOnMGLevel(int Level) { SolveOnMGLevel_ = Level;  VSPGeom().SolveOnMGLevel() = Level; };
    
    /** Turn off the calculation of spanwise loading for a panel solver...
     * 
     * this is here since the code can get confused for cart3d tri file
     * geometries that just really don't have the information to figure
     * out how to calculate spanwise loading information... without this
     * the code might just quit after bashing it's head into the ground
     **/
     
    int &PanelSpanWiseLoading(void) { return PanelSpanWiseLoading_; };
    
    /** Set reduced frequency for an unsteady pitch/yaw analysis **/
    
    double &ReducedFrequency(void) { return ReducedFrequency_; };
    
    /** Get back the actual unsteady angle rate **/
    
    double Unsteady_AngleRate(void) { return Unsteady_AngleRate_; };
    
    /** Set the unsteady maximum angle... pitch or yaw **/
    
    double &Unsteady_AngleMax(void) { return Unsteady_AngleMax_; };
    
    /** Set the unsteady max heave value **/
    
    double &Unsteady_HMax(void) { return Unsteady_HMax_; };

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
        
    /** Number of nodes on the finest grid level **/
    
    int NumberOfGridNodes(void) { return VSPGeom().Grid(1).NumberOfNodes(); };
    
    /** Number of vortex loop on the finest mesh level **/
    
    int NumberOfVortexLoops(void) { return VSPGeom().Grid(0).NumberOfLoops(); };
    
    /** Total number of adjoint equations **/
    
    int NumberOfAdjointEquations(void) { return NumberOfEquations_; };

    /** Do the adjoint solve **/

    int &DoAdjointSolve(void) { return DoAdjointSolve_; };
    
    /** Adjoint force type, inviscid, viscous, or total **/
    
    int &AdjointSolutionForceType(void) { return AdjointSolutionForceType_; };

    /** Adjoint force/moment cases to solve... Forces in x,y,z ~ 1,2,3 ... Moments in x,y,z ~ 4,5, 6 **/
    
    int &DoAdjointSolveForThisForceMomentCase(int i) { return DoAdjointSolveForThisForceMomentCase_[i]; };
    
    /** User defined list of component groups to calculate adjoint gradients for **/
    
    int &UserAdjointComponentList(int i) { return UserAdjointComponentList_[i]; };

    /** This is an optimization solve
     * 
     * this must be set to let the solver know it needs to evaluate the 
     * optimization functions... for both the foward and adjoint solvers
     * 
     **/

    int &OptimizationSolve(void) { return OptimizationSolve_; };
                    
    /** Calculate nodal pressures on the mesh for FEM analysis **/
    
    void CalculateNodalPressures(void);
    
    /** Pressure at node i of the mesh **/
    
    double NodalCp(int i) { return NodalCp_[i]; };

    /** Set the matrix preconditioner **/
    
    int &Preconditioner(void ) { return Preconditioner_; };
    
    /** Force update of matric preconditioners every newton iteration **/
   
    int &UpdateMatrixPreconditioner(void) { return UpdateMatrixPreconditioner_; };

    /** Turn on the wake matrix preconditioner **/
    
    int &UseWakeNodeMatrixPreconditioner(void) { return UseWakeNodeMatrixPreconditioner_; };

    /** Set the user case string **/
    
 //   char *CaseString(void) { return CaseString_; };
        
    char (&CaseString(void))[MAX_CHAR_SIZE] { return CaseString_; };
        
    /** Write a Generic File header to FILE id fid **/
    
    void WriteCaseHeader(FILE *fid);
    
    /** Write out a cart3d tri file of the top level mesh **/
    
    void WriteOutCart3dTriFile(void);

};

#include "END_NAME_SPACE.H"

#endif
