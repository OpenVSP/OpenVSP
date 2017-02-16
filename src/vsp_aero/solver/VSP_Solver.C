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
    
    FirstTimeSolve_ = 1;
    
    DoSymmetryPlaneSolve_ = 0;
    
    SetFarFieldDist_ = 0;
    
    NumberOfWakeTrailingNodes_ = 64; // Must be a power of 2
    
    SearchID_ = 0;
    
    SaveRestartFile_ = 0;
    
    JacobiRelaxationFactor_ = 0.90;
    
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
    
    sprintf(CaseString_,"No Comment");
    
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
    double Area, gamma, gm1, gm2, gm3, f1, pinf, rho;

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
        
    Gamma_ = new double[NumberOfVortexLoops_ + 1];    
    
    Diagonal_ = new double[NumberOfVortexLoops_ + 1];     
    
    GammaOld_= new double[NumberOfVortexLoops_ + 1];     
    
    Delta_= new double[NumberOfVortexLoops_ + 1];     
   
    zero_double_array(Gamma_,    NumberOfVortexLoops_); Gamma_[0]         = 0.;   
    zero_double_array(Diagonal_, NumberOfVortexLoops_); Diagonal_[0]      = 0.;
    zero_double_array(GammaOld_, NumberOfVortexLoops_); GammaOld_[0]      = 0.;
    zero_double_array(Delta_,    NumberOfVortexLoops_); Delta_[0]         = 0.;
   
    Residual_ = new double[NumberOfEquations_ + 1];    

    RightHandSide_ = new double[NumberOfEquations_ + 1];     
     
    MatrixVecTemp_ = new double[NumberOfEquations_ + 1];     
   
    zero_double_array(Residual_,      NumberOfEquations_); Residual_[0]      = 0.;
    zero_double_array(RightHandSide_, NumberOfEquations_); RightHandSide_[0] = 0.;
    zero_double_array(MatrixVecTemp_, NumberOfEquations_); RightHandSide_[0] = 0.;
  
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       LocalFreeStreamVelocity_[i] = new double[5];
       
    }      
    
    // Allocate space for span loading data
        
    Span_Cx_  = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cy_  = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cz_  = new double*[VSPGeom().NumberOfSurfaces() + 1];

    Span_Cn_  = new double*[VSPGeom().NumberOfSurfaces() + 1];

    Span_Cl_  = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cs_  = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cd_  = new double*[VSPGeom().NumberOfSurfaces() + 1];
    
    Span_Cmx_ = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmy_ = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Cmz_ = new double*[VSPGeom().NumberOfSurfaces() + 1];    
    
    Span_LE_   = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Yavg_ = new double*[VSPGeom().NumberOfSurfaces() + 1];
    Span_Area_ = new double*[VSPGeom().NumberOfSurfaces() + 1];
    
    Local_Vel_ = new double*[VSPGeom().NumberOfSurfaces() + 1];
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
        
       NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
 
       Span_Cx_[i]   = new double[NumberOfStations + 1];

       Span_Cy_[i]   = new double[NumberOfStations + 1];

       Span_Cz_[i]   = new double[NumberOfStations + 1];

       Span_Cmx_[i]  = new double[NumberOfStations + 1];

       Span_Cmy_[i]  = new double[NumberOfStations + 1];

       Span_Cmz_[i]  = new double[NumberOfStations + 1];
       
       Span_Cn_[i]   = new double[NumberOfStations + 1];
     
       Span_Cl_[i]   = new double[NumberOfStations + 1];

       Span_Cs_[i]   = new double[NumberOfStations + 1];
       
       Span_Cd_[i]   = new double[NumberOfStations + 1];
       
       Span_LE_[i]   = new double[NumberOfStations + 1];
                     
       Span_Yavg_[i] = new double[NumberOfStations + 1];
       
       Span_Area_[i] = new double[NumberOfStations + 1];
       
       Local_Vel_[i] = new double[NumberOfStations + 1];
 
       zero_double_array(Span_Cx_[i],   NumberOfStations);
       zero_double_array(Span_Cy_[i],   NumberOfStations);
       zero_double_array(Span_Cz_[i],   NumberOfStations);      
 
       zero_double_array(Span_Cn_[i],   NumberOfStations);       

       zero_double_array(Span_Cl_[i],   NumberOfStations);
       zero_double_array(Span_Cs_[i],   NumberOfStations);
       zero_double_array(Span_Cd_[i],   NumberOfStations);      
       
       zero_double_array(Span_Cmx_[i],  NumberOfStations);
       zero_double_array(Span_Cmy_[i],  NumberOfStations);
       zero_double_array(Span_Cmz_[i],  NumberOfStations);       
        
       zero_double_array(Span_LE_[i],   NumberOfStations);    
       zero_double_array(Span_Yavg_[i], NumberOfStations);       
       zero_double_array(Span_Area_[i], NumberOfStations);
       
       zero_double_array(Local_Vel_[i], NumberOfStations);
     
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
    
    // Limits on max velocity, and min/max pressures
    
    gamma = 1.4;
    
    gm1 = gamma - 1.;

    gm2 = 0.5*gm1*SQR(Mach_);

    gm3 = 1./gm1;

    pinf = 1./(gamma*SQR(Mach_));

    QMax_ = 0.75*sqrt( 1./gm2 + 1. );
    
    printf("QMax_: %f \n",QMax_);
     
    rho = pow( 1. - gm2*( QMax_ - 1. ), gm3 );

    CpMin_ = 2.*( pow( rho, gamma ) - 1. )*pinf;
    
    printf("CpMin_: %f \n",CpMin_);
    
    CpMin_ = -2.;

    f1 = 1. + 0.5*(gamma-1.)*Mach_*Mach_;

    CpMax_ = 2.*( pow(f1,(gamma)/(gamma-1)) - 1. ) / ( 1.4*Mach_*Mach_ );
    
    printf("CpMax_: %f \n",CpMax_);
    
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
    
    printf("CpBase_: %f \n",CpBase_);
    
}

/*##############################################################################
#                                                                              #
#              VSP_SOLVER DetermineNumberOfKelvinConstrains                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::DetermineNumberOfKelvinConstrains(void)
{
   
    int j, k, p, Edge, Node, Loop, Loop1, Loop2, Next, StackSize, Done, FoundOne;
    int Node1, Node2, *LoopStack, NumberOfVortexSheets, NotFlipped, KelvinGroup;
    double Vec1[3], Vec2[3], Dot;
        
    MGLevel_ = 1;
    
    // Traverse the loops, edge to edge, and determine the number of 
    // independent bodies... this is the number of Kelvin constraints
    // we have to impose
    
    LoopStack = new int[NumberOfVortexLoops_ + 1];
    
    LoopInKelvinConstraintGroup_ = new int[NumberOfVortexLoops_ + 1];
    
    zero_int_array(LoopStack, NumberOfVortexLoops_);
    
    zero_int_array(LoopInKelvinConstraintGroup_, NumberOfVortexLoops_);
    
    LoopIsOnBaseRegion_ = new int[NumberOfVortexLoops_ + 1];
    
    zero_int_array(LoopIsOnBaseRegion_, NumberOfVortexLoops_);
        
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
    
    // Check if there are any periodic wakes that might break a region in two

    NumberOfVortexSheets = 0;
  
    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {

       NumberOfVortexSheets = MAX(NumberOfVortexSheets, VSPGeom().Grid(MGLevel_).WingSurface(j));
       
    }    
    
    printf("There are: %10d Vortex Sheets \n", NumberOfVortexSheets);
    
    for ( k = 1 ; k <= NumberOfVortexSheets ; k++ ) {
    
       Done = 0;
       
       j = 1;
       
       while ( j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() && !Done ) {
          
          // Search for a kutta node on this vortex sheet
          
          if ( VSPGeom().Grid(MGLevel_).WingSurface(j) == k ) {
          
             // If it's periodic... we must check if it creates a closed region
             
             if ( VSPGeom().Grid(MGLevel_).WingSurfaceIsPeriodic(j) ) {

               Node = VSPGeom().Grid(MGLevel_).KuttaNode(j);
               
               // Grab a loop that contains this kutta node
               
               FoundOne = 0;
               
               k = 1;
               
               while ( !FoundOne ) {
                  
                  p = 1;
                  
                  while ( p <= VSPGeom().Grid(MGLevel_).LoopList(k).NumberOfEdges()  && !FoundOne ) {

                     Edge = VSPGeom().Grid(MGLevel_).LoopList(k).Edge(p);
                     
                     Node1 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Node1();
                     Node2 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Node2();
                     
                     if ( Node == Node1 || Node == Node2 ) {
                        
                        Loop = k;
                        
                        FoundOne = 1;
                        
                     }
                     
                     p++;
                     
                  }
                  
                  k++;
                  
               }
                              
               // Go edge, by edge and see if we cover the entire kelvin region
               
               zero_int_array(LoopStack, NumberOfVortexLoops_);
               
               StackSize = Next = 1;
                
               LoopStack[Next] = Loop;
               
               KelvinGroup = LoopInKelvinConstraintGroup_[Loop];
                   
               while ( Next <= StackSize ) {
                   
                  Loop = LoopStack[Next];
                   
                  for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).LoopList(Loop).NumberOfEdges() ; j++ ) {
                      
                     Edge = VSPGeom().Grid(MGLevel_).LoopList(Loop).Edge(j);
                     
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
                        
                        else if ( LoopInKelvinConstraintGroup_[Loop1] != -KelvinGroup ){
                           
                           printf("WTF... how did we jump to another Kelvin Group... \n"); fflush(NULL);
                           exit(1);
                           
                        }    
                        
                     }                       
                       
                  }
                   
                  Next++;   
           
               }
               
               // Check and see if there are any loops in this Kelvin group that were not flipped
               
               NotFlipped = 0;
               
               for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; j++ ) {                  
                        
                  if ( LoopInKelvinConstraintGroup_[j] == KelvinGroup ) {
                     
                     NotFlipped++;
                     
                  }
                  
               }
               
               printf("Base region found for vortex sheet system: %d \n",k);fflush(NULL);
          
               // If not all were flipped... then there is a base region
               
               if ( NotFlipped > 0 ) {
                                    
                  // Determine which region ... + or - ... is the base region
                  
                  Vec1[0] = Vec1[1] = Vec1[2] = 0.;
                  
                  Vec2[0] = Vec2[1] = Vec2[2] = 0.;

                  for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; j++ ) {                  
                           
                     if ( LoopInKelvinConstraintGroup_[j] ==  KelvinGroup ) {
                        
                        Vec1[0] += VSPGeom().Grid(MGLevel_).LoopList(j).Nx();
                        Vec1[1] += VSPGeom().Grid(MGLevel_).LoopList(j).Ny();
                        Vec1[2] += VSPGeom().Grid(MGLevel_).LoopList(j).Nz();
                        
                     }
                     
                     if ( LoopInKelvinConstraintGroup_[j] == -KelvinGroup ) {
                        
                        Vec2[0] += VSPGeom().Grid(MGLevel_).LoopList(j).Nx();
                        Vec2[1] += VSPGeom().Grid(MGLevel_).LoopList(j).Ny();
                        Vec2[2] += VSPGeom().Grid(MGLevel_).LoopList(j).Nz();
                        
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
                 
                     for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; j++ ) {                  
                           
                        if ( LoopInKelvinConstraintGroup_[j] == KelvinGroup ) {
                           
                           LoopIsOnBaseRegion_[j] = 1;
                           
                           LoopInKelvinConstraintGroup_[j] = KelvinGroup;
                    
                        }
                        
                     }
                     
                     for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; j++ ) {                  
                           
                        if ( LoopInKelvinConstraintGroup_[j] == -KelvinGroup ) {
                        
                           LoopInKelvinConstraintGroup_[j] = KelvinGroup;
                    
                        }
                        
                     }                     
                     
                  }
                  
                  else {
             
                     for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; j++ ) {                  
                           
                        if ( LoopInKelvinConstraintGroup_[j] == -KelvinGroup ) {
                           
                           LoopIsOnBaseRegion_[j] = 1;
                           
                           LoopInKelvinConstraintGroup_[j] = KelvinGroup;
                           
                        }
                        
                     }
                     
                     for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; j++ ) {                  
                           
                        if ( LoopInKelvinConstraintGroup_[j] == KelvinGroup ) {
                         
                           LoopInKelvinConstraintGroup_[j] = KelvinGroup;
                    
                        }
                        
                     }                        
                     
                  }  
                         
               }                 
               
               Done = 1;
               
             }
             
          }
          
          j++;
          
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
    VSP_NODE VSP_Node1, VSP_Node2;
    
    // Free stream velocity vector

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
    
    FreeStreamVelocity_[0] = CA*CB;
    FreeStreamVelocity_[1] =   -SB;
    FreeStreamVelocity_[2] = SA*CB;

    // Calculate local free stream conditions
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

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
          
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {

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
               
          }             
          
       }    
       
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
    
    // Trail initial wake, and the Trefftz flat wake with free stream
    
    if ( Mach_ < 1. ) {
     
       WakeAngle_[0] = FreeStreamVelocity_[0];
       WakeAngle_[1] = FreeStreamVelocity_[1];
       WakeAngle_[2] = FreeStreamVelocity_[2];
       
    }
    
    // Trail initial wake, and the Trefftz flat in x-direction... this is done more to guarantee that the
    // flat Trefftz wake lines within the Mach cone of the Trefftz integration line... as the final wake is 
    // allowed to move if user does wake relaxation...
    
    else {
     
       WakeAngle_[0] = 1.;
       WakeAngle_[1] = 0.;
       WakeAngle_[2] = 0.;
     
    }       

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

       NumberOfVortexSheets_ = MAX(NumberOfVortexSheets_, VSPGeom().Grid(MGLevel_).WingSurface(j));
       
    }    
    
    printf("There are: %10d Vortex Sheets \n", NumberOfVortexSheets_);
    
    VortexSheet_ = new VORTEX_SHEET[NumberOfVortexSheets_ + 1];

    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
       NumberOfKuttaNodes = 0;
       
       for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {
          
          if ( VSPGeom().Grid(MGLevel_).WingSurface(j) == k ) NumberOfKuttaNodes++;
          
       }
       
       printf("There are: %10d kutta nodes for vortex sheet: %10d \n",NumberOfKuttaNodes,k);
       
       VortexSheet(k).SizeTrailingVortexList(NumberOfKuttaNodes);
       
    }
           
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
    
       NumEdges = 0;
       
       for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {
          
          if ( VSPGeom().Grid(MGLevel_).WingSurface(j) == k ) {
          
             NumEdges++;
      
             VortexSheet(k).TrailingVortexEdge(NumEdges).Wing() = k;
             
             VortexSheet(k).IsPeriodic() = VSPGeom().Grid(MGLevel_).WingSurfaceIsPeriodic(j);

             VortexSheet(k).TrailingVortexEdge(NumEdges).Node() = VSPGeom().Grid(MGLevel_).KuttaNode(j);
                         
             // Pass in edge data and create edge cofficients
             
             VSP_Node1.x() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeX(j);
             VSP_Node1.y() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeY(j);
             VSP_Node1.z() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeZ(j);
                    
             VSP_Node2.x() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeX(j) + WakeAngle_[0] * 1.e6;
             VSP_Node2.y() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeY(j) + WakeAngle_[1] * 1.e6;
             VSP_Node2.z() = VSPGeom().Grid(MGLevel_).WakeTrailingEdgeZ(j) + WakeAngle_[2] * 1.e6;
       
             // Set sigma
        //djk
             VortexSheet(k).TrailingVortexEdge(NumEdges).Sigma() = 0.1*Sigma[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
             
             // Set trailing edge direction 
             
             VortexSheet(k).TrailingVortexEdge(NumEdges).TEVec(0) = VecX[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
             VortexSheet(k).TrailingVortexEdge(NumEdges).TEVec(1) = VecY[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
             VortexSheet(k).TrailingVortexEdge(NumEdges).TEVec(2) = VecZ[VSPGeom().Grid(MGLevel_).KuttaNode(j)];
            
             // Create trailing wakes... specify number of sub vortices per trail
   
             WakeDist =  MAX(VSP_Node1.x() + 0.5*FarDist, Xmax_ + 0.25*FarDist) - VSP_Node1.x();
      
             VortexSheet(k).TrailingVortexEdge(NumEdges).Setup(NumberOfWakeTrailingNodes_,WakeDist,VSP_Node1,VSP_Node2);
             
          }
             
       }
       
       VortexSheet(k).SetupVortexSheets();
       
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
   
    // Initialize free stream
    
    InitializeFreeStream();
    
    // Initialize the wake trailing vortices
    
    InitializeTrailingVortices();
    
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
    
                       //123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789   
    fprintf(StatusFile_,"  Iter      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS \n");

    printf("Solving... \n\n");fflush(NULL);

    if ( DumpGeom_ ) WakeIterations_ = 0;
    
    for ( CurrentWakeIteration_ = 1 ; CurrentWakeIteration_ <= WakeIterations_ ; CurrentWakeIteration_++ ) {
   
       // Solve the linear system

       SolveLinearSystem();
 
       // Update wake locations

       if ( WakeIterations_ > 1 ) UpdateWakeLocations();

       // Calculate forces

       CalculateForces();

       // Output status

       OutputStatusFile(0);
    
       printf("\n");
       
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
    
    // Calculate spanwise load distributions for lifting surfaces
    
    CalculateSpanWiseLoading();
    
    // Write out FEM loading file
    
    CreateFEMLoadFile();

    // Interpolate solution from grid 1 to 0
    
    InterpolateSolutionFromGrid(1);

    // Output and survey point results
    
    if ( NumberofSurveyPoints_ > 0 ) CalculateVelocitySurvey();
    
    // Write out ADB Geometry
    
    if ( Case == 0 || Case == 1 ) {

       WriteOutAerothermalDatabaseGeometry();
       
       if ( Write2DFEMFile_ ) WriteFEM2DGeometry();
       
    }       
    
    // Write out 2d FEM geometry if requested
    
    if ( Case == 0 || Case == 1 ) {

       if ( Write2DFEMFile_ ) WriteFEM2DGeometry();
       
    }          
    
    // Write out ADB Solution

    WriteOutAerothermalDatabaseSolution();
    
    // Write out 2d FEM solution if requested
    
    if ( Write2DFEMFile_ ) WriteFEM2DSolution();
 
    // Close up files
    
    if ( Case <= 0                    ) fclose(StatusFile_);
    if ( Case <= 0                    ) fclose(LoadFile_);
    if ( Case <= 0                    ) fclose(ADBFile_);
    if ( Case <= 0                    ) fclose(ADBCaseListFile_);
    if ( Case <= 0 && Write2DFEMFile_ ) fclose(FEM2DLoadFile_);

}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER SolveLinearSystem                             #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::SolveLinearSystem(void)
{
    
    // First time... calculate matrix diagonal

    if ( FirstTimeSolve_ ) {
    
       CalculateDiagonal();       
       
       FirstTimeSolve_ = 0;
       
    }

    // Solver the linear system

    Do_GMRES_Solve();    
    
    // Update the vortex strengths on the wake

    UpdateVortexEdgeStrengths(1);

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

}

/*##############################################################################
#                                                                              #
#                    VSP_SOLVER CalculateNeighborCoefs                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateNeighborCoefs(void)
{

    int i, j, Edge, Loop1, Loop2;
    double **NeighborCoef_;
    
    // Create coefficient space
    
    NeighborCoef_ = new double*[NumberOfVortexLoops_ + 1];
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       NeighborCoef_[i] = new double[VortexLoop(i).NumberOfEdges() + 1];
       
    }

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

       for ( j = 1 ; j <= VortexLoop(i).NumberOfEdges() ; j++ ) {
             
          Edge = VortexLoop(i).Edge(j);
          
          Loop1 = SurfaceVortexEdge(Edge).VortexLoop1();
          Loop2 = SurfaceVortexEdge(Edge).VortexLoop2();
                    
       }
     
       Diagonal_[i] = 1./Diagonal_[i];
        
    }    

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
    
    UpdateVortexEdgeStrengths(1);
              
    // Surface vortex induced velocities

    for ( Level = 1 ; Level < NumberOfMGLevels_ ; Level++ ) {
       
       RestrictSolutionFromGrid(Level);
           
       UpdateVortexEdgeStrengths(Level+1);
  
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
   
    if ( CurrentWakeIteration_ > NoWakeIteration_ ) {
          
       for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
        
         for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
   
            xyz[0] = VortexLoop(i).xyz_c()[0];
            xyz[1] = VortexLoop(i).xyz_c()[1];
            xyz[2] = VortexLoop(i).xyz_c()[2];
              
            VortexSheet(k).InducedVelocity(xyz, q);
   
            Temp = vector_dot(VortexLoop(i).Normal(), q);
   
            // If there is a symmetry plane, calculate influence of the reflection
          
            if ( DoSymmetryPlaneSolve_ ) {
             
              xyz[0] = VortexLoop(i).xyz_c()[0];
              xyz[1] = VortexLoop(i).xyz_c()[1];
              xyz[2] = VortexLoop(i).xyz_c()[2];
            
              if ( DoSymmetryPlaneSolve_ == SYM_X ) xyz[0] *= -1;
              if ( DoSymmetryPlaneSolve_ == SYM_Y ) xyz[1] *= -1;
              if ( DoSymmetryPlaneSolve_ == SYM_Z ) xyz[2] *= -1;
            
              VortexSheet(k).InducedVelocity(xyz, q);
      
              if ( DoSymmetryPlaneSolve_ == SYM_X ) q[0] *= -1;
              if ( DoSymmetryPlaneSolve_ == SYM_Y ) q[1] *= -1;
              if ( DoSymmetryPlaneSolve_ == SYM_Z ) q[2] *= -1;
            
              Temp += vector_dot(VortexLoop(i).Normal(), q);
            
            }   
   
            vec_out[i] += Temp;
   
         }
   
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

   int i;
  
   // Precondition using Jacobi

#pragma omp parallel for    
   for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {

      vec_in[i] *= JacobiRelaxationFactor_*Diagonal_[i];

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

    UpdateVortexEdgeStrengths(1);
 
    // Surface vortex induced velocities

    for ( Level = 1 ; Level < NumberOfMGLevels_ ; Level++ ) {
        
       RestrictSolutionFromGrid(Level);
       
       UpdateVortexEdgeStrengths(Level+1);

    }

    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
    
       U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(q,VortexEdge)            
       for ( j = 1 ; j <= NumberOfVortexEdgesForInteractionListEntry_[i] ; j++ ) {
        
          VortexEdge = SurfaceVortexEdgeInteractionList_[i][j];
       
          if ( !VortexEdge->IsTrailingEdge() ) {               

             VortexEdge->InducedVelocity(VortexLoop(i).xyz_c(), q);
         
             U += q[0];
             V += q[1];
             W += q[2];
          
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
               
             }                
                         
          }
          
       }
       
       VortexLoop(i).U() += U;
       VortexLoop(i).V() += V;
       VortexLoop(i).W() += W; 
       
    }
    
    // Trailing vortex induced velocities
    
    if ( CurrentWakeIteration_ > NoWakeIteration_ ) {
       
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
                
             if ( DoSymmetryPlaneSolve_ ) {
                
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
               
             }   
   
          }
          
          VortexLoop(i).U() += U;
          VortexLoop(i).V() += V;
          VortexLoop(i).W() += W;
            
       }
       
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

    int i, j, k, m, Iter, IterMax;
    double xyz[3], xyz_te[3], q[5], U, V, W;

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
                
                // If there is a symmetry plane, calculate influence of the reflection
                
                if ( DoSymmetryPlaneSolve_ ) {
        
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
             
             // If there is a symmetry plane, calculate influence of the reflection
             
             if ( DoSymmetryPlaneSolve_ ) {
     
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
                
             }
             
          }
          
       }
       
    }

    // Iterate on wake shape
      
    IterMax = 5;
    
    for ( Iter = 1 ; Iter <= IterMax ; Iter++ ) {
 
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

       if ( CurrentWakeIteration_ > NoWakeIteration_ ) {
          
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
                        
                      }                   
 
                   }
                   
                   VortexSheet(m).TrailingVortexEdge(i).U(j) += U;
                   VortexSheet(m).TrailingVortexEdge(i).V(j) += V;
                   VortexSheet(m).TrailingVortexEdge(i).W(j) += W;
                   
                }
              
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
       
       // Now update the trailing vortex locations
                 
       for ( m = 1 ; m <= NumberOfVortexSheets_ ; m++ ) {     
              
          for ( i = 1 ; i <= VortexSheet(m).NumberOfTrailingVortices() ; i++ ) {

             VortexSheet(m).TrailingVortexEdge(i).UpdateLocation();
             
          }

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
    double ResMax;

#pragma omp parallel for
    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {
     
       GammaOld_[i] = Gamma_[i];
       
       Delta_[i] = 0.;
       
    }

    CalculateResidual();
    
    DoMatrixPrecondition(Residual_);

    GMRES_Solver(NumberOfVortexLoops_+1,  // Number of Equations, 0 <= i < Neq
                 3,                       // Max number of outer iterations
                 500,                     // Max number of inner (restart) iterations
                 1,                       // Output flag, verbose = 0, or 1
                 Delta_,                  // Initial guess and solution vector
                 Residual_,               // Right hand side of Ax = b
                 0.001,                   // Maximum error tolerance <-----
                 0.1,                     // Residual reduction factor
                 ResMax,                  // Final log10 of residual reduction   
                 Iters);                  // Final iteration count      

    // Update solution vector

#pragma omp parallel for
    for ( i = 0 ; i <= NumberOfVortexLoops_ ; i++ ) {

       Gamma_[i] = GammaOld_[i] + Delta_[i];

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
     
      if ( Verbose && Iter == 0 ) printf("Wake Iteration: %5d ... GMRES Iteration: %5d ... Reduction: %10.5f ...  Maximum: %10.5f \r",CurrentWakeIteration_, 0,log10(rho/rho_zero),log10(rho)); fflush(NULL);
      
      if ( Iter == 0 ) rho_tol = rho * ErrorReduction;
    
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
    
         if ( Verbose ) printf("Wake Iteration: %5d ... GMRES Iteration: %5d ... Reduction: %10.5f ...  Maximum: %10.5f \r",CurrentWakeIteration_,TotalIterations,log10(rho/rho_zero),log10(rho)); fflush(NULL);
         
         if ( rho <= ErrorMax ) Done = 1;

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
    
    // Integrate forces and moments
    
    IntegrateForcesAndMoments();

    // Calculate 2D clmax limited forces and moments, as well as span wide
    // loading information
    
    if ( ModelType_ == VLM_MODEL ) CalculateCLmaxLimitedForces();

}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CalculateTrefftzForces                    #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateTrefftzForces(void)
{

    int j, k, p, Loop, Hits;
    double xyz[3], q[3], qtot[3], Factor;
    double mag1, mag2, dot, angle;

    // Loop over vortex edges and calculate forces via K-J theorem, using only wake induced velocities applied at TE

    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {

       // Calculate an averaged local velocity using the left/right loops

       qtot[0] = qtot[1] = qtot[2] = 0.;
       
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
 
             qtot[0] += LocalFreeStreamVelocity_[Loop][0] * Factor;
             qtot[1] += LocalFreeStreamVelocity_[Loop][1] * Factor;
             qtot[2] += LocalFreeStreamVelocity_[Loop][2] * Factor;
   
             Hits++;
          
          }
          
       }

       qtot[0] /= Hits;
       qtot[1] /= Hits;
       qtot[2] /= Hits;
       
       // Trailing vortices induced velocities... shift the current bound vortex to the 
       // 'trailing edge' of the trailing vortex.
 
       for ( p = 1 ; p <= NumberOfVortexSheets_ ; p++ ) {

          xyz[0] = SurfaceVortexEdge(j).Xc();
          xyz[1] = SurfaceVortexEdge(j).Yc();
          xyz[2] = SurfaceVortexEdge(j).Zc();

          VortexSheet(p).InducedKuttaVelocity(xyz, q);

          qtot[0] += q[0];
          qtot[1] += q[1];
          qtot[2] += q[2];
     
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {

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
            
          }
        
       }

       SurfaceVortexEdge(j).CalculateTrefftzForces(qtot);
       
    }
   
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER CalculateKuttaJukowskiForces                  #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateKuttaJukowskiForces(void)
{

    int j, Loop1, Loop2;
    double Fx, Fy, Fz, Hits;

    // Loop over vortex edges and calculate forces via K-J theorem

#pragma omp parallel for private(Loop1, Loop2, Fx, Fy, Fz, Hits)
    for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
     
       Fx = SurfaceVortexEdge(j).Fx() = 0.;
       Fy = SurfaceVortexEdge(j).Fy() = 0.;
       Fz = SurfaceVortexEdge(j).Fz() = 0.;
          
       if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {
        
          Hits = 0.;
          
          Loop1 = SurfaceVortexEdge(j).VortexLoop1();
          
          if ( Loop1 != 0 ) {

             SurfaceVortexEdge(j).CalculateForces(VortexLoop(Loop1));
             
             Fx += SurfaceVortexEdge(j).Fx();
             Fy += SurfaceVortexEdge(j).Fy();
             Fz += SurfaceVortexEdge(j).Fz();
             
             Hits += 1.;
            
          }

          Loop2 = SurfaceVortexEdge(j).VortexLoop2();

          if ( Loop2 != 0 ) {
           
             SurfaceVortexEdge(j).CalculateForces(VortexLoop(Loop2));
             
             Fx += SurfaceVortexEdge(j).Fx();
             Fy += SurfaceVortexEdge(j).Fy();
             Fz += SurfaceVortexEdge(j).Fz();
             
             Hits += 1.;
             
          }       
                   
          // Edge forces
          
          SurfaceVortexEdge(j).Fx() = Fx / Hits;
          SurfaceVortexEdge(j).Fy() = Fy / Hits;
          SurfaceVortexEdge(j).Fz() = Fz / Hits;
          
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
    double Fact1, Fact2, CpI, Cp;

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
          
          Fx = SurfaceVortexEdge(j).Trefftz_Fx();
          Fy = SurfaceVortexEdge(j).Trefftz_Fy();
          Fz = SurfaceVortexEdge(j).Trefftz_Fz();
          
       }
       
       else {
          
          Fx = SurfaceVortexEdge(j).Fx();
          Fy = SurfaceVortexEdge(j).Fy();
          Fz = SurfaceVortexEdge(j).Fz();
          
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
       
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
   
       VortexLoop(i).NormalForce() = -VortexLoop(i).Fx() * VortexLoop(i).Nx()
                                   + -VortexLoop(i).Fy() * VortexLoop(i).Ny()
                                   + -VortexLoop(i).Fz() * VortexLoop(i).Nz();
                                   
       VortexLoop(i).dCp() = VortexLoop(i).NormalForce() / VortexLoop(i).Area();      
       
       // 2nd order correction
       
       if ( Mach_ < 1. ) {
       
          Cp = VortexLoop(i).dCp();
          
          CpI = sqrt(1.-Mach_*Mach_)*Cp;
          
          CpI = ABS(Cp);
          
          Fact1 = 1./sqrt(1.-Mach_*Mach_);
          
          Fact2 = 1. + 0.25*CpI*Mach_*Mach_/sqrt(1.-Mach_*Mach_); 
   
          VortexLoop(i).CompressibilityFactor() = Fact2;
   
          VortexLoop(i).dCp() = Cp * Fact2;
          
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

    int i, j, Loop1, Loop2;
    double Vec[3], Mag, Mag1, Mag2, Gradient;
    double *U, *V, *W, *UDenom, *VDenom, *WDenom;
    double Cp, Fact, Ratio, CpC, q2;
    
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

       Mag = vector_dot(VortexLoop(Loop1).Normal(), VortexLoop(Loop2).Normal());

       if ( Mag > 0.95 && !SurfaceVortexEdge(j).IsTrailingEdge() ) {
   
          Vec[0] = VortexLoop(Loop1).Xc() - SurfaceVortexEdge(j).Xc();
          Vec[1] = VortexLoop(Loop1).Yc() - SurfaceVortexEdge(j).Yc();
          Vec[2] = VortexLoop(Loop1).Zc() - SurfaceVortexEdge(j).Zc();
          
          Mag1 = sqrt(vector_dot(Vec,Vec));

          Vec[0] = VortexLoop(Loop2).Xc() - SurfaceVortexEdge(j).Xc();
          Vec[1] = VortexLoop(Loop2).Yc() - SurfaceVortexEdge(j).Yc();
          Vec[2] = VortexLoop(Loop2).Zc() - SurfaceVortexEdge(j).Zc();
          
          Mag2 = sqrt(vector_dot(Vec,Vec));
   
          Vec[0] = VortexLoop(Loop2).Xc() - VortexLoop(Loop1).Xc();
          Vec[1] = VortexLoop(Loop2).Yc() - VortexLoop(Loop1).Yc();
          Vec[2] = VortexLoop(Loop2).Zc() - VortexLoop(Loop1).Zc();
          
          Mag = sqrt(vector_dot(Vec,Vec));
          
          Vec[0] /= Mag;
          Vec[1] /= Mag;
          Vec[2] /= Mag;
          
          Gradient = -0.5*( VortexLoop(Loop2).Gamma() - VortexLoop(Loop1).Gamma() ) / ( Mag1 + Mag2 );
   
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
    
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
       
       Mag = vector_dot(VortexLoop(i).Normal(), VortexLoop(i).Velocity());
       
       VortexLoop(i).U() -= Mag * VortexLoop(i).Nx();
       VortexLoop(i).V() -= Mag * VortexLoop(i).Ny();
       VortexLoop(i).W() -= Mag * VortexLoop(i).Nz();
      
    }       
    
    // Calculate Cp
                   
    for ( i = 1 ; i <= NumberOfVortexLoops_ ; i++ ) {
 
       VortexLoop(i).dCp() =  pow(VortexLoop(i).U(),2.)
                            + pow(VortexLoop(i).V(),2.)
                            + pow(VortexLoop(i).W(),2.);

       if ( VortexLoop(i).dCp() > QMax_*QMax_ ) {
       
          VortexLoop(i).dCp() = QMax_*QMax_;

       } 
       
       q2 = VortexLoop(i).dCp();

       VortexLoop(i).dCp() = 1. - VortexLoop(i).dCp(); 
       
       // 2nd order correction
 
       Cp = VortexLoop(i).dCp();

       Fact = 1. - 0.25*Cp*Mach_*Mach_*Cp;
Fact = 1.;       
       CpC = Cp*Fact; 
 
       Ratio = 1;
       
       if ( Cp != 0 ) {
          
          Ratio = CpC / Cp;
          
       }
       
       if ( Ratio <= 0. ) Ratio = 1.;
Ratio = 1.;             
       // Update Cp, and keep track of the ratio

       VortexLoop(i).CompressibilityFactor() = Ratio;

       VortexLoop(i).dCp() = CpC;       
 
    }  
    
    // Enforce base pressures
    

        
    //Cp *= ABS(sin(angle_in_radians));    
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

void VSP_SOLVER::IntegrateForcesAndMoments(void)
{

    int j, Loop1, Loop2;
    double Fx, Fy, Fz;
    double CA, SA, CB, SB;
    double Cx, Cy, Cz, Cmx, Cmy, Cmz, CompressibilityFactor;

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);

    // Loop over vortex edges and integrate the forces / moments
    
    Cx = Cy = Cz = Cmx = Cmy = Cmz = 0.;

    if ( Mach_ < 1. ) {
   
#pragma omp parallel for reduction(+:Cx, Cy, Cz, Cmx, Cmy, Cmz) private(Loop1, Loop2, Fx, Fy, Fz, CompressibilityFactor)
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
     
          Loop1 = SurfaceVortexEdge(j).LoopL();
          Loop2 = SurfaceVortexEdge(j).LoopR();
                
          // Integrate forces
          
          if ( ModelType_ == PANEL_MODEL ) {
             
             if ( SurfaceVortexEdge(j).IsTrailingEdge() ) {

                // Simplified compressibility factor for TE calculation
                
                CompressibilityFactor = 1. + 0.25*Mach_*Mach_;

                Fx = -SurfaceVortexEdge(j).Trefftz_Fx() * CompressibilityFactor;
                Fy = -SurfaceVortexEdge(j).Trefftz_Fy() * CompressibilityFactor;
                Fz = -SurfaceVortexEdge(j).Trefftz_Fz() * CompressibilityFactor;
                
                Cx += Fx;
                Cy += Fy;
                Cz += Fz;
                
             }
                
             if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {
                
                CompressibilityFactor = 0.5*( VortexLoop(Loop1).CompressibilityFactor() + VortexLoop(Loop2).CompressibilityFactor() );
                
                Fx = SurfaceVortexEdge(j).Trefftz_Fx() * CompressibilityFactor;
                Fy = SurfaceVortexEdge(j).Trefftz_Fy() * CompressibilityFactor;
                Fz = SurfaceVortexEdge(j).Trefftz_Fz() * CompressibilityFactor;
                                
                Cmx += Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
                Cmy += Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                Cmz += Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw
                
             }      
         
          }
          
          else if ( ModelType_ == VLM_MODEL && !SurfaceVortexEdge(j).IsTrailingEdge() ) {
             
             CompressibilityFactor = 0.5*( VortexLoop(Loop1).CompressibilityFactor() + VortexLoop(Loop2).CompressibilityFactor() );
             
             Fx = SurfaceVortexEdge(j).Trefftz_Fx() * CompressibilityFactor;
             Fy = SurfaceVortexEdge(j).Trefftz_Fy() * CompressibilityFactor;
             Fz = SurfaceVortexEdge(j).Trefftz_Fz() * CompressibilityFactor;
                          
             Cx += Fx;
             Cy += Fy;
             Cz += Fz;
             
             Cmx += Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
             Cmy += Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
             Cmz += Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw
               
          }             
          
       }
             
    }
    
    else {
    
#pragma omp parallel for reduction(+:Cx, Cy, Cz, Cmx, Cmy, Cmz) private(Loop1, Loop2, Fx, Fy, Fz)
       for ( j = 1 ; j <= NumberOfSurfaceVortexEdges_ ; j++ ) {
    
          Loop1 = SurfaceVortexEdge(j).LoopL();
          Loop2 = SurfaceVortexEdge(j).LoopR();
          
          // Edge forces
   
          Fx = SurfaceVortexEdge(j).Fx();
          Fy = SurfaceVortexEdge(j).Fy();
          Fz = SurfaceVortexEdge(j).Fz();
  
          // Integrate forces
          
          if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {
             
             Cx += Fx;
             Cy += Fy;
             Cz += Fz;
             
          }             
             
          // Integrate moments

          Cmx += Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll

          Cmy += Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                
          Cmz += Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw
          
       }
          
    }
 
    CFx_[0] = Cx;
    CFy_[0] = Cy;
    CFz_[0] = Cz;
    
    CMx_[0] = Cmx;
    CMy_[0] = Cmy;
    CMz_[0] = Cmz;

    CFx_[0] /= (0.5*Sref_);
    CFy_[0] /= (0.5*Sref_);
    CFz_[0] /= (0.5*Sref_);
    
    CMx_[0] /= (0.5*Sref_*Bref_); // Roll
    CMy_[0] /= (0.5*Sref_*Cref_); // Pitch
    CMz_[0] /= (0.5*Sref_*Bref_); // Yaw

    // Adjust for symmetry
    
    if ( DoSymmetryPlaneSolve_ ) {
       
       CFx_[0] *= 2.;
       CFy_[0] *= 2.; 
       CFz_[0] *= 2.; 
   
       CMx_[0] *= 2.; 
       CMy_[0] *= 2.; 
       CMz_[0] *= 2.; 
       
    }
   
    if ( DoSymmetryPlaneSolve_ == SYM_X ) CMy_[0] = CMz_[0] = CFx_[0] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Y ) CMx_[0] = CMz_[0] = CFy_[0] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Z ) CMx_[0] = CMy_[0] = CFz_[0] = 0.;
    
    // Now calculate CL, CD, CS
 
    CL_[0] = (-CFx_[0] * SA + CFz_[0] * CA );
    CD_[0] = ( CFx_[0] * CA + CFz_[0] * SA ) * CB - CFy_[0] * SB;
    CS_[0] = ( CFx_[0] * CA + CFz_[0] * SA ) * SB + CFy_[0] * CB; 
        
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER CalculateCLmaxLimitedForces                   #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CalculateCLmaxLimitedForces(void)
{

    int i, j, k, Loop, Loop1, Loop2;
    int NumberOfStations, SpanStation, SurfaceID; 
    double Fx, Fy, Fz;
    double Length, Re, Cf, Cl, Cd, Cs, Ct, Cn, Cx, Cy, Cz;
    double Swet, SwetTotal, StallFact, CompressibilityFactor;
    double CA, SA, CB, SB, CMx, CMy, CMz, Cmx, Cmy, Cmz, Cl_2d, dCD;

    CA = cos(AngleOfAttack_);
    SA = sin(AngleOfAttack_);

    CB = cos(AngleOfBeta_);
    SB = sin(AngleOfBeta_);
    
    // Zero out spanwise loading arrays
    
    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
      
       NumberOfStations = VSPGeom().VSP_Surface(i).NumberOfSpanStations();
 
       zero_double_array(Span_Cx_[i],   NumberOfStations);
       zero_double_array(Span_Cy_[i],   NumberOfStations);
       zero_double_array(Span_Cz_[i],   NumberOfStations);       
 
       zero_double_array(Span_Cn_[i],   NumberOfStations);       
 
       zero_double_array(Span_Cl_[i],   NumberOfStations);
       zero_double_array(Span_Cd_[i],   NumberOfStations);
       zero_double_array(Span_Cs_[i],   NumberOfStations);       
      
       zero_double_array(Span_Cmx_[i],  NumberOfStations);
       zero_double_array(Span_Cmy_[i],  NumberOfStations);
       zero_double_array(Span_Cmz_[i],  NumberOfStations);       
      
       zero_double_array(Span_Yavg_[i], NumberOfStations);       
       zero_double_array(Span_Area_[i], NumberOfStations);
       
       zero_double_array(Local_Vel_[i], NumberOfStations);
         
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
                 
       CompressibilityFactor = 0.5*( VortexLoop(Loop1).CompressibilityFactor() + VortexLoop(Loop2).CompressibilityFactor() );

       if ( !SurfaceVortexEdge(j).IsTrailingEdge() ) {
          
          // Extract out forces
          
          if ( Mach_ < 1. ) {
             
             Fx = SurfaceVortexEdge(j).Trefftz_Fx() * CompressibilityFactor;
             Fy = SurfaceVortexEdge(j).Trefftz_Fy() * CompressibilityFactor;
             Fz = SurfaceVortexEdge(j).Trefftz_Fz() * CompressibilityFactor;
             
          }
          
          else {
             
             Fx = SurfaceVortexEdge(j).Fx();
             Fy = SurfaceVortexEdge(j).Fy();
             Fz = SurfaceVortexEdge(j).Fz();
             
          }
          
          // Fix for wing tip loads
  
          if ( SurfaceVortexEdge(j).IsBoundaryEdge() && !SurfaceVortexEdge(j).IsLeadingEdge() ) {
         
             Fx = Fy = Fz = 0.;
         
          }
          
          // Sum up span wise loading

          for ( k = 1 ; k <= 2 ; k++ ) {
           
             if ( k == 1 ) Loop = SurfaceVortexEdge(j).Loop1();
             
             if ( k == 2 ) Loop = SurfaceVortexEdge(j).Loop2();
     
             // Wing Surface
             
             if ( VortexLoop(Loop).DegenWingID() > 0 ) {
                
                SurfaceID = VortexLoop(Loop).SurfaceID();
                
                SpanStation = VortexLoop(Loop).SpanStation();
                
                // Chordwise integrated forces

                Span_Cx_[SurfaceID][SpanStation] += 0.5*Fx;

                Span_Cy_[SurfaceID][SpanStation] += 0.5*Fy;

                Span_Cz_[SurfaceID][SpanStation] += 0.5*Fz;
                               
                Span_Cn_[SurfaceID][SpanStation] += 0.5 * Fx * VortexLoop(Loop).Nx()
                                                  + 0.5 * Fy * VortexLoop(Loop).Ny()
                                                  + 0.5 * Fz * VortexLoop(Loop).Nz();

                // Chordwise integrated moments
   
                Span_Cmx_[SurfaceID][SpanStation] += 0.5 * Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - 0.5 * Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
            
                Span_Cmy_[SurfaceID][SpanStation] += 0.5 * Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - 0.5 * Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                      
                Span_Cmz_[SurfaceID][SpanStation] += 0.5 * Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - 0.5 * Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw      
             
             }
             
             // Body Surface
             
             else {
            
                SurfaceID = VortexLoop(Loop).SurfaceID();
                
                SpanStation = 1;
           
                // Chordwise integrated forces
                       
                Span_Cx_[SurfaceID][SpanStation] += 0.5*Fx;

                Span_Cy_[SurfaceID][SpanStation] += 0.5*Fy;

                Span_Cz_[SurfaceID][SpanStation] += 0.5*Fz;
                               
                Span_Cn_[SurfaceID][SpanStation] += 0.5 * Fx * VortexLoop(Loop).Nx()
                                                  + 0.5 * Fy * VortexLoop(Loop).Ny()
                                                  + 0.5 * Fz * VortexLoop(Loop).Nz();
                
                // Chordwise integrated moments
                       
                Span_Cmx_[SurfaceID][SpanStation] += 0.5 * Fz * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] ) - 0.5 * Fy * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] );   // Roll
               
                Span_Cmy_[SurfaceID][SpanStation] += 0.5 * Fx * ( SurfaceVortexEdge(j).Zc() - XYZcg_[2] ) - 0.5 * Fz * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] );   // Pitch
                         
                Span_Cmz_[SurfaceID][SpanStation] += 0.5 * Fy * ( SurfaceVortexEdge(j).Xc() - XYZcg_[0] ) - 0.5 * Fx * ( SurfaceVortexEdge(j).Yc() - XYZcg_[1] );   // Yaw

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
    
    CL_[0] = CS_[0] = CD_[0] = CFx_[0] = CFy_[0] = CFz_[0]= CMx_[0] = CMy_[0] = CMz_[0] = SwetTotal = CDo_ = 0.;  

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
                    
          Cl =   ( -Span_Cx_[i][k] * SA + Span_Cz_[i][k] * CA )                              / (0.5*Span_Area_[i][k]);
   
          Cd = ( (  Span_Cx_[i][k] * CA + Span_Cz_[i][k] * SA ) * CB - Span_Cy_[i][k] * SB ) / (0.5*Span_Area_[i][k]);
   
          Cs = ( (  Span_Cx_[i][k] * CA + Span_Cz_[i][k] * SA ) * SB + Span_Cy_[i][k] * CB ) / (0.5*Span_Area_[i][k]);

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
          Span_Cd_[i][k] = StallFact * Cd;
          Span_Cs_[i][k] = StallFact * Cs;
          
          Span_Cx_[i][k] = StallFact * Cx;
          Span_Cy_[i][k] = StallFact * Cy;
          Span_Cz_[i][k] = StallFact * Cz;
           
          Span_Cmx_[i][k] = StallFact * Cmx;
          Span_Cmy_[i][k] = StallFact * Cmy;
          Span_Cmz_[i][k] = StallFact * Cmz;

          // Integrate spanwise forces and moments
          
           CL_[0] += 0.5 *  Span_Cl_[i][k] * Span_Area_[i][k];
           CD_[0] += 0.5 *  Span_Cd_[i][k] * Span_Area_[i][k];
           CS_[0] += 0.5 *  Span_Cs_[i][k] * Span_Area_[i][k];
 
          CFx_[0] += 0.5 *  Span_Cx_[i][k] * Span_Area_[i][k];
          CFy_[0] += 0.5 *  Span_Cy_[i][k] * Span_Area_[i][k];
          CFz_[0] += 0.5 *  Span_Cz_[i][k] * Span_Area_[i][k];
          
          CMx_[0] += 0.5 * Span_Cmx_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
          CMy_[0] += 0.5 * Span_Cmy_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
          CMz_[0] += 0.5 * Span_Cmz_[i][k] * Span_Area_[i][k] * VSPGeom().VSP_Surface(i).LocalChord(k);
          
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
    
    CL_[0] /= 0.5*Sref_;
    CD_[0] /= 0.5*Sref_;
    CS_[0] /= 0.5*Sref_;
    
    CFx_[0] /= 0.5*Sref_;
    CFy_[0] /= 0.5*Sref_;
    CFz_[0] /= 0.5*Sref_;

    CMx_[0] /= 0.5*Bref_*Sref_;
    CMy_[0] /= 0.5*Cref_*Sref_;
    CMz_[0] /= 0.5*Bref_*Sref_;

          
    // Adjust for symmetry
    
    if ( DoSymmetryPlaneSolve_ ) {
       
       CFx_[0] *= 2.;
       CFy_[0] *= 2.; 
       CFz_[0] *= 2.; 
   
       CMx_[0] *= 2.; 
       CMy_[0] *= 2.; 
       CMz_[0] *= 2.; 
       
       CL_[0] *= 2.; 
       CD_[0] *= 2.; 
       CS_[0] *= 2.; 
       
    }
        
    if ( DoSymmetryPlaneSolve_ == SYM_X ) CMy_[0] = CMz_[0] = CFx_[0] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Y ) CMx_[0] = CMz_[0] = CFy_[0] = CS_[0] = 0.;
    if ( DoSymmetryPlaneSolve_ == SYM_Z ) CMx_[0] = CMy_[0] = CFz_[0] = 0.;
              
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
#                          VSP_SOLVER CreateFEMLoadFile                        #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::CreateFEMLoadFile(void)
{
 
    int i, k, NumberOfStations;
    double TotalLift;
    double Vec[3], VecQC[3], VecQC_Def[3], RVec[3], Force[3], Moment[3], Chord;
    char LoadFileName[2000];
    FILE *LoadFile;
    
    // Open the fem load file
    
    sprintf(LoadFileName,"%s.fem",FileName_);
    
    if ( (LoadFile = fopen(LoadFileName, "w")) == NULL ) {

       printf("Could not open the fem load file for output! \n");

       exit(1);

    }

    TotalLift = 0.;  

    for ( i = 1 ; i <= VSPGeom().NumberOfSurfaces() ; i++ ) { 
     
       if ( VSPGeom().VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
        
          fprintf(LoadFile,"Wing Surface: %d \n",i);
          fprintf(LoadFile,"SpanStations: %d \n",VSPGeom().VSP_Surface(i).NumberOfSpanStations());
          fprintf(LoadFile,"\n");
          
          //                123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789  
          fprintf(LoadFile,"   Wing    XLE_ORIG  YLE_ORIG  ZLE_ORIG  XTE_ORIG  YTE_ORIG  ZTE_ORIG  XQC_ORIG  YQC_ORIG  ZQC_ORIG  S_ORIG     Area     Chord     XLE_DEF   YLE_DEF   ZLE_DEF   XTE_DEF   YTE_DEF   ZTE_DEF   XQC_DEF   YQC_DEF   ZQC_DEF    S_DEF       Cl        Cd        Cs        Cx        Cy        Cz       Cmx       Cmy       Cmz \n");

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

             fprintf(LoadFile,"%9d %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf %9.5lf \n",
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
          
          fprintf(LoadFile,"\n");
          fprintf(LoadFile,"   Planform:\n");
          fprintf(LoadFile,"\n");

          fprintf(LoadFile,"   Root LE: %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Root_LE(0),VSPGeom().VSP_Surface(i).Root_LE(1),VSPGeom().VSP_Surface(i).Root_LE(2));
          fprintf(LoadFile,"   Root TE: %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Root_TE(0),VSPGeom().VSP_Surface(i).Root_TE(1),VSPGeom().VSP_Surface(i).Root_TE(2));
          fprintf(LoadFile,"   Root QC: %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Root_QC(0),VSPGeom().VSP_Surface(i).Root_QC(1),VSPGeom().VSP_Surface(i).Root_QC(2));

          fprintf(LoadFile,"\n");

          fprintf(LoadFile,"   Tip LE:  %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Tip_LE(0),VSPGeom().VSP_Surface(i).Tip_LE(1),VSPGeom().VSP_Surface(i).Tip_LE(2));
          fprintf(LoadFile,"   Tip TE:  %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Tip_TE(0),VSPGeom().VSP_Surface(i).Tip_TE(1),VSPGeom().VSP_Surface(i).Tip_TE(2));
          fprintf(LoadFile,"   Tip QC:  %9.5lf %9.5lf %9.5lf \n",VSPGeom().VSP_Surface(i).Tip_QC(0),VSPGeom().VSP_Surface(i).Tip_QC(1),VSPGeom().VSP_Surface(i).Tip_QC(2));
                      
       }
                 
    }

    
        
    
    fprintf(LoadFile,"\n\n");
    fprintf(LoadFile,"Total Forces and Moments \n");
    fprintf(LoadFile,"\n\n");
     
    fprintf(LoadFile,"Total CL:  %lf \n", CL_[0]);
    fprintf(LoadFile,"Total CD:  %lf \n", CD_[0]);
    fprintf(LoadFile,"Total CS:  %lf \n", CS_[0]);
    
    fprintf(LoadFile,"Total CFx: %lf \n", CFx_[0]);
    fprintf(LoadFile,"Total CFy: %lf \n", CFy_[0]);
    fprintf(LoadFile,"Total CFz: %lf \n", CFz_[0]);

    fprintf(LoadFile,"Total CMx: %lf \n", CMx_[0]);
    fprintf(LoadFile,"Total CMy: %lf \n", CMy_[0]);
    fprintf(LoadFile,"Total CMz: %lf \n", CMz_[0]);
    

 
    fclose(LoadFile);

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
          
          // If there is a symmetry plane, calculate influence of the reflection
          
          if ( DoSymmetryPlaneSolve_ ) {
   
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
       
       // If there is a symmetry plane, calculate influence of the reflection
       
       if ( DoSymmetryPlaneSolve_ ) {

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
       
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_1(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_1(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_1(2);
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_); 
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_2(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_2(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_2(2);
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_); 
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_3(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_3(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_3(2);
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_); 
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_4(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_4(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_4(2);
             
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
       
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_1(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_1(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_1(2);
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_); 
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_2(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_2(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_2(2);
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_); 
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_3(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_3(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_3(2);
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_); 
             
             x = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_4(0);
             y = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_4(1);
             z = VSPGeom().VSP_Surface(j).ControlSurface(k).Node_4(2);
             
             fwrite(&x, f_size, 1, ADBFile_); 
             fwrite(&y, f_size, 1, ADBFile_); 
             fwrite(&z, f_size, 1, ADBFile_);                              
                          
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

       fwrite(&Cp, f_size, 1, ADBFile_); // Wall or Edge Pressure, Pa

    }

    // Write out wake shape
    
    fwrite(&NumberOfTrailingVortexEdges_, i_size, 1, ADBFile_);
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
      
       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {

          VortexSheet(k).TrailingVortexEdge(i).WriteToFile(ADBFile_);

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
   
          SurfaceVortexEdgeInteractionList_[k] = CreateInteractionList(xyz, NumberOfEdges);
                                
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

    InteractionList = CreateInteractionList(xyz, NumberOfEdges);

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

    InteractionList = CreateInteractionList(xyz, NumberOfEdges);

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

VSP_EDGE **VSP_SOLVER::CreateInteractionList(double xyz[3], int &NumberOfInteractionEdges)
{

    int i, j, Level, Loop;
    int Level_1, Level_2, Used, i_1, i_2;
    int StackSize, MoveDownLevel, Next, Found;
    double Distance, FarAway, Mu, TanMu, Test;
    VSP_EDGE **InteractionEdgeList;
    
    // Mach angle
    
    TanMu = 1.e9;
    
    if ( Mach_ > 1. ) {
       
       Mu = asin(1./Mach_);
       
       TanMu = sin(Mu);
       
    }
       
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
       
       FirstTimeSetup_ = 0;
       
    }

    // Define faraway criteria... how far away we need to be from a loop to treat it as faraway
    // Ratio of distance to maximum loop size
    
    FarAway = 3.0;
    
    if ( Mach_ > 1. ) FarAway = 6.;

    // Insert loops on coarsest level into stack
    
    Level = VSPGeom().NumberOfGridLevels() - 1;
 
    StackSize = 0;

    for ( Loop = 1 ; Loop <= VSPGeom().Grid(Level).NumberOfLoops() ; Loop++ ) {
     
       StackSize++;
       
       LoopStackList_[StackSize].Level = Level;
       LoopStackList_[StackSize].Loop  = Loop;

    }
      
    // Update the search ID value... reset things after we done all the loops
    
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

       Test = MAX(VSPGeom().Grid(Level).LoopList(Loop).Length(), VSPGeom().Grid(Level).LoopList(Loop).Length()/TanMu);
  
       Test = FarAway * ( Test + VSPGeom().Grid(Level).LoopList(Loop).CentroidOffSet() );
       
       if ( Level == 1 || ( Test <= Distance && !inside_box(VSPGeom().Grid(Level).LoopList(Loop).BoundBox(), xyz) ) ) {
      
          // Add these edges to the list
          
          for ( i = 1 ; i <= VSPGeom().Grid(Level).LoopList(Loop).NumberOfEdges() ; i++ ) {
    
             j = VSPGeom().Grid(Level).LoopList(Loop).Edge(i);
             
             EdgeIsUsed_[Level][j] = SearchID_;
             
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
       
       UpdateVortexEdgeStrengths(Level+1);
   
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
    int *FixedNode,  NodeHits;
    double Fact, *dCp, *Denom, *Res, *Dif, *Sum, Delta, Eps;
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
   
          dCp[Node] = 1.;
          
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
     
      Sum[Node1] += 1.;
      Sum[Node2] += 1.;

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
     
           Delta = ( Res[Node1] - Res[Node2] );

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
          
          if ( ModelType_ == VLM_MODEL || Sum[Node1] > 2. ) { Wgt1 = 1.; NodeHits++; }
          if ( ModelType_ == VLM_MODEL || Sum[Node2] > 2. ) { Wgt2 = 1.; NodeHits++; }
          
          CpAvg += Wgt1*dCp[Node1] + Wgt2*dCp[Node2];
          
       }
       
       if ( NodeHits > 0 ) VSPGeom().Grid(g_c).LoopList(i_c).dCp() = CpAvg / NodeHits;
       
    }
    
    // Enforce base pressures
    
    if ( ModelType_ == PANEL_MODEL ) {
       
       for ( i_c = 1 ; i_c <= NumberOfVortexLoops_ ; i_c++ ) {
          
          if ( LoopIsOnBaseRegion_[i_c] ) VSPGeom().Grid(g_c).LoopList(i_c).dCp() = CpBase_;
          
       }  
    
    }
    
    // Prolongate solution from course grid at Level, to finer grid and Level - 1

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

void VSP_SOLVER::UpdateVortexEdgeStrengths(int Level)
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
       
          VortexSheet(k).UpdateVortexStrengths();
          
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

    // Control surface groups
    for ( int i = 1 ; i <= NumberOfControlGroups_ ; i++ ) {

        fprintf(fid,dataFormatStr, ControlSurfaceGroup_[i].Name(), ControlSurfaceGroup_[i].ControlSurface_DeflectionAngle(), "deg");

    }
    
    fprintf(fid,"\n");
}

