//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Optimizer.H"

/*##############################################################################
#                                                                              #
#                       VSP_OPTIMIZER Constructor                              #
#                                                                              #
##############################################################################*/

VSP_OPTIMIZER::VSP_OPTIMIZER(void)
{

    WakeIterations_                = 0;
    NumberOfRotors_                = 0;
    NumStabCases_                  = 7;
    NumberOfThreads_               = 1;
    StabControlRun_                = 0;
    SetFreeStream_                 = 0;
    SaveRestartFile_               = 0;
    DoRestartRun_                  = 0;
    DoSymmetry_                    = 0;
    SetFarDist_                    = 0;
    Symmetry_                      = 0;
    NumberOfWakeNodes_             = 0;
    DumpGeom_                      = 0;
    NoWakeIteration_               = 0;
    NumberofSurveyPoints_          = 0;
    NumberOfSurveyTimeSteps_       = 0;
    LoadFEMDeformation_            = 0;
    DoGroundEffectsAnalysis_       = 0;
    Write2DFEMFile_                = 0;
    DoUnsteadyAnalysis_            = 0;
    StartFromSteadyState_          = 0;
    UnsteadyAnalysisType_          = 0;
    NumberOfTimeSteps_             = 0;
    NumberOfTimeSamples_           = 0;
    RotorAnalysisRun_              = 0;
    CreateNoiseFiles_              = 0;
    NoPanelSpanWiseLoading_        = 0;
    DoAdjointSolve_                = 0;
    OptimizationSolve_             = 0;
    SetupHighLiftFile_             = 0;
    RestartAndInterrogateSolution_ = 0;
    NumberOfQuadTrees_             = 0;
    NumberOfInlets_                = 0;
    NumberOfNozzles_               = 0;
    DoComplexDiffTest              = 0;
    DoFiniteDiffTest               = 0;
    
    NumberOfThreads_               = 1;
    OptimizationFunction_[1]       = 1;
    OptimizationSet_[1]            = 1;
    
    BladeRPM_                      = 0.;
    WakeRelax_                     = 1.;
    GMRESReductionFactor_          = 1.;
    CoreSizeFactor_                = 1.;
    
    ArrayOffSet_ = 0;

}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER Destructor                                #
#                                                                              #
##############################################################################*/

VSP_OPTIMIZER::~VSP_OPTIMIZER(void)
{

    // Nothing to do 

}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER Copy                                      #
#                                                                              #
##############################################################################*/

VSP_OPTIMIZER::VSP_OPTIMIZER(const VSP_OPTIMIZER &Optimizer)
{

    printf("VSP_OPTIMIZER copy not implemented! \n");
    fflush(NULL);
    exit(1);

}

/*##############################################################################
#                                                                              #
#                         VSP_OPTIMIZER Setup                                  #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::Setup(char *FileName)
{
   
    int p;

    sprintf(FileName_,"%s",FileName);

    printf("Working on file: %s \n",FileName_); fflush(NULL);
    
#ifdef VSPAERO_OPENMP

    printf("Initializing OPENMP for %d threads \n",NumberOfThreads_);
   
    omp_set_num_threads(NumberOfThreads_);
    
    NumberOfThreads_ = omp_get_max_threads();

    printf("NumberOfThreads_: %d \n",NumberOfThreads_);
#else
    NumberOfThreads_ = 1;
    printf("Single threaded build.\n");
#endif

    // Create VSP Solver object

    Solver_ = new VSPAERO_SOLVER::VSP_SOLVER ;
    
    // Settings

    Solver().OptimizationSolve() = 1;

    Solver().NumberOfOptimizationFunctions() = NumberOfOptimizationFunctions_;

    for ( p = 1 ; p <= NumberOfOptimizationFunctions_ ; p++ ) {

       Solver().SetOptimizationFunction(p, OptimizationFunction_[p], OptimizationSet_[p]);

    } 
     
    Solver().TimeAccurate() = DoUnsteadyAnalysis_;
    
    Solver().TimeAnalysisType() = 0;
     
    // Load the vspaero case file
           
    LoadCaseFile(Solver());
    
    // Read in the geometry
                     
    Solver().ReadFile(FileName_);
 
    // Setup stuff
                 
    Solver().Setup();


  
    // Turn on adept stack and create VSP Adjoint object
        
    Adjoint_ = new VSPAERO_ADJOINT::VSP_SOLVER;
   
    // Settings
    
    Adjoint().OptimizationSolve() = 1;
    
    Adjoint().DoAdjointSolve() = 1;

    Adjoint().NumberOfOptimizationFunctions() = NumberOfOptimizationFunctions_;

    for ( p = 1 ; p <= NumberOfOptimizationFunctions_ ; p++ ) {

       Adjoint().SetOptimizationFunction(p, OptimizationFunction_[p], OptimizationSet_[p]);
       
    } 
     
    Adjoint().TimeAccurate() = DoUnsteadyAnalysis_;
    
    Adjoint().TimeAnalysisType() = 0;
            
    // Load the vspaero case file

    LoadCaseFile(Adjoint());    
            
    // Read in the geometry
  
    Adjoint().ReadFile(FileName_);
    
    // Setup stuff

    Adjoint().Setup();
  
    // Create some temp arrays
    
    MaxTempArraySize_ = 3*std::max(std::max(Solver().NumberOfGridNodes(), Solver().NumberOfVortexLoops()), Adjoint().NumberOfAdjointEquations());
    
    TempArray_[0] = new double[MaxTempArraySize_ + 1];
    TempArray_[1] = new double[MaxTempArraySize_ + 1];
    TempArray_[2] = new double[MaxTempArraySize_ + 1];

}

/*##############################################################################
#                                                                              #
#                  VSP_OPTIMIZER SetMachNumber                                 #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetMachNumber(double Mach)
{
   
   Solver().Mach() = Mach;
   
   Adjoint().Mach() = Mach;
      
}
   
/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER SetAoADegrees                             #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetAoADegrees(double Alpha)
{
   
   Solver().AngleOfAttack() = Alpha*TORAD;
   
   Adjoint().AngleOfAttack() = Alpha*TORAD;
      
}   

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER SetBeta                                   #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetBetaDegrees(double Beta)
{
   
   Solver().AngleOfBeta() = Beta*TORAD;
   
   Adjoint().AngleOfBeta() = Beta*TORAD;
      
}   

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER SetVinf                                   #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetVinf(double Vinf)
{
   
   Solver().Vinf() = Vinf;
   
   Adjoint().Vinf() = Vinf;
      
}   

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER SetDensity                                #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetDensity(double Density)
{
   
   Solver().Density() = Density;
   
   Adjoint().Density() = Density;
      
}   

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER SetDensity                                #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetReCref(double ReCref)
{
   
   Solver().ReCref() = ReCref;
   
   Adjoint().ReCref() = ReCref;
      
}   

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER SetRotationalRate_p                       #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetRotationalRate_p(double P)
{
   
   Solver().RotationalRate_p() = P;
   
   Adjoint().RotationalRate_p() = P;
      
}   

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER SetRotationalRate_q                       #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetRotationalRate_q(double Q)
{
   
   Solver().RotationalRate_q() = Q;
   
   Adjoint().RotationalRate_q() = Q;
      
}   

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER SetRotationalRate_q                       #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetRotationalRate_r(double R)
{
   
   Solver().RotationalRate_r() = R;
   
   Adjoint().RotationalRate_r() = R;
      
}   

/*##############################################################################
#                                                                              #
#               VSP_OPTIMIZER SetGroupOmegaRotationRate                        #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetGroupOmegaRotationRate(int Group, double Omega)
{
   
   Solver().GroupOmegaRate(Group) = Omega;
   
   Adjoint().GroupOmegaRate(Group) = Omega;
      
}   

/*##############################################################################
#                                                                              #
#                          VSP_OPTIMIZER SetSref                               #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetSref(double Sref)
{
   
   Solver().Sref() = Sref;
   
   Adjoint().Sref() = Sref;
      
}   

/*##############################################################################
#                                                                              #
#                          VSP_OPTIMIZER SetBref                               #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetBref(double Bref)
{
   
   Solver().Bref() = Bref;
   
   Adjoint().Bref() = Bref;
      
}   

/*##############################################################################
#                                                                              #
#                          VSP_OPTIMIZER SetCref                               #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetCref(double Cref)
{
   
   Solver().Cref() = Cref;
   
   Adjoint().Cref() = Cref;
      
}   

/*##############################################################################
#                                                                              #
#                           VSP_OPTIMIZER SetXcg                               #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetXcg(double Xcg)
{
   
   Solver().Xcg() = Xcg;
   
   Adjoint().Xcg() = Xcg;
      
}   

/*##############################################################################
#                                                                              #
#                           VSP_OPTIMIZER SetYcg                               #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetYcg(double Ycg)
{
   
   Solver().Ycg() = Ycg;
   
   Adjoint().Ycg() = Ycg;
      
}   

/*##############################################################################
#                                                                              #
#                           VSP_OPTIMIZER SetZcg                               #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetZcg(double Zcg)
{
   
   Solver().Zcg() = Zcg;
   
   Adjoint().Zcg() = Zcg;
      
}  

/*##############################################################################
#                                                                              #
#                           VSP_OPTIMIZER SetClo2D                             #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetClo2D(double Clo2D)
{
   
   Solver().Clo2D() = Clo2D;
   
   Adjoint().Clo2D() = Clo2D;
      
}  

/*##############################################################################
#                                                                              #
#                           VSP_OPTIMIZER SetClMax                             #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetClMax(double ClMax)
{
   
   Solver().ClMax() = ClMax;
   
   Adjoint().ClMax() = ClMax;
      
}  

/*##############################################################################
#                                                                              #
#                    VSP_OPTIMIZER SetNumWakeNodes                             #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetNumWakeNodes(int NumWakeNodes)
{
   
   Solver().SetNumberOfWakeTrailingNodes(NumWakeNodes);
   
   Adjoint().SetNumberOfWakeTrailingNodes(NumWakeNodes);
      
}  

/*##############################################################################
#                                                                              #
#                    VSP_OPTIMIZER SetWakeIterations                           #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetWakeIterations(int WakeIterations)
{
   
   Solver().WakeIterations() = WakeIterations;
   
   Adjoint().WakeIterations() = WakeIterations;
      
}  

/*##############################################################################
#                                                                              #
#                           VSP_OPTIMIZER SetFarDist                           #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetFarDist(double FarDist)
{
   
   Solver().SetFarFieldDist(FarDist);
   
   Adjoint().SetFarFieldDist(FarDist);
      
}

/*##############################################################################
#                                                                              #
#                 VSP_OPTIMIZER TurnOnKarmanTsienCorrection                    #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::TurnOnKarmanTsienCorrection(void)
{
   
   Solver().KarmanTsienCorrection() = 1;
   
   Adjoint().KarmanTsienCorrection() = 1;
      
}

/*##############################################################################
#                                                                              #
#                 VSP_OPTIMIZER TurnOffKarmanTsienCorrection                   #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::TurnOffKarmanTsienCorrection(void)
{
   
   Solver().KarmanTsienCorrection() = 1;
   
   Adjoint().KarmanTsienCorrection() = 1;
      
}

/*##############################################################################
#                                                                              #
#                    VSP_OPTIMIZER SetGMRESToleranceFactor                     #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetGMRESToleranceFactor(double tolFactor)
{

   Solver().User_GMRES_ToleranceFactor() = tolFactor;

   Adjoint().User_GMRES_ToleranceFactor() = tolFactor;

}

/*##############################################################################
#                                                                              #
#                         VSP_OPTIMIZER Solve                                  #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::Solve(void)
{

    // Run the solver
    
    SolveForward();
 
    // Run the adjoint
        
    SolveAdjoint();   

}

/*##############################################################################
#                                                                              #
#                         VSP_OPTIMIZER Solve                                  #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SolveForward(void)
{

    Solver().Solve(0);    

}


/*##############################################################################
#                                                                              #
#                         VSP_OPTIMIZER Solve                                  #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SolveAdjoint(void)
{

    Adjoint().Optimization_Solve(0);    

}

/*##############################################################################
#                                                                              #
#                         VSP_OPTIMIZER ShiftInputVector                       #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::ShiftInputVector(double *Vec1, double *Vec2, int Length)
{

    int i;
    
    for ( i = 1 ; i <= Length ; i++ ) {
       
       Vec2[i] = Vec1[i - ArrayOffSet_];
       
    }

}


/*##############################################################################
#                                                                              #
#                         VSP_OPTIMIZER ShiftOutputVector                      #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::ShiftOutputVector(double *Vec1, double *Vec2, int Length)
{

    int i;
    
    for ( i = 1 ; i <= Length ; i++ ) {
       
       Vec2[i - ArrayOffSet_] = Vec1[i];
       
    }

}

/*##############################################################################
#                                                                              #
#       VSP_OPTIMIZER CalculateMatrixVectorProductAndRightHandSide             #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::CalculateMatrixVectorProductAndRightHandSide(double *VecIn, double *VecOut, double *RHS )
{

    // Shift inputs
    
    ShiftInputVector(VecIn,TempArray_[0],Solver().NumberOfVortexLoops());
    
    // Do matrix-vector mult, and get the RHS vector
    
    Solver().CalculateMatrixVectorProductAndRightHandSide(TempArray_[0], TempArray_[1], TempArray_[2]);

    // Shift outputs 
        
    ShiftOutputVector(TempArray_[1],VecOut,Solver().NumberOfVortexLoops());
    
    ShiftOutputVector(TempArray_[2],RHS,Solver().NumberOfVortexLoops());
    
}

/*##############################################################################
#                                                                              #
#     VSP_OPTIMIZER CalculateAdjointMatrixVectorProductAndRightHandSide        #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::CalculateAdjointMatrixVectorProductAndRightHandSide(double *VecIn, double *VecOut, double *RHS)
{

    // Shift inputs
     
    ShiftInputVector(VecIn,TempArray_[0],Adjoint().NumberOfAdjointEquations());
    
    // Do matrix-vector mult, and get the RHS vector
    
    Adjoint().CalculateAdjointMatrixVectorProductAndRightHandSide(TempArray_[0], TempArray_[1], TempArray_[2]);

    // Shift outputs 
        
    ShiftOutputVector(TempArray_[1],VecOut,Adjoint().NumberOfAdjointEquations());
    
    ShiftOutputVector(TempArray_[2],RHS,Adjoint().NumberOfAdjointEquations());
       
}

/*##############################################################################
#                                                                              #
#              VSP_OPTIMIZER CalculateForwardResidual                          #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::CalculateForwardResidual(double *Gamma, double *Residual)
{

    // Shift inputs
    
    ShiftInputVector(Gamma,TempArray_[0],Solver().NumberOfVortexLoops());
    
    // Do matrix-vector mult, and get the RHS vector
    
    Solver().CalculateForwardResidual(TempArray_[0], TempArray_[1]);

    // Shift outputs 
        
    ShiftOutputVector(TempArray_[1],Residual,Solver().NumberOfVortexLoops());

}


/*##############################################################################
#                                                                              #
#              VSP_OPTIMIZER CalculateOptimizationFunctions                    #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::CalculateOptimizationFunctions(double *Gamma)
{

    // Shift inputs
    
    ShiftInputVector(Gamma,TempArray_[0],Solver().NumberOfVortexLoops());
    
    // Do matrix-vector mult, and get the RHS vector
    
    Solver().CalculateOptimizationFunctions(TempArray_[0]);

}
/*##############################################################################
#                                                                              #
#              VSP_OPTIMIZER SolveAdjointLinearSystem                          #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SolveAdjointLinearSystem(double *Psi, double *RHS)
{

    // Shift inputs
     
    ShiftInputVector(RHS,TempArray_[1],Adjoint().NumberOfAdjointEquations());
    
    // Do matrix-vector mult, and get the RHS vector
    
    Adjoint().SolveAdjointLinearSystem(TempArray_[0], TempArray_[1]);

    // Shift outputs 
        
    ShiftOutputVector(TempArray_[0],Psi,Adjoint().NumberOfAdjointEquations());
   
}

/*##############################################################################
#                                                                              #
#            VSP_OPTIMIZER CalculateOptimizationFunctionPartials               #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::CalculateOptimizationFunctionPartials(int Case, double *pF_pMesh, double *pF_pInputVariable)
{

    int i;
    
    // Calculate partials of function wrt mesh and input variables

    Adjoint().CalculateOptimizationFunctionPartials(Case, TempArray_[0], TempArray_[1]);

    // Shift outputs for gradients wrt mesh
    
    for ( i = 1 ; i <= Adjoint().VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {
       
        pF_pMesh[3*i-2 - ArrayOffSet_] = TempArray_[0][3*i-2];
        pF_pMesh[3*i-1 - ArrayOffSet_] = TempArray_[0][3*i-1];
        pF_pMesh[3*i   - ArrayOffSet_] = TempArray_[0][3*i  ];
        
    }

    // Shift output for gradients wrt input variables ... we assume there's just one group
    // this needs to be updated with better logic if there's more than 1 group!
    
    for ( i = 1 ; i <= OPT_GRADIENT_NUMBER_OF_INPUTS + 1 ; i++ ) {
       
        pF_pInputVariable[i - ArrayOffSet_] = TempArray_[1][i];

    }

}

/*##############################################################################
#                                                                              #
#            VSP_OPTIMIZER CalculateOptimizationFunctionPartials               #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::CalculateAdjointResidualPartialProducts(double *Psi, double *pR_pMesh, double *pR_pInputVariable)
{

    int i;

    // Shift inputs
     
    ShiftInputVector(Psi,TempArray_[0],Adjoint().NumberOfAdjointEquations());
        
    // Calculate partials of function wrt mesh and input variables

    Adjoint().CalculateAdjointResidualPartialProducts(TempArray_[0], TempArray_[1], TempArray_[2]);

    // Shift outputs for gradients wrt residual - vector products
    
    for ( i = 1 ; i <= Adjoint().VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {
       
        pR_pMesh[3*i-2 - ArrayOffSet_] = TempArray_[1][3*i-2];
        pR_pMesh[3*i-1 - ArrayOffSet_] = TempArray_[1][3*i-1];
        pR_pMesh[3*i   - ArrayOffSet_] = TempArray_[1][3*i  ];
        
    }

    // Shift output for gradients wrt input variables ... we assume there's just one group
    // this needs to be updated with better logic if there's more than 1 group!
    
    for ( i = 1 ; i <= OPT_GRADIENT_NUMBER_OF_INPUTS + 1 ; i++ ) {
       
        pR_pInputVariable[i - ArrayOffSet_] = TempArray_[2][i];

    }

}

/*##############################################################################
#                                                                              #
#                         VSP_OPTIMIZER SetGradientVector                      #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SetGradientVector(int Case, double *Vec)
{

    // Possible user defined input vector for gradients
    
    if ( Vec != NULL ) {
       
       ShiftInputVector(Vec,TempArray_[0],Solver().OptimizationFunctionLength(Case));

       Adjoint().SetOptimizationFunctionInputGradientVector(Case, Solver().OptimizationFunctionLength(Case), Solver().OptimizationNumberOfTimeSteps(Case), OptimizationSet_[Case], TempArray_[0]);
       
    }

}

/*##############################################################################
#                                                                              #
#                       VSP_OPTIMIZER LoadCaseFile                             #
#                                                                              #
##############################################################################*/

template <typename T>
void VSP_OPTIMIZER::LoadCaseFile(T &VSP)
{

    int i, j, k, NumberOfControlSurfaces, Done, Dir, Surface;
    double x,y,z, DumDouble, HingeVec[3], RotAngle, DeltaHeight;
    double Value, MassFlow, Velocity, DeltaCp;
    FILE *case_file;
    char file_name_w_ext[2000], DumChar[2000], DumChar2[2000], Comma[2000], *Next;
    char SymmetryFlag[2000];
    VSPAERO_SOLVER::QUAT Quat, InvQuat, Vec;

    // Delimiters
    
    sprintf(Comma,",");

    // Open the case file

    sprintf(file_name_w_ext,"%s.vspaero",FileName_);

    if ( (case_file = fopen(file_name_w_ext,"r")) == NULL ) {

       printf("Could not open the file: %s for input! \n",file_name_w_ext);

       exit(1);

    }

    Sref_              = 1.0;
    Cref_              = 1.0;  
    Bref_              = 1.0; 
    Xcg_               = 0.0;
    Ycg_               = 0.0;
    Zcg_               = 0.0;
    Mach_              = 0.3;
    AoA_               = 5.0;
    Beta_              = 0.0;
    Vinf_              = 100.;
    Vref_              = 100.;
    Machref_           = 0.3;
    Rho_               = 0.002377;
    ReCref_            = 10000000.;
    ClMax_             = -1.;
    Clo2D_             =  0.;    
    MaxTurningAngle_   = -1.;
    FarDist_           = -1.;
    NumberOfWakeNodes_ = -1;
    WakeIterations_    = 5;

    fscanf(case_file,"Sref = %lf \n",&Sref_);
    fscanf(case_file,"Cref = %lf \n",&Cref_);
    fscanf(case_file,"Bref = %lf \n",&Bref_);
    fscanf(case_file,"X_cg = %lf \n",&Xcg_);
    fscanf(case_file,"Y_cg = %lf \n",&Ycg_);
    fscanf(case_file,"Z_cg = %lf \n",&Zcg_);

    // Load in Mach list
    
    fgets(DumChar,2000,case_file);

    if ( strstr(DumChar,Comma) == NULL ) {

       sscanf(DumChar,"Mach = %lf \n",&DumDouble);
       
       if ( !SetFreeStream_ ) {
          
          NumberOfMachs_ = 1;
       
          MachList_[1] = Mach_ = DumDouble;  printf("Mach: %lf \n",MachList_[NumberOfMachs_]);
          
       }
       
       else {
          
          printf("Mach list overridden by command line -fs inputs \n");
          
       }
       
    }
    
    else {
       
       Next = strtok(DumChar,Comma);

       sscanf(DumChar,"Mach = %lf \n",&DumDouble);
       
       if ( !SetFreeStream_ ) {
          
          NumberOfMachs_ = 1;
       
          MachList_[1] = Mach_ = DumDouble;  printf("Mach: %lf \n",MachList_[NumberOfMachs_]);
          
       }
       
       else {
          
          printf("Mach list overridden by command line -fs inputs \n");
          
       }          
       
       while ( Next != NULL ) {
          
           Next = strtok(NULL,Comma);
           
           if ( Next != NULL ) {
              
              DumDouble = atof(Next);
              
              if ( !SetFreeStream_ ) {
                 
                 MachList_[++NumberOfMachs_] = DumDouble ;  printf("Mach: %lf \n",MachList_[NumberOfMachs_]);
             
              }
             
              else {
                
                 printf("Mach list overridden by command line -fs inputs \n");
                
              }   
                    
           }
           
       }
       
       Mach_ = MachList_[1];
       
    }
    
    // Load in AoA list
    
    fgets(DumChar,2000,case_file);

    if ( strstr(DumChar,Comma) == NULL ) {

       sscanf(DumChar,"AoA = %lf \n",&DumDouble);
       
       if ( !SetFreeStream_ ) {
          
          NumberOfAoAs_ = 1;
       
          AoAList_[1] = AoA_ = DumDouble;  printf("AoA: %lf \n",AoAList_[NumberOfAoAs_]);

       }
       
       else {
          
          printf("Alpha list overridden by command line -fs inputs \n");
          
       }   
              
    }
    
    else {

       Next = strtok(DumChar,Comma);
       
       sscanf(DumChar,"AoA = %lf \n",&DumDouble);
       
       if ( !SetFreeStream_ ) {
          
          NumberOfAoAs_ = 1;
       
          AoAList_[1] = AoA_ = DumDouble;  printf("AoA: %lf \n",AoAList_[NumberOfAoAs_]);

       }
       
       else {
          
          printf("Alpha list overridden by command line -fs inputs \n");
          
       }   
              
       while ( Next != NULL ) {
          
           Next = strtok(NULL,Comma);
           
           if ( Next != NULL ) {
              
              DumDouble = atof(Next);
              
              if ( !SetFreeStream_ ) {              
              
                 AoAList_[++NumberOfAoAs_] = DumDouble;
              
              }
             
              else {
                
                 printf("Alpha list overridden by command line -fs inputs \n");
                
              }   
                            
           }
           
       }
       
       AoA_ = AoAList_[1];
       
    }    
    
    // Load in Beta list
    
    fgets(DumChar,2000,case_file);

    if ( strstr(DumChar,Comma) == NULL ) {

       sscanf(DumChar,"Beta = %lf \n",&DumDouble);
       
       if ( !SetFreeStream_ ) {       
          
          NumberOfBetas_ = 1;
       
          BetaList_[1] = DumDouble;
          
       }

       else {
          
          printf("Beta list overridden by command line -fs inputs \n");
          
       }   
              
    }
    
    else {

       Next = strtok(DumChar,Comma);
       
       sscanf(DumChar,"Beta = %lf \n",&DumDouble);
       
       if ( !SetFreeStream_ ) {
          
          NumberOfBetas_ = 1;
                 
          BetaList_[1] = Beta_ = DumDouble;  printf("Beta: %lf \n",BetaList_[NumberOfBetas_]);
          
       }
          
       else {
          
          printf("Beta list overridden by command line -fs inputs \n");
          
       }             
       
       while ( Next != NULL ) {
          
           Next = strtok(NULL,Comma);
           
           if ( Next != NULL ) {
              
              DumDouble = atof(Next);
              
              if ( !SetFreeStream_ ) {                       
              
                 BetaList_[++NumberOfBetas_] = DumDouble;
                 
              }
              
              else {
                
                 printf("Beta list overridden by command line -fs inputs \n");
                
              }                 
              
           }
           
       }
       
       Beta_ = BetaList_[1];
       
    }    

    fscanf(case_file,"Vinf = %lf \n",&Vinf_);
    
    // Look for Vref
    
    fgets(DumChar,2000,case_file);
    
    if ( strstr(DumChar,"Vref") != NULL ) {
       
       sscanf(DumChar,"Vref = %lf \n",&Vref_);

       // Look for Machref
       
       fgets(DumChar,2000,case_file);
       
       if ( strstr(DumChar,"Machref") != NULL ) {
          
          sscanf(DumChar,"Machref = %lf \n",&Machref_);
          
          fscanf(case_file,"Rho = %lf \n",&Rho_);
          
       }
       
       else {
          
          printf("Machref must be defined if Vref is defined \n");
          fflush(NULL);
          exit(1);
          
       }
    
    }
    
    else {
       
       Vref_ = Vinf_;
       
       Machref_ = -1.;
       
       sscanf(DumChar,"Rho = %lf \n",&Rho_);
       
    }

    // Load in ReCref list
    
    fgets(DumChar,2000,case_file);

    if ( strstr(DumChar,Comma) == NULL ) {

       sscanf(DumChar,"ReCref = %lf \n",&DumDouble);

       NumberOfReCrefs_ = 1;
    
       ReCrefList_[1] = ReCref_ = DumDouble;  printf("ReCref_: %lf \n",ReCrefList_[NumberOfReCrefs_]);
           
    }
    
    else {

       Next = strtok(DumChar,Comma);
       
       sscanf(DumChar,"ReCref = %lf \n",&DumDouble);
    
       NumberOfReCrefs_ = 1;
    
       ReCrefList_[1] = ReCref_ = DumDouble;  printf("ReCref_: %lf \n",ReCrefList_[NumberOfReCrefs_]);
              
       while ( Next != NULL ) {
          
           Next = strtok(NULL,Comma);
           
           if ( Next != NULL ) {
              
              DumDouble = atof(Next);
    
              ReCrefList_[++NumberOfReCrefs_] = DumDouble;  printf("ReCref_: %lf \n",ReCrefList_[NumberOfReCrefs_]);
       
           }
           
       }
       
       ReCref_ = ReCrefList_[1];
       
    }      
    
    fscanf(case_file,"ClMax = %lf \n",&ClMax_);

    // Look for Clo2D
    
    fgets(DumChar,2000,case_file);
    
    if ( strstr(DumChar,"Clo2D") != NULL ) {    
       
       sscanf(DumChar,"Clo2D = %lf \n",&Clo2D_);
       fscanf(case_file,"MaxTurningAngle = %lf \n",&MaxTurningAngle_);
       
    }
    
    else {          
          
       fscanf(case_file,"MaxTurningAngle = %lf \n",&MaxTurningAngle_);
       
    }
    
    fscanf(case_file,"Symmetry = %s \n",SymmetryFlag);
    fscanf(case_file,"FarDist = %lf \n",&FarDist_);
    fscanf(case_file,"NumWakeNodes = %d \n",&NumberOfWakeNodes_);
    fscanf(case_file,"WakeIters = %d \n",&WakeIterations_);
    
    if ( FarDist_ > 0. ) SetFarDist_ = 1;
    
    if ( WakeIterations_ != 0 && WakeIterations_ <= 3 ) WakeIterations_ = 3;
    
    if ( WakeIterations_ == 0 && DoUnsteadyAnalysis_ == 0 ) {
       
       WakeIterations_ = 1;
       
       VSP.GMRESTightConvergence() = 1;
       
    }
        
    if ( MaxTurningAngle_ <= 0. ) MaxTurningAngle_ = -1.;

    printf("Sref            = %lf \n",Sref_);
    printf("Cref            = %lf \n",Cref_);
    printf("Bref            = %lf \n",Bref_);
    printf("X_cg            = %lf \n",Xcg_);
    printf("Y_cg            = %lf \n",Ycg_);
    printf("Z_cg            = %lf \n",Zcg_);
    printf("Mach            = "); { for ( i = 1 ; i < NumberOfMachs_ ; i++ ) { printf("%f, ",MachList_[i]); }; printf("%f \n",MachList_[NumberOfMachs_]); };
    printf("AoA             = "); { for ( i = 1 ; i < NumberOfAoAs_  ; i++ ) { printf("%f, ", AoAList_[i]); }; printf("%f \n", AoAList_[NumberOfAoAs_ ]); };
    printf("Beta            = "); { for ( i = 1 ; i < NumberOfBetas_ ; i++ ) { printf("%f, ",BetaList_[i]); }; printf("%f \n",BetaList_[NumberOfBetas_]); };
    printf("Vinf            = %lf \n",Vinf_);
    printf("Vref            = %lf \n",Vref_);
    printf("Rho             = %lf \n",Rho_);
    printf("ReCref          = "); { for ( i = 1 ; i < NumberOfReCrefs_ ; i++ ) { printf("%f, ",ReCrefList_[i]); }; printf("%f \n",ReCrefList_[NumberOfReCrefs_]); };
    
    printf("ClMax           = %lf \n",ClMax_);
    printf("Clo2D           = %lf \n",Clo2D_);    
    printf("MaxTurningAngle = %lf \n",MaxTurningAngle_);
    printf("Symmetry        = %s  \n",SymmetryFlag);
    printf("FarDist         = %lf \n",FarDist_);
    printf("NumWakeNodes    = %d  \n",NumberOfWakeNodes_);
    printf("WakeIters       = %d  \n",WakeIterations_);

    VSP.Sref() = Sref_;
    VSP.Cref() = Cref_;
    VSP.Bref() = Bref_;

    VSP.Xcg() = Xcg_;
    VSP.Ycg() = Ycg_;
    VSP.Zcg() = Zcg_;
    
    VSP.Mach() = Mach_;
    VSP.AngleOfAttack() = AoA_ * TORAD;
    VSP.AngleOfBeta() = Beta_ * TORAD;
    
    VSP.Vinf() = Vinf_;
    
    VSP.Vref() = Vref_;
    
    VSP.Machref() = Machref_;
    
    VSP.Density() = Rho_;
    
    VSP.ReCref() = ReCref_;
    
    VSP.Clo2D() = Clo2D_;
    
    VSP.ClMax() = ClMax_;
    
    VSP.MaxTurningAngle() = MaxTurningAngle_;
    
    VSP.WakeIterations() = WakeIterations_;
    
    VSP.RotationalRate_p() = 0.0;
    VSP.RotationalRate_q() = 0.0;
    VSP.RotationalRate_r() = 0.0;    
    
    // Symmetry options
    
    if ( strcmp(SymmetryFlag,"X") == 0 ) DoSymmetry_ = SYM_X;
    if ( strcmp(SymmetryFlag,"Y") == 0 ) DoSymmetry_ = SYM_Y;
    if ( strcmp(SymmetryFlag,"Z") == 0 ) DoSymmetry_ = SYM_Z;
        
    if ( DoSymmetry_ == SYM_X ) VSP.DoSymmetryPlaneSolve(SYM_X);
    if ( DoSymmetry_ == SYM_Y ) VSP.DoSymmetryPlaneSolve(SYM_Y);
    if ( DoSymmetry_ == SYM_Z ) VSP.DoSymmetryPlaneSolve(SYM_Z);
    
    // Look for wake relaxation factor
    
    rewind(case_file);
    
    while ( fgets(DumChar,2000,case_file) != NULL ) {

      if ( strstr(DumChar,"WakeRelax") != NULL ) {
         
         sscanf(DumChar,"WakeRelax = %lf \n",&WakeRelax_);
         
         printf("Setting wake relaxation factor to: %f \n",WakeRelax_);
         
         VSP.WakeRelax() = WakeRelax_;
         
      }
      
    }
        
    // Look for GMRES residual scale factor
    
    rewind(case_file);
    
    while ( fgets(DumChar,2000,case_file) != NULL ) {

      if ( strstr(DumChar,"GMRESReductionFactor") != NULL ) {
         
         sscanf(DumChar,"GMRESReductionFactor = %lf \n",&GMRESReductionFactor_);
         
         printf("Setting GMRES reduction factor to: %f \n",GMRESReductionFactor_);
         
         VSP.User_GMRES_ToleranceFactor() = GMRESReductionFactor_;
         
      }
      
    }    

    // Look for coresize factor
    
    rewind(case_file);
    
    while ( fgets(DumChar,2000,case_file) != NULL ) {

      if ( strstr(DumChar,"CoreSizeFactor") != NULL ) {
         
         sscanf(DumChar,"CoreSizeFactor = %lf \n",&CoreSizeFactor_);
         
         printf("Setting CoreSizeFactor to: %f \n",CoreSizeFactor_);
         
         VSP.CoreSizeFactor() = CoreSizeFactor_;
         
      }
      
    } 
    
    // Load in the control surface data
    
    rewind(case_file);
    
    NumberOfControlGroups_ = 0;

    while ( fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfControlGroups") != NULL ) {
          
          sscanf(DumChar,"NumberOfControlGroups = %d \n",&NumberOfControlGroups_);

          printf( "NumberOfControlGroups = %d \n", NumberOfControlGroups_ );

          if (NumberOfControlGroups_ < 0) {

              printf( "INVALID NumberOfControlGroups: %d\n", NumberOfControlGroups_ );

              exit( 1 );

          }

          ControlSurfaceGroup_ = new VSPAERO_SOLVER::CONTROL_SURFACE_GROUP[NumberOfControlGroups_ + 1];
          
          for ( i = 1 ; i <= NumberOfControlGroups_ ; i++ ) {
             
             // Control groupname
             
             fgets( ControlSurfaceGroup_[i].Name(), 200, case_file );
             
             ControlSurfaceGroup_[i].Name()[strcspn(ControlSurfaceGroup_[i].Name(), "\n")] = 0;
             
             printf("ControlSurfaceGroup_[%d].Name(): %s \n",i,ControlSurfaceGroup_[i].Name());
             
             // List of control surfaces in this group
             
             fgets(DumChar,300,case_file);
        
             // Determine the number of control surfaces in the group
         
             if ( strstr(DumChar,Comma) == NULL ) {
         
                NumberOfControlSurfaces = 1;
                
                ControlSurfaceGroup_[i].SizeList(NumberOfControlSurfaces);

                printf( "There is %d control surface in group: %d \n", NumberOfControlSurfaces, i );
                
                DumChar[strcspn(DumChar, "\n")] = 0;
                
                sprintf(ControlSurfaceGroup_[i].ControlSurface_Name(1),"%s",DumChar);
                
                printf("Control Surface(1): %s___ \n",ControlSurfaceGroup_[i].ControlSurface_Name(1));
                
                // Read in the control surface direction
                
                fgets(DumChar,2000,case_file);
                
                ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(1) = atof(DumChar);
                                                
                printf("Control surface(%d) direction: %f \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(1));
   
                //// Read in the control surface deflection
                //
                //fgets(DumChar,2000,case_file);
                //
                //ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle() = atof(DumChar);
                //
                //printf("Control surface(%d) deflection %f \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle());
                // 
             }
             
             else {
                
                // Save a copy of Dumchar
                
                sprintf(DumChar2,"%s",DumChar);
            
                // Figure out how many control surfaces are in the list
                
                Next = strtok(DumChar,Comma);
         
                NumberOfControlSurfaces = 1;
   
                while ( Next != NULL ) {
                   
                    Next = strtok(NULL,Comma);
                    
                    if ( Next != NULL ) {
                       
                       NumberOfControlSurfaces++;
                       
                    }
                    
                }
                
                // Size the control surface list
                
                ControlSurfaceGroup_[i].SizeList(NumberOfControlSurfaces);
                
                printf("There are %d control surfaces in group: %d \n",NumberOfControlSurfaces,i);
         
                // Reparse the list to get the actual control surface names
                
                sprintf(DumChar,"%s",DumChar2);
                
                Next = strtok(DumChar,Comma);
  
                NumberOfControlSurfaces = 1;
                
                sprintf(ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces),"%s",Next);
                
                ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(NumberOfControlSurfaces) = 1;
                
                printf("Control surface(%d): %s___ \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces));
   
                while ( Next != NULL ) {
                   
                    Next = strtok(NULL,",\n");
         
                    if ( Next != NULL ) {
                       
                       NumberOfControlSurfaces++;

                       sprintf(ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces),"%s", Next );
                
                       printf("Control surface(%d): %s \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_Name(NumberOfControlSurfaces));
   
                    }
                    
                }
                
                // Now read in the control surface directions
                
                fgets(DumChar,2000,case_file);
                
                Next = strtok(DumChar,Comma);
  
                NumberOfControlSurfaces = 1;
        
                ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(NumberOfControlSurfaces) = atof(Next);
                
                printf("Control surface(%d) direction: %f \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(NumberOfControlSurfaces));
   
                while ( Next != NULL ) {
                   
                    Next = strtok(NULL,Comma);
         
                    if ( Next != NULL ) {
                       
                       NumberOfControlSurfaces++;
                       
                       ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(NumberOfControlSurfaces) = atof(Next);
                
                       printf("Control surface(%d) direction %f \n",NumberOfControlSurfaces,ControlSurfaceGroup_[i].ControlSurface_DeflectionDirection(NumberOfControlSurfaces));
   
                    }
                    
                } 

             }

             // Finally, read in the control group deflection

             fscanf( case_file, "%lf\n", &(ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle()) );

             printf( "Control group deflection angle: %f \n", ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle() );

          }
          
       }
    
    }
          
    // Load in the rotor data
    
    rewind(case_file);
    
    NumberOfRotors_ = 0;
    
    Done = 0;
    
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfRotors") != NULL ) {
          
          sscanf(DumChar,"NumberOfRotors = %d \n",&NumberOfRotors_);
          
          printf("NumberOfRotors: %d \n",NumberOfRotors_);
          
          VSP.SetNumberOfRotors(NumberOfRotors_);
          
          for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
           
             fgets(DumChar,2000,case_file);
             fgets(DumChar,2000,case_file);
             
             printf("\nLoading data for rotor: %5d \n",i);
             
             VSP.RotorDisk(i).Load_STP_Data(case_file);
             
          }
          
          Done = 1;
          
       }
       
    }
    
    // Adjust rotors for ground effects calculation

    if ( DoGroundEffectsAnalysis_ ) {
           
       RotAngle = AoAList_[1] * TORAD;
       
       DeltaHeight = HeightAboveGround_ - Zcg_;
       
       HingeVec[0] = 0.;
       HingeVec[1] = 1.;
       HingeVec[2] = 0.;
           
       Quat.FormRotationQuat(HingeVec,RotAngle);
   
       InvQuat = Quat;
      
       InvQuat.FormInverse();   

       for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
          
          // Rotate rotors and shift in z
          
          x = DOUBLE(VSP.RotorDisk(i).XYZ(0));
          y = DOUBLE(VSP.RotorDisk(i).XYZ(1));
          z = DOUBLE(VSP.RotorDisk(i).XYZ(2));

          VSP.RotorDisk(i).XYZ(0) = (x - Xcg_)*cos(RotAngle) - (z - Zcg_)*sin(RotAngle) + Xcg_;
          VSP.RotorDisk(i).XYZ(1) = y;
          VSP.RotorDisk(i).XYZ(2) = (x - Xcg_)*sin(RotAngle) + (z - Zcg_)*cos(RotAngle) + Zcg_ + DeltaHeight;    
     
          // Rotate rotors to align with free stream
          
          Vec(0) = DOUBLE(VSP.RotorDisk(i).Normal(0));
          Vec(1) = DOUBLE(VSP.RotorDisk(i).Normal(1));
          Vec(2) = DOUBLE(VSP.RotorDisk(i).Normal(2));
       
          Vec = Quat * Vec * InvQuat;
       
          VSP.RotorDisk(i).Normal(0) = Vec(0);
          VSP.RotorDisk(i).Normal(1) = Vec(1); 
          VSP.RotorDisk(i).Normal(2) = Vec(2); 
          
       }
                    
    }
        
    // Load in the velocity survey data
    
    rewind(case_file);
    
    NumberofSurveyPoints_ = 0;
    
    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberofSurveyPoints") != NULL ) {

          sscanf(DumChar,"NumberofSurveyPoints = %d \n",&NumberofSurveyPoints_);
          
          printf("NumberofSurveyPoints: %d \n",NumberofSurveyPoints_);
          
          VSP.SetNumberOfSurveyPoints(NumberofSurveyPoints_);
          
          for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {
             
             fscanf(case_file,"%d %lf %lf %lf \n",&j,&x,&y,&z);
             
             printf("Survey Point: %10d: %10.5f %10.5f %10.5f \n",i,x,y,z);
             
             VSP.SurveyPointList(i).x() = x;
             VSP.SurveyPointList(i).y() = y;
             VSP.SurveyPointList(i).z() = z;
             
          }
       
       }
       
    }
    
    // Load in the quad tree survey planes
    
    rewind(case_file);
    
    NumberOfQuadTrees_ = 0;
    
    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfQuadTrees") != NULL ) {

          sscanf(DumChar,"NumberOfQuadTrees = %d \n",&NumberOfQuadTrees_);
          
          printf("NumberOfQuadTrees: %d \n",NumberOfQuadTrees_);
          
          VSP.SetNumberOfQuadTrees(NumberOfQuadTrees_);
          
          for ( i = 1 ; i <= NumberOfQuadTrees_ ; i++ ) {
             
             fscanf(case_file,"%d %d %lf \n",&j,&Dir,&Value);
             
             printf("QuadTree(%d): %10d: %10.5f \n",j,Dir,Value);
             
             VSP.QuadTreeDirection(i) = Dir;
             VSP.QuadTreeValue(i) = Value;
             
          }
       
       }
       
    }

    // Determine the number of inlets and nozzles
    
    rewind(case_file);
    
    NumberOfInlets_ = NumberOfNozzles_ = 0;

    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfInlets") != NULL ) {

          sscanf(DumChar,"NumberOfInlets = %d \n",&NumberOfInlets_);
          
          printf("NumberOfInlets: %d \n",NumberOfInlets_);
	  
       }
       
    }
    
    rewind(case_file);
	  
    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfNozzles") != NULL ) {

          sscanf(DumChar,"NumberOfNozzles = %d \n",&NumberOfNozzles_);
          
          printf("NumberOfNozzles: %d \n",NumberOfNozzles_);
	  
       }
       
    }
    
    VSP.SetNumberOfEngineFaces(NumberOfInlets_ + NumberOfNozzles_);
    
    // Read in inlet data
    
    rewind(case_file);
    	      
    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfInlets") != NULL ) {

          sscanf(DumChar,"NumberOfInlets = %d \n",&NumberOfInlets_);
          
          printf("NumberOfInlets: %d \n",NumberOfInlets_);
          
          for ( i = 1 ; i <= NumberOfInlets_ ; i++ ) {
             
             fscanf(case_file,"%d %d %lf \n",&j,&Surface,&MassFlow);

             printf("EngineFace(%d) with surface %10d and massflow of %10.5f is an inlet \n",j,Surface,-MassFlow);
                 
             VSP.EngineFace(i).SurfaceType() = INLET_FACE;
             VSP.EngineFace(i).SurfaceID() = Surface;
             VSP.EngineFace(i).MassFlow() = MassFlow;
             
          }
       
       }
       
    }
      
    // Read in nozzle data
    
    rewind(case_file);
    	      
    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfNozzles") != NULL ) {

          sscanf(DumChar,"NumberOfNozzles = %d \n",&NumberOfInlets_);
          
          printf("NumberOfNozzles: %d \n",NumberOfNozzles_);
          
          for ( i = 1 ; i <= NumberOfNozzles_ ; i++ ) {
	     
	          k = NumberOfInlets_ + i;
             
             fscanf(case_file,"%d %d %lf %lf \n",&j,&Surface,&Velocity,&DeltaCp);

	          fscanf(case_file,"%lf %lf %lf \n",&(VSP.EngineFace(k).NozzleXYZ(0)),&(VSP.EngineFace(k).NozzleXYZ(1)),&(VSP.EngineFace(k).NozzleXYZ(2)));
	          fscanf(case_file,"%lf %lf %lf \n",&(VSP.EngineFace(k).NozzleNormal(0)),&(VSP.EngineFace(k).NozzleNormal(1)),&(VSP.EngineFace(k).NozzleNormal(2)));	     
	          fscanf(case_file,"%lf \n",&(VSP.EngineFace(i).NozzleRadius()));
                      
             printf("EngineFace(%d) with surface %10d and velocity of %10.5f is a nozzle \n",j,Surface,Velocity);
	          printf("Nozzle xyz: %f %f %f \n",VSP.EngineFace(k).NozzleXYZ(0),VSP.EngineFace(k).NozzleXYZ(1),VSP.EngineFace(k).NozzleXYZ(2));
 	          printf("Nozzle normal: %f %f %f \n",VSP.EngineFace(k).NozzleNormal(0),VSP.EngineFace(k).NozzleNormal(1),VSP.EngineFace(k).NozzleNormal(2));
	          printf("Nozzle radius: %f \n",VSP.EngineFace(k).NozzleRadius());
         
             VSP.EngineFace(k).SurfaceType() = NOZZLE_FACE;                
             VSP.EngineFace(k).SurfaceID() = Surface;
             VSP.EngineFace(k).NozzleVelocity() = Velocity;
             VSP.EngineFace(k).NozzleDeltaCp() = DeltaCp;
             
          }
       
       }
       
    }
                
    // Look for number of output time steps for survey points
    
    rewind(case_file);
        
    Done = 0;
        
    while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {

       if ( strstr(DumChar,"NumberOfSurveyTimeSteps") != NULL ) {

          sscanf(DumChar,"NumberOfSurveyTimeSteps = %d \n",&NumberOfSurveyTimeSteps_);
          
          printf("NumberOfSurveyTimeSteps: %d \n",NumberOfSurveyTimeSteps_);
          
          VSP.NumberOfSurveyTimeSteps() = NumberOfSurveyTimeSteps_;
          
          Done = 1;

       }
       
    }
        
    // Load in unsteady aero data
    
    rewind(case_file);
    
    Done = 0;

    if ( VSP.TimeAnalysisType() == 0 ) {

       while ( !Done && fgets(DumChar,2000,case_file) != NULL ) {
   
          if ( strstr(DumChar,"TimeStep") != NULL ) {
   
             printf("Unsteady analysis data: \n");
                 
             sscanf(DumChar,"TimeStep = %lf \n",&TimeStep_);
             
             printf("TimeStep: %f \n",TimeStep_);
                                                  
             fgets(DumChar,2000,case_file);
             
             if ( strstr(DumChar, "NumberOfTimeSteps") != NULL) {
                
                 sscanf(DumChar,"NumberOfTimeSteps = %d \n",&NumberOfTimeSteps_);
                 
                 printf("NumberOfTimeSteps: %d \n",NumberOfTimeSteps_);
                 
                 Done = 1;
                 
             }
   
             VSP.TimeStep()          = TimeStep_;
             VSP.NumberOfTimeSteps() = NumberOfTimeSteps_;
   
          }
                  
       }
       
    }
        
    fclose(case_file);
    
}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER DOUBLE                                    #
#                                                                              #
##############################################################################*/

double VSP_OPTIMIZER::DOUBLE(double a)
{ 
 
 return (double) a; 
 
}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER DOUBLE                                    #
#                                                                              #
##############################################################################*/

double VSP_OPTIMIZER::DOUBLE(adept::adouble a)
{
 
 return (double) a.value();
 
}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER UpdateGeometry                            #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::UpdateGeometry(double *NodeXYZ)
{

    int i;
    
    for ( i = 1 ; i <= Adjoint().VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {
       
         Solver().VSPGeom().Grid(0).NodeList(i).x() = NodeXYZ[3*i-2 - ArrayOffSet_];
         Solver().VSPGeom().Grid(0).NodeList(i).y() = NodeXYZ[3*i-1 - ArrayOffSet_]; 
         Solver().VSPGeom().Grid(0).NodeList(i).z() = NodeXYZ[3*i   - ArrayOffSet_]; 

        Adjoint().VSPGeom().Grid(0).NodeList(i).x() = NodeXYZ[3*i-2 - ArrayOffSet_];
        Adjoint().VSPGeom().Grid(0).NodeList(i).y() = NodeXYZ[3*i-1 - ArrayOffSet_]; 
        Adjoint().VSPGeom().Grid(0).NodeList(i).z() = NodeXYZ[3*i   - ArrayOffSet_]; 
                
    }
    
    Solver().VSPGeom().UpdateMeshes();

    Adjoint().VSPGeom().UpdateMeshes();
      
}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER GetForwardSolutionVector                  #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetForwardSolutionVector(double *Vec)
{

    int i;
 
    for ( i = 1 ; i <= Solver().NumberOfVortexLoops() ; i++ ) {
       
       Vec[i - ArrayOffSet_] = Solver().ForwardSolutionVector(i);
       
    }
    
}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER GetAdjointSolutionVector                  #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetAdjointSolutionVector(double *Vec)
{

    int i;
 
    for ( i = 1 ; i <= Adjoint().NumberOfAdjointEquations() ; i++ ) {
       
       Vec[i - ArrayOffSet_] = Adjoint().AdjointSolutionVector(i);
       
    }
 
}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER GetFunctionValue                          #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetFunctionValue(double &Vec)
{

    Solver().OptimizationFunction(Vec);
      
}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER GetFunctionValue                          #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetFunctionValue(int Case, double &Vec)
{

    Solver().OptimizationFunction(Case, Vec);
      
}

/*##############################################################################
#                                                                              #
#                 VSP_OPTIMIZER GetUnsteadyFunctionValue                       #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetUnsteadyFunctionValue(int Case, int Time, double *Vec)
{
   
    Solver().OptimizationFunction(Case, Time, TempArray_[0]);

    ShiftOutputVector(TempArray_[0],Vec,Solver().OptimizationFunctionLength(1));
      
}

/*##############################################################################
#                                                                              #
#                 VSP_OPTIMIZER GetUnsteadyFunctionValue                       #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetUnsteadyFunctionValue(int Time, double *Vec)
{
   
    Solver().OptimizationFunction(1, Time, TempArray_[0]);
    
    ShiftOutputVector(TempArray_[0],Vec,Solver().OptimizationFunctionLength(1));
         
}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER GetFunctionValue                          #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetFunctionValue(double *Vec)
{
   
    Solver().OptimizationFunction(TempArray_[0]);
    
    ShiftOutputVector(TempArray_[0],Vec,Solver().OptimizationFunctionLength(1));
    
      
}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER GetFunctionValue                          #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetFunctionValue(int Case, double *Vec)
{

    Solver().OptimizationFunction(Case, TempArray_[0]);

    ShiftOutputVector(TempArray_[0],Vec,Solver().OptimizationFunctionLength(Case));
      
}
  
/*##############################################################################
#                                                                              #
#                  VSP_OPTIMIZER GetFunctionGradients                          #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetFunctionGradients(double *Gradients)
{

    int i;
    
    for ( i = 1 ; i <= Adjoint().VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {
       
        Gradients[3*i-2 - ArrayOffSet_] = Adjoint().dF_dMesh_X(i);
        Gradients[3*i-1 - ArrayOffSet_] = Adjoint().dF_dMesh_Y(i);
        Gradients[3*i   - ArrayOffSet_] = Adjoint().dF_dMesh_Z(i);
        
    }
      
}

/*##############################################################################
#                                                                              #
#                  VSP_OPTIMIZER GetFunctionGradients                          #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetFunctionGradients(int p, double *Gradients)
{

    int i;
    
    for ( i = 1 ; i <= Adjoint().VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {
       
        Gradients[3*i-2 - ArrayOffSet_] = Adjoint().dF_dMesh_X(p,i);
        Gradients[3*i-1 - ArrayOffSet_] = Adjoint().dF_dMesh_Y(p,i);
        Gradients[3*i   - ArrayOffSet_] = Adjoint().dF_dMesh_Z(p,i);
        
    }
      
}

/*##############################################################################
#                                                                              #
#                  VSP_OPTIMIZER GetFunctionGradients                          #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetFunctionGradients(int p, int Time, double *Gradients)
{

    int i;
    
    for ( i = 1 ; i <= Adjoint().VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {
       
        Gradients[3*i-2 - ArrayOffSet_] = Adjoint().dF_dMesh_X(p,Time,i);
        Gradients[3*i-1 - ArrayOffSet_] = Adjoint().dF_dMesh_Y(p,Time,i);
        Gradients[3*i   - ArrayOffSet_] = Adjoint().dF_dMesh_Z(p,Time,i);
        
    }
      
}

/*##############################################################################
#                                                                              #
#              VSP_OPTIMIZER GetFuGetNodalPressuresnctionGradients             #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetNodalPressures(double *Pressure)
{

    int i;
    
    Solver().CalculateNodalPressures();
    
    for ( i = 1 ; i <= Solver().VSPGeom().Grid(0).NumberOfNodes() ; i++ ) {
       
        Pressure[i - ArrayOffSet_] = Solver().NodalCp(i);

    }
      
}

/*##############################################################################
#                                                                              #
#              VSP_OPTIMIZER GetSpanWiseChordAndAreaForWing                    #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetSpanWiseChordAndAreaForWing(int Wing, double *Chord, double *Area)
{

    int i;

    for ( i = 1 ; i <= Solver().NumberOfSpanStationsForWing(Wing) ; i++ ) {
       
        Chord[i - ArrayOffSet_] = Solver().SpanWiseChordForWing(Wing,i);

         Area[i - ArrayOffSet_] = Solver().SpanWiseAreaForWing(Wing,i);

    }
      
}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER GetpFupMesh                               #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::GetpFupMesh(double *pFu_pP, double *pFu_pMesh)
{

    Adjoint().Optimization_Calculate_pFu_pMesh(pFu_pP,pFu_pMesh );
          
}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER SupressStdout                             #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::SupressStdout(void) {
    fflush(stdout);

    stdoutfd = dup(1);
    int nullfd = open("/dev/null", O_WRONLY);
    // check nullfd for error omitted
    dup2(nullfd, 1);
    close(nullfd);
}

/*##############################################################################
#                                                                              #
#                      VSP_OPTIMIZER ResumeStdout                              #
#                                                                              #
##############################################################################*/

void VSP_OPTIMIZER::ResumeStdout(void) {
    fflush(stdout);
    dup2(stdoutfd, 1);
    close(stdoutfd);
}

















