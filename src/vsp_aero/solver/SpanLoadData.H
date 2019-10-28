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
    
    void SizeList(int NumberOfSpanStations);
    
    int NumberOfSpanStations(void) { return NumberOfSpanStations_; };
    
    double &Span_Cno(int i) { return Span_Cno_[i]; };
    double &Span_Cso(int i) { return Span_Cso_[i]; };
    double &Span_Cto(int i) { return Span_Cto_[i]; };
    double &Span_Cqo(int i) { return Span_Cqo_[i]; };
    double &Span_Cpo(int i) { return Span_Cpo_[i]; };
           
    double &Span_Cn(int i) { return Span_Cn_[i]; };
    double &Span_Cs(int i) { return Span_Cs_[i]; };
    double &Span_Ct(int i) { return Span_Ct_[i]; };
    double &Span_Cq(int i) { return Span_Cq_[i]; };
    double &Span_Cp(int i) { return Span_Cp_[i]; };
 
    double &Span_S(int i) { return Span_S_[i]; };
    double &Span_Area(int i) { return Span_Area_[i]; };
    double &Span_Chord(int i) { return Span_Chord_[i]; };
    double &Local_Velocity(int i) { return Local_Velocity_[i]; };
    
    int &NumberOfTimeSamples(void) { return NumberOfTimeSamples_; };

    void CalculateAverageForcesAndMoments(void);


};

#endif
