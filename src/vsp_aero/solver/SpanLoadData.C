//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "SpanLoadData.H"

/*##############################################################################
#                                                                              #
#                          SPAN_LOAD_DATA Constructor                          #
#                                                                              #
##############################################################################*/

SPAN_LOAD_DATA::SPAN_LOAD_DATA(void)
{

    NumberOfSpanStations_ = 0;

    NumberOfTimeSamples_ = 0;

    ActualTimeSamples_ = 0;
    
    SurfaceID_ = 0;
    
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
#                            SPAN_LOAD_DATA Destructor                         #
#                                                                              #
##############################################################################*/

SPAN_LOAD_DATA::~SPAN_LOAD_DATA(void)
{

    if ( Span_Cno_ != NULL ) delete [] Span_Cno_;
    if ( Span_Cso_ != NULL ) delete [] Span_Cso_;  
    if ( Span_Cto_ != NULL ) delete [] Span_Cto_;
    if ( Span_Cqo_ != NULL ) delete [] Span_Cqo_;
    if ( Span_Cpo_ != NULL ) delete [] Span_Cpo_; 
                          
    if ( Span_Cn_  != NULL ) delete [] Span_Cn_;
    if ( Span_Cs_  != NULL ) delete [] Span_Cs_;
    if ( Span_Ct_  != NULL ) delete [] Span_Ct_;  
    if ( Span_Cq_  != NULL ) delete [] Span_Cq_;
    if ( Span_Cp_  != NULL ) delete [] Span_Cp_;  

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
#                           SPAN_LOAD_DATA Copy                                #
#                                                                              #
##############################################################################*/

SPAN_LOAD_DATA::SPAN_LOAD_DATA(const SPAN_LOAD_DATA &SPAN_LOAD_DATA)
{

    // Use operator =

    (*this)= SPAN_LOAD_DATA;

}

/*##############################################################################
#                                                                              #
#                          SPAN_LOAD_DATA Operator =                           #
#                                                                              #
##############################################################################*/

SPAN_LOAD_DATA &SPAN_LOAD_DATA::operator=(const SPAN_LOAD_DATA &SpanLoadData)
{

    int i;
    
    SurfaceID_ = SpanLoadData.SurfaceID_;

    if ( Span_Cno_ != NULL ) delete [] Span_Cno_;
    if ( Span_Cso_ != NULL ) delete [] Span_Cso_;  
    if ( Span_Cto_ != NULL ) delete [] Span_Cto_;
    if ( Span_Cqo_ != NULL ) delete [] Span_Cqo_;
    if ( Span_Cpo_ != NULL ) delete [] Span_Cpo_; 
                          
    if ( Span_Cn_  != NULL ) delete [] Span_Cn_;
    if ( Span_Cs_  != NULL ) delete [] Span_Cs_;
    if ( Span_Ct_  != NULL ) delete [] Span_Ct_;  
    if ( Span_Cq_  != NULL ) delete [] Span_Cq_;
    if ( Span_Cp_  != NULL ) delete [] Span_Cp_;  

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
    
    Span_Cno_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cso_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cto_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cqo_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cpo_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
                                                                         
    Span_Cn_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cs_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Ct_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cq_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cp_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cp_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    
    Time_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    XYZ_QC_[0] = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    XYZ_QC_[1] = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    XYZ_QC_[2] = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    RotationAngle_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_S_         = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Area_      = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Chord_     = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Local_Velocity_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    for ( i = 1 ; i <= NumberOfSpanStations_*MAX(ActualTimeSamples_,NumberOfTimeSamples_) ; i++ ) {

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
#                  SPAN_LOAD_DATA SizeSpanLoadingList                          #
#                                                                              #
##############################################################################*/

void SPAN_LOAD_DATA::SizeSpanLoadingList(void)
{

    int i;
    
    if ( Span_Cno_ != NULL ) delete [] Span_Cno_;
    if ( Span_Cso_ != NULL ) delete [] Span_Cso_;  
    if ( Span_Cto_ != NULL ) delete [] Span_Cto_;
    if ( Span_Cqo_ != NULL ) delete [] Span_Cqo_;
    if ( Span_Cpo_ != NULL ) delete [] Span_Cpo_; 
                          
    if ( Span_Cn_  != NULL ) delete [] Span_Cn_;
    if ( Span_Cs_  != NULL ) delete [] Span_Cs_;
    if ( Span_Ct_  != NULL ) delete [] Span_Ct_;  
    if ( Span_Cq_  != NULL ) delete [] Span_Cq_;
    if ( Span_Cp_  != NULL ) delete [] Span_Cp_;  

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
       
       printf("Error in setting up storage for span loading information! \n");
       
       printf("NumberOfSpanStations_: %d \n",NumberOfSpanStations_);
       
       fflush(NULL);exit(1);
       
    }

    ActualTimeSamples_ = 0;

    Span_Cno_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cso_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cto_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cqo_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cpo_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
                                                                         
    Span_Cn_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cs_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Ct_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cq_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cp_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Cp_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    Time_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    
    XYZ_QC_[0] = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    XYZ_QC_[1] = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    XYZ_QC_[2] = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    RotationAngle_  = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_S_         = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Area_      = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Span_Chord_     = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];
    Local_Velocity_ = new double[NumberOfSpanStations_*(NumberOfTimeSamples_ + 5)];

    for ( i = 0 ; i <= NumberOfSpanStations_*NumberOfTimeSamples_ ; i++ ) {

       Span_Cno_[i]       = 0.;
       Span_Cso_[i]       = 0.;    
       Span_Cto_[i]       = 0.;                                           
       Span_Cqo_[i]       = 0.;
       Span_Cpo_[i]       = 0.;
       
       Span_Cn_[i]        = 0.;
       Span_Cs_[i]        = 0.;    
       Span_Ct_[i]        = 0.;                                           
       Span_Cq_[i]        = 0.;
       Span_Cp_[i]        = 0.;

       XYZ_QC_[0][i]      = 0.;
       XYZ_QC_[1][i]      = 0.;
       XYZ_QC_[2][i]      = 0.;
       
       Time_[i]           = 0.;
       
       RotationAngle_[i]  = 0.;

       Span_S_[i]         = 0.;
       Span_Area_[i]      = 0.;
       Span_Chord_[i]     = 0.;
       Local_Velocity_[i] = 0.;
       
    }

}

/*##############################################################################
#                                                                              #
#               SPAN_LOAD_DATA CalculateAverageForcesAndMoments                #
#                                                                              #
##############################################################################*/

void SPAN_LOAD_DATA::CalculateAverageForcesAndMoments(void)
{

    int i, j;

    for ( i = 1 ; i <= NumberOfSpanStations_ ; i++ ) {

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
#                      SPAN_LOAD_DATA ZeroForcesAndMoments                     #
#                                                                              #
##############################################################################*/

void SPAN_LOAD_DATA::ZeroForcesAndMoments(void)
{

    int i;

    for ( i = 0 ; i <= NumberOfSpanStations_*MAX(ActualTimeSamples_,NumberOfTimeSamples_) ; i++ ) {

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









