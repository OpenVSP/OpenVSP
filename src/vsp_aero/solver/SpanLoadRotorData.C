//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "SpanLoadRotorData.H"

/*##############################################################################
#                                                                              #
#                          SPAN_LOAD_ROTOR_DATA Constructor                    #
#                                                                              #
##############################################################################*/

SPAN_LOAD_ROTOR_DATA::SPAN_LOAD_ROTOR_DATA(void)
{

    NumberOfSpanStations_ = 0;

    NumberOfTimeSamples_ = 0;

    ActualTimeSamples_ = 0;
    
    SurfaceID_ = 0;
    
    Span_Cxo_ = NULL;
    Span_Cyo_ = NULL;
    Span_Czo_ = NULL;
                  
    Span_Cx_ = NULL;
    Span_Cy_ = NULL;
    Span_Cz_ = NULL;

    Span_Cmxo_ = NULL;
    Span_Cmyo_ = NULL;
    Span_Cmzo_ = NULL;
                      
    Span_Cmx_ = NULL;
    Span_Cmy_ = NULL;
    Span_Cmz_ = NULL;
    
    Span_Cl_ = NULL;
    Span_Cd_ = NULL;
    Span_Cw_ = NULL;        
    
    Span_Cno_ = NULL;
    Span_Cso_ = NULL;   
    Span_Cto_ = NULL;
    Span_Cqo_ = NULL;
    Span_Cpo_ = NULL;  

    Span_Cn_ = NULL;
    Span_Cs_ = NULL;   
    Span_Ct_ = NULL;
    Span_Cq_ = NULL;
    Span_Cp_ = NULL;  
    
    Time_ = NULL;
    
    XYZ_QC_[0] = NULL;
    XYZ_QC_[1] = NULL;
    XYZ_QC_[2] = NULL;
    
    RotationAngle_ = NULL;
    Span_S_ = NULL;
    Span_Area_ = NULL;
    Span_Chord_ = NULL;
    Local_Velocity_ = NULL;

}

/*##############################################################################
#                                                                              #
#                            SPAN_LOAD_ROTOR_DATA Destructor                   #
#                                                                              #
##############################################################################*/

SPAN_LOAD_ROTOR_DATA::~SPAN_LOAD_ROTOR_DATA(void)
{

    if ( Span_Cxo_       != NULL ) delete [] Span_Cxo_;  
    if ( Span_Cyo_       != NULL ) delete [] Span_Cyo_;  
    if ( Span_Czo_       != NULL ) delete [] Span_Czo_;  
                                                        
    if ( Span_Cx_        != NULL ) delete [] Span_Cx_;   
    if ( Span_Cy_        != NULL ) delete [] Span_Cy_;   
    if ( Span_Cz_        != NULL ) delete [] Span_Cz_;   
                                   
    if ( Span_Cmxo_      != NULL ) delete [] Span_Cmxo_; 
    if ( Span_Cmyo_      != NULL ) delete [] Span_Cmyo_; 
    if ( Span_Cmzo_      != NULL ) delete [] Span_Cmzo_; 
                                                                                                 
    if ( Span_Cmx_       != NULL ) delete [] Span_Cmx_; 
    if ( Span_Cmy_       != NULL ) delete [] Span_Cmy_; 
    if ( Span_Cmz_       != NULL ) delete [] Span_Cmz_; 
                                            
    if ( Span_Cl_        != NULL ) delete [] Span_Cl_;
    if ( Span_Cd_        != NULL ) delete [] Span_Cd_;
    if ( Span_Cw_        != NULL ) delete [] Span_Cw_;   
                         
    if ( Span_Cno_       != NULL ) delete [] Span_Cno_;
    if ( Span_Cso_       != NULL ) delete [] Span_Cso_;  
    if ( Span_Cto_       != NULL ) delete [] Span_Cto_;
    if ( Span_Cqo_       != NULL ) delete [] Span_Cqo_;
    if ( Span_Cpo_       != NULL ) delete [] Span_Cpo_; 
                                
    if ( Span_Cn_        != NULL ) delete [] Span_Cn_;
    if ( Span_Cs_        != NULL ) delete [] Span_Cs_;
    if ( Span_Ct_        != NULL ) delete [] Span_Ct_;  
    if ( Span_Cq_        != NULL ) delete [] Span_Cq_;
    if ( Span_Cp_        != NULL ) delete [] Span_Cp_;  

    if ( Time_           != NULL ) delete [] Time_;   
    if ( XYZ_QC_[0]      != NULL ) delete [] XYZ_QC_[0];   
    if ( XYZ_QC_[1]      != NULL ) delete [] XYZ_QC_[1];   
    if ( XYZ_QC_[2]      != NULL ) delete [] XYZ_QC_[2];
    if ( RotationAngle_  != NULL ) delete [] RotationAngle_;   
    if ( Span_S_         != NULL ) delete [] Span_S_;        
    if ( Span_Area_      != NULL ) delete [] Span_Area_;     
    if ( Span_Chord_     != NULL ) delete [] Span_Chord_;    
    if ( Local_Velocity_ != NULL ) delete [] Local_Velocity_;
    
    SurfaceID_ = 0;
    
    NumberOfSpanStations_ = 0;

    NumberOfTimeSamples_ = 0;

    ActualTimeSamples_ = 0;

}

/*##############################################################################
#                                                                              #
#                           SPAN_LOAD_ROTOR_DATA Copy                          #
#                                                                              #
##############################################################################*/

SPAN_LOAD_ROTOR_DATA::SPAN_LOAD_ROTOR_DATA(const SPAN_LOAD_ROTOR_DATA &SPAN_LOAD_ROTOR_DATA)
{

    // Use operator =

    (*this)= SPAN_LOAD_ROTOR_DATA;

}

/*##############################################################################
#                                                                              #
#                          SPAN_LOAD_ROTOR_DATA Operator =                     #
#                                                                              #
##############################################################################*/

SPAN_LOAD_ROTOR_DATA &SPAN_LOAD_ROTOR_DATA::operator=(const SPAN_LOAD_ROTOR_DATA &SpanLoadData)
{

    int i;
    
    SurfaceID_ = SpanLoadData.SurfaceID_;

    if ( Span_Cxo_       != NULL ) delete [] Span_Cxo_;  
    if ( Span_Cyo_       != NULL ) delete [] Span_Cyo_;  
    if ( Span_Czo_       != NULL ) delete [] Span_Czo_;  
                                                        
    if ( Span_Cx_        != NULL ) delete [] Span_Cx_;   
    if ( Span_Cy_        != NULL ) delete [] Span_Cy_;   
    if ( Span_Cz_        != NULL ) delete [] Span_Cz_;   
               
    if ( Span_Cmxo_      != NULL ) delete [] Span_Cmxo_; 
    if ( Span_Cmyo_      != NULL ) delete [] Span_Cmyo_; 
    if ( Span_Cmzo_      != NULL ) delete [] Span_Cmzo_; 
                                                                                                                  
    if ( Span_Cmx_       != NULL ) delete [] Span_Cmx_; 
    if ( Span_Cmy_       != NULL ) delete [] Span_Cmy_; 
    if ( Span_Cmz_       != NULL ) delete [] Span_Cmz_; 
                                            
    if ( Span_Cl_        != NULL ) delete [] Span_Cl_;
    if ( Span_Cd_        != NULL ) delete [] Span_Cd_;
    if ( Span_Cw_        != NULL ) delete [] Span_Cw_;   

    if ( Span_Cno_       != NULL ) delete [] Span_Cno_;
    if ( Span_Cso_       != NULL ) delete [] Span_Cso_;  
    if ( Span_Cto_       != NULL ) delete [] Span_Cto_;
    if ( Span_Cqo_       != NULL ) delete [] Span_Cqo_;
    if ( Span_Cpo_       != NULL ) delete [] Span_Cpo_; 
                                
    if ( Span_Cn_        != NULL ) delete [] Span_Cn_;
    if ( Span_Cs_        != NULL ) delete [] Span_Cs_;
    if ( Span_Ct_        != NULL ) delete [] Span_Ct_;  
    if ( Span_Cq_        != NULL ) delete [] Span_Cq_;
    if ( Span_Cp_        != NULL ) delete [] Span_Cp_;  

    if ( Time_           != NULL ) delete [] Time_;   
    if ( XYZ_QC_[0]      != NULL ) delete [] XYZ_QC_[0];   
    if ( XYZ_QC_[1]      != NULL ) delete [] XYZ_QC_[1];   
    if ( XYZ_QC_[2]      != NULL ) delete [] XYZ_QC_[2];   
    if ( RotationAngle_  != NULL ) delete [] RotationAngle_;                           
    if ( Span_S_         != NULL ) delete [] Span_S_;        
    if ( Span_Area_      != NULL ) delete [] Span_Area_;     
    if ( Span_Chord_     != NULL ) delete [] Span_Chord_;    
    if ( Local_Velocity_ != NULL ) delete [] Local_Velocity_;

    NumberOfSpanStations_ = SpanLoadData.NumberOfSpanStations_;
    
    NumberOfTimeSamples_ = SpanLoadData.NumberOfTimeSamples_;
    
    ActualTimeSamples_ = SpanLoadData.ActualTimeSamples_;
    
    Span_Cxo_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cyo_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Czo_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
                  
    Span_Cx_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cy_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cz_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    Span_Cmxo_  = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cmyo_  = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cmzo_  = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    
    Span_Cmx_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cmy_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cmz_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    
    Span_Cl_ = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cd_ = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cw_ = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];    
        
    Span_Cno_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cso_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cto_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cqo_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cpo_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
                                                                           
    Span_Cn_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cs_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Ct_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cq_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cp_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cp_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    
    Time_ = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    XYZ_QC_[0] = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    XYZ_QC_[1] = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    XYZ_QC_[2] = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    RotationAngle_  = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_S_         = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Area_      = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Chord_     = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Local_Velocity_ = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    for ( i = 1 ; i <= NumberOfSpanStations_*MAX(ActualTimeSamples_,NumberOfTimeSamples_) ; i++ ) {

       Span_Cxo_[i]       = SpanLoadData.Span_Cxo_[i];
       Span_Cyo_[i]       = SpanLoadData.Span_Cyo_[i];   
       Span_Czo_[i]       = SpanLoadData.Span_Czo_[i];   
                
       Span_Cx_[i]        = SpanLoadData.Span_Cx_[i]; 
       Span_Cy_[i]        = SpanLoadData.Span_Cy_[i];    
       Span_Cz_[i]        = SpanLoadData.Span_Cz_[i];    
           
       Span_Cmxo_[i]      = SpanLoadData.Span_Cmx_[i];
       Span_Cmyo_[i]      = SpanLoadData.Span_Cmy_[i];   
       Span_Cmzo_[i]      = SpanLoadData.Span_Cmz_[i];   
                    
       Span_Cmx_[i]       = SpanLoadData.Span_Cmx_[i];
       Span_Cmy_[i]       = SpanLoadData.Span_Cmy_[i];   
       Span_Cmz_[i]       = SpanLoadData.Span_Cmz_[i];   
    
       Span_Cno_[i]       = SpanLoadData.Span_Cno_[i];
       Span_Cso_[i]       = SpanLoadData.Span_Cso_[i];    
       Span_Cto_[i]       = SpanLoadData.Span_Cto_[i];                                           
       Span_Cqo_[i]       = SpanLoadData.Span_Cqo_[i];
       Span_Cpo_[i]       = SpanLoadData.Span_Cpo_[i];
       
       Span_Cn_[i]        = SpanLoadData.Span_Cn_[i];
       Span_Cs_[i]        = SpanLoadData.Span_Cs_[i];    
       Span_Ct_[i]        = SpanLoadData.Span_Ct_[i];                                           
       Span_Cq_[i]        = SpanLoadData.Span_Cq_[i];
       Span_Cp_[i]        = SpanLoadData.Span_Cp_[i];
        
       XYZ_QC_[0][i]      = SpanLoadData.XYZ_QC_[0][i];
       XYZ_QC_[1][i]      = SpanLoadData.XYZ_QC_[1][i];
       XYZ_QC_[2][i]      = SpanLoadData.XYZ_QC_[2][i];
       
       Time_[i]           = SpanLoadData.Time_[i];
       
       RotationAngle_[i]  = SpanLoadData.RotationAngle_[i];
     
       Span_S_[i]         = SpanLoadData.Span_S_[i];        
       Span_Area_[i]      = SpanLoadData.Span_Area_[i];     
       Span_Chord_[i]     = SpanLoadData.Span_Chord_[i];    
       Local_Velocity_[i] = SpanLoadData.Local_Velocity_[i];
       
    }
        
    return *this;

}

/*##############################################################################
#                                                                              #
#                  SPAN_LOAD_ROTOR_DATA SizeSpanLoadingList                    #
#                                                                              #
##############################################################################*/

void SPAN_LOAD_ROTOR_DATA::SizeSpanLoadingList(void)
{

    int i;
    
    if ( Span_Cxo_       != NULL ) delete [] Span_Cxo_;  
    if ( Span_Cyo_       != NULL ) delete [] Span_Cyo_;  
    if ( Span_Czo_       != NULL ) delete [] Span_Czo_;  
                                                        
    if ( Span_Cx_        != NULL ) delete [] Span_Cx_;   
    if ( Span_Cy_        != NULL ) delete [] Span_Cy_;   
    if ( Span_Cz_        != NULL ) delete [] Span_Cz_;   

    if ( Span_Cmxo_      != NULL ) delete [] Span_Cmxo_; 
    if ( Span_Cmyo_      != NULL ) delete [] Span_Cmyo_; 
    if ( Span_Cmzo_      != NULL ) delete [] Span_Cmzo_; 
                                                         
    if ( Span_Cmx_       != NULL ) delete [] Span_Cmx_; 
    if ( Span_Cmy_       != NULL ) delete [] Span_Cmy_; 
    if ( Span_Cmz_       != NULL ) delete [] Span_Cmz_; 
                                            
    if ( Span_Cl_        != NULL ) delete [] Span_Cl_;
    if ( Span_Cd_        != NULL ) delete [] Span_Cd_;
    if ( Span_Cw_        != NULL ) delete [] Span_Cw_;   
        
    if ( Span_Cno_       != NULL ) delete [] Span_Cno_;
    if ( Span_Cso_       != NULL ) delete [] Span_Cso_;  
    if ( Span_Cto_       != NULL ) delete [] Span_Cto_;
    if ( Span_Cqo_       != NULL ) delete [] Span_Cqo_;
    if ( Span_Cpo_       != NULL ) delete [] Span_Cpo_; 
                                
    if ( Span_Cn_        != NULL ) delete [] Span_Cn_;
    if ( Span_Cs_        != NULL ) delete [] Span_Cs_;
    if ( Span_Ct_        != NULL ) delete [] Span_Ct_;  
    if ( Span_Cq_        != NULL ) delete [] Span_Cq_;
    if ( Span_Cp_        != NULL ) delete [] Span_Cp_;  

    if ( Time_           != NULL ) delete [] Time_;                                                          
    if ( XYZ_QC_[0]      != NULL ) delete [] XYZ_QC_[0];   
    if ( XYZ_QC_[1]      != NULL ) delete [] XYZ_QC_[1];   
    if ( XYZ_QC_[2]      != NULL ) delete [] XYZ_QC_[2];
    if ( RotationAngle_  != NULL ) delete [] RotationAngle_;                           
    if ( Span_S_         != NULL ) delete [] Span_S_;        
    if ( Span_Area_      != NULL ) delete [] Span_Area_;     
    if ( Span_Chord_     != NULL ) delete [] Span_Chord_;    
    if ( Local_Velocity_ != NULL ) delete [] Local_Velocity_;
    
    NumberOfTimeSamples_ = MAX(NumberOfTimeSamples_, 1);
    
    if ( NumberOfSpanStations_ == 0 ) {
       
       PRINTF("Error in setting up storage for span loading information! \n");
       
       PRINTF("NumberOfSpanStations_: %d \n",NumberOfSpanStations_);
       
       fflush(NULL);exit(1);
       
    }

    ActualTimeSamples_ = 0;

    Span_Cxo_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cyo_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Czo_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
                  
    Span_Cx_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cy_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cz_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
      
    Span_Cmxo_  = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cmyo_  = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cmzo_  = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
                      
    Span_Cmx_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cmy_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cmz_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    
    Span_Cl_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cd_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cw_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];    
        
    Span_Cno_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cso_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cto_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cqo_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cpo_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
                                                                           
    Span_Cn_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cs_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Ct_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cq_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cp_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cp_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    
    Span_Cno_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cso_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cto_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cqo_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cpo_   = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
                                                                           
    Span_Cn_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cs_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Ct_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cq_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cp_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cp_    = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    Time_ = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    
    XYZ_QC_[0] = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    XYZ_QC_[1] = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    XYZ_QC_[2] = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    RotationAngle_  = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_S_         = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Area_      = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Chord_     = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Local_Velocity_ = new VSPAERO_DOUBLE[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    for ( i = 0 ; i <= NumberOfSpanStations_*NumberOfTimeSamples_ ; i++ ) {

             Span_Cxo_[i] = 0.;
             Span_Cyo_[i] = 0.; 
             Span_Czo_[i] = 0.; 
                          
              Span_Cx_[i] = 0.;
              Span_Cy_[i] = 0.; 
              Span_Cz_[i] = 0.; 
                          
            Span_Cmxo_[i] = 0.;
            Span_Cmyo_[i] = 0.; 
            Span_Cmzo_[i] = 0.; 
                          
             Span_Cmx_[i] = 0.;
             Span_Cmy_[i] = 0.; 
             Span_Cmz_[i] = 0.; 
                          
              Span_Cl_[i] = 0.;
              Span_Cd_[i] = 0.; 
              Span_Cw_[i] = 0.;    
                          
             Span_Cno_[i] = 0.;
             Span_Cso_[i] = 0.;    
             Span_Cto_[i] = 0.;                                           
             Span_Cqo_[i] = 0.;
             Span_Cpo_[i] = 0.;
                          
              Span_Cn_[i] = 0.;
              Span_Cs_[i] = 0.;    
              Span_Ct_[i] = 0.;                                           
              Span_Cq_[i] = 0.;
              Span_Cp_[i] = 0.;
                          
            XYZ_QC_[0][i] = 0.;
            XYZ_QC_[1][i] = 0.;
            XYZ_QC_[2][i] = 0.;
       
                 Time_[i] = 0.;
       
        RotationAngle_[i] = 0.;

               Span_S_[i] = 0.;
            Span_Area_[i] = 0.;
           Span_Chord_[i] = 0.;
       Local_Velocity_[i] = 0.;
       
    }

}

/*##############################################################################
#                                                                              #
#               SPAN_LOAD_ROTOR_DATA CalculateAverageForcesAndMoments          #
#                                                                              #
##############################################################################*/

void SPAN_LOAD_ROTOR_DATA::CalculateAverageForcesAndMoments(void)
{

    int i, j;

    for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {

             Span_Cxo_[i] = 0.;
             Span_Cyo_[i] = 0.; 
             Span_Czo_[i] = 0.; 
                          
              Span_Cx_[i] = 0.;
              Span_Cy_[i] = 0.; 
              Span_Cz_[i] = 0.; 
             
            Span_Cmxo_[i] = 0.;
            Span_Cmyo_[i] = 0.; 
            Span_Cmzo_[i] = 0.; 
                                       
             Span_Cmx_[i] = 0.;
             Span_Cmy_[i] = 0.; 
             Span_Cmz_[i] = 0.; 
                          
              Span_Cl_[i] = 0.;
              Span_Cd_[i] = 0.; 
              Span_Cw_[i] = 0.;    
                          
             Span_Cno_[i] = 0.;
             Span_Cso_[i] = 0.;
             Span_Cto_[i] = 0.;
             Span_Cqo_[i] = 0.;
             Span_Cpo_[i] = 0.;
                          
              Span_Cn_[i] = 0.;
              Span_Cs_[i] = 0.;
              Span_Ct_[i] = 0.;
              Span_Cq_[i] = 0.;
              Span_Cp_[i] = 0.;
                            
            XYZ_QC_[0][i] = 0.;
            XYZ_QC_[1][i] = 0.;
            XYZ_QC_[2][i] = 0.;
                          
                 Time_[i] = 0.;
                          
        RotationAngle_[i] = 0.;
                                   
               Span_S_[i] = 0.;
            Span_Area_[i] = 0.;
           Span_Chord_[i] = 0.;
       Local_Velocity_[i] = 0.;
       
    }
    
    for ( j = 1 ; j <= ActualTimeSamples_ ; j++ ) {
       
       for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {
   
                Span_Cxo_[i] +=       Span_Cxo_[j*NumberOfSpanStations_ + i];
                Span_Cyo_[i] +=       Span_Cyo_[j*NumberOfSpanStations_ + i];
                Span_Czo_[i] +=       Span_Czo_[j*NumberOfSpanStations_ + i];
                               
                 Span_Cx_[i] +=        Span_Cx_[j*NumberOfSpanStations_ + i];
                 Span_Cy_[i] +=        Span_Cy_[j*NumberOfSpanStations_ + i];
                 Span_Cz_[i] +=        Span_Cz_[j*NumberOfSpanStations_ + i];
                              
                Span_Cmx_[i] +=       Span_Cmx_[j*NumberOfSpanStations_ + i];
                Span_Cmy_[i] +=       Span_Cmy_[j*NumberOfSpanStations_ + i];
                Span_Cmz_[i] +=       Span_Cmz_[j*NumberOfSpanStations_ + i];
                
                 Span_Cl_[i] +=     Span_Cl_[j*NumberOfSpanStations_ + i];
                 Span_Cd_[i] +=     Span_Cd_[j*NumberOfSpanStations_ + i];
                 Span_Cw_[i] +=     Span_Cw_[j*NumberOfSpanStations_ + i];
                              
                Span_Cno_[i] +=       Span_Cno_[j*NumberOfSpanStations_ + i];
                Span_Cso_[i] +=       Span_Cso_[j*NumberOfSpanStations_ + i];
                Span_Cto_[i] +=       Span_Cto_[j*NumberOfSpanStations_ + i];
                Span_Cqo_[i] +=       Span_Cqo_[j*NumberOfSpanStations_ + i];
                Span_Cpo_[i] +=       Span_Cpo_[j*NumberOfSpanStations_ + i];
                                               
                 Span_Cn_[i] +=        Span_Cn_[j*NumberOfSpanStations_ + i]; 
                 Span_Cs_[i] +=        Span_Cs_[j*NumberOfSpanStations_ + i];
                 Span_Ct_[i] +=        Span_Ct_[j*NumberOfSpanStations_ + i];
                 Span_Cq_[i] +=        Span_Cq_[j*NumberOfSpanStations_ + i]; 
                 Span_Cp_[i] +=        Span_Cp_[j*NumberOfSpanStations_ + i]; 
                                                
                  Span_S_[i] +=         Span_S_[j*NumberOfSpanStations_ + i];     
               Span_Area_[i] +=      Span_Area_[j*NumberOfSpanStations_ + i];  
              Span_Chord_[i] +=     Span_Chord_[j*NumberOfSpanStations_ + i];   
          Local_Velocity_[i] += Local_Velocity_[j*NumberOfSpanStations_ + i]; 
          
       }
       
    }

    for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {

             Span_Cxo_[i] /= ActualTimeSamples_;
             Span_Cyo_[i] /= ActualTimeSamples_;
             Span_Czo_[i] /= ActualTimeSamples_;
                            
              Span_Cx_[i] /= ActualTimeSamples_;
              Span_Cy_[i] /= ActualTimeSamples_;
              Span_Cz_[i] /= ActualTimeSamples_;
                            
             Span_Cmx_[i] /= ActualTimeSamples_;
             Span_Cmy_[i] /= ActualTimeSamples_;
             Span_Cmz_[i] /= ActualTimeSamples_;
              
              Span_Cl_[i] /= ActualTimeSamples_;
              Span_Cd_[i] /= ActualTimeSamples_;
              Span_Cw_[i] /= ActualTimeSamples_;
                 
             Span_Cno_[i] /= ActualTimeSamples_;
             Span_Cso_[i] /= ActualTimeSamples_;
             Span_Cto_[i] /= ActualTimeSamples_;
             Span_Cqo_[i] /= ActualTimeSamples_;
             Span_Cpo_[i] /= ActualTimeSamples_;
              
              Span_Cn_[i] /= ActualTimeSamples_;
              Span_Cs_[i] /= ActualTimeSamples_;
              Span_Ct_[i] /= ActualTimeSamples_;
              Span_Cq_[i] /= ActualTimeSamples_;
              Span_Cp_[i] /= ActualTimeSamples_;
                                  
               Span_S_[i] /= ActualTimeSamples_;
            Span_Area_[i] /= ActualTimeSamples_;
           Span_Chord_[i] /= ActualTimeSamples_;
       Local_Velocity_[i] /= ActualTimeSamples_;
       
    }
    
}
 
/*##############################################################################
#                                                                              #
#                      SPAN_LOAD_ROTOR_DATA ZeroForcesAndMoments               #
#                                                                              #
##############################################################################*/

void SPAN_LOAD_ROTOR_DATA::ZeroForcesAndMoments(void)
{

    int i;
    
printf("ActualTimeSamples_: %d \n",ActualTimeSamples_);
printf("NumberOfTimeSamples_: %d \n",NumberOfTimeSamples_);fflush(NULL);
    for ( i = 0 ; i <= NumberOfSpanStations_*MAX(ActualTimeSamples_,NumberOfTimeSamples_) ; i++ ) {

             Span_Cxo_[i] = 0.;
             Span_Cyo_[i] = 0.; 
             Span_Czo_[i] = 0.; 
                           
              Span_Cx_[i] = 0.;
              Span_Cy_[i] = 0.; 
              Span_Cz_[i] = 0.; 
                           
             Span_Cmx_[i] = 0.;
             Span_Cmy_[i] = 0.; 
             Span_Cmz_[i] = 0.; 
             
              Span_Cl_[i] = 0.;
              Span_Cd_[i] = 0.; 
              Span_Cw_[i] = 0.;    
             
             Span_Cno_[i] = 0.;
             Span_Cso_[i] = 0.;
             Span_Cto_[i] = 0.;
             Span_Cqo_[i] = 0.;
             Span_Cpo_[i] = 0.;
                        
              Span_Cn_[i] = 0.;
              Span_Cs_[i] = 0.;
              Span_Ct_[i] = 0.;
              Span_Cq_[i] = 0.;
              Span_Cp_[i] = 0.;
                            
            XYZ_QC_[0][i] = 0.;
            XYZ_QC_[1][i] = 0.;
            XYZ_QC_[2][i] = 0.;
            
                 Time_[i] = 0.;
       
        RotationAngle_[i] = 0.;
                                   
               Span_S_[i] = 0.;
            Span_Area_[i] = 0.;
           Span_Chord_[i] = 0.;
       Local_Velocity_[i] = 0.;
       
    }
    
    ActualTimeSamples_ = 0;
  
}









