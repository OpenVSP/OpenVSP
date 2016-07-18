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
    
    ThereAreChildren_ = 0 ;
    
    Gamma_ = NULL;

    TrailingVortexList_ = NULL;
   
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
    
}

/*##############################################################################
#                                                                              #
#                                VORTEX_SHEET Copy                             #
#                                                                              #
##############################################################################*/

VORTEX_SHEET::VORTEX_SHEET(const VORTEX_SHEET &Solver_)
{

    init();

    // Just * use the operator = code

    *this = Solver_;

}

/*##############################################################################
#                                                                              #
#                               VORTEX_SHEET operator=                         #
#                                                                              #
##############################################################################*/

VORTEX_SHEET& VORTEX_SHEET::operator=(const VORTEX_SHEET &Trailing_Vortex)
{

 
    return *this;

}

/*##############################################################################
#                                                                              #
#                            VORTEX_SHEET destructor                           #
#                                                                              #
##############################################################################*/

VORTEX_SHEET::~VORTEX_SHEET(void)
{
 
   if ( NumberOfTrailingVortices_ != 0 ) {
      
      delete [] TrailingVortexList_;
      
   } 

}

/*##############################################################################
#                                                                              #
#                       VORTEX_SHEET SizeTrailingVortexList                    #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SizeTrailingVortexList(int NumberOfTrailingVortices)
{
   
   if ( NumberOfTrailingVortices_ != 0 ) {
      
      delete [] TrailingVortexList_;
      
   }
   
   NumberOfTrailingVortices_ = NumberOfTrailingVortices;
   
   TrailingVortexList_ = new VORTEX_TRAIL[NumberOfTrailingVortices_ + 2];
   
   Gamma_ = new double[NumberOfTrailingVortices_ + 2]; 
   
   AgglomeratedTrailingVortexList_= new VORTEX_TRAIL*[NumberOfTrailingVortices_ + 2];
   
}

/*##############################################################################
#                                                                              #
#                      VORTEX_SHEET SetupVortexSheets                          #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SetupVortexSheets(void)
{
   
    // Closed vortex sheet (likely a nacelle... )
    
    if ( IsPeriodic_ ) {
       
       SetupCircularVortexSheets();
       
    }
    
    // Open - planar-ish - vortex sheet
    
    else {
       
       SetupPlanarVortexSheets();
       
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
     
    m = 1;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
       
       k = 0;
       
       p = 1;
       
       for ( j = 1 ; j <= NumberOfTrailingVortices_ - 1 ; j+=m ) {
       
          k++;
    
          // Set the two trailing vortices for this sheet
          
          VortexSheetListForLevel_[Level][k].SetTrailingVortices(*TrailingVortexListForLevel_[Level][k], 
                                                                 *TrailingVortexListForLevel_[Level][k+1]);
                                                                 
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
   
}

/*##############################################################################
#                                                                              #
#                  VORTEX_SHEET SetupCircularVortexSheets                      #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::SetupCircularVortexSheets(void)
{

    int j, k, m, p, Level, Done;

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
    
//    if ( k < 4 ) Level--;
    
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
    
    // Set up the trailing vortices for each sub level
    
    m = 1;
    
    for ( Level = 1 ; Level <= NumberOfLevels_ ; Level++ ) {
       
       k = 0;
       
       for ( j = 1 ; j <= NumberOfTrailingVortices_ + 1 ; j+=m ) {
       
          k++;
    
          TrailingVortexListForLevel_[Level][k] = &(TrailingVortexList_[j]);
   
       }
       
       j -= m;
       
       if ( j != NumberOfTrailingVortices_  + 1) {
          
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
   
}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET UpdateVortexStrengths                    #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::UpdateVortexStrengths(void)
{

    int i;
    
    // Make a copy of the circulation strengths
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_  ; i++ ) {

       Gamma_[i] = TrailingVortexList_[i].Gamma();
       
    } 
        
}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET InducedVelocity                          #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::InducedVelocity(double xyz_p[3], double q[3])
{

    int i;
    double U, V, W, dq[3];
    VORTEX_TRAIL *TrailingVortex;
    VORTEX_SHEET *VortexSheet;

    // Zero evaluation level and initialize gamma to finest level
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_  ; i++ ) {

       TrailingVortexList_[i].Evaluate() = 1;
       
       TrailingVortexList_[i].Gamma() = Gamma_[i];
       
    }           
    
    // If the vortex sheet is periodic, the first and last trailing vortex are the
    // same... we double book keep for closure, and 1/2 the gamma for each
    
    if ( IsPeriodic_ ) {
       
       TrailingVortexList_[                            1].Gamma() = 0.5 * Gamma_[1];
       
       TrailingVortexList_[NumberOfTrailingVortices_ + 1].Gamma() = 0.5 * Gamma_[1];
       
    }
       
    // Agglomerate the trailing vortices
    
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
    
    q[0] = U;
    q[1] = V;
    q[2] = W;   
    
}

/*##############################################################################
#                                                                              #
#                        VORTEX_SHEET InducedVelocity                          #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::InducedVelocity(double xyz_p[3], double q[3], double xyz_te[3])
{

    int i;
    double U, V, W, dq[3], Dist;
    VORTEX_TRAIL *TrailingVortex;
    VORTEX_SHEET *VortexSheet;

    // Zero evaluation level and initialize gamma to finest level
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_  ; i++ ) {

       TrailingVortexList_[i].Evaluate() = 1;
       
       TrailingVortexList_[i].Gamma() = Gamma_[i];
       
    }           
    
    // If the vortex sheet is periodic, the first and last trailing vortex are the
    // same... we double book keep for closure, and 1/2 the gamma for each
    
    if ( IsPeriodic_ ) {
       
       TrailingVortexList_[                            1].Gamma() = 0.5 * Gamma_[1];
       
       TrailingVortexList_[NumberOfTrailingVortices_ + 1].Gamma() = 0.5 * Gamma_[1];
       
    }
       
    // Agglomerate the trailing vortices
    
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
          
          Dist = sqrt( pow(xyz_te[0] - TrailingVortexList_[i].TE_Node().x(),2.)
                     + pow(xyz_te[1] - TrailingVortexList_[i].TE_Node().y(),2.)
                     + pow(xyz_te[2] - TrailingVortexList_[i].TE_Node().z(),2.) );
          
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

    int i;
    double U, V, W, xyz_k[3], dq[3];
    VORTEX_TRAIL *TrailingVortex;
    VORTEX_SHEET *VortexSheet;

    // Zero evaluation level and initialize gamma to finest level
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_  ; i++ ) {

       TrailingVortexList_[i].Evaluate() = 1;
       
       TrailingVortexList_[i].Gamma() = Gamma_[i];
       
    }           
    
    // If the vortex sheet is periodic, the first and last trailing vortex are the
    // same... we double book keep for closure, and 1/2 the gamma for each
    
    if ( IsPeriodic_ ) {
       
       TrailingVortexList_[                            1].Gamma() = 0.5 * Gamma_[1];
       
       TrailingVortexList_[NumberOfTrailingVortices_ + 1].Gamma() = 0.5 * Gamma_[1];
       
    }

    // Agglomerate the trailing vortices

    if ( NumberOfTrailingVortices_ >= 4 ) {
   
       for ( i = 1 ; i <= NumberOfVortexSheetsForLevel_[NumberOfLevels_] ; i++ ) {
          
          VortexSheet = &VortexSheetListForLevel_[NumberOfLevels_][i];
       
          CreateTrailingVortexInteractionList(*VortexSheet, xyz_k);
          
       } 
       
    }

    // Create a unique list of the agglomerated trailing vortices

    NumberOfAgglomeratedTrailingVortices_ = 0;
    
    for ( i = 1 ; i <= NumberOfTrailingVortices_ ; i++ ) {

       if ( TrailingVortexList_[i].Evaluate() ) {
          
          AgglomeratedTrailingVortexList_[++NumberOfAgglomeratedTrailingVortices_] = &TrailingVortexList_[i];
          
       }
       
    }    

    // Evalulate the agglomerated trailing vortices
    
    q[0] = q[1] = q[2] = U = V = W = 0.;

#pragma omp parallel for reduction(+:U,V,W) private(dq,xyz_k,TrailingVortex)      
    for ( i = 1 ; i <= NumberOfAgglomeratedTrailingVortices_ ; i++ ) {

       TrailingVortex = AgglomeratedTrailingVortexList_[i];
       
       xyz_k[0] = TrailingVortex->TE_Node().x();
       xyz_k[1] = xyz_p[1];
       xyz_k[2] = xyz_p[2];
       
       TrailingVortex->InducedVelocity(xyz_k,dq);
       
       U += dq[0];
       V += dq[1];
       W += dq[2];

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

    if ( VortexSheet.FarAway(xyz_p) || VortexSheet.ThereAreChildren() == 0 ) {
       
       VortexSheet.InjectCirculation();

    }
    
    else {

       if ( VortexSheet.ThereAreChildren() >= 1 ) CreateTrailingVortexInteractionList(VortexSheet.Child1(), xyz_p);

       if ( VortexSheet.ThereAreChildren() >= 2 ) CreateTrailingVortexInteractionList(VortexSheet.Child2(), xyz_p);

    }
  
}

/*##############################################################################
#                                                                              #
#                       VORTEX_SHEET InjectCirculation                         #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::InjectCirculation(void)
{
   
    if ( ThereAreChildren_ == 2 ) {

       Child1().InjectCirculation();
       
       Child2().InjectCirculation();
       
       VortexTrail1().Gamma() += 0.5*Child1().VortexTrail2().Gamma();
    
       VortexTrail2().Gamma() += 0.5*Child1().VortexTrail2().Gamma();
    
       Child1().VortexTrail2().Evaluate() = 0;
      
    }
    
    else if ( ThereAreChildren_ == 1 ) {
       
       Child1().InjectCirculation();
            
    }
    
    else {
       
       // Nothing to do... Evaluate is set to 1 by default
       
    }
  
}

/*##############################################################################
#                                                                              #
#                            VORTEX_SHEET Setup                                #
#                                                                              #
##############################################################################*/

void VORTEX_SHEET::Setup(void)
{

    Span_ = sqrt( pow(VortexTrail1().TE_Node().y() - VortexTrail2().TE_Node().y(),2.)
                + pow(VortexTrail1().TE_Node().z() - VortexTrail2().TE_Node().z(),2.) );
               
    MidSpan_[0] = 0.5*( VortexTrail1().TE_Node().x() + VortexTrail2().TE_Node().x() );         
    MidSpan_[1] = 0.5*( VortexTrail1().TE_Node().y() + VortexTrail2().TE_Node().y() ); 
    MidSpan_[2] = 0.5*( VortexTrail1().TE_Node().z() + VortexTrail2().TE_Node().z() ); 
             
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























