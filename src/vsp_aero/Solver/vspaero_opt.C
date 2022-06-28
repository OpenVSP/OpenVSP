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
int TestCase_1(char *FileName);
int TestCase_2(char *FileName);
int TestCase_3(char *FileName);
int TestCase_4(char *FileName);
int TestCase_5(char *FileName);

/*##############################################################################
#                                                                              #
#                                 main                                         #
#                                                                              #
##############################################################################*/

int main(int argc, char **argv)
{
 
    int TestCase;   
    char *FileName, GradientFileName[2000];
    
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
        
    else {
       
       printf("Unknown test case! \n");
       fflush(NULL);
       exit(1);
       
    }
    
    return 1;     
           
}

/*##############################################################################
#                                                                              #
#                                 main                                         #
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

    Optimizer.NumberOfThreads() = 4;
    
    Optimizer.DoUnsteadyAnalysis() = 0;
    
    Optimizer.NumberOfOptimizationFunctions() = 1;
    
    Optimizer.OptimizationFunction(1) = OPT_CL;

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
    
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                                 main                                         #
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
    
    Optimizer.OptimizationFunction(1) = OPT_CL;
    Optimizer.OptimizationFunction(2) = OPT_CD;
    Optimizer.OptimizationFunction(3) = OPT_CMy;
    
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
    
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                                 main                                         #
#                                                                              #
##############################################################################*/

int TestCase_3(char *FileName)
{
    int i, p;
    double Function, *Vec;
    char GradientFileName[2000];
    FILE *GRADFile;
    
    printf("Running test case #3... \n");fflush(NULL);
       
    VSP_OPTIMIZER Optimizer;

    Optimizer.NumberOfThreads() = 4;
    
    Optimizer.DoUnsteadyAnalysis() = 0;
    
    Optimizer.NumberOfOptimizationFunctions() = 1;
    
    Optimizer.OptimizationFunction(1) = OPT_WING_LOAD;
    
    Optimizer.Setup(FileName);
    
    // User supplied input gradient vector
    
    Vec = new double[Optimizer.OptimizationFunctionLength(1) + 1];
    
    for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(1) ; i++ ) {
       
       Vec[i] = 1.;
       
    }

    Optimizer.Solve(Vec);
   
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
    
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                                 main                                         #
#                                                                              #
##############################################################################*/

int TestCase_4(char *FileName)
{
    int i, p;
    double Function, *Vec;
    char GradientFileName[2000];
    FILE *GRADFile;
    
    printf("Running test case #4... \n");fflush(NULL);
       
    VSP_OPTIMIZER Optimizer;

    Optimizer.NumberOfThreads() = 4;
    
    Optimizer.DoUnsteadyAnalysis() = 1;
    
    Optimizer.NumberOfOptimizationFunctions() = 1;
    
    Optimizer.OptimizationFunction(1) = OPT_CD;
    
    Optimizer.Setup(FileName);
    
    // User supplied input gradient vector
    
    Vec = new double[Optimizer.OptimizationFunctionLength(1) + 1];
    
    for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(1) ; i++ ) {
       
       Vec[i] = 1.;
       
    }

    Optimizer.Solve(Vec);
   
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
    
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                                 main                                         #
#                                                                              #
##############################################################################*/

int TestCase_5(char *FileName)
{
    int i, p;
    double *Vec;
    double *VecIn, *VecOut, *RHS;
    double *AdjointVecIn, *AdjointVecOut, *AdjointRHS;
    
    printf("Running test case #5... \n");fflush(NULL);
       
    VSP_OPTIMIZER Optimizer;

    Optimizer.NumberOfThreads() = 4;
    
    Optimizer.DoUnsteadyAnalysis() = 0;
    
    Optimizer.NumberOfOptimizationFunctions() = 1;
    
    Optimizer.OptimizationFunction(1) = OPT_WING_LOAD;
    
    Optimizer.Setup(FileName);
    
    // User supplied input gradient vector
    
    Vec = new double[Optimizer.OptimizationFunctionLength(1) + 1];
    
    for ( i = 1 ; i <= Optimizer.OptimizationFunctionLength(1) ; i++ ) {
       
       Vec[i] = 1.;
       
    }

    Optimizer.Solve(Vec);
    
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
