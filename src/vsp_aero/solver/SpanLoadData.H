//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef SPAN_LOAD_DATA_H
#define SPAN_LOAD_DATA_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.H"

class SPAN_LOAD_DATA {

private:

    int SurfaceID_;
    
    // Spanwise forces and moments, for wings and rotors
    
    int NumberOfSpanStations_;
    int NumberOfTimeSamples_;
    int ActualTimeSamples_;
    
    double *Span_Cno_; // Local viscous 2d normal     force coefficient
    double *Span_Cso_; // Local viscous 2d tangential force coefficient
    double *Span_Cto_; // Local viscous 2d thrust     force coefficient
    double *Span_Cqo_; // Local viscous 2d torque           coefficient   
    double *Span_Cpo_; // Local viscous 2d power            coefficient   

    double *Span_Cn_;  // Local inviscid 2d normal     force coefficient
    double *Span_Cs_;  // Local inviscid 2d tangential force coefficient    
    double *Span_Ct_;  // Local inviscid 2d thrust     force coefficient
    double *Span_Cq_;  // Local inviscid 2d torque           coefficient    
    double *Span_Cp_;  // Local inviscid 2d power            coefficient
    
    double *Time_;
    double *XYZ_QC_[3];
    double *RotationAngle_;
    double *Span_S_;
    double *Span_Area_;
    double *Span_Chord_;
    double *Local_Velocity_;

public:

    SPAN_LOAD_DATA(void);
   ~SPAN_LOAD_DATA(void);
    SPAN_LOAD_DATA(const SPAN_LOAD_DATA &SPAN_LOAD_DATA);

    SPAN_LOAD_DATA& operator=(const SPAN_LOAD_DATA &SPAN_LOAD_DATA);
    
    int &SurfaceID(void) { return SurfaceID_; };

    // Spanwise forces and moments, for wings and rotors
    
    void SetNumberOfSpanStations(int NumberOfSpanStations) { NumberOfSpanStations_ = NumberOfSpanStations; };
    void SetNumberOfNumberOfTimeSamples(int NumberOfTimeSamples) { NumberOfTimeSamples_ = NumberOfTimeSamples; };
    
    void SizeSpanLoadingList(void);
    
    int NumberOfSpanStations(void) { return NumberOfSpanStations_; };
    int NumberOfTimeSamples(void) { return NumberOfTimeSamples_; };
    
    // Access span loading data
    
    double &Span_Cno(int TimeStep, int SpanStation) { return Span_Cno_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double &Span_Cso(int TimeStep, int SpanStation) { return Span_Cso_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double &Span_Cto(int TimeStep, int SpanStation) { return Span_Cto_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double &Span_Cqo(int TimeStep, int SpanStation) { return Span_Cqo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double &Span_Cpo(int TimeStep, int SpanStation) { return Span_Cpo_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
           
    double &Span_Cn(int TimeStep, int SpanStation) { return Span_Cn_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double &Span_Cs(int TimeStep, int SpanStation) { return Span_Cs_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double &Span_Ct(int TimeStep, int SpanStation) { return Span_Ct_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double &Span_Cq(int TimeStep, int SpanStation) { return Span_Cq_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double &Span_Cp(int TimeStep, int SpanStation) { return Span_Cp_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
 
    double &X_QC(int TimeStep, int SpanStation) { return XYZ_QC_[0][TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double &Y_QC(int TimeStep, int SpanStation) { return XYZ_QC_[1][TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double &Z_QC(int TimeStep, int SpanStation) { return XYZ_QC_[2][TimeStep*NumberOfSpanStations_ + SpanStation]; };
    
    double &RotationAngle(int TimeStep, int SpanStation) { return RotationAngle_[TimeStep*NumberOfSpanStations_ + SpanStation]; };

    double &Time(int TimeStep, int SpanStation) { return Time_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
   
    double         &Span_S(int TimeStep, int SpanStation) { return         Span_S_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double      &Span_Area(int TimeStep, int SpanStation) { return      Span_Area_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double     &Span_Chord(int TimeStep, int SpanStation) { return     Span_Chord_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
    double &Local_Velocity(int TimeStep, int SpanStation) { return Local_Velocity_[TimeStep*NumberOfSpanStations_ + SpanStation]; };
                           
    // Access averaged span loading data
    
    double &Span_Cno(int SpanStation) { return Span_Cno_[SpanStation]; };
    double &Span_Cso(int SpanStation) { return Span_Cso_[SpanStation]; };
    double &Span_Cto(int SpanStation) { return Span_Cto_[SpanStation]; };
    double &Span_Cqo(int SpanStation) { return Span_Cqo_[SpanStation]; };
    double &Span_Cpo(int SpanStation) { return Span_Cpo_[SpanStation]; };
           
    double &Span_Cn(int SpanStation) { return Span_Cn_[SpanStation]; };
    double &Span_Cs(int SpanStation) { return Span_Cs_[SpanStation]; };
    double &Span_Ct(int SpanStation) { return Span_Ct_[SpanStation]; };
    double &Span_Cq(int SpanStation) { return Span_Cq_[SpanStation]; };
    double &Span_Cp(int SpanStation) { return Span_Cp_[SpanStation]; };
 
    double         &Span_S(int SpanStation) { return         Span_S_[SpanStation]; };
    double      &Span_Area(int SpanStation) { return      Span_Area_[SpanStation]; };
    double     &Span_Chord(int SpanStation) { return     Span_Chord_[SpanStation]; };
    double &Local_Velocity(int SpanStation) { return Local_Velocity_[SpanStation]; };
        
    int &ActualTimeSamples(void) { return ActualTimeSamples_; };

    void CalculateAverageForcesAndMoments(void);

    void ZeroForcesAndMoments(void);

};

#endif
