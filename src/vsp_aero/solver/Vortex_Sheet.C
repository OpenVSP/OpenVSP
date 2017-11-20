#include "Vortex_Sheet.H"

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
     
    NumberOfLevels_ = 0;

    NumberOfTrailingVortices_ = 0;
    
    NumberOfStartingVortices_ = 0;
    
    ThereAreChildren_ = 0 ;
    
    TrailingGamma_ = NULL;
    
    StartingGamma_ = NULL;

    TrailingVortexList_ = NULL;
    
    StartingVortexList_ = NULL;
   
    NumberOfTrailingVorticesForLevel_ = NULL;
    
    TrailingVortexEvaluatedAtLevel_ = NULL;
    
    TrailingVortexGammaForLevel_ = NULL;
    
    TrailingVortexListForLevel_ = NULL;
  
    NumberOfVortexSheetsForLevel_ = NULL;
    
    VortexSheetListForLevel_ = NULL;
    
    VortexTrail1_ = NULL;
    
    VortexTrail2_ = NULL;

    Child1_ = NULL;
    
    Child2_ = NULL;
    
    AgglomeratedTrailingVortexList_ = NULL;

    TimeAccurate_ = 0;
    
    Evaluate_ = 0;
    
    MaxConvectedDistance_ = 1.e12;
    
}

/*##############################################################################
#                                                                              #
#                                VORTEX_SHEET Copy                             #
#                                                                              #
##############################################################################*/

VORTEX_SHEET::VORTEX_SHEET(const VORTEX_SHEET &Solver_)
{
   
    printf("Copy not implemented! \n");
    fflush(NULL);
    exit(1);

}

/*##############################################################################
#                                                                              #
#                               VORTEX_SHEET operator=                         #
#                                                                              #
##############################################################################*/

VORTEX_SHEET& VORTEX_SHEET::operator=(const VORTEX_SHEET &Trailing_Vortex)
{

    printf("operator== not implemented! \n");
    fflush(NULL);
    exit(1);

}

/*##############################################################################
#                                                                              #
#                            VORTEX_SHEET destructor                           #
#                                                                              #
##############################################################################*/

VORTEX_SHEET::~VORTEX_SHEET(void)
{

    int i, Level;

    if ( NumberOfTrailingVortices_ != 0 ) {

       if ( TrailingVortexList_ != NULL ) delete [] TrailingVortexList_;

       for ( i = 0 ; i <= NumberOfTrailingVortices_ ; i++ ) {
    
           // Trailing and starting gammas
   
           if ( TrailingGamma_[i] != NULL ) delete [] TrailingGamma_[i];
        
           if ( StartingGamma_[i] != NULL ) delete [] StartingGamma_[i];

       }  

       if ( TrailingGamma_ != NULL ) delete [] TrailingGamma_;
        
       if ( StartingGamma_ != NULL ) delete [] StartingGamma_;
       
    } 
     
    if ( StartingVortexList_ != NULL ) delete [] StartingVortexList_;

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       if ( VortexSheetListForLevel_[Level] != NULL ) delete [] VortexSheetListForLevel_[Level];

       if ( TrailingVortexListForLevel_[Level] != NULL ) delete [] TrailingVortexListForLevel_[Level];

    }

    if ( VortexSheetListForLevel_ != NULL ) delete [] VortexSheetListForLevel_;

    if ( TrailingVortexListForLevel_ != NULL ) delete [] TrailingVortexListForLevel_;

}

/*##############################################################################
#                                                                              #
#                       VORTEX_SHEET SizeTrailingVortexList                    #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SizeTrailingVortexList(int NumberOfTrailingVortices)
{
      
    int i, Level;

    // Clear up old stuff

    if ( NumberOfTrailingVortices_ != 0 ) {

       if ( TrailingVortexList_ != NULL ) delete [] TrailingVortexList_;

       for ( i = 0 ; i <= NumberOfTrailingVortices_ ; i++ ) {

           // Trailing and starting gammas

           if ( TrailingGamma_[i] != NULL ) delete [] TrailingGamma_[i];
           
           if ( StartingGamma_[i] != NULL ) delete [] StartingGamma_[i];

       }  
     
       if ( TrailingGamma_ != NULL ) delete [] TrailingGamma_;
    
       if ( StartingGamma_ != NULL ) delete [] StartingGamma_;
       
    }

    if ( StartingVortexList_ != NULL ) delete [] StartingVortexList_;

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       if ( VortexSheetListForLevel_[Level] != NULL ) delete [] VortexSheetListForLevel_[Level];

       if ( TrailingVortexListForLevel_[Level] != NULL ) delete [] TrailingVortexListForLevel_[Level];
  
    }

    if ( VortexSheetListForLevel_ != NULL ) delete [] VortexSheetListForLevel_;
    
    if ( TrailingVortexListForLevel_ != NULL ) delete [] TrailingVortexListForLevel_;

    // Resize 
   
    NumberOfTrailingVortices_ = NumberOfTrailingVortices;
   
    TrailingVortexList_ = new VORTEX_TRAIL[NumberOfTrailingVortices_ + 2];
   
    TrailingGamma_ = new double*[NumberOfTrailingVortices_ + 2]; 
   
    StartingGamma_ = new double*[NumberOfTrailingVortices_ + 2]; 

    AgglomeratedTrailingVortexList_= new VORTEX_TRAIL*[NumberOfTrailingVortices_ + 2];
      
}

/*##############################################################################
#                                                                              #
#                      VORTEX_SHEET SetupVortexSheets                          #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SetupVortexSheets(void)
{
   
    int i;
    
    // Closed vortex sheet (likely a nacelle... )

    if ( IsPeriodic_ ) {
       
       SetupCircularVortexSheets();
       
    }
    
    // Open - planar-ish - vortex sheet
    
    else {
       
       SetupPlanarVortexSheets();
       
    }
    
    // Size gamma list

    for ( i = 0 ; i <= NumberOfTrailingVortices_ + 1 ; i++ ) {

       // Trailing and starting gammas
       
       TrailingGamma_[i] = new double[NumberOfSubVortices() + 2];
       
       StartingGamma_[i] = new double[NumberOfSubVortices() + 2];
       
       zero_double_array(TrailingGamma_[i], NumberOfSubVortices() + 1);
       
       zero_double_array(StartingGamma_[i], NumberOfSubVortices() + 1);
           
    }
  
}

/*##############################################################################
#                                                                              #
#                  VORTEX_SHEET SetupPlanarVortexSheets                        #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SetupPlanarVortexSheets(void)
{

    int i, j, k, m, p, Level, Done;

    // Determine the number of vortex sheets at each level
    
    k = NumberOfTrailingVortices_ - 1;
    
    Done = 0;
    
    Level = 1;
  
    while ( !Done ) {
     
       j = k;
       
       k /= 2;
       
       if ( 2 * k != j ) k++;
       
       if ( k == 1 ) Done = 1;
    
       Level++;

    }
    
    NumberOfLevels_ = Level;

    // Size the vortex sheet list
    
    NumberOfVortexSheetsForLevel_ = new int[NumberOfLevels_ + 1];
   
    VortexSheetListForLevel_ = new VORTEX_SHEET*[NumberOfLevels_ + 1];
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
       
       VortexSheetListForLevel_[Level] = NULL;
       
    }

    // Size the trailing vortex list
    
    NumberOfTrailingVorticesForLevel_ = new int[NumberOfLevels_ + 1]; 
     
    TrailingVortexListForLevel_ = new VORTEX_TRAIL**[NumberOfLevels_ + 1];
    
    k = NumberOfTrailingVortices_ - 1;

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
     
       // Size vortex sheet list for this level
       
       NumberOfVortexSheetsForLevel_[Level] = k;
       
       VortexSheetListForLevel_[Level] = new VORTEX_SHEET[k + 1];

       // Size trailing vortex list for this level
       
       NumberOfTrailingVorticesForLevel_[Level] = k + 1;
       
       TrailingVortexListForLevel_[Level] = new VORTEX_TRAIL*[k + 2];
     
       j = k;
       
       k /= 2;
       
       if ( 2 * k != j ) k++;
       
    }
    
    // Set up the trailing vortices for each sub level
    
    m = 1;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
       
       k = 0;
       
       for ( j = 1 ; j <= NumberOfTrailingVortices_  ; j+=m ) {
       
          k++;
    
          TrailingVortexListForLevel_[Level][k] = &(TrailingVortexList_[j]);
          
       }
       
       j -= m;
       
       if ( j != NumberOfTrailingVortices_ ) {
          
          j = NumberOfTrailingVortices_;
          
          k++;
          
          TrailingVortexListForLevel_[Level][k] = &(TrailingVortexList_[j]);
          
       }
       
       if ( k != NumberOfTrailingVorticesForLevel_[Level] ) {
          
          printf("wtf! \n");
          printf("Level: %d ... k: %d ... NumberOfTrailingVorticesForLevel_[%d]: %d \n",
          Level,
          k, 
          Level,
          NumberOfTrailingVorticesForLevel_[Level]);
          
          fflush(NULL);
          
          exit(1);
          
       }
       
       m *= 2;
       
    }
 
    // Set up the vortex sheets for each sub level, along with children pointers
    // Level = 1 is the finest level... it contains all the trailing vortices...
    // NumberOfLevels_ is the coarsest level... 
    
    m = 1;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
       
       k = 0;
       
       p = 1;
       
       for ( j = 1 ; j <= NumberOfTrailingVortices_ - 1 ; j+=m ) {
       
          k++;
    
          // Set the two trailing vortices for this sheet
          
          VortexSheetListForLevel_[Level][k].SetTrailingVortices(*TrailingVortexListForLevel_[Level][k], 
                                                                 *TrailingVortexListForLevel_[Level][k+1]);
                                             
          VortexSheetListForLevel_[Level][k].TimeAccurate() = TimeAccurate_;
          VortexSheetListForLevel_[Level][k].TimeStep()     = TimeStep_;
          VortexSheetListForLevel_[Level][k].Vinf()         = Vinf_;
                                                                 
          VortexSheetListForLevel_[Level][k].Setup();
          
          // Set the two vortex sheet children for this sheet
          
          if ( Level > 1 ) {
           
             if ( p + 1 <= NumberOfVortexSheetsForLevel_[Level-1] ) {

                VortexSheetListForLevel_[Level][k].SetupChildren(VortexSheetListForLevel_[Level-1][p  ],
                                                                 VortexSheetListForLevel_[Level-1][p+1]);
                                                 
             }
             
             else {
            
                VortexSheetListForLevel_[Level][k].SetupChildren(VortexSheetListForLevel_[Level-1][p  ]);
                                                           
             }                
                                                              
          }

          p += 2;
          
       }
    
       j -= m;

       if ( k != NumberOfVortexSheetsForLevel_[Level] ) {
          
          printf("wtf! \n");
          printf("Level: %d ... k: %d ... NumberOfVortexSheetsForLevel_[%d]: %d \n",
          Level,
          k, 
          Level,
          NumberOfVortexSheetsForLevel_[Level]);
          
          fflush(NULL);
          
          exit(1);
          
       }
       
       m *= 2;
       
    }    

    NumberOfStartingVortices_ = VortexSheetListForLevel_[1][1].NumberOfSubVortices();

    NumberOfSubVortices_ = VortexSheetListForLevel_[1][1].NumberOfSubVortices();   

}

/*##############################################################################
#                                                                              #
#                  VORTEX_SHEET SetupCircularVortexSheets                      #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SetupCircularVortexSheets(void)
{

    int i, j, k, m, p, Level, Done;
    VSP_NODE NodeA, NodeB;

    // Make trailing vortex list periodic

    TrailingVortexList_[NumberOfTrailingVortices_ + 1] = TrailingVortexList_[1];
       
    // Determine the number of vortex sheets at each level
    
    k = NumberOfTrailingVortices_;
    
    Done = 0;
    
    Level = 1;
  
    while ( !Done ) {
     
       j = k;
       
       k /= 2;
       
       if ( 2 * k != j ) k++;
       
       if ( k <= 4 ) Done = 1;
          
       Level++;

    }

    NumberOfLevels_ = Level;

    // Size the vortex sheet list
    
    NumberOfVortexSheetsForLevel_ = new int[NumberOfLevels_ + 1];
   
    VortexSheetListForLevel_ = new VORTEX_SHEET*[NumberOfLevels_ + 1];
    
    // Size the trailing vortex list
    
    NumberOfTrailingVorticesForLevel_ = new int[NumberOfLevels_ + 1]; 
     
    TrailingVortexListForLevel_ = new VORTEX_TRAIL**[NumberOfLevels_ + 1];
    
    k = NumberOfTrailingVortices_;

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       // Size vortex sheet list for this level
       
       NumberOfVortexSheetsForLevel_[Level] = k;
       
       VortexSheetListForLevel_[Level] = new VORTEX_SHEET[k + 1];
     
       // Size trailing vortex list for this level
       
       NumberOfTrailingVorticesForLevel_[Level] = k + 1;
       
       TrailingVortexListForLevel_[Level] = new VORTEX_TRAIL*[k + 2];
     
       j = k;
       
       k /= 2;
       
       if ( 2 * k != j ) k++;
       
    }
    
    // Set up the vortex sheets for each sub level, along with children pointers
    // Level = 1 is the finest level... it contains all the trailing vortices...
    // NumberOfLevels_ is the coarsest level... 
        
    m = 1;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
       
       k = 0;
       
       for ( j = 1 ; j <= NumberOfTrailingVortices_ + 1 ; j+=m ) {
       
          k++;
    
          TrailingVortexListForLevel_[Level][k] = &(TrailingVortexList_[j]);
   
       }
       
       j -= m;
       
       if ( j != NumberOfTrailingVortices_ + 1 ) {
          
          j = NumberOfTrailingVortices_ + 1;
          
          k++;

          TrailingVortexListForLevel_[Level][k] = &(TrailingVortexList_[j]);

       }
       
       if ( k != NumberOfTrailingVorticesForLevel_[Level] ) {
          
          printf("wtf! \n");
          printf("Level: %d ... k: %d ... NumberOfTrailingVorticesForLevel_[%d]: %d \n",
          Level,
          k, 
          Level,
          NumberOfTrailingVorticesForLevel_[Level]);
          
          fflush(NULL);
          
          exit(1);
          
       }
       
       m *= 2;
       
    }
 
    // Set up the vortex sheets for each sub level, along with children pointers
     
    m = 1;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
       
       k = 0;
       
       p = 1;
       
       for ( j = 1 ; j <= NumberOfTrailingVortices_  ; j+=m ) {
       
          k++;
    
          // Set the two trailing vortices for this sheet
          
          VortexSheetListForLevel_[Level][k].SetTrailingVortices(*TrailingVortexListForLevel_[Level][k], 
                                                                 *TrailingVortexListForLevel_[Level][k+1]);
                       
          VortexSheetListForLevel_[Level][k].TimeAccurate() = TimeAccurate_;
          VortexSheetListForLevel_[Level][k].TimeStep()     = TimeStep_;
          VortexSheetListForLevel_[Level][k].Vinf()         = Vinf_;
                                                                           
          VortexSheetListForLevel_[Level][k].Setup();
          
          // Set the two vortex sheet children for this sheet
          
          if ( Level > 1 ) {
           
             if ( p + 1 <= NumberOfVortexSheetsForLevel_[Level-1] ) {
                
                VortexSheetListForLevel_[Level][k].SetupChildren(VortexSheetListForLevel_[Level-1][p  ],
                                                                 VortexSheetListForLevel_[Level-1][p+1]);
                                                     
             }
             
             else {
                
                VortexSheetListForLevel_[Level][k].SetupChildren(VortexSheetListForLevel_[Level-1][p  ]);
                                                           
             }                
                                                              
          }
          
          p += 2;
          
       }
    
       j -= m;

       if ( k != NumberOfVortexSheetsForLevel_[Level] ) {
          
          printf("wtf! \n");
          printf("Level: %d ... k: %d ... NumberOfVortexSheetsForLevel_[%d]: %d \n",
          Level,
          k, 
          Level,
          NumberOfVortexSheetsForLevel_[Level]);
          
          fflush(NULL);
          
          exit(1);
          
       }
       
       m *= 2;
       
    }  
    
    NumberOfStartingVortices_ = VortexSheetListForLevel_[1][1].NumberOfSubVortices();

    NumberOfSubVortices_ = VortexSheetListForLevel_[1][1].NumberOfSubVortices();       
 
    fflush(NULL);
        
}

/*##############################################################################
#                                                                              #
#                       VORTEX_SHEET SaveVortexState                           #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SaveVortexState(void)
{

    int i, NumberOfTrailingVortices;
    
    // Save the current trailing vortex state
    
    NumberOfTrailingVortices = NumberOfTrailingVortices_;
    
    if ( IsPeriodic_ ) NumberOfTrailingVortices++;
    
    for ( i = 1 ; i <= NumberOfTrailingVortices  ; i++ ) {
       
       TrailingVortexList_[i].SaveVortexState();

    }   

}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET UpdateVortexStrengths                    #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::UpdateVortexStrengths(int UpdateType)
{

    int i, j, NumberOfTrailingVortices;
    
    // Update strengths along entire trailing wake
    
    NumberOfTrailingVortices = NumberOfTrailingVortices_;
    
    if ( IsPeriodic_ ) NumberOfTrailingVortices++;
        
    if ( !TimeAccurate_ ) {

       // Steady state... trailing edge value convected to infinity
       
       for ( i = 1 ; i <= NumberOfTrailingVortices ; i++ ) {
      
          for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
   
             TrailingVortexList_[i].Gamma(j) = TrailingVortexList_[i].Gamma();
   
          }
          
       }
       
    }
    
    // Convect the vorticity
    
    else {
        
       // Time accurate... so convect trailing edge value one time step
       
       for ( i = 1 ; i <= NumberOfTrailingVortices ; i++ ) {
          
          TrailingVortexList_[i].MaxConvectedDistance() = MaxConvectedDistance_;
          
          TrailingVortexList_[i].ConvectWakeVorticity(UpdateType);    
      
       } 
       
    }
    
    // Make a copy of the circulation strengths

    for ( i = 1 ; i <= NumberOfTrailingVortices  ; i++ ) {

       for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {

          TrailingGamma_[i][j] = TrailingVortexList_[i].Gamma(j);
     
       }

    } 
     
    if ( TimeAccurate_ ) {
                 
       // Zero out starting vortices
       
       for ( i = 0 ; i <= NumberOfTrailingVortices  ; i++ ) {
          
          for ( j = 0 ; j <= NumberOfSubVortices() ; j++ ) {
   
             StartingGamma_[i][j] = 0.;
             
          }
      
       } 
     
       // Integrate along span to calculate starting vortices strengths
   
       for ( i = 1 ; i <= NumberOfTrailingVortices  ; i++ ) {
          
          for ( j = 1 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
   
             StartingGamma_[i][j] = StartingGamma_[i-1][j] + TrailingGamma_[i][j];
   
          }
          
       }  
       
       // Now calculate delta-gammas
       
       for ( i = 1 ; i <= NumberOfTrailingVortices  ; i++ ) {
          
          for ( j = 1 ; j < NumberOfSubVortices() + 1 ; j++ ) {
   
             StartingGamma_[i][j] -= StartingGamma_[i][j+1];
   
          }   
                 
       }    
       
    }      
        
}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET InducedVelocity                          #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::InducedVelocity(double xyz_p[3], double q[3])
{

    int i, j, k, NumberOfTrailingVortices;
    double U, V, W, dq[3];
    double TotalTime;
    VORTEX_TRAIL *TrailingVortex;
    VORTEX_SHEET *VortexSheet;


    // Zero evaluation level and initialize trailing and shed vortex gammas

    NumberOfTrailingVortices = NumberOfTrailingVortices_;
    
    if ( IsPeriodic_ ) NumberOfTrailingVortices++;
   
    if ( !TimeAccurate_ ) {
       
       for ( i = 1 ; i <= NumberOfTrailingVortices ; i++ ) {
   
          TrailingVortexList_[i].Evaluate() = 1;
     
          TrailingVortexList_[i].Gamma() = TrailingGamma_[i][0];
  
       }   
  
    } 

    else {
       
       for ( i = 1 ; i <= NumberOfTrailingVortices ; i++ ) {
   
          TrailingVortexList_[i].Evaluate() = 1;
          
          for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
          
             TrailingVortexList_[i].Gamma(j) = TrailingGamma_[i][j];
                       
          }
          
       }    
           
       for ( i = 1 ; i <= NumberOfTrailingVortices - 1 ; i++ ) {
          
          for ( j = 1 ; j <= NumberOfSubVortices() ; j++ ) {
   
             VortexSheetListForLevel_[1][i].StartingVortexList(j).Gamma() = StartingGamma_[i][j];
             
          }
          
       }
       
    }

    // If the vortex sheet is periodic, the first and last trailing vortex are the
    // same... we double book keep for closure, and 1/2 the gamma for each
    
    if ( IsPeriodic_ ) {
       
       for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
     
          TrailingVortexList_[                            1].Gamma(j) = 0.5 * TrailingGamma_[1][j];
       
          TrailingVortexList_[NumberOfTrailingVortices_ + 1].Gamma(j) = 0.5 * TrailingGamma_[1][j];

       }
       
    }

    // Agglomerate the trailing vortices .. we start at the coarsest level

    if ( NumberOfTrailingVortices_ >= 4 ) {

       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[NumberOfLevels_] ; i++ ) {
          
          VortexSheet = &VortexSheetListForLevel_[NumberOfLevels_][i];

          CreateTrailingVortexInteractionList(*VortexSheet, xyz_p);
          
       }
       
    }

    // Create a unique list of the agglomerated trailing vortices

    NumberOfAgglomeratedTrailingVortices_ = 0;
   
    for ( i = 1 ; i <= NumberOfTrailingVortices ; i++ ) {

       if ( TrailingVortexList_[i].Evaluate() ) {
      
          AgglomeratedTrailingVortexList_[++NumberOfAgglomeratedTrailingVortices_] = &TrailingVortexList_[i];
          
       }
       
    }    

    // Evalulate the agglomerated trailing vortices
    
    q[0] = q[1] = q[2] = U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(dq,TrailingVortex)      
    for ( i = 1 ; i <= NumberOfAgglomeratedTrailingVortices_ ; i++ ) {

       TrailingVortex = AgglomeratedTrailingVortexList_[i];
      
       TrailingVortex->InducedVelocity(xyz_p,dq);
              
       U += dq[0];
       V += dq[1];
       W += dq[2];

    } 

    // If this is an unsteady solution, we have to evaluate the starting
    // vortices for all the previous time steps

    if ( TimeAccurate_ ) {

       // Start at the coarsest level
       
#pragma omp parallel for reduction(+:U,V,W) private(dq,VortexSheet)             
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[NumberOfLevels_] ; i++ ) {
          
          VortexSheet = &VortexSheetListForLevel_[NumberOfLevels_][i];
          
          dq[0] = dq[1] = dq[2] = 0.;
       
          StartingVorticesInducedVelocity(*VortexSheet, xyz_p, dq);
          
          U += dq[0];
          V += dq[1];
          W += dq[2];          
          
       } 
       
    }    
    
    q[0] = U;
    q[1] = V;
    q[2] = W;   
    
}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET InducedVelocity                          #
#                                                                              #
#  This version does some extra checking to make sure we don't do an           #
#  evaluation of a trailing vortex induced velocity on itself... this is used  #
#  by the wake update location routines.                                       #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::InducedVelocity(double xyz_p[3], double q[3], double xyz_te[3])
{

    int i, j;
    double U, V, W, dq[3], Dist;
    VORTEX_TRAIL *TrailingVortex;
    VORTEX_SHEET *VortexSheet;
 
    // Zero evaluation level and initialize trailing vortex gammas to finest level
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_  ; i++ ) {

       TrailingVortexList_[i].Evaluate() = 1;
       
       for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
       
          TrailingVortexList_[i].Gamma(j) = TrailingGamma_[i][j];
                    
       }
       
    }    
        
    // Initialize the starting vortex gammas to the finest level

    if ( TimeAccurate_ ) {

       for ( i = 1 ; i <= NumberOfTrailingVortices_ - 1 ; i++ ) {
          
          for ( j = 1 ; j <= NumberOfSubVortices() ; j++ ) {
   
             VortexSheetListForLevel_[1][i].StartingVortexList(j).Gamma() = StartingGamma_[i][j];
             
          }
          
       }        
       
    }
    
    // If the vortex sheet is periodic, the first and last trailing vortex are the
    // same... we double book keep for closure, and 1/2 the gamma for each
    
    if ( IsPeriodic_ ) {
       
       for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
       
          TrailingVortexList_[                            1].Gamma(j) = 0.5 * TrailingGamma_[1][j];
       
          TrailingVortexList_[NumberOfTrailingVortices_ + 1].Gamma(j) = 0.5 * TrailingGamma_[1][j];
          
       }
       
    }
       
    // Agglomerate the trailing vortices .. we start at the coarsest level
    
    if ( NumberOfTrailingVortices_ >= 4 ) {

       // Start at the coarsest level (NumberOfLevels_)
       
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[NumberOfLevels_] ; i++ ) {
          
          VortexSheet = &VortexSheetListForLevel_[NumberOfLevels_][i];
   
          CreateTrailingVortexInteractionList(*VortexSheet, xyz_p);
          
       }
       
    }
    
    // Create a unique list of the agglomerated trailing vortices

    NumberOfAgglomeratedTrailingVortices_ = 0;

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       if ( TrailingVortexList_[i].Evaluate() ) {
          
          Dist = sqrt( pow(xyz_te[0] - TrailingVortexList_[i].TE_Node().x(),2.)
                     + pow(xyz_te[1] - TrailingVortexList_[i].TE_Node().y(),2.)
                     + pow(xyz_te[2] - TrailingVortexList_[i].TE_Node().z(),2.) );
                     
          // Don't do self induced velocities                     
          
          if ( Dist >= 0.5*TrailingVortexList_[i].Sigma() ) AgglomeratedTrailingVortexList_[++NumberOfAgglomeratedTrailingVortices_] = &TrailingVortexList_[i];
          
       }
       
    }    

    // Evalulate the agglomerated trailing vortices
    
    q[0] = q[1] = q[2] = U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(dq,TrailingVortex)      
    for ( i = 1 ; i <= NumberOfAgglomeratedTrailingVortices_ ; i++ ) {

       TrailingVortex = AgglomeratedTrailingVortexList_[i];
      
       TrailingVortex->InducedVelocity(xyz_p,dq);
  
       U += dq[0];
       V += dq[1];
       W += dq[2];

    } 
    
    // If this is an unsteady solution, we have to evaluate the starting
    // vortices for all the previous time steps

    if ( TimeAccurate_ ) {

       // Loop over the vortex sheets for the finest level

#pragma omp parallel for reduction(+:U,V,W) private(dq,VortexSheet)                    
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[NumberOfLevels_] ; i++ ) {
          
          VortexSheet = &VortexSheetListForLevel_[NumberOfLevels_][i];
          
          dq[0] = dq[1] = dq[2] = 0.;
       
          StartingVorticesInducedVelocity(*VortexSheet, xyz_p, dq);
          
          U += dq[0];
          V += dq[1];
          W += dq[2];          
          
       } 
       
    }        
                
    q[0] = U;
    q[1] = V;
    q[2] = W;   
    
}

/*##############################################################################
#                                                                              #
#                   VORTEX_SHEET InducedKuttaVelocity                          #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::InducedKuttaVelocity(double xyz_p[3], double q[3])
{

    int i, j;
    double U, V, W, Vec[3], xyz_k[3], dq[3], Mag, Fact;
    VORTEX_TRAIL *TrailingVortex;
    VORTEX_SHEET *VortexSheet;

    // Zero evaluation level and initialize trailing vortex gammas to finest level
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_  ; i++ ) {

       TrailingVortexList_[i].Evaluate() = 1;
       
       for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
       
          TrailingVortexList_[i].Gamma(j) = TrailingGamma_[i][j];
                    
       }
       
    }    
        
    // Initialize the starting vortex gammas to the finest level

    if ( TimeAccurate_ ) {

       for ( i = 1 ; i <= NumberOfTrailingVortices_ - 1 ; i++ ) {
          
          for ( j = 1 ; j <= NumberOfSubVortices() ; j++ ) {
   
             VortexSheetListForLevel_[1][i].StartingVortexList(j).Gamma() = StartingGamma_[i][j];
             
          }
          
       }  
       
    }              
    
    // If the vortex sheet is periodic, the first and last trailing vortex are the
    // same... we double book keep for closure, and 1/2 the gamma for each
    
    if ( IsPeriodic_ ) {
       
       for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
       
          TrailingVortexList_[                            1].Gamma(j) = 0.5 * TrailingGamma_[1][j];
       
          TrailingVortexList_[NumberOfTrailingVortices_ + 1].Gamma(j) = 0.5 * TrailingGamma_[1][j];
          
       }
       
    }

    // Agglomerate the trailing vortices .. we start at the coarsest level

    if ( NumberOfTrailingVortices_ >= 4 ) {
   
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[NumberOfLevels_] ; i++ ) {
          
          VortexSheet = &VortexSheetListForLevel_[NumberOfLevels_][i];

          CreateTrailingVortexInteractionList(*VortexSheet, xyz_p);
          
       } 
       
    }

    // Create a unique list of the agglomerated trailing vortices

    NumberOfAgglomeratedTrailingVortices_ = 0;
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       if ( TrailingVortexList_[i].Evaluate() ) {

          AgglomeratedTrailingVortexList_[++NumberOfAgglomeratedTrailingVortices_] = &TrailingVortexList_[i];
          
       }
       
    }    

    // Evaluate the agglomerated trailing vortices
    
    q[0] = q[1] = q[2] = U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(dq,xyz_k,TrailingVortex,Vec,Mag)      
    for ( i = 1 ; i <= NumberOfAgglomeratedTrailingVortices_ ; i++ ) {

       TrailingVortex = AgglomeratedTrailingVortexList_[i];
       
       // Distance from point to TE
       
       Vec[0] = TrailingVortex->TE_Node().x() - xyz_p[0];
       Vec[1] = TrailingVortex->TE_Node().y() - xyz_p[1];
       Vec[2] = TrailingVortex->TE_Node().z() - xyz_p[2];
       
       Mag = vector_dot(Vec,FreeStreamVelocity_); 
       
       // Shift the x location to the wing trailing edge of this trailing vortex

       xyz_k[0] = xyz_p[0] + Mag * FreeStreamVelocity_[0];
       xyz_k[1] = xyz_p[1] + Mag * FreeStreamVelocity_[1];
       xyz_k[2] = xyz_p[2] + Mag * FreeStreamVelocity_[2];

       TrailingVortex->InducedVelocity(xyz_k,dq);
       
       Vec[0] = TrailingVortex->TE_Node().x() - xyz_k[0];
       Vec[1] = TrailingVortex->TE_Node().y() - xyz_k[1];
       Vec[2] = TrailingVortex->TE_Node().z() - xyz_k[2];
       
       Mag = vector_dot(Vec,Vec); 
              
       if ( Mag > 0.01*TrailingVortex->Sigma() ) {
                 
          U += dq[0];
          V += dq[1];
          W += dq[2];
          
       }

    } 

    // Approximate KT correction
    
    if ( !TrailingVortex->NoKarmanTsienCorrection() ) {
       
       U *= pow(1.+0.25*TrailingVortex->Mach()*TrailingVortex->Mach(),2.);
       V *= pow(1.+0.25*TrailingVortex->Mach()*TrailingVortex->Mach(),2.);
       W *= pow(1.+0.25*TrailingVortex->Mach()*TrailingVortex->Mach(),2.);

    }
    
    // If this is an unsteady solution, we have to evaluate the starting
    // vortices for all the previous time steps

    if ( TimeAccurate_ ) {

       // Loop over the vortex sheets for the finest level
#pragma omp parallel for reduction(+:U,V,W) private(dq,xyz_k,VortexSheet)                        
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[NumberOfLevels_] ; i++ ) {
          
          VortexSheet = &VortexSheetListForLevel_[NumberOfLevels_][i];

          dq[0] = dq[1] = dq[2] = 0.;
          
          xyz_k[0] = VortexSheet->Child1().VortexTrail1().TE_Node().x();
          xyz_k[1] = xyz_p[1];
          xyz_k[2] = xyz_p[2];          
       
          StartingVorticesInducedVelocity(*VortexSheet, xyz_k, dq);
          
          U += dq[0];
          V += dq[1];
          W += dq[2];
       
       } 
       
    }     

    q[0] = U;
    q[1] = V;
    q[2] = W;   
    
}

/*##############################################################################
#                                                                              #
#              VORTEX_SHEET CreateTrailingVortexInteractionList                #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::CreateTrailingVortexInteractionList(VORTEX_SHEET &VortexSheet, double xyz_p[3])
{

    int NumChildren;
    
    NumChildren = 0;
    
    if ( VortexSheet.ThereAreChildren() == 1 ) NumChildren = 1;
    
    if ( VortexSheet.ThereAreChildren() == 2 ) NumChildren = 2;
    
    if ( VortexSheet.FarAway(xyz_p) || NumChildren == 0 ) {

       VortexSheet.InjectCirculation();
       
       VortexSheet.Evaluate() = 1;

    }
    
    else {

       VortexSheet.Evaluate() = 0;

       if ( NumChildren >= 1 ) CreateTrailingVortexInteractionList(VortexSheet.Child1(), xyz_p);

       if ( NumChildren == 2 ) CreateTrailingVortexInteractionList(VortexSheet.Child2(), xyz_p);

    }
  
}

/*##############################################################################
#                                                                              #
#                       VORTEX_SHEET InjectCirculation                         #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::InjectCirculation(void)
{
    int j;
    
    if ( ThereAreChildren_ == 2 ) {

       // Recursively inject circulations up to the children level
       
       Child1().InjectCirculation();
       
       Child2().InjectCirculation();
       
       // Now inject the children to this level
              
       for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
       
          VortexTrail1().Gamma(j) += 0.5*Child1().VortexTrail2().Gamma(j);
    
          VortexTrail2().Gamma(j) += 0.5*Child1().VortexTrail2().Gamma(j);
          
       }
       
       // Inject the starting vortices as well
     
       if ( TimeAccurate_ ) {
   
          for ( j = 1 ; j <= NumberOfSubVortices() ; j++ ) {
    
             StartingVortexList(j).Gamma() = 0.5*Child1().StartingVortexList(j).Gamma() + 0.5*Child2().StartingVortexList(j).Gamma();
   
          }
   
       }
           
       // Turn off evaluation of the center trailing vortex (child 1 - vortex 2)                    
   
       Child1().VortexTrail2().Evaluate() = 0;
       
       Child1().Evaluate() = 0;
       
       Child2().Evaluate() = 0;
      
    }
    
    else if ( ThereAreChildren_ == 1 ) {
       
       Child1().InjectCirculation();
       
       Child1().Evaluate() = 0;
            
    }
    
    else {
       
       Evaluate() = 1;
       
       // Nothing to do... Evaluate is set to 1 by default
       
    }
  
}

/*##############################################################################
#                                                                              #
#            VORTEX_SHEET StartingVorticesInducedVelocity                      #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::StartingVorticesInducedVelocity(VORTEX_SHEET &VortexSheet, double xyz_p[3], double dq[3])
{

    int j;
    double q[3];

    if ( VortexSheet.Evaluate() == 1 ) {

      // Calculate all shed bound vortices for the vortex sheet

      for ( j = 1 ; j <= VortexSheet.NumberOfStartingVortices() ; j++ ) {

         if ( VortexSheet.StartingVortexList(j).S() <= MaxConvectedDistance_ ) {
            
            VortexSheet.StartingVortexList(j).InducedVelocity(xyz_p, q);
            
            dq[0] += q[0];
            dq[1] += q[1];
            dq[2] += q[2];
            
         }

      }

    }
    
    else {

       if ( VortexSheet.ThereAreChildren() >= 1 ) StartingVorticesInducedVelocity(VortexSheet.Child1(), xyz_p, dq);

       if ( VortexSheet.ThereAreChildren() >= 2 ) StartingVorticesInducedVelocity(VortexSheet.Child2(), xyz_p, dq);

    }
  
}


/*##############################################################################
#                                                                              #
#                            VORTEX_SHEET Setup                                #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::Setup(void)
{

    int i;
    VSP_NODE NodeA, NodeB;

    // Span of this vortex sheet
    
    Span_ = sqrt( pow(VortexTrail1().TE_Node().y() - VortexTrail2().TE_Node().y(),2.)
                + pow(VortexTrail1().TE_Node().z() - VortexTrail2().TE_Node().z(),2.) );
    
    // Mid span location
               
    MidSpan_[0] = 0.5*( VortexTrail1().TE_Node().x() + VortexTrail2().TE_Node().x() );         
    MidSpan_[1] = 0.5*( VortexTrail1().TE_Node().y() + VortexTrail2().TE_Node().y() ); 
    MidSpan_[2] = 0.5*( VortexTrail1().TE_Node().z() + VortexTrail2().TE_Node().z() ); 
    
    // Unsteady parameters
    
    VortexTrail1().TimeAccurate() = VortexTrail2().TimeAccurate() = TimeAccurate_;
    
    VortexTrail1().TimeStep() = VortexTrail2().TimeStep() = TimeStep_;
    
    VortexTrail1().Vinf() = VortexTrail2().Vinf() = Vinf_;
     
    // Starting vortex list
    
    NumberOfStartingVortices_ = VortexTrail1().NumberOfSubVortices();

    StartingVortexList_ = new VSP_EDGE[NumberOfStartingVortices_ + 1];
    
    NumberOfSubVortices_ = VortexTrail1().NumberOfSubVortices();

    for ( i = 1 ; i <= NumberOfStartingVortices_ ; i++ ) {
       
       NodeA.x() = VortexTrail1().VortexEdge(i).X2();
       NodeA.y() = VortexTrail1().VortexEdge(i).Y2();
       NodeA.z() = VortexTrail1().VortexEdge(i).Z2();
      
       NodeB.x() = VortexTrail2().VortexEdge(i).X2();
       NodeB.y() = VortexTrail2().VortexEdge(i).Y2();
       NodeB.z() = VortexTrail2().VortexEdge(i).Z2();
       
       StartingVortexList_[i].DegenWing()      = VortexTrail1().Wing();
       
       StartingVortexList_[i].Node()           = VortexTrail1().Node();
       
       StartingVortexList_[i].IsTrailingEdge() = 0;
       
       StartingVortexList_[i].Sigma()          = VortexTrail1().Sigma();
       
       StartingVortexList_[i].S()              = VortexTrail1().S(i);
   
       StartingVortexList_[i].Setup(NodeA, NodeB);      
       
    }
                   
}

/*##############################################################################
#                                                                              #
#                            VORTEX_SHEET FarAway                              #
#                                                                              #
##############################################################################*/

int VORTEX_SHEET::FarAway(double xyz_[3])
{

    double Distance;

    Distance = sqrt( pow(xyz_[1] - MidSpan_[1],2.) 
                   + pow(xyz_[2] - MidSpan_[2],2.) );
                   
    if ( Distance / Span_ > 2. ) return 1;
    
    return 0;               

}























