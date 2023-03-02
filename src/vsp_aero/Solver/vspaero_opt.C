//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#ifndef WIN32
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "VSP_Optimizer.H"

// The code...

int main(int argc, char **argv);

int TestCase_1(char *FileName); // Single, SCALAR, optimization function 

int TestCase_2(char *FileName); // 3 SCALAR optimization functions, 1 forward solve, 3 adjoint solves

int TestCase_3(char *FileName); // Single VECTOR optimization function, with user supplied intitial gradient 

int TestCase_4(char *FileName); // UNSTEADY analysis... with a single scalar optimization function, with user supplied intitial gradient 

int TestCase_5(char *FileName); // Single VECTOR optimization function, with user supplied intitial gradient...
                                // followed by a matrix-vector multiply, and an adjoint matrix-vector multiply

int TestCase_6(char *FileName); // UNSTEADY analysis... with 3 VECTOR optimization functions, with user supplied intitial gradient 

int TestCase_7(char *FileName); // Dead stupid optimization of a wing...

int TestCase_8(char *FileName); // Dead stupid UNSTEADY optimization of a wing... with 2 rotors... 

int TestCase_9(char *FileName); // Unsteady rotor CT and gradients with respect to feather

int TestCase_10(char *FileName);

int TestCase_11(char *FileName);

double *ReadOpenVSPDesFile(char *FileName, int &NumberOfDesignVariables);
void CreateVSPGeometry(char *FileName, int NumberOfDesignVariables, double *ParameterValues);
double **CalculateOpenVSPGeometryGradients(char *FileName, int NumberOfDesignVariables, double *ParameterValues);
void DeleteOpenVSPGeometryGradients(double **dMesh_dParameter, int NumberOfDesignVariables);
double *ReadVSPGeomFile(char *FileName, int &NumberOfMeshNodes);
double Normalize(double *Vector, int Length);
void CGState(double *Old, double *New, int Length);
void CG_Optimizer(char *FileName, int NumberOfParameterValues, double *ParameterValues, double *Gradient, VSP_OPTIMIZER &Optimizer);

/*##############################################################################
#                                                                              #
#                                 main                                         #
#                                                                              #
##############################################################################*/

int main(int argc, char **argv)
{
 
    int TestCase;   
    char *FileName;
    
    // Grab the test case to run
    
    if ( argc < 3 ) {
       
       printf("Error... please specify the test case to run... \n");
       fflush(NULL);
       exit(1);
       
    }
    
    else {
       
       TestCase = atoi(argv[argc-2]);
       
       FileName = argv[argc-1];
       
    }

    printf("FileName: %s \n",FileName);fflush(NULL);
    
    // Single, SCALAR, optimization function 
    
    if ( TestCase == 1 ) {
       
       return TestCase_1(FileName);
       
    }
    
    // 3 SCALAR optimization functions, 1 forward solve, 3 adjoint solves
    
    else if ( TestCase == 2 ) {
       
       return TestCase_2(FileName);
       
    }
    
    // Single VECTOR optimization function, with user supplied intitial gradient 
    
    else if ( TestCase == 3 ) {
       
       return TestCase_3(FileName);   
       
    }  

    // UNSTEADY analysis... with a single scalar optimization function, with user supplied intitial gradient 
       
    else if ( TestCase == 4 ) {
       
       return TestCase_4(FileName);    
       
    }

    // Single VECTOR optimization function, with user supplied intitial gradient...
    // followed by a matrix-vector multiply, and an adjoint matrix-vector multiply

    else if ( TestCase == 5 ) {
       
       return TestCase_5(FileName);    
       
    }
    
    // UNSTEADY analysis... with 3 VECTOR optimization functions, with user supplied intitial gradient 

    else if ( TestCase == 6 ) {
       
       return TestCase_6(FileName);    
       
    }

    // Dead stupid optimization of a wing...

    else if ( TestCase == 7 ) {
       
       return TestCase_7(FileName);    
       
    }   

    // Dead stupid UNSTEADY optimization of a wing... with 2 rotors... 

    else if ( TestCase == 8 ) {
       
       return TestCase_8(FileName);    
       
    }   

    // Rotor CT gradients wrt feather...

    else if ( TestCase == 9 ) {
       
       return TestCase_9(FileName);    
       
    }   

    // Panel fuselage solve test

    else if ( TestCase == 10 ) {
       
       return TestCase_10(FileName);    
       
    }   
      
    // Matrix vector products, residuals, and right hand side test

    else if ( TestCase == 11 ) {
       
       return TestCase_11(FileName);    
       
    }   
                             
    else {
       
       printf("Unknown test case! \n");
       fflush(NULL);
       exit(1);
       
    }
    
    return 1;     
           
}

/*##############################################################################
#                                                                              #
#                            TestCase_1                                        #
#                                                                              #
#        Single, SCALAR, optimization function                                 #
#                                                                              #
##############################################################################*/

int TestCase_1(char *FileName)
{
    int i, p;
    double Function;
    char GradientFileName[2000];
    FILE *GRADFile;
    
    printf("Running test case #1... \n");fflush(NULL);
       
    VSP_OPTIMIZER Optimizer;

    Optimizer.NumberOfThreads() = 1;
    
    Optimizer.DoUnsteadyAnalysis() = 0;
    
    Optimizer.NumberOfOptimizationFunctions() = 1;
    
    Optimizer.OptimizationFunction(1) = OPT_CL_INVISCID;

    Optimizer.SetArrayOffSetType(DEFAULT_ARRAYS);

    Optimizer.Setup(FileName);

    Optimizer.SetMachNumber(0.11);
       
    Optimizer.SetAoADegrees(5.);
       
    Optimizer.SetBetaDegrees(0.);
       
    Optimizer.SetVinf(100.);
       
    Optimizer.SetDensity(1.e-5);
       
    Optimizer.SetReCref(1000000.);
       
    Optimizer.SetRotationalRate_p(0.);
       
    Optimizer.SetRotationalRate_q(0.);
       
    Optimizer.SetRotationalRate_r(0.);

    Optimizer.Solve();
   
    // Open the gradient file

    sprintf(GradientFileName,"%s.opt.gradient",FileName);

    if ( (GRADFile = fopen(GradientFileName, "w")) == NULL ) {
    
       printf("Could not open the gradient output file! \n");
    
       exit(1);
    
    }
           
    // Output the gradient data

    fprintf(GRADFile,"%d \n",Optimizer.NumberOfNodes());fflush(NULL);
    
    for ( p = 1 ; p <= Optimizer.NumberOfOptimizationFunctions() ; p++ ) {
       
       Optimizer.GetFunctionValue(p,Function);

       fprintf(GRADFile,"Function: %d value: %f \n",p,Function);fflush(NULL);
   
                       //1234567890    1234567890 1234567890 1234567890    1234567890 1234567890 1234567890    
       
       fprintf(GRADFile,"   Node            X          Y          Z            dFdX       dFdY       dFdZ \n");
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          fprintf(GRADFile,"%10d   %10.6e %10.6e %10.6e   %10.6e %10.6e %10.6e \n",
           i,
           Optimizer.NodeX(i),
           Optimizer.NodeY(i),
           Optimizer.NodeZ(i),
           Optimizer.GradientX(p,i),
           Optimizer.GradientY(p,i),
           Optimizer.GradientZ(p,i));
           
           fflush(NULL);
       }
       
       fprintf(GRADFile," \n\n\n");
       
    }     
     
    fclose(GRADFile);
    
    // Gradient with respect to input values 
    
    printf("\n\n\n\n");
    printf("Gradient with respect to Alpha   : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_ALPHA  ));
    printf("Gradient with respect to Beta    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_BETA   ));
    printf("Gradient with respect to Mach    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_MACH   ));
    printf("Gradient with respect to Vinf    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_VINF   ));
    printf("Gradient with respect to Density : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_DENSITY));
    printf("Gradient with respect to ReCref  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_RECREF ));
    printf("Gradient with respect to P rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_P_RATE ));
    printf("Gradient with respect to Q rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_Q_RATE ));
    printf("Gradient with respect to R rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_R_RATE ));
  
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                            TestCase_2                                        #
#                                                                              #
#  3 SCALAR optimization functions, 1 forward solve, 3 adjoint solves          #
#                                                                              #
##############################################################################*/

int TestCase_2(char *FileName)
{
    int i, p;
    double Function;
    char GradientFileName[2000];
    FILE *GRADFile;
    
    printf("Running test case #2... \n");fflush(NULL);
       
    VSP_OPTIMIZER Optimizer;

    Optimizer.NumberOfThreads() = 4;
    
    Optimizer.DoUnsteadyAnalysis() = 0;
    
    Optimizer.NumberOfOptimizationFunctions() = 3;
    
    Optimizer.OptimizationFunction(1) = OPT_CL_TOTAL;
    Optimizer.OptimizationFunction(2) = OPT_CD_TOTAL;
    Optimizer.OptimizationFunction(3) = OPT_CMY_TOTAL;
    
    Optimizer.SetArrayOffSetType(DEFAULT_ARRAYS);
    
    Optimizer.Setup(FileName);

    Optimizer.Solve();
   
    // Open the gradient file

    sprintf(GradientFileName,"%s.opt.gradient",FileName);

    if ( (GRADFile = fopen(GradientFileName, "w")) == NULL ) {
    
       printf("Could not open the gradient output file! \n");
    
       exit(1);
    
    }
           
    // Output the gradient data

    fprintf(GRADFile,"%d \n",Optimizer.NumberOfNodes());fflush(NULL);
    
    for ( p = 1 ; p <= Optimizer.NumberOfOptimizationFunctions() ; p++ ) {
       
       Optimizer.GetFunctionValue(p,Function);

       fprintf(GRADFile,"Function: %d value: %f \n",p,Function);fflush(NULL);
   
                       //1234567890    1234567890 1234567890 1234567890    1234567890 1234567890 1234567890    
       
       fprintf(GRADFile,"   Node            X          Y          Z            dFdX       dFdY       dFdZ \n");
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          fprintf(GRADFile,"%10d   %10.6e %10.6e %10.6e   %10.6e %10.6e %10.6e \n",
           i,
           Optimizer.NodeX(i),
           Optimizer.NodeY(i),
           Optimizer.NodeZ(i),
           Optimizer.GradientX(p,i),
           Optimizer.GradientY(p,i),
           Optimizer.GradientZ(p,i));
           
           fflush(NULL);
       }
       
       fprintf(GRADFile," \n\n\n");
       
    }     
     
    fclose(GRADFile);

    // Gradient with respect to input values 
 
    printf("\n\n\n\n");
    printf("Gradient with respect to Alpha   : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_ALPHA  ));
    printf("Gradient with respect to Beta    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_BETA   ));
    printf("Gradient with respect to Mach    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_MACH   ));
    printf("Gradient with respect to Vinf    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_VINF   ));
    printf("Gradient with respect to Density : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_DENSITY));
    printf("Gradient with respect to ReCref  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_RECREF ));
    printf("Gradient with respect to P rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_P_RATE ));
    printf("Gradient with respect to Q rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_Q_RATE ));
    printf("Gradient with respect to R rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_R_RATE ));
        
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                            TestCase_3                                        #
#                                                                              #
#   Single VECTOR optimization function, with user supplied intitial gradient  #
#                                                                              #
##############################################################################*/

int TestCase_3(char *FileName)
{
    int i, p, Case, Wing;
    double *Function, *Vec;
    char GradientFileName[2000];
    FILE *GRADFile;
    
    printf("Running test case #3... \n");fflush(NULL);
       
    VSP_OPTIMIZER Optimizer;

    Optimizer.NumberOfThreads() = 4;
    
    Optimizer.DoUnsteadyAnalysis() = 0;
    
    Optimizer.NumberOfOptimizationFunctions() = Case = 1;
    
    Optimizer.OptimizationFunction(Case) = OPT_WING_CL_TOTAL; // xyz force coefficents vs span for wing 1
    
    Optimizer.OptimizationSet(Case) = Wing = 1; // We have to specify which wing this is

    Optimizer.SetArrayOffSetType(DEFAULT_ARRAYS);
    
    Optimizer.Setup(FileName);
    
    Optimizer.SolveForward();

    // User supplied input gradient vector
    
    printf("Function Length: %d \n",Optimizer.OptimizationFunctionLength(1));
        
    Case = 1; // We have only 1 optimization function, which is a vector... OPT_WING_CL_TOTAL ... of the wing loading
 
    Vec = new double[Optimizer.OptimizationFunctionLength(Case) + 1];
    
    Function = new double[Optimizer.OptimizationFunctionLength(Case) + 1];    
    
    for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(Case) ; i++ ) {
       
       Vec[i] = 1.;
       
    }

    Optimizer.SetGradientVector(Case, Vec);

    Optimizer.SolveAdjoint();
    
    // Open the gradient file

    sprintf(GradientFileName,"%s.opt.gradient",FileName);

    if ( (GRADFile = fopen(GradientFileName, "w")) == NULL ) {
    
       printf("Could not open the gradient output file! \n");
    
       exit(1);
    
    }
           
    // Output the gradient data

    fprintf(GRADFile,"%d \n",Optimizer.NumberOfNodes());fflush(NULL);
    
    for ( p = 1 ; p <= Optimizer.NumberOfOptimizationFunctions() ; p++ ) {
       
       Optimizer.GetFunctionValue(p,Function);
       
       fprintf(GRADFile,"Function: %d \n",p);fflush(NULL);

       for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(p) ; i++ ) {
          
          fprintf(GRADFile,"Function(%d): %f \n",p,Function[i]);fflush(NULL);
          
       }
   
                       //1234567890    1234567890 1234567890 1234567890    1234567890 1234567890 1234567890    
       
       fprintf(GRADFile,"   Node            X          Y          Z            dFdX       dFdY       dFdZ \n");
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          fprintf(GRADFile,"%10d   %10.6e %10.6e %10.6e   %10.6e %10.6e %10.6e \n",
           i,
           Optimizer.NodeX(i),
           Optimizer.NodeY(i),
           Optimizer.NodeZ(i),
           Optimizer.GradientX(p,i),
           Optimizer.GradientY(p,i),
           Optimizer.GradientZ(p,i));
           
           fflush(NULL);
       }
       
       fprintf(GRADFile," \n\n\n");
       
    }     
     
    fclose(GRADFile);

    // Gradient with respect to input values 
 
    printf("\n\n\n\n");
    printf("Gradient with respect to Alpha   : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_ALPHA  ));
    printf("Gradient with respect to Beta    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_BETA   ));
    printf("Gradient with respect to Mach    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_MACH   ));
    printf("Gradient with respect to Vinf    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_VINF   ));
    printf("Gradient with respect to Density : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_DENSITY));
    printf("Gradient with respect to ReCref  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_RECREF ));
    printf("Gradient with respect to P rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_P_RATE ));
    printf("Gradient with respect to Q rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_Q_RATE ));
    printf("Gradient with respect to R rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_R_RATE ));
        
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                            TestCase_4                                        #
#                                                                              #
#   UNSTEADY analysis... with a single scalar optimization function, with      #
#   user supplied intitial gradient                                            #
#                                                                              #
##############################################################################*/

int TestCase_4(char *FileName)
{
    int i, p, t, Case;
    double *Function, *Vec;
    char GradientFileName[2000];
    FILE *GRADFile;
    
    printf("Running test case #4... \n");fflush(NULL);
       
    VSP_OPTIMIZER Optimizer;

    Optimizer.NumberOfThreads() = 1;
    
    Optimizer.DoUnsteadyAnalysis() = 1;
    
    Optimizer.NumberOfOptimizationFunctions() = 1;

    Optimizer.OptimizationFunction(1) = OPT_ROTOR_CT_TOTAL;
        
    Optimizer.OptimizationSet(1) = 1;

    Optimizer.SetArrayOffSetType(DEFAULT_ARRAYS);
    
    Optimizer.Setup(FileName);
    
    Optimizer.SolveForward();
   
    // User supplied input gradient vector
    
    printf("Function Length: %d \n",Optimizer.OptimizationFunctionLength(1));
        
    Case = 1; // We have only 1 optimization function, which is a scalar... 
 
    Vec = new double[Optimizer.OptimizationFunctionLength(Case) + 1];
    
    Function = new double[Optimizer.OptimizationFunctionLength(Case) + 1];    
    
    for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(Case) ; i++ ) {
       
       Vec[i] = 1.;
       
    }

    Optimizer.SetGradientVector(Case, Vec);

    Optimizer.SolveAdjoint();
    
    
    // Open the gradient file

    sprintf(GradientFileName,"%s.opt.gradient",FileName);

    if ( (GRADFile = fopen(GradientFileName, "w")) == NULL ) {
    
       printf("Could not open the gradient output file! \n");
    
       exit(1);
    
    }
           
    // Output the gradient data

    fprintf(GRADFile,"%d \n",Optimizer.NumberOfNodes());fflush(NULL);
    
    for ( p = 1 ; p <= Optimizer.NumberOfOptimizationFunctions() ; p++ ) {
       
       Optimizer.GetFunctionValue(p,Function);

       fprintf(GRADFile,"Function: %d \n",p);fflush(NULL);

       for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(p) ; i++ ) {
          
          fprintf(GRADFile,"Function(%d): %e \n",p,Function[i]);fflush(NULL);
          
       }
          
                       //1234567890    1234567890 1234567890 1234567890    1234567890 1234567890 1234567890    
       
       fprintf(GRADFile,"   Node            X          Y          Z            dFdX       dFdY       dFdZ \n");
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          fprintf(GRADFile,"%10d   %10.6e %10.6e %10.6e   %10.6e %10.6e %10.6e \n",
           i,
           Optimizer.NodeX(i),
           Optimizer.NodeY(i),
           Optimizer.NodeZ(i),
           Optimizer.GradientX(p,i),
           Optimizer.GradientY(p,i),
           Optimizer.GradientZ(p,i));
           
           fflush(NULL);
       }
       
       fprintf(GRADFile," \n\n\n");
       
       fprintf(GRADFile,"Unsteady Results: \n\n");
              
       for ( t = 1 ; t <= Optimizer.OptimizationNumberOfTimeSteps(p) ; t++ ) {
          
          fprintf(GRADFile,"Time Step: %d \n\n",t);

          Optimizer.GetUnsteadyFunctionValue(p,t,Function);
          
          fprintf(GRADFile,"Function: %d \n",p);fflush(NULL);
          
          for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(p) ; i++ ) {
             
             fprintf(GRADFile,"Function(%d): %e \n",p,Function[i]);fflush(NULL);
             
          }
       
          for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
             
             fprintf(GRADFile,"%10d   %10.6e %10.6e %10.6e   %10.6e %10.6e %10.6e \n",
              i,
              Optimizer.NodeX(i),
              Optimizer.NodeY(i),
              Optimizer.NodeZ(i),
              Optimizer.GradientX(p,t,i),
              Optimizer.GradientY(p,t,i),
              Optimizer.GradientZ(p,t,i));
              
              fflush(NULL);
          }
          
          fprintf(GRADFile,"\n");
          
       }          
       
    }     
     
    fclose(GRADFile);

    // Gradient with respect to input values 
 
    printf("\n\n\n\n");
    printf("Gradient with respect to Alpha   : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_ALPHA  )); fflush(NULL);
    printf("Gradient with respect to Beta    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_BETA   )); fflush(NULL);
    printf("Gradient with respect to Mach    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_MACH   )); fflush(NULL);
    printf("Gradient with respect to Vinf    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_VINF   )); fflush(NULL);
    printf("Gradient with respect to Density : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_DENSITY)); fflush(NULL);
    printf("Gradient with respect to ReCref  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_RECREF )); fflush(NULL);
    printf("Gradient with respect to P rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_P_RATE )); fflush(NULL);
    printf("Gradient with respect to Q rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_Q_RATE )); fflush(NULL);
    printf("Gradient with respect to R rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_R_RATE )); fflush(NULL);
    
    printf("Gradient with respect to rotor Omega  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_NUMBER_OF_INPUTS + 1)); fflush(NULL);
      
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                            TestCase_5                                        #
#                                                                              #
# Single vector optimization functions, with user supplied intitial gradient.. #
# followed by a matrix-vector multiply, and an adjoint matrix-vector multiply  #
#                                                                              #
##############################################################################*/

int TestCase_5(char *FileName)
{
    int i, Case, Wing;
    double *Vec;
    double *VecIn, *VecOut, *RHS;
    double *AdjointVecIn, *AdjointVecOut, *AdjointRHS;
    
    printf("Running test case #5... \n");fflush(NULL);
       
    VSP_OPTIMIZER Optimizer;

    Optimizer.NumberOfThreads() = 4;
    
    Optimizer.DoUnsteadyAnalysis() = 0;
  
    Optimizer.NumberOfOptimizationFunctions() = 1;
    
    Case = 1;
    
    Optimizer.OptimizationFunction(Case) = OPT_WING_CX_TOTAL; // xyz force coefficents vs span for wing 1
    
    Optimizer.OptimizationSet(Case) = Wing = 1; // We have to specify which wing this is

    Optimizer.SetArrayOffSetType(DEFAULT_ARRAYS);
     
    Optimizer.Setup(FileName);
    
    Optimizer.SolveForward();
   
    // User supplied input gradient vector
    
    printf("Function Length: %d \n",Optimizer.OptimizationFunctionLength(1));
        
    Case = 1; // We have only 1 optimization function, which is a vector... OPT_WING_CL_TOTAL ... of the wing loading
 
    Vec = new double[Optimizer.OptimizationFunctionLength(Case) + 1];
  
    for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(Case) ; i++ ) {
       
       Vec[i] = 1.;
       
    }

    Optimizer.SetGradientVector(Case, Vec);

    Optimizer.SolveAdjoint();
    
    // Do matrix-vector multiply
 
     VecIn = new double[Optimizer.NumberOfLoops() + 1];
    VecOut = new double[Optimizer.NumberOfLoops() + 1];
       RHS = new double[Optimizer.NumberOfLoops() + 1];
       
    for ( i = 1 ; i <= Optimizer.NumberOfLoops() ; i++ ) {
       
        VecIn[i] = 1.;
       VecOut[i] = 0.;
          RHS[i] = 0.;
       
    }
    
    printf("\n");
    printf("Doing matrix-vector multiply... and grabbing rhs \n");fflush(NULL);
  
    Optimizer.CalculateMatrixVectorProductAndRightHandSide(VecIn, VecOut, RHS);
    
    // Do adjoint matrix-vector multiply
 
     AdjointVecIn = new double[Optimizer.NumberOfLoops() + 1];
    AdjointVecOut = new double[Optimizer.NumberOfLoops() + 1];
       AdjointRHS = new double[Optimizer.NumberOfLoops() + 1];

    for ( i = 1 ; i <= Optimizer.NumberOfLoops() ; i++ ) {
       
        AdjointVecIn[i] = 1.;
       AdjointVecOut[i] = 0.;
          AdjointRHS[i] = 0.;
       
    }
      
    printf("Doing adjoint matrix-vector multiply... and grabbing rhs \n");fflush(NULL);
      
    Optimizer.CalculateAdjointMatrixVectorProductAndRightHandSide(AdjointVecIn, AdjointVecOut, AdjointRHS);
        
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                            TestCase_6                                        #
#                                                                              #
# UNSTEADY analysis... with 3 VECTOR optimization functions with user supplied #
# intitial gradient                                                            #                  
#                                                                              #
##############################################################################*/

int TestCase_6(char *FileName)
{
   
    int i, p, Case, Wing;
    double *Function1, *Function2, *Function3, *Vec1, *Vec2, *Vec3;
    char GradientFileName[2000];
    FILE *GRADFile;
    
    printf("Running test case #6... \n");fflush(NULL);
       
    VSP_OPTIMIZER Optimizer;

    Optimizer.NumberOfThreads() = 4;
    
    Optimizer.DoUnsteadyAnalysis() = 1;
    
    Optimizer.NumberOfOptimizationFunctions() = 3;
    
    
    Case = 1;
    
    Optimizer.OptimizationFunction(Case) = OPT_WING_CX_TOTAL; 
    
    Optimizer.OptimizationSet(Case) = Wing = 1; // We have to specify which wing this is


    Case = 2;
    
    Optimizer.OptimizationFunction(Case) = OPT_WING_CY_TOTAL; 
    
    Optimizer.OptimizationSet(Case) = Wing = 1; // We have to specify which wing this is
    

    Case = 3;
    
    Optimizer.OptimizationFunction(Case) = OPT_WING_CZ_TOTAL; 
    
    Optimizer.OptimizationSet(Case) = Wing = 1; // We have to specify which wing this is
    
    
    Optimizer.SetArrayOffSetType(DEFAULT_ARRAYS);
            
    Optimizer.Setup(FileName);
    
    Optimizer.SolveForward();

    // User supplied input gradient vector
            
    Case = 1; 
 
    printf("Function %d Length: %d \n",Case,Optimizer.OptimizationFunctionLength(Case));

    Vec1 = new double[Optimizer.OptimizationFunctionLength(Case) + 1];
    
    Function1 = new double[Optimizer.OptimizationFunctionLength(Case) + 1];
    
    for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(Case) ; i++ ) {
       
       Vec1[i] = 1.;
       
    }

    Optimizer.SetGradientVector(Case, Vec1);


    Case = 2; 
 
    printf("Function %d Length: %d \n",Case,Optimizer.OptimizationFunctionLength(Case));
 
    Vec2 = new double[Optimizer.OptimizationFunctionLength(Case) + 1];
    
    Function2 = new double[Optimizer.OptimizationFunctionLength(Case) + 1];
    
    for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(Case) ; i++ ) {
       
       Vec2[i] = 1.;
       
    }
    
    Optimizer.SetGradientVector(Case, Vec2);
   
   
    Case = 3; 

    printf("Function %d Length: %d \n",Case,Optimizer.OptimizationFunctionLength(Case));
 
    Vec3 = new double[Optimizer.OptimizationFunctionLength(Case) + 1];

    Function3 = new double[Optimizer.OptimizationFunctionLength(Case) + 1];
    
    for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(Case) ; i++ ) {
       
       Vec3[i] = 1.;
       
    }
    
    Optimizer.SetGradientVector(Case, Vec3);
    
    printf("Solving adjoint... \n");fflush(NULL);
    
    Optimizer.SolveAdjoint();
    
    // Open the gradient file

    sprintf(GradientFileName,"%s.opt.gradient",FileName);

    if ( (GRADFile = fopen(GradientFileName, "w")) == NULL ) {
    
       printf("Could not open the gradient output file! \n");
    
       exit(1);
    
    }
           
    // Output the gradient data

    fprintf(GRADFile,"%d \n",Optimizer.NumberOfNodes());fflush(NULL);
    
    for ( p = 1 ; p <= Optimizer.NumberOfOptimizationFunctions() ; p++ ) {
       
       if ( p == 1 ) Optimizer.GetFunctionValue(p,Function1);
       if ( p == 2 ) Optimizer.GetFunctionValue(p,Function2);
       if ( p == 3 ) Optimizer.GetFunctionValue(p,Function3);

       fprintf(GRADFile,"Function: %d \n",p);

       for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(p) ; i++ ) {
          
          if ( p == 1 ) fprintf(GRADFile,"F(%d): %f \n",p,Function1[i]);
          if ( p == 2 ) fprintf(GRADFile,"F(%d): %f \n",p,Function2[i]);
          if ( p == 3 ) fprintf(GRADFile,"F(%d): %f \n",p,Function3[i]);
          
       }
        
                       //1234567890    1234567890 1234567890 1234567890    1234567890 1234567890 1234567890    
       
       fprintf(GRADFile,"   Node            X          Y          Z            dFdX       dFdY       dFdZ \n");
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          fprintf(GRADFile,"%10d   %10.6e %10.6e %10.6e   %10.6e %10.6e %10.6e \n",
           i,
           Optimizer.NodeX(i),
           Optimizer.NodeY(i),
           Optimizer.NodeZ(i),
           Optimizer.GradientX(p,i),
           Optimizer.GradientY(p,i),
           Optimizer.GradientZ(p,i));
           
           fflush(NULL);
       }
       
       fprintf(GRADFile," \n\n\n");
       
    }     
     
    fclose(GRADFile);

    // Gradient with respect to input values 
 
    printf("\n\n\n\n");
    printf("Gradient with respect to Alpha   : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_ALPHA  ));
    printf("Gradient with respect to Beta    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_BETA   ));
    printf("Gradient with respect to Mach    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_MACH   ));
    printf("Gradient with respect to Vinf    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_VINF   ));
    printf("Gradient with respect to Density : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_DENSITY));
    printf("Gradient with respect to ReCref  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_RECREF ));
    printf("Gradient with respect to P rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_P_RATE ));
    printf("Gradient with respect to Q rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_Q_RATE ));
    printf("Gradient with respect to R rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_R_RATE ));
        
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                            TestCase_7                                        #
#                                                                              #
#        Simple optimization                                                   #
#                                                                              #
##############################################################################*/

int TestCase_7(char *FileName)
{
    int i, j, Iter, IterMax, NumberOfParameterValues, NumberOfNodes, k, Done;
    double CL, CD, CM, CLreq, *NodeXYZ, *dFdMesh[3], *dF_dParameter, *Gradient, *GradientOld, F, Fnew, Delta, StepSize;
    double Lambda_1, Lambda_2, Lambda_3, **dMesh_dParameter;
    double *ParameterValues, *NewParameterValues, *BestParameterValues;
    char HistoryFileName[2000], CommandLine[2000], OptimizationSetupFileName[2000];
    FILE *HistoryFile, *OptimizationSetupFile;
    
    printf("Running test case #7... \n");fflush(NULL);

    // Default design CL
    
    CLreq = 0.4;    
    
    // Default CL, CD, and CM weights
   
    Lambda_1 = 100.;
    Lambda_2 = 100.;
    Lambda_3 = 100.;

    // Read in the optimization options
    
    sprintf(OptimizationSetupFileName,"%s.opt",FileName);

    if ( (OptimizationSetupFile = fopen(OptimizationSetupFileName, "r")) == NULL ) {
    
       printf("Could not open the optimization setup file: %s! \n",OptimizationSetupFileName);
       printf("Using default optimization values... \n");fflush(NULL);
    
    }
    
    else {
    
       printf("Reading in optimization values from file: %s \n", OptimizationSetupFileName);fflush(NULL);
       
       fscanf(OptimizationSetupFile,"CLreq = %lf \n",&CLreq);
       fscanf(OptimizationSetupFile,"Lambda1 = %lf \n",&Lambda_1);
       fscanf(OptimizationSetupFile,"Lambda2 = %lf \n",&Lambda_2);
       fscanf(OptimizationSetupFile,"Lambda3 = %lf \n",&Lambda_3);
       
    }
    
    printf("CLreq = %f \n",CLreq);
    printf("Lambda1 = %f \n",Lambda_1);
    printf("Lambda2 = %f \n",Lambda_2);
    printf("Lambda3 = %f \n",Lambda_3);

    // Read in the OpenVSP des file
    
    ParameterValues = ReadOpenVSPDesFile(FileName,NumberOfParameterValues);
    
    dF_dParameter = new double[NumberOfParameterValues + 1];
    
    NewParameterValues = new double[NumberOfParameterValues + 1];
    
    BestParameterValues = new double[NumberOfParameterValues + 1];
    
    Gradient = new double[NumberOfParameterValues + 1];
    
    GradientOld = new double[NumberOfParameterValues + 1];

    for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
    
       GradientOld[j] = Gradient[j] = dF_dParameter[j] = 0.;
       
       BestParameterValues[j] = ParameterValues[j];
       
    }
       
    // Calculate the mesh derivatives wrt the design parameters
    
    dMesh_dParameter = CalculateOpenVSPGeometryGradients(FileName,NumberOfParameterValues,ParameterValues);
    
    // Create an initial OpenVSP VSP_GEOM file with the initial parameter values

    CreateVSPGeometry(FileName,NumberOfParameterValues,ParameterValues);
            
    // Create an optimizer object
       
    VSP_OPTIMIZER Optimizer;
    
    // Set number of threads to use during solve process

    Optimizer.NumberOfThreads() = 4;
   
    // This is a steady state analysis
    
    Optimizer.DoUnsteadyAnalysis() = 0;
    
    // We have 3 design variables Test
    
    Optimizer.NumberOfOptimizationFunctions() = 3;

    // First design variable is CL

    Optimizer.OptimizationSet(1) = 1;
    
    Optimizer.OptimizationFunction(1) = OPT_CL_TOTAL;
    
    // Second design variable is CD
    
    Optimizer.OptimizationSet(2) = 1;
    
    Optimizer.OptimizationFunction(2) = OPT_CD_TOTAL;
  
    // Second design variable is pitching moment, CM
    
    Optimizer.OptimizationSet(3) = 1;
    
    Optimizer.OptimizationFunction(3) = OPT_CMY_TOTAL;
    
    // Use VSPAERO array convention... ie arrays go from 1 to N
    
    Optimizer.SetArrayOffSetType(DEFAULT_ARRAYS);

    // Set up the problem, here file name is the user provided vspaero model name
    
    Optimizer.Setup(FileName);
    
    // Turn on Karman-Tsien compressibility corrrections... this has to be done after Setup()
    
    Optimizer.TurnOnKarmanTsienCorrection();    

    // Allocate some space for the derivatives 
    
    dFdMesh[0] = new double[Optimizer.NumberOfNodes() + 1];
    dFdMesh[1] = new double[Optimizer.NumberOfNodes() + 1];
    dFdMesh[2] = new double[Optimizer.NumberOfNodes() + 1];
    
    // Open the history file

    sprintf(HistoryFileName,"%s.opt.history",FileName);

    if ( (HistoryFile = fopen(HistoryFileName, "w")) == NULL ) {
    
       printf("Could not open the optimization history output file! \n");
    
       exit(1);
    
    }
    
    // Clean up any old opt adb files
    
    sprintf(CommandLine,"rm %s.opt.*.adb",FileName);
    
    system(CommandLine);
       
    // Maximum number of design steps
    
    IterMax = 25;
    
                        //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 
    fprintf(HistoryFile,"    Iter        CL         CD         CM         F        GradF      1DSteps   StepSize \n");

    k = 0;

    StepSize = 0.;
    
    for ( Iter = 1 ; Iter <= IterMax ; Iter++ ) {

       // Solve the forward problem and the adjoint
       
       Optimizer.Solve();
       
       // Save the .adb file for later viewing
       
       sprintf(CommandLine,"cp %s.adb %s.opt.%d.adb",FileName,FileName,Iter);
       
       system(CommandLine);
       
       if ( Iter == 1 ) {
          
          // Save the .adb file for later viewing
          
          sprintf(CommandLine,"cp %s.adb %s.opt.adb",FileName,FileName);
          
          system(CommandLine);
          
          // Copy over the .cases file for viewer
          
          sprintf(CommandLine,"cp %s.adb.cases %s.opt.adb.cases",FileName,FileName);
          
          system(CommandLine);          
       
       }          
       
       Optimizer.GetFunctionValue(1,CL);
       Optimizer.GetFunctionValue(2,CD);
       Optimizer.GetFunctionValue(3,CM);

       // Calculate the final objective function and it's gradients
       // Here we have f = L1*(CL - CLreq)^2 + L2*CD^2 + L3*CM^2

       F = Lambda_1 * pow(CL - CLreq,2.) + Lambda_2 * pow(CD,2.) + Lambda_3 * pow(CM,2.);
              
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          dFdMesh[0][i] = 2.*Lambda_1*(CL - CLreq)*Optimizer.GradientX(1,i) + 2.*Lambda_2*CD*Optimizer.GradientX(2,i) + 2.*Lambda_3*CM*Optimizer.GradientX(3,i);
          dFdMesh[1][i] = 2.*Lambda_1*(CL - CLreq)*Optimizer.GradientY(1,i) + 2.*Lambda_2*CD*Optimizer.GradientY(2,i) + 2.*Lambda_3*CM*Optimizer.GradientY(3,i);
          dFdMesh[2][i] = 2.*Lambda_1*(CL - CLreq)*Optimizer.GradientZ(1,i) + 2.*Lambda_2*CD*Optimizer.GradientZ(2,i) + 2.*Lambda_3*CM*Optimizer.GradientZ(3,i);
                   
       }
       
       // Chain rule... calculate derivatives wrt parameters
       
       for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
          
          dF_dParameter[j] = 0;
       
          for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
             
             dF_dParameter[j] +=   dFdMesh[0][i] * dMesh_dParameter[j][3*i-2]
                                 + dFdMesh[1][i] * dMesh_dParameter[j][3*i-1]
                                 + dFdMesh[2][i] * dMesh_dParameter[j][3*i  ];
                        
          }     
          
       }  

       
       // Store old and new gradients
       
       for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
       
          GradientOld[j] = Gradient[j];
          
          Gradient[j] = dF_dParameter[j];
          
       }
                    
       // Conjugate gradient adjustment of the gradient...
       
       if ( Iter > 1 ) CGState(GradientOld,Gradient,NumberOfParameterValues);
         
       // Calculate magnitude of the gradient and normalize it
       
       Delta = Normalize(Gradient, NumberOfParameterValues);

       StepSize = Delta;       
       
       if ( StepSize > 0.1) StepSize = 0.1;

//StepSize = 0.05;

       if ( Iter != IterMax ) {
          
          // A really bad 1D search...
          
          k = 0;
          
          Done = 0;
   
          while ( !Done && k < 10 ) {
             
             printf("\n\n\n\n\n\n At search iteration %d the current Step Size: %f \n\n\n\n\n\n ",k,StepSize);
          
             for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
             
                ParameterValues[j] -= StepSize * Gradient[j];
                
   //             NewParameterValues[j] = ParameterValues[j] - StepSize * Gradient[j];
         
             }
             
             // Update the OpenVSP geometry based on these new parameters
             
             CreateVSPGeometry(FileName,NumberOfParameterValues,ParameterValues);
             
             // Read in the VSPGEOM mesh
             
             NodeXYZ = ReadVSPGeomFile(FileName,NumberOfNodes);
             
             // Update the solver and adjoint meshes
             
             Optimizer.UpdateGeometry(NodeXYZ);

             delete [] NodeXYZ;
             
             // Do a forward solve only to evaluate the functional
             
             Optimizer.SolveForward();
             
             Optimizer.GetFunctionValue(1,CL);
             Optimizer.GetFunctionValue(2,CD);
             Optimizer.GetFunctionValue(3,CM);
                    
             Fnew = Lambda_1 * pow(CL - CLreq,2.) + Lambda_2 * pow(CD,2.) + Lambda_3 * pow(CM,2.);

             printf("1D search... current F: %f ... previous F: %f \n",Fnew,F); fflush(NULL);
             
             // Just keep going until function has increased... 
             
             if ( Fnew > F ) {
                
                printf("Stopping 1D search and backing up a step... \n");                
                
                // Back up 
                
                if ( k > 0 ) {

                   for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
                   
             //         ParameterValues[j] = BestParameterValues[j];
                 
                      ParameterValues[j] += StepSize * Gradient[j];
                 
                   }
                   
                   // Update the OpenVSP geometry based on these new parameters
                   
                   CreateVSPGeometry(FileName,NumberOfParameterValues,ParameterValues);
                   
                   // Read in the VSPGEOM mesh
                   
                   NodeXYZ = ReadVSPGeomFile(FileName,NumberOfNodes);
                   
                   // Update the solver and adjoint meshes
                   
                   Optimizer.UpdateGeometry(NodeXYZ);
                   
                   delete [] NodeXYZ;
                   
                }
                   
                Done = 1; 
                
                if ( k == 0 ) F = Fnew;
                
             }
             
             else {
                
                F = Fnew;
                
                for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
                
                   BestParameterValues[j] = NewParameterValues[j];
                
                }                
                
             }                             
             
             if ( !Done ) {
                
                k++;
             
                if ( !Done ) StepSize *= 1.618;
               
             }
             
          }
             
       }

       fprintf(HistoryFile,"%10d %10.5f %10.5f %10.5f %10.5f %10.5f %10d %10.5f \n",Iter,CL,CD,CM,F,Delta,k,StepSize);

       printf("\n\n\n\n\n\n Delta, StepSize: %f %f \n\n\n\n\n\n\n\n",Delta,StepSize);fflush(NULL);
       
       // Update the mesh derivatives... or not
       
       //if ( Iter != IterMax ) {
       //   
       //   DeleteOpenVSPGeometryGradients(dMesh_dParameter, NumberOfParameterValues);
       //          
       //   dMesh_dParameter = CalculateOpenVSPGeometryGradients(FileName,NumberOfParameterValues,ParameterValues);
       //   
       //}
       
    }
    
    fclose(HistoryFile);
    
    // Write out a tri file because we can...     
    
    Optimizer.WriteOutCart3dTriFile();

    // Gradient with respect to input values ... because we can...
 
    printf("\n\n\n\n");
    printf("Gradient with respect to Alpha   : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_ALPHA  ));
    printf("Gradient with respect to Beta    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_BETA   ));
    printf("Gradient with respect to Mach    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_MACH   ));
    printf("Gradient with respect to Vinf    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_VINF   ));
    printf("Gradient with respect to Density : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_DENSITY));
    printf("Gradient with respect to ReCref  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_RECREF ));
    printf("Gradient with respect to P rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_P_RATE ));
    printf("Gradient with respect to Q rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_Q_RATE ));
    printf("Gradient with respect to R rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_R_RATE ));
        
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                            TestCase_8                                        #
#                                                                              #
#        Simple unsteady optimization                                          #
#                                                                              #
##############################################################################*/

int TestCase_8(char *FileName)
{
    int i, j, p, Iter, IterMax, NumberOfParameterValues, NumberOfNodes, k, Done;
    double CL, CD, CM, CLreq, *NodeXYZ, *dFdMesh[3], *dF_dParameter, *Gradient, *GradientOld, F, Fnew, Delta, StepSize;
    double Lambda_1, Lambda_2, Lambda_3, *ParameterValues, **dMesh_dParameter;
    char HistoryFileName[2000], CommandLine[2000];
    FILE *HistoryFile;
    
    printf("Running test case #8... \n");fflush(NULL);

    // Read in the OpenVSP des file
    
    ParameterValues = ReadOpenVSPDesFile(FileName,NumberOfParameterValues);
    
    dF_dParameter = new double[NumberOfParameterValues + 1];
    
    Gradient = new double[NumberOfParameterValues + 1];
    
    GradientOld = new double[NumberOfParameterValues + 1];

    for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
    
       GradientOld[j] = Gradient[j] = dF_dParameter[j] = 0.;
       
    }
              
    // Calculate the mesh derivatives wrt the design parameters
    
    dMesh_dParameter = CalculateOpenVSPGeometryGradients(FileName,NumberOfParameterValues,ParameterValues);
    
    // Create an initial OpenVSP VSP_GEOM file with the initial parameter values
     
    CreateVSPGeometry(FileName,NumberOfParameterValues,ParameterValues);
            
    // Create an optimizer object
       
    VSP_OPTIMIZER Optimizer;
    
    // Set number of threads to use during solve process

    Optimizer.NumberOfThreads() = 4;
    
    // This is a steady state analysis
    
    Optimizer.DoUnsteadyAnalysis() = 1;
    
    // We have 3 design variables 
    
    Optimizer.NumberOfOptimizationFunctions() = 3;
    
    // First design variable is CL
    
    Optimizer.OptimizationFunction(1) = OPT_CL_TOTAL;
    
    // Second design variable is CD
    
    Optimizer.OptimizationFunction(2) = OPT_CD_TOTAL;

    // Second design variable is pitching moment, CM
    
    Optimizer.OptimizationFunction(3) = OPT_CMY_TOTAL;
    
    // Use VSPAERO array convention... ie arrays go from 1 to N
    
    Optimizer.SetArrayOffSetType(DEFAULT_ARRAYS);

    // Set up the problem, here file name is the user provided vspaero model name
    
    Optimizer.Setup(FileName);

    // Allocate some space for the derivatives 
    
    dFdMesh[0] = new double[Optimizer.NumberOfNodes() + 1];
    dFdMesh[1] = new double[Optimizer.NumberOfNodes() + 1];
    dFdMesh[2] = new double[Optimizer.NumberOfNodes() + 1];
    
    // Open the history file

    sprintf(HistoryFileName,"%s.opt.history",FileName);

    if ( (HistoryFile = fopen(HistoryFileName, "w")) == NULL ) {
    
       printf("Could not open the optimization history output file! \n");
    
       exit(1);
    
    }
    
    // Clean up any old opt adb files
    
    sprintf(CommandLine,"rm %s.opt.*.adb",FileName);
    
    system(CommandLine);
       
    // Maximum number of design steps
    
    IterMax = 10;

    // Design CL
    
    CLreq = 0.4;

    fprintf(HistoryFile,"Iter CL CD CM F GradF 1DSteps StepSize \n");
    
    k = 0;

    StepSize = 0.;
    
    for ( Iter = 1 ; Iter <= IterMax ; Iter++ ) {

       // Solve the forward problem and the adjoint
       
       Optimizer.Solve();
       
       // Save the .adb file for later viewing
       
       sprintf(CommandLine,"cp %s.adb %s.opt.%d.adb",FileName,FileName,Iter);
       
       system(CommandLine);
       
       if ( Iter == 1 ) {
          
          // Save the .adb file for later viewing
          
          sprintf(CommandLine,"cp %s.adb %s.opt.adb",FileName,FileName);
          
          system(CommandLine);
          
          // Copy over the .cases file for viewer
          
          sprintf(CommandLine,"cp %s.adb.cases %s.opt.adb.cases",FileName,FileName);
          
          system(CommandLine);          
       
       }          
       
       Optimizer.GetFunctionValue(1,CL);
       Optimizer.GetFunctionValue(2,CD);
       Optimizer.GetFunctionValue(3,CM);

       // Calculate the final objective function and it's gradients
       // Here we have f = L1*(CL - CLreq)^2 + L2*CD^2 + L3*CM^2
       
       Lambda_1 = 100.;
       Lambda_2 = 100.;
       Lambda_3 = 100.;
       
       F = Lambda_1 * pow(CL - CLreq,2.) + Lambda_2 * pow(CD,2.) + Lambda_3 * pow(CM,2.);
              
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          dFdMesh[0][i] = 2.*Lambda_1*(CL - CLreq)*Optimizer.GradientX(1,i) + 2.*Lambda_2*CD*Optimizer.GradientX(2,i) + 2.*Lambda_3*CM*Optimizer.GradientX(3,i);
          dFdMesh[1][i] = 2.*Lambda_1*(CL - CLreq)*Optimizer.GradientY(1,i) + 2.*Lambda_2*CD*Optimizer.GradientY(2,i) + 2.*Lambda_3*CM*Optimizer.GradientY(3,i);
          dFdMesh[2][i] = 2.*Lambda_1*(CL - CLreq)*Optimizer.GradientZ(1,i) + 2.*Lambda_2*CD*Optimizer.GradientZ(2,i) + 2.*Lambda_3*CM*Optimizer.GradientZ(3,i);
               
       }
       
       // Chain rule... calculate derivatives wrt parameters
       
       for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
          
          dF_dParameter[j] = 0;
       
          for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
             
             dF_dParameter[j] +=   dFdMesh[0][i] * dMesh_dParameter[j][3*i-2]
                                 + dFdMesh[1][i] * dMesh_dParameter[j][3*i-1]
                                 + dFdMesh[2][i] * dMesh_dParameter[j][3*i  ];
                        
          }     
          
       }  
       
       // Store old and new gradients
       
       for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
       
          GradientOld[j] = Gradient[j];
          
          Gradient[j] = dF_dParameter[j];
          
       }
       
       // Conjugate gradient adjustment of the gradient...
       
       if ( Iter > 1 ) CGState(GradientOld,Gradient,NumberOfParameterValues);
  
       // Calculate magnitude of the gradient and normalize it
       
       Delta = Normalize(Gradient, NumberOfParameterValues);

       StepSize = Delta;       
       
       if ( StepSize > 1. ) StepSize = 1.;

       fprintf(HistoryFile,"%d %f %f %f %f %f %d %f \n",Iter,CL,CD,CM,F,Delta,k,StepSize);
              
       printf("\n\n\n\n\n\n Delta, StepSize: %f %f \n\n\n\n\n\n\n\n",Delta,StepSize);fflush(NULL);
       
       if ( Iter != IterMax ) {
          
          // A really bad 1D search...
          
          k = 0;
          
          Done = 0;
          
          while ( !Done && k < 10 ) {
             
             printf("\n\n\n\n\n\n At search iteration %d the current Step Size: %f \n\n\n\n\n\n ",k,StepSize);
          
             for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
             
                ParameterValues[j] -= StepSize * Gradient[j];
                
             }
             
             // Update the OpenVSP geometry based on these new parameters
             
             CreateVSPGeometry(FileName,NumberOfParameterValues,ParameterValues);
             
             // Read in the VSPGEOM mesh
             
             NodeXYZ = ReadVSPGeomFile(FileName,NumberOfNodes);
             
             // Update the solver and adjoint meshes
             
             Optimizer.UpdateGeometry(NodeXYZ);

             delete [] NodeXYZ;
             
             // Do a forward solve only to evaluate the functional
             
             Optimizer.SolveForward();
             
             Optimizer.GetFunctionValue(1,CL);
             Optimizer.GetFunctionValue(2,CD);
             Optimizer.GetFunctionValue(3,CM);
                    
             Fnew = Lambda_1 * pow(CL - CLreq,2.) + Lambda_2 * pow(CD,2.) + Lambda_3 * pow(CM,2.);

             printf("1D search... current F: %f ... previous F: %f \n",Fnew,F); fflush(NULL);
             
             // Just keep going until function has increased... 
             
             if ( Fnew > F ) {
                
                // Back up 
                
                if ( k > 0 ) {

                   for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
                   
                      ParameterValues[j] += StepSize * Gradient[j];
                      
                   }
                   
                   // Update the OpenVSP geometry based on these new parameters
                   
                   CreateVSPGeometry(FileName,NumberOfParameterValues,ParameterValues);
                   
                   // Read in the VSPGEOM mesh
                   
                   NodeXYZ = ReadVSPGeomFile(FileName,NumberOfNodes);
                   
                   // Update the solver and adjoint meshes
                   
                   Optimizer.UpdateGeometry(NodeXYZ);
                   
                   delete [] NodeXYZ;
                   
                }
                   
                Done = 1; 
                
             }
             
             else {
                
                F = Fnew;
                
             }            
             
             k++;
             
             if ( !Done ) StepSize *= 1.618;
             
          }
             
       }
       
       // Update mesh derivatives ... or not... 
       
     //  if ( Iter != IterMax ) {
     //     
     //     DeleteOpenVSPGeometryGradients(dMesh_dParameter, NumberOfParameterValues);
     //            
     //     dMesh_dParameter = CalculateOpenVSPGeometryGradients(FileName,NumberOfParameterValues,ParameterValues);
     //     
     //  }       
       
    }
    
    fclose(HistoryFile);
    
    // Write out a tri file because we can...     
    
    Optimizer.WriteOutCart3dTriFile();

    // Gradient with respect to input values ... because we can...
 
    printf("\n\n\n\n");
    printf("Gradient with respect to Alpha   : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_ALPHA  ));
    printf("Gradient with respect to Beta    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_BETA   ));
    printf("Gradient with respect to Mach    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_MACH   ));
    printf("Gradient with respect to Vinf    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_VINF   ));
    printf("Gradient with respect to Density : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_DENSITY));
    printf("Gradient with respect to ReCref  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_RECREF ));
    printf("Gradient with respect to P rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_P_RATE ));
    printf("Gradient with respect to Q rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_Q_RATE ));
    printf("Gradient with respect to R rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_R_RATE ));
        
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                            TestCase_9                                        #
#                                                                              #
#  Rotor CT derivatives wrt feather                                            #
#                                                                              #
##############################################################################*/

int TestCase_9(char *FileName)
{
    int i, j, p, NumberOfParameterValues, NumberOfNodes;
    double Function, CTGradient, CPGradient, EffGradient, Gradient, CP, CT, Eff, *NodeXYZ, *dFdMesh[3];
    double *ParameterValues, **dMesh_dParameter;
    char HistoryFileName[2000], CommandLine[2000];
    FILE *HistoryFile;
    
    printf("Running test case #9... \n");fflush(NULL);

    // Read in the OpenVSP des file
    
    ParameterValues = ReadOpenVSPDesFile(FileName,NumberOfParameterValues);

    ParameterValues[1] = 2.;

    // Calculate the mesh derivatives wrt the design parameters
    
    dMesh_dParameter = CalculateOpenVSPGeometryGradients(FileName,NumberOfParameterValues,ParameterValues);
    
    // Create an initial OpenVSP VSP_GEOM file with the initial parameter values
     
    CreateVSPGeometry(FileName,NumberOfParameterValues,ParameterValues);
            
    // Create an optimizer object
       
    VSP_OPTIMIZER Optimizer;
    
    // Set number of threads to use during solve process

    Optimizer.NumberOfThreads() = 1;
    
    // This is a steady state analysis
    
    Optimizer.DoUnsteadyAnalysis() = 1;
    
    // We have 2 design variables 
    
    Optimizer.NumberOfOptimizationFunctions() = 3;
    
    // First design variable is CT
    
    Optimizer.OptimizationSet(1) = 1;
    
    Optimizer.OptimizationFunction(1) = OPT_ROTOR_CT_TOTAL;
    
    Optimizer.OptimizationSet(2) = 1;
    
    Optimizer.OptimizationFunction(2) = OPT_ROTOR_CP_TOTAL;

    Optimizer.OptimizationSet(3) = 1;
    
    Optimizer.OptimizationFunction(3) = OPT_ROTOR_EFFICIENCY;     
    
    // Use VSPAERO array convention... ie arrays go from 1 to N
    
    Optimizer.SetArrayOffSetType(DEFAULT_ARRAYS);

    // Set up the problem, here file name is the user provided vspaero model name
    
    Optimizer.Setup(FileName);

    // Allocate some space for the derivatives 
    
    dFdMesh[0] = new double[Optimizer.NumberOfNodes() + 1];
    dFdMesh[1] = new double[Optimizer.NumberOfNodes() + 1];
    dFdMesh[2] = new double[Optimizer.NumberOfNodes() + 1];
    
    // Open the history file

    sprintf(HistoryFileName,"%s.opt.history",FileName);

    if ( (HistoryFile = fopen(HistoryFileName, "w")) == NULL ) {
    
       printf("Could not open the optimization history output file! \n");
    
       exit(1);
    
    }

    // Do a sweep over rotor feather angle
    
    for ( p = 1 ; p <= 10 ; p++ ) {

       // Solve the forward problem and the adjoint
       
       Optimizer.Solve();

       Optimizer.GetFunctionValue(1,CT);
        
       Optimizer.GetFunctionValue(2,CP);
       
       Optimizer.GetFunctionValue(3,Eff);
              
       // CT Gradient
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          dFdMesh[0][i] = Optimizer.GradientX(1,i);
          dFdMesh[1][i] = Optimizer.GradientY(1,i);
          dFdMesh[2][i] = Optimizer.GradientZ(1,i);
               
       }
              
       // Chain rule... calculate derivatives wrt parameters

       CTGradient = 0;
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          CTGradient += dFdMesh[0][i] * dMesh_dParameter[1][3*i-2]
                      + dFdMesh[1][i] * dMesh_dParameter[1][3*i-1]
                      + dFdMesh[2][i] * dMesh_dParameter[1][3*i  ];

       }

       // CP Gradient
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          dFdMesh[0][i] = Optimizer.GradientX(2,i);
          dFdMesh[1][i] = Optimizer.GradientY(2,i);
          dFdMesh[2][i] = Optimizer.GradientZ(2,i);
               
       }
              
       // Chain rule... calculate derivatives wrt parameters

       CPGradient = 0;
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          CPGradient += dFdMesh[0][i] * dMesh_dParameter[1][3*i-2]
                      + dFdMesh[1][i] * dMesh_dParameter[1][3*i-1]
                      + dFdMesh[2][i] * dMesh_dParameter[1][3*i  ];

       }
              
       // Efficiency Gradient
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          dFdMesh[0][i] = Optimizer.GradientX(3,i);
          dFdMesh[1][i] = Optimizer.GradientY(3,i);
          dFdMesh[2][i] = Optimizer.GradientZ(3,i);
               
       }
              
       // Chain rule... calculate derivatives wrt parameters

       EffGradient = 0;
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          EffGradient += dFdMesh[0][i] * dMesh_dParameter[1][3*i-2]
                       + dFdMesh[1][i] * dMesh_dParameter[1][3*i-1]
                       + dFdMesh[2][i] * dMesh_dParameter[1][3*i  ];

       }
                     
       // F Gradient
                     
       Function = 0.53817*CT/CP;
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          dFdMesh[0][i] = 0.53817*(Optimizer.GradientX(1,i)/CP - CT/(CP*CP)*Optimizer.GradientX(2,i));
          dFdMesh[1][i] = 0.53817*(Optimizer.GradientY(1,i)/CP - CT/(CP*CP)*Optimizer.GradientY(2,i));
          dFdMesh[2][i] = 0.53817*(Optimizer.GradientZ(1,i)/CP - CT/(CP*CP)*Optimizer.GradientZ(2,i));
               
       }
              
       // Chain rule... calculate derivatives wrt parameters

       Gradient = 0;
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          Gradient += dFdMesh[0][i] * dMesh_dParameter[1][3*i-2]
                    + dFdMesh[1][i] * dMesh_dParameter[1][3*i-1]
                    + dFdMesh[2][i] * dMesh_dParameter[1][3*i  ];

       }

       // Output CT and it's gradient wrt feather
       
       fprintf(HistoryFile,"%d %f ... %e %e %e %e ... %e %e %e %e \n",p,ParameterValues[1],CT,CP,Eff,Function,CTGradient,CPGradient,EffGradient,Gradient);
          
       // Update feather angle
      
       ParameterValues[1] += 0.1;
  
       // Update the OpenVSP geometry based on these new parameters
       
       CreateVSPGeometry(FileName,NumberOfParameterValues,ParameterValues);
       
       // Read in the VSPGEOM mesh
       
       NodeXYZ = ReadVSPGeomFile(FileName,NumberOfNodes);
       
       // Update the solver and adjoint meshes
       
       Optimizer.UpdateGeometry(NodeXYZ);

       // Update the mesh derivatives wrt the design parameters
   
       DeleteOpenVSPGeometryGradients(dMesh_dParameter, NumberOfParameterValues);
   
       dMesh_dParameter = CalculateOpenVSPGeometryGradients(FileName,NumberOfParameterValues,ParameterValues);
    
       delete [] NodeXYZ;
                    
    }
 
    fclose(HistoryFile);
    
    // Write out a tri file because we can...     
    
    Optimizer.WriteOutCart3dTriFile();

    // Gradient with respect to input values ... because we can...
 
    printf("\n\n\n\n");
    printf("Gradient with respect to Alpha   : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_ALPHA  ));
    printf("Gradient with respect to Beta    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_BETA   ));
    printf("Gradient with respect to Mach    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_MACH   ));
    printf("Gradient with respect to Vinf    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_VINF   ));
    printf("Gradient with respect to Density : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_DENSITY));
    printf("Gradient with respect to ReCref  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_RECREF ));
    printf("Gradient with respect to P rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_P_RATE ));
    printf("Gradient with respect to Q rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_Q_RATE ));
    printf("Gradient with respect to R rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_R_RATE ));
        
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                            ReadOpenVSPDesFile                                #
#                                                                              #
##############################################################################*/

double *ReadOpenVSPDesFile(char *FileName, int &NumberOfDesignVariables)
{
    
    int i;
    double *ParameterValues;
    char DesignFileName[2000], Variable[2000];
    FILE *DesignFile;
    
    // Open the OpenVSP des file

    sprintf(DesignFileName,"%s.des",FileName);
    
    printf("Opening: %s \n",DesignFileName);fflush(NULL);

    if ( (DesignFile = fopen(DesignFileName, "r")) == NULL ) {
    
       printf("Could not open the OpenVSP des file! \n");
    
       exit(1);
    
    }
    
    // Parse the des file, replace parameters with new values    
    
    fscanf(DesignFile,"%d \n",&NumberOfDesignVariables);

    ParameterValues = new double[NumberOfDesignVariables + 1];

    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {

       fscanf(DesignFile,"%s%lf\n",Variable,&(ParameterValues[i]));

    }
    
    fclose(DesignFile);
    
    return ParameterValues;
 
}

/*##############################################################################
#                                                                              #
#                            CreateVSPGeometry                                 #
#                                                                              #
##############################################################################*/

void CreateVSPGeometry(char *FileName, int NumberOfDesignVariables, double *ParameterValues)
{
    
    int i, NumVars;
    double Value;
    char DesignFileName[2000], Variable[2000], CommandLine[2000];
    FILE *DesignFile, *OptDesFile;
    
    // Open the OpenVSP des file

    sprintf(DesignFileName,"%s.des",FileName);
    
    printf("Opening: %s \n",DesignFileName);fflush(NULL);

    if ( (DesignFile = fopen(DesignFileName, "r")) == NULL ) {
    
       printf("Could not open the OpenVSP des file! \n");
    
       exit(1);
    
    }
    
    // Open Opt des file
    
    sprintf(DesignFileName,"Opt.des",FileName);
    
    printf("Opening: %s \n",DesignFileName);fflush(NULL);

    if ( (OptDesFile = fopen(DesignFileName, "w")) == NULL ) {
    
       printf("Could not open the OpenVSP Opt des file! \n");
    
       exit(1);
    
    }
    
    // Parse the des file, replace parameters with new values    
    
    fscanf(DesignFile,"%d \n",&NumVars);
    
    if ( NumberOfDesignVariables != NumVars ) {
       
       printf("Number of design variables does not match OpenVSP des file! \n");
       fflush(NULL);exit(1);
       
    }
    
    fprintf(OptDesFile,"%d\n",NumVars);
    
    fflush(NULL);
    
    for ( i = 1 ; i <= NumVars ; i++ ) {

       fscanf(DesignFile,"%s%lf\n",Variable,&Value);

       fprintf(OptDesFile,"%s %lf\n",Variable,ParameterValues[i]);
       
    }
    
    fclose(DesignFile);
    fclose(OptDesFile);   

    // Run vsp script to create VSPGEOM file
    
    sprintf(CommandLine,"vsp -script CreateVSPGEOM.script > vsp.out >&1 ");
    
    system(CommandLine);
 
}

/*##############################################################################
#                                                                              #
#                    CalculateOpenVSPGeometryGradients                         #
#                                                                              #
##############################################################################*/

double **CalculateOpenVSPGeometryGradients(char *FileName, int NumberOfDesignVariables, double *ParameterValues)
{
    
    int i, j, NumberOfMeshNodes;
    double **dMesh_dParameter, *NewParameterValues, Delta, *MeshMinus;

    printf("Calculating OpenVSP mesh gradients ... \n");fflush(NULL);
    
    // Create space for the mesh gradients
    
    NewParameterValues = new double[NumberOfDesignVariables + 1];
    
    dMesh_dParameter = new double*[NumberOfDesignVariables + 1];

    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {

       NewParameterValues[i] = ParameterValues[i];
       
    }
    
    // Create the baseline geometry
    
    CreateVSPGeometry(FileName,NumberOfDesignVariables,ParameterValues);
    
    dMesh_dParameter[0] = ReadVSPGeomFile(FileName,NumberOfMeshNodes);
    
    // Loop over parameters and calculate mesh gradients using finite differences
    
    Delta = 0.001;
    
    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {
    
       printf("Working on parameter: %d out of %d \n",i,NumberOfDesignVariables);fflush(NULL);
         
       // + Perturbation
         
       NewParameterValues[i] = ParameterValues[i] + Delta;
       
       printf("Creating vsp geom file... \n");fflush(NULL);
       
       CreateVSPGeometry(FileName,NumberOfDesignVariables,NewParameterValues);
       
       printf("Reading in vspgeom mesh... \n");fflush(NULL);
       
       dMesh_dParameter[i] = ReadVSPGeomFile(FileName,NumberOfMeshNodes);
       

       // - Perturbation
       
       NewParameterValues[i] = ParameterValues[i] - Delta;
       
       printf("Creating vsp geom file... \n");fflush(NULL);
       
       CreateVSPGeometry(FileName,NumberOfDesignVariables,NewParameterValues);
       
       printf("Reading in vspgeom mesh... \n");fflush(NULL);
 
       MeshMinus = ReadVSPGeomFile(FileName,NumberOfMeshNodes);
       
       printf("Using FD to calculate mesh gradient ... for %d nodes \n",NumberOfMeshNodes);
       
       // Calculate derivative using central differences
       
       for ( j = 1 ; j <= 3*NumberOfMeshNodes ; j++ ) {
          
        //  dMesh_dParameter[i][j] = ( dMesh_dParameter[i][j] - dMesh_dParameter[0][j] )/Delta;

          dMesh_dParameter[i][j] = ( dMesh_dParameter[i][j] - MeshMinus[j] )/(2.*Delta);

       }
       
       delete [] MeshMinus;
         
    }

    return dMesh_dParameter;
 
}

/*##############################################################################
#                                                                              #
#                    DeleteOpenVSPGeometryGradients                            #
#                                                                              #
##############################################################################*/

void DeleteOpenVSPGeometryGradients(double **dMesh_dParameter, int NumberOfDesignVariables)
{
 
    int i;
    
    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {

       delete [] dMesh_dParameter[i];
       
    }
    
    delete [] dMesh_dParameter;
    
}
/*##############################################################################
#                                                                              #
#                                ReadVSPGeomFile                               #
#                                                                              #
##############################################################################*/

double *ReadVSPGeomFile(char *FileName, int &NumberOfMeshNodes)
{
 
    int i;
    double *MeshNodesXYZ;
    char VSPGeomFileName[2000];
    FILE *VSPGeomFile;

    // Open the OpenVSP des file

    sprintf(VSPGeomFileName,"%s.vspgeom",FileName);
    
    printf("Opening: %s \n",VSPGeomFileName);fflush(NULL);

    if ( (VSPGeomFile = fopen(VSPGeomFileName, "r")) == NULL ) {
    
       printf("Could not open the OpenVSP vspgeom file! \n");
    
       exit(1);
    
    }
    
    // Read in xyz data
    
    fscanf(VSPGeomFile,"%d",&NumberOfMeshNodes);

    printf("NumberOfMeshNodes: %d \n",NumberOfMeshNodes);
        
    // Read in xyz data
        
    MeshNodesXYZ = new double[3*NumberOfMeshNodes + 1];
     
    for ( i = 1 ; i <= NumberOfMeshNodes ; i++ ) {
       
       fscanf(VSPGeomFile,"%lf %lf %lf \n",
              &(MeshNodesXYZ[3*i-2]),
              &(MeshNodesXYZ[3*i-1]),
              &(MeshNodesXYZ[3*i  ]));

    }  
    
    fclose(VSPGeomFile);
    
    return MeshNodesXYZ;

}

/*##############################################################################
#                                                                              #
#                                   Normalize                                  #
#                                                                              #
##############################################################################*/

double Normalize(double *Vector, int Length)
{
   
   int i;
   double Mag;
   
   Mag = 0.;
   
   for ( i = 1 ; i <= Length ; i++ ) {
      
      Mag += Vector[i]*Vector[i];
      
   }
   
   Mag = sqrt(Mag);
   
   for ( i = 1 ; i <= Length ; i++ ) {
      
      Vector[i] /= Mag;
      
   }
   
   return Mag;
   
}

/*##############################################################################
#                                                                              #
#                                   CGState                                    #
#                                                                              #
##############################################################################*/

void CGState(double *Old, double *New, int Length)
{
   
   int i;
   double Dot;
   
   Dot = 0.;
   
   for ( i = 1 ; i <= Length ; i++ ) {
      
      Dot += Old[i]*New[i];
      
   }   
   
   for ( i = 1 ; i <= Length ; i++ ) {
      
      New[i] -= Dot * Old[i];
      
   }      
   
}

/*##############################################################################
#                                                                              #
#                       SteepestDescentOptimizer                               #
#                                                                              #
##############################################################################*/

void SteepestDescentOptimizer(char *FileName, int NumberOfParameterValues, double *ParameterValues, double *Gradient, VSP_OPTIMIZER &Optimizer)
{
   
    int j, NumberOfNodes;
    double *NodeXYZ, StepSize;
  
    StepSize = 5.;
           
    for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
  
       ParameterValues[j] -= StepSize * Gradient[j];
       
    }
    
    // Update the OpenVSP geometry based on these new parameters
    
    CreateVSPGeometry(FileName,NumberOfParameterValues,ParameterValues);

    // Read in the VSPGEOM mesh
    
    NodeXYZ = ReadVSPGeomFile(FileName,NumberOfNodes);
    
    // Update the solver and adjoint meshes
    
    Optimizer.UpdateGeometry(NodeXYZ);
    
    delete [] NodeXYZ;
             
}

/*##############################################################################
#                                                                              #
#                       ConjugateGradientOptimizer                             #
#                                                                              #
##############################################################################*/

void ConjugateGradientOptimizer(char *FileName, int NumberOfParameterValues, double *ParameterValues, double *Gradient, VSP_OPTIMIZER &Optimizer)
{
   
    int j, NumberOfNodes, Iter, Done;
    double *NodeXYZ, StepSize, Magnitude, OldMagnitude, *OldGradient, *CurrentParameterValues;
  
    // Normalize the gradient
    
    Magnitude = OldMagnitude = Normalize(Gradient,NumberOfParameterValues);
    
    OldGradient = new double[NumberOfParameterValues + 1];
    
    CurrentParameterValues = new double[NumberOfParameterValues + 1];
        
    for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
  
       OldGradient[j] = Gradient[j];
       
       CurrentParameterValues[j] = ParameterValues[j];
       
    }    
    
    StepSize = 0.1*Magnitude;
    
    Done = 0;

    Iter = 1;
    
    while ( !Done ) {
       
       if ( Iter > 1 ) CGState(OldGradient,Gradient,NumberOfParameterValues); // CG correction

       for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
       
          CurrentParameterValues[j] = ParameterValues[j] + StepSize * Gradient[j];
          
       }
    
       // Update the OpenVSP geometry based on these new parameters
       
       CreateVSPGeometry(FileName,NumberOfParameterValues,ParameterValues);
       
       // Read in the VSPGEOM mesh
    
       NodeXYZ = ReadVSPGeomFile(FileName,NumberOfNodes);
       
       // Update the solver and adjoint meshes
       
       Optimizer.UpdateGeometry(NodeXYZ);
       
       delete [] NodeXYZ;
       
    }
                   
}
   
/*##############################################################################
#                                                                              #
#                            TestCase_1                                        #
#                                                                              #
#        Single, SCALAR, optimization function for panel solve                 #
#                                                                              #
##############################################################################*/

int TestCase_10(char *FileName)
{
    int i, p;
    double Function;
    char GradientFileName[2000];
    FILE *GRADFile;
    
    printf("Running test case #1... \n");fflush(NULL);
       
    VSP_OPTIMIZER Optimizer;

    Optimizer.NumberOfThreads() = 1;
    
    Optimizer.DoUnsteadyAnalysis() = 0;
    
    Optimizer.NumberOfOptimizationFunctions() = 1;
    
    Optimizer.OptimizationFunction(1) = OPT_CMY_TOTAL;

    Optimizer.SetArrayOffSetType(DEFAULT_ARRAYS);

    Optimizer.Setup(FileName);

    Optimizer.SetMachNumber(0.0);
       
    Optimizer.SetAoADegrees(5.);
       
    Optimizer.SetBetaDegrees(0.);
       
    Optimizer.SetVinf(1.);
       
    Optimizer.SetDensity(1.);
       
    Optimizer.SetReCref(1000000.);
       
    Optimizer.SetRotationalRate_p(0.);
       
    Optimizer.SetRotationalRate_q(0.);
       
    Optimizer.SetRotationalRate_r(0.);

    Optimizer.Solve();
   
    // Open the gradient file

    sprintf(GradientFileName,"%s.opt.gradient",FileName);

    if ( (GRADFile = fopen(GradientFileName, "w")) == NULL ) {
    
       printf("Could not open the gradient output file! \n");
    
       exit(1);
    
    }
           
    // Output the gradient data

    fprintf(GRADFile,"%d \n",Optimizer.NumberOfNodes());fflush(NULL);
    
    for ( p = 1 ; p <= Optimizer.NumberOfOptimizationFunctions() ; p++ ) {
       
       Optimizer.GetFunctionValue(p,Function);

       fprintf(GRADFile,"Function: %d value: %f \n",p,Function);fflush(NULL);
   
                       //1234567890    1234567890 1234567890 1234567890    1234567890 1234567890 1234567890    
       
       fprintf(GRADFile,"   Node            X          Y          Z            dFdX       dFdY       dFdZ \n");
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          fprintf(GRADFile,"%10d   %10.6e %10.6e %10.6e   %10.6e %10.6e %10.6e \n",
           i,
           Optimizer.NodeX(i),
           Optimizer.NodeY(i),
           Optimizer.NodeZ(i),
           Optimizer.GradientX(p,i),
           Optimizer.GradientY(p,i),
           Optimizer.GradientZ(p,i));
           
           fflush(NULL);
       }
       
       fprintf(GRADFile," \n\n\n");
       
    }     
     
    fclose(GRADFile);
    
    // Gradient with respect to input values 
    
    printf("\n\n\n\n");
    printf("Gradient with respect to Alpha   : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_ALPHA  ));
    printf("Gradient with respect to Beta    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_BETA   ));
    printf("Gradient with respect to Mach    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_MACH   ));
    printf("Gradient with respect to Vinf    : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_VINF   ));
    printf("Gradient with respect to Density : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_DENSITY));
    printf("Gradient with respect to ReCref  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_RECREF ));
    printf("Gradient with respect to P rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_P_RATE ));
    printf("Gradient with respect to Q rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_Q_RATE ));
    printf("Gradient with respect to R rate  : %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_R_RATE ));
  
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                            TestCase_11                                       #
#                                                                              #
#        Forward and adjoint solves, given user supplied vectors, calculation  #
#        of mesh and residual-vector product partial gradients                 #
#                                                                              #
##############################################################################*/

int TestCase_11(char *FileName)
{
    int i, Case;
    double *ForwardVecIn, *ForwardVecOut, *ForwardRHS;
    double *AdjointVecIn, *AdjointVecOut, *AdjointRHS, *AdjointPsi;
    double *pF_pMesh, *pF_pInputVariable;
    double *pR_pMesh, *pR_pInputVariable;
        
    printf("Running test case #11... \n");fflush(NULL);
       
    VSP_OPTIMIZER Optimizer;

    Optimizer.NumberOfThreads() = 1;
    
    Optimizer.DoUnsteadyAnalysis() = 0;
    
    Optimizer.NumberOfOptimizationFunctions() = 1;
    
    Optimizer.OptimizationFunction(1) = OPT_CL_INVISCID;

    Optimizer.SetArrayOffSetType(DEFAULT_ARRAYS);

    Optimizer.Setup(FileName);

    Optimizer.SetMachNumber(0.11);
       
    Optimizer.SetAoADegrees(5.);
       
    Optimizer.SetBetaDegrees(0.);
       
    Optimizer.SetVinf(100.);
       
    Optimizer.SetDensity(1.e-5);
       
    Optimizer.SetReCref(1000000.);
       
    Optimizer.SetRotationalRate_p(0.);
       
    Optimizer.SetRotationalRate_q(0.);
       
    Optimizer.SetRotationalRate_r(0.);
    
    // Forward solution vectors and right hand side
    
    ForwardVecIn = new double[Optimizer.NumberOfLoops() + 1];

    ForwardVecOut = new double[Optimizer.NumberOfLoops() + 1];
    
    ForwardRHS = new double[Optimizer.NumberOfLoops() + 1];
    
    // Adjoint solution vectors and right hand side
    
    AdjointVecIn = new double[Optimizer.NumberOfAdjointEquations() + 1];

    AdjointVecOut = new double[Optimizer.NumberOfAdjointEquations() + 1];
    
    AdjointRHS = new double[Optimizer.NumberOfAdjointEquations() + 1];
    
    AdjointPsi = new double[Optimizer.NumberOfAdjointEquations() + 1];

    // Partial gradients
    
    pF_pMesh = new double[3*Optimizer.NumberOfNodes() + 1];
    pF_pInputVariable = new double[OPT_GRADIENT_NUMBER_OF_INPUTS + 2];   
    
    pR_pMesh = new double[3*Optimizer.NumberOfNodes() + 1];
    pR_pInputVariable = new double[OPT_GRADIENT_NUMBER_OF_INPUTS + 2];
        
    // Get the matrix vector product, given user supplied vector... also returns the right handside vector
            
       for ( i = 1 ; i <= Optimizer.NumberOfLoops() ; i++ ) {
       
          ForwardVecIn[i] = 1.;
          
          ForwardVecOut[i] = 0.;
          
          ForwardRHS[i] = 0.;
          
       }
       
       Optimizer.CalculateMatrixVectorProductAndRightHandSide(ForwardVecIn, ForwardVecOut, ForwardRHS);
       
       for ( i = 1 ; i <= Optimizer.NumberOfLoops() ; i++ ) {
       
          printf("ForwardVecIn: %e --- ForwardVecOut: %e --- ForwardRHS: %e \n",
                 ForwardVecIn[i], 
                 ForwardVecOut[i],
                 ForwardRHS[i]);
                 
       }    

    // Calculate a FORWARD matrix vector product, and RHS.. we just set the input vector to 1's here...
  
       for ( i = 1 ; i <= Optimizer.NumberOfAdjointEquations() ; i++ ) {
       
          AdjointVecIn[i] = 1.;
          
          AdjointVecOut[i] = 0.;
          
          AdjointRHS[i] = 0.;
          
          AdjointPsi[i] = 0.;
          
       }    
       
       
       Optimizer.CalculateAdjointMatrixVectorProductAndRightHandSide(AdjointVecIn, AdjointVecOut, AdjointRHS);
       
       for ( i = 1 ; i <= Optimizer.NumberOfAdjointEquations() ; i++ ) {
       
          printf("AdjointVecIn: %e --- AdjointVecOut: %e --- AdjointRHS: %e \n",
                 AdjointVecIn[i], 
                 AdjointVecOut[i],
                 AdjointRHS[i]);
                 
       } 
    
    // Now do a solve... forward and adjoint
    
       Optimizer.Solve();
       
       for ( i = 1 ; i <= Optimizer.NumberOfLoops() ; i++ ) {
       
          ForwardVecOut[i] = 0.;
          
          ForwardRHS[i] = 0.;
          
       }
    
    // Calculate a FORWARD matrix vector product, and get RHS... we use the solution vector from above for the input vector...
    // so we expect the output vector to the right hand side
    
       // Get the forward solution vector
       
       Optimizer.GetForwardSolutionVector(ForwardVecIn);
       
       // Now do the matrix vector product, and get the rhs vector
       
       Optimizer.CalculateMatrixVectorProductAndRightHandSide(ForwardVecIn, ForwardVecOut, ForwardRHS);
       
       // Compare the matrix vector product (vecout) with the rhs vector... the should be the same
       
       for ( i = 1 ; i <= Optimizer.NumberOfLoops() ; i++ ) {
       
          printf("ForwardVecIn: %e --- ForwardVecOut: %e --- ForwardRHS: %e \n",
                 ForwardVecIn[i], 
                 ForwardVecOut[i],
                 ForwardRHS[i]);
                 
       }   
       
    // Calculate an ADJOINT matrix vector product, and get RHS
       
       for ( i = 1 ; i <= Optimizer.NumberOfAdjointEquations() ; i++ ) {
       
          AdjointVecOut[i] = 0.;
          
          AdjointRHS[i] = 0.;
          
          AdjointPsi[i] = 0.;
          
       }    
             
       // Get the adjoint solution vector from the above solve
       
       Optimizer.GetAdjointSolutionVector(AdjointVecIn);
       
       // Calculate matrix vector product, and get RHS... we expect the matrix vector product to be equal to the rhs vector
       
       Optimizer.CalculateAdjointMatrixVectorProductAndRightHandSide(AdjointVecIn, AdjointVecOut, AdjointRHS);
       
       for ( i = 1 ; i <= Optimizer.NumberOfAdjointEquations() ; i++ ) {
       
          printf("AdjointVecIn: %e --- AdjointVecOut: %e --- AdjointRHS: %e \n",
                 AdjointVecIn[i], 
                 AdjointVecOut[i],
                 AdjointRHS[i]);
                 
       } 
    
    // Now solve the adjoint system.. here we supply the RHS from the above calculation... so we expect
    // Psi to come back as the adjoint solution we obtained previously above (AdjointVecOut)
    
       Optimizer.SolveAdjointLinearSystem(AdjointPsi,AdjointRHS);
       
       for ( i = 1 ; i <= Optimizer.NumberOfAdjointEquations() ; i++ ) {
       
          printf("AdjointVecOut: %e --- Psi: %e \n",
                 AdjointVecOut[i],
                 AdjointPsi[i]);
                 
       }        
    
    // Calculate partials wrt mesh and input variables
    
    Case = 1;

       Optimizer.CalculateOptimizationFunctionPartials(Case, pF_pMesh, pF_pInputVariable);  
          
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
       
          printf("pF_pMesh: %e %e %e       %e %e %e \n",
                 Optimizer.pGradient_wrtMeshX(i),
                 Optimizer.pGradient_wrtMeshY(i),
                 Optimizer.pGradient_wrtMeshZ(i),     
                 pF_pMesh[3*i-2],
                 pF_pMesh[3*i-1],
                 pF_pMesh[3*i  ]);
                 
       }        
       
       for ( i = 1 ; i <= OPT_GRADIENT_NUMBER_OF_INPUTS + 1 ; i++ ) {
       
          printf("pF_pInputVariable: %e    %e\n",
                 Optimizer.pF_dInputVariable(i),
                 pF_pInputVariable[i]);
                 
       }     

    // Calculate partials, residual vector products... here we pass in the Psi
    // solution vector we accessed previously... 

       Optimizer.CalculateAdjointResidualPartialProducts(AdjointPsi, pR_pMesh, pR_pInputVariable);  
          
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
       
          printf("pR_pMesh: %e %e %e       %e %e %e \n",
                 Optimizer.pGradient_wrtResdiualX(i),
                 Optimizer.pGradient_wrtResdiualY(i),
                 Optimizer.pGradient_wrtResdiualZ(i),
                 pR_pMesh[3*i-2],
                 pR_pMesh[3*i-1],
                 pR_pMesh[3*i  ]);
                 
       }        
       
       for ( i = 1 ; i <= OPT_GRADIENT_NUMBER_OF_INPUTS + 1 ; i++ ) {
       
          printf("pR_pInputVariable: %e    %e\n",
                 Optimizer.pR_dInputVariable(i),       
                 pR_pInputVariable[i]);
                 
       }  
        
    // Compare total gradients that were previously calculated with the sum of the partials
    
             //1234567890   1234567890 1234567890 1234567890   1234567890 1234567890 1234567890   1234567890 1234567890 1234567890    
       
       printf("   Node            X          Y          Z            dFdX       dFdY       dFdZ       dFdX_sum   dFdY_sum   dFdZ_sum \n");
       
       for ( i = 1 ; i <= Optimizer.NumberOfNodes() ; i++ ) {
          
          printf("%10d   %10.6e %10.6e %10.6e   %10.6e %10.6e %10.6e   %10.6e %10.6e %10.6e \n",
           i,
           Optimizer.NodeX(i),
           Optimizer.NodeY(i),
           Optimizer.NodeZ(i),
           Optimizer.GradientX(i),
           Optimizer.GradientY(i),
           Optimizer.GradientZ(i),
           pF_pMesh[3*i-2] - pR_pMesh[3*i-2],
           pF_pMesh[3*i-1] - pR_pMesh[3*i-1],
           pF_pMesh[3*i  ] - pR_pMesh[3*i  ]);                  
                      
       }
       
       // Gradient with respect to input values 
       
       printf("\n\n\n\n");
       printf("Gradient with respect to Alpha   : %e ... %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_ALPHA  ), pF_pInputVariable[OPT_GRADIENT_WRT_ALPHA  ] - pR_pInputVariable[OPT_GRADIENT_WRT_ALPHA  ]);
       printf("Gradient with respect to Beta    : %e ... %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_BETA   ), pF_pInputVariable[OPT_GRADIENT_WRT_BETA   ] - pR_pInputVariable[OPT_GRADIENT_WRT_BETA  ]);
       printf("Gradient with respect to Mach    : %e ... %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_MACH   ), pF_pInputVariable[OPT_GRADIENT_WRT_MACH   ] - pR_pInputVariable[OPT_GRADIENT_WRT_MACH  ]);
       printf("Gradient with respect to Vinf    : %e ... %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_VINF   ), pF_pInputVariable[OPT_GRADIENT_WRT_VINF   ] - pR_pInputVariable[OPT_GRADIENT_WRT_VINF  ]);
       printf("Gradient with respect to Density : %e ... %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_DENSITY), pF_pInputVariable[OPT_GRADIENT_WRT_DENSITY] - pR_pInputVariable[OPT_GRADIENT_WRT_DENSITY  ]);
       printf("Gradient with respect to ReCref  : %e ... %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_RECREF ), pF_pInputVariable[OPT_GRADIENT_WRT_RECREF ] - pR_pInputVariable[OPT_GRADIENT_WRT_RECREF  ]);
       printf("Gradient with respect to P rate  : %e ... %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_P_RATE ), pF_pInputVariable[OPT_GRADIENT_WRT_P_RATE ] - pR_pInputVariable[OPT_GRADIENT_WRT_P_RATE  ]);
       printf("Gradient with respect to Q rate  : %e ... %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_Q_RATE ), pF_pInputVariable[OPT_GRADIENT_WRT_Q_RATE ] - pR_pInputVariable[OPT_GRADIENT_WRT_Q_RATE  ]);
       printf("Gradient with respect to R rate  : %e ... %e \n",Optimizer.dF_dInputVariable(OPT_GRADIENT_WRT_R_RATE ), pF_pInputVariable[OPT_GRADIENT_WRT_R_RATE ] - pR_pInputVariable[OPT_GRADIENT_WRT_R_RATE  ]);
                                    
    return 1;
    
}


