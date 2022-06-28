//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "SpanLoadData.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                          SPAN_LOAD_DATA Constructor                          #
#                                                                              #
##############################################################################*/

SPAN_LOAD_DATA::SPAN_LOAD_DATA(void)
{

    NumberOfSpanStations_ = 0;
    
    // Viscous forces
      
    Span_Cxo_ = NULL;
    Span_Cyo_ = NULL;
    Span_Czo_ = NULL;  
              
    // Inviscid forces
                
    Span_Cx_ = NULL;
    Span_Cy_ = NULL;
    Span_Cz_ = NULL;

    // Trefftz, induced drag, forces
    
    Span_Cxi_ = NULL; 
    Span_Cyi_ = NULL;
    Span_Czi_ = NULL;  
    
    // Viscous moments
      
    Span_Cmxo_ = NULL; 
    Span_Cmyo_ = NULL;
    Span_Cmzo_ = NULL;
          
    // Inviscid moments
          
    Span_Cmx_ = NULL;  
    Span_Cmy_ = NULL;
    Span_Cmz_ = NULL;   
    
    // Trefftz, induced drag, moments

    Span_Cmxi_ = NULL; 
    Span_Cmyi_ = NULL;
    Span_Cmzi_ = NULL;
    
    // Lift, drag, normal, spanwise forces
            
    Span_Cn_ = NULL;
    Span_Cl_ = NULL;
    Span_Cs_ = NULL;
    Span_Cd_ = NULL;
    
    // Span Min Cp
    
    Span_CpMin_ = NULL;
    
    // Geometrical information
    
    Span_XLE_ = NULL;
    Span_YLE_ = NULL;
    Span_ZLE_ = NULL;    
        
    Span_XTE_ = NULL;
    Span_YTE_ = NULL;
    Span_ZTE_ = NULL;    
    
    Span_XLE_Def_ = NULL;
    Span_YLE_Def_ = NULL;
    Span_ZLE_Def_ = NULL;    
        
    Span_XTE_Def_ = NULL;
    Span_YTE_Def_ = NULL;
    Span_ZTE_Def_ = NULL;    
        
    Span_Xavg_ = NULL;
    Span_Yavg_ = NULL;
    Span_Zavg_ = NULL;

    Span_Svec_ = NULL;    
    
    Span_Nvec_ = NULL;
    
    Span_Area_ = NULL;
    
    Span_Chord_ = NULL;
    
    Span_S_ = NULL;
    
    Span_S_Def_ = NULL;
        
    Local_Velocity_ = NULL;
    
    StallFact_ = NULL;
    
    IsStalled_ = NULL;
    
    SlatPercentage_ = NULL;
    
    SlatMachDeploy_ = NULL;
    
    SpanStationHasSlat_ = NULL;
        
    IsARotor_ = 0;

}

/*##############################################################################
#                                                                              #
#                            SPAN_LOAD_DATA Destructor                         #
#                                                                              #
##############################################################################*/

SPAN_LOAD_DATA::~SPAN_LOAD_DATA(void)
{
       
    int i;
    
    // Viscous forces
      
    if ( Span_Cxo_ != NULL ) delete [] Span_Cxo_;
    if ( Span_Cyo_ != NULL ) delete [] Span_Cyo_;
    if ( Span_Czo_ != NULL ) delete [] Span_Czo_;
    
    Span_Cxo_ = NULL; 
    Span_Cyo_ = NULL;
    Span_Czo_ = NULL;

    // Inviscid forces
                
    if ( Span_Cx_ != NULL ) delete [] Span_Cx_;
    if ( Span_Cy_ != NULL ) delete [] Span_Cy_;
    if ( Span_Cz_ != NULL ) delete [] Span_Cz_;
    
    Span_Cx_ = NULL;
    Span_Cy_ = NULL;
    Span_Cz_ = NULL;
    
    // Trefftz, induced drag, forces
    
    if ( Span_Cxi_ != NULL ) delete [] Span_Cxi_; 
    if ( Span_Cyi_ != NULL ) delete [] Span_Cyi_;
    if ( Span_Czi_ != NULL ) delete [] Span_Czi_; 
    
    Span_Cxi_ = NULL;
    Span_Cyi_ = NULL;
    Span_Czi_ = NULL;

    // Viscous moments
      
    if ( Span_Cmxo_ != NULL ) delete [] Span_Cmxo_; 
    if ( Span_Cmyo_ != NULL ) delete [] Span_Cmyo_;
    if ( Span_Cmzo_ != NULL ) delete [] Span_Cmzo_;
    
    Span_Cmxo_ = NULL;
    Span_Cmyo_ = NULL;
    Span_Cmzo_ = NULL;
    
    // Inviscid moments
          
    if ( Span_Cmx_ != NULL ) delete [] Span_Cmx_;  
    if ( Span_Cmy_ != NULL ) delete [] Span_Cmy_;
    if ( Span_Cmz_ != NULL ) delete [] Span_Cmz_;
    
    Span_Cmx_ = NULL;
    Span_Cmy_ = NULL;
    Span_Cmz_ = NULL;
    
    // Trefftz, induced drag, moments

    if ( Span_Cmxi_ != NULL ) delete [] Span_Cmxi_; 
    if ( Span_Cmyi_ != NULL ) delete [] Span_Cmyi_;
    if ( Span_Cmzi_ != NULL ) delete [] Span_Cmzi_;
    
    Span_Cmxi_ = NULL;
    Span_Cmyi_ = NULL;
    Span_Cmzi_ = NULL;

    // Lift, drag, normal, spanwise forces
            
    if ( Span_Cn_ != NULL ) delete [] Span_Cn_; 
    if ( Span_Cl_ != NULL ) delete [] Span_Cl_; 
    if ( Span_Cs_ != NULL ) delete [] Span_Cs_; 
    if ( Span_Cd_ != NULL ) delete [] Span_Cd_; 
    
    Span_Cn_ = NULL;
    Span_Cl_ = NULL;
    Span_Cs_ = NULL;
    Span_Cd_ = NULL;
        
    // Span Min Cp

    if ( Span_CpMin_ != NULL ) delete [] Span_CpMin_; 
    
    Span_CpMin_ = NULL;
        
    // Geometrical information
    
    if ( Span_XLE_ == NULL ) delete [] Span_XLE_;
    if ( Span_YLE_ == NULL ) delete [] Span_YLE_;
    if ( Span_ZLE_ == NULL ) delete [] Span_ZLE_;   
    
    Span_XLE_ = NULL;
    Span_YLE_ = NULL;
    Span_ZLE_ = NULL;
    
    if ( Span_XTE_ == NULL ) delete [] Span_XTE_;
    if ( Span_YTE_ == NULL ) delete [] Span_YTE_;
    if ( Span_ZTE_ == NULL ) delete [] Span_ZTE_;     
    
    Span_XTE_ = NULL;
    Span_YTE_ = NULL;
    Span_ZTE_ = NULL;
      
    if ( Span_XLE_Def_ == NULL ) delete [] Span_XLE_Def_;
    if ( Span_YLE_Def_ == NULL ) delete [] Span_YLE_Def_;
    if ( Span_ZLE_Def_ == NULL ) delete [] Span_ZLE_Def_;   

    Span_XLE_Def_ = NULL;
    Span_YLE_Def_ = NULL;
    Span_ZLE_Def_ = NULL;

    if ( Span_XTE_Def_ == NULL ) delete [] Span_XTE_Def_;
    if ( Span_YTE_Def_ == NULL ) delete [] Span_YTE_Def_;
    if ( Span_ZTE_Def_ == NULL ) delete [] Span_ZTE_Def_;    
    
    Span_XTE_Def_ = NULL;
    Span_YTE_Def_ = NULL;
    Span_ZTE_Def_ = NULL;
             
    if ( Span_Xavg_ != NULL ) delete [] Span_Xavg_;
    if ( Span_Yavg_ != NULL ) delete [] Span_Yavg_;
    if ( Span_Zavg_ != NULL ) delete [] Span_Zavg_;
    
    Span_Xavg_ = NULL;
    Span_Yavg_ = NULL;
    Span_Zavg_ = NULL;
    
    if ( Span_Svec_ != NULL ) {

       for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {
                 
          delete [] Span_Svec_[i];
          
          Span_Svec_[i] = NULL;
          
       }
       
       delete Span_Svec_;
       
    }
    
    Span_Svec_ = NULL;
    
    if ( Span_Nvec_ != NULL ) {

       for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {
                 
          delete [] Span_Nvec_[i];
          
          Span_Nvec_[i] = NULL;
          
       }
       
       delete Span_Nvec_;
       
    }
    
    Span_Nvec_ = NULL;
        
    if ( Span_Area_ != NULL ) delete [] Span_Area_;
    
    Span_Area_ = NULL;
    
    if ( Span_Chord_ != NULL ) delete [] Span_Chord_;
    
    Span_Chord_ = NULL;
    
    if ( Span_S_ != NULL ) delete [] Span_S_;
    
    Span_S_ = NULL;
    
    if ( Span_S_Def_ != NULL ) delete [] Span_S_Def_;
    
    Span_S_Def_ = NULL;

    if ( Local_Velocity_ != NULL ) {

       for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {
                 
          delete [] Local_Velocity_[i];
          
          Local_Velocity_[i] = NULL;
          
       }
       
       delete Local_Velocity_;
       
       Local_Velocity_ = NULL;
       
    }

    if ( StallFact_ != NULL ) delete [] StallFact_;
    
    StallFact_ = NULL;
    
    if ( IsStalled_ != NULL ) delete [] IsStalled_;
    
    IsStalled_ = NULL;
    
    if ( SlatPercentage_ != NULL ) delete [] SlatPercentage_;
    
    SlatPercentage_ = NULL;
    
    if ( SlatMachDeploy_ != NULL ) delete [] SlatMachDeploy_;
    
    SlatMachDeploy_ = NULL;
    
    if ( SpanStationHasSlat_ != NULL ) delete [] SpanStationHasSlat_;
    
    SpanStationHasSlat_ = NULL;
    
    NumberOfSpanStations_ = 0;
    
    IsARotor_ = 0;

}

/*##############################################################################
#                                                                              #
#                           SPAN_LOAD_DATA Copy                                #
#                                                                              #
##############################################################################*/

SPAN_LOAD_DATA::SPAN_LOAD_DATA(const SPAN_LOAD_DATA &SpanLoadData)
{

    // Use operator =

    (*this)= SpanLoadData;

}

/*##############################################################################
#                                                                              #
#                          SPAN_LOAD_DATA Operator =                           #
#                                                                              #
##############################################################################*/

SPAN_LOAD_DATA &SPAN_LOAD_DATA::operator=(const SPAN_LOAD_DATA &SpanLoadData)
{

    int i, j;
    
    Size(SpanLoadData.NumberOfSpanStations_);
    
    ZeroAll();
    
    for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {
       
       // Viscous forces
         
       Span_Cxo_[i] = SpanLoadData.Span_Cxo_[i];
       Span_Cyo_[i] = SpanLoadData.Span_Cyo_[i];
       Span_Czo_[i] = SpanLoadData.Span_Czo_[i];
                 
       // Inviscid forces
                   
       Span_Cx_[i] = SpanLoadData.Span_Cx_[i];
       Span_Cy_[i] = SpanLoadData.Span_Cy_[i];
       Span_Cz_[i] = SpanLoadData.Span_Cz_[i];
  
       // Trefftz, induced drag, forces
       
       Span_Cxi_[i] = SpanLoadData.Span_Cxi_[i]; 
       Span_Cyi_[i] = SpanLoadData.Span_Cyi_[i];
       Span_Czi_[i] = SpanLoadData.Span_Czi_[i]; 
       
       // Viscous moments
         
       Span_Cmxo_[i] = SpanLoadData.Span_Cmxo_[i];
       Span_Cmyo_[i] = SpanLoadData.Span_Cmyo_[i];
       Span_Cmzo_[i] = SpanLoadData.Span_Cmzo_[i];
             
       // Inviscid moments
        
       Span_Cmx_[i] = SpanLoadData.Span_Cmx_[i];
       Span_Cmy_[i] = SpanLoadData.Span_Cmy_[i];
       Span_Cmz_[i] = SpanLoadData.Span_Cmz_[i];
        
       // Trefftz, induced drag, moments
       
       Span_Cmxi_[i] = SpanLoadData.Span_Cmxi_[i];
       Span_Cmyi_[i] = SpanLoadData.Span_Cmyi_[i];
       Span_Cmzi_[i] = SpanLoadData.Span_Cmzi_[i];
       
       // Lift, drag, normal, spanwise forces
               
       Span_Cn_[i] = SpanLoadData.Span_Cn_[i];
       Span_Cl_[i] = SpanLoadData.Span_Cl_[i];
       Span_Cs_[i] = SpanLoadData.Span_Cs_[i];
       Span_Cd_[i] = SpanLoadData.Span_Cd_[i];
       
       // Span Min Cp

       Span_CpMin_[i] = SpanLoadData.Span_CpMin_[i];

       // Geometrical information

       Span_XLE_[i] = SpanLoadData.Span_XLE_[i];
       Span_YLE_[i] = SpanLoadData.Span_YLE_[i];
       Span_ZLE_[i] = SpanLoadData.Span_ZLE_[i];   
              
       Span_XTE_[i] = SpanLoadData.Span_XTE_[i];
       Span_YTE_[i] = SpanLoadData.Span_YTE_[i];
       Span_ZTE_[i] = SpanLoadData.Span_ZTE_[i];   

       Span_XLE_Def_[i] = SpanLoadData.Span_XLE_Def_[i];
       Span_YLE_Def_[i] = SpanLoadData.Span_YLE_Def_[i];
       Span_ZLE_Def_[i] = SpanLoadData.Span_ZLE_Def_[i];   
              
       Span_XTE_Def_[i] = SpanLoadData.Span_XTE_Def_[i];
       Span_YTE_Def_[i] = SpanLoadData.Span_YTE_Def_[i];
       Span_ZTE_Def_[i] = SpanLoadData.Span_ZTE_Def_[i];   
                  
       Span_Xavg_[i] = SpanLoadData.Span_Xavg_[i];
       Span_Yavg_[i] = SpanLoadData.Span_Yavg_[i];
       Span_Zavg_[i] = SpanLoadData.Span_Zavg_[i];

       for ( j = 0 ; j <= 2 ; j++ ) {
                
          Span_Svec_[i][j] = SpanLoadData.Span_Svec_[i][j];
          
       }

       for ( j = 0 ; j <= 2 ; j++ ) {
                
          Span_Nvec_[i][j] = SpanLoadData.Span_Nvec_[i][j];
          
       }
                     
       Span_Area_[i] = SpanLoadData.Span_Area_[i];
       
       Span_Chord_[i] = SpanLoadData.Span_Chord_[i];
       
       Span_S_[i] = SpanLoadData.Span_S_[i];

       Span_S_Def_[i] = SpanLoadData.Span_S_Def_[i];

       for ( j = 0 ; j <= 3 ; j++ ) {
                
          Local_Velocity_[i][j] = SpanLoadData.Local_Velocity_[i][j];
          
       }
       
       StallFact_[i] = SpanLoadData.StallFact_[i];
       
       IsStalled_[i] = SpanLoadData.IsStalled_[i];
       
       SlatPercentage_[i] = SpanLoadData.SlatPercentage_[i];
       
       SlatMachDeploy_[i] = SpanLoadData.SlatMachDeploy_[i];
       
       SpanStationHasSlat_[i] = SpanLoadData.SpanStationHasSlat_[i];
              
    }       
    
    IsARotor_ = SpanLoadData.IsARotor_;
        
    return *this;

}

/*##############################################################################
#                                                                              #
#                            SPAN_LOAD_DATA Size                               #
#                                                                              #
##############################################################################*/

void SPAN_LOAD_DATA::Size(int NumberOfSpanStations)
{

    int i;

    NumberOfSpanStations_ = NumberOfSpanStations;
    
    // Viscous forces
      
    Span_Cxo_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Cyo_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Czo_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
              
    // Inviscid forces
                
    Span_Cx_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Cy_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Cz_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
  
    // Trefftz, induced drag, forces
    
    Span_Cxi_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1]; 
    Span_Cyi_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Czi_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1]; 
    
    // Viscous moments
      
    Span_Cmxo_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Cmyo_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Cmzo_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
          
    // Inviscid moments
     
    Span_Cmx_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Cmy_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Cmz_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
      
    // Trefftz, induced drag, moments

    Span_Cmxi_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Cmyi_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Cmzi_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    
    // Lift, drag, normal, spanwise forces
            
    Span_Cn_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Cl_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Cs_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Cd_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    
    // Span Min Cp
    
    Span_CpMin_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    
    // Geometrical information

    Span_XLE_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_YLE_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_ZLE_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1]; 
        
    Span_XTE_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_YTE_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_ZTE_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1]; 

    Span_XLE_Def_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_YLE_Def_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_ZLE_Def_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1]; 
        
    Span_XTE_Def_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_YTE_Def_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_ZTE_Def_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1]; 
               
    Span_Xavg_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Yavg_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    Span_Zavg_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];

    Span_Svec_ = new VSPAERO_DOUBLE*[NumberOfSpanStations_ + 1];

    for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {
              
       Span_Svec_[i] = new VSPAERO_DOUBLE[3];
       
    }
    
    Span_Nvec_= new VSPAERO_DOUBLE*[NumberOfSpanStations_ + 1];
    
    for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {
              
       Span_Nvec_[i] = new VSPAERO_DOUBLE[3];
       
    }
            
    Span_Area_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    
    Span_Chord_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    
    Span_S_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];

    Span_S_Def_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];

    Local_Velocity_ = new VSPAERO_DOUBLE*[NumberOfSpanStations_ + 1];
    
    for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {
              
       Local_Velocity_[i] = new VSPAERO_DOUBLE[4];
       
    }

    // Stall Flag

    StallFact_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    
    IsStalled_ = new int[NumberOfSpanStations_ + 1];
    
    SlatPercentage_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    
    SlatMachDeploy_ = new VSPAERO_DOUBLE[NumberOfSpanStations_ + 1];
    
    SpanStationHasSlat_ = new int[NumberOfSpanStations_ + 1];
    
    ZeroAll();

}

/*##############################################################################
#                                                                              #
#                       SPAN_LOAD_DATA ZeroAll                                 #
#                                                                              #
##############################################################################*/

void SPAN_LOAD_DATA::ZeroAll(void)
{

    int i, j;

    for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {
       
       // Viscous forces
         
       Span_Cxo_[i] = 0.;
       Span_Cyo_[i] = 0.;
       Span_Czo_[i] = 0.;
                 
       // Inviscid forces
                   
       Span_Cx_[i] = 0.;
       Span_Cy_[i] = 0.;
       Span_Cz_[i] = 0.;

       // Trefftz, induced drag, forces
       
       Span_Cxi_[i] = 0.;
       Span_Cyi_[i] = 0.;
       Span_Czi_[i] = 0.;
       
       // Viscous moments
         
       Span_Cmxo_[i] = 0.;
       Span_Cmyo_[i] = 0.;
       Span_Cmzo_[i] = 0.;
             
       // Inviscid moments
        
       Span_Cmx_[i] = 0.;
       Span_Cmy_[i] = 0.;
       Span_Cmz_[i] = 0.;
   
       // Trefftz, induced drag, moments
       
       Span_Cmxi_[i] = 0.;
       Span_Cmyi_[i] = 0.;
       Span_Cmzi_[i] = 0.;
       
       // Lift, drag, normal, spanwise forces
               
       Span_Cn_[i] = 0.;
       Span_Cl_[i] = 0.;
       Span_Cs_[i] = 0.;
       Span_Cd_[i] = 0.;
       
       // Span Min Cp
       
       Span_CpMin_[i] = 0.;
           
       // Geometrical information       
    
       Span_XLE_[i] = 0.;
       Span_YLE_[i] = 0.;
       Span_ZLE_[i] = 0.;
           
       Span_XTE_[i] = 0.;
       Span_YTE_[i] = 0.;
       Span_ZTE_[i] = 0.;

       Span_XLE_Def_[i] = 0.;
       Span_YLE_Def_[i] = 0.;
       Span_ZLE_Def_[i] = 0.;
           
       Span_XTE_Def_[i] = 0.;
       Span_YTE_Def_[i] = 0.;
       Span_ZTE_Def_[i] = 0.;
              
       Span_Xavg_[i] = 0.;
       Span_Yavg_[i] = 0.;
       Span_Zavg_[i] = 0.;

       for ( j = 0 ; j <= 2 ; j++ ) {
                
          Span_Svec_[i][j] = 0.;
          
       }
       
       for ( j = 0 ; j <= 2 ; j++ ) {
                
          Span_Nvec_[i][j] = 0.;
          
       }       
              
       Span_Area_[i] = 0.;
       
       Span_Chord_[i] = 0.;
       
       Span_S_[i] = 0.;
       
       Span_S_Def_[i] = 0.;

       for ( j = 0 ; j <= 3 ; j++ ) {
                
          Local_Velocity_[i][j] = 0.;
          
       }
       
       // Stall Flag
       
       StallFact_[i] = 0.;
       
       IsStalled_[i] = 0;
    
       SlatPercentage_[i] = 0.;
       
       SlatMachDeploy_[i] = 0.;
       
       SpanStationHasSlat_[i] = 0;

    }       
    
}

/*##############################################################################
#                                                                              #
#                    SPAN_LOAD_DATA ZeroForcesAndMoments                       #
#                                                                              #
##############################################################################*/

void SPAN_LOAD_DATA::ZeroForcesAndMoments(void)
{

    int i, j;

    for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {
       
       // Viscous forces
         
       Span_Cxo_[i] = 0.;
       Span_Cyo_[i] = 0.;
       Span_Czo_[i] = 0.;
                 
       // Inviscid forces
                   
       Span_Cx_[i] = 0.;
       Span_Cy_[i] = 0.;
       Span_Cz_[i] = 0.;
  
       // Trefftz, induced drag, forces
       
       Span_Cxi_[i] = 0.;
       Span_Cyi_[i] = 0.;
       Span_Czi_[i] = 0.;
       
       // Viscous moments
         
       Span_Cmxo_[i] = 0.;
       Span_Cmyo_[i] = 0.;
       Span_Cmzo_[i] = 0.;
             
       // Inviscid moments
        
       Span_Cmx_[i] = 0.;
       Span_Cmy_[i] = 0.;
       Span_Cmz_[i] = 0.;
    
       // Trefftz, induced drag, moments
       
       Span_Cmxi_[i] = 0.;
       Span_Cmyi_[i] = 0.;
       Span_Cmzi_[i] = 0.;
       
       // Lift, drag, normal, spanwise forces
               
       Span_Cn_[i] = 0.;
       Span_Cl_[i] = 0.;
       Span_Cs_[i] = 0.;
       Span_Cd_[i] = 0.;
              
       // Span Min Cp
       
       Span_CpMin_[i] = 0.;       
       
       // Geometrical information
       
       Span_Xavg_[i] = 0.;
       Span_Yavg_[i] = 0.;
       Span_Zavg_[i] = 0.;
              
       // Area
       
       Span_Area_[i] = 0.;
              
       // Velocity
       
       for ( j = 0 ; j <= 3 ; j++ ) {
                
          Local_Velocity_[i][j] = 0.;
          
       }
       
       // Stall Flag
        
       StallFact_[i] = 0.;
       
       IsStalled_[i] = 0;
     
    }     
    
}

/*##############################################################################
#                                                                              #
#                    SPAN_LOAD_DATA UpdateGeometryLocation                     #
#                                                                              #
##############################################################################*/

void SPAN_LOAD_DATA::UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat)
{


    int i;
    QUAT Vec;

    for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {

       // Leading edge location
       
       Vec(0) = Span_XLE_[i] - OVec[0];
       Vec(1) = Span_YLE_[i] - OVec[1];
       Vec(2) = Span_ZLE_[i] - OVec[2];   
     
       Vec = Quat * Vec * InvQuat;

       Span_XLE_[i] = Vec(0) + OVec[0] + TVec[0];
       Span_YLE_[i] = Vec(1) + OVec[1] + TVec[1];
       Span_ZLE_[i] = Vec(2) + OVec[2] + TVec[2];      

       // Trailing edge location
       
       Vec(0) = Span_XTE_[i] - OVec[0];
       Vec(1) = Span_YTE_[i] - OVec[1];
       Vec(2) = Span_ZTE_[i] - OVec[2];   
       
       Vec = Quat * Vec * InvQuat;

       Span_XTE_[i] = Vec(0) + OVec[0] + TVec[0];
       Span_YTE_[i] = Vec(1) + OVec[1] + TVec[1];
       Span_ZTE_[i] = Vec(2) + OVec[2] + TVec[2];      

       // Defelected Leading edge location
       
       Vec(0) = Span_XLE_Def_[i] - OVec[0];
       Vec(1) = Span_YLE_Def_[i] - OVec[1];
       Vec(2) = Span_ZLE_Def_[i] - OVec[2];   
       
       Vec = Quat * Vec * InvQuat;

       Span_XLE_Def_[i] = Vec(0) + OVec[0] + TVec[0];
       Span_YLE_Def_[i] = Vec(1) + OVec[1] + TVec[1];
       Span_ZLE_Def_[i] = Vec(2) + OVec[2] + TVec[2];      

       // Deflected Trailing edge location
       
       Vec(0) = Span_XTE_Def_[i] - OVec[0];
       Vec(1) = Span_YTE_Def_[i] - OVec[1];
       Vec(2) = Span_ZTE_Def_[i] - OVec[2];   

       Vec = Quat * Vec * InvQuat;

       Span_XTE_Def_[i] = Vec(0) + OVec[0] + TVec[0];
       Span_YTE_Def_[i] = Vec(1) + OVec[1] + TVec[1];
       Span_ZTE_Def_[i] = Vec(2) + OVec[2] + TVec[2];     
                     
       // Direction vector 

       Vec(0) = Span_Svec_[i][0];
       Vec(1) = Span_Svec_[i][1];
       Vec(2) = Span_Svec_[i][2];
       
       Vec = Quat * Vec * InvQuat;
       
       Span_Svec_[i][0] = Vec(0); 
       Span_Svec_[i][1] = Vec(1); 
       Span_Svec_[i][2] = Vec(2);    
       
       // Normal vector
       
       Vec(0) = Span_Nvec_[i][0];
       Vec(1) = Span_Nvec_[i][1];
       Vec(2) = Span_Nvec_[i][2];
       Vec(3) = 0.;
       
       Vec = Quat * Vec * InvQuat;

       Span_Nvec_[i][0] = Vec(0); 
       Span_Nvec_[i][1] = Vec(1); 
       Span_Nvec_[i][2] = Vec(2);    
                     
    }    

}

#include "END_NAME_SPACE.H"
