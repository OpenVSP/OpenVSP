#include "Vortex_Sheet.H"
#include "Vortex_Sheet.H"

/*##############################################################################
#                                                                              #
#                      Allocate space for statics                              #
#                                                                              #
##############################################################################*/

    double VORTEX_SHEET::FarAway_ = 5.;

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
    
    Level_ = 0;
     
    NumberOfLevels_ = 0;

    NumberOfTrailingVortices_ = 0;
    
    NumberOfSubVortices_ = 0;
    
    NumberOfStartingVortices_ = 0;
    
    ThereAreChildren_ = 0 ;
    
    TimeAccurate_ = 0;
    
    QuasiTimeAccurate_ = 1;
    
    Evaluate_ = 0;
    
    DoGroundEffectsAnalysis_ = 0;
    
    CurrentTimeStep_ = 0;
    
    CoreSize_ = 0.;
    
    Distance_ = 0.;
    
    TrailingGamma_ = NULL;
    
    StartingGamma_ = NULL;

    VortexTrailingGamma1_ = NULL;
    
    VortexTrailingGamma2_ = NULL;

    TrailingGammaListForLevel_ = NULL;

    TrailingVortexList_ = NULL;
    
    AgglomeratedTrailingVortexList_ = NULL;

    NumberOfTrailingVorticesForLevel_ = NULL;
    
    NumberOfVortexSheetsForLevel_ = NULL;
    
    TrailingVortexListForLevel_ = NULL;
    
    VortexSheetListForLevel_ = NULL;
  
    VortexTrail1_ = NULL;
    
    VortexTrail2_ = NULL;

    Child1_ = NULL;
    
    Child2_ = NULL;
    
    NumberOfCommonTE_ = 0;
    
    NumberOfCommonNodesForTE_ = NULL;
    
    CommonTEList_ = NULL;
    
    IsARotor_ = 0;
    
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
    
    Level_                    = VortexSheet.Level_;
     
    NumberOfLevels_           = VortexSheet.NumberOfLevels_;

    NumberOfTrailingVortices_ = VortexSheet.NumberOfTrailingVortices_;
    
    NumberOfStartingVortices_ = VortexSheet.NumberOfStartingVortices_;

    NumberOfSubVortices_      = VortexSheet.NumberOfSubVortices_;

    TimeAccurate_             = VortexSheet.TimeAccurate_;
    
    QuasiTimeAccurate_        = VortexSheet.QuasiTimeAccurate_;

    TimeAnalysisType_         = VortexSheet.TimeAnalysisType_;
 
    DoGroundEffectsAnalysis_  = VortexSheet.DoGroundEffectsAnalysis_;
    
    Vinf_                     = VortexSheet.Vinf_;
    
    FreeStreamVelocity_[0]    = VortexSheet.FreeStreamVelocity_[0];
    
    FreeStreamVelocity_[1]    = VortexSheet.FreeStreamVelocity_[1];
    
    FreeStreamVelocity_[2]    = VortexSheet.FreeStreamVelocity_[2];

    TimeStep_                 = VortexSheet.TimeStep_;
   
    CurrentTimeStep_          = VortexSheet.CurrentTimeStep_;
   
    ThereAreChildren_         = VortexSheet.ThereAreChildren_;
    
    CoreSize_                 = VortexSheet.CoreSize_;
    
    Distance_                 = VortexSheet.Distance_;

    Span_                     = VortexSheet.Span_;

    Evaluate_                 = VortexSheet.Evaluate_;
     
    VortexTrail1_             = VortexSheet.VortexTrail1_;
    
    VortexTrail2_             = VortexSheet.VortexTrail2_;
     
    Child1_                   = VortexSheet.Child1_;
    
    Child2_                   = VortexSheet.Child2_;
    
    IsARotor_                 = VortexSheet.IsARotor_;

    // Trailing vortex list

    TrailingVortexList_ = new VORTEX_TRAIL*[NumberOfTrailingVortices_ + 2];
  
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
       
       TrailingVortexList_[i] = new VORTEX_TRAIL;

       *(TrailingVortexList_[i]) = *(VortexSheet.TrailingVortexList_[i]);
       
    }

    // Trailing and starting gammas
    
    TrailingGamma_ = new VSPAERO_DOUBLE*[NumberOfTrailingVortices_ + 3]; 
    
    StartingGamma_ = new VSPAERO_DOUBLE*[NumberOfTrailingVortices_ + 3]; 

    // Size trailing and starting gammas lists

    for ( i = 0 ; i <= NumberOfTrailingVortices_ + 2 ; i++ ) {

       TrailingGamma_[i] = new VSPAERO_DOUBLE[NumberOfSubVortices() + 2];
    
       StartingGamma_[i] = new VSPAERO_DOUBLE[NumberOfSubVortices() + 2];
             
    }

    // Size the vortex sheet list
    
    NumberOfVortexSheetsForLevel_ = new int[NumberOfLevels_ + 1];
   
    VortexSheetListForLevel_ = new VORTEX_SHEET*[NumberOfLevels_ + 1];
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
       
       VortexSheetListForLevel_[Level] = NULL;
       
    }

    // Size the trailing vortex list
    
    NumberOfTrailingVorticesForLevel_ = new int[NumberOfLevels_ + 1]; 
     
    TrailingVortexListForLevel_ = new VORTEX_TRAIL**[NumberOfLevels_ + 1];

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
     
       // Size vortex sheet list for this level
       
       NumberOfVortexSheetsForLevel_[Level] = k = VortexSheet.NumberOfVortexSheetsForLevel_[Level];
       
       VortexSheetListForLevel_[Level] = new VORTEX_SHEET[k + 1];

       // Copy over vortex sheet data

       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[Level] ; i++ ) {
          
          VortexSheetListForLevel_[Level][i] = VortexSheet.VortexSheetListForLevel_[Level][i];
          
       }
      
       // Size trailing vortex list for this level
       
       NumberOfTrailingVorticesForLevel_[Level] = k = VortexSheet.NumberOfTrailingVorticesForLevel_[Level];
       
       TrailingVortexListForLevel_[Level] = new VORTEX_TRAIL*[k + 2];
     
       // Copy over vortex trail data

       for ( i = 1 ; i <= NumberOfTrailingVorticesForLevel_[Level] ; i++ ) {
          
          TrailingVortexListForLevel_[Level][i] = VortexSheet.TrailingVortexListForLevel_[Level][i];
          
       }
         
    }    

    // Agglomerated trailing vortex list
    
    AgglomeratedTrailingVortexList_= new VORTEX_TRAIL*[NumberOfTrailingVortices_ + 1];
    
    return *this;

}

/*##############################################################################
#                                                                              #
#                               VORTEX_SHEET operator+=                        #
#                                                                              #
##############################################################################*/

VORTEX_SHEET& VORTEX_SHEET::operator+=(const VORTEX_SHEET &VortexSheet)
{

    int i, j, Level;
    
    Verbose_                  = VortexSheet.Verbose_;
    
    WingSurface_              = VortexSheet.WingSurface_;

    IsPeriodic_               = VortexSheet.IsPeriodic_;
    
    Level_                    = VortexSheet.Level_;
     
    NumberOfLevels_           = VortexSheet.NumberOfLevels_;

    NumberOfTrailingVortices_ = VortexSheet.NumberOfTrailingVortices_;
    
    NumberOfStartingVortices_ = VortexSheet.NumberOfStartingVortices_;

    NumberOfSubVortices_      = VortexSheet.NumberOfSubVortices_;

    TimeAccurate_             = VortexSheet.TimeAccurate_;
    
    QuasiTimeAccurate_        = VortexSheet.QuasiTimeAccurate_;

    TimeAnalysisType_         = VortexSheet.TimeAnalysisType_;
 
    DoGroundEffectsAnalysis_  = VortexSheet.DoGroundEffectsAnalysis_;
    
    Vinf_                     = VortexSheet.Vinf_;
    
    FreeStreamVelocity_[0]    = VortexSheet.FreeStreamVelocity_[0];
    
    FreeStreamVelocity_[1]    = VortexSheet.FreeStreamVelocity_[1];
    
    FreeStreamVelocity_[2]    = VortexSheet.FreeStreamVelocity_[2];

    TimeStep_                 = VortexSheet.TimeStep_;
   
    CurrentTimeStep_          = VortexSheet.CurrentTimeStep_;
   
    ThereAreChildren_         = VortexSheet.ThereAreChildren_;
    
    CoreSize_                 = VortexSheet.CoreSize_;
    
    Distance_                 = VortexSheet.Distance_;

    Span_                     = VortexSheet.Span_;

    Evaluate_                 = VortexSheet.Evaluate_;
    
    IsARotor_                 = VortexSheet.IsARotor_;
    
    // Trailing vortex list

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
 
       *(TrailingVortexList_[i]) += *(VortexSheet.TrailingVortexList_[i]);
       
    }

    // Update bound vortices
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
       
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[Level] ; i++ ) {
          
          VortexSheetListForLevel_[Level][i].TimeAccurate_ = TimeAccurate_;
          
          VortexSheetListForLevel_[Level][i].QuasiTimeAccurate_ = QuasiTimeAccurate_;
          
          VortexSheetListForLevel_[Level][i].TimeAnalysisType_ = TimeAnalysisType_;
       
          VortexSheetListForLevel_[Level][i].CurrentTimeStep_ = CurrentTimeStep_;
          
          VortexSheetListForLevel_[Level][i].UpdateGeometryLocation();
          
       }
       
    }
    
    // Trailing and starting gammas

    if ( NumberOfTrailingVortices_ > 0 ) {
       
       for ( i = 0 ; i <= NumberOfTrailingVortices_ + 1 ; i++ ) {
  
          for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
   
             TrailingGamma_[i][j] = VortexSheet.TrailingGamma_[i][j];
       
             StartingGamma_[i][j] = VortexSheet.StartingGamma_[i][j];
             
          }
               
       }
       
    }

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
      
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[Level] ; i++ ) {
         
          for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
                                                         
             VortexSheetListForLevel_[Level][i].StartingGamma_[0][j] = VortexSheet.VortexSheetListForLevel_[Level][i].StartingGamma_[0][j];

          }
          
       }
     
    }

    // Agglomerated gammas

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       for ( i = 1 ; i <= NumberOfTrailingVorticesForLevel_[Level] ; i++ ) {
    
          for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
 
             TrailingGammaListForLevel_[Level][i][j] = VortexSheet.TrailingGammaListForLevel_[Level][i][j];

          }

       }
             
    }
   
    // Agglomerated bound vortex data structure

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
      
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[Level] ; i++ ) {   
             
          for ( j = 1 ; j <= NumberOfSubVortices() + 1 ; j++ ) {  

              VortexSheetListForLevel_[Level][i].BoundVortex().Gamma(j) = VortexSheetListForLevel_[Level][i].StartingGamma(j);
            
          }
          
          VortexSheetListForLevel_[Level][i].BoundVortex().UpdateGamma();
  
          VortexSheetListForLevel_[Level][i].BoundVortex().CurrentTimeStep() = CurrentTimeStep_;
          
       }
       
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

    if ( NumberOfTrailingVortices_ != 0 ) {

       if ( TrailingVortexList_ != NULL ) {

          for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
             
             delete TrailingVortexList_[i];
             
          }
          
          delete [] TrailingVortexList_;

       }

       for ( i = 0 ; i <= NumberOfTrailingVortices_ ; i++ ) {
    
           // Trailing and starting gammas
   
           if ( TrailingGamma_[i] != NULL ) delete [] TrailingGamma_[i];
        
           if ( StartingGamma_[i] != NULL ) delete [] StartingGamma_[i];

       }  

       if ( TrailingGamma_ != NULL ) delete [] TrailingGamma_;
        
       if ( StartingGamma_ != NULL ) delete [] StartingGamma_;
       
    } 

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       if ( VortexSheetListForLevel_[Level] != NULL ) delete [] VortexSheetListForLevel_[Level];

       if ( TrailingVortexListForLevel_[Level] != NULL ) delete [] TrailingVortexListForLevel_[Level];

    }

    if ( VortexSheetListForLevel_ != NULL ) delete [] VortexSheetListForLevel_;

    if ( TrailingVortexListForLevel_ != NULL ) delete [] TrailingVortexListForLevel_;
  
    if ( AgglomeratedTrailingVortexList_ != NULL ) delete [] AgglomeratedTrailingVortexList_;

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       for ( i = 1 ; i <= NumberOfTrailingVorticesForLevel_[Level] ; i++ ) {

          if ( TrailingGammaListForLevel_[Level][i] != NULL ) delete [] TrailingGammaListForLevel_[Level][i];
          
       }
       
       if ( TrailingGammaListForLevel_[Level] != NULL ) delete [] TrailingGammaListForLevel_[Level];

    }
      
    if ( TrailingGammaListForLevel_ != NULL ) delete [] TrailingGammaListForLevel_;
  
    if ( NumberOfTrailingVorticesForLevel_ != NULL ) delete [] NumberOfTrailingVorticesForLevel_;
     
    if ( NumberOfVortexSheetsForLevel_ != NULL ) delete [] NumberOfVortexSheetsForLevel_;
  
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

       if ( TrailingVortexList_ != NULL ) {

          for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
             
             delete [] TrailingVortexList_[i];
             
          }
          
          delete [] TrailingVortexList_;
          
          TrailingVortexList_ = NULL;
          
       }

       for ( i = 0 ; i <= NumberOfTrailingVortices_ ; i++ ) {

           // Trailing and starting gammas

           if ( TrailingGamma_[i] != NULL ) delete [] TrailingGamma_[i];
           
           if ( StartingGamma_[i] != NULL ) delete [] StartingGamma_[i];

       }  
     
       if ( TrailingGamma_ != NULL ) delete [] TrailingGamma_;
    
       if ( StartingGamma_ != NULL ) delete [] StartingGamma_;
       
       TrailingGamma_ = NULL;
       
       StartingGamma_ = NULL;
       
    }

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       if ( VortexSheetListForLevel_[Level] != NULL ) delete [] VortexSheetListForLevel_[Level];

       if ( TrailingVortexListForLevel_[Level] != NULL ) delete [] TrailingVortexListForLevel_[Level];
  
    }

    if ( VortexSheetListForLevel_ != NULL ) delete [] VortexSheetListForLevel_;
    
    if ( TrailingVortexListForLevel_ != NULL ) delete [] TrailingVortexListForLevel_;
    
    VortexSheetListForLevel_ = NULL;
    
    TrailingVortexListForLevel_ = NULL;

    // Resize 
   
    NumberOfTrailingVortices_ = NumberOfTrailingVortices;
  
    TrailingVortexList_ = new VORTEX_TRAIL*[NumberOfTrailingVortices_ + 2]; // Allocate 1 extra in case we are periodic
    
    // Allocate space for trailing vortices - but not the possibly periodic one
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
       
       TrailingVortexList_[i] = new VORTEX_TRAIL;
       
    }
   
    TrailingGamma_ = new VSPAERO_DOUBLE*[NumberOfTrailingVortices_ + 3]; 
   
    StartingGamma_ = new VSPAERO_DOUBLE*[NumberOfTrailingVortices_ + 3]; 

    AgglomeratedTrailingVortexList_= new VORTEX_TRAIL*[NumberOfTrailingVortices_ + 1];
          
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
    
    // Size trailing and starting gammas lists

    for ( i = 0 ; i <= NumberOfTrailingVortices_ + 2 ; i++ ) {

       TrailingGamma_[i] = new VSPAERO_DOUBLE[NumberOfSubVortices() + 2];
       
       zero_double_array(TrailingGamma_[i], NumberOfSubVortices() + 1);
       
       StartingGamma_[i] = new VSPAERO_DOUBLE[NumberOfSubVortices() + 2];
 
       zero_double_array(StartingGamma_[i], NumberOfSubVortices() + 1);
                    
    }
    
    // Calculate bounding box of trailing edges
    
    TEBox_.x_min =  1.e9;
    TEBox_.x_max = -1.e9;
    
    TEBox_.y_min =  1.e9;
    TEBox_.y_max = -1.e9;
    
    TEBox_.z_min =  1.e9;
    TEBox_.z_max = -1.e9;
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
       
       TEBox_.x_min = MIN(TEBox_.x_min, TrailingVortexList_[i]->TE_Node().x());
       TEBox_.x_max = MAX(TEBox_.x_max, TrailingVortexList_[i]->TE_Node().x());

       TEBox_.y_min = MIN(TEBox_.y_min, TrailingVortexList_[i]->TE_Node().y());
       TEBox_.y_max = MAX(TEBox_.y_max, TrailingVortexList_[i]->TE_Node().y());

       TEBox_.z_min = MIN(TEBox_.z_min, TrailingVortexList_[i]->TE_Node().z());
       TEBox_.z_max = MAX(TEBox_.z_max, TrailingVortexList_[i]->TE_Node().z());
       
    }

}

/*##############################################################################
#                                                                              #
#                  VORTEX_SHEET SetupPlanarVortexSheets                        #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SetupPlanarVortexSheets(void)
{

    int j, k, m, p, Level, Done;

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
    
    TrailingGammaListForLevel_ = new VSPAERO_DOUBLE**[NumberOfLevels_ + 1];
    
    k = NumberOfTrailingVortices_ - 1;
    
    TotalNumberOfVortexSheets_ = 0;

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
     
       // Size vortex sheet list for this level
       
       NumberOfVortexSheetsForLevel_[Level] = k;
       
       TotalNumberOfVortexSheets_ += k;
       
       VortexSheetListForLevel_[Level] = new VORTEX_SHEET[k + 1];

       // Size trailing vortex and gamma list for this level
       
       NumberOfTrailingVorticesForLevel_[Level] = k + 1;
       
       TrailingVortexListForLevel_[Level] = new VORTEX_TRAIL*[k + 2];
       
       TrailingGammaListForLevel_[Level] = new VSPAERO_DOUBLE*[k + 2];

       j = k;
       
       k /= 2;
       
       if ( 2 * k != j ) k++;
       
    }
    
    // Set up the trailing vortices for each sub level
    
    m = 1;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
       
       k = 0;
       
       for ( j = 1 ; j <= NumberOfTrailingVortices_ ; j+=m ) {
       
          k++;
    
          TrailingVortexListForLevel_[Level][k] = TrailingVortexList_[j];
          
          TrailingGammaListForLevel_[Level][k] = new VSPAERO_DOUBLE[TrailingVortexList_[j]->NumberOfSubVortices() + 3];
          
       }
       
       j -= m;
       
       if ( j != NumberOfTrailingVortices_ ) {
          
          j = NumberOfTrailingVortices_;
          
          k++;
          
          TrailingVortexListForLevel_[Level][k] = TrailingVortexList_[j];
          
          TrailingGammaListForLevel_[Level][k] = new VSPAERO_DOUBLE[TrailingVortexList_[j]->NumberOfSubVortices() + 3];
          
       }
       
       if ( k != NumberOfTrailingVorticesForLevel_[Level] ) {
          
          PRINTF("wtf! \n");
          PRINTF("Level: %d ... k: %d ... NumberOfTrailingVorticesForLevel_[%d]: %d \n",
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
                                                                 
          // Set the trailing gammas;
          
          VortexSheetListForLevel_[Level][k].SetVortexGammas(TrailingGammaListForLevel_[Level][k],
                                                             TrailingGammaListForLevel_[Level][k+1]);
                             
          // Set various settings / values
          
          VortexSheetListForLevel_[Level][k].TimeAccurate()     = TimeAccurate_;
          VortexSheetListForLevel_[Level][k].QuasiTimeAccurate_ = QuasiTimeAccurate_;
          VortexSheetListForLevel_[Level][k].TimeAnalysisType() = TimeAnalysisType_;
          VortexSheetListForLevel_[Level][k].TimeStep()         = TimeStep_;
          VortexSheetListForLevel_[Level][k].Vinf()             = Vinf_;
          VortexSheetListForLevel_[Level][k].Level()            = Level;
          VortexSheetListForLevel_[Level][k].ThereAreChildren() = 0;
                                                                 
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
          
          PRINTF("wtf! \n");
          PRINTF("Level: %d ... k: %d ... NumberOfVortexSheetsForLevel_[%d]: %d \n",
          Level,
          k, 
          Level,
          NumberOfVortexSheetsForLevel_[Level]);
          
          fflush(NULL);
          
          exit(1);
          
       }
       
       m *= 2;
       
    }    
    
    // Set unqiue sheet ID
    
    j = 0;
   
    for ( Level = NumberOfLevels_ ; Level >= 1 ; Level-- ) {
       
       for ( k = 1 ; k <= NumberOfVortexSheetsForLevel_[Level] ; k++ ) {
          
          VortexSheetListForLevel_[Level][k].SheetID() = ++j;
          
       }
       
    }
     
    NumberOfStartingVortices_ = VortexSheetListForLevel_[1][1].NumberOfSubVortices();

    NumberOfSubVortices_ = VortexSheetListForLevel_[1][1].NumberOfSubVortices();   
    
    // Calculate the average core size... factor of 2 since sigma is 1/2 of 
    // distance between each trailing vortex at the trailing edge.
    
    CoreSize_ = 0.;
    
    for ( j = 1 ; j <= NumberOfTrailingVortices_ ; j++ ) {
       
       CoreSize_ += pow(2.*TrailingVortexList_[j]->Sigma(),2.);
 
    }
    
    CoreSize_ /= NumberOfTrailingVortices_;
    
    CoreSize_ = sqrt(CoreSize_); 
 
}

/*##############################################################################
#                                                                              #
#                  VORTEX_SHEET SetupCircularVortexSheets                      #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SetupCircularVortexSheets(void)
{

    int j, k, m, p, Level, Done;
    VSP_NODE NodeA, NodeB;

    // Make trailing vortex list periodic... by pointing to the first one

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

    TrailingGammaListForLevel_ = new VSPAERO_DOUBLE**[NumberOfLevels_ + 1];
    
    k = NumberOfTrailingVortices_;
    
    TotalNumberOfVortexSheets_ = 0;

    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {

       // Size vortex sheet list for this level
       
       NumberOfVortexSheetsForLevel_[Level] = k;
       
       TotalNumberOfVortexSheets_ += k;
       
       VortexSheetListForLevel_[Level] = new VORTEX_SHEET[k + 1];
     
       // Size trailing vortex and gamma list for this level
       
       NumberOfTrailingVorticesForLevel_[Level] = k + 1;
       
       TrailingVortexListForLevel_[Level] = new VORTEX_TRAIL*[k + 2];

       TrailingGammaListForLevel_[Level] = new VSPAERO_DOUBLE*[k + 2];
     
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
       
       for ( j = 1 ; j <= NumberOfTrailingVortices_ ; j+=m ) {
       
          k++;
    
          TrailingVortexListForLevel_[Level][k] = TrailingVortexList_[j];

          TrailingGammaListForLevel_[Level][k] = new VSPAERO_DOUBLE[TrailingVortexList_[j]->NumberOfSubVortices() + 3];
   
       }
       
       j -= m;
       
       if ( j != NumberOfTrailingVortices_ + 1) {
          
          j = NumberOfTrailingVortices_ + 1;
          
          k++;

          TrailingVortexListForLevel_[Level][k] = TrailingVortexList_[j];

          TrailingGammaListForLevel_[Level][k] = new VSPAERO_DOUBLE[TrailingVortexList_[j]->NumberOfSubVortices() + 3];

       }
       
       if ( k != NumberOfTrailingVorticesForLevel_[Level] ) {
          
          PRINTF("wtf! \n");
          PRINTF("Level: %d ... k: %d ... NumberOfTrailingVorticesForLevel_[%d]: %d \n",
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
       
       for ( j = 1 ; j <= NumberOfTrailingVortices_ ; j+=m ) {
       
          k++;
    
          // Set the two trailing vortices for this sheet
          
          VortexSheetListForLevel_[Level][k].SetTrailingVortices(*TrailingVortexListForLevel_[Level][k], 
                                                                 *TrailingVortexListForLevel_[Level][k+1]);

          // Set the trailing gammas;
          
          VortexSheetListForLevel_[Level][k].SetVortexGammas(TrailingGammaListForLevel_[Level][k],
                                                             TrailingGammaListForLevel_[Level][k+1]);

          // Set various settings / values
                       
          VortexSheetListForLevel_[Level][k].TimeAccurate()     = TimeAccurate_;
          VortexSheetListForLevel_[Level][k].QuasiTimeAccurate_ = QuasiTimeAccurate_;
          VortexSheetListForLevel_[Level][k].TimeAnalysisType() = TimeAnalysisType_;
          VortexSheetListForLevel_[Level][k].TimeStep()         = TimeStep_;
          VortexSheetListForLevel_[Level][k].Vinf()             = Vinf_;
          VortexSheetListForLevel_[Level][k].Level()            = Level;
          VortexSheetListForLevel_[Level][k].ThereAreChildren() = 0;
                                                                           
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
          
          PRINTF("wtf! \n");
          PRINTF("Level: %d ... k: %d ... NumberOfVortexSheetsForLevel_[%d]: %d \n",
          Level,
          k, 
          Level,
          NumberOfVortexSheetsForLevel_[Level]);
          
          fflush(NULL);
          
          exit(1);
          
       }
       
       m *= 2;
       
    }  
     
    // Set unqiue sheet ID
    
    j = 0;
   
    for ( Level = NumberOfLevels_ ; Level >= 1 ; Level-- ) {
       
       for ( k = 1 ; k <= NumberOfVortexSheetsForLevel_[Level] ; k++ ) {
          
          VortexSheetListForLevel_[Level][k].SheetID() = ++j;
          
       }
       
    }    
   
    NumberOfStartingVortices_ = VortexSheetListForLevel_[1][1].NumberOfSubVortices();

    NumberOfSubVortices_ = VortexSheetListForLevel_[1][1].NumberOfSubVortices();       

    // Calculate the average core size... factor of 2 since sigma is 1/2 of 
    // distance between each trailing vortex at the trailing edge.
    
    CoreSize_ = 0.;
    
    for ( j = 1 ; j <= NumberOfTrailingVortices_ ; j++ ) {
       
       CoreSize_ += pow(2.*TrailingVortexList_[j]->Sigma(),2.);
       
    }
    
    CoreSize_ /= NumberOfTrailingVortices_;
    
    CoreSize_ = sqrt(CoreSize_);

}

/*##############################################################################
#                                                                              #
#                       VORTEX_SHEET SaveVortexState                           #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SaveVortexState(void)
{

    int i;
    
    // Save the current trailing vortex state

    for ( i = 1 ; i <= NumberOfTrailingVortices_  ; i++ ) {
       
       TrailingVortexList_[i]->SaveVortexState();

    }   

}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET UpdateVortexStrengths                    #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::UpdateVortexStrengths(int UpdateType)
{

    int i, j, k, Level, NumSubVorticesMax;
    VSPAERO_DOUBLE S1, S2, Wgt1, Wgt2;
    
    // Steady state solution ... update strengths along entire trailing wake

    if ( !TimeAccurate_ || QuasiTimeAccurate_ ) {

       // Steady state... trailing edge value convected to infinity
       
       for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
      
          for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
   
             TrailingVortexList_[i]->Gamma(j) = TrailingVortexList_[i]->Gamma();
   
          }

       }
      
    }
    
    // Unsteady solution, but we are starting up from a steady state solution
    
    else if ( UpdateType < 0 ) {

       // Steady state... trailing edge value convected to infinity
       
       for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
     
          for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
   
             TrailingVortexList_[i]->Gamma(j) = TrailingVortexList_[i]->Gamma();
   
          }
          
          if ( TimeAccurate_ ) TrailingVortexList_[i]->CurrentTimeStep() = CurrentTimeStep_;
 
       }
       
    }    
    
    // Unsteady solution, so we must convect the wake vorticity down stream
    
    else {

       // Time accurate... so convect trailing edge value one time step
       
       for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
          
          TrailingVortexList_[i]->ConvectWakeVorticity(UpdateType);    

          TrailingVortexList_[i]->CurrentTimeStep() = CurrentTimeStep_;
              
       } 
       
    }

    // Make a copy of the circulation strengths

    for ( i = 1 ; i <= NumberOfTrailingVortices_  ; i++ ) {

       for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {

          TrailingGamma_[i][j] = TrailingVortexList_[i]->Gamma(j);
     
       }

    }    

    // Agglomerate the vortex strengths
    
    // Initialize level 1
    
    Level = 1;
      
    for ( i = 1 ; i <= NumberOfTrailingVorticesForLevel_[Level] ; i++ ) {

       for ( j = 0 ; j <= NumberOfSubVortices() + 2 ; j++ ) {

          TrailingGammaListForLevel_[Level][i][j] = TrailingVortexListForLevel_[Level][i]->Gamma(j);

       }
     
    }

    // Move up levels and inject

    for ( Level = 2 ; Level <= NumberOfLevels_ ; Level++ ) {
      
       // Direct injection

       k = 1;
         
       for ( i = 1 ; i <= NumberOfTrailingVorticesForLevel_[Level] ; i++ ) {
   
          for ( j = 0 ; j <= NumberOfSubVortices() + 2 ; j++ ) {
 
             TrailingGammaListForLevel_[Level][i][j] = TrailingGammaListForLevel_[Level-1][k][j];

          }
          
          k += 2;
          
          if ( k > NumberOfTrailingVorticesForLevel_[Level-1] ) k = NumberOfTrailingVorticesForLevel_[Level-1];
          
       }
      
       // Agglomerate neighbors

       k = 2;
       
       for ( i = 1 ; i <= NumberOfTrailingVorticesForLevel_[Level] - 1 ; i++ ) {

          if ( k < NumberOfTrailingVorticesForLevel_[Level-1] ) {
             
             S1 = sqrt( pow(TrailingVortexListForLevel_[Level][i  ]->TE_Node().x() - TrailingVortexListForLevel_[Level-1][k]->TE_Node().x(), 2.) 
                      + pow(TrailingVortexListForLevel_[Level][i  ]->TE_Node().y() - TrailingVortexListForLevel_[Level-1][k]->TE_Node().y(), 2.) 
                      + pow(TrailingVortexListForLevel_[Level][i  ]->TE_Node().z() - TrailingVortexListForLevel_[Level-1][k]->TE_Node().z(), 2.) );

             S2 = sqrt( pow(TrailingVortexListForLevel_[Level][i+1]->TE_Node().x() - TrailingVortexListForLevel_[Level-1][k]->TE_Node().x(), 2.) 
                      + pow(TrailingVortexListForLevel_[Level][i+1]->TE_Node().y() - TrailingVortexListForLevel_[Level-1][k]->TE_Node().y(), 2.) 
                      + pow(TrailingVortexListForLevel_[Level][i+1]->TE_Node().z() - TrailingVortexListForLevel_[Level-1][k]->TE_Node().z(), 2.) );
 
             Wgt1 = S2/(S1 + S2);
             
             Wgt2 = 1. - Wgt1;        

             for ( j = 0 ; j <= NumberOfSubVortices() + 2 ; j++ ) {

                TrailingGammaListForLevel_[Level][i  ][j] += Wgt1*TrailingGammaListForLevel_[Level-1][k][j];
             
                TrailingGammaListForLevel_[Level][i+1][j] += Wgt2*TrailingGammaListForLevel_[Level-1][k][j];

             }
          
          }
          
          k += 2;
          
          if ( k > NumberOfTrailingVorticesForLevel_[Level-1] ) k = NumberOfTrailingVorticesForLevel_[Level-1];
          
       }
            
    }

    // Calculate the bound vortex strengths
 
    if ( TimeAccurate_ && !QuasiTimeAccurate_ ) {
       
       NumSubVorticesMax = MIN(CurrentTimeStep_ , NumberOfSubVortices());

       // Zero out starting vortices
       
       for ( i = 0 ; i <= NumberOfTrailingVortices_ + 1 ; i++ ) {
          
//          for ( j = 0 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
          for ( j = 0 ; j <= NumSubVorticesMax + 1 ; j++ ) {

             StartingGamma_[i][j] = 0.;
             
          }
      
       } 
   
       // Integrate along span to calculate starting vortices strengths
   
       for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
          
//          for ( j = 1 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
          for ( j = 1 ; j <= NumSubVorticesMax ; j++ ) {
   
             StartingGamma_[i][j] = StartingGamma_[i-1][j] + TrailingGamma_[i][j];
   
          }
          
       }  
     
       // Now calculate delta-gammas
      
       for ( i = 1 ; i <= NumberOfTrailingVortices_  ; i++ ) {
          
          for ( j = 1 ; j < NumSubVorticesMax ; j++ ) {
//          for ( j = 1 ; j < NumberOfSubVortices() + 1 ; j++ ) {
   
             StartingGamma_[i][j] -= StartingGamma_[i][j+1];

          }   
           
       }    
       
       // Initialize level 1
       
       Level = 1;
       
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[Level] ; i++ ) {
         
          for ( j = 1 ; j <= NumSubVorticesMax ; j++ ) {
//          for ( j = 1 ; j <= NumberOfSubVortices() + 1 ; j++ ) {

             VortexSheetListForLevel_[Level][i].StartingGamma(j) = StartingGamma_[i][j];

          }

          VortexSheetListForLevel_[Level][i].BoundVortex().UpdateGamma();
          
          VortexSheetListForLevel_[Level][i].BoundVortex().CurrentTimeStep() = CurrentTimeStep_;
              
       }

       // Agglomerate the bound vortices in the spanwise direction
   
       for ( Level = 2 ; Level <= NumberOfLevels_ ; Level++ ) {
         
          for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[Level] ; i++ ) {
            
             // 1 Child
             
             if ( VortexSheetListForLevel_[Level][i].ThereAreChildren() == 1 ) {
                  
                for ( j = 1 ; j <= NumSubVorticesMax ; j++ ) {
//                for ( j = 1 ; j <= NumberOfSubVortices() + 1 ; j++ ) {
      
                   VortexSheetListForLevel_[Level][i].StartingGamma(j) = VortexSheetListForLevel_[Level][i].Child1().StartingGamma(j);
      
                }
                
             }
             
             // 2 Children
             
             else {
 
                S1 = VortexSheetListForLevel_[Level][i].Child1().Span();
                
                S2 = VortexSheetListForLevel_[Level][i].Child2().Span();
                         
                Wgt1 = S1/(S1 + S2);
                
                Wgt2 = 1. - Wgt1;        

                for ( j = 1 ; j <= NumSubVorticesMax ; j++ ) {
//                for ( j = 1 ; j <= NumberOfSubVortices() + 1 ; j++ ) {

                   VortexSheetListForLevel_[Level][i].StartingGamma(j) = Wgt1*VortexSheetListForLevel_[Level][i].Child1().StartingGamma(j)
                                                                       + Wgt2*VortexSheetListForLevel_[Level][i].Child2().StartingGamma(j);
      
                }
                
             }
                             
          }
        
       }
       
       // Agglomerated bound vortex data structure

       for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
         
          for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[Level] ; i++ ) {   
                
             for ( j = 1 ; j <= NumSubVorticesMax ; j++ ) {  
//             for ( j = 1 ; j <= NumberOfSubVortices() + 1 ; j++ ) {  

                 VortexSheetListForLevel_[Level][i].BoundVortex().Gamma(j) = VortexSheetListForLevel_[Level][i].StartingGamma(j);
               
             }
             
             VortexSheetListForLevel_[Level][i].BoundVortex().UpdateGamma();
             
             VortexSheetListForLevel_[Level][i].BoundVortex().CurrentTimeStep() = CurrentTimeStep_;
             
          }
          
       }
          
    }

}

/*##############################################################################
#                                                                              #
#                 VORTEX_SHEET UpdateConvectedDistance                         #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::UpdateConvectedDistance(void)
{
   
    int i, j;
    
    for ( j = 1 ; j <= NumberOfLevels_ ; j++ ) {
   
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[j] ; i++ ) {
          
          VortexSheetListForLevel_[j][i].CurrentTimeStep() = CurrentTimeStep_;
   
       } 
       
    }
 
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
            
       TrailingVortexList_[i]->CurrentTimeStep() = CurrentTimeStep_;
       
       TrailingVortexList_[i]->CreateSearchTree();
           
    }     
   
}

/*##############################################################################
#                                                                              #
#                    VORTEX_SHEET CreateInteractionSheetList                   #
#                                                                              #
##############################################################################*/

VORTEX_SHEET_ENTRY *VORTEX_SHEET::CreateInteractionSheetList(VSPAERO_DOUBLE xyz_p[3], int &NumberOfEvaluatedSheets)
{

    int i, j;
    VORTEX_SHEET *VortexSheet;    
    VORTEX_SHEET_ENTRY *SheetList;

    // Zero evaluation level and initialize trailing and shed vortex gammas

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       TrailingVortexList_[i]->Evaluate() = 0;
       
       TrailingVortexList_[i]->Searched() = 0;

    }   
    
    for ( j = 1 ; j <= NumberOfLevels_ ; j++ ) {
   
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[j] ; i++ ) {
          
          VortexSheetListForLevel_[j][i].Evaluate() = 0;

       }
       
    }
    
    // Agglomerate the trailing vortices .. we start at the coarsest level   

    if ( NumberOfTrailingVortices_ >= 4 ) {

       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[NumberOfLevels_] ; i++ ) {
          
          VortexSheet = &VortexSheetListForLevel_[NumberOfLevels_][i];

          CreateVortexSheetInteractionList(*VortexSheet, xyz_p);
          
       }
       
    }
    
    NumberOfEvaluatedSheets = 0;
    
    // Create a unique list of those vortex sheets that are to be evaluated
    
    for ( j = 1 ; j <= NumberOfLevels_ ; j++ ) {
   
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[j] ; i++ ) {
          
          if ( VortexSheetListForLevel_[j][i].Evaluate() ) NumberOfEvaluatedSheets++;
             
             
       } 
       
    }

    SheetList = new VORTEX_SHEET_ENTRY[NumberOfEvaluatedSheets + 1];
    
    NumberOfEvaluatedSheets = 0;
    
    // Create a unique list of those vortex sheets that are to be evaluated
    
    for ( j = 1 ; j <= NumberOfLevels_ ; j++ ) {
   
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[j] ; i++ ) {
          
          if ( VortexSheetListForLevel_[j][i].Evaluate() ) {

             NumberOfEvaluatedSheets++;

             SheetList[NumberOfEvaluatedSheets].Level    = j;
             SheetList[NumberOfEvaluatedSheets].Sheet    = i;
             SheetList[NumberOfEvaluatedSheets].SheetID  = VortexSheetListForLevel_[j][i].SheetID();
             SheetList[NumberOfEvaluatedSheets].Distance = VortexSheetListForLevel_[j][i].Distance();
             
          }
             
             
       } 
       
    }   

    return SheetList; 

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

       TrailingVortexList_[i]->WakeDampingIsOn() = 1;

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

       TrailingVortexList_[i]->WakeDampingIsOn() = 0;

    }       
    
}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET InducedVelocity                          #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::InducedVelocity(int NumberOfSheets, VORTEX_SHEET_ENTRY *SheetList, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3])
{
   
    int i, j, Level, Sheet, NumVortices;
    VSPAERO_DOUBLE U, V, W, dq[3];
    VORTEX_TRAIL *TrailingVortex;
    VORTEX_SHEET *VortexSheet;
    
    // Zero evaluation level and initialize trailing and shed vortex gammas

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       TrailingVortexList_[i]->Evaluate() = 0;

       TrailingVortexList_[i]->Searched() = 0;

    }   

    for ( j = 1 ; j <= NumberOfLevels_ ; j++ ) {
   
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[j] ; i++ ) {
          
          VortexSheetListForLevel_[j][i].Evaluate() = 0;

       }
       
    }
        
    // Evaluate the sheets that were marked
           
    for ( i = 1 ; i <= NumberOfSheets ; i++ ) {

       // Grab the level and sheet 
       
       Level = SheetList[i].Level;
       Sheet = SheetList[i].Sheet;

       VortexSheet = &(VortexSheetListForLevel_[Level][Sheet]);
     
       VortexSheet->Evaluate() = 1;

       // Steady state calculation
       
       if ( !TimeAccurate_) {

          if ( VortexSheet->Level() > VortexSheet->VortexTrail1().Evaluate() ) {
             
             VortexSheet->VortexTrail1().Gamma(0) = VortexSheet->TrailingGamma1(0);
             
             VortexSheet->VortexTrail1().Evaluate() = VortexSheet->Level();
             
          }
          
          if ( VortexSheet->Level() > VortexSheet->VortexTrail2().Evaluate() ) {
    
             VortexSheet->VortexTrail2().Gamma(0) = VortexSheet->TrailingGamma2(0);
             
             VortexSheet->VortexTrail2().Evaluate() = VortexSheet->Level();
             
          }
  
       }
       
       // Time accurate
       
       else {
          
          NumVortices = MIN( CurrentTimeStep_ + 1, NumberOfSubVortices());
    
          if ( VortexSheet->Level() > VortexSheet->VortexTrail1().Evaluate() ) {
 
             for ( j = 0 ; j <= NumVortices + 1 ; j++ ) {
                
                VortexSheet->VortexTrail1().Gamma(j) = VortexSheet->TrailingGamma1(j);
                                  
             }
                     
             VortexSheet->VortexTrail1().Evaluate() = VortexSheet->Level();
             
          }
        
          if ( VortexSheet->Level() > VortexSheet->VortexTrail2().Evaluate() ) {

             for ( j = 0 ; j <= NumVortices + 1 ; j++ ) {
                
                VortexSheet->VortexTrail2().Gamma(j) = VortexSheet->TrailingGamma2(j);
                                  
             }
                          
             VortexSheet->VortexTrail2().Evaluate() = VortexSheet->Level();
             
          }                    
    
       }
       
    }

    // Create a unique list of the agglomerated trailing vortices

    NumberOfAgglomeratedTrailingVortices_ = 0;

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       if ( TrailingVortexList_[i]->Evaluate() ) {
      
          AgglomeratedTrailingVortexList_[++NumberOfAgglomeratedTrailingVortices_] = TrailingVortexList_[i];
          
       }
       
    }    

    // Evalulate the agglomerated trailing vortices
    
    q[0] = q[1] = q[2] = U = V = W = 0.;

    for ( i = 1 ; i <= NumberOfAgglomeratedTrailingVortices_ ; i++ ) {

       TrailingVortex = AgglomeratedTrailingVortexList_[i];

 //      TrailingVortex->InducedVelocity(xyz_p,dq,CoreSize_); // This was wrong, it should not be using the core model here!
       TrailingVortex->InducedVelocity(xyz_p,dq);
              
       U += dq[0];
       V += dq[1];
       W += dq[2];

    } 

    q[0] = U;
    q[1] = V;
    q[2] = W;   
    
    // If this is an unsteady solution, we have to evaluate the starting
    // vortices for all the previous time steps

    if ( TimeAccurate_ && !QuasiTimeAccurate_ ) {

       // Start at the coarsest level
       
       U = V = W = 0.;
       
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[NumberOfLevels_] ; i++ ) {
         
          VortexSheet = &VortexSheetListForLevel_[NumberOfLevels_][i];
          
          dq[0] = dq[1] = dq[2] = 0.;
    
     //     StartingVorticesInducedVelocity(*VortexSheet, xyz_p, dq, CoreSize_); // This was wrong, it should not be using the core model here!
          StartingVorticesInducedVelocity(*VortexSheet, xyz_p, dq);
          
          U += dq[0];
          V += dq[1];
          W += dq[2];          
         
       } 

       q[0] += U;
       q[1] += V;
       q[2] += W;   
           
    }    

}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET InducedVelocity                          #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::InducedVelocity(int NumberOfSheets, VORTEX_SHEET_ENTRY *SheetList, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE xyz_te[3], VSPAERO_DOUBLE q[3])
{
   
    int i, j, Level, Sheet, NumVortices;
    VSPAERO_DOUBLE U, V, W, dq[3], Dist;
    VORTEX_TRAIL *TrailingVortex;
    VORTEX_SHEET *VortexSheet;
    
    // Zero evaluation level and initialize trailing and shed vortex gammas

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       TrailingVortexList_[i]->Evaluate() = 0;
       
       TrailingVortexList_[i]->Searched() = 0;

    }   

    for ( j = 1 ; j <= NumberOfLevels_ ; j++ ) {
   
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[j] ; i++ ) {
          
          VortexSheetListForLevel_[j][i].Evaluate() = 0;

       }
       
    }
        
    // Evaluate the sheets that were marked
           
    for ( i = 1 ; i <= NumberOfSheets ; i++ ) {

       // Grab the level and sheet 
       
       Level = SheetList[i].Level;
       Sheet = SheetList[i].Sheet;

       VortexSheet = &(VortexSheetListForLevel_[Level][Sheet]);
     
       VortexSheet->Evaluate() = 1;

       // Steady state calculation
       
       if ( !TimeAccurate_) {

          if ( VortexSheet->Level() > VortexSheet->VortexTrail1().Evaluate() ) {
             
             VortexSheet->VortexTrail1().Gamma(0) = VortexSheet->TrailingGamma1(0);
             
             VortexSheet->VortexTrail1().Evaluate() = VortexSheet->Level();
             
          }
          
          if ( VortexSheet->Level() > VortexSheet->VortexTrail2().Evaluate() ) {
    
             VortexSheet->VortexTrail2().Gamma(0) = VortexSheet->TrailingGamma2(0);
             
             VortexSheet->VortexTrail2().Evaluate() = VortexSheet->Level();
             
          }
  
       }
       
       // Time accurate
       
       else {
          
          NumVortices = MIN( CurrentTimeStep_ + 1, NumberOfSubVortices());
   
          if ( VortexSheet->Level() > VortexSheet->VortexTrail1().Evaluate() ) {
 
             for ( j = 0 ; j <= NumVortices + 1 ; j++ ) {
                
                VortexSheet->VortexTrail1().Gamma(j) = VortexSheet->TrailingGamma1(j);
                                  
             }
                     
             VortexSheet->VortexTrail1().Evaluate() = VortexSheet->Level();
             
          }
        
          if ( VortexSheet->Level() > VortexSheet->VortexTrail2().Evaluate() ) {

             for ( j = 0 ; j <= NumVortices + 1 ; j++ ) {
                
                VortexSheet->VortexTrail2().Gamma(j) = VortexSheet->TrailingGamma2(j);
                                  
             }
                          
             VortexSheet->VortexTrail2().Evaluate() = VortexSheet->Level();
             
          }                    
    
       }
       
    }

    // Create a unique list of the agglomerated trailing vortices

    NumberOfAgglomeratedTrailingVortices_ = 0;

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       if ( TrailingVortexList_[i]->Evaluate() ) {

          Dist = sqrt( (xyz_te[0] - TrailingVortexList_[i]->TE_Node().x())*(xyz_te[0] - TrailingVortexList_[i]->TE_Node().x())
                     + (xyz_te[1] - TrailingVortexList_[i]->TE_Node().y())*(xyz_te[1] - TrailingVortexList_[i]->TE_Node().y())
                     + (xyz_te[2] - TrailingVortexList_[i]->TE_Node().z())*(xyz_te[2] - TrailingVortexList_[i]->TE_Node().z()) ); 
                                         
          // Don't do self induced velocities                     

          if ( Dist >= 0.5*TrailingVortexList_[i]->Sigma() ) AgglomeratedTrailingVortexList_[++NumberOfAgglomeratedTrailingVortices_] = TrailingVortexList_[i];
          
       }
       
    }    

    // Evalulate the agglomerated trailing vortices
    
    q[0] = q[1] = q[2] = U = V = W = 0.;

    for ( i = 1 ; i <= NumberOfAgglomeratedTrailingVortices_ ; i++ ) {

       TrailingVortex = AgglomeratedTrailingVortexList_[i];

       TrailingVortex->InducedVelocity(xyz_p,dq,CoreSize_);
              
       U += dq[0];
       V += dq[1];
       W += dq[2];

    } 

    q[0] = U;
    q[1] = V;
    q[2] = W;   
    
    // If this is an unsteady solution, we have to evaluate the starting
    // vortices for all the previous time steps

    if ( TimeAccurate_ && !QuasiTimeAccurate_ ) {

       // Start at the coarsest level

       U = V = W = 0.;

       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[NumberOfLevels_] ; i++ ) {
         
          VortexSheet = &VortexSheetListForLevel_[NumberOfLevels_][i];
          
          dq[0] = dq[1] = dq[2] = 0.;
       
          StartingVorticesInducedVelocity(*VortexSheet,xyz_p,dq,CoreSize_);
          
          U += dq[0];
          V += dq[1];
          W += dq[2];          
          
       } 
 
       q[0] += U;
       q[1] += V;
       q[2] += W;   
          
    }    
  
}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET InducedKuttaVelocity                     #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::InducedKuttaVelocity(int NumberOfSheets, VORTEX_SHEET_ENTRY *SheetList, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3])
{
   
    int i, j, Level, Sheet, NumVortices;
    VSPAERO_DOUBLE U, V, W, dq[3], Vec[3], xyz_k[3], Mag;
    VORTEX_TRAIL *TrailingVortex;
    VORTEX_SHEET *VortexSheet;
    
    // Zero evaluation level and initialize trailing and shed vortex gammas

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       TrailingVortexList_[i]->Evaluate() = 0;

       TrailingVortexList_[i]->Searched() = 0;

    }   

    for ( j = 1 ; j <= NumberOfLevels_ ; j++ ) {
   
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[j] ; i++ ) {
          
          VortexSheetListForLevel_[j][i].Evaluate() = 0;

       }
       
    }
        
    // Evaluate the sheets that were marked
           
    for ( i = 1 ; i <= NumberOfSheets ; i++ ) {

       // Grab the level and sheet 
       
       Level = SheetList[i].Level;
       Sheet = SheetList[i].Sheet;

       VortexSheet = &(VortexSheetListForLevel_[Level][Sheet]);
     
       VortexSheet->Evaluate() = 1;

       // Steady state calculation
       
       if ( !TimeAccurate_) {

          if ( VortexSheet->Level() > VortexSheet->VortexTrail1().Evaluate() ) {
             
             VortexSheet->VortexTrail1().Gamma(0) = VortexSheet->TrailingGamma1(0);
             
             VortexSheet->VortexTrail1().Evaluate() = VortexSheet->Level();
             
          }
          
          if ( VortexSheet->Level() > VortexSheet->VortexTrail2().Evaluate() ) {
    
             VortexSheet->VortexTrail2().Gamma(0) = VortexSheet->TrailingGamma2(0);
             
             VortexSheet->VortexTrail2().Evaluate() = VortexSheet->Level();
             
          }
  
       }
       
       // Time accurate
       
       else {
          
          NumVortices = MIN( CurrentTimeStep_ + 1, NumberOfSubVortices());
    
          if ( VortexSheet->Level() > VortexSheet->VortexTrail1().Evaluate() ) {
 
             for ( j = 0 ; j <= NumVortices + 1 ; j++ ) {
                
                VortexSheet->VortexTrail1().Gamma(j) = VortexSheet->TrailingGamma1(j);
                                  
             }
                     
             VortexSheet->VortexTrail1().Evaluate() = VortexSheet->Level();
             
          }
        
          if ( VortexSheet->Level() > VortexSheet->VortexTrail2().Evaluate() ) {

             for ( j = 0 ; j <= NumVortices + 1 ; j++ ) {
                
                VortexSheet->VortexTrail2().Gamma(j) = VortexSheet->TrailingGamma2(j);
                                  
             }
                          
             VortexSheet->VortexTrail2().Evaluate() = VortexSheet->Level();
             
          }                    
    
       }
       
    }

    // Create a unique list of the agglomerated trailing vortices

    NumberOfAgglomeratedTrailingVortices_ = 0;

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       if ( TrailingVortexList_[i]->Evaluate() ) {
      
          AgglomeratedTrailingVortexList_[++NumberOfAgglomeratedTrailingVortices_] = TrailingVortexList_[i];
          
       }
       
    }    

    // Evaluate the agglomerated trailing vortices
    
    q[0] = q[1] = q[2] = U = V = W = 0.;

     
#pragma omp parallel for reduction(+:U,V,W) private(Vec,Mag,xyz_k,dq,TrailingVortex) schedule(dynamic)
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

       U += dq[0];
       V += dq[1];
       W += dq[2];

    } 
    
    q[0] = U;
    q[1] = V;
    q[2] = W;       
  
    // If this is an unsteady solution, we have to evaluate the starting
    // vortices for all the previous time steps

    if ( TimeAccurate_ && !QuasiTimeAccurate_ ) {

       // Loop over the vortex sheets for the finest level

       U = V = W = 0.;
       
#pragma omp parallel for reduction(+:U,V,W) private(dq,xyz_k,VortexSheet) schedule(dynamic)       
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
       
       q[0] += U;
       q[1] += V;
       q[2] += W;          
       
    }       
  
}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET InducedVelocity                          #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3])
{

    int i;
    VSPAERO_DOUBLE U, V, W, dq[3];
    VORTEX_TRAIL *TrailingVortex;
    VORTEX_SHEET *VortexSheet;

    // Zero evaluation level and initialize trailing and shed vortex gammas

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       TrailingVortexList_[i]->Evaluate() = 0;
       
       TrailingVortexList_[i]->Searched() = 0;

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

       if ( TrailingVortexList_[i]->Evaluate() ) {
      
          AgglomeratedTrailingVortexList_[++NumberOfAgglomeratedTrailingVortices_] = TrailingVortexList_[i];
          
       }
       
    }    

    // Evalulate the agglomerated trailing vortices
    
    q[0] = q[1] = q[2] = U = V = W = 0.;

    for ( i = 1 ; i <= NumberOfAgglomeratedTrailingVortices_ ; i++ ) {

       TrailingVortex = AgglomeratedTrailingVortexList_[i];

       TrailingVortex->InducedVelocity(xyz_p,dq);
              
       U += dq[0];
       V += dq[1];
       W += dq[2];

    } 

    // If this is an unsteady solution, we have to evaluate the starting
    // vortices for all the previous time steps

    if ( TimeAccurate_ && !QuasiTimeAccurate_ ) {

       // Start at the coarsest level
       
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

void VORTEX_SHEET::InducedVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3], VSPAERO_DOUBLE xyz_te[3])
{

    int i;
    VSPAERO_DOUBLE U, V, W, dq[3], Dist;
    VORTEX_TRAIL *TrailingVortex;
    VORTEX_SHEET *VortexSheet;

    // Zero evaluation level and initialize trailing vortex gammas to finest level

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       TrailingVortexList_[i]->Evaluate() = 0;
       
       TrailingVortexList_[i]->Searched() = 0;

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
       
       if ( TrailingVortexList_[i]->Evaluate() ) {

          Dist = sqrt( (xyz_te[0] - TrailingVortexList_[i]->TE_Node().x())*(xyz_te[0] - TrailingVortexList_[i]->TE_Node().x())
                     + (xyz_te[1] - TrailingVortexList_[i]->TE_Node().y())*(xyz_te[1] - TrailingVortexList_[i]->TE_Node().y())
                     + (xyz_te[2] - TrailingVortexList_[i]->TE_Node().z())*(xyz_te[2] - TrailingVortexList_[i]->TE_Node().z()) ); 
                                         
          // Don't do self induced velocities                     
          
          if ( Dist >= 0.5*TrailingVortexList_[i]->Sigma() ) AgglomeratedTrailingVortexList_[++NumberOfAgglomeratedTrailingVortices_] = TrailingVortexList_[i];
          
       }
       
    }    

    // Evalulate the agglomerated trailing vortices

    q[0] = q[1] = q[2] = U = V = W = 0.;

    for ( i = 1 ; i <= NumberOfAgglomeratedTrailingVortices_ ; i++ ) {

       TrailingVortex = AgglomeratedTrailingVortexList_[i];

       TrailingVortex->InducedVelocity(xyz_p,dq,CoreSize_);
  
       U += dq[0];
       V += dq[1];
       W += dq[2];

    } 
    
    // If this is an unsteady solution, we have to evaluate the starting
    // vortices for all the previous time steps

    if ( TimeAccurate_ && !QuasiTimeAccurate_ ) {

       // Loop over the vortex sheets for the finest level

       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[NumberOfLevels_] ; i++ ) {
          
          VortexSheet = &VortexSheetListForLevel_[NumberOfLevels_][i];
          
          dq[0] = dq[1] = dq[2] = 0.;
       
          StartingVorticesInducedVelocity(*VortexSheet, xyz_p, dq, CoreSize_);
          
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

void VORTEX_SHEET::InducedKuttaVelocity(VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE q[3])
{

    int i;
    VSPAERO_DOUBLE U, V, W, Vec[3], xyz_k[3], dq[3], Mag;
    VORTEX_TRAIL *TrailingVortex;
    VORTEX_SHEET *VortexSheet;

    // Zero evaluation level and initialize trailing vortex gammas to finest level
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       TrailingVortexList_[i]->Evaluate() = 0;
       
       TrailingVortexList_[i]->Searched() = 0;

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
       
       if ( TrailingVortexList_[i]->Evaluate() ) {

          AgglomeratedTrailingVortexList_[++NumberOfAgglomeratedTrailingVortices_] = TrailingVortexList_[i];
          
       }
       
    }    

    // Evaluate the agglomerated trailing vortices
    
    q[0] = q[1] = q[2] = U = V = W = 0.;

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
          
       U += dq[0];
       V += dq[1];
       W += dq[2];

    } 
    
    // If this is an unsteady solution, we have to evaluate the starting
    // vortices for all the previous time steps

    if ( TimeAccurate_ && !QuasiTimeAccurate_ ) {

       // Loop over the vortex sheets for the finest level

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

void VORTEX_SHEET::CreateTrailingVortexInteractionList(VORTEX_SHEET &VortexSheet, VSPAERO_DOUBLE xyz_p[3])
{

    int j, NumChildren, NumVortices;
    
    NumChildren = 0;
 
    if ( VortexSheet.ThereAreChildren() == 1 ) NumChildren = 1;
    
    if ( VortexSheet.ThereAreChildren() == 2 ) NumChildren = 2;

    if ( VortexSheet.FarAway(xyz_p) || NumChildren == 0 ) {

       // Steady state calculation
       
       if ( !TimeAccurate_) {

          if ( VortexSheet.Level() > VortexSheet.VortexTrail1().Evaluate() ) {
             
             VortexSheet.VortexTrail1().Gamma(0) = VortexSheet.TrailingGamma1(0);
             
             VortexSheet.VortexTrail1().Evaluate() = VortexSheet.Level();
             
          }
          
          if ( VortexSheet.Level() > VortexSheet.VortexTrail2().Evaluate() ) {
    
             VortexSheet.VortexTrail2().Gamma(0) = VortexSheet.TrailingGamma2(0);
             
             VortexSheet.VortexTrail2().Evaluate() = VortexSheet.Level();
             
          }
  
       }
       
       // Time accurate
       
       else {
          
          NumVortices = MIN( CurrentTimeStep_ + 1, NumberOfSubVortices());
    
          if ( VortexSheet.Level() > VortexSheet.VortexTrail1().Evaluate() ) {
 
             for ( j = 0 ; j <= NumVortices + 1 ; j++ ) {
                
                VortexSheet.VortexTrail1().Gamma(j) = VortexSheet.TrailingGamma1(j);
                                  
             }
                     
             VortexSheet.VortexTrail1().Evaluate() = VortexSheet.Level();
             
          }
        
          if ( VortexSheet.Level() > VortexSheet.VortexTrail2().Evaluate() ) {

             for ( j = 0 ; j <= NumVortices + 1 ; j++ ) {
                
                VortexSheet.VortexTrail2().Gamma(j) = VortexSheet.TrailingGamma2(j);
                                  
             }
                          
             VortexSheet.VortexTrail2().Evaluate() = VortexSheet.Level();
             
          }                    
    
       }

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
#                VORTEX_SHEET CreateVortexSheetInteractionList                 #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::CreateVortexSheetInteractionList(VORTEX_SHEET &VortexSheet, VSPAERO_DOUBLE xyz_p[3])
{

    int NumChildren;
    
    NumChildren = 0;
 
    if ( VortexSheet.ThereAreChildren() == 1 ) NumChildren = 1;
    
    if ( VortexSheet.ThereAreChildren() == 2 ) NumChildren = 2;

    if ( VortexSheet.FarAway(xyz_p) || NumChildren == 0 ) {

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
#            VORTEX_SHEET StartingVorticesInducedVelocity                      #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::StartingVorticesInducedVelocity(VORTEX_SHEET &VortexSheet, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE dq[3])
{

    VSPAERO_DOUBLE q[3];

    if ( VortexSheet.Evaluate() == 1 ) {

      // Calculate all shed bound vortices for the vortex sheet

      VortexSheet.BoundVortex().InducedVelocity(xyz_p,q);

      dq[0] += q[0];
      dq[1] += q[1];
      dq[2] += q[2];
        
    }
    
    else {

       if ( VortexSheet.ThereAreChildren() >= 1 ) StartingVorticesInducedVelocity(VortexSheet.Child1(), xyz_p, dq);

       if ( VortexSheet.ThereAreChildren() >= 2 ) StartingVorticesInducedVelocity(VortexSheet.Child2(), xyz_p, dq);

    }

}

/*##############################################################################
#                                                                              #
#            VORTEX_SHEET StartingVorticesInducedVelocity                      #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::StartingVorticesInducedVelocity(VORTEX_SHEET &VortexSheet, VSPAERO_DOUBLE xyz_p[3], VSPAERO_DOUBLE dq[3], VSPAERO_DOUBLE CoreSize)
{

    VSPAERO_DOUBLE q[3];

    if ( VortexSheet.Evaluate() == 1 ) {

      // Calculate all shed bound vortices for the vortex sheet

      VortexSheet.BoundVortex().InducedVelocity(xyz_p,q,CoreSize);

      dq[0] += q[0];
      dq[1] += q[1];
      dq[2] += q[2];
        
    }
    
    else {

       if ( VortexSheet.ThereAreChildren() >= 1 ) StartingVorticesInducedVelocity(VortexSheet.Child1(), xyz_p, dq, CoreSize);

       if ( VortexSheet.ThereAreChildren() >= 2 ) StartingVorticesInducedVelocity(VortexSheet.Child2(), xyz_p, dq, CoreSize);

    }

}

/*##############################################################################
#                                                                              #
#                            VORTEX_SHEET Setup                                #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::Setup(void)
{

    VSP_NODE NodeA, NodeB;

    // Span of this vortex sheet

    Span_ = sqrt( SQR(VortexTrail1().TE_Node().x() - VortexTrail2().TE_Node().x())
                + SQR(VortexTrail1().TE_Node().y() - VortexTrail2().TE_Node().y())
                + SQR(VortexTrail1().TE_Node().z() - VortexTrail2().TE_Node().z()) );

    // Unsteady parameters
    
    VortexTrail1().TimeAccurate() = VortexTrail2().TimeAccurate() = TimeAccurate_;
    
    VortexTrail1().TimeStep() = VortexTrail2().TimeStep() = TimeStep_;
    
    VortexTrail1().Vinf() = VortexTrail2().Vinf() = Vinf_;

    StartingGamma_ = new VSPAERO_DOUBLE*[1];
    
    StartingGamma_[0] = new VSPAERO_DOUBLE[VortexTrail1().NumberOfSubVortices() + 2];
    
    zero_double_array(StartingGamma_[0], VortexTrail1().NumberOfSubVortices() + 1);

    // Starting vortex list
    
    NumberOfStartingVortices_ = VortexTrail1().NumberOfSubVortices();

    NumberOfSubVortices_ = VortexTrail1().NumberOfSubVortices();

    // Setup the bound vortex list
    
    BoundVortex_.Setup(VortexTrail1(), VortexTrail2());  
                   
}

/*##############################################################################
#                                                                              #
#                    VORTEX_TRAIL UpdateWakeLocation                           #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE VORTEX_SHEET::UpdateWakeLocation(void)
{

    int i, Level;
    VSPAERO_DOUBLE Delta, MaxDelta;
    
    MaxDelta = 0.;

    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       if ( DoGroundEffectsAnalysis_ ) TrailingVortexList_[i]->DoGroundEffectsAnalysis() = 1;
       
       Delta = TrailingVortexList_[i]->UpdateWakeLocation();
       
       MaxDelta = MAX(MaxDelta,Delta);
    
    }

    // Update bound vortices
    
    if ( TimeAccurate_ && !QuasiTimeAccurate_ ) {

       for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
          
          for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[Level] ; i++ ) {
   
             VortexSheetListForLevel_[Level][i].CurrentTimeStep() = CurrentTimeStep_;
             
             VortexSheetListForLevel_[Level][i].UpdateGeometryLocation();
             
          }
          
       }

    }

    return MaxDelta;
    
}

/*##############################################################################
#                                                                              #
#                    VORTEX_SHEET UpdateGeometryLocation                       #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat, int *ComponentInThisGroup)
{
   
    int i, Level, UpdatedGeometry;
   
    // Update each trailing wake shape
    
    UpdatedGeometry = 0;
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
 
       if ( ComponentInThisGroup[TrailingVortexList_[i]->ComponentID()] ) {
          
           TrailingVortexList_[i]->UpdateGeometryLocation(TVec, OVec, Quat, InvQuat);
           
           UpdatedGeometry = 1;
           
       }
       
    }

    // Update bound vortices
    
    if ( UpdatedGeometry ) {
       
       for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
          
          for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[Level] ; i++ ) {
             
             VortexSheetListForLevel_[Level][i].CurrentTimeStep() = CurrentTimeStep_;
       
             VortexSheetListForLevel_[Level][i].UpdateGeometryLocation();
       
          }
          
       }
       
    }
  
}

/*##############################################################################
#                                                                              #
#                    VORTEX_SHEET UpdateGeometryLocation                       #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::UpdateGeometryLocation(void)
{

    BoundVortex_.CurrentTimeStep() = CurrentTimeStep_;

    BoundVortex_.UpdateGeometryLocation(VortexTrail1(), VortexTrail2());  
       
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
 
       if ( ComponentInThisGroup[TrailingVortexList_[i]->ComponentID()] ) {

           TrailingVortexList_[i]->IsARotor() = 1;
           
           IsARotor_ = 1;

           PRINTF("i: %d .... is a rotor! \n");fflush(NULL);
           
       }
       
    }
    
}

/*##############################################################################
#                                                                              #
#                            VORTEX_SHEET Distance                             #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE VORTEX_SHEET::Distance(VSPAERO_DOUBLE xyz[3])
{

    VSPAERO_DOUBLE Distance;
    TEST_NODE Test;

    Distance = 1.e9;
   
    // Trail 1
    
    if ( !VortexTrail1().Searched() ) {
   
       Test.xyz[0] = xyz[0];
       Test.xyz[1] = xyz[1];
       Test.xyz[2] = xyz[2];
       
       Test.found = 0;
   
       Test.distance = 1.e9;
     
       VortexTrail1().Search().SearchTree(Test);

       VortexTrail1().Distance() = Test.distance;
       
       VortexTrail1().Searched() = 1;

    }
    
    // Trail 2
    
    if ( !VortexTrail2().Searched() ) {
   
       Test.xyz[0] = xyz[0];
       Test.xyz[1] = xyz[1];
       Test.xyz[2] = xyz[2];
       
       Test.found = 0;
   
       Test.distance = 1.e9;
     
       VortexTrail2().Search().SearchTree(Test);
 
       VortexTrail2().Distance() = Test.distance;
       
       VortexTrail2().Searched() = 1;
       
    }    

    Distance = sqrt(MIN(VortexTrail1().Distance(),VortexTrail2().Distance()));

    return Distance;

}

/*##############################################################################
#                                                                              #
#                            VORTEX_SHEET FarAway                              #
#                                                                              #
##############################################################################*/

int VORTEX_SHEET::FarAway(VSPAERO_DOUBLE xyz[3])
{

    // See if we are far enough away...
    
    Distance_ = Distance(xyz);

    if ( Distance_ >= 5.*FarAway_*Span_ ) return 1;
   
    return 0;

}

/*##############################################################################
#                                                                              #
#                       VORTEX_SHEET ZeroEdgeVelocities                        #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::ZeroEdgeVelocities(void)
{

    int i;
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
       
       TrailingVortex(i).ZeroEdgeVelocities();
       
    }

}

/*##############################################################################
#                                                                              #
#                       VORTEX_SHEET ProlongateEdgeVelocities                  #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::ProlongateEdgeVelocities(void)
{

    int i;
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {
       
       TrailingVortex(i).ProlongateEdgeVelocities();
       
    }

}

/*##############################################################################
#                                                                              #
#                       VORTEX_SHEET SizeCommonTEList                          #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SizeCommonTEList(int NumberOfCommonTE)
{

    NumberOfCommonTE_ = NumberOfCommonTE;
    
    CommonTEList_ = new COMMON_VORTEX_SHEET[NumberOfCommonTE_ + 1];
    
    NumberOfCommonNodesForTE_ = new int[NumberOfTrailingVortices_ + 1];
    
    zero_int_array(NumberOfCommonNodesForTE_, NumberOfTrailingVortices_);

}
















