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

// VSPAERO header

#include "VSP_Solver.H"

// OpenVSP headers

#include "VSP_Geom_API.h"


class PARAMETER_DATA {
   
public:
   
   double *ParameterValues;
   char **ParameterNames;

};

using namespace VSPAERO_SOLVER;

// The code...

int main(int argc, char **argv);

int TestCase_1(char *GeometryFileName, int SolverMode); // Dead stupid optimization of a VLM wing...

PARAMETER_DATA *ReadOpenVSPDesFile(char *FileName, int &NumberOfDesignVariables);

double *CreateVSPGeometry(char *FileName, int NumberOfDesignVariables, char **ParameterNames, double *ParameterValues, VSP_SOLVER &VSP_VLM);

void SaveVSPGeomFile(char *FileName, int NumberOfDesignVariables, char **ParameterNames, double *ParameterValues, VSP_SOLVER &VSP_VLM);

double **CalculateOpenVSPGeometryGradients(char *FileName, int NumberOfDesignVariables, PARAMETER_DATA *ParameterData, VSP_SOLVER &VSP_VLM);

void DeleteMeshGradients(int NumberOfDesignVariables, double **dMesh_dParameter);

double Normalize(double *Vector, int Length);

void CGState(double *Old, double *New, int Length);

/*##############################################################################
#                                                                              #
#                                 main                                         #
#                                                                              #
##############################################################################*/

int main(int argc, char **argv)
{
 
    int TestCase;
    double *MeshGeom;   
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
    
    // Dead stupid optimization of a wing...

    if ( TestCase == 1 ) {
       
       return TestCase_1(FileName, VLM_MODEL);    
       
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
# Simple wing optimization                                                     #
#                                                                              #
##############################################################################*/

int TestCase_1(char *GeometryFileName, int SolverMode)
{
   
    int Case, NumberOfThreads;
    
    int i, j, Iter, IterMax, NumberOfParameterValues, NumberOfNodes, k, Done;
    double CL, CD, CM, CLreq, CDreq, CMreq, *NodeXYZ, *dFdMesh[3], *dF_dParameter, *Gradient, *GradientOld, F, Fnew, Delta, StepSize;
    double Lambda_1, Lambda_2, Lambda_3, **dMesh_dParameter;
    double *ParameterValues, *NewParameterValues, *BestParameterValues, Time0, TotalTime, *MeshNodes;
    char HistoryFileName[2000], CommandLine[2000], OptimizationSetupFileName[2000], **ParameterNames;
    char OpenVSP_FileName[2000], OpenVSP_VSPGeomFileName[2000];
    FILE *HistoryFile, *OptimizationSetupFile;
    
    int NumberOfForwardSolves, NumberOfAdjointSolves, NumberOfGeometryUpdates;
    double Time, ForwardSolveTime, AdjointSolveTime, GeometryUpdateTime;

    // OPENMP stuff
    
#ifdef VSPAERO_OPENMP
   
    NumberOfThreads = 4;
   
    omp_set_num_threads(NumberOfThreads);
    
    NumberOfThreads = omp_get_max_threads();

    printf("NumberOfThreads_: %d \n",NumberOfThreads);
    
#else

    NumberOfThreads = 1;

    printf("Single threaded build.\n");

#endif

    // OpenVSP parameter data
            
    PARAMETER_DATA *ParameterData;
    
    // VSPAERO Solver
        
    VSP_SOLVER VSP_VLM;

    // Zero out statistics
    
    NumberOfForwardSolves = NumberOfAdjointSolves = NumberOfGeometryUpdates = 0;
    
    ForwardSolveTime = AdjointSolveTime = GeometryUpdateTime = 0.;

    printf("Running wing optimization... \n");fflush(NULL);

    // Read in the OpenVSP geometry
    
    snprintf(OpenVSP_FileName,sizeof(OpenVSP_FileName)*sizeof(char),"%s.vsp3",GeometryFileName);

    vsp::ReadVSPFile( OpenVSP_FileName );
    
    // Initialize the starting geometry
    
    Time = Time0 = myclock();
    
    if ( SolverMode == VLM_MODEL ) {
  
     //old way vsp::SetAnalysisInputDefaults( "DegenGeomMesh" );
     //old way 
     //old way vsp::SetIntAnalysisInput("DegenGeomMesh", "Set", {vsp::SET_ALL}, 0);
     //old way 
     //old way vsp::SetIntAnalysisInput("DegenGeomMesh", "DegenGeomMeshType", {vsp::NGON_MESH_TYPE}, 0);
     //old way 
     //old way string compgeom_resid = vsp::ExecAnalysis( "DegenGeomMesh" );    
     //old way 
     //old way sprintf(OpenVSP_VSPGeomFileName,"%s.vspgeom",GeometryFileName);
     //old way 
     //old way vsp::ExportFile( OpenVSP_VSPGeomFileName, vsp::SET_ALL, vsp::EXPORT_VSPGEOM );
    
       vsp::SetAnalysisInputDefaults( "VSPAEROComputeGeometry" );
       
       vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "GeomSet", {vsp::SET_NONE}, 0);       // Thick geometry -- "Panel"
       
       vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "ThinGeomSet", {vsp::SET_ALL}, 0);  // Thin geometry -- "VLM"
       
       int m_SymFlagVec = 0;
       
       vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "Symmetry", {m_SymFlagVec}, 0);

       vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "CullFracFlag", {1}, 0);
       
       vsp::SetDoubleAnalysisInput("VSPAEROComputeGeometry", "CullFrac", {0.1}, 0);

       string compgeom_resid = vsp::ExecAnalysis( "VSPAEROComputeGeometry" );
   
    }
    
    else if ( SolverMode == PANEL_MODEL ) {
       
       vsp::SetAnalysisInputDefaults( "VSPAEROComputeGeometry" );
       
       vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "GeomSet", {vsp::SET_ALL}, 0);       // Thick geometry -- "Panel"
       
       vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "ThinGeomSet", {vsp::SET_NONE}, 0);  // Thin geometry -- "VLM"
       
       int m_SymFlagVec = 0;
       
       vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "Symmetry", {m_SymFlagVec}, 0);

       vsp::SetIntAnalysisInput("VSPAEROComputeGeometry", "CullFracFlag", {1}, 0);
       
       vsp::SetDoubleAnalysisInput("VSPAEROComputeGeometry", "CullFrac", {0.1}, 0);

       string compgeom_resid = vsp::ExecAnalysis( "VSPAEROComputeGeometry" );
       
    }
    
    else {
       
       printf("Unknown OPENVSP/VSPAERO Solve type! \n");
       
       fflush(NULL);exit(1);
       
    }
       
    GeometryUpdateTime += myclock() - Time;
    
    // Initialize VSPAERO solver settings

    VSP_VLM.Sref() = 300.;

    VSP_VLM.Cref() =   5.;

    VSP_VLM.Bref() =  60.;
    
    VSP_VLM.Xcg() = 2.;

    VSP_VLM.Ycg() = 0.;

    VSP_VLM.Zcg() = 0.;
    
    VSP_VLM.Mach() = 0.11;
    
    VSP_VLM.AngleOfAttack() = 5. * TORAD;

    VSP_VLM.AngleOfBeta() = 0. * TORAD;
    
    VSP_VLM.Vinf() = VSP_VLM.Vref() = 100.;
    
    VSP_VLM.Density() = 1.e-5;
    
    VSP_VLM.ReCref() = 1000000.;
    
    VSP_VLM.RotationalRate_p() = 0.0;
    
    VSP_VLM.RotationalRate_q() = 0.0;
    
    VSP_VLM.RotationalRate_r() = 0.0;    

    VSP_VLM.DoSymmetryPlaneSolve() = 0;
        
    VSP_VLM.StallModelIsOn() = 0;
    
    VSP_VLM.WakeIterations() = 15;
    
    VSP_VLM.ForwardGMRESConvergenceFactor() = 1.;
    
    VSP_VLM.AdjointGMRESConvergenceFactor() = 1.;
    
    VSP_VLM.NonLinearConvergenceFactor() = 1.;

    VSP_VLM.WakeRelax() = 1.;
    
    VSP_VLM.ImplicitWake() = 1;
    
    VSP_VLM.DoAdjointSolve() = 1; // This needs to be set before Setup() is called... 
    
    if ( SolverMode == VLM_MODEL   ) VSP_VLM.AdjointSolutionForceType() = ADJOINT_TOTAL_FORCES; // Only total forces gradients

    if ( SolverMode == PANEL_MODEL ) VSP_VLM.AdjointSolutionForceType() = ADJOINT_TOTAL_FORCES_USING_WAKE_FORCES; // Only total forces gradients, but based on Trefftz forces
            
    VSP_VLM.ReadFile(GeometryFileName, sizeof(GeometryFileName)*sizeof(char));

    VSP_VLM.Setup();

    // Load in the optimization parameter value list

    ParameterData = ReadOpenVSPDesFile(GeometryFileName,NumberOfParameterValues);
    
    dF_dParameter = new double[NumberOfParameterValues + 1];
    
    ParameterValues = new double[NumberOfParameterValues + 1];
    
    NewParameterValues = new double[NumberOfParameterValues + 1];
    
    BestParameterValues = new double[NumberOfParameterValues + 1];

    Gradient = new double[NumberOfParameterValues + 1];
    
    GradientOld = new double[NumberOfParameterValues + 1];

    for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
    
       GradientOld[j] = Gradient[j] = dF_dParameter[j] = 0.;
       
       ParameterValues[i] = ParameterData->ParameterValues[i];
 
    }
    
    // Calculate partials of mesh wrt parameters

    Time = myclock();
        
    dMesh_dParameter = CalculateOpenVSPGeometryGradients(GeometryFileName,NumberOfParameterValues,ParameterData,VSP_VLM);
    
    GeometryUpdateTime += myclock() - Time;
    
    NumberOfGeometryUpdates += 2*NumberOfParameterValues + 1;

    printf("Finished calculate mesh gradients... \n");fflush(NULL);

    // Allocate some space for the derivatives 
    
    dFdMesh[0] = new double[VSP_VLM.VSPGeom().Grid(0).NumberOfSurfaceNodes() + 1];
    dFdMesh[1] = new double[VSP_VLM.VSPGeom().Grid(0).NumberOfSurfaceNodes() + 1];
    dFdMesh[2] = new double[VSP_VLM.VSPGeom().Grid(0).NumberOfSurfaceNodes() + 1];
    
    // Open the history file

    snprintf(HistoryFileName,sizeof(HistoryFileName)*sizeof(char),"%s.opt.history",GeometryFileName);

    if ( (HistoryFile = fopen(HistoryFileName, "w")) == NULL ) {
    
       printf("Could not open the optimization history output file! \n");
    
       exit(1);
    
    }
    
    // Clean up any old opt adb files
    
    snprintf(CommandLine,sizeof(CommandLine)*sizeof(char),"rm %s.opt.*.adb",GeometryFileName);
    
    system(CommandLine);
      
    // Design requirements
    
    CLreq = 0.55; 
    CDreq = 0.0;
    CMreq = 0.0;
              
    // Maximum number of design steps
    
    IterMax = 50;
    
                        //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 
    fprintf(HistoryFile,"    Iter        CL         CD         CM         F        GradF      1DSteps   StepSize     Time   \n");

    k = 0;

    Delta = StepSize = 0.;

    TotalTime = myclock() - Time0;
    
    Case = 0;
    
    for ( Iter = 1 ; Iter <= IterMax ; Iter++ ) {

       // Solve the forward problem and the adjoint
       
       printf("Running VSPAERO forward and adjoint solvers... \n");fflush(NULL);
       
       Case++;

       VSP_VLM.DoAdjointSolve() = 1;
       
       Time = myclock();
       
       VSP_VLM.Solve(Case);

       AdjointSolveTime += myclock() - Time;
       
       NumberOfForwardSolves++;
       
       if ( VSP_VLM.AdjointSolutionForceType() == ADJOINT_INVISCID_AND_VISCOUS_FORCES ) NumberOfAdjointSolves += 12;

       if ( VSP_VLM.AdjointSolutionForceType() == ADJOINT_INVISCID_FORCES             ) NumberOfAdjointSolves += 6;

       if ( VSP_VLM.AdjointSolutionForceType() == ADJOINT_VISCOUS_FORCES              ) NumberOfAdjointSolves += 6;

       if ( VSP_VLM.AdjointSolutionForceType() == ADJOINT_TOTAL_FORCES                ) NumberOfAdjointSolves += 6;

       // Save the .adb file for later viewing
       
       snprintf(CommandLine,sizeof(CommandLine)*sizeof(char),"cp %s.adb %s.opt.%d.adb",GeometryFileName,GeometryFileName,Iter);
       
       system(CommandLine);
       
       if ( Iter == 1 ) {
          
          // Save the .adb file for later viewing
          
          snprintf(CommandLine,sizeof(CommandLine)*sizeof(char),"cp %s.adb %s.opt.adb",GeometryFileName,GeometryFileName);
          
          system(CommandLine);
          
          // Copy over the .cases file for viewer
          
          snprintf(CommandLine,sizeof(CommandLine)*sizeof(char),"cp %s.adb.cases %s.opt.adb.cases",GeometryFileName,GeometryFileName);
          
          system(CommandLine);          
       
       }          
       
       // Grab total Cl, CD, and CM ... this included both invisicid and viscous contributions
       
       if ( SolverMode == VLM_MODEL   ) {
          
          CL = VSP_VLM.CLi() + VSP_VLM.CLo();
          CD = VSP_VLM.CDi() + VSP_VLM.CDo();
          CM = VSP_VLM.CMiy() + VSP_VLM.CMoy();
          
       }
       
       else if ( SolverMode == PANEL_MODEL ) {
          
          CL = VSP_VLM.CLiw() + VSP_VLM.CLo();
          CD = VSP_VLM.CDiw() + VSP_VLM.CDo();
          CM = VSP_VLM.CMiy() + VSP_VLM.CMoy();
          
       }
       
       else {
          
          printf("Unknown VSPAERO solver type! \n");
          fflush(NULL);exit(1);
          
       }

       // Default CL, CD, and CM weights
       
       if ( Iter == 1 ) {
          
         Lambda_1 = 1. / pow(CL-CLreq,2.);
         Lambda_2 = 1. / pow(CD-CDreq,2.);
         Lambda_3 = 1. / pow(CM-CMreq,2.);
        
       }
           
       // Calculate the final objective function and it's gradients
       // Here we have f = L1*(CL - CLreq)^2 + L2*CD^2 + L3*CM^2

       F = Lambda_1 * pow(CL - CLreq,2.) + Lambda_2 * pow(CD-CDreq,2.) + Lambda_3 * pow(CM-CMreq,2.);
              
       for ( i = 1 ; i <= VSP_VLM.VSPGeom().Grid(1).NumberOfSurfaceNodes() ; i++ ) {

          dFdMesh[0][i] = 2.*Lambda_1*(CL - CLreq)*VSP_VLM.VSPGeom().Grid(1).NodeList(i).DCLt_DX() 
                        + 2.*Lambda_2*(CD - CDreq)*VSP_VLM.VSPGeom().Grid(1).NodeList(i).DCDt_DX() 
                        + 2.*Lambda_3*(CM - CMreq)*VSP_VLM.VSPGeom().Grid(1).NodeList(i).DCMmt_DX();
                                                                        
          dFdMesh[1][i] = 2.*Lambda_1*(CL - CLreq)*VSP_VLM.VSPGeom().Grid(1).NodeList(i).DCLt_DY()
                        + 2.*Lambda_2*(CD - CDreq)*VSP_VLM.VSPGeom().Grid(1).NodeList(i).DCDt_DY()
                        + 2.*Lambda_3*(CM - CMreq)*VSP_VLM.VSPGeom().Grid(1).NodeList(i).DCMmt_DY();
                                                                         
          dFdMesh[2][i] = 2.*Lambda_1*(CL - CLreq)*VSP_VLM.VSPGeom().Grid(1).NodeList(i).DCLt_DZ() 
                        + 2.*Lambda_2*(CD - CDreq)*VSP_VLM.VSPGeom().Grid(1).NodeList(i).DCDt_DZ() 
                        + 2.*Lambda_3*(CM - CMreq)*VSP_VLM.VSPGeom().Grid(1).NodeList(i).DCMmt_DZ();
       
       }
       
       printf("Doing chain rule... \n");fflush(NULL);

       // Chain rule... calculate derivatives wrt parameters
       
       for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
          
          dF_dParameter[j] = 0;
       
          for ( i = 1 ; i <= VSP_VLM.VSPGeom().Grid(1).NumberOfSurfaceNodes() ; i++ ) {
             
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

       fprintf(HistoryFile,"%10d %10.5f %10.5f %10.5f %10.5f %10.5f %10d %10.5f %10.5f \n",Iter,CL,CD,CM,F,Delta,k,StepSize,TotalTime);

       StepSize = Delta;       
       
       if ( StepSize > 0.1 ) StepSize = 0.1;

       if ( Iter != IterMax ) {
          
          // A really bad 1D search...
          
          k = 0;
          
          Done = 0;
   
          while ( !Done && k < 10 ) {
             
             printf("\n\n\n\n\n\n At search iteration %d the current Step Size: %f \n\n\n\n\n\n ",k,StepSize);
          
             for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
             
                ParameterValues[j] -= StepSize * Gradient[j];
                
             }

             Time = myclock();
             
             NodeXYZ = CreateVSPGeometry(GeometryFileName,NumberOfParameterValues,ParameterData->ParameterNames, ParameterValues,VSP_VLM);
             
             GeometryUpdateTime += myclock() - Time;

             NumberOfGeometryUpdates++;

             // Update mesh
             
             for ( j = 1 ; j <= VSP_VLM.VSPGeom().Grid(0).NumberOfSurfaceNodes() ; j++ ) {

                VSP_VLM.VSPGeom().Grid(0).NodeList(j).x() = NodeXYZ[3*j-2];
                VSP_VLM.VSPGeom().Grid(0).NodeList(j).y() = NodeXYZ[3*j-1];
                VSP_VLM.VSPGeom().Grid(0).NodeList(j).z() = NodeXYZ[3*j  ];
      
             }
                         
             Time = myclock();
                          
             VSP_VLM.VSPGeom().UpdateMeshes();
             
             GeometryUpdateTime += myclock() - Time;

             delete [] NodeXYZ;
             
             // Do a forward solve only to evaluate the functional
             
             Case++;
             
             Time = myclock();
             
             VSP_VLM.DoAdjointSolve() = 0;
                          
             VSP_VLM.Solve(Case);
             
             ForwardSolveTime += myclock() - Time;
                          
             NumberOfForwardSolves++;
             
             CL = VSP_VLM.CLt();
             CD = VSP_VLM.CDt();
             CM = VSP_VLM.CMty();
      
             Fnew = Lambda_1 * pow(CL - CLreq,2.) + Lambda_2 * pow(CD-CDreq,2.) + Lambda_3 * pow(CM-CMreq,2.);

             printf("1D search... current F: %f ... previous F: %f \n",Fnew,F); fflush(NULL);
           
             // Just keep going until function has increased... 
             
             if ( Fnew > F ) {
                
                printf("Stopping 1D search and backing up a step... \n");                
                
                // Back up 
                
                if ( k > 0 ) {

                   for ( j = 1 ; j <= NumberOfParameterValues ; j++ ) {
                   
                      ParameterValues[j] += StepSize * Gradient[j];
                 
                   }
    
                   Time = myclock();
                   
                   NodeXYZ = CreateVSPGeometry(GeometryFileName,NumberOfParameterValues,ParameterData->ParameterNames, ParameterValues, VSP_VLM);
                  
                   GeometryUpdateTime += myclock() - Time;
                  
                   NumberOfGeometryUpdates++;
                  
                   // Update mesh
                   
                   for ( j = 1 ; j <= VSP_VLM.VSPGeom().Grid(0).NumberOfSurfaceNodes() ; j++ ) {
                   
                      VSP_VLM.VSPGeom().Grid(0).NodeList(j).x() = NodeXYZ[3*j-2];
                      VSP_VLM.VSPGeom().Grid(0).NodeList(j).y() = NodeXYZ[3*j-1];
                      VSP_VLM.VSPGeom().Grid(0).NodeList(j).z() = NodeXYZ[3*j  ];
                   
                   }
                                
                   // Update the solver and adjoint meshes
                   
                   VSP_VLM.VSPGeom().UpdateMeshes();
                   
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

       TotalTime = myclock() - Time0;
       
    //   // Update the mesh gradients
    //   
    //   dMesh_dParameter = CalculateOpenVSPGeometryGradients(GeometryFileName,NumberOfParameterValues,ParameterData,VSP_VLM);

    }
    
    // Save the final state
    
    SaveVSPGeomFile(GeometryFileName, NumberOfParameterValues,ParameterData->ParameterNames, ParameterValues, VSP_VLM);
    
    // Output some solver stats...

    printf("\n\n");
    printf("NumberOfForwardSolves:   %d \n",NumberOfForwardSolves);
    printf("NumberOfAdjointSolves:   %d \n",NumberOfAdjointSolves);
    printf("NumberOfGeometryUpdates: %d \n",NumberOfGeometryUpdates);
    printf("\n\n");
    printf("Forward  Solve  Time %f seconds \n",ForwardSolveTime);
    printf("Adoint   Solve  Time %f seconds \n",AdjointSolveTime);
    printf("Geometry Update Time %f seconds \n",GeometryUpdateTime);
    printf("Total Optimization time: %f seconds \n",TotalTime);
        
    fprintf(HistoryFile,"\n\n");
    fprintf(HistoryFile,"NumberOfForwardSolves:   %d \n",NumberOfForwardSolves);
    fprintf(HistoryFile,"NumberOfAdjointSolves:   %d \n",NumberOfAdjointSolves);
    fprintf(HistoryFile,"NumberOfGeometryUpdates: %d \n",NumberOfGeometryUpdates);
    fprintf(HistoryFile,"\n\n");
    fprintf(HistoryFile,"Forward  Solve  Time %f seconds \n",ForwardSolveTime);
    fprintf(HistoryFile,"Adoint   Solve  Time %f seconds \n",AdjointSolveTime);
    fprintf(HistoryFile,"Geometry Update Time %f seconds \n",GeometryUpdateTime);
    fprintf(HistoryFile,"Total Optimization time: %f seconds \n",TotalTime);
       
    fclose(HistoryFile);
    
    // Free up memory
    
    DeleteMeshGradients(NumberOfParameterValues,dMesh_dParameter);
    
    return 1;
    
}

/*##############################################################################
#                                                                              #
#                            ReadOpenVSPDesFile                                #
#                                                                              #
##############################################################################*/

PARAMETER_DATA *ReadOpenVSPDesFile(char *FileName, int &NumberOfDesignVariables)
{
    
    int i;
    double *ParameterValues;
    char DesignFileName[2000], Variable[2000];
    FILE *DesignFile;
    PARAMETER_DATA *ParameterData;
    
    ParameterData = new PARAMETER_DATA;
    
    // Open the OpenVSP des file

    snprintf(DesignFileName,sizeof(DesignFileName)*sizeof(char),"%s.des",FileName);
    
    printf("Opening: %s \n",DesignFileName);fflush(NULL);

    if ( (DesignFile = fopen(DesignFileName, "r")) == NULL ) {
    
       printf("Could not open the OpenVSP des file! \n");
    
       exit(1);
    
    }
    
    // Parse the des file, replace parameters with new values    
    
    fscanf(DesignFile,"%d \n",&NumberOfDesignVariables);

    ParameterData->ParameterValues = new double[NumberOfDesignVariables + 1];
    
    ParameterData->ParameterNames = new char*[NumberOfDesignVariables + 1];

    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {

       ParameterData->ParameterNames[i] = new char[11];

       fscanf(DesignFile,"%s%lf\n",(Variable),&( ParameterData->ParameterValues[i]));
       
       strncpy(ParameterData->ParameterNames[i],Variable,11);

    }
    
    fclose(DesignFile);
    
    return ParameterData;
 
}

/*##############################################################################
#                                                                              #
#                            CreateVSPGeometry                                 #
#                                                                              #
##############################################################################*/

double *CreateVSPGeometry(char *FileName, int NumberOfDesignVariables, char **ParameterNames, double *ParameterValues, VSP_SOLVER &VSP_VLM)
{
    
    int i, j, Node, NumVars;
    int Surface;
    int surf_indx;
    int degen_type;
    int *DidThisNode;    
    double Value, *MeshNodesXYZ;
    char DesignFileName[2000], Variable[2000], CommandLine[2000];
    FILE *DesignFile, *OptDesFile;
    
    // Update the VSP geometry

    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {

       double temp = vsp::SetParmVal( ParameterNames[i], ParameterValues[i] );
    
    }

    vsp::Update();
 
    // Now parse the new geometry and create a new list of xyz node data
    
    vector<double> uvec, wvec;
     
    uvec.resize( 1 );
    
    wvec.resize( 1 );
     
    MeshNodesXYZ = new double[3*VSP_VLM.VSPGeom().Grid(1).NumberOfNodes() + 1];

    DidThisNode = new int[VSP_VLM.VSPGeom().Grid(1).NumberOfNodes() + 1];
    
    zero_int_array(DidThisNode, VSP_VLM.VSPGeom().Grid(1).NumberOfNodes());
    
    // Loop over loops and calculate the new xyz locations of every node... yes, this means
    // we do a lot of the nodes more than once... oh well.

    for ( i = 1 ; i <= VSP_VLM.VSPGeom().Grid(1).NumberOfSurfaceLoops() ; i++ ) {
       
       Surface = VSP_VLM.VSPGeom().Grid(1).LoopList(i).SurfaceID();
       
       surf_indx = VSP_VLM.VSPGeom().VSPSurfaceIDForSurface(Surface);
       
       degen_type = VSP_VLM.VSPGeom().VSPSurfaceDegenType(Surface);
       
       string geom_id = VSP_VLM.VSPGeom().SurfaceGIDList(Surface);

       for ( j = 1 ; j <= VSP_VLM.VSPGeom().Grid(1).LoopList(i).NumberOfNodes() ; j++ ) {
          
          Node = VSP_VLM.VSPGeom().Grid(1).LoopList(i).Node(j);
          
          if ( !DidThisNode[Node] ) {
          
             uvec[0] = VSP_VLM.VSPGeom().Grid(1).LoopList(i).U_Node(j);
             wvec[0] = VSP_VLM.VSPGeom().Grid(1).LoopList(i).V_Node(j);
             
             vector< vec3d > ptvec = vsp::CompVecDegenPnt01( geom_id, surf_indx, degen_type, uvec, wvec);
             
             MeshNodesXYZ[3*Node - 2] = ptvec[0].x();
             MeshNodesXYZ[3*Node - 1] = ptvec[0].y();
             MeshNodesXYZ[3*Node    ] = ptvec[0].z();
             
           //  printf("Node: %d --> x,y,z: %f, %f ... %f, %f .. %f, %f \n",
           //  Node,
           //  VSP_VLM.VSPGeom().Grid(1).NodeList(Node).x(), ptvec[0].x(),
           //  VSP_VLM.VSPGeom().Grid(1).NodeList(Node).y(), ptvec[0].y(),
           //  VSP_VLM.VSPGeom().Grid(1).NodeList(Node).z(), ptvec[0].z());
               
             DidThisNode[Node] = 1;
             
          }
          
       }
      
    }
    
    delete [] DidThisNode;
    
    return MeshNodesXYZ;

}

/*##############################################################################
#                                                                              #
#                            SaveVSPGeomFile                                   #
#                                                                              #
##############################################################################*/

void SaveVSPGeomFile(char *FileName, int NumberOfDesignVariables, char **ParameterNames, double *ParameterValues, VSP_SOLVER &VSP_VLM)
{
    
    int i, NumVars;
    double Value;
    char NewFileName[2000], DesignFileName[2000], Variable[2000];
    FILE *DesignFile, *OptDesFile;
    
    // Update the VSP geometry

    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {
       
       printf("ParameterValues[%d]: %f \n",i,ParameterValues[i]);fflush(NULL);

       double temp = vsp::SetParmVal( ParameterNames[i], ParameterValues[i] );
    
    }

    vsp::Update();
    
    // Save the current geometry out as a vspgeom file
    
    vsp::SetAnalysisInputDefaults( "DegenGeomMesh" );
    
    vsp::SetIntAnalysisInput("DegenGeomMesh", "Set", {vsp::SET_ALL}, 0);
    
    vsp::SetIntAnalysisInput("DegenGeomMesh", "DegenGeomMeshType", {vsp::NGON_MESH_TYPE}, 0);

    vsp::PrintAnalysisInputs( "DegenGeomMesh" );

    string compgeom_resid = vsp::ExecAnalysis( "DegenGeomMesh" );    
    
    snprintf(NewFileName,sizeof(NewFileName)*sizeof(char),"%s.Opt.Final.vspgeom",FileName);
    
    vsp::ExportFile( NewFileName, vsp::SET_ALL, vsp::EXPORT_VSPGEOM );
    
    // Save the current design
    
    snprintf(DesignFileName,sizeof(DesignFileName)*sizeof(char),"%s.des",FileName);
    
    if ( (DesignFile = fopen(DesignFileName, "r")) == NULL ) {
    
       printf("Could not open the OpenVSP des file! \n");
    
       exit(1);
    
    }
    
    // Open Opt des file
    
    snprintf(DesignFileName,sizeof(DesignFileName)*sizeof(char),"%s.Opt.Final.des",FileName);
    
    //printf("Opening: %s \n",DesignFileName);fflush(NULL);

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

       fscanf(DesignFile,"%1999s%lf\n",Variable,&Value);

       fprintf(OptDesFile,"%s %20.10e\n",Variable,ParameterValues[i]);
       
    }
    
    fclose(DesignFile);
    
    fclose(OptDesFile);       
    
}

/*##############################################################################
#                                                                              #
#                    CalculateOpenVSPGeometryGradients                         #
#                                                                              #
##############################################################################*/

double **CalculateOpenVSPGeometryGradients(char *FileName, int NumberOfDesignVariables, PARAMETER_DATA *ParameterData, VSP_SOLVER &VSP_VLM)
{
    
    int i, j, NumberOfMeshNodes;
    double **dMesh_dParameter, *NewParameterValues, Delta, *MeshMinus;
    
    printf("Calculating OpenVSP mesh gradients ... \n");fflush(NULL);
    
    // Create space for the mesh gradients
    
    NewParameterValues = new double[NumberOfDesignVariables + 1];
    
    dMesh_dParameter = new double*[NumberOfDesignVariables + 1];

    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {

       NewParameterValues[i] = ParameterData->ParameterValues[i];
       
    }
    
    // Create the baseline geometry

    dMesh_dParameter[0] = CreateVSPGeometry(FileName,NumberOfDesignVariables,ParameterData->ParameterNames,ParameterData->ParameterValues,VSP_VLM);

    // Loop over parameters and calculate mesh gradients using finite differences
    
    Delta = 1.;
    
    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {
    
       printf("Working on parameter: %d out of %d \r",i,NumberOfDesignVariables);fflush(NULL);
         
       // + Perturbation
       
       NewParameterValues[i] = ParameterData->ParameterValues[i] + Delta;
       
       dMesh_dParameter[i] = CreateVSPGeometry(FileName,NumberOfDesignVariables,ParameterData->ParameterNames,NewParameterValues,VSP_VLM);

       // - Perturbation
    
       NewParameterValues[i] = ParameterData->ParameterValues[i] - Delta;
       
       MeshMinus = CreateVSPGeometry(FileName,NumberOfDesignVariables,ParameterData->ParameterNames,NewParameterValues,VSP_VLM);

       // Calculate derivative using central differences
       
       for ( j = 1 ; j <= 3*VSP_VLM.VSPGeom().Grid(1).NumberOfSurfaceNodes() ; j++ ) {

          dMesh_dParameter[i][j] = ( dMesh_dParameter[i][j] - MeshMinus[j] )/(2.*Delta);
       
       }
       
       delete [] MeshMinus;
       
       NewParameterValues[i] = ParameterData->ParameterValues[i];

    }

    return dMesh_dParameter;
 
}

/*##############################################################################
#                                                                              #
#                    CalculateOpenVSPGeometryGradients                         #
#                                                                              #
##############################################################################*/

void DeleteMeshGradients(int NumberOfDesignVariables, double **dMesh_dParameter)
{
    
    int i;
    
    for ( i = 1 ; i <= NumberOfDesignVariables ; i++ ) {

       delete [] dMesh_dParameter[i];
       
    }
    
    delete [] dMesh_dParameter;
    
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

   Dot = 0.;
   
   for ( i = 1 ; i <= Length ; i++ ) {
      
      Dot += Old[i]*New[i];
      
   }   
    
}


