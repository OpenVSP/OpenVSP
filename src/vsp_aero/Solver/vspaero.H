//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef _VSPAERO_H_
#define _VSPAERO_H_

// --- INCLUDES ---
#include "./AdjointGradient.H"
#include "./BoundaryConditionData.H"
#include "./CharSizes.H"
#include "./ComponentGroup.H"
#include "./ControlSurface.H"
#include "./ControlSurfaceGroup.H"
#include "./EngineFace.H"
#include "./FastMatrix.H"
#include "./GlobalWakeLoop.H"
#include "./Gradient.H"
#include "./Interaction.H"
#include "./InteractionLoop.H"
#include "./MatPrecon.H"
#include "./MergeSort.H"
#include "./OptimizationFunction.H"
#include "./QuadCell.H"
#include "./QuadEdge.H"
#include "./QuadNode.H"
#include "./QuadTree.H"
#include "./RotorDisk.H"
#include "./SaveState.H"
#include "./Search.H"
#include "./SearchLeaf.H"
#include "./VSPAERO_OMP.H"
#include "./VSP_Agglom.H"
#include "./VSP_Edge.H"
#include "./VSP_Geom.H"
#include "./VSP_Grid.H"
#include "./VSP_Loop.H"
#include "./VSP_Node.H"
#include "./VSP_Solver.H"
#include "./Vortex_Sheet.H"
#include "./Vortex_Trail.H"
#include "./WakeEdgeData.H"
#include "./matrix.H"
#include "./memory.H"
#include "./quat.H"
#include "./time.H"
#include "./utils.H"
#include <vector>

extern char *FileName;
extern double Sref_;
extern double Cref_;
extern double Bref_;
extern double Xcg_;
extern double Ycg_;
extern double Zcg_;
extern double Cl_;
extern double Cd_;
extern double Cm_;
extern double Mach_;
extern double AoA_;
extern double Beta_;
extern double Rho_;
extern double Vinf_;
extern double Vref_;
extern double Machref_;
extern double ReCref_;
extern double Clo2D_;
extern double CLMax2D_;
extern double FarDist_;
extern double TimeStep_;
extern double ReducedFrequency_;
extern double UnsteadyAngleMax_;
extern double UnsteadyHMax_;     
extern double HeightAboveGround_;
extern double BladeRPM_;
extern double WakeRelax_;
extern double FarAway_;
extern double ForwardGMRESConvergenceFactor_;
extern double AdjointGMRESConvergenceFactor_;
extern double NonLinearConvergenceFactor_;
extern double CoreSizeFactor_;

extern int AdjointSolutionForceType_;
extern int NumberOfAdjointForceAndMomentCases_;
extern int AdjointSolveForThisForceMomentCase_[7];
extern int NumberOfAdjointComponentListSettings_;
extern int UserAdjointComponentList_[10000];
#define MAXRUNCASES 100000

// Number of Machs, AoAs, and Betas

extern int NumberOfMachs_;
extern int NumberOfAoAs_;
extern int NumberOfBetas_;
extern int NumberOfReCrefs_;

// Mach, AoA, and Beta Lists

extern double   MachList_[MAXRUNCASES];
extern double    AoAList_[MAXRUNCASES];
extern double   BetaList_[MAXRUNCASES];
extern double ReCrefList_[MAXRUNCASES];

// Control surfaces

extern int NumberOfControlGroups_;
//CONTROL_SURFACE_GROUP *ControlSurfaceGroup_;

// Stability and control Mach, AoA, and Beta Lists

extern double Stab_MachList_[MAXRUNCASES];
extern double  Stab_AoAList_[MAXRUNCASES];
extern double Stab_BetaList_[MAXRUNCASES];

// Stability Rotational rates list

extern double RotationalRate_pList_[MAXRUNCASES];
extern double RotationalRate_qList_[MAXRUNCASES];
extern double RotationalRate_rList_[MAXRUNCASES];

// Deltas for calculating derivaties

extern double Delta_AoA_;
extern double Delta_Beta_;
extern double Delta_Mach_;
extern double Delta_P_;
extern double Delta_Q_;
extern double Delta_R_;
extern double Delta_Control_;

// KJ forces and moments

extern double CFixForCase[MAXRUNCASES];
extern double CFiyForCase[MAXRUNCASES];
extern double CFizForCase[MAXRUNCASES];
    
extern double CMixForCase[MAXRUNCASES];
extern double CMiyForCase[MAXRUNCASES];
extern double CMizForCase[MAXRUNCASES];

extern double CLiForCase[MAXRUNCASES];
extern double CDiForCase[MAXRUNCASES];
extern double CSiForCase[MAXRUNCASES];

extern double CMilForCase[MAXRUNCASES];
extern double CMimForCase[MAXRUNCASES];
extern double CMinForCase[MAXRUNCASES];

// Wake forces

extern double CFwxForCase[MAXRUNCASES];
extern double CFwyForCase[MAXRUNCASES];
extern double CFwzForCase[MAXRUNCASES];

extern double CLwForCase[MAXRUNCASES];
extern double CDwForCase[MAXRUNCASES];
extern double CSwForCase[MAXRUNCASES];

// Viscous forces and moments

extern double CFoxForCase[MAXRUNCASES];
extern double CFoyForCase[MAXRUNCASES];
extern double CFozForCase[MAXRUNCASES];
    
extern double CMoxForCase[MAXRUNCASES];
extern double CMoyForCase[MAXRUNCASES];
extern double CMozForCase[MAXRUNCASES];

extern double CLoForCase[MAXRUNCASES];
extern double CDoForCase[MAXRUNCASES];
extern double CSoForCase[MAXRUNCASES];

extern double CMolForCase[MAXRUNCASES];
extern double CMomForCase[MAXRUNCASES];
extern double CMonForCase[MAXRUNCASES];

// Total forces and moments

extern double CFtxForCase[MAXRUNCASES];
extern double CFtyForCase[MAXRUNCASES];
extern double CFtzForCase[MAXRUNCASES];

extern double CMtxForCase[MAXRUNCASES];
extern double CMtyForCase[MAXRUNCASES];
extern double CMtzForCase[MAXRUNCASES];

extern double CLtForCase[MAXRUNCASES];
extern double CDtForCase[MAXRUNCASES];
extern double CStForCase[MAXRUNCASES];

extern double CMtlForCase[MAXRUNCASES];
extern double CMtmForCase[MAXRUNCASES];
extern double CMtnForCase[MAXRUNCASES];

// Stall factor

extern double MinStallFactor[MAXRUNCASES];

// Optimization function for case

extern double OptimizationFunctionForCase[MAXRUNCASES];

// Stability derivatives

extern double dCFx_wrt[MAXRUNCASES];
extern double dCFy_wrt[MAXRUNCASES];
extern double dCFz_wrt[MAXRUNCASES];

extern double dCMx_wrt[MAXRUNCASES];
extern double dCMy_wrt[MAXRUNCASES];
extern double dCMz_wrt[MAXRUNCASES];

extern double dCL_wrt[MAXRUNCASES];
extern double dCD_wrt[MAXRUNCASES];
extern double dCS_wrt[MAXRUNCASES];

extern double dCMl_wrt[MAXRUNCASES];
extern double dCMm_wrt[MAXRUNCASES];
extern double dCMn_wrt[MAXRUNCASES];

//FILE *StabFile;
//FILE *VorviewFlt;

extern int WakeIterations_;
extern int NumberOfRotors_;
extern int NumStabCases_;
extern int NumberOfThreads_;
extern int StabControlRun_;
extern int SetFreeStream_ ;
extern int SaveRestartFile_;
extern int DoRestartRun_;
extern int SetFarDist_;
extern int Symmetry_;
extern int NumberOfWakeNodes_;
extern int DumpGeom_;
extern int ImplicitWake_;
extern int ImplicitWakeStartIteration_;
extern int NumberofSurveyPoints_;
extern int NumberOfSurveyTimeSteps_;
extern int LoadFEMDeformation_;
extern int DoGroundEffectsAnalysis_;
extern int Write2DFEMFile_;
extern int WriteTecplotFile_;
extern int DoUnsteadyAnalysis_;
extern int StartFromSteadyState_;
extern int UnsteadyAnalysisType_;
extern int NumberOfTimeSteps_;
extern int StartAveragingTimeStep_;
extern int CreateNoiseFiles_;
extern int NoPanelSpanWiseLoading_;
extern int DoAdjointSolve_;
extern int OptimizationSolve_;
extern int OptimizationFunction_;
extern int SetupHighLiftFile_;
extern int RestartAndInterrogateSolution_;
extern int NumberOfQuadTrees_;
extern int NumberOfInlets_;
extern int NumberOfNozzles_;
extern int DoFiniteDiffTest_;
extern int FiniteDiffTestStartNode_;
extern int FiniteDiffTestStepSize_;
extern int FiniteDiffTestLoadGroup_;
extern int StallModelIsOn_;
extern int SolveOnMGLevel_;
extern int UpdateMatrixPreconditioner_;
extern int UseWakeNodeMatrixPreconditioner_;
extern int FreezeMultiPoleAtIteration_;
extern int FreezeWakeAtIteration_;
extern int FreezeWakeRootVortices_;
extern int QuadTreeBufferLevels_;
extern int VSPAERO_InputFileFormatOutput_;
extern int RestartFromPreviousSolve_;

// Optimization variables

extern int OptimizationNumberOfIterations_;
extern int OptimizationNumber1DSearchSteps_;
extern int OptimizationUsingWakeForces_;
extern int OptimizationUpdateGeometryGradients_;

extern double Optimization_CL_Weight_;
extern double Optimization_CD_Weight_;
extern double Optimization_CS_Weight_;

extern double Optimization_CML_Weight_;
extern double Optimization_CMM_Weight_;
extern double Optimization_CMN_Weight_;

extern double Optimization_CL_Required_;
extern double Optimization_CD_Required_;
extern double Optimization_CS_Required_;

extern double Optimization_CML_Required_;
extern double Optimization_CMM_Required_;
extern double Optimization_CMN_Required_;

extern double Optimization_Lambda_1_;
extern double Optimization_Lambda_2_;
extern double Optimization_Lambda_3_;
extern double Optimization_Lambda_4_;
extern double Optimization_Lambda_5_;
extern double Optimization_Lambda_6_;

extern double OptimizationGradientReduction_;
extern double OptimizationFunctionReduction_;

// Optimization function variables

class PARAMETER_DATA {
public:
    double *ParameterValues;
    char **ParameterNames;
    int NumParameters;
    double **dMesh_dParameter;
};

class GRADIENT_DATA {
   
public:
   
   double *dF_dParameter;
   double CL, CD, CS, CML, CMM, CMN;
};

int SearchForCharacterVariable(FILE *, const char *, char *);
int SearchForFloatVariable(FILE *, const char *, double &);
int SearchForFloatVariableList(FILE *, const char *, double *, int &);
int SearchForIntVariableList(FILE *, const char *, int *, int &);
int SearchForIntegerVariable(FILE *, const char *, int &);
int main(int, char **);
void ApplyControlDeflections();
void CalculateAerodynamicCenter();
void CalculateStabilityDerivatives();
void FiniteDiffTestSolve();
void FiniteDifference_StabilityAndControlSolve();
void LoadCaseFile(int);
void Noise();
void PRINT_STAB_LINE(double, double, double, double, double, double, double, double);
void ParseInput(int, char *[]);
void PrintUsageHelp();
void Solve();
void UnsteadyStabilityAndControlSolve();
void WriteOutAdjointStabilityDerivatives();
void WriteOutVorviewFLTFile();
void VSPAERO_Optimize();
void SaveVSPGeomFile(char *, int, char **, double *);

GRADIENT_DATA *VSPAERO_Optimize_Gradients(PARAMETER_DATA *ParameterData, int, bool);
PARAMETER_DATA *VSPAERO_Optimize_Setup();
std::vector<double> get_dF_dParameter_array(double*, int, int);
std::vector<std::string> get_ParameterNames_array(char**, int);

#endif // _VSPAERO_H_