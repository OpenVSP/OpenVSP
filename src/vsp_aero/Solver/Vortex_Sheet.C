#include "Vortex_Sheet.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                              VORTEX_SHEET constructor                        #
#                                                                              #
##############################################################################*/

VORTEX_SHEET::VORTEX_SHEET(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                 VORTEX_SHEET init                            #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::init(void)
{

    Verbose_ = 0;
    
    IsPeriodic_ = 0;
    
    WingSurface_ = 0;

    NumberOfTrailingVortices_ = 0;

    TimeAccurate_ = 0;

    Time_ = 0;
    
    CoreWidth_ = 0.;
    
    CoreSizeFactor_ = 1.;

    IsARotor_ = 0;
        
    DoVortexStretching_ = 0;
        
    OptimizationSolve_ = 0;
    
    Is2D_ = 0;
    
    TrailingVortexList_ = NULL;

    NumberOfWakeLoops_ = 0;
    
    WakeLoopList_ = NULL;
    
}

/*##############################################################################
#                                                                              #
#                                VORTEX_SHEET Copy                             #
#                                                                              #
##############################################################################*/

VORTEX_SHEET::VORTEX_SHEET(const VORTEX_SHEET &VortexSheet)
{
   
    init();

    // Just * use the operator = code

    *this = VortexSheet;
     
}

/*##############################################################################
#                                                                              #
#                               VORTEX_SHEET operator=                         #
#                                                                              #
##############################################################################*/

VORTEX_SHEET& VORTEX_SHEET::operator=(const VORTEX_SHEET &VortexSheet)
{

    int i, k, Level;

    Verbose_                  = VortexSheet.Verbose_;
    
    WingSurface_              = VortexSheet.WingSurface_;

    IsPeriodic_               = VortexSheet.IsPeriodic_;
   
    NumberOfTrailingVortices_ = VortexSheet.NumberOfTrailingVortices_;

    TimeAccurate_             = VortexSheet.TimeAccurate_;
    
    TimeAnalysisType_         = VortexSheet.TimeAnalysisType_;
    
    Vinf_                     = VortexSheet.Vinf_;
    
    FreeStreamVelocity_[0]    = VortexSheet.FreeStreamVelocity_[0];
    
    FreeStreamVelocity_[1]    = VortexSheet.FreeStreamVelocity_[1];
    
    FreeStreamVelocity_[2]    = VortexSheet.FreeStreamVelocity_[2];

    Time_                 = VortexSheet.Time_;
   
    Time_          = VortexSheet.Time_;
   
    CoreWidth_                = VortexSheet.CoreWidth_;
    
    CoreSizeFactor_           = VortexSheet.CoreSizeFactor_;
    
    IsARotor_                 = VortexSheet.IsARotor_;

    // Trailing vortex list

    TrailingVortexList_ = new VORTEX_TRAIL[NumberOfTrailingVortices_ + 2];

    // Wake loop list
    
    WakeLoopList_ = new GLOBAL_WAKE_LOOP[NumberOfWakeLoops_ + 1];

    for ( i = 1 ; i <= NumberOfWakeLoops_ ; i++ ) {
       
       WakeLoopList_[i] = VortexSheet.WakeLoopList_[i];
       
    }

    return *this;

}

/*##############################################################################
#                                                                              #
#                            VORTEX_SHEET destructor                           #
#                                                                              #
##############################################################################*/

VORTEX_SHEET::~VORTEX_SHEET(void)
{

    int i, Level;

    if ( NumberOfTrailingVortices_ != 0 )delete [] TrailingVortexList_;

    TrailingVortexList_ = NULL;
   
    Verbose_ = 0;
    
    IsPeriodic_ = 0;
    
    WingSurface_ = 0;

    NumberOfTrailingVortices_ = 0;

    TimeAccurate_ = 0;

    Time_ = 0;
    
    CoreWidth_ = 0.;
    
    CoreSizeFactor_ = 1.;
    
    IsARotor_ = 0;
    
    NumberOfWakeLoops_ = 0;
    
    WakeLoopList_ = NULL;

}

/*##############################################################################
#                                                                              #
#                       VORTEX_SHEET SizeTrailingVortexList                    #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SizeTrailingVortexList(int NumberOfTrailingVortices)
{
      
    int i;

    // Clear up old stuff

    if ( NumberOfTrailingVortices_ != 0 ) delete [] TrailingVortexList_;
          
    TrailingVortexList_ = NULL;

    // Resize 
   
    NumberOfTrailingVortices_ = NumberOfTrailingVortices;
  
    TrailingVortexList_ = new VORTEX_TRAIL[NumberOfTrailingVortices_ + 2]; // Allocate 1 extra in case we are periodic

}

/*##############################################################################
#                                                                              #
#                       VORTEX_SHEET SizeTrailingVortexList                    #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SizeWakeLoopList(void)
{
      
    int i;

    if ( NumberOfWakeLoops_ != 0 ) {
       
       if ( WakeLoopList_ != NULL ) delete [] WakeLoopList_;
       
       NumberOfWakeLoops_ = 0;
       
       WakeLoopList_ = NULL;
       
    }
    
    NumberOfWakeLoops_ = ( NumberOfTrailingVortices_ - 1 ) * ( NumberOfWakeTrailingNodes_ - 1 );
    
    if ( IsPeriodic_ ) NumberOfWakeLoops_ += NumberOfWakeTrailingNodes_ - 1;
    
    NumberOfKuttaEdges_ = NumberOfTrailingVortices_ - 1;
    
    if ( IsPeriodic_ ) NumberOfKuttaEdges_++;

    WakeLoopList_ = new GLOBAL_WAKE_LOOP[NumberOfWakeLoops_ + 1];

}

/*##############################################################################
#                                                                              #
#                      VORTEX_SHEET SetupVortexSheets                          #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SetupVortexSheets(void)
{
   
    int i;
    
    // Catch degenerate case with no kutta nodes...
    
    if ( NumberOfTrailingVortices_ > 0 ) {
          
       // Closed vortex sheet (likely a nacelle... )
   
       if ( IsPeriodic_ ) {
          
          SetupCircularVortexSheets();
          
       }
       
       // Open - planar-ish - vortex sheet
       
       else {
          
          SetupPlanarVortexSheets();
          
       }

    }
    
    SizeWakeLoopList();
    
}

/*##############################################################################
#                                                                              #
#                  VORTEX_SHEET SetupPlanarVortexSheets                        #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SetupPlanarVortexSheets(void)
{

    int j;
    
    // Calculate the average core size... 
    
    CoreWidth_ = 0.;
    
    for ( j = 1 ; j <= NumberOfTrailingVortices_ ; j++ ) {
       
       CoreWidth_ += pow(TrailingVortexList_[j].Sigma(),2.);
 
    }
    
    CoreWidth_ /= NumberOfTrailingVortices_;

    CoreWidth_ = sqrt(CoreWidth_); 
    
    // User adjustment factor for core size
    
    CoreWidth_ *= CoreSizeFactor_;

    if ( 1||Verbose_ ) printf("CoreWidth_: %f \n",CoreWidth_);

}

/*##############################################################################
#                                                                              #
#                  VORTEX_SHEET SetupCircularVortexSheets                      #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SetupCircularVortexSheets(void)
{

    int j;

    // Calculate the average core size... 
    
    CoreWidth_ = 0.;
    
    for ( j = 1 ; j <= NumberOfTrailingVortices_ ; j++ ) {
       
       CoreWidth_ += pow(2.*TrailingVortexList_[j].Sigma(),2.);

    }
    
    CoreWidth_ /= NumberOfTrailingVortices_;
    
    CoreWidth_ = sqrt(CoreWidth_); 

    // User adjustment factor for core size
    
    CoreWidth_ *= CoreSizeFactor_;

    if ( Verbose_ ) printf("CoreWidth_: %f \n",CoreWidth_);
    
}

/*##############################################################################
#                                                                              #
#                 VORTEX_SHEET UpdateConvectedDistance                         #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::UpdateConvectedDistance(void)
{
   
    int i;

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
            
       TrailingVortexList_[i].Time() = Time_;
       
       TrailingVortexList_[i].DeltaTime() = DeltaTime_;
      
    }     
   
}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET TurnWakeDampingOn                        #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::TurnWakeDampingOn(void)
{
   
    int i;
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       TrailingVortexList_[i].WakeDampingIsOn() = 1;

    }       
    
}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET TurnWakeDampingOff                       #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::TurnWakeDampingOff(void)
{
   
    int i;
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       TrailingVortexList_[i].WakeDampingIsOn() = 0;

    }       
    
}

/*##############################################################################
#                                                                              #
#                    VORTEX_TRAIL UpdateWakeLocation                           #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::UpdateWakeLocation(double WakeRelax, double &ResMax, double &ResL2)
{

    int i, im, ip;
    double Rmax, RL2;

    ResMax = ResL2 = 0.;

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
       
       im = i - 1;
       ip = i + 1;
       
       if ( i == 1 ) im = i;
       if ( i == NumberOfTrailingVortices_ ) ip = i;
       
       if ( !TrailingVortexList_[i].TE_Node_Region_Is_Concave() &&
            !TrailingVortexList_[i].IsFrozenTrailingEdge()         ) {
      
          TrailingVortexList_[i].UpdateWakeLocation(WakeRelax,Rmax,RL2);
          
          ResMax = MAX(ResMax,Rmax);
          
          ResL2 += RL2;

       }
    
    }

}

/*##############################################################################
#                                                                              #
#                    VORTEX_TRAIL StoreWakeLocation                            #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::StoreWakeLocation(void)
{

    int i;
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
       
       TrailingVortexList_[i].StoreWakeLocation();

    }

}

/*##############################################################################
#                                                                              #
#                    VORTEX_TRAIL UpdateUnsteadyWakeLocation                   #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::UpdateUnsteadyWakeLocation(double WakeRelax, double &ResMax, double &ResL2)
{

    int i, im, ip;
    double Rmax, RL2;
    
    ResMax = ResL2 = 0.;

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
       
       im = i - 1;
       ip = i + 1;
       
       if ( i == 1 ) im = i;
       if ( i == NumberOfTrailingVortices_ ) ip = i;

       if ( !TrailingVortexList_[i].TE_Node_Region_Is_Concave() &&
            !TrailingVortexList_[i].IsFrozenTrailingEdge()         ) {

          TrailingVortexList_[i].UpdateUnsteadyWakeLocation(WakeRelax,Rmax,RL2);

          ResMax = MAX(ResMax, Rmax);
          
          ResL2 += RL2;
 
       }
        
    }

}

/*##############################################################################
#                                                                              #
#                      VORTEX_SHEET UpdateRotorFlags                           #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::UpdateRotorFlags(int *ComponentInThisGroup)
{
   
    int i;
   
    // Update each trailing wake shape
    
    IsARotor_ = 0;

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       TrailingVortexList_[i].IsARotor() = 0;

    }    
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
 
       if ( ComponentInThisGroup[TrailingVortexList_[i].ComponentID()] ) {

           TrailingVortexList_[i].IsARotor() = 1;
           
           IsARotor_ = 1;

       }
       
    }
    
}

/*##############################################################################
#                                                                              #
#                      VORTEX_SHEET CalculateWakeResidual                      #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::CalculateWakeResidual(double *Residual)
{
   
    int i, j;

    // Loop over trailing edge vortices
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
    
       if ( !TrailingVortexList_[i].TE_Node_Region_Is_Concave() && 
            !TrailingVortexList_[i].IsFrozenTrailingEdge()         ) {
             
          for ( j = 1 ; j < TrailingVortexList_[i].NumberOfNodes() ; j++ ) {
          
             // Steady solution
             
             if ( !TimeAccurate_ ) {

                TrailingVortexList_[i].EdgeList(j)->CalculateWakeResidual(Residual);

             }
             
             // Time accurate
             
             else {
                
                TrailingVortexList_[i].EdgeList(j)->CalculateUnsteadyWakeResidual(Residual,DeltaTime_);
                
             }
 
          }
          
       }
      
    }
              
}

/*##############################################################################
#                                                                              #
#                      VORTEX_SHEET WakeMatrixMultiply                         #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::WakeMatrixMultiply(double *VecIn, double *VecOut)
{
   
    int i, j;
    double Omega;

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       if ( !TrailingVortexList_[i].TE_Node_Region_Is_Concave() && 
            !TrailingVortexList_[i].IsFrozenTrailingEdge()         ) {
          
          for ( j = 1 ; j < TrailingVortexList_[i].NumberOfNodes() ; j++ ) {
    
             if ( !TimeAccurate_ ) {
    
                TrailingVortexList_[i].EdgeList(j)->WakeMatrixMultiply(VecIn,VecOut);

             }
             
             else {
                
                TrailingVortexList_[i].EdgeList(j)->UnsteadyWakeMatrixMultiply(VecIn,VecOut,DeltaTime_);
                   
             }               
                
          }
          
          // Fix TE node equation
   
          j = 1;
          
          VecOut[TrailingVortexList_[i].WakeResidualEquationNumberX(j)] = VecIn[TrailingVortexList_[i].WakeResidualEquationNumberX(j)];  
                                                                          
          VecOut[TrailingVortexList_[i].WakeResidualEquationNumberY(j)] = VecIn[TrailingVortexList_[i].WakeResidualEquationNumberY(j)];  
                                                                          
          VecOut[TrailingVortexList_[i].WakeResidualEquationNumberZ(j)] = VecIn[TrailingVortexList_[i].WakeResidualEquationNumberZ(j)];  

       }
       
       else {
   
          for ( j = 1 ; j <= TrailingVortexList_[i].NumberOfNodes() ; j++ ) {
    
             VecOut[TrailingVortexList_[i].WakeResidualEquationNumberX(j)] = VecIn[TrailingVortexList_[i].WakeResidualEquationNumberX(j)];  
                                                                             
             VecOut[TrailingVortexList_[i].WakeResidualEquationNumberY(j)] = VecIn[TrailingVortexList_[i].WakeResidualEquationNumberY(j)];  
                                                                             
             VecOut[TrailingVortexList_[i].WakeResidualEquationNumberZ(j)] = VecIn[TrailingVortexList_[i].WakeResidualEquationNumberZ(j)];  
                   
          }
          
       }          
            
    }
             
}

/*##############################################################################
#                                                                              #
#                 VORTEX_SHEET DoWakeForwardMatrixPrecondition                 #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::DoWakeForwardMatrixPrecondition(double *VecIn)
{
   
    int i, j;
    
    if ( !TimeAccurate_ ) {

       for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
   
          if ( !TrailingVortexList_[i].TE_Node_Region_Is_Concave() && 
               !TrailingVortexList_[i].IsFrozenTrailingEdge()          ) {
                                 
             for ( j = 1 ; j < TrailingVortexList_[i].NumberOfNodes() ; j++ ) {
       
                TrailingVortexList_[i].EdgeList(j)->DoWakeForwardMatrixPrecondition(VecIn);

             }
             
          }
             
       }
       
    }
   
}

/*##############################################################################
#                                                                              #
#            VORTEX_SHEET WakeTranposeMatrixMultiplyMeshPerturbations          #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::WakeTranposeMatrixMultiplyMeshPerturbations(double *VecIn, double *VecOut)
{
   
    int i, j;

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       if ( !TrailingVortexList_[i].TE_Node_Region_Is_Concave() && 
            !TrailingVortexList_[i].IsFrozenTrailingEdge()         ) {
           
           if ( !TimeAccurate_ ) {
                             
              for ( j = 1 ; j < TrailingVortexList_[i].NumberOfNodes() ; j++ ) {
              
                 TrailingVortexList_[i].EdgeList(j)->WakeTranposeMatrixMultiplyMeshPerturbations(VecIn,VecOut);
                 
              }
               
           }
           
           else {
              
              for ( j = 1 ; j < TrailingVortexList_[i].NumberOfNodes() ; j++ ) {
              
                 TrailingVortexList_[i].EdgeList(j)->UnsteadyWakeTranposeMatrixMultiplyMeshPerturbations(VecIn,VecOut);
                 
              }
               
           }
           
           // Fix TE node equation
           
           j = 1;
           
           VecOut[TrailingVortexList_[i].WakeResidualEquationNumberX(j)] += VecIn[TrailingVortexList_[i].WakeResidualEquationNumberX(j)];  
                                                                        
           VecOut[TrailingVortexList_[i].WakeResidualEquationNumberY(j)] += VecIn[TrailingVortexList_[i].WakeResidualEquationNumberY(j)];  
                                                                        
           VecOut[TrailingVortexList_[i].WakeResidualEquationNumberZ(j)] += VecIn[TrailingVortexList_[i].WakeResidualEquationNumberZ(j)];  
           
       }
       
       else {
          
          for ( j = 1 ; j <= TrailingVortexList_[i].NumberOfNodes() ; j++ ) {
          
             VecOut[TrailingVortexList_[i].WakeResidualEquationNumberX(j)] += VecIn[TrailingVortexList_[i].WakeResidualEquationNumberX(j)];  
                                                                          
             VecOut[TrailingVortexList_[i].WakeResidualEquationNumberY(j)] += VecIn[TrailingVortexList_[i].WakeResidualEquationNumberY(j)];  
                                                                          
             VecOut[TrailingVortexList_[i].WakeResidualEquationNumberZ(j)] += VecIn[TrailingVortexList_[i].WakeResidualEquationNumberZ(j)];   
                                        
          }          
          
       }

    }
   
}

#include "END_NAME_SPACE.H"















