//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Solver.H"

/*##############################################################################
#                                                                              #
#                                VSP_SOLVER constructor                        #
#                                                                              #
##############################################################################*/

VSP_SOLVER::VSP_SOLVER(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                   VSP_SOLVER init                            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::init(void)
{

    Verbose_ = 0;
    
    FirstTimeSetup_ = 1;
 
    DoSymmetryPlaneSolve_ = 0;
    
    SetFarFieldDist_ = 0;
    
    NumberOfWakeTrailingNodes_ = 64; // Must be a power of 2
    
    SearchID_ = 0;
    
    SaveRestartFile_ = 0;
    
    JacobiRelaxationFactor_ = 0.25;
    
    DumpGeom_ = 0;
    
    ForceType_ = 0;
    
    AveragingIteration_ = 0;
    
    NumberOfAveragingSteps_ = 0;
    
    NoWakeIteration_ = 0;
    
    MaxTurningAngle_ = -1.;
    
    Clmax_2d_ = -1.;
     
    CDo_ = 0.;
    
    NumberOfKelvinConstraints_ = 0;
    
    LoadDeformationFile_ = 0;
    
    Write2DFEMFile_ = 0;
    
    TimeAccurate_ = 0;
    
    TimeAnalysisType_ = 0;

    NumberOfTimeSteps_ = 1;
    
    ReducedFrequency_ = 0.0;
    
    Unsteady_AngleRate_ = 0.;
    
    Unsteady_Angle_ = 0.;
    
    Unsteady_AngleMax_ = 0.;
    
    Unsteady_H_ = 0.;
    
    Unsteady_HMax_ = 0.;
    
    Preconditioner_ = MATCON;
    
    CalculateVortexLift_ = 1;

    CalculateLeadingEdgeSuction_ = 0;
    
    NoKarmanTsienCorrection_ = 0;

    sprintf(CaseString_,"No Comment");
    
    VortexSheet_ = NULL;
    
     CL_Unsteady_ = NULL;
     CD_Unsteady_ = NULL;
     CS_Unsteady_ = NULL;
    CFx_Unsteady_ = NULL;
    CFy_Unsteady_ = NULL;
    CFz_Unsteady_ = NULL;
    CMx_Unsteady_ = NULL;
    CMy_Unsteady_ = NULL;
    CMz_Unsteady_ = NULL;
 
    AngleZero_ = 0.;

}

/*##############################################################################
#                                                                              #
#                                  VSP_SOLVER Copy                             #
#                                                                              #
##############################################################################*/

VSP_SOLVER::VSP_SOLVER(const VSP_SOLVER &Solver)
{

    init();

    // Just * use the operator = code

    *this = Solver;

}

/*##############################################################################
#                                                                              #
#                                 VSP_SOLVER operator=                         #
#                                                                              #
##############################################################################*/

VSP_SOLVER& VSP_SOLVER::operator=(const VSP_SOLVER &MGSolver)
{

    printf("VSP_SOLVER operator= not implemented! \n");
    exit(1);
    
    return *this;

}

/*##############################################################################
#                                                                              #
#                              VSP_SOLVER destructor                           #
#                                                                              #
##############################################################################*/

VSP_SOLVER::~VSP_SOLVER(void)
{


}

/*##############################################################################
#                                                                              #
#                                   VSP_SOLVER Setup                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Setup(void)
{
 
    int i, j, NumberOfStations;
    double Area;

    // Copy over data
    
    MGLevel_ = 1;
    
    NumberOfMGLevels_ = VSPGeom().NumberOfGridLevels() - 1;

    // Size the lists and determine the size of the domain
    
    Xmin_ = 1.e9;
    Xmax_ = -Xmin_;

    Ymin_ = 1.e9;
    Ymax_ = -Ymin_;

    Zmin_ = 1.e9;
    Zmax_ = -Zmin_;

    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfNodes() ; j++ ) {

       Xmin_ = MIN(Xmin_,VSPGeom().Grid(MGLevel_).NodeList(j).x());
       Xmax_ = MAX(Xmax_,VSPGeom().Grid(MGLevel_).NodeList(j).x());

       Ymin_ = MIN(Ymin_,VSPGeom().Grid(MGLevel_).NodeList(j).y());
       Ymax_ = MAX(Ymax_,VSPGeom().Grid(MGLevel_).NodeList(j).y());

       Zmin_ = MIN(Zmin_,VSPGeom().Grid(MGLevel_).NodeList(j).z());
       Zmax_ = MAX(Zmax_,VSPGeom().Grid(MGLevel_).NodeList(j).z());

    }

    // Allocate space for the vortex edges

    NumberOfSurfaceVortexEdges_  = VSPGeom().Grid(MGLevel_).NumberOfEdges();
    
    NumberOfTrailingVortexEdges_ = VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes();
    
    NumberOfVortexLoops_         = VSPGeom().Grid(MGLevel_).NumberOfLoops();
    
    printf("Number Of Trailing Vortices: %d \n",NumberOfTrailingVortexEdges_);
    
    // VLM model
    
    if ( ModelType_ == VLM_MODEL ) {
       
       NumberOfKelvinConstraints_ = 0;
       
    }
    
    // Panel Model
    
    else if ( ModelType_ == PANEL_MODEL ) {
       
       DetermineNumberOfKelvinConstrains();
    
    }
    
    // Uknown model
    
    else {
       
       printf("Unknown Model Type! \n");fflush(NULL);
       
       exit(1);
       
    }
    
    NumberOfEquations_ = NumberOfVortexLoops_ + NumberOfKelvinConstraints_;

    // Allocate space for the vortex edges and loops
  
    SurfaceVortexEdge_ = new VSP_EDGE*[NumberOfSurfaceVortexEdges_ + 1];

    TrailingVortexEdge_ = new VORTEX_TRAIL[NumberOfTrailingVortexEdges_ + 1];
    
    VortexLoop_ = new VSP_LOOP*[NumberOfVortexLoops_ + 1];
 
    LocalFreeStreamVelocity_ =  new double*[NumberOfVortexLoops_ + 1];  
    
    UnsteadyTrailingWakeVelocity_ =  new double*[NumberOfVortexLoops_ + 1];  
    
    LocalBodySurfaceVelocity_ =  new double*[NumberOfVortexLoops_ + 1];  
        
    Gamma_ = new double[NumberOfVortexLoops_ + 1];    
    
    Diagonal_ = new double[NumberOfVortexLoops_ + 1];     
    
    GammaNM1_ = new double[NumberOfVortexLoops_ + 1];     
    
    GammaNM2_ = new double[NumberOfVortexLoops_ + 1];     
    
    Delta_= new double[NumberOfVortexLoops_ + 1];     
   
    zero_double_array(Gamma_,    NumberOfVortexLoops_);    Gamma_[0] = 0.;   
    zero_double_array(GammaNM1_, NumberOfVortexLoops_); GammaNM1_[0] = 0.;
    zero_double_array(GammaNM2_, NumberOfVortexLoops_); GammaNM2_[0] = 0.;
    zero_double_array(Diagonal_, NumberOfVortexLoops_); Diagonal_[0] = 0.;    
    zero_double_array(Delta_,    NumberOfVortexLoops_);    Delta_[0] = 0.;
   
    Residual_ = new double[NumberOfEquations_ + 1];    

    RightHandSide_ = new double[NumberOfEquations_ + 1];     
     
    MatrixVecTemp_ = new double[NumberOfEquations_ + 1];     
   
    zero_double_array(Residual_,      NumberOfEquations_); Residual_[0]      = 0.;
    zero_double_array(RightHandSide_, NumberOfEquations_); RightHandSide_[0] = 0.;
    zero_double_array(MatrixVecTemp_, NumberOfEquations_); RightHandSide_[0] = 0.;
  
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       LocalFreeStreamVelocity_[i] = new double[5];
       
       UnsteadyTrailingWakeVelocity_[i] = new double[3];
       
       LocalBodySurfaceVelocity_[i] = new double[3];
       
       // Zero out local free stream velocity

       LocalFreeStreamVelocity_[i][0] = 0.;
       LocalFreeStreamVelocity_[i][1] = 0.;
       LocalFreeStreamVelocity_[i][2] = 0.;
       LocalFreeStreamVelocity_[i][3] = 0.;
       LocalFreeStreamVelocity_[i][4] = 0.;
       
       // Zero out trailing wake velocities
       
       UnsteadyTrailingWakeVelocity_[i][0] = 0.;
       UnsteadyTrailingWakeVelocity_[i][1] = 0.;
       UnsteadyTrailingWakeVelocity_[i][2] = 0.;
       
       // Zero out local body surface velocities
       
       LocalBodySurfaceVelocity_[i][0] = 0.;
       LocalBodySurfaceVelocity_[i][1] = 0.;
       LocalBodySurfaceVelocity_[i][2] = 0.;             
       
    }      
    
    // Allocate space for span loading data
        
    Span_Cx_    = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cy_    = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cz_    = new double*[VSPGeom().NumberOfSurfaces() + 1];

    Span_Cxi_   = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cyi_   = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Czi_   = new double*[VSPGeom().NumberOfSurfaces() + 1];
    
    Span_Cn_    = new double*[VSPGeom().NumberOfSurfaces() + 1];

    Span_Cl_    = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cs_    = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cd_    = new double*[VSPGeom().NumberOfSurfaces() + 1];
    
    Span_Cmx_   = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmy_   = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmz_   = new double*[VSPGeom().NumberOfSurfaces() + 1];    
    
    Span_LE_    = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Yavg_  = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Area_  = new double*[VSPGeom().NumberOfSurfaces() + 1];
    
    Local_Vel_ = new double*[VSPGeom().NumberOfSurfaces() + 1];
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
        
       NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
 
       Span_Cx_[i]    = new double[NumberOfStations + 1];

       Span_Cy_[i]    = new double[NumberOfStations + 1];

       Span_Cz_[i]    = new double[NumberOfStations + 1];

       Span_Cxi_[i]    = new double[NumberOfStations + 1];

       Span_Cyi_[i]    = new double[NumberOfStations + 1];

       Span_Czi_[i]    = new double[NumberOfStations + 1];

       Span_Cmx_[i]   = new double[NumberOfStations + 1];

       Span_Cmy_[i]   = new double[NumberOfStations + 1];

       Span_Cmz_[i]   = new double[NumberOfStations + 1];
       
       Span_Cn_[i]    = new double[NumberOfStations + 1];
     
       Span_Cl_[i]    = new double[NumberOfStations + 1];

       Span_Cs_[i]    = new double[NumberOfStations + 1];
        
       Span_Cd_[i]    = new double[NumberOfStations + 1];

       Span_LE_[i]    = new double[NumberOfStations + 1];
                     
       Span_Yavg_[i]  = new double[NumberOfStations + 1];
       
       Span_Area_[i]  = new double[NumberOfStations + 1];
       
       Local_Vel_[i]  = new double[NumberOfStations + 1];
 
       zero_double_array(Span_Cx_[i],    NumberOfStations);
       zero_double_array(Span_Cy_[i],    NumberOfStations);
       zero_double_array(Span_Cz_[i],    NumberOfStations);      

       zero_double_array(Span_Cxi_[i],   NumberOfStations);
       zero_double_array(Span_Cyi_[i],   NumberOfStations);
       zero_double_array(Span_Czi_[i],   NumberOfStations);      
        
       zero_double_array(Span_Cn_[i],    NumberOfStations);       

       zero_double_array(Span_Cl_[i],    NumberOfStations);
       zero_double_array(Span_Cs_[i],    NumberOfStations);
       zero_double_array(Span_Cd_[i],    NumberOfStations); 
       
       zero_double_array(Span_Cmx_[i],   NumberOfStations);
       zero_double_array(Span_Cmy_[i],   NumberOfStations);
       zero_double_array(Span_Cmz_[i],   NumberOfStations);       
        
       zero_double_array(Span_LE_[i],    NumberOfStations);    
       zero_double_array(Span_Yavg_[i],  NumberOfStations);       
       zero_double_array(Span_Area_[i],  NumberOfStations);
       
       zero_double_array(Local_Vel_[i],  NumberOfStations);
     
    }    

    // Create vortex loop list

    Setup_VortexLoops();

    // Create vortex edge list
 
    Setup_VortexEdges();

    // Calculate average vortex edge spacing
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       Area = VortexLoop(SurfaceVortexEdge(j).VortexLoop1()).Area()
            + VortexLoop(SurfaceVortexEdge(j).VortexLoop2()).Area();
     
       SurfaceVortexEdge(j).LocalSpacing() = sqrt(0.5*(Area));

    }  
                       
    // Zero out stuff on this grid 

    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; j++ ) {

       VSPGeom().Grid(MGLevel_).LoopList(j).Gamma() = 0.;

    }
    
    // Create interaction list

    CreateSurfaceVorticesInteractionList();
    
}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER DetermineNumberOfKelvinConstrains                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DetermineNumberOfKelvinConstrains(void)
{
   
    int j, k, p, n, q, Edge, Node, Loop, Loop1, Loop2, Next, StackSize, Done, FoundOne;
    int Node1, Node2, *LoopStack, NotFlipped, KelvinGroup, Wing;
    double Vec1[3], Vec2[3], Dot;
        
    MGLevel_ = 1;
    
    // Traverse the loops, edge to edge, and determine the number of 
    // independent bodies... this is the number of Kelvin constraints
    // we have to impose
    
    LoopStack = new int[NumberOfVortexLoops_ + 1];
    
    LoopIsOnBaseRegion_ = new int[NumberOfVortexLoops_ + 1];
    
    LoopInKelvinConstraintGroup_ = new int[NumberOfVortexLoops_ + 1];
    
    zero_int_array(LoopStack, NumberOfVortexLoops_);
    
    zero_int_array(LoopIsOnBaseRegion_, NumberOfVortexLoops_);
    
    zero_int_array(LoopInKelvinConstraintGroup_, NumberOfVortexLoops_);
    
    NumberOfKelvinConstraints_ = 1;

    StackSize = Next = 1;
    
    LoopStack[Next] = 1;
    
    LoopInKelvinConstraintGroup_[1] = NumberOfKelvinConstraints_;
    
    Done = 0;
    
    while ( !Done ) {
       
       while ( Next <= StackSize ) {
          
          Loop = LoopStack[Next];
          
          for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).LoopList(Loop).NumberOfEdges() ; j++ ) {
             
             Edge = VSPGeom().Grid(MGLevel_).LoopList(Loop).Edge(j);
             
             Loop1 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Loop1();
             
             Loop2 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Loop2();
             
             if ( LoopInKelvinConstraintGroup_[Loop1] == 0 ) {
                
                LoopInKelvinConstraintGroup_[Loop1] = NumberOfKelvinConstraints_;
                
                LoopStack[++StackSize] = Loop1;
                
             }
             
             if ( LoopInKelvinConstraintGroup_[Loop2] == 0 ) {
                
                LoopInKelvinConstraintGroup_[Loop2] = NumberOfKelvinConstraints_;
                
                LoopStack[++StackSize] = Loop2;
        
             }          
              
          }
          
          Next++;   
  
       }   

       FoundOne = 0;
       
       j = 1;
       
       while ( j <= VSPGeom().Grid(MGLevel_).NumberOfLoops() && !FoundOne ) {
          
          if ( LoopInKelvinConstraintGroup_[j] == 0 ) {
             
             StackSize = Next = 1;
             
             LoopStack[Next] = j;
    
             LoopInKelvinConstraintGroup_[j] = ++NumberOfKelvinConstraints_;
             
             FoundOne = 1;
             
          }
          
          j++;
          
       }
       
       if ( !FoundOne ) Done = 1;
       
    }
    
    // Determine the number of vortex sheets
    
    NumberOfVortexSheets_ = 0;
    
    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {

       NumberOfVortexSheets_ = MAX(NumberOfVortexSheets_, VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j));
       
    }   

    printf("There are: %10d Vortex Sheets \n", NumberOfVortexSheets_);
             
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       // Find a node on this vortex sheet
       
       FoundOne = 0;
       
       j = 1;
            
       while ( !FoundOne && j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ) {
   
          Wing = VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j);
          
          if ( Wing == k && VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNodeIsPeriodic(j) ) {
  
             Node = VSPGeom().Grid(MGLevel_).KuttaNode(j);;
             
             FoundOne = 1;
             
          }
          
          j++;
          
       }

       if ( FoundOne ) {

          printf("Looking for node: %d \n",Node);fflush(NULL);
          
          // Grab a loop that contains this kutta node
         
          FoundOne = 0;
         
          n = 1;
         
          while ( !FoundOne && n <= NumberOfVortexLoops_ ) {
            
             p = 1;
                        
             while ( p <= VSPGeom().Grid(MGLevel_).LoopList(n).NumberOfEdges() && !FoundOne ) {
      
                Edge = VSPGeom().Grid(MGLevel_).LoopList(n).Edge(p);
               
                Node1 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Node1();
                Node2 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Node2();
               
                if ( Node == Node1 || Node == Node2 ) {
                  
                   Loop = n;
                  
                   FoundOne = 1;
                  
                }
               
                p++;
               
             }
            
             n++;
            
          }
         
          if ( !FoundOne ) {
            
             printf("Error in determining number of Kelvin regions for a periodic wake surface! \n");
             printf("Looking for node: %d \n",Node);
             fflush(NULL);
             exit(1);
            
          }
         
          // Go edge, by edge and see if we cover the entire kelvin region
         
          zero_int_array(LoopStack, NumberOfVortexLoops_);
         
          StackSize = Next = 1;
          
          LoopStack[Next] = Loop;
         
          KelvinGroup = LoopInKelvinConstraintGroup_[Loop];

          while ( Next <= StackSize ) {
             
             Loop = LoopStack[Next];
             
             for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).LoopList(Loop).NumberOfEdges() ; p++ ) {
                
                Edge = VSPGeom().Grid(MGLevel_).LoopList(Loop).Edge(p);
               
                if ( !VSPGeom().Grid(MGLevel_).EdgeList(Edge).IsTrailingEdge() ) {
               
                   Loop1 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Loop1();
                   
                   Loop2 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Loop2();

                   if ( LoopInKelvinConstraintGroup_[Loop1] == KelvinGroup ) {
                      
                      LoopInKelvinConstraintGroup_[Loop1] = -KelvinGroup;
                      
                      LoopStack[++StackSize] = Loop1;
                      
                   }
                  
                   else if ( LoopInKelvinConstraintGroup_[Loop1] != -KelvinGroup ){
                     
                      printf("WTF... how did we jump to another Kelvin Group... \n"); fflush(NULL);
                      exit(1);
                      
                   }
                   
                   if ( LoopInKelvinConstraintGroup_[Loop2] == KelvinGroup ) {
                      
                      LoopInKelvinConstraintGroup_[Loop2] = -KelvinGroup;
                      
                      LoopStack[++StackSize] = Loop2;
              
                   }    
                  
                   else if ( LoopInKelvinConstraintGroup_[Loop2] != -KelvinGroup ){
                     
                      printf("WTF... how did we jump to another Kelvin Group... \n"); fflush(NULL);
                      exit(1);
                     
                   }    
                  
                }                       
                 
             }
             
             Next++;   
     
          }
      
          // Check and see if there are any loops in this Kelvin group that were not flipped
         
          NotFlipped = 0;

          for ( p = 1 ; p <= NumberOfVortexLoops_ ; p++ ) {                  
                  
             if ( LoopInKelvinConstraintGroup_[p] == KelvinGroup ) {
               
                NotFlipped++;
               
             }
            
          }
            
          // If not all were flipped... then there is a base region
         
          if ( NotFlipped > 0 ) {
                              
             printf("Base region found for vortex sheet system: %d \n",k);fflush(NULL);
                                        
             // Determine which region ... + or - ... is the base region
            
             Vec1[0] = Vec1[1] = Vec1[2] = 0.;
            
             Vec2[0] = Vec2[1] = Vec2[2] = 0.;

             for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                if ( LoopInKelvinConstraintGroup_[p] ==  KelvinGroup ) {
                  
                   Vec1[0] += VSPGeom().Grid(MGLevel_).LoopList(p).Nx();
                   Vec1[1] += VSPGeom().Grid(MGLevel_).LoopList(p).Ny();
                   Vec1[2] += VSPGeom().Grid(MGLevel_).LoopList(p).Nz();
                  
                }
               
                if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                  
                   Vec2[0] += VSPGeom().Grid(MGLevel_).LoopList(p).Nx();
                   Vec2[1] += VSPGeom().Grid(MGLevel_).LoopList(p).Ny();
                   Vec2[2] += VSPGeom().Grid(MGLevel_).LoopList(p).Nz();
                  
                }                     
               
             }
            
             Dot = sqrt(vector_dot(Vec1,Vec1));
            
             Vec1[0] /= Dot; 
             Vec1[1] /= Dot; 
             Vec1[2] /= Dot; 
            
             Dot = sqrt(vector_dot(Vec2,Vec2));
            
             Vec2[0] /= Dot; 
             Vec2[1] /= Dot; 
             Vec2[2] /= Dot;
            
             // Remove those loops in the base region from the Kelvin group, and add them
             // to the base region list
            
             if ( Vec1[0] > Vec2[0] ) {
           
                for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                   if ( LoopInKelvinConstraintGroup_[p] == KelvinGroup ) {
                     
                      LoopIsOnBaseRegion_[p] = 1;
                     
                      LoopInKelvinConstraintGroup_[p] = KelvinGroup;
              
                   }
                  
                }
               
                for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                   if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                  
                      LoopInKelvinConstraintGroup_[p] = KelvinGroup;
              
                   }
                  
                }                     
               
             }
            
             else {
       
                for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                   if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                     
                      LoopIsOnBaseRegion_[p] = 1;
                     
                      LoopInKelvinConstraintGroup_[p] = KelvinGroup;
                     
                   }
                  
                }
               
                for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                   if ( LoopInKelvinConstraintGroup_[p] == KelvinGroup ) {
                   
                      LoopInKelvinConstraintGroup_[p] = KelvinGroup;
              
                   }
                  
                }                        
               
             }  
                   
          }
          
          else {
             
             for ( p = 1 ; p <= NumberOfVortexLoops_ ; p++ ) {                  
                  
                if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                  
                   LoopInKelvinConstraintGroup_[p] = KelvinGroup;
                  
                }
            
             }
          
          }
         
       }                 
       
    }

    printf("There are %d Kelvin constraints \n",NumberOfKelvinConstraints_);
    
    delete [] LoopStack; 
 
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER Setup_VortexLoops                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Setup_VortexLoops(void)
{
    
    int j, Level;

    // Set up the Vortex Loops... 

    for ( Level = 1 ; Level <= NumberOfMGLevels_ ; Level++ ) {

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfLoops() ; j++ ) {
   
          VSPGeom().Grid(Level).LoopList(j).VortexLoop() = j;
          
       }
       
    }
       
    for ( j = 1 ; j <= VSPGeom().Grid(1).NumberOfLoops() ; j++ ) {

       VortexLoop_[j] = &(VSPGeom().Grid(1).LoopList(j));
       
    }
       
    // Set up a dummy loop at 0
    
    VortexLoop_[0] = new VSP_LOOP;
   
}    

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER Setup_VortexEdges                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Setup_VortexEdges(void)
{
    
    int j, Node1, Node2, LoopL, LoopR, VortexLoop1, VortexLoop2, Level;

    VSP_NODE VSP_Node1, VSP_Node2;
    
    // Setup the surface vortex edge data
    
    for ( Level = 1 ; Level <= NumberOfMGLevels_ ; Level++ ) {

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfEdges() ; j++ ) {
       
          // Pointer from grid edge to vortex edge
      
          VSPGeom().Grid(Level).EdgeList(j).VortexEdge() = j;
          
          // Pass in edge data and create edge cofficients
          
          Node1 = VSPGeom().Grid(Level).EdgeList(j).Node1();
          Node2 = VSPGeom().Grid(Level).EdgeList(j).Node2();

          VSPGeom().Grid(Level).EdgeList(j).Setup(VSPGeom().Grid(Level).NodeList(Node1),
                                                  VSPGeom().Grid(Level).NodeList(Node2));
           
          LoopL = VSPGeom().Grid(Level).EdgeList(j).LoopL();
          LoopR = VSPGeom().Grid(Level).EdgeList(j).LoopR();

          VortexLoop1 = VortexLoop2 = 0;

          if ( LoopL > 0 ) VortexLoop1 = VSPGeom().Grid(Level).LoopList(LoopL).VortexLoop();
          if ( LoopR > 0 ) VortexLoop2 = VSPGeom().Grid(Level).LoopList(LoopR).VortexLoop();

          // Keep track of the two bounding vortex loops

          VSPGeom().Grid(Level).EdgeList(j).VortexLoop1() = VortexLoop1;
          VSPGeom().Grid(Level).EdgeList(j).VortexLoop2() = VortexLoop2;                                    

       }
       
    }
    
    for ( j = 1 ; j <= VSPGeom().Grid(1).NumberOfEdges() ; j++ ) {
    
       SurfaceVortexEdge_[j] = &(VSPGeom().Grid(1).EdgeList(j));
       
    }
    
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER InitializeFreeStream                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::InitializeFreeStream(void)
{
 
    int i, j;
    double xyz[3], q[5], CA, SA, CB, SB, Rate_P, Rate_Q, Rate_R;
    double gamma, f1, gm1, gm2, gm3;
    VSP_NODE VSP_Node1, VSP_Node2;

    // Limits on max velocity, and min/max pressures
    
    if ( Mach_ <= 0. ) Mach_ = 0.001;

    gamma = 1.4;

    gm1 = gamma - 1.;

    gm2 = 0.5*gm1*SQR(Mach_);

    gm3 = 1./gm1;

    QMax_ = 0.75*sqrt( 1./gm2 + 1. );    
    
    CpMin_ = -2./(gamma*Mach_*Mach_);

    f1 = 1. + 0.5*(gamma-1.)*Mach_*Mach_;

    CpMax_ = 2.*( pow(f1,(gamma)/(gamma-1)) - 1. ) / ( 1.4*Mach_*Mach_ );
    
    DCpMax_ = CpMax_ - CpMin_;

    // Base pressure
    
    if ( Mach_ <= 1.1 ) {

       CpBase_ = -0.20 - 0.25/40.*pow(Mach_ + 1., 4.);

    }

    else if ( Mach_ > 1.1 && Mach_ <= 4. ) {

       CpBase_ = -1.418/pow(Mach_+1.,1.95);

    }

    else {

	    CpBase_ = -1./(Mach_*Mach_);

    }    
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
 
       // Zero out free stream velocities

       LocalFreeStreamVelocity_[i][0] = 0.;
       LocalFreeStreamVelocity_[i][1] = 0.;
       LocalFreeStreamVelocity_[i][2] = 0.;
       LocalFreeStreamVelocity_[i][3] = 0.;
       LocalFreeStreamVelocity_[i][4] = 0.;
             
       // Zero out trailing wake velocities
       
       UnsteadyTrailingWakeVelocity_[i][0] = 0.;
       UnsteadyTrailingWakeVelocity_[i][1] = 0.;
       UnsteadyTrailingWakeVelocity_[i][2] = 0.;
       
       // Zero out local body surface velocities
       
       LocalBodySurfaceVelocity_[i][0] = 0.;
       LocalBodySurfaceVelocity_[i][1] = 0.;
       LocalBodySurfaceVelocity_[i][2] = 0.;             
       
    }

    // Calculate body motion velocities

    if ( TimeAccurate_ ) CalculateSurfaceMotion();
        
    if ( TimeAccurate_ && TimeAnalysisType_ == Q_ANALYSIS ) AngleOfAttack_ = Unsteady_Angle_;
    
    if ( TimeAccurate_ && TimeAnalysisType_ == R_ANALYSIS )   AngleOfBeta_ = -Unsteady_Angle_;
    
    // Free stream velocity vector

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
    
    FreeStreamVelocity_[0] = CA*CB;
    FreeStreamVelocity_[1] =   -SB;
    FreeStreamVelocity_[2] = SA*CB;

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
 
       // Calculate local free stream conditions

       LocalFreeStreamVelocity_[i][0] = FreeStreamVelocity_[0];
       LocalFreeStreamVelocity_[i][1] = FreeStreamVelocity_[1];
       LocalFreeStreamVelocity_[i][2] = FreeStreamVelocity_[2];
       LocalFreeStreamVelocity_[i][3] = 0.;  // Delta-P
       LocalFreeStreamVelocity_[i][4] = 1.;  // Local free stream velocity magnitude
             
    }        
       
    // Rotational rates... note these rates are wrt to the body stability
    // axes... so we have to convert them to equivalent freestream velocities...
    // in the VSPAERO axes system with has X and Z pointing in the opposite
    // directions
    
    Rate_P = RotationalRate_[0] / Vinf_;
    Rate_Q = RotationalRate_[1] / Vinf_;
    Rate_R = RotationalRate_[2] / Vinf_;
       
    // Add in rotational velocities

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       xyz[0] = VortexLoop(i).Xc() - Xcg();            
       xyz[1] = VortexLoop(i).Yc() - Ycg();           
       xyz[2] = VortexLoop(i).Zc() - Zcg();      

       // P - Roll
       
       LocalFreeStreamVelocity_[i][0] += 0.;
       LocalFreeStreamVelocity_[i][1] += -xyz[2] * Rate_P;
       LocalFreeStreamVelocity_[i][2] += +xyz[1] * Rate_P;
        
       // Q - Pitch

       LocalFreeStreamVelocity_[i][0] += -xyz[2] * Rate_Q;
       LocalFreeStreamVelocity_[i][1] += 0.;
       LocalFreeStreamVelocity_[i][2] += +xyz[0] * Rate_Q;

       // R - Yaw
       
       LocalFreeStreamVelocity_[i][0] += -xyz[1] * Rate_R;
       LocalFreeStreamVelocity_[i][1] += +xyz[0] * Rate_R;
       LocalFreeStreamVelocity_[i][2] += 0.;
    
    }  
    
    // Add in rotor induced velocities

    for ( j = 1 ; j <= NumberOfRotors_ ; j++ ) {
     
     RotorDisk(j).Density() = Density_;
 
     RotorDisk(j).Vinf(0) = FreeStreamVelocity_[0] * Vinf_;
     RotorDisk(j).Vinf(1) = FreeStreamVelocity_[1] * Vinf_;
     RotorDisk(j).Vinf(2) = FreeStreamVelocity_[2] * Vinf_;
     
    }
        
    for ( j = 1 ; j <= NumberOfRotors_ ; j++ ) {
     
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

          xyz[0] = VortexLoop(i).Xc();            
          xyz[1] = VortexLoop(i).Yc();           
          xyz[2] = VortexLoop(i).Zc();
       
          RotorDisk(j).Velocity(xyz, q);                   

          LocalFreeStreamVelocity_[i][0] += q[0] / Vinf_;
          LocalFreeStreamVelocity_[i][1] += q[1] / Vinf_;
          LocalFreeStreamVelocity_[i][2] += q[2] / Vinf_;
          LocalFreeStreamVelocity_[i][3] += q[3];
          LocalFreeStreamVelocity_[i][4] += q[4] / Vinf_;
     
          // If there is a ground effects, z - plane
          
          if ( DoGroundEffectsAnalysis() ) {

             xyz[0] = VortexLoop(i).Xc();            
             xyz[1] = VortexLoop(i).Yc();           
             xyz[2] = VortexLoop(i).Zc();
          
             xyz[2] *= -1.;
            
             RotorDisk(j).Velocity(xyz, q);      
   
             q[2] *= -1.;
            
             LocalFreeStreamVelocity_[i][0] += q[0] / Vinf_;
             LocalFreeStreamVelocity_[i][1] += q[1] / Vinf_;
             LocalFreeStreamVelocity_[i][2] += q[2] / Vinf_;
             LocalFreeStreamVelocity_[i][3] += q[3];
             LocalFreeStreamVelocity_[i][4] += q[4] / Vinf_;
               
          }      
                    
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {

             xyz[0] = VortexLoop(i).Xc();            
             xyz[1] = VortexLoop(i).Yc();           
             xyz[2] = VortexLoop(i).Zc();
             
             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
            
             RotorDisk(j).Velocity(xyz, q);      
   
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
            
             LocalFreeStreamVelocity_[i][0] += q[0] / Vinf_;
             LocalFreeStreamVelocity_[i][1] += q[1] / Vinf_;
             LocalFreeStreamVelocity_[i][2] += q[2] / Vinf_;
             LocalFreeStreamVelocity_[i][3] += q[3];
             LocalFreeStreamVelocity_[i][4] += q[4] / Vinf_;
             
             // If there is a ground effects, z - plane
             
             if ( DoGroundEffectsAnalysis() ) {
  
                xyz[2] *= -1.;
               
                RotorDisk(j).Velocity(xyz, q);      
      
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;      
                                                      q[2] *= -1.;
               
                LocalFreeStreamVelocity_[i][0] += q[0] / Vinf_;
                LocalFreeStreamVelocity_[i][1] += q[1] / Vinf_;
                LocalFreeStreamVelocity_[i][2] += q[2] / Vinf_;
                LocalFreeStreamVelocity_[i][3] += q[3];
                LocalFreeStreamVelocity_[i][4] += q[4] / Vinf_;
                  
             }    
               
          }             
          
       }    
       
    }
    
    // Add in surface velocity terms
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       LocalFreeStreamVelocity_[i][0] -= LocalBodySurfaceVelocity_[i][0];
       LocalFreeStreamVelocity_[i][1] -= LocalBodySurfaceVelocity_[i][1];
       LocalFreeStreamVelocity_[i][2] -= LocalBodySurfaceVelocity_[i][2];     
     
    }        

}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER InitializeTrailingVortices                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::InitializeTrailingVortices(void)
{
 
    int i, j, k, NumEdges, Node1, Node2, Loop, NumberOfKuttaNodes;
    double FarDist, *Sigma, *VecX, *VecY, *VecZ, Vec[3], VecTe[3];
    double Scale_X, Scale_Y, Scale_Z, Dot, Dot1, Dot2, WakeDist;
    VSP_NODE VSP_Node1, VSP_Node2;
    
    // Initial wake in the free stream direction

    WakeAngle_[0] = FreeStreamVelocity_[0];
    WakeAngle_[1] = FreeStreamVelocity_[1];
    WakeAngle_[2] = FreeStreamVelocity_[2];

    // Determine how far to allow wakes to adapt... beyond this the wakes go straight off to
    // 'infinity' in the free stream direction
    
    Scale_X = Scale_Y = Scale_Z = 1.;
    
    if ( DoSymmetryPlaneSolve_ == SYM_X) Scale_X = 2.;
    if ( DoSymmetryPlaneSolve_ == SYM_Y) Scale_Y = 2.;
    if ( DoSymmetryPlaneSolve_ == SYM_Z) Scale_Z = 2.;
    
    FarDist = MAX3(Scale_X*(Xmax_ - Xmin_), Scale_Y*(Ymax_-Ymin_), Scale_Z*(Zmax_-Zmin_));
    
    printf("Xmax_ - Xmin_: %f \n",Xmax_ - Xmin_);
    printf("Ymax_ - Ymin_: %f \n",Ymax_ - Ymin_);
    printf("Zmax_ - Zmin_: %f \n",Zmax_ - Zmin_);
    printf("\n");
    
    // Override far field distance
    
    if ( SetFarFieldDist_ ) FarDist = FarFieldDist_;

    printf("Wake FarDist set to: %f \n",FarDist);
    printf("\n");
 
    // Determine the minimum trailing edge spacing for edge kutta node

    Sigma = new double[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];

    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfNodes() ; j++ ) {

       Sigma[j] = 1.e9;
       
    }
  
    for ( i = 1 ; i <= VSPGeom().Grid(MGLevel_).NumberOfEdges() ; i++ ) {
     
       if ( VSPGeom().Grid(MGLevel_).EdgeList(i).IsTrailingEdge() ) {     

          Node1 = VSPGeom().Grid(MGLevel_).EdgeList(i).Node1();
          Node2 = VSPGeom().Grid(MGLevel_).EdgeList(i).Node2();
          
          Sigma[Node1] = MIN(Sigma[Node1], VSPGeom().Grid(MGLevel_).EdgeList(i).Length());
          Sigma[Node2] = MIN(Sigma[Node2], VSPGeom().Grid(MGLevel_).EdgeList(i).Length());
        
       }
       
    } 
    
    // Determine trailing edge angle
    
    VecX = new double[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    VecY = new double[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    VecZ = new double[VSPGeom().Grid(MGLevel_).NumberOfNodes() + 1];
    
    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfNodes() ; j++ ) {

       VecX[j] = 0.;
       VecY[j] = 0.;
       VecZ[j] = 0.;
       
    }
    
    for ( i = 1 ; i <= VSPGeom().Grid(MGLevel_).NumberOfEdges() ; i++ ) {
     
       if ( VSPGeom().Grid(MGLevel_).EdgeList(i).IsTrailingEdge() ) { 
        
          // Determine trailing edge loop
          
          VecTe[0] = VecTe[1] = VecTe[2] = 0.;
          
          for ( j = 1 ; j <= 2 ; j++ ) {
             
             if ( j == 1 ) Loop = VSPGeom().Grid(MGLevel_).EdgeList(i).Loop1();
             
             if ( j == 2 ) Loop = VSPGeom().Grid(MGLevel_).EdgeList(i).Loop2();
             
             if ( Loop != 0 ) {
             
                // Edge vector crossed into normal... gives TE direction
                
                vector_cross(VSPGeom().Grid(MGLevel_).EdgeList(i).Vec(), VSPGeom().Grid(MGLevel_).LoopList(Loop).Normal(), Vec);
      
                Dot1 = VSPGeom().Grid(MGLevel_).EdgeList(i).Vec()[0];
                Dot2 = Vec[0];
                
                VecTe[0] += Dot1 * VSPGeom().Grid(MGLevel_).EdgeList(i).Vec()[0] + Dot2 * Vec[0];
                VecTe[1] += Dot1 * VSPGeom().Grid(MGLevel_).EdgeList(i).Vec()[1] + Dot2 * Vec[1];
                VecTe[2] += Dot1 * VSPGeom().Grid(MGLevel_).EdgeList(i).Vec()[2] + Dot2 * Vec[2];
      
                Dot = sqrt(vector_dot(VecTe,VecTe));
                
                VecTe[0] /= Dot;
                VecTe[1] /= Dot;
                VecTe[2] /= Dot;
               
                Node1 = VSPGeom().Grid(MGLevel_).EdgeList(i).Node1();
                Node2 = VSPGeom().Grid(MGLevel_).EdgeList(i).Node2();
                
                if ( Loop == VSPGeom().Grid(MGLevel_).EdgeList(i).LoopL() ) {
                 
                   VecX[Node1] += VecTe[0];
                   VecY[Node1] += VecTe[1];
                   VecZ[Node1] += VecTe[2];
                   
                   VecX[Node2] += VecTe[0];
                   VecY[Node2] += VecTe[1];
                   VecZ[Node2] += VecTe[2];
                   
                }
                
                else {
                 
                   VecX[Node1] += VecTe[0];
                   VecY[Node1] += VecTe[1];
                   VecZ[Node1] += VecTe[2];
                   
                   VecX[Node2] += VecTe[0];
                   VecY[Node2] += VecTe[1];
                   VecZ[Node2] += VecTe[2];
                   
                }    
                
             }       
             
          }

       }
       
    }    
    
    // Determine the number of vortex sheets
    
    NumberOfVortexSheets_ = 0;
    
    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {

       NumberOfVortexSheets_ = MAX(NumberOfVortexSheets_, VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j));
       
    }    

    printf("There are: %10d Vortex Sheets \n", NumberOfVortexSheets_);

    if ( VortexSheet_ != NULL ) delete [] VortexSheet_;

    VortexSheet_ = new VORTEX_SHEET[NumberOfVortexSheets_ + 1];

    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
       NumberOfKuttaNodes = 0;
       
       for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {
          
          if ( VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j) == k ) NumberOfKuttaNodes++;
          
       }
       
       VortexSheet(k).SizeTrailingVortexList(NumberOfKuttaNodes);
       
    }
           
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
    
       NumEdges = 0;
       
       VortexSheet(k).TimeAccurate() = TimeAccurate_;
       
       VortexSheet(k).Vinf() = 1.;
    
       VortexSheet(k).TimeStep() = TimeStep_;       
       
       VortexSheet(k).FreeStreamVelocity(0) = FreeStreamVelocity_[0];
       VortexSheet(k).FreeStreamVelocity(1) = FreeStreamVelocity_[1];
       VortexSheet(k).FreeStreamVelocity(2) = FreeStreamVelocity_[2];
              
       for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {
          
          if ( VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j) == k ) {
          
             NumEdges++;
             
             VortexSheet(k).TrailingVortexEdge(NumEdges).TimeAccurate() = TimeAccurate_;

             VortexSheet(k).TrailingVortexEdge(NumEdges).Vinf() = 1.;
          
             VortexSheet(k).TrailingVortexEdge(NumEdges).TimeStep() = TimeStep_;                
                          
             // Pointer to the wing this trailing vortex leaves from
      
             VortexSheet(k).TrailingVortexEdge(NumEdges).Wing() = k;
             
             // Flag if the vortex sheet is periodic (eg would be a nacelle)
             
             VortexSheet(k).IsPeriodic() = VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNodeIsPeriodic(j);

             // Pointer to the kutta node
             
             VortexSheet(k).TrailingVortexEdge(NumEdges).Node() = VSPGeom().Grid(MGLevel_).KuttaNode(j);
                     
             // Pass in edge data and create edge cofficients
             
             VSP_Node1.x() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeX(j);
             VSP_Node1.y() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeY(j);
             VSP_Node1.z() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeZ(j);
                    
             VSP_Node2.x() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeX(j) + WakeAngle_[0] * 1.e6;
             VSP_Node2.y() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeY(j) + WakeAngle_[1] * 1.e6;
             VSP_Node2.z() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeZ(j) + WakeAngle_[2] * 1.e6;
       
             // Set sigma
           
             VortexSheet(k).TrailingVortexEdge(NumEdges).Sigma() = 0.5*Sigma[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
             
             VortexSheet(k).TrailingVortexEdge(NumEdges).Sigma() = Sigma[VSPGeom().Grid(MGLevel_).KuttaNode(j)];

             // Set trailing edge direction 
             
             VortexSheet(k).TrailingVortexEdge(NumEdges).TEVec(0) = VecX[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
             VortexSheet(k).TrailingVortexEdge(NumEdges).TEVec(1) = VecY[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
             VortexSheet(k).TrailingVortexEdge(NumEdges).TEVec(2) = VecZ[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
            
             // Create trailing wakes... specify number of sub vortices per trail
   
             WakeDist =  MAX(VSP_Node1.x() + 0.5*FarDist, Xmax_ + 0.25*FarDist) - VSP_Node1.x();
      
             VortexSheet(k).TrailingVortexEdge(NumEdges).Setup(NumberOfWakeTrailingNodes_,WakeDist,VSP_Node1,VSP_Node2);
             
             // Turn on/off KT correction
             
             VortexSheet(k).TrailingVortexEdge(NumEdges).NoKarmanTsienCorrection() = NoKarmanTsienCorrection_;
             
          }
             
       }
       
       VortexSheet(k).SetupVortexSheets();
       
       if ( VortexSheet(k).IsPeriodic() ) {
          
          printf("There are: %10d kutta nodes for vortex sheet: %10d     <----- Periodic Wake \n",VortexSheet(k).NumberOfTrailingVortices(),k); fflush(NULL);
          
       }
       
       else {
          
          printf("There are: %10d kutta nodes for vortex sheet: %10d  \n",VortexSheet(k).NumberOfTrailingVortices(),k); fflush(NULL);
          
       }

    }

    delete [] Sigma;
    delete [] VecX;
    delete [] VecY;
    delete [] VecZ;

}

/*##############################################################################
#                                                                              #
#                               VSP_SOLVER Solve                               #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Solve(int Case)
{
 
    int i, j, k, p, Loop, Level;
    double Normal[3];
    char StatusFileName[2000], LoadFileName[2000], ADBFileName[2000];
   
    // Zero out solution
   
    zero_double_array(Gamma_,    NumberOfVortexLoops_);    Gamma_[0] = 0.;   
    zero_double_array(GammaNM1_, NumberOfVortexLoops_); GammaNM1_[0] = 0.;
    zero_double_array(GammaNM2_, NumberOfVortexLoops_); GammaNM2_[0] = 0.;
    zero_double_array(Delta_,    NumberOfVortexLoops_);    Delta_[0] = 0.;
        
    CurrentTime_ = 0.;
       
    // Keep track of unsteady forces and moments
    
    if ( TimeAccurate_ ) {
       
       if (  CL_Unsteady_ != NULL ) delete []  CL_Unsteady_;    
       if (  CD_Unsteady_ != NULL ) delete []  CD_Unsteady_;      
       if (  CS_Unsteady_ != NULL ) delete []  CS_Unsteady_;       
       if ( CFx_Unsteady_ != NULL ) delete [] CFx_Unsteady_;      
       if ( CFy_Unsteady_ != NULL ) delete [] CFy_Unsteady_;       
       if ( CFz_Unsteady_ != NULL ) delete [] CFz_Unsteady_;       
       if ( CMx_Unsteady_ != NULL ) delete [] CMx_Unsteady_;       
       if ( CMy_Unsteady_ != NULL ) delete [] CMy_Unsteady_;       
       if ( CMz_Unsteady_ != NULL ) delete [] CMz_Unsteady_;       
             
        CL_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
        CD_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
        CS_Unsteady_ = new double[NumberOfTimeSteps_ + 1];    
       CFx_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CFy_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CFz_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMx_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMy_Unsteady_ = new double[NumberOfTimeSteps_ + 1];
       CMz_Unsteady_ = new double[NumberOfTimeSteps_ + 1];       

       Unsteady_AngleRate_ = 2. * ReducedFrequency_ / Cref_;
       
       TimeStep_ = Cref_ / NumberOfTimeSamples_;

       if ( TimeAnalysisType_ == P_ANALYSIS || 
            TimeAnalysisType_ == Q_ANALYSIS ||
            TimeAnalysisType_ == R_ANALYSIS || 
            TimeAnalysisType_ == HEAVE_ANALYSIS    ) {
               
          TimeStep_ = Cref_ / 16.;
          
          Unsteady_AngleRate_ = 2. * PI / (TimeStep_ * NumberOfTimeSamples_);
          
          ReducedFrequency_ = 0.5 * Unsteady_AngleRate_ * Cref_;

       }
       
       printf("TimeAnalysisType_: %d \n",TimeAnalysisType_);
       
       printf("TimeStep_: %f \n",TimeStep_);

       printf("ReducedFrequency_: %f \n",ReducedFrequency_);
       
       printf("Unsteady_AngleRate_: %f \n",Unsteady_AngleRate_);
       
       if ( TimeAnalysisType_ == P_ANALYSIS ) AngleZero_ = 0.;
       
       if ( TimeAnalysisType_ == Q_ANALYSIS ) AngleZero_ = AngleOfAttack_;
          
       if ( TimeAnalysisType_ == R_ANALYSIS ) AngleZero_ = AngleOfBeta_;
 
    }
           
    // Initialize free stream
    
    InitializeFreeStream();

    // Initialize the wake trailing vortices
    
    InitializeTrailingVortices();
    
    ZeroVortexState();

    // Calculate the right hand side
    
    CalculateRightHandSide();
        
    // Do a restart
    
    if ( DoRestart_ == 1 ) {
     
       LoadRestartFile();
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

           VortexLoop(i).Gamma() = Gamma_[i];
    
        }
        
    }
    
    else {
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

           VortexLoop(i).Gamma() = Gamma_[i] = 0.;
    
        }
               
    }

    // Open status file
    
    if ( Case == 0 || Case == 1 ) {
       
       sprintf(StatusFileName,"%s.history",FileName_);
       
       if ( (StatusFile_ = fopen(StatusFileName, "w")) == NULL ) {
   
          printf("Could not open the history file for output! \n");
   
          exit(1);
   
       }    
       
    }

    // Header for history file
    
    if ( ABS(Case) > 0 ) {
       
       // Write out generic header
       
       WriteCaseHeader(StatusFile_);
       
       // Status update to user
       
       fprintf(StatusFile_,"\n\nSolver Case: %d \n\n",ABS(Case));
       
    }
    
    if ( !TimeAccurate_ ) {

                          //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789          
       fprintf(StatusFile_,"  Iter      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS \n");
   
    }
    
    else {
       
       if ( TimeAnalysisType_ == HEAVE_ANALYSIS ) {
                 
                             //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789     
          fprintf(StatusFile_,"  Time      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS      H       CL_Un     CDi_Un    CS_Un     CFx_Un     CFy_Un    CFz_Un   CMx_Un    CMy_Un    CMz_Un \n");

       }
       
       else if ( TimeAnalysisType_ == P_ANALYSIS || TimeAnalysisType_ == Q_ANALYSIS ||  TimeAnalysisType_ == R_ANALYSIS ) {
         
                             //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789          
          fprintf(StatusFile_,"  Time      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS  UnstdyAng   CL_Un     CDi_Un    CS_Un     CFx_Un     CFy_Un    CFz_Un   CMx_Un    CMy_Un    CMz_Un \n");

       }
       
       else {

                             //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
          fprintf(StatusFile_,"  Time      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS    CL_Un     CDi_Un    CS_Un     CFx_Un     CFy_Un    CFz_Un   CMx_Un    CMy_Un    CMz_Un \n");
      
       }
   
    }

    // Open the adb and case list files the first time only
    
    if ( Case == 0 || Case == 1 ) {

       sprintf(ADBFileName,"%s.adb",FileName_);
       
       if ( (ADBFile_ = fopen(ADBFileName, "wb")) == NULL ) {
   
          printf("Could not open the aerothermal data base file for binary output! \n");
   
          exit(1);
   
       }
       
       sprintf(ADBFileName,"%s.adb.cases",FileName_);
       
       if ( (ADBCaseListFile_ = fopen(ADBFileName, "w")) == NULL ) {
   
          printf("Could not open the aerothermal data base case list file for output! \n");
   
          exit(1);
   
       }       
       
    }    
    
    // Write out ADB Geometry
    
    if ( Case == 0 || Case == 1 ) {

       WriteOutAerothermalDatabaseGeometry();
       
       if ( Write2DFEMFile_ ) WriteFEM2DGeometry();
       
    }             
        
    printf("Solving... \n\n");fflush(NULL);

    if ( DumpGeom_ ) WakeIterations_ = 0;
    
    if ( TimeAccurate_ ) WakeIterations_ = 1;
    
    // Solve at the each time step... or single solve if just a steady state solution

    if ( !TimeAccurate_ ) NumberOfTimeSteps_ = 1;
    
    for ( Time_ = 1 ; Time_ <= NumberOfTimeSteps_ ; Time_++ ) {
       
       CurrentTime_ = Time_*TimeStep_;
       
       // If time accurate we save the trailing vorticity state
   
       if ( TimeAccurate_ ) SaveVortexState();

       // Inner iteration on wake shape

       for ( CurrentWakeIteration_ = 1 ; CurrentWakeIteration_ <= WakeIterations_ ; CurrentWakeIteration_++ ) {
          
          if ( TimeAccurate_ ) {
             
             // Update free stream for unsteady cases
              
             InitializeFreeStream();
             
             // Update right hand side
             
             CalculateRightHandSide();
             
          }          

          // Solve the linear system
   
          SolveLinearSystem();
    
          // Update wake locations

          if ( WakeIterations_ > 1 || TimeAccurate_ ) UpdateWakeLocations();

          // Calculate forces
   
          CalculateForces();
          
          // Output status
   
          OutputStatusFile(0);
       
          printf("\n");
          
       }
       
       // If time accurate, restore the trailing vortex state and convect vorticity
   
       if ( TimeAccurate_ ) ConvectWakeVorticity();
       
       // Write out ADB Solution for time accurate cases
       
       if ( TimeAccurate_ ) {
          
          sprintf(CaseString_,"Time: %-f ...",CurrentTime_);

          InterpolateSolutionFromGrid(1);
          
          WriteOutAerothermalDatabaseSolution();
          
       }
                   
    }

    if ( ForceType_ == FORCE_AVERAGE ) OutputStatusFile(1);

    OutputZeroLiftDragToStatusFile();

    // Open the load file the first time only
    
    if ( Case == 0 || Case == 1 ) {
    
       sprintf(LoadFileName,"%s.lod",FileName_);
       
       if ( (LoadFile_ = fopen(LoadFileName, "w")) == NULL ) {
   
          printf("Could not open the spanwise loading file for output! \n");
   
          exit(1);
   
       }
       
    }       
    
    // Calculate spanwise load distributions for lifting surfaces
    
    CalculateSpanWiseLoading();
    
    // Write out FEM loading file
    
    CreateFEMLoadFile(Case);

    // Interpolate solution from grid 1 to 0
    
    InterpolateSolutionFromGrid(1);

    // Output and survey point results
    
    if ( NumberofSurveyPoints_ > 0 ) CalculateVelocitySurvey();
    
    // Write out 2d FEM geometry if requested
    
    if ( Case == 0 || Case == 1 ) {

       if ( Write2DFEMFile_ ) WriteFEM2DGeometry();
       
    }          

    // Write out ADB Solution

    if ( !TimeAccurate_ ) WriteOutAerothermalDatabaseSolution();
    
    // Write out 2d FEM solution if requested
    
    if ( Write2DFEMFile_ ) WriteFEM2DSolution();
 
    // Close up files
    
    if ( Case <= 0                    ) fclose(StatusFile_);
    if ( Case <= 0                    ) fclose(LoadFile_);
    if ( Case <= 0                    ) fclose(ADBFile_);
    if ( Case <= 0                    ) fclose(ADBCaseListFile_);
    if ( Case <= 0                    ) fclose(FEMLoadFile_);
    if ( Case <= 0 && Write2DFEMFile_ ) fclose(FEM2DLoadFile_);
    
}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER CalculateRightHandSide                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateRightHandSide(void)
{
 
    int i, j, k, p, t, Loop, Level;
    double Normal[3];
    char StatusFileName[2000], LoadFileName[2000], ADBFileName[2000];

    // Update righthandside, Mach dependence 

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       RightHandSide_[i] = -vector_dot(VortexLoop(i).Normal(), LocalFreeStreamVelocity_[i]);

    }
    
    // Modify righthandside for control surface deflections
    
    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          for ( k = 1 ; k <= VSPGeom().VSP_Surface(j).NumberOfControlSurfaces() ; k++ ) {
       
             for ( p = 1 ; p <= VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfLoops() ; p++ ) {
             
                Loop = VSPGeom().VSP_Surface(j).ControlSurface(k).LoopList(p);
          
                Normal[0] = VortexLoop(Loop).Normal()[0];
                Normal[1] = VortexLoop(Loop).Normal()[1];
                Normal[2] = VortexLoop(Loop).Normal()[2];

                VSPGeom().VSP_Surface(j).ControlSurface(k).RotateNormal(Normal);

                RightHandSide_[Loop] = -vector_dot(Normal, LocalFreeStreamVelocity_[Loop]);
                
             }
             
          }
          
       }
       
    }
    
    if ( ModelType_ == PANEL_MODEL ) {
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
          
          if ( LoopIsOnBaseRegion_[i] ) RightHandSide_[i] = 0.;
          
       }       
       
    }    

    for ( Level = 1 ; Level <= NumberOfMGLevels_ ; Level++ ) {
 
       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {

          VSPGeom().Grid(Level).EdgeList(i).Mach() = Mach_;
          
       }
       
    }

    for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {

       TrailingVortexEdge(i).Mach() = Mach_;

    }
    
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER SolveLinearSystem                             #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::SolveLinearSystem(void)
{
 
 double time1, time2;   
  
    // Calculate preconditioners
    
    if ( CurrentWakeIteration_ == 1 ) {

       if ( Preconditioner_ != MATCON ) CalculateDiagonal();       
          
       if ( Preconditioner_ == SSOR   ) CalculateNeighborCoefs();
    
       if ( Preconditioner_ == MATCON ) CreateMatrixPreconditioners();
       
    }

    // Solver the linear system

    Do_GMRES_Solve();    
    
    // Update the vortex strengths on the wake

    UpdateVortexEdgeStrengths(1, ALL_WAKE_GAMMAS);

    if ( SaveRestartFile_ ) WriteRestartFile();
    
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER CalculateDiagonal                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateDiagonal(void)
{

    int i, j, Edge, Node1, Node2;
    double q[4], Ws;
    
    zero_double_array(Diagonal_,NumberOfVortexLoops_);
     
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       // Loop 1
       
       SurfaceVortexEdge(j).Gamma() = 1.;
       
       i = SurfaceVortexEdge(j).VortexLoop1();

       SurfaceVortexEdge(j).InducedVelocity(VortexLoop(i).xyz_c(), q);

       Diagonal_[i] += vector_dot(VortexLoop(i).Normal(), q);
       
       // Loop 2
       
       SurfaceVortexEdge(j).Gamma() = -1.;
       
       i = SurfaceVortexEdge(j).VortexLoop2();
     
       SurfaceVortexEdge(j).InducedVelocity(VortexLoop(i).xyz_c(), q);
  
       Diagonal_[i] += vector_dot(VortexLoop(i).Normal(), q);
       
    }
 
    // If flow is supersonic add in generalized principart part of downwash
    
    if ( Mach_ > 1. ) {

       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

          Ws = SurfaceVortexEdge(j).GeneralizedPrincipalPartOfDownWash();
        
          // Loop 1
           
          if ( SurfaceVortexEdge(j).VortexLoop1IsDownWind() ) {
           
             i = SurfaceVortexEdge(j).VortexLoop1();
        
             Diagonal_[i] += Ws * SurfaceVortexEdge(j).VortexLoop1DownWindWeight();
       
          }

          // Loop 2

          if ( SurfaceVortexEdge(j).VortexLoop2IsDownWind() ) {
           
             i = SurfaceVortexEdge(j).VortexLoop2();

             Diagonal_[i] += Ws * SurfaceVortexEdge(j).VortexLoop2DownWindWeight();
       
          }

       }
       
    }
  
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       if ( ABS(Diagonal_[i]) == 0. ) {
        
          if ( VortexLoop(i).SurfaceType() == DEGEN_BODY_SURFACE ) printf("Loop: %d on body surface: %d has zero diagonal... \n",i,VortexLoop(i).DegenBodyID());fflush(NULL);
          if ( VortexLoop(i).SurfaceType() == DEGEN_WING_SURFACE ) printf("Loop: %d on wing surface: %d has zero diagonal... \n",i,VortexLoop(i).DegenWingID());fflush(NULL);
          if ( VortexLoop(i).SurfaceType() == CART3D_SURFACE     ) printf("Loop: %d on wing surface: %d has zero diagonal... \n",i,VortexLoop(i).Cart3dID());fflush(NULL);
         
          printf("Area: %e \n",VortexLoop(i).Area());
          
          for ( j = 1 ; j <= VortexLoop(i).NumberOfEdges() ; j++ ) {
             
             Edge = VortexLoop(i).Edge(j);
             
             Node1 = SurfaceVortexEdge(j).Node1();
             Node2 = SurfaceVortexEdge(j).Node2();
             
             printf("Edge: %d --> Node 1: %d @ %lf %lf %lf ... Node 2: %d @ %lf %lf %lf \n",
                    Edge,
                    Node1,
                    VSPGeom().Grid(MGLevel_).NodeList(Node1).x(),
                    VSPGeom().Grid(MGLevel_).NodeList(Node1).y(),
                    VSPGeom().Grid(MGLevel_).NodeList(Node1).z(),
                    Node2,
                    VSPGeom().Grid(MGLevel_).NodeList(Node2).x(),
                    VSPGeom().Grid(MGLevel_).NodeList(Node2).y(),
                    VSPGeom().Grid(MGLevel_).NodeList(Node2).z());
          }                    
                    
       }
     
       Diagonal_[i] = 1./Diagonal_[i];
        
    }  
    
    if ( ModelType_ == PANEL_MODEL ) {
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
          
          if ( LoopIsOnBaseRegion_[i] ) Diagonal_[i] = 1.;
          
       }       
       
    }      
    
    // Find smallest diagonal coefficient
    
    MaxDiagonal_ = -1.e9;

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       MaxDiagonal_ = MAX(MaxDiagonal_,Diagonal_[i]);

    }    
          
}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CalculateNeighborCoefs                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateNeighborCoefs(void)
{

    int j, k, Edge, Loop1, Loop2;
    double q[4];

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       SurfaceVortexEdge(j).EdgeCoef(0) = 0.;
       SurfaceVortexEdge(j).EdgeCoef(1) = 0.;

    }
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       Loop1 = SurfaceVortexEdge(j).VortexLoop1();
       Loop2 = SurfaceVortexEdge(j).VortexLoop2();
       
       for ( k = 1 ; k <= VortexLoop(Loop2).NumberOfEdges() ; k++ ) {
          
          Edge = VortexLoop(Loop2).Edge(k);
             
          SurfaceVortexEdge(Edge).Gamma() = 1.;
          
          if ( SurfaceVortexEdge(Edge).VortexLoop1() != Loop2 ) SurfaceVortexEdge(Edge).Gamma() = -1.;
          
          SurfaceVortexEdge(Edge).InducedVelocity(VortexLoop(Loop1).xyz_c(), q);
          
          SurfaceVortexEdge(j).EdgeCoef(0) += vector_dot(VortexLoop(Loop1).Normal(), q);
          
       }
       
       Loop1 = SurfaceVortexEdge(j).VortexLoop2();
       Loop2 = SurfaceVortexEdge(j).VortexLoop1();
              
       for ( k = 1 ; k <= VortexLoop(Loop2).NumberOfEdges() ; k++ ) {
          
          Edge = VortexLoop(Loop2).Edge(k);
             
          SurfaceVortexEdge(Edge).Gamma() = 1.;
          
          if ( SurfaceVortexEdge(Edge).VortexLoop1() != Loop2 ) SurfaceVortexEdge(Edge).Gamma() = -1.;
          
          SurfaceVortexEdge(Edge).InducedVelocity(VortexLoop(Loop1).xyz_c(), q);
          
          SurfaceVortexEdge(j).EdgeCoef(1) += vector_dot(VortexLoop(Loop1).Normal(), q);
          
       }
       
    }       

    // Scale the coefficients... note that Diagonal_ holds the inverse 
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       SurfaceVortexEdge(j).EdgeCoef(0) *= Diagonal_[SurfaceVortexEdge(j).VortexLoop1()];
       SurfaceVortexEdge(j).EdgeCoef(1) *= Diagonal_[SurfaceVortexEdge(j).VortexLoop2()];

    }    
    
    // Zero out boundaries
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       if ( SurfaceVortexEdge(j).VortexLoop1() == 0 ) SurfaceVortexEdge(j).EdgeCoef(1) = 0.;
       if ( SurfaceVortexEdge(j).VortexLoop2() == 0 ) SurfaceVortexEdge(j).EdgeCoef(0) = 0.;

    }      
    
    // Zero out trailing edges
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {
          
          SurfaceVortexEdge(j).EdgeCoef(0) = 0.;
          SurfaceVortexEdge(j).EdgeCoef(1) = 0.;
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CreateMatrixPreconditioners                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateMatrixPreconditioners(void)
{

    int i, j, k, m, Neq, iLoop, jLoop, Edge;
    double q[4];

#pragma omp parallel for private(Neq,i,j,m,iLoop,jLoop,Edge,q)
    for ( k = 1 ; k <= NumberOfMatrixPreconditioners_ ; k++ ) {
       
       Neq = MatrixPreconditionerList_[k].NumberOfVortexLoops();
       
       for ( i = 1 ; i <= Neq ; i++ ) {
          
          iLoop = MatrixPreconditionerList_[k].VortexLoopList(i);

          for ( j = 1 ; j <= Neq ; j++ ) {
             
             jLoop = MatrixPreconditionerList_[k].VortexLoopList(j);
             
             MatrixPreconditionerList_[k].A()(i,j) = 0.;
             
             for ( m = 1 ; m <= VortexLoop(jLoop).NumberOfEdges() ; m++ ) {
                
                Edge = VortexLoop(jLoop).Edge(m);
                
                if ( !SurfaceVortexEdge(Edge).IsTrailingEdge() ) {
                   
                   SurfaceVortexEdge(Edge).Gamma() = 1.;
                   
                   if ( SurfaceVortexEdge(Edge).VortexLoop1() != jLoop ) SurfaceVortexEdge(Edge).Gamma() = -1.;
                   
                   SurfaceVortexEdge(Edge).InducedVelocity(VortexLoop(iLoop).xyz_c(), q);
                   
                   MatrixPreconditionerList_[k].A()(i,j) += vector_dot(VortexLoop(iLoop).Normal(), q);
                   
                }
 
             }

          }

       }
       
       MatrixPreconditionerList_[k].LU();
       
    }
                    
}

/*##############################################################################
#                                                                              #
#           VSP_SOLVER CreateMatrixPreconditionersDataStructure                #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateMatrixPreconditionersDataStructure(void)
{

    int i, j, k, p, r, Done, Loops, Edge, Level, *LoopList, *NumLoops;
    int MinLoops, MaxLoops, AvgLoops;
    
    printf("Creating matrix preconditioners... \n");
    
    LoopList = new int[NumberOfVortexLoops_ + 1];
    
    NumLoops = new int[NumberOfVortexLoops_ + 1];
    
    zero_int_array(LoopList, NumberOfVortexLoops_);
    
    zero_int_array(NumLoops, NumberOfVortexLoops_);

    // Determine level to start at
    
    MaxLoops = 500;
    
    Done = 0;
    
    Level = VSPGeom().NumberOfGridLevels();
    

    while ( !Done && Level >= 1 ) {
       
       Loops = 0;

       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfLoops() ; i++ ) {
          
          NumLoops[i] = CalculateNumberOfFineLoops(Level, VSPGeom().Grid(Level).LoopList(i), LoopList);
 
          Loops = MAX(Loops,NumLoops[i]);

       }
       
       if ( Loops <= 1.25*MaxLoops ) Done = 1;

       Level--;
       
    }
    
    Level++;
    
    // Now group vortex loops into groups, one each per matrix preconditioner

    MatrixPreconditionerList_ = new MATPRECON[VSPGeom().Grid(Level).NumberOfLoops() + 1];

    i = 1; 

    Done = Loops = p = 0;
    
    zero_int_array(LoopList, NumberOfVortexLoops_);
    
    MinLoops = 1.e9;
    MaxLoops = -MinLoops;
    AvgLoops = 0;
    
    while ( i <= VSPGeom().Grid(Level).NumberOfLoops() && !Done ) {
  
       Loops += CalculateNumberOfFineLoops(Level, VSPGeom().Grid(Level).LoopList(i), LoopList);

       if ( ( Loops >= MaxLoops || Loops + NumLoops[i+1] > 1.25*MaxLoops ) || i == VSPGeom().Grid(Level).NumberOfLoops() ) {
/*       
          // Now buffer this region 
          
          for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {
             
             if ( LoopList[j] == 1 ) {
                
                for ( k = 1 ; k <= VSPGeom().Grid(1).LoopList(j).NumberOfEdges() ; k++ ) {

                   Edge = VSPGeom().Grid(1).LoopList(j).Edge(k);
                
                   r = VSPGeom().Grid(1).EdgeList(Edge).Loop1()
                     + VSPGeom().Grid(1).EdgeList(Edge).Loop2() - j;
                        
                   if ( r > 0 && LoopList[r] == 0 ) {
                      
                      LoopList[r] = 2;
                      
                      Loops++;
                      
                   }
                   
                }
                
             }
             
          }
*/
          MinLoops = MIN(MinLoops,Loops);
          MaxLoops = MAX(MaxLoops,Loops);
          AvgLoops += Loops;
          
          p++;
          
          if ( Verbose_ ) printf("Preconditioning Matrix %d contains %d fine loops \n",p,Loops); fflush(NULL);
   
          MatrixPreconditionerList_[p].Size(Loops);

          k = 0;
          
          for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {
             
             if ( LoopList[j] ) MatrixPreconditionerList_[p].VortexLoopList(++k) = j;

          }
          
          if ( k != Loops ) {
             
             printf("Error in creating preconditioning matrix data structure! \n");
             
             fflush(NULL);
             
             exit(1);
             
          }
   
          zero_int_array(LoopList, NumberOfVortexLoops_);
          
          Loops = 0;
          
       }
       
       i++;
       
    }
    
    AvgLoops /= p;
    
    printf("Min matrix: %d Loops \n",MinLoops);
    printf("Max matrix: %d Loops \n",MaxLoops);
    printf("Avg matrix: %d Loops \n",AvgLoops);

    printf("\n");
    
    NumberOfMatrixPreconditioners_ = p;
    
    delete [] LoopList;
    delete [] NumLoops;
    

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER CalculateNumberOfFineLoops                    #
#                                                                              #
##############################################################################*/

int VSP_SOLVER::CalculateNumberOfFineLoops(int Level, VSP_LOOP &Loop, int *LoopList)
{

    int i, FineLoops;

    FineLoops = 0;
    
    if ( Level == 2 ) {

       for ( i = 1 ; i <= Loop.NumberOfFineGridLoops() ; i++ ) {
    
          FineLoops++;
          
          LoopList[Loop.FineGridLoop(i)] = 1;

       }
       
    }
    
    else {
  
       for ( i = 1 ; i <= Loop.NumberOfFineGridLoops() ; i++ ) {
    
          FineLoops += CalculateNumberOfFineLoops(Level-1,VSPGeom().Grid(Level-1).LoopList(Loop.FineGridLoop(i)),LoopList);

       }
       
    }
    
    return FineLoops;

}

/*##############################################################################
#                                                                              #
#                 VSP_SOLVER DoPreconditionedMatrixMultiply                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DoPreconditionedMatrixMultiply(double *vec_in, double *vec_out)
{
 
    DoMatrixMultiply(vec_in,vec_out);

    DoMatrixPrecondition(vec_out);
   
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER DoMatrixMultiply                            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DoMatrixMultiply(double *vec_in, double *vec_out)
{

    if ( ModelType_ == VLM_MODEL ) {
      
       MatrixMultiply(vec_in, vec_out);
      
    }
   
    else if ( ModelType_ == PANEL_MODEL ) {

       MatrixMultiply(vec_in, MatrixVecTemp_);
    
       MatrixTransposeMultiply(MatrixVecTemp_, vec_out);

    }

}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER MatrixMultiply                            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::MatrixMultiply(double *vec_in, double *vec_out)
{

    int i, j, k, Level;
    double xyz[3], q[4], Ws, Temp;
    VSP_EDGE *VortexEdge;
    
    zero_double_array(vec_out,NumberOfVortexLoops_);
    
    Gamma_[0] = 0.;
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       Gamma_[i] = vec_in[i];
      
    }

    UpdateVortexEdgeStrengths(1, IMPLICIT_WAKE_GAMMAS);
              
    // Surface vortex induced velocities

    for ( Level = 1 ; Level < NumberOfMGLevels_ ; Level++ ) {
       
       RestrictSolutionFromGrid(Level);
           
       UpdateVortexEdgeStrengths(Level+1, IMPLICIT_WAKE_GAMMAS);
  
    }

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
    
       Temp = 0.;

#pragma omp parallel for reduction(+:Temp) private(xyz,q,VortexEdge)            
       for ( j = 1 ; j <= NumberOfVortexEdgesForInteractionListEntry_[i] ; j++ ) {
        
          VortexEdge = SurfaceVortexEdgeInteractionList_[i][j];
      
          if ( !VortexEdge->IsTrailingEdge() ) {              

             // Calculate influence of this edge
             
             VortexEdge->InducedVelocity(VortexLoop(i).xyz_c(), q);
         
             Temp += vector_dot(VortexLoop(i).Normal(), q);
             
             // If there is ground effects, z plane...
             
             if ( DoGroundEffectsAnalysis() ) {

               xyz[0] = VortexLoop(i).xyz_c()[0];
               xyz[1] = VortexLoop(i).xyz_c()[1];
               xyz[2] = VortexLoop(i).xyz_c()[2];
               
               xyz[2] *= -1.;
               
               VortexEdge->InducedVelocity(xyz, q);
         
               q[2] *= -1.;
     
               Temp += vector_dot(VortexLoop(i).Normal(), q);
               
             }    
                          
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {

                xyz[0] = VortexLoop(i).xyz_c()[0];
                xyz[1] = VortexLoop(i).xyz_c()[1];
                xyz[2] = VortexLoop(i).xyz_c()[2];
               
                if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
               
                VortexEdge->InducedVelocity(xyz, q);
         
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
     
                Temp += vector_dot(VortexLoop(i).Normal(), q);
                  
                if ( DoGroundEffectsAnalysis() ) {
   
                  xyz[2] *= -1.;
                  
                  VortexEdge->InducedVelocity(xyz, q);
            
                  if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                  if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                        q[2] *= -1.;

                  Temp += vector_dot(VortexLoop(i).Normal(), q);
                  
                }                   
               
             }             
             
          }
            
       }
       
       vec_out[i] = Temp;
       
    }

    // If flow is supersonic add in generalized principart part of downwash
    
    if ( Mach_ > 1. ) {

#pragma omp parallel for private(Ws,i)     
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

          if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {   
         
             Ws = SurfaceVortexEdge(j).GeneralizedPrincipalPartOfDownWash();
            
             // Loop 1
              
             if ( SurfaceVortexEdge(j).VortexLoop1IsDownWind() ) {
              
                i = SurfaceVortexEdge(j).VortexLoop1();
            
                vec_out[i] += Ws * SurfaceVortexEdge(j).Gamma() * SurfaceVortexEdge(j).VortexLoop1DownWindWeight();
          
             }

             // Loop 2

             if ( SurfaceVortexEdge(j).VortexLoop2IsDownWind() ) {
              
                i = SurfaceVortexEdge(j).VortexLoop2();

                vec_out[i] -= Ws * SurfaceVortexEdge(j).Gamma() * SurfaceVortexEdge(j).VortexLoop2DownWindWeight();
          
             }
             
          }

       }
       
    }    
 
    // Trailing vortex induced velocities
     
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
      for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

         xyz[0] = VortexLoop(i).xyz_c()[0];
         xyz[1] = VortexLoop(i).xyz_c()[1];
         xyz[2] = VortexLoop(i).xyz_c()[2];
           
         VortexSheet(k).InducedVelocity(xyz, q);

         Temp = vector_dot(VortexLoop(i).Normal(), q);

         // If there is ground effects, z plane...
       
         if ( DoGroundEffectsAnalysis() ) {
          
           xyz[0] = VortexLoop(i).xyz_c()[0];
           xyz[1] = VortexLoop(i).xyz_c()[1];
           xyz[2] = VortexLoop(i).xyz_c()[2];
         
           xyz[2] *= -1.;
         
           VortexSheet(k).InducedVelocity(xyz, q);
   
           q[2] *= -1.;
         
           Temp += vector_dot(VortexLoop(i).Normal(), q);
         
         }   
         
         // If there is a symmetry plane, calculate influence of the reflection
       
         if ( DoSymmetryPlaneSolve_ ) {
          
           xyz[0] = VortexLoop(i).xyz_c()[0];
           xyz[1] = VortexLoop(i).xyz_c()[1];
           xyz[2] = VortexLoop(i).xyz_c()[2];
         
           if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
           if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
           if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
         
           VortexSheet(k).InducedVelocity(xyz, q);
   
           if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
           if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
           if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
         
           Temp += vector_dot(VortexLoop(i).Normal(), q);
           
            // If there is ground effects, z plane...
          
            if ( DoGroundEffectsAnalysis() ) {

              xyz[2] *= -1.;
            
              VortexSheet(k).InducedVelocity(xyz, q);
   
              if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
              if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                    q[2] *= -1.;
            
              Temp += vector_dot(VortexLoop(i).Normal(), q);
            
            }   
        
         }   

         vec_out[i] += Temp;

      }

    }

    vec_out[0] = vec_in[0];

    // Kelvin constraint
    
    if ( ModelType_ == PANEL_MODEL ) {

       for ( k = 1 ; k <= NumberOfKelvinConstraints_ ; k++ ) {
          
          vec_out[NumberOfVortexLoops_ + k] = 0.;
          
       }
       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
          
          vec_out[NumberOfVortexLoops_ + LoopInKelvinConstraintGroup_[i]] += vec_in[i];
          
       }

    }

    // Base region
    
    if ( ModelType_ == PANEL_MODEL ) {
       
       for ( k = 1 ; k <= NumberOfVortexLoops_ ; k++ ) {
          
          if ( LoopIsOnBaseRegion_[k] ) vec_out[k] = vec_in[k];
          
       }       
       
    }
    
}

/*##############################################################################
#                                                                              #
#                   VSP_SOLVER MatrixTransposeMultiply                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::MatrixTransposeMultiply(double *vec_in, double *vec_out)
{

    int i;
    
    vec_out[0] = vec_in[0];
    
    // Kelvin constraint

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       vec_out[i] = vec_in[i] + vec_in[NumberOfVortexLoops_ + LoopInKelvinConstraintGroup_[i]];
    
    }

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER DoMatrixPrecondition                          #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DoMatrixPrecondition(double *vec_in)
{

    int i, j, k;

    // Precondition using Jacobi

    if ( Preconditioner_ == JACOBI ) {
       
#pragma omp parallel for    
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
          vec_in[i] *= JacobiRelaxationFactor_*Diagonal_[i];
   
       }
       
    }

    else if ( Preconditioner_ == SSOR ) {

#pragma omp parallel for    
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
         vec_in[i] *= (2.-JacobiRelaxationFactor_)*Diagonal_[i] / JacobiRelaxationFactor_;

       }
   
       // Edge SSOR
   
#pragma omp parallel for       
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
           vec_in[i] *= JacobiRelaxationFactor_;
   
       }

       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
          
          if ( SurfaceVortexEdge(j).VortexLoop2() < SurfaceVortexEdge(j).VortexLoop1() ) { 
             
             vec_in[SurfaceVortexEdge(j).VortexLoop1()] -= SurfaceVortexEdge(j).EdgeCoef(0) * vec_in[SurfaceVortexEdge(j).VortexLoop2()];
   
          }
          
          else {
             
             vec_in[SurfaceVortexEdge(j).VortexLoop2()] -= SurfaceVortexEdge(j).EdgeCoef(1) * vec_in[SurfaceVortexEdge(j).VortexLoop1()];
     
          }
   
       }    

#pragma omp parallel for            
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
           vec_in[i] *= JacobiRelaxationFactor_;
   
       }

       for ( j = NumberOfSurfaceVortexEdges_ ; j >= 1 ; j-- ) {
          
          if ( SurfaceVortexEdge(j).VortexLoop2() > SurfaceVortexEdge(j).VortexLoop1() ) { 
             
             vec_in[SurfaceVortexEdge(j).VortexLoop1()] -= SurfaceVortexEdge(j).EdgeCoef(0) * vec_in[SurfaceVortexEdge(j).VortexLoop2()];
             
          }
          
          else {
             
             vec_in[SurfaceVortexEdge(j).VortexLoop2()] -= SurfaceVortexEdge(j).EdgeCoef(1) * vec_in[SurfaceVortexEdge(j).VortexLoop1()];
             
          }
          
       }     

    }
    
    else if ( Preconditioner_ == MATCON ) {

#pragma omp parallel for         
       for ( k = 1 ; k <= NumberOfMatrixPreconditioners_ ; k++ ) {

          for ( i = 1 ; i <= MatrixPreconditionerList_[k].NumberOfVortexLoops() ; i++ ) {

             MatrixPreconditionerList_[k].x(i) = vec_in[MatrixPreconditionerList_[k].VortexLoopList(i)];
             
          }
          
          MatrixPreconditionerList_[k].Solve();
          
          for ( i = 1 ; i <= MatrixPreconditionerList_[k].NumberOfVortexLoops() ; i++ ) {
      
             vec_in[MatrixPreconditionerList_[k].VortexLoopList(i)] = MatrixPreconditionerList_[k].x(i);
 
          }          
          
       }

    }

    else {
       
       printf("Unknown preconditioner! \n");fflush(NULL);
       exit(1);
       
    }
    
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateVelocities                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateVelocities(void)
{

    int i, j, k, Level;
    double q[3], xyz[3], Ws, U, V, W;
    VSP_EDGE *VortexEdge;
    
    // Freestream component... includes rotor wash, and any rotational rates
    
#pragma omp parallel for
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
       VortexLoop(i).U() = LocalFreeStreamVelocity_[i][0];
       VortexLoop(i).V() = LocalFreeStreamVelocity_[i][1];
       VortexLoop(i).W() = LocalFreeStreamVelocity_[i][2];

    }
       
    // Update the vortex strengths

    UpdateVortexEdgeStrengths(1, ALL_WAKE_GAMMAS);
    
    // Surface vortex induced velocities

    for ( Level = 1 ; Level < NumberOfMGLevels_ ; Level++ ) {
        
       RestrictSolutionFromGrid(Level);
       
       UpdateVortexEdgeStrengths(Level+1, ALL_WAKE_GAMMAS);

    }

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
    
       U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(q,VortexEdge,xyz)            
       for ( j = 1 ; j <= NumberOfVortexEdgesForInteractionListEntry_[i] ; j++ ) {
        
          VortexEdge = SurfaceVortexEdgeInteractionList_[i][j];
       
          if ( !VortexEdge->IsTrailingEdge() ) {               

             VortexEdge->InducedVelocity(VortexLoop(i).xyz_c(), q);
         
             U += q[0];
             V += q[1];
             W += q[2];
          
             // If there is ground effects, z plane...
             
             if ( DoGroundEffectsAnalysis() ) {
                
                xyz[0] = VortexLoop(i).xyz_c()[0];
                xyz[1] = VortexLoop(i).xyz_c()[1];
                xyz[2] = VortexLoop(i).xyz_c()[2];
      
                xyz[2] *= -1.;
               
                VortexEdge->InducedVelocity(xyz, q);        
      
                q[2] *= -1.;
               
                U += q[0];
                V += q[1];
                W += q[2];
               
             }     
                       
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {
                
                xyz[0] = VortexLoop(i).xyz_c()[0];
                xyz[1] = VortexLoop(i).xyz_c()[1];
                xyz[2] = VortexLoop(i).xyz_c()[2];
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
               
                VortexEdge->InducedVelocity(xyz, q);        
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
               
                U += q[0];
                V += q[1];
                W += q[2];
                
                // If there is ground effects, z plane...
                
                if ( DoGroundEffectsAnalysis() ) {
      
                   xyz[2] *= -1.;
                  
                   VortexEdge->InducedVelocity(xyz, q);        
         
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;         
                                                         q[2] *= -1.;
                  
                   U += q[0];
                   V += q[1];
                   W += q[2];
                  
                }                     
               
             }                
                         
          }
          
       }
       
       VortexLoop(i).U() += U;
       VortexLoop(i).V() += V;
       VortexLoop(i).W() += W; 
       
    }
    
    // Trailing vortex induced velocities
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       U = V = W = 0.;
       
       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
          
          xyz[0] = VortexLoop(i).xyz_c()[0];
          xyz[1] = VortexLoop(i).xyz_c()[1];
          xyz[2] = VortexLoop(i).xyz_c()[2];
                   
          VortexSheet(k).InducedVelocity(xyz, q);
 
          U += q[0];
          V += q[1];
          W += q[2];
   
          // If there is a symmetry plane, calculate influence of the reflection
             
          if ( DoGroundEffectsAnalysis() ) {
             
             xyz[2] *= -1.;
            
             VortexSheet(k).InducedVelocity(xyz, q);
   
             q[2] *= -1.;
            
             U += q[0];
             V += q[1];
             W += q[2];
            
          }   
             
          // If there is a symmetry plane, calculate influence of the reflection
             
          if ( DoSymmetryPlaneSolve_ ) {
             
             xyz[0] = VortexLoop(i).xyz_c()[0];
             xyz[1] = VortexLoop(i).xyz_c()[1];
             xyz[2] = VortexLoop(i).xyz_c()[2];
                       
             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
            
             VortexSheet(k).InducedVelocity(xyz, q);
   
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
            
             U += q[0];
             V += q[1];
             W += q[2];
            
             // If there is a symmetry plane, calculate influence of the reflection
                
             if ( DoGroundEffectsAnalysis() ) {
                
                xyz[2] *= -1.;
               
                VortexSheet(k).InducedVelocity(xyz, q);
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                      q[2] *= -1.;
               
                U += q[0];
                V += q[1];
                W += q[2];
               
             }   
                      
          }   

       }
       
       VortexLoop(i).U() += U;
       VortexLoop(i).V() += V;
       VortexLoop(i).W() += W;
         
    }
       
    // If flow is supersonic add in generalized principart part of downwash
    
    if ( Mach_ > 1. ) {

#pragma omp parallel for private(i,j,Ws)
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

          Ws = SurfaceVortexEdge(j).GeneralizedPrincipalPartOfDownWash();
         
          // Loop 1
           
          if ( SurfaceVortexEdge(j).VortexLoop1IsDownWind() ) {
           
             i = SurfaceVortexEdge(j).VortexLoop1();
         
             Ws *= SurfaceVortexEdge(j).Gamma() * SurfaceVortexEdge(j).VortexLoop1DownWindWeight();
           
             VortexLoop(i).U() += Ws * VortexLoop(i).Normal()[0];
             VortexLoop(i).V() += Ws * VortexLoop(i).Normal()[1];
             VortexLoop(i).W() += Ws * VortexLoop(i).Normal()[2];
 
          }

          // Loop 2

          if ( SurfaceVortexEdge(j).VortexLoop2IsDownWind() ) {
           
             i = SurfaceVortexEdge(j).VortexLoop2();

             Ws *= SurfaceVortexEdge(j).Gamma() * SurfaceVortexEdge(j).VortexLoop2DownWindWeight();
             
             VortexLoop(i).U() -= Ws * VortexLoop(i).Normal()[0];
             VortexLoop(i).V() -= Ws * VortexLoop(i).Normal()[1];
             VortexLoop(i).W() -= Ws * VortexLoop(i).Normal()[2];
      
          }

       }
       
    }   
    
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER UpdateWakeLocations                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::UpdateWakeLocations(void)
{

    int i, j, k, m, Iter, IterMax, Done;
    double xyz[3], xyz_te[3], q[5], U, V, W, Delta, MaxDelta;

    // Initialize to free stream values

    for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {

       for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
        
          VortexSheet(m).TrailingVortexEdge(i).FreeStreamVelocity(0) = FreeStreamVelocity_[0];
          VortexSheet(m).TrailingVortexEdge(i).FreeStreamVelocity(1) = FreeStreamVelocity_[1];
          VortexSheet(m).TrailingVortexEdge(i).FreeStreamVelocity(2) = FreeStreamVelocity_[2];
             
          for ( j = 1 ; j <= VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
      
             VortexSheet(m).TrailingVortexEdge(i).Utmp(j) = FreeStreamVelocity_[0];
             VortexSheet(m).TrailingVortexEdge(i).Vtmp(j) = FreeStreamVelocity_[1];
             VortexSheet(m).TrailingVortexEdge(i).Wtmp(j) = FreeStreamVelocity_[2];
             
          }
   
       }
       
    }
    
    // Add in the rotor induced velocities
 
    for ( k = 1 ; k <= NumberOfRotors_ ; k++ ) {
     
       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     

          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
            
             for ( j = 1 ; j <= VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
   
                xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
             
                RotorDisk(k).Velocity(xyz, q);                   
   
                VortexSheet(m).TrailingVortexEdge(i).Utmp(j) += q[0] / Vinf_;
                VortexSheet(m).TrailingVortexEdge(i).Vtmp(j) += q[1] / Vinf_;
                VortexSheet(m).TrailingVortexEdge(i).Wtmp(j) += q[2] / Vinf_;
             
                // If there is ground effects, z plane...
                
                if ( DoGroundEffectsAnalysis() ) {
        
                   xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                   xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                   xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
                        
                   xyz[2] *= -1.;
                  
                   RotorDisk(k).Velocity(xyz, q);        
         
                   q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortexEdge(i).Utmp(j) += q[0] / Vinf_;
                   VortexSheet(m).TrailingVortexEdge(i).Vtmp(j) += q[1] / Vinf_;
                   VortexSheet(m).TrailingVortexEdge(i).Wtmp(j) += q[2] / Vinf_;
                  
                }     
                                
                // If there is a symmetry plane, calculate influence of the reflection
                
                if ( DoSymmetryPlaneSolve_ ) {
        
                   xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                   xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                   xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
        
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
                  
                   RotorDisk(k).Velocity(xyz, q);        
         
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortexEdge(i).Utmp(j) += q[0] / Vinf_;
                   VortexSheet(m).TrailingVortexEdge(i).Vtmp(j) += q[1] / Vinf_;
                   VortexSheet(m).TrailingVortexEdge(i).Wtmp(j) += q[2] / Vinf_;
                  
                   // If there is ground effects, z plane...
                   
                   if ( DoGroundEffectsAnalysis() ) {
   
                      xyz[2] *= -1.;
                     
                      RotorDisk(k).Velocity(xyz, q);        
            
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;            
                                                            q[2] *= -1.;
                     
                      VortexSheet(m).TrailingVortexEdge(i).Utmp(j) += q[0] / Vinf_;
                      VortexSheet(m).TrailingVortexEdge(i).Vtmp(j) += q[1] / Vinf_;
                      VortexSheet(m).TrailingVortexEdge(i).Wtmp(j) += q[2] / Vinf_;
                     
                   }  
                                  
                }                
   
             }
             
          }    

       }
       
    }
    
    // Wing surface vortex induced velocities

    for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     
              
       for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
                         
          for ( j = 1 ; j <= VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
           
             xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
             xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
             xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
                
             CalculateSurfaceInducedVelocityAtPoint(xyz, q);
   
             VortexSheet(m).TrailingVortexEdge(i).Utmp(j) += q[0];
             VortexSheet(m).TrailingVortexEdge(i).Vtmp(j) += q[1];
             VortexSheet(m).TrailingVortexEdge(i).Wtmp(j) += q[2];
         
             // If there is ground effects, z plane ...
             
             if ( DoGroundEffectsAnalysis() ) {
     
                xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
                     
                xyz[2] *= -1.;
               
                CalculateSurfaceInducedVelocityAtPoint(xyz, q);
      
                q[2] *= -1.;
               
                VortexSheet(m).TrailingVortexEdge(i).Utmp(j) += q[0];
                VortexSheet(m).TrailingVortexEdge(i).Vtmp(j) += q[1];
                VortexSheet(m).TrailingVortexEdge(i).Wtmp(j) += q[2];
                
             }
                          
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {
     
                xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0]; 
                xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];        
                xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2]; 
             
                if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
               
                CalculateSurfaceInducedVelocityAtPoint(xyz, q);
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
               
                VortexSheet(m).TrailingVortexEdge(i).Utmp(j) += q[0];
                VortexSheet(m).TrailingVortexEdge(i).Vtmp(j) += q[1];
                VortexSheet(m).TrailingVortexEdge(i).Wtmp(j) += q[2];
                
                // If there is ground effects, z plane ...
                
                if ( DoGroundEffectsAnalysis() ) {
 
                   xyz[2] *= -1.;
                  
                   CalculateSurfaceInducedVelocityAtPoint(xyz, q);
         
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;         
                                                         q[2] *= -1.;
                  
                   VortexSheet(m).TrailingVortexEdge(i).Utmp(j) += q[0];
                   VortexSheet(m).TrailingVortexEdge(i).Vtmp(j) += q[1];
                   VortexSheet(m).TrailingVortexEdge(i).Wtmp(j) += q[2];
                   
                }
                             
             }
             
          }
          
       }
       
    }

    // Iterate on wake shape
    
    Done = 0;
    
    Iter = 1;
    
    IterMax = 5; 
    
    if ( TimeAccurate_ ) IterMax = 1;

    while ( Iter <= IterMax && !Done ) {

       // Initialize with freestream, surface induced, and rotor induced velocities... these have not changed

       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {    

          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
   
             for ( j = 1 ; j <= VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
   
                VortexSheet(m).TrailingVortexEdge(i).U(j) = VortexSheet(m).TrailingVortexEdge(i).Utmp(j);
                VortexSheet(m).TrailingVortexEdge(i).V(j) = VortexSheet(m).TrailingVortexEdge(i).Vtmp(j);
                VortexSheet(m).TrailingVortexEdge(i).W(j) = VortexSheet(m).TrailingVortexEdge(i).Wtmp(j);
                
             }
             
          }
          
       }
       
       // Trailing vortex induced velocities

       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     
           
          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
                           
             for ( j = 1 ; j <= VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() ; j++ ) {
   
                U = V = W = 0.;
                
                for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {     
   
                   xyz_te[0] = VortexSheet(m).TrailingVortexEdge(i).TE_Node().x();
                   xyz_te[1] = VortexSheet(m).TrailingVortexEdge(i).TE_Node().y();
                   xyz_te[2] = VortexSheet(m).TrailingVortexEdge(i).TE_Node().z();
           
                   xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0];
                   xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];
                   xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2];
                           
                   VortexSheet(k).InducedVelocity(xyz, q, xyz_te);
              
                   U += q[0];
                   V += q[1];
                   W += q[2];
                   
                   // If there is ground effects, z plane ...
         
                   if ( DoGroundEffectsAnalysis() ) {

                      xyz_te[0] = VortexSheet(m).TrailingVortexEdge(i).TE_Node().x();
                      xyz_te[1] = VortexSheet(m).TrailingVortexEdge(i).TE_Node().y();
                      xyz_te[2] = VortexSheet(m).TrailingVortexEdge(i).TE_Node().z();
                  
                      xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0];
                      xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];
                      xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2];
                       
                      xyz[2] *= -1.; xyz_te[2] *= -1.;
                     
                      VortexSheet(k).InducedVelocity(xyz, q, xyz_te);
             
                      q[2] *= -1.;
                     
                      U += q[0];
                      V += q[1];
                      W += q[2];
                     
                   }  
                                      
                   // If there is a symmetry plane, calculate influence of the reflection
         
                   if ( DoSymmetryPlaneSolve_ ) {

                      xyz_te[0] = VortexSheet(m).TrailingVortexEdge(i).TE_Node().x();
                      xyz_te[1] = VortexSheet(m).TrailingVortexEdge(i).TE_Node().y();
                      xyz_te[2] = VortexSheet(m).TrailingVortexEdge(i).TE_Node().z();
                  
                      xyz[0] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[0];
                      xyz[1] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[1];
                      xyz[2] = VortexSheet(m).TrailingVortexEdge(i).xyz_c(j)[2];
                       
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) { xyz[0] *= -1.; xyz_te[0] *= -1.; };
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) { xyz[1] *= -1.; xyz_te[1] *= -1.; };
                      if ( DoSymmetryPlaneSolve_ == SYM_Z ) { xyz[2] *= -1.; xyz_te[2] *= -1.; };
                     
                      VortexSheet(k).InducedVelocity(xyz, q, xyz_te);
             
                      if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                      if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
                     
                      U += q[0];
                      V += q[1];
                      W += q[2];
                      
                      // If there is ground effects, z plane ...
            
                      if ( DoGroundEffectsAnalysis() ) {

                         xyz[2] *= -1.; xyz_te[2] *= -1.;
                        
                         VortexSheet(k).InducedVelocity(xyz, q, xyz_te);
                
                         if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                         if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;                
                                                               q[2] *= -1.;
                        
                         U += q[0];
                         V += q[1];
                         W += q[2];
                        
                      }                        
                     
                   }                   

                }
                
                VortexSheet(m).TrailingVortexEdge(i).U(j) += U;
                VortexSheet(m).TrailingVortexEdge(i).V(j) += V;
                VortexSheet(m).TrailingVortexEdge(i).W(j) += W;
                
             }
           
          }
          
       }

       // Force last segment to free stream conditions
               
       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     
              
          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {

             j = VortexSheet(m).TrailingVortexEdge(i).NumberOfSubVortices() + 1;
      
             VortexSheet(m).TrailingVortexEdge(i).U(j) = FreeStreamVelocity_[0];
             VortexSheet(m).TrailingVortexEdge(i).V(j) = FreeStreamVelocity_[1];
             VortexSheet(m).TrailingVortexEdge(i).W(j) = FreeStreamVelocity_[2];
             
          }
          
       }       
       
       // Now update the location of the wake
                 
       MaxDelta = 0.;
                 
       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     
              
          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {
             
             if ( DoGroundEffectsAnalysis() ) VortexSheet(m).TrailingVortexEdge(i).DoGroundEffectsAnalysis() = 1;

             Delta = VortexSheet(m).TrailingVortexEdge(i).UpdateLocation();
             
             MaxDelta = MAX(MaxDelta,Delta);
             
          }

       }

       if ( Iter > 1 && log10(MaxDelta) <= -2. ) Done = 1;
       
       if ( Verbose_ ) printf("MaxDelta: %f \n",log10(MaxDelta)); 

       Iter++;
       
    }

}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER SaveVortexState                             #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::SaveVortexState(void)
{
   
    int k;
   
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
       VortexSheet(k).SaveVortexState();
   
    }
   
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER ConvectWakeVorticity                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ConvectWakeVorticity(void)
{

    int i, j, k, m, Level, Node1;
    double xyz[3], xyz_te[3], q[5], U, V, W;

    // Update vortex strengths
    
    UpdateVortexEdgeStrengths(1, EXPLICIT_WAKE_GAMMAS);
    
    // Trailing vortex induced velocities
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       U = V = W = 0.;
       
       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
          
          xyz[0] = VortexLoop(i).xyz_c()[0];
          xyz[1] = VortexLoop(i).xyz_c()[1];
          xyz[2] = VortexLoop(i).xyz_c()[2];
                   
          VortexSheet(k).InducedVelocity(xyz, q);
 
          U += q[0];
          V += q[1];
          W += q[2];
   
          // If there is ground effects, z plane ...
             
          if ( DoGroundEffectsAnalysis() ) {
             
             xyz[0] = VortexLoop(i).xyz_c()[0];
             xyz[1] = VortexLoop(i).xyz_c()[1];
             xyz[2] = VortexLoop(i).xyz_c()[2];
                       
             xyz[2] *= -1.;
            
             VortexSheet(k).InducedVelocity(xyz, q);
   
             q[2] *= -1.;
            
             U += q[0];
             V += q[1];
             W += q[2];
            
          }   
             
          // If there is a symmetry plane, calculate influence of the reflection
             
          if ( DoSymmetryPlaneSolve_ ) {
             
             xyz[0] = VortexLoop(i).xyz_c()[0];
             xyz[1] = VortexLoop(i).xyz_c()[1];
             xyz[2] = VortexLoop(i).xyz_c()[2];
                          
             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
            
             VortexSheet(k).InducedVelocity(xyz, q);
   
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
            
             U += q[0];
             V += q[1];
             W += q[2];
             
             // If there is ground effects, z plane ...
                
             if ( DoGroundEffectsAnalysis() ) {
  
                xyz[2] *= -1.;
               
                VortexSheet(k).InducedVelocity(xyz, q);
   
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                      q[2] *= -1.;
               
                U += q[0];
                V += q[1];
                W += q[2];
               
             }                
            
          }   

       }
       
       UnsteadyTrailingWakeVelocity_[i][0] = U;
       UnsteadyTrailingWakeVelocity_[i][1] = V;
       UnsteadyTrailingWakeVelocity_[i][2] = W;
         
    }
    
    // Update vortex strengths
    
    UpdateVortexEdgeStrengths(1, ALL_WAKE_GAMMAS);
        
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateSurfaceMotion                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateSurfaceMotion(void)
{

    int i;
    double U, V, W, Theta, Hdot, RotVec[3];
    QUAT Quat, InvQuat, Vec1, Vec2;

    // Unsteady heaving analysis
    
    if ( TimeAnalysisType_ == HEAVE_ANALYSIS ) {
    
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
        
          Unsteady_H_ =  Unsteady_HMax_ * sin ( Unsteady_AngleRate_*CurrentTime_ );     
          
          Hdot = Unsteady_HMax_ * cos ( Unsteady_AngleRate_*CurrentTime_) * Unsteady_AngleRate_;   
 
          // Calculate body velocities
          
          LocalBodySurfaceVelocity_[i][0] = 0.;
          LocalBodySurfaceVelocity_[i][1] = 0.;
          LocalBodySurfaceVelocity_[i][2] = Hdot;
        
       }
       
    }
    
    // Unsteady Pitch, Roll, and Yaw Analysis 
    
    else if ( TimeAnalysisType_ == P_ANALYSIS ||
              TimeAnalysisType_ == Q_ANALYSIS || 
              TimeAnalysisType_ == R_ANALYSIS ) {
    
       if ( TimeAnalysisType_ == P_ANALYSIS ) {
          
          RotVec[0] = -1.;
          RotVec[1] =  0.;
          RotVec[2] =  0.;

       }
       
       else if ( TimeAnalysisType_ == Q_ANALYSIS ) { 
          
          RotVec[0] =  0.;
          RotVec[1] =  1.;
          RotVec[2] =  0.;

       }          
             
       else if ( TimeAnalysisType_ == R_ANALYSIS ) { 
          
          RotVec[0] =  0.;
          RotVec[1] =  0.;
          RotVec[2] = -1.;
  
       }   

       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
          // Surface location at t - dt
//djk          
          Unsteady_Angle_ = AngleZero_ + Unsteady_AngleMax_ * TORAD * sin ( Unsteady_AngleRate_*(CurrentTime_ - TimeStep_) - 0.5*PI);  

          Quat.FormRotationQuat(RotVec,Unsteady_Angle_);
   
          InvQuat = Quat;
   
          InvQuat.FormInverse();
       
          // Rotate

          Vec1(0) = VortexLoop(i).Xc() - Xcg();
          Vec1(1) = VortexLoop(i).Yc() - Ycg();
          Vec1(2) = VortexLoop(i).Zc() - Zcg();
   
          Vec1 = Quat * Vec1 * InvQuat;

          Vec1(0) += Xcg();
          Vec1(1) += Ycg();
          Vec1(2) += Zcg();
                   
          // Surface location at t
//djk          
          Unsteady_Angle_ = AngleZero_ + Unsteady_AngleMax_ * TORAD * sin ( Unsteady_AngleRate_*CurrentTime_ - 0.5*PI);  
          
          Quat.FormRotationQuat(RotVec,Unsteady_Angle_);
   
          InvQuat = Quat;
   
          InvQuat.FormInverse();
       
          // Rotate

          Vec2(0) = VortexLoop(i).Xc() - Xcg();
          Vec2(1) = VortexLoop(i).Yc() - Ycg();
          Vec2(2) = VortexLoop(i).Zc() - Zcg();
   
          Vec2 = Quat * Vec2 * InvQuat;

          Vec2(0) += Xcg();
          Vec2(1) += Ycg();
          Vec2(2) += Zcg();

          // Calculate body velocities
          
          LocalBodySurfaceVelocity_[i][0] = ( Vec2(0) - Vec1(0) ) / TimeStep_;
          LocalBodySurfaceVelocity_[i][1] = ( Vec2(1) - Vec1(1) ) / TimeStep_;
          LocalBodySurfaceVelocity_[i][2] = ( Vec2(2) - Vec1(2) ) / TimeStep_;
                    
          // Angle at time t
//djk          
          Unsteady_Angle_ = AngleZero_ + Unsteady_AngleMax_ * TORAD * sin ( Unsteady_AngleRate_*(CurrentTime_) - 0.5*PI);  
           
       }
       
    }    

}

/*##############################################################################
#                                                                              #
#                           VSP_SOLVER Do_GMRES_Solve                          #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::Do_GMRES_Solve(void)
{

    int i, Iters;
    double ResMax, ResRed, ResFin, Fact;

#pragma omp parallel for
    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       GammaNM2_[i] = GammaNM1_[i];
     
       GammaNM1_[i] = Gamma_[i];
       
       Delta_[i] = 0.;
       
    }
    
    // Calculate the initial, preconditioned, residual

    CalculateResidual();
    
    DoMatrixPrecondition(Residual_);

    // VLM model convergence criteria
    
    if ( ModelType_ == VLM_MODEL ) {
       
       ResMax = 0.1;
       ResRed = 0.1;
       
    }
    
    // Panel model convergence criteria
     
    else {

       ResMax = 0.1;
       ResRed = 0.1;

    }       
    
    // Use preconditioned GMRES to solve the linear system
     
    GMRES_Solver(NumberOfVortexLoops_+1,  // Number of Equations, 0 <= i < Neq
                 3,                       // Max number of outer iterations
                 500,                     // Max number of inner (restart) iterations
                 1,                       // Output flag, verbose = 0, or 1
                 Delta_,                  // Initial guess and solution vector
                 Residual_,               // Right hand side of Ax = b
                 ResMax,                  // Maximum error tolerance
                 ResRed,                  // Residual reduction factor
                 ResFin,                  // Final log10 of residual reduction   
                 Iters);                  // Final iteration count      

    // Update solution vector

#pragma omp parallel for
    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

       Gamma_[i] = GammaNM1_[i] + Delta_[i];

    }

    if ( Verbose_ ) {
       
       double KelvinSum = 0.;
   
       for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
          KelvinSum += Gamma_[i];
   
       }
       
       printf("\n\nKelvin Sum: %f \n",KelvinSum);
       
    }
    
    if ( Verbose_) printf("log10(ABS(L2Residual_)): %lf \n",L2Residual_);

}

/*##############################################################################
#                                                                              #
#                           VSP_SOLVER CalculateResidual                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateResidual(void)
{

    int i;
    double Dot;

    // VLM Model
   
    if ( ModelType_ == VLM_MODEL ) {
      
       MatrixMultiply(Gamma_, Residual_);

#pragma omp parallel for 
       for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
          Residual_[i] = RightHandSide_[i] - Residual_[i];

       }
      
       // Add in unsteady terms
    
       if ( TimeAccurate_ ) {

          // Time dependent wake terms
          
#pragma omp parallel for        
          for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
             Residual_[i] -= vector_dot(VortexLoop(i).Normal(), UnsteadyTrailingWakeVelocity_[i]);
          
          }                     
       
       }      
      
       Dot = 0.;
    
#pragma omp parallel for reduction(+:Dot)        
       for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

          Dot += Residual_[i]*Residual_[i];
       
       }

       L2Residual_ = log10(sqrt(Dot/NumberOfVortexLoops_));
        
    }
   
    // Panel model
   
    else if ( ModelType_ == PANEL_MODEL ) {
       
       MatrixMultiply(Gamma_, Residual_);
      
#pragma omp parallel for 
       for ( i = 0 ; i <= NumberOfVortexLoops_  ; i++ ) {
     
          MatrixVecTemp_[i] = RightHandSide_[i] - Residual_[i];

       }     
      
       // Add in unsteady terms
    
       if ( TimeAccurate_ ) {

          // Time dependent wake terms
          
#pragma omp parallel for        
          for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
             MatrixVecTemp_[i] -= vector_dot(VortexLoop(i).Normal(), UnsteadyTrailingWakeVelocity_[i]);
          
          }                 
       
       }       
      
       Dot = 0.;
    
#pragma omp parallel for reduction(+:Dot)        
       for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

          Dot += MatrixVecTemp_[i]*MatrixVecTemp_[i];
        
       }

       L2Residual_ = log10(sqrt(Dot/NumberOfVortexLoops_));
             
       MatrixTransposeMultiply(MatrixVecTemp_, Residual_);
      
    }

}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER GMRES_Solver                              #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::GMRES_Solver(int Neq,                   // Number of Equations, 0 <= i < Neq
                              int IterMax,               // Max number of outer iterations
                              int NumRestart,            // Max number of inner (restart) iterations
                              int Verbose,               // Output flag, verbose = 0, or 1
                              double *x,                 // Initial guess and solution vector
                              double *RightHandSide,     // Right hand side of Ax = b
                              double ErrorMax,           // Maximum error tolerance
                              double ErrorReduction,     // Residual reduction factor
                              double &ResFinal,          // Final log10 of residual reduction
                              int    &IterFinal)         // Final iteration count
{

    int i, j, k, Iter, Done, TotalIterations;

    double av, *c, Epsilon, *g, **h, Dot, Mu, *r;
    double rho, rho_zero, rho_tol, *s, **v, *y;
    
    Epsilon = 1.0e-03;
    
    TotalIterations = 0;

    // Allocate memory
    
    c = new double[NumRestart + 1];
    g = new double[NumRestart + 1];
    s = new double[NumRestart + 1];
    y = new double[NumRestart + 1];

    h = new double*[NumRestart + 1];

    for ( i = 0 ; i <= NumRestart ; i++ ) {

       h[i] = new double[NumRestart + 1];

    }

    v = new double*[NumRestart + 1];

    for ( i = 0 ; i <= NumRestart ; i++ ) {

       v[i] = new double[Neq + 1];

    }

    r = new double[Neq + 1];

    // Outer iterative loop
    
    Iter = 0;

    rho = 1.e9;

    rho_zero = rho;

    rho_tol = 0.;

    Done = 0;
    
    while ( Iter < IterMax && ( ( rho > rho_tol || rho > ErrorMax ) && !Done ) ) {

      // Matrix Multiplication

      DoPreconditionedMatrixMultiply(x,r);

#pragma omp parallel for
      for ( i = 0; i < Neq; i++ ) {

        r[i] = RightHandSide[i] - r[i];
   
      }

      rho = sqrt(VectorDot(Neq,r,r));

      if ( Iter == 0 ) rho_zero = rho;

      if ( Iter == 0 ) rho_tol = rho * ErrorReduction;
    
      if ( Verbose && Iter == 0 && !TimeAccurate_ ) printf("Wake Iteration: %5d / %-5d ... GMRES Iteration: %5d ... Reduction: %10.5f / %-10.5f ...  Maximum: %10.5f / %-10.5f \r",CurrentWakeIteration_, WakeIterations_, 0,log10(rho/rho_zero),log10(ErrorReduction), log10(rho), log10(ErrorMax)); fflush(NULL);
      if ( Verbose && Iter == 0 &&  TimeAccurate_ ) printf("TimeStep: %-5d / %-5d ... Time: %10.5f ... GMRES Iteration: %5d ... Reduction: %10.5f / %-10.5f ...  Maximum: %-10.5f / %10.5f \r",Time_,NumberOfTimeSteps_,CurrentTime_, 0,log10(rho/rho_zero),log10(ErrorReduction), log10(rho), log10(ErrorMax)); fflush(NULL);
          
      for ( i = 0; i < Neq; i++ ) {
      
         v[0][i] = r[i] / rho;
      
      }
    
      g[0] = rho;

      for ( i = 1; i < NumRestart + 1; i++ ) {

        g[i] = 0.0;

      }
    
      for ( i = 0; i < NumRestart + 1; i++ ) {

         for ( j = 0; j < NumRestart; j++ ) {

            h[i][j] = 0.0;
        
         }

      }

      k = 0;

      while ( k < NumRestart && ( ( rho > rho_tol || rho > ErrorMax ) && !Done )  ) {

         // Matrix multiply
     
         DoPreconditionedMatrixMultiply(v[k], v[k+1]);

         av = sqrt(VectorDot(Neq,v[k+1],v[k+1]));
          
         for ( j = 0; j < k+1; j++ ) {

            h[j][k] = VectorDot( Neq, v[k+1], v[j] );

#pragma omp parallel for 
            for ( i = 0; i < Neq; i++ ) {
 
               v[k+1][i] = v[k+1][i] - h[j][k] * v[j][i];
 
            }

         }
      
         h[k+1][k] = sqrt ( VectorDot( Neq, v[k+1], v[k+1] ) );
    
         if ( ( av + Epsilon * h[k+1][k] ) == av ) {
           
            for ( j = 0; j < k+1; j++ )  {
 
               Dot = VectorDot( Neq, v[k+1], v[j] );
  
               h[j][k] = h[j][k] + Dot;
 
#pragma omp parallel for 
               for ( i = 0; i < Neq; i++ ) {
  
                  v[k+1][i] = v[k+1][i] - Dot * v[j][i];

               }
 
            }
 
            h[k+1][k] = sqrt ( VectorDot( Neq, v[k+1], v[k+1] ) );

         }
     
         if ( h[k+1][k] != 0.0 ) {

#pragma omp parallel for
            for ( i = 0; i < Neq; i++ )  {
 
               v[k+1][i] = v[k+1][i] / h[k+1][k];
 
            }

         }
     
         if ( 0 < k ) {

            for ( i = 0; i < k + 2; i++ ) {
 
               y[i] = h[i][k];
 
            }
 
            for ( j = 0; j < k; j++ ) {
 
               ApplyGivensRotation( c[j], s[j], j, y );
 
            }
 
            for ( i = 0; i < k + 2; i++ ) {
 
               h[i][k] = y[i];
 
            }

         }
     
         Mu = sqrt ( h[k][k] * h[k][k] + h[k+1][k] * h[k+1][k] );

         c[k] = h[k][k] / Mu;

         s[k] = -h[k+1][k] / Mu;

         h[k][k] = c[k] * h[k][k] - s[k] * h[k+1][k];

         h[k+1][k] = 0.0;

         ApplyGivensRotation( c[k], s[k], k, g );
     
         rho = fabs ( g[k+1] );

         TotalIterations = TotalIterations + 1;
    
         if ( Verbose && !TimeAccurate_) printf("Wake Iteration: %5d / %-5d ... GMRES Iteration: %5d ... Reduction: %10.5f / %-10.5f ...  Maximum: %10.5f / %-10.5f \r",CurrentWakeIteration_,WakeIterations_,TotalIterations,log10(rho/rho_zero),log10(ErrorReduction), log10(rho), log10(ErrorMax)); fflush(NULL);
         if ( Verbose &&  TimeAccurate_) printf("TimeStep: %5d / %-5d ... Time: %10.5f ... GMRES Iteration: %5d ... Reduction: %10.5f / %-10.5f ...  Maximum: %10.5f / %-10.5f \r",Time_,NumberOfTimeSteps_,CurrentTime_,TotalIterations,log10(rho/rho_zero),log10(ErrorReduction), log10(rho), log10(ErrorMax)); fflush(NULL);

         if ( rho <= ErrorMax && rho <= rho_tol ) Done = 1;

         k++;

      }
    
      k--;
    
      y[k] = g[k] / h[k][k];

      for ( i = k - 1; 0 <= i; i-- ) {

         y[i] = g[i];
 
         for ( j = i+1; j < k + 1; j++ ) {
 
            y[i] = y[i] - h[i][j] * y[j];
 
         }
 
         y[i] = y[i] / h[i][i];

      }

#pragma omp parallel for private(j)    
      for ( i = 0; i < Neq; i++ ) {

         for ( j = 0; j < k + 1; j++ ) {
 
            x[i] = x[i] + v[j][i] * y[j];
 
         }

       }

       Iter++;
    
    }

    IterFinal = TotalIterations;

    ResFinal = log10(rho/rho_zero);

    // Free up memory

    delete [] c;
    delete [] g;
    delete [] r;
    delete [] s;
    delete [] y;

    for ( i = 0 ; i <= NumRestart ; i++ ) {

       delete [] h[i];

    }

    delete [] h;

    for ( i = 0 ; i <= NumRestart ; i++ ) {

       delete [] v[i];

    }

    delete [] v;

    return;

}

/*##############################################################################
#                                                                              #
#                              VSP_SOLVER VectorDot                            #
#                                                                              #
##############################################################################*/

double VSP_SOLVER::VectorDot(int Neq, double *r, double *s) 
{

    int i;
    double dot;

    dot = 0.;

#pragma omp parallel for reduction(+:dot)
    for ( i = 0 ; i < Neq ; i++ ) {

       dot += r[i] * s[i];
    
    }

    return dot;

}

/*##############################################################################
#                                                                              #
#                        VSP_SOLVER  ApplyGivensRotation                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ApplyGivensRotation(double c, double s, int k, double *g)
{

  double g1, g2;

  g1 = c * g[k] - s * g[k+1];
  g2 = s * g[k] + c * g[k+1];

  g[k]   = g1;
  g[k+1] = g2;

  return;

}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateForces                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateForces(void)
{
   
    // Calculate velocities

    CalculateVelocities();
   
    if ( TimeAccurate_ ) CalculateUnsteadyForces();
   
    // If subsonic, we do a modified trailing edge induced drag calculation
    
    if ( Mach_ < 1 ) {
       
       CalculateTrefftzForces();
       
    }
    
    // Calculate forces by applying JK theorem to each edge
    
    else {
       
       CalculateKuttaJukowskiForces();
       
    }

    // Calculate Delta-Cps, or surface pressures
    
    if ( ModelType_ == VLM_MODEL ) CalculateDeltaCPs();
      
    if ( ModelType_ == PANEL_MODEL ) CalculateSurfacePressures();

    // Apply vortex burst model

    if ( ModelType_ == VLM_MODEL && CalculateVortexLift_ ) CalculateVortexBurstLocation();

    // Integrate forces and moments
    
    if ( TimeAccurate_ ) IntegrateForcesAndMoments(1);
     
    IntegrateForcesAndMoments(0);
    
    // Calculate 2D clmax limited forces and moments, as well as span wide
    // loading information
    
    if ( ModelType_ == VLM_MODEL ) {
       
       if ( TimeAccurate_ ) CalculateCLmaxLimitedForces(1);
       
       CalculateCLmaxLimitedForces(0);

    }
   
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER ReCalculateForces                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ReCalculateForces(void)
{
   
    // Calculate 2D clmax limited forces and moments, as well as span wide
    // loading information... this routine is likely to be called after 
    // simply changing the value of ReCref_ - so user is really looking for
    // the change in viscous drag for the new value of ReCref_
    
    if ( ModelType_ == VLM_MODEL ) CalculateCLmaxLimitedForces(0);   
   
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateTrefftzForces                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateTrefftzForcesOld(void)
{

    int i, j, k, p, Loop, Loop1, Loop2, Surf, Level, Hits;
    double xyz[3], q[3], qtot[3], Factor, AttachedForce[3];
    double Normal[3], Dot, VortexLift, LeadingEdgeSuction, LeadingEdgeThrust, Sign, Sweep;
    double mag1, mag2, dot, angle, Theta, ToC, RoC, EtaToC, Kt, Area, DeltaCp, Fact;
    double xyzLE[3], xyzTE[3], S[3], WingCref;
    double *u, *v, *w, *Denom, Wgt;
    
    // Calculate velocities at trailing edge of each vortex sheet
    
    Level = 1;
    
    u = new double[VSPGeom().Grid(Level).NumberOfNodes() + 1];
    v = new double[VSPGeom().Grid(Level).NumberOfNodes() + 1];
    w = new double[VSPGeom().Grid(Level).NumberOfNodes() + 1];
    
    Denom = new double[VSPGeom().Grid(Level).NumberOfNodes() + 1];
    
    zero_double_array(u, VSPGeom().Grid(Level).NumberOfNodes());
    zero_double_array(v, VSPGeom().Grid(Level).NumberOfNodes());
    zero_double_array(w, VSPGeom().Grid(Level).NumberOfNodes());
    
    zero_double_array(Denom, VSPGeom().Grid(Level).NumberOfNodes());

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       mag1 = sqrt(vector_dot(FreeStreamVelocity_,FreeStreamVelocity_));
       
       mag2 = sqrt(vector_dot(VortexLoop(i).Normal(),VortexLoop(i).Normal()));
       
       dot = vector_dot(FreeStreamVelocity_, VortexLoop(i).Normal());
       
       dot /= (mag1*mag2);
       
       dot = MIN(MAX(-1.,dot),1.);
       
       angle = ABS(acos(dot) - 0.5*PI)/TORAD;
       
       Factor = 1.;
   
       // Limit maximum turning angle
       
       if ( MaxTurningAngle_ > 0. && angle > MaxTurningAngle_ ) Factor = MaxTurningAngle_/angle;
   
       qtot[0] = LocalFreeStreamVelocity_[i][0] * Factor;
       qtot[1] = LocalFreeStreamVelocity_[i][1] * Factor;
       qtot[2] = LocalFreeStreamVelocity_[i][2] * Factor;
       
       Wgt = VortexLoop(i).Area() / VortexLoop(i).NumberOfNodes();
   
       for ( i = 1 ; i <= VortexLoop(j).NumberOfNodes() ; i++ ) {
          
          u[VortexLoop(i).Node(j)] += qtot[0] * Wgt;
          v[VortexLoop(i).Node(j)] += qtot[1] * Wgt;
          v[VortexLoop(i).Node(j)] += qtot[2] * Wgt;
          
          Denom[VortexLoop(i).Node(j)] += Wgt;
          
       }
       
    }

    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfNodes() ; i++ ) {

       u[i] /= Denom[i];
       v[i] /= Denom[i];
       v[i] /= Denom[i];
          
    }

    for ( p = 1 ; p <= NumberOfVortexSheets_ ; p++ ) {

       for ( i = 1 ; i <= VortexSheet(p).NumberOfTrailingVortices() ; i++ ) {
          
          j = VortexSheet(p).TrailingVortexEdge(i).Node();
          
          VortexSheet(p).TrailingVortexEdge(i).LocalVelocityAtTrailingEdge(0) = u[j];
          VortexSheet(p).TrailingVortexEdge(i).LocalVelocityAtTrailingEdge(1) = v[j];
          VortexSheet(p).TrailingVortexEdge(i).LocalVelocityAtTrailingEdge(2) = w[j];
          
       }
       
    }
    
    delete [] u;
    delete [] v;
    delete [] w;
    delete [] Denom;
                          
    // Loop over vortex edges and calculate forces via K-J theorem, using only wake induced velocities applied at TE

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       // Calculate an averaged normal

       Normal[0] = Normal[1] = Normal[2] = 0.;

       Hits = 0;
       
       for ( k = 1 ; k <= 2 ; k++ ) {
          
          if ( k == 1 ) Loop = SurfaceVortexEdge(j).VortexLoop1();
          if ( k == 2 ) Loop = SurfaceVortexEdge(j).VortexLoop2();
       
          if ( Loop != 0 ) {
           
             // Normal
             
             Normal[0] += VortexLoop(Loop).FlatPlateNormal()[0];
             Normal[1] += VortexLoop(Loop).FlatPlateNormal()[1];
             Normal[2] += VortexLoop(Loop).FlatPlateNormal()[2];

             Hits++;
          
          }
          
       }

       Normal[0] /= Hits;
       Normal[1] /= Hits;
       Normal[2] /= Hits;

       // Trailing vortices induced velocities... shift the current bound vortex to the 
       // 'trailing edge' of the trailing vortex.

       for ( p = 1 ; p <= NumberOfVortexSheets_ ; p++ ) {

          // Evaluation location

          xyz[0] = SurfaceVortexEdge(j).Xc();
          xyz[1] = SurfaceVortexEdge(j).Yc();
          xyz[2] = SurfaceVortexEdge(j).Zc();

          VortexSheet(p).InducedKuttaVelocity(xyz, q);

          qtot[0] += q[0];
          qtot[1] += q[1];
          qtot[2] += q[2];

          // If there is ground effects, z plane ...
          
          if ( DoGroundEffectsAnalysis() ) {

             xyz[0] = SurfaceVortexEdge(j).Xc();
             xyz[1] = SurfaceVortexEdge(j).Yc();
             xyz[2] = SurfaceVortexEdge(j).Zc();

             xyz[2] *= -1.;
            
             VortexSheet(p).InducedKuttaVelocity(xyz, q);
      
             q[2] *= -1.;
  
             qtot[0] += q[0];
             qtot[1] += q[1];
             qtot[2] += q[2];
            
          }
          
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {

             xyz[0] = SurfaceVortexEdge(j).Xc();
             xyz[1] = SurfaceVortexEdge(j).Yc();
             xyz[2] = SurfaceVortexEdge(j).Zc();
          
             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
            
             VortexSheet(p).InducedKuttaVelocity(xyz, q);
      
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
  
             qtot[0] += q[0];
             qtot[1] += q[1];
             qtot[2] += q[2];
            
             // If there is ground effects, z plane ...
             
             if ( DoGroundEffectsAnalysis() ) {

                xyz[2] *= -1.;
               
                VortexSheet(p).InducedKuttaVelocity(xyz, q);
         
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;         
                                                      q[2] *= -1.;
     
                qtot[0] += q[0];
                qtot[1] += q[1];
                qtot[2] += q[2];
               
             }
              
          }
        
       }

       // Calculate fully attached induced drag
       
       SurfaceVortexEdge(j).CalculateTrefftzForces(qtot);
       
       // If this is a wing leading edge... determine the amount of
       // leading edge suction and vortex lift we may have
       
       if ( ( CalculateVortexLift_ || CalculateLeadingEdgeSuction_ ) && SurfaceVortexEdge(j).IsLeadingEdge() ) {

          // Surface ID
          
          Surf = SurfaceVortexEdge(j).DegenWing();
          
          // Root LE, TE locations

          xyzLE[0] = VSPGeom().VSP_Surface(Surf).Root_LE(0);
          xyzLE[1] = VSPGeom().VSP_Surface(Surf).Root_LE(1);
          xyzLE[2] = VSPGeom().VSP_Surface(Surf).Root_LE(2);
          
          xyzTE[0] = VSPGeom().VSP_Surface(Surf).Root_TE(0);
          xyzTE[1] = VSPGeom().VSP_Surface(Surf).Root_TE(1);
          xyzTE[2] = VSPGeom().VSP_Surface(Surf).Root_TE(2);          
          
          // Root chord vector and length
          
          S[0] = xyzTE[0] - xyzLE[0];
          S[1] = xyzTE[1] - xyzLE[1];
          S[2] = xyzTE[2] - xyzLE[2];
          
          WingCref = sqrt(vector_dot(S,S));
          
          S[0] /= WingCref;
          S[1] /= WingCref;
          S[2] /= WingCref;
         
          // Save fully attached force values
          
          AttachedForce[0] = SurfaceVortexEdge(j).Trefftz_Fx();
          AttachedForce[1] = SurfaceVortexEdge(j).Trefftz_Fy();
          AttachedForce[2] = SurfaceVortexEdge(j).Trefftz_Fz();    
          
          Sign = 1.;
          
          if ( vector_dot(AttachedForce, Normal) < 0. ) Sign = -1.;
          
          // Local angle of attack
        
          Dot = vector_dot(Normal, FreeStreamVelocity_);
          
          Dot = MAX(-1.,MIN(1.,Dot));
          
          Theta = 0.5*PI - acos(Dot);

          // Calculate the local leading edge sweep

          Dot = vector_dot(S,SurfaceVortexEdge(j).Vec());
          
          Dot = MIN(1.,MAX(-1.,Dot));
          
          Sweep = acos(Dot);
          
          Sweep = 0.5*PI - Sweep;
             
          // Calculate fraction of attached flow
          
          Kt = 1.;
          
          if ( CalculateLeadingEdgeSuction_ ) {
             
             ToC    = SurfaceVortexEdge(j).ThicknessToChord();
             EtaToC = SurfaceVortexEdge(j).LocationOfMaxThickness();
             RoC    = SurfaceVortexEdge(j).RadiusToChord();

             Kt = CalculateLeadingEdgeSuctionFraction(Mach_, ToC, RoC, EtaToC, ABS(Theta), Sweep);
             
          }

          // Calculate component of downwash that is normal to local surface

          Dot = vector_dot(qtot,Normal);

          qtot[0] = Dot * Normal[0];
          qtot[1] = Dot * Normal[1]; 
          qtot[2] = Dot * Normal[2]; 

          // Calculate leading edge thrust component

          SurfaceVortexEdge(j).CalculateTrefftzForces(qtot);

          LeadingEdgeThrust = sqrt( pow(SurfaceVortexEdge(j).Trefftz_Fx(),2.)
                                  + pow(SurfaceVortexEdge(j).Trefftz_Fy(),2.)
                                  + pow(SurfaceVortexEdge(j).Trefftz_Fz(),2.) );
                                   
                                                                  
          // Loop for the leading edge... one is zero, one is real... door 1 or 2?
          
          Loop = SurfaceVortexEdge(j).VortexLoop1() + SurfaceVortexEdge(j).VortexLoop2();
                   
          if ( VortexLoop(Loop).NumberOfEdges() == 3 ) LeadingEdgeThrust *= 1.25;

          LeadingEdgeSuction = LeadingEdgeThrust/cos(Sweep);
                
          // Calculate onset of vortex lift

          VortexLift = 0.;
        
          if ( Sweep >= 45.*TORAD ) {

             VortexLift = Sign * LeadingEdgeSuction;
             
             Kt = 0.;
             
             VortexLoop(SurfaceVortexEdge(j).VortexLoop1()).CompressibilityFactor() = 1.;
             VortexLoop(SurfaceVortexEdge(j).VortexLoop2()).CompressibilityFactor() = 1.;

          }

          // Sum up forces = attached + vortex - lost leading edge thrust
   
          SurfaceVortexEdge(j).Trefftz_Fx() = AttachedForce[0] + VortexLift*Normal[0] + (1.-Kt)*LeadingEdgeSuction*cos(AngleOfAttack_);
          SurfaceVortexEdge(j).Trefftz_Fy() = AttachedForce[1] + VortexLift*Normal[1];
          SurfaceVortexEdge(j).Trefftz_Fz() = AttachedForce[2] + VortexLift*Normal[2] + (1.-Kt)*LeadingEdgeSuction*sin(AngleOfAttack_);                 

          // Limit forces to something sane...

          Area = VortexLoop(Loop).Area();

          DeltaCp = ( SurfaceVortexEdge(j).Trefftz_Fx() * Normal[0]
                    + SurfaceVortexEdge(j).Trefftz_Fy() * Normal[1]
                    + SurfaceVortexEdge(j).Trefftz_Fz() * Normal[2] ) / Area;
                    
          if ( ABS(DeltaCp) > 2.*DCpMax_ ) {
             
             Fact = sqrt(DCpMax_/ABS(DeltaCp));
             
             VortexLift *= Fact;

             SurfaceVortexEdge(j).Trefftz_Fx() = AttachedForce[0] + VortexLift*Normal[0] + (1.-Kt)*LeadingEdgeSuction*cos(AngleOfAttack_);
             SurfaceVortexEdge(j).Trefftz_Fy() = AttachedForce[1] + VortexLift*Normal[1];
             SurfaceVortexEdge(j).Trefftz_Fz() = AttachedForce[2] + VortexLift*Normal[2] + (1.-Kt)*LeadingEdgeSuction*sin(AngleOfAttack_);                 

          }   

       }   
           
    }
   
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateTrefftzForces                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateTrefftzForces(void)
{

    int j, k, p, Loop, Loop1, Loop2, Hits, Surf;
    double xyz[3], q[3], qtot[3], LocalVelocity[3], Factor, AttachedForce[3];
    double Normal[3], Dot, VortexLift, LeadingEdgeSuction, LeadingEdgeThrust, Sign, Sweep;
    double mag1, mag2, dot, angle, Theta, ToC, RoC, EtaToC, Kt, Area, DeltaCp, Fact;
    double xyzLE[3], xyzTE[3], S[3], WingCref;
 
    // Loop over vortex edges and calculate forces via K-J theorem, using only wake induced velocities applied at TE

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       // Calculate an averaged local velocity using the left/right loops

       LocalVelocity[0] = LocalVelocity[1] = LocalVelocity[2] = 0.;
       
       Normal[0] = Normal[1] = Normal[2] = 0.;

       Hits = 0;
       
       for ( k = 1 ; k <= 2 ; k++ ) {
          
          if ( k == 1 ) Loop = SurfaceVortexEdge(j).VortexLoop1();
          if ( k == 2 ) Loop = SurfaceVortexEdge(j).VortexLoop2();
       
          if ( Loop != 0 ) {
           
             mag1 = sqrt(vector_dot(FreeStreamVelocity_,FreeStreamVelocity_));
             
             mag2 = sqrt(vector_dot(VortexLoop(Loop).Normal(),VortexLoop(Loop).Normal()));
             
             dot = vector_dot(FreeStreamVelocity_, VortexLoop(Loop).Normal());
             
             dot /= (mag1*mag2);
             
             dot = MIN(MAX(-1.,dot),1.);
             
             angle = ABS(acos(dot) - 0.5*PI)/TORAD;
             
             Factor = 1.;
        
             // Limit maximum turning angle
             
             if ( MaxTurningAngle_ > 0. && angle > MaxTurningAngle_ ) Factor = MaxTurningAngle_/angle;
 
             LocalVelocity[0] += LocalFreeStreamVelocity_[Loop][0] * Factor;
             LocalVelocity[1] += LocalFreeStreamVelocity_[Loop][1] * Factor;
             LocalVelocity[2] += LocalFreeStreamVelocity_[Loop][2] * Factor;
                               
             // Normal
             
             Normal[0] += VortexLoop(Loop).FlatPlateNormal()[0];
             Normal[1] += VortexLoop(Loop).FlatPlateNormal()[1];
             Normal[2] += VortexLoop(Loop).FlatPlateNormal()[2];

             Hits++;
          
          }
          
       }

       LocalVelocity[0] /= Hits;
       LocalVelocity[1] /= Hits;
       LocalVelocity[2] /= Hits;
       
       Normal[0] /= Hits;
       Normal[1] /= Hits;
       Normal[2] /= Hits;

       // Trailing vortices induced velocities... shift the current bound vortex to the 
       // 'trailing edge' of the trailing vortex.

       qtot[0] = qtot[1] = qtot[2] = 0.;

       for ( p = 1 ; p <= NumberOfVortexSheets_ ; p++ ) {

          // Evaluation location

          xyz[0] = SurfaceVortexEdge(j).Xc();
          xyz[1] = SurfaceVortexEdge(j).Yc();
          xyz[2] = SurfaceVortexEdge(j).Zc();

          VortexSheet(p).InducedKuttaVelocity(xyz, q);

          qtot[0] += q[0];
          qtot[1] += q[1];
          qtot[2] += q[2];

          // If there is ground effects, z plane ...
          
          if ( DoGroundEffectsAnalysis() ) {

             xyz[0] = SurfaceVortexEdge(j).Xc();
             xyz[1] = SurfaceVortexEdge(j).Yc();
             xyz[2] = SurfaceVortexEdge(j).Zc();

             xyz[2] *= -1.;
            
             VortexSheet(p).InducedKuttaVelocity(xyz, q);
      
             q[2] *= -1.;
  
             qtot[0] += q[0];
             qtot[1] += q[1];
             qtot[2] += q[2];
            
          }
          
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {

             xyz[0] = SurfaceVortexEdge(j).Xc();
             xyz[1] = SurfaceVortexEdge(j).Yc();
             xyz[2] = SurfaceVortexEdge(j).Zc();
          
             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
            
             VortexSheet(p).InducedKuttaVelocity(xyz, q);
      
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
  
             qtot[0] += q[0];
             qtot[1] += q[1];
             qtot[2] += q[2];
            
             // If there is ground effects, z plane ...
             
             if ( DoGroundEffectsAnalysis() ) {

                xyz[2] *= -1.;
               
                VortexSheet(p).InducedKuttaVelocity(xyz, q);
         
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;         
                                                      q[2] *= -1.;
     
                qtot[0] += q[0];
                qtot[1] += q[1];
                qtot[2] += q[2];
               
             }
              
          }
        
       }

       // Calculate fully attached induced drag
       
       SurfaceVortexEdge(j).CalculateNormalTrefftzForces(qtot);
       
       qtot[0] += LocalVelocity[0];
       qtot[1] += LocalVelocity[1];
       qtot[2] += LocalVelocity[2];
       
       SurfaceVortexEdge(j).CalculateTrefftzForces(qtot);
       
       // If this is a wing leading edge... determine the amount of
       // leading edge suction and vortex lift we may have
       
       if ( ( CalculateVortexLift_ || CalculateLeadingEdgeSuction_ ) && SurfaceVortexEdge(j).IsLeadingEdge() ) {

          // Surface ID
          
          Surf = SurfaceVortexEdge(j).DegenWing();
          
          // Root LE, TE locations

          xyzLE[0] = VSPGeom().VSP_Surface(Surf).Root_LE(0);
          xyzLE[1] = VSPGeom().VSP_Surface(Surf).Root_LE(1);
          xyzLE[2] = VSPGeom().VSP_Surface(Surf).Root_LE(2);
          
          xyzTE[0] = VSPGeom().VSP_Surface(Surf).Root_TE(0);
          xyzTE[1] = VSPGeom().VSP_Surface(Surf).Root_TE(1);
          xyzTE[2] = VSPGeom().VSP_Surface(Surf).Root_TE(2);          
          
          // Root chord vector and length
          
          S[0] = xyzTE[0] - xyzLE[0];
          S[1] = xyzTE[1] - xyzLE[1];
          S[2] = xyzTE[2] - xyzLE[2];
          
          WingCref = sqrt(vector_dot(S,S));
          
          S[0] /= WingCref;
          S[1] /= WingCref;
          S[2] /= WingCref;
         
          // Save fully attached force values
          
          AttachedForce[0] = SurfaceVortexEdge(j).Trefftz_Fx();
          AttachedForce[1] = SurfaceVortexEdge(j).Trefftz_Fy();
          AttachedForce[2] = SurfaceVortexEdge(j).Trefftz_Fz();    
          
          Sign = 1.;
          
          if ( vector_dot(AttachedForce, Normal) < 0. ) Sign = -1.;
          
          // Local angle of attack
        
          Dot = vector_dot(Normal, FreeStreamVelocity_);
          
          Dot = MAX(-1.,MIN(1.,Dot));
          
          Theta = 0.5*PI - acos(Dot);

          // Calculate the local leading edge sweep

          Dot = vector_dot(S,SurfaceVortexEdge(j).Vec());
          
          Dot = MIN(1.,MAX(-1.,Dot));
          
          Sweep = acos(Dot);
          
          Sweep = 0.5*PI - Sweep;
             
          // Calculate fraction of attached flow
          
          Kt = 1.;
          
          if ( CalculateLeadingEdgeSuction_ ) {
             
             ToC    = SurfaceVortexEdge(j).ThicknessToChord();
             EtaToC = SurfaceVortexEdge(j).LocationOfMaxThickness();
             RoC    = SurfaceVortexEdge(j).RadiusToChord();

             Kt = CalculateLeadingEdgeSuctionFraction(Mach_, ToC, RoC, EtaToC, ABS(Theta), Sweep);
             
          }

          // Calculate component of downwash that is normal to local surface

          Dot = vector_dot(qtot,Normal);

          qtot[0] = Dot * Normal[0];
          qtot[1] = Dot * Normal[1]; 
          qtot[2] = Dot * Normal[2]; 

          // Calculate leading edge thrust component

          SurfaceVortexEdge(j).CalculateTrefftzForces(qtot);

          LeadingEdgeThrust = sqrt( pow(SurfaceVortexEdge(j).Trefftz_Fx(),2.)
                                  + pow(SurfaceVortexEdge(j).Trefftz_Fy(),2.)
                                  + pow(SurfaceVortexEdge(j).Trefftz_Fz(),2.) );
                                   
                                                                  
          // Loop for the leading edge... one is zero, one is real... door 1 or 2?
          
          Loop = SurfaceVortexEdge(j).VortexLoop1() + SurfaceVortexEdge(j).VortexLoop2();
                   
          if ( VortexLoop(Loop).NumberOfEdges() == 3 ) LeadingEdgeThrust *= 1.25;

          LeadingEdgeSuction = LeadingEdgeThrust/cos(Sweep);
                
          // Calculate onset of vortex lift

          VortexLift = 0.;
        
          if ( Sweep >= 45.*TORAD ) {

             VortexLift = Sign * LeadingEdgeSuction;
             
             Kt = 0.;
             
             VortexLoop(SurfaceVortexEdge(j).VortexLoop1()).CompressibilityFactor() = 1.;
             VortexLoop(SurfaceVortexEdge(j).VortexLoop2()).CompressibilityFactor() = 1.;

          }

          // Sum up forces = attached + vortex - lost leading edge thrust
   
          SurfaceVortexEdge(j).Trefftz_Fx() = AttachedForce[0] + VortexLift*Normal[0] + (1.-Kt)*LeadingEdgeSuction*cos(AngleOfAttack_);
          SurfaceVortexEdge(j).Trefftz_Fy() = AttachedForce[1] + VortexLift*Normal[1];
          SurfaceVortexEdge(j).Trefftz_Fz() = AttachedForce[2] + VortexLift*Normal[2] + (1.-Kt)*LeadingEdgeSuction*sin(AngleOfAttack_);                 

          // Limit forces to something sane...

          Area = VortexLoop(Loop).Area();

          DeltaCp = ( SurfaceVortexEdge(j).Trefftz_Fx() * Normal[0]
                    + SurfaceVortexEdge(j).Trefftz_Fy() * Normal[1]
                    + SurfaceVortexEdge(j).Trefftz_Fz() * Normal[2] ) / Area;
                    
          if ( ABS(DeltaCp) > 2.*DCpMax_ ) {
             
             Fact = sqrt(DCpMax_/ABS(DeltaCp));
             
             VortexLift *= Fact;

             SurfaceVortexEdge(j).Trefftz_Fx() = AttachedForce[0] + VortexLift*Normal[0] + (1.-Kt)*LeadingEdgeSuction*cos(AngleOfAttack_);
             SurfaceVortexEdge(j).Trefftz_Fy() = AttachedForce[1] + VortexLift*Normal[1];
             SurfaceVortexEdge(j).Trefftz_Fz() = AttachedForce[2] + VortexLift*Normal[2] + (1.-Kt)*LeadingEdgeSuction*sin(AngleOfAttack_);                 

          }   

       }   
           
    }
   
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER CalculateKuttaJukowskiForces                  #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateKuttaJukowskiForces(void)
{

    int j, Loop, Loop1, Loop2;
    double Fx, Fy, Fz, Hits;
    double AttachedForce[3];
    double Normal[3], Dot, VortexLift, LeadingEdgeThrust, Sign, Sweep, Theta;
    VSP_LOOP VLoop;

    // Loop over vortex edges and calculate forces via K-J theorem

#pragma omp parallel for private(Loop, Loop1, Loop2, Fx, Fy, Fz, Hits, Normal, Sweep, Dot, VortexLift, LeadingEdgeThrust, Sign, Theta)
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
     
       Fx = SurfaceVortexEdge(j).Fx() = Normal[0] = 0.;
       Fy = SurfaceVortexEdge(j).Fy() = Normal[1] = 0.;
       Fz = SurfaceVortexEdge(j).Fz() = Normal[2] = 0.;
          
       if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {
        
          Hits = 0.;
          
          Loop1 = SurfaceVortexEdge(j).VortexLoop1();
          
          if ( Loop1 != 0 ) {

             SurfaceVortexEdge(j).CalculateForces(VortexLoop(Loop1));
             
             Fx += SurfaceVortexEdge(j).Fx();
             Fy += SurfaceVortexEdge(j).Fy();
             Fz += SurfaceVortexEdge(j).Fz();

             Normal[0] += VortexLoop(Loop1).FlatPlateNormal()[0];
             Normal[1] += VortexLoop(Loop1).FlatPlateNormal()[1];
             Normal[2] += VortexLoop(Loop1).FlatPlateNormal()[2];             
             
             Hits += 1.;
            
          }

          Loop2 = SurfaceVortexEdge(j).VortexLoop2();

          if ( Loop2 != 0 ) {
           
             SurfaceVortexEdge(j).CalculateForces(VortexLoop(Loop2));
             
             Fx += SurfaceVortexEdge(j).Fx();
             Fy += SurfaceVortexEdge(j).Fy();
             Fz += SurfaceVortexEdge(j).Fz();

             Normal[0] += VortexLoop(Loop2).FlatPlateNormal()[0];
             Normal[1] += VortexLoop(Loop2).FlatPlateNormal()[1];
             Normal[2] += VortexLoop(Loop2).FlatPlateNormal()[2];                  
             
             Hits += 1.;
             
          }       

          // Local Normal
          
          Normal[0] /= Hits;
          Normal[1] /= Hits;
          Normal[2] /= Hits;          
                   
          // Edge forces
          
          SurfaceVortexEdge(j).Fx() = Fx / Hits;
          SurfaceVortexEdge(j).Fy() = Fy / Hits;
          SurfaceVortexEdge(j).Fz() = Fz / Hits;
          
       }

       // If this is a wing leading edge... determine the amount of
       // leading edge suction and vortex lift we may have
       
       if ( CalculateVortexLift_ && SurfaceVortexEdge(j).IsLeadingEdge() ) {

          // Save fully attached force values
          
          AttachedForce[0] = SurfaceVortexEdge(j).Fx();
          AttachedForce[1] = SurfaceVortexEdge(j).Fy();
          AttachedForce[2] = SurfaceVortexEdge(j).Fz();    
          
          Sign = 1.;
          
          if ( vector_dot(AttachedForce, Normal) < 0. ) Sign = -1.;
          
          // Local angle of attack
        
          Dot = vector_dot(Normal, FreeStreamVelocity_);
          
          Dot = MAX(-1.,MIN(1.,Dot));
          
          Theta = 0.5*PI - acos(Dot);

          // Determine the effective sweep angle of the leading edge
          
          Sweep = vector_dot(FreeStreamVelocity_,SurfaceVortexEdge(j).Vec());
          
          Sweep = MAX(-1.,MIN(1.,Sweep));
          
          Sweep = acos(Sweep);
          
          if ( Sweep > PI ) Sweep -= PI;
          
          Sweep = 0.5*PI - Sweep;
          
          Sweep /= pow(cos(Theta),2.);     
        
          LeadingEdgeThrust = sqrt( pow(SurfaceVortexEdge(j).Fx(),2.)
                                  + pow(SurfaceVortexEdge(j).Fy(),2.)
                                  + pow(SurfaceVortexEdge(j).Fz(),2.) )*cos(Sweep);
                                         
          // Double up if the leading edge loop is a tri... I really want the loading on the
          // entire leading edge region.
          
          Loop = SurfaceVortexEdge(j).VortexLoop1() + SurfaceVortexEdge(j).VortexLoop2();
         
          if ( VortexLoop(Loop).NumberOfEdges() == 3 ) LeadingEdgeThrust *= 1.5;

          // Calculate onset of vortex lift

          VortexLift = 0.;
        
          if ( Sweep >= 45.*TORAD ) {

             VortexLift = Sign * LeadingEdgeThrust/cos(Sweep);
  
             VortexLoop(SurfaceVortexEdge(j).VortexLoop1()).CompressibilityFactor() = 1.;
             VortexLoop(SurfaceVortexEdge(j).VortexLoop2()).CompressibilityFactor() = 1.;

          }

          // Sum up forces = attached + vortex - lost leading edge thrust
   
          SurfaceVortexEdge(j).Fx() = AttachedForce[0] + VortexLift*Normal[0] + LeadingEdgeThrust*cos(AngleOfAttack_);
          SurfaceVortexEdge(j).Fy() = AttachedForce[1] + VortexLift*Normal[1];
          SurfaceVortexEdge(j).Fz() = AttachedForce[2] + VortexLift*Normal[2] + LeadingEdgeThrust*sin(AngleOfAttack_);                 

       }   
               
    }
                 
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER CalculateUnsteadyForces                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateUnsteadyForces(void)
{

    int i, j, Edge;
    double DeltaCp;

#pragma omp parallel for       
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
     
       SurfaceVortexEdge(j).Unsteady_Fx() = 0.;
       SurfaceVortexEdge(j).Unsteady_Fy() = 0.;
       SurfaceVortexEdge(j).Unsteady_Fz() = 0.;
       
    }
    
#pragma omp parallel for private(DeltaCp,Edge,j)             
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       DeltaCp = ( Gamma_[i] - GammaNM1_[i] ) / TimeStep_;

       for ( j = 1 ; j <= VortexLoop(i).NumberOfEdges() ; j++ ) {
          
          Edge = VortexLoop(i).Edge(j);
          
          SurfaceVortexEdge(Edge).Unsteady_Fx() -= DeltaCp * VortexLoop(i).Area() * VortexLoop(i).Nx()/VortexLoop(i).NumberOfEdges();
          SurfaceVortexEdge(Edge).Unsteady_Fy() -= DeltaCp * VortexLoop(i).Area() * VortexLoop(i).Ny()/VortexLoop(i).NumberOfEdges();
          SurfaceVortexEdge(Edge).Unsteady_Fz() -= DeltaCp * VortexLoop(i).Area() * VortexLoop(i).Nz()/VortexLoop(i).NumberOfEdges();
          
       }
       
    }
                 
}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER CalculateDeltaCPs                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateDeltaCPs(void)
{

    int i, j, Loop1, Loop2;
    double Fx, Fy, Fz, Wgt1, Wgt2;
    double Fact, CpI, Cp, Mc, Fc;

    // Loop over vortex edges and calculate forces via K-J theorem
 
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
       VortexLoop(i).Fx() = 0.;
       VortexLoop(i).Fy() = 0.;
       VortexLoop(i).Fz() = 0.;

    }
     
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       Loop1 = SurfaceVortexEdge(j).VortexLoop1();
       Loop2 = SurfaceVortexEdge(j).VortexLoop2();

       // Edge forces

       if ( Mach_ < 1. ) {
          
          Fx = SurfaceVortexEdge(j).Trefftz_Fx() + SurfaceVortexEdge(j).Unsteady_Fx();
          Fy = SurfaceVortexEdge(j).Trefftz_Fy() + SurfaceVortexEdge(j).Unsteady_Fy();
          Fz = SurfaceVortexEdge(j).Trefftz_Fz() + SurfaceVortexEdge(j).Unsteady_Fz();
          
       }
       
       else {
          
          Fx = SurfaceVortexEdge(j).Fx() + SurfaceVortexEdge(j).Unsteady_Fx();
          Fy = SurfaceVortexEdge(j).Fy() + SurfaceVortexEdge(j).Unsteady_Fy();
          Fz = SurfaceVortexEdge(j).Fz() + SurfaceVortexEdge(j).Unsteady_Fz();
          
       }

       // Loop level forces
       
       if ( ( Loop1 != 0 && Loop2 != 0 ) || SurfaceVortexEdge(j).IsLeadingEdge() ) {

          Wgt1 = VortexLoop(Loop1).Area()/(VortexLoop(Loop1).Area() + VortexLoop(Loop2).Area());    

          Wgt2 = 1 - Wgt1;
                  
          VortexLoop(Loop1).Fx() += Wgt1*Fx;
          VortexLoop(Loop1).Fy() += Wgt1*Fy;
          VortexLoop(Loop1).Fz() += Wgt1*Fz;

          VortexLoop(Loop2).Fx() += Wgt2*Fx;
          VortexLoop(Loop2).Fy() += Wgt2*Fy;
          VortexLoop(Loop2).Fz() += Wgt2*Fz;
       
       }

    }
    
    // Calculate normal force on each vortex loop
    
    Mc = 0.75; // Max Mach number at which Karman Tsien correction is applied
               // - Above this we linearly drop the correction to zero by Mach 1.
       
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
       VortexLoop(i).NormalForce() = -VortexLoop(i).Fx() * VortexLoop(i).Nx()
                                   + -VortexLoop(i).Fy() * VortexLoop(i).Ny()
                                   + -VortexLoop(i).Fz() * VortexLoop(i).Nz();
                                   
       VortexLoop(i).dCp() = VortexLoop(i).NormalForce() / VortexLoop(i).Area();      
       
       // 2nd order correction
       
       if ( !NoKarmanTsienCorrection_ && Mach_ < 1. ) {
       
          Cp = VortexLoop(i).dCp();

          CpI = ABS(Cp)/pow(LocalFreeStreamVelocity_[i][4],2.);
          
          // Adjust 2nd order correction model, phase out after Mc ~ 0.75 based on
          // NACA airfoil data
          
          if ( Mach_ < Mc ) {

             Fact = 1. + 0.25*CpI*Mach_*Mach_; 
             
          }
          
          else {
             
             Fc = 0.25*CpI*Mc*Mc;
             
             Fact = 1. + Fc*(1. - (Mach_*Mach_ - Mc*Mc)/(1. - Mc*Mc));
             
          }
   
          VortexLoop(i).CompressibilityFactor() = Fact;
   
          VortexLoop(i).dCp() = Cp * Fact;
    
       }
       
       else {
          
          VortexLoop(i).CompressibilityFactor() = 1.;
          
       }

    }  
   
}

/*##############################################################################
#                                                                              #
#                  VSP_SOLVER CalculateSurfacePressures                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateSurfacePressures(void)
{

    int i, j, Loop1, Loop2, *LoopHits, *EdgeHits, Edge;
    double Vec1[3], Vec2[3], Vec[3], Dot, Mag1, Mag2, Gradient;
    double *U, *V, *W, *UDenom, *VDenom, *WDenom;
    double Cp, Fact, Ratio, CpC, q2, CpCrit;
    
    LoopHits = new int[VSPGeom().Grid(1).NumberOfLoops() + 1];
    
    zero_int_array(LoopHits, VSPGeom().Grid(1).NumberOfLoops());
    
    EdgeHits = new int[VSPGeom().Grid(1).NumberOfEdges() + 1];
    
    zero_int_array(EdgeHits, VSPGeom().Grid(1).NumberOfEdges());
    
    for ( i = 1 ; i <= VSPGeom().Grid(1).NumberOfLoops() ; i++ ) {
     
       for ( j = 1 ; j <= VSPGeom().Grid(1).LoopList(i).NumberOfEdges() ; j++ ) {
        
          Edge = VSPGeom().Grid(1).LoopList(i).Edge(j);
          
          Loop1 = VSPGeom().Grid(1).EdgeList(Edge).Loop1();
          Loop2 = VSPGeom().Grid(1).EdgeList(Edge).Loop2();
          
          if ( i != Loop1 ) LoopHits[Loop1]++;
          if ( i != Loop2 ) LoopHits[Loop2]++;
          
       }
       
       for ( j = 1 ; j <= VSPGeom().Grid(1).LoopList(i).NumberOfEdges() ; j++ ) {
        
          Edge = VSPGeom().Grid(1).LoopList(i).Edge(j);
          
          Loop1 = VSPGeom().Grid(1).EdgeList(Edge).Loop1();
          Loop2 = VSPGeom().Grid(1).EdgeList(Edge).Loop2();
          
          EdgeHits[Edge] = MAX3(1, LoopHits[Loop1], LoopHits[Loop2] );
          
       }       
          
       for ( j = 1 ; j <= VSPGeom().Grid(1).LoopList(i).NumberOfEdges() ; j++ ) {
        
          Edge = VSPGeom().Grid(1).LoopList(i).Edge(j);
          
          Loop1 = VSPGeom().Grid(1).EdgeList(Edge).Loop1();
          Loop2 = VSPGeom().Grid(1).EdgeList(Edge).Loop2();
          
          LoopHits[Loop1] = 0;
          LoopHits[Loop2] = 0;
          
       }       
       
    }   
              
    
    U = new double[NumberOfVortexLoops_ + 1];
    V = new double[NumberOfVortexLoops_ + 1];
    W = new double[NumberOfVortexLoops_ + 1];

    UDenom = new double[NumberOfVortexLoops_ + 1];
    VDenom = new double[NumberOfVortexLoops_ + 1];
    WDenom = new double[NumberOfVortexLoops_ + 1];
    
    zero_double_array(U, NumberOfVortexLoops_);
    zero_double_array(V, NumberOfVortexLoops_);
    zero_double_array(W, NumberOfVortexLoops_);
    
    zero_double_array(UDenom, NumberOfVortexLoops_);
    zero_double_array(VDenom, NumberOfVortexLoops_);
    zero_double_array(WDenom, NumberOfVortexLoops_);
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       Loop1 = SurfaceVortexEdge(j).LoopL();
       Loop2 = SurfaceVortexEdge(j).LoopR();

       Dot = vector_dot(VortexLoop(Loop1).Normal(), VortexLoop(Loop2).Normal());

       if ( !SurfaceVortexEdge(j).IsTrailingEdge() && ( Loop1 != Loop2 ) && Loop1 != 0 && Loop2 != 0 ) {

          // Vector between centroids of loop 1 and 2
          
          Vec[0] = VortexLoop(Loop2).Xc() - VortexLoop(Loop1).Xc();
          Vec[1] = VortexLoop(Loop2).Yc() - VortexLoop(Loop1).Yc();
          Vec[2] = VortexLoop(Loop2).Zc() - VortexLoop(Loop1).Zc();
         
          Mag1 = sqrt(vector_dot(Vec,Vec));
         
          Vec[0] /= Mag1;
          Vec[1] /= Mag1;
          Vec[2] /= Mag1;     
          
          // Loop 1
          
          Vec1[0] = SurfaceVortexEdge(j).Xc() - VortexLoop(Loop1).Xc();
          Vec1[1] = SurfaceVortexEdge(j).Yc() - VortexLoop(Loop1).Yc();
          Vec1[2] = SurfaceVortexEdge(j).Zc() - VortexLoop(Loop1).Zc();
          
          Mag1 = sqrt(vector_dot(Vec1,Vec1));
          
          Vec1[0] /= Mag1;
          Vec1[1] /= Mag1;
          Vec1[2] /= Mag1;
          
          // Loop 2
          
          Vec2[0] = VortexLoop(Loop2).Xc() - SurfaceVortexEdge(j).Xc();
          Vec2[1] = VortexLoop(Loop2).Yc() - SurfaceVortexEdge(j).Yc();
          Vec2[2] = VortexLoop(Loop2).Zc() - SurfaceVortexEdge(j).Zc();
          
          Mag2 = sqrt(vector_dot(Vec2,Vec2));
   
          Vec2[0] /= Mag2;
          Vec2[1] /= Mag2;
          Vec2[2] /= Mag2;

          Gradient = -0.5*( VortexLoop(Loop2).Gamma() - VortexLoop(Loop1).Gamma() ) / ( Mag1 + Mag2 );
          
          Gradient /= EdgeHits[j];
          
          U[Loop1] += Gradient * Vec[0] * ABS(Vec[0]); UDenom[Loop1] += ABS(Vec[0]);
          V[Loop1] += Gradient * Vec[1] * ABS(Vec[1]); VDenom[Loop1] += ABS(Vec[1]);
          W[Loop1] += Gradient * Vec[2] * ABS(Vec[2]); WDenom[Loop1] += ABS(Vec[2]);
          
          U[Loop2] += Gradient * Vec[0] * ABS(Vec[0]); UDenom[Loop2] += ABS(Vec[0]);
          V[Loop2] += Gradient * Vec[1] * ABS(Vec[1]); VDenom[Loop2] += ABS(Vec[1]);
          W[Loop2] += Gradient * Vec[2] * ABS(Vec[2]); WDenom[Loop2] += ABS(Vec[2]);
          
       }

    }

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       if ( UDenom[i] > 0. ) { U[i] /= UDenom[i]; VortexLoop(i).U() += U[i]; }
       if ( VDenom[i] > 0. ) { V[i] /= VDenom[i]; VortexLoop(i).V() += V[i]; }
       if ( WDenom[i] > 0. ) { W[i] /= WDenom[i]; VortexLoop(i).W() += W[i]; }
      
    }    
    
    // Subtract out normal component
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       Dot = vector_dot(VortexLoop(i).Normal(), VortexLoop(i).Velocity());

       VortexLoop(i).U() -= Dot * VortexLoop(i).Nx();
       VortexLoop(i).V() -= Dot * VortexLoop(i).Ny();
       VortexLoop(i).W() -= Dot * VortexLoop(i).Nz();
      
    }       
    

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {

          Loop1 = SurfaceVortexEdge(j).LoopL();
          Loop2 = SurfaceVortexEdge(j).LoopR();

          VortexLoop(Loop1).U() = 0.;
          VortexLoop(Loop1).V() = 0.;
          VortexLoop(Loop1).W() = 0.;

          VortexLoop(Loop2).U() = 0.;
          VortexLoop(Loop2).V() = 0.;
          VortexLoop(Loop2).W() = 0.;          
          
       }

    }
        
    // Calculate Cp

    CpCrit = 1./(1.4*Mach_*Mach_)*(pow( (1.+0.5*(1.4-1)*Mach_*Mach_)/(1+0.5*(1.4-1)),1.4/(1.4-1))-1.);
 
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
 
       VortexLoop(i).dCp() =  pow(VortexLoop(i).U(),2.)
                            + pow(VortexLoop(i).V(),2.)
                            + pow(VortexLoop(i).W(),2.);

       if ( VortexLoop(i).dCp() > QMax_*QMax_*pow(LocalFreeStreamVelocity_[i][4],2.) ) {
       
          VortexLoop(i).dCp() = QMax_*QMax_*pow(LocalFreeStreamVelocity_[i][4],2.);

       } 
       
       q2 = VortexLoop(i).dCp();

       VortexLoop(i).dCp() = 1. - VortexLoop(i).dCp(); 
       
       // 2nd order correction
 
       Cp = VortexLoop(i).dCp()/pow(LocalFreeStreamVelocity_[i][4],2.);

       Fact = 1.;
       
       if ( !NoKarmanTsienCorrection_ ) Fact = 1. - 0.25*Mach_*Mach_*MAX(Cp,CpCrit);

       CpC = Cp*Fact; 
       
       CpC = MIN(MAX(CpMin_*pow(LocalFreeStreamVelocity_[i][4],2.), CpC),CpMax_);
 
       Ratio = 1;
       
       if ( Cp != 0 ) {
          
          Ratio = CpC / Cp;
          
       }
       
       if ( Ratio <= 0. ) Ratio = 1.;

       // Update Cp, and keep track of the ratio

       VortexLoop(i).CompressibilityFactor() = Ratio;

       VortexLoop(i).dCp() = CpC;     
       
       // Add in unsteady correction
       
       if ( TimeAccurate_ ) {
          
          VortexLoop(i).dCp() += ( Gamma_[i] - GammaNM1_[i] ) / TimeStep_;
          
       }
 
    }  

    // Enforce base pressures
  
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       if ( LoopIsOnBaseRegion_[i] ) VortexLoop(i).dCp() = CpBase_;
       
    }
    
    delete [] U;
    delete [] V;
    delete [] W;

    delete [] UDenom;
    delete [] VDenom;
    delete [] WDenom;
            
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER IntegrateForcesAndMoments                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::IntegrateForcesAndMoments(int UnsteadyEvaluation)
{

    int j, Loop1, Loop2, LoadCase;
    double Fx, Fy, Fz;
    double CA, SA, CB, SB;
    double Cx, Cy, Cz, Cmx, Cmy, Cmz, SteadyComponent, CompressibilityFactor;
    double Cx1, Cy1, Cz1, Cmx1, Cmy1, Cmz1;
    double Cx2, Cy2, Cz2, Cmx2, Cmy2, Cmz2;
    double CL1, CL2, CD1, CD2, CS1, CS2;
    MATRIX T(3,3), X(3), B(3);
    
    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
    
    SteadyComponent = 1.;
    
    if ( UnsteadyEvaluation == 1 ) SteadyComponent = 0.;

    // Loop over vortex edges and integrate the forces / moments
    
    if ( ModelType_ == VLM_MODEL ) {
 
       Cx1 = Cy1 = Cz1 = Cmx1 = Cmy1 = Cmz1 = 0.;
       Cx2 = Cy2 = Cz2 = Cmx2 = Cmy2 = Cmz2 = 0.;
       
       if ( Mach_ < 1. ) {
          
#pragma omp parallel for reduction(+:Cx1, Cy1, Cz1, Cmx1, Cmy1, Cmz1, Cx2, Cy2, Cz2, Cmx2, Cmy2, Cmz2) private(Loop1, Loop2, Fx, Fy, Fz, CompressibilityFactor)
          for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
   
             Loop1 = SurfaceVortexEdge(j).LoopL();
             Loop2 = SurfaceVortexEdge(j).LoopR();
             
             if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {

                CompressibilityFactor = 0.5*( VortexLoop(Loop1).CompressibilityFactor() + VortexLoop(Loop2).CompressibilityFactor() );
   
                // Sum up forces and moments from each edge
                
                Fx = ( SteadyComponent*SurfaceVortexEdge(j).Normal_Trefftz_Fx() + SurfaceVortexEdge(j).Unsteady_Fx() ) * CompressibilityFactor;
                Fy = ( SteadyComponent*SurfaceVortexEdge(j).Normal_Trefftz_Fy() + SurfaceVortexEdge(j).Unsteady_Fy() ) * CompressibilityFactor;
                Fz = ( SteadyComponent*SurfaceVortexEdge(j).Normal_Trefftz_Fz() + SurfaceVortexEdge(j).Unsteady_Fz() ) * CompressibilityFactor;
                             
                Cx1 += Fx;
                Cy1 += Fy;
                Cz1 += Fz;
                
                Cmx1 += Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
                Cmy1 += Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                Cmz1 += Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw
                  
             }
             
             if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {

                CompressibilityFactor = 0.5*( VortexLoop(Loop1).CompressibilityFactor() + VortexLoop(Loop2).CompressibilityFactor() );
   
                // Sum up forces and moments from each edge
                
                Fx = ( SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fx() + SurfaceVortexEdge(j).Unsteady_Fx() ) * CompressibilityFactor;
                Fy = ( SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fy() + SurfaceVortexEdge(j).Unsteady_Fy() ) * CompressibilityFactor;
                Fz = ( SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fz() + SurfaceVortexEdge(j).Unsteady_Fz() ) * CompressibilityFactor;
                             
                Cx2 += Fx;
                Cy2 += Fy;
                Cz2 += Fz;
                
                Cmx2 += Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
                Cmy2 += Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                Cmz2 += Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw
                  
             }             
             
          }
          
       }
       
       else {       
      
#pragma omp parallel for reduction(+:Cx, Cy, Cz, Cmx, Cmy, Cmz) private(Loop1, Loop2, Fx, Fy, Fz)
          for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
             if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {
                
                Loop1 = SurfaceVortexEdge(j).LoopL();
                Loop2 = SurfaceVortexEdge(j).LoopR();
                
                // Edge forces
         
                Fx = SteadyComponent*SurfaceVortexEdge(j).Fx() + SurfaceVortexEdge(j).Unsteady_Fx();
                Fy = SteadyComponent*SurfaceVortexEdge(j).Fy() + SurfaceVortexEdge(j).Unsteady_Fy();
                Fz = SteadyComponent*SurfaceVortexEdge(j).Fz() + SurfaceVortexEdge(j).Unsteady_Fz();
                                
                // Integrate forces
                
                Cx += Fx;
                Cy += Fy;
                Cz += Fz;
                
                // Integrate moments
      
                Cmx += Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
                Cmy += Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                Cmz += Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw
                  
             }
               
          }
             
       }

    }
    
    // Integrate forces for Panel model
    
    else if ( ModelType_ == PANEL_MODEL ) {
    
       Cx1 = Cy1 = Cz1 = Cmx1 = Cmy1 = Cmz1 = 0.;
       Cx2 = Cy2 = Cz2 = Cmx2 = Cmy2 = Cmz2 = 0.;

#pragma omp parallel for reduction(+:Cx1, Cy1, Cz1, Cmx1, Cmy1, Cmz1, Cx2, Cy2, Cz2, Cmx2, Cmy2, Cmz2) private(Loop1, Loop2, Fx, Fy, Fz, CompressibilityFactor)
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
     
          Loop1 = SurfaceVortexEdge(j).LoopL();
          Loop2 = SurfaceVortexEdge(j).LoopR();
               
          // Forces based on trailing edge analysis... we will use this to get the induced drag

          if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {

             // Approximate KT correction

             if ( !NoKarmanTsienCorrection_ ) CompressibilityFactor = pow(1.+0.25*Mach_*Mach_,2.);

             Fx = -( SteadyComponent*SurfaceVortexEdge(j).Normal_Trefftz_Fx() + SurfaceVortexEdge(j).Unsteady_Fx() ) * CompressibilityFactor;
             Fy = -( SteadyComponent*SurfaceVortexEdge(j).Normal_Trefftz_Fy() + SurfaceVortexEdge(j).Unsteady_Fy() ) * CompressibilityFactor;
             Fz = -( SteadyComponent*SurfaceVortexEdge(j).Normal_Trefftz_Fz() + SurfaceVortexEdge(j).Unsteady_Fz() ) * CompressibilityFactor;
   
             Cx1 += Fx;
             Cy1 += Fy;
             Cz1 += Fz;
                   
             Cmx1 += Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
             Cmy1 += Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
             Cmz1 += Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw
             
          }
          
          // Forces based on non-TE edges... with KT correction... we will use this to get the lift, side force, and moments
          
          if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {

             // KT correction
             
             CompressibilityFactor = 0.5*( VortexLoop(Loop1).CompressibilityFactor() + VortexLoop(Loop2).CompressibilityFactor() );

             Fx = ( SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fx() + SurfaceVortexEdge(j).Unsteady_Fx() ) * CompressibilityFactor;
             Fy = ( SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fy() + SurfaceVortexEdge(j).Unsteady_Fy() ) * CompressibilityFactor;
             Fz = ( SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fz() + SurfaceVortexEdge(j).Unsteady_Fz() ) * CompressibilityFactor;

             Cx2 += Fx;
             Cy2 += Fy;
             Cz2 += Fz;
                   
             Cmx2 += Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
             Cmy2 += Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
             Cmz2 += Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw
             
          }
          
       }

    }  

    // Calculate lift and drag using both integrations
    
    CL1 = -Cx1 * SA + Cz1 * CA;
    CL2 = -Cx2 * SA + Cz2 * CA;

    CD1 =  ( Cx1 * CA + Cz1 * SA ) * CB - Cy1 * SB;
    CD2 =  ( Cx2 * CA + Cz2 * SA ) * CB - Cy2 * SB;

    CS1 =  ( Cx1 * CA + Cz1 * SA ) * SB + Cy1 * CB; 
    CS2 =  ( Cx2 * CA + Cz2 * SA ) * SB + Cy2 * CB; 

    // Take average, correct for obvious errors
        
    if ( Verbose_ ) printf("CL1,2: %f %f \n",CL1,CL2);
    if ( Verbose_ ) printf("CD1,2: %f %f \n",CD1,CD2);

    B(1) = CD1; // Based on normal velocity, corrected for KT
    B(2) = CS2; // Based on full   velocity
    B(3) = CL2; // Based on full   velocity

    Cmx = Cmx2;
    Cmy = Cmy2;
    Cmz = Cmz2;       
    
    // Transform back to xyz space
    
    T(1,1) =  CA*CB ; T(1,2) = -SB ; T(1,3) = SA*CB;  // CD
    T(2,1) =  CA*SB ; T(2,2) =  CB ; T(2,3) = SA*SB;  // CS
    T(3,1) = -SA    ; T(3,2) =  0. ; T(3,3) = CA;     // CL
           
    X = B/T;

    Cx = X(1);
    Cy = X(2);
    Cz = X(3);
       
    LoadCase = 0;
    
    if ( UnsteadyEvaluation ) LoadCase = 2;
    
    CFx_[LoadCase] = Cx;
    CFy_[LoadCase] = Cy;
    CFz_[LoadCase] = Cz;
    
    CMx_[LoadCase] = Cmx;
    CMy_[LoadCase] = Cmy;
    CMz_[LoadCase] = Cmz;

    CFx_[LoadCase] /= (0.5*Sref_);
    CFy_[LoadCase] /= (0.5*Sref_);
    CFz_[LoadCase] /= (0.5*Sref_);
    
    CMx_[LoadCase] /= (0.5*Sref_*Bref_); // Roll
    CMy_[LoadCase] /= (0.5*Sref_*Cref_); // Pitch
    CMz_[LoadCase] /= (0.5*Sref_*Bref_); // Yaw

    // Adjust for symmetry
  
    if ( DoSymmetryPlaneSolve_ ) {
       
       CFx_[LoadCase] *= 2.;
       CFy_[LoadCase] *= 2.; 
       CFz_[LoadCase] *= 2.; 
   
       CMx_[LoadCase] *= 2.; 
       CMy_[LoadCase] *= 2.; 
       CMz_[LoadCase] *= 2.; 
       
    }
   
    if ( DoSymmetryPlaneSolve_ == SYM_X ) CMy_[LoadCase] = CMz_[LoadCase] = CFx_[LoadCase] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Y ) CMx_[LoadCase] = CMz_[LoadCase] = CFy_[LoadCase] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Z ) CMx_[LoadCase] = CMy_[LoadCase] = CFz_[LoadCase] = 0.;
    
    // Now calculate CL, CD, CS
 
    CL_[LoadCase] = (-CFx_[LoadCase] * SA + CFz_[LoadCase] * CA );
    CD_[LoadCase] = ( CFx_[LoadCase] * CA + CFz_[LoadCase] * SA ) * CB - CFy_[LoadCase] * SB;
    CS_[LoadCase] = ( CFx_[LoadCase] * CA + CFz_[LoadCase] * SA ) * SB + CFy_[LoadCase] * CB; 

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER CalculateCLmaxLimitedForces                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateCLmaxLimitedForces(int UnsteadyEvaluation)
{

    int i, j, k, Loop, Loop1, Loop2, LoadCase;
    int NumberOfStations, SpanStation, SurfaceID; 
    double Fx, Fy, Fz, Fxi, Fyi, Fzi, Wgt, Area1, Area2;
    double Length, Re, Cf, Cl, Cd, Cdi, Cs, Ct, Cn, Cx, Cy, Cz;
    double Swet, SwetTotal, StallFact, CompressibilityFactor;
    double CA, SA, CB, SB, CMx, CMy, CMz, Cmx, Cmy, Cmz, Cl_2d, dCD;
    double Normal[3], Mag, SteadyComponent;

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
    
    SteadyComponent = 1.;
    
    if ( UnsteadyEvaluation == 1 ) SteadyComponent = 0.;
        
    // Zero out spanwise loading arrays
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
      
       NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
 
       zero_double_array(Span_Cx_[i],    NumberOfStations);
       zero_double_array(Span_Cy_[i],    NumberOfStations);
       zero_double_array(Span_Cz_[i],    NumberOfStations);       

       zero_double_array(Span_Cxi_[i],   NumberOfStations);
       zero_double_array(Span_Cyi_[i],   NumberOfStations);
       zero_double_array(Span_Czi_[i],   NumberOfStations);     
        
       zero_double_array(Span_Cn_[i],    NumberOfStations);       
 
       zero_double_array(Span_Cl_[i],    NumberOfStations);
       zero_double_array(Span_Cd_[i],    NumberOfStations);
       zero_double_array(Span_Cs_[i],    NumberOfStations);       
      
       zero_double_array(Span_Cmx_[i],   NumberOfStations);
       zero_double_array(Span_Cmy_[i],   NumberOfStations);
       zero_double_array(Span_Cmz_[i],   NumberOfStations);       
      
       zero_double_array(Span_Yavg_[i],  NumberOfStations);       
       zero_double_array(Span_Area_[i],  NumberOfStations);
       
       zero_double_array(Local_Vel_[i],  NumberOfStations);
         
    }   
    
    // Initialize LE edge array
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
       
       for ( j = 1 ; j <= VSPGeom().VSP_Surface(i).NumberOfSpanStations() ; j++ ) { 
    
          Span_LE_[i][j] = 1.e9;
          
       }
       
    }
            
    // Loop over vortex edges and calculate forces via K-J theorem, using only wake induced velocities applied at TE
  
    Fx = Fy = Fz = CMx = CMy = CMz = 0.;
    
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
       
       Loop1 = SurfaceVortexEdge(j).LoopL();
       Loop2 = SurfaceVortexEdge(j).LoopR();
                 
       if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {
          
          // Extract out forces
          
          if ( Mach_ < 1. ) {

             CompressibilityFactor = 0.5*( VortexLoop(Loop1).CompressibilityFactor() + VortexLoop(Loop2).CompressibilityFactor() );

             Fx  = ( SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fx() + SurfaceVortexEdge(j).Unsteady_Fx() ) * CompressibilityFactor;
             Fy  = ( SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fy() + SurfaceVortexEdge(j).Unsteady_Fy() ) * CompressibilityFactor;
             Fz  = ( SteadyComponent*SurfaceVortexEdge(j).Trefftz_Fz() + SurfaceVortexEdge(j).Unsteady_Fz() ) * CompressibilityFactor;

             Fxi = ( SteadyComponent*SurfaceVortexEdge(j).Normal_Trefftz_Fx() + SurfaceVortexEdge(j).Unsteady_Fx() ) * CompressibilityFactor;
             Fyi = ( SteadyComponent*SurfaceVortexEdge(j).Normal_Trefftz_Fy() + SurfaceVortexEdge(j).Unsteady_Fy() ) * CompressibilityFactor;
             Fzi = ( SteadyComponent*SurfaceVortexEdge(j).Normal_Trefftz_Fz() + SurfaceVortexEdge(j).Unsteady_Fz() ) * CompressibilityFactor;

          }
          
          else {
             
             Fx = SteadyComponent*SurfaceVortexEdge(j).Fx() + SurfaceVortexEdge(j).Unsteady_Fx();
             Fy = SteadyComponent*SurfaceVortexEdge(j).Fy() + SurfaceVortexEdge(j).Unsteady_Fy();
             Fz = SteadyComponent*SurfaceVortexEdge(j).Fz() + SurfaceVortexEdge(j).Unsteady_Fz();
             
          }
         
          // Sum up span wise loading
   
          for ( k = 1 ; k <= 2 ; k++ ) {
           
             if ( k == 1 ) Loop = SurfaceVortexEdge(j).Loop1();
             
             if ( k == 2 ) Loop = SurfaceVortexEdge(j).Loop2();
             
             Wgt = VortexLoop(Loop).Area() / ( VortexLoop(SurfaceVortexEdge(j).Loop1()).Area() + VortexLoop(SurfaceVortexEdge(j).Loop2()).Area() );
   
             // Wing Surface
             
             if ( VortexLoop(Loop).DegenWingID() > 0 ) {
                
                SurfaceID = VortexLoop(Loop).SurfaceID();
                
                SpanStation = VortexLoop(Loop).SpanStation();
                
                // Chordwise integrated forces
   
                Span_Cx_[SurfaceID][SpanStation] += Wgt*Fx;
   
                Span_Cy_[SurfaceID][SpanStation] += Wgt*Fy;
   
                Span_Cz_[SurfaceID][SpanStation] += Wgt*Fz;
                                               
                Span_Cn_[SurfaceID][SpanStation] += Wgt * Fx * VortexLoop(Loop).Nx()
                                                  + Wgt * Fy * VortexLoop(Loop).Ny()
                                                  + Wgt * Fz * VortexLoop(Loop).Nz();
                                                                  
                // Chordwise integrated induced forces
                
                Span_Cxi_[SurfaceID][SpanStation] += Wgt*Fxi;
   
                Span_Cyi_[SurfaceID][SpanStation] += Wgt*Fyi;
   
                Span_Czi_[SurfaceID][SpanStation] += Wgt*Fzi;                

                // Chordwise integrated moments
   
                Span_Cmx_[SurfaceID][SpanStation] += Wgt * Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Wgt * Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
            
                Span_Cmy_[SurfaceID][SpanStation] += Wgt * Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Wgt * Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                      
                Span_Cmz_[SurfaceID][SpanStation] += Wgt * Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Wgt * Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw    

                
             }
             
             // Body Surface
             
             else {
            
                SurfaceID = VortexLoop(Loop).SurfaceID();
                
                SpanStation = 1;
           
                // Chordwise integrated forces
                
                if ( Mach_ < 1. ) {
                       
                   Span_Cx_[SurfaceID][SpanStation] += Wgt*Fx;
   
                   Span_Cy_[SurfaceID][SpanStation] += Wgt*Fy;
   
                   Span_Cz_[SurfaceID][SpanStation] += Wgt*Fz;
                                  
                   Span_Cn_[SurfaceID][SpanStation] += Wgt * Fx * VortexLoop(Loop).Nx()
                                                     + Wgt * Fy * VortexLoop(Loop).Ny()
                                                     + Wgt * Fz * VortexLoop(Loop).Nz();
                                                     
                   // Chordwise integrated induced forces
                   
                   Span_Cxi_[SurfaceID][SpanStation] += Wgt*Fxi;
      
                   Span_Cyi_[SurfaceID][SpanStation] += Wgt*Fyi;
      
                   Span_Czi_[SurfaceID][SpanStation] += Wgt*Fzi;                
                                                      
                }
                
                // Chordwise integrated moments
                       
                Span_Cmx_[SurfaceID][SpanStation] += Wgt * Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Wgt * Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
               
                Span_Cmy_[SurfaceID][SpanStation] += Wgt * Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Wgt * Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                         
                Span_Cmz_[SurfaceID][SpanStation] += Wgt * Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Wgt * Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw
   
             }
             
          }      
       
       }
        
    }
    
    // Calculate span station areas and y values

    for ( j = 1 ; j <= NumberOfVortexLoops_ ; j++ ) {

       if ( VortexLoop(j).DegenWingID() > 0 ) {
           
          SurfaceID = VortexLoop(j).SurfaceID();
                
          SpanStation = VortexLoop(j).SpanStation();
          
       }
       
       else {
          
          SurfaceID = VortexLoop(j).SurfaceID();
                
          SpanStation = 1;
          
       }          
       
       // Find LE... assume it the minimum X location
       
       Span_LE_[SurfaceID][SpanStation] = MIN(Span_LE_[SurfaceID][SpanStation], VortexLoop(j).Xc());
       
       // Average y span location, and strip area
              
       Span_Yavg_[SurfaceID][SpanStation] += VortexLoop(j).Yc() * VortexLoop(j).Area();

       Span_Area_[SurfaceID][SpanStation] += VortexLoop(j).Area();
       
       Local_Vel_[SurfaceID][SpanStation] += LocalFreeStreamVelocity_[j][4] * VortexLoop(j).Area();

    }
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) {
       
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
           
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
          
       }
       
       else {
          
          NumberOfStations = 1;
          
       }     
              
       for ( j = 1 ; j <= NumberOfStations ; j++ ) {
          
          Span_Yavg_[i][j] /= Span_Area_[i][j];
          
          Local_Vel_[i][j] /= Span_Area_[i][j];
          
       }
       
    }    

    // Calculate maximum CL... imposing local 2D Clmax limits
        
    LoadCase = 0;
    
    if ( UnsteadyEvaluation ) LoadCase = 2;
        
    CL_[LoadCase] = CS_[LoadCase] = CD_[LoadCase] = CFx_[LoadCase] = CFy_[LoadCase] = CFz_[LoadCase]= CMx_[LoadCase] = CMy_[LoadCase] = CMz_[LoadCase] = SwetTotal = CDo_ = 0.;  

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
          
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();

       }
       
       else {
          
          NumberOfStations = 1;

       }   
       
       VSPGeom().VSP_Surface(i).CDo() = 0.;
           
       for ( k = 1 ; k <= NumberOfStations ; k++ ) {
         
          Cx = Span_Cx_[i][k] / (0.5*Span_Area_[i][k]);
          
          Cy = Span_Cy_[i][k] / (0.5*Span_Area_[i][k]);
          
          Cz = Span_Cz_[i][k] / (0.5*Span_Area_[i][k]);
          
          Cn = Span_Cn_[SurfaceID][SpanStation] / (0.5*Span_Area_[i][k]);
                    
          Cl =    ( -Span_Cx_[i][k] * SA + Span_Cz_[i][k] * CA )                              / (0.5*Span_Area_[i][k]);
   
          Cd =  ( (  Span_Cx_[i][k] * CA + Span_Cz_[i][k] * SA ) * CB - Span_Cy_[i][k] * SB ) / (0.5*Span_Area_[i][k]);
   
          Cs =  ( (  Span_Cx_[i][k] * CA + Span_Cz_[i][k] * SA ) * SB + Span_Cy_[i][k] * CB ) / (0.5*Span_Area_[i][k]);

          Cdi = ( (  Span_Cxi_[i][k] * CA + Span_Czi_[i][k] * SA ) * CB - Span_Cyi_[i][k] * SB ) / (0.5*Span_Area_[i][k]);

          Ct = sqrt( Cl*Cl + Cs*Cs );
          
          Cmx = Span_Cmx_[i][k] / (0.5*Span_Area_[i][k]*VSPGeom().VSP_Surface(i).LocalChord(k)); 

          Cmy = Span_Cmy_[i][k] / (0.5*Span_Area_[i][k]*VSPGeom().VSP_Surface(i).LocalChord(k)); 

          Cmz = Span_Cmz_[i][k] / (0.5*Span_Area_[i][k]*VSPGeom().VSP_Surface(i).LocalChord(k)); 
      
          // Adjust forces and moments for local 2d stall
          
          StallFact = 1.;
          
          if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE && Clmax_2d_ > 0. ) {
          
             Cl_2d = Clmax_2d_ * pow(Local_Vel_[i][k], 2.);
          
             if ( ABS(Ct) > 0. ) StallFact = ABS(MIN(ABS(Ct), Cl_2d) / ABS(Ct));
             
          }
          
          Span_Cl_[i][k] = StallFact * Cl;
          Span_Cd_[i][k] = StallFact * Cdi;
          Span_Cs_[i][k] = StallFact * Cs;
          
          Span_Cx_[i][k] = StallFact * Cx;
          Span_Cy_[i][k] = StallFact * Cy;
          Span_Cz_[i][k] = StallFact * Cz;
           
          Span_Cmx_[i][k] = StallFact * Cmx;
          Span_Cmy_[i][k] = StallFact * Cmy;
          Span_Cmz_[i][k] = StallFact * Cmz;

          // Integrate spanwise forces and moments
          
           CL_[LoadCase] += 0.5 *  Span_Cl_[i][k] * Span_Area_[i][k];
           CD_[LoadCase] += 0.5 *  Span_Cd_[i][k] * Span_Area_[i][k];
           CS_[LoadCase] += 0.5 *  Span_Cs_[i][k] * Span_Area_[i][k];
 
          CFx_[LoadCase] += 0.5 *  Span_Cx_[i][k] * Span_Area_[i][k];
          CFy_[LoadCase] += 0.5 *  Span_Cy_[i][k] * Span_Area_[i][k];
          CFz_[LoadCase] += 0.5 *  Span_Cz_[i][k] * Span_Area_[i][k];
          
          CMx_[LoadCase] += 0.5 * Span_Cmx_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
          CMy_[LoadCase] += 0.5 * Span_Cmy_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
          CMz_[LoadCase] += 0.5 * Span_Cmz_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
          
          // Spanwise viscous drag for wings
          
          if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
          
             Length = VSPGeom().VSP_Surface(i).LocalChord(k);
             
             Re = ReCref_ * Local_Vel_[i][k] * Length / Cref_;
             
             Cf = 0.455 / pow(log10(Re),2.58);
           
             Swet = 2.*Span_Area_[i][k];
   
             // Add in 25% for thickness drag to wings
             
             Swet *= 1.25;
                    
             // Bump by 25% for miscellaneous
             
             dCD = 1.25 * Cf * pow(Local_Vel_[i][k], 2.) * Swet;
             
             // Save at component level
             
             VSPGeom().VSP_Surface(i).CDo() += dCD / Sref_;
             
             // Total drag
      
             CDo_ += dCD;
             
             SwetTotal += Swet;
             
          }
          
       }
         
    }    

    CL_[LoadCase] /= 0.5*Sref_;
    CD_[LoadCase] /= 0.5*Sref_;
    CS_[LoadCase] /= 0.5*Sref_;
    
    CFx_[LoadCase] /= 0.5*Sref_;
    CFy_[LoadCase] /= 0.5*Sref_;
    CFz_[LoadCase] /= 0.5*Sref_;

    CMx_[LoadCase] /= 0.5*Bref_*Sref_;
    CMy_[LoadCase] /= 0.5*Cref_*Sref_;
    CMz_[LoadCase] /= 0.5*Bref_*Sref_;
  
    // Adjust for symmetry
    
    if ( DoSymmetryPlaneSolve_  ) {
       
       CFx_[LoadCase] *= 2.;
       CFy_[LoadCase] *= 2.; 
       CFz_[LoadCase] *= 2.; 
   
       CMx_[LoadCase] *= 2.; 
       CMy_[LoadCase] *= 2.; 
       CMz_[LoadCase] *= 2.; 
       
       CL_[LoadCase] *= 2.; 
       CD_[LoadCase] *= 2.; 
       CS_[LoadCase] *= 2.; 
       
    }
        
    if ( DoSymmetryPlaneSolve_ == SYM_X ) CMy_[LoadCase] = CMz_[LoadCase] = CFx_[LoadCase] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Y ) CMx_[LoadCase] = CMz_[LoadCase] = CFy_[LoadCase] = CS_[LoadCase] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Z ) CMx_[LoadCase] = CMy_[LoadCase] = CFz_[LoadCase] = 0.;
              
    // Loop over body surfaces and calculate skin friction drag
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
          
          Length = VSPGeom().VSP_Surface(i).AverageChord();
          
          Re = ReCref_ * Length / Cref_;
          
          Cf = 0.455 / pow(log10(Re),2.58);
        
          Swet = VSPGeom().VSP_Surface(i).WettedArea();
          
          // Body is split into 4 parts, wetted area over accounted for...  also add in 10% for form drag
          
          Swet *= 0.25 * 1.10;
    
          // Bump by 25% for miscellaneous
          
          dCD = 1.25 * Cf * Swet;
          
          // Save at component level
          
          VSPGeom().VSP_Surface(i).CDo() = dCD / Sref_;
          
          // Total drag
   
          CDo_ += dCD;
          
          SwetTotal += Swet;
          
       }
 
    }
    
    CDo_ /= Sref_;
    
    // Check that user is not asking for averaged forces... AND not running enough iterations
    
    if ( ForceType_ == FORCE_AVERAGE && WakeIterations_ <= AveragingIteration_ ) {
       
       printf("\n");
       printf("Not enough wake iterations specified to do requested averaging! \n");
       printf("Force and moment averaging turned off! \n");
       printf("\n"); fflush(NULL);
       
       ForceType_ = 0;
       
    }       

    // Running sum of forces and moments
    
    if ( ForceType_ == FORCE_AVERAGE && CurrentWakeIteration_ >= AveragingIteration_ ) {

        CL_[1] +=     CL_[0];
        CD_[1] +=     CD_[0];
        CS_[1] +=     CS_[0];
                  
       CFx_[1] +=    CFx_[0];
       CFy_[1] +=    CFy_[0];
       CFz_[1] +=    CFz_[0];
                  
       CMx_[1] +=    CMx_[0];
       CMy_[1] +=    CMy_[0];
       CMz_[1] +=    CMz_[0];
       
       NumberOfAveragingSteps_++;

    }       
    
    else {
       
       NumberOfAveragingSteps_ = 0;
 
        CL_[1] = 0.;
        CD_[1] = 0.;
        CS_[1] = 0.;
                  
       CFx_[1] = 0.;
       CFy_[1] = 0.;
       CFz_[1] = 0.;
                  
       CMx_[1] = 0.;
       CMy_[1] = 0.;     
          
    }
    
    // Calculate averages of forces and moments
    
    if ( ForceType_ == FORCE_AVERAGE && CurrentWakeIteration_ == WakeIterations_ ) {

        CL_[1] /= NumberOfAveragingSteps_;
        CD_[1] /= NumberOfAveragingSteps_;
        CS_[1] /= NumberOfAveragingSteps_;
                  
       CFx_[1] /= NumberOfAveragingSteps_;
       CFy_[1] /= NumberOfAveragingSteps_;
       CFz_[1] /= NumberOfAveragingSteps_;
                  
       CMx_[1] /= NumberOfAveragingSteps_;
       CMy_[1] /= NumberOfAveragingSteps_;     
          
    }       
    
}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CalculateSpanWiseLoading                       #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateSpanWiseLoading(void)
{
 
    int i, k, NumberOfStations;
    double TotalLift, CFx, CFy, CFz;
    double CL, CD, CS, CMx, CMy, CMz;
    
    // Write out generic header
    
    WriteCaseHeader(LoadFile_);
    
    // Write out column labels
    
                    // 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 
    fprintf(LoadFile_,"   Wing      Yavg     Chord     V/Vinf      Cl        Cd        Cs        Cx        Cy       Cz        Cmx       Cmy       Cmz \n");

    TotalLift = 0.;  

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
        
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
              
          for ( k = 1 ; k <= NumberOfStations ; k++ ) {

             fprintf(LoadFile_,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
                     i,
                     Span_Yavg_[i][k],
                     VSPGeom().VSP_Surface(i).LocalChord(k),
                     Local_Vel_[i][k],
                     Span_Cl_[i][k],
                     Span_Cd_[i][k],
                     Span_Cs_[i][k],
                     Span_Cx_[i][k],
                     Span_Cy_[i][k],
                     Span_Cz_[i][k],
                     Span_Cmx_[i][k],
                     Span_Cmy_[i][k],
                     Span_Cmz_[i][k]);
            
             TotalLift += 0.5 * Span_Cl_[i][k] * Span_Area_[i][k];
      
          }
          
       }
                 
    }
    
    fprintf(LoadFile_,"\n\n\n");

                    // 123456789 123456789012345678901234567890123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789   
    fprintf(LoadFile_,"Comp      Component-Name                             Mach       AoA      Beta       CL        CDi       CS       CFx       CFy       CFz       Cmx       Cmy       Cmz \n");

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
        
          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
           
          CL = CD = CS = CFx = CFy = CFz = CMx = CMy = CMz = 0.;
          
          for ( k = 1 ; k <= NumberOfStations ; k++ ) {

             CL  += 0.5 *  Span_Cl_[i][k] * Span_Area_[i][k];
             CD  += 0.5 *  Span_Cd_[i][k] * Span_Area_[i][k];
             CS  += 0.5 *  Span_Cs_[i][k] * Span_Area_[i][k];
             CFx += 0.5 *  Span_Cx_[i][k] * Span_Area_[i][k];
             CFy += 0.5 *  Span_Cy_[i][k] * Span_Area_[i][k];
             CFz += 0.5 *  Span_Cz_[i][k] * Span_Area_[i][k];
             CMx += 0.5 * Span_Cmx_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
             CMy += 0.5 * Span_Cmy_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
             CMz += 0.5 * Span_Cmz_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
      
          }
          
          CL /= 0.5*Sref_;
          CD /= 0.5*Sref_;
          CS /= 0.5*Sref_;

          CFx /= 0.5*Sref_;
          CFy /= 0.5*Sref_;
          CFz /= 0.5*Sref_;
          
          CMx /= 0.5*Bref_*Sref_;
          CMy /= 0.5*Cref_*Sref_;
          CMz /= 0.5*Bref_*Sref_;
          
          fprintf(LoadFile_,"%-9d %-40s %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
                  i,
                  VSPGeom().VSP_Surface(i).ComponentName(),
                  Mach_,
                  AngleOfAttack_/TORAD,
                  AngleOfBeta_/TORAD,                 
                  CL,
                  CD,
                  CS,
                  CFx,
                  CFy,
                  CFz,
                  CMx,
                  CMy,
                  CMz);        
          
       }
            
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
        
          k = 1;
    
          fprintf(LoadFile_,"%-9d %-40s %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
                  i,
                  VSPGeom().VSP_Surface(i).ComponentName(),
                  Mach_,
                  AngleOfAttack_/TORAD,
                  AngleOfBeta_/TORAD,                                   
                  0.5 *   Span_Cl_[i][k] * Span_Area_[i][k] / (0.5*Sref_),
                  0.5 *   Span_Cd_[i][k] * Span_Area_[i][k] / (0.5*Sref_),
                  0.5 *   Span_Cs_[i][k] * Span_Area_[i][k] / (0.5*Sref_),
                  0.5 *   Span_Cx_[i][k] * Span_Area_[i][k] / (0.5*Sref_),
                  0.5 *   Span_Cy_[i][k] * Span_Area_[i][k] / (0.5*Sref_),
                  0.5 *   Span_Cz_[i][k] * Span_Area_[i][k] / (0.5*Sref_),
                  0.5 *  Span_Cmx_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k) / (0.5*Bref_*Sref_),
                  0.5 *  Span_Cmy_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k) / (0.5*Cref_*Sref_),
                  0.5 *  Span_Cmz_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k) / (0.5*Bref_*Sref_));
         
          TotalLift += 0.5 * Span_Cl_[i][k] * Span_Area_[i][k];

       }
                 
    }
    
    fprintf(LoadFile_,"\n\n\n");

    TotalLift /= 0.5*Sref_;

}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CalculateVortexBurstLocation                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateVortexBurstLocation(void)
{
 
    int i, j, k, Loop, SurfaceID, NumberOfStations, Burst;
    double X, WingCref, Vec[3], S[3], xyzLE[3], xyzTE[3], XBurst, CL, Area;
    
    double Normal[3], Sweep, Dot, LocalSweep, Length, Fact;

    // Calculate strip wise forces, as we need them below
    
    CalculateCLmaxLimitedForces(0);
    
    // Loop over each surface
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
    
       // If it's a wing, check it's sweep and see if the vortex has burst
       
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {

          // Calculate the local lift coefficient
          
          CL = Area = 0.;
          
          for ( k = 1 ; k <= VSPGeom().VSP_Surface(i).NumberOfSpanStations() ; k++ ) {

             CL += 0.5 * Span_Cl_[i][k] * Span_Area_[i][k];
 
             Area += Span_Area_[i][k];
      
          }
          
          CL /= 0.5*Area;
          
          CL = ABS(CL);

          // Root LE, TE locations

          xyzLE[0] = VSPGeom().VSP_Surface(i).Root_LE(0);
          xyzLE[1] = VSPGeom().VSP_Surface(i).Root_LE(1);
          xyzLE[2] = VSPGeom().VSP_Surface(i).Root_LE(2);
          
          xyzTE[0] = VSPGeom().VSP_Surface(i).Root_TE(0);
          xyzTE[1] = VSPGeom().VSP_Surface(i).Root_TE(1);
          xyzTE[2] = VSPGeom().VSP_Surface(i).Root_TE(2);          
          
          // Root chord vector and length
          
          S[0] = xyzTE[0] - xyzLE[0];
          S[1] = xyzTE[1] - xyzLE[1];
          S[2] = xyzTE[2] - xyzLE[2];
          
          WingCref = sqrt(vector_dot(S,S));
          
          S[0] /= WingCref;
          S[1] /= WingCref;
          S[2] /= WingCref;

          // Calculate average leading edge sweep
          
          Sweep = Length = 0;
          
          for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
             
             if ( SurfaceVortexEdge(j).DegenWing() == i ) {
                
                if ( SurfaceVortexEdge(j).IsLeadingEdge() ) {
        
                   // Calculate normal
         
                   Loop = SurfaceVortexEdge(j).VortexLoop1() + SurfaceVortexEdge(j).VortexLoop2();
        
                   Normal[0] = VortexLoop(Loop).FlatPlateNormal()[0];
                   Normal[1] = VortexLoop(Loop).FlatPlateNormal()[1];
                   Normal[2] = VortexLoop(Loop).FlatPlateNormal()[2];

                   // Determine local sweep angle
                   
                   Dot = vector_dot(S,SurfaceVortexEdge(j).Vec());
                   
                   Dot = MIN(1.,MAX(-1.,Dot));
                   
                   LocalSweep = acos(Dot);
                   
                   LocalSweep = 0.5*PI - LocalSweep;

                   // Average sweep
                   
                   Sweep += LocalSweep * SurfaceVortexEdge(j).Length();
                   
                   Length += SurfaceVortexEdge(j).Length();
                   
                }
                
             }
             
          }
          
          // Average sweep and angle of attack across wing LE
          
          Sweep /= Length;
          
          // Decide if trailing edge bursting has occurred 

          if ( Sweep >= 40.*TORAD && CL > 0.9 ) {
             
             // Calculate the burst location

             XBurst = 2./(1.+exp(5.0*(CL - 0.9)))*WingCref;

             // Apply bursting model to edges down stream of burst location
     
             if ( XBurst >= 0.1 ) {
               
                Fact = 1.4*sin(Sweep)/CL;
               
             }
            
             else {
    
               Fact = (1.4*sin(Sweep)/CL)/pow(CL/(1.4*sin(Sweep)),0.3);
    
             }               
            
             for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
        
                if ( SurfaceVortexEdge(j).DegenWing() == i ) {

                   Vec[0] = SurfaceVortexEdge(j).Xc() - xyzLE[0];
                   Vec[1] = SurfaceVortexEdge(j).Yc() - xyzLE[1];
                   Vec[2] = SurfaceVortexEdge(j).Zc() - xyzLE[2];
    
                   X = sqrt(vector_dot(Vec,Vec));
                   
                   if ( X >= XBurst ) {

                      if ( SurfaceVortexEdge(j).VortexLoop1() > 0 ) VortexLoop(SurfaceVortexEdge(j).VortexLoop1()).dCp() *= Fact;
                      if ( SurfaceVortexEdge(j).VortexLoop2() > 0 ) VortexLoop(SurfaceVortexEdge(j).VortexLoop2()).dCp() *= Fact;

                      SurfaceVortexEdge(j).Trefftz_Fx() *= Fact;
                      SurfaceVortexEdge(j).Trefftz_Fy() *= Fact;
                      SurfaceVortexEdge(j).Trefftz_Fz() *= Fact;
          
                   }
                   
                }
                
             }      
             
          }
          
       }
       
    }           

}

/*##############################################################################
#                                                                              #
#                          VSP_SOLVER CreateFEMLoadFile                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateFEMLoadFile(int Case)
{
   
    char LoadFileName[2000];
   
    if ( Case == 0 || Case == 1 ) {
       
       // Open the fem load file
    
       sprintf(LoadFileName,"%s.fem",FileName_);
       
       if ( (FEMLoadFile_ = fopen(LoadFileName, "w")) == NULL ) {
   
          printf("Could not open the fem load file for output! \n");
   
          exit(1);
   
       }
       
    }
    
    WriteCaseHeader(FEMLoadFile_);
       
    // Write out FEM beam load file for VLM model
    
    if ( ModelType_ == VLM_MODEL ) {
       
      CreateFEMLoadFileFromVLMSolve(Case);
       
    }
    
    // Write out FEM beam load file for Panel model
    
    else if ( ModelType_ == PANEL_MODEL ) {
       
       CreateFEMLoadFileFromPanelSolve(Case);
       
    }
      
}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER CreateFEMLoadFileFromVLMSolve                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateFEMLoadFileFromVLMSolve(int Case)
{
 
    int i, k, NumberOfStations;
    double TotalLift;
    double Vec[3], VecQC[3], VecQC_Def[3], RVec[3], Force[3], Moment[3], Chord;    

    TotalLift = 0.;  

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
        
          fprintf(FEMLoadFile_,"Wing Surface: %d \n",i);
          fprintf(FEMLoadFile_,"SpanStations: %d \n",VSPGeom().VSP_Surface(i).NumberOfSpanStations());
          fprintf(FEMLoadFile_,"\n");
          
          //                    123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789  
          fprintf(FEMLoadFile_,"   Wing    XLE_ORIG  YLE_ORIG  ZLE_ORIG  XTE_ORIG  YTE_ORIG  ZTE_ORIG  XQC_ORIG  YQC_ORIG  ZQC_ORIG  S_ORIG     Area     Chord     XLE_DEF   YLE_DEF   ZLE_DEF   XTE_DEF   YTE_DEF   ZTE_DEF   XQC_DEF   YQC_DEF   ZQC_DEF    S_DEF       Cl        Cd        Cs        Cx        Cy        Cz       Cmx       Cmy       Cmz \n");

          NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
              
          for ( k = 1 ; k <= NumberOfStations ; k++ ) {

             // Calculate local deformed quarter chord location
             
             Vec[0] = VSPGeom().VSP_Surface(i).xTE_Def(k) - VSPGeom().VSP_Surface(i).xLE_Def(k);
             Vec[1] = VSPGeom().VSP_Surface(i).yTE_Def(k) - VSPGeom().VSP_Surface(i).yLE_Def(k);
             Vec[2] = VSPGeom().VSP_Surface(i).zTE_Def(k) - VSPGeom().VSP_Surface(i).zLE_Def(k);
             
             Chord = sqrt(vector_dot(Vec,Vec));
             
             Vec[0] /= Chord;
             Vec[1] /= Chord;
             Vec[2] /= Chord;

             VecQC_Def[0] = VSPGeom().VSP_Surface(i).xLE_Def(k) + 0.25*Chord*Vec[0];
             VecQC_Def[1] = VSPGeom().VSP_Surface(i).yLE_Def(k) + 0.25*Chord*Vec[1];
             VecQC_Def[2] = VSPGeom().VSP_Surface(i).zLE_Def(k) + 0.25*Chord*Vec[2];
             
             // Calculate local undeformed quarter chord location
            
             Vec[0] = VSPGeom().VSP_Surface(i).xTE(k) - VSPGeom().VSP_Surface(i).xLE(k);
             Vec[1] = VSPGeom().VSP_Surface(i).yTE(k) - VSPGeom().VSP_Surface(i).yLE(k);
             Vec[2] = VSPGeom().VSP_Surface(i).zTE(k) - VSPGeom().VSP_Surface(i).zLE(k);
             
             Chord = sqrt(vector_dot(Vec,Vec));
             
             Vec[0] /= Chord;
             Vec[1] /= Chord;
             Vec[2] /= Chord;

             VecQC[0] = VSPGeom().VSP_Surface(i).xLE(k) + 0.25*Chord*Vec[0];
             VecQC[1] = VSPGeom().VSP_Surface(i).yLE(k) + 0.25*Chord*Vec[1];
             VecQC[2] = VSPGeom().VSP_Surface(i).zLE(k) + 0.25*Chord*Vec[2];
                       
             // Transfer moments to the deformed, quarter chord location
          
             RVec[0] = XYZcg_[0] - VecQC_Def[0];
             RVec[1] = XYZcg_[1] - VecQC_Def[1];
             RVec[2] = XYZcg_[2] - VecQC_Def[2];
             
             Force[0] = Span_Cx_[i][k];
             Force[1] = Span_Cy_[i][k];
             Force[2] = Span_Cz_[i][k];
             
             vector_cross(RVec,Force,Moment);
             
             Moment[0] += Span_Cmx_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
             Moment[1] += Span_Cmy_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
             Moment[2] += Span_Cmz_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
             
             Moment[0] /= VSPGeom().VSP_Surface(i).LocalChord(k);
             Moment[1] /= VSPGeom().VSP_Surface(i).LocalChord(k);
             Moment[2] /= VSPGeom().VSP_Surface(i).LocalChord(k);
                       
             // Note... all forces and moments are referenced to the local chord and local quarter chord
             // Ie... these are '2D' coefficients

             fprintf(FEMLoadFile_,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
                     i,
                     VSPGeom().VSP_Surface(i).xLE(k),
                     VSPGeom().VSP_Surface(i).yLE(k),
                     VSPGeom().VSP_Surface(i).zLE(k),
                     VSPGeom().VSP_Surface(i).xTE(k),
                     VSPGeom().VSP_Surface(i).yTE(k),
                     VSPGeom().VSP_Surface(i).zTE(k),    
                     VecQC[0],
                     VecQC[1],
                     VecQC[2],                                      
                     VSPGeom().VSP_Surface(i).s(k),       
                     Span_Area_[i][k],
                     VSPGeom().VSP_Surface(i).LocalChord(k),              
                     VSPGeom().VSP_Surface(i).xLE_Def(k),
                     VSPGeom().VSP_Surface(i).yLE_Def(k),
                     VSPGeom().VSP_Surface(i).zLE_Def(k),
                     VSPGeom().VSP_Surface(i).xTE_Def(k),
                     VSPGeom().VSP_Surface(i).yTE_Def(k),
                     VSPGeom().VSP_Surface(i).zTE_Def(k),
                     VecQC_Def[0],
                     VecQC_Def[1],
                     VecQC_Def[2],
                     VSPGeom().VSP_Surface(i).s_Def(k),       
                     Span_Cl_[i][k],
                     Span_Cd_[i][k],
                     Span_Cs_[i][k],
                     Span_Cx_[i][k],
                     Span_Cy_[i][k],
                     Span_Cz_[i][k],
                     Moment[0],
                     Moment[1],
                     Moment[2]);
         
          }
          
          fprintf(FEMLoadFile_,"\n");
          fprintf(FEMLoadFile_,"   Planform:\n");
          fprintf(FEMLoadFile_,"\n");

          fprintf(FEMLoadFile_,"   Root LE: %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Root_LE(0),VSPGeom().VSP_Surface(i).Root_LE(1),VSPGeom().VSP_Surface(i).Root_LE(2));
          fprintf(FEMLoadFile_,"   Root TE: %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Root_TE(0),VSPGeom().VSP_Surface(i).Root_TE(1),VSPGeom().VSP_Surface(i).Root_TE(2));
          fprintf(FEMLoadFile_,"   Root QC: %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Root_QC(0),VSPGeom().VSP_Surface(i).Root_QC(1),VSPGeom().VSP_Surface(i).Root_QC(2));

          fprintf(FEMLoadFile_,"\n");

          fprintf(FEMLoadFile_,"   Tip LE:  %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Tip_LE(0),VSPGeom().VSP_Surface(i).Tip_LE(1),VSPGeom().VSP_Surface(i).Tip_LE(2));
          fprintf(FEMLoadFile_,"   Tip TE:  %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Tip_TE(0),VSPGeom().VSP_Surface(i).Tip_TE(1),VSPGeom().VSP_Surface(i).Tip_TE(2));
          fprintf(FEMLoadFile_,"   Tip QC:  %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Tip_QC(0),VSPGeom().VSP_Surface(i).Tip_QC(1),VSPGeom().VSP_Surface(i).Tip_QC(2));
                      
       }
                 
    }
    
    fprintf(FEMLoadFile_,"\n\n");
    fprintf(FEMLoadFile_,"Total Forces and Moments \n");
    fprintf(FEMLoadFile_,"\n\n");
     
    fprintf(FEMLoadFile_,"Total CL:  %lf \n", CL_[0]);
    fprintf(FEMLoadFile_,"Total CD:  %lf \n", CD_[0]);
    fprintf(FEMLoadFile_,"Total CS:  %lf \n", CS_[0]);
    
    fprintf(FEMLoadFile_,"Total CFx: %lf \n", CFx_[0]);
    fprintf(FEMLoadFile_,"Total CFy: %lf \n", CFy_[0]);
    fprintf(FEMLoadFile_,"Total CFz: %lf \n", CFz_[0]);

    fprintf(FEMLoadFile_,"Total CMx: %lf \n", CMx_[0]);
    fprintf(FEMLoadFile_,"Total CMy: %lf \n", CMy_[0]);
    fprintf(FEMLoadFile_,"Total CMz: %lf \n", CMz_[0]);

}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER CreateFEMLoadFileFromPanelSolve                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateFEMLoadFileFromPanelSolve(int Case)
{
 
    int i, j, k, Node, Node1, Node2, *OnVortexSheet;
    double *Fx, *Fy, *Fz, fx, fy, fz, Cl, Cd, Cs, CompressibilityFactor;
    double CA, SA, CB, SB;

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
        
    // Mark nodes on trailing edge of each lifting surface
    
    OnVortexSheet = new int[VSPGeom().Grid(1).NumberOfNodes() + 1];
    
    zero_int_array(OnVortexSheet, VSPGeom().Grid(1).NumberOfNodes());
    
    Fx  = new double[VSPGeom().Grid(1).NumberOfNodes() + 1];
    Fy  = new double[VSPGeom().Grid(1).NumberOfNodes() + 1];
    Fz  = new double[VSPGeom().Grid(1).NumberOfNodes() + 1];
    
    zero_double_array(Fx, VSPGeom().Grid(1).NumberOfNodes());
    zero_double_array(Fy, VSPGeom().Grid(1).NumberOfNodes());
    zero_double_array(Fz, VSPGeom().Grid(1).NumberOfNodes());
        
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          Node = VortexSheet(k).TrailingVortexEdge(i).Node();

          OnVortexSheet[Node] = k;     

       }
       
    }
   
    // Calculate forces for each lifting surface
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
             
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
     
          if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {
             
             Node1 = SurfaceVortexEdge(j).Node1();
             Node2 = SurfaceVortexEdge(j).Node2();
             
             if ( OnVortexSheet[Node1] == k && OnVortexSheet[Node2] == k ) {

                // Assume trailing edge is at CpMax_ and calculate KT factor accordingly
                
                CompressibilityFactor = 1. + 0.25*Mach_*Mach_*CpMax_;

                fx = ( -SurfaceVortexEdge(j).Trefftz_Fx() + SurfaceVortexEdge(j).Unsteady_Fx() ) * CompressibilityFactor;
                fy = ( -SurfaceVortexEdge(j).Trefftz_Fy() + SurfaceVortexEdge(j).Unsteady_Fy() ) * CompressibilityFactor;
                fz = ( -SurfaceVortexEdge(j).Trefftz_Fz() + SurfaceVortexEdge(j).Unsteady_Fz() ) * CompressibilityFactor;
                
                Fx[Node1] += 0.5*fx;
                Fy[Node1] += 0.5*fy;
                Fz[Node1] += 0.5*fz;
                
                Fx[Node2] += 0.5*fx;
                Fy[Node2] += 0.5*fy;
                Fz[Node2] += 0.5*fz;                
                
             }
             
          }
          
       }
       
    }
    
    // Write out data
    
    //                    123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 
    fprintf(FEMLoadFile_,"   Wing       X         Y         Z         Cx        Cy        Cz        CL        CD        CS \n");

    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          Node = VortexSheet(k).TrailingVortexEdge(i).Node();
          
          // Nodal forces, non-dimensionalized by Sref
          
          fx = Fx[Node] / (0.5*Sref_);
          fy = Fy[Node] / (0.5*Sref_);
          fz = Fz[Node] / (0.5*Sref_);
          
          // Nodal force coefficients, again non-dimensionalized by Sref
          
          Cl =   ( -fx * SA + fz * CA );
          Cd = ( (  fx * CA + fz * SA ) * CB - fy * SB );
          Cs = ( (  fx * CA + fz * SA ) * SB + fy * CB );

          fprintf(FEMLoadFile_,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf\n",
          k,   
          VSPGeom().Grid(1).NodeList(Node).x(),
          VSPGeom().Grid(1).NodeList(Node).y(),
          VSPGeom().Grid(1).NodeList(Node).z(),
          fx,
          fy,
          fz,
          Cl,
          Cd,
          Cs);
          
       }
       
    }
    
    fprintf(FEMLoadFile_,"\n");
    fprintf(FEMLoadFile_,"Note: Force coefficients are NOT 2D - they are the full 3D forces, non-dimensionalized by 1/2 Sref \n");

}         
          
/*##############################################################################
#                                                                              #
#                         VSP_SOLVER WriteFEM2DGeometry                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteFEM2DGeometry(void)
{

    int i, j, k, Node1, Node2, Node3, SurfaceID;
    int number_of_nodes, number_of_tris;
    char LoadFileName[2000];
    
    float Sref = Sref_;
    float Cref = Cref_;
    float Bref = Bref_;
    float X_cg = XYZcg_[0];
    float Y_cg = XYZcg_[1];
    float Z_cg = XYZcg_[2];

    float DumFloat;

    float Area;
    
    float x, y, z;

    // Open the fem load file
    
    sprintf(LoadFileName,"%s.fem2d",FileName_);
    
    if ( (FEM2DLoadFile_ = fopen(LoadFileName, "w")) == NULL ) {

       printf("Could not open the fem load file for output! \n");

       exit(1);

    }
    
    // Write out header 

    number_of_tris  = VSPGeom().Grid().NumberOfLoops();
    number_of_nodes = VSPGeom().Grid().NumberOfNodes();

    fprintf(FEM2DLoadFile_,"NumberOfNodes:%d \n",number_of_nodes);
    fprintf(FEM2DLoadFile_,"NumberOfTris: %d \n",number_of_tris);

    // Write out node data
    
    fprintf(FEM2DLoadFile_,"Nodal data: \n");
    fprintf(FEM2DLoadFile_,"X, Y, Z \n");

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfNodes() ; j++ ) {

       fprintf(FEM2DLoadFile_,"%f %f %f \n",
        VSPGeom().Grid().NodeList(j).x(),
        VSPGeom().Grid().NodeList(j).y(),
        VSPGeom().Grid().NodeList(j).z());
       
    }
        
    // Write out triangulated surface mesh
    
    fprintf(FEM2DLoadFile_,"Tri data: \n");
    fprintf(FEM2DLoadFile_,"Node1, Node2, Node3, SurfType, SurfID, Area, Nx, Ny, Nz \n");

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {

       SurfaceID   = VSPGeom().Grid().LoopList(j).DegenBodyID()
                   + VSPGeom().Grid().LoopList(j).DegenWingID()
                   + VSPGeom().Grid().LoopList(j).Cart3dID();
                 
       fprintf(FEM2DLoadFile_,"%d %d %d %d %d %f %f %f %f \n",
         VSPGeom().Grid().LoopList(j).Node1(), 
         VSPGeom().Grid().LoopList(j).Node2(), 
         VSPGeom().Grid().LoopList(j).Node3(), 
         VSPGeom().Grid().LoopList(j).SurfaceType(),
         SurfaceID, 
         VSPGeom().Grid().LoopList(j).Area(),
         VSPGeom().Grid().LoopList(j).Nx(),
         VSPGeom().Grid().LoopList(j).Ny(),
         VSPGeom().Grid().LoopList(j).Nz());

    }

}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER WriteFEM2DSolution                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteFEM2DSolution(void)
{

    int j;

    fprintf(FEM2DLoadFile_,"\n");
    
    WriteCaseHeader(FEM2DLoadFile_);
            
    // Write out solution

    fprintf(FEM2DLoadFile_,"Solution Data\n");    
    fprintf(FEM2DLoadFile_,"Tri, DeltaCp_or_Cp \n");

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {

       fprintf(FEM2DLoadFile_,"%d %f \n", j, VSPGeom().Grid().LoopList(j).dCp());

    }

}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER CalculateVelocitySurvey                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateVelocitySurvey(void)
{

    int i, j, k, p;
    double xyz[3], q[5];
    double *U, *V, *W;
    char SurveyFileName[2000];
    FILE *SurveyFile;
    
    U = new double[NumberofSurveyPoints_ + 1];
    V = new double[NumberofSurveyPoints_ + 1];
    W = new double[NumberofSurveyPoints_ + 1];

    zero_double_array(U, NumberofSurveyPoints_);
    zero_double_array(V, NumberofSurveyPoints_);
    zero_double_array(W, NumberofSurveyPoints_);

    // Initialize to free stream values

#pragma omp parallel for  
    for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {

       U[i] = FreeStreamVelocity_[0];
       V[i] = FreeStreamVelocity_[1];
       W[i] = FreeStreamVelocity_[2];

    }
    
    // Add in the rotor induced velocities
 
    for ( k = 1 ; k <= NumberOfRotors_ ; k++ ) {
    
//#pragma omp parallel for private(xyz,q)           
       for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {
    
          xyz[0] = SurveyPointList(i).x();
          xyz[1] = SurveyPointList(i).y();
          xyz[2] = SurveyPointList(i).z();
   
          RotorDisk(k).Velocity(xyz, q);                   
   
          U[i] += q[0] / Vinf_;
          V[i] += q[1] / Vinf_;
          W[i] += q[2] / Vinf_;
          
          // If ground effects... add in ground effects ... z plane
          
          if ( DoGroundEffectsAnalysis() ) {
   
             xyz[0] = SurveyPointList(i).x();
             xyz[1] = SurveyPointList(i).y();
             xyz[2] = SurveyPointList(i).z();
                
             xyz[2] *= -1.;
            
             RotorDisk(k).Velocity(xyz, q);        
   
             q[2] *= -1.;
            
             U[i] += q[0] / Vinf_;
             V[i] += q[1] / Vinf_;
             W[i] += q[2] / Vinf_;
   
          }             
          
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {
   
             xyz[0] = SurveyPointList(i).x();
             xyz[1] = SurveyPointList(i).y();
             xyz[2] = SurveyPointList(i).z();
             
             if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
             
             RotorDisk(k).Velocity(xyz, q);        
   
             if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
            
             U[i] += q[0] / Vinf_;
             V[i] += q[1] / Vinf_;
             W[i] += q[2] / Vinf_;
             
             // If ground effects... add in ground effects ... z plane
             
             if ( DoGroundEffectsAnalysis() ) {

                xyz[2] *= -1.;
               
                RotorDisk(k).Velocity(xyz, q);        
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                      q[2] *= -1.;
               
                U[i] += q[0] / Vinf_;
                V[i] += q[1] / Vinf_;
                W[i] += q[2] / Vinf_;
   
             }
               
          }            
         
       }    
       
    }

    // Wing surface vortex induced velocities

    for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {
  
       xyz[0] = SurveyPointList(i).x();
       xyz[1] = SurveyPointList(i).y();
       xyz[2] = SurveyPointList(i).z();
    
       CalculateSurfaceInducedVelocityAtPoint(xyz, q);

       U[i] += q[0];
       V[i] += q[1];
       W[i] += q[2];
       
       // If ground effects... add in ground effects ... z plane
       
       if ( DoGroundEffectsAnalysis() ) {

         xyz[0] = SurveyPointList(i).x();
         xyz[1] = SurveyPointList(i).y();
         xyz[2] = SurveyPointList(i).z();
       
         xyz[2] *= -1.;
         
         CalculateSurfaceInducedVelocityAtPoint(xyz, q);

         q[2] *= -1.;
         
         U[i] += q[0];
         V[i] += q[1];
         W[i] += q[2];
         
       }
              
       // If there is a symmetry plane, calculate influence of the reflection
       
       if ( DoSymmetryPlaneSolve_ ) {

          xyz[0] = SurveyPointList(i).x();
          xyz[1] = SurveyPointList(i).y();
          xyz[2] = SurveyPointList(i).z();
         
          if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
          if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
          if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
         
          CalculateSurfaceInducedVelocityAtPoint(xyz, q);

          if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
          if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
          if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
         
          U[i] += q[0];
          V[i] += q[1];
          W[i] += q[2];

          // If ground effects... add in ground effects ... z plane
          
          if ( DoGroundEffectsAnalysis() ) {

            xyz[2] *= -1.;
            
            CalculateSurfaceInducedVelocityAtPoint(xyz, q);
   
            if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
            if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                  q[2] *= -1.;
            
            U[i] += q[0];
            V[i] += q[1];
            W[i] += q[2];
            
          }
                
       }
       
    }

    // Wake induced velocities

    for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {

       for ( p = 1 ; p <= NumberOfVortexSheets_ ; p++ ) {
       
          for ( k = 1 ; k <= VortexSheet(p).NumberOfTrailingVortices() ; k++ ) {
   
             xyz[0] = SurveyPointList(i).x();
             xyz[1] = SurveyPointList(i).y();
             xyz[2] = SurveyPointList(i).z();
   
             VortexSheet(p).TrailingVortexEdge(k).InducedVelocity(xyz, q);
                
             U[i] += q[0];
             V[i] += q[1];
             W[i] += q[2];
               
             // If ground effects... add in ground effects ... z plane
   
             if ( DoGroundEffectsAnalysis() ) {
   
                xyz[0] = SurveyPointList(i).x();
                xyz[1] = SurveyPointList(i).y();
                xyz[2] = SurveyPointList(i).z();
                        
                xyz[2] *= -1.;
               
                VortexSheet(p).TrailingVortexEdge(k).InducedVelocity(xyz, q);
      
                q[2] *= -1.;
               
                U[i] += q[0];
                V[i] += q[1];
                W[i] += q[2];
               
             }      
                             
             // If there is a symmetry plane, calculate influence of the reflection
   
             if ( DoSymmetryPlaneSolve_ ) {
   
                xyz[0] = SurveyPointList(i).x();
                xyz[1] = SurveyPointList(i).y();
                xyz[2] = SurveyPointList(i).z();
                        
                if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1.;
               
                VortexSheet(p).TrailingVortexEdge(k).InducedVelocity(xyz, q);
      
                if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1.;
               
                U[i] += q[0];
                V[i] += q[1];
                W[i] += q[2];
                
                // If ground effects... add in ground effects ... z plane
      
                if ( DoGroundEffectsAnalysis() ) {
  
                   xyz[2] *= -1.;
                  
                   VortexSheet(p).TrailingVortexEdge(k).InducedVelocity(xyz, q);
         
                   if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1.;
                   if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1.;
                                                         q[2] *= -1.;
                  
                   U[i] += q[0];
                   V[i] += q[1];
                   W[i] += q[2];
                  
                }                      
               
             }                   
   
          }
          
       }

    }
    
    // Write out the velocity survey
    
    sprintf(SurveyFileName,"%s.svy",FileName_);
    
    if ( (SurveyFile = fopen(SurveyFileName, "w")) == NULL ) {

       printf("Could not open the survey file for output! \n");

       exit(1);

    }    
                       //0123456789x0123456789x0123456789x   0123456789x0123456789x0123456789x 
    fprintf(SurveyFile, "     X          Y          Z             U          V          W \n");

    for ( i = 1 ; i <= NumberofSurveyPoints_ ; i++ ) {

       xyz[0] = SurveyPointList(i).x();
       xyz[1] = SurveyPointList(i).y();
       xyz[2] = SurveyPointList(i).z();
       
       fprintf(SurveyFile, "%10.5f %10.5f%10.5f    %10.5f %10.5f %10.5f \n",
               SurveyPointList(i).x(),
               SurveyPointList(i).y(),
               SurveyPointList(i).z(),
               U[i],
               V[i],
               W[i]);
     
    }    
    
    fclose(SurveyFile);
    
    delete [] U;
    delete [] V;
    delete [] W;
 
}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutAerothermalDatabaseFiles                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutAerothermalDatabaseFiles(void)
{

    FILE *ADBFile_;
    char ADBFileName[2000], DumChar[2000];
    int i, j, k, Node1, Node2, Node3, SurfaceType, SurfaceID;
    int i_size, c_size, f_size, DumInt, number_of_nodes, number_of_tris;
    int Level, NumberOfCoarseEdges, NumberOfCoarseNodes, MaxLevels;
    int NumberOfKuttaTE, NumberOfKuttaNodes;
    int NumberOfControlSurfaces;
    
    int num_Mach = 1;
    int num_Alpha = 1;
    
    float Sref = Sref_;
    float Cref = Cref_;
    float Bref = Bref_;
    float X_cg = XYZcg_[0];
    float Y_cg = XYZcg_[1];
    float Z_cg = XYZcg_[2];

    float FreeStreamPressure         = 1000;
    float DynamicPressure            = 100000.;

    float DumFloat;
    
    float Cp;
    
    float Area;
    
    float x, y, z;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);
    
    sprintf(ADBFileName,"%s.adb",FileName_);
    
    if ( (ADBFile_ = fopen(ADBFileName, "wb")) == NULL ) {

       printf("Could not open the aerothermal data base file for binary output! \n");

       exit(1);

    }
    
    // Write out coded id to allow us to determine endiannes of files

    DumInt = -123789456; // Version 1 of the ADB file

    fwrite(&DumInt, i_size, 1, ADBFile_);
    
    // Write out model type... VLM or PANEL
    
    fwrite(&ModelType_, i_size, 1, ADBFile_);
    
    // Write out symmetry flag
    
    fwrite(&DoSymmetryPlaneSolve_, i_size, 1, ADBFile_);

    // Write out header to aerodynamics file

    number_of_tris  = VSPGeom().Grid().NumberOfLoops();
    number_of_nodes = VSPGeom().Grid().NumberOfNodes();

    fwrite(&number_of_nodes, i_size, 1, ADBFile_);
    fwrite(&number_of_tris,  i_size, 1, ADBFile_);
    fwrite(&num_Mach,        i_size, 1, ADBFile_);
    fwrite(&num_Alpha,       i_size, 1, ADBFile_);
    fwrite(&Sref,            f_size, 1, ADBFile_);
    fwrite(&Cref,            f_size, 1, ADBFile_);
    fwrite(&Bref,            f_size, 1, ADBFile_);
    fwrite(&X_cg,            f_size, 1, ADBFile_);
    fwrite(&Y_cg,            f_size, 1, ADBFile_);
    fwrite(&Z_cg,            f_size, 1, ADBFile_);

    DumFloat = Mach_;

    fwrite(&DumFloat, f_size, 1, ADBFile_);

    DumFloat = AngleOfAttack_;

    fwrite(&DumFloat, f_size, 1, ADBFile_);

    DumFloat = AngleOfBeta_;

    fwrite(&DumFloat, f_size, 1, ADBFile_);
    
    // Write out wing ID flags, names...

    DumInt = VSPGeom().NumberOfDegenWings();
    
    fwrite(&DumInt, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
        
          fwrite(&(i), i_size, 1, ADBFile_);
    
          sprintf(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
          
          fwrite(DumChar, c_size, 100, ADBFile_);
          
       }
     
    }
    
    // Write out body ID flags, names...
 
    DumInt = VSPGeom().NumberOfDegenBodies();
    
    fwrite(&DumInt, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
        
          fwrite(&(i), i_size, 1, ADBFile_);
    
          sprintf(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
          
          fwrite(DumChar, c_size, 100, ADBFile_);
          
       }
     
    }
    
    // Write out Cart3d ID flags, names...
 
    DumInt = VSPGeom().NumberOfCart3dSurfaces();
    
    fwrite(&DumInt, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == CART3D_SURFACE ) {
        
          fwrite(&(i), i_size, 1, ADBFile_);
    
          sprintf(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
          
          fwrite(DumChar, c_size, 100, ADBFile_);
          
       }
     
    }    
    
    // Write out triangulated surface mesh

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {

       Node1       = VSPGeom().Grid().LoopList(j).Node1();
       Node2       = VSPGeom().Grid().LoopList(j).Node2();
       Node3       = VSPGeom().Grid().LoopList(j).Node3();

       SurfaceType = VSPGeom().Grid().LoopList(j).SurfaceType();
       
       SurfaceID   = VSPGeom().Grid().LoopList(j).DegenBodyID()
                   + VSPGeom().Grid().LoopList(j).DegenWingID()
                   + VSPGeom().Grid().LoopList(j).Cart3dID();

       Area        = VSPGeom().Grid().LoopList(j).Area();
       
       fwrite(&(Node1),       i_size, 1, ADBFile_);
       fwrite(&(Node2),       i_size, 1, ADBFile_);
       fwrite(&(Node3),       i_size, 1, ADBFile_);
       fwrite(&(SurfaceType), i_size, 1, ADBFile_);
       fwrite(&(SurfaceID),   i_size, 1, ADBFile_);
       fwrite(&Area,          f_size, 1, ADBFile_);

    }

    // Write out node data

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfNodes() ; j++ ) {

       x = VSPGeom().Grid().NodeList(j).x();
       y = VSPGeom().Grid().NodeList(j).y();
       z = VSPGeom().Grid().NodeList(j).z();
       
       fwrite(&(x), f_size, 1, ADBFile_);
       fwrite(&(y), f_size, 1, ADBFile_);
       fwrite(&(z), f_size, 1, ADBFile_);
       
    }

    // Write out aero data base file
 
    fwrite(&(FreeStreamPressure), f_size, 1, ADBFile_); // Freestream static pressure
    fwrite(&(DynamicPressure),    f_size, 1, ADBFile_); // Freestream dynamic pressure
    
    // Write out min and min and max Cp
    
    DumFloat = CpMin_;
    
    fwrite(&(DumFloat), f_size, 1, ADBFile_);
    
    DumFloat = CpMax_;
    
    fwrite(&(DumFloat), f_size, 1, ADBFile_);
        
    // Loop over surfaces and write out solution

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {
  
       Cp = VSPGeom().Grid().LoopList(j).dCp();
       
 //      Cp = VSPGeom().Grid().LoopList(j).Gamma();
       
       fwrite(&Cp, f_size, 1, ADBFile_); // Wall or Edge Pressure, Pa

    }

    // Write out wake shape
    
    fwrite(&NumberOfTrailingVortexEdges_, i_size, 1, ADBFile_);
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
      
       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          VortexSheet(k).TrailingVortexEdge(i).WriteToFile(ADBFile_);

       }
       
    }     

    // Write out the rotor data
    
    fwrite(&NumberOfRotors_, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
     
       RotorDisk(i).Write_Binary_STP_Data(ADBFile_);
     
    }
    
    // Write out the edges for each grid level
        
    MaxLevels = VSPGeom().NumberOfGridLevels();

    fwrite(&MaxLevels, i_size, 1, ADBFile_); 
   
    // Loop over each level

    for ( Level = 1 ; Level < MaxLevels ; Level++ ) {

       NumberOfCoarseNodes = VSPGeom().Grid(Level).NumberOfNodes();

       NumberOfCoarseEdges = VSPGeom().Grid(Level).NumberOfEdges();
 
       fwrite(&NumberOfCoarseNodes, i_size, 1, ADBFile_); 

       fwrite(&NumberOfCoarseEdges, i_size, 1, ADBFile_); 

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

          x = VSPGeom().Grid(Level).NodeList(j).x();
          y = VSPGeom().Grid(Level).NodeList(j).y();
          z = VSPGeom().Grid(Level).NodeList(j).z();
          
          fwrite(&(x), f_size, 1, ADBFile_);
          fwrite(&(y), f_size, 1, ADBFile_);
          fwrite(&(z), f_size, 1, ADBFile_);         
     
       }
       
       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfEdges() ; j++ ) {

          SurfaceID = VSPGeom().Grid(Level).EdgeList(j).DegenBody()
                    + VSPGeom().Grid(Level).EdgeList(j).DegenWing()
                    + VSPGeom().Grid(Level).EdgeList(j).Cart3DSurface();
                    
          if ( VSPGeom().Grid(Level).EdgeList(j).Loop1() == VSPGeom().Grid(Level).EdgeList(j).Loop2() )  SurfaceID = 999;             

          Node1 = VSPGeom().Grid(Level).EdgeList(j).Node1();
          Node2 = VSPGeom().Grid(Level).EdgeList(j).Node2();  

          fwrite(&SurfaceID, i_size, 1, ADBFile_);
          
          fwrite(&Node1, i_size, 1, ADBFile_);
          fwrite(&Node2, i_size, 1, ADBFile_);

       }
  
    }
    
    // Write out kutta edges
    
    Level = 1;
    
    NumberOfKuttaTE = 0;
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
 
       if ( VSPGeom().Grid(Level).EdgeList(i).IsTrailingEdge() ) {     
          
          NumberOfKuttaTE++;
          
       }
       
    }
    
    fwrite(&NumberOfKuttaTE, i_size, 1, ADBFile_);
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
 
       if ( VSPGeom().Grid(Level).EdgeList(i).IsTrailingEdge() ) {     

          fwrite(&i, i_size, 1, ADBFile_); 
          
       }
       
    }
    
    // Write out kutta nodes
    
    NumberOfKuttaNodes = 0;
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
            
       NumberOfKuttaNodes += VortexSheet(k).NumberOfTrailingVortices();
       
    }

    fwrite(&NumberOfKuttaNodes, i_size, 1, ADBFile_);
 
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
            
       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          Node1 = VortexSheet(k).TrailingVortexEdge(i).Node();
  
          fwrite(&Node1, i_size, 1, ADBFile_); 

       }
    
    }
    
    // Write out control surfaces
    
    NumberOfControlSurfaces = 0;
    
    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          NumberOfControlSurfaces += VSPGeom().VSP_Surface(j).NumberOfControlSurfaces();
          
       }
 
    }

    fwrite(&NumberOfControlSurfaces, i_size, 1, ADBFile_); 
    
    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          for ( k = 1 ; k <= VSPGeom().VSP_Surface(j).NumberOfControlSurfaces() ; k++ ) {
       
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node_1(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node_1(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node_1(2);
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_); 
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node_2(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node_2(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node_2(2);
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_); 
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node_3(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node_3(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node_3(2);
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_); 
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node_4(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node_4(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node_4(2);
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_);                              
                          
          }
          
       }
       
    }    
        
    fclose(ADBFile_);

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutAerothermalDatabaseGeometry                #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutAerothermalDatabaseGeometry(void)
{

    char DumChar[2000];
    int i, j, k, p, r, Node1, Node2, Node3, SurfaceType, SurfaceID;
    int i_size, c_size, f_size, DumInt, number_of_nodes, number_of_tris;
    int Level, NumberOfCoarseEdges, NumberOfCoarseNodes, MaxLevels;
    int NumberOfKuttaTE, NumberOfKuttaNodes, NumberOfControlLoops;
    int NumberOfControlSurfaces, Loop;
    
    int num_Mach = 1;
    int num_Alpha = 1;
    
    float Sref = Sref_;
    float Cref = Cref_;
    float Bref = Bref_;
    float X_cg = XYZcg_[0];
    float Y_cg = XYZcg_[1];
    float Z_cg = XYZcg_[2];

    float DumFloat;

    float Area;
    
    float x, y, z;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Write out coded id to allow us to determine endiannes of files

    DumInt = -123789456; // Version 1 of the ADB file

    fwrite(&DumInt, i_size, 1, ADBFile_);
    
    // Write out model type... VLM or PANEL
    
    fwrite(&ModelType_, i_size, 1, ADBFile_);
    
    // Write out symmetry flag
    
    fwrite(&DoSymmetryPlaneSolve_, i_size, 1, ADBFile_);

    // Write out header to aerodynamics file

    number_of_tris  = VSPGeom().Grid().NumberOfLoops();
    number_of_nodes = VSPGeom().Grid().NumberOfNodes();

    fwrite(&number_of_nodes, i_size, 1, ADBFile_);
    fwrite(&number_of_tris,  i_size, 1, ADBFile_);
    fwrite(&Sref,            f_size, 1, ADBFile_);
    fwrite(&Cref,            f_size, 1, ADBFile_);
    fwrite(&Bref,            f_size, 1, ADBFile_);
    fwrite(&X_cg,            f_size, 1, ADBFile_);
    fwrite(&Y_cg,            f_size, 1, ADBFile_);
    fwrite(&Z_cg,            f_size, 1, ADBFile_);
    
    // Write out wing ID flags, names...

    DumInt = VSPGeom().NumberOfDegenWings();
    
    fwrite(&DumInt, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
        
          fwrite(&(i), i_size, 1, ADBFile_);
    
          sprintf(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
          
          fwrite(DumChar, c_size, 100, ADBFile_);
          
       }
     
    }
    
    // Write out body ID flags, names...
 
    DumInt = VSPGeom().NumberOfDegenBodies();
    
    fwrite(&DumInt, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_BODY_SURFACE ) {
        
          fwrite(&(i), i_size, 1, ADBFile_);
    
          sprintf(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
          
          fwrite(DumChar, c_size, 100, ADBFile_);
          
       }
     
    }
    
    // Write out Cart3d ID flags, names...
 
    DumInt = VSPGeom().NumberOfCart3dSurfaces();
    
    fwrite(&DumInt, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == CART3D_SURFACE ) {
        
          fwrite(&(i), i_size, 1, ADBFile_);
    
          sprintf(DumChar,"%s",VSPGeom().VSP_Surface(i).ComponentName());
          
          fwrite(DumChar, c_size, 100, ADBFile_);
          
       }
     
    }    
    
    // Write out triangulated surface mesh

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {

       Node1       = VSPGeom().Grid().LoopList(j).Node1();
       Node2       = VSPGeom().Grid().LoopList(j).Node2();
       Node3       = VSPGeom().Grid().LoopList(j).Node3();

       SurfaceType = VSPGeom().Grid().LoopList(j).SurfaceType();
       
       SurfaceID   = VSPGeom().Grid().LoopList(j).DegenBodyID()
                   + VSPGeom().Grid().LoopList(j).DegenWingID()
                   + VSPGeom().Grid().LoopList(j).Cart3dID();

       Area        = VSPGeom().Grid().LoopList(j).Area();
       
       fwrite(&(Node1),       i_size, 1, ADBFile_);
       fwrite(&(Node2),       i_size, 1, ADBFile_);
       fwrite(&(Node3),       i_size, 1, ADBFile_);
       fwrite(&(SurfaceType), i_size, 1, ADBFile_);
       fwrite(&(SurfaceID),   i_size, 1, ADBFile_);
       fwrite(&Area,          f_size, 1, ADBFile_);

    }

    // Write out node data

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfNodes() ; j++ ) {

       x = VSPGeom().Grid().NodeList(j).x();
       y = VSPGeom().Grid().NodeList(j).y();
       z = VSPGeom().Grid().NodeList(j).z();
       
       fwrite(&(x), f_size, 1, ADBFile_);
       fwrite(&(y), f_size, 1, ADBFile_);
       fwrite(&(z), f_size, 1, ADBFile_);
       
    }

    // Write out the rotor data
    
    fwrite(&NumberOfRotors_, i_size, 1, ADBFile_);

    for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
     
       RotorDisk(i).Write_Binary_STP_Data(ADBFile_);
     
    }
    
    // Write out the edges for each grid level
        
    MaxLevels = VSPGeom().NumberOfGridLevels();

    fwrite(&MaxLevels, i_size, 1, ADBFile_); 
   
    // Loop over each level

    for ( Level = 1 ; Level < MaxLevels ; Level++ ) {

       NumberOfCoarseNodes = VSPGeom().Grid(Level).NumberOfNodes();

       NumberOfCoarseEdges = VSPGeom().Grid(Level).NumberOfEdges();
 
       fwrite(&NumberOfCoarseNodes, i_size, 1, ADBFile_); 

       fwrite(&NumberOfCoarseEdges, i_size, 1, ADBFile_); 

       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

          x = VSPGeom().Grid(Level).NodeList(j).x();
          y = VSPGeom().Grid(Level).NodeList(j).y();
          z = VSPGeom().Grid(Level).NodeList(j).z();
          
          fwrite(&(x), f_size, 1, ADBFile_);
          fwrite(&(y), f_size, 1, ADBFile_);
          fwrite(&(z), f_size, 1, ADBFile_);         
     
       }
       
       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfEdges() ; j++ ) {

          SurfaceID = VSPGeom().Grid(Level).EdgeList(j).DegenBody()
                    + VSPGeom().Grid(Level).EdgeList(j).DegenWing()
                    + VSPGeom().Grid(Level).EdgeList(j).Cart3DSurface();
                    
          if ( VSPGeom().Grid(Level).EdgeList(j).Loop1() == VSPGeom().Grid(Level).EdgeList(j).Loop2() )  SurfaceID *= -1;         

          Node1 = VSPGeom().Grid(Level).EdgeList(j).Node1();
          Node2 = VSPGeom().Grid(Level).EdgeList(j).Node2();  

          fwrite(&SurfaceID, i_size, 1, ADBFile_);
          
          fwrite(&Node1, i_size, 1, ADBFile_);
          fwrite(&Node2, i_size, 1, ADBFile_);

       }
  
    }
    
    // Write out kutta edges
    
    Level = 1;
    
    NumberOfKuttaTE = 0;
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
 
       if ( VSPGeom().Grid(Level).EdgeList(i).IsTrailingEdge() ) {     
          
          NumberOfKuttaTE++;
          
       }
       
    }
    
    fwrite(&NumberOfKuttaTE, i_size, 1, ADBFile_);
    
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
 
       if ( VSPGeom().Grid(Level).EdgeList(i).IsTrailingEdge() ) {     

          fwrite(&i, i_size, 1, ADBFile_); 
          
       }
       
    }
    
    // Write out kutta nodes
    
    NumberOfKuttaNodes = 0;
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
            
       NumberOfKuttaNodes += VortexSheet(k).NumberOfTrailingVortices();
       
    }

    fwrite(&NumberOfKuttaNodes, i_size, 1, ADBFile_);
 
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
            
       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          Node1 = VortexSheet(k).TrailingVortexEdge(i).Node();
  
          fwrite(&Node1, i_size, 1, ADBFile_); 

       }
    
    }
    
    // Write out control surfaces
    
    NumberOfControlSurfaces = 0;
    
    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          NumberOfControlSurfaces += VSPGeom().VSP_Surface(j).NumberOfControlSurfaces();
          
       }
 
    }

    fwrite(&NumberOfControlSurfaces, i_size, 1, ADBFile_); 
    
    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          for ( k = 1 ; k <= VSPGeom().VSP_Surface(j).NumberOfControlSurfaces() ; k++ ) {
             
             p = VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfNodes();
             
             fwrite(&p, i_size, 1, ADBFile_);
       
             for ( p = 1 ; p <= VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfNodes() ; p++ ) {
        
                x = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node(p)[0];
                y = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node(p)[1];
                z = VSPGeom().VSP_Surface(j).ControlSurface(k).XYZ_Node(p)[2];
                
                fwrite(&x, f_size, 1, ADBFile_); 
                fwrite(&y, f_size, 1, ADBFile_); 
                fwrite(&z, f_size, 1, ADBFile_); 
             
             }
             
             // Hinge data
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_1(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_1(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_1(2);   
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_);     
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_2(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_2(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeNode_2(2);   
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_);       
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeVec(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeVec(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).HingeVec(2);   
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_);                   
             
             // Affected loops
             
             NumberOfControlLoops = 0;
             
             for ( p = 1 ; p <= VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfLoops() ; p++ ) {
                
                Loop = VSPGeom().VSP_Surface(j).ControlSurface(k).LoopList(p);
                
                NumberOfControlLoops += VSPGeom().Grid(1).LoopList(Loop).NumberOfFineGridLoops();
                
             }
             
             fwrite(&NumberOfControlLoops, i_size, 1, ADBFile_);
             
             for ( p = 1 ; p <= VSPGeom().VSP_Surface(j).ControlSurface(k).NumberOfLoops() ; p++ ) {
                
                Loop = VSPGeom().VSP_Surface(j).ControlSurface(k).LoopList(p);
                
                for ( r = 1 ; r <= VSPGeom().Grid(1).LoopList(Loop).NumberOfFineGridLoops() ; r++ ) {
                   
                   fwrite(&(VSPGeom().Grid(1).LoopList(Loop).FineGridLoop(r)), i_size, 1, ADBFile_);
                   
                }
          
             }             
                                                                 
          }
          
       }
       
    }    

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER WriteOutAerothermalDatabaseSolution                #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteOutAerothermalDatabaseSolution(void)
{

    char DumChar[2000];
    int i, j, k, Node1, Node2, Node3, SurfaceType, SurfaceID;
    int i_size, c_size, f_size, DumInt, number_of_nodes, number_of_tris;
    int Level, NumberOfCoarseEdges, NumberOfCoarseNodes, MaxLevels;
    int NumberOfKuttaTE, NumberOfKuttaNodes;
    int NumberOfControlSurfaces;

    float DumFloat;
    
    float Cp;

    // Write out case data to adb case file
    
    fprintf(ADBCaseListFile_,"%10.7f %10.7f %10.7f    %-200s \n",Mach_, AngleOfAttack_/TORAD, AngleOfBeta_/TORAD, CaseString_);
    
    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Write out Mach, Alpha, Beta

    DumFloat = Mach_;

    fwrite(&DumFloat, f_size, 1, ADBFile_);

    DumFloat = AngleOfAttack_;

    fwrite(&DumFloat, f_size, 1, ADBFile_);

    DumFloat = AngleOfBeta_;

    fwrite(&DumFloat, f_size, 1, ADBFile_);    

    // Write out min and min and max Cp
    
    DumFloat = CpMin_;
    
    fwrite(&(DumFloat), f_size, 1, ADBFile_);
    
    DumFloat = CpMax_;
    
    fwrite(&(DumFloat), f_size, 1, ADBFile_);
        
    // Loop over surfaces and write out solution

    for ( j = 1 ; j <= VSPGeom().Grid().NumberOfLoops() ; j++ ) {
  
       Cp = VSPGeom().Grid().LoopList(j).dCp();
//Cp = VSPGeom().Grid().LoopList(j).Gamma();
       fwrite(&Cp, f_size, 1, ADBFile_); // Wall or Edge Pressure, Pa

    }

    // Write out wake shape
    
    fwrite(&NumberOfTrailingVortexEdges_, i_size, 1, ADBFile_);
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
      
       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          VortexSheet(k).TrailingVortexEdge(i).WriteToFile(ADBFile_);

       }
       
    }     
    
    // Write out control surface deflection angles

    for ( j = 1 ; j <= VSPGeom().NumberOfSurfaces() ; j++ ) {
       
       if ( VSPGeom().VSP_Surface(j).SurfaceType() == DEGEN_WING_SURFACE ) {
       
          for ( k = 1 ; k <= VSPGeom().VSP_Surface(j).NumberOfControlSurfaces() ; k++ ) {

             DumFloat = VSPGeom().VSP_Surface(j).ControlSurface(k).DeflectionAngle();

             fwrite(&(DumFloat), f_size, 1, ADBFile_); 

          }
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                        VSP_SOLVER WriteRestartFile                           #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteRestartFile(void)
{
    
    int i, d_size;
    char FileNameWithExt[2000];
    FILE *RestartFile;

    d_size = sizeof(double);
    
    // Open restart file
    
    sprintf(FileNameWithExt,"%s.restart",FileName_);
    
    if ( (RestartFile = fopen(FileNameWithExt, "wb")) == NULL ) {

       printf("Could not open the restart file for output! \n");

       exit(1);

    }   
    
    // Write out the vortex strengths
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       fwrite(&(Gamma_[i]), d_size, 1, RestartFile);
       
    }    
    
    fclose(RestartFile);
    
    // Write out the wake shape
  
}

/*##############################################################################
#                                                                              #
#                        VSP_SOLVER LoadRestartFile                            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::LoadRestartFile(void)
{

    int i, d_size;
    char FileNameWithExt[2000];
    FILE *RestartFile;
    
    d_size = sizeof(double);
    
    // Open status file
    
    sprintf(FileNameWithExt,"%s.restart",FileName_);
    
    if ( (RestartFile = fopen(FileNameWithExt, "rb")) == NULL ) {

       printf("Could not open the restart file for output! \n");

       exit(1);

    }   
    
    // Write out the vortex strengths
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       fread(&(Gamma_[i]), d_size, 1, RestartFile);
       
    }    
    
    fclose(RestartFile);
    
    // Read in the wake shape
  
}

/*##############################################################################
#                                                                              #
#            VSP_SOLVER CreateSurfaceVorticesInteractionList                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateSurfaceVorticesInteractionList(void)
{
 
    int i, j, k, NumberOfEdges, TotalHits, Included, Edge, DoCheck;
    double xyz[3], SpeedRatio;
   
    // Allocate space for final interaction lists
    
    NumberOfVortexEdgesForInteractionListEntry_ = new int[NumberOfVortexLoops_ + 1];

    SurfaceVortexEdgeInteractionList_ = new VSP_EDGE**[NumberOfVortexLoops_ + 1];

    DoCheck = 0;
    
    TotalHits = 0;
    
    SpeedRatio = 0.;
    
    printf("Creating interaction lists... \n\n");fflush(NULL);

    if ( 0&&Mach_ > 1. ) {
       
       NumberOfVortexEdgesForInteractionListEntry_[1] = NumberOfSurfaceVortexEdges_;  
       
       SurfaceVortexEdgeInteractionList_[1] = new VSP_EDGE*[NumberOfSurfaceVortexEdges_ + 1]; 
       
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
          
          SurfaceVortexEdgeInteractionList_[1][j] = SurfaceVortexEdge_[j];
          
       }
       
       for ( k = 2 ; k <= NumberOfVortexLoops_ ; k++ ) {
       
          NumberOfVortexEdgesForInteractionListEntry_[k] = NumberOfSurfaceVortexEdges_;
           
          SurfaceVortexEdgeInteractionList_[k] = SurfaceVortexEdgeInteractionList_[1];
          
       }
       
    }
    
    else {
    
       for ( k = 1 ; k <= NumberOfVortexLoops_ ; k++ ) {
        
          if ( (k/1000)*1000 == k ) printf("%d / %d \r",k,NumberOfVortexLoops_);fflush(NULL);
   
          xyz[0] = VortexLoop(k).Xc();
          xyz[1] = VortexLoop(k).Yc();
          xyz[2] = VortexLoop(k).Zc();
  
          SurfaceVortexEdgeInteractionList_[k] = CreateInteractionList(VortexLoop(k).ComponentID(),xyz, NumberOfEdges);
                                
          NumberOfVortexEdgesForInteractionListEntry_[k] = NumberOfEdges;    
          
          TotalHits += NumberOfEdges;
          
          SpeedRatio += ((double)NumberOfEdges);
          
          // Check if list includes vortex loop k
   
          if ( DoCheck ) {
           
             Included = 0;
             
             for ( j = 1 ; j <= NumberOfEdges ; j++ ) {
              
                for ( i = 1 ; i <= VortexLoop(k).NumberOfEdges() ; i++ ) {
                            
                   Edge = VortexLoop(k).Edge(i);
                   
                   if ( SurfaceVortexEdgeInteractionList_[k][j] == SurfaceVortexEdge_[Edge] ) Included++;
                   
                }
   
             }
             
             if ( Included != VortexLoop(k).NumberOfEdges() ) printf("\n\n\nIncluded was: %d out of: %d \n",Included,VortexLoop(k).NumberOfEdges());
             
          }
          
       }
       
       SpeedRatio = NumberOfVortexLoops_ * NumberOfSurfaceVortexEdges_ /SpeedRatio;

       if ( Verbose_ ) printf("\nSpeed Up Ratio: %lf \n\n\n",SpeedRatio);fflush(NULL);
       
    }

}

/*##############################################################################
#                                                                              #
#            VSP_SOLVER CalculateSurfaceInducedVelocityAtPoint                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateSurfaceInducedVelocityAtPoint(double xyz[3], double q[3])
{
 
    int j, NumberOfEdges;
    double U, V, W, dq[3];
    VSP_EDGE **InteractionList, *VortexEdge;
     
    // Create interaction list for this xyz location

    InteractionList = CreateInteractionList(0,xyz, NumberOfEdges);

    U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(VortexEdge,dq)
    for ( j = 1 ; j <= NumberOfEdges ; j++ ) {
     
       VortexEdge = InteractionList[j];
    
       if ( !VortexEdge->IsTrailingEdge() ) {              

          VortexEdge->InducedVelocity(xyz, dq);
      
          U += dq[0];
          V += dq[1];
          W += dq[2];
          
       }
         
    }
    
    q[0] = U;
    q[1] = V;
    q[2] = W;    
    
    delete [] InteractionList;
 
}

/*##############################################################################
#                                                                              #
#            VSP_SOLVER CalculateWingSurfaceInducedVelocityAtPoint             #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateWingSurfaceInducedVelocityAtPoint(double xyz[3], double q[3])
{
 
    int j, NumberOfEdges;
    double U, V, W, dq[3];
    VSP_EDGE **InteractionList, *VortexEdge;
     
    // Create interaction list for this xyz location

    InteractionList = CreateInteractionList(0,xyz, NumberOfEdges);

    U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(VortexEdge,dq)
    for ( j = 1 ; j <= NumberOfEdges ; j++ ) {
     
       VortexEdge = InteractionList[j];
    
       if ( !VortexEdge->IsTrailingEdge() && VortexEdge->DegenWing() ) {              

          VortexEdge->InducedVelocity(xyz, dq);
      
          U += dq[0];
          V += dq[1];
          W += dq[2];
          
       }
         
    }
    
    q[0] = U;
    q[1] = V;
    q[2] = W;    
    
    delete [] InteractionList;
 
}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CreateInteractionList                          #
#                                                                              #
##############################################################################*/

VSP_EDGE **VSP_SOLVER::CreateInteractionList(int ComponentID, double xyz[3], int &NumberOfInteractionEdges)
{

    int i, j, Level, Loop;
    int Level_1, Level_2, Used, i_1, i_2;
    int StackSize, MoveDownLevel, Next, Found, AddEdges;
    double Distance, FarAway, Mu, Test, NormalDistance, Vec[3], Tolerance, Ratio;
    VSP_EDGE **InteractionEdgeList;

    // Allocate space if this is the first time through
    
    if ( FirstTimeSetup_ ) {
  
       // Allocate space for temporary interaction lists
       
       EdgeIsUsed_ = new int*[VSPGeom().NumberOfGridLevels() + 1];
      
       for ( Level = VSPGeom().NumberOfGridLevels() - 1 ; Level >= 1  ; Level-- ) {
        
          EdgeIsUsed_[Level] = new int[VSPGeom().Grid(Level).NumberOfEdges() + 1];
        
          zero_int_array(EdgeIsUsed_[Level], VSPGeom().Grid(Level).NumberOfEdges());
          
       }
                    
       MaxStackSize_ = 0;

       // Loop over all grid levels for this surface
       
       for ( Level = VSPGeom().NumberOfGridLevels() - 1 ; Level >= 1  ; Level-- ) {

          // Loop over all vortex loops
          
          MaxStackSize_ += VSPGeom().Grid(Level).NumberOfLoops();
         
       }

       LoopStackList_ = new STACK_ENTRY[MaxStackSize_ + 1];
       
       // Create Matrix preconditioner
       
       if ( Preconditioner_ == MATCON ) CreateMatrixPreconditionersDataStructure();

       FirstTimeSetup_ = 0;
       
    }

    // Define faraway criteria... how far away we need to be from a loop to treat it as faraway
    // Ratio of distance to maximum loop size

    FarAway = 5.;

    // Insert loops on coarsest level into stack
    
    Level = VSPGeom().NumberOfGridLevels() - 1;
 
    StackSize = 0;

    for ( Loop = 1 ; Loop <= VSPGeom().Grid(Level).NumberOfLoops() ; Loop++ ) {
     
       StackSize++;
       
       LoopStackList_[StackSize].Level = Level;
       LoopStackList_[StackSize].Loop  = Loop;

    }
      
    // Update the search ID value... reset things after we have done all the loops
    
    SearchID_++;
    
    if ( SearchID_ > NumberOfVortexLoops_ ) {
     
       for ( Level = 1 ; Level < VSPGeom().NumberOfGridLevels() ; Level++ ) {
      
          zero_int_array(EdgeIsUsed_[Level], VSPGeom().Grid(Level).NumberOfEdges()); 
          
       }
       
       SearchID_ = 1;
       
    }

    // Now loop over stack and begin AGMP process

    Next = 1;
        
    while ( Next <= StackSize ) {
     
       Level = LoopStackList_[Next].Level;
       Loop  = LoopStackList_[Next].Loop;

       // If we are far enough away from this loop, add it's edges to the interaction list
             
       MoveDownLevel = 0;

       Distance = sqrt( pow(xyz[0] - VSPGeom().Grid(Level).LoopList(Loop).Xc(),2.)
                      + pow(xyz[1] - VSPGeom().Grid(Level).LoopList(Loop).Yc(),2.)
                      + pow(xyz[2] - VSPGeom().Grid(Level).LoopList(Loop).Zc(),2.) );

       Test = FarAway * ( VSPGeom().Grid(Level).LoopList(Loop).Length() + VSPGeom().Grid(Level).LoopList(Loop).CentroidOffSet() );
      
       if ( Level == 1 || ( Test <= Distance && !inside_box(VSPGeom().Grid(Level).LoopList(Loop).BoundBox(), xyz) ) ) {
      
          // Check for nearly planar, and close, panels on different surfaces

          Ratio = Distance / ( VSPGeom().Grid(Level).LoopList(Loop).Length() + VSPGeom().Grid(Level).LoopList(Loop).CentroidOffSet() );

          AddEdges = 1;
        
          if ( ComponentID > 0 && ComponentID != VSPGeom().Grid(Level).LoopList(Loop).ComponentID() && Level == 1 && Ratio <= 2. ) {
             
             // Calculate normal distance
             
             Vec[0] = xyz[0] - VSPGeom().Grid(Level).LoopList(Loop).Xc();
             Vec[1] = xyz[1] - VSPGeom().Grid(Level).LoopList(Loop).Yc();
             Vec[2] = xyz[2] - VSPGeom().Grid(Level).LoopList(Loop).Zc();
             
             NormalDistance = ABS(vector_dot(Vec,VSPGeom().Grid(Level).LoopList(Loop).Normal()));
             
             // Tolerance
             
             Tolerance = sqrt(VSPGeom().Grid(Level).LoopList(Loop).Area());
      
             if ( ABS(NormalDistance) <= 0.25*Tolerance ) AddEdges = 0;

          }
     
          // Add these edges to the list
          
          if ( AddEdges ) {
 
             for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfEdges() ; i++ ) {
       
                j = VSPGeom().Grid(Level).LoopList(Loop).Edge(i);
                
                EdgeIsUsed_[Level][j] = SearchID_;
                
             }
             
          }
          
       }
       
       // If xyz too close to this loop, move down a level
       
       else {
        
          MoveDownLevel = 1;
          
       }

       if ( MoveDownLevel ) {
        
          if ( Level > 1 ) {
         
             for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
               
                StackSize++;
                
                if ( StackSize > MaxStackSize_ ) {
                   
                  printf("stack size must be resized! \n");fflush(NULL);
                  exit(1);
                    
                }
                  
                LoopStackList_[StackSize].Level = Level - 1;
                LoopStackList_[StackSize].Loop  = VSPGeom().Grid(Level).LoopList(Loop).FineGridLoop(i);
     
             }   
             
          }

       }
       
       // Move onto next entry in the stack

       Next++;
       
    }

    // Pack all unique edges... move up from coarse to fine grids

    NumberOfInteractionEdges = 0;
    
    for ( Level = VSPGeom().NumberOfGridLevels() - 1 ; Level >= 1  ; Level-- ) {
     
       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
        
          // This edge was marked as being used
          
          if ( EdgeIsUsed_[Level][i] == SearchID_ ) {
          
             // Check that the edge is not already used on a coarser grid
             
             if ( Level + 1 < VSPGeom().NumberOfGridLevels() ) {
              
                i_1 = i ; Level_1 = Level;
              
                Level_2 = Level + 1;
                
                Used = 0;
                
                // Loop down grid levels to check if it's used on a coarser grid
                
                while ( Level_2 < VSPGeom().NumberOfGridLevels() && !Used ) {
              
                  i_2 = VSPGeom().Grid(Level_1).EdgeList(i_1).CourseGridEdge();

                  if ( i_2 > 0 && EdgeIsUsed_[Level_2][i_2] == SearchID_ ) {
                   
                    Used = 1;
                    
                  }
                  
                  Level_1 = Level_2;
                  
                  Level_2 = Level_1 + 1;
                  
                  i_1 = i_2;
                  
                }
           
                // If not used, then add it to the list
                
                if ( !Used ) {
                 
                   NumberOfInteractionEdges++;
                   
                }
                
                // Edge was used on a coarser grid, so remove it from the list
                
                else {
                 
                   EdgeIsUsed_[Level][i] = 0;
                   
                }
           
             }
             
             // This is the coarset grid, so we definitely use this edge
             
             else {
              
                NumberOfInteractionEdges++;
                
             }
                           
          }
          
       }

    }

    // Force any trailing edges to be evaluated at the finest grid level
     
    for ( Level = VSPGeom().NumberOfGridLevels() - 1 ; Level >= 1  ; Level-- ) {
     
       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
        
          // This edge was marked as being used
          
          if ( EdgeIsUsed_[Level][i] == SearchID_ ) {
           
             // If this edge is on trailing edge, force it to be evaluated on the finest grid
             
             if ( VSPGeom().Grid(Level).EdgeList(i).IsTrailingEdge() ) {
              
                if ( Level != 1  ) {

                   // Determine the fine grid edge this coarse edge comes from
                   
                   i_1 = i ; Level_1 = Level;
                       
                   Found = 0;
                   
                   // Loop down grid levels to check if it's used on a coarser grid
       
                   while ( Level_1 >= 2 && !Found ) {
                    
                     i_2 = VSPGeom().Grid(Level_1).EdgeList(i_1).FineGridEdge();
                         
                     if ( Level_1 == 2 ) Found = i_2;

                     Level_1--;
                     
                     i_1 = i_2;
                     
                   }
                  
                   // Zero out this coarse grid edge as being used
                   
                   EdgeIsUsed_[Level][    i] = 0;
                   
                   // Replace with the fine grid version
                         
                   EdgeIsUsed_[    1][Found] = SearchID_;
                   
                }
                
             }
             
          }
          
       }
       
    }

    InteractionEdgeList = new VSP_EDGE*[NumberOfInteractionEdges + 1];
    
    NumberOfInteractionEdges = 0;
     
    // Create the final interaction list
    
    for ( Level = VSPGeom().NumberOfGridLevels() - 1 ; Level >= 1  ; Level-- ) {
        
       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
        
          if ( EdgeIsUsed_[Level][i] == SearchID_ ) {
           
             NumberOfInteractionEdges++;
           
             InteractionEdgeList[NumberOfInteractionEdges] = &(VSPGeom().Grid(Level).EdgeList(i));
   
          }
          
       }
     
    }      

    return InteractionEdgeList;
    
}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CalculateMPVelocity                            #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateMPVelocity(void)
{

    int i, j, Level;
    double q[4], U, V, W;
    VSP_EDGE *VortexEdge;
    
    for ( Level = 1 ; Level < NumberOfMGLevels_ ; Level++ ) {
        
       RestrictSolutionFromGrid(Level);
       
       UpdateVortexEdgeStrengths(Level+1, 0);
   
    }
    
    i = 111;
    
    U = V = W = 0.;

double xyz[3];
xyz[0] = 10.;
xyz[1] =  1.;
xyz[2] = 10.;

    printf("NumberOfVortexEdgesForInteractionListEntry_[i]: %d \n",NumberOfVortexEdgesForInteractionListEntry_[i]);
    
    for ( j = 1 ; j <= NumberOfVortexEdgesForInteractionListEntry_[i] ; j++ ) {
     
       VortexEdge = SurfaceVortexEdgeInteractionList_[i][j];
    
       if ( !VortexEdge->IsTrailingEdge() ) {              

          VortexEdge->InducedVelocity(xyz, q);
      
          U += q[0];
          V += q[1];
          W += q[2];
          
       }
         
    }
    
    printf("New U,V,W: %lf %lf %lf \n",U,V,W);
 
}

/*##############################################################################
#                                                                              #
#                   VSP_SOLVER ProlongateSolutionFromGrid                      #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ProlongateSolutionFromGrid(int Level)
{
 
    int i_c, i_f, g_c, g_f;
    double Fact;
   
    g_c = Level;
    g_f = Level - 1;
    
    // Subtract out initial restricted solution from coarse the coarse grid
    // solution - this ends up with the correction being stored.

    for ( i_f = 1 ; i_f <= VSPGeom().Grid(g_f).NumberOfLoops() ; i_f++ ) {

       i_c = VSPGeom().Grid(g_f).LoopList(i_f).CoarseGridLoop();
       
       Fact = VSPGeom().Grid(g_f).LoopList(i_f).Area() 
            / VSPGeom().Grid(g_c).LoopList(i_c).Area();
  
       VSPGeom().Grid(g_c).LoopList(i_c).Gamma() -= Fact * VSPGeom().Grid(g_f).LoopList(i_f).Gamma();
       
    }

    // Prolongate correction from coarse to fine grid - direct injection

    for ( i_f = 1 ; i_f <= VSPGeom().Grid(g_f).NumberOfLoops() ; i_f++ ) {

       i_c = VSPGeom().Grid(g_f).LoopList(i_f).CoarseGridLoop();
  
       VSPGeom().Grid(g_f).LoopList(i_f).Gamma() += VSPGeom().Grid(g_c).LoopList(i_c).Gamma();
       
    }

}

/*##############################################################################
#                                                                              #
#                   VSP_SOLVER RestrictSolutionFromGrid                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::RestrictSolutionFromGrid(int Level)
{
 
    int i_c, i_f, g_c, g_f;
    double Fact;

    // Restrict solution from Level i, to level i+1
    
    g_f = Level;
    g_c = Level + 1;
    
    // Zero out stuff on the coarsest grid
    
#pragma omp parallel for      
    for ( i_c = 1 ; i_c <= VSPGeom().Grid(g_c).NumberOfLoops() ; i_c++ ) {

       VSPGeom().Grid(g_c).LoopList(i_c).Gamma() = 0.;

    }
  
    // Restrict the solution to the coarse grid
 
    for ( i_f = 1 ; i_f <= VSPGeom().Grid(g_f).NumberOfLoops() ; i_f++ ) {

       i_c = VSPGeom().Grid(g_f).LoopList(i_f).CoarseGridLoop();
       
       Fact = VSPGeom().Grid(g_f).LoopList(i_f).Area()
            / VSPGeom().Grid(g_c).LoopList(i_c).Area();

       VSPGeom().Grid(g_c).LoopList(i_c).Gamma() += Fact*VSPGeom().Grid(g_f).LoopList(i_f).Gamma();
  
    }

}


/*##############################################################################
#                                                                              #
#                       VSP_SOLVER InterpolateSolutionFromGrid                 #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::InterpolateSolutionFromGrid(int Level)
{
 
    int i, j, i_c, i_f, g_c, g_f, Edge, Iter, Node, Node1, Node2;
    int *FixedNode,  NodeHits, Loop1, Loop2;
    double Fact, *dCp, *Denom, *Res, *Dif, *Sum, Delta, Eps, Ds;
    double Wgt1, Wgt2, CpAvg;
    
    g_c = Level;
    g_f = Level - 1;
    
    // Smooth pressure
    
    dCp = new double[VSPGeom().Grid(g_c).NumberOfNodes() + 1];
    Denom = new double[VSPGeom().Grid(g_c).NumberOfNodes() + 1];
    
    zero_double_array(dCp, VSPGeom().Grid(g_c).NumberOfNodes());
    zero_double_array(Denom, VSPGeom().Grid(g_c).NumberOfNodes());
    
    // Create nodal value of CP

    for ( i_c = 1 ; i_c <= VSPGeom().Grid(g_c).NumberOfLoops() ; i_c++ ) {
     
       for ( j = 1 ; j <= VSPGeom().Grid(g_c).LoopList(i_c).NumberOfEdges() ; j++ ) {
        
          Edge = VSPGeom().Grid(g_c).LoopList(i_c).Edge(j);
          
          Node1 = VSPGeom().Grid(g_c).EdgeList(Edge).Node1();
          Node2 = VSPGeom().Grid(g_c).EdgeList(Edge).Node2();
          
          dCp[Node1] += VSPGeom().Grid(g_c).LoopList(i_c).dCp() * VSPGeom().Grid(g_c).LoopList(i_c).Area();
          dCp[Node2] += VSPGeom().Grid(g_c).LoopList(i_c).dCp() * VSPGeom().Grid(g_c).LoopList(i_c).Area();
                               
          Denom[Node1] += VSPGeom().Grid(g_c).LoopList(i_c).Area();
          Denom[Node2] += VSPGeom().Grid(g_c).LoopList(i_c).Area();
          
       }
       
    }

    for ( j = 1 ; j <= VSPGeom().Grid(g_c).NumberOfNodes() ; j++ ) {
       
       dCp[j] /= Denom[j];
       
    }

    // Enforce kutta condition on trailing edge
  
    FixedNode = new int[VSPGeom().Grid(g_c).NumberOfNodes() + 1];
    
    zero_int_array(FixedNode, VSPGeom().Grid(g_c).NumberOfNodes());

    if ( ModelType_ == VLM_MODEL ) {
       
       for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfKuttaNodes() ; i++ ) {
   
          Node = VSPGeom().Grid(g_c).KuttaNode(i);
   
          dCp[Node] = 0.;
          
          FixedNode[Node] = 1.;
   
       }
       
    }
    
    // Panel Model
    
    else if ( ModelType_ == PANEL_MODEL ) {
       
       for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfKuttaNodes() ; i++ ) {
   
          Node = VSPGeom().Grid(g_c).KuttaNode(i);
   
          dCp[Node] = CpMax_;
          
          FixedNode[Node] = 1.;
   
       }
       
       for ( i_c = 1 ; i_c <= VSPGeom().Grid(g_c).NumberOfLoops() ; i_c++ ) {
        
          for ( j = 1 ; j <= VSPGeom().Grid(g_c).LoopList(i_c).NumberOfEdges() ; j++ ) {
           
             Edge = VSPGeom().Grid(g_c).LoopList(i_c).Edge(j);
             
             Node1 = VSPGeom().Grid(g_c).EdgeList(Edge).Node1();
             Node2 = VSPGeom().Grid(g_c).EdgeList(Edge).Node2();
             
             if ( LoopIsOnBaseRegion_[Node1] ) {
                
                FixedNode[Node1] = 1.;
                
                dCp[Node1] = CpBase_;
                
             }
       
             if ( LoopIsOnBaseRegion_[Node2] ) {
                
                FixedNode[Node2] = 1.;
                
                dCp[Node2] = CpBase_;
                
             }
             
          }
          
       }       
    }

    Res = new double[VSPGeom().Grid(g_c).NumberOfNodes() + 1];

    Dif = new double[VSPGeom().Grid(g_c).NumberOfNodes() + 1];

    Sum = new double[VSPGeom().Grid(g_c).NumberOfNodes() + 1];

    zero_double_array(Sum,VSPGeom().Grid(g_c).NumberOfNodes());

    // Count edge hits per node

    for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfEdges() ; i++ ) {

       // Edge to node pointers

       Node1 = VSPGeom().Grid(g_c).EdgeList(i).Node1();
       Node2 = VSPGeom().Grid(g_c).EdgeList(i).Node2();

       Ds = sqrt( pow(VSPGeom().Grid(g_c).NodeList(Node2).x() - VSPGeom().Grid(g_c).NodeList(Node1).x(),2.)
                + pow(VSPGeom().Grid(g_c).NodeList(Node2).y() - VSPGeom().Grid(g_c).NodeList(Node1).y(),2.)
                + pow(VSPGeom().Grid(g_c).NodeList(Node2).z() - VSPGeom().Grid(g_c).NodeList(Node1).z(),2.) );

       Sum[Node1] += 1./(Ds);
       Sum[Node2] += 1./(Ds);
      
    }

    // Loop over and smooth all residuals

    for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfNodes() ; i++ ) {

       Res[i] = dCp[i];

       Dif[i] = 0.;

    }

    // Do a few iterations of smoothing

    for ( Iter = 1 ; Iter <= 250 ; Iter++ ) {

       // Loop over the edges and scatter fluxes

       for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfEdges() ; i++ ) {

           // Edge to node pointers

           Node1 = VSPGeom().Grid(g_c).EdgeList(i).Node1();
           Node2 = VSPGeom().Grid(g_c).EdgeList(i).Node2();
     
           Ds = sqrt( pow(VSPGeom().Grid(g_c).NodeList(Node2).x() - VSPGeom().Grid(g_c).NodeList(Node1).x(),2.)
                    + pow(VSPGeom().Grid(g_c).NodeList(Node2).y() - VSPGeom().Grid(g_c).NodeList(Node1).y(),2.)
                    + pow(VSPGeom().Grid(g_c).NodeList(Node2).z() - VSPGeom().Grid(g_c).NodeList(Node1).z(),2.) );
 
           Delta = ( Res[Node1] - Res[Node2] )/(Ds);

           Dif[Node1] -= Delta;
           Dif[Node2] += Delta;

       }

       Eps = 0.001;

       for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfNodes() ; i++ ) {

          if ( !FixedNode[i] ) {

             Fact = Eps*Sum[i];

             Res[i] = ( dCp[i] + Fact*Res[i] + Eps*Dif[i] )/( 1. + Fact );

          }

          Dif[i] = 0.;

       }

    }

    // Update nodal values with smoothed result
    
    for ( i = 1 ; i <= VSPGeom().Grid(g_c).NumberOfNodes() ; i++ ) {

       dCp[i] = Res[i];

    }           
 
    // Finally, update loop values

    for ( i_c = 1 ; i_c <= VSPGeom().Grid(g_c).NumberOfLoops() ; i_c++ ) {
       
       NodeHits = 0;
       
       CpAvg = 0.;

       for ( j = 1 ; j <= VSPGeom().Grid(g_c).LoopList(i_c).NumberOfEdges() ; j++ ) {
    
          Edge = VSPGeom().Grid(g_c).LoopList(i_c).Edge(j);
          
          Node1 = VSPGeom().Grid(g_c).EdgeList(Edge).Node1();
          Node2 = VSPGeom().Grid(g_c).EdgeList(Edge).Node2();
          
          Wgt1 = Wgt2 = 0.;
          
          if ( ModelType_ == VLM_MODEL || Sum[Node1] > 0. ) { Wgt1 = 1.; NodeHits++; }
          if ( ModelType_ == VLM_MODEL || Sum[Node2] > 0. ) { Wgt2 = 1.; NodeHits++; }
          
          CpAvg += Wgt1*dCp[Node1] + Wgt2*dCp[Node2];
          
       }
       
       if ( NodeHits > 0 ) VSPGeom().Grid(g_c).LoopList(i_c).dCp() = CpAvg / NodeHits;
       
    }
    
    // Enforce base pressures and TE stagnation points
    
    if ( ModelType_ == PANEL_MODEL ) {
       
       // Base
       
       for ( i_c = 1 ; i_c <= NumberOfVortexLoops_ ; i_c++ ) {
          
          if ( LoopIsOnBaseRegion_[i_c] ) VSPGeom().Grid(g_c).LoopList(i_c).dCp() = CpBase_;
          
       }            
    
    }
    
    // Prolongate solution from coarse to fine grid

#pragma omp parallel for private(i_c)
    for ( i_f = 1 ; i_f <= VSPGeom().Grid(g_f).NumberOfLoops() ; i_f++ ) {

       i_c = VSPGeom().Grid(g_f).LoopList(i_f).CoarseGridLoop();

       VSPGeom().Grid(g_f).LoopList(i_f).dCp() = VSPGeom().Grid(g_c).LoopList(i_c).dCp();

       VSPGeom().Grid(g_f).LoopList(i_f).Gamma() = VSPGeom().Grid(g_c).LoopList(i_c).Gamma();

    }  
    
    delete [] dCp;
    delete [] Denom;

}

/*##############################################################################
#                                                                              #
#                       VSP_SOLVER UpdateVortexEdgeStrengths                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::UpdateVortexEdgeStrengths(int Level, int UpdateType)
{

    int i, j, k, Node1, Node2;

    // Copy current value over to vortex loops
    
    if ( Level == 1 ) {
     
#pragma omp parallel for    
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
        
          VortexLoop(i).Gamma() = Gamma_[i];

       }
  
    }
    
    // Calculate delta-gammas for each surface vortex edge
    
#pragma omp parallel for       
    for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {

       VSPGeom().Grid(Level).EdgeList(i).Gamma() = VSPGeom().Grid(Level).LoopList(VSPGeom().Grid(Level).EdgeList(i).VortexLoop1()).Gamma()
                                                 - VSPGeom().Grid(Level).LoopList(VSPGeom().Grid(Level).EdgeList(i).VortexLoop2()).Gamma();
                  
    }    

    // Calculate node wise delta gammas on finest grid

    if ( Level == 1 ) {

#pragma omp parallel for
       for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {

          VSPGeom().Grid(Level).NodeList(j).dGamma() = 0.;
          
       }

#pragma omp parallel for private(Node1,Node2)        
       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
    
          if ( VSPGeom().Grid(Level).EdgeList(i).IsTrailingEdge() ) {     

             Node1 = VSPGeom().Grid(Level).EdgeList(i).Node1();
             Node2 = VSPGeom().Grid(Level).EdgeList(i).Node2();
                          
             VSPGeom().Grid(Level).NodeList(Node1).dGamma() += VSPGeom().Grid(Level).EdgeList(i).Gamma();
             VSPGeom().Grid(Level).NodeList(Node2).dGamma() -= VSPGeom().Grid(Level).EdgeList(i).Gamma();
        
          }
          
       }

       // Calculate delta-gammas for each trailing vortex edge
       
       for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

          for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {
   
             Node1 = VortexSheet(k).TrailingVortexEdge(i).Node();
   
             VortexSheet(k).TrailingVortexEdge(i).Gamma() = VSPGeom().Grid(Level).NodeList(Node1).dGamma();          

          }
          
          if ( TimeAccurate_ ) VortexSheet(k).MaxConvectedDistance() = CurrentTime_;
       
          VortexSheet(k).UpdateVortexStrengths(UpdateType);
          
       }

    }
    

    
}

/*##############################################################################
#                                                                              #
#                          VSP_SOLVER ZeroVortexState                          #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::ZeroVortexState(void)
{

    int i, j, k, Level;

    // Copy current value over to vortex loops
    
    for ( Level = 1 ; Level <= NumberOfMGLevels_ ; Level++ ) {
       
       if ( Level == 1 ) {
   
          for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
           
             VortexLoop(i).Gamma() = 0.;
   
          }
     
       }
       
       // Calculate delta-gammas for each surface vortex edge
       
       for ( i = 1 ; i <= VSPGeom().Grid(Level).NumberOfEdges() ; i++ ) {
   
          VSPGeom().Grid(Level).EdgeList(i).Gamma() = 0.;
                     
       }    
   
       // Calculate node wise delta gammas on finest grid
   
       if ( Level == 1 ) {
   
          for ( j = 1 ; j <= VSPGeom().Grid(Level).NumberOfNodes() ; j++ ) {
   
             VSPGeom().Grid(Level).NodeList(j).dGamma() = 0.;
             
          }
 
          // Calculate delta-gammas for each trailing vortex edge
          
          for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
   
             for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {
      
                VortexSheet(k).TrailingVortexEdge(i).Gamma() = 0.;
   
             }

             VortexSheet(k).UpdateVortexStrengths(IMPLICIT_WAKE_GAMMAS);
             VortexSheet(k).UpdateVortexStrengths(EXPLICIT_WAKE_GAMMAS);
             VortexSheet(k).UpdateVortexStrengths(ALL_WAKE_GAMMAS);
             
          }
     
       }
       
    }
    
}

/*##############################################################################
#                                                                              #
#                      VSP_SOLVER OutputStatusFile                             #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::OutputStatusFile(int Type)
{

    int i;
    double E, AR, ToQS;
    
    AR = Bref_ * Bref_ / Sref_;

    ToQS = 0.;
    
    for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
     
       ToQS += RotorDisk(i).RotorThrust() / ( 0.5 * Density_ * Vinf_ * Vinf_ * Sref_);
       
    }    
 
    E = (CL(Type) *CL(Type) /(PI * AR)) / CD(Type) ;
 
    i = CurrentWakeIteration_;
    
    if ( Type == 1 ) {
       
       i = 99999;
       
       fprintf(StatusFile_,"\n\n\n");
       
    }
    
    if ( !TimeAccurate_ ) {
       
       fprintf(StatusFile_,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf\n",
               i,
               Mach_,
               AngleOfAttack_/TORAD,
               AngleOfBeta_/TORAD,
               CL(Type),
               CDo(),
               CD(Type),
               CDo() + CD(Type),
               CS(Type),            
               CL(Type)/(CDo() + CD(Type)),
               E,
               CFx(Type),
               CFy(Type),
               CFz(Type),
               CMx(Type),
               CMy(Type),
               CMz(Type),
               ToQS);
               
    }
    
    else {
       
        CL_Unsteady_[Time_] = CL(Type);
        CD_Unsteady_[Time_] = CD(Type);
        CS_Unsteady_[Time_] = CS(Type);
       CFx_Unsteady_[Time_] = CFx(Type);
       CFy_Unsteady_[Time_] = CFy(Type);
       CFz_Unsteady_[Time_] = CFz(Type);
       CMx_Unsteady_[Time_] = CMx(Type);
       CMy_Unsteady_[Time_] = CMy(Type);
       CMz_Unsteady_[Time_] = CMz(Type);

       if ( TimeAnalysisType_ == HEAVE_ANALYSIS ) {
          
          fprintf(StatusFile_,"%9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf\n",
                  CurrentTime_,
                  Mach_,
                  AngleOfAttack_/TORAD,
                  AngleOfBeta_/TORAD,
                  CL(Type),
                  CDo(),
                  CD(Type),
                  CDo() + CD(Type),
                  CS(Type),            
                  CL(Type)/(CDo() + CD(Type)),
                  E,
                  CFx(Type),
                  CFy(Type),
                  CFz(Type),
                  CMx(Type),
                  CMy(Type),
                  CMz(Type),
                  ToQS,
                  Unsteady_H_,
                  CL(2),
                  CD(2),
                  CS(2),
                  CFx(2),
                  CFy(2),
                  CFz(2),
                  CMx(2),
                  CMy(2),
                  CMz(2));
                  
       }
       
       else if ( TimeAnalysisType_ == P_ANALYSIS || TimeAnalysisType_ == Q_ANALYSIS || TimeAnalysisType_ == R_ANALYSIS ) {
          
          fprintf(StatusFile_,"%9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf\n",
                  CurrentTime_,
                  Mach_,
                  AngleOfAttack_/TORAD,
                  AngleOfBeta_/TORAD,
                  CL(Type),
                  CDo(),
                  CD(Type),
                  CDo() + CD(Type),
                  CS(Type),            
                  CL(Type)/(CDo() + CD(Type)),
                  E,
                  CFx(Type),
                  CFy(Type),
                  CFz(Type),
                  CMx(Type),
                  CMy(Type),
                  CMz(Type),
                  ToQS,
                  Unsteady_Angle_*180./3.14159,
                  CL(2),
                  CD(2),
                  CS(2),
                  CFx(2),
                  CFy(2),
                  CFz(2),
                  CMx(2),
                  CMy(2),
                  CMz(2));                  
                  
       }        
       
       else {
          
          fprintf(StatusFile_,"%9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf\n",
                  CurrentTime_,
                  Mach_,
                  AngleOfAttack_/TORAD,
                  AngleOfBeta_/TORAD,
                  CL(Type),
                  CDo(),
                  CD(Type),
                  CDo() + CD(Type),
                  CS(Type),            
                  CL(Type)/(CDo() + CD(Type)),
                  E,
                  CFx(Type),
                  CFy(Type),
                  CFz(Type),
                  CMx(Type),
                  CMy(Type),
                  CMz(Type),
                  ToQS,
                  CL(2),
                  CD(2),
                  CS(2),
                  CFx(2),
                  CFy(2),
                  CFz(2),
                  CMx(2),
                  CMy(2),
                  CMz(2));                  
               
       }            
               
    }       

}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER OutputZeroLiftDragToStatusFile                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::OutputZeroLiftDragToStatusFile(void)
{
 
    int i;
    
    fprintf(StatusFile_,"\n");
    fprintf(StatusFile_,"\n");
    fprintf(StatusFile_,"\n");    
    fprintf(StatusFile_,"Skin Friction Drag Break Out:\n");    
    fprintf(StatusFile_,"\n");   
    fprintf(StatusFile_,"\n");       
                       //1234567890123456789012345678901234567890: 123456789
    fprintf(StatusFile_,"Surface                                      CDo \n");
    fprintf(StatusFile_,"\n");
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       fprintf(StatusFile_,"%-40s  %9.5lf \n",
               VSPGeom().VSP_Surface(i).ComponentName(),
               VSPGeom().VSP_Surface(i).CDo());

    } 
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER WriteCaseHeader                               #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::WriteCaseHeader(FILE *fid)
{
    char headerFormatStr[] = "%-20s %12s %-20s\n";
    char dataFormatStr[] =   "%-20s %12.7lf %-20s\n";
    
    fprintf(fid,"***************************************************************************************************************************************************************************************** \n");
    fprintf(fid,"\n");
    
    //          123456789012345678901234567890123456789
    fprintf(fid,headerFormatStr, "# Name", "Value   ", "  Units");
    fprintf(fid,dataFormatStr, "Sref_", Sref(), "Lunit^2");
    fprintf(fid,dataFormatStr, "Cref_", Cref(), "Lunit");
    fprintf(fid,dataFormatStr, "Bref_", Bref(), "Lunit");
    fprintf(fid,dataFormatStr, "Xcg_", Xcg(), "Lunit");
    fprintf(fid,dataFormatStr, "Ycg_", Ycg(), "Lunit");
    fprintf(fid,dataFormatStr, "Zcg_", Zcg(), "Lunit");
    fprintf(fid,dataFormatStr, "Mach_", Mach(), "no_unit");
    fprintf(fid,dataFormatStr, "AoA_", AngleOfAttack()/TORAD, "deg");
    fprintf(fid,dataFormatStr, "Beta_", AngleOfBeta()/TORAD, "deg");
    fprintf(fid,dataFormatStr, "Rho_", Density(), "Munit/Lunit^3");
    fprintf(fid,dataFormatStr, "Vinf_", Vinf(), "Lunit/Tunit");
    fprintf(fid,dataFormatStr, "Roll__Rate", RotationalRate_p(), "rad/Tunit");
    fprintf(fid,dataFormatStr, "Pitch_Rate", RotationalRate_q(), "rad/Tunit");
    fprintf(fid,dataFormatStr, "Yaw___Rate", RotationalRate_r(), "rad/Tunit");
    /*
    char control_name[20];
    for ( int n = 1 ; n <= NumberOfControlGroups_ ; n++ ) {
        //                    1234567890123456789
        sprintf(control_name,"Control_Group_%-5d",n);
        fprintf(fid,dataFormatStr, control_name, Delta_Control_, "deg");
    }
    */
    
    fprintf(fid,"\n");
}

/*##############################################################################
#                                                                              #
#                VSP_SOLVER CalculateLeadingEdgeSuctionFraction                #
#                                                                              #
##############################################################################*/

double VSP_SOLVER::CalculateLeadingEdgeSuctionFraction(double Mach, double ToC, double RoC, double EtaToC, double AoA, double Sweep)
{
   
   double Rin, RoCn, ToCn, Machn, Betan, Ctn;
   double e1, e2, e3, Ptt, Pat, k, Kt;
   
   // Normal RoC
   
   RoCn = RoC / pow(cos(Sweep),2.);
   
   // Normal ToC
   
   ToCn = ToC / cos(Sweep);   

   // Normal Leading edge radius index
   
   Rin = RoCn * EtaToC / pow(ToCn,2.);
   
   // Exponents
   
   e1 =  0.40*pow(Rin,0.16) - 0.7;
   e2 =  1.60*pow(Rin,0.10) - 3.0;
   e3 = -0.32*pow(Rin,0.10) - 0.3;
   
   // Normal Mach
   
   Machn = Mach * cos(Sweep);
   
   // Catch supersonic case
   
   if ( Machn >= 1. ) return 0.;   
   
   // Betan
   
   Betan = sqrt(1. - pow(Machn,2.));
   
   // Ctn
   
   Ctn = 2.*PI*pow(sin(AoA),2.) / ( Betan * pow(cos(Sweep),3.) );
   
   // Ptt
   
   Ptt = Ctn * Betan * pow( ToCn * pow(EtaToC/0.5,e1) / 0.09, e2);
   
   // k
   
   k = ( 0.14*(1. - (1. - sqrt(Rin)*pow(Machn,5.))) + 0.11*sqrt(Rin))*pow((1.-Machn)/Machn,0.48*(1.+pow(Rin,0.3)));
   
   // Kt

   Kt = (1. + sqrt(pow(ToCn,1.2))) * k * pow(Ptt,e3);
   
   Kt = MAX(0.,MIN(Kt,1.));
   
   return Kt;
 
}



